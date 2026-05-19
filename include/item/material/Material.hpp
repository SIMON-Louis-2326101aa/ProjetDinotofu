// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Represents a stackable material, plant, monster component, book or information item.
// Français : Représente un matériau, une plante, un composant de monstre, un livre ou une information empilable.

#ifndef INCLUDE_ITEM_MATERIAL_MATERIAL_HPP
#define INCLUDE_ITEM_MATERIAL_MATERIAL_HPP

#include "item/Item.hpp"

#include <string>

class Material : public Item
{
private:
    std::string id;
    std::string category;
    int quantity;

public:
    Material();

    Material(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        const std::string& category,
        int value,
        int quantity = 1
    );

    std::string getId() const;
    std::string getCategory() const;
    int getQuantity() const;

    void addQuantity(int amount);
    bool removeQuantity(int amount);
    void setQuantity(int amount);

    void display() const override;
};

#endif
