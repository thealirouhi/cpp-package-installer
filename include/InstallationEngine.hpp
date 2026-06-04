#ifndef INSTALLATION_ENGINE_HPP
#define INSTALLATION_ENGINE_HPP

#include <string>
#include <vector>

#include "Installable.hpp"
#include "Module.hpp"
#include "Package.hpp"
#include "SystemLogger.hpp"
#include "TransactionContext.hpp"

class InstallationEngine {
private:
    std::vector<Installable*> allComponents;
    SystemLogger logger;

    Installable* getComponent(const std::string& id);

public:
    InstallationEngine();
    ~InstallationEngine();

    // creation
    void addModule(const std::string& id, const std::string& title);
    void addPackage(const std::string& id, const std::string& title);

    // structure
    void attach(const std::string& parentId, const std::string& childId);

    // lifecycle commands
    void install(const std::string& id);
    void uninstall(const std::string& id);

    // bulk operations
    void installAll();
    void uninstallAll();

    // state control commands
    void mockFail(const std::string& id);
    void resolve(const std::string& id);

    // optional utility (useful for debugging / CMD C3 type commands)
    void printComponent(const std::string& id) const;
};

#endif