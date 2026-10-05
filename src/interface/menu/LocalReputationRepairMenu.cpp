#include "interface/menu/LocalReputationRepairMenu.hpp"
#include "world/LocalReputationRepairSystem.hpp"
#include "economy/Money.hpp"
#include "world/LocalReputationSystem.hpp"
#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "core/Console.hpp"

void LocalReputationRepairMenu::open(Player& player)
{
    while (true)
    {
        const std::string cityId = player.getCurrentCityId();
        const LocalReputationResult reputation = LocalReputationSystem::evaluate(player, cityId);
        MenuScreen screen("MÉDIATION LOCALE", "local_reputation.repair");
        screen.addSubtitle("Réparer une relation locale par des actes visibles");
        screen.addLine("Ville : " + cityId + " | réputation : " + std::to_string(reputation.score) + " (" + reputation.label + ").");
        screen.addLine(reputation.reactionLine);
        screen.addLine("Ici, une mauvaise réputation peut être réparée. Elle n'est pas seulement transformée en taxe permanente.");
        screen.addBackOption("Retour", "local_reputation.repair.back");

        if (reputation.score < 0)
        {
            const int cost = LocalReputationRepairSystem::fineCostEconomyUnits(reputation.score);
            const int gain = LocalReputationRepairSystem::fineReputationGain(reputation.score);
            screen.addOption(
                1,
                "Régler une amende réparatrice",
                Money::formatEconomyUnits(cost) + " | réparation estimée : +" + std::to_string(gain) + " réputation. Le paiement ne garantit pas une bonne réputation instantanée.",
                true,
                "local_reputation.repair.fine"
            );
            screen.addOption(
                2,
                "Effectuer un service communautaire",
                "Prend 2 segments de journée | +3 réputation | limité à une fois par jour dans cette ville.",
                true,
                "local_reputation.repair.service"
            );
        }
        else
        {
            screen.addLine("Aucune réhabilitation n'est nécessaire actuellement. Continue surtout à laisser des traces positives réelles dans cette ville.");
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une voie de réparation locale.");
        Console::clear();
        if (choice == 0) return;
        if (choice == 1)
        {
            MessageScreen::show("AMENDE RÉPARATRICE", "local_reputation.repair.fine.result", LocalReputationRepairSystem::payFine(player, cityId), false);
        }
        else if (choice == 2)
        {
            MessageScreen::show("SERVICE COMMUNAUTAIRE", "local_reputation.repair.service.result", LocalReputationRepairSystem::performCommunityService(player, cityId), false);
        }
    }
}
