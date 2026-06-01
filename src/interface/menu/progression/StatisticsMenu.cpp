// EN: StatisticsMenu.cpp implements a normalized statistics hub for terminal navigation.
// FR: StatisticsMenu.cpp implémente un centre de statistiques normalisé pour la navigation terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/progression/StatisticsMenu.hpp"

#include "core/Console.hpp"
#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "item/Inventory.hpp"
#include "progression/Level.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <vector>


namespace
{

    MenuOptionItemData makeStatisticsItemData(const std::string& actionType, const std::string& name, const std::string& detail)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "progression";
        itemData.section = "Statistiques / progression";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = "Consultable";
        itemData.important = actionType == "inspect";
        return itemData;
    }

    std::string skillNameFromId(const std::string& id)
    {
        static const std::map<std::string, std::string> names = {
            {"night_vision", "Vision nocturne"},
            {"survival_breath", "Souffle de survie"},
            {"steady_guard", "Garde stable"},
            {"living_rampart", "Rempart vivant"},
            {"sure_hand", "Main sûre"},
            {"careful_dosage", "Dosage prudent"},
            {"ranger_eye", "Œil de rôdeur"},
            {"chain_execution", "Enchaînement"},
            {"reflex_counter", "Contre réflexe"},
            {"cautious_channeling", "Canalisation prudente"},
            {"shadow_step", "Pas de l'ombre"},
            {"arcane_impulse", "Élan arcanique"},
            {"tracking_mark", "Marque de pisteur"},
            {"prepared_volley", "Salve préparée"},
            {"blade_discipline", "Discipline de lame"},
            {"splitting_blow", "Frappe fendue"},
            {"armor_crack", "Fracasse-garde"},
            {"reach_control", "Contrôle d'allonge"}
        };

        auto found = names.find(id);
        if (found != names.end())
        {
            return found->second;
        }

        return id;
    }

    void appendSkillList(std::vector<std::string>& lines, const std::string& title, const std::vector<std::string>& skills)
    {
        lines.push_back(title);
        if (skills.empty())
        {
            lines.push_back("- Aucune pour le moment.");
            return;
        }

        for (const std::string& skillId : skills)
        {
            lines.push_back("- " + skillNameFromId(skillId));
        }
    }

    std::string progressLine(const std::string& label, int current, int target)
    {
        if (current >= target)
        {
            return "- " + label + " : trace maîtrisée";
        }

        return "- " + label + " : " + std::to_string(current) + "/" + std::to_string(target);
    }

    std::string toLowerLocal(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    std::string masteryState(int current, int target)
    {
        if (target <= 0 || current >= target)
        {
            return "maîtrisée";
        }

        if (current <= 0)
        {
            return "à découvrir";
        }

        if (current * 2 >= target)
        {
            return "proche";
        }

        return "en éveil";
    }

    void appendSkillRoadmap(std::vector<std::string>& lines, const Player& player)
    {
        const std::string playerClass = toLowerLocal(player.getType());

        lines.push_back("");
        lines.push_back("Pistes de maîtrise visibles :");

        if (playerClass.find("assassin") != std::string::npos)
        {
            lines.push_back("- Voie de classe | Source : assassin | État : mobilité et exécution à renforcer");
        }
        else if (playerClass.find("colosse") != std::string::npos)
        {
            lines.push_back("- Voie de classe | Source : colosse | État : endurance et protection à renforcer");
        }
        else if (playerClass.find("mage") != std::string::npos)
        {
            lines.push_back("- Voie de classe | Source : mage | État : canalisation et grimoires à renforcer");
        }
        else
        {
            lines.push_back("- Voie de classe | Source : " + player.getType() + " | État : style de combat à stabiliser");
        }

        lines.push_back("- Héritage racial | Source : " + player.getRaceText() + " | État : affinités à observer en combat");
        lines.push_back(
            "- Arme dominante | Source : " + player.getEquippedWeapon().getName()
            + " | État : " + masteryState(
                std::max({
                    player.getDaggerKillProgress(),
                    player.getBowKillProgress(),
                    player.getBareHandKillProgress(),
                    player.getStaffKillProgress(),
                    player.getSwordKillProgress(),
                    player.getAxeKillProgress(),
                    player.getHammerKillProgress(),
                    player.getSpearKillProgress()
                }),
                7
            )
        );
        lines.push_back("- Apprentissage libre | Source : bibliothèques, grimoires, essais et conditions spéciales | État : verrouillé par rareté et prérequis");
    }

    void showStatisticsScreen(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }
}


