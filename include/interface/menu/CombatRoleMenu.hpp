// EN: CombatRoleMenu.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRoleMenu.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Displays active combat role actions such as provocation, threat reduction, and future ally support skills.

#ifndef INCLUDE_INTERFACE_MENU_COMBATROLEMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATROLEMENU_HPP

#include "entity/Entity.hpp"

class CombatRoleMenu
{
public:
    // Returns true when the selected role action consumes the current turn.
    // EN: open declares or implements a focused behavior used by this module.
    // FR: open déclare ou implémente un comportement précis utilisé par ce module.
    static bool open(Entity& entity);
};

#endif
