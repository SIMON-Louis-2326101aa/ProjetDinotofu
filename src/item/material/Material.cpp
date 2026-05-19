// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements stackable material-like inventory entries.
// Français : Implémente les entrées d'inventaire empilables de type matériau.

#include "item/material/Material.hpp"

#include <iostream>

Material::Material() : Item()
{
    id = "unknown_material";
    category = "Inconnu";
    quantity = 0;
}

Material::Material(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    const std::string& category,
    int value,
    int quantity
) : Item(name, description, value)
{
    this->id = id;
    this->category = category;
    setQuantity(quantity);
}

std::string Material::getId() const
{
    return id;
}

std::string Material::getCategory() const
{
    return category;
}

int Material::getQuantity() const
{
    return quantity;
}

void Material::addQuantity(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    quantity += amount;
}

bool Material::removeQuantity(int amount)
{
    if (amount <= 0)
    {
        return true;
    }

    if (quantity < amount)
    {
        return false;
    }

    quantity -= amount;
    return true;
}

void Material::setQuantity(int amount)
{
    if (amount < 0)
    {
        amount = 0;
    }

    quantity = amount;
}

void Material::display() const
{
    std::cout << "===== MATÉRIAU =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Catégorie : " << category << std::endl;
    std::cout << "Quantité : " << quantity << std::endl;
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur unitaire : " << value << " pièces" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << std::endl;
}
