// EN: Material.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Material.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    std::string quality;
    int quantity;

public:
    // EN: Material declares or implements a focused behavior used by this module.
    // FR: Material déclare ou implémente un comportement précis utilisé par ce module.
    Material();

    Material(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        const std::string& category,
        int value,
        int quantity = 1,
        const std::string& quality = "normal"
    );

    std::string getId() const;
    std::string getCategory() const;
    std::string getQuality() const;
    std::string getQualityLabel() const;
    // EN: getQualityPricePercent declares or implements a focused behavior used by this module.
    // FR: getQualityPricePercent déclare ou implémente un comportement précis utilisé par ce module.
    int getQualityPricePercent() const;
    // EN: getQualityCraftWeight declares or implements a focused behavior used by this module.
    // FR: getQualityCraftWeight déclare ou implémente un comportement précis utilisé par ce module.
    int getQualityCraftWeight() const;
    // EN: hasSpecialQuality declares or implements a focused behavior used by this module.
    // FR: hasSpecialQuality déclare ou implémente un comportement précis utilisé par ce module.
    bool hasSpecialQuality() const;
    // EN: getQuantity declares or implements a focused behavior used by this module.
    // FR: getQuantity déclare ou implémente un comportement précis utilisé par ce module.
    int getQuantity() const;

    // EN: setQuality declares or implements a focused behavior used by this module.
    // FR: setQuality déclare ou implémente un comportement précis utilisé par ce module.
    void setQuality(const std::string& quality);
    // EN: addQuantity declares or implements a focused behavior used by this module.
    // FR: addQuantity déclare ou implémente un comportement précis utilisé par ce module.
    void addQuantity(int amount);
    // EN: removeQuantity declares or implements a focused behavior used by this module.
    // FR: removeQuantity déclare ou implémente un comportement précis utilisé par ce module.
    bool removeQuantity(int amount);
    // EN: setQuantity declares or implements a focused behavior used by this module.
    // FR: setQuantity déclare ou implémente un comportement précis utilisé par ce module.
    void setQuantity(int amount);

    // EN: display declares or implements a focused behavior used by this module.
    // FR: display déclare ou implémente un comportement précis utilisé par ce module.
    void display() const override;
};

#endif
