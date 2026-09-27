#ifndef INCLUDE_COMBAT_DIALOGUE_ENCOUNTERDIALOGUESYSTEM_HPP
#define INCLUDE_COMBAT_DIALOGUE_ENCOUNTERDIALOGUESYSTEM_HPP

#include <string>

class Player;
class EnemyCombatQueue;
class Random;

class EncounterDialogueSystem
{
public:
    static bool display(
        const Player& player,
        const EnemyCombatQueue& wave,
        Random& random,
        const std::string& screenIdPrefix
    );
};

#endif
