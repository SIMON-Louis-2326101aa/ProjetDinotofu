// EN: PvpMode.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PvpMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pvp/PvpMode.hpp"

#include "combat/TurnManager.hpp"

#include "core/Console.hpp"
#include "class_system/ClassCatalog.hpp"
#include "interface/CombatDisplay.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/DeathRuleRules.hpp"
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
        DeathRuleMode deathRule = DeathRuleMode::NonDefinitive;
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

    void showLocalPvpResultScreen(
        const Player& winner,
        const Player& loser,
        bool deadlyDuel,
        bool definitiveDeadlyDuel
    )
    {
        MenuScreen screen("BILAN PVP LOCAL", "pvp.local.result.conclusion");
        screen.addSubtitle("Résumé de sortie de duel");
        screen.addLine("Résultat : victoire de " + winner.getName());
        screen.addLine("Vaincu : " + loser.getName());
        screen.addLine("Mode : " + std::string(deadlyDuel ? "duel sérieux" : "duel amical"));
        screen.addLine("Mort définitive : " + std::string(definitiveDeadlyDuel ? "oui" : "non"));
        screen.addLine("PV vainqueur : " + std::to_string(winner.getHp()) + "/" + std::to_string(winner.getMaxHp()));
        screen.addLine("PV vaincu : " + std::to_string(loser.getHp()) + "/" + std::to_string(loser.getMaxHp()));
        screen.addLine("Statistiques : victoire/défaite JcJ mises à jour");
        screen.addLine("Sauvegarde J2 : traitée après ce bilan si J2 est persistant");
        screen.setDisplayOnlyInput("Résumé affiché sans saisie directe.");
        TerminalInterface::renderMenuScreen(screen, false);
    }

    void showPlayer2SaveStatusScreen(
        const Player& player2,
        const std::string& accountName,
        const std::string& status,
        const std::string& detail
    )
    {
        MenuScreen screen("SAUVEGARDE J2", "pvp.local.player2.save.status");
        screen.addSubtitle("Bilan de persistance du deuxième joueur");
        screen.addLine("Joueur : " + player2.getName());
        screen.addLine("Compte : " + accountName);
        screen.addLine("Statut : " + status);
        screen.addLine("Détail : " + detail);
        screen.addLine("PV actuels : " + std::to_string(player2.getHp()) + "/" + std::to_string(player2.getMaxHp()));
        screen.setDisplayOnlyInput("Résumé affiché sans saisie directe.");
        TerminalInterface::renderMenuScreen(screen, false);
    }

    // EN: applyFriendlyDuelSymbolicReward declares or implements a focused behavior used by this module.
    // FR: applyFriendlyDuelSymbolicReward déclare ou implémente un comportement précis utilisé par ce module.
    void applyFriendlyDuelSymbolicReward(Player& winner)
    {
        int symbolicGold = 3;
        int symbolicExperience = 8;

        winner.getInventory().earnGold(symbolicGold);
        winner.gainExperience(symbolicExperience);

        MessageScreen::show(
            "GAIN SYMBOLIQUE",
            "pvp.local.friendly.reward",
            {
                "Gain symbolique de l'arène pour " + winner.getName() + " : "
                    + std::to_string(symbolicGold) + " or et "
                    + std::to_string(symbolicExperience) + " expérience."
            },
            false
        );
    }


    MenuOptionItemData makePvpCreationItemData(
        const std::string& kind,
        const std::string& name,
        const std::string& detail,
        const std::string& status = ""
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = "Création J2";
        itemData.actionType = "select";
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.important = false;
        return itemData;
    }

    MenuScreen buildPvpClassCategoryScreen(const std::string& playerName)
    {
        MenuScreen screen("FAMILLE DE CLASSE J2", "pvp.local.j2.class_category");
        screen.addLine(playerName + " doit choisir une famille de classe.");
        screen.addLine("Les numéros restent visibles pour garder le lien avec le terminal.");

        for (int choice = 1; choice <= ClassCatalog::getClassCategoryCount(); ++choice)
        {
            const std::string categoryName = ClassCatalog::getClassCategoryNameByChoice(choice);
            const int classCount = ClassCatalog::getPlayableClassCountByCategoryChoice(choice);
            screen.addOption(
                choice,
                categoryName,
                std::to_string(classCount) + " classe(s) disponibles.",
                true,
                "pvp.local.j2.class_category." + std::to_string(choice),
                makePvpCreationItemData("class_category", categoryName, std::to_string(classCount) + " classe(s) disponibles.")
            );
        }

        return screen;
    }

    MenuScreen buildPvpClassChoiceScreen(
        int categoryChoice,
        const std::vector<ClassOptionInfo>& classes,
        std::size_t pageIndex,
        std::size_t totalPages,
        std::size_t firstIndex,
        std::size_t lastIndex
    )
    {
        const std::string categoryName = ClassCatalog::getClassCategoryNameByChoice(categoryChoice);
        MenuScreen screen("CLASSE J2", "pvp.local.j2.class_choice");
        screen.setPagination(pageIndex, totalPages);
        screen.addLine("Famille sélectionnée : " + categoryName + ".");
        screen.addLine("Choisis la classe qui portera le duel.");
        screen.addLine("Page : " + std::to_string(pageIndex + 1) + "/" + std::to_string(totalPages));
        screen.addLine("Classes affichées : " + PagedMenu::rangeText(firstIndex, lastIndex, classes.size()));

        for (std::size_t index = firstIndex; index < lastIndex; ++index)
        {
            const ClassOptionInfo& option = classes[index];
            const int localChoice = static_cast<int>(index - firstIndex + 1);
            MenuOptionItemData itemData = makePvpCreationItemData(
                "class",
                option.name,
                option.role,
                "PV " + std::to_string(option.maxHp)
                    + " · Dégâts " + std::to_string(option.minDamage) + "-" + std::to_string(option.maxDamage)
                    + " · Crit " + std::to_string(option.criticalDamage)
            );
            itemData.progress = option.categoryName;
            itemData.quantity = "Soin x" + std::to_string(option.healingPotionCount);
            itemData.reward = "Dégâts x" + std::to_string(option.damagePotionCount);

            screen.addOption(
                localChoice,
                option.name,
                option.role + " | PV " + std::to_string(option.maxHp)
                    + " | Dégâts " + std::to_string(option.minDamage) + "-" + std::to_string(option.maxDamage),
                true,
                "pvp.local.j2.class_choice." + std::to_string(index + 1),
                itemData
            );
        }

        if (totalPages > 1 && pageIndex > 0)
        {
            screen.addOption(98, "Page précédente", "Voir les classes précédentes.", true, "pvp.local.j2.class_choice.page.previous");
        }

        if (totalPages > 1 && pageIndex + 1 < totalPages)
        {
            screen.addOption(99, "Page suivante", "Voir les classes suivantes.", true, "pvp.local.j2.class_choice.page.next");
        }

        return screen;
    }

    int askPvpClassChoice(int categoryChoice)
    {
        const std::vector<ClassOptionInfo> classes = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);
        constexpr std::size_t itemsPerPage = 8;
        std::size_t pageIndex = 0;
        const std::size_t totalPages = PagedMenu::pageCount(classes.size(), itemsPerPage);

        while (true)
        {
            const std::size_t firstIndex = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(classes.size(), pageIndex, itemsPerPage);

            MenuScreen screen = buildPvpClassChoiceScreen(
                categoryChoice,
                classes,
                pageIndex,
                totalPages,
                firstIndex,
                lastIndex
            );

            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Veuillez choisir une classe visible sur la page."
            );

            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                Console::clear();
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                Console::clear();
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(choice - 1);
            if (selectedIndex >= classes.size() || selectedIndex >= lastIndex)
            {
                Console::clear();
                MessageScreen::show(
                    "CHOIX REFUSÉ",
                    "pvp.local.j2.class_choice.invalid_page",
                    {"Cette classe n'est pas visible sur la page actuelle."},
                    false
                );
                continue;
            }

            return static_cast<int>(selectedIndex + 1);
        }
    }

    // EN: askDifficultyForSecondPlayer declares or implements a focused behavior used by this module.
    // FR: askDifficultyForSecondPlayer déclare ou implémente un comportement précis utilisé par ce module.
    DifficultyMode askDifficultyForSecondPlayer()
    {
        MenuScreen screen("DIFFICULTÉ J2", "pvp.local.j2.difficulty");
        screen.addLine("Choisis la difficulté du personnage J2.");
        screen.addOption(1, "Facile", "Simulation plus douce.", true, "pvp.local.j2.difficulty.easy");
        screen.addOption(2, "Normal", "Règles standard.", true, "pvp.local.j2.difficulty.normal");
        screen.addOption(3, "Difficile", "Pénalités et dangers renforcés.", true, "pvp.local.j2.difficulty.hard");
        screen.addOption(4, "Cauchemar", "La marge d'erreur devient faible.", true, "pvp.local.j2.difficulty.nightmare");
        screen.addOption(5, "Léthal", "La mort peut devenir définitive.", true, "pvp.local.j2.difficulty.lethal");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Veuillez choisir une difficulté affichée.");

        if (choice == 1) return DifficultyMode::Easy;
        if (choice == 3) return DifficultyMode::Hard;
        if (choice == 4) return DifficultyMode::Nightmare;
        if (choice == 5) return DifficultyMode::Lethal;
        return DifficultyMode::Normal;
    }


    DeathRuleMode askDeathRuleForSecondPlayer(DifficultyMode difficulty)
    {
        DeathRuleMode forcedRule = DeathRuleRules::defaultForDifficulty(difficulty);

        if (DeathRuleRules::isChoiceForced(difficulty))
        {
            MessageScreen::show(
                "RÈGLE DE MORT J2",
                "pvp.local.j2.death_rule.forced",
                {
                    "Règle appliquée : " + DeathRuleRules::displayName(forcedRule) + ".",
                    DeathRuleRules::shortSummary(difficulty, forcedRule)
                },
                false
            );
            return forcedRule;
        }

        MenuScreen screen("RÈGLE DE MORT J2", "pvp.local.j2.death_rule");
        screen.addLine("Choisis la règle de mort du personnage J2.");
        screen.addLine("Elle est séparée de la difficulté, sauf Facile et Léthal.");
        screen.addOption(1, "Mort non définitive", "Pénalités possibles, mais pas de registre mort jouable.", true, "pvp.local.j2.death_rule.non_definitive");
        screen.addOption(2, "Mort définitive", "Challenge supplémentaire : une vraie chute peut tuer le personnage.", true, "pvp.local.j2.death_rule.definitive");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Veuillez choisir 1 ou 2.");
        return DeathRuleRules::normalizeForDifficulty(
            difficulty,
            choice == 2 ? DeathRuleMode::Definitive : DeathRuleMode::NonDefinitive
        );
    }

    // EN: createManualPlayer declares or implements a focused behavior used by this module.
    // FR: createManualPlayer déclare ou implémente un comportement précis utilisé par ce module.
    Player createManualPlayer(const std::string& playerLabel, DifficultyMode difficulty)
    {
        std::string playerName = MessageScreen::askText(
            "NOM DU JOUEUR 2",
            "pvp.local.j2.manual_name",
            {
                playerLabel + ", quel est ton nom ?",
                "Ce personnage peut rester éphémère selon le chemin choisi."
            },
            "Nom du joueur 2",
            "Nom affiché pendant le duel local.",
            false,
            1,
            32
        );

        Console::clear();

        int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            buildPvpClassCategoryScreen(playerName),
            "Veuillez choisir une famille affichée."
        );

        Console::clear();

        int classChoice = askPvpClassChoice(categoryChoice);

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
            MenuScreen screen("JOUEUR 2", "pvp.local.j2.source");
            screen.addLine("J2 peut charger/créer un compte local, ou utiliser un personnage éphémère.");
            screen.addOption(1, "Charger / créer un compte local", "Le personnage sera sauvegardé et pourra recevoir des statistiques JcJ.", true, "pvp.local.j2.account");
            screen.addOption(2, "Utiliser un personnage éphémère", "Combat rapide, sans compte persistant.", true, "pvp.local.j2.ephemeral");

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Veuillez choisir 1 ou 2.");
            Console::clear();

            if (choice == 2)
            {
                PvpPlayerSlot slot;
                slot.difficulty = DifficultyMode::Normal;
                slot.deathRule = DeathRuleRules::defaultForDifficulty(slot.difficulty);
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
                slot.deathRule = result.deathRule;
                slot.persistent = true;
                slot.ephemeral = false;
                return slot;
            }

            slot.difficulty = askDifficultyForSecondPlayer();
            Console::clear();
            slot.deathRule = askDeathRuleForSecondPlayer(slot.difficulty);
            Console::clear();
            slot.player = createManualPlayer("Joueur 2", slot.difficulty);
            slot.persistent = true;
            slot.ephemeral = false;

            SaveManager::saveAccountSnapshot(slot.accountName);
            SaveManager::savePlayerSnapshot(slot.player, slot.accountName, slot.difficulty, slot.deathRule);
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

    bool usesDefinitiveDeath(DifficultyMode difficulty, DeathRuleMode deathRule)
    {
        return DifficultyRules::isPermanentDeath(difficulty, deathRule);
    }

    bool canStartDeadlyDuel(
        DifficultyMode firstDifficulty,
        DeathRuleMode firstDeathRule,
        DifficultyMode secondDifficulty,
        DeathRuleMode secondDeathRule
    )
    {
        return usesDefinitiveDeath(firstDifficulty, firstDeathRule)
            == usesDefinitiveDeath(secondDifficulty, secondDeathRule);
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
        DeathRuleMode deathRule1,
        const std::string& account1,
        const PvpPlayerSlot& player2Slot
    )
    {
        std::vector<std::string> lines;

        std::string p1Line = player1.getName() + " | compte local : " + account1
            + " | difficulté : " + pvpDifficultyName(difficulty1)
            + " | " + DeathRuleRules::displayName(deathRule1);
        if (player1.isClone()) p1Line += " | CLONE";
        lines.push_back(p1Line);

        std::string p2Line = player2Slot.player.getName() + " | " + pvpAccountStatus(player2Slot)
            + " | difficulté : " + pvpDifficultyName(player2Slot.difficulty)
            + " | " + DeathRuleRules::displayName(player2Slot.deathRule);
        if (player2Slot.player.isClone()) p2Line += " | CLONE";
        lines.push_back(p2Line);
        lines.push_back("");

        if (player1.isClone() || player2Slot.player.isClone())
        {
            lines.push_back("Verdict : combat amical uniquement.");
            lines.push_back("Un clone peut se battre pour le fun, mais pas créer de vrai butin.");
        }
        else if (!player2Slot.persistent || account1 == player2Slot.accountName)
        {
            lines.push_back("Verdict : combat amical uniquement.");
            lines.push_back("Le duel mortel exige deux comptes locaux différents et persistants.");
        }
        else if (!canStartDeadlyDuel(difficulty1, deathRule1, player2Slot.difficulty, player2Slot.deathRule))
        {
            lines.push_back("Verdict : combat mortel visible, mais verrouillé.");
            lines.push_back("Une existence avec mort définitive ne peut pas être mélangée avec une simulation non définitive.");
        }
        else if (usesDefinitiveDeath(difficulty1, deathRule1))
        {
            lines.push_back("Verdict : duel mortel définitif autorisé.");
            lines.push_back("Le perdant quittera le registre des vivants.");
        }
        else
        {
            lines.push_back("Verdict : duel mortel non définitif autorisé.");
            lines.push_back("Le butin restera limité par la valeur du gagnant pour éviter les abus.");
        }

        MessageScreen::show("CONTRÔLE DE L'ARÈNE", "pvp.local.security_summary", lines, false);
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
    bool stealOneWeapon(Player& winner, Player& loser, Random& random, int chance, std::vector<std::string>& lootLines)
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
            lootLines.push_back(winner.getName() + " récupère l'arme : " + weapon.getName() + ".");
            return true;
        }

        return false;
    }

    // EN: stealOneArmor declares or implements a focused behavior used by this module.
    // FR: stealOneArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneArmor(Player& winner, Player& loser, Random& random, int chance, std::vector<std::string>& lootLines)
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
            lootLines.push_back(winner.getName() + " récupère l'armure : " + armor.getName() + ".");
            return true;
        }

        return false;
    }



    // EN: stealEquippedWeaponOnLethalDeath declares or implements a focused behavior used by this module.
    // FR: stealEquippedWeaponOnLethalDeath déclare ou implémente un comportement précis utilisé par ce module.
    bool stealEquippedWeaponOnLethalDeath(Player& winner, Player& loser, Random& random, std::vector<std::string>& lootLines)
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
            lootLines.push_back("L'arme équipée de " + loser.getName() + " échappe au pillage dans le chaos du duel.");
            return false;
        }

        loser.unequipWeapon();
        winner.getInventory().addWeapon(weapon);
        loser.getInventory().removeWeapon(index);

        lootLines.push_back(winner.getName() + " récupère l'arme portée par le défunt : " + weapon.getName() + ".");
        return true;
    }

    // EN: stealEquippedArmorOnLethalDeath declares or implements a focused behavior used by this module.
    // FR: stealEquippedArmorOnLethalDeath déclare ou implémente un comportement précis utilisé par ce module.
    bool stealEquippedArmorOnLethalDeath(Player& winner, Player& loser, Random& random, std::vector<std::string>& lootLines)
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
            lootLines.push_back("L'armure équipée de " + loser.getName() + " est trop endommagée pour être récupérée proprement.");
            return false;
        }

        loser.unequipArmor();
        winner.getInventory().addArmor(armor);
        loser.getInventory().removeArmor(index);

        lootLines.push_back(winner.getName() + " récupère l'armure portée par le défunt : " + armor.getName() + ".");
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
        MessageScreen::show(
            "ESTIMATION AVANT DUEL",
            "pvp.local.duel_value",
            {
                "Estimation de l'inventaire et de l'équipement de " + player1.getName()
                    + " : " + std::to_string(estimateInventoryAndEquipmentValue(player1)) + " pièces.",
                "Estimation de l'inventaire et de l'équipement de l'opposant " + player2.getName()
                    + " : " + std::to_string(estimateInventoryAndEquipmentValue(player2)) + " pièces."
            },
            false
        );
    }

    // EN: askProceedAfterDuelValueEstimation declares or implements a focused behavior used by this module.
    // FR: askProceedAfterDuelValueEstimation déclare ou implémente un comportement précis utilisé par ce module.
    bool askProceedAfterDuelValueEstimation(const Player& player1, const Player& player2)
    {
        displayDuelValueEstimation(player1, player2);

        MenuScreen screen("CONFIRMATION DU DUEL", "pvp.local.duel_confirm");
        screen.addLine("Procéder au combat ?");
        screen.addOption(1, "Oui", "L'arène verrouille le duel.", true, "pvp.local.duel_confirm.yes");
        screen.addOption(2, "Non", "Annuler avant engagement.", true, "pvp.local.duel_confirm.no");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Veuillez choisir 1 ou 2.");
        Console::clear();
        return choice == 1;
    }

    // EN: stealOneWeaponWithValueCap declares or implements a focused behavior used by this module.
    // FR: stealOneWeaponWithValueCap déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneWeaponWithValueCap(Player& winner, Player& loser, Random& random, int chance, int& stolenValue, int valueCap, std::vector<std::string>& lootLines)
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
            lootLines.push_back(winner.getName() + " récupère l'arme : " + weapon.getName() + " (valeur estimée " + std::to_string(value) + ").");
            return true;
        }

        return false;
    }

    // EN: stealOneArmorWithValueCap declares or implements a focused behavior used by this module.
    // FR: stealOneArmorWithValueCap déclare ou implémente un comportement précis utilisé par ce module.
    bool stealOneArmorWithValueCap(Player& winner, Player& loser, Random& random, int chance, int& stolenValue, int valueCap, std::vector<std::string>& lootLines)
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
            lootLines.push_back(winner.getName() + " récupère l'armure : " + armor.getName() + " (valeur estimée " + std::to_string(value) + ").");
            return true;
        }

        return false;
    }

    // EN: applyDeadlyDuelReward declares or implements a focused behavior used by this module.
    // FR: applyDeadlyDuelReward déclare ou implémente un comportement précis utilisé par ce module.
    void applyDeadlyDuelReward(Player& winner, Player& loser, Random& random, bool lethalDuel, int winnerPreFightValue)
    {
        std::vector<std::string> lootLines;
        lootLines.push_back("Vainqueur : " + winner.getName() + ".");
        lootLines.push_back("Perdant : " + loser.getName() + ".");

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
            lootLines.push_back("Or récupéré : " + std::to_string(stolenGold) + ".");
        }
        else
        {
            lootLines.push_back("Aucun or récupérable.");
        }

        if (!lethalDuel)
        {
            lootLines.push_back("Limite anti-abus : le perdant ne pourra pas perdre plus que la valeur estimée de son adversaire avant duel (" + std::to_string(valueCap) + " pièces).");
        }

        int stealChance = lethalDuel ? 78 : 35;
        int attempts = lethalDuel ? 5 : 1;

        bool itemStolen = false;

        if (lethalDuel)
        {
            itemStolen = stealEquippedWeaponOnLethalDeath(winner, loser, random, lootLines) || itemStolen;
            itemStolen = stealEquippedArmorOnLethalDeath(winner, loser, random, lootLines) || itemStolen;
        }

        for (int i = 0; i < attempts; ++i)
        {
            if (lethalDuel)
            {
                itemStolen = stealOneWeapon(winner, loser, random, stealChance, lootLines) || itemStolen;
                itemStolen = stealOneArmor(winner, loser, random, stealChance, lootLines) || itemStolen;
            }
            else
            {
                itemStolen = stealOneWeaponWithValueCap(winner, loser, random, stealChance, stolenValue, valueCap, lootLines) || itemStolen;
                itemStolen = stealOneArmorWithValueCap(winner, loser, random, stealChance, stolenValue, valueCap, lootLines) || itemStolen;
            }
        }

        if (!itemStolen)
        {
            lootLines.push_back("Aucun équipement non lié n'a été récupéré.");
        }

        int xpReward = lethalDuel ? 260 : 70;
        winner.gainExperience(xpReward);
        lootLines.push_back("Expérience gagnée : " + std::to_string(xpReward) + ".");

        if (lethalDuel)
        {
            lootLines.push_back("Duel à mort définitive : le perdant est considéré comme mort définitivement.");
            lootLines.push_back("Les objets liés à l'âme ou au corps restent protégés.");
            lootLines.push_back("Les reliques et objets rares ont une forte chance d'être récupérés, mais pas une garantie absolue.");
        }

        MessageScreen::show(
            lethalDuel ? "BUTIN DU DUEL DÉFINITIF" : "BUTIN DU DUEL MORTEL",
            lethalDuel ? "pvp.deadly_duel.loot.lethal" : "pvp.deadly_duel.loot.limited",
            lootLines,
            false
        );
    }

    bool askDeadlyDuelIfAllowed(
        const Player& player1,
        const std::string& account1,
        const PvpPlayerSlot& player2Slot,
        DifficultyMode difficulty1,
        DeathRuleMode deathRule1
    )
    {
        if (player1.isClone() || player2Slot.player.isClone())
        {
            MessageScreen::show(
                "DUEL AMICAL IMPOSÉ",
                "pvp.local.deadly.clone_refused",
                {
                    "Un clone ne peut pas participer à un combat à mort.",
                    "Un clone sert à jouer, tester et transporter une copie, pas à générer du vrai butin."
                }
            );
            return false;
        }

        if (!player2Slot.persistent || account1 == player2Slot.accountName)
        {
            MessageScreen::show(
                "DUEL AMICAL",
                "pvp.local.deadly.account_refused",
                {
                    "Gain symbolique seulement.",
                    "Le combat mortel n'est proposé que pour deux comptes différents."
                }
            );
            return false;
        }

        MenuScreen screen("TYPE DE DUEL", "pvp.local.duel_type");
        screen.addOption(1, "Combat amical", "Gain symbolique, consommables et usure remboursés par l'arène.", true, "pvp.local.duel_type.friendly");
        screen.addOption(2, "Combat à mort", "Butin réel. Risque définitif si les deux personnages utilisent la mort définitive.", true, "pvp.local.duel_type.deadly");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Veuillez choisir 1 ou 2.");
        Console::clear();

        if (choice == 1)
        {
            MessageScreen::show(
                "COMBAT AMICAL",
                "pvp.local.duel_type.friendly_confirmed",
                {"Gain symbolique, consommables et usure d'équipement remboursés par l'arène."}
            );
            return false;
        }

        if (!canStartDeadlyDuel(difficulty1, deathRule1, player2Slot.difficulty, player2Slot.deathRule))
        {
            MessageScreen::show(
                "COMBAT À MORT REFUSÉ",
                "pvp.local.duel_type.deadly_refused",
                {
                    "Un personnage avec mort définitive ne peut pas engager un duel mortel contre une simulation non définitive.",
                    "Il faut deux personnages avec la mort définitive, ou deux personnages sans mort définitive."
                }
            );
            return false;
        }

        std::vector<std::string> lines;
        if (usesDefinitiveDeath(difficulty1, deathRule1))
        {
            lines.push_back("Deux existences à mort définitive s'affrontent : le perdant sera inscrit comme mort.");
            lines.push_back("Ce n'est pas une grosse pénalité : c'est une mort définitive, sauf bénédiction capable de briser le verdict.");
        }
        else
        {
            lines.push_back("Deux personnages non définitif s'affrontent : butin limité, pas d'effacement définitif.");
        }

        MessageScreen::show("COMBAT À MORT CONFIRMÉ", "pvp.local.duel_type.deadly_confirmed", lines);

        if (!askProceedAfterDuelValueEstimation(player1, player2Slot.player))
        {
            MessageScreen::show(
                "DUEL ANNULÉ",
                "pvp.local.duel_type.cancelled_after_estimation",
                {"Duel mortel annulé après estimation."}
            );
            return false;
        }

        return true;
    }
}

