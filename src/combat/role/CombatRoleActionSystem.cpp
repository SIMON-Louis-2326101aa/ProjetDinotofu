// EN: CombatRoleActionSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatRoleActionSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Applies first active role behaviours such as automatic tank provocation and future healer/support hooks.

#include "combat/role/CombatRoleActionSystem.hpp"

#include "combat/role/CombatRoleSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

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

            MessageScreen::show(
                "MENACE RÉDUITE",
                "combat.role.assassin.threat_drop",
                {
                    entity.getName() + " disparaît assez longtemps du regard ennemi pour réduire sa menace."
                },
                false
            );
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
        MessageScreen::show(
            "PROVOCATION IMPOSSIBLE",
            "combat.role.provocation.refused",
            {
                entity.getName() + " tente de provoquer l'ennemi, mais ce rôle ne lui correspond pas vraiment."
            }
        );
        return false;
    }

    entity.startProvocation(turns);

    MessageScreen::show(
        "PROVOCATION",
        "combat.role.provocation.active",
        {
            entity.getName() + " utilise Provocation.",
            "Les ennemis auront beaucoup plus de mal à ignorer sa présence.",
            "La posture de défense se verrouille dans le même mouvement.",
            "Durée : " + std::to_string(turns) + " tour(s)."
        }
    );

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

    std::vector<std::string> lines;
    if (isHazakProtectingHestia)
    {
        lines.push_back("Hazak n'aime pas jouer les boucliers, mais Hestia ne doit pas voir ça.");
        lines.push_back("Il coupe la trajectoire ennemie avant que le coup ne l'atteigne.");
    }
    else
    {
        lines.push_back(protector.getName() + " se place devant " + endangeredAlly.getName() + " pour absorber la pression.");
    }

    lines.push_back("Provocation : le groupe devient plus dur à contourner.");

    MessageScreen::show("PROTECTION ALLIÉE", "combat.role.ally_protection", lines, false);
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
    const int beforeHp = ally.getHp();
    ally.heal(healAmount);
    support.markHealingThreat();

    MessageScreen::show(
        "SOUTIEN RAPIDE",
        "combat.role.support_recovery",
        {
            support.getName() + " improvise un soutien rapide pour " + ally.getName() + ".",
            "PV : " + std::to_string(beforeHp) + " -> " + std::to_string(ally.getHp()) + "/" + std::to_string(ally.getMaxHp()),
            "Soin brut : +" + std::to_string(healAmount) + " PV.",
            "Menace : même un petit soutien peut attirer l'attention."
        },
        false
    );

    return true;
}

// EN: displayRoleIdentity declares or implements a focused behavior used by this module.
// FR: displayRoleIdentity déclare ou implémente un comportement précis utilisé par ce module.
void CombatRoleActionSystem::displayRoleIdentity(const Entity& entity)
{
    std::vector<std::string> lines;

    if (CombatRoleSystem::isTank(entity))
    {
        lines.push_back(entity.getName() + " possède un profil de tank.");
        lines.push_back("Il peut attirer la pression ennemie.");
    }
    else if (CombatRoleSystem::isHealer(entity))
    {
        lines.push_back(entity.getName() + " possède un profil de soigneur.");
        lines.push_back("Soigner un allié attirera l'attention ennemie.");
        if (nameIs(entity, "Hestia"))
        {
            lines.push_back("Particularité : Hestia soigne/protège plus fort que ses stats ne le laissent croire.");
        }
    }
    else if (CombatRoleSystem::isSummoner(entity))
    {
        lines.push_back(entity.getName() + " possède un profil d'invocateur.");
        lines.push_back("Le duel peut devenir un combat de groupe.");
    }
    else if (CombatRoleSystem::isAssassin(entity))
    {
        lines.push_back(entity.getName() + " possède un profil d'assassin.");
        lines.push_back("Menace plus discrète, mais coups dangereux.");
    }
    else
    {
        return;
    }

    MessageScreen::show("PROFIL DE RÔLE", "combat.role.identity", lines, false);
}
