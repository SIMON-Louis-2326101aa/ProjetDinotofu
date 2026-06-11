// EN: InitiativeQueue.cpp builds and sorts initiative entries.
// FR: InitiativeQueue.cpp construit et trie les entrées d'initiative.

#include "combat/initiative/InitiativeQueue.hpp"

#include "core/Random.hpp"

#include <algorithm>

void InitiativeQueue::clear()
{
    entries.clear();
}

void InitiativeQueue::add(const InitiativeRoll& entry)
{
    entries.push_back(entry);
}

void InitiativeQueue::rollAndSort(Random& random)
{
    for (InitiativeRoll& entry : entries)
    {
        entry.dieRoll = random.rollD20();
        entry.totalScore = entry.baseScore + entry.dieRoll;
    }

    std::stable_sort(entries.begin(), entries.end(), [](const InitiativeRoll& left, const InitiativeRoll& right) {
        if (left.totalScore != right.totalScore) return left.totalScore > right.totalScore;
        if (left.baseScore != right.baseScore) return left.baseScore > right.baseScore;
        if (left.side != right.side) return left.side == InitiativeSide::Players;
        return left.slotIndex < right.slotIndex;
    });
}

const std::vector<InitiativeRoll>& InitiativeQueue::getEntries() const
{
    return entries;
}
