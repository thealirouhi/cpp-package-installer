#include "InstallationEngine.hpp"
#include <iostream>

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

void InstallationEngine::install(const std::string& id)
{
    Installable* comp = getComponent(id);

    if (!comp)
    {
        std::cout << "ERROR: Invalid command\n";
        return;
    }

    TransactionContext tx;
    comp->install(tx);
}

void InstallationEngine::uninstall(const std::string& id)
{
    Installable* comp = getComponent(id);

    if (!comp)
    {
        std::cout << "ERROR: Invalid command\n";
        return;
    }

    comp->uninstall();
}

void InstallationEngine::installAll()
{
    TransactionContext tx;

    for (auto* comp : allComponents)
    {
        comp->install(tx);
    }
}

void InstallationEngine::uninstallAll()
{
    for (auto it = allComponents.rbegin(); it != allComponents.rend(); ++it)
    {
        (*it)->uninstall();
    }
}
void InstallationEngine::mockFail(const std::string& id)
{
    Installable* comp = getComponent(id);

    if (!comp)
    {
        std::cout << "ERROR: Invalid command\n";
        return;
    }

    comp->setMockFail(true);
}

void InstallationEngine::resolve(const std::string& id)
{
    Installable* comp = getComponent(id);

    if (!comp)
    {
        std::cout << "ERROR: Invalid command\n";
        return;
    }

    comp->setMockFail(false);
}

