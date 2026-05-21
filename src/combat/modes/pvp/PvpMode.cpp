// EN: PvpMode.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PvpMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/PvpMode.hpp"

#include "combat/TurnManager.hpp"

#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "interface/CombatDisplay.hpp"
#include "progression/DifficultyRules.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    struct PvpPlayerSlot
    {
        Player player;
        std::string accountName;
        DifficultyMode difficulty;
        bool persistent = false;
        bool ephemeral = true;
    };

    struct FriendlyArenaSnapshot
    {
        int hp;
        int maxHp;
        int minDamage;
        int maxDamage;
        int criticalDamage;
        int equippedWeaponIndex;
        int equippedArmorIndex;
        int gold;
        std::vector<Weapon> weapons;
        std::vector<Armor> armors;
        std::vector<Consumable> consumables;
        std::vector<Material> materials;
    };

    // EN: createFriendlyArenaSnapshot declares or implements a focused behavior used by this module.
    // FR: createFriendlyArenaSnapshot déclare ou implémente un comportement précis utilisé par ce module.
    FriendlyArenaSnapshot createFriendlyArenaSnapshot(const Player& player)
    {
        FriendlyArenaSnapshot snapshot;
        snapshot.hp = player.getHp();
        snapshot.maxHp = player.getMaxHp();
        snapshot.minDamage = player.getMinDamage();
        snapshot.maxDamage = player.getMaxDamage();
        snapshot.criticalDamage = player.getCriticalDamage();
        snapshot.equippedWeaponIndex = player.getEquippedWeaponIndex();
        snapshot.equippedArmorIndex = player.getEquippedArmorIndex();
        snapshot.gold = player.getInventory().getGold();
        snapshot.weapons = player.getInventory().getWeapons();
        snapshot.armors = player.getInventory().getArmors();
        snapshot.consumables = player.getInventory().getConsumables();
        snapshot.materials = player.getInventory().getMaterials();
        return snapshot;
    }

    // EN: restoreFriendlyArenaSnapshot declares or implements a focused behavior used by this module.
    // FR: restoreFriendlyArenaSnapshot déclare ou implémente un comportement précis utilisé par ce module.
    void restoreFriendlyArenaSnapshot(Player& player, const FriendlyArenaSnapshot& snapshot)
    {
        player.unequipArmor();
        player.unequipWeapon();
        player.getInventory().clearAll();
        player.getInventory().setGold(snapshot.gold);

        for (const Weapon& weapon : snapshot.weapons)
        {
            player.getInventory().addWeapon(weapon);
        }

        for (const Armor& armor : snapshot.armors)
        {
            player.getInventory().addArmor(armor);
        }

        for (const Consumable& consumable : snapshot.consumables)
        {
            player.getInventory().addConsumable(consumable);
        }

        for (const Material& material : snapshot.materials)
        {
            player.getInventory().addMaterial(material);
        }

        if (snapshot.equippedWeaponIndex >= 0)
        {
            player.equipWeapon(snapshot.equippedWeaponIndex);
        }

        if (snapshot.equippedArmorIndex >= 0)
        {
            player.equipArmor(snapshot.equippedArmorIndex);
        }

        player.setLoadedCombatStats(
            snapshot.maxHp,
            snapshot.minDamage,
            snapshot.maxDamage,
            snapshot.criticalDamage,
            snapshot.hp
        );
    }

    // EN: applyFriendlyDuelSymbolicReward declares or implements a focused behavior used by this module.
    // FR: applyFriendlyDuelSymbolicReward déclare ou implémente un comportement précis utilisé par ce module.
    void applyFriendlyDuelSymbolicReward(Player& winner)
    {
        int symbolicGold = 3;
        int symbolicExperience = 8;

        winner.getInventory().earnGold(symbolicGold);
        winner.gainExperience(symbolicExperience);

        std::cout << "Gain symbolique de l'arène pour " << winner.getName() << " : "
                  << symbolicGold << " or et " << symbolicExperience << " expérience." << std::endl;
    }

    // EN: askDifficultyForSecondPlayer declares or implements a focused behavior used by this module.
    // FR: askDifficultyForSecondPlayer déclare ou implémente un comportement précis utilisé par ce module.
    DifficultyMode askDifficultyForSecondPlayer()
    {
        std::cout << "Difficulté du personnage J2 ?" << std::endl;
        std::cout << "1 : Facile" << std::endl;
        std::cout << "2 : Normal" << std::endl;
        std::cout << "3 : Difficile" << std::endl;
        std::cout << "4 : Cauchemar" << std::endl;
        std::cout << "5 : Léthal" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(1, 5, "Veuillez choisir une difficulté valide.");

        if (choice == 1) return DifficultyMode::Easy;
        if (choice == 3) return DifficultyMode::Hard;
        if (choice == 4) return DifficultyMode::Nightmare;
        if (choice == 5) return DifficultyMode::Lethal;
        return DifficultyMode::Normal;
    }

    // EN: createManualPlayer declares or implements a focused behavior used by this module.
    // FR: createManualPlayer déclare ou implémente un comportement précis utilisé par ce module.
    Player createManualPlayer(const std::string& playerLabel, DifficultyMode difficulty)
    {
        std::cout << playerLabel << ", quel est ton nom ?" << std::endl;
        std::cout << "> ";

        std::string playerName;
        std::getline(std::cin >> std::ws, playerName);

        while (playerName.empty())
        {
            std::cout << "Entre un vrai nom." << std::endl;
            std::cout << "> ";
            std::getline(std::cin >> std::ws, playerName);
        }

        Console::clear();

        std::cout << "Très bien, " << playerName << "." << std::endl;
        std::cout << "Choisis d'abord ta famille de classe." << std::endl;
        std::cout << std::endl;

        ClassCatalog::displayClassCategories();

        std::cout << std::endl;
        std::cout << "> ";

        int categoryChoice = Console::askNumberBetween(
            1,
            ClassCatalog::getClassCategoryCount(),
            "Veuillez entrer un chiffre correspondant à une famille affichée."
        );

        Console::clear();

        std::cout << "Famille sélectionnée : "
                  << ClassCatalog::getClassCategoryNameByChoice(categoryChoice)
                  << "."
                  << std::endl;
        std::cout << "Choisis maintenant ta classe." << std::endl;
        std::cout << std::endl;

        ClassCatalog::displayClassesByCategoryChoice(categoryChoice);

        int maxClassChoice = ClassCatalog::getPlayableClassCountByCategoryChoice(categoryChoice);

        std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
        std::cout << "> ";

        int classChoice = Console::askNumberBetween(
            1,
            maxClassChoice,
            "Veuillez entrer un chiffre correspondant à une classe affichée."
        );

        Player player(playerName, ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice));
        player.initializeStarterInventory(difficulty);
        return player;
    }

    // EN: askSecondPlayer declares or implements a focused behavior used by this module.
    // FR: askSecondPlayer déclare ou implémente un comportement précis utilisé par ce module.
    PvpPlayerSlot askSecondPlayer(Random& random)
    {
        (void)random;

        while (true)
        {
            std::cout << "========== JOUEUR 2 ==========" << std::endl;
            std::cout << "1 : Charger / créer un compte local" << std::endl;
            std::cout << "2 : Utiliser un personnage éphémère" << std::endl;
            std::cout << "==============================" << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(1, 2, "Veuillez choisir 1 ou 2.");
            Console::clear();

            if (choice == 2)
            {
                PvpPlayerSlot slot;
                slot.difficulty = DifficultyMode::Normal;
                slot.player = createManualPlayer("Joueur 2 éphémère", slot.difficulty);
                slot.persistent = false;
                slot.ephemeral = true;
                return slot;
            }

            PvpPlayerSlot slot;
            slot.accountName = AccountMenu::open();

            CharacterMenuResult result = CharacterMenu::open(slot.accountName, slot.player);

            if (result.characterLoaded)
            {
                slot.difficulty = result.difficulty;
                slot.persistent = true;
                slot.ephemeral = false;
                return slot;
            }

            slot.difficulty = askDifficultyForSecondPlayer();
            Console::clear();
            slot.player = createManualPlayer("Joueur 2", slot.difficulty);
            slot.persistent = true;
            slot.ephemeral = false;

            SaveManager::saveAccountSnapshot(slot.accountName);
            SaveManager::savePlayerSnapshot(slot.player, slot.accountName, slot.difficulty);
            return slot;
        }
    }

    std::string normalizePvpName(const std::string& value)
    {
        std::string normalized;

        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                normalized += static_cast<char>(std::tolower(character));
            }
        }

        return normalized;
    }

    // EN: isSameCharacterName declares or implements a focused behavior used by this module.
    // FR: isSameCharacterName déclare ou implémente un comportement précis utilisé par ce module.
    bool isSameCharacterName(const Player& first, const Player& second)
    {
        return normalizePvpName(first.getName()) == normalizePvpName(second.getName());
    }

    // EN: isLethal declares or implements a focused behavior used by this module.
    // FR: isLethal déclare ou implémente un comportement précis utilisé par ce module.
    bool isLethal(DifficultyMode difficulty)
    {
        return difficulty == DifficultyMode::Lethal;
    }

    // EN: canStartDeadlyDuel declares or implements a focused behavior used by this module.
    // FR: canStartDeadlyDuel déclare ou implémente un comportement précis utilisé par ce module.
    bool canStartDeadlyDuel(DifficultyMode first, DifficultyMode second)
    {
        return isLethal(first) == isLethal(second);
    }


    std::string pvpDifficultyName(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Facile";
            case DifficultyMode::Hard:
                return "Difficile";
            case DifficultyMode::Nightmare:
                return "Cauchemar";
            case DifficultyMode::Lethal:
                return "Léthal";
            case DifficultyMode::Normal:
            default:
                return "Normal";
        }
    }

    std::string pvpAccountStatus(const PvpPlayerSlot& slot)
    {
        if (!slot.persistent)
        {
            return "personnage éphémère";
        }

        if (slot.accountName.empty())
        {
            return "compte local inconnu";
        }

        return "compte local : " + slot.accountName;
    }

    void displayPvpSecuritySummary(
        const Player& player1,
        DifficultyMode difficulty1,
        const std::string& account1,
        const PvpPlayerSlot& player2Slot
    )
    {
        std::cout << "========== CONTRÔLE DE L'ARÈNE ==========" << std::endl;
        std::cout << player1.getName() << " | compte local : " << account1
                  << " | difficulté : " << pvpDifficultyName(difficulty1);
        if (player1.isClone())
        {
            std::cout << " | CLONE";
        }
        std::cout << std::endl;

        std::cout << player2Slot.player.getName() << " | " << pvpAccountStatus(player2Slot)
                  << " | difficulté : " << pvpDifficultyName(player2Slot.difficulty);
        if (player2Slot.player.isClone())
        {
            std::cout << " | CLONE";
        }
        std::cout << std::endl;
        std::cout << std::endl;

        if (player1.isClone() || player2Slot.player.isClone())
        {
            std::cout << "Verdict : combat amical uniquement." << std::endl;
            std::cout << "Un clone peut se battre pour le fun, mais pas créer de vrai butin." << std::endl;
        }
        else if (!player2Slot.persistent || account1 == player2Slot.accountName)
        {
            std::cout << "Verdict : combat amical uniquement." << std::endl;
            std::cout << "Le duel mortel exige deux comptes locaux différents et persistants." << std::endl;
        }
        else if (!canStartDeadlyDuel(difficulty1, player2Slot.difficulty))
        {
            std::cout << "Verdict : combat mortel visible, mais verrouillé." << std::endl;
            std::cout << "Une existence Léthal ne peut pas être mélangée avec une simulation non Léthal." << std::endl;
        }
        else if (isLethal(difficulty1))
        {
            std::cout << "Verdict : duel mortel Léthal autorisé." << std::endl;
            std::cout << "Le perdant quittera le registre des vivants." << std::endl;
        }
        else
        {
            std::cout << "Verdict : duel mortel non Léthal autorisé." << std::endl;
            std::cout << "Le butin restera limité par la valeur du gagnant pour éviter les abus." << std::endl;
        }

        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: isProtectedStarterWeapon declares or implements a focused behavior used by this module.
    // FR: isProtectedStarterWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool isProtectedStarterWeapon(const Weapon& weapon)
    {
        return weapon.getName() == "Mains nues";
    }

    // EN: isProtectedStarterArmor declares or implements a focused behavior used by this module.
    // FR: isProtectedStarterArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool isProtectedStarterArmor(const Armor& armor)
    {
        return armor.getName() == "Tenue simple";
    }

    // EN: stealOneWeapon declares or implements a focused behavior used by this module.
    // FR: stealOneWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneWeapon(Player& winner, Player& loser, Random& random, int chance)
    {
        if (random.between(1, 100) > chance)
        {
            return false;
        }

        for (int i = 0; i < loser.getInventory().getWeaponCount(); ++i)
        {
            if (i == loser.getEquippedWeaponIndex())
            {
                continue;
            }

            Weapon weapon = loser.getInventory().getWeapon(i);

            if (isProtectedStarterWeapon(weapon))
            {
                continue;
            }

            winner.getInventory().addWeapon(weapon);
            loser.getInventory().removeWeapon(i);
            std::cout << winner.getName() << " récupère l'arme : " << weapon.getName() << "." << std::endl;
            return true;
        }

        return false;
    }

    // EN: stealOneArmor declares or implements a focused behavior used by this module.
    // FR: stealOneArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneArmor(Player& winner, Player& loser, Random& random, int chance)
    {
        if (random.between(1, 100) > chance)
        {
            return false;
        }

        for (int i = 0; i < loser.getInventory().getArmorCount(); ++i)
        {
            if (i == loser.getEquippedArmorIndex())
            {
                continue;
            }

            Armor armor = loser.getInventory().getArmor(i);

            if (isProtectedStarterArmor(armor))
            {
                continue;
            }

            winner.getInventory().addArmor(armor);
            loser.getInventory().removeArmor(i);
            std::cout << winner.getName() << " récupère l'armure : " << armor.getName() << "." << std::endl;
            return true;
        }

        return false;
    }



    // EN: stealEquippedWeaponOnLethalDeath declares or implements a focused behavior used by this module.
    // FR: stealEquippedWeaponOnLethalDeath déclare ou implémente un comportement précis utilisé par ce module.
    bool stealEquippedWeaponOnLethalDeath(Player& winner, Player& loser, Random& random)
    {
        int index = loser.getEquippedWeaponIndex();

        if (index < 0 || !loser.getInventory().hasWeapon(index))
        {
            return false;
        }

        Weapon weapon = loser.getInventory().getWeapon(index);

        if (isProtectedStarterWeapon(weapon))
        {
            return false;
        }

        int chance = 75;

        if (weapon.getName().find("Relique") != std::string::npos
            || weapon.getName().find("relique") != std::string::npos
            || weapon.getName().find("Unique") != std::string::npos
            || weapon.getName().find("unique") != std::string::npos)
        {
            chance = 85;
        }

        if (random.between(1, 100) > chance)
        {
            std::cout << "L'arme équipée de " << loser.getName()
                      << " échappe au pillage dans le chaos du duel." << std::endl;
            return false;
        }

        loser.unequipWeapon();
        winner.getInventory().addWeapon(weapon);
        loser.getInventory().removeWeapon(index);

        std::cout << winner.getName() << " récupère l'arme portée par le défunt : "
                  << weapon.getName() << "." << std::endl;
        return true;
    }

    // EN: stealEquippedArmorOnLethalDeath declares or implements a focused behavior used by this module.
    // FR: stealEquippedArmorOnLethalDeath déclare ou implémente un comportement précis utilisé par ce module.
    bool stealEquippedArmorOnLethalDeath(Player& winner, Player& loser, Random& random)
    {
        int index = loser.getEquippedArmorIndex();

        if (index < 0 || !loser.getInventory().hasArmor(index))
        {
            return false;
        }

        Armor armor = loser.getInventory().getArmor(index);

        if (isProtectedStarterArmor(armor))
        {
            return false;
        }

        int chance = 65;

        if (armor.getName().find("Relique") != std::string::npos
            || armor.getName().find("relique") != std::string::npos
            || armor.getName().find("Unique") != std::string::npos
            || armor.getName().find("unique") != std::string::npos)
        {
            chance = 80;
        }

        if (random.between(1, 100) > chance)
        {
            std::cout << "L'armure équipée de " << loser.getName()
                      << " est trop endommagée pour être récupérée proprement." << std::endl;
            return false;
        }

        loser.unequipArmor();
        winner.getInventory().addArmor(armor);
        loser.getInventory().removeArmor(index);

        std::cout << winner.getName() << " récupère l'armure portée par le défunt : "
                  << armor.getName() << "." << std::endl;
        return true;
    }

    // EN: nameContainsAny declares or implements a focused behavior used by this module.
    // FR: nameContainsAny déclare ou implémente un comportement précis utilisé par ce module.
    bool nameContainsAny(const std::string& value, const std::vector<std::string>& words)
    {
        std::string normalized = normalizePvpName(value);

        for (const std::string& word : words)
        {
            if (normalized.find(normalizePvpName(word)) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    // EN: rarityValueBonus declares or implements a focused behavior used by this module.
    // FR: rarityValueBonus déclare ou implémente un comportement précis utilisé par ce module.
    int rarityValueBonus(const std::string& name)
    {
        if (nameContainsAny(name, {"Relique", "Unique", "God", "Divin"})) return 5;
        if (nameContainsAny(name, {"Héroïque", "Heroique", "Légendaire", "Legendaire"})) return 3;
        if (nameContainsAny(name, {"Rare", "Mystique"})) return 2;
        return 1;
    }

    // EN: estimateWeaponValue declares or implements a focused behavior used by this module.
    // FR: estimateWeaponValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateWeaponValue(const Weapon& weapon)
    {
        int value = weapon.getValue();
        value += weapon.getMinDamageBonus() * 8;
        value += weapon.getMaxDamageBonus() * 10;
        value += weapon.getCriticalBonus() * 6;
        value = value * rarityValueBonus(weapon.getName());

        if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0)
        {
            value = value * std::max(1, weapon.getDurability()) / weapon.getMaxDurability();
        }

        return std::max(1, value);
    }

    // EN: estimateArmorValue declares or implements a focused behavior used by this module.
    // FR: estimateArmorValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateArmorValue(const Armor& armor)
    {
        int value = armor.getValue();
        value += armor.getMaxHpBonus() * 8;
        value = value * rarityValueBonus(armor.getName());

        if (!armor.isIndestructible() && armor.getMaxDurability() > 0)
        {
            value = value * std::max(1, armor.getDurability()) / armor.getMaxDurability();
        }

        return std::max(1, value);
    }

    // EN: estimateMaterialStackValue declares or implements a focused behavior used by this module.
    // FR: estimateMaterialStackValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateMaterialStackValue(const Material& material)
    {
        return std::max(1, material.getValue() * material.getQualityPricePercent() / 100) * material.getQuantity();
    }

    // EN: estimateInventoryAndEquipmentValue declares or implements a focused behavior used by this module.
    // FR: estimateInventoryAndEquipmentValue déclare ou implémente un comportement précis utilisé par ce module.
    int estimateInventoryAndEquipmentValue(const Player& player)
    {
        int total = player.getInventory().getGold();

        for (const Weapon& weapon : player.getInventory().getWeapons())
        {
            total += estimateWeaponValue(weapon);
        }

        for (const Armor& armor : player.getInventory().getArmors())
        {
            total += estimateArmorValue(armor);
        }

        for (const Consumable& consumable : player.getInventory().getConsumables())
        {
            total += consumable.getValue();
        }

        for (const Material& material : player.getInventory().getMaterials())
        {
            total += estimateMaterialStackValue(material);
        }

        return total;
    }

    // EN: displayDuelValueEstimation declares or implements a focused behavior used by this module.
    // FR: displayDuelValueEstimation déclare ou implémente un comportement précis utilisé par ce module.
    void displayDuelValueEstimation(const Player& player1, const Player& player2)
    {
        std::cout << "========== ESTIMATION AVANT DUEL ==========" << std::endl;
        std::cout << "Estimation de l'inventaire et de l'équipement de "
                  << player1.getName() << " : "
                  << estimateInventoryAndEquipmentValue(player1) << " pièces." << std::endl;
        std::cout << "Estimation de l'inventaire et de l'équipement de l'opposant "
                  << player2.getName() << " : "
                  << estimateInventoryAndEquipmentValue(player2) << " pièces." << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: askProceedAfterDuelValueEstimation declares or implements a focused behavior used by this module.
    // FR: askProceedAfterDuelValueEstimation déclare ou implémente un comportement précis utilisé par ce module.
    bool askProceedAfterDuelValueEstimation(const Player& player1, const Player& player2)
    {
        displayDuelValueEstimation(player1, player2);
        std::cout << "Procéder au combat ?" << std::endl;
        std::cout << "1 : Oui" << std::endl;
        std::cout << "2 : Non" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(1, 2, "Veuillez choisir 1 ou 2.");
        Console::clear();
        return choice == 1;
    }

    // EN: stealOneWeaponWithValueCap declares or implements a focused behavior used by this module.
    // FR: stealOneWeaponWithValueCap déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneWeaponWithValueCap(Player& winner, Player& loser, Random& random, int chance, int& stolenValue, int valueCap)
    {
        if (random.between(1, 100) > chance)
        {
            return false;
        }

        for (int i = 0; i < loser.getInventory().getWeaponCount(); ++i)
        {
            if (i == loser.getEquippedWeaponIndex())
            {
                continue;
            }

            Weapon weapon = loser.getInventory().getWeapon(i);

            if (isProtectedStarterWeapon(weapon))
            {
                continue;
            }

            int value = estimateWeaponValue(weapon);
            if (stolenValue + value > valueCap)
            {
                continue;
            }

            winner.getInventory().addWeapon(weapon);
            loser.getInventory().removeWeapon(i);
            stolenValue += value;
            std::cout << winner.getName() << " récupère l'arme : " << weapon.getName()
                      << " (valeur estimée " << value << ")." << std::endl;
            return true;
        }

        return false;
    }

    // EN: stealOneArmorWithValueCap declares or implements a focused behavior used by this module.
    // FR: stealOneArmorWithValueCap déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneArmorWithValueCap(Player& winner, Player& loser, Random& random, int chance, int& stolenValue, int valueCap)
    {
        if (random.between(1, 100) > chance)
        {
            return false;
        }

        for (int i = 0; i < loser.getInventory().getArmorCount(); ++i)
        {
            if (i == loser.getEquippedArmorIndex())
            {
                continue;
            }

            Armor armor = loser.getInventory().getArmor(i);

            if (isProtectedStarterArmor(armor))
            {
                continue;
            }

            int value = estimateArmorValue(armor);
            if (stolenValue + value > valueCap)
            {
                continue;
            }

            winner.getInventory().addArmor(armor);
            loser.getInventory().removeArmor(i);
            stolenValue += value;
            std::cout << winner.getName() << " récupère l'armure : " << armor.getName()
                      << " (valeur estimée " << value << ")." << std::endl;
            return true;
        }

        return false;
    }

    // EN: applyDeadlyDuelReward declares or implements a focused behavior used by this module.
    // FR: applyDeadlyDuelReward déclare ou implémente un comportement précis utilisé par ce module.
    void applyDeadlyDuelReward(Player& winner, Player& loser, Random& random, bool lethalDuel, int winnerPreFightValue)
    {
        std::cout << std::endl;
        std::cout << "========== BUTIN DU DUEL MORTEL ==========" << std::endl;

        int valueCap = lethalDuel ? 1000000000 : std::max(1, winnerPreFightValue);
        int stolenValue = 0;
        int goldPercent = lethalDuel ? 45 : 18;
        int stolenGold = loser.getInventory().getGold() * goldPercent / 100;

        if (!lethalDuel)
        {
            stolenGold = std::min(stolenGold, valueCap);
        }

        if (stolenGold > 0 && loser.getInventory().spendGold(stolenGold))
        {
            winner.getInventory().earnGold(stolenGold);
            stolenValue += stolenGold;
            std::cout << "Or récupéré : " << stolenGold << "." << std::endl;
        }
        else
        {
            std::cout << "Aucun or récupérable." << std::endl;
        }

        if (!lethalDuel)
        {
            std::cout << "Limite anti-abus : le perdant ne pourra pas perdre plus que la valeur estimée de son adversaire avant duel ("
                      << valueCap << " pièces)." << std::endl;
        }

        int stealChance = lethalDuel ? 78 : 35;
        int attempts = lethalDuel ? 5 : 1;

        bool itemStolen = false;

        if (lethalDuel)
        {
            itemStolen = stealEquippedWeaponOnLethalDeath(winner, loser, random) || itemStolen;
            itemStolen = stealEquippedArmorOnLethalDeath(winner, loser, random) || itemStolen;
        }

        for (int i = 0; i < attempts; ++i)
        {
            if (lethalDuel)
            {
                itemStolen = stealOneWeapon(winner, loser, random, stealChance) || itemStolen;
                itemStolen = stealOneArmor(winner, loser, random, stealChance) || itemStolen;
            }
            else
            {
                itemStolen = stealOneWeaponWithValueCap(winner, loser, random, stealChance, stolenValue, valueCap) || itemStolen;
                itemStolen = stealOneArmorWithValueCap(winner, loser, random, stealChance, stolenValue, valueCap) || itemStolen;
            }
        }

        if (!itemStolen)
        {
            std::cout << "Aucun équipement non lié n'a été récupéré." << std::endl;
        }

        int xpReward = lethalDuel ? 260 : 70;
        winner.gainExperience(xpReward);
        std::cout << "Expérience gagnée : " << xpReward << "." << std::endl;

        if (lethalDuel)
        {
            std::cout << "Duel Léthal : le perdant est considéré comme mort définitivement." << std::endl;
            std::cout << "Les objets liés à l'âme ou au corps restent protégés." << std::endl;
            std::cout << "Les reliques et objets rares ont une forte chance d'être récupérés, mais pas une garantie absolue." << std::endl;
        }

        std::cout << "==========================================" << std::endl;
        std::cout << std::endl;
    }

    bool askDeadlyDuelIfAllowed(
        const Player& player1,
        const std::string& account1,
        const PvpPlayerSlot& player2Slot,
        DifficultyMode difficulty1
    )
    {
        if (player1.isClone() || player2Slot.player.isClone())
        {
            std::cout << "Combat JcJ amical imposé : un clone ne peut pas participer à un combat à mort." << std::endl;
            std::cout << "Un clone sert à jouer, tester et transporter une copie, pas à générer du vrai butin." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        if (!player2Slot.persistent || account1 == player2Slot.accountName)
        {
            std::cout << "Combat JcJ amical : gain symbolique seulement." << std::endl;
            std::cout << "Le combat mortel n'est proposé que pour deux comptes différents." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        std::cout << "========== TYPE DE DUEL ==========" << std::endl;
        std::cout << "1 : Combat amical" << std::endl;
        std::cout << "2 : Combat à mort" << std::endl;
        std::cout << "==================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(1, 2, "Veuillez choisir 1 ou 2.");
        Console::clear();

        if (choice == 1)
        {
            std::cout << "Combat amical : gain symbolique, consommables et usure d'équipement remboursés par l'arène." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        if (!canStartDeadlyDuel(difficulty1, player2Slot.difficulty))
        {
            std::cout << "Combat à mort refusé." << std::endl;
            std::cout << "Un personnage Léthal ne peut pas engager un duel mortel contre une simulation non Léthal." << std::endl;
            std::cout << "Il faut deux personnages Léthal, ou deux personnages non Léthal." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        std::cout << "Combat à mort confirmé." << std::endl;
        if (isLethal(difficulty1))
        {
            std::cout << "Deux existences Léthal s'affrontent : le perdant sera inscrit comme mort." << std::endl;
            std::cout << "Ce n'est pas une grosse pénalité : c'est une mort définitive, sauf bénédiction future." << std::endl;
        }
        else
        {
            std::cout << "Deux personnages non Léthal s'affrontent : butin limité, pas d'effacement définitif." << std::endl;
        }
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();

        if (!askProceedAfterDuelValueEstimation(player1, player2Slot.player))
        {
            std::cout << "Duel mortel annulé après estimation." << std::endl;
            std::cout << std::endl;
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        return true;
    }
}

// EN: run declares or implements a focused behavior used by this module.
// FR: run déclare ou implémente un comportement précis utilisé par ce module.
void PvpMode::run(Player& player1, Random& random, const std::string& account1, DifficultyMode difficulty1)
{
    std::cout << "À votre tour, Joueur 2." << std::endl;
    std::cout << "J2 peut charger/créer un compte local, ou utiliser un personnage éphémère." << std::endl;
    std::cout << std::endl;

    if (player1.isAlteredByCheats())
    {
        std::cout << "Duel refusé." << std::endl;
        std::cout << "Un personnage altéré ne peut pas agir en JcJ." << std::endl;
        std::cout << "L'arène refuse les données instables pour protéger les autres comptes." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    PvpPlayerSlot player2Slot = askSecondPlayer(random);
    Player& player2 = player2Slot.player;

    if (player2.isAlteredByCheats())
    {
        std::cout << "Duel refusé." << std::endl;
        std::cout << "Un personnage altéré ne peut pas agir en JcJ." << std::endl;
        std::cout << "J2 doit choisir un personnage non altéré ou un personnage éphémère sain." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    if (isSameCharacterName(player1, player2))
    {
        std::cout << "Duel refusé." << std::endl;
        std::cout << "Deux personnages portant le même nom ne peuvent pas s'affronter." << std::endl;
        std::cout << "Raison : éviter les doublons copiés/collés et la duplication de ressources par suicide répété." << std::endl;
        std::cout << std::endl;
        Console::waitForEnter();
        Console::clear();
        return;
    }

    displayPvpSecuritySummary(player1, difficulty1, account1, player2Slot);
    Console::waitForEnter();
    Console::clear();

    bool deadlyDuel = askDeadlyDuelIfAllowed(player1, account1, player2Slot, difficulty1);
    bool lethalDeadlyDuel = deadlyDuel && isLethal(difficulty1) && isLethal(player2Slot.difficulty);
    FriendlyArenaSnapshot player1FriendlySnapshot = createFriendlyArenaSnapshot(player1);
    FriendlyArenaSnapshot player2FriendlySnapshot = createFriendlyArenaSnapshot(player2);
    int player1PreFightValue = estimateInventoryAndEquipmentValue(player1);
    int player2PreFightValue = estimateInventoryAndEquipmentValue(player2);

    Console::clear();

    std::cout << player2.getName() << ", tes statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << "Le duel peut maintenant commencer." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);

    player2.recordCombatStarted();
    int turn = random.chooseFirstTurn();

    std::cout << "Préparez-vous..." << std::endl;
    Console::pauseSeconds(2);
    std::cout << "Le combat commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!player1.isDead() && !player2.isDead())
    {
        bool turnFinished = false;

        if (turn == 1)
        {
            turnFinished = TurnManager::playHumanTurn(
                player1,
                player2,
                random,
                POTION_HEAL_AMOUNT,
                POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                turn = 2;
            }
        }
        else
        {
            turnFinished = TurnManager::playHumanTurn(
                player2,
                player1,
                random,
                POTION_HEAL_AMOUNT,
                POTION_DAMAGE_BONUS
            );

            if (turnFinished)
            {
                turn = 1;
            }
        }
    }

    CombatDisplay::displayCombatResult(player1, player2);

    Player* winner = player1.isDead() ? &player2 : &player1;
    Player* loser = player1.isDead() ? &player1 : &player2;

    winner->recordPvpVictory();
    loser->recordPvpDefeat();

    if (deadlyDuel)
    {
        winner->recordVictory();
        loser->recordDefeat();

        if (lethalDeadlyDuel)
        {
            std::string loserAccount = (loser == &player1) ? account1 : player2Slot.accountName;
            winner->recordPvpLethalElimination(loser->getName(), loserAccount);
        }

        int winnerPreFightValue = (winner == &player1) ? player1PreFightValue : player2PreFightValue;
        applyDeadlyDuelReward(*winner, *loser, random, lethalDeadlyDuel, winnerPreFightValue);
    }
    else
    {
        restoreFriendlyArenaSnapshot(player1, player1FriendlySnapshot);
        restoreFriendlyArenaSnapshot(player2, player2FriendlySnapshot);

        std::cout << "Combat amical terminé : PV, consommables, équipement et inventaire reviennent à l'état d'avant-duel." << std::endl;
        std::cout << "Aucun gain ou dommage sérieux n'est conservé hors statistiques JcJ." << std::endl;
        applyFriendlyDuelSymbolicReward(*winner);
        std::cout << "Stat JcJ mise à jour : victoire pour " << winner->getName()
                  << ", défaite pour " << loser->getName() << "." << std::endl;
        std::cout << std::endl;
    }

    if (player2Slot.persistent)
    {
        SaveManager::savePlayerSnapshot(player2, player2Slot.accountName, player2Slot.difficulty);

        if (lethalDeadlyDuel && player2.isDead())
        {
            if (SaveManager::movePlayableCharacterToDead(player2Slot.accountName, player2.getName()))
            {
                std::cout << "J2 est mort en duel Léthal : le personnage rejoint le registre des morts." << std::endl;
            }
            else
            {
                std::cout << "Attention : impossible de déplacer automatiquement J2 dans le registre des morts." << std::endl;
            }
        }
        else
        {
            std::cout << "Sauvegarde J2 mise à jour : " << player2.getName() << "." << std::endl;
        }

        std::cout << std::endl;
    }
}
