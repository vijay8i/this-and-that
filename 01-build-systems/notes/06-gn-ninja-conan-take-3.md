# Is there such a thing as a perfect build system? (Part VI)

(thoughts in progress...)

If you landed here without context, start with Part I to understand where the
journey begins. The TL;DR version is I was curious to know if there are
alternatives (in 2024) that are as good or better than _cmake_. But why even
bother? Because I am a sucker for clean readable code, and build scripts are
code (to me).

In Part I through Part V, I reminisced about _waf.io_ and how it lets you write
build tasks in Python. Did some head banging with _meson_ since I prefer my way
of laying out source code which _meson_ did not appreciate. And, I finally
decided to triple down on _gn_ + _conan_ + _ninja_ combo because:

- they work well together and stay out of each other's business
- each does one thing and does it well
- the DSL of each reads better to my eyes

Using this trio got me to a setup that is better than _meson_, IMO, for dealing
with third-party dependencies. But it did take me more time than anticipated. In
the final stretch I have two things remaining that I want to square off, and
then wrap up.

1. How do I declare/define build arguments and inform _gn_ to use the arguments
   to configure the build?
2. How can I be sure that the dependencies get installed at _gen-time_, not
   _build-time_.

## Generating _debug_ and _release_ builds

To figure out how to convey build time arguments via _gn_ to _ninja_, I gave
myself the task of creating _release_ and _debug_ builds of the _ad-hoc_
project's executable. Turns out `declare_args` is the magic word to declare
build arguments. I was mildly confused between the `args` command and `--args`
option in _gn_ at first. I figured out that the former is used to query what
arguments have been _set_ for a build and the latter is used to _setup_ the
build with the args specified.

Anyway, here are the required changes to `build/BUILCONFIG.gn`:

```lua
declare_args() {
  # Set the default build type to "debug"
  build_type = "debug"
}

# Apply build type specific config
if (build_type == "debug") {
  shared_binary_target_configs += [ "//build:debug_config" ]
} else if (build_type == "release") {
  shared_binary_target_configs += [ "//build:release_config" ]
}
```

And `build/BUILD.gn`:

```lua
config("debug_config") {
  cflags = [
    "-g",
    "-O0",
    "-DDEBUG",
  ]
}

config("release_config") {
  cflags = [
    "-O3",
    "-DNDEBUG",
  ]
}
```

With the above changes, the _build_type_ arg can be set on the command line and
queried as below:

```bash
# generate release build
$ gn gen out/release --args='build_type="release"'
Done. Made 3 targets from 7 files in 637ms

# check that release has the necessary arg set...
$ gn args --list=build_type  out/release
build_type
    Current value = "release"
      From //out/release/args.gn:1
    Overridden from the default = "debug"
      From //build/BUILDCONFIG.gn:4

    Set the default build type to "debug"

$ gn args --list --short --overrides-only  out/release
build_type = "release"

# generate debug build (default) 
$ gn gen out/debug
Done. Made 3 targets from 7 files in 626ms

# check that the default is not overriden
$ gn args --list=build_type out/debug
build_type
    Current value (from the default) = "debug"
      From //build/BUILDCONFIG.gn:4

    Set the default build type to "debug"

# no output since debug is the default build type and it is not overriden
$ gn args --list=build_type --overrides-only out/debug
```

I think it would be cool if I can figure how to set the _build_type_ based on
_out_dir_, so I can avoid having to use _--args_ cli option. That is an exercise
for another day.

## Install dependencies at _gen-time_

There is not a lot of material on _gn_ that explicitly talks about its
_philosophy_ and _principles_. My understanding is that at the very least it is
within _gn_'s domain of concern to produce efficient _build.ninja_ files to
reduce the build time. However, that is not automatic. There is nothing _gn_ can
do if the build/release engineer inadvertently injects blocking functions into
the instructions for _ninja_. This was my fear, since I was just getting to know
_gn_.

To make sure my third-party dependencies get installed at _gen-time_, I started
examining the files that _gn_ generated for _ninja_. And look for any log
created by _ninja_; turns out, by default _ninja_ keeps log of its activities in
`.ninja_log` and there is no option to turn it off!

```log
# ninja log v6
1	9	1720382113205771305	obj/experiments/using-uvw/3pp_for_main.stamp	56bd3c99f2d29d7a
0	6618	1720382113205275679	obj/experiments/using-uvw/main.o	cea83dedfafea84e
6619	6862	1720382119824411012	main	c6f7449545583e16
6863	6870	1720382120067502031	obj/ad-hoc.stamp	bef81623a49bbf09
```

