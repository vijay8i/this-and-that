# Is there such a thing as a perfect build system? (Part I)

(thoughts in progress...)

Build systems are complex and often don't get much love from application
developers, especially those working in organizations with dedicated
build/release engineers. As a developer without the means to delegate build
tasks, I am always interested in build systems that allow me to get the job done
with a low count of head banging episodes.

In this series, my aim is to explore the state of the art in build tools and
document my experience. Let me start with the last tool I worked with in this
space (of building artefacts from source written in a variety of languages).

[`Waf`](^1) is an elegant build system that I extensively used in my last start
up. With little to no training, the team could comfortably write _Waf_ scripts
using its API; the scripting language was `Python`! It was really cool to write
code that builds code. Compared to a Makefile, in my opinion, the _Waf_ scripts
were more readable and maintainable for any one coming in contact with it many
moons later.

As an added bonus, since an instance of _Waf_ is committed into version control
post install, developers can simply checkout the repo and start building the
project without having to install any additional tools other than the standard
compiler tool chain. This worked well and let us _develop_ complex build
pipelines to automate tasks that could generate a gamut of artefacts.

Use _Waf_, case is closed? Well, that would indeed be the case if the
dependencies have a single origin. In reality, we had to deal with dozens of
dependencies that came from various sources, with their own build setup. So we
had to spawn processes to build these dependencies using their native build
setup. This resulted in unstructured and undocumented build steps in the code,
and since this code is not the product it would not receive enough scrutiny for
correctness or efficiency. Left unchecked the entire build process would come to
a crawl. Part of my job was to periodically review the scripts and fix when
possible by informing Waf that the outcome of the spawned process is a target's
dependency, to prevent unconditional spawning.

> [!NOTE]\
> It's not _Waf_'s fault for making it easy for developers to spawn processes
> willy-nilly. I think _waf_ is a fine tool that gets you going for small to
> medium projects and a worthwhile alternative to have in your toolbox.

As systems get more complex, build performance becomes a major concern. It is
hard to change the build system once it gets established. Choosing wisely at the
offset of a project start can literally save big bucks by minimizing down time
for developers.

When performance is the driving factor, a two step build pipeline is arguably
the way to go. The first step generates a build file and the second step runs
the build file with as much parallelism as possible. This two step process was
not popular until the _ninja_ came along. Sure, using _cmake_ automatically
implies a two step process, but it was mostly done to address the pain of
creating _project_ files for IDEs and generating cross platform Makefiles. Once
_cmake_ started supporting _ninja_, the intent of this two step process changed
to imply performance.

Performance is delivered by _ninja_ because it restricts itself to being an
assembler whose sole purpose it to chomp through one or more assembly file(s)
that describe the targets, the tool to process each target, and the dependencies
of the target. This implies that all the decision making is made _a priori_ and
not during build time. While these assembly file(s) can be written by hand, it
gets tricky and feels like grunt work very quickly. Ninja leaves the DX of
creating such files to _meta build tools_ such as _cmake_ and _meson_ among
others, which provide higher order constructs that get transformed into the
assembly of instructions for _ninja_ which then builds the artefacts
(executables, libraries, docs, ...).

For step one, the most popular choice is _cmake_. But _CMakeLists.txt_ syntax is
simply not for me; it gives me allergies. I get it that it is a DSL. The moment
I see syntax for functions, loops and conditionals that look like wild
mushrooms, I am signing out. I mean why oh why? At that point why can't we
simply use a general purpose scripting language and provide a well designed
build-systems specific library to achieve the same result. Or at the very least
use syntax that doesn't require a cognitive context switch.

So, I am looking for an alternative to _cmake_ and Internet tells me that I have
two options: _gn_, and _meson_. Meson seemed to fit my preference for syntax; it
is pythonic and often seen in use with _ninja_ in many foss projects.

This article goes into the details of getting a simple program in C++ built with
a couple of 3rd party library dependencies, and write down my experience. It
gets technical beyond this point.

## Using `meson` and `ninja`

Consider the project tree structure below. It is adhoc and I started with it
wanting to learn about `uvw` on a whim.

```tree
.
├── 3p
│   └── uvw
│       ├── src
│       │   ...
│       │   ...
│       ├── subprojects
│       │   └── libuv.wrap
|       ├── CMakeLists.txt
│       └── meson.build
├── experiments
│   └── using-uvw
│       ├── main.cpp
│
```

I git cloned _uvw_ into `3p` folder. And I noticed that _uvw_ supports building
using either _cmake_ or _meson_. Since I did not have _meson_ installed
(initially), I used _cmake_ to succesfully build and run tests by simply
following the instructions in README.md of _uvw_.

Next I wanted to use the header files from _uvw_ and link to the library that
was built in the first step. Turns out this simple desire of mine is
non-intuitive to fulfill with either _cmake_ or _meson_. Both tools quickly
start _suggesting_ how to setup the project structure the way _they_ want rather
than how _I_ want.

I had two alternatives: resort to using a _Makefile_ or learn more on how to
make it work with either _cmake_ or _meson_. _Make_ is wild; and I have enough
scars to remind me to not go near it. Given my dislike for _cmake_'s DSL, and
considering _meson_'s DSL is closer to Python, I decided to go with _meson_.

