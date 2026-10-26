// EN: Guild-specific money-management terminal UI extracted from QuestMenu.
// FR: Interface terminale de gestion financière de guilde extraite de QuestMenu.

#include "interface/menu/quest/GuildFinanceMenu.hpp"

#include "core/Console.hpp"
#include "economy/GuildFinanceSystem.hpp"
#include "economy/Money.hpp"
#include "item/inventory/CarryLoadSystem.hpp"
#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

static void openGuildCurrencyExchange(Player& player)
{
    auto coinLabel = [](CoinType type) {
        return Money::coinName(type) + " (" + Money::coinAbbreviation(type) + ")";
    };
    const std::vector<CoinType> allTypes = {
        CoinType::Copper,
        CoinType::Iron,
        CoinType::Electrum,
        CoinType::Gold,
        CoinType::Platinum
    };

    auto conversionTerms = [](CoinType source, CoinType target) {
        const long long sourceValue = Money::coinValueInCopper(source);
        const long long targetValue = Money::coinValueInCopper(target);
        const long long sourcePerLot = sourceValue < targetValue ? targetValue / sourceValue : 1;
        const long long targetPerLot = sourceValue > targetValue ? sourceValue / targetValue : 1;
        return std::pair<long long, long long>{sourcePerLot, targetPerLot};
    };

    while (true)
    {
        MenuScreen screen("BUREAU DE CHANGE DE LA GUILDE", "quest.guild.currency_exchange");
        screen.addSubtitle("Organiser volontairement les cinq piles physiques de ta bourse");
        screen.addLine("Bourse actuelle : " + player.getInventory().getWalletLine() + ".");
        screen.addLine("Valeur totale exacte : " + player.getInventory().getWalletTotalLine() + ".");
        screen.addLine(Money::coinScaleText() + ".");
        screen.addLine("Le change ne crée ni ne détruit d'argent. Il modifie seulement les pièces physiques que tu portes.");
        screen.addLine("La forme de la bourse peut aussi changer l'impression sociale qu'elle donne : beaucoup de cuivre et une pièce d'or ne racontent pas la même chose.");
        screen.addBackOption("Retour à la guilde", "quest.guild.currency_exchange.back");
        screen.addOption(1, "Conversion personnalisée", "Choisir la pièce de départ, la pièce d'arrivée, puis le nombre de conversions.", true, "quest.guild.currency_exchange.custom");
        screen.addOption(2, "Tout vers les pièces les plus élevées", "Créer une bourse compacte : PP d'abord, puis PO, PE, PF et le reste exact en PC.", player.getInventory().getTotalCopper() > 0, "quest.guild.currency_exchange.compact");
        screen.addOption(3, "Tout vers la pièce la plus faible", "Transformer toute la valeur en PC physiques. Très précis, très encombrant, et socialement assez modeste.", player.getInventory().getTotalCopper() > 0, "quest.guild.currency_exchange.copper_all");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une opération de change.");
        Console::clear();
        if (choice == 0) return;

        if (choice == 2 || choice == 3)
        {
            const long long before = player.getInventory().getTotalCopper();
            const int coinLoadBefore = CarryLoadSystem::coinLoad(player.getInventory());
            const std::string oldWallet = player.getInventory().getWalletLine();
            const bool success = choice == 2
                ? player.getInventory().compactCoinsToHighest()
                : player.getInventory().flattenCoinsToCopper();
            if (success)
            {
                player.refreshCurrencyTitles();
                const int coinLoadAfter = CarryLoadSystem::coinLoad(player.getInventory());
                if (choice == 2 && coinLoadBefore >= 10 && coinLoadAfter * 2 <= coinLoadBefore)
                {
                    player.grantTitle("Ça pèse moins comme ça");
                }
                player.refreshCarryAndStorageTitles();
            }
            const long long after = player.getInventory().getTotalCopper();

            MessageScreen::show(
                success && before == after ? "CHANGE EFFECTUÉ" : "ERREUR DE CHANGE",
                choice == 2 ? "quest.guild.currency_exchange.compact.done" : "quest.guild.currency_exchange.copper_all.done",
                {
                    choice == 2
                        ? "La guilde regroupe chaque tranche possible vers la pièce la plus élevée sans perdre le moindre PC."
                        : "La guilde casse toute la bourse jusqu'au cuivre : chaque PC devient une pièce physique.",
                    "Avant : " + oldWallet + ".",
                    "Après : " + player.getInventory().getWalletLine() + ".",
                    "Valeur exacte : " + player.getInventory().getWalletTotalLine() + ".",
                    "Valeur conservée : " + std::string(before == after ? "oui" : "ERREUR") + "."
                },
                false
            );
            Console::clear();
            continue;
        }

        if (choice != 1)
        {
            continue;
        }

        MenuScreen sourceScreen("CONVERSION PERSONNALISÉE — SOURCE", "quest.guild.currency_exchange.custom.source");
        sourceScreen.addLine("Choisis la pile physique que tu veux donner au guichet.");
        sourceScreen.addBackOption("Retour", "quest.guild.currency_exchange.custom.source.back");
        for (std::size_t i = 0; i < allTypes.size(); ++i)
        {
            const CoinType type = allTypes[i];
            const long long count = player.getInventory().getCoinCount(type);
            sourceScreen.addOption(
                static_cast<int>(i + 1),
                coinLabel(type),
                "En bourse : " + std::to_string(count) + " " + Money::coinAbbreviation(type) + ".",
                count > 0,
                "quest.guild.currency_exchange.custom.source." + Money::coinAbbreviation(type)
            );
        }

        const int sourceChoice = TerminalInterface::askMenuChoiceFromOptions(sourceScreen, "Choisis une pile présente dans ta bourse.");
        Console::clear();
        if (sourceChoice <= 0 || static_cast<std::size_t>(sourceChoice) > allTypes.size()) continue;
        const CoinType sourceType = allTypes[static_cast<std::size_t>(sourceChoice - 1)];
        const long long sourceCount = player.getInventory().getCoinCount(sourceType);

        MenuScreen targetScreen("CONVERSION PERSONNALISÉE — DESTINATION", "quest.guild.currency_exchange.custom.target");
        targetScreen.addLine("Source : " + coinLabel(sourceType) + " | disponible : " + std::to_string(sourceCount) + " " + Money::coinAbbreviation(sourceType) + ".");
        targetScreen.addLine("Choisis maintenant la pièce physique que tu veux recevoir.");
        targetScreen.addBackOption("Retour", "quest.guild.currency_exchange.custom.target.back");

        int targetOption = 1;
        std::vector<CoinType> targetTypes;
        for (CoinType targetType : allTypes)
        {
            if (targetType == sourceType)
            {
                continue;
            }

            const auto [sourcePerLot, targetPerLot] = conversionTerms(sourceType, targetType);
            const long long maxLots = sourcePerLot > 0 ? sourceCount / sourcePerLot : 0;
            targetTypes.push_back(targetType);
            targetScreen.addOption(
                targetOption++,
                coinLabel(sourceType) + " -> " + coinLabel(targetType),
                "1 conversion : " + std::to_string(sourcePerLot) + " " + Money::coinAbbreviation(sourceType)
                    + " -> " + std::to_string(targetPerLot) + " " + Money::coinAbbreviation(targetType)
                    + " | maximum possible : " + std::to_string(maxLots) + ".",
                maxLots > 0,
                "quest.guild.currency_exchange.custom.target." + Money::coinAbbreviation(targetType)
            );
        }

        const int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une destination possible.");
        Console::clear();
        if (targetChoice <= 0 || static_cast<std::size_t>(targetChoice) > targetTypes.size()) continue;
        const CoinType targetType = targetTypes[static_cast<std::size_t>(targetChoice - 1)];
        const auto [sourcePerLot, targetPerLot] = conversionTerms(sourceType, targetType);
        const long long maxLots = sourceCount / sourcePerLot;
        const int maxQuantity = static_cast<int>(std::min(maxLots, 1000000LL));
        if (maxQuantity <= 0) continue;

        const int lotCount = MessageScreen::askQuantity(
            "NOMBRE DE CONVERSIONS",
            "quest.guild.currency_exchange.custom.quantity",
            {
                "Conversion choisie : " + std::to_string(sourcePerLot) + " " + Money::coinAbbreviation(sourceType)
                    + " -> " + std::to_string(targetPerLot) + " " + Money::coinAbbreviation(targetType) + ".",
                "Pièces source disponibles : " + std::to_string(sourceCount) + " " + Money::coinAbbreviation(sourceType) + ".",
                "Maximum possible : " + std::to_string(maxLots) + " conversion(s).",
                "Pour le maximum, le guichet utiliserait " + std::to_string(sourcePerLot * maxLots) + " " + Money::coinAbbreviation(sourceType)
                    + " et remettrait " + std::to_string(targetPerLot * maxLots) + " " + Money::coinAbbreviation(targetType) + "."
            },
            1,
            maxQuantity,
            "Le nombre doit rester entre 1 et le maximum convertible affiché."
        );
        Console::clear();

        const long long before = player.getInventory().getTotalCopper();
        if (player.getInventory().convertCoinLots(sourceType, targetType, lotCount))
        {
            player.refreshCurrencyTitles();
            const long long after = player.getInventory().getTotalCopper();
            MessageScreen::show(
                "CHANGE EFFECTUÉ",
                "quest.guild.currency_exchange.custom.done",
                {
                    std::to_string(sourcePerLot * static_cast<long long>(lotCount)) + " " + Money::coinAbbreviation(sourceType)
                        + " échangé(s) contre " + std::to_string(targetPerLot * static_cast<long long>(lotCount)) + " " + Money::coinAbbreviation(targetType) + ".",
                    "Bourse : " + player.getInventory().getWalletLine() + ".",
                    "Valeur exacte : " + player.getInventory().getWalletTotalLine() + ".",
                    "Valeur conservée : " + std::string(before == after ? "oui" : "ERREUR") + "."
                },
                false
            );
            Console::clear();
        }
    }
}

