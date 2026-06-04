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

