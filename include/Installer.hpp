#ifndef INSTALLER_HPP
#define INSTALLER_HPP

#include "Installable.hpp"
#include "TransactionContext.hpp"

class Installer {
public:
    bool install(Installable* root, TransactionContext& tx);
};

#endif