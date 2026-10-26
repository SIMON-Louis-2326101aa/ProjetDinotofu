// EN: DeathPenaltySystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DeathPenaltySystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares future death penalties for non-lethal runs.
// Français : Ce fichier prépare les futures pénalités de mort pour les parties non létales.

#include "progression/death/DeathPenaltySystem.hpp"
#include "combat/death/LethalSpiritResistanceSystem.hpp"
#include "economy/Money.hpp"

#include "item/Inventory.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"
#include "progression/DifficultyRules.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"

#include <iostream>
#include <string>

namespace
{
    // EN: rollChance declares or implements a focused behavior used by this module.
    // FR: rollChance déclare ou implémente un comportement précis utilisé par ce module.
    bool rollChance(Random& random, int percentage)
    {
        if (percentage <= 0)
        {
            return false;
        }

        if (percentage >= 100)
        {
            return true;
        }

        return random.between(1, 100) <= percentage;
    }

    // EN: calculateDurabilityLoss declares or implements a focused behavior used by this module.
    // FR: calculateDurabilityLoss déclare ou implémente un comportement précis utilisé par ce module.
    int calculateDurabilityLoss(int currentDurability, int percentage)
    {
        int loss = currentDurability * percentage / 100;

        if (currentDurability > 0 && loss <= 0 && percentage > 0)
        {
            loss = 1;
        }

        return loss;
    }

    // EN: isBaseWeapon declares or implements a focused behavior used by this module.
    // FR: isBaseWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool isBaseWeapon(const Weapon& weapon)
    {
        return weapon.getName() == "Mains nues";
    }

    // EN: isBaseArmor declares or implements a focused behavior used by this module.
    // FR: isBaseArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool isBaseArmor(const Armor& armor)
    {
        return armor.getName() == "Tenue simple";
    }

    std::string weaponHistorySubjectId(const Weapon& weapon)
    {
        return weapon.getPersistentId().empty() ? "weapon:" + weapon.getName() : weapon.getPersistentId();
    }

    std::string armorHistorySubjectId(const Armor& armor)
    {
        return armor.getPersistentId().empty() ? "armor:" + armor.getName() : armor.getPersistentId();
    }

    void recordEquipmentLoss(Player& player, const std::string& category, const std::string& subjectId, const std::string& label)
    {
        if (subjectId.empty() || label.empty()) return;
        player.recordHistoricalEvent(category, subjectId, label, true);
    }
}

