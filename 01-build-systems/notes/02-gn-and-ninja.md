# Is there such a thing as a perfect build system? (Part II)
(thoughts in progress...)

In Part I, I explored `meson` with `ninja` in my quest to find the 
perfect build system that works for me. To be clear, there was no
audition done for *ninja*; it was simply accepted for the role of
chomping through a build file (reductive and potentially insulting
simplification here). I mean when the scope is limited, there's fewer
ways to go screw up; *ninja* just does its job and stays out of your
way. Good for *ninja* and that's all there is to be said about *ninja*.

The search &mdash; focus for this *series* &mdash; is to find the right
artist to take on the role of a `meta build tool`. That is a tough role
requiring real hardwork; the artists that made it to the final list were
`cmake`, `meson`, and `gn`. Sorry to say but `cmake` was sent home out
of *prejudice*, and without a doubt I know that decision might very 
well come back and bite me &ndash; someday.

The role of a *meta build tool* is complex and multidimensional. To 
*play* it well requires knowledge of operating systems, compilers,
programming languages, linkers, and tools. Which makes it hard to appease
all developers of diverse experience levels.

> I realize I was overly critical of *meson*; so fwiw and for the
> record, let me say that I respect how much one can accomplish
> using *meson* with just  a day's worth of RTFM (I know, I am a 
> slow reader).

I left Part I concluding that `meson` is not for me; and that I have
to explore `gn` + `ninja` combo someday in my quest to find the perfect
build system. And that someday is today.

## Learning from experience
Getting things working with *meson* for an ad-hoc project structure
turned out to be a wee bit of an adventure. That is to be expected
when diving in sans planning. I did that because at the surface level
I was able to map the the *DSL* of *meson* to that of *cmake* which I
had experience with; how hard could that be :roll_eyes:.

On the other hand, *gn*'s DSL at a first glance looked different enough
for me to start with something `simple` to follow along till I got the
hang of it. The *simple* project folder structure looks as below. The
objective is to build a static and shared library out of `lib` folder
and a statically linked and dynamically linked executable out of `app`
folder.

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

I started with the [quick start document](^1) and worked my way through.

The general concepts in *gn* are similar to *cmake* and *meson* &mdash; 
create a directory structure that breaks the system into subsystems, 
functional blocks (features) and modules (components); and then describe
what each folder contains in a `BUILD.gn` file in that folder. Each 
target (defined in *BUILD.gn*) can be referenced as a dependency by 
others, and the target can declare its own dependencies. At the top level,
create one or more `group` of features and declare the dependencies for
each. Btw, the `group` name becomes a target that can be used to direct
`ninja` what to build.

> Remember to design your build like code.
> &mdash; Brett Wilson

The first thing I took note of and appreciated is that *gn* automatically
looks for and picks up the values in `.gn` file unlike *meson*'s 
`--native-file` option. In fact it provides a help message that is 
somehow reassuring (to me).

[^1]: https://gn.googlesource.com/gn/+/main/docs/quick_start.md

```bash
$ gn gen out
ERROR Can't find source root.
I could not find a ".gn" file in the current directory or any parent,
and the --root command-line argument was not specified.
```

After getting required build files in place, we can fulfill our objective
by getting *gn* to generate the build files for *ninja*. And let *ninja*
do its job.

```bash
# from the root of `simple`
$ gn gen out/simple
$ ninja -C out/simple
```

Now that the basic objective has been met, I felt a bit more adventurous.
My first venturing on my own into the *gn* world was to figure out how
keep my *BUILD.gn* files `DRY`. Here is what I mean; the first iteration
of my `app`'s *BUILD.gn* looked as below:

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
> PS: found a neat trick to highlight *gn* syntax by setting the fence
> block's language to be `lua`. :shushing_face:

Clearly having to repeat `cflags`, `lib_dirs`, `ld_flags` and more for
each target is not *DRY* at all. Intuitively, I thought it should be
as simple as moving those to my `group` and be done; that doesn't work.

Turns out the solution to be DRY is to define a `config` in a higher
level or more generic target;  and then use that config in each target
that requires it. I can go along with that &ndash; explicit is 
better than implicit (provided it is logical and consistent). Here is 
how that looks like. 

In `simple/BUILD.gn` we define our `bleeding-edge` config:
```lua
# Define a common config
config("bleeding-edge") {
  cflags = [ "-std=c++23" ]
  lib_dirs = [ "/opt/local/libexec/llvm-18/lib" ]
  ldflags = [ "-Wl,-rpath,/opt/local/libexec/llvm-18/lib" ]
}
```

And elsewhere (e.g`simple/src/app/BUILD.gn`) we use `bleeding-edge`
config like so:
```lua
executable("app-static") {
  sources = [ "main.cc" ]
  deps = [ "//src/lib:ss-shared" ]
  include_dirs = [ "../lib" ]
  configs += [ "//:bleeding-edge" ]
}
```

Having to repeat `configs += ["//:bleeding-edge/"]` feels error prone.
So the next step is to see if we can define `bleeding-edge` config in
`build/BUILD.gn` and let `build/BUILDCONFIG.gn` make use of it in
`set_defaults` like so:

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

And guess what? It works! Compared to *meson*, the nuts and bolts 
approach of *gn* is beginning to grow on me. 

The last bit to check on is to ensure that the app does link to the
correct library; i.e., static app target should use the static library
and the dynamically linked app target should use the shared library. 

This is not a *gn* issue per se. It requires the developer to set things
up correctly and document the build defines the library user has to set
in order to link correctly. Boring stuff, but for completeness sake here
is how that looks:

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

Somewhere in the getting started documentation the library developer
would ask the user of the library to `-DSTATS_API_IS_DLL=1` to use
the shared library, and `-DSTATS_API_IS_DLL=0` to use the static library.

Since both the library developer and the user of the library is me, and
since this article is about using *gn*, let me just dump the `app` and
`lib` targets here so we can wrap up on a positive note.

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

Next the bootstrap file to inform *gn* where toolchain config is (`simple/.gn`):
```lua
buildconfig = "//build/BUILDCONFIG.gn"
```

The app targets (`simple/src/app/BUILD.gn`):
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

The lib targets (`simple/src/lib/BUILD.gn`):
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

And we can verify that we got it right by using `otool`; here is the
whole session transcript:
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

Nice. With this initial experience I think I am ready to go the distance
with *gn* and take on the `ad-hoc` project which has a few challenges
like fetching third party dependency (`uvw`) that in turn depends on
`uv`. And to make it interesting, I did not follow the suggested 
guidelines of *meson* to use *subprojects*.

Stay tuned for Part III.

# References
- [Using GN build, Artisanl metabuild](https://docs.google.com/presentation/d/15Zwb53JcncHfEwHpnG_PoIbbzQ3GQi_cpujYwbpcbZo/edit#slide=id.g119d702868_0_12)
- [GN Reference](https://gn.googlesource.com/gn/+/master/docs/reference.md#buildargs)
- [Best practices for writing GN templates, Fuchsia](https://fuchsia.dev/fuchsia-src/development/build/build_system/best_practices_templates)
- [Using dependencies from Conan](https://vcmi.eu/developers/Conan/)
