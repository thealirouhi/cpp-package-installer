#include "ComponentState.hpp"

std::string stateToString(ComponentState state) {
    switch (state) {
        case ComponentState::PENDING:
            return "PENDING";

        case ComponentState::INSTALLED:
            return "INSTALLED";

        case ComponentState::FAILED:
            return "FAILED";
    }

    return "";
}