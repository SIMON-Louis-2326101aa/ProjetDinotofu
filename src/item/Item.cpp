// EN: Item.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Item.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Item.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>
#include <atomic>
#include <chrono>

// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
Item::Item()
{
    name = "Objet inconnu";
    description = "Un objet mystérieux dont l'utilité reste floue.";
    value = 0;
    persistentId.clear();
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
    persistentId.clear();
}

std::string Item::getName() const
{
    return name;
}

std::string Item::getDescription() const
{
    return description;
}

// EN: getValue declares or implements a focused behavior used by this module.
// FR: getValue déclare ou implémente un comportement précis utilisé par ce module.
int Item::getValue() const
{
    return value;
}


const std::string& Item::getPersistentId() const
{
    return persistentId;
}

void Item::setPersistentId(const std::string& id)
{
    persistentId = id;
}

void Item::ensurePersistentId(const std::string& prefix)
{
    if (!persistentId.empty())
    {
        return;
    }

    static std::atomic<unsigned long long> counter{1};
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const unsigned long long serial = counter.fetch_add(1);
    persistentId = prefix + "-" + std::to_string(static_cast<unsigned long long>(now)) + "-" + std::to_string(serial);
}

std::vector<std::string> Item::toDisplayLines() const
{
    return {
        "===== OBJET =====",
        "Nom : " + name,
        "Description : " + description,
        "Valeur : " + std::to_string(value) + " pièces",
        "================="
    };
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Item::display() const
{
    MessageScreen::show("OBJET", "item.display", toDisplayLines(), false);
}
