// EN: InventoryMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventoryMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/InventoryMenu.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventory/InventoryDisplay.hpp"
#include "interface/menu/inventory/InventorySelection.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/TerminalInterface.hpp"

#include <algorithm>
#include <utility>
#include <vector>


namespace
{
    std::string selectionPolicyLabel(const std::string& policy)
    {
        if (policy == "favorite") return "Favori si disponible, sinon demander";
        if (policy == "auto") return "Automatique prudent";
        return "Toujours demander";
    }



    std::string lootPolicyLabel(const std::string& policy)
    {
        if (policy == "manual") return "Manuel uniquement";
        if (policy == "filter") return "Filtrage personnalisé";
        return "Tout récupérer automatiquement";
    }

    std::string enabledLabel(bool enabled)
    {
        return enabled ? "IGNORÉ" : "récupéré";
    }

    void openIgnoredMaterialLootMenu(Player& player)
    {
        while (true)
        {
            MenuScreen screen("FILTRE DE BUTIN — OBJETS", "inventory.loot_filter.materials");
            screen.addLine("Ici, tu peux exclure individuellement des matériaux déjà connus/possédés.");
            screen.addLine("Un objet filtré n'est jamais vendu ni détruit automatiquement : il est simplement laissé sur place et signalé.");
            screen.addLine("");

            std::vector<std::string> seenIds;
            std::vector<std::pair<std::string, std::string>> entries;
            for (const Material& material : player.getInventory().getMaterials())
            {
                if (std::find(seenIds.begin(), seenIds.end(), material.getId()) != seenIds.end()) continue;
                seenIds.push_back(material.getId());
                entries.push_back({material.getId(), material.getName()});
            }

            if (entries.empty())
            {
                screen.addLine("Aucun matériau connu à filtrer pour le moment.");
            }
            else
            {
                for (std::size_t i = 0; i < entries.size(); ++i)
                {
                    const bool ignored = player.isLootMaterialIgnored(entries[i].first);
                    screen.addOption(
                        static_cast<int>(i + 1),
                        std::string(ignored ? "[IGNORÉ] " : "[RAMASSÉ] ") + entries[i].second,
                        ignored ? "Autoriser de nouveau ce matériau." : "Laisser automatiquement ce matériau quand le filtrage est actif.",
                        true,
                        "inventory.loot_filter.material.toggle"
                    );
                }
            }
            screen.addBackOption("Retour", "inventory.loot_filter.materials.back");
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un matériau affiché.");
            Console::clear();
            if (choice == 0) return;
            if (choice >= 1 && static_cast<std::size_t>(choice) <= entries.size())
            {
                const std::string& id = entries[static_cast<std::size_t>(choice - 1)].first;
                player.setLootMaterialIgnored(id, !player.isLootMaterialIgnored(id));
            }
        }
    }

