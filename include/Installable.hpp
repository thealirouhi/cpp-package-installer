#ifndef INSTALLABLE_HPP
#define INSTALLABLE_HPP

#include <string>
#include <vector>

#include "ComponentState.hpp"
#include "Observer.hpp"
#include "TransactionContext.hpp"

class Installable {
protected:
    std::string id;
    std::string title;

    ComponentState state;

    bool mockFail;

    int installedParentsCount;

    bool explicitlyInstalled;

    std::vector<Observer*> observers;

public:
    Installable(
        const std::string& id,
        const std::string& title
    );

    virtual ~Installable() = default;

    // getters

    std::string getId() const;
    std::string getTitle() const;

    ComponentState getState() const;

    bool isMockFail() const;

    int getInstalledParentsCount() const;

    bool isExplicitlyInstalled() const;

    // setters

    void setMockFail(bool value);

    void setExplicitlyInstalled(bool value);

    void setState(ComponentState newState);

    // dependency counter

    void incrementParents();

    void decrementParents();

    // observer

    void addObserver(Observer* observer);

    // polymorphic interface

    virtual bool isPackage() const;

    virtual bool install(TransactionContext& tx) = 0;

    virtual void uninstall() = 0;

    virtual void forcePending();
};

#endif