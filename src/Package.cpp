#include "Package.hpp"

Package::Package(
    const std::string& id,
    const std::string& title
)
    : Installable(id, title)
{
}

bool Package::isPackage() const
{
    return true;
}

void Package::addChild(Installable* child)
{
    children.push_back(child);
}

bool Package::install(TransactionContext& tx)
{
    // 1. mock failure
    if (isMockFail())
    {
        setState(ComponentState::FAILED);
        return false;
    }

    // 2. already installed
    if (getState() == ComponentState::INSTALLED)
    {
        return true;
    }

    // 3. install children
    for (Installable* child : children)
    {
        bool ok = child->install(tx);

        if (!ok)
        {
            // rollback everything done in this transaction
            for (int i = (int)tx.stateChangedNodes.size() - 1; i >= 0; --i)
            {
                tx.stateChangedNodes[i]->forcePending();
            }

            tx.stateChangedNodes.clear();
            tx.countIncreasedNodes.clear();

            setState(ComponentState::FAILED);
            return false;
        }
    }

    // 4. success → mark installed
    setState(ComponentState::INSTALLED);

    // record this package too
    tx.stateChangedNodes.push_back(this);

    return true;
}

void Package::uninstall()
{
    // Only mark itself pending
    setState(ComponentState::PENDING);
}