// EN: Item.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Item.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Item.hpp"

#include "economy/Money.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <sstream>

// EN: Item declares or implements a focused behavior used by this module.
// FR: Item déclare ou implémente un comportement précis utilisé par ce module.
Item::Item()
{
    name = "Objet inconnu";
    description = "Un objet mystérieux dont l'utilité reste floue.";
    value = 0;
    persistentId.clear();
    ownershipBinding = ItemOwnershipBinding::None;
    uniqueRecoveryItem = false;
    questRecoveryItem = false;
    personalRecoveryItem = false;
    recoveryPriority = 0;
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
    ownershipBinding = ItemOwnershipBinding::None;
    uniqueRecoveryItem = false;
    questRecoveryItem = false;
    personalRecoveryItem = false;
    recoveryPriority = 0;
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


ItemOwnershipBinding Item::getOwnershipBinding() const
{
    return ownershipBinding;
}

void Item::setOwnershipBinding(ItemOwnershipBinding binding)
{
    ownershipBinding = binding;
}

void Item::setSoulBound(bool soulBound)
{
    ownershipBinding = soulBound ? ItemOwnershipBinding::SoulBound : ItemOwnershipBinding::None;
}

bool Item::isSoulBound() const
{
    return ownershipBinding == ItemOwnershipBinding::SoulBound;
}

bool Item::canChangeOwner() const
{
    return ownershipBinding == ItemOwnershipBinding::None;
}

std::string Item::getOwnershipBindingSaveText() const
{
    return isSoulBound() ? "soul_bound" : "none";
}

void Item::loadOwnershipBindingFromSaveText(const std::string& saveText)
{
    ownershipBinding = saveText == "soul_bound" ? ItemOwnershipBinding::SoulBound : ItemOwnershipBinding::None;
}

std::string Item::getOwnershipBindingDisplayText() const
{
    return isSoulBound() ? "Lié à l'âme — changement de propriétaire impossible" : "Transférable";
}

void Item::setUniqueRecoveryItem(bool unique)
{
    uniqueRecoveryItem = unique;
}

void Item::setQuestRecoveryItem(bool quest)
{
    questRecoveryItem = quest;
}

void Item::setPersonalRecoveryItem(bool personal)
{
    personalRecoveryItem = personal;
}

bool Item::isUniqueRecoveryItem() const
{
    return uniqueRecoveryItem;
}

bool Item::isQuestRecoveryItem() const
{
    return questRecoveryItem;
}

bool Item::isPersonalRecoveryItem() const
{
    return personalRecoveryItem;
}

void Item::setRecoveryPriority(int priority)
{
    recoveryPriority = std::clamp(priority, 0, 100);
}

int Item::getRecoveryPriority() const
{
    return recoveryPriority;
}

int Item::getAutomaticRecoveryPriorityScore() const
{
    int score = std::clamp(recoveryPriority, 0, 100);
    if (uniqueRecoveryItem) score = std::max(score, 90);
    if (personalRecoveryItem) score = std::max(score, 95);
    if (questRecoveryItem) score = 100;
    return score;
}

bool Item::isProtectedFromAutomaticAbandon() const
{
    return uniqueRecoveryItem || questRecoveryItem || personalRecoveryItem;
}

bool Item::hasSameRecoveryMetadata(const Item& other) const
{
    return uniqueRecoveryItem == other.uniqueRecoveryItem
        && questRecoveryItem == other.questRecoveryItem
        && personalRecoveryItem == other.personalRecoveryItem
        && recoveryPriority == other.recoveryPriority;
}

std::string Item::getRecoveryTraitsSaveText() const
{
    std::string result;
    auto append = [&](const std::string& tag) {
        if (!result.empty()) result += ",";
        result += tag;
    };
    if (uniqueRecoveryItem) append("unique");
    if (questRecoveryItem) append("quest");
    if (personalRecoveryItem) append("personal");
    return result;
}

void Item::loadRecoveryMetadataFromSaveText(const std::string& traitsText, int priority)
{
    uniqueRecoveryItem = false;
    questRecoveryItem = false;
    personalRecoveryItem = false;
    std::istringstream stream(traitsText);
    std::string token;
    while (std::getline(stream, token, ','))
    {
        if (token == "unique") uniqueRecoveryItem = true;
        else if (token == "quest") questRecoveryItem = true;
        else if (token == "personal") personalRecoveryItem = true;
    }
    setRecoveryPriority(priority);
}

std::string Item::getRecoveryMetadataDisplayText() const
{
    std::vector<std::string> labels;
    if (questRecoveryItem) labels.push_back("Quête");
    if (personalRecoveryItem) labels.push_back("Personnel");
    if (uniqueRecoveryItem) labels.push_back("Unique");
    if (labels.empty() && recoveryPriority <= 0) return "Ordinaire";

    std::string text;
    for (std::size_t i = 0; i < labels.size(); ++i)
    {
        if (i > 0) text += " / ";
        text += labels[i];
    }
    if (text.empty()) text = "Priorité spéciale";
    if (recoveryPriority > 0) text += " — priorité " + std::to_string(recoveryPriority) + "/100";
    if (isProtectedFromAutomaticAbandon()) text += " — protégé du tri automatique";
    return text;
}

std::vector<std::string> Item::toDisplayLines() const
{
    return {
        "===== OBJET =====",
        "Nom : " + name,
        "Description : " + description,
        "Valeur : " + Money::formatEconomyUnits(value),
        "Propriété : " + getOwnershipBindingDisplayText(),
        "Récupération : " + getRecoveryMetadataDisplayText(),
        "================="
    };
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Item::display() const
{
    MessageScreen::show("OBJET", "item.display", toDisplayLines(), false);
}