static void openGuildMembershipFinance(Player& player)
{
    while (true)
    {
        const GuildFinanceOverview overview = GuildFinanceSystem::overview(player);
        MenuScreen screen("ADHÉSION DE GUILDE", "quest.guild.money.membership");
        screen.addLine("Adhésion de référence : " + GuildFinanceSystem::membershipName() + ".");
        screen.addLine("Durée d'une période : 7 jours.");
        screen.addLine("Prix : " + Money::formatEconomyUnits(GuildFinanceSystem::membershipRenewalPriceEconomyUnits()) + ".");
        screen.addLine("Cette cotisation existait déjà pour les petits services de guilde ; elle ouvre désormais aussi le découvert réseau et les prêts selon le dossier financier.");
        if (overview.member)
        {
            screen.addLine("Statut : ADHÉRENT.");
            if (overview.membershipExpiresAtDay >= 0)
            {
                screen.addLine("Période active jusqu'à la fin du jour " + std::to_string(overview.membershipExpiresAtDay + 1) + ".");
            }
            screen.addLine(std::string("Renouvellement automatique : ") + (overview.cancellationRequested ? "annulation demandée" : "actif si la bourse peut payer à l'échéance") + ".");
        }
        else
        {
            screen.addLine("Statut : NON ADHÉRENT.");
        }
        screen.addLine("Avantages financiers : découvert réseau et prêts de guilde selon le dossier, sans jamais rendre la bourse physique négative.");
        screen.addBackOption("Retour à la gestion de l'argent", "quest.guild.money.membership.back");
        screen.addOption(
            1,
            overview.member ? "Renouveler l'adhésion" : "Prendre l'adhésion",
            overview.member ? "Repayer une période de 7 jours à partir d'aujourd'hui." : "Payer une première période de 7 jours.",
            player.hasTitle("Aventurier"),
            "quest.guild.money.membership.activate"
        );
        screen.addOption(
            2,
            "Demander l'annulation",
            "La période déjà payée reste active jusqu'à son terme ; aucun remboursement rétroactif.",
            overview.member && !overview.cancellationRequested,
            "quest.guild.money.membership.cancel"
        );

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action d'adhésion.");
        Console::clear();
        if (choice == 0) return;

        if (choice == 1)
        {
            const int price = GuildFinanceSystem::membershipRenewalPriceEconomyUnits();
            if (!player.hasTitle("Aventurier"))
            {
                MessageScreen::show("ADHÉSION IMPOSSIBLE", "quest.guild.money.membership.no_adventurer", {"Il faut d'abord être inscrit comme Aventurier auprès de la guilde."}, false);
                continue;
            }
            if (!player.getInventory().spendEconomyUnits(price))
            {
                MessageScreen::show(
                    "ADHÉSION REFUSÉE",
                    "quest.guild.money.membership.no_money",
                    {
                        "Coût : " + Money::formatEconomyUnits(price) + ".",
                        "Bourse actuelle : " + player.getInventory().getWalletLine() + ".",
                        "La cotisation ne crée jamais de découvert automatiquement."
                    },
                    false
                );
                continue;
            }

            player.activateLocalSubscription(
                GuildFinanceSystem::membershipId(),
                GuildFinanceSystem::membershipName(),
                7,
                price
            );
            player.recordCanonicalEvent("guild_finance_membership", "standard", "Adhésion financière de guilde activée ou renouvelée", 1);
            MessageScreen::show(
                "ADHÉSION VALIDÉE",
                "quest.guild.money.membership.success",
                {
                    "Adhésion active : " + GuildFinanceSystem::membershipName() + ".",
                    "Fin de période : fin du jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(GuildFinanceSystem::membershipId()) + 1) + ".",
                    "Les anciens avantages de la cotisation restent valables ; découvert et prêts utilisent maintenant ce même statut d'adhérent."
                },
                false
            );
        }
        else if (choice == 2)
        {
            if (player.requestLocalSubscriptionCancellation(GuildFinanceSystem::membershipId()))
            {
                MessageScreen::show(
                    "ANNULATION ENREGISTRÉE",
                    "quest.guild.money.membership.cancelled",
                    {
                        "L'adhésion reste active jusqu'à la fin de la période déjà payée.",
                        "À son expiration, aucun nouveau découvert ni prêt ne pourra être ouvert sans nouvelle adhésion.",
                        "Les dettes éventuellement existantes ne disparaîtront évidemment pas avec l'adhésion."
                    },
                    false
                );
            }
        }
    }
}

