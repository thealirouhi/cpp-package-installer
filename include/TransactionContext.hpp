#ifndef TRANSACTION_CONTEXT_HPP
#define TRANSACTION_CONTEXT_HPP

#include <vector>

class Installable;

struct TransactionContext {
    std::vector<Installable*> stateChangedNodes;
    std::vector<Installable*> countIncreasedNodes;
};

#endif