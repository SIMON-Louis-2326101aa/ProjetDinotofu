// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/InventoryMenu.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventory/InventoryDisplay.hpp"
#include "interface/menu/inventory/InventorySelection.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"

bool InventoryMenu::open(Player& player)
{
    while (true)
    {
        InventoryDisplay::displayMainMenu();

        int menuChoice = Console::askNumberBetween(
            0,
            6,
            "Choix invalide. Entre un chiffre entre 0 et 6."
        );

        Console::clear();

        if (menuChoice == 0)
        {
            return false;
        }

        if (menuChoice == 1)
        {
            BestiaryMenu::open();
            continue;
        }

        if (menuChoice == 2)
        {
            InventoryDisplay::displaySimpleFullInventory(player);
            continue;
        }

        if (menuChoice == 3)
        {
            InventorySelection::openWeapons(player);
            continue;
        }

        if (menuChoice == 4)
        {
            InventorySelection::openArmors(player);
            continue;
        }

        if (menuChoice == 5)
        {
            bool turnConsumed = InventorySelection::openConsumables(player);

            if (turnConsumed)
            {
                return true;
            }

            continue;
        }

        if (menuChoice == 6)
        {
            InventorySelection::openMaterials(player);
            continue;
        }
    }
}