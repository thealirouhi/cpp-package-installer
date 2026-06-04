#include "Installable.hpp"

Installable::Installable(
    const std::string& id,
    const std::string& title
)
    : id(id),
      title(title),
      state(ComponentState::PENDING),
      mockFail(false),
      installedParentsCount(0),
      explicitlyInstalled(false)
{
}

std::string Installable::getId() const
{
    return id;
}

std::string Installable::getTitle() const
{
    return title;
}

ComponentState Installable::getState() const
{
    return state;
}

bool Installable::isMockFail() const
{
    return mockFail;
}

int Installable::getInstalledParentsCount() const
{
    return installedParentsCount;
}

bool Installable::isExplicitlyInstalled() const
{
    return explicitlyInstalled;
}

void Installable::setMockFail(bool value)
{
    mockFail = value;
}

void Installable::setExplicitlyInstalled(bool value)
{
    explicitlyInstalled = value;
}

void Installable::incrementParents()
{
    ++installedParentsCount;
}

void Installable::decrementParents()
{
    if (installedParentsCount > 0)
    {
        --installedParentsCount;
    }
}

void Installable::addObserver(Observer* observer)
{
    observers.push_back(observer);
}

void Installable::setState(ComponentState newState)
{
    if (state == newState)
    {
        return;
    }

    ComponentState oldState = state;
    state = newState;

    for (Observer* observer : observers)
    {
        observer->onStateChanged(
            this,
            oldState,
            newState
        );
    }
}

bool Installable::isPackage() const
{
    return false;
}

void Installable::forcePending()
{
    setState(ComponentState::PENDING);

    mockFail = false;
    installedParentsCount = 0;
    explicitlyInstalled = false;
}