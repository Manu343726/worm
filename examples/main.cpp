
#include "sprout/string.hpp"
#include "worm/detail/sfinae.hpp"
#include "stmp/stmp.hpp"

#include <iostream>

#define STRINGFY_(x) #x
#define EXPAND(x) x
#define STRINGFY(x) STRINGFY_(x)
#define CAT__(x,y) x ## y
#define CAT_(x,y) CAT__(x,y)
#define CAT(x,y) CAT_(x, y)

template<typename T>
constexpr const char concept_message[] = "no message";

template<typename Lhs, typename Rhs, std::size_t... Is, std::size_t... Js>
constexpr std::array<char, sizeof...(Is) + sizeof...(Js)>
str_cat(const Lhs& lhs, const Rhs& rhs,
        std::index_sequence<Is...>, std::index_sequence<Js...>)
{
    return {{ lhs[Is]..., rhs[Js]... }};
}

template<std::size_t Lhs, std::size_t Rhs>
constexpr auto str_cat(const std::array<char,Lhs>& lhs, const std::array<char, Rhs>& rhs)
{
    return str_cat(lhs, rhs, std::make_index_sequence<Lhs-1>{}, std::make_index_sequence<Rhs>{});
}

template<std::size_t Lhs, std::size_t Rhs>
constexpr auto str_cat(const std::array<char,Lhs>& lhs, const char(&rhs)[Rhs])
{
    return str_cat(lhs, rhs, std::make_index_sequence<Lhs-1>{}, std::make_index_sequence<Rhs>{});
}

template<std::size_t Lhs, std::size_t Rhs>
constexpr auto str_cat(const char(&lhs)[Lhs], const std::array<char, Rhs>& rhs)
{
    return str_cat(lhs, rhs, std::make_index_sequence<Lhs-1>{}, std::make_index_sequence<Rhs>{});
}

template<std::size_t Lhs, std::size_t Rhs>
constexpr auto str_cat(const char(&lhs)[Lhs], const char(&rhs)[Rhs])
{
    return str_cat(lhs, rhs, std::make_index_sequence<Lhs-1>{}, std::make_index_sequence<Rhs>{});
}

template<std::size_t Lhs, std::size_t... Is>
constexpr std::array<char, sizeof...(Is)>
str_cpy(const char(&lhs)[Lhs], std::index_sequence<Is...>)
{
    return {{ lhs[Is]... }};
}

template<std::size_t Lhs>
constexpr auto str_cpy(const std::array<char,Lhs>& lhs)
{
    return str_cpy(lhs, std::make_index_sequence<Lhs>{});
}

template<std::size_t Lhs>
constexpr auto str_cpy(const char(&lhs)[Lhs])
{
    return str_cpy(lhs, std::make_index_sequence<Lhs>{});
}

template<typename Lhs, std::size_t... Is>
const char*
to_str(const Lhs& lhs, std::index_sequence<Is...>)
{
    static char str[] = { lhs[Is]... };
    return str;
}

template<std::size_t Lhs>
constexpr const char* to_str(const std::array<char,Lhs>& lhs)
{
    return to_str(lhs, std::make_index_sequence<Lhs>{});
}

#define ASSERT_CONSEXPR_STRING(str) static_assert(str[0] == str[0], "Non-constexpr string!")


#define REQUIRES_EXPR__(expr,id) struct CAT(check_,id) \
{ \
    template<typename T_, typename = void> \
    struct valid : std::false_type \
    { \
        static constexpr auto message = sprout::to_string( "(requires) " STRINGFY(expr) " [FAILED]\n"); \
        static constexpr const char* ptr_ = message.data(); \
    }; \
     \
    template<typename T_> \
    struct valid<T_,worm::detail::void_t<decltype(expr)>> : std::true_type \
    { \
        static constexpr auto message = sprout::to_string( "(requires) " STRINGFY(expr) " [SUCCEED]\n"); \
        static constexpr const char* ptr_ = message.data(); \
    }; \
    static constexpr bool value = (checks::template push<CAT(check_,id)>(),valid<T>::value); \
     \
    static constexpr auto message = valid<T>::message; \
    constexpr CAT(check_,id)() = default; \
}; \
static constexpr CAT(check_,id) CAT(CAT(check_,id), _c) = CAT(check_,id){};

