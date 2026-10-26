// EN: Guild finance: membership, deposits, network overdraft and physical guild loans.
// FR: Finances de guilde : adhésion, dépôts, découvert réseau et prêts physiques.
#include "economy/GuildFinanceSystem.hpp"

#include "economy/Money.hpp"
#include "entity/Player.hpp"
#include "quest/Quest.hpp"

#include <algorithm>
#include <limits>
#include <string>

namespace
{
    constexpr const char* kMembershipId = "guild_adventurer_standard_weekly";
    constexpr const char* kMembershipName = "Cotisation aventurier standard";
    constexpr int kMembershipPriceEconomyUnits = 160;
    constexpr const char* kDepositCategory = "guild_finance_deposits";
    constexpr const char* kWithdrawalCategory = "guild_finance_withdrawals";
    constexpr const char* kOverdraftDrawCategory = "guild_finance_overdraft_draws";
    constexpr const char* kOverdraftRepaymentCategory = "guild_finance_overdraft_repayments";
    constexpr const char* kLoanPrincipalCategory = "guild_finance_loan_principals";
    constexpr const char* kLoanInitialTotalCategory = "guild_finance_loan_initial_totals";
    constexpr const char* kLoanTermWeeksCategory = "guild_finance_loan_term_weeks";
    constexpr const char* kLoanBaseRateCategory = "guild_finance_loan_base_rates";
    constexpr const char* kLoanLongSurchargeCategory = "guild_finance_loan_long_surcharges";
    constexpr const char* kLoanAutoCategory = "guild_finance_loan_auto";
    constexpr const char* kLoanRepaymentCategory = "guild_finance_loan_repayments";
    constexpr const char* kLoanExtensionWeeksCategory = "guild_finance_loan_extension_weeks";
    constexpr const char* kLoanExtensionRateCategory = "guild_finance_loan_extension_rates";
    constexpr const char* kLoanExtensionFeeCategory = "guild_finance_loan_extension_fees";
    constexpr const char* kLoanOverdueCategory = "guild_finance_loan_overdue";
    constexpr const char* kLoanReminderCategory = "guild_finance_loan_reminders";
    constexpr const char* kLoanClosedCategory = "guild_finance_loan_closed";
    constexpr const char* kLoanClosedOnTimeCategory = "guild_finance_loan_closed_on_time";
    constexpr const char* kAccountKey = "adventurer_guild_account";
    constexpr long long kRegisteredAccountCapacityCopper = 2LL * Money::COPPER_PER_GOLD;
    constexpr long long kMemberAccountCapacityCopper = 5LL * Money::COPPER_PER_GOLD;
    constexpr int kLoanTotalCostRateCapPercent = 30;

