#include "Module.hpp"

Module::Module(
    const std::string& id,
    const std::string& title
)
    : Installable(id, title)
{
}

bool Module::install(TransactionContext& tx)
{
    // Mock failure has highest priority
    if (isMockFail())
    {
        setState(ComponentState::FAILED);
        return false;
    }

    // Already installed
    if (getState() == ComponentState::INSTALLED)
    {
        return true;
    }

    // Install successfully
    setState(ComponentState::INSTALLED);

    // Record transaction for potential rollback
    tx.stateChangedNodes.push_back(this);

    return true;
}

void Module::uninstall()
{
    setState(ComponentState::PENDING);
}