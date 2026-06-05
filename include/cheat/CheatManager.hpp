// EN: CheatManager.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CheatManager.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Recognizes, confirms and activates hidden cheat alterations on the current character.
// Description : Reconnaît, confirme et active les altérations cachées du personnage actuel.

#ifndef INCLUDE_CHEAT_CHEATMANAGER_HPP
#define INCLUDE_CHEAT_CHEATMANAGER_HPP

#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"

#include <string>

class CheatManager
{
private:
    // EN: normalizeCode declares or implements a focused behavior used by this module.
    // FR: normalizeCode déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeCode(const std::string& code);
    // EN: confirmFirstAlteration declares or implements a focused behavior used by this module.
    // FR: confirmFirstAlteration déclare ou implémente un comportement précis utilisé par ce module.
    static bool confirmFirstAlteration(Player& player);
    // EN: displayToggleResult declares or implements a focused behavior used by this module.
    // FR: displayToggleResult déclare ou implémente un comportement précis utilisé par ce module.
    static void displayToggleResult(const std::string& effectText, bool enabled);
    // EN: displayInstantResult declares or implements a focused behavior used by this module.
    // FR: displayInstantResult déclare ou implémente un comportement précis utilisé par ce module.
    static void displayInstantResult(const std::string& effectText);
    // EN: resetCharacter declares or implements a focused behavior used by this module.
    // FR: resetCharacter déclare ou implémente un comportement précis utilisé par ce module.
    static void resetCharacter(Player& player, DifficultyMode difficulty);
    // EN: switchClassAndStarterEquipment declares or implements a focused behavior used by this module.
    // FR: switchClassAndStarterEquipment déclare ou implémente un comportement précis utilisé par ce module.
    static void switchClassAndStarterEquipment(Player& player, DifficultyMode difficulty);
    // EN: displayKnownAlterations declares or implements a focused behavior used by this module.
    // FR: displayKnownAlterations déclare ou implémente un comportement précis utilisé par ce module.
    static void displayKnownAlterations(const Player& player);

public:
    // EN: openAlteredDataMenu declares or implements a focused behavior used by this module.
    // FR: openAlteredDataMenu déclare ou implémente un comportement précis utilisé par ce module.
    static void openAlteredDataMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule);
    // EN: tryActivateHiddenCode declares or implements a focused behavior used by this module.
    // FR: tryActivateHiddenCode déclare ou implémente un comportement précis utilisé par ce module.
    static bool tryActivateHiddenCode(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule, const std::string& code);
    static bool activateCode(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule, const std::string& code, bool displayUnknownMessage = true);
};

#endif
