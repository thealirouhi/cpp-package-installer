#ifndef MODULE_HPP
#define MODULE_HPP

#include "Installable.hpp"

class Module : public Installable
{
public:
    Module(
        const std::string& id,
        const std::string& title
    );

    bool install(TransactionContext& tx) override;

    void uninstall() override;
};

#endif