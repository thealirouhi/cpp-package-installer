#include <iostream>

#include "Module.hpp"
#include "Package.hpp"
// #include "InstallationEngine.hpp"
#include "TransactionContext.hpp"

int main()
{
    TransactionContext tx;
    // InstallationEngine installationEngine;

    // Leaf modules
    Module *a = new Module("A", "Core A");
    Module *b = new Module("B", "Core B");
    Module *c = new Module("C", "Core C");

    // Composite package
    Package *pkg1 = new Package("P1", "Package 1");
    pkg1->addChild(a);
    pkg1->addChild(b);

    Package *root = new Package("ROOT", "Root Package");
    root->addChild(pkg1);
    root->addChild(c);

    std::cout << "=== INSTALL ROOT ===\n";
    // bool ok = installationEngine.install(root, tx);
    bool ok = root->install(tx);

    std::cout << "\nInstall result: " << (ok ? "SUCCESS" : "FAIL") << "\n";

    std::cout << "\n=== SECOND INSTALL (should handle duplicates) ===\n";
    // installationEngine.install(root, tx);
    bool ok2 = root->install(tx);

    std::cout << "\n=== UNINSTALL ROOT ===\n";
    root->uninstall();

    delete root;

    return 0;
}