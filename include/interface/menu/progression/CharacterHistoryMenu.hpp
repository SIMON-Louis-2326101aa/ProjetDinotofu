// EN: CharacterHistoryMenu exposes durable character events and canonical activity records.
// FR: CharacterHistoryMenu expose les événements durables et l'activité canonique du personnage.
#ifndef INCLUDE_INTERFACE_MENU_PROGRESSION_CHARACTERHISTORYMENU_HPP
#define INCLUDE_INTERFACE_MENU_PROGRESSION_CHARACTERHISTORYMENU_HPP

class Player;

class CharacterHistoryMenu
{
public:
    static void open(const Player& player);
};

#endif
