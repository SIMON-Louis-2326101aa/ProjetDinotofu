// EN: SummonCombatSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonCombatSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Handles active summons during combat and lets summoner classes slowly turn duels into small group fights.

#ifndef INCLUDE_COMBAT_SUMMON_SUMMONCOMBATSYSTEM_HPP
#define INCLUDE_COMBAT_SUMMON_SUMMONCOMBATSYSTEM_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "combat/summon/Summon.hpp"
#include "combat/summon/SummonControlMode.hpp"
#include "core/Random.hpp"
#include "entity/Entity.hpp"
#include "entity/Player.hpp"

#include <vector>

class SummonCombatSystem
{
public:
    // EN: createInitialSummonsFor declares or implements a focused behavior used by this module.
    // FR: createInitialSummonsFor déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<Summon> createInitialSummonsFor(const Player& owner);

    // EN: hasActiveSummons declares or implements a focused behavior used by this module.
    // FR: hasActiveSummons déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasActiveSummons(const std::vector<Summon>& summons);

    // EN: calculateUsedSlots declares or implements a focused behavior used by this module.
    // FR: calculateUsedSlots déclare ou implémente un comportement précis utilisé par ce module.
    static int calculateUsedSlots(const std::vector<Summon>& summons);

    static void displaySummonArrival(
        const Player& owner,
        const std::vector<Summon>& summons
    );

    static SummonControlMode askPlayerSummonControlMode(
        const Player& owner,
        const std::vector<Summon>& summons
    );

    static void playSummonTurnsAgainstEntity(
        std::vector<Summon>& summons,
        Entity& target,
        Random& random
    );

    static void playPlayerSummonTurnsAgainstEntity(
        std::vector<Summon>& summons,
        Entity& target,
        Random& random,
        SummonControlMode controlMode
    );

    static void playSummonTurnsAgainstWave(
        std::vector<Summon>& summons,
        EnemyCombatQueue& wave,
        Random& random
    );

    static void playPlayerSummonTurnsAgainstWave(
        std::vector<Summon>& summons,
        EnemyCombatQueue& wave,
        Random& random,
        SummonControlMode controlMode
    );

    // EN: removeInactiveSummons declares or implements a focused behavior used by this module.
    // FR: removeInactiveSummons déclare ou implémente un comportement précis utilisé par ce module.
    static void removeInactiveSummons(std::vector<Summon>& summons);

    // EN: maintainSummonLink declares or implements a focused behavior used by this module.
    // FR: maintainSummonLink déclare ou implémente un comportement précis utilisé par ce module.
    static bool maintainSummonLink(Summon& summon);

    // EN: sacrificeSummonAgainstEntity declares or implements a focused behavior used by this module.
    // FR: sacrificeSummonAgainstEntity déclare ou implémente un comportement précis utilisé par ce module.
    static void sacrificeSummonAgainstEntity(Summon& summon, Entity& target, Random& random);

    // EN: hasTargetableSummons declares or implements a focused behavior used by this module.
    // FR: hasTargetableSummons déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasTargetableSummons(const std::vector<Summon>& summons);

    static int chooseRandomTargetableSummonIndex(
        const std::vector<Summon>& summons,
        Random& random
    );

    static int chooseStrategicTargetableSummonIndex(
        const std::vector<Summon>& summons,
        const Entity& attacker,
        Random& random
    );

    // EN: displayTargetableSummons declares or implements a focused behavior used by this module.
    // FR: displayTargetableSummons déclare ou implémente un comportement précis utilisé par ce module.
    static void displayTargetableSummons(const std::vector<Summon>& summons);

    static void entityAttacksSummon(
        Entity& attacker,
        Summon& summon,
        Random& random
    );

private:
    static void playManualSummonTurnAgainstEntity(
        Summon& summon,
        Entity& target,
        Random& random
    );

    static void playManualSummonTurnAgainstWave(
        Summon& summon,
        EnemyCombatQueue& wave,
        Random& random
    );

    // EN: hasSpecialAbility declares or implements a focused behavior used by this module.
    // FR: hasSpecialAbility déclare ou implémente un comportement précis utilisé par ce module.
    static bool hasSpecialAbility(const Summon& summon);

    static void executeSummonAttack(
        Summon& summon,
        Entity& target,
        Random& random
    );

    static bool executeSummonSpecialAbility(
        Summon& summon,
        Entity& target,
        Random& random
    );

    static int rollSummonDamage(
        const Summon& summon,
        Random& random,
        bool& dodged
    );
};

#endif
