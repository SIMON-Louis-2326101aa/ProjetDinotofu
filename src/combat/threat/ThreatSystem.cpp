// EN: ThreatSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ThreatSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Handles simple threat rules such as ally healers becoming priority targets and tanks using provocation.

#include "combat/threat/ThreatSystem.hpp"

#include "combat/role/CombatRoleSystem.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

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
}

// EN: markAllyHealingAction declares or implements a focused behavior used by this module.
// FR: markAllyHealingAction déclare ou implémente un comportement précis utilisé par ce module.
void ThreatSystem::markAllyHealingAction(Entity& healer, const Entity& healedAlly)
{
    healer.markHealingThreat();

    std::cout << "Menace : " << healer.getName()
              << " vient de soigner "
              << healedAlly.getName()
              << ". Les ennemis attentifs risquent de viser le soigneur en priorité."
              << std::endl;
    std::cout << std::endl;
}

// EN: markSelfHealingAction declares or implements a focused behavior used by this module.
// FR: markSelfHealingAction déclare ou implémente un comportement précis utilisé par ce module.
void ThreatSystem::markSelfHealingAction(const Entity& healer)
{
    std::cout << healer.getName()
              << " se soigne lui-même. La menace ennemie ne change pas vraiment."
              << std::endl;
    std::cout << std::endl;
}

// EN: tryActivatePassiveProvocation declares or implements a focused behavior used by this module.
// FR: tryActivatePassiveProvocation déclare ou implémente un comportement précis utilisé par ce module.
void ThreatSystem::tryActivatePassiveProvocation(Entity& entity, Random& random)
{
    if (!isNaturalProvoker(entity))
    {
        return;
    }

    if (entity.isProvoking())
    {
        return;
    }

    int chance = 35;

    if (toLower(entity.getName()) == "sanctus")
    {
        chance = 55;
    }

    if (random.between(1, 100) > chance)
    {
        return;
    }

    entity.startProvocation(2);

    if (toLower(entity.getName()) == "sanctus")
    {
        std::cout << "Sanctus lève sa garde sacrée." << std::endl;
        std::cout << "Provocation : les ennemis sentent qu'ils devront passer par lui." << std::endl;
    }
    else
    {
        std::cout << entity.getName()
                  << " prend l'avant de la ligne et attire l'attention ennemie."
                  << std::endl;
        std::cout << "Provocation : le ciblage ennemi est perturbé." << std::endl;
    }

    std::cout << std::endl;
}

bool ThreatSystem::shouldForceTargetMainEntity(
    const Entity& mainTarget,
    const std::string& attackerName
)
{
    (void) attackerName;

    return mainTarget.isProvoking() || mainTarget.hasHealingThreat();
}

void ThreatSystem::notifyForcedTarget(
    const Entity& target,
    const std::string& attackerName
)
{
    if (target.isProvoking())
    {
        std::cout << attackerName
                  << " tente de chercher une cible plus facile, mais la provocation de "
                  << target.getName()
                  << " l'oblige à rester focalisé."
                  << std::endl;
        std::cout << std::endl;
        return;
    }

    if (target.hasHealingThreat())
    {
        std::cout << attackerName
                  << " a vu "
                  << target.getName()
                  << " soigner un allié. Sa prochaine attaque se dirige vers le soigneur."
                  << std::endl;
        std::cout << std::endl;
    }
}

// EN: consumeForcedTargetIfNeeded declares or implements a focused behavior used by this module.
// FR: consumeForcedTargetIfNeeded déclare ou implémente un comportement précis utilisé par ce module.
void ThreatSystem::consumeForcedTargetIfNeeded(Entity& target)
{
    if (target.hasHealingThreat())
    {
        target.clearHealingThreat();
    }

    if (target.isProvoking())
    {
        target.decreaseProvocationTurn();
    }
}

// EN: isNaturalProvoker declares or implements a focused behavior used by this module.
// FR: isNaturalProvoker déclare ou implémente un comportement précis utilisé par ce module.
bool ThreatSystem::isNaturalProvoker(const Entity& entity)
{
    std::string name = toLower(entity.getName());
    std::string type = toLower(entity.getType());

    return name == "sanctus"
        || CombatRoleSystem::isTank(entity);
}