I was unable to find the format of this file but I can reasonably guess the
first entry points to an activity of interest that took `9 - 1` units of time. I
want to be sure that is just creating a timestamp file and not actually
installing the dependencies in that step. Turning on all the available options
in _ninja_ still did not answer my question definitively:

```bash
$ ninja -v -d explain -d stats  -C out/release
ninja: Entering directory `out/release'
ninja explain: deps for 'obj/experiments/using-uvw/main.o' are missing
ninja explain: obj/experiments/using-uvw/main.o is dirty
ninja explain: main is dirty
ninja explain: obj/ad-hoc.stamp is dirty
ninja explain: output obj/experiments/using-uvw/3pp_for_main.stamp doesn't exist
ninja explain: obj/experiments/using-uvw/3pp_for_main.stamp is dirty
ninja explain: main is dirty
[1/4] touch obj/experiments/using-uvw/3pp_for_main.stamp
[2/4] /opt/local/bin/clang++ -MMD -MF obj/experiments/using-uvw/main.o.d  -I../../third-party/direct_deploy/uvw/include -I../../third-party/direct_deploy/libuv/include -std=c++23 -O3 -DNDEBUG  -c ../../experiments/using-uvw/main.cc -o obj/experiments/using-uvw/main.o
[3/4] /opt/local/bin/clang++ -Wl,-rpath,/opt/local/libexec/llvm-18/lib -L/opt/local/libexec/llvm-18/lib -L../../third-party/direct_deploy/libuv/lib -o main @main.rsp  -luv
[4/4] touch obj/ad-hoc.stamp
metric                  count   avg (us)        total (ms)
.ninja parse            1       232.0           0.2
.ninja_log load         1       12.0            0.0
.ninja_deps load        1       5.0             0.0
node stat               29      5.9             0.2
depfile load            1       37.0            0.0
ComputeCriticalPath     1       3.0             0.0
StartEdge               5       309.4           1.5
FinishCommand           4       367.8           1.5

path->node hash load 0.87 (84 entries / 97 buckets)
```

> [!NOTE]\
> It would be nice if _ninja_ had an option to include a timestamp on its
> verbose output.

Since the script (`conan.py`) to install the dependencies is in my control, I
modified it to log to a file whenever it was called from `conan.gni` template:

```python
#!/usr/bin/env python3

import sys
from utils import run, find_project_root, log_process_info


def main():
    cmd = ["conan"] + sys.argv[1:]
    run(cmd, check=True)


if __name__ == "__main__":
    log_process_info(find_project_root() + "/" + "conan-install.log")
    main()
```

With the logging in place, I could see a new entry added to the log file
whenever _gn_ runs and not when _ninja_ runs. Now I am sure that the
dependencies are installed at _gen-time_ and not at _build-time_. I also
discovered that the script is invoked everytime _gn gen_ command is issued. At
first I thought that I should figure out a way to skip invoking the script if
the dependencies have been installed, by learning more _gn_ fu. On second
thought, I think that would be a wrong thing to work on since it is a concern of
_conan_. I still would like to know if that is doable using _gn_, but for now I
think this is good enough.

## Wrap up

I wanted to document my experience in revisiting an area of software engineering
that I haven't paid attention to in a while; and it is done.

Anything that we touch and use has pros and cons, and it is tempting to end with
such a list and call it a wrap. I won't do that since my quest has been
subjective from the start, and creating such a list would feel out of context.
Instead, let me just write down a few thoughts on the nature and purpose of my
quest in conclusion.

At an abstract level, I do not like products that work hard to dumbify the user
to the point where no one knows what a button does and why. I see a red flag
whenever I find the word "easy" in the documentation of a product.

So, better is not easy. Better to me is when I enjoy working with a tool or
within a system to build or create something that helps me or others. It is the
process I mostly care about, and every step of that process should be something
I can reason about and make sense of. Good tools and systems are lean on policy
and rich in mechanisms. If I can compose using those mechanisms with minimal
context, that makes the tool or the system better.

There is no such thing as a perfect build system. What you have often is either
a _de facto_ or _de jure_ build system. Without a doubt, _cmake_ is the _de
facto_ build system for C/C++ projects. It may also be the _de jure_ in many
organizations. So, you can't go wrong investing time in learning it.

For the rebels who do not rest until they find something _better_ than the
_status quo_, I think the combination of `gn` + `conan` + `ninja` is worth
considering. I enjoyed the process, both the discovery and making use of this
trio.

I am not sure where to place _meson_ on this spectrum. In the end, it is to each
their own.

/fin.