static void openGuildOverdraftManagement(Player& player)
{
    while (true)
    {
        const GuildFinanceOverview overview = GuildFinanceSystem::overview(player);
        MenuScreen screen("DÉCOUVERT DE GUILDE", "quest.guild.money.overdraft");
        screen.addLine("Découvert utilisé : " + Money::formatCopper(overview.overdraftUsedCopper) + ".");
        screen.addLine("Plafond de découvert : " + Money::formatCopper(overview.overdraftCeilingCopper) + ".");
        screen.addLine("Marge restante : " + Money::formatCopper(overview.overdraftRemainingCopper) + ".");
        screen.addLine("Confiance financière : " + std::to_string(overview.trustScore) + "/100 — " + overview.trustLabel + ".");
        if (!overview.member)
        {
            screen.addLine("Adhésion inactive : la dette existante reste remboursable, mais aucun nouveau crédit ne peut être utilisé.");
        }
        else
        {
            screen.addLine("Le découvert est un crédit réseau : il peut compléter un paiement chez un service affilié, mais ne peut jamais être retiré en espèces.");
        }
        screen.addBackOption("Retour à la gestion de l'argent", "quest.guild.money.overdraft.back");
        screen.addOption(1, "Rembourser depuis la bourse", "Utiliser des pièces réellement portées pour réduire le découvert.", overview.overdraftUsedCopper > 0 && player.getInventory().getTotalCopper() > 0, "quest.guild.money.overdraft.repay.wallet");
        screen.addOption(2, "Rembourser depuis le compte de guilde", "Affecter une partie du solde déjà déposé à la dette, sans téléporter de pièces.", overview.overdraftUsedCopper > 0 && overview.depositedCopper > 0, "quest.guild.money.overdraft.repay.account");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une opération sur le découvert.");
        Console::clear();
        if (choice == 0) return;

        if (choice == 1)
        {
            const long long maximum = std::min(overview.overdraftUsedCopper, player.getInventory().getTotalCopper());
            if (maximum <= 0) continue;
            const int amount = MessageScreen::askQuantity(
                "REMBOURSER LE DÉCOUVERT",
                "quest.guild.money.overdraft.repay.wallet.quantity",
                {
                    "Dette actuelle : " + Money::formatCopper(overview.overdraftUsedCopper) + ".",
                    "Bourse : " + player.getInventory().getWalletLine() + ".",
                    "Aucun remboursement ne peut dépasser la dette réellement utilisée."
                },
                1,
                static_cast<int>(std::min<long long>(maximum, 1000000000LL)),
                "Le montant doit rester entre 1 PC et le maximum remboursable."
            );
            std::string reason;
            if (GuildFinanceSystem::repayOverdraftFromWallet(player, amount, &reason))
            {
                MessageScreen::show("REMBOURSEMENT EFFECTUÉ", "quest.guild.money.overdraft.repay.wallet.done", {
                    "Remboursé depuis la bourse : " + Money::formatCopper(amount) + ".",
                    "Découvert restant : " + Money::formatCopper(GuildFinanceSystem::overdraftUsedCopper(player)) + "."
                }, false);
            }
            else
            {
                MessageScreen::show("REMBOURSEMENT REFUSÉ", "quest.guild.money.overdraft.repay.wallet.failed", {reason}, false);
            }
        }
        else if (choice == 2)
        {
            const long long maximum = std::min(overview.overdraftUsedCopper, overview.depositedCopper);
            if (maximum <= 0) continue;
            const int amount = MessageScreen::askQuantity(
                "REMBOURSER DEPUIS LE COMPTE",
                "quest.guild.money.overdraft.repay.account.quantity",
                {
                    "Dette actuelle : " + Money::formatCopper(overview.overdraftUsedCopper) + ".",
                    "Solde de guilde : " + Money::formatCopper(overview.depositedCopper) + ".",
                    "Cette opération réduit directement le solde déposé et la dette."
                },
                1,
                static_cast<int>(std::min<long long>(maximum, 1000000000LL)),
                "Le montant doit rester entre 1 PC et le maximum remboursable."
            );
            std::string reason;
            if (GuildFinanceSystem::repayOverdraftFromGuildBalance(player, amount, &reason))
            {
                MessageScreen::show("REMBOURSEMENT EFFECTUÉ", "quest.guild.money.overdraft.repay.account.done", {
                    "Remboursé depuis le compte de guilde : " + Money::formatCopper(amount) + ".",
                    "Solde de guilde restant : " + Money::formatCopper(GuildFinanceSystem::depositedBalanceCopper(player)) + ".",
                    "Découvert restant : " + Money::formatCopper(GuildFinanceSystem::overdraftUsedCopper(player)) + "."
                }, false);
            }
            else
            {
                MessageScreen::show("REMBOURSEMENT REFUSÉ", "quest.guild.money.overdraft.repay.account.failed", {reason}, false);
            }
        }
    }
}

