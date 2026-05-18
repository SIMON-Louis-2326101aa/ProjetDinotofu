// English: This file prepares future death penalties for non-lethal runs.
// Français : Ce fichier prépare les futures pénalités de mort pour les parties non létales.

#include "progression/death/DeathPenaltySystem.hpp"

#include "item/Inventory.hpp"
#include "progression/DifficultyRules.hpp"

#include <iostream>

DeathPenaltyResult DeathPenaltySystem::applyNonLethalDeathPenalty(
    Player& player,
    DifficultyMode difficulty,
    Random& random
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
    std::cout << "Armes, armures, objets uniques et reliques : conservés." << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::endl;
}

void DeathPenaltySystem::displayLethalDeathCorruption()
{
    std::cout << "Morts du personnage : [STATISTIQUE CORROMPUE]" << std::endl;
    std::cout << "Vous ne deviez pas mourir." << std::endl;
    std::cout << "Statut : personnage supprimé du registre des vivants." << std::endl;
    std::cout << "Connais-tu quelqu'un capable d'échapper à la mort ? Moi non..." << std::endl;
    std::cout << std::endl;
}

void DeathPenaltySystem::displayLethalCurrentDeathStatistic()
{
    std::cout << "Morts du personnage : [STATISTIQUE CORROMPUE]" << std::endl;
    std::cout << "But de mission : survivre." << std::endl;
    std::cout << std::endl;
}
