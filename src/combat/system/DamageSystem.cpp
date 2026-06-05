// EN: DamageSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DamageSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/DamageSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"

#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
    std::string normalizeEquipmentEffectText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool equipmentNameContains(const std::string& text, const std::string& needle)
    {
        return normalizeEquipmentEffectText(text).find(normalizeEquipmentEffectText(needle)) != std::string::npos;
    }
}

// EN: calculateReceivedDamage declares or implements a focused behavior used by this module.
// FR: calculateReceivedDamage déclare ou implémente un comportement précis utilisé par ce module.
DamageReport DamageSystem::calculateReceivedDamage(Entity& defender, int rawDamage)
{
    DamageReport rapport;
    rapport.rawDamage = rawDamage;

    int remainingDamage = rawDamage;

    Player* defendingPlayer = dynamic_cast<Player*>(&defender);

    if (defendingPlayer != nullptr && defendingPlayer->hasEquippedArmor() && !defendingPlayer->hasBossEquipmentSeal())
    {
        Armor* armor = defendingPlayer->getInventory().getMutableArmor(
            defendingPlayer->getEquippedArmorIndex()
        );

        if (armor != nullptr && !armor->isBroken())
        {
            int armorAbsorption = armor->getDamageReduction();
            const std::string armorName = armor->getName();

            if (equipmentNameContains(armorName, "cotte runique") || equipmentNameContains(armorName, "runique de garde"))
            {
                if (rawDamage <= 24)
                {
                    armorAbsorption += 2;
                    rapport.equipmentEffectLines.push_back("Rune de garde : la cotte runique amortit mieux un choc faible.");
                }
            }
            else if (equipmentNameContains(armorName, "manteau cousu d'ombre") || equipmentNameContains(armorName, "cousu d'ombre"))
            {
                if (rawDamage <= 18)
                {
                    armorAbsorption += 2;
                    rapport.equipmentEffectLines.push_back("Fil d'ombre : le manteau dévie légèrement un coup rapide.");
                }
                else if (rawDamage >= 34 && armorAbsorption > 1)
                {
                    armorAbsorption -= 1;
                    rapport.equipmentEffectLines.push_back("Fil d'ombre : le tissu léger encaisse moins bien ce choc lourd.");
                }
            }
            else if (equipmentNameContains(armorName, "harnais d'écailles") || equipmentNameContains(armorName, "écailles polies"))
            {
                if (rawDamage >= 15)
                {
                    armorAbsorption += 2;
                    rapport.equipmentEffectLines.push_back("Écailles polies : les plaques répartissent mieux l'impact.");
                }
            }

            if (armorAbsorption > remainingDamage)
            {
                armorAbsorption = remainingDamage;
            }

            if (armorAbsorption > 0)
            {
                rapport.armorUsed = true;
                rapport.armorAbsorbedDamage = armorAbsorption;

                if (!defendingPlayer->hasIndestructibleEquipment())
                {
                    armor->loseDurability(1);

                    if ((equipmentNameContains(armorName, "harnais d'écailles") || equipmentNameContains(armorName, "écailles polies"))
                        && rawDamage >= 30
                        && !armor->isBroken())
                    {
                        armor->loseDurability(1);
                        rapport.armorExtraDurabilityLost = true;
                    }
                }

                if (armor->isBroken())
                {
                    rapport.armorBrokenDuringImpact = true;
                }

                remainingDamage -= armorAbsorption;
            }
        }
    }

    if (defendingPlayer != nullptr && defendingPlayer->hasEquippedArmor() && defendingPlayer->hasBossEquipmentSeal())
    {
        rapport.armorBlockedByBossSeal = true;
    }

    int classReductionPercentage =
        CombatClassSystem::getBaseDamageReductionPercentage(defender);

    Monster* defendingMonster = dynamic_cast<Monster*>(&defender);
    if (defendingMonster != nullptr)
    {
        const std::string raceText = defendingMonster->getRaceText();
        const std::string typeText = defendingMonster->getType();

        if (raceText.find("Slime") != std::string::npos)
        {
            classReductionPercentage += 6;
        }
        else if (raceText.find("Plante") != std::string::npos || typeText.find("résistante") != std::string::npos)
        {
            classReductionPercentage += 4;
        }
        else if (raceText.find("Mort-vivant") != std::string::npos)
        {
            classReductionPercentage += 5;
        }
        else if (raceText.find("Dragon") != std::string::npos || raceText.find("Draconide") != std::string::npos)
        {
            classReductionPercentage += 8;
        }

        if (defendingMonster->isElite())
        {
            classReductionPercentage += 3;
        }
    }

    if (classReductionPercentage > 0 && remainingDamage > 0)
    {
        int classReduction = remainingDamage * classReductionPercentage / 100;

        if (classReduction > remainingDamage)
        {
            classReduction = remainingDamage;
        }

        if (classReduction > 0)
        {
            rapport.classReducedDamage = classReduction;
            remainingDamage -= classReduction;
        }
    }

    if (remainingDamage < 0)
    {
        remainingDamage = 0;
    }

    rapport.receivedDamage = remainingDamage;

    return rapport;
}

