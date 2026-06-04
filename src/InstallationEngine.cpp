#include "InstallationEngine.hpp"
#include <iostream>

#include "Module.hpp"
#include "Package.hpp"

InstallationEngine::InstallationEngine()
{
}

InstallationEngine::~InstallationEngine()
{
    for (auto* c : allComponents)
    {
        delete c;
    }
}

void InstallationEngine::addModule(const std::string& id, const std::string& title)
{
    if (getComponent(id) != nullptr)
    {
        std::cout << "ERROR: Component with ID " << id << " already exists\n";
        return;
    }

    Installable* m = new Module(id, title);
    m->addObserver(&logger);
    allComponents.push_back(m);
}

void InstallationEngine::addPackage(const std::string& id, const std::string& title)
{
    if (getComponent(id) != nullptr)
    {
        std::cout << "ERROR: Component with ID " << id << " already exists\n";
        return;
    }

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
    if (!parent)
    {
        std::cout << "ERROR: Component " << parentId << " does not exist\n";
        return;
    }

    Installable* child = getComponent(childId);
    if (!child)
    {
        std::cout << "ERROR: Component " << childId << " does not exist\n";
        return;
    }

    if (!parent->isPackage())
    {
        std::cout << "ERROR: Cannot attach to a module\n";
        return;
    }

    if (parent->getState() == ComponentState::INSTALLED)
    {
        std::cout << "ERROR: Cannot attach to an already installed package\n";
        return;
    }

    Package* pkg = static_cast<Package*>(parent);
    if (pkg->hasChild(childId))
    {
        std::cout << "ERROR: Component " << childId << " is already attached to " << parentId << "\n";
        return;
    }

    pkg->addChild(child);
}

void InstallationEngine::install(const std::string& id)
{
    Installable* comp = getComponent(id);

    if (!comp)
    {
        std::cout << "ERROR: Component " << id << " does not exist\n";
        return;
    }

    if (comp->getState() == ComponentState::INSTALLED)
    {
        std::cout << "ERROR: Component " << id << " is already installed\n";
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
        std::cout << "ERROR: Component " << id << " does not exist\n";
        return;
    }

    if (comp->getState() != ComponentState::INSTALLED)
    {
        std::cout << "ERROR: Component " << id << " is not currently installed\n";
        return;
    }

    for (auto* c : allComponents)
    {
        if (c->isPackage() && c != comp && c->getState() == ComponentState::INSTALLED)
        {
            Package* pkg = static_cast<Package*>(c);
            if (pkg->containsDescendant(id))
            {
                std::cout << "ERROR: Component " << id << " is required by another package\n";
                return;
            }
        }
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
    bool anyInstalled = false;
    for (auto* c : allComponents)
    {
        if (c->getState() == ComponentState::INSTALLED)
        {
            anyInstalled = true;
            break;
        }
    }

    if (!anyInstalled)
    {
        std::cout << "ERROR: No installed components to uninstall\n";
        return;
    }

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
        std::cout << "ERROR: Component " << id << " does not exist\n";
        return;
    }

    if (comp->isMockFail())
    {
        std::cout << "ERROR: Component " << id << " is already set to fail\n";
        return;
    }

    comp->setMockFail(true);
}

void InstallationEngine::resolve(const std::string& id)
{
    Installable* comp = getComponent(id);

    if (!comp)
    {
        std::cout << "ERROR: Component " << id << " does not exist\n";
        return;
    }

    if (!comp->isMockFail())
    {
        std::cout << "ERROR: Component " << id << " is not in a mock fail state\n";
        return;
    }

    comp->setMockFail(false);
}

void InstallationEngine::printComponent(const std::string& id) const
{
    for (const auto* c : allComponents)
    {
        if (c->getId() == id)
        {
            std::string type = c->isPackage() ? "PACKAGE" : "MODULE";
            std::cout << c->getId() << " (" << type << "): "
                      << c->getTitle() << " ["
                      << stateToString(c->getState()) << "]\n";
            return;
        }
    }

    std::cout << "ERROR: Component " << id << " does not exist\n";
}
