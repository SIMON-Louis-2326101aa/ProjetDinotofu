// EN: InventoryMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/InventoryMenu.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventory/InventoryDisplay.hpp"
#include "interface/menu/inventory/InventorySelection.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/TerminalInterface.hpp"

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
bool InventoryMenu::open(Player& player)
{
    while (true)
    {
        MenuScreen screen = InventoryDisplay::buildMainScreen();
        int menuChoice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option visible de l'inventaire."
        );

        Console::clear();

        if (menuChoice == 0) return false;

        if (menuChoice == 1)
        {
            BestiaryMenu::openBestiary();
            continue;
        }

        if (menuChoice == 2)
        {
            BestiaryMenu::openEncyclopedia();
            continue;
        }

        if (menuChoice == 3)
        {
            BestiaryMenu::openDiscoveryNotebook();
            continue;
        }

        if (menuChoice == 4)
        {
            InventoryDisplay::displaySimpleFullInventory(player);
            continue;
        }

        if (menuChoice == 5)
        {
            InventorySelection::openWeapons(player);
            continue;
        }

        if (menuChoice == 6)
        {
            InventorySelection::openArmors(player);
            continue;
        }

        if (menuChoice == 7)
        {
            bool turnConsumed = InventorySelection::openConsumables(player);
            if (turnConsumed) return true;
            continue;
        }

        if (menuChoice == 8)
        {
            InventorySelection::openMaterials(player);
            continue;
        }

        if (menuChoice == 9)
        {
            InventorySelection::openCraft(player);
            continue;
        }

        if (menuChoice == 10)
        {
            QuestMenu::consultOnly(player);
            continue;
        }
    }
}
