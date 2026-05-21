// EN: LootGenerator.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: LootGenerator.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// English: Generates simple monster loot and sends it directly to the player inventory.
// Français : Génère les premiers loots de monstres et les ajoute directement à l'inventaire du joueur.

#ifndef INCLUDE_COMBAT_LOOT_LOOTGENERATOR_HPP
#define INCLUDE_COMBAT_LOOT_LOOTGENERATOR_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"
#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

class LootGenerator
{
public:
    static void giveDefeatedWaveLoot(
        Player& player,
        const EnemyCombatQueue& wave,
        Random& random,
        DifficultyMode difficulty
    );

    static void giveDefeatedBossLoot(
        Player& player,
        const Boss& boss,
        Random& random,
        DifficultyMode difficulty
    );

private:
    // EN: getLootChance declares or implements a focused behavior used by this module.
    // FR: getLootChance déclare ou implémente un comportement précis utilisé par ce module.
    static int getLootChance(DifficultyMode difficulty);
    // EN: getLootQuantity declares or implements a focused behavior used by this module.
    // FR: getLootQuantity déclare ou implémente un comportement précis utilisé par ce module.
    static int getLootQuantity(Random& random, DifficultyMode difficulty, const Monster& monster);
    // EN: tryGiveMonsterLoot declares or implements a focused behavior used by this module.
    // FR: tryGiveMonsterLoot déclare ou implémente un comportement précis utilisé par ce module.
    static bool tryGiveMonsterLoot(Player& player, const Monster& monster, Random& random, DifficultyMode difficulty, int lootChance);
};

#endif
