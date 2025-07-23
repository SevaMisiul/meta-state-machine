// Copyright (c) [2025] [Vsevolod Misiul]
// Distributed under the MIT License.
// See accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#pragma once

#include "utils.hpp"
#include "variant.hpp"
#include "front_interface.hpp"

namespace msm {

namespace detail {

template<typename StateMachine, typename Row, typename T>
struct TransitionExecutorInternal;

template<typename T>
void print_debug() {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

} // namespace detail

template<typename SMDefinition>
class StateMachine {
public:
    template<typename StateMachine, typename Row, typename T>
    friend struct detail::TransitionExecutorInternal;
public:
    using sm_definition = SMDefinition;
    using transition_table = typename SMDefinition::transition_table;
    using state_set = typename transition_table::state_set;

    template<typename... Args>
    explicit constexpr StateMachine(Args&&... args)
        : m_current_state(0) {}

    template<typename InitialState, typename... Args>
    explicit StateMachine(InitialState init, Args&&... args)
        : m_current_state(init)
        , m_definition(std::forward<Args>(args)...) {}

    template<typename Event>
    void send_event() {
        TransitionExecutor<StateMachine, Event> executor(*this);
        detail::dispatcher<decltype(executor), state_set>::execute(m_current_state.index(), executor);
    }

    template<typename State>
    bool check_state() const {
        return m_current_state.index() == detail::get_index<state_set, State>::value;
    }

    template<typename State>
    void change_state() {
        m_current_state.template emplace<State>();
    }

    SMDefinition* operator->() {
        return &m_definition;
    }

private:
    detail::Variant<state_set> m_current_state;
    SMDefinition m_definition;

private:
    template<typename SrcState, typename Event, typename TransitionTable>
    struct find_row {};

    template<typename SrcState, typename Event, typename Row1, typename... Rows>
    struct find_row<SrcState, Event, typename SMDefinition::template TransitionTable<Row1, Rows...>> {
        using type = typename std::conditional<
            std::is_same<typename Row1::src_state_type, SrcState>::value &&
            std::is_same<typename Row1::event_type, Event>::value,
            Row1,
            typename find_row<SrcState, Event, typename SMDefinition::template TransitionTable<Rows...>>::type
        >::type;
    };

    template<typename SrcState, typename Event>
    struct find_row<SrcState, Event, typename SMDefinition::template TransitionTable<>> {
        using type = detail::InvalidTransition;
    };

    template<typename StateMachine_, typename Event>
    struct TransitionExecutor {
        explicit TransitionExecutor(StateMachine_ & sm) : m_sm(sm) {}

        template<typename CurrState>
        void operator()() const {
            using row = typename find_row<CurrState, Event, typename StateMachine_::transition_table>::type;
            detail::TransitionExecutorInternal<StateMachine_, row> executor(m_sm);
            executor();
        }

    private:
        StateMachine_ & m_sm;
    };
};

} // namespace msm