// EN: run declares or implements a focused behavior used by this module.
// FR: run déclare ou implémente un comportement précis utilisé par ce module.
void PvpMode::run(Player& player1, Random& random, const std::string& account1, DifficultyMode difficulty1, DeathRuleMode deathRule1)
{
    MessageScreen::show(
        "JOUEUR 2",
        "pvp.local.j2.intro",
        {
            "À votre tour, Joueur 2.",
            "J2 peut charger/créer un compte local, ou utiliser un personnage éphémère."
        },
        false
    );

    if (player1.isAlteredByCheats())
    {
        MessageScreen::show(
            "DUEL REFUSÉ",
            "pvp.local.refused.player1_altered",
            {
                "Un personnage altéré ne peut pas agir en JcJ.",
                "L'arène refuse les données instables pour protéger les autres comptes."
            }
        );
        return;
    }

    PvpPlayerSlot player2Slot = askSecondPlayer(random);
    Player& player2 = player2Slot.player;

    if (player2.isAlteredByCheats())
    {
        MessageScreen::show(
            "DUEL REFUSÉ",
            "pvp.local.refused.player2_altered",
            {
                "Un personnage altéré ne peut pas agir en JcJ.",
                "J2 doit choisir un personnage non altéré ou un personnage éphémère sain."
            }
        );
        return;
    }

    if (isSameCharacterName(player1, player2))
    {
        MessageScreen::show(
            "DUEL REFUSÉ",
            "pvp.local.refused.same_name",
            {
                "Deux personnages portant le même nom ne peuvent pas s'affronter.",
                "Raison : éviter les doublons copiés/collés et la duplication de ressources par suicide répété."
            }
        );
        return;
    }

    displayPvpSecuritySummary(player1, difficulty1, deathRule1, account1, player2Slot);
    Console::waitForEnter();
    Console::clear();

    bool deadlyDuel = askDeadlyDuelIfAllowed(player1, account1, player2Slot, difficulty1, deathRule1);
    bool lethalDeadlyDuel = deadlyDuel
        && usesDefinitiveDeath(difficulty1, deathRule1)
        && usesDefinitiveDeath(player2Slot.difficulty, player2Slot.deathRule);
    FriendlyArenaSnapshot player1FriendlySnapshot = createFriendlyArenaSnapshot(player1);
    FriendlyArenaSnapshot player2FriendlySnapshot = createFriendlyArenaSnapshot(player2);
    int player1PreFightValue = estimateInventoryAndEquipmentValue(player1);
    int player2PreFightValue = estimateInventoryAndEquipmentValue(player2);

    Console::clear();

    MessageScreen::show(
        "DUEL PRÊT",
        "pvp.local.ready",
        {
            player2.getName() + ", tes statistiques ont été gravées dans l'arène.",
            "Le duel peut maintenant commencer."
        },
        false
    );

    CombatDisplay::displayCombatState(
        CombatDisplay::buildDuelSnapshot(player1, player2, "ÉTAT DU DUEL", "Pré-combat : deux combattants verrouillés"),
        false
    );

    Console::pauseSeconds(2);

    player2.recordCombatStarted();
    int turn = random.chooseFirstTurn();

    Console::pauseSeconds(2);
    MessageScreen::show(
        "DÉBUT DU DUEL",
        "pvp.local.fight.start",
        {"Le combat commence maintenant."},
        false
    );

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

    showLocalPvpResultScreen(*winner, *loser, deadlyDuel, lethalDeadlyDuel);

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

        MessageScreen::show(
            "COMBAT AMICAL TERMINÉ",
            "pvp.local.friendly.finished",
            {
                "PV, consommables, équipement et inventaire reviennent à l'état d'avant-duel.",
                "Aucun gain ou dommage sérieux n'est conservé hors statistiques JcJ.",
                "Stat JcJ mise à jour : victoire pour " + winner->getName() + ", défaite pour " + loser->getName() + "."
            },
            false
        );
        applyFriendlyDuelSymbolicReward(*winner);
    }

    if (player2Slot.persistent)
    {
        SaveManager::savePlayerSnapshot(player2, player2Slot.accountName, player2Slot.difficulty, player2Slot.deathRule);

        if (lethalDeadlyDuel && player2.isDead())
        {
            if (SaveManager::movePlayableCharacterToDead(player2Slot.accountName, player2.getName()))
            {
                showPlayer2SaveStatusScreen(
                    player2,
                    player2Slot.accountName,
                    "registre des morts",
                    "J2 est mort en duel à mort définitive : le personnage rejoint le registre des morts."
                );
            }
            else
            {
                showPlayer2SaveStatusScreen(
                    player2,
                    player2Slot.accountName,
                    "alerte registre",
                    "Attention : le registre des morts refuse d'emporter J2 proprement."
                );
            }
        }
        else
        {
            showPlayer2SaveStatusScreen(
                player2,
                player2Slot.accountName,
                "mise à jour",
                "Sauvegarde J2 mise à jour après le duel local."
            );
        }
    }
}