    void openLootPreferences(Player& player)
    {
        while (true)
        {
            MenuScreen screen("RAMASSAGE DU BUTIN", "inventory.loot_preferences");
            screen.addLine("Par défaut, tout le butin récupérable est ramassé automatiquement.");
            screen.addLine("Manuel uniquement demande confirmation pour chaque objet trouvé.");
            screen.addLine("Filtrage personnalisé ramasse tout sauf les catégories/objets que tu as explicitement exclus.");
            screen.addLine("Un filtre ne vend et ne détruit jamais le butin : l'objet est seulement laissé sur place.");
            screen.addLine("");
            screen.addLine("Mode actuel : " + lootPolicyLabel(player.getLootPickupPolicy()));
            screen.addOption(1, "Tout récupérer automatiquement", "Valeur par défaut : ramasser tout ce qui peut l'être.", true, "inventory.loot.all");
            screen.addOption(2, "Manuel uniquement", "Demander pour chaque matériau/équipement trouvé.", true, "inventory.loot.manual");
            screen.addOption(3, "Filtrage personnalisé", "Ramasser tout sauf les exclusions configurées ci-dessous.", true, "inventory.loot.filter");
            screen.addLine("");
            screen.addOption(4, "Matériaux : " + enabledLabel(player.isLootCategoryIgnored("material")), "Ignorer ou récupérer tous les matériaux générés comme butin.", true, "inventory.loot.category.material");
            screen.addOption(5, "Armes : " + enabledLabel(player.isLootCategoryIgnored("weapon")), "Ignorer ou récupérer les armes trouvées sur les ennemis.", true, "inventory.loot.category.weapon");
            screen.addOption(6, "Armures : " + enabledLabel(player.isLootCategoryIgnored("armor")), "Ignorer ou récupérer les armures trouvées sur les ennemis.", true, "inventory.loot.category.armor");
            screen.addOption(7, "Consommables : " + enabledLabel(player.isLootCategoryIgnored("consumable")), "Prépare le filtrage des consommables récupérés comme butin.", true, "inventory.loot.category.consumable");
            screen.addOption(8, "Objets/matériaux précis...", "Choisir des matériaux précis à laisser sur place lorsque le filtrage est actif.", true, "inventory.loot.materials");
            screen.addBackOption("Retour", "inventory.loot.back");

            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une préférence de ramassage.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 1) player.setLootPickupPolicy("all");
            else if (choice == 2) player.setLootPickupPolicy("manual");
            else if (choice == 3) player.setLootPickupPolicy("filter");
            else if (choice == 4) player.setLootCategoryIgnored("material", !player.isLootCategoryIgnored("material"));
            else if (choice == 5) player.setLootCategoryIgnored("weapon", !player.isLootCategoryIgnored("weapon"));
            else if (choice == 6) player.setLootCategoryIgnored("armor", !player.isLootCategoryIgnored("armor"));
            else if (choice == 7) player.setLootCategoryIgnored("consumable", !player.isLootCategoryIgnored("consumable"));
            else if (choice == 8) openIgnoredMaterialLootMenu(player);
        }
    }
    void openSelectionPreferences(Player& player)
    {
        while (true)
        {
            MenuScreen screen("PRÉFÉRENCES DE SÉLECTION", "inventory.preferences");
            screen.addLine("Par défaut, Dinotofu demande toujours ton choix avant de consommer une munition ou un support magique.");
            screen.addLine("Favori concerne uniquement les munitions : une munition favorite compatible peut être choisie automatiquement.");
            screen.addLine("Épinglé concerne uniquement l'ordre d'affichage dans l'inventaire et ne provoque jamais une consommation automatique.");
            screen.addLine("Le mode Automatique prudent privilégie les ressources communes/standard et évite les choix rares quand possible.");
            screen.addLine("");
            screen.addLine("Munitions : " + selectionPolicyLabel(player.getAmmunitionSelectionPolicy()));
            screen.addLine("Matériaux magiques : " + selectionPolicyLabel(player.getMagicMaterialSelectionPolicy()));
            screen.addOption(1, "Munitions — Toujours demander", "Comportement recommandé et valeur par défaut.", true, "inventory.preferences.ammo.ask");
            screen.addOption(2, "Munitions — Favori si disponible", "Utiliser automatiquement la munition favorite compatible ; sinon demander.", true, "inventory.preferences.ammo.favorite");
            screen.addOption(3, "Munitions — Automatique prudent", "Privilégier les munitions standard disponibles.", true, "inventory.preferences.ammo.auto");
            screen.addOption(4, "Matériaux — Toujours demander", "Comportement recommandé et valeur par défaut.", true, "inventory.preferences.material.ask");
            screen.addOption(5, "Matériaux — Automatique prudent", "Privilégier un support compatible de faible rareté/qualité.", true, "inventory.preferences.material.auto");
            screen.addOption(6, "Ramassage du butin...", "Mode par défaut : tout récupérer. Permet aussi manuel et filtrage.", true, "inventory.preferences.loot");
            screen.addBackOption("Retour", "inventory.preferences.back");

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide. Choisis une préférence affichée.");
            Console::clear();
            if (choice == 0) return;
            if (choice == 1) player.setAmmunitionSelectionPolicy("ask");
            else if (choice == 2) player.setAmmunitionSelectionPolicy("favorite");
            else if (choice == 3) player.setAmmunitionSelectionPolicy("auto");
            else if (choice == 4) player.setMagicMaterialSelectionPolicy("ask");
            else if (choice == 5) player.setMagicMaterialSelectionPolicy("auto");
            else if (choice == 6) openLootPreferences(player);
        }
    }
}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
bool InventoryMenu::open(Player& player)
{
    bool highLoadAdviceShown = false;
    while (true)
    {
        const CarryLoadSummary load = CarryLoadSystem::evaluate(player);
        player.refreshCarryAndStorageTitles();

        bool ownsAnyVault = player.hasCityVault();
        for (const PlayerCityVault& record : player.getCityVaultRecords())
        {
            if (record.purchased) { ownsAnyVault = true; break; }
        }

        if (load.percent >= 85 && !highLoadAdviceShown)
        {
            std::vector<std::string> notice;
            notice.push_back("Sac presque plein : " + std::to_string(load.used) + "/" + std::to_string(load.capacity) + " — " + std::to_string(load.percent) + " %.");
            if (!ownsAnyVault)
            {
                const long long neededCopper = Money::copperFromEconomyUnits(player.getCityVaultPurchaseCost());
                if (player.getInventory().getTotalCopper() >= neededCopper)
                    notice.push_back("Dinotofu : « Si tu comptes garder tout ça, un coffre municipal commencerait à être vraiment utile. »");
                else
                    notice.push_back("Dinotofu : « Le coffre coûte encore cher... Vends peut-être quelques doublons avant de vouloir tout garder. »");
            }
            else
            {
                notice.push_back("Pense à vendre ou déposer les réserves dont tu n'as pas besoin pour la prochaine sortie.");
            }
            MessageScreen::show("INVENTAIRE CHARGÉ", "inventory.carry.warning", notice);
            highLoadAdviceShown = true;
            Console::clear();
        }
        if (load.percent < 85) highLoadAdviceShown = false;

        MenuScreen screen = InventoryDisplay::buildMainScreen();
        screen.setTitle("INVENTAIRE [" + std::to_string(load.used) + "/" + std::to_string(load.capacity) + "]");
        screen.addSubtitle("Encombrement : " + std::to_string(load.used) + "/" + std::to_string(load.capacity) + " — " + std::to_string(load.percent) + " % — " + load.label);
        if (load.movementBlocked) screen.addLine("Charge >= 200 % : voyage et départ en exploration impossibles tant que le sac n'est pas allégé.");
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

        if (menuChoice == 11)
        {
            openSelectionPreferences(player);
            continue;
        }
    }
}
