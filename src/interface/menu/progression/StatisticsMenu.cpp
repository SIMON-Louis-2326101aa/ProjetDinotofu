// EN: StatisticsMenu.cpp implements a normalized statistics hub for terminal navigation.
// FR: StatisticsMenu.cpp implémente un centre de statistiques normalisé pour la navigation terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/progression/StatisticsMenu.hpp"

#include "core/Console.hpp"
#include "entity/Player.hpp"
#include "interface/menu/common/MenuFrame.hpp"
#include "interface/TerminalInterface.hpp"
#include "item/Inventory.hpp"
#include "progression/Level.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>


namespace
{
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

    void displaySkillList(const std::string& title, const std::vector<std::string>& skills)
    {
        std::cout << title << std::endl;
        if (skills.empty())
        {
            std::cout << "- Aucune pour le moment." << std::endl;
            return;
        }

        for (const std::string& skillId : skills)
        {
            std::cout << "- " << skillNameFromId(skillId) << std::endl;
        }
    }

    void displayProgressLine(const std::string& label, int current, int target)
    {
        if (current >= target)
        {
            std::cout << "- " << label << " : trace maîtrisée" << std::endl;
            return;
        }

        std::cout << "- " << label << " : " << current << "/" << target << std::endl;
    }
}


MenuScreen StatisticsMenu::buildHubScreen()
{
    MenuScreen screen("STATISTIQUES", "statistics.hub");
    screen.addOption(1, "Résumé du personnage", "Identité, niveau, expérience et état général.", true, "statistics.summary");
    screen.addOption(2, "Statistiques de combat", "Combats, boss, JcJ, morts et difficulté.", true, "statistics.combat");
    screen.addOption(3, "Équipement et objets récents", "Équipement actuel et traces d'utilisation.", true, "statistics.equipment");
    screen.addOption(4, "Compétences / progression", "Passifs, techniques et traces d'apprentissage.", true, "statistics.skills");
    screen.addOption(5, "États spéciaux et conséquences", "Altérations, clones, dettes et marques de boss.", true, "statistics.states");
    screen.addOption(6, "Affichage complet historique", "Afficher les statistiques longues du personnage.", true, "statistics.full_history");
    screen.addBackOption("Retour", "statistics.back");
    return screen;
}

// EN: open displays the statistics hub and returns to the caller without consuming combat turns.
// FR: open affiche le menu central des statistiques et revient à l'appelant sans consommer de tour.
void StatisticsMenu::open(const Player& player, DifficultyMode difficulty)
{
    const bool difficultyKnown = true;
    while (true)
    {
        TerminalInterface::renderMenuScreen(buildHubScreen());

        int choice = Console::askNumberBetween(0, 6, "Choix invalide. Entre un chiffre entre 0 et 6.");
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
            player.displayStats();
            player.displayCareerStatistics(difficulty);
        }

        Console::waitForEnter();
        Console::clear();
    }
}

// EN: open displays statistics when the caller does not know the current difficulty context.
// FR: open affiche les statistiques quand l'appelant ne connaît pas le contexte de difficulté actuel.
void StatisticsMenu::open(const Player& player)
{
    const bool difficultyKnown = false;
    DifficultyMode difficulty = DifficultyMode::Normal;

    while (true)
    {
        TerminalInterface::renderMenuScreen(buildHubScreen());

        int choice = Console::askNumberBetween(0, 6, "Choix invalide. Entre un chiffre entre 0 et 6.");
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
            player.displayStats();
            player.displayCareerStatistics(difficulty);
            std::cout << "Note : difficulté exacte non transmise par ce chemin de menu." << std::endl;
            std::cout << std::endl;
        }

        Console::waitForEnter();
        Console::clear();
    }
}

// EN: displaySummary prints a compact character overview.
// FR: displaySummary affiche un résumé compact du personnage.
void StatisticsMenu::displaySummary(const Player& player)
{
    MenuFrame::title("RÉSUMÉ DU PERSONNAGE");
    std::cout << "Nom : " << player.getName() << std::endl;
    std::cout << "Race : " << player.getRaceText() << std::endl;
    std::cout << "Classe : " << player.getType() << std::endl;
    std::cout << "Niveau : " << player.getLevel() << std::endl;

    int nextLevelExperience = Level::getExperienceRequiredForNextLevel(player.getLevel());
    if (nextLevelExperience > 0)
    {
        std::cout << "Expérience : " << player.getExperience() << "/" << nextLevelExperience << std::endl;
    }
    else
    {
        std::cout << "Expérience : niveau maximum" << std::endl;
    }

    std::cout << "PV : " << player.getHp() << "/" << player.getMaxHp() << std::endl;
    std::cout << "Or : " << player.getInventory().getGold() << std::endl;
    std::cout << "État : " << (player.isAlteredByCheats() ? "Altéré" : "Normal") << std::endl;
    std::cout << "Clone : " << (player.isClone() ? "oui" : "non") << std::endl;
    std::cout << std::endl;
}

