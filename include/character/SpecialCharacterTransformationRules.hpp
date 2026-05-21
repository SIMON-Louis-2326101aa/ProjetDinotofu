// EN: SpecialCharacterTransformationRules.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterTransformationRules.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: canTransformIntoSkuro declares or implements a focused behavior used by this module.
    // FR: canTransformIntoSkuro déclare ou implémente un comportement précis utilisé par ce module.
    static bool canTransformIntoSkuro(const std::string& characterName);
    // EN: shouldSanctusTransformAfterDamageTaken declares or implements a focused behavior used by this module.
    // FR: shouldSanctusTransformAfterDamageTaken déclare ou implémente un comportement précis utilisé par ce module.
    static bool shouldSanctusTransformAfterDamageTaken(int damageTaken, int maxHp);
    // EN: shouldSanctusTransformAfterDamageDealt declares or implements a focused behavior used by this module.
    // FR: shouldSanctusTransformAfterDamageDealt déclare ou implémente un comportement précis utilisé par ce module.
    static bool shouldSanctusTransformAfterDamageDealt(int damageDealt, int targetMaxHp);
    // EN: displaySanctusToSkuroForeshadowing declares or implements a focused behavior used by this module.
    // FR: displaySanctusToSkuroForeshadowing déclare ou implémente un comportement précis utilisé par ce module.
    static void displaySanctusToSkuroForeshadowing();
};

#endif