    long long categoryTotal(const Player& player, const std::string& category)
    {
        long long total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.count > 0)
            {
                total += static_cast<long long>(record.count);
            }
        }
        return total;
    }

    long long categoryKeyTotal(const Player& player, const std::string& category, const std::string& key)
    {
        long long total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key && record.count > 0)
            {
                total += static_cast<long long>(record.count);
            }
        }
        return total;
    }

    const PlayerJournalRecord* findCategoryKeyRecord(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return &record;
            }
        }
        return nullptr;
    }

    bool recordCopperAmount(Player& player, const std::string& category, const std::string& label, long long amount, const std::string& key = kAccountKey)
    {
        if (amount <= 0) return false;
        long long remaining = amount;
        while (remaining > 0)
        {
            const int chunk = static_cast<int>(std::min<long long>(remaining, std::numeric_limits<int>::max()));
            player.recordCanonicalEvent(category, key, label, chunk);
            remaining -= chunk;
        }
        return true;
    }

    long long percentageFee(long long principalCopper, int percent)
    {
        if (principalCopper <= 0 || percent <= 0) return 0;
        return (principalCopper * static_cast<long long>(percent) + 99LL) / 100LL;
    }

    int extensionRateForWeeks(int extraWeeks)
    {
        if (extraWeeks == 1) return 3;
        if (extraWeeks == 2) return 5;
        if (extraWeeks == 3) return 7;
        return 0;
    }

    std::string nextLoanId(const Player& player)
    {
        const std::string prefix = "loan_d" + std::to_string(player.getWorldDaysElapsed()) + "_";
        int sequence = 1;
        while (true)
        {
            const std::string candidate = prefix + std::to_string(sequence);
            if (findCategoryKeyRecord(player, kLoanPrincipalCategory, candidate) == nullptr)
            {
                return candidate;
            }
            ++sequence;
        }
    }

    GuildLoanState loanStateForId(const Player& player, const std::string& loanId)
    {
        GuildLoanState loan;
        const PlayerJournalRecord* principalRecord = findCategoryKeyRecord(player, kLoanPrincipalCategory, loanId);
        if (principalRecord == nullptr || principalRecord->count <= 0)
        {
            return loan;
        }

        loan.loanId = loanId;
        loan.principalCopper = principalRecord->count;
        loan.originalTotalDueCopper = categoryKeyTotal(player, kLoanInitialTotalCategory, loanId);
        loan.extensionFeesCopper = categoryKeyTotal(player, kLoanExtensionFeeCategory, loanId);
        loan.totalDueCopper = loan.originalTotalDueCopper + loan.extensionFeesCopper;
        loan.repaidCopper = categoryKeyTotal(player, kLoanRepaymentCategory, loanId);
        loan.remainingCopper = std::max(0LL, loan.totalDueCopper - loan.repaidCopper);
        loan.startDay = principalRecord->lastDay;
        loan.originalTermWeeks = static_cast<int>(categoryKeyTotal(player, kLoanTermWeeksCategory, loanId));
        loan.extensionWeeks = static_cast<int>(categoryKeyTotal(player, kLoanExtensionWeeksCategory, loanId));
        loan.dueDay = loan.startDay + (loan.originalTermWeeks + loan.extensionWeeks) * 7;
        loan.baseRatePercent = static_cast<int>(categoryKeyTotal(player, kLoanBaseRateCategory, loanId));
        loan.longDurationSurchargePercent = static_cast<int>(categoryKeyTotal(player, kLoanLongSurchargeCategory, loanId));
        loan.extensionRatePercent = static_cast<int>(categoryKeyTotal(player, kLoanExtensionRateCategory, loanId));
        loan.totalCostRatePercent = loan.baseRatePercent + loan.longDurationSurchargePercent + loan.extensionRatePercent;
        loan.automaticRepayment = categoryKeyTotal(player, kLoanAutoCategory, loanId) > 0;
        loan.overdue = categoryKeyTotal(player, kLoanOverdueCategory, loanId) > 0
            || (loan.remainingCopper > 0 && player.getWorldDaysElapsed() > loan.dueDay);
        loan.active = loan.remainingCopper > 0;
        return loan;
    }

    void recordLoanClosureIfNeeded(Player& player, const GuildLoanState& beforePayment, long long paymentAmount)
    {
        if (!beforePayment.active || paymentAmount < beforePayment.remainingCopper)
        {
            return;
        }
        if (categoryKeyTotal(player, kLoanClosedCategory, beforePayment.loanId) <= 0)
        {
            player.recordCanonicalEvent(kLoanClosedCategory, beforePayment.loanId, "Prêt de guilde entièrement remboursé", 1);
            const bool onTime = !beforePayment.overdue && player.getWorldDaysElapsed() <= beforePayment.dueDay;
            if (onTime)
            {
                player.recordCanonicalEvent(kLoanClosedOnTimeCategory, beforePayment.loanId, "Prêt remboursé dans les temps", 1);
                // "Bon payeur" récompense une habitude, pas un unique remboursement.
                if (categoryTotal(player, kLoanClosedOnTimeCategory) >= 3) player.grantTitle("Bon payeur");
                if (player.getWorldDaysElapsed() < beforePayment.dueDay) player.grantTitle("Avant l'échéance");
            }
            if (beforePayment.extensionWeeks > 0) player.grantTitle("On peut s'arranger");
            player.recordHistoricalEvent(
                "guild_finance",
                "loan_repaid",
                std::string("Prêt de guilde remboursé intégralement") + (onTime ? " dans les temps." : " après échéance."),
                false
            );
        }
    }

    void allocateDebtPayment(
        long long amount,
        long long overdraftDebt,
        long long loanDebt,
        GuildDebtRepaymentPriority priority,
        long long& overdraftPayment,
        long long& loanPayment
    )
    {
        overdraftPayment = 0;
        loanPayment = 0;
        if (amount <= 0) return;

        if (priority == GuildDebtRepaymentPriority::OverdraftFirst)
        {
            overdraftPayment = std::min(amount, overdraftDebt);
            loanPayment = std::min(amount - overdraftPayment, loanDebt);
            return;
        }
        if (priority == GuildDebtRepaymentPriority::LoanFirst)
        {
            loanPayment = std::min(amount, loanDebt);
            overdraftPayment = std::min(amount - loanPayment, overdraftDebt);
            return;
        }

        overdraftPayment = std::min(overdraftDebt, (amount + 1) / 2);
        loanPayment = std::min(loanDebt, amount - overdraftPayment);
        long long remaining = amount - overdraftPayment - loanPayment;
        if (remaining > 0)
        {
            const long long extraOverdraft = std::min(remaining, overdraftDebt - overdraftPayment);
            overdraftPayment += extraOverdraft;
            remaining -= extraOverdraft;
        }
        if (remaining > 0)
        {
            loanPayment += std::min(remaining, loanDebt - loanPayment);
        }
    }
}

const std::string& GuildFinanceSystem::membershipId()
{
    static const std::string value = kMembershipId;
    return value;
}

const std::string& GuildFinanceSystem::membershipName()
{
    static const std::string value = kMembershipName;
    return value;
}

int GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
{
    return kMembershipPriceEconomyUnits;
}

bool GuildFinanceSystem::isMember(const Player& player)
{
    return player.hasActiveLocalSubscription(kMembershipId)
        || player.hasActiveLocalSubscription("guild_adventurer_silver_weekly");
}

long long GuildFinanceSystem::depositedBalanceCopper(const Player& player)
{
    const long long deposits = categoryTotal(player, kDepositCategory);
    const long long withdrawals = categoryTotal(player, kWithdrawalCategory);
    return std::max(0LL, deposits - withdrawals);
}

