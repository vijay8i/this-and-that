# Is there such a thing as a perfect build system? (Part I)
(thoughts in progress...)

An elegant build framework that I have used in the past is [`waf.io`](^1)
and it worked well for our use cases. We wrote build tasks using `Python`
by implementing steps required by *waf.io*, and ended up with code that
builds code that in my opinion was readable and maintainable for any one
coming in contact with it many moons later. 

However, *waf.io* has this design quirk of having to commit itself into
version control post install. Ironically, this feature is what made me
choose it for a complex build pipeline in my last startup. Developers
could simply checkout the monorepo and start building without having to
install any additional tools other than the standard compiler tool chain.

Before I go any further, let me confess: the title is a clickbait. There
is no such thing as a perfect build system. I have not found one yet that
is free from warts, quirks, and kinks; so far. But that should not
stop us from looking for it.

Reality of modern software development is that we have to deal with dozens
of dependencies that come with their own build systems, and *waf* to my
knowledge doesn't handle integration with those dependencies well. My
recollection is that you end up writing a bunch of Python code to work
with third-party dependencies, which if written poorly can slow down the
build process.

> [!NOTE]
> It's possible that *waf* may have addressed the kind of issues I had
> using it almost a decade ago. Even otherwise, I think *waf* is a fine
> tool that gets you going for small to medium projects and a worthwhile
> alternative to have in your toolbox.

If performance is the main concern then a two step build pipeline is
arguably the way to go. The first step generates a build file and the
second step runs the build file with as much parallelism as possible. 
This two step process was made popular by a tool named `ninja` which
takes charge of the **second** step. 

Ninja was designed with constraints of what it can do, what it wants to
do and what it does to a very narrow scope which is to compile a large
code base fast by chomping through one or more assembly file(s) that
contain sequence of instructions. 

While such assembly file(s) can be written by hand, it can get tricky
and feels like grunt work very quickly. Ninja leaves the DX of creating
such files to meta build tools such as `cmake` and `meson` among others,
which provide higher order constructs that get transformed into the 
assembly of instructions for *ninja* which then builds the artefacts
(executables, libraries, docs, ...).

For step 1, the most popular choice is *cmake*. But `CMakeLists.txt`
syntax is simply not for me; it gives me allergies. I get it that it is
a DSL. The moment I see syntax for functions, loops and conditionals that
look like wild mushrooms, I am signing out. I mean why oh why? At that
point why can't we simply use a general purpose scripting language and
provide a well designed build-systems specific library to achieve the
same result. Or at the very least use syntax that doesn't require a
cognitive context switch.

So, I am looking for an alternative to *cmake* and Internet tells me that
I have two options: *gn*, and *meson*. Meson seemed to fit my preference
for syntax; it is pythonic and often seen in use with *ninja* in many
foss projects.

This article goes into the details of getting a simple program in C++
built with a couple of 3rd party library dependencies, and write down
my experience. It gets technical beyond this point.

## Using `meson` and `ninja`
Consider the project tree structure below. It is adhoc and I started 
with it wanting to learn about `uvw` on a whim.

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

I git cloned *uvw* into `3p` folder. And I noticed that *uvw* supports
building using either *cmake* or *meson*. Since I did not have *meson*
installed (initially), I used *cmake* to succesfully build and run tests
by simply following the instructions in README.md of *uvw*.

Next I wanted to use the header files from *uvw* and link to the library
that was built in the first step. Turns out this simple desire of mine
is non-intuitive to fulfill with either *cmake* or *meson*. Both tools 
quickly start *suggesting* how to setup the project structure the way
*they* want rather than how *I* want.

I had two alternatives: resort to using a *Makefile* or learn more on how
to make it work with either *cmake* or *meson*. *Make* is wild; and I have
enough scars to remind me to not go near it. Given my dislike for
*cmake*'s DSL, and considering *meson*'s DSL is closer to Python, I 
decided to go with *meson*.

Getting started with *meson* seemed easy enough. Took only couple of 
*RTFM* minutes.

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

In case it is not obvious, let me point out that *meson* managed to
pull in the required dependency `libuv` of `uvw` because the author of
`uvw` declares it as a dependency in *meson.build* like so:

```python
libuv_dep = dependency('libuv', version: '1.48.0', required: true)
```

That is really cool. 

> [!NOTE]
> In an ideal world,  I prefer a fetch tool that obtains the source
> from a git url and copies it to a destination folder of my choice,
> and keep the source in sync with remote (for a specified release tag
> or hash). This would only work if everyone agreed on a universal 
> `meta build tool`. A topic that quickly crosses into a different
> dimension, which I will not get into for this exercise. Yes, I know 
> *cmake* can do this... but it doesn't have my vote to be the universal
> *meta build tool* for various reasons, including the syntax.

After the above step, I got `libuv` and `libuvw` compiled and installed
locally. Note that *libuvw* headers are in source; only the compiled `dll`
gets installed. So now the next challenge is to make my example use them.
As a reminder it is a challenge only because my directory structure goes
*against the grain* of what is recommended or expected.

After a bit more of *RTFM* minutes, I figured out that I need to
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

