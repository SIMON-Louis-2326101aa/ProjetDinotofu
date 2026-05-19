// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// English: Generates simple monster loot and sends it directly to the player inventory.
// Français : Génère les premiers loots de monstres et les ajoute directement à l'inventaire du joueur.

#ifndef INCLUDE_COMBAT_LOOT_LOOTGENERATOR_HPP
#define INCLUDE_COMBAT_LOOT_LOOTGENERATOR_HPP

#include "combat/EnemyCombatQueue.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
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

private:
    static int getLootChance(DifficultyMode difficulty);
    static int getLootQuantity(Random& random, DifficultyMode difficulty, const Monster& monster);
    static bool tryGiveMonsterLoot(Player& player, const Monster& monster, Random& random, DifficultyMode difficulty, int lootChance);
};

#endif
