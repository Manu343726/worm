
#include "sprout/string.hpp"
#include "worm/detail/sfinae.hpp"
#include "worm/detail/meta_utils.hpp"
#include "stmp/stmp.hpp"

#include <iostream>

#define STRINGFY_(x) #x
#define EXPAND(x) x
#define STRINGFY(x) STRINGFY_(x)
#define CAT__(x,y) x ## y
#define CAT_(x,y) CAT__(x,y)
#define CAT(x,y) CAT_(x, y)

constexpr std::size_t parse_type_name_(const char* name)
{
    return *name == '=' ? 2 : parse_type_name_(name + 1) + 1;
}

template<typename T>
constexpr const char* type_name()
{
    return __PRETTY_FUNCTION__ + parse_type_name_(__PRETTY_FUNCTION__);
}


#define ASSERT_CONSTEXPR_STRING(str) static_assert(str[0] == str[0], "Non-constexpr string!")

#define REQUIRES_EXPR___(expr, message_, id, ...) struct CAT(check_,id) \
{ \
    template<typename Ts_ = worm::detail::list<Ts...>, typename Head_ = Head, typename Tail_ = worm::detail::list<Tail...>, typename First_ = First, \
             typename Second_ = Second, typename T_ = T, typename U_ = U, typename = void> \
    struct valid : std::false_type \
    { \
        static constexpr auto message = " " + sprout::to_string(message_ " [FAILED]\n"); \
    }; \
     \
    template<typename... Ts_, typename Head_, typename... Tail_, typename First_, \
             typename Second_, typename T_, typename U_> \
    struct valid<worm::detail::list<Ts_...>, \
                 Head_, worm::detail::list<Tail_...>, \
                 First_, Second_, T_, U_, \
                 __VA_ARGS__> : std::true_type \
    { \
        static constexpr auto message = " " + sprout::to_string(message_ " [SUCCEED]\n"); \
    }; \
    static constexpr bool value = (checks::template push<CAT(check_,id)>(),valid<>::value); \
     \
    static constexpr auto message = valid<>::message; \
    constexpr CAT(check_,id)() = default; \
}; \
static constexpr CAT(check_,id) CAT(CAT(check_,id), _c) = CAT(check_,id){};

#define REQUIRES_EXPR__(expr, message, id, ...) REQUIRES_EXPR___(expr, message, id, __VA_ARGS__)
#define REQUIRES_EXPR_(expr, message, ...) REQUIRES_EXPR__(expr, message, __COUNTER__, __VA_ARGS__)


#define REQUIRES_EXPR_EXPECTED(expr, expected) \
    REQUIRES_EXPR_(expr, "\"" STRINGFY(expr) "\" of type \"" STRINGFY(expected) "\"", \
                   worm::detail::enable_if_t<std::is_same<decltype(expr), expected>::value>)

#define REQUIRES_EXPR_CONVERTIBLE(expr, expected) \
    REQUIRES_EXPR_(expr, "\"" STRINGFY(expr) "\" convertible to \"" STRINGFY(expected) "\"", \
                   worm::detail::enable_if_t<std::is_convertible<decltype(expr), expected>::value>)


#define REQUIRES_EXPR(expr) REQUIRES_EXPR_(expr, "\"" STRINGFY(expr) "\"", worm::detail::void_t<decltype(expr)>)

#define REQUIRES_TRAIT_EXPECTED(trait, expected) \
    REQUIRES_EXPR_(trait, "\"" STRINGFY(trait) "\" giving \"" STRINGFY(expected) "\"", \
                   worm::detail::enable_if_t<trait{} == expected>)

#define REQUIRES_TRAIT(...) REQUIRES_TRAIT_EXPECTED((__VA_ARGS__), true)

template<typename meta_list>
using type_list = typename meta_list::template value<>;

template<typename T, typename Refinement>
struct refinement
{
    using from = T;
    using concept = Refinement;
};