// EN: displayCombatStats prints combat, boss, PvP and death counters.
// FR: displayCombatStats affiche les compteurs de combat, boss, JcJ et morts.
void StatisticsMenu::displayCombatStats(const Player& player, DifficultyMode difficulty, bool difficultyKnown)
{
    MenuFrame::title("STATISTIQUES DE COMBAT");
    std::cout << "Combats lancés : " << player.getCombatsStarted() << std::endl;
    std::cout << "Victoires : " << player.getVictories() << std::endl;
    std::cout << "Défaites : " << player.getDefeats() << std::endl;
    std::cout << "Fuites réussies : " << player.getEscapes() << std::endl;
    std::cout << "Ennemis vaincus : " << player.getEnemiesKilled() << std::endl;
    std::cout << "Boss vaincus : " << player.getBossesKilled() << std::endl;
    std::cout << "JcJ remportés : " << player.getPvpVictories() << std::endl;
    std::cout << "JcJ perdus : " << player.getPvpDefeats() << std::endl;

    if (difficultyKnown && difficulty == DifficultyMode::Lethal)
    {
        std::cout << "Morts du personnage : [STATISTIQUE CORROMPUE]" << std::endl;
        std::cout << "Vous ne deviez pas mourir." << std::endl;
    }
    else
    {
        std::cout << "Morts du personnage : " << player.getDeaths() << std::endl;

        if (!difficultyKnown)
        {
            std::cout << "Note : le contexte de difficulté exact n'est pas connu depuis ce sous-menu." << std::endl;
        }
    }

    const std::vector<std::string>& eliminations = player.getPvpLethalEliminations();
    if (!eliminations.empty())
    {
        std::cout << "Éliminations Léthal en JcJ :" << std::endl;
        for (const std::string& entry : eliminations)
        {
            std::cout << "- " << entry << std::endl;
        }
    }

    std::cout << std::endl;
}

// EN: displayEquipmentUsage prints currently equipped and recently used equipment information.
// FR: displayEquipmentUsage affiche l'équipement actuel et l'historique récent d'utilisation.
void StatisticsMenu::displayEquipmentUsage(const Player& player)
{
    player.displaySimpleEquipment();

    MenuFrame::title("ÉQUIPEMENT RÉCENT");
    const std::vector<std::string>& usage = player.getRecentCombatEquipmentUsage();

    if (usage.empty())
    {
        std::cout << "Aucun historique récent d'équipement." << std::endl;
        std::cout << std::endl;
        return;
    }

    std::map<std::string, int> counts;
    for (const std::string& itemName : usage)
    {
        counts[itemName]++;
    }

    for (const auto& pair : counts)
    {
        std::cout << "- " << pair.first << " : " << pair.second << " apparition(s) récente(s)" << std::endl;
    }

    std::cout << std::endl;
}

// EN: displaySkillStats prints unlocked skills and skill progress.
// FR: displaySkillStats affiche les compétences débloquées et leur progression.
void StatisticsMenu::displaySkillStats(const Player& player)
{
    MenuFrame::title("COMPÉTENCES");

    displaySkillList("Passives connues :", player.getUnlockedPassiveSkills());
    std::cout << std::endl;

    displaySkillList("Actives connues :", player.getUnlockedActiveSkills());
    std::cout << std::endl;

    std::cout << "Traces d'entraînement visibles :" << std::endl;
    displayProgressLine("Armes courtes / dagues", player.getDaggerKillProgress(), 5);
    displayProgressLine("Arcs et tirs", player.getBowKillProgress(), 8);
    displayProgressLine("Combat à mains nues", player.getBareHandKillProgress(), 10);
    displayProgressLine("Bâtons et canalisation", player.getStaffKillProgress(), 6);
    displayProgressLine("Épées et discipline", player.getSwordKillProgress(), 7);
    displayProgressLine("Haches et ouverture", player.getAxeKillProgress(), 7);
    displayProgressLine("Marteaux et fracture de garde", player.getHammerKillProgress(), 7);
    displayProgressLine("Lances et contrôle de distance", player.getSpearKillProgress(), 7);

    std::cout << std::endl;
    std::cout << "Ces traces montrent ce que ton personnage répète assez souvent pour l'intégrer à son style." << std::endl;
    std::cout << std::endl;
}

// EN: displaySpecialStates prints altered, clone and boss consequence states.
// FR: displaySpecialStates affiche les états altéré, clone et conséquences de boss.
void StatisticsMenu::displaySpecialStates(const Player& player)
{
    MenuFrame::title("ÉTATS SPÉCIAUX");
    std::cout << "Altéré : " << (player.isAlteredByCheats() ? "oui" : "non") << std::endl;
    std::cout << "Clone : " << (player.isClone() ? "oui" : "non") << std::endl;

    if (player.hasWorldGazePenalty())
    {
        std::cout << "Regard du monde : actif" << std::endl;
        std::cout << "Tentatives de rupture Léthal : " << player.getLethalCheatAttemptCount() << std::endl;
    }
    else if (player.getLethalCheatAttemptCount() > 0)
    {
        std::cout << "Trace interdite : ancienne tentative de cheat Léthal détectée" << std::endl;
        std::cout << "Tentatives : " << player.getLethalCheatAttemptCount() << std::endl;
    }

    if (player.hasZelefCorrosionPresent())
    {
        std::cout << "Corrosion présente : " << player.getZelefMaxHpStolen() << " PV max retenus" << std::endl;
    }

    if (player.hasGrinkaBossTheftPresent())
    {
        std::cout << "Volé par un boss : Grinka détient encore quelque chose." << std::endl;
    }

    if (player.hasBossEquipmentSeal())
    {
        std::cout << "Équipement scellé : " << player.getBossEquipmentSealReason() << std::endl;
    }

    std::cout << std::endl;
}
