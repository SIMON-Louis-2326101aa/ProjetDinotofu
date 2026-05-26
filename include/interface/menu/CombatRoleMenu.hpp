// EN: CombatRoleMenu.hpp exposes combat role actions through shared UI models.
// FR: CombatRoleMenu.hpp expose les actions de rôle de combat via les modèles d'interface partagés.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Displays active combat role actions such as provocation, threat reduction, and future ally support skills.

#ifndef INCLUDE_INTERFACE_MENU_COMBATROLEMENU_HPP
#define INCLUDE_INTERFACE_MENU_COMBATROLEMENU_HPP

#include "entity/Entity.hpp"
#include "interface/model/MenuScreen.hpp"

class CombatRoleMenu
{
public:
    static MenuScreen buildScreen(const Entity& entity);

    // Returns true when the selected role action consumes the current turn.
    static bool open(Entity& entity);
};

#endif