#define BEGIN_CONCEPT(name,...) namespace { template<typename... Args> \
                                struct name { using checks = atch::meta_list<name>; \
                                using concept_args = worm::detail::list<Args...>; \
                                using Self = name; \
                                static constexpr auto concept_name = sprout::to_string(STRINGFY(name)); \
                                static constexpr auto refinements_str = sprout::to_string(STRINGFY((__VA_ARGS__)));\
\
                                template<typename Ts, typename Head = typename Ts::head, typename Tail = typename Ts::tail,\
                                         typename First = Head, typename Second = typename Ts::second, typename T = First, \
                                         typename U = Second> \
                                struct checks_holder_; \
\
                                template<typename... Ts, typename Head, typename... Tail, typename First, typename Second, \
                                         typename T, typename U> \
                                struct checks_holder_<worm::detail::list<Ts...>, \
                                                     Head, worm::detail::list<Tail...>, \
                                                     First, Second, T, U> \
                                { constexpr checks_holder_(int i) : i{i} {} int i; using refines = atch::type_list<__VA_ARGS__>;

#define END_CONCEPT(name) }; using checks_holder = checks_holder_<concept_args, typename concept_args::head, typename concept_args::tail, \
                                                                  typename concept_args::first, typename concept_args::second>; \
                             static constexpr decltype(Message<name>()) message = Message<name>(); \
                             static constexpr checks_holder checks_holder_instance_ = checks_holder{42}; \
                             static constexpr bool value = Value<name>(); \
                             ASSERT_CONSTEXPR_STRING(name::message);}; } \
\
                          template<typename... Ts> \
                          constexpr decltype(Message<name<Ts...>>()) name<Ts...>::message;


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
    static constexpr auto message = sprout::to_string("");
    static constexpr bool value = true;
};

template<typename Concept>
constexpr auto Message(typename std::enable_if<!Concept::checks_holder::refines::empty>::type* = nullptr)
{
    return Concept::concept_name + " requires:\n" +
           fold<type_list<typename Concept::checks>,0>::message +
           "While refining " + Concept::refinements_str + " with " + Concept::concept_name + ":\n" +
           fold<typename Concept::checks_holder::refines,0>::message;
}

template<typename Concept>
constexpr auto Message(typename std::enable_if<Concept::checks_holder::refines::empty>::type* = nullptr)
{
    return Concept::concept_name + " requires:\n" + fold<type_list<typename Concept::checks>,0>::message;
}

template<typename Concept>
constexpr bool Value()
{
    return fold<typename Concept::checks_holder::refines,0>::value && fold<type_list<typename Concept::checks>,0>::value;
}

template<typename T>
constexpr typename std::decay<T>::type lvalue;

template<typename T>
constexpr typename std::decay<T>::type&& rvalue = lvalue<T>;

#define CONCEPT_FROM_TRAIT(Concept, Trait) BEGIN_CONCEPT(Concept)  REQUIRES_TRAIT(Trait<Ts...>) END_CONCEPT(Concept)

CONCEPT_FROM_TRAIT(DefaultConstructible, std::is_default_constructible)
CONCEPT_FROM_TRAIT(CopyConstructible, std::is_copy_constructible)
CONCEPT_FROM_TRAIT(Destructible, std::is_destructible)
CONCEPT_FROM_TRAIT(CopyAssignable, std::is_copy_assignable)

BEGIN_CONCEPT(Semiregular, DefaultConstructible<T>, CopyConstructible<T>, Destructible<T>, CopyAssignable<T>)
    REQUIRES_EXPR_EXPECTED(&lvalue<T_>, const T_*)
END_CONCEPT(Semiregular)

BEGIN_CONCEPT(EqualityComparable)
    REQUIRES_EXPR_CONVERTIBLE(std::declval<T_>() == std::declval<T_>(), bool)
    REQUIRES_EXPR_CONVERTIBLE(std::declval<T_>() == std::declval<T_>(), bool)
END_CONCEPT(EqualityComparable)

BEGIN_CONCEPT(Regular, Semiregular<T>, EqualityComparable<T>)
END_CONCEPT(Regular)

BEGIN_CONCEPT(Allocatable)
    REQUIRES_EXPR_EXPECTED(new T_, T_*)
    REQUIRES_EXPR(delete std::declval<T_*>())
    REQUIRES_EXPR_EXPECTED(new T_[666], T_*)
    REQUIRES_EXPR(delete [] new T_[666])
END_CONCEPT(Allocatable)


BEGIN_CONCEPT(AllIntegral, AllIntegral<Tail>...)
    REQUIRES_TRAIT(std::is_integral<Head_>)
END_CONCEPT(AllIntegral)

template<typename T>
using uncvref_t =
typename std::remove_cv<typename std::remove_reference<T>::type>::type;

BEGIN_CONCEPT(Iterable)

END_CONCEPT(Iterable)

int main()
{
    using concept = Regular<int>;

    std::cout << concept::message << std::endl;
    std::cout << concept::value << std::endl;
}


