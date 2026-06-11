// EN: StatisticsMenu.hpp declares the terminal statistics menu for player summaries and detailed progress.
// FR: StatisticsMenu.hpp déclare le menu terminal de statistiques pour les résumés joueur et la progression détaillée.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MENU_PROGRESSION_STATISTICSMENU_HPP
#define INCLUDE_INTERFACE_MENU_PROGRESSION_STATISTICSMENU_HPP

#include "progression/DifficultyMode.hpp"
#include "interface/model/MenuScreen.hpp"

class Player;

class StatisticsMenu
{
public:
    static MenuScreen buildHubScreen();
    // EN: open displays the statistics hub and returns to the caller without consuming combat turns.
    // FR: open affiche le menu central des statistiques et revient à l'appelant sans consommer de tour.
    static void open(Player& player, DifficultyMode difficulty);

    // EN: open displays statistics when the caller does not know the current difficulty context.
    // FR: open affiche les statistiques quand l'appelant ne connaît pas le contexte de difficulté actuel.
    static void open(Player& player);

private:
    // EN: displaySummary prints a compact character overview.
    // FR: displaySummary affiche un résumé compact du personnage.
    static void displaySummary(const Player& player);

    // EN: displayCombatStats prints combat, boss, PvP and death counters.
    // FR: displayCombatStats affiche les compteurs de combat, boss, JcJ et morts.
    static void displayCombatStats(const Player& player, DifficultyMode difficulty, bool difficultyKnown);

    // EN: displayEquipmentUsage prints currently equipped and recently used equipment information.
    // FR: displayEquipmentUsage affiche l'équipement actuel et l'historique récent d'utilisation.
    static void displayEquipmentUsage(const Player& player);

    // EN: displaySkillStats prints unlocked skills and skill progress.
    // FR: displaySkillStats affiche les compétences débloquées et leur progression.
    static void displaySkillStats(const Player& player);

    // EN: displayTitleCatalog prints owned and unlockable character titles.
    // FR: displayTitleCatalog affiche les titres possédés et les titres déblocables.
    static void displayTitleCatalog(Player& player);

    // EN: displayTopThreeStats prints persistent Top 3 counters from the canonical engine journal.
    // FR: displayTopThreeStats affiche les Top 3 persistants issus du journal moteur canonique.
    static void displayTopThreeStats(const Player& player);
    static void displayTitleEquipMenu(Player& player);

    // EN: displaySpecialStates prints altered, clone and boss consequence states.
    // FR: displaySpecialStates affiche les états altéré, clone et conséquences de boss.
    static void displaySpecialStates(const Player& player);
};

#endif
