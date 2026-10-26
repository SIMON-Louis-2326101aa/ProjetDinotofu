#include "combat/group/TeamEmploymentOfferSystem.hpp"

#include "combat/group/TeamEmploymentSystem.hpp"
#include "combat/group/TeamEmploymentMissionSystem.hpp"
#include "entity/Player.hpp"
#include "world/City.hpp"

#include <algorithm>

TeamEmploymentOffer TeamEmploymentOfferSystem::localOfferFor(const Player& player)
{
    TeamEmploymentOffer offer;
    if (TeamEmploymentSystem::hasActiveContract(player))
    {
        return offer;
    }

    const City* city = City::findById(player.getCurrentCityId());
    if (city == nullptr)
    {
        return offer;
    }

    const int day = std::max(0, player.getWorldDaysElapsed());
    const int level = std::max(1, player.getLevel());
    const int rawCopper = 45 + level * 6 + std::max(1, city->getMinimumLevel()) * 4;

    offer.available = true;
    offer.organizationId = "field_team_" + city->getId();
    offer.organizationName = "Escouade locale affiliée à " + city->getGuildName();
    offer.leaderName = "Responsable de terrain — " + city->getName();
    offer.roleName = "membre de terrain sous contrat";
    offer.contractId = offer.organizationId + "_day_" + std::to_string(day);
    offer.salaryEconomyUnits = std::clamp((rawCopper + 9) / 10, 4, 25);
    offer.payIntervalDays = 7;
    offer.durationDays = 14;
    offer.description = "Contrat court : tu rejoins temporairement une équipe déjà dirigée. Tu reçois ses ordres de mission et son salaire, sans payer la masse salariale de l'escouade.";
    return offer;
}

bool TeamEmploymentOfferSystem::acceptLocalOffer(Player& player)
{
    const TeamEmploymentOffer offer = localOfferFor(player);
    if (!offer.available)
    {
        return false;
    }

    const bool started = TeamEmploymentSystem::startExternalContract(
        player,
        offer.contractId,
        offer.leaderName,
        offer.organizationId,
        offer.organizationName,
        offer.roleName,
        offer.salaryEconomyUnits,
        offer.payIntervalDays,
        offer.durationDays
    );
    if (!started) return false;

    // Un emploi réel commence avec un ordre réel : le joueur ne touche pas un salaire
    // tout en bénéficiant passivement d'une équipe sans aucune obligation de service.
    TeamEmploymentMissionSystem::assignCurrentMission(player);
    return true;
}
