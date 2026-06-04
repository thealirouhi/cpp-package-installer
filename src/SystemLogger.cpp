#include "SystemLogger.hpp"

void SystemLogger::onStateChanged(
    const Installable *component,
    ComponentState oldState,
    ComponentState newState)
{
    std::string msg =
        "[OBSERVER] Component " + component->getId() +
        " changed from " + stateToString(oldState) +
        " to " + stateToString(newState);

    std::cout << msg << "\n";
}