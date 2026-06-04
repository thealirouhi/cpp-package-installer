#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <vector>

#include "Installable.hpp"

class Package : public Installable
{
private:
    std::vector<Installable*> children;

public:
    Package(
        const std::string& id,
        const std::string& title
    );

    bool isPackage() const override;

    bool hasChild(const std::string& childId) const;

    bool containsDescendant(const std::string& descendantId) const;

    void addChild(Installable* child);

    bool install(TransactionContext& tx) override;

    void uninstall() override;
};

#endif