static long long guildLoanSignedTotal(long long principal, const GuildLoanTerms& terms)
{
    const long long fee = (principal * static_cast<long long>(terms.effectiveRatePercent) + 99LL) / 100LL;
    return principal + fee;
}

static void openGuildLoanManagement(Player& player)
{
    while (true)
    {
        const GuildFinanceOverview overview = GuildFinanceSystem::overview(player);
        const GuildLoanState loan = GuildFinanceSystem::activeLoan(player);
        const long long maximumPrincipal = GuildFinanceSystem::loanMaximumPrincipalCopper(player);

        MenuScreen screen("PRÊTS DE GUILDE", "quest.guild.money.loans");
        screen.addLine("Confiance financière : " + std::to_string(overview.trustScore) + "/100 — " + overview.trustLabel + ".");
        screen.addLine("Plafond actuel de capital : " + Money::formatCopper(maximumPrincipal) + ". Maximum théorique du système : 5 PO.");
        screen.addLine("Taux signés : 1 sem. 12 % | 2 sem. 14 % | 3 sem. 16 % + 2 % durée longue | 4 sem. 18 % + 2 % | 6 sem. 21 % + 3 % | 8 sem. 24 % + 4 %.");
        screen.addLine("L'intérêt signé reste fixe même si tu rembourses tôt. Aucun intérêt composé automatique n'est ajouté.");
        screen.addLine("Les prolongations préventives coûtent +3 % / +5 % / +7 % pour +1 / +2 / +3 semaines, avec plafond global de coût à 30 % du capital initial.");

        if (loan.active)
        {
            screen.addLine("PRÊT ACTIF — capital reçu : " + Money::formatCopper(loan.principalCopper) + ".");
            screen.addLine("Coût contractuel actuel : " + std::to_string(loan.totalCostRatePercent) + " % | total dû : " + Money::formatCopper(loan.totalDueCopper) + ".");
            screen.addLine("Déjà remboursé : " + Money::formatCopper(loan.repaidCopper) + " | reste dû : " + Money::formatCopper(loan.remainingCopper) + ".");
            screen.addLine("Échéance : fin du jour " + std::to_string(loan.dueDay + 1) + " | " + std::string(loan.overdue ? "EN RETARD" : "dans les temps") + ".");
            screen.addLine(std::string("Mode : ") + (loan.automaticRepayment ? "automatique à l'échéance depuis le compte de guilde" : "manuel") + ".");
            if (loan.overdue)
            {
                const GuildDebtSituation debtSituation = GuildFinanceSystem::assessDebtSituation(player);
                screen.addLine("Situation de retard : " + debtSituation.statusLabel + ".");
                screen.addLine("Nouveau crédit : suspendu jusqu'à régularisation. La guilde ne conclut pas automatiquement à la mauvaise foi.");
            }
        }
        else if (!overview.member)
        {
            screen.addLine("Aucun nouveau prêt : l'adhésion de guilde est inactive.");
        }
        else if (maximumPrincipal <= 0)
        {
            screen.addLine("Le dossier est encore trop jeune pour un prêt. Une confiance d'au moins 55/100 est requise pour la durée la plus courte.");
        }
        else
        {
            screen.addLine("Aucun prêt actif. La guilde peut avancer de l'argent physique dans la limite du dossier.");
        }

        screen.addBackOption("Retour à la gestion de l'argent", "quest.guild.money.loans.back");
        screen.addOption(1, "Demander un nouveau prêt", "Recevoir un capital physique après validation du montant, de la durée et du mode de remboursement.", !loan.active && overview.member && maximumPrincipal > 0, "quest.guild.money.loans.open");
        screen.addOption(2, "Rembourser depuis la bourse", "Remboursement anticipé autorisé ; le coût signé ne diminue pas.", loan.active && player.getInventory().getTotalCopper() > 0, "quest.guild.money.loans.repay.wallet");
        screen.addOption(3, "Rembourser depuis le compte de guilde", "Utiliser des fonds réellement déposés, sans téléporter les pièces de la bourse.", loan.active && overview.depositedCopper > 0, "quest.guild.money.loans.repay.account");
        screen.addOption(4, "Demander une prolongation", "À demander avant l'échéance ; elle ajoute un coût fixe au capital initial sans intérêts composés.", loan.active && !loan.overdue && player.getWorldDaysElapsed() < loan.dueDay && loan.totalCostRatePercent < 30, "quest.guild.money.loans.extend");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une opération sur le prêt.");
        Console::clear();
        if (choice == 0) return;

        if (choice == 1)
        {
            const int maximum = static_cast<int>(std::min<long long>(maximumPrincipal, 1000000000LL));
            if (maximum <= 0) continue;
            const int principal = MessageScreen::askQuantity(
                "MONTANT DU PRÊT",
                "quest.guild.money.loans.open.amount",
                {
                    "Plafond de ton dossier : " + Money::formatCopper(maximumPrincipal) + ".",
                    "Le capital choisi sera ajouté physiquement à ta bourse après signature.",
                    "Un nouveau prêt sera refusé tant que celui-ci ne sera pas entièrement remboursé."
                },
                1,
                maximum,
                "Le montant doit rester dans le plafond du dossier."
            );

            MenuScreen termScreen("DURÉE DU PRÊT", "quest.guild.money.loans.open.term");
            termScreen.addLine("Les longues durées demandent davantage de confiance et supportent une majoration dès la signature.");
            termScreen.addBackOption("Annuler", "quest.guild.money.loans.open.term.back");
            const std::vector<int> weeksOptions = {1, 2, 3, 4, 6, 8};
            for (std::size_t i = 0; i < weeksOptions.size(); ++i)
            {
                const GuildLoanTerms terms = GuildFinanceSystem::loanTermsForWeeks(weeksOptions[i]);
                const std::string surcharge = terms.longDurationSurchargePercent > 0
                    ? " + " + std::to_string(terms.longDurationSurchargePercent) + " % durée longue"
                    : "";
                termScreen.addOption(
                    static_cast<int>(i) + 1,
                    std::to_string(terms.weeks) + " semaine(s) — " + std::to_string(terms.baseRatePercent) + " %" + surcharge,
                    "Coût signé : " + std::to_string(terms.effectiveRatePercent) + " % | confiance requise : " + std::to_string(terms.minimumTrustScore) + "/100 | total : " + Money::formatCopper(guildLoanSignedTotal(principal, terms)) + ".",
                    overview.trustScore >= terms.minimumTrustScore,
                    "quest.guild.money.loans.open.term." + std::to_string(terms.weeks)
                );
            }
            const int termChoice = TerminalInterface::askMenuChoiceFromOptions(termScreen, "Choisis une durée accessible à ton dossier.");
            Console::clear();
            if (termChoice <= 0 || termChoice > static_cast<int>(weeksOptions.size())) continue;
            const int weeks = weeksOptions[static_cast<std::size_t>(termChoice - 1)];
            const GuildLoanTerms terms = GuildFinanceSystem::loanTermsForWeeks(weeks);
            if (overview.trustScore < terms.minimumTrustScore) continue;

            MenuScreen modeScreen("MODE DE REMBOURSEMENT", "quest.guild.money.loans.open.mode");
            modeScreen.addLine("Le capital et le coût total sont identiques dans les deux modes.");
            modeScreen.addBackOption("Annuler", "quest.guild.money.loans.open.mode.back");
            modeScreen.addOption(1, "Manuel", "Tu décides quand payer depuis la bourse ou le compte de guilde.", true, "quest.guild.money.loans.open.mode.manual");
            modeScreen.addOption(2, "Automatique à l'échéance", "À l'échéance, la guilde prélève autant que possible uniquement sur le solde déjà déposé chez elle.", true, "quest.guild.money.loans.open.mode.auto");
            const int modeChoice = TerminalInterface::askMenuChoiceFromOptions(modeScreen, "Choisis le mode de remboursement.");
            Console::clear();
            if (modeChoice != 1 && modeChoice != 2) continue;

            const long long totalDue = guildLoanSignedTotal(principal, terms);
            MenuScreen confirmScreen("SIGNATURE DU PRÊT", "quest.guild.money.loans.open.confirm");
            confirmScreen.addLine("Capital reçu : " + Money::formatCopper(principal) + ".");
            confirmScreen.addLine("Durée : " + std::to_string(weeks) + " semaine(s).");
            confirmScreen.addLine("Taux de base : " + std::to_string(terms.baseRatePercent) + " %.");
            if (terms.longDurationSurchargePercent > 0)
            {
                confirmScreen.addLine("Majoration de durée longue : +" + std::to_string(terms.longDurationSurchargePercent) + " %.");
            }
            confirmScreen.addLine("Coût total signé : " + std::to_string(terms.effectiveRatePercent) + " % | total à rembourser : " + Money::formatCopper(totalDue) + ".");
            confirmScreen.addLine("Rembourser plus tôt est autorisé mais ne réduit pas ce coût déjà signé.");
            confirmScreen.addBackOption("Refuser le contrat", "quest.guild.money.loans.open.confirm.back");
            confirmScreen.addOption(1, "Signer le contrat", modeChoice == 2 ? "Capital versé maintenant ; prélèvement automatique du compte de guilde prévu à l'échéance." : "Capital versé maintenant ; remboursements manuels.", true, "quest.guild.money.loans.open.confirm.accept");
            const int confirmation = TerminalInterface::askMenuChoiceFromOptions(confirmScreen, "Confirme ou refuse le contrat.");
            Console::clear();
            if (confirmation != 1) continue;

            const GuildLoanCreateResult result = GuildFinanceSystem::createLoan(player, principal, weeks, modeChoice == 2);
            if (!result.success)
            {
                MessageScreen::show("PRÊT REFUSÉ", "quest.guild.money.loans.open.failed", {result.failureReason}, false);
                continue;
            }
            MessageScreen::show(
                "PRÊT SIGNÉ",
                "quest.guild.money.loans.open.done",
                {
                    "Capital reçu : " + Money::formatCopper(result.loan.principalCopper) + ".",
                    "Total dû : " + Money::formatCopper(result.loan.totalDueCopper) + ".",
                    "Échéance : fin du jour " + std::to_string(result.loan.dueDay + 1) + ".",
                    "Bourse actuelle : " + player.getInventory().getWalletLine() + "."
                },
                false
            );
            continue;
        }

        if (choice == 2 || choice == 3)
        {
            const GuildLoanState current = GuildFinanceSystem::activeLoan(player);
            if (!current.active) continue;
            const long long available = choice == 2 ? player.getInventory().getTotalCopper() : GuildFinanceSystem::depositedBalanceCopper(player);
            const long long maximumLong = std::min(current.remainingCopper, available);
            if (maximumLong <= 0) continue;
            const int maximum = static_cast<int>(std::min<long long>(maximumLong, 1000000000LL));
            const int amount = MessageScreen::askQuantity(
                "REMBOURSEMENT DU PRÊT",
                choice == 2 ? "quest.guild.money.loans.repay.wallet.amount" : "quest.guild.money.loans.repay.account.amount",
                {
                    "Reste dû : " + Money::formatCopper(current.remainingCopper) + ".",
                    choice == 2 ? "Bourse : " + player.getInventory().getWalletLine() + "." : "Solde de guilde : " + Money::formatCopper(available) + ".",
                    "Le remboursement anticipé ne modifie pas les intérêts déjà signés."
                },
                1,
                maximum,
                "Le remboursement ne peut dépasser ni les fonds disponibles ni le reste dû."
            );
            std::string reason;
            const bool paid = choice == 2
                ? GuildFinanceSystem::repayLoanFromWallet(player, amount, &reason)
                : GuildFinanceSystem::repayLoanFromGuildBalance(player, amount, &reason);
            if (!paid)
            {
                MessageScreen::show("REMBOURSEMENT REFUSÉ", "quest.guild.money.loans.repay.failed", {reason}, false);
            }
            else
            {
                const GuildLoanState after = GuildFinanceSystem::activeLoan(player);
                MessageScreen::show(
                    "REMBOURSEMENT EFFECTUÉ",
                    "quest.guild.money.loans.repay.done",
                    {
                        "Payé : " + Money::formatCopper(amount) + ".",
                        after.active ? "Reste dû : " + Money::formatCopper(after.remainingCopper) + "." : "Prêt intégralement remboursé.",
                        "Aucune réduction rétroactive du coût signé."
                    },
                    false
                );
            }
            continue;
        }

        if (choice == 4)
        {
            const GuildLoanState current = GuildFinanceSystem::activeLoan(player);
            if (!current.active || current.overdue || player.getWorldDaysElapsed() >= current.dueDay) continue;
            MenuScreen extensionScreen("PROLONGATION PRÉVENTIVE", "quest.guild.money.loans.extend");
            extensionScreen.addLine("Coût contractuel actuel : " + std::to_string(current.totalCostRatePercent) + " % sur le capital initial.");
            extensionScreen.addLine("Le plafond global est 30 %. Si une grille dépasserait ce plafond, seule la marge restante est facturée.");
            extensionScreen.addBackOption("Annuler", "quest.guild.money.loans.extend.back");
            extensionScreen.addOption(1, "+1 semaine", "Coût prévu : jusqu'à +3 % du capital initial.", current.totalCostRatePercent < 30, "quest.guild.money.loans.extend.1");
            extensionScreen.addOption(2, "+2 semaines", "Coût prévu : jusqu'à +5 % du capital initial.", current.totalCostRatePercent < 30, "quest.guild.money.loans.extend.2");
            extensionScreen.addOption(3, "+3 semaines", "Coût prévu : jusqu'à +7 % du capital initial.", current.totalCostRatePercent < 30, "quest.guild.money.loans.extend.3");
            const int extensionChoice = TerminalInterface::askMenuChoiceFromOptions(extensionScreen, "Choisis la prolongation.");
            Console::clear();
            if (extensionChoice < 1 || extensionChoice > 3) continue;
            const GuildLoanExtensionResult extension = GuildFinanceSystem::extendActiveLoan(player, extensionChoice);
            if (!extension.success)
            {
                MessageScreen::show("PROLONGATION REFUSÉE", "quest.guild.money.loans.extend.failed", {extension.failureReason}, false);
            }
            else
            {
                MessageScreen::show(
                    "PROLONGATION VALIDÉE",
                    "quest.guild.money.loans.extend.done",
                    {
                        "Durée ajoutée : " + std::to_string(extension.requestedWeeks) + " semaine(s).",
                        "Taux ajouté : +" + std::to_string(extension.appliedRatePercent) + " %.",
                        "Frais fixes ajoutés : " + Money::formatCopper(extension.addedFeeCopper) + ".",
                        "Nouveau reste dû : " + Money::formatCopper(extension.loan.remainingCopper) + ".",
                        "Nouvelle échéance : fin du jour " + std::to_string(extension.loan.dueDay + 1) + "."
                    },
                    false
                );
            }
        }
    }
}

