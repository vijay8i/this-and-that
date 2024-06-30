# Research
- [Hacking CPP]((https://hackingcpp.com/cpp/educational_videos.html#cpp23))
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [CPPCon 2023](https://github.com/CppCon/CppCon2023)
- [Subspace (of safe C++)](https://orodu.net/)

# Curious minds want to know
## Explicit template instantiation
- https://stackoverflow.com/questions/60225945/explicit-c-template-instantiation-with-clang
- https://stackoverflow.com/questions/2351148/explicit-template-instantiation-when-is-it-used

## Lambdas

## Typedef vs using 
- https://stackoverflow.com/questions/10747810/what-is-the-difference-between-typedef-and-using

## Expected
- https://andreasfertig.blog/2022/09/cpp23-stdexpected-the-superior-way-of-returning-a-value-or-an-error/

## SFINAE
- https://www.cppstories.com/2016/02/notes-on-c-sfinae/

## Pipe operator
- https://www.cppstories.com/2024/pipe-operator/

## Higher order functions
- file:///Users/vijay/Downloads/higher_order_functions_ndc_oslo_2018.pdf

# Design patterns
## Creational
## Singleton
- https://github.com/hnrck/singleton_example
- https://github.com/HowardHinnant/date/blob/master/src/tz.cpp#L573-L578

## For Mac developers
- https://guide.macports.org/
- https://poweruser.blog/using-dtrace-with-sip-enabled-3826a352e64b

### Port FUCs (Frequently used commands)
1. Which package provides this artefact?
    ```bash
    port provides /opt/local/bin/pkg-config
    port provides /opt/local/bin/clang++-mp-18 `which xz`
    
    ```
2. Which packages does this package depend on?
    ```bash
    port deps pkgconfig
    port deps clang-18 xz
    ```
3. What packages depend on this package?
    ```bash
    port dependents pkgconfig
    port dependents clang-18 xz
    # OR
    port echo dependentof:pkgconfig
    port echo dependentof:clang-18 dependentof:xz
    ```
4. What does this package provide?
    ```bash
    port contents pkgconfig
    port contents clang-18 xz
    ```


## Version Control
- [Git Submodule vs Subtree](https://adam-p.ca/blog/2022/02/git-submodule-subtree/#:~:text=TL%3BDR%3A%20Subtree%20is%20better,to%20edit%20and%20push%20it.)