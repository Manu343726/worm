//
// Created by manu343726 on 18/06/15.
//

#ifndef WORM_SFINAE_HPP
#define WORM_SFINAE_HPP

#include <type_traits>

namespace worm
{
    namespace detail
    {
        namespace { struct dummy {}; }
        template<typename T>
        using void_t = typename std::conditional<true,void,T>::type;

        template<typename T>
        using no_void_t = typename std::conditional<false,void,T>::type;
    }
}

#endif //WORM_SFINAE_HPP
