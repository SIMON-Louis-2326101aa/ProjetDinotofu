// EN: DeathPenaltySystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DeathPenaltySystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file prepares future death penalties for non-lethal runs.
// Français : Ce fichier prépare les futures pénalités de mort pour les parties non létales.

#include "progression/death/DeathPenaltySystem.hpp"

#include "item/Inventory.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"
#include "progression/DifficultyRules.hpp"

#include <iostream>

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
    std::cout << "========== CONSÉQUENCES DE LA MORT ==========" << std::endl;
    std::cout << "Ton personnage survit, mais la mort ne repart jamais les mains vides." << std::endl;
    std::cout << std::endl;
    std::cout << "Or perdu : " << result.getLostGold() << " pièces" << std::endl;
    std::cout << "Expérience perdue : " << result.getLostExperience() << std::endl;
    std::cout << "Consommables perdus : " << result.getLostConsumables() << std::endl;
    std::cout << "Durabilité perdue sur l'arme équipée : " << result.getWeaponDurabilityLost() << std::endl;
    std::cout << "Durabilité perdue sur l'armure équipée : " << result.getArmorDurabilityLost() << std::endl;

    if (result.wasWeaponBroken())
    {
        std::cout << "Arme équipée : brisée par le choc." << std::endl;
    }

    if (result.wasArmorBroken())
    {
        std::cout << "Armure équipée : brisée par le choc." << std::endl;
    }

    if (result.wasWeaponIrreparable())
    {
        std::cout << "Arme équipée : détruite au point de devenir irréparable." << std::endl;
    }

    if (result.wasArmorIrreparable())
    {
        std::cout << "Armure équipée : détruite au point de devenir irréparable." << std::endl;
    }

    if (result.wasWeaponStolen())
    {
        std::cout << "Arme équipée : volée pendant l'effondrement du combat." << std::endl;
    }

    if (result.wasArmorStolen())
    {
        std::cout << "Armure équipée : volée pendant l'effondrement du combat." << std::endl;
    }

    if (result.getRecoveredMaterialFragments() > 0)
    {
        std::cout << "Fragments récupérables générés : "
                  << result.getRecoveredMaterialFragments()
                  << std::endl;
    }

    std::cout << "Objets liés au corps, à l'âme ou au personnage : protégés par règle spéciale future." << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::endl;
}

// EN: displayLethalDeathCorruption declares or implements a focused behavior used by this module.
// FR: displayLethalDeathCorruption déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalDeathCorruption()
{
    std::cout << "Morts du personnage : [STATISTIQUE CORROMPUE]" << std::endl;
    std::cout << "Vous ne deviez pas mourir." << std::endl;
    std::cout << "Statut : personnage supprimé du registre des vivants." << std::endl;
    std::cout << "Connais-tu quelqu'un capable d'échapper à la mort ? Moi non..." << std::endl;
    std::cout << std::endl;
}

// EN: displayLethalCurrentDeathStatistic declares or implements a focused behavior used by this module.
// FR: displayLethalCurrentDeathStatistic déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalCurrentDeathStatistic()
{
    std::cout << "Morts du personnage : [STATISTIQUE CORROMPUE]" << std::endl;
    std::cout << "But de mission : survivre." << std::endl;
    std::cout << std::endl;
}

// EN: displayLethalSurvivalAnomaly declares or implements a focused behavior used by this module.
// FR: displayLethalSurvivalAnomaly déclare ou implémente un comportement précis utilisé par ce module.
void DeathPenaltySystem::displayLethalSurvivalAnomaly()
{
    std::cout << "La mort t'a trouvé." << std::endl;
    std::cout << "Puis elle a reculé." << std::endl;
    std::cout << std::endl;
    std::cout << "Quelqu'un, quelque part, a payé le prix à ta place." << std::endl;
    std::cout << std::endl;
    std::cout << "Tu ouvres les yeux avec 1 PV." << std::endl;
    std::cout << "Ton équipement a disparu." << std::endl;
    std::cout << "Ton inventaire n'est plus qu'un souvenir." << std::endl;
    std::cout << std::endl;
    std::cout << "Statut : anomalie de survie." << std::endl;
    std::cout << std::endl;
    std::cout << "Tes bénédictions se sont consumées." << std::endl;
    std::cout << "Mais ton nom existe encore." << std::endl;
    std::cout << "Pour l'instant." << std::endl;
    std::cout << std::endl;
}
