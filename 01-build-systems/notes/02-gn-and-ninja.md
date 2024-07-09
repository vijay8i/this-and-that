# Is there such a thing as a perfect build system? (Part II)

(thoughts in progress...)

In Part I, I explored `meson` with `ninja` in my quest to find the perfect build
system that works for me. To be clear, there was no audition done for _ninja_;
it was simply accepted for the role of chomping through a build file (reductive
and potentially insulting simplification here). I mean when the scope is
limited, there's fewer ways to go screw up; _ninja_ just does its job and stays
out of your way. Good for _ninja_ and that's all there is to be said about
_ninja_.

The search &mdash;focus for this _series_&mdash; is to find the right artist to
take on the role of a `meta build tool`. The artists that made it to the final
list were `cmake`, `meson`, and `gn`. Sorry to say but `cmake` was sent home out
of _prejudice_, and without a doubt I know that decision might very well come
back and bite me &ndash; someday.

The role of a _meta build tool_ is complex and multidimensional. To _play_ it
well requires knowledge of operating systems, compilers, programming languages,
linkers, and tools. Which makes it hard to appease all developers of diverse
experience levels.

I left Part I concluding that `meson` is not for me; and that I have to explore
`gn` + `ninja` combo before figuring out which way to go. That is the topic for
today.

## Learning from experience

Getting things working with _meson_ for an ad-hoc project structure turned into
a wee bit of an adventure &ndash; as expected when diving in sans planning. I
did that because at the surface level I was able to map the the _DSL_ of _meson_
to that of _cmake_ which I had experience with; how hard could that be
:roll_eyes:.

On the other hand, _gn_'s DSL at a first glance looked different enough for me
to take a pause and start with something `simple` to follow along till I got the
hang of it. I started with the [quick start document](^1) and worked my way
through to create _simple_ project structure that looks as below.

```bash
simple
├── .gn
├── BUILD.gn
├── build
│   ├── BUILD.gn
│   ├── BUILDCONFIG.gn
│   └── toolchain
│       └── BUILD.gn
└── src
   ├── app
   │   ├── BUILD.gn
   │   └── main.cc
   └── lib
       ├── BUILD.gn
       ├── simple-stats.cc
       └── simple-stats.hh
```

The objective is to build a static and shared library out of `lib` folder and a
statically linked and dynamically linked executable out of `app` folder. At the
end of this exercise I had a map of _gn_ concepts that I could relate to similar
kind in _cmake_ and _meson_.

Broadly, the process is to create a directory structure that breaks the system
into subsystems, functional blocks (features) and modules (components); and then
describe what each folder contains in a `BUILD.gn` file in that folder. Each
target (defined in _BUILD.gn_) can be referenced as a dependency by others, and
the target can declare its own dependencies. At the top level, create one or
more `group` of features and declare the dependencies for each. The `group` name
can also be a target that can be used to direct `ninja` what to _build_.

> [!TIP]\
> Remember to design your build like code. &mdash; Brett Wilson

The first thing I took note of and appreciated is that _gn_ automatically looks
for and picks up the values in `.gn` file unlike _meson_'s `--native-file`
option. In fact it provides a help message that was somehow reassuring (to me).

```bash
$ gn gen out
ERROR Can't find source root.
I could not find a ".gn" file in the current directory or any parent,
and the --root command-line argument was not specified.
```

After getting required build _scripts_ in place, I let _gn_ generate the build
files for _ninja_. Followed by unleashing _ninja_ do its job.

```bash
# from the root of `simple`
$ gn gen out/simple
$ ninja -C out/simple
```

With the basic objective met, I felt adventurous. So I ventured to figure out
how to keep my _BUILD.gn_ files `DRY`. Here is what I mean; the first iteration
of my `app`'s _BUILD.gn_ looked as below:

```lua
cflags = []
cflags += [ "-std=c++23" ]
lib_dirs = []
lib_dirs += [ "/opt/local/libexec/llvm-18/lib" ]
ldflags = []
ldflags += [ "-Wl,-rpath,/opt/local/libexec/llvm-18/lib" ]

executable("app-static") {
  sources = [ "main.cc" ]
  deps = [ "//src/lib:ss-shared" ]
  include_dirs = [ "../lib" ]
}

executable("app-shared") {
  sources = [ "main.cc" ]
  deps = [ "//src/lib:ss-static" ]
  include_dirs = [ "../lib" ]
}
```