Problem solved? I wish. The whole exercise started with me wanting to
try C++23 with something a bit more complicated than a `hello world`.
Notice that my `project` is setting `default_options: ['cpp_std=c++23'].

Unfortunately, I found out *ninja* was using `-std=c++2b` when compiling;
even worse *meson* proudly announces that it is using the system installed
C++ compiler, which doesn't support `c++23`. Net result is I got a ton of
compilation errors, obviously.

So now I have to chase down two problems: why *meson* was generating
*ninja.build* with wrong compiler options, and how to make *ninja* use
a compiler that is not the system default. 

This sure looked like a rabbit hole to me and it was. The output of 
*ninja* showed that it was using `c++` and a quick `which c++` told
me that my `c++` doesn't point to `clang++` which supports the 2023
features that I want.

I could either change the system `c++` default or somehow figure out
how to make meson and ninja talk to each other on which compiler to
use &mdash; the fear of the unknown long term pain vs short term pain?

> [!TIP]
> Take the short term pain when you can handle it, always.

I thought I could smartypant my way out by setting environment variables
in the script file as below:

```python
# set the compiler to use (doesn't work!)
# because there is no way to pass the env to build the executable()
env = environment()
env.set('CC', '/opt/local/bin/clang++')
env.set('CC_FOR_BUILD', '/opt/local/bin/clang++')
```

I should have known that would not work since `executable()` doesn't
accept an *environment* argument. Remember that I am just trying to see
how far I can go with my conceptual knowledge without having to learn
*meson* in depth. I know I am not being fair to *meson*! 

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
> [!NOTE]
> See [epilog](#epilog); I did not set `CXX` and `CXX_FOR_BUILD` at 
> this step; would have saved me 30 minutes if I did properly RTFM.

I finally had to use the `--native-file` option, which at first did not
work &mdash; for like 30 minutes I was scratching my head and feverishly
searching [`SO`](^2). Since I ran out of options, and since there was
nothing else to try, I went for `rm -rf build`; and like magic it works.
While I am thrilled that it worked, it did not give me the good vibes I
was hoping to get out of *meson*.

> Cache can be your friend, cache can be your worst enemy!
> It can save you time and suck away time if you ignore it.
> It can... okay you get the point.

Fwiw, here is the native file.
```ini
[binaries]
c = '/opt/local/bin/clang'
cpp = '/opt/local/bin/clang++'
```

And here is how you use it to generate correct *ninja.build* files with
the compiler of your choice. If it doesn't work, remember to remove the
build folder and start over.

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

Going through the docs of *meson* and having gone through the exercise
of building a simple program with third party dependencies, I get the
feeling that it is strongly opinionated software that lacks coherence.

Speaking of opinions I have a few of my own from the whole ordeal.

- The DX would have been better if *meson* automatically looked for
  `native.ini` and read it without having to use `--native-file` option.
- Having to specify env variables (that too using a file and an extra
  cli argument) outside the build script for *meson* to do the 
  right thing is simply ignoring developer experience. On any
  given day the poor developer has to remember two dozen things to
  get his or her job done. Having to remember yet another dimension
  is unnecessary pain inflicted for no good reason.
- Weak sauce argument on why they chose to make *meson.build* 
  Turing-incomplete.  Do they really believe that *meson* will
  be around and `Python` will go away?
- The script file already allows conditionals and loops, so the argument
  for not being Turing-complete seems bogus to me.

My first impression of *meson* is that it baked into its implementation
some strong opinions that most likely were truthy at some point in time,
and probably served the authors of *meson* well on their job. At present
times however, it just feels odd to embrace those opinions. Either that
or I am far removed from the nuances of what it takes to design a meta
build tool that just works for the happy paths.

I can buy into the convention over configuration argument; but I have
a difficult time adopting conventions and configurations that have a 
narrow view of how software gets built. For example, here is a note from
meson's website about reproducible builds:
> Roughly what this means is that if two different people compile the
> project from source, their outputs are bitwise identical to each other.
> This allows people to verify that binaries downloadable from the net
> actually come from the corresponding sources and have not, for example,
> had malware added to them.

How exactly is this achieved if the build script is unable to specify 
what compiler it needs without depending on the developer to read the
docs, set the necessary exports and what not.

There are many such good intents badly designed in meson. To be sure 
however, I understand that build systems are complex, and requires an
enormous effort and community to get it right. My ranting is in no way
an attempt to throw shade at *meson*. I appreciate that I managed to 
build an artefact out of an ad hoc project structure using *meson* and
that is fricking cool.

In summary, my hunt for a perfect build system for C/C++ projects is not
over. I really hoped that `meson` could be it. Unfortunately it is not
because imo it violates the `principle of least surprise` step after
step after a mere few steps of using it.

For a harried developer who just wants to build software, that is 
increasingly becoming complex, `meson` adds complexity for no good
reason (that my attention deficited mind can comprehend).

## Epilog
To be able to override the default compiler that *meson* uses, one must
set `CC`, `CC_FOR_BUILD`, `CXX`, and `CXX_FOR_BUILD` vars on CLI like so:
```bash
CC=`which clang` CC_FOR_BUILD=`which clang` CXX=`which clang++` CXX_FOR_BUILD=`which clang++` meson setup --reconfigure -Dprefix=$PROJECT_ROOT/3p/local build
```

I made the mistake of setting only `CC` and `CC_FOR_BUILD`, conveniently
ignoring the fact that I was trying to build C++ code. So most of my rant
was for nothing, and can be brushed away as a skills issue. :roll_eyes:, sigh!

Still, the pitfalls exist. 

You have to remember to `rm -rf build` to ensure that environment 
variables get picked up and get used. And, why can't I set the environment
variables for `executable` or `project`? And, many more questions arose
as I tried to get things to work. 

I hear you; it's open source. So use it, or don't, or fix it; just don't
complain. Fair. At this point, I still have the option to explore `gn` 
before I can arrive at a conclusion on which way to go. So my next hop
is to investigate Google's `gn` (generate ninja) program.

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