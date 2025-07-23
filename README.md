# MSM: A Minimal C++11 State Machine

**MSM** is a lightweight, header-only, C++11 compatible library for creating finite state machines using template metaprogramming. It provides a clean, declarative syntax to define states, events, and transitions with zero dynamic memory allocation, ensuring high performance and type safety.

## Features ✨

  * **Header-Only**: Just include `state_machine.hpp` and you're ready to go.
  * **C++11 Compatible**: Works with any modern C++ compiler.
  * **Type-Safe**: States and events are represented by types, preventing many common errors at compile-time.
  * **Declarative Syntax**: Define your entire state machine logic in a clear and readable transition table.
  * **Zero-Overhead**: No dynamic memory allocation or virtual function calls, making it suitable for performance-critical applications.
  * **Actions on Transition**: Easily bind member functions to be executed when a transition occurs.

## Requirements

  * A C++11 (or newer) compatible compiler (e.g., GCC 4.8+, Clang 3.3+, MSVC 2015+).

## Installation

This is a header-only library. Simply copy the `state_machine.hpp` file into your project's include path.

For a ready-to-use version, see the single header file in the `single-header` branch.

-----

## How to Use

Using the library involves three simple steps:

### 1\. Define Your States and Events

States and events are represented by simple empty `struct`s. This approach is type-safe and has no runtime cost.

```cpp
// Define the states
struct Idle {};
struct Running {};
struct Stopped {};

// Define the events
struct StartEvent {};
struct StopEvent {};
struct ResetEvent {};
```

### 2\. Define the State Machine Structure

Create a class that inherits from `msm::FrontInterface<YourClass>`. Inside this class, define your action methods and the `transition_table`.

  * `transition_table`: A list of `Row` definitions.
  * `Row`: Defines a single transition: `Row<CurrentState, Event, NextState, &ActionHandler>`. The action handler can be `nullptr` if no action is needed.

<!-- end list -->

```cpp
#include "state_machine.hpp"
#include <iostream>

class VendingMachine_ : public msm::FrontInterface<VendingMachine_> {
public:
    // Actions to be called on transitions
    void onStart() { std::cout << "Event: Start -> Entering Running State\n"; }
    void onStop()  { std::cout << "Event: Stop -> Entering Stopped State\n"; }
    void onReset() { std::cout << "Event: Reset -> Returning to Idle State\n"; }

    // The core of the state machine: the transition table
    using transition_table = TransitionTable<
    //  +-----------+--------------+-----------+------------------------------+
    //  | From      | Event        | To        | Action                       |
    //  +-----------+--------------+-----------+------------------------------+
        Row<Idle,     StartEvent,    Running,   &VendingMachine_::onStart>,
        Row<Running,  StopEvent,     Stopped,   &VendingMachine_::onStop>,
        Row<Stopped,  ResetEvent,    Idle,      &VendingMachine_::onReset>,
        Row<Running,  ResetEvent,    Idle,      nullptr> // Transition with no action
    >;
};

// Create a final type alias for your state machine
using VendingMachine = msm::StateMachine<VendingMachine_>;
```

### 3\. Use the State Machine

Instantiate the state machine and send events to it.

```cpp
int main() {
    // Create the state machine. It starts in the first state from the
    // transition table by default (`Idle`).
    VendingMachine sm;
    std::cout << "Initial state: Idle\n";
    ASSERT(sm.check_state<Idle>());

    // An event that has no transition from the current state is ignored.
    sm.send_event<StopEvent>();
    ASSERT(sm.check_state<Idle>()); // Still Idle

    // Send a valid event to trigger a transition.
    sm.send_event<StartEvent>(); // "Event: Start -> Entering Running State"
    ASSERT(sm.check_state<Running>());

    // Send another event.
    sm.send_event<StopEvent>();  // "Event: Stop -> Entering Stopped State"
    ASSERT(sm.check_state<Stopped>());

    // You can also create a machine starting in a specific state.
    VendingMachine sm_stopped{Stopped{}};
    ASSERT(sm_stopped.check_state<Stopped>());

    // Transitioning with a nullptr action just changes the state.
    sm.send_event<ResetEvent>(); // No output message here
    ASSERT(sm.check_state<Idle>());
    
    return 0;
}
```

-----

## API Reference

Key public methods of `msm::StateMachine<YourDefinition>`:

  * **`StateMachine()`**: Constructs the state machine. The initial state is the *first source state* encountered in the transition table.
  * **`StateMachine(InitialState state)`**: Constructs the state machine in a specific initial state.
  * **`void send_event<Event>()`**: Sends an event to the state machine. If a valid transition exists for the current state and this event, the state is changed and the associated action is executed. Otherwise, nothing happens.
  * **`bool check_state<State>() const`**: Returns `true` if the machine is currently in the specified `State`, `false` otherwise.
  * **`YourDefinition* operator->()`**: Provides direct access to the instance of your definition class, allowing you to call its public methods (e.g., `sm->some_public_method()`).

## License

This library is licensed under the MIT License. See the LICENSE file for details.