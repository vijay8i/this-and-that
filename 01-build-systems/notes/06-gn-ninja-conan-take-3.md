# Is there such a thing as a perfect build system? (Part VI)
(thoughts in progress...)

If you landed here without context, start with Part I to understand 
where the journey begins. The TL;DR version is I was curious to know
if there are alternatives (in 2024) that are as good or better than *cmake*.
But why even bother? Because I am a sucker for clean readable code, and
build scripts are code (to me).

In Part I through Part V, I reminisced about *waf.io* and how it lets
you write build tasks in Python. Did some head banging with *meson*
since I prefer my way of laying out source code which *meson* did not
appreciate. And, I finally decided to triple down on *gn* + *conan* + *ninja* combo
because:
  - they work well together and stay out of each other's business 
  - each does one thing and does it well 
  - the DSL of each reads better to my eyes 
 
Using this trio got me to a setup that is better than *meson*, IMO, for
dealing with third-party dependencies. But it did take me more time than
anticipated. In the final stretch I have two things remaining that I 
want to square off, and then wrap up.

1. How do I declare/define build arguments and inform *gn* to use
   the arguments to configure the build?
2. How can I be sure that the dependencies get installed at *gen-time*,
   not *build-time*.

## Generating *debug* and *release* builds
To figure out how to convey build time arguments via *gn* to *ninja*, I
gave myself the task of creating *release* and *debug* builds of the 
*ad-hoc* project's executable. Turns out `declare_args` is the magic word
to declare build arguments. It only took a couple of RTFM minutes to
declare and make use of it to configure the build. I was mildly confused
between the `args` command and `--args` option in *gn*; the former is used
to query what arguments have been *set* for a build and the latter is used
to *setup* the build with the args specified. 

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

With the above changes, the *build_type* arg can be set on the command
line and queried as below:
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

I think it would be cool if I can figure how to set the *build_type*
based on *out_dir*, so I can avoid having to use *--args* cli option.
That is an exercise for another day.

## Install dependencies at *gen-time*
There is not a lot of material on *gn* that explicitly talks about its
*philosophy* and *principles*. My understanding is that at the very least
it is within the *gn*'s domain of concern to produce efficient *build.ninja*
files to reduce the build time. However, that is not automatic. There is
nothing *gn* can do if the build/release engineer inadvertently injects
blocking functions into the instructions for *ninja*. This was my fear,
since I was just getting to know *gn*.

To make sure my third-party dependencies get installed at *gen-time*,
I started examining the files that *gn* generated for *ninja*. And look
for any log created by *ninja*; turns out, by default *ninja* keeps log
of its activities in `.ninja_log` and there is no option to turn it off!

```log
# ninja log v6
1	9	1720382113205771305	obj/experiments/using-uvw/3pp_for_main.stamp	56bd3c99f2d29d7a
0	6618	1720382113205275679	obj/experiments/using-uvw/main.o	cea83dedfafea84e
6619	6862	1720382119824411012	main	c6f7449545583e16
6863	6870	1720382120067502031	obj/ad-hoc.stamp	bef81623a49bbf09
```

I was unable to find the format of this file but I can reasonably guess
the first entry points to an activity of interest that took `9 - 1` units
of time. I want to be sure that is just creating a timestamp file and not
actually installing the dependencies in that step. Turning on all the
available options in *ninja* still did not answer my question definitively:

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

> [!NOTE]
> It would be nice if *ninja* had an option to include a timestamp on
> its verbose output.

Since the script (`conan.py`) to install the dependencies is in my
control, I modified it to log to a file whenever it was called from
`conan.gni` template:

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

With the logging in place, I could see that a new entry is added to the
log file whenever *gn* runs and not when *ninja* runs. Now I am sure that
the dependencies are installed at *gen-time* and not at *build-time*. I
also discovered that the script is invoked everytime *gn gen* command is
issued. At first I thought that I should figure out a way to skip 
invoking the script if the dependencies have been installed, by learning
more *gn* fu. On second thought, I think that would be a wrong thing to
work on since it is a concern of *conan*. I still would like to know if
that is doable using *gn*, but for now I think this is good enough.

## Wrap up
You won't find a list of pros and cons in this section for I did not
set out to write that kind of article; heck, I didn't even think I was
writing an article. That is not to say, there aren't pros and cons when
choosing a build system. There will always be pros and cons to anything
we touch and use. However, I do have a few thoughts on the nature and 
purpose of my quest.

At an abstract level, I do not like products that work hard to dumbify
the user, to the point where no one knows what a button does and why. I
see a red flag whenever I find the word "easy" in the documentation of
a product.

So, better is not easy. Better to me is when I enjoy working with a tool
or within a system to build or create something that helps me or others.
It is the process I mostly care about, and every step of that process 
should be something I can reason about and make sense. Good tools and
systems are lean on policy and rich in mechanisms. If I can compose using
those mechanisms with minimal context, that makes the tool or the system
better.

There is no such thing as a perfect build system. What you have often is
either a *de facto* or *de jure* build system.  Without a doubt, *cmake*
is the *de facto* build system for C/C++ projects. It may also be the
*de jure* in many organizations. So, you can't go wrong investing time
in learning it.

For the rebels who do not rest until they find something *better* than
the *status quo*, I think the combination of `gn` + `conan` + `ninja` is
worth considering. I enjoyed the process, both the discovery and making
use of this trio. 

The fake rebels can settle for *meson*. To each their own. Peace.

/fin.
