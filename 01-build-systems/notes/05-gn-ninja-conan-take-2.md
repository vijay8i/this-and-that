# Is there such a thing as a perfect build system? (Part V)
(thoughts in progress...)

To meet the objectives for my quest, I had to take a detour into learning
`conan` which I documented in part IV.

> [!NOTE]
> In the field of software construction, most problems can be solved by
> adding another layer of indirection. The property I am looking for is
> to be able to (a) avoid those layers and (b) if needed, write the
> indirections myself with a basic understanding of how the tools work.

To recap, the plan was:
- [x] Figure out how to fetch third-party dependencies (`uv` and `uvw`).
- [x] Integrate the previous step using *gn* `actions`.
- [x] Finally use the `third-party` dependencies.

The first step of fetching third-party dependencies using *conan* took
more RTFM minutes than with *meson*. But I like working with tools
that do one thing and do it well, so I think those extra RTFM minutes
are worth it. The feature in *conan* that enables the creation of `pkg-config`'s
`*.pc` files using `PkgConfigDeps` generator is foundational to the plan.
In my opinion *pkg-config* is a simple and elegant solution to build 
C/C++ dependencies. I recommend reading the [guide to `pkg-config`](^1)
to learn about it. It has that property of do one thing and do it well.
So all in all, for step 1 of my plan, I am happy to connect with *conan*
and *pkg-config*, and add them to my toolchest.

[^1]: https://people.freedesktop.org/~dbn/pkg-config-guide.html

In the second step I hit a wall so hard that I almost gave up on *gn*.
What I wanted to achieve was to be able to specify the `third-party`
dependency packages required by a target and then automatically invoke
`conan` to not only fetch the packages but also install their pkg-config
files that I can later reference and use to build my target.

Seeing the *BUILD.gn* for the target I was trying to build can provide
a better explaination than me rambling. So here it is, the contents of
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

It took several iterations and plenty of RTFM minutes, and trawling 
through *gn*'s newsgroup messages for answers, before I got something
working. 

Ignore for now and don't ask if this is the best way to build an executable,
and instead focus on `conan_install` and `pkg_config` template function
invocations. The key idea is that the developer/build-engineer can now
simply type `gn gen out/ad-hoc`, and `gn` will automagically use `conan`
to fetch, download, build, and deploy `third-party` dependencies. This
allows me to avoid having to use `git submodule` to manage my target's
`third-party` dependencies, and it is one less step for the developer to
remember to get started on a project.

> [!WARNING]
> Do not blindly fetch dependencies from a central registry that is not
> under your control as it introduces `supply-side-security` issues. In
> production, I would most likely setup up my own registry which will 
> contain curated collection of dependencies that are reviewed before
> adoption and on an ongoing basis. On balance a `git submodule` approach
> has the same set of issues if not managed for security.


The reason I had trouble in getting a seemingly trival task accomplished
using *gn* was because I did not understand `action` and `template` in
depth. I sort of could infer what roles `action` and `template` function
blocks take. The former is used by the build tool during the build whereas
the latter is used by *gn* itself while generating the build files for
*ninja*. I was trying to install dependencies using *conan* in a *action*
block; since that action is not executed until build time, the subsequent
step to retrieve `pkg-config` info from those dependencies would fail
(since the *action* never ran). This became apparent after [reading this thread](^1)
about the difference between `action` and `exec_script`.

> [!TIP]
> It might be obvious to those working with build systems but was not
> to me. And that is the notion of `gen-time` vs `build-time`, which in
> hind sight is like doh! kind of realization. During `gen-time`, *gn*
> generates the necessary build files, and during `build-time` *ninja* 
> executes those `build files`. So, *actions*s are for *ninja* and *templates*
> are for *gn*. It get's confusing because a *template* can include an
> *action*. One way to make sense is to see the *action* as an `async` 
> function that is `awaited` for by `ninja`.

After a bit more of head banging, it became clear that the stuff I wanted
to automate required template functions, not actions. I ended up with two
template functions that allow me to check off the second step. The python
scripts are not important as they are trivial in what they do.

The first template(`//build/common/conan.gni`) provides the automation to
fetch dependencies uisng *conan*. I see it as a *seed* from which the full
cababilities of *cmake*'s `FetchContent_*` can be implemented.
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

Second template (`//build/common/pkg_config.gni`) provides a configuration
for the target to utilize:
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

The last step required me to understand build systems by one more layer
than I want to. And that is this notion of a `build graph`. After I got
the `conan` template to kind of work, I was stumped at figuring out how
to use it in my target. 

I was getting this unhelpful message from *gn* at all attempts of mine to
use the template (which was incorrectly, initially):
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

One of the key ideas of *gn* or any build metal tool for that matter is
to ensure that the build files generated are correct and free of cycles.
This is achieved by validating an internally constructed build graph from
all the input files that are parsed to build a DAG of targets (and other
stuff). Any unused target becomes questionable, and *gn* was complaining
that my target 'install_third_party_packages' is unused. It is up to me
now decide if I needed it and keep it or remove it otherwise. Obviously
I need the function to execute but I don't care about using the target
directly in my executable. How to go about fixing this situation was not
obvious to me at all. The solution, it turns out, is to satisfy *gn* by 
stating that 'install_third_party_packages' is a `meta-target` like here
in `//build/common/conan.gni`:

```lua
  group(target_name) {
    metadata = {
      conan_installed = [ true ]
    }
  }
```

The `group()` *function* allows you to create `meta-targets` that just 
collect a set of dependencies into one named target. In my case, the 
target doesn't produce any build output itself, but it is required to 
execute in order for other dependencies to build. Since the Conan 
installation happens during `gen-time`, we need a way to represent this
in the *gn*'s build graph to keep it happy. It is happy if the DAG it
constructs internally can `account` for all **targets** appearing in
BUILD.gn files either directly or indirectly have a purpose. Any unused
or unaccounted *targets* make *gn* unhappy, unless you tell it, which I
did by using `group` function.

After sorting out that last bit, I have a setup that is better than what
I got from *meson* &mdash; subjectively speaking. I am happy with the
results so far considering the fact that I barely managed to get my feet
wet with *gn* and *conan*, and didn't even look into *ninja*. I can see
that once I get comfortable with *gn* templates and actions, I could whip
up more elegant build setups.

> [!CAUTION]
> Please do not use this setup for your work. There are dozens of nuances
> left unaddressed, and it exists solely for the purpose of learning a 
> new way to design build systems.

# Where am I?
I think I am on the home stretch now. Coming up with a plan and accomplishing
it feels good. But the job is not done. I want to have release and debug
builds, and want to be convinced that I am not slowing down *ninja* because
of build-time dependencies that should have been gen-time dependencies. 
Finally, do an honest evaluation of the build setup and be able to arrive
at some conclusions.

Stay tuned.

<!-- short links -->
[^1]: https://groups.google.com/a/chromium.org/g/gn-dev/c/x4uy_Mhdb_Q/m/-G6r4X17AAAJ

# References
- [GN template best practices](https://fuchsia.dev/fuchsia-src/development/build/build_system/best_practices_templates)
- [GN action vs exec_script](https://groups.google.com/a/chromium.org/g/gn-dev/c/x4uy_Mhdb_Q/m/-G6r4X17AAAJ)
- [GN template, run newly-built executable by name](https://groups.google.com/a/chromium.org/g/gn-dev/c/3Cr_8Ej9C0g/m/nYD3aT4kBQAJ)