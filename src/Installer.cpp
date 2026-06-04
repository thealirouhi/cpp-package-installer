#include "Installer.hpp"

bool Installer::install(Installable *root, TransactionContext &tx)
{
    if (!root)
        return false;

    return root->install(tx);
}