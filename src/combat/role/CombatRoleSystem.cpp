// EN: CombatRoleSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRoleSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
            // EN: [] declares or implements a focused behavior used by this module.
            // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }

    // EN: contains declares or implements a focused behavior used by this module.
    // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
    bool contains(const std::string& value, const std::string& searched)
    {
        return value.find(searched) != std::string::npos;
    }
}

// EN: getRole declares or implements a focused behavior used by this module.
// FR: getRole déclare ou implémente un comportement précis utilisé par ce module.
CombatRole CombatRoleSystem::getRole(const Entity& entity)
{
    std::string name = toLower(entity.getName());
    std::string type = toLower(entity.getType());

    if (name == "sanctus"
        || name == "mattzelda"
        || name == "henrique"
        || contains(type, "paladin")
        || contains(type, "colosse")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "chevalier")
        || contains(type, "gardien")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "tank")
        || contains(type, "protecteur"))
    {
        return CombatRole::Tank;
    }

    if (name == "hestia"
        || contains(type, "pretre")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "prêtre")
        || contains(type, "healer")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "soigneur")
        || contains(name, "shaman")
        || contains(name, "chamane")
        || contains(name, "oracle")
        || contains(type, "médecin")
        || contains(type, "medecin"))
    {
        return CombatRole::Healer;
    }

    if (name == "aoi"
        || name == "fail"
        || contains(type, "invocateur")
        || contains(type, "nécromancien")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "necromancien")
        || contains(type, "pactisant")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "dompteur")
        || contains(type, "marionnettiste")
        || contains(type, "totémiste")
        || contains(type, "totemiste")
        || contains(type, "corbeaumancien"))
    {
        return CombatRole::Summoner;
    }

    if (name == "hazak"
        || name == "skuro"
        || name == "trexof"
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "assassin")
        || contains(type, "ombrelame"))
    {
        return CombatRole::Assassin;
    }

    if (name == "fire flight"
        || name == "louis"
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "barde")
        || contains(type, "support")
        || contains(type, "soutien")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "alchimiste")
        || contains(type, "cuisinier"))
    {
        return CombatRole::Support;
    }

    if (name == "kanadé"
        || name == "kanade"
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "hybride")
        || contains(type, "mage-lame")
        // EN: contains declares or implements a focused behavior used by this module.
        // FR: contains déclare ou implémente un comportement précis utilisé par ce module.
        || contains(type, "artificier")
        || contains(type, "chevalier draconique")
        || contains(type, "rôdeur arcanique")
        || contains(type, "rodeur arcanique")
        || contains(type, "moine solaire")
        || contains(type, "cartographe")
        || contains(type, "récupérateur")
        || contains(type, "recuperateur"))
    {
        return CombatRole::Hybrid;
    }

    return CombatRole::DamageDealer;
}

// EN: isTank declares or implements a focused behavior used by this module.
// FR: isTank déclare ou implémente un comportement précis utilisé par ce module.
bool CombatRoleSystem::isTank(const Entity& entity)
{
    return getRole(entity) == CombatRole::Tank;
}

// EN: isHealer declares or implements a focused behavior used by this module.
// FR: isHealer déclare ou implémente un comportement précis utilisé par ce module.
bool CombatRoleSystem::isHealer(const Entity& entity)
{
    return getRole(entity) == CombatRole::Healer;
}

// EN: isSummoner declares or implements a focused behavior used by this module.
// FR: isSummoner déclare ou implémente un comportement précis utilisé par ce module.
bool CombatRoleSystem::isSummoner(const Entity& entity)
{
    return getRole(entity) == CombatRole::Summoner;
}

// EN: isAssassin declares or implements a focused behavior used by this module.
// FR: isAssassin déclare ou implémente un comportement précis utilisé par ce module.
bool CombatRoleSystem::isAssassin(const Entity& entity)
{
    return getRole(entity) == CombatRole::Assassin;
}


// EN: isSupport declares or implements a focused behavior used by this module.
// FR: isSupport déclare ou implémente un comportement précis utilisé par ce module.
bool CombatRoleSystem::isSupport(const Entity& entity)
{
    return getRole(entity) == CombatRole::Support;
}

// EN: isHybrid declares or implements a focused behavior used by this module.
// FR: isHybrid déclare ou implémente un comportement précis utilisé par ce module.
bool CombatRoleSystem::isHybrid(const Entity& entity)
{
    return getRole(entity) == CombatRole::Hybrid;
}