> [!TIP]\
> PS: found a neat trick to highlight _gn_ syntax by setting the fence block's
> language to be `lua`. :shushing_face:

Clearly having to repeat `cflags`, `lib_dirs`, `ld_flags` and more for each
target is not _DRY_ at all. Intuitively, I thought it should be as simple as
moving those to my `group` and be done; that doesn't work.

Turns out the solution to be DRY is to define a `config` in a higher level or
more generic target; and then use that config in each target that requires it. I
can go along with that &ndash; explicit is better than implicit (provided it is
logical and consistent). Here is how that looks like.

In `simple/BUILD.gn` we define our `bleeding-edge` config:

```lua
# Define a common config
config("bleeding-edge") {
  cflags = [ "-std=c++23" ]
  lib_dirs = [ "/opt/local/libexec/llvm-18/lib" ]
  ldflags = [ "-Wl,-rpath,/opt/local/libexec/llvm-18/lib" ]
}
```

And elsewhere (e.g`simple/src/app/BUILD.gn`) we use `bleeding-edge` config like
so:

```lua
executable("app-static") {
  sources = [ "main.cc" ]
  deps = [ "//src/lib:ss-shared" ]
  include_dirs = [ "../lib" ]
  configs += [ "//:bleeding-edge" ]
}
```

Having to repeat `configs += ["//:bleeding-edge/"]` feels error prone. So the
next step is to see if we can define `bleeding-edge` config in `build/BUILD.gn`
and let `build/BUILDCONFIG.gn` make use of it in `set_defaults` like so:

```lua
set_defaults("executable") {
  ...
  # Add "bleeding-edge" to defaults
  configs += [ "//build:bleeding-edge" ]
}

set_defaults("static_library") {
  ...
  # Add "bleeding-edge" to defaults
  configs += [ "//build:bleeding-edge" ]
}

set_defaults("shared_library") {
  ...
  # Add "bleeding-edge" to defaults
  configs += [ "//build:bleeding-edge" ]
}
```

It works but didn't feel _DRY_ enough. In _gn_ you can declare and initialize
variables outside a block, and use them inside a block. So instead of adding
_//build:bleeding-edge_ to all target types in each block, we can do this:

```lua
shared_binary_target_configs = [
  "//build:compiler_defaults",
  "//build:bleeding-edge",
]

set_defaults("executable") {
  configs = shared_binary_target_configs
  configs += [ "//build:executable_ldconfig" ]
}

set_defaults("static_library") {
  configs = shared_binary_target_configs
}

set_defaults("shared_library") {
  configs = shared_binary_target_configs
}

set_defaults("source_set") {
  configs = shared_binary_target_configs
}
```

Compared to _meson_, the nuts and bolts approach of _gn_ is beginning to grow on
me.

The last bit to check on is to ensure that the app does link to the correct
library; i.e., static app target should use the static library and the
dynamically linked app target should use the shared library. This is not a _gn_
issue per se. It requires the developer to set things up correctly and document
the _-Ddefines_ the library user has to set in order to link correctly. Boring
stuff, but for completeness sake here is how that looks:

```C++
#if defined _WIN32 || defined __CYGWIN__
  #define STATS_API_Export __declspec(dllexport)
  #define STATS_API_Import __declspec(dllimport)
#elif __GNUC__ >= 4
  #define STATS_API_Export __attribute__ ((visibility ("default")))
  #define STATS_API_Import __attribute__ ((visibility ("default")))
#else
  #define STATS_API_Export
  #define STATS_API_Import
#endif

#if defined (STATS_API_BUILD_AS_STATIC_LIB)
# if !defined (STATS_API_IS_DLL)
#   define STATS_API_IS_DLL 0
# endif /* ! STATS_API_IS_DLL */
#else
# if !defined (STATS_API_IS_DLL)
#   define STATS_API_IS_DLL 1
# endif /* ! STATS_API_IS_DLL */
#endif /* STATS_API_BUILD_AS_STATIC_LIB */

#if defined (STATS_API_IS_DLL)
#  if (STATS_API_IS_DLL == 1)
#    if defined (STATS_API_BUILD_AS_SHARED_LIB)
#      define STATS_API STATS_API_Export
#    else
#      define STATS_API STATS_API_Import
#    endif /* STATS_API_BUILD_AS_SHARED_LIB */
#  else
#    define STATS_API
#  endif   /* ! STATS_API_IS_DLL == 1 */
#else
#  define STATS_API
#endif     /* STATS_API_IS_DLL */
```

