// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Recognizes, confirms and activates hidden cheat alterations on the current character.
// Description : Reconnaît, confirme et active les altérations cachées du personnage actuel.

#ifndef INCLUDE_CHEAT_CHEATMANAGER_HPP
#define INCLUDE_CHEAT_CHEATMANAGER_HPP

#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>

class CheatManager
{
private:
    static std::string normalizeCode(const std::string& code);
    static bool confirmFirstAlteration(Player& player);
    static void displayToggleResult(const std::string& effectText, bool enabled);
    static void displayInstantResult(const std::string& effectText);
    static void resetCharacter(Player& player, DifficultyMode difficulty);
    static void switchClassAndStarterEquipment(Player& player, DifficultyMode difficulty);
    static void displayKnownAlterations(const Player& player);

public:
    static void openAlteredDataMenu(Player& player, DifficultyMode difficulty);
    static bool tryActivateHiddenCode(Player& player, DifficultyMode difficulty, const std::string& code);
    static bool activateCode(Player& player, DifficultyMode difficulty, const std::string& code, bool displayUnknownMessage = true);
};

#endif
