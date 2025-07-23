#include <gtest/gtest.h>
#include <state_machine.hpp>

struct StateA {};
struct StateB {};
struct StateC {};
struct StateD {}; // Transition without action

struct EventGoToB {};
struct EventGoToC {};
struct EventStayInC {};
struct EventGoToD {};
struct EventGoToA {};

struct UnhandledEvent {}; // Event that should not be handled

// --- Create a "mock" model to track actions ---
// We will add public flags and counters to it so that tests can check them.
// It is also assumed that StateMachine provides a get_front() method to access
// an instance of this model. If not, the flags can be made static.

class MockApplication_ : public msm::FrontInterface<MockApplication_> {
public:
    // Flags and counters to check action calls
    bool transition_A_to_B_called = false;
    bool transition_B_to_C_called = false;
    int self_transition_C_count = 0;

    // Methods that will be our actions
    void on_transition_A_to_B() { transition_A_to_B_called = true; }
    void on_transition_B_to_C() { transition_B_to_C_called = true; }
    void on_self_transition_C() { self_transition_C_count++; }

    // Method to reset the mock's state before each test
    void reset() {
        transition_A_to_B_called = false;
        transition_B_to_C_called = false;
        self_transition_C_count = 0;
    }

    // --- Transition table covering different scenarios ---
    using transition_table = TransitionTable<
        //    Initial State | Event           | Next State    | Action
        // -----------------|-----------------|---------------|-------------------------------------
        Row<StateA,           EventGoToB,       StateB,         &MockApplication_::on_transition_A_to_B>,
        Row<StateB,           EventGoToC,       StateC,         &MockApplication_::on_transition_B_to_C>,
        Row<StateC,           EventStayInC,     StateC,         &MockApplication_::on_self_transition_C>,
        Row<StateA,           EventGoToD,       StateD,         nullptr>, // Transition without action
        Row<StateD,           EventGoToA,       StateA,         nullptr>  // Cyclic transition
    >;
};

// Define the type of our state machine for tests
using MockSm = msm::StateMachine<MockApplication_>;


// --- Test fixture class for isolating tests ---

class StateMachineTest : public ::testing::Test {
protected:
    MockSm sm;

    void SetUp() override {
        sm->reset();
    }
};

TEST_F(StateMachineTest, InitialStateIsFirstStateInTableByDefault) {
    ASSERT_TRUE(sm.check_state<StateA>());
    ASSERT_FALSE(sm.check_state<StateB>());
}

TEST_F(StateMachineTest, CanBeConstructedWithSpecificState) {
    MockSm sm_in_b{StateB{}};
    ASSERT_TRUE(sm_in_b.check_state<StateB>());

    MockSm sm_in_c{StateC{}};
    ASSERT_TRUE(sm_in_c.check_state<StateC>());
}


TEST_F(StateMachineTest, SendValidEventAndChangesState) {
    ASSERT_TRUE(sm.check_state<StateA>());

    sm.send_event<EventGoToB>();

    ASSERT_TRUE(sm.check_state<StateB>());
    ASSERT_FALSE(sm.check_state<StateA>());
    ASSERT_TRUE(sm->transition_A_to_B_called);
}

TEST_F(StateMachineTest, SendChainOfEventsCorrectly) {
    ASSERT_TRUE(sm.check_state<StateA>());

    sm.send_event<EventGoToB>();
    sm.send_event<EventGoToC>();

    ASSERT_TRUE(sm.check_state<StateC>());
    ASSERT_TRUE(sm->transition_A_to_B_called);
    ASSERT_TRUE(sm->transition_B_to_C_called);
}

TEST_F(StateMachineTest, IgnoresEventWithNoTransitionFromCurrentState) {
    ASSERT_TRUE(sm.check_state<StateA>());

    sm.send_event<EventGoToC>();

    ASSERT_TRUE(sm.check_state<StateA>());
    ASSERT_FALSE(sm->transition_A_to_B_called);
    ASSERT_FALSE(sm->transition_B_to_C_called);

    sm.send_event<UnhandledEvent>();

    ASSERT_TRUE(sm.check_state<StateA>());
}


TEST_F(StateMachineTest, HandlesSelfTransitionAndCallsAction) {
    MockSm sm_in_c{StateC{}};
    sm_in_c->reset();

    sm_in_c.send_event<EventStayInC>();
    sm_in_c.send_event<EventStayInC>();

    ASSERT_TRUE(sm_in_c.check_state<StateC>());
    ASSERT_EQ(sm_in_c->self_transition_C_count, 2);
}


TEST_F(StateMachineTest, HandlesTransitionWithNullptrAction) {
    ASSERT_TRUE(sm.check_state<StateA>());

    sm.send_event<EventGoToD>();

    ASSERT_TRUE(sm.check_state<StateD>());
    ASSERT_FALSE(sm->transition_A_to_B_called);
    ASSERT_FALSE(sm->transition_B_to_C_called);
    ASSERT_EQ(sm->self_transition_C_count, 0);
}


TEST_F(StateMachineTest, HandlesCyclicTransitions) {
    ASSERT_TRUE(sm.check_state<StateA>());

    sm.send_event<EventGoToD>();
    ASSERT_TRUE(sm.check_state<StateD>());
    sm.send_event<EventGoToA>();

    ASSERT_TRUE(sm.check_state<StateA>());

    sm.send_event<EventGoToB>();
    ASSERT_TRUE(sm.check_state<StateB>());
    ASSERT_TRUE(sm->transition_A_to_B_called);
}
