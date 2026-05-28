// EN: EquipmentMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EquipmentMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/EquipmentMenu.hpp"

#include "core/Console.hpp"

#include "interface/menu/equipment/EquipmentDisplay.hpp"
#include "interface/menu/equipment/EquipmentComparison.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include <iostream>
#include <string>
#include <vector>


namespace
{
    std::string equipmentWeaponDurabilityText(const Weapon& weapon)
    {
        if (weapon.isIndestructible())
        {
            return "indestructible";
        }

        return std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability());
    }

    std::string equipmentArmorDurabilityText(const Armor& armor)
    {
        if (armor.isIndestructible())
        {
            return "indestructible";
        }

        return std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability());
    }

    void showSimpleEquipmentScreen(const Player& player)
    {
        std::vector<std::string> lines;

        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();
            lines.push_back("Arme équipée : " + weapon.getName() + " (" + equipmentWeaponDurabilityText(weapon) + ")" + (weapon.isBroken() ? " - Cassée" : ""));

            if (!weapon.isBroken() && !player.hasBossEquipmentSeal())
            {
                std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(player, weapon.getType(), weapon.getName());
                lines.push_back(affinityLabel.empty()
                    ? "Affinité arme/classe : aucune maîtrise particulière avec cette arme."
                    : "Affinité arme/classe : active, " + affinityLabel + ".");
            }
            else if (weapon.isBroken())
            {
                lines.push_back("Affinité arme/classe : inactive, l'arme est cassée.");
            }
            else
            {
                lines.push_back("Affinité arme/classe : bloquée par le sceau de boss.");
            }
        }
        else
        {
            lines.push_back("Arme équipée : Aucune");
        }

        if (player.hasEquippedArmor())
        {
            Armor armor = player.getEquippedArmor();
            lines.push_back("Armure équipée : " + armor.getName() + " (" + equipmentArmorDurabilityText(armor) + ")" + (armor.isBroken() ? " - Cassée" : ""));
        }
        else
        {
            lines.push_back("Armure équipée : Aucune");
        }

        lines.push_back("Or : " + std::to_string(player.getInventory().getGold()) + " pièces");
        MessageScreen::show("ÉQUIPEMENT SIMPLE", "equipment.simple.summary", lines);
    }

    void showDetailedEquipmentScreen(const Player& player)
    {
        std::vector<std::string> lines;

        lines.push_back("=== Arme équipée ===");
        if (player.hasEquippedWeapon())
        {
            Weapon weapon = player.getEquippedWeapon();
            lines.push_back("Nom : " + weapon.getName());
            lines.push_back("Description : " + weapon.getDescription());
            lines.push_back("Bonus dégâts : +" + std::to_string(weapon.getMinDamageBonus()) + " à +" + std::to_string(weapon.getMaxDamageBonus()));
            lines.push_back("Bonus critique : +" + std::to_string(weapon.getCriticalBonus()));
            lines.push_back("Durabilité : " + equipmentWeaponDurabilityText(weapon));

            if (weapon.isBroken())
            {
                lines.push_back("État : cassée, ses bonus ne s'appliquent plus.");
                lines.push_back("Affinité arme/classe : inactive, l'arme est cassée.");
            }
            else if (player.hasBossEquipmentSeal())
            {
                lines.push_back("État : utilisable, mais le sceau de boss bloque ses bonus.");
                lines.push_back("Affinité arme/classe : bloquée par le sceau de boss.");
            }
            else
            {
                lines.push_back("État : Utilisable");
                std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(player, weapon.getType(), weapon.getName());
                lines.push_back(affinityLabel.empty()
                    ? "Affinité arme/classe : aucune maîtrise particulière avec cette arme."
                    : "Affinité arme/classe : active, " + affinityLabel + ".");
            }
        }
        else
        {
            lines.push_back("Aucune arme équipée.");
        }

        lines.push_back("");
        lines.push_back("=== Armure équipée ===");
        if (player.hasEquippedArmor())
        {
            Armor armor = player.getEquippedArmor();
            lines.push_back("Nom : " + armor.getName());
            lines.push_back("Description : " + armor.getDescription());
            lines.push_back("Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()));
            lines.push_back("Réduction dégâts : " + std::to_string(armor.getDamageReduction()));
            lines.push_back("Durabilité : " + equipmentArmorDurabilityText(armor));
            lines.push_back(armor.isBroken() ? "État : cassée, ses bonus ne s'appliquent plus." : "État : Utilisable");
        }
        else
        {
            lines.push_back("Aucune armure équipée.");
        }

        lines.push_back("");
        lines.push_back("Or : " + std::to_string(player.getInventory().getGold()) + " pièces");
        MessageScreen::show("ÉQUIPEMENT DÉTAILLÉ", "equipment.details.summary", lines);
    }

    void showEquipmentWeaponInspection(const Weapon& weapon)
    {
        MessageScreen::show(
            "INSPECTION - ARME",
            "equipment.weapon.inspect.details",
            {
                "Nom : " + weapon.getName(),
                "Description : " + weapon.getDescription(),
                "Bonus dégâts : +" + std::to_string(weapon.getMinDamageBonus()) + " à +" + std::to_string(weapon.getMaxDamageBonus()),
                "Bonus critique : +" + std::to_string(weapon.getCriticalBonus()),
                "Durabilité : " + equipmentWeaponDurabilityText(weapon),
                weapon.isBroken() ? "État : Cassée" : "État : Utilisable",
                "Valeur : " + std::to_string(weapon.getValue()) + " pièces"
            }
        );
    }

    void showEquipmentArmorInspection(const Armor& armor)
    {
        MessageScreen::show(
            "INSPECTION - ARMURE",
            "equipment.armor.inspect.details",
            {
                "Nom : " + armor.getName(),
                "Description : " + armor.getDescription(),
                "Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()),
                "Réduction dégâts : " + std::to_string(armor.getDamageReduction()),
                "Durabilité : " + equipmentArmorDurabilityText(armor),
                armor.isBroken() ? "État : Cassée" : "État : Utilisable",
                "Valeur : " + std::to_string(armor.getValue()) + " pièces"
            }
        );
    }

    void showEquipmentWeaponEquipResult(const Player& player, const Weapon& weapon)
    {
        MessageScreen::show(
            "ARME ÉQUIPÉE",
            "equipment.weapon.equip.result",
            {
                player.getName() + " équipe : " + weapon.getName() + ".",
                "Durabilité : " + equipmentWeaponDurabilityText(weapon),
                weapon.isBroken() ? "Attention : cette arme est cassée, elle ne donnera aucun bonus." : "La prise en main est bonne. Cette arme est prête au combat."
            }
        );
    }

    void showEquipmentArmorEquipResult(const Player& player, const Armor& armor)
    {
        MessageScreen::show(
            "ARMURE ÉQUIPÉE",
            "equipment.armor.equip.result",
            {
                player.getName() + " équipe : " + armor.getName() + ".",
                "Durabilité : " + equipmentArmorDurabilityText(armor),
                armor.isBroken() ? "Attention : cette armure est cassée, elle ne donnera aucun bonus." : "Ses protections sont maintenant actives.",
                player.getName() + " possède maintenant " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " PV."
            }
        );
    }

    void showEquipmentNotice(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }
}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::open(Player& player)
{
    while (true)
    {
        MenuScreen screen = EquipmentDisplay::buildMainScreen();
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option visible de l'équipement."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 1)
        {
            showSimpleEquipmentScreen(player);
            continue;
        }

        if (choice == 2)
        {
            showDetailedEquipmentScreen(player);
            continue;
        }

        if (choice == 3)
        {
            equipWeaponFromInventory(player);
            continue;
        }

        if (choice == 4)
        {
            equipArmorFromInventory(player);
            continue;
        }
    }
}

