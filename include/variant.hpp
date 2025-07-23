// Copyright (c) [2025] [Vsevolod Misiul]
// Distributed under the MIT License.
// See accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#pragma once

#include <utility>

#include "type_set.hpp"

namespace msm {
namespace detail {

template<typename Visitor, typename Set>
struct dispatcher;

template<typename Visitor, typename... Args>
struct dispatcher<Visitor, TypeSet<Args...>> {
    using FuncPtr = void(*)(Visitor);

    template<typename T>
    static void call_visitor(Visitor v) {
        v.template operator()<T>();
    }

    static void execute(size_t index, Visitor v) {
        static constexpr FuncPtr jump_table[] = { &call_visitor<Args>... };

        if (index < sizeof...(Args)) {
            jump_table[index](v);
        }
    }
};

template<typename Array>
struct Variant {};

template<typename ...Args>
struct Variant<TypeSet<Args...>> {
    using set = TypeSet<Args...>;

    template<typename InitialType>
    explicit constexpr Variant(InitialType init) : m_curr_id{get_index<set, InitialType>::value} {}

    explicit constexpr Variant(int id) : m_curr_id{id} {}

    template<typename T>
    void emplace() {
        m_curr_id = get_index<set, T>::value;
    }

    template<typename Visitor>
    void visit(Visitor&& visitor) const {
        detail::dispatcher<Visitor, set>::execute(m_curr_id, std::forward<Visitor>(visitor));
    }

    int index() const {
        return m_curr_id;
    }

private:
    int m_curr_id = -1;
};

} // namespace detail
} // namespace msm
