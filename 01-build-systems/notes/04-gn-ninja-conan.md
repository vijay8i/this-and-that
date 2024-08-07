# Is there such a thing as a perfect build system? (Part IV)

(thoughts in progress...)

In Part III, I concluded that learning _gn_ is a better investment of time than
_meson_. Learning is not free and mastering anything takes time. To be clear, if
you are comfortable with _cmake_ or _meson_ then good for you, keep using it
&ndash; after all, perfection is in the eye of the beholder.

My search for a better meta build tool started only because it's been a while (a
lot while) since I messed around with _cmake_; in that time, _cmake_ itself has
changed (for the better). So, I am faced with the choice of dropping my old ways
of using _cmake_ or learn the modern ways of using it. Old habits die hard, and,
in my opinion, the best way to kill them fast is to change the environment.

So, that is my excuse to look for a new tool to learn, with the hope that it
avoids the kinks and warts of its alternatives.

The objective today is to build the `ad-hoc` project using _gn_ and _ninja_.
From constructing the `simple` project, I know how to use the dependencies once
I know where they are located. But I have no clue how to make _gn_ fetch remote
dependencies and build them, which was effortless with _meson_.

However, with _meson_ there were two distinct steps to build the `ad-hoc`
project. The first step was to operate within `3p` folder to fetch and build the
dependencies, and then switch to `experiments/using-uvw` folder. It would be
nice if I could do it in one step from within the project root. I think it is
possible with _meson_, but my understanding is that it requires the use of
_subproject_, which I avoided using &ndash; simply because I decided, for no
good reason, to hold the folder structure of `experiments/using-uvw` as
immutable.

A quick recap of the properties that I am looking for in my build system.

- [ ] the tool(s) can be used for a wide range of projects.
- [ ] the evolution of the tool should not be linked to a specific project.
- [ ] seperation of concerns.
- [ ] fast, free, extensible, and open source.
- [ ] syntax should not be headache-inducing.
- [ ] most importantly the tool should not dictate how to layout my code

So far, _gn_ and _ninja_ check off these boxes. But I still have to figure out
how to fetch third-party dependencies. There are tools available under
`depot_tools` umbrella from Google for fetching and syncing source code, which
are tightly coupled to the needs of Google's own projects &mdash; and so do not
meet my requirements.

Therefore, I decided to go with `conan` for fetching third-party dependencies. I
have no idea how that is going to turn out. But we have to start somewhere, and
so here is the plan:

- Figure out how to fetch third-party dependencies (`uv` and `uvw`).
- Integrate the previous step using _gn_ `actions`(?).
- Finally _use_ the `third-party` dependencies.

## Conan detour

Steps to install, configure and get going with `conan`.

### Install using `pip` and create a profile

```bash
$ pip install --user conan
$ conan --version
Conan version 2.4.1

$ conan profile detect --force
detect_api: Found clang 18.1
detect_api: clang>=8, using the major as version

Detected profile:
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=gnu17
compiler.libcxx=libc++
compiler.version=18
os=Macos

WARN: This profile is a guess of your environment, please check it.
WARN: Defaulted to cppstd='gnu17' for apple-clang.
WARN: The output of this command is not guaranteed to be stable and can change in future Conan versions.
WARN: Use your own profile files for stability.
Saving detected profile to ~/.conan2/profiles/default
```

### Edit default profile to support the latest cppstd

```bash
# On Mac
$ sed -i '' 's/cppstd=gnu17/cppstd=c\+\+23/g' ~/.conan2/profiles/default
# On Linux
# $ sed -i 's/cppstd=gnu17/cppstd=c\+\+23/g' ~/.conan2/profiles/default
```

### Edit settings.yml to add c++23 and c23 in the supported compiler list

Find compiler/clang section in ~/.conan2/settings.yml

- add c++23 to "cppstd" value list,
- add c23 to "cstd" value list

### Fetch dependencies and generate package dependency info