long long GuildFinanceSystem::accountCapacityCopper(const Player& player)
{
    if (!player.hasTitle("Aventurier")) return 0;
    return isMember(player) ? kMemberAccountCapacityCopper : kRegisteredAccountCapacityCopper;
}

long long GuildFinanceSystem::accountRemainingCapacityCopper(const Player& player)
{
    return std::max(0LL, accountCapacityCopper(player) - depositedBalanceCopper(player));
}

bool GuildFinanceSystem::depositCopper(Player& player, long long amount, std::string* failureReason)
{
    if (amount <= 0)
    {
        if (failureReason) *failureReason = "Le montant doit être supérieur à zéro.";
        return false;
    }
    if (!player.hasTitle("Aventurier"))
    {
        if (failureReason) *failureReason = "L'inscription Aventurier est requise pour ouvrir un solde de guilde.";
        return false;
    }
    if (player.getInventory().getTotalCopper() < amount)
    {
        if (failureReason) *failureReason = "La bourse ne contient pas assez d'argent physique.";
        return false;
    }
    const long long remainingCapacity = accountRemainingCapacityCopper(player);
    if (amount > remainingCapacity)
    {
        if (failureReason)
        {
            *failureReason = remainingCapacity <= 0
                ? "Le compte de guilde a atteint sa capacité de dépôt. Le coffre personnel reste prévu pour le stockage sécurisé plus important."
                : "Ce dépôt dépasserait la capacité du compte de guilde. Marge restante : " + Money::formatCopper(remainingCapacity) + ".";
        }
        return false;
    }
    if (!player.getInventory().spendCopper(amount))
    {
        if (failureReason) *failureReason = "Le dépôt n'a pas pu être prélevé proprement dans la bourse.";
        return false;
    }

    recordCopperAmount(player, kDepositCategory, "Dépôt volontaire au compte de guilde", amount);
    player.recordHistoricalEvent("guild_finance", "guild_account", "Dépôt au compte de guilde : " + Money::formatCopper(amount), false);
    return true;
}

bool GuildFinanceSystem::withdrawCopper(Player& player, long long amount, std::string* failureReason)
{
    if (amount <= 0)
    {
        if (failureReason) *failureReason = "Le montant doit être supérieur à zéro.";
        return false;
    }

    const long long balance = depositedBalanceCopper(player);
    if (balance < amount)
    {
        if (failureReason) *failureReason = "Le solde déposé à la guilde est insuffisant.";
        return false;
    }

    player.getInventory().earnCopper(amount);
    recordCopperAmount(player, kWithdrawalCategory, "Retrait volontaire du compte de guilde", amount);
    player.recordHistoricalEvent("guild_finance", "guild_account", "Retrait du compte de guilde : " + Money::formatCopper(amount), false);
    return true;
}

int GuildFinanceSystem::financialTrustScore(const Player& player)
{
    if (!player.hasTitle("Aventurier")) return 0;

    int score = 45;
    int completedGuild = 0;
    int failedGuild = 0;
    for (const Quest& quest : player.getQuestLog().getQuests())
    {
        if (!quest.guildQuest) continue;
        if (quest.turnedIn) ++completedGuild;
        if (quest.failed) ++failedGuild;
    }

    score += std::min(25, completedGuild * 2);
    score -= std::min(30, failedGuild * 6);
    if (isMember(player)) score += 8;
    if (depositedBalanceCopper(player) > 0) score += 2;

    const int repaidOnTime = static_cast<int>(categoryTotal(player, kLoanClosedOnTimeCategory));
    score += std::min(15, repaidOnTime * 5);
    if (overdraftUsedCopper(player) > 0) score -= 3;
    if (activeLoan(player).overdue) score -= 12;

    return std::clamp(score, 0, 100);
}

std::string GuildFinanceSystem::financialTrustLabel(int score)
{
    if (score >= 85) return "Excellent dossier";
    if (score >= 70) return "Dossier fiable";
    if (score >= 55) return "Dossier correct";
    if (score >= 40) return "Dossier prudent";
    if (score > 0) return "Dossier fragile";
    return "Aucun dossier financier";
}

long long GuildFinanceSystem::overdraftUsedCopper(const Player& player)
{
    const long long draws = categoryTotal(player, kOverdraftDrawCategory);
    const long long repayments = categoryTotal(player, kOverdraftRepaymentCategory);
    return std::max(0LL, draws - repayments);
}

long long GuildFinanceSystem::overdraftCeilingCopper(const Player& player)
{
    if (!isMember(player)) return 0;
    // Une échéance de prêt dépassée suspend immédiatement toute nouvelle utilisation de crédit.
    // La dette existante reste remboursable et n'est jamais effacée par cette restriction.
    if (activeLoan(player).overdue) return 0;

    const int score = financialTrustScore(player);
    if (score >= 90) return Money::COPPER_PER_GOLD;
    if (score >= 80) return 750;
    if (score >= 70) return 500;
    if (score >= 60) return 250;
    if (score >= 50) return 100;
    return 30;
}

long long GuildFinanceSystem::overdraftRemainingCopper(const Player& player)
{
    return std::max(0LL, overdraftCeilingCopper(player) - overdraftUsedCopper(player));
}