DeathPenaltyResult DeathPenaltySystem::applyNonLethalDeathPenalty(
    Player& player,
    DifficultyMode difficulty,
    Random& random,
    bool canStealEquipment
)
{
    DeathPenaltyResult result;

    int economyLossPercentage = DifficultyRules::getNonLethalDeathEconomyLossPercentage(difficulty);
    int experienceLossPercentage = DifficultyRules::getNonLethalDeathExperienceLossPercentage(difficulty);
    int inventoryLossPercentage = DifficultyRules::getNonLethalDeathInventoryLossPercentage(difficulty);

    int lostEconomyUnits = static_cast<int>(player.getInventory().getEconomyUnits() * economyLossPercentage / 100);
    int lostExperience = player.getExperience() * experienceLossPercentage / 100;

    if (lostEconomyUnits > 0 && player.getInventory().spendEconomyUnits(lostEconomyUnits))
    {
        result.addLostEconomyUnits(lostEconomyUnits);
    }

    if (lostExperience > 0)
    {
        player.loseExperience(lostExperience);
        result.addLostExperience(lostExperience);
    }

    int consumableCount = player.getInventory().getConsumableCount();
    int consumablesToRemove = consumableCount * inventoryLossPercentage / 100;

    if (consumableCount > 0 && consumablesToRemove <= 0 && inventoryLossPercentage > 0)
    {
        consumablesToRemove = 1;
    }

    for (int i = 0; i < consumablesToRemove && player.getInventory().getConsumableCount() > 0; ++i)
    {
        int index = random.between(0, player.getInventory().getConsumableCount() - 1);

        if (player.getInventory().removeConsumable(index))
        {
            result.addLostConsumables(1);
        }
    }

    if (player.hasEquippedWeapon() && !player.hasEquipmentProtection())
    {
        Weapon weaponSnapshot = player.getEquippedWeapon();
        Weapon* weapon = player.getInventory().getMutableWeapon(player.getEquippedWeaponIndex());

        if (weapon != nullptr && !weapon->isIndestructible() && !isBaseWeapon(*weapon))
        {
            if (canStealEquipment
                // EN: rollChance declares or implements a focused behavior used by this module.
                // FR: rollChance déclare ou implémente un comportement précis utilisé par ce module.
                && rollChance(random, DifficultyRules::getDeathWeaponTheftChance(difficulty)))
            {
                if (player.destroyEquippedWeapon())
                {
                    result.markWeaponStolen();
                    recordEquipmentLoss(
                        player,
                        "equipment_lost",
                        weaponHistorySubjectId(weaponSnapshot),
                        "Équipement perdu : " + weaponSnapshot.getName() + " a été volé après une défaite non définitive."
                    );
                }
            }
            else
            {
                int durabilityLoss = calculateDurabilityLoss(
                    weapon->getDurability(),
                    DifficultyRules::getDeathEquipmentDurabilityLossPercentage(difficulty)
                );

                if (!player.hasIndestructibleEquipment())
                {
                    weapon->loseDurability(durabilityLoss);
                    result.addWeaponDurabilityLost(durabilityLoss);
                }

                bool weaponForcedBreak = false;
                if (weapon->getDurability() > 0
                    && rollChance(random, DifficultyRules::getDeathEquipmentForcedBreakChance(difficulty)))
                {
                    if (!player.hasIndestructibleEquipment())
                    {
                        weapon->loseDurability(weapon->getDurability());
                        result.markWeaponBroken();
                        weaponForcedBreak = true;
                    }
                }

                if (weapon->isBroken()
                    // EN: rollChance declares or implements a focused behavior used by this module.
                    // FR: rollChance déclare ou implémente un comportement précis utilisé par ce module.
                    && rollChance(random, DifficultyRules::getDeathEquipmentIrreparableChance(difficulty)))
                {
                    if (player.destroyEquippedWeapon())
                    {
                        result.markWeaponIrreparable();
                        result.addRecoveredMaterialFragments(1);
                        recordEquipmentLoss(
                            player,
                            "equipment_destroyed",
                            weaponHistorySubjectId(weaponSnapshot),
                            "Équipement détruit : " + weaponSnapshot.getName() + " est devenu irréparable après une défaite non définitive."
                        );
                    }
                }
                else if (weaponForcedBreak)
                {
                    recordEquipmentLoss(
                        player,
                        "equipment_forced_break",
                        weaponHistorySubjectId(weaponSnapshot),
                        "Casse forcée : " + weaponSnapshot.getName() + " a été brisé par les conséquences d'une défaite non définitive."
                    );
                }
            }
        }
        else
        {
            (void)weaponSnapshot;
        }
    }

    if (player.hasEquippedArmor() && !player.hasEquipmentProtection())
    {
        Armor armorSnapshot = player.getEquippedArmor();
        Armor* armor = player.getInventory().getMutableArmor(player.getEquippedArmorIndex());

        if (armor != nullptr && !armor->isIndestructible() && !isBaseArmor(*armor))
        {
            if (canStealEquipment
                // EN: rollChance declares or implements a focused behavior used by this module.
                // FR: rollChance déclare ou implémente un comportement précis utilisé par ce module.
                && rollChance(random, DifficultyRules::getDeathArmorTheftChance(difficulty)))
            {
                if (player.destroyEquippedArmor())
                {
                    result.markArmorStolen();
                    recordEquipmentLoss(
                        player,
                        "equipment_lost",
                        armorHistorySubjectId(armorSnapshot),
                        "Équipement perdu : " + armorSnapshot.getName() + " a été volé après une défaite non définitive."
                    );
                }
            }
            else
            {
                int durabilityLoss = calculateDurabilityLoss(
                    armor->getDurability(),
                    DifficultyRules::getDeathEquipmentDurabilityLossPercentage(difficulty)
                );

                if (!player.hasIndestructibleEquipment())
                {
                    armor->loseDurability(durabilityLoss);
                    result.addArmorDurabilityLost(durabilityLoss);
                }

                bool armorForcedBreak = false;
                if (armor->getDurability() > 0
                    && rollChance(random, DifficultyRules::getDeathEquipmentForcedBreakChance(difficulty)))
                {
                    if (!player.hasIndestructibleEquipment())
                    {
                        armor->loseDurability(armor->getDurability());
                        result.markArmorBroken();
                        armorForcedBreak = true;
                    }
                }

                if (armor->isBroken()
                    // EN: rollChance declares or implements a focused behavior used by this module.
                    // FR: rollChance déclare ou implémente un comportement précis utilisé par ce module.
                    && rollChance(random, DifficultyRules::getDeathEquipmentIrreparableChance(difficulty)))
                {
                    if (player.destroyEquippedArmor())
                    {
                        result.markArmorIrreparable();
                        result.addRecoveredMaterialFragments(1);
                        recordEquipmentLoss(
                            player,
                            "equipment_destroyed",
                            armorHistorySubjectId(armorSnapshot),
                            "Équipement détruit : " + armorSnapshot.getName() + " est devenu irréparable après une défaite non définitive."
                        );
                    }
                }
                else if (armorForcedBreak)
                {
                    recordEquipmentLoss(
                        player,
                        "equipment_forced_break",
                        armorHistorySubjectId(armorSnapshot),
                        "Casse forcée : " + armorSnapshot.getName() + " a été brisé par les conséquences d'une défaite non définitive."
                    );
                }
            }
        }
    }

    return result;
}

