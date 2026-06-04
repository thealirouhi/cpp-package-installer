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

bool Package::hasChild(const std::string& childId) const
{
    for (const auto* c : children)
    {
        if (c->getId() == childId)
        {
            return true;
        }
    }

    return false;
}

bool Package::containsDescendant(const std::string& descendantId) const
{
    for (const auto* c : children)
    {
        if (c->getId() == descendantId)
        {
            return true;
        }

        if (c->isPackage())
        {
            const Package* pkg = static_cast<const Package*>(c);
            if (pkg->containsDescendant(descendantId))
            {
                return true;
            }
        }
    }

    return false;
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
            for (int i = (int)tx.stateChangedNodes.size() - 1; i >= 0; --i)
            {
                tx.stateChangedNodes[i]->forcePending();
            }

            for (auto* node : tx.countIncreasedNodes)
            {
                node->decrementParents();
            }

            tx.stateChangedNodes.clear();
            tx.countIncreasedNodes.clear();

            setState(ComponentState::FAILED);
            return false;
        }

        child->incrementParents();
        tx.countIncreasedNodes.push_back(child);
    }

    // 4. success → mark installed
    setState(ComponentState::INSTALLED);

    // record this package too
    tx.stateChangedNodes.push_back(this);

    return true;
}

void Package::uninstall()
{
    if (getState() == ComponentState::PENDING)
    {
        return;
    }

    setState(ComponentState::PENDING);

    for (int i = (int)children.size() - 1; i >= 0; --i)
    {
        Installable* child = children[i];
        child->decrementParents();

        if (child->getInstalledParentsCount() == 0 && !child->isExplicitlyInstalled())
        {
            child->uninstall();
        }
    }
}