#define REQUIRES_EXPR_(expr, id) REQUIRES_EXPR__(expr, id)
#define UNARY_REQUIRES_EXPR(expr) REQUIRES_EXPR_(expr, __COUNTER__)

template<typename meta_list>
using type_list = typename meta_list::template value<>;

template<typename... Refinements>
struct Refines
{
    using refines = atch::type_list<Refinements...>;
};

#define BEGIN_CONCEPT(name,...) struct name : Refines<__VA_ARGS__> { using checks = atch::meta_list<name>; \
                                    static constexpr auto concept_name = sprout::to_string(STRINGFY(name));

#define END_CONCEPT(name) static constexpr decltype(Message<name>()) message = Message<name>(); \
                          static constexpr bool value = Value<name>(); \
                          ASSERT_CONSEXPR_STRING(name::message);}; \
                          template<typename Ts> \
                          constexpr decltype(Message<name<Ts>>()) name<Ts>::message;


template<typename Is>
struct indent_;

template<std::size_t... Is>
struct indent_<std::index_sequence<Is...>>
{
    static constexpr char str[sizeof...(Is)] = {(Is, ' ')...};
};

template<std::size_t... Is>
constexpr char indent_<std::index_sequence<Is...>>::str[sizeof...(Is)];

template<std::size_t Indent>
using indent = indent_<std::make_index_sequence<Indent+1>>;

template<typename Checks, std::size_t Indent = 10>
struct fold;

template<typename Head, typename Head2, typename... Tail, std::size_t Indent>
struct fold<atch::type_list<Head, Head2, Tail...>, Indent>
{
    using next = fold<atch::type_list<Head2,Tail...>,Indent>;

    static constexpr auto message = indent<Indent>::str +  Head::message + next::message;
    static constexpr bool value = Head::value && next::value;
};

template<typename Head, std::size_t Indent>
struct fold<atch::type_list<Head>, Indent>
{
    static constexpr auto message = indent<Indent>::str + Head::message;
    static constexpr bool value = Head::value;
};

template<std::size_t Indent>
struct fold<atch::type_list<>,Indent>
{
    static constexpr auto message = sprout::to_string("{}\n");
    static constexpr bool value = true;
};

template<typename Concept>
constexpr auto Message()
{
    return "{" + Concept::concept_name +
           "\n(refines)" + fold<typename Concept::refines,0>::message +
           fold<type_list<typename Concept::checks>,0>::message + "}";
}

template<typename Concept>
constexpr bool Value()
{
    return fold<typename Concept::refines,4>::value && fold<type_list<typename Concept::checks>,0>::value;
}




template<typename T>
BEGIN_CONCEPT(Addable)
    UNARY_REQUIRES_EXPR(std::declval<T_>() + std::declval<T_>())
END_CONCEPT(Addable)

template<typename T>
BEGIN_CONCEPT(Comparable)
    UNARY_REQUIRES_EXPR(std::declval<T_>() == std::declval<T_>())
    UNARY_REQUIRES_EXPR(std::declval<T_>() != std::declval<T_>())
END_CONCEPT(Comparable)

template<typename T>
BEGIN_CONCEPT(Arithmetic, Addable<T>, Comparable<T>)
    UNARY_REQUIRES_EXPR(std::declval<T_>() - std::declval<T_>())
END_CONCEPT(Arithmetic)

template<typename T>
BEGIN_CONCEPT(Integral, Arithmetic<T>)
    UNARY_REQUIRES_EXPR(std::declval<T_>() >> std::declval<T_>())
    UNARY_REQUIRES_EXPR(std::declval<T_>() << std::declval<T_>())
    UNARY_REQUIRES_EXPR(std::declval<T_>() % std::declval<T_>())
END_CONCEPT(Integral)

static_assert(Integral<int>::value,"");
static_assert(!Integral<float>::value,"");


int main()
{
    std::cout << Integral<float>::message.c_str() << std::endl;
}


