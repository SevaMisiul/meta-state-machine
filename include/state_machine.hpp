// Copyright (c) [2025] [Vsevolod Misiul]
// Distributed under the MIT License.
// See accompanying file LICENSE or copy at https://opensource.org/licenses/MIT

#pragma once

#include <stdlib.h>
#include <type_traits>
#include <utility>

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

namespace msm {

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
