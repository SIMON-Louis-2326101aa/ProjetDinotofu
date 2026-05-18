// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Item.hpp"

#include <iostream>

Item::Item()
{
    name = "Objet inconnu";
    description = "Un objet mystérieux dont l'utilité reste floue.";
    value = 0;
}

Item::Item(
    const std::string& name,
    const std::string& description,
    int value
)
{
    this->name = name;
    this->description = description;
    this->value = value;
}

std::string Item::getName() const
{
    return name;
}

std::string Item::getDescription() const
{
    return description;
}

int Item::getValue() const
{
    return value;
}

void Item::display() const
{
    std::cout << "===== OBJET =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur : " << value << " pièces" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << std::endl;
}