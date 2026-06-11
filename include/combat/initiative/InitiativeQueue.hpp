// EN: InitiativeQueue.hpp builds and sorts initiative entries.
// FR: InitiativeQueue.hpp construit et trie les entrées d'initiative.

#ifndef INCLUDE_COMBAT_INITIATIVE_INITIATIVEQUEUE_HPP
#define INCLUDE_COMBAT_INITIATIVE_INITIATIVEQUEUE_HPP

#include "combat/initiative/InitiativeRoll.hpp"

#include <vector>

class Random;

class InitiativeQueue
{
private:
    std::vector<InitiativeRoll> entries;

public:
    void clear();
    void add(const InitiativeRoll& entry);
    void rollAndSort(Random& random);
    const std::vector<InitiativeRoll>& getEntries() const;
};

#endif
