#include "economy/GuildFinanceSystem.hpp"
#include "economy/Money.hpp"
#include "entity/Player.hpp"
#include "save/SaveManager.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"

#include <cassert>
#include <filesystem>
#include <string>

int main()
{
    namespace fs = std::filesystem;
    fs::remove_all("assets/saves");

    Player unregistered;
    unregistered.getInventory().earnCopper(200);
    std::string failure;
    assert(!GuildFinanceSystem::depositCopper(unregistered, 50, &failure));
    assert(!failure.empty());
    assert(GuildFinanceSystem::depositedBalanceCopper(unregistered) == 0);
    assert(GuildFinanceSystem::accountCapacityCopper(unregistered) == 0);
    assert(GuildFinanceSystem::overdraftCeilingCopper(unregistered) == 0);
    assert(GuildFinanceSystem::overdraftUsedCopper(unregistered) == 0);

    Player player;
    assert(player.grantTitle("Aventurier"));
    player.getInventory().earnCopper(10000);
    const long long walletBefore = player.getInventory().getTotalCopper();

    // Registered non-members get a deliberately small, finite guild account.
    assert(GuildFinanceSystem::accountCapacityCopper(player) == 2LL * Money::COPPER_PER_GOLD);
    failure.clear();
    assert(GuildFinanceSystem::depositCopper(player, 300, &failure));
    assert(failure.empty());
    assert(player.getInventory().getTotalCopper() == walletBefore - 300);
    assert(GuildFinanceSystem::depositedBalanceCopper(player) == 300);

    // Deposits cannot exceed the account cap; nothing is silently confiscated.
    assert(!GuildFinanceSystem::depositCopper(player, 2LL * Money::COPPER_PER_GOLD, &failure));
    assert(GuildFinanceSystem::depositedBalanceCopper(player) == 300);

    assert(GuildFinanceSystem::withdrawCopper(player, 125, &failure));
    assert(GuildFinanceSystem::depositedBalanceCopper(player) == 175);
    assert(player.getInventory().getTotalCopper() == walletBefore - 175);
    assert(!GuildFinanceSystem::withdrawCopper(player, 176, &failure));
    assert(GuildFinanceSystem::depositedBalanceCopper(player) == 175);

    const GuildFinanceOverview beforeMembership = GuildFinanceSystem::overview(player);
    assert(!beforeMembership.member);
    assert(beforeMembership.overdraftCeilingCopper == 0);
    assert(beforeMembership.overdraftRemainingCopper == 0);
    assert(beforeMembership.trustScore > 0);

    player.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    player.recordCanonicalEvent("guild_finance_membership", "standard", "Adhésion test", 1);

    const GuildFinanceOverview afterMembership = GuildFinanceSystem::overview(player);
    assert(afterMembership.member);
    assert(afterMembership.membershipExpiresAtDay >= player.getWorldDaysElapsed());
    assert(afterMembership.accountCapacityCopper == 5LL * Money::COPPER_PER_GOLD);
    assert(afterMembership.overdraftCeilingCopper > 0);
    assert(afterMembership.overdraftUsedCopper == 0);
    assert(afterMembership.overdraftRemainingCopper == afterMembership.overdraftCeilingCopper);
    assert(afterMembership.trustScore >= beforeMembership.trustScore);
    assert(GuildFinanceSystem::financialTrustLabel(afterMembership.trustScore) != "Aucun dossier financier");

    // A partner payment uses physical money first, then only the missing part as network credit.
    const long long walletBeforeCreditPayment = player.getInventory().getTotalCopper();
    assert(walletBeforeCreditPayment > 50);
    const long long paymentAmount = walletBeforeCreditPayment + std::min(25LL, afterMembership.overdraftRemainingCopper);
    GuildFinancePaymentResult payment = GuildFinanceSystem::payAffiliatedService(
        player,
        paymentAmount,
        "test_partner",
        "Service partenaire de test",
        true
    );
    assert(payment.success);
    assert(payment.walletPaidCopper == walletBeforeCreditPayment);
    assert(payment.overdraftDrawnCopper == paymentAmount - walletBeforeCreditPayment);
    assert(player.getInventory().getTotalCopper() == 0);
    assert(GuildFinanceSystem::overdraftUsedCopper(player) == payment.overdraftDrawnCopper);

    // The overdraft cannot be withdrawn as cash. Repayment is explicit from real funds.
    player.getInventory().earnCopper(10);
    assert(GuildFinanceSystem::repayOverdraftFromWallet(player, 10, &failure));
    assert(GuildFinanceSystem::overdraftUsedCopper(player) == payment.overdraftDrawnCopper - 10);
    assert(player.getInventory().getTotalCopper() == 0);

    const long long debtBeforeAccountRepayment = GuildFinanceSystem::overdraftUsedCopper(player);
    if (debtBeforeAccountRepayment > 0)
    {
        const long long fromAccount = std::min(5LL, debtBeforeAccountRepayment);
        const long long accountBeforeRepayment = GuildFinanceSystem::depositedBalanceCopper(player);
        assert(GuildFinanceSystem::repayOverdraftFromGuildBalance(player, fromAccount, &failure));
        assert(GuildFinanceSystem::overdraftUsedCopper(player) == debtBeforeAccountRepayment - fromAccount);
        assert(GuildFinanceSystem::depositedBalanceCopper(player) == accountBeforeRepayment - fromAccount);
    }

    // Existing schema 26 persistence is enough: membership, account and overdraft are reconstructed
    // from the already-persisted subscription and canonical finance records.
    const long long expectedBalance = GuildFinanceSystem::depositedBalanceCopper(player);
    const long long expectedDebt = GuildFinanceSystem::overdraftUsedCopper(player);
    const std::string account = "guild_finance_test_account";
    player.setOwnershipMetadata(account, account);
    assert(SaveManager::saveAccountSnapshot(account));
    assert(SaveManager::savePlayerSnapshot(player, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));
    const auto summaries = SaveManager::listPlayableCharacters(account);
    assert(!summaries.empty());
    Player loaded;
    DifficultyMode loadedDifficulty = DifficultyMode::Easy;
    DeathRuleMode loadedDeathRule = DeathRuleMode::NonDefinitive;
    assert(SaveManager::loadPlayerSnapshot(summaries.front(), loaded, loadedDifficulty, loadedDeathRule));
    assert(GuildFinanceSystem::isMember(loaded));
    assert(GuildFinanceSystem::depositedBalanceCopper(loaded) == expectedBalance);
    assert(GuildFinanceSystem::overdraftUsedCopper(loaded) == expectedDebt);

    // Expired membership closes new credit but never erases existing debt or blocks repayment.
    loaded.advanceWorldDays(8);
    assert(!GuildFinanceSystem::isMember(loaded));
    assert(GuildFinanceSystem::overdraftCeilingCopper(loaded) == 0);
    assert(GuildFinanceSystem::overdraftRemainingCopper(loaded) == 0);
    assert(GuildFinanceSystem::overdraftUsedCopper(loaded) == expectedDebt);
    GuildFinancePaymentResult refusedCredit = GuildFinanceSystem::payAffiliatedService(
        loaded,
        loaded.getInventory().getTotalCopper() + 1,
        "test_partner_after_expiry",
        "Service partenaire après expiration",
        true
    );
    assert(!refusedCredit.success);

    // Finance bookkeeping never produces a negative physical wallet.
    assert(player.getInventory().getTotalCopper() >= 0);
    assert(GuildFinanceSystem::depositedBalanceCopper(player) >= 0);
    assert(GuildFinanceSystem::overdraftUsedCopper(player) >= 0);

    // Real guild loans: long terms carry a signing surcharge in addition to the base rate.
    Player loanPlayer;
    assert(loanPlayer.grantTitle("Aventurier"));
    loanPlayer.getInventory().earnCopper(2000);
    loanPlayer.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    assert(GuildFinanceSystem::depositCopper(loanPlayer, 100, &failure));
    for (int i = 0; i < 5; ++i)
    {
        Quest q;
        q.id = "finance_completed_" + std::to_string(i);
        q.guildQuest = true;
        q.turnedIn = true;
        loanPlayer.getQuestLog().getQuests().push_back(q);
    }
    assert(GuildFinanceSystem::financialTrustScore(loanPlayer) >= 65);
    const GuildLoanTerms threeWeeks = GuildFinanceSystem::loanTermsForWeeks(3);
    assert(threeWeeks.baseRatePercent == 16);
    assert(threeWeeks.longDurationSurchargePercent == 2);
    assert(threeWeeks.effectiveRatePercent == 18);
    assert(threeWeeks.minimumTrustScore == 65);

    const long long loanWalletBefore = loanPlayer.getInventory().getTotalCopper();
    GuildLoanCreateResult opened = GuildFinanceSystem::createLoan(loanPlayer, 500, 3, false);
    assert(opened.success);
    assert(opened.loan.active);
    assert(opened.loan.principalCopper == 500);
    assert(opened.loan.totalDueCopper == 590);
    assert(opened.loan.totalCostRatePercent == 18);
    assert(loanPlayer.getInventory().getTotalCopper() == loanWalletBefore + 500);
    assert(!GuildFinanceSystem::createLoan(loanPlayer, 10, 1, false).success);

    // Asking before the due date is allowed and adds a fixed fee to the original principal.
    GuildLoanExtensionResult extension = GuildFinanceSystem::extendActiveLoan(loanPlayer, 3);
    assert(extension.success);
    assert(extension.appliedRatePercent == 7);
    assert(extension.addedFeeCopper == 35);
    assert(extension.loan.totalCostRatePercent == 25);
    assert(extension.loan.totalDueCopper == 625);
    assert(extension.loan.extensionWeeks == 3);

    // Early repayment is allowed, but the signed total does not shrink.
    const long long signedTotal = extension.loan.totalDueCopper;
    assert(GuildFinanceSystem::repayLoanFromWallet(loanPlayer, 25, &failure));
    GuildLoanState afterEarlyPayment = GuildFinanceSystem::activeLoan(loanPlayer);
    assert(afterEarlyPayment.active);
    assert(afterEarlyPayment.totalDueCopper == signedTotal);
    assert(afterEarlyPayment.remainingCopper == signedTotal - 25);

    // Loan state is reconstructed from schema-26 canonical records after save/load.
    const std::string loanAccount = "guild_loan_test_account";
    loanPlayer.setOwnershipMetadata(loanAccount, loanAccount);
    assert(SaveManager::saveAccountSnapshot(loanAccount));
    assert(SaveManager::savePlayerSnapshot(loanPlayer, loanAccount, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));
    const auto loanSummaries = SaveManager::listPlayableCharacters(loanAccount);
    assert(!loanSummaries.empty());
    Player loadedLoanPlayer;
    assert(SaveManager::loadPlayerSnapshot(loanSummaries.front(), loadedLoanPlayer, loadedDifficulty, loadedDeathRule));
    const GuildLoanState loadedLoan = GuildFinanceSystem::activeLoan(loadedLoanPlayer);
    assert(loadedLoan.active);
    assert(loadedLoan.principalCopper == 500);
    assert(loadedLoan.totalDueCopper == signedTotal);
    assert(loadedLoan.remainingCopper == signedTotal - 25);
    assert(loadedLoan.totalCostRatePercent == 25);

    // Automatic repayment only uses money that was actually deposited with the guild.
    Player autoPlayer;
    assert(autoPlayer.grantTitle("Aventurier"));
    autoPlayer.getInventory().earnCopper(1000);
    autoPlayer.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    assert(GuildFinanceSystem::depositCopper(autoPlayer, 200, &failure));
    for (int i = 0; i < 5; ++i)
    {
        Quest q;
        q.id = "finance_auto_completed_" + std::to_string(i);
        q.guildQuest = true;
        q.turnedIn = true;
        autoPlayer.getQuestLog().getQuests().push_back(q);
    }
    GuildLoanCreateResult autoLoan = GuildFinanceSystem::createLoan(autoPlayer, 100, 1, true);
    assert(autoLoan.success);
    assert(autoLoan.loan.totalDueCopper == 112);
    const long long autoAccountBefore = GuildFinanceSystem::depositedBalanceCopper(autoPlayer);
    autoPlayer.advanceWorldDays(8);
    assert(!GuildFinanceSystem::activeLoan(autoPlayer).active);
    assert(GuildFinanceSystem::depositedBalanceCopper(autoPlayer) == autoAccountBefore - 112);

    // Manual loans become overdue without compounding extra interest.
    Player latePlayer;
    assert(latePlayer.grantTitle("Aventurier"));
    latePlayer.getInventory().earnCopper(1000);
    latePlayer.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    assert(GuildFinanceSystem::depositCopper(latePlayer, 100, &failure));
    for (int i = 0; i < 5; ++i)
    {
        Quest q;
        q.id = "finance_late_completed_" + std::to_string(i);
        q.guildQuest = true;
        q.turnedIn = true;
        latePlayer.getQuestLog().getQuests().push_back(q);
    }
    GuildLoanCreateResult lateLoan = GuildFinanceSystem::createLoan(latePlayer, 100, 1, false);
    assert(lateLoan.success);
    const long long lateSignedTotal = lateLoan.loan.totalDueCopper;
    latePlayer.advanceWorldDays(8);
    const GuildLoanState overdueLoan = GuildFinanceSystem::activeLoan(latePlayer);
    assert(overdueLoan.active);
    assert(overdueLoan.overdue);
    assert(overdueLoan.totalDueCopper == lateSignedTotal);
    assert(latePlayer.getCanonicalJournalCategoryTotal("guild_finance_loan_reminders") >= 1);

    // Le retard suspend tout nouveau crédit, même si l'adhésion est renouvelée.
    latePlayer.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        30,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    assert(GuildFinanceSystem::isMember(latePlayer));
    assert(GuildFinanceSystem::overdraftCeilingCopper(latePlayer) == 0);
    assert(GuildFinanceSystem::overdraftRemainingCopper(latePlayer) == 0);
    const GuildDebtSituation lateSituation = GuildFinanceSystem::assessDebtSituation(latePlayer);
    assert(lateSituation.overdue);
    assert(lateSituation.overdueDays >= 1);
    assert(lateSituation.newCreditRestricted);
    assert(!lateSituation.statusLabel.empty());
    // Le moteur décrit la capacité financière sans accuser automatiquement le joueur de mauvaise foi.
    assert(lateSituation.canCoverNow);

    // At the extreme term, signing reaches 28%; an extension is clipped to the 30% global cap.
    Player longLoanPlayer;
    assert(longLoanPlayer.grantTitle("Aventurier"));
    longLoanPlayer.getInventory().earnCopper(2000);
    longLoanPlayer.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    assert(GuildFinanceSystem::depositCopper(longLoanPlayer, 100, &failure));
    for (int i = 0; i < 13; ++i)
    {
        Quest q;
        q.id = "finance_long_completed_" + std::to_string(i);
        q.guildQuest = true;
        q.turnedIn = true;
        longLoanPlayer.getQuestLog().getQuests().push_back(q);
    }
    longLoanPlayer.recordCanonicalEvent("guild_finance_loan_closed_on_time", "old_loan_1", "Ancien prêt remboursé dans les temps", 1);
    longLoanPlayer.recordCanonicalEvent("guild_finance_loan_closed_on_time", "old_loan_2", "Ancien prêt remboursé dans les temps", 1);
    longLoanPlayer.recordCanonicalEvent("guild_finance_loan_closed_on_time", "old_loan_3", "Ancien prêt remboursé dans les temps", 1);
    assert(GuildFinanceSystem::financialTrustScore(longLoanPlayer) >= 90);
    const GuildLoanTerms eightWeeks = GuildFinanceSystem::loanTermsForWeeks(8);
    assert(eightWeeks.baseRatePercent == 24);
    assert(eightWeeks.longDurationSurchargePercent == 4);
    assert(eightWeeks.effectiveRatePercent == 28);
    GuildLoanCreateResult longLoan = GuildFinanceSystem::createLoan(longLoanPlayer, 100, 8, false);
    assert(longLoan.success);
    assert(longLoan.loan.totalDueCopper == 128);
    GuildLoanExtensionResult cappedExtension = GuildFinanceSystem::extendActiveLoan(longLoanPlayer, 1);
    assert(cappedExtension.success);
    assert(cappedExtension.appliedRatePercent == 2);
    assert(cappedExtension.loan.totalCostRatePercent == 30);
    assert(!GuildFinanceSystem::extendActiveLoan(longLoanPlayer, 1).success);

    // When overdraft and loan coexist, the player explicitly controls repayment priority.
    Player comboPlayer;
    assert(comboPlayer.grantTitle("Aventurier"));
    comboPlayer.getInventory().earnCopper(1000);
    comboPlayer.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    assert(GuildFinanceSystem::depositCopper(comboPlayer, 100, &failure));
    for (int i = 0; i < 10; ++i)
    {
        Quest q;
        q.id = "finance_combo_completed_" + std::to_string(i);
        q.guildQuest = true;
        q.turnedIn = true;
        comboPlayer.getQuestLog().getQuests().push_back(q);
    }
    assert(GuildFinanceSystem::createLoan(comboPlayer, 200, 1, false).success);
    const long long comboWallet = comboPlayer.getInventory().getTotalCopper();
    GuildFinancePaymentResult comboCredit = GuildFinanceSystem::payAffiliatedService(
        comboPlayer,
        comboWallet + 50,
        "combo_partner",
        "Service combiné de test",
        true
    );
    assert(comboCredit.success);
    assert(GuildFinanceSystem::overdraftUsedCopper(comboPlayer) == 50);
    comboPlayer.getInventory().earnCopper(80);
    const long long comboLoanBefore = GuildFinanceSystem::activeLoan(comboPlayer).remainingCopper;
    GuildDebtRepaymentResult priorityPayment = GuildFinanceSystem::repayDebtsFromWallet(
        comboPlayer,
        30,
        GuildDebtRepaymentPriority::OverdraftFirst
    );
    assert(priorityPayment.success);
    assert(priorityPayment.overdraftPaidCopper == 30);
    assert(priorityPayment.loanPaidCopper == 0);
    assert(GuildFinanceSystem::overdraftUsedCopper(comboPlayer) == 20);
    assert(GuildFinanceSystem::activeLoan(comboPlayer).remainingCopper == comboLoanBefore);

    GuildDebtRepaymentResult splitPayment = GuildFinanceSystem::repayDebtsFromGuildBalance(
        comboPlayer,
        40,
        GuildDebtRepaymentPriority::Split
    );
    assert(splitPayment.success);
    assert(splitPayment.overdraftPaidCopper == 20);
    assert(splitPayment.loanPaidCopper == 20);
    assert(GuildFinanceSystem::overdraftUsedCopper(comboPlayer) == 0);
    assert(GuildFinanceSystem::activeLoan(comboPlayer).remainingCopper == comboLoanBefore - 20);

    fs::remove_all("assets/saves");
    return 0;
}