MenuScreen StatisticsMenu::buildHubScreen()
{
    MenuScreen screen("STATISTIQUES", "statistics.hub");
    screen.addOption(1, "Résumé du personnage", "Identité, niveau, expérience et état général.", true, "statistics.summary", makeStatisticsItemData("inspect", "Résumé du personnage", "Identité, niveau, expérience et état général."));
    screen.addOption(2, "Statistiques de combat", "Combats, boss, JcJ, morts et difficulté.", true, "statistics.combat", makeStatisticsItemData("inspect", "Statistiques de combat", "Combats, boss, JcJ, morts et difficulté."));
    screen.addOption(3, "Équipement et objets récents", "Équipement actuel et traces d'utilisation.", true, "statistics.equipment", makeStatisticsItemData("inspect", "Équipement et objets récents", "Équipement actuel et traces d'utilisation."));
    screen.addOption(4, "Compétences / progression", "Passifs, techniques et traces d'apprentissage.", true, "statistics.skills", makeStatisticsItemData("inspect", "Compétences / progression", "Passifs, techniques et traces d'apprentissage."));
    screen.addOption(5, "États spéciaux et conséquences", "Altérations, clones, dettes et marques de boss.", true, "statistics.states", makeStatisticsItemData("inspect", "États spéciaux", "Altérations, clones, dettes et marques de boss."));
    screen.addOption(6, "Affichage complet historique", "Afficher les statistiques longues du personnage.", true, "statistics.full_history", makeStatisticsItemData("inspect", "Historique complet", "Afficher les statistiques longues du personnage."));
    screen.addBackOption("Retour", "statistics.back");
    return screen;
}

void StatisticsMenu::open(const Player& player, DifficultyMode difficulty)
{
    const bool difficultyKnown = true;
    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildHubScreen(),
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displaySummary(player);
        }
        else if (choice == 2)
        {
            displayCombatStats(player, difficulty, difficultyKnown);
        }
        else if (choice == 3)
        {
            displayEquipmentUsage(player);
        }
        else if (choice == 4)
        {
            displaySkillStats(player);
        }
        else if (choice == 5)
        {
            displaySpecialStates(player);
        }
        else if (choice == 6)
        {
            MessageScreen::show(
                "HISTORIQUE COMPLET",
                "statistics.full_history.intro",
                {
                    "Affichage long du personnage.",
                    "Cette vue garde encore le registre complet, même si certaines lignes restent très compactes."
                }
            );
            player.displayStats();
            player.displayCareerStatistics(difficulty);
            Console::waitForEnter();
            Console::clear();
        }
    }
}

void StatisticsMenu::open(const Player& player)
{
    const bool difficultyKnown = false;
    DifficultyMode difficulty = DifficultyMode::Normal;

    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildHubScreen(),
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displaySummary(player);
        }
        else if (choice == 2)
        {
            displayCombatStats(player, difficulty, difficultyKnown);
        }
        else if (choice == 3)
        {
            displayEquipmentUsage(player);
        }
        else if (choice == 4)
        {
            displaySkillStats(player);
        }
        else if (choice == 5)
        {
            displaySpecialStates(player);
        }
        else if (choice == 6)
        {
            MessageScreen::show(
                "HISTORIQUE COMPLET",
                "statistics.full_history.intro.unknown_difficulty",
                {
                    "Affichage long du personnage.",
                    "La difficulté exacte n'est pas transmise par ce chemin de menu.",
                    "Cette vue garde encore le registre complet, même si certaines lignes restent très compactes."
                }
            );
            player.displayStats();
            player.displayCareerStatistics(difficulty);
            Console::waitForEnter();
            Console::clear();
        }
    }
}

