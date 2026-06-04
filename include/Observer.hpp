#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include "ComponentState.hpp"

class Installable;

class Observer {
public:
    virtual void onStateChanged(
        const Installable* component,
        ComponentState oldState,
        ComponentState newState
    ) = 0;

    virtual ~Observer() = default;
};

#endif