There are two options to work with third-party dependencies. Either use them
from where they are installed (in `conan`'s cache) or export them into your
source tree.

To get started for either case you need `conanfile.txt` or `conanfile.py`.

For our `ad-hoc` project we need `libuv` and `uvw`. We specifiy these
requirements like so in `conanfile.txt`:

```ini
[requires]
  libuv/1.48.0
  uvw/3.4.0

[generators]
PkgConfigDeps
```

And then let `conan` do its thing. You can read more about creating a deploy of
dependencies for developer use [here](^1). Since I wanted to replicate what I
did with `meson` I decided to use the deployer option.

> [!NOTE]\
> The `direct_deploy` and `full_deploy` option at the time of writing doesn't
> generate `*.pc` files with the correct path information. It works fine with
> the default `install` (which is to use the dependencies from Conan's cache).
> You can find more details on the nature of the issue and its resolution
> [here](^2). For now I just hacked my local installation to fix the _bug_(?).

```bash
# verify we have nothing installed to start with
$ conan list '*'
Found 0 pkg/version recipes matching * in local cache
Local Cache
  WARN: There are no matching recipe references

# fetch (into cache), build and install dependencies into third-party folder
$ conan install . --output-folder=third-party  --deployer=direct_deploy --deployer-folder=third-party --build=missing

# verify we have the dependencies fetched and built
conan list '*'
Found 2 pkg/version recipes matching * in local cache
Local Cache
  libuv
    libuv/1.48.0
  uvw
    uvw/3.4.0
```

If all goes well, you should have a `third-party` folder populated like so:

```bash
$ tree third-party -L 3
third-party
├── direct_deploy
│   ├── libuv
│   │   ├── conaninfo.txt
│   │   ├── conanmanifest.txt
│   │   ├── include
│   │   ├── lib
│   │   └── licenses
│   └── uvw
│       ├── conaninfo.txt
│       ├── conanmanifest.txt
│       ├── include
│       └── licenses
├── libuv-static.pc
└── uvw.pc

9 directories, 6 files
```

> [!TIP]\
> If you are following along, your folder might contain a few extraneous scripts
> that are generated by _conan_. I hacked my local installation to suppress the
> generation of these files since I didn't find them to be useful or haven't
> figured out when I would need them, in the context of _designing_ a build
> system using `gn`+`conan`+`ninja`. The details and motivation for this hack
> can be found [here](^3).

You can remove unwanted dependencies, as is the case here since I have imported
the dependencies into the project's source tree. In fact, let me do just that to
ensure that I have a `hermitically sealed` build.

```bash
$ conan remove '*' 
Found 2 pkg/version recipes matching * in local cache
Remove the recipe and all the packages of 'libuv/1.48.0#173dd89ec67daf9eee376829a9297da5'? (yes/no): y
Remove the recipe and all the packages of 'uvw/3.4.0#b5a165daeb1129086cb502f3fd429b90'? (yes/no): y
Remove summary:
Local Cache
  libuv/1.48.0#173dd89ec67daf9eee376829a9297da5: Removed recipe and all binaries
  uvw/3.4.0#b5a165daeb1129086cb502f3fd429b90: Removed recipe and all binaries

$ conan list '*'
Found 0 pkg/version recipes matching * in local cache
Local Cache
  WARN: There are no matching recipe references
```

## Where am I?

This section is a postscript of my journey for the last couple of days in trying
to build my `ad-hoc` project. It was not as easy as it was with _meson_! There
were times when I was ready to eat my words and get back to the trodden path of
comfort that _cmake_ provides; or hear myself saying that I could salvage this
quest by settling with _meson_, and go about claiming to have found something
better than _cmake_.

Most answers to the question, "Is there a better way," often have this struggle
built in. You have to climb the hill without knowing if you are going to find a
better way. So that's where I am. I am going to continue to climb the `gn` +
`ninja` + `conan` hill; yes, there were way more pitfalls and detours on the
journey so far than I had with _meson_; but my intuition says this hill is worth
climbing.

To be continued...

<!-- short links -->

[^1]: https://docs.conan.io/2/examples/extensions/deployers/dev/development_deploy.html#examples-extensions-builtin-deployers-development

[^2]: https://github.com/conan-io/conan/issues/16543

[^3]: https://github.com/conan-io/conan/issues/16547

[^4]: https://vcmi.eu/developers/Conan/

# References

- [Using GN build](https://docs.google.com/presentation/d/15Zwb53JcncHfEwHpnG_PoIbbzQ3GQi_cpujYwbpcbZo/edit#slide=id.g11ee2284cd_0_0)
- [Guide to pkg-config](https://people.freedesktop.org/~dbn/pkg-config-guide.html)
- [Conan, software package manager for C and C++ developers](https://conan.io/)
