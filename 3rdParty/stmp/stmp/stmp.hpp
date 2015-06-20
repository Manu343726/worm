//   file: type_list.hpp
// author: Filip Roséen <filip.roseen@gmail.com>
// source: http://b.atch.se/posts/constexpr-meta-container

#ifndef ATCH_TYPE_LIST_HPP
#define ATCH_TYPE_LIST_HPP

#include <cstddef>
#include <utility>

namespace atch {
    template<std::size_t N>
    using ic = std::integral_constant<std::size_t, N>;

    template<std::size_t... Ns>
    using iseq = std::index_sequence<Ns...>;

    template<std::size_t N>
    using make_iseq = std::make_index_sequence<N>;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    template<class... Ts>
    struct type_list {
        using size_type = std::size_t;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        static constexpr size_type size () { return sizeof... (Ts); }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<size_type Idx>
        struct at {
            template<size_type N, class U, class... Us>
            struct access_helper
                    : access_helper<N+1, Us...>
            { };

            template<class U, class... Us>
            struct access_helper<Idx, U, Us...> {
                using result = U;
            };

            using result = typename access_helper<0, Ts...>::result;
        };

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<class... Us>
        struct push {
            using result = type_list<Ts..., Us...>;
        };

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        template<size_type Idx, class U>
        struct set {

            template<size_type N>
            static auto at_helper (ic<N>)   -> typename at<N>::result;
            static auto at_helper (ic<Idx>) -> U;

            template<size_type... Ns>
            static auto set_helper (iseq<Ns...>)
                    -> type_list<decltype (at_helper (ic<Ns> {}))...>
            ;

            using result = decltype (set_helper (make_iseq<size()> {}));
        };

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        struct init {
            template<size_type... Ns>
            static auto helper (iseq<Ns...>)
                    -> type_list<typename at<Ns>::result...>
            ;

            using result = decltype (helper (make_iseq<size ()-1> {}));
        };
    };

} /* namespace atch */

#endif /* include guard */

//   file: meta_counter.hpp
// author: Filip Roséen <filip.roseen@gmail.com>
// source: http://b.atch.se/posts/constexpr-meta-container

#ifndef ATCH_META_COUNTER_HPP
#define ATCH_META_COUNTER_HPP

#include <cstddef>

namespace atch { namespace {

        template<class Tag>
        struct meta_counter {
            using size_type = std::size_t;

            template<size_type N>
            struct ident {
                friend constexpr size_type adl_lookup (ident<N>);
                static constexpr size_type value = N;
            };

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<class Ident>
            struct writer {
                friend constexpr size_type adl_lookup (Ident) {
                    return Ident::value;
                }

                static constexpr size_type value = Ident::value;
            };

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<size_type N, int = adl_lookup (ident<N> {})>
            static constexpr size_type value_reader (int, ident<N>) {
                return N;
            }

            template<size_type N>
            static constexpr size_type value_reader (float, ident<N>, size_type R = value_reader (0, ident<N-1> ())) {
                return R;
            }

            static constexpr size_type value_reader (float, ident<0>) {
                return 0;
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<size_type Max = 64>
            static constexpr size_type value (size_type R = value_reader (0, ident<Max> {})) {
                return R;
            }

            template<size_type N = 1, class H = meta_counter>
            static constexpr size_type next (size_type R = writer<ident<N + H::value ()>>::value) {
                return R;
            }
        };
    }}

#endif /* include guard */

//   file: meta_list.hpp
// author: Filip Roséen <filip.roseen@gmail.com>
// source: http://b.atch.se/posts/constexpr-meta-container

#ifndef ATCH_META_LIST_HPP
#define ATCH_META_LIST_HPP

#include <cstddef>

namespace atch { namespace {

        template<class Tag>
        struct meta_list {
            using   counter = atch::meta_counter<meta_list<Tag>>;
            using size_type = typename counter::size_type;

            template<size_type N, class = void>
            struct ident {
                friend auto adl_lookup (ident<N>);

                static constexpr size_type value = N;
            };

            template<class Dummy>
            struct ident<0, Dummy> {
                friend auto adl_lookup (ident<0>) {
                    return atch::type_list<> {};
                }
            };

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<class Ident, class Value>
            struct writer {
                friend auto adl_lookup (ident<Ident::value>) {
                    return Value {};
                }

                static constexpr size_type value = Ident::value;
            };

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<
                    class State,
                    class     H = meta_list,
                    class Ident = typename H::template ident<H::counter::next ()>
            >
            static constexpr size_type push_state (size_type R = writer<Ident, State>::value) {
                return R;
            }

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<
                    class     H = meta_list,
                    size_type N = H::counter::value ()>
            using value_ident = typename H::template ident<N>;

            template<
                    class     H = meta_list,
                    class Ident = typename H::template value_ident<>
            >
            using value = decltype (adl_lookup (Ident {}));

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            template<class... Ts, class H = meta_list>
            static constexpr void push (
                    size_type = push_state<
                            typename H::template value<>::template push<Ts...>::result
                    > ()
            ) {}

            template<class H = meta_list>
            static constexpr void pop (
                    size_type = push_state<
                            typename H::template value<>::init::result
                    > ()
            ) {}

            template<size_type Idx, class T, class H = meta_list>
            static constexpr void set (
                    size_type = push_state<
                            typename H::template value<>::template set<Idx, T>::result
                    > ()
            ) {}
        };
    }}

#endif /* include guard */

#ifdef STMP_EXAMPLE_MAIN
//   file: main.cpp
// author: Filip Roséen <filip.roseen@gmail.com>
// source: http://b.atch.se/posts/constexpr-meta-container

#include <iostream>

template<class T>
void print_type () {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int main () {
    using LX = atch::meta_list<class A>;

    LX::push<void, void, void, void> ();
    LX::set<0, class Hello> ();
    LX::set<2, class World> ();
    LX::pop ();

    print_type<LX::value<>> ();
}
#endif /* STMP_EXAMPLE_MAIN */
