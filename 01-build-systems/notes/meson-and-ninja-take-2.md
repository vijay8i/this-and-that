# Is there such a thing as a perfect build system? (Part III)
(thoughts in progress...)

I Part II, I left my quest to find a perfect build system (that meets my
requirements and preferences) wanting to move forward building the 
`ad-hoc` project with *gn* to compare with my *meson* experience.

Reading what I wrote again (in Part II), the fairness guy (in me) was
all upset. For you see I had followed instructions (on how to use *gn*)
and got a `simple` project done, and was ready to forge aliance with *gn*.
But I had lodged several complaints (in Part I) about *meson*, without
following instructions, and yet managed to build a much more complex
project. That makes me sort of a hypocrite (the fairness guy told me).

So today instead of continuing with *gn*, I am going to build `simple`
project using *meson*. And write down my experience in doing so. The
objectives remain the same. From `app` and `lib` folders, generate 
four artefacts:
  - app-shared
  - app-static
  - libss-shared.so
  - libss-static.a

And hopefully that makes the fairness guy happy.

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

