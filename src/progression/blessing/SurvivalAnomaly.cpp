// English: Applies the irreversible cost of surviving a lethal death through blessings.
// Français : Applique le coût irréversible d'une survie à une mort léthale grâce aux bénédictions.
#include "progression/blessing/SurvivalAnomaly.hpp"

#include "entity/Player.hpp"

#include <algorithm>

const std::string& SurvivalAnomaly::permanentCurseId()
{
    static const std::string id = "lethal_survival_scar";
    return id;
}

bool SurvivalAnomaly::canApply(const Player& player)
{
    return player.isDead()
        && player.canReceiveBlessings()
        && player.hasLethalSurvivalBlessing();
}

void SurvivalAnomaly::apply(Player& player)
{
    if (!canApply(player))
    {
        return;
    }

    player.consumeAllBlessings();
    player.unequipWeapon();
    player.unequipArmor();
    player.getInventory().clearAll();

    player.reviveWithHealthPercentage(1);
    if (player.getHp() > 1)
    {
        player.takeDamage(player.getHp() - 1);
    }
    if (player.getHp() <= 0)
    {
        player.heal(1);
    }

    PlayerCurse scar;
    scar.id = permanentCurseId();
    scar.name = "Marque de survie interdite";
    scar.severity = "Irréversible";
    scar.origin = "Toutes les bénédictions ont été consumées pour repousser une mort définitive.";
    scar.description = "Le personnage ne peut plus jamais recevoir de bénédiction et reçoit seulement 95 % des soins normaux.";
    scar.removalHint = "Aucune église, aucun boss et aucun objet connu ne peut retirer cette marque.";
    scar.symptomCategories = "health,spirit,corruption";
    scar.discoveredSymptomCategories = "health,spirit,corruption";
    scar.excludedSymptomCategories = "";
    scar.diagnosisLevel = 3;
    scar.appliedAtDay = player.getWorldDaysElapsed();
    scar.expiresAtDay = -1;
    scar.exorcismProgress = 0;
    scar.exorcismRequiredVisits = 0;
    scar.curseLevel = 1;
    scar.maxCurseLevel = 1;
    scar.evolvesOverTime = false;
    scar.escalationIntervalDays = 0;
    scar.nextEscalationDay = -1;
    scar.churchRemovalMaxLevel = 0;
    scar.becomesSpecialRemovalWhenTooHigh = false;
    scar.highLevelRemovalHint = "Impossible à retirer.";
    scar.removableByChurch = false;
    scar.bossIdRequiredToBreak = 0;
    scar.lifeLong = true;

    player.addOrRefreshCurse(scar);
    player.setHealingReceivedPercent(95);
}