GuildFinancePaymentResult GuildFinanceSystem::payAffiliatedService(
    Player& player,
    long long amount,
    const std::string& serviceKey,
    const std::string& serviceLabel,
    bool authorizeOverdraft
)
{
    GuildFinancePaymentResult result;
    if (amount <= 0)
    {
        result.failureReason = "Le montant du service doit être supérieur à zéro.";
        return result;
    }

    const long long wallet = player.getInventory().getTotalCopper();
    if (wallet >= amount)
    {
        if (!player.getInventory().spendCopper(amount))
        {
            result.failureReason = "Le paiement n'a pas pu être prélevé proprement dans la bourse.";
            return result;
        }
        result.success = true;
        result.walletPaidCopper = amount;
        player.recordHistoricalEvent("guild_finance", serviceKey, "Service affilié payé sans crédit : " + serviceLabel + " — " + Money::formatCopper(amount), false);
        return result;
    }

    const long long neededCredit = amount - wallet;
    if (!authorizeOverdraft)
    {
        result.failureReason = "La bourse est insuffisante et l'utilisation du découvert n'a pas été autorisée.";
        return result;
    }
    if (!isMember(player))
    {
        result.failureReason = "Le découvert de guilde est réservé aux adhérents actifs.";
        return result;
    }
    if (overdraftRemainingCopper(player) < neededCredit)
    {
        result.failureReason = "La marge de découvert restante est insuffisante.";
        return result;
    }

    if (wallet > 0 && !player.getInventory().spendCopper(wallet))
    {
        result.failureReason = "La part physique du paiement n'a pas pu être prélevée proprement.";
        return result;
    }

    recordCopperAmount(player, kOverdraftDrawCategory, "Crédit réseau : " + serviceLabel, neededCredit);
    player.recordCanonicalEvent("guild_finance_credit_services", serviceKey, serviceLabel, 1);
    player.recordHistoricalEvent(
        "guild_finance",
        serviceKey,
        "Découvert utilisé pour " + serviceLabel + " : " + Money::formatCopper(neededCredit),
        false
    );
    result.success = true;
    result.walletPaidCopper = wallet;
    result.overdraftDrawnCopper = neededCredit;
    return result;
}

bool GuildFinanceSystem::repayOverdraftFromWallet(Player& player, long long amount, std::string* failureReason)
{
    if (amount <= 0)
    {
        if (failureReason) *failureReason = "Le remboursement doit être supérieur à zéro.";
        return false;
    }
    const long long debt = overdraftUsedCopper(player);
    if (debt <= 0 || amount > debt)
    {
        if (failureReason) *failureReason = debt <= 0 ? "Aucun découvert n'est actuellement utilisé." : "Le remboursement dépasse le découvert utilisé.";
        return false;
    }
    if (player.getInventory().getTotalCopper() < amount || !player.getInventory().spendCopper(amount))
    {
        if (failureReason) *failureReason = "La bourse ne contient pas assez d'argent physique pour ce remboursement.";
        return false;
    }
    recordCopperAmount(player, kOverdraftRepaymentCategory, "Remboursement du découvert depuis la bourse", amount);
    player.recordHistoricalEvent("guild_finance", "overdraft_repayment_wallet", "Remboursement du découvert depuis la bourse : " + Money::formatCopper(amount), false);
    return true;
}

bool GuildFinanceSystem::repayOverdraftFromGuildBalance(Player& player, long long amount, std::string* failureReason)
{
    if (amount <= 0)
    {
        if (failureReason) *failureReason = "Le remboursement doit être supérieur à zéro.";
        return false;
    }
    const long long debt = overdraftUsedCopper(player);
    if (debt <= 0 || amount > debt)
    {
        if (failureReason) *failureReason = debt <= 0 ? "Aucun découvert n'est actuellement utilisé." : "Le remboursement dépasse le découvert utilisé.";
        return false;
    }
    if (depositedBalanceCopper(player) < amount)
    {
        if (failureReason) *failureReason = "Le solde déposé à la guilde est insuffisant pour ce remboursement.";
        return false;
    }

    recordCopperAmount(player, kWithdrawalCategory, "Affectation du solde de guilde au remboursement du découvert", amount);
    recordCopperAmount(player, kOverdraftRepaymentCategory, "Remboursement du découvert depuis le solde de guilde", amount);
    player.recordHistoricalEvent("guild_finance", "overdraft_repayment_account", "Remboursement du découvert depuis le compte de guilde : " + Money::formatCopper(amount), false);
    return true;
}

GuildLoanTerms GuildFinanceSystem::loanTermsForWeeks(int weeks)
{
    GuildLoanTerms terms;
    terms.weeks = weeks;
    if (weeks == 1)
    {
        terms.baseRatePercent = 12;
        terms.minimumTrustScore = 55;
    }
    else if (weeks == 2)
    {
        terms.baseRatePercent = 14;
        terms.minimumTrustScore = 60;
    }
    else if (weeks == 3)
    {
        terms.baseRatePercent = 16;
        terms.longDurationSurchargePercent = 2;
        terms.minimumTrustScore = 65;
    }
    else if (weeks == 4)
    {
        terms.baseRatePercent = 18;
        terms.longDurationSurchargePercent = 2;
        terms.minimumTrustScore = 70;
    }
    else if (weeks == 6)
    {
        terms.baseRatePercent = 21;
        terms.longDurationSurchargePercent = 3;
        terms.minimumTrustScore = 80;
    }
    else if (weeks == 8)
    {
        terms.baseRatePercent = 24;
        terms.longDurationSurchargePercent = 4;
        terms.minimumTrustScore = 90;
    }
    else
    {
        terms.weeks = 0;
        return terms;
    }
    terms.effectiveRatePercent = terms.baseRatePercent + terms.longDurationSurchargePercent;
    return terms;
}

