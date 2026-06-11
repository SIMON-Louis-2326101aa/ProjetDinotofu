// EN: ObservationSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ObservationSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.

#include "combat/system/ObservationSystem.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <string>
#include <vector>

namespace
{
    std::string yesNo(bool value)
    {
        return value ? "oui" : "non";
    }

    std::vector<std::string> buildVisibleStatLines(const Entity& target)
    {
        std::vector<std::string> lines;
        lines.push_back("Nom : " + target.getName());
        lines.push_back("Type : " + target.getType());
        lines.push_back("PV : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()));
        lines.push_back("Dégâts : " + std::to_string(target.getMinDamage()) + "-" + std::to_string(target.getMaxDamage()));
        lines.push_back("Critique : " + std::to_string(target.getCriticalDamage()));
        lines.push_back("Potions de soin : " + std::to_string(target.getHealingPotionCount()));
        lines.push_back("Potions offensives : " + std::to_string(target.getDamagePotionCount()));

        if (target.isProvoking())
        {
            lines.push_back("Provocation : active, encore " + std::to_string(target.getProvocationTurns()) + " tour(s)");
        }

        if (target.isInDefensePosture())
        {
            lines.push_back(
                "Posture défensive : " + target.getDefensePostureLabel()
                + " | réduction " + std::to_string(target.getDefenseReductionPercent()) + "%"
                + " | contre " + std::to_string(target.getDefenseCounterChance()) + "%"
            );
        }

        if (target.hasActiveCombatStatus())
        {
            lines.push_back("Altérations actives : oui");
            if (target.hasBurning()) lines.push_back("- Brûlure : " + yesNo(true));
            if (target.hasPoison()) lines.push_back("- Poison : " + yesNo(true));
            if (target.hasFrost()) lines.push_back("- Givre : " + yesNo(true));
            if (target.hasShock()) lines.push_back("- Choc : " + yesNo(true));
            if (target.hasBleeding()) lines.push_back("- Saignement : " + yesNo(true));
            if (target.hasWeakening()) lines.push_back("- Affaiblissement : " + yesNo(true));
            if (target.hasVulnerability()) lines.push_back("- Vulnérabilité : " + yesNo(true));
            if (target.hasElementalWard()) lines.push_back("- Égide élémentaire : résistance " + std::to_string(target.getElementalWardResistancePercent()) + "%");
            if (target.hasRegeneration()) lines.push_back("- Régénération : " + yesNo(true));
            if (target.hasPowerBoost()) lines.push_back("- Puissance chanceuse : +" + std::to_string(target.getPowerBoostPercent()) + "% dégâts");
            if (target.hasPrecisionBoost()) lines.push_back("- Précision chanceuse : +" + std::to_string(target.getPrecisionRollBonus()) + " au jet");
            if (target.hasGuardBoost()) lines.push_back("- Protection chanceuse : -" + std::to_string(target.getGuardReductionPercent()) + "% dégâts reçus");
        }
        else
        {
            lines.push_back("Altérations actives : aucune visible");
        }

        lines.push_back("Note IG : seules les statistiques déjà décryptées sont affichées ici.");
        return lines;
    }
}

// EN: displayFailedDecryptionAttempt declares or implements a focused behavior used by this module.
// FR: displayFailedDecryptionAttempt déclare ou implémente un comportement précis utilisé par ce module.
void ObservationSystem::displayFailedDecryptionAttempt(const Entity& target)
{
    MessageScreen::show(
        "DÉCRYPTAGE ÉCHOUÉ",
        "combat.observation.decrypt.failed",
        {
            "Tentative de décryptage échouée.",
            "Les données de " + target.getName() + " restent brouillées par une force beaucoup trop élevée.",
            "L'interface refuse encore d'afficher ses statistiques."
        }
    );
}

// EN: displayTerminalStats declares or implements a focused behavior used by this module.
// FR: displayTerminalStats déclare ou implémente un comportement précis utilisé par ce module.
void ObservationSystem::displayTerminalStats(const Entity& target)
{
    if (!target.areStatsVisible())
    {
        displayFailedDecryptionAttempt(target);
        return;
    }

    MessageScreen::show(
        "STATISTIQUES DÉCRYPTÉES",
        "combat.observation.stats.visible",
        buildVisibleStatLines(target)
    );
}
