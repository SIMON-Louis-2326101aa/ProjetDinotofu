// EN: DeathPenaltySystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DeathPenaltySystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares future death penalties for non-lethal runs.
// Français : Ce fichier prépare les futures pénalités de mort pour les parties non létales.

#include "progression/death/DeathPenaltySystem.hpp"

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
}

DeathPenaltyResult DeathPenaltySystem::applyNonLethalDeathPenalty(
    Player& player,
    DifficultyMode difficulty,
    Random& random,
    bool canStealEquipment
)
{
    DeathPenaltyResult result;

    int goldLossPercentage = DifficultyRules::getNonLethalDeathGoldLossPercentage(difficulty);
    int experienceLossPercentage = DifficultyRules::getNonLethalDeathExperienceLossPercentage(difficulty);
    int inventoryLossPercentage = DifficultyRules::getNonLethalDeathInventoryLossPercentage(difficulty);

    int lostGold = player.getInventory().getGold() * goldLossPercentage / 100;
    int lostExperience = player.getExperience() * experienceLossPercentage / 100;

    if (lostGold > 0 && player.getInventory().spendGold(lostGold))
    {
        result.addLostGold(lostGold);
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

                if (rollChance(random, DifficultyRules::getDeathEquipmentForcedBreakChance(difficulty)))
                {
                    if (!player.hasIndestructibleEquipment())
                    {
                        weapon->loseDurability(weapon->getDurability());
                        result.markWeaponBroken();
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
                    }
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

                if (rollChance(random, DifficultyRules::getDeathEquipmentForcedBreakChance(difficulty)))
                {
                    if (!player.hasIndestructibleEquipment())
                    {
                        armor->loseDurability(armor->getDurability());
                        result.markArmorBroken();
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
                    }
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
    screen.addLine("Or perdu : " + std::to_string(result.getLostGold()) + " pièces");
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
    MenuScreen screen("MORT LÉTHALE", "death.lethal.corruption");
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

// EN: displayLethalSurvivalAnomaly declares or implements a focused behavior used by this module.
// FR: displayLethalSurvivalAnomaly déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalSurvivalAnomaly()
{
    MenuScreen screen("ANOMALIE DE SURVIE", "death.lethal.survival_anomaly");
    screen.addLine("La mort t'a trouvé.");
    screen.addLine("Puis elle a reculé.");
    screen.addLine("");
    screen.addLine("Quelqu'un, quelque part, a payé le prix à ta place.");
    screen.addLine("");
    screen.addLine("Tu ouvres les yeux avec 1 PV.");
    screen.addLine("Ton équipement a disparu.");
    screen.addLine("Ton inventaire n'est plus qu'un souvenir.");
    screen.addLine("");
    screen.addLine("Statut : anomalie de survie.");
    screen.addLine("");
    screen.addLine("Tes bénédictions se sont consumées.");
    screen.addLine("Mais ton nom existe encore.");
    screen.addLine("Mais le registre continue de te regarder.");
    screen.setDisplayOnlyInput("Anomalie de survie affichée sans choix joueur.");
    TerminalInterface::renderMenuScreen(screen, false);
}