long long GuildFinanceSystem::loanMaximumPrincipalCopper(const Player& player)
{
    if (!isMember(player)) return 0;
    const int score = financialTrustScore(player);
    if (score >= 90) return 5LL * Money::COPPER_PER_GOLD;
    if (score >= 85) return 2LL * Money::COPPER_PER_GOLD;
    if (score >= 75) return Money::COPPER_PER_GOLD;
    if (score >= 65) return 500;
    if (score >= 55) return 250;
    return 0;
}

GuildLoanState GuildFinanceSystem::activeLoan(const Player& player)
{
    GuildLoanState selected;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category != kLoanPrincipalCategory || record.count <= 0) continue;
        GuildLoanState candidate = loanStateForId(player, record.key);
        if (!candidate.active) continue;
        if (!selected.active || candidate.startDay > selected.startDay)
        {
            selected = candidate;
        }
    }
    return selected;
}

GuildLoanCreateResult GuildFinanceSystem::createLoan(Player& player, long long principalCopper, int weeks, bool automaticRepayment)
{
    GuildLoanCreateResult result;
    if (!player.hasTitle("Aventurier"))
    {
        result.failureReason = "L'inscription Aventurier est requise pour demander un prêt de guilde.";
        return result;
    }
    if (!isMember(player))
    {
        result.failureReason = "Une adhésion de guilde active est requise pour ouvrir un nouveau prêt.";
        return result;
    }
    if (activeLoan(player).active)
    {
        result.failureReason = "Un prêt de guilde est déjà actif. Aucun nouveau prêt ne peut servir à empiler ou rembourser l'ancien.";
        return result;
    }
    const GuildLoanTerms terms = loanTermsForWeeks(weeks);
    if (terms.weeks <= 0)
    {
        result.failureReason = "Durée de prêt invalide.";
        return result;
    }
    const int trust = financialTrustScore(player);
    if (trust < terms.minimumTrustScore)
    {
        result.failureReason = "Le dossier financier n'est pas encore assez fiable pour cette durée. Confiance requise : " + std::to_string(terms.minimumTrustScore) + "/100.";
        return result;
    }
    const long long maximum = loanMaximumPrincipalCopper(player);
    if (principalCopper <= 0 || principalCopper > maximum)
    {
        result.failureReason = maximum <= 0
            ? "Le dossier financier n'autorise encore aucun montant de prêt."
            : "Le montant demandé dépasse le plafond actuel de " + Money::formatCopper(maximum) + ".";
        return result;
    }

    const long long interest = percentageFee(principalCopper, terms.effectiveRatePercent);
    const long long totalDue = principalCopper + interest;
    const std::string loanId = nextLoanId(player);

    recordCopperAmount(player, kLoanPrincipalCategory, "Capital physique avancé par la guilde", principalCopper, loanId);
    recordCopperAmount(player, kLoanInitialTotalCategory, "Dette totale fixée à la signature", totalDue, loanId);
    player.recordCanonicalEvent(kLoanTermWeeksCategory, loanId, "Durée initiale du prêt", terms.weeks);
    player.recordCanonicalEvent(kLoanBaseRateCategory, loanId, "Taux de base du prêt", terms.baseRatePercent);
    if (terms.longDurationSurchargePercent > 0)
    {
        player.recordCanonicalEvent(kLoanLongSurchargeCategory, loanId, "Majoration de durée longue", terms.longDurationSurchargePercent);
    }
    if (automaticRepayment)
    {
        player.recordCanonicalEvent(kLoanAutoCategory, loanId, "Remboursement automatique à l'échéance depuis le compte de guilde", 1);
    }

    player.getInventory().earnCopper(principalCopper);
    player.recordHistoricalEvent(
        "guild_finance",
        "loan_opened",
        "Prêt de guilde signé : " + Money::formatCopper(principalCopper)
            + " sur " + std::to_string(weeks) + " semaine(s), coût signé " + std::to_string(terms.effectiveRatePercent)
            + " %, total dû " + Money::formatCopper(totalDue) + ".",
        false
    );

    player.grantTitle("Bon pour accord");
    if (trust >= 90)
    {
        player.grantTitle("La confiance se mérite");
        if (maximum >= 5LL * Money::COPPER_PER_GOLD) player.grantTitle("Capital confiance");
    }

    result.success = true;
    result.loan = loanStateForId(player, loanId);
    return result;
}

