// EN: Material.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Material.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements stackable material-like inventory entries.
// Français : Implémente les entrées d'inventaire empilables de type matériau.

#include "item/material/Material.hpp"

#include <cctype>
#include <iostream>
#include <string>


namespace
{
    std::string normalizeQuality(std::string quality)
    {
        for (char& c : quality)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        if (quality == "pure" || quality == "pur") return "pure";
        if (quality == "impure" || quality == "impur") return "impure";
        if (quality == "exceptional" || quality == "exceptionnel" || quality == "exceptionnelle") return "exceptional";
        if (quality == "high" || quality == "haute") return "high";
        if (quality == "low" || quality == "faible") return "low";
        return "normal";
    }

    // EN: startsWith declares or implements a focused behavior used by this module.
    // FR: startsWith déclare ou implémente un comportement précis utilisé par ce module.
    bool startsWith(const std::string& value, const std::string& prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    // EN: isRepairKitId declares or implements a focused behavior used by this module.
    // FR: isRepairKitId déclare ou implémente un comportement précis utilisé par ce module.
    bool isRepairKitId(const std::string& id)
    {
        return id == "weak_repair_kit"
            || id == "medium_repair_kit"
            || id == "big_repair_kit"
            || id == "tinkerer_complete_repair_kit"
            || startsWith(id, "weak_repair_kit_used_")
            || startsWith(id, "medium_repair_kit_used_")
            || startsWith(id, "big_repair_kit_used_")
            || startsWith(id, "tinkerer_complete_repair_kit_used_");
    }

    // EN: isUsedRepairKitId declares or implements a focused behavior used by this module.
    // FR: isUsedRepairKitId déclare ou implémente un comportement précis utilisé par ce module.
    bool isUsedRepairKitId(const std::string& id)
    {
        return startsWith(id, "weak_repair_kit_used_")
            || startsWith(id, "medium_repair_kit_used_")
            || startsWith(id, "big_repair_kit_used_")
            || startsWith(id, "tinkerer_complete_repair_kit_used_");
    }

    // EN: maxRepairKitDurability declares or implements a focused behavior used by this module.
    // FR: maxRepairKitDurability déclare ou implémente un comportement précis utilisé par ce module.
    int maxRepairKitDurability(const std::string& id)
    {
        if (startsWith(id, "medium_repair_kit")) return 3;
        if (startsWith(id, "big_repair_kit")) return 4;
        if (startsWith(id, "tinkerer_complete_repair_kit")) return 5;
        return 2;
    }

    // EN: usedRepairKitDurability declares or implements a focused behavior used by this module.
    // FR: usedRepairKitDurability déclare ou implémente un comportement précis utilisé par ce module.
    int usedRepairKitDurability(const std::string& id)
    {
        std::size_t pos = id.find_last_of('_');

        if (pos == std::string::npos || pos + 1 >= id.size())
        {
            return 1;
        }

        try
        {
            return std::stoi(id.substr(pos + 1));
        }
        catch (...)
        {
            return 1;
        }
    }
}

// EN: Material declares or implements a focused behavior used by this module.
// FR: Material déclare ou implémente un comportement précis utilisé par ce module.
Material::Material() : Item()
{
    id = "unknown_material";
    category = "Inconnu";
    quality = "normal";
    quantity = 0;
}

Material::Material(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    const std::string& category,
    int value,
    int quantity,
    const std::string& quality
// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
) : Item(name, description, value)
{
    this->id = id;
    this->category = category;
    setQuality(quality);
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

std::string Material::getQuality() const
{
    return quality;
}

std::string Material::getQualityLabel() const
{
    if (quality == "pure") return "Pur";
    if (quality == "impure") return "Impur";
    if (quality == "exceptional") return "Exceptionnel";
    if (quality == "high") return "Haute qualité";
    if (quality == "low") return "Faible qualité";
    return "Qualité normale";
}

// EN: getQualityPricePercent declares or implements a focused behavior used by this module.
// FR: getQualityPricePercent déclare ou implémente un comportement précis utilisé par ce module.
int Material::getQualityPricePercent() const
{
    if (quality == "exceptional") return 260;
    if (quality == "pure" || quality == "high") return 175;
    if (quality == "impure" || quality == "low") return 55;
    return 100;
}

// EN: getQualityCraftWeight declares or implements a focused behavior used by this module.
// FR: getQualityCraftWeight déclare ou implémente un comportement précis utilisé par ce module.
int Material::getQualityCraftWeight() const
{
    if (quality == "exceptional") return 4;
    if (quality == "pure" || quality == "high") return 3;
    if (quality == "impure" || quality == "low") return 1;
    return 2;
}

// EN: hasSpecialQuality declares or implements a focused behavior used by this module.
// FR: hasSpecialQuality déclare ou implémente un comportement précis utilisé par ce module.
bool Material::hasSpecialQuality() const
{
    return quality != "normal";
}

// EN: setQuality declares or implements a focused behavior used by this module.
// FR: setQuality déclare ou implémente un comportement précis utilisé par ce module.
void Material::setQuality(const std::string& quality)
{
    this->quality = normalizeQuality(quality);
}

// EN: getQuantity declares or implements a focused behavior used by this module.
// FR: getQuantity déclare ou implémente un comportement précis utilisé par ce module.
int Material::getQuantity() const
{
    return quantity;
}

// EN: addQuantity declares or implements a focused behavior used by this module.
// FR: addQuantity déclare ou implémente un comportement précis utilisé par ce module.
void Material::addQuantity(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    quantity += amount;
}

// EN: removeQuantity declares or implements a focused behavior used by this module.
// FR: removeQuantity déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: setQuantity declares or implements a focused behavior used by this module.
// FR: setQuantity déclare ou implémente un comportement précis utilisé par ce module.
void Material::setQuantity(int amount)
{
    if (amount < 0)
    {
        amount = 0;
    }

    quantity = amount;
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Material::display() const
{
    std::cout << "===== MATÉRIAU =====" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Catégorie : " << category << std::endl;
    if (category != "Livre" && category != "Renseignement" && category != "Outil")
    {
        std::cout << "Qualité : " << getQualityLabel() << std::endl;
    }
    if (category == "Outil" && isRepairKitId(id))
    {
        if (isUsedRepairKitId(id))
        {
            std::cout << "Nombre : " << quantity << " kit(s) entamé(s)" << std::endl;
            std::cout << "Durabilité par kit : " << usedRepairKitDurability(id)
                      << "/" << maxRepairKitDurability(id) << std::endl;
        }
        else
        {
            std::cout << "Nombre : " << quantity << " kit(s) intact(s)" << std::endl;
            std::cout << "Durabilité par kit : " << maxRepairKitDurability(id)
                      << "/" << maxRepairKitDurability(id) << std::endl;
        }
    }
    else
    {
        std::cout << "Quantité : " << quantity << std::endl;
    }
    std::cout << "Description : " << description << std::endl;
    std::cout << "Valeur unitaire : " << (value * getQualityPricePercent() / 100) << " pièces";
    if (hasSpecialQuality())
    {
        std::cout << " (base " << value << ")";
    }
    std::cout << std::endl;

    if (category == "Matériau de monstre")
    {
        std::cout << "Utilité prévue : revente, artisanat, trophées et recettes liées aux monstres." << std::endl;
    }
    else if (category == "Matériau")
    {
        std::cout << "Utilité prévue : réparation, amélioration et fabrication d'équipement." << std::endl;
    }
    else if (category == "Plante")
    {
        std::cout << "Utilité prévue : potions, remèdes, quêtes et secrets botaniques." << std::endl;
    }
    else if (category == "Outil")
    {
        std::cout << "Utilité prévue : accepter une réparation autonome. Un kit intact reste empilé ; un kit entamé est affiché séparément avec sa durabilité." << std::endl;
    }
    else if (category == "Renseignement" || category == "Livre")
    {
        std::cout << "Utilité prévue : débloquer ou compléter des informations du bestiaire." << std::endl;
    }
    std::cout << "====================" << std::endl;
    std::cout << std::endl;
}