Somewhere in the getting started documentation the library developer would ask
the user of the library to `-DSTATS_API_IS_DLL=1` to use the shared library, and
`-DSTATS_API_IS_DLL=0` to use the static library.

Since both the library developer and the user of the library is me, and since
this article is about using _gn_, let me just dump the `app` and `lib` targets
here so we can wrap up the day on a positive note.

First the root file `simple/BUILD.gn`...

```lua
group("simple") {
  deps = [
    "//src/app:app-shared",
    "//src/app:app-static",
    "//src/lib:ss-shared",
    "//src/lib:ss-static",
  ]
}
```

Next is to inform _gn_ where the the toolchain config in our bootstrap file
(`simple/.gn`):

```lua
buildconfig = "//build/BUILDCONFIG.gn"
```

We declare our two app targets in `simple/src/app/BUILD.gn`:

```lua
executable("app-static") {
  sources = [ "main.cc" ]
  deps = [ "//src/lib:ss-static" ]
  include_dirs = [ "../lib" ]

  defines = [ "STATS_API_IS_DLL=0" ]
}

executable("app-shared") {
  sources = [ "main.cc" ]
  deps = [ "//src/lib:ss-shared" ]
  include_dirs = [ "../lib" ]

  defines = [ "STATS_API_IS_DLL=1" ]
}
```

And declare our lib targets in `simple/src/lib/BUILD.gn`:

```lua
shared_library("ss-shared") {
  sources = [
    "simple-stats.cc"
  ]
  defines = [ "STATS_API_BUILD_AS_SHARED_LIB" ]
}

static_library("ss-static") {
  sources = [
    "simple-stats.cc"
  ]
  defines = [ "STATS_API_BUILD_AS_STATIC_LIB" ]
}
```

Finally, we can verify that we got it right by using `otool`; here is the whole
session transcript:

```bash
$ gn gen out/simple
Done. Made 5 targets from 6 files in 5ms

$ ninja -C out/simple
ninja: Entering directory `out/simple'
[9/9] STAMP obj/simple.stamp

$ otool -L out/simple/app-static
out/simple/app-static:
        @rpath/libc++.1.dylib (compatibility version 1.0.0, current version 1.0.0)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1336.61.1)
        @rpath/libunwind.1.dylib (compatibility version 1.0.0, current version 1.0.0)

$ out/simple/app-static
Sum: 15
Average: 3.00
Median: 3.00

$ otool -L out/simple/app-shared
out/simple/app-shared:
        @executable_path/./libss-shared.so (compatibility version 0.0.0, current version 0.0.0)
        @rpath/libc++.1.dylib (compatibility version 1.0.0, current version 1.0.0)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1336.61.1)
        @rpath/libunwind.1.dylib (compatibility version 1.0.0, current version 1.0.0)

$ out/simple/app-shared
Sum: 15
Average: 3.00
Median: 3.00
```

Nice. With this initial experience I think I am ready to go the distance with
_gn_ and take on the `ad-hoc` project. Recall that I did not follow the
suggested guidelines of _meson_ to use _subprojects_ to set it up and that
generated some rant from me. In addition it has a few challenges like fetching
third party dependency (`uvw`) that in turn depends on `uv`, which _meson_
handled admirably.

To be continued...

<!-- short links -->

[^1]: https://gn.googlesource.com/gn/+/main/docs/quick_start.md

# References

- [Using GN build, Artisanl metabuild](https://docs.google.com/presentation/d/15Zwb53JcncHfEwHpnG_PoIbbzQ3GQi_cpujYwbpcbZo/edit#slide=id.g119d702868_0_12)
- [GN Reference](https://gn.googlesource.com/gn/+/master/docs/reference.md#buildargs)
- [Best practices for writing GN templates, Fuchsia](https://fuchsia.dev/fuchsia-src/development/build/build_system/best_practices_templates)
- [Using dependencies from Conan](https://vcmi.eu/developers/Conan/)
