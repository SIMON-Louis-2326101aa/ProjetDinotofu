// EN: InitiativeRoll.hpp stores one reusable initiative result.
// FR: InitiativeRoll.hpp mémorise un résultat d'initiative réutilisable.

#ifndef INCLUDE_COMBAT_INITIATIVE_INITIATIVEROLL_HPP
#define INCLUDE_COMBAT_INITIATIVE_INITIATIVEROLL_HPP

#include <string>

enum class InitiativeSide
{
    Players,
    Enemies
};

struct InitiativeRoll
{
    std::string id;
    std::string label;
    InitiativeSide side = InitiativeSide::Players;
    int slotIndex = -1;
    int baseScore = 0;
    int dieRoll = 0;
    int totalScore = 0;
};

#endif