void DeathPenaltySystem::displayNonLethalDeathPenalty(
    const DeathPenaltyResult& result
)
{
    MenuScreen screen("CONSÉQUENCES DE LA MORT", "death.non_lethal.penalty");
    screen.addLine("Ton personnage survit, mais la mort ne repart jamais les mains vides.");
    screen.addLine("");
    screen.addLine("Argent perdu : " + Money::formatEconomyUnits(result.getLostEconomyUnits()));
    screen.addLine("Expérience perdue : " + std::to_string(result.getLostExperience()));
    screen.addLine("Consommables perdus : " + std::to_string(result.getLostConsumables()));
    screen.addLine("Durabilité perdue sur l'arme équipée : " + std::to_string(result.getWeaponDurabilityLost()));
    screen.addLine("Durabilité perdue sur l'armure équipée : " + std::to_string(result.getArmorDurabilityLost()));

    if (result.wasWeaponBroken())
    {
        screen.addLine("Arme équipée : brisée par le choc.");
    }

    if (result.wasArmorBroken())
    {
        screen.addLine("Armure équipée : brisée par le choc.");
    }

    if (result.wasWeaponIrreparable())
    {
        screen.addLine("Arme équipée : détruite au point de devenir irréparable.");
    }

    if (result.wasArmorIrreparable())
    {
        screen.addLine("Armure équipée : détruite au point de devenir irréparable.");
    }

    if (result.wasWeaponStolen())
    {
        screen.addLine("Arme équipée : volée pendant l'effondrement du combat.");
    }

    if (result.wasArmorStolen())
    {
        screen.addLine("Armure équipée : volée pendant l'effondrement du combat.");
    }

    if (result.getRecoveredMaterialFragments() > 0)
    {
        screen.addLine("Fragments récupérables générés : " + std::to_string(result.getRecoveredMaterialFragments()));
    }

    screen.addFooterLine("Objets liés au corps, à l'âme ou au personnage : une marque profonde les protège.");
    screen.setDisplayOnlyInput("Résumé de pénalité affiché sans saisie directe.");
    TerminalInterface::renderMenuScreen(screen, false);
}

// EN: displayLethalDeathCorruption declares or implements a focused behavior used by this module.
// FR: displayLethalDeathCorruption déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalDeathCorruption()
{
    MenuScreen screen("MORT DÉFINITIVE", "death.lethal.corruption");
    screen.addLine("Morts du personnage : [STATISTIQUE CORROMPUE]");
    screen.addLine("Vous ne deviez pas mourir.");
    screen.addLine("Statut : personnage supprimé du registre des vivants.");
    screen.addLine("Connais-tu quelqu'un capable d'échapper à la mort ? Moi non...");
    screen.setDisplayOnlyInput("Message létal affiché sans choix joueur.");
    TerminalInterface::renderMenuScreen(screen, false);
}

