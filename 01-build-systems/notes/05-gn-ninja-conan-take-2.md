# Is there such a thing as a perfect build system? (Part V)

(thoughts in progress...)

In the field of software construction, most problems can be solved by adding
another layer of indirection. But that has a cost which I prefer to avoid. And
if I can't avoid then I prefer to be in a position to write the indirection
myself with a basic understanding of how a tool works.

In part IV I had to take a detour to learn about `conan`, a tool that seems to
align with my preferences, in order to be able to build the `ad-hoc` project
with minimal steps (for the developer or release engineer)

To recap, the plan was:

- [x] Figure out how to fetch third-party dependencies (`uv` and `uvw`).
- [x] Integrate the previous step using _gn_ `actions`.
- [x] Finally use the `third-party` dependencies.

The first step of fetching third-party dependencies using _conan_ took more RTFM
minutes than with _meson_. Conan allows developers to write _tasks_ in Python
which to me is a big _green_ flag, so I think those extra RTFM minutes are worth
it. The feature in _conan_ that enables the creation of `pkg-config`'s `*.pc`
files using `PkgConfigDeps` generator is foundational to my plan.

> [!TIP]\
> _pkg-config_ is an elegant solution to `pubsub` C/C++ dependencies. I
> recommend reading the [guide to `pkg-config`](^1) to learn about it. It has
> that property of do one thing and do it well.

I am happy to connect with _conan_ and _pkg-config_, and happy to welcome both
into my toolchest.

In the second step I hit a wall so hard that I almost gave up on _gn_. What I
wanted to achieve was to be able to specify the `third-party` dependency
packages required by a target and then automatically invoke `conan` to not only
fetch the packages but also install their pkg-config files that I can later
reference to build my target.

Seeing the _BUILD.gn_ for the target I was trying to build can provide a better
explanation than me rambling. So here are the contents of
`ad-hoc/experiments/using-uvw/BUILD.gn`:

```lua
import("//build/common/conan.gni")
import("//build/common/pkg_config.gni")

conan_install("install_third_party_packages") {
  conanfile = "//conanfile.txt"
  deploy_dir = "//third-party"
}

pkg_config("target_defaults") {
  # project root releative path to find *.pc files of the dependencies
  pkg_deps_path = "//third-party"  # project root absolute

  # order matters (if the packages have dependencies)
  pkg_deps = [
    "libuv-static",
    "uvw",
  ]
}

executable("main") {
  sources = [ "main.cc" ]
  configs += [ ":target_defaults" ]
}
```

It took several iterations and plenty of trawls through _gn_'s newsgroup
messages looking for answers to get something working.

Ignore for now and don't ask if this is the best way to build an executable, and
instead focus on `conan_install` and `pkg_config` template function invocations.
The key idea is that the developer/build-engineer can now simply type
`gn gen out/ad-hoc`, and `gn` will automagically use `conan` to fetch, download,
build, and deploy `third-party` dependencies. This allows me to avoid having to
use `git submodule` to manage my target's `third-party` dependencies, and it is
one less step for the developer to remember to get started on a project.

> [!WARNING]\
> Do not blindly fetch dependencies from a central registry that is not under
> your control as it introduces `supply-side-security` issues. In production, I
> would most likely setup up my own registry which will contain curated
> collection of dependencies that are reviewed before adoption and on an ongoing
> basis. On balance a `git submodule` approach has the same set of issues if not
> managed for security.

The reason I had trouble in getting a seemingly trival task accomplished using
_gn_ was because I did not understand `action` and `template` in depth. I sort
of could infer the roles of `action` and `template` function blocks. The former
is used by the _ninja_ during the build whereas the latter is used by _gn_
itself to generate build files for _ninja_. I was trying to install dependencies
using _conan_ in an _action_ block; since that action is not executed until
build time, the subsequent step to retrieve `pkg-config` info from those
dependencies would fail (since the _action_ never ran). This became apparent
after [reading this thread](^2) about the difference between `action` and
`exec_script`.

> [!TIP]\
> It might be obvious to those working with build systems as their main job
> function. And that is this notion of `gen-time` vs `build-time`, which in hind
> sight is like a doh! moment having worked with _cmake_. During `gen-time`,
> _gn_ generates the necessary build files, and during `build-time` _ninja_
> executes those `build files`. So, *actions*s are for _ninja_ and _templates_
> are for _gn_. It gets confusing because a _template_ can include an _action_.
> One way to make sense is to see the _action_ as an `async` function bundled by
> _gn_ that is then `awaited` on by `ninja`.

After a bit of head banging, it became clear that the stuff I wanted to automate
required template functions, not actions. I ended up with two template functions
that allow me to check off the second step. The python scripts are not important
as they are trivial in what they do.

The first template(`//build/common/conan.gni`) provides the automation to fetch
dependencies using _conan_. I see this as a _seed_ from which the full
cababilities of _cmake_'s `FetchContent_*` suite could be implemented.

```lua
template("conan_install") {
  forward_variables_from(invoker,
                         [
                           "conanfile",
                           "deploy_dir",
                           "deps",
                         ])
  assert(defined(conanfile), "conanfile must be set")
  assert(defined(deploy_dir), "deploy_dir must be set")

  deploy_dir = rebase_path(deploy_dir, ".")
  _args = [
    "install",
    rebase_path(conanfile, "."),
    "--output-folder=${deploy_dir}",
    "--deployer=direct_deploy",
    "--deployer-folder=${deploy_dir}",
    "--build=missing",
  ]
  exec_script("//build/common/conan.py", _args)

  # this is needed to avoid the following error
  # You set the variable "target_name" here and it was unused before it went out of scope.
  group(target_name) {
    metadata = {
      conan_installed = [ true ]
    }
  }
}
```