Getting started with _meson_ seemed easy enough. Took only couple of _RTFM_
minutes.

```sh
# install meson using pip3
pip3 install --user meson
source ~/.bash_profile
# export PROJECT_ROOT=$HOME/path/to/project/root  (OR)
# export PROJECT_ROOT=`pwd`
# and then from 3p/uvw do the following 
meson setup --reconfigure -Dprefix=$PROJECT_ROOT/3p/local build
ninja -C build install
```

In case it is not obvious, let me point out that _meson_ managed to pull in the
required dependency `libuv` of `uvw` because the author of `uvw` declares it as
a dependency in _meson.build_ like so:

```python
libuv_dep = dependency('libuv', version: '1.48.0', required: true)
```

That is really cool.

> [!NOTE] In an ideal world, I prefer a fetch tool that obtains the source from
> a git url and copies it to a destination folder of my choice, and keep the
> source in sync with remote (for a specified release tag or hash). This would
> only work if everyone agreed on a universal `meta build tool`. A topic that
> quickly crosses into a different dimension, which I will not get into for this
> exercise. Yes, I know _cmake_ can do this... but it doesn't have my vote to be
> the universal _meta build tool_ for various reasons, including the syntax.

After the above step, I got `libuv` and `libuvw` compiled and installed locally.
Note that _libuvw_ headers are in source; only the compiled `dll` gets
installed. So now the next challenge is to make my example use them. As a
reminder it is a challenge only because my directory structure goes _against the
grain_ of what is recommended or expected.

After a bit more of _RTFM_ minutes, I figured out that I need to
`declare_dependency` explicitly in my `meson.build` file (where my code
resides). Here is how that looks:

```python
project(
  'using_uvw', 
  'cpp', 
  version: '3.5.0', 
  default_options: ['cpp_std=c++23']
)

fs = import('fs')
# spec out 'uvw' and 'uv dependencies
local_lib_dir = join_paths(
  meson.current_source_dir(), 
  '../../3p/local/lib'
)

# Manually specify the uv dependency
libuv_includes = include_directories(
  fs.relative_to('../../3p/local/include', meson.current_source_dir())
)
libuv_dep = declare_dependency(
  include_directories: libuv_includes,
  link_args: ['-L' + local_lib_dir, '-luv']
)

# Manually specify the uvw dependency, it shares the 'uv' library
libuvw_includes = include_directories(
  fs.relative_to('../../3p/uvw/src/', meson.current_source_dir())
)
uvw_dep = declare_dependency(
  include_directories: libuvw_includes,
  link_args: ['-L' + local_lib_dir, '-luvw']
)

executable(
  'main', 'main.cpp', 
  dependencies: [uvw_dep, libuv_dep]
)
```

With that in place, I got meson and ninja to do their thing:

```sh
# export PROJECT_ROOT=$HOME/path/to/project/root
# In $PROJECT_ROOT/experiments/using-uvw 
meson setup --reconfigure build
ninja -C build
```

Problem solved? I wish. The whole exercise started with me wanting to try C++23
with something a bit more complicated than a `hello world`. Notice that my
`project` is setting `default_options: ['cpp_std=c++23'].

Unfortunately, I found out _ninja_ was using `-std=c++2b` when compiling; even
worse _meson_ proudly announces that it is using the system installed C++
compiler, which doesn't support `c++23`. Net result is I got a ton of
compilation errors, obviously.

So now I have to chase down two problems: why _meson_ was generating
_ninja.build_ with wrong compiler options, and how to make _ninja_ use a
compiler that is not the system default.

This sure looked like a rabbit hole to me and it was. The output of _ninja_
showed that it was using `c++` and a quick `which c++` told me that my `c++`
doesn't point to `clang++` which supports the 2023 features that I want.

I could either change the system `c++` default or somehow figure out how to make
meson and ninja talk to each other on which compiler to use &mdash; the fear of
the unknown long term pain vs short term pain?

> [!TIP]\
> Take the short term pain when you can handle it, always.

I thought I could smartypant my way out by setting environment variables in the
script file as below:

```python
# set the compiler to use (doesn't work!)
# because there is no way to pass the env to build the executable()
env = environment()
env.set('CC', '/opt/local/bin/clang++')
env.set('CC_FOR_BUILD', '/opt/local/bin/clang++')
```

I should have known that would not work since `executable()` doesn't accept an
_environment_ argument. Remember that I am just trying to see how far I can go
with my conceptual knowledge without having to learn _meson_ in depth. I know I
am not being fair to _meson_!

```shell
C++ compiler for the host machine: c++ (clang 15.0.0 "Apple clang version 15.0.0 (clang-1500.1.0.2.5)")
C++ linker for the host machine: c++ ld64 1022.1
```

I then tried setting the environment variable on the CLI.

```shell
# this doesn't work, ends up using the system default; don't know why
CC=/opt/local/bin/clang++ meson setup build

