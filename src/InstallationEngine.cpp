#include "InstallationEngine.hpp"

InstallationEngine::InstallationEngine()
{
    // nothing required for now
}

void InstallationEngine::addModule(const std::string& id, const std::string& title)
{
    Installable* m = new Module(id, title);

    m->addObserver(&logger);

    allComponents.push_back(m);
}

void InstallationEngine::addPackage(const std::string& id, const std::string& title)
{
    Installable* p = new Package(id, title);

    p->addObserver(&logger);

    allComponents.push_back(p);
}

#include <iostream>

Installable* InstallationEngine::getComponent(const std::string& id)
{
    for (auto* c : allComponents)
    {
        if (c->getId() == id)
        {
            return c;
        }
    }

    return nullptr;
}

void InstallationEngine::attach(const std::string& parentId, const std::string& childId)
{
    Installable* parent = getComponent(parentId);
    Installable* child  = getComponent(childId);

    if (!parent || !child)
    {
        return;
    }

    if (!parent->isPackage())
    {
        return;
    }

    Package* pkg = static_cast<Package*>(parent);
    pkg->addChild(child);
}