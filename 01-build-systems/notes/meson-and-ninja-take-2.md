# Is there such a thing as a perfect build system? (Part III)
(thoughts in progress...)

I Part II, I left my quest to find a perfect build system (that meets my
requirements and preferences) wanting to move forward building the 
`ad-hoc` project with *gn* to compare with the experience of building
the same using *meson*.

Reading what I wrote again (in Part II), the fairness guy (in me) was
all upset. For you see I had followed instructions (on how to use *gn*)
and got a `simple` project done, and was ready to forge aliance with *gn*.
But I had lodged several complaints (in Part I) about *meson*, without
following instructions, and yet managed to build with it a much more
complex project. That makes me sort of a hypocrite (the fairness guy
told me).

So today instead of continuing with *gn*, I am going to build `simple`
project using *meson*. And write down my experience in doing so. The
objectives remain the same. From `app` and `lib` folders, generate 
four artefacts:
  - app-shared
  - app-static
  - libss-shared.so
  - libss-static.a

Hopefully that makes the fairness guy happy.

Since I got some experience with *meson* already, I blindly went ahead
and replicated the `simple` project structure.  I then replaced all 
`BUILD.gn` files with empty `meson.build` files; removed `.gn` file
and created `native.ini` in its place. Here is how that looks:

```
simple/
├── meson.build
├── native.ini
└── src
    ├── app
    │   ├── main.cc
    │   └── meson.build
    └── lib
        ├── meson.build
        ├── simple-stats.cc
        └── simple-stats.hh
```

Next, I populated the meson build system with the knowledge gained from
doing the `ad-hoc` project and referring to [Meson tutorial](^1) to
ensure I am not over complicating things. I got it done and it works.
And output of `otool` checks out; static app uses static lib, and
dynamically linked app uses the shared lib.

Here are the files for comparison with *BUILD.gn* syntax.

First the root file `simple/meson.build`...
```python
project(
  'simple', 
  'cpp', 
  version: '1.0.0', 
  default_options: ['cpp_std=c++23']
)

subdir('src/lib')
subdir('src/app')
```

And the *config* file `simple/native.ini`
```ini
[binaries]
c = '/opt/local/bin/clang'
cpp = '/opt/local/bin/clang++'
```

The app targets (`simple/src/app/BUILD.gn`) next:
```python
ss_include = include_directories('../lib')

static_dep = declare_dependency(
  include_directories: ss_include,
  compile_args: '-DSTATS_API_IS_DLL=0',
  link_with: ss_static
)

executable(
  'app-static',
  'main.cc',
  dependencies: static_dep,
  link_args: ['-L/opt/local/libexec/llvm-18/lib/', '-Wl,-rpath,/opt/local/libexec/llvm-18/lib']
)

shared_dep = declare_dependency(
  include_directories: ss_include,
  compile_args: '-DSTATS_API_IS_DLL=1',
  link_with: ss_shared
)

executable(
  'app-shared',
  'main.cc',
  dependencies: shared_dep,
  link_args: ['-L/opt/local/libexec/llvm-18/lib/', '-Wl,-rpath,/opt/local/libexec/llvm-18/lib']
)
```

And the lib targets (`simple/src/lib/meson.build`) finally:
```python
ss_sources = ['simple-stats.cc']

ss_shared = shared_library(
  'ss-shared', 
  ss_sources, 
  cpp_args: '-DSTATS_API_BUILD_AS_SHARED_LIB'
)

ss_static = static_library(
  'ss-static', 
  ss_sources, 
  cpp_args: '-DSTATS_API_BUILD_AS_STATIC_LIB'
)
```

Having gone through this exercise I genuinely think investing time into
*gn* is the better path to take. It is an opinion and if you press me
as to why I think that, I won't be able to give you an objective answer.
But I think I can offer up two thoughts and you can decide if they
are subjective or objective.
- The documentation of *gn* is way better than *meson*. The reference
  manual of *gn* is in one big page and I can simply use the browser's
  find function to find the whereabouts of what I am looking for fast;
  and when I get there I find not only a good description but also example
  code right there. Colocation and locality of reference seems to be a
  lost idea in documentation. The concept of literate programming equally
  works well for literate documentation.
- Building a *build metatool* is hard and there are trade-offs. I think
  *meson* distributes complexity while *gn* concentrates it and forces you to 
  deal with it first. Once you get past the difficult job of figuring
  out the config of your toolchain, the rest of the code (that builds code)
  seems to all fit well together (conceptually and syntactically)

Now. I still have not managed to build `ad-hoc` project using *gn* which
I think will be the litmus test for me to make the final call. 

<!-- live references -->
[^1]: https://mesonbuild.com/Tutorial.html

# References
- [Meson Quickstart Guide](https://mesonbuild.com/Quick-guide.html)
- [How to link static library to shared library or to a binary](https://stackoverflow.com/a/34697930)
