#include "interface/menu/shop/ShopServiceSupport.hpp"

#include "core/Console.hpp"
#include "economy/Money.hpp"
#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "world/LocalReputationSystem.hpp"

namespace
{
    std::string buildLocalReputationLineForPlayer(const Player& player)
    {
        const LocalReputationResult summary = LocalReputationSystem::evaluate(player, player.getCurrentCityId());
        std::string line = "Réputation locale : " + summary.label
            + " (score " + std::to_string(summary.score)
            + ", services réussis " + std::to_string(summary.successfulPersonalServices)
            + ", incidents " + std::to_string(summary.failedPersonalServices + summary.warningNotes) + ")";

        if (summary.discountPercent > 0)
        {
            line += " | avantage services/auberge/transport : -" + std::to_string(summary.discountPercent) + "%";
        }
        else if (summary.surchargePercent > 0)
        {
            line += " | méfiance locale : +" + std::to_string(summary.surchargePercent) + "% sur les services, impact réduit ailleurs";
        }

        line += ". " + summary.reactionLine;
        return line;
    }

    void advanceLocalServiceTime(Player& player, int units, std::vector<std::string>& resultLines)
    {
        const int beforeDay = player.getWorldDaysElapsed();
        const int beforeProgress = player.getWorldDayProgressUnits();
        player.advanceWorldDayUnits(units);

        resultLines.push_back("Temps écoulé : +" + std::to_string(units) + " segment(s) de journée.");
        resultLines.push_back(player.formatWorldTimeChange(beforeDay, beforeProgress));
        std::vector<std::string> timeReportLines = player.consumeWorldTimeReportLines();
        resultLines.insert(resultLines.end(), timeReportLines.begin(), timeReportLines.end());

        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
        if (expired > 0)
        {
            resultLines.push_back("Attention : " + std::to_string(expired) + " quête(s) ont dépassé leur date limite pendant ce service.");
        }
    }
}

namespace ShopServiceSupport
{

    std::string localReputationLineForPlayer(const Player& player)
    {
        return buildLocalReputationLineForPlayer(player);
    }

    bool askShopConfirmation(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::string& confirmLabel,
        const std::string& cancelLabel,
        const std::string& actionPrefix
    )
    {
        MenuScreen screen(title, screenId);
        screen.setChoiceInput("Choisis 1 pour confirmer ou 2 pour annuler.");
        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }
        screen.addOption(1, confirmLabel, "Valider l'action affichée.", true, actionPrefix + ".confirm");
        screen.addOption(2, cancelLabel, "Revenir sans rien changer.", true, actionPrefix + ".cancel");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix refusé : utilise 1 pour confirmer ou 2 pour annuler."
        );
        return choice == 1;
    }

    void showShopResult(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        Console::clear();
        MessageScreen::show(title, screenId, lines);
    }

    bool payServiceWithVoucherOrGold(
        Player& player,
        const std::string& voucherId,
        const std::string& voucherName,
        int fallbackPrice,
        std::vector<std::string>& resultLines
    )
    {
        if (player.getInventory().countMaterialById(voucherId) > 0)
        {
            player.getInventory().removeMaterialQuantityById(voucherId, 1);
            resultLines.push_back("Justificatif utilisé : " + voucherName + " x1.");
            return true;
        }

        if (!player.getInventory().spendGold(fallbackPrice))
        {
            resultLines.push_back("Paiement refusé : il manque " + Money::formatGoldWithRaw(fallbackPrice) + ".");
            return false;
        }

        resultLines.push_back("Paiement effectué : " + Money::formatGoldWithRaw(fallbackPrice) + ".");
        return true;
    }

    void showLocalServiceResult(
        const std::string& title,
        const std::string& screenId,
        Player& player,
        std::vector<std::string> resultLines,
        int timeUnits
    )
    {
        if (timeUnits > 0)
        {
            advanceLocalServiceTime(player, timeUnits, resultLines);
        }
        resultLines.push_back(buildLocalReputationLineForPlayer(player));
        showShopResult(title, screenId, resultLines);
    }
}
