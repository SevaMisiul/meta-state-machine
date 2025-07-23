// Copyright (c) [2025] [Vsevolod Misiul]
// Distributed under the MIT License.
// See accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#pragma once

#include <type_traits>
#include <stdlib.h>

namespace msm {
namespace detail {

template<typename... Args>
struct TypeSet {
    static constexpr size_t size = sizeof...(Args);
};

template<typename Set, typename T>
struct contains;

template<typename U, typename... Args, typename T>
struct contains<TypeSet<U, Args...>, T> {
    static constexpr bool value = contains<TypeSet<Args...>, T>::value;
};

template<typename... Args, typename T>
struct contains<TypeSet<T, Args...>, T> {
    static constexpr bool value = true;
};

template<typename T>
struct contains<TypeSet<>, T> {
    static constexpr bool value = false;
};

template<typename Set, typename T>
struct prepend {
};

template<typename... args, typename a>
struct prepend<TypeSet<args...>, a> {
    using type = TypeSet<a, args...>;
};

template<typename Set, typename T>
struct filter {
    using type = TypeSet<>;
};

template<typename... Args, typename T>
struct filter<TypeSet<T, Args...>, T> {
    using type = typename filter<TypeSet<Args...>, T>::type;
};

template<typename U, typename... Args, typename T>
struct filter<TypeSet<U, Args...>, T> {
    using type = typename prepend<typename filter<TypeSet<Args...>, T>::type, U>::type;
};

template <typename Set, typename T>
struct get_index_internal;

template <typename U, typename... Rest, typename T>
struct get_index_internal<TypeSet<U, Rest...>, T> {
    static constexpr size_t value = get_index_internal<TypeSet<Rest...>, T>::value;
};

template <typename... Rest, typename T>
struct get_index_internal<TypeSet<T, Rest...>, T> {
    static constexpr size_t value = sizeof...(Rest);
};

template <typename T>
struct get_index_internal<TypeSet<>, T> {
    static constexpr size_t value = -1;
};

template<typename Set, typename T>
struct get_index {
    static constexpr size_t value = Set::size - get_index_internal<Set, T>::value - 1;
};

} // namespace detail
} // namespace msm