Second template (`//build/common/pkg_config.gni`) provides a configuration for
the target to utilize:

```lua
template("pkg_config") {
  config(target_name) {
    forward_variables_from(invoker, "*")
    assert(defined(pkg_deps), "pkg_deps must be set")
    if (defined(pkg_deps_path)) {
      deps_base = rebase_path(pkg_deps_path, "")
    } else {
      deps_base = rebase_path(".", "//")
    }

    _args = [
      "-p",
      deps_base,
    ]
    _args += pkg_deps

    _result = exec_script("//build/common/pkg_config.py", _args, "json")

    if (_result.cflags != []) {
      if (!defined(cflags)) {
        cflags = []
      }
      cflags += _result.cflags
    }

    if (_result.include_dirs != []) {
      if (!defined(include_dirs)) {
        include_dirs = []
      }
      include_dirs += rebase_path(_result.include_dirs, ".")
    }

    if (_result.libs != []) {
      if (!defined(libs)) {
        libs = []
      }
      libs += _result.libs
    }

    if (_result.lib_dirs != []) {
      if (!defined(lib_dirs)) {
        lib_dirs = []
      }

      lib_dirs += rebase_path(_result.lib_dirs, ".")
    }

    if (_result.ldflags != []) {
      if (!defined(ldflags)) {
        ldflags = []
      }
      ldflags += _result.ldflags
    }
  }
}
```

The last step required me to understand build systems by one more layer than I
want to. And that is this notion of a `build graph`. After I got the `conan`
template to kind of work, I was stumped at figuring out how to use it in my
target.

I was getting this unhelpful message from _gn_ at all attempts of mine to use
the template &mdash;incorrectly, initially:

```bash
ERROR at //experiments/using-uvw/BUILD.gn:4:1: Assignment had no effect.
conan_install("install_third_party_packages") {
^----------------------------------------------
You set the variable "target_name" here and it was unused before it went
out of scope.
Via these template invocations:
  conan_install("install_third_party_packages")  //experiments/using-uvw/BUILD.gn:4
See //experiments/using-uvw/BUILD.gn:4:1: whence it was called.
conan_install("install_third_party_packages") {
^----------------------------------------------
See //BUILD.gn:3:12: which caused the file to be included.
  deps = [ "//experiments/using-uvw:main" ]
           ^-----------------------------
```

One of the key ideas of _gn_ or any build metal tool for that matter is to
ensure that the build instructions generated for _ninja_ are correct and free of
cycles. This is achieved by validating the DAG of _targets_ from parsing the
_*.gn_ files. Any unused target is questioned, and _gn_ was complaining that my
target 'install_third_party_packages' is unused. It is up to me now to decide if
I needed it or remove it otherwise.

Obviously I need the function to execute but I don't care about using the output
directly in my executable target. How to go about fixing this situation was not
obvious to me at all. The solution, it turns out, is to satisfy _gn_ by stating
that 'install_third_party_packages' is a `meta-target` like here in
`//build/common/conan.gni`:

```lua
group(target_name) {
  metadata = {
    conan_installed = [ true ]
  }
}
```

The `group()` _function_ allows you to create _meta-targets_ that collect a set
of dependencies into one named target. In my case, the target doesn't produce
any build output itself, but it is required to execute in order for other
dependencies to build. Since the Conan installation happens during `gen-time`,
we need a way to represent this in the _gn_'s build graph to keep it happy. It
is happy if the DAG it constructs internally is _accounted_ for all _targets_
appearing in BUILD.gn files either directly or indirectly, and have a purpose.
Any unused or unaccounted _targets_ make _gn_ unhappy, unless you tell it, which
I did by using `group` function.

After sorting out that last bit, I have a setup that is better than what I got
from _meson_ &mdash;subjectively speaking. I am happy with the results so far
considering the fact that I barely managed to get my feet wet with _gn_ and
_conan_, and didn't even look into _ninja_. I can see that once I get
comfortable with _gn_ templates and actions, I could whip up more elegant build
setups.

> [!CAUTION]\
> Please do not use this setup for your work. There are dozens of nuances left
> unaddressed, and it exists solely for the purpose of learning a new way to
> design build systems.

# Where am I?

I think I am on the home stretch now. Coming up with a plan and accomplishing it
feels good. But the job is not done. I want to understand how to _configure_
_gn_ to setup _ninja_ to build release and debug artefacts, and be convinced
that I am not slowing down _ninja_ because of _build-time_ dependencies that
should have been _gen-time_ dependencies.

To be continued...

<!-- short links -->

[^1]: https://people.freedesktop.org/~dbn/pkg-config-guide.html

[^2]: https://groups.google.com/a/chromium.org/g/gn-dev/c/x4uy_Mhdb_Q/m/-G6r4X17AAAJ

# References

- [GN template best practices](https://fuchsia.dev/fuchsia-src/development/build/build_system/best_practices_templates)
- [GN action vs exec_script](https://groups.google.com/a/chromium.org/g/gn-dev/c/x4uy_Mhdb_Q/m/-G6r4X17AAAJ)
- [GN template, run newly-built executable by name](https://groups.google.com/a/chromium.org/g/gn-dev/c/3Cr_8Ej9C0g/m/nYD3aT4kBQAJ)
