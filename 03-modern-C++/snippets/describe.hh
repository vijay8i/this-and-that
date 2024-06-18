// Print types as strings, including functions, member 

// Written by Chris Uzdavinis

#include <type_traits>
#include <typeinfo>
#include <string>
#include <utility>

namespace type_to_string::detail {

template <typename T> struct Describe;

template <typename T, class ClassT> 
struct Describe<T (ClassT::*)>  {
    static auto describe() -> std::string;
};
template <typename RetT, typename... ArgsT> 
struct Describe<RetT(ArgsT...)>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...)>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) volatile>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const volatile>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) volatile noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const volatile noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...)&>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const &>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) volatile &>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) & noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const volatile &>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const & noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) volatile & noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const volatile & noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) &&>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const &&>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) volatile &&>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) && noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const volatile &&>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const && noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) volatile && noexcept>  {
     static auto describe() -> std::string;
};
template <typename RetT, class ClassT, typename... ArgsT> 
struct Describe<RetT(ClassT::*)(ArgsT...) const volatile && noexcept>  {
     static auto describe() -> std::string;
};

template <typename T>  
auto describe() -> std::string
{
    using namespace std::string_literals;
    auto terminal = [&](char const * desc) {
        return desc + " "s + typeid(T).name();
    };
    if constexpr(std::is_const_v<T>) {
        return "const " + describe<std::remove_const_t<T>>();
    }
    else if constexpr(std::is_volatile_v<T>) {
        return "volatile " + describe<std::remove_volatile_t<T>>();
    }
    else if constexpr (std::is_same_v<bool, T>) {
        return "bool";
    }
    else if constexpr(std::is_same_v<char, T>) {
        return "char";
    }
    else if constexpr(std::is_same_v<signed char, T>) {
        return "signed char";
    }
    else if constexpr(std::is_same_v<unsigned char, T>) {
        return "unsigned char";
    }
    else if constexpr(std::is_unsigned_v<T>) {
        return "unsigned " + describe<std::make_signed_t<T>>();
    }
    else if constexpr(std::is_void_v<T>) {
        return "void";
    }
    else if constexpr(std::is_integral_v<T>) {
        if constexpr(std::is_same_v<short, T>) 
            return "short";
        else if constexpr(std::is_same_v<int, T>) 
            return "int";
        else if constexpr(std::is_same_v<long, T>) 
            return "long";
        else if constexpr(std::is_same_v<long long, T>) 
            return "long long";
    }
    else if constexpr(std::is_same_v<float, T>) {
        return "float";
    }
    else if constexpr(std::is_same_v<double, T>) {
        return "double";
    }
    else if constexpr(std::is_same_v<long double, T>) {
        return "long double";
    }
    else if constexpr(std::is_same_v<std::nullptr_t, T>) { 
        return "nullptr_t";
    }
    else if constexpr(std::is_class_v<T>) {
        return terminal("class");
    }
    else if constexpr(std::is_union_v<T>) {
        return terminal("union");
    }
    else if constexpr(std::is_enum_v<T>) {
        std::string result;
        if (!std::is_convertible_v<T, std::underlying_type_t<T>>) {
            result += "scoped ";
        }
        return result + terminal("enum");
    }  
    else if constexpr(std::is_pointer_v<T>) {
        return "pointer to " + describe<std::remove_pointer_t<T>>();
    }
    else if constexpr(std::is_lvalue_reference_v<T>) {
        return "lvalue-ref to " + describe<std::remove_reference_t<T>>();
    }
    else if constexpr(std::is_rvalue_reference_v<T>) {
        return "rvalue-ref to " + describe<std::remove_reference_t<T>>();
    }
    else if constexpr(std::is_bounded_array_v<T>) {
        return "array[" + std::to_string(std::extent_v<T>) + "] of " +
            describe<std::remove_extent_t<T>>();
    }
    else if constexpr(std::is_unbounded_array_v<T>) {
        return "array[] of " + describe<std::remove_extent_t<T>>();
    }
    else if constexpr(std::is_function_v<T>) {
        return Describe<T>::describe();
    }
    else if constexpr(std::is_member_object_pointer_v<T>) {
        return Describe<T>::describe();
    }
    else if constexpr(std::is_member_function_pointer_v<T>) {
        return Describe<T>::describe();
    }
}

struct Comma {
    char const * sep = "";
    auto operator()(std::string const& str) -> std::string {
        return std::exchange(sep, ", ") + str;
    }
};

template <typename RetT, typename... ArgsT> 
auto Describe<RetT(ArgsT...)>::describe() -> std::string  {
    Comma comma;
    std::string result = "function taking (";
    ((result += comma(detail::describe<ArgsT>())), ...);
    return result + "), returning " + detail::describe<RetT>();
}

template <typename T, class ClassT> 
auto Describe<T (ClassT::*)>::describe() -> std::string  {
    return "pointer to member of " + detail::describe<ClassT>() +
        " of type " + detail::describe<T>();
}

enum Qualifiers {NONE=0, CONST=1, VOLATILE=2, NOEXCEPT=4, LVREF=8, RVREF=16};

template <typename RetT, typename ClassT, typename... ArgsT>
auto describeMemberPointer(Qualifiers q) -> std::string  {
    std::string result = "pointer to ";
    if (NONE != (q & CONST)) result += "const ";
    if (NONE != (q & VOLATILE)) result += "volatile ";
    if (NONE != (q & NOEXCEPT)) result += "noexcept ";
    if (NONE != (q & LVREF)) result += "lvalue-ref ";
    if (NONE != (q & RVREF)) result += "rvalue-ref ";
    result += "member function of " + detail::describe<ClassT>() + " taking (";
    Comma comma;
    ((result += comma(detail::describe<ArgsT>())), ...);
    return result + "), and returning " + detail::describe<RetT>();
}

template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...)>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(NONE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(CONST);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) volatile>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(VOLATILE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) volatile noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(VOLATILE | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const volatile>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(CONST | VOLATILE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(CONST | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const volatile noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(CONST | VOLATILE | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) &>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const &>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | CONST);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) & noexcept>::describe() -> std::string {
  return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) volatile &>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | VOLATILE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) volatile & noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | VOLATILE | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const volatile &>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | CONST | VOLATILE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const & noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | CONST | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const volatile & noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(LVREF | CONST | VOLATILE | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...)&&>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const &&>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | CONST);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) && noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) volatile &&>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | VOLATILE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) volatile && noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | VOLATILE | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const volatile &&>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | CONST | VOLATILE);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const && noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | CONST | NOEXCEPT);
}
template <typename RetT, class ClassT, typename... ArgsT> 
auto Describe<RetT(ClassT::*)(ArgsT...) const volatile && noexcept>::describe() -> std::string  {
    return describeMemberPointer<RetT, ClassT, ArgsT...>(RVREF | CONST | VOLATILE | NOEXCEPT);
}

} // type_to_string::detail


// entry point function
template <typename T> auto describe() -> std::string {
    return type_to_string::detail::describe<T>();
}

// can we use auto template argument deduction?
// we can but it doesn't make sense since the decltype of T is
// getting modified by receiving it as an argument
// auto describe(const auto& T) -> std::string {
//     return type_to_string::detail::describe<decltype(T)>();
// }