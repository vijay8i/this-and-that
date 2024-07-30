# The Zig Zag

Greenfield product development that calls for systems software development is
the funnest of all software endeavors. Getting started however is the most nerve
wracking experience, especially if you are the architect. Much of what you put
in place in the early stage becomes the bedrock for the evolution of the
project, its velocity and eventually its success or failure. It's a nightmare
for any CTO to carry the badge of a failed project due to technological choices
made under his or her watch.

One such critical choice that has lasting impact on velocity but often not given
enough consideration is the build system. I did an exploratory state of the art
survey of build systems recently, and I was happy to discover `gn+conan+ninja`
trio as a better alternative to CMake, Meson, and the rest. While I wrote in
conclusion that it is worth investing time in getting to know them, the truth
was that it felt overly complex. If the objective is to modernize an existing
product's build system for speed and maintainability, then sure the trio can do
the job and I wouldn't mind taking on such a project.

But if you are starting from scratch, the language (which is another critical
choice to be made) and the build system requires a big think upfront. There are
two languages that I have been following for a while now &mdash; Rust and Zig.

Both these (systems) languages have something that C and C++ do not. They come
with a build system. In my opinion and experience, the impact a language has
that comes with its own tooling can be 10x or more.