bool GuildFinanceSystem::repayLoanFromWallet(Player& player, long long amount, std::string* failureReason)
{
    const GuildLoanState loan = activeLoan(player);
    if (!loan.active)
    {
        if (failureReason) *failureReason = "Aucun prêt de guilde n'est actuellement actif.";
        return false;
    }
    if (amount <= 0 || amount > loan.remainingCopper)
    {
        if (failureReason) *failureReason = "Le remboursement doit être positif et ne peut pas dépasser le reste dû.";
        return false;
    }
    if (player.getInventory().getTotalCopper() < amount || !player.getInventory().spendCopper(amount))
    {
        if (failureReason) *failureReason = "La bourse ne contient pas assez d'argent physique pour ce remboursement.";
        return false;
    }
    recordCopperAmount(player, kLoanRepaymentCategory, "Remboursement manuel du prêt depuis la bourse", amount, loan.loanId);
    player.recordHistoricalEvent("guild_finance", "loan_repayment_wallet", "Remboursement du prêt depuis la bourse : " + Money::formatCopper(amount), false);
    recordLoanClosureIfNeeded(player, loan, amount);
    return true;
}

bool GuildFinanceSystem::repayLoanFromGuildBalance(Player& player, long long amount, std::string* failureReason)
{
    const GuildLoanState loan = activeLoan(player);
    if (!loan.active)
    {
        if (failureReason) *failureReason = "Aucun prêt de guilde n'est actuellement actif.";
        return false;
    }
    if (amount <= 0 || amount > loan.remainingCopper)
    {
        if (failureReason) *failureReason = "Le remboursement doit être positif et ne peut pas dépasser le reste dû.";
        return false;
    }
    if (depositedBalanceCopper(player) < amount)
    {
        if (failureReason) *failureReason = "Le compte de guilde ne contient pas assez de fonds déposés pour ce remboursement.";
        return false;
    }

    recordCopperAmount(player, kWithdrawalCategory, "Affectation du solde de guilde au remboursement du prêt", amount);
    recordCopperAmount(player, kLoanRepaymentCategory, "Remboursement manuel du prêt depuis le compte de guilde", amount, loan.loanId);
    player.recordHistoricalEvent("guild_finance", "loan_repayment_account", "Remboursement du prêt depuis le compte de guilde : " + Money::formatCopper(amount), false);
    recordLoanClosureIfNeeded(player, loan, amount);
    return true;
}

GuildLoanExtensionResult GuildFinanceSystem::extendActiveLoan(Player& player, int extraWeeks)
{
    GuildLoanExtensionResult result;
    result.requestedWeeks = extraWeeks;
    const GuildLoanState loan = activeLoan(player);
    if (!loan.active)
    {
        result.failureReason = "Aucun prêt actif ne peut être prolongé.";
        return result;
    }
    if (player.getWorldDaysElapsed() >= loan.dueDay)
    {
        result.failureReason = "L'échéance est déjà atteinte : une prolongation préventive n'est plus possible. Il faudra passer par la future négociation de retard.";
        return result;
    }
    const int requestedRate = extensionRateForWeeks(extraWeeks);
    if (requestedRate <= 0)
    {
        result.failureReason = "Une prolongation doit être de 1, 2 ou 3 semaines.";
        return result;
    }
    const int remainingRateRoom = std::max(0, kLoanTotalCostRateCapPercent - loan.totalCostRatePercent);
    const int appliedRate = std::min(requestedRate, remainingRateRoom);
    if (appliedRate <= 0)
    {
        result.failureReason = "Le plafond global de frais du prêt est déjà atteint ; la guilde refuse une nouvelle prolongation.";
        return result;
    }

    const long long addedFee = percentageFee(loan.principalCopper, appliedRate);
    player.recordCanonicalEvent(kLoanExtensionWeeksCategory, loan.loanId, "Prolongation préventive du prêt", extraWeeks);
    player.recordCanonicalEvent(kLoanExtensionRateCategory, loan.loanId, "Taux ajouté par prolongation", appliedRate);
    recordCopperAmount(player, kLoanExtensionFeeCategory, "Frais fixes de prolongation", addedFee, loan.loanId);
    player.recordHistoricalEvent(
        "guild_finance",
        "loan_extension",
        "Prolongation demandée avant échéance : +" + std::to_string(extraWeeks) + " semaine(s), +"
            + std::to_string(appliedRate) + " %, soit " + Money::formatCopper(addedFee) + " ajouté(s) au contrat.",
        false
    );

    result.success = true;
    result.appliedRatePercent = appliedRate;
    result.addedFeeCopper = addedFee;
    result.loan = loanStateForId(player, loan.loanId);
    return result;
}

