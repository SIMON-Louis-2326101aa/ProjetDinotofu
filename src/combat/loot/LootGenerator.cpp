// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// English: Implements simple monster loot generation for the first playable reward pass.
// Français : Implémente une génération simple de loots de monstres pour la première version jouable des récompenses.

#include "combat/loot/LootGenerator.hpp"

#include "entity/Race.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <iostream>

void LootGenerator::giveDefeatedWaveLoot(
    Player& player,
    const EnemyCombatQueue& wave,
    Random& random,
    DifficultyMode difficulty
)
{
    if (wave.getDefeatedEnemyCount() <= 0)
    {
        return;
    }

    int lootChance = getLootChance(difficulty);
    bool atLeastOneLoot = false;

    std::cout << "========== BUTIN ==========" << std::endl;

    for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
    {
        const Monster& monster = wave.getDefeatedEnemy(i);
        if (tryGiveMonsterLoot(player, monster, random, difficulty, lootChance))
        {
            atLeastOneLoot = true;
        }
    }

    if (!atLeastOneLoot)
    {
        std::cout << "Aucun matériau intéressant récupéré cette fois." << std::endl;
    }

    std::cout << "===========================" << std::endl;
    std::cout << std::endl;
}

int LootGenerator::getLootChance(DifficultyMode difficulty)
{
    switch (difficulty)
    {
        case DifficultyMode::Easy:
            return 75;

        case DifficultyMode::Hard:
            return 55;

        case DifficultyMode::Nightmare:
            return 45;

        case DifficultyMode::Lethal:
            return 40;

        case DifficultyMode::Normal:
        default:
            return 65;
    }
}

int LootGenerator::getLootQuantity(
    Random& random,
    DifficultyMode difficulty,
    const Monster& monster
)
{
    int quantity = 1;

    if (monster.isElite())
    {
        quantity++;
    }

    if (monster.getLevel() >= 4 && random.between(1, 100) <= 35)
    {
        quantity++;
    }

    switch (difficulty)
    {
        case DifficultyMode::Easy:
            if (random.between(1, 100) <= 35) quantity++;
            break;

        case DifficultyMode::Hard:
            if (random.between(1, 100) <= 15) quantity++;
            break;

        case DifficultyMode::Nightmare:
            if (random.between(1, 100) <= 8) quantity++;
            break;

        case DifficultyMode::Lethal:
            break;

        case DifficultyMode::Normal:
        default:
            if (random.between(1, 100) <= 25) quantity++;
            break;
    }

    return quantity;
}

bool LootGenerator::tryGiveMonsterLoot(
    Player& player,
    const Monster& monster,
    Random& random,
    DifficultyMode difficulty,
    int lootChance
)
{
    if (random.between(1, 100) > lootChance)
    {
        return false;
    }

    int quantity = getLootQuantity(random, difficulty, monster);
    Material loot;

    switch (monster.getRace())
    {
        case Race::Gobelin:
            loot = MaterialCatalog::createGoblinEar(quantity);
            break;

        case Race::Bete:
            loot = MaterialCatalog::createWolfFang(quantity);
            break;

        case Race::Humain:
        case Race::Orc:
        case Race::MortVivant:
            loot = MaterialCatalog::createRustedMetalFragment(quantity);
            break;

        case Race::Plante:
            loot = MaterialCatalog::createBitterHealingLeaf(quantity);
            break;

        default:
            loot = MaterialCatalog::createWornLeatherPiece(quantity);
            break;
    }

    player.getInventory().addMaterial(loot);

    std::cout << monster.getName()
              << " laisse quelque chose derrière lui : "
              << loot.getName()
              << " x"
              << loot.getQuantity()
              << "."
              << std::endl;

    return true;
}
