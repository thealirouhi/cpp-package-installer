#ifndef SYSTEM_LOGGER_HPP
#define SYSTEM_LOGGER_HPP

#include <iostream>

#include "Observer.hpp"
#include "Installable.hpp"
#include "ComponentState.hpp"

class SystemLogger : public Observer {
public:
    void onStateChanged(
        const Installable* component,
        ComponentState oldState,
        ComponentState newState
    ) override;
};

#endif