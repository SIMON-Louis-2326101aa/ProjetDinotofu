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
    Item();

    Item(
        const std::string& name,
        const std::string& description,
        int value
    );

    virtual ~Item() = default;

    std::string getName() const;
    std::string getDescription() const;
    int getValue() const;

    virtual void display() const;
};

#endif
