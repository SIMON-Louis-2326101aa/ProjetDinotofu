// EN: Item.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Item.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_ITEM_HPP
#define INCLUDE_ITEM_ITEM_HPP

#include <string>

class Item
{
protected:
    std::string name;
    std::string description;
    int value;

public:
    // EN: Item declares or implements a focused behavior used by this module.
    // FR: Item déclare ou implémente un comportement précis utilisé par ce module.
    Item();

    Item(
        const std::string& name,
        const std::string& description,
        int value
    );

    virtual ~Item() = default;

    std::string getName() const;
    std::string getDescription() const;
    // EN: getValue declares or implements a focused behavior used by this module.
    // FR: getValue déclare ou implémente un comportement précis utilisé par ce module.
    int getValue() const;

    // EN: display declares or implements a focused behavior used by this module.
    // FR: display déclare ou implémente un comportement précis utilisé par ce module.
    virtual void display() const;
};

#endif