// EN: equipWeaponFromInventory declares or implements a focused behavior used by this module.
// FR: equipWeaponFromInventory déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::equipWeaponFromInventory(Player& player)
{
    if (player.getInventory().getWeaponCount() <= 0)
    {
        showEquipmentNotice("CHANGER ARME", "equipment.weapon.empty", {player.getName() + " n'a aucune arme à équiper.", "Les armes récupérées apparaîtront ici."});
        return false;
    }

    MenuScreen weaponListScreen = EquipmentDisplay::buildWeaponListScreen(player);
    int choice = TerminalInterface::askMenuChoiceFromOptions(
        weaponListScreen,
        "Choix invalide. Entre un numéro d'arme visible, ou -1 pour revenir."
    );

    Console::clear();

    if (choice == -1)
    {
        return false;
    }

    if (!player.getInventory().hasWeapon(choice))
    {
        showEquipmentNotice("ARME INTROUVABLE", "equipment.weapon.missing", {"Cette arme n'existe pas dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
        return false;
    }

    Weapon newWeapon = player.getInventory().getWeapon(choice);

    MenuScreen selectedWeaponScreen = EquipmentDisplay::buildSelectedWeaponScreen(newWeapon);
    int action = TerminalInterface::askMenuChoiceFromOptions(
        selectedWeaponScreen,
        "Choix invalide. Choisis une action visible pour cette arme."
    );

    Console::clear();

    if (action == 0)
    {
        return false;
    }

    if (action == 1)
    {
        showEquipmentWeaponInspection(newWeapon);
        return false;
    }

    if (action == 2)
    {
        EquipmentComparison::displayWeaponComparison(player, newWeapon);
        return false;
    }

    if (!player.equipWeapon(choice))
    {
        showEquipmentNotice("ARME NON ÉQUIPÉE", "equipment.weapon.equip.failed", {"Impossible d'équiper cette arme.", "Elle a peut-être été déplacée, retirée ou rendue indisponible."});
        return false;
    }

    showEquipmentWeaponEquipResult(player, player.getEquippedWeapon());
    return false;
}

// EN: equipArmorFromInventory declares or implements a focused behavior used by this module.
// FR: equipArmorFromInventory déclare ou implémente un comportement précis utilisé par ce module.
bool EquipmentMenu::equipArmorFromInventory(Player& player)
{
    if (player.getInventory().getArmorCount() <= 0)
    {
        showEquipmentNotice("CHANGER TENUE", "equipment.armor.empty", {player.getName() + " n'a aucune armure à équiper.", "Les protections récupérées apparaîtront ici."});
        return false;
    }

    MenuScreen armorListScreen = EquipmentDisplay::buildArmorListScreen(player);
    int choice = TerminalInterface::askMenuChoiceFromOptions(
        armorListScreen,
        "Choix invalide. Entre un numéro d'armure visible, ou -1 pour revenir."
    );

    Console::clear();

    if (choice == -1)
    {
        return false;
    }

    if (!player.getInventory().hasArmor(choice))
    {
        showEquipmentNotice("ARMURE INTROUVABLE", "equipment.armor.missing", {"Cette armure n'existe pas dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
        return false;
    }

    Armor newArmor = player.getInventory().getArmor(choice);

    MenuScreen selectedArmorScreen = EquipmentDisplay::buildSelectedArmorScreen(newArmor);
    int action = TerminalInterface::askMenuChoiceFromOptions(
        selectedArmorScreen,
        "Choix invalide. Choisis une action visible pour cette armure."
    );

    Console::clear();

    if (action == 0)
    {
        return false;
    }

    if (action == 1)
    {
        showEquipmentArmorInspection(newArmor);
        return false;
    }

    if (action == 2)
    {
        EquipmentComparison::displayArmorComparison(player, newArmor);
        return false;
    }

    if (!player.equipArmor(choice))
    {
        showEquipmentNotice("ARMURE NON ÉQUIPÉE", "equipment.armor.equip.failed", {"Impossible d'équiper cette armure.", "Elle a peut-être été déplacée, retirée ou rendue indisponible."});
        return false;
    }

    showEquipmentArmorEquipResult(player, player.getEquippedArmor());
    return false;
}
