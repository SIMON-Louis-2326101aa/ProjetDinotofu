// EN: EscapeSystem.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: EscapeSystem.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_SYSTEM_ESCAPESYSTEM_HPP
#define INCLUDE_COMBAT_SYSTEM_ESCAPESYSTEM_HPP

#include "core/Random.hpp"

#include "entity/Entity.hpp"
#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"
#include "progression/DifficultyMode.hpp"


enum class MonsterEscapeAttemptState
{
    NotEligible,
    Blocked,
    Progressed,
    Escaped
};

struct MonsterEscapeAttemptResult
{
    MonsterEscapeAttemptState state = MonsterEscapeAttemptState::NotEligible;
    int requiredOpenings = 1;
    int progressBefore = 0;
    int progressAfter = 0;
    int baseChancePercent = 0;
    int mobilityModifierPercent = 0;
    int carryLoadModifierPercent = 0;
    int retryBonusPercent = 0;
    int effectiveChancePercent = 0;
    int roll = 0;
};

class EscapeSystem
{
public:
    // EN: playerAttemptsEscape declares or implements a focused behavior used by this module.
    // FR: playerAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool playerAttemptsEscape(Player& player, Random& random);
    static bool playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty);
    static bool playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty, int enemyCount);
    static int playerEscapeMobilityModifier(const Player& player);
    static MonsterEscapeAttemptResult attemptPlayerEscapeStep(
        Player& player,
        Random& random,
        DifficultyMode difficulty,
        int opponentCount
    );

    // Shared escape pressure for every camp: more living opponents able to intercept means more
    // successive openings are required. Waiting/reserve opponents can count when physically nearby.
    static int escapeOpeningsRequired(int opponentCount);
    static bool rollEscapeAgainstOpposition(Random& random, int perOpeningChancePercent, int opponentCount);

    // Monster escape uses the same opposition pressure as the player, but resolves one opening per
    // committed escape action. A successful opening is remembered for the current monster instance,
    // making a follow-up attempt easier instead of restarting from zero every turn.
    static int monsterEscapeMobilityModifier(const Monster& monster);
    static MonsterEscapeAttemptResult attemptMonsterEscapeStep(
        Monster& monster,
        Random& random,
        int opponentCount,
        int baseChancePercent,
        bool requireLowHpEligibility
    );
    // EN: playerAttemptsBossEscape declares or implements a focused behavior used by this module.
    // FR: playerAttemptsBossEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool playerAttemptsBossEscape(const Player& player, const Boss& boss);

    static bool playerAttemptsDuelEscape(
        Player& runner,
        Entity& opponent,
        Random& random
    );

    // EN: monsterCanAttemptEscape declares or implements a focused behavior used by this module.
    // FR: monsterCanAttemptEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool monsterCanAttemptEscape(const Monster& monster);
    // EN: monsterAttemptsEscape declares or implements a focused behavior used by this module.
    // FR: monsterAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
    static bool monsterAttemptsEscape(Monster& monster, Random& random);
    static bool monsterAttemptsEscape(Monster& monster, Random& random, int opponentCount);

private:
    static constexpr int LOW_MONSTER_ESCAPE_CHANCE = 20;

    static int calculateDuelEscapeChance(
        const Player& runner,
        const Entity& opponent
    );

    // EN: endCombatBySurrender declares or implements a focused behavior used by this module.
    // FR: endCombatBySurrender déclare ou implémente un comportement précis utilisé par ce module.
    static void endCombatBySurrender(Player& runner);
};

#endif
