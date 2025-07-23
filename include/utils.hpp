// Copyright (c) [2025] [Vsevolod Misiul]
// Distributed under the MIT License.
// See accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#pragma once

#include <type_traits>

namespace msm {
namespace detail {

struct InvalidTransition {};

template<typename StateMachine, typename Row, typename = void>
struct TransitionExecutorInternal {
    explicit TransitionExecutorInternal(StateMachine & stateMachine)
        : m_sm(stateMachine) {}

    void operator()() {
        m_sm.template change_state<typename Row::dst_state_type>();
        (m_sm.m_definition.*Row::action)();
    }

private:
    StateMachine & m_sm;
};

template<typename StateMachine, typename Row>
struct TransitionExecutorInternal<StateMachine, Row,
    typename std::enable_if<Row::action == nullptr>::type> {

    explicit TransitionExecutorInternal(StateMachine & stateMachine)
        : m_sm(stateMachine) {}

    void operator()() {
        m_sm.template change_state<typename Row::dst_state_type>();
    }

private:
    StateMachine & m_sm;
};

template<typename StateMachine>
struct TransitionExecutorInternal<StateMachine, InvalidTransition> {
    explicit TransitionExecutorInternal(StateMachine & stateMachine) {}

    void operator()() const {}
};

} // namespace detail
} // namespace msm
