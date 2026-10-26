#include "combat/group/EmployerTacticalOrderSystem.hpp"
#include "core/Random.hpp"

#include <algorithm>

EmployerTacticalOrder EmployerTacticalOrderSystem::decide(
    const EmployerTacticalOrderContext& context,
    Random& random)
{
    EmployerTacticalOrder order;
    const int intelligence = std::clamp(context.leaderIntelligence, 0, 100);

    // Un chef n'aboie pas obligatoirement une nouvelle consigne à chaque respiration.
    // Ici la décision est faite au début d'un engagement : il peut laisser la formation standard.
    int issueChance = 48 + intelligence / 4;
    if (context.bossContext) issueChance += 8;
    if (context.criticalHpAllies > 0) issueChance += 12;
    issueChance = std::clamp(issueChance, 45, 86);
    if (random.between(1, 100) > issueChance)
    {
        order.rationale = "le chef conserve la doctrine standard sans ajouter d'ordre particulier";
        return order;
    }

    if (context.criticalHpAllies > 0 && context.reserveAvailable)
    {
        order.kind = EmployerTacticalOrderKind::ExtractWounded;
        order.code = "extract_wounded";
        order.title = "Extraire les blessés";
        order.instruction = "Protégez la rotation et sortez les membres en danger dès qu'une fenêtre apparaît.";
        order.rationale = "un ou plusieurs alliés sont proches de la rupture et une réserve existe";
        return order;
    }

    if (context.supportMemberActive && context.activeEnemyCount >= 2)
    {
        order.kind = EmployerTacticalOrderKind::ProtectSupport;
        order.code = "protect_support";
        order.title = "Protéger le soutien";
        order.instruction = "Gardez notre soutien debout ; ne laissez pas la ligne l'isoler.";
        order.rationale = "le groupe dépend d'un soutien identifiable sous pression";
        return order;
    }

    if (context.activeEnemyCount >= 2)
    {
        if (random.between(1, 100) <= 55)
        {
            order.kind = EmployerTacticalOrderKind::FocusFire;
            order.code = "focus_fire";
            order.title = "Concentrer la cible";
            order.instruction = "On tombe une cible ensemble avant d'ouvrir la suivante.";
            order.rationale = "plusieurs adversaires occupent encore la ligne";
        }
        else
        {
            order.kind = EmployerTacticalOrderKind::ControlledBreakthrough;
            order.code = "controlled_breakthrough";
            order.title = "Percée contrôlée";
            order.instruction = "Cherchez une ouverture coordonnée sans casser la formation.";
            order.rationale = "la ligne ennemie est assez dense pour justifier une action coordonnée";
        }
        return order;
    }

    order.kind = EmployerTacticalOrderKind::HoldLine;
    order.code = "hold_line";
    order.title = "Tenir la ligne";
    order.instruction = context.bossContext
        ? "Pas de geste inutile : gardez la formation et survivez aux fenêtres dangereuses."
        : "Restez groupés et ne donnez pas d'ouverture gratuite.";
    order.rationale = "une seule menace principale reste active";
    return order;
}

std::string EmployerTacticalOrderSystem::label(EmployerTacticalOrderKind kind)
{
    switch (kind)
    {
        case EmployerTacticalOrderKind::ProtectSupport: return "Protéger le soutien";
        case EmployerTacticalOrderKind::FocusFire: return "Concentrer la cible";
        case EmployerTacticalOrderKind::HoldLine: return "Tenir la ligne";
        case EmployerTacticalOrderKind::ExtractWounded: return "Extraire les blessés";
        case EmployerTacticalOrderKind::ControlledBreakthrough: return "Percée contrôlée";
        case EmployerTacticalOrderKind::None:
        default: return "Doctrine standard";
    }
}