GuildLoanDayResult GuildFinanceSystem::processEndOfWorldDayLoan(Player& player)
{
    GuildLoanDayResult result;
    GuildLoanState loan = activeLoan(player);
    if (!loan.active) return result;

    result.hadActiveLoan = true;
    if (player.getWorldDaysElapsed() < loan.dueDay)
    {
        return result;
    }

    if (player.getWorldDaysElapsed() == loan.dueDay && loan.automaticRepayment)
    {
        result.automaticPaymentAttempted = true;
        const long long available = depositedBalanceCopper(player);
        const long long payment = std::min(available, loan.remainingCopper);
        if (payment > 0)
        {
            recordCopperAmount(player, kWithdrawalCategory, "Prélèvement automatique du compte de guilde à l'échéance", payment);
            recordCopperAmount(player, kLoanRepaymentCategory, "Remboursement automatique du prêt à l'échéance", payment, loan.loanId);
            recordLoanClosureIfNeeded(player, loan, payment);
            result.automaticPaymentCopper = payment;
            result.reportLines.push_back("Prêt de guilde : prélèvement automatique de " + Money::formatCopper(payment) + " depuis le solde déposé.");
            loan = loanStateForId(player, loan.loanId);
        }
        else
        {
            result.reportLines.push_back("Prêt de guilde : prélèvement automatique prévu, mais le compte de guilde ne contient aucun fonds disponible.");
        }
    }

    if (!loan.active)
    {
        result.fullyRepaid = true;
        result.reportLines.push_back("Prêt de guilde : contrat entièrement remboursé à l'échéance.");
        return result;
    }

    if (categoryKeyTotal(player, kLoanOverdueCategory, loan.loanId) <= 0)
    {
        player.recordCanonicalEvent(kLoanOverdueCategory, loan.loanId, "Échéance de prêt dépassée avec dette restante", 1);
        player.recordHistoricalEvent("guild_finance", "loan_overdue", "Échéance de prêt dépassée : " + Money::formatCopper(loan.remainingCopper) + " reste(nt) dû(s).", false);
        result.becameOverdue = true;
        result.reportLines.push_back("Prêt de guilde : échéance atteinte avec " + Money::formatCopper(loan.remainingCopper) + " encore dû(s). Aucun intérêt composé n'est ajouté automatiquement.");
        result.reportLines.push_back("La guilde suspend désormais tout nouveau crédit jusqu'à régularisation ou future négociation du retard.");
    }

    const int overdueDays = std::max(1, player.getWorldDaysElapsed() - loan.dueDay + 1);
    if (overdueDays == 1 || overdueDays % 7 == 0)
    {
        const std::string reminderKey = loan.loanId + ":day_" + std::to_string(overdueDays);
        if (categoryKeyTotal(player, kLoanReminderCategory, reminderKey) <= 0)
        {
            player.recordCanonicalEvent(kLoanReminderCategory, reminderKey, "Rappel de dette de guilde", 1);
            result.reportLines.push_back("Rappel de la guilde : " + Money::formatCopper(loan.remainingCopper) + " reste(nt) dû(s). Le retard dure " + std::to_string(overdueDays) + " jour(s).");
        }
    }
    return result;
}

GuildDebtSituation GuildFinanceSystem::assessDebtSituation(const Player& player)
{
    GuildDebtSituation out;
    const GuildLoanState loan = activeLoan(player);
    const long long overdraft = overdraftUsedCopper(player);
    out.totalDebtCopper = overdraft + (loan.active ? loan.remainingCopper : 0);
    out.immediatelyAvailableCopper = player.getInventory().getTotalCopper() + depositedBalanceCopper(player);
    out.overdue = loan.active && loan.overdue;
    out.overdueDays = out.overdue ? std::max(1, player.getWorldDaysElapsed() - loan.dueDay + 1) : 0;
    out.newCreditRestricted = out.overdue;
    out.canCoverNow = out.totalDebtCopper > 0 && out.immediatelyAvailableCopper >= out.totalDebtCopper;

    // On ne qualifie jamais automatiquement la situation de mauvaise foi.
    // Le moteur n'en a pas encore la preuve : refus explicites, négociations et témoins viendront plus tard.
    const long long hardshipThreshold = out.totalDebtCopper <= 0 ? 0 : std::max(1LL, out.totalDebtCopper / 4);
    out.likelyFinancialHardship = out.overdue && out.immediatelyAvailableCopper < hardshipThreshold;

    if (!out.overdue) out.statusLabel = "Aucun retard actif";
    else if (out.likelyFinancialHardship) out.statusLabel = "Difficulté financière probable";
    else if (out.canCoverNow) out.statusLabel = "Fonds suffisants — situation à régulariser";
    else out.statusLabel = "Capacité de paiement partielle — situation à discuter";
    return out;
}

GuildDebtRepaymentResult GuildFinanceSystem::repayDebtsFromWallet(Player& player, long long amount, GuildDebtRepaymentPriority priority)
{
    GuildDebtRepaymentResult result;
    const long long overdraftDebt = overdraftUsedCopper(player);
    const GuildLoanState loan = activeLoan(player);
    const long long loanDebt = loan.active ? loan.remainingCopper : 0;
    const long long totalDebt = overdraftDebt + loanDebt;
    if (totalDebt <= 0)
    {
        result.failureReason = "Aucune dette de guilde n'est actuellement active.";
        return result;
    }
    if (amount <= 0 || amount > totalDebt)
    {
        result.failureReason = "Le remboursement doit être positif et ne peut pas dépasser la dette totale.";
        return result;
    }
    if (player.getInventory().getTotalCopper() < amount || !player.getInventory().spendCopper(amount))
    {
        result.failureReason = "La bourse ne contient pas assez d'argent physique pour ce remboursement global.";
        return result;
    }

    allocateDebtPayment(amount, overdraftDebt, loanDebt, priority, result.overdraftPaidCopper, result.loanPaidCopper);
    if (result.overdraftPaidCopper > 0)
    {
        recordCopperAmount(player, kOverdraftRepaymentCategory, "Remboursement global affecté au découvert", result.overdraftPaidCopper);
    }
    if (result.loanPaidCopper > 0 && loan.active)
    {
        recordCopperAmount(player, kLoanRepaymentCategory, "Remboursement global affecté au prêt", result.loanPaidCopper, loan.loanId);
        recordLoanClosureIfNeeded(player, loan, result.loanPaidCopper);
    }
    player.recordHistoricalEvent(
        "guild_finance",
        "combined_debt_repayment_wallet",
        "Remboursement global depuis la bourse : " + Money::formatCopper(amount)
            + " (découvert " + Money::formatCopper(result.overdraftPaidCopper)
            + ", prêt " + Money::formatCopper(result.loanPaidCopper) + ").",
        false
    );
    result.success = true;
    result.totalPaidCopper = amount;
    return result;
}

