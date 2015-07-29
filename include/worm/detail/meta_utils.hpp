//
// Created by manu343726 on 29/07/15.
//

#ifndef WORM_META_UTILS_HPP
#define WORM_META_UTILS_HPP

namespace worm
{
    namespace detail
    {
        template<typename... Ts>
        struct raw_list_
        {
            static constexpr std::size_t size = sizeof...(Ts);
            static constexpr bool empty = size == 0;
        };

        template<typename Ts>
        struct list_impl;

        template<typename... Ts>
        using list = list_impl<raw_list_<Ts...>>;

        template<>
        struct list_impl<raw_list_<>> : raw_list_<>
        {
            typedef struct {} head;
            typedef list<> tail;
            typedef struct {} first;
            typedef struct {} second;
        };

        template<typename First>
        struct list_impl<raw_list_<First>> : raw_list_<First>
        {
            typedef First head;
            typedef list<> tail;
            typedef First first;
            typedef struct {} second;
        };

        template<typename First, typename Second, typename... Tail>
        struct list_impl<raw_list_<First, Second, Tail...>> : raw_list_<First, Second, Tail...>
        {
            typedef First head;
            typedef list<Second, Tail...> tail;
            typedef First first;
            typedef Second second;
        };

        template<typename T>
        struct unpack
        {
            using type = list<T>;
        };

        template<template<typename...> class T, typename... Args>
        struct unpack<T<Args...>>
        {
            using type = list<Args...>;
        };

        template<typename T>
        using unpack_t = typename unpack<T>::type;

        template<typename T>
        using void_t = typename std::conditional<true, void, T>::type;

        template<bool expr>
        using enable_if_t = typename std::enable_if<expr>::type;
    }
}

#endif //WORM_META_UTILS_HPP
