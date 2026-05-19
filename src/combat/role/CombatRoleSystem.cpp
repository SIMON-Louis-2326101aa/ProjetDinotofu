// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Infers a simple combat role from an entity class/type for future targeting and AI rules.

#include "combat/role/CombatRoleSystem.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string toLower(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }

    bool contains(const std::string& value, const std::string& searched)
    {
        return value.find(searched) != std::string::npos;
    }
}

CombatRole CombatRoleSystem::getRole(const Entity& entity)
{
    std::string name = toLower(entity.getName());
    std::string type = toLower(entity.getType());

    if (name == "sanctus"
        || contains(type, "paladin")
        || contains(type, "colosse")
        || contains(type, "chevalier")
        || contains(type, "gardien")
        || contains(type, "tank"))
    {
        return CombatRole::Tank;
    }

    if (contains(type, "pretre")
        || contains(type, "prêtre")
        || contains(type, "healer")
        || contains(type, "soigneur"))
    {
        return CombatRole::Healer;
    }

    if (contains(type, "invocateur")
        || contains(type, "nécromancien")
        || contains(type, "necromancien")
        || contains(type, "pactisant")
        || contains(type, "dompteur"))
    {
        return CombatRole::Summoner;
    }

    if (name == "hazak"
        || contains(type, "assassin")
        || contains(type, "ombrelame"))
    {
        return CombatRole::Assassin;
    }

    if (contains(type, "barde")
        || contains(type, "support")
        || contains(type, "alchimiste"))
    {
        return CombatRole::Support;
    }

    if (contains(type, "hybride")
        || contains(type, "mage-lame")
        || contains(type, "artificier"))
    {
        return CombatRole::Hybrid;
    }

    return CombatRole::DamageDealer;
}

bool CombatRoleSystem::isTank(const Entity& entity)
{
    return getRole(entity) == CombatRole::Tank;
}

bool CombatRoleSystem::isHealer(const Entity& entity)
{
    return getRole(entity) == CombatRole::Healer;
}

bool CombatRoleSystem::isSummoner(const Entity& entity)
{
    return getRole(entity) == CombatRole::Summoner;
}

bool CombatRoleSystem::isAssassin(const Entity& entity)
{
    return getRole(entity) == CombatRole::Assassin;
}
