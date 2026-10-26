#include "world/home/PlayerHomeSystem.hpp"

#include "entity/Player.hpp"

#include <algorithm>

PlayerHomePurchasePlan PlayerHomeSystem::planPurchase(const Player& player, const std::string& cityId, bool buyOutright)
{
    PlayerHomePurchasePlan plan;
    if (cityId.empty())
    {
        plan.reason = "Une maison doit être rattachée à une ville réelle.";
        return plan;
    }
    if (player.getHomeState().owned)
    {
        plan.reason = "Ce personnage possède déjà sa maison principale dans une autre ville ou celle-ci.";
        return plan;
    }

    plan.valid = true;
    plan.fullyPurchased = buyOutright;
    if (buyOutright)
    {
        plan.upfrontCost = 24000;
        plan.recurringRent = 0;
        plan.reason = "Achat définitif très coûteux : pas de loyer récurrent.";
    }
    else
    {
        plan.upfrontCost = 4200;
        plan.recurringRent = 280;
        plan.reason = "Accès moins cher, mais loyer récurrent pour conserver les services du logement.";
    }
    return plan;
}

bool PlayerHomeSystem::registerPurchase(Player& player, const std::string& cityId, bool buyOutright, int currentDay)
{
    const PlayerHomePurchasePlan plan = planPurchase(player, cityId, buyOutright);
    if (!plan.valid) return false;
    if (!player.getInventory().spendCopper(plan.upfrontCost)) return false;

    PlayerHomeState& home = player.getMutableHomeState();
    home.owned = true;
    home.cityId = cityId;
    home.fullyPurchased = buyOutright;
    home.rentAmount = plan.recurringRent;
    home.nextRentDueDay = buyOutright ? -1 : std::max(0, currentDay) + 7;
    player.recordCanonicalEvent("logement_principal", cityId, buyOutright ? "Maison achetée définitivement" : "Maison avec loyer", 1);
    return true;
}


bool PlayerHomeSystem::isRentDue(const Player& player, int currentDay)
{
    const PlayerHomeState& home = player.getHomeState();
    return home.owned && !home.fullyPurchased && home.rentAmount > 0
        && home.nextRentDueDay >= 0 && currentDay >= home.nextRentDueDay;
}

bool PlayerHomeSystem::servicesActive(const Player& player, int currentDay)
{
    const PlayerHomeState& home = player.getHomeState();
    if (!home.owned) return false;
    if (home.fullyPurchased) return true;
    return !isRentDue(player, currentDay);
}

bool PlayerHomeSystem::payRent(Player& player, int currentDay)
{
    PlayerHomeState& home = player.getMutableHomeState();
    if (!home.owned || home.fullyPurchased || home.rentAmount <= 0) return false;
    if (!isRentDue(player, currentDay)) return false;
    if (!player.getInventory().spendCopper(home.rentAmount)) return false;

    home.nextRentDueDay = std::max(0, currentDay) + 7;
    player.recordCanonicalEvent("logement_principal", home.cityId, "Loyer du logement principal payé", 1);
    player.recordCanonicalEvent("loyers_logement_payes", home.cityId, "Loyer du logement principal payé", 1);
    return true;
}

bool PlayerHomeSystem::canHouseHestia(const Player& player)
{
    return player.getHomeState().owned;
}

std::vector<std::string> PlayerHomeSystem::describeResidents(const Player& player)
{
    std::vector<std::string> lines;
    const PlayerHomeState& home = player.getHomeState();
    if (!home.owned) return {"Aucun logement principal."};

    lines.push_back("Maison principale : " + home.cityId + (home.fullyPurchased ? " (achetée)" : " (loyer actif)"));
    if (!home.npcSpouseName.empty()) lines.push_back("Conjoint PNJ : " + home.npcSpouseName + ".");
    if (!home.playerSpouseId.empty()) lines.push_back("Conjoint joueur : " + home.playerSpouseId + ".");
    if (player.getHestiaStoryState().rescued && player.getHestiaStoryState().residence == "player_home")
        lines.push_back("Hestia vit ici aussi : sa présence n'entre pas en conflit avec celle d'un conjoint.");
    return lines;
}

bool PlayerHomeSystem::registerNpcSpouse(Player& player, const std::string& npcName, bool specialRingQuestCompleted)
{
    if (!specialRingQuestCompleted || npcName.empty()) return false;
    PlayerHomeState& home = player.getMutableHomeState();
    if (!home.owned || !home.npcSpouseName.empty()) return false;
    home.npcSpouseName = npcName;
    player.recordCanonicalEvent("mariage_pnj", npcName, "Mariage avec un PNJ", 1);
    return true;
}

bool PlayerHomeSystem::registerPlayerSpouse(Player& player, const std::string& playerId, bool specialRingQuestCompleted)
{
    if (!specialRingQuestCompleted || playerId.empty()) return false;
    PlayerHomeState& home = player.getMutableHomeState();
    if (home.playerMarriageSlotUsed || !home.playerSpouseId.empty()) return false;
    home.playerMarriageSlotUsed = true;
    home.playerSpouseId = playerId;
    player.recordCanonicalEvent("mariage_joueur", playerId, "Mariage entre joueurs", 1);
    return true;
}