# neither does this :-(
CC_FOR_BUILD=/opt/local/bin/clang++ meson setup build
```

> [!NOTE] See [epilog](#epilog); I did not set `CXX` and `CXX_FOR_BUILD` at this
> step; would have saved me 30 minutes if I did properly RTFM.

I finally had to use the `--native-file` option, which at first did not work
&mdash; for like 30 minutes I was scratching my head and feverishly searching
[`SO`](^2). Since I ran out of options, and since there was nothing else to try,
I went for `rm -rf build`; and like magic it works. While I am thrilled that it
worked, it did not give me the good vibes I was hoping to get out of _meson_.

> Cache can be your friend, cache can be your worst enemy! It can save you time
> and suck away time if you ignore it. It can... okay you get the point.

Fwiw, here is the native file.

```ini
[binaries]
c = '/opt/local/bin/clang'
cpp = '/opt/local/bin/clang++'
```

And here is how you use it to generate correct _ninja.build_ files with the
compiler of your choice. If it doesn't work, remember to remove the build folder
and start over.

```shell
meson setup build  --native-file native.ini

...
C++ compiler for the host machine: /opt/local/bin/clang++ (clang 17.0.6 "clang version 17.0.6")
C++ linker for the host machine: /opt/local/bin/clang++ ld64 1022.1
...
# doesn't make sense that I should pass in `-C` args to compile
# meson compile -C build

# would rather use ninja directly since I don't see me creating build
# pipelines using meson
ninja -C build
```

Going through the docs of _meson_ and having gone through the exercise of
building a simple program with third party dependencies, I get the feeling that
it is strongly opinionated software that lacks coherence.

Speaking of opinions I have a few of my own from the whole ordeal.

- Having to specify env variables (that too using a file and an extra cli
  argument) outside the build script for _meson_ to do the right thing is simply
  ignoring developer experience. On any given day the poor developer has to
  remember two dozen things to get his or her job done. Having to remember yet
  another dimension is unnecessary pain inflicted for no good reason.
- Weak sauce argument on why they chose to make _meson.build_ Turing-incomplete.
  Do they really believe that _meson_ will be around and _Python_ will go away?

I can buy into the convention over configuration argument; but I have a
difficult time adopting conventions and configurations that have a narrow view
of how software gets built. For example, here is a note from meson's website
about reproducible builds:

> Roughly what this means is that if two different people compile the project
> from source, their outputs are bitwise identical to each other.

This will not be the case if the developer fails to set the necessary exports
and doesn't use _--native-file_ option, since there is not guarantee which
compiler will be invoked.

My ranting here is in no way an attempt to throw shade at _meson_. I managed to
build an artefact out of an ad hoc project structure using _meson_ and that is
to be appreciated, and I do.

In summary, my hunt for a perfect build system for C/C++ projects is not over. I
had hopes that it would end at `meson`. Unfortunately, it violates the
`principle of least surprise` step after step after a mere few steps of using
it. For a harried developer who just wants to build software, that is
increasingly becoming complex, `meson` adds complexity for no good reason (that
my attention deficited mind can comprehend).

## Epilog

To override the default compiler that _meson_ uses, one must set `CC`,
`CC_FOR_BUILD`, `CXX`, and `CXX_FOR_BUILD` vars on CLI like so:

```bash
CC=`which clang` CC_FOR_BUILD=`which clang` CXX=`which clang++` CXX_FOR_BUILD=`which clang++` meson setup --reconfigure -Dprefix=$PROJECT_ROOT/3p/local build
```

I made the mistake of setting only `CC` and `CC_FOR_BUILD`, conveniently
ignoring the fact that I was trying to build C++ code. So most of my rant was
for nothing, and can be brushed away as a skills issue. :roll_eyes:, sigh!

Still, the pitfalls exist.

You have to remember to `rm -rf build` to ensure that environment variables get
picked up and get used. And, why can't I set the environment variables for
`executable` or `project`? And, many more questions arose as I tried to get
things to work.

I hear you; it's open source. So use it, or don't, or fix it; just don't
complain. Fair. At this point, I still have the option to explore `gn` before I
can arrive at a conclusion on which way to go. So my next hop is to investigate
Google's `gn` (generate ninja) program.

To be continued...

<!-- short links -->

[^1]: https://waf.io/

[^2]: https://stackoverflow.com

## References:

- [WAF - The build system](https://waf.io/)
- [Ninja build tool](https://lwn.net/Articles/706404/)
- [Node Addon using Ninja](https://stackoverflow.com/questions/58633310/how-to-compile-node-js-native-api-extension-via-meson-build-system)
- [Define dependency not found using `pkg-config`](https://stackoverflow.com/questions/47010355/mesonbuild-how-to-define-dependency-to-a-library-that-cannot-be-found-by-pkg-c)
- [Meson monorepo for individual subproject builds](https://embeddedartistry.com/blog/2023/06/05/meson-pattern-monorepo-that-supports-individual-subproject-builds/)
- [Setting CC and CXX on CLI for Meson](https://github.com/mesonbuild/meson/issues/7284#issuecomment-641660087)
- [Modern CMake, please stop using old cmake syntax](https://cliutils.gitlab.io/modern-cmake/)