// EN: displayLethalCurrentDeathStatistic declares or implements a focused behavior used by this module.
// FR: displayLethalCurrentDeathStatistic déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalCurrentDeathStatistic()
{
    MenuScreen screen("STATISTIQUE CORROMPUE", "death.lethal.current_statistic");
    screen.addLine("Morts du personnage : [STATISTIQUE CORROMPUE]");
    screen.addLine("But de mission : survivre.");
    screen.setDisplayOnlyInput("Statistique corrompue affichée sans choix joueur.");
    TerminalInterface::renderMenuScreen(screen, false);
}

void DeathPenaltySystem::displayLethalSpiritResistance(const LethalSpiritResistanceResult& result)
{
    MenuScreen screen(
        result.survived ? "L'ESPRIT REFUSE DE CÉDER" : "L'ESPRIT S'ÉTEINT",
        result.survived ? "death.lethal.spirit_resistance.success" : "death.lethal.spirit_resistance.failure"
    );
    screen.addLine("Cette résistance n'existe qu'une seule fois dans la vie du personnage.");
    screen.addLine("Aucun allié ne réussit à t'arracher à la mort. Il ne reste que ta propre volonté.");
    screen.addLine("");

    int successes = 0;
    int failures = 0;
    for (std::size_t i = 0; i < result.rolls.size(); ++i)
    {
        const int roll = result.rolls[i];
        const bool success = roll >= LethalSpiritResistanceSystem::D20SuccessThreshold;
        if (success) ++successes; else ++failures;
        screen.addLine(
            "Pastille de volonté " + std::to_string(i + 1) + " — d20 : " + std::to_string(roll)
            + (success ? " → l'esprit tient." : " → la prise se relâche.")
        );
        if (successes >= LethalSpiritResistanceSystem::RequiredSuccesses
            || failures >= LethalSpiritResistanceSystem::RequiredFailures)
        {
            break;
        }
    }

    screen.addLine("");
    screen.addLine(
        "Résultat des pastilles : " + std::to_string(result.successes) + "/3 tenues, "
        + std::to_string(result.failures) + "/3 perdues."
    );
    if (result.survived)
    {
        screen.addLine("Quelque chose en toi refuse de lâcher prise. Tu reviens à 1 PV.");
        screen.addLine("Cette résistance spirituelle est désormais consumée pour toujours.");
    }
    else
    {
        screen.addLine("La volonté seule ne suffit pas cette fois.");
        screen.addLine("Cette unique résistance spirituelle est tout de même consumée : elle ne reviendra pas lors d'une future mort.");
    }
    screen.setDisplayOnlyInput("Résistance spirituelle létale résolue automatiquement.");
    TerminalInterface::renderMenuScreen(screen, false);
}

// EN: displayLethalSurvivalAnomaly declares or implements a focused behavior used by this module.
// FR: displayLethalSurvivalAnomaly déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalSurvivalAnomaly()
{
    MenuScreen screen("ANOMALIE DE SURVIE", "death.lethal.survival_anomaly");
    screen.addLine("La mort t'a trouvé. Puis elle a reculé.");
    screen.addLine("");
    screen.addLine("Toutes tes bénédictions ont été consumées pour t'arracher au registre des morts.");
    screen.addLine("Tu ouvres les yeux avec exactement 1 PV.");
    screen.addLine("Ton équipement, ton inventaire et ton argent ont été détruits.");
    screen.addLine("");
    screen.addLine("Malédiction irréversible : Marque de survie interdite.");
    screen.addLine("Effet permanent : aucune nouvelle bénédiction ne peut être reçue.");
    screen.addLine("Effet permanent : tous les soins reçus sont réduits de 5 %.");
    screen.addLine("Cette marque ne peut être retirée ni par une église, ni par un boss, ni par un objet connu.");
    screen.addLine("");
    screen.addLine("Statut : anomalie de survie. Ton nom existe encore, mais le registre continue de te regarder.");
    screen.setDisplayOnlyInput("Cette survie est automatique : aucune validation joueur n'est demandée après la mort.");
    TerminalInterface::renderMenuScreen(screen, false);
}
