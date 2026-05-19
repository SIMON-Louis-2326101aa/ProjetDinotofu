// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Prepares transformation rules for special characters, especially Sanctus becoming Skuro.
// TODO: Later, call these rules from the damage system when an entity receives or deals a huge hit.

#ifndef INCLUDE_CHARACTER_SPECIALCHARACTERTRANSFORMATIONRULES_HPP
#define INCLUDE_CHARACTER_SPECIALCHARACTERTRANSFORMATIONRULES_HPP

#include <string>

class SpecialCharacterTransformationRules
{
public:
    static bool canTransformIntoSkuro(const std::string& characterName);
    static bool shouldSanctusTransformAfterDamageTaken(int damageTaken, int maxHp);
    static bool shouldSanctusTransformAfterDamageDealt(int damageDealt, int targetMaxHp);
    static void displaySanctusToSkuroForeshadowing();
};

#endif
