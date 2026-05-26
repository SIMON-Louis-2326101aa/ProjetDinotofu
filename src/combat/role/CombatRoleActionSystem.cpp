// EN: CombatRoleActionSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRoleActionSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Applies first active role behaviours such as automatic tank provocation and future healer/support hooks.

#include "combat/role/CombatRoleActionSystem.hpp"

#include "combat/role/CombatRoleSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    std::string toLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    // EN: nameIs declares or implements a focused behavior used by this module.
    // FR: nameIs déclare ou implémente un comportement précis utilisé par ce module.
    bool nameIs(const Entity& entity, const std::string& expected)
    {
        return toLower(entity.getName()) == toLower(expected);
    }

}

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
    std::cout << "La posture de défense se verrouille dans le même mouvement." << std::endl;
    std::cout << std::endl;

    DefensePostureSystem::enterDefensePosture(entity);

    return true;
}

bool CombatRoleActionSystem::tryActivateAllyProtection(
    Entity& protector,
    Entity& endangeredAlly,
    Random& random
)
{
    bool isProtectiveTank = CombatRoleSystem::isTank(protector);
    bool isHazakProtectingHestia = nameIs(protector, "Hazak") && nameIs(endangeredAlly, "Hestia");

    if (!isProtectiveTank && !isHazakProtectingHestia)
    {
        return false;
    }

    if (protector.isProvoking() || endangeredAlly.isDead())
    {
        return false;
    }

    if (endangeredAlly.getMaxHp() <= 0)
    {
        return false;
    }

    int allyHpPercent = endangeredAlly.getHp() * 100 / endangeredAlly.getMaxHp();

    if (allyHpPercent > 40)
    {
        return false;
    }

    int chance = nameIs(protector, "Sanctus") ? 80 : 50;

    if (nameIs(protector, "Henrique") || nameIs(protector, "Mattzelda"))
    {
        chance += 10;
    }

    if (isHazakProtectingHestia)
    {
        chance = 92;
    }

    if (random.between(1, 100) > chance)
    {
        return false;
    }

    protector.startProvocation(2);
    DefensePostureSystem::enterDefensePosture(protector);

    if (isHazakProtectingHestia)
    {
        std::cout << "Hazak n'aime pas jouer les boucliers, mais Hestia ne doit pas voir ça." << std::endl;
        std::cout << "Il coupe la trajectoire ennemie avant que le coup ne l'atteigne." << std::endl;
    }
    else
    {
        std::cout << protector.getName()
                  << " se place devant "
                  << endangeredAlly.getName()
                  << " pour absorber la pression."
                  << std::endl;
    }

    std::cout << "Provocation : le groupe devient plus dur à contourner." << std::endl;
    std::cout << std::endl;

    return true;
}

bool CombatRoleActionSystem::tryActivateSupportRecovery(
    Entity& support,
    Entity& ally,
    Random& random
)
{
    if (!CombatRoleSystem::isSupport(support)
        // EN: isHybrid declares or implements a focused behavior used by this module.
        // FR: isHybrid déclare ou implémente un comportement précis utilisé par ce module.
        && !CombatRoleSystem::isHybrid(support)
        // EN: isHealer declares or implements a focused behavior used by this module.
        // FR: isHealer déclare ou implémente un comportement précis utilisé par ce module.
        && !CombatRoleSystem::isHealer(support))
    {
        return false;
    }

    if (ally.isDead() || ally.getHp() >= ally.getMaxHp())
    {
        return false;
    }

    int allyHpPercent = ally.getHp() * 100 / ally.getMaxHp();

    int chance = CombatRoleSystem::isHealer(support) ? 55 : 35;
    if (nameIs(support, "Hestia")) chance = 75;
    if (nameIs(support, "Sanctus")) chance += 10;
    if (nameIs(ally, "Aoi") && nameIs(support, "Sanctus")) chance += 10;

    if (allyHpPercent > 55 || random.between(1, 100) > chance)
    {
        return false;
    }

    int healAmount = random.between(10, 18);

    if (CombatRoleSystem::isHealer(support))
    {
        healAmount += random.between(6, 12);
    }

    if (nameIs(support, "Hestia"))
    {
        healAmount += random.between(8, 16);
    }
    ally.heal(healAmount);
    support.markHealingThreat();

    std::cout << support.getName()
              << " improvise un soutien rapide pour "
              << ally.getName()
              << ". +"
              << healAmount
              << " PV."
              << std::endl;
    std::cout << "Menace : même un petit soutien peut attirer l'attention." << std::endl;
    std::cout << std::endl;

    return true;
}

// EN: displayRoleIdentity declares or implements a focused behavior used by this module.
// FR: displayRoleIdentity déclare ou implémente un comportement précis utilisé par ce module.
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
        if (nameIs(entity, "Hestia"))
        {
            std::cout << "Particularité : Hestia soigne/protège plus fort que ses stats ne le laissent croire." << std::endl;
        }
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