GuildDebtRepaymentResult GuildFinanceSystem::repayDebtsFromGuildBalance(Player& player, long long amount, GuildDebtRepaymentPriority priority)
{
    GuildDebtRepaymentResult result;
    const long long overdraftDebt = overdraftUsedCopper(player);
    const GuildLoanState loan = activeLoan(player);
    const long long loanDebt = loan.active ? loan.remainingCopper : 0;
    const long long totalDebt = overdraftDebt + loanDebt;
    if (totalDebt <= 0)
    {
        result.failureReason = "Aucune dette de guilde n'est actuellement active.";
        return result;
    }
    if (amount <= 0 || amount > totalDebt)
    {
        result.failureReason = "Le remboursement doit être positif et ne peut pas dépasser la dette totale.";
        return result;
    }
    if (depositedBalanceCopper(player) < amount)
    {
        result.failureReason = "Le compte de guilde ne contient pas assez de fonds déposés pour ce remboursement global.";
        return result;
    }

    allocateDebtPayment(amount, overdraftDebt, loanDebt, priority, result.overdraftPaidCopper, result.loanPaidCopper);
    recordCopperAmount(player, kWithdrawalCategory, "Affectation du compte de guilde au remboursement global des dettes", amount);
    if (result.overdraftPaidCopper > 0)
    {
        recordCopperAmount(player, kOverdraftRepaymentCategory, "Remboursement global affecté au découvert", result.overdraftPaidCopper);
    }
    if (result.loanPaidCopper > 0 && loan.active)
    {
        recordCopperAmount(player, kLoanRepaymentCategory, "Remboursement global affecté au prêt", result.loanPaidCopper, loan.loanId);
        recordLoanClosureIfNeeded(player, loan, result.loanPaidCopper);
    }
    player.recordHistoricalEvent(
        "guild_finance",
        "combined_debt_repayment_account",
        "Remboursement global depuis le compte de guilde : " + Money::formatCopper(amount)
            + " (découvert " + Money::formatCopper(result.overdraftPaidCopper)
            + ", prêt " + Money::formatCopper(result.loanPaidCopper) + ").",
        false
    );
    result.success = true;
    result.totalPaidCopper = amount;
    return result;
}

GuildDebtRepaymentResult GuildFinanceSystem::clearAllDebtsByCheat(Player& player)
{
    GuildDebtRepaymentResult result;
    const long long overdraftDebt = overdraftUsedCopper(player);
    const GuildLoanState loan = activeLoan(player);
    const long long loanDebt = loan.active ? loan.remainingCopper : 0;

    if (overdraftDebt > 0)
    {
        recordCopperAmount(player, kOverdraftRepaymentCategory, "Effacement artificiel du découvert par code de triche", overdraftDebt);
    }
    if (loanDebt > 0)
    {
        recordCopperAmount(player, kLoanRepaymentCategory, "Effacement artificiel du prêt par code de triche", loanDebt, loan.loanId);
        if (categoryKeyTotal(player, kLoanClosedCategory, loan.loanId) <= 0)
        {
            player.recordCanonicalEvent(kLoanClosedCategory, loan.loanId, "Prêt fermé artificiellement par code de triche", 1);
        }
    }

    result.success = true;
    result.overdraftPaidCopper = overdraftDebt;
    result.loanPaidCopper = loanDebt;
    result.totalPaidCopper = overdraftDebt + loanDebt;
    player.recordHistoricalEvent(
        "guild_finance",
        "cheat_debt_clear",
        "Effacement artificiel des dettes de guilde : " + Money::formatCopper(result.totalPaidCopper) + ".",
        true
    );
    return result;
}

GuildFinanceOverview GuildFinanceSystem::overview(const Player& player)
{
    GuildFinanceOverview out;
    out.adventurerRegistered = player.hasTitle("Aventurier");
    out.member = isMember(player);
    out.cancellationRequested = player.isLocalSubscriptionCancellationRequested(kMembershipId);
    out.membershipExpiresAtDay = player.getLocalSubscriptionExpiresAtDay(kMembershipId);
    out.membershipRenewalPriceEconomyUnits = kMembershipPriceEconomyUnits;
    out.depositedCopper = depositedBalanceCopper(player);
    out.accountCapacityCopper = accountCapacityCopper(player);
    out.accountRemainingCapacityCopper = accountRemainingCapacityCopper(player);
    out.trustScore = financialTrustScore(player);
    out.trustLabel = financialTrustLabel(out.trustScore);
    out.overdraftUsedCopper = overdraftUsedCopper(player);
    out.overdraftCeilingCopper = overdraftCeilingCopper(player);
    out.overdraftRemainingCopper = overdraftRemainingCopper(player);
    return out;
}
