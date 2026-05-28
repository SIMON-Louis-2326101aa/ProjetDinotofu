// EN: CombatPotionMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatPotionMenu.hpp"

#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/potions/CombatPotionDisplay.hpp"
#include "interface/menu/potions/CombatPotionUse.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/Consumable.hpp"

#include <iostream>

// EN: openQuickHealing declares or implements a focused behavior used by this module.
// FR: openQuickHealing déclare ou implémente un comportement précis utilisé par ce module.
bool CombatPotionMenu::openQuickHealing(Player& player)
{
    std::vector<int> indices = CombatPotionUtils::getPotionIndices(
        player,
        ConsumableType::Healing
    );

    if (indices.empty())
    {
        CombatPotionDisplay::showEmptyCategory("soin rapide");
        return false;
    }

    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            CombatPotionDisplay::buildQuickHealingScreen(player, indices),
            "Choix invalide. Sélectionne une potion affichée."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        int consumableIndex = indices[choice - 1];

        if (!player.getInventory().hasConsumable(consumableIndex))
        {
            CombatPotionDisplay::showPotionMissing();
            return false;
        }

        Consumable potion = player.getInventory().getConsumable(consumableIndex);

        int action = TerminalInterface::askMenuChoiceFromOptions(
            CombatPotionDisplay::buildSelectedHealingPotionScreen(potion),
            "Choix invalide. Entre 0, 1 ou 2."
        );

        Console::clear();

        if (action == 0)
        {
            return false;
        }

        if (action == 1)
        {
            CombatPotionDisplay::showPotionDetails(potion);
            return false;
        }

        if (action == 2)
        {
            return CombatPotionUse::useHealingPotion(
                player,
                consumableIndex,
                potion
            );
        }
    }
}

bool CombatPotionMenu::openAgainstSingleTarget(
    Player& player,
    Entity& target,
    Random& random,
    int potionDamageBonus
)
{
    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            CombatPotionDisplay::buildMainScreen(player),
            "Choix invalide. Entre un chiffre entre 0 et 7."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 1)
        {
            CombatPotionDisplay::displayPotions(player);
            return false;
        }

        if (choice == 2)
        {
            return openCategory(
                player,
                ConsumableType::Healing,
                &target,
                nullptr,
                random,
                potionDamageBonus
            );
        }

        if (choice == 3)
        {
            return openCategory(
                player,
                ConsumableType::Buff,
                &target,
                nullptr,
                random,
                potionDamageBonus
            );
        }

        if (choice == 4)
        {
            return openCategory(
                player,
                ConsumableType::Damage,
                &target,
                nullptr,
                random,
                potionDamageBonus
            );
        }

        if (choice == 5)
        {
            return openCategory(
                player,
                ConsumableType::Buff,
                &target,
                nullptr,
                random,
                potionDamageBonus
            );
        }

        if (choice == 6)
        {
            return openCategory(
                player,
                ConsumableType::Debuff,
                &target,
                nullptr,
                random,
                potionDamageBonus
            );
        }

        if (choice == 7)
        {
            return openCategory(
                player,
                ConsumableType::Special,
                &target,
                nullptr,
                random,
                potionDamageBonus
            );
        }
    }
}

bool CombatPotionMenu::openAgainstWave(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int potionDamageBonus
)
{
    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            CombatPotionDisplay::buildMainScreen(player),
            "Choix invalide. Entre un chiffre entre 0 et 7."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 1)
        {
            CombatPotionDisplay::displayPotions(player);
            return false;
        }

        if (choice == 2)
        {
            return openCategory(
                player,
                ConsumableType::Healing,
                nullptr,
                &wave,
                random,
                potionDamageBonus
            );
        }

        if (choice == 3)
        {
            return openCategory(
                player,
                ConsumableType::Buff,
                nullptr,
                &wave,
                random,
                potionDamageBonus
            );
        }

        if (choice == 4)
        {
            return openCategory(
                player,
                ConsumableType::Damage,
                nullptr,
                &wave,
                random,
                potionDamageBonus
            );
        }

        if (choice == 5)
        {
            return openCategory(
                player,
                ConsumableType::Buff,
                nullptr,
                &wave,
                random,
                potionDamageBonus
            );
        }

        if (choice == 6)
        {
            return openCategory(
                player,
                ConsumableType::Debuff,
                nullptr,
                &wave,
                random,
                potionDamageBonus
            );
        }

        if (choice == 7)
        {
            return openCategory(
                player,
                ConsumableType::Special,
                nullptr,
                &wave,
                random,
                potionDamageBonus
            );
        }
    }
}

bool CombatPotionMenu::openCategory(
    Player& player,
    ConsumableType type,
    Entity* target,
    EnemyCombatQueue* wave,
    Random& random,
    int potionDamageBonus
)
{
    std::vector<int> indices = CombatPotionUtils::getPotionIndices(
        player,
        type
    );

    if (indices.empty())
    {
        CombatPotionDisplay::showEmptyCategory(CombatPotionUtils::typeToText(type));
        return false;
    }

    return openPotionSelection(
        player,
        indices,
        type,
        target,
        wave,
        random,
        potionDamageBonus
    );
}

bool CombatPotionMenu::openPotionSelection(
    Player& player,
    const std::vector<int>& indices,
    ConsumableType type,
    Entity* target,
    EnemyCombatQueue* wave,
    Random& random,
    int potionDamageBonus
)
{
    if (indices.empty())
    {
        return false;
    }

    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            CombatPotionDisplay::buildFilteredPotionsScreen(player, indices),
            "Choix invalide. Sélectionne une potion affichée."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        int consumableIndex = indices[choice - 1];

        if (!player.getInventory().hasConsumable(consumableIndex))
        {
            CombatPotionDisplay::showPotionMissing();
            return false;
        }

        Consumable potion = player.getInventory().getConsumable(consumableIndex);

        int action = TerminalInterface::askMenuChoiceFromOptions(
            CombatPotionDisplay::buildSelectedPotionScreen(potion),
            "Choix invalide. Entre 0, 1 ou 2."
        );

        Console::clear();

        if (action == 0)
        {
            return false;
        }

        if (action == 1)
        {
            CombatPotionDisplay::showPotionDetails(potion);
            return false;
        }

        if (action == 2)
        {
            return CombatPotionUse::useSelectedPotion(
                player,
                consumableIndex,
                type,
                target,
                wave,
                random,
                potionDamageBonus
            );
        }
    }
}
