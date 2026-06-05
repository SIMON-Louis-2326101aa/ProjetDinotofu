// EN: Material.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Material.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements stackable material-like inventory entries.
// Français : Implémente les entrées d'inventaire empilables de type matériau.

#include "item/material/Material.hpp"

#include <algorithm>
#include <cctype>
#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
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

    std::string lowerMaterialText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool materialUsesPurityVocabulary(const std::string& id, const std::string& name, const std::string& category)
    {
        const std::string probe = lowerMaterialText(id + " " + name + " " + category);

        return probe.find("minerai") != std::string::npos
            || probe.find("metal") != std::string::npos
            || probe.find("métal") != std::string::npos
            || probe.find("fragment de métal") != std::string::npos
            || probe.find("poussi") != std::string::npos
            || probe.find("résidu") != std::string::npos
            || probe.find("residu") != std::string::npos
            || probe.find("slime") != std::string::npos
            || probe.find("cristal") != std::string::npos
            || probe.find("arcane") != std::string::npos;
    }

    bool isIntrinsicallyDamagedMaterial(const std::string& id, const std::string& name, const std::string& category)
    {
        const std::string probe = lowerMaterialText(id + " " + name + " " + category);

        if (probe.find("fragment de boss") != std::string::npos)
        {
            return false;
        }

        return id == "rusted_metal_fragment"
            || id == "worn_leather_piece"
            || id == "battle_torn_badge"
            || id == "cracked_bone"
            || probe.find("abîmé") != std::string::npos
            || probe.find("abime") != std::string::npos
            || probe.find("rouillé") != std::string::npos
            || probe.find("rouille") != std::string::npos
            || probe.find("fissuré") != std::string::npos
            || probe.find("fissure") != std::string::npos
            || probe.find("cassé") != std::string::npos
            || probe.find("casse") != std::string::npos;
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
    if (quality == "pure")
    {
        return materialUsesPurityVocabulary(id, getName(), category) ? "Pur" : "Haute qualité";
    }

    if (quality == "impure")
    {
        return materialUsesPurityVocabulary(id, getName(), category) ? "Impur" : "Qualité faible";
    }

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
    std::string normalized = normalizeQuality(quality);

    if (isIntrinsicallyDamagedMaterial(id, getName(), category)
        && (normalized == "exceptional" || normalized == "pure" || normalized == "high"))
    {
        // FR: Un composant déjà abîmé/rouillé/fissuré ne peut pas être vendu ou looté comme haute qualité.
        // EN: Already damaged/rusted/cracked components cannot logically become high or exceptional quality.
        normalized = "normal";
    }

    this->quality = normalized;
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
std::vector<std::string> Material::toDisplayLines() const
{
    std::vector<std::string> lines = {
        "===== MATÉRIAU =====",
        "Nom : " + name,
        "Catégorie : " + category
    };

    if (category != "Livre" && category != "Renseignement" && category != "Outil")
    {
        lines.push_back("Qualité : " + getQualityLabel());
    }

    if (category == "Outil" && isRepairKitId(id))
    {
        if (isUsedRepairKitId(id))
        {
            lines.push_back("Nombre : " + std::to_string(quantity) + " kit(s) entamé(s)");
            lines.push_back(
                "Durabilité par kit : "
                + std::to_string(usedRepairKitDurability(id))
                + "/"
                + std::to_string(maxRepairKitDurability(id))
            );
        }
        else
        {
            lines.push_back("Nombre : " + std::to_string(quantity) + " kit(s) intact(s)");
            lines.push_back(
                "Durabilité par kit : "
                + std::to_string(maxRepairKitDurability(id))
                + "/"
                + std::to_string(maxRepairKitDurability(id))
            );
        }
    }
    else
    {
        lines.push_back("Quantité : " + std::to_string(quantity));
    }

    std::string valueLine = "Valeur unitaire : " + std::to_string(value * getQualityPricePercent() / 100) + " pièces";
    if (hasSpecialQuality())
    {
        valueLine += " (base " + std::to_string(value) + ")";
    }

    lines.push_back("Description : " + description);
    lines.push_back(valueLine);

    if (category == "Matériau de monstre")
    {
        lines.push_back("Utilité connue : revente, artisanat, trophées et recettes liées aux monstres.");
    }
    else if (category == "Matériau")
    {
        lines.push_back("Utilité connue : réparation, amélioration et fabrication d'équipement.");
    }
    else if (category == "Plante")
    {
        lines.push_back("Utilité connue : potions, remèdes, quêtes et secrets botaniques.");
    }
    else if (category == "Outil")
    {
        lines.push_back("Utilité connue : réparation autonome. Un kit intact reste empilé ; un kit entamé garde la marque de son usure.");
    }
    else if (category == "Renseignement" || category == "Livre")
    {
        lines.push_back("Utilité connue : débloquer ou compléter des informations du bestiaire.");
    }

    lines.push_back("====================");

    return lines;
}


// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Material::display() const
{
    MessageScreen::show("MATÉRIAU / INFO", "item.material.display", toDisplayLines(), false);
}