// EN: buildDamageReportLines declares or implements a focused behavior used by this module.
// FR: buildDamageReportLines déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> DamageSystem::buildDamageReportLines(const Entity& defender, const DamageReport& rapport)
{
    std::vector<std::string> lines;

    if (rapport.armorBlockedByBossSeal)
    {
        lines.push_back("Le sceau de boss empêche l'armure de " + defender.getName() + " de répondre correctement.");
    }

    if (rapport.armorAbsorbedDamage > 0)
    {
        lines.push_back(
            "Armure : " + defender.getName() + " absorbe "
            + std::to_string(rapport.armorAbsorbedDamage) + " dégât(s)."
        );
    }
    else if (rapport.armorUsed)
    {
        lines.push_back(
            "Armure : " + defender.getName()
            + " encaisse le choc, mais n'absorbe aucun dégât."
        );
    }

    for (const std::string& effectLine : rapport.equipmentEffectLines)
    {
        lines.push_back(effectLine);
    }

    if (rapport.armorExtraDurabilityLost)
    {
        lines.push_back("Entretien : les écailles polies encaissent fort, mais l'impact use davantage les fixations.");
    }

    if (rapport.armorBrokenDuringImpact)
    {
        lines.push_back("Rupture : l'armure se fissure sous l'impact et se brise.");
        lines.push_back("Elle ne protégera plus son porteur tant qu'elle ne sera pas réparée.");
    }

    if (rapport.classReducedDamage > 0)
    {
        lines.push_back(
            "Résistance : " + defender.getName() + " réduit naturellement "
            + std::to_string(rapport.classReducedDamage)
            + " dégât(s) grâce à sa résistance de classe."
        );
    }

    if (!lines.empty())
    {
        lines.push_back(
            "Dégâts reçus après protection : "
            + std::to_string(rapport.receivedDamage)
            + " / "
            + std::to_string(rapport.rawDamage)
            + " brut(s)."
        );
    }

    return lines;
}

// EN: displayDamageReport declares or implements a focused behavior used by this module.
// FR: displayDamageReport déclare ou implémente un comportement précis utilisé par ce module.
void DamageSystem::displayDamageReport(const Entity& defender, const DamageReport& rapport)
{
    std::vector<std::string> lines = buildDamageReportLines(defender, rapport);

    if (lines.empty())
    {
        return;
    }

    MessageScreen::show(
        "RAPPORT DE DÉGÂTS",
        "combat.damage.report",
        lines,
        false
    );
}

// EN: applyArmorProtection declares or implements a focused behavior used by this module.
// FR: applyArmorProtection déclare ou implémente un comportement précis utilisé par ce module.
int DamageSystem::applyArmorProtection(Entity& defender, int rawDamage)
{
    DamageReport rapport = calculateReceivedDamage(defender, rawDamage);
    displayDamageReport(defender, rapport);

    return rapport.receivedDamage;
}
