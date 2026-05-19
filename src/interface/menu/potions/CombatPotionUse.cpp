// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionUse.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "interface/menu/CombatTargetMenu.hpp"

#include "item/Inventory.hpp"

#include <iostream>

bool CombatPotionUse::useHealingPotion(
    Player& player,
    int consumableIndex,
    const Consumable& potion
)
{
    if (!player.getInventory().hasConsumable(consumableIndex))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.heal(potion.getPower());
    ThreatSystem::markSelfHealingAction(player);
    player.getInventory().removeConsumable(consumableIndex);

    std::cout << player.getName()
              << " boit "
              << potion.getName()
              << " et récupère "
              << potion.getPower()
              << " PV."
              << std::endl;
    std::cout << std::endl;

    return true;
}

bool CombatPotionUse::useSelectedPotion(
    Player& player,
    int consumableIndex,
    ConsumableType type,
    Entity* target,
    EnemyCombatQueue* wave,
    Random& random,
    int potionDamageBonus
)
{
    if (!player.getInventory().hasConsumable(consumableIndex))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consumable potion = player.getInventory().getConsumable(consumableIndex);

    if (type == ConsumableType::Healing)
    {
        return useHealingPotion(player, consumableIndex, potion);
    }

    if (type == ConsumableType::Damage)
    {
        int totalBonus = potion.getPower();

        if (potionDamageBonus > 0)
        {
            totalBonus = potion.getPower();
        }

        if (wave != nullptr)
        {
            bool attackLaunched = CombatTargetMenu::openForBoostedAttack(
                player,
                *wave,
                random,
                totalBonus
            );

            if (attackLaunched)
            {
                player.getInventory().removeConsumable(consumableIndex);
            }

            return attackLaunched;
        }

        if (target != nullptr)
        {
            player.getInventory().removeConsumable(consumableIndex);

            std::cout << player.getName()
                      << " utilise "
                      << potion.getName()
                      << "."
                      << std::endl;
            std::cout << std::endl;

            CombatAttack::executeBoostedAttack(
                player,
                *target,
                random,
                totalBonus
            );

            return true;
        }

        std::cout << "Aucune cible offensive disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (type == ConsumableType::Buff || type == ConsumableType::Debuff)
    {
        std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
        std::cout << "La potion existe, mais son effet n'est pas encore codé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;

    return false;
}