void StatisticsMenu::displaySummary(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Nom : " + player.getName());
    lines.push_back("Race : " + player.getRaceText());
    lines.push_back("Classe : " + player.getType());
    lines.push_back("Niveau : " + std::to_string(player.getLevel()));

    int nextLevelExperience = Level::getExperienceRequiredForNextLevel(player.getLevel());
    if (nextLevelExperience > 0)
    {
        lines.push_back("Expérience : " + std::to_string(player.getExperience()) + "/" + std::to_string(nextLevelExperience));
    }
    else
    {
        lines.push_back("Expérience : niveau maximum");
    }

    lines.push_back("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
    lines.push_back("Or : " + std::to_string(player.getInventory().getGold()));
    lines.push_back("État : " + std::string(player.isAlteredByCheats() ? "Altéré" : "Normal"));
    lines.push_back("Clone : " + std::string(player.isClone() ? "oui" : "non"));

    showStatisticsScreen("RÉSUMÉ DU PERSONNAGE", "statistics.summary.detail", lines);
}

void StatisticsMenu::displayCombatStats(const Player& player, DifficultyMode difficulty, bool difficultyKnown)
{
    std::vector<std::string> lines;
    lines.push_back("Combats lancés : " + std::to_string(player.getCombatsStarted()));
    lines.push_back("Victoires : " + std::to_string(player.getVictories()));
    lines.push_back("Défaites : " + std::to_string(player.getDefeats()));
    lines.push_back("Fuites réussies : " + std::to_string(player.getEscapes()));
    lines.push_back("Ennemis vaincus : " + std::to_string(player.getEnemiesKilled()));
    lines.push_back("Boss vaincus : " + std::to_string(player.getBossesKilled()));
    lines.push_back("JcJ remportés : " + std::to_string(player.getPvpVictories()));
    lines.push_back("JcJ perdus : " + std::to_string(player.getPvpDefeats()));

    if (difficultyKnown && difficulty == DifficultyMode::Lethal)
    {
        lines.push_back("Morts du personnage : [STATISTIQUE CORROMPUE]");
        lines.push_back("Vous ne deviez pas mourir.");
    }
    else
    {
        lines.push_back("Morts du personnage : " + std::to_string(player.getDeaths()));

        if (!difficultyKnown)
        {
            lines.push_back("Note : le contexte de difficulté exact n'est pas connu depuis ce sous-menu.");
        }
    }

    const std::vector<std::string>& eliminations = player.getPvpLethalEliminations();
    if (!eliminations.empty())
    {
        lines.push_back("Éliminations Léthal en JcJ :");
        for (const std::string& entry : eliminations)
        {
            lines.push_back("- " + entry);
        }
    }

    showStatisticsScreen("STATISTIQUES DE COMBAT", "statistics.combat.detail", lines);
}

void StatisticsMenu::displayEquipmentUsage(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Équipement actuel :");
    lines.push_back("- Arme : " + player.getEquippedWeapon().getName());
    lines.push_back("- Armure : " + player.getEquippedArmor().getName());
    lines.push_back("");
    lines.push_back("Équipement récent :");

    const std::vector<std::string>& usage = player.getRecentCombatEquipmentUsage();
    if (usage.empty())
    {
        lines.push_back("- Aucun historique récent d'équipement.");
    }
    else
    {
        std::map<std::string, int> counts;
        for (const std::string& itemName : usage)
        {
            counts[itemName]++;
        }

        for (const auto& pair : counts)
        {
            lines.push_back("- " + pair.first + " : " + std::to_string(pair.second) + " apparition(s) récente(s)");
        }
    }

    showStatisticsScreen("ÉQUIPEMENT ET OBJETS RÉCENTS", "statistics.equipment.detail", lines);
}

void StatisticsMenu::displaySkillStats(const Player& player)
{
    std::vector<std::string> lines;
    appendSkillList(lines, "Passives connues :", player.getUnlockedPassiveSkills());
    lines.push_back("");
    appendSkillList(lines, "Actives connues :", player.getUnlockedActiveSkills());
    lines.push_back("");
    lines.push_back("Traces d'entraînement visibles :");
    lines.push_back(progressLine("Armes courtes / dagues", player.getDaggerKillProgress(), 5));
    lines.push_back(progressLine("Arcs et tirs", player.getBowKillProgress(), 8));
    lines.push_back(progressLine("Combat à mains nues", player.getBareHandKillProgress(), 10));
    lines.push_back(progressLine("Bâtons et canalisation", player.getStaffKillProgress(), 6));
    lines.push_back(progressLine("Épées et discipline", player.getSwordKillProgress(), 7));
    lines.push_back(progressLine("Haches et ouverture", player.getAxeKillProgress(), 7));
    lines.push_back(progressLine("Marteaux et fracture de garde", player.getHammerKillProgress(), 7));
    lines.push_back(progressLine("Lances et contrôle de distance", player.getSpearKillProgress(), 7));
    lines.push_back("");
    lines.push_back("Ces traces montrent ce que ton personnage répète assez souvent pour l'intégrer à son style.");
    appendSkillRoadmap(lines, player);

    showStatisticsScreen("COMPÉTENCES", "statistics.skills.detail", lines);
}

void StatisticsMenu::displaySpecialStates(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Altéré : " + std::string(player.isAlteredByCheats() ? "oui" : "non"));
    lines.push_back("Clone : " + std::string(player.isClone() ? "oui" : "non"));

    if (player.hasWorldGazePenalty())
    {
        lines.push_back("Regard du monde : actif");
        lines.push_back("Tentatives de rupture Léthal : " + std::to_string(player.getLethalCheatAttemptCount()));
    }
    else if (player.getLethalCheatAttemptCount() > 0)
    {
        lines.push_back("Trace interdite : ancienne tentative de cheat Léthal détectée");
        lines.push_back("Tentatives : " + std::to_string(player.getLethalCheatAttemptCount()));
    }

    if (player.hasZelefCorrosionPresent())
    {
        lines.push_back("Corrosion présente : " + std::to_string(player.getZelefMaxHpStolen()) + " PV max retenus");
    }

    if (player.hasGrinkaBossTheftPresent())
    {
        lines.push_back("Volé par un boss : Grinka détient encore quelque chose.");
    }

    if (player.hasBossEquipmentSeal())
    {
        lines.push_back("Équipement scellé : " + player.getBossEquipmentSealReason());
    }

    if (lines.size() == 2)
    {
        lines.push_back("Aucune conséquence spéciale majeure visible pour le moment.");
    }

    showStatisticsScreen("ÉTATS SPÉCIAUX", "statistics.states.detail", lines);
}
