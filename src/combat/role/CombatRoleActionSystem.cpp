// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Applies first active role behaviours such as automatic tank provocation and future healer/support hooks.

#include "combat/role/CombatRoleActionSystem.hpp"

#include "combat/role/CombatRoleSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include <iostream>

void CombatRoleActionSystem::tryActivateAutomaticRoleReaction(
    Entity& entity,
    Random& random
)
{
    if (CombatRoleSystem::isTank(entity))
    {
        ThreatSystem::tryActivatePassiveProvocation(entity, random);
        return;
    }

    if (CombatRoleSystem::isAssassin(entity) && entity.hasHealingThreat())
    {
        if (random.between(1, 100) <= 35)
        {
            entity.clearHealingThreat();

            std::cout << entity.getName()
                      << " disparaît assez longtemps du regard ennemi pour réduire sa menace."
                      << std::endl;
            std::cout << std::endl;
        }
    }
}

bool CombatRoleActionSystem::activateManualProvocation(
    Entity& entity,
    int turns
)
{
    if (!CombatRoleSystem::isTank(entity))
    {
        std::cout << entity.getName()
                  << " tente de provoquer l'ennemi, mais ce rôle ne lui correspond pas vraiment."
                  << std::endl;
        std::cout << std::endl;
        return false;
    }

    entity.startProvocation(turns);

    std::cout << entity.getName()
              << " utilise Provocation. Les ennemis auront beaucoup plus de mal à ignorer sa présence."
              << std::endl;
    std::cout << std::endl;

    return true;
}

void CombatRoleActionSystem::displayRoleIdentity(const Entity& entity)
{
    if (CombatRoleSystem::isTank(entity))
    {
        std::cout << entity.getName()
                  << " possède un profil de tank : il peut attirer la pression ennemie."
                  << std::endl;
    }
    else if (CombatRoleSystem::isHealer(entity))
    {
        std::cout << entity.getName()
                  << " possède un profil de soigneur : soigner un allié attirera l'attention ennemie."
                  << std::endl;
    }
    else if (CombatRoleSystem::isSummoner(entity))
    {
        std::cout << entity.getName()
                  << " possède un profil d'invocateur : le duel peut devenir un combat de groupe."
                  << std::endl;
    }
    else if (CombatRoleSystem::isAssassin(entity))
    {
        std::cout << entity.getName()
                  << " possède un profil d'assassin : menace plus discrète, mais coups dangereux."
                  << std::endl;
    }
    else
    {
        return;
    }

    std::cout << std::endl;
}
