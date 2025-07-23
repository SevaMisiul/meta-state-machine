// Copyright (c) [2025] [Vsevolod Misiul]
// Distributed under the MIT License.
// See accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#pragma once

#include <type_traits>

#include "type_set.hpp"

namespace msm {

template<typename This>
class FrontInterface {
public:
    using sm_action_type = void (This::*)();

    template<typename SrcState, typename Event, typename DstState, sm_action_type Action>
    struct Row {
        using src_state_type = SrcState;
        using event_type = Event;
        using dst_state_type = DstState;
        static constexpr auto action = Action;
    };

private:
    template<typename... Rows>
    struct StateSet {
        using type = detail::TypeSet<>;
    };

    template<typename SrcState, typename Event, typename DstState, sm_action_type Action, typename... Rows>
    struct StateSet<Row<SrcState, Event, DstState, Action>, Rows...> {
        using type = typename detail::prepend<
            typename detail::filter<typename StateSet<Rows...>::type, SrcState>::type,
            SrcState
        >::type;
    };

public:
    template<typename ...Rows>
    struct TransitionTable {
        using state_set = typename StateSet<Rows...>::type;
    };
};

} // namespace msm
