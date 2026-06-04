#ifndef COMPONENT_STATE_HPP
#define COMPONENT_STATE_HPP

#include <string>

enum class ComponentState {
    PENDING,
    INSTALLED,
    FAILED
};

std::string stateToString(ComponentState state);

#endif