void GuildFinanceMenu::openMoneyManagement(Player& player)
{
    while (true)
    {
        const GuildFinanceOverview overview = GuildFinanceSystem::overview(player);
        MenuScreen screen("GESTION DE L'ARGENT", "quest.guild.money");
        screen.addSubtitle("Bourse physique, change, compte de guilde et adhésion");
        screen.addLine("Bourse physique : " + player.getInventory().getWalletLine() + ".");
        screen.addLine("Valeur totale portée : " + player.getInventory().getWalletTotalLine() + ".");
        screen.addLine("Solde déposé à la guilde : " + Money::formatCopper(overview.depositedCopper) + " / " + Money::formatCopper(overview.accountCapacityCopper) + ".");
        if (overview.depositedCopper > overview.accountCapacityCopper)
        {
            screen.addLine("Ancien solde hors plafond : rien n'est confisqué, mais les nouveaux dépôts restent bloqués jusqu'à repasser sous la limite.");
        }
        screen.addLine(std::string("Adhésion : ") + (overview.member ? "ACTIVE" : "inactive") + ".");
        screen.addLine("Confiance financière : " + std::to_string(overview.trustScore) + "/100 — " + overview.trustLabel + ".");
        if (overview.member)
        {
            screen.addLine("Découvert utilisé : " + Money::formatCopper(overview.overdraftUsedCopper) + " | Plafond de découvert : " + Money::formatCopper(overview.overdraftCeilingCopper) + " | Marge restante : " + Money::formatCopper(overview.overdraftRemainingCopper) + ".");
        }
        else
        {
            screen.addLine("Découvert utilisé : " + Money::formatCopper(overview.overdraftUsedCopper) + " | Nouveau crédit : réservé aux adhérents de la guilde.");
        }
        screen.addLine("Important : bourse, compte de guilde, découvert et coffre personnel sont distincts. Le compte de guilde est volontairement plafonné pour ne pas remplacer le coffre.");
        screen.addBackOption("Retour à la guilde", "quest.guild.money.back");
        screen.addOption(1, "Bourse et conversion des pièces", "PC, PF, PE, PO et PP : changer uniquement la forme physique de la même valeur.", true, "quest.guild.money.exchange");
        screen.addOption(2, "Déposer de l'argent à la guilde", "Sortir une somme réelle de la bourse et la placer sur le compte de guilde plafonné.", overview.adventurerRegistered && player.getInventory().getTotalCopper() > 0 && overview.accountRemainingCapacityCopper > 0, "quest.guild.money.deposit");
        screen.addOption(3, "Retirer de l'argent de la guilde", "Récupérer physiquement une partie du solde déposé.", overview.depositedCopper > 0, "quest.guild.money.withdraw");
        screen.addOption(4, "Adhésion de guilde", overview.member ? "Consulter, renouveler ou demander l'annulation." : "Voir les avantages et prendre la cotisation aventurier standard.", true, "quest.guild.money.membership");
        screen.addOption(5, "Découvert de guilde", overview.overdraftUsedCopper > 0 ? "Consulter et rembourser la dette de crédit réseau." : (overview.member ? "Consulter le plafond réel du crédit réseau." : "Réservé aux adhérents pour les nouveaux crédits."), overview.member || overview.overdraftUsedCopper > 0, "quest.guild.money.overdraft");
        const GuildLoanState currentLoan = GuildFinanceSystem::activeLoan(player);
        screen.addOption(6, "Prêts de guilde", currentLoan.active ? "Consulter et rembourser le prêt actif." : (overview.member ? "Demander un prêt physique selon la confiance financière." : "Réservé aux adhérents pour les nouveaux prêts."), overview.member || currentLoan.active, "quest.guild.money.loans");
        screen.addOption(7, "Rembourser plusieurs dettes", "Si découvert et prêt coexistent, choisir la source et la priorité. Découvert d'abord est recommandé par défaut.", overview.overdraftUsedCopper > 0 && currentLoan.active && (player.getInventory().getTotalCopper() > 0 || overview.depositedCopper > 0), "quest.guild.money.debts.repay");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une opération financière.");
        Console::clear();
        if (choice == 0) return;
        if (choice == 1)
        {
            openGuildCurrencyExchange(player);
            continue;
        }
        if (choice == 4)
        {
            openGuildMembershipFinance(player);
            continue;
        }
        if (choice == 5)
        {
            openGuildOverdraftManagement(player);
            continue;
        }
        if (choice == 6)
        {
            openGuildLoanManagement(player);
            continue;
        }
        if (choice == 7)
        {
            const GuildLoanState debtLoan = GuildFinanceSystem::activeLoan(player);
            const long long overdraftDebt = GuildFinanceSystem::overdraftUsedCopper(player);
            if (!debtLoan.active || overdraftDebt <= 0) continue;

            MenuScreen sourceScreen("SOURCE DU REMBOURSEMENT", "quest.guild.money.debts.repay.source");
            sourceScreen.addLine("Découvert : " + Money::formatCopper(overdraftDebt) + " | prêt : " + Money::formatCopper(debtLoan.remainingCopper) + ".");
            sourceScreen.addBackOption("Annuler", "quest.guild.money.debts.repay.source.back");
            sourceScreen.addOption(1, "Bourse physique", "Utiliser les pièces réellement portées.", player.getInventory().getTotalCopper() > 0, "quest.guild.money.debts.repay.source.wallet");
            sourceScreen.addOption(2, "Compte de guilde", "Utiliser uniquement les fonds déjà déposés auprès de la guilde.", overview.depositedCopper > 0, "quest.guild.money.debts.repay.source.account");
            const int sourceChoice = TerminalInterface::askMenuChoiceFromOptions(sourceScreen, "Choisis la source du remboursement.");
            Console::clear();
            if (sourceChoice != 1 && sourceChoice != 2) continue;

            const long long sourceAvailable = sourceChoice == 1 ? player.getInventory().getTotalCopper() : overview.depositedCopper;
            const long long totalDebt = overdraftDebt + debtLoan.remainingCopper;
            const int maximum = static_cast<int>(std::min<long long>(std::min(sourceAvailable, totalDebt), 1000000000LL));
            if (maximum <= 0) continue;
            const int amount = MessageScreen::askQuantity(
                "REMBOURSEMENT GLOBAL",
                "quest.guild.money.debts.repay.amount",
                {
                    "Dette totale : " + Money::formatCopper(totalDebt) + ".",
                    sourceChoice == 1 ? "Bourse disponible : " + player.getInventory().getWalletLine() + "." : "Compte de guilde disponible : " + Money::formatCopper(sourceAvailable) + ".",
                    "Tu choisiras ensuite quelle dette doit être réduite en priorité."
                },
                1,
                maximum,
                "Le montant ne peut dépasser ni les fonds disponibles ni la dette totale."
            );

            MenuScreen priorityScreen("PRIORITÉ DE REMBOURSEMENT", "quest.guild.money.debts.repay.priority");
            priorityScreen.addLine("Le choix recommandé et présenté en premier est le découvert, généralement plus contraignant pour les paiements du réseau.");
            priorityScreen.addBackOption("Annuler", "quest.guild.money.debts.repay.priority.back");
            priorityScreen.addOption(1, "Découvert d'abord [recommandé]", "Réduit le découvert au maximum, puis affecte le reste au prêt.", true, "quest.guild.money.debts.repay.priority.overdraft");
            priorityScreen.addOption(2, "Prêt d'abord", "Réduit le prêt au maximum, puis affecte le reste au découvert.", true, "quest.guild.money.debts.repay.priority.loan");
            priorityScreen.addOption(3, "Répartir", "Partage autant que possible entre les deux dettes, puis utilise le reliquat là où il reste une dette.", true, "quest.guild.money.debts.repay.priority.split");
            const int priorityChoice = TerminalInterface::askMenuChoiceFromOptions(priorityScreen, "Choisis la priorité.");
            Console::clear();
            if (priorityChoice < 1 || priorityChoice > 3) continue;
            const GuildDebtRepaymentPriority priority = priorityChoice == 1
                ? GuildDebtRepaymentPriority::OverdraftFirst
                : (priorityChoice == 2 ? GuildDebtRepaymentPriority::LoanFirst : GuildDebtRepaymentPriority::Split);
            const GuildDebtRepaymentResult result = sourceChoice == 1
                ? GuildFinanceSystem::repayDebtsFromWallet(player, amount, priority)
                : GuildFinanceSystem::repayDebtsFromGuildBalance(player, amount, priority);
            if (!result.success)
            {
                MessageScreen::show("REMBOURSEMENT REFUSÉ", "quest.guild.money.debts.repay.failed", {result.failureReason}, false);
            }
            else
            {
                MessageScreen::show(
                    "DETTES RÉDUITES",
                    "quest.guild.money.debts.repay.done",
                    {
                        "Total payé : " + Money::formatCopper(result.totalPaidCopper) + ".",
                        "Vers le découvert : " + Money::formatCopper(result.overdraftPaidCopper) + ".",
                        "Vers le prêt : " + Money::formatCopper(result.loanPaidCopper) + "."
                    },
                    false
                );
            }
            continue;
        }
        if (choice == 2)
        {
            const long long wallet = player.getInventory().getTotalCopper();
            if (wallet <= 0 || !overview.adventurerRegistered) continue;
            const long long remainingCapacity = overview.accountRemainingCapacityCopper;
            const int maximum = static_cast<int>(std::min<long long>(std::min(wallet, remainingCapacity), 1000000000LL));
            if (maximum <= 0) continue;
            const int amount = MessageScreen::askQuantity(
                "DÉPÔT À LA GUILDE",
                "quest.guild.money.deposit.quantity",
                {
                    "Bourse actuelle : " + player.getInventory().getWalletLine() + ".",
                    "Solde déposé actuel : " + Money::formatCopper(overview.depositedCopper) + ".",
                    "Capacité du compte : " + Money::formatCopper(overview.accountCapacityCopper) + ".",
                    "Marge de dépôt restante : " + Money::formatCopper(overview.accountRemainingCapacityCopper) + ".",
                    "Choisis la valeur exacte en PC à confier au comptoir."
                },
                1,
                maximum,
                "Le dépôt doit rester entre 1 PC et la plus petite limite entre ta bourse et la marge du compte."
            );
            std::string reason;
            if (GuildFinanceSystem::depositCopper(player, amount, &reason))
            {
                MessageScreen::show(
                    "DÉPÔT EFFECTUÉ",
                    "quest.guild.money.deposit.done",
                    {
                        "Déposé : " + Money::formatCopper(amount) + ".",
                        "Bourse restante : " + player.getInventory().getWalletLine() + ".",
                        "Solde de guilde : " + Money::formatCopper(GuildFinanceSystem::depositedBalanceCopper(player)) + "."
                    },
                    false
                );
            }
            else
            {
                MessageScreen::show("DÉPÔT REFUSÉ", "quest.guild.money.deposit.failed", {reason}, false);
            }
            continue;
        }
        if (choice == 3)
        {
            const long long balance = GuildFinanceSystem::depositedBalanceCopper(player);
            if (balance <= 0) continue;
            const int maximum = static_cast<int>(std::min<long long>(balance, 1000000000LL));
            const int amount = MessageScreen::askQuantity(
                "RETRAIT DE LA GUILDE",
                "quest.guild.money.withdraw.quantity",
                {
                    "Solde déposé : " + Money::formatCopper(balance) + ".",
                    "Le retrait revient dans la bourse physique sans modifier les autres piles déjà portées."
                },
                1,
                maximum,
                "Le retrait doit rester entre 1 PC et le solde disponible."
            );
            std::string reason;
            if (GuildFinanceSystem::withdrawCopper(player, amount, &reason))
            {
                MessageScreen::show(
                    "RETRAIT EFFECTUÉ",
                    "quest.guild.money.withdraw.done",
                    {
                        "Retiré : " + Money::formatCopper(amount) + ".",
                        "Bourse actuelle : " + player.getInventory().getWalletLine() + ".",
                        "Solde de guilde : " + Money::formatCopper(GuildFinanceSystem::depositedBalanceCopper(player)) + "."
                    },
                    false
                );
            }
            else
            {
                MessageScreen::show("RETRAIT REFUSÉ", "quest.guild.money.withdraw.failed", {reason}, false);
            }
        }
    }
}
