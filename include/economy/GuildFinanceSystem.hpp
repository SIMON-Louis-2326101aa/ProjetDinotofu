// EN: Guild finance: membership, deposits, overdraft and guild loans.
// FR: Finances de guilde : adhésion, dépôts, découvert et prêts de guilde.
#ifndef INCLUDE_ECONOMY_GUILDFINANCESYSTEM_HPP
#define INCLUDE_ECONOMY_GUILDFINANCESYSTEM_HPP

#include <string>
#include <vector>

class Player;

struct GuildFinanceOverview
{
    bool adventurerRegistered = false;
    bool member = false;
    bool cancellationRequested = false;
    int membershipExpiresAtDay = -1;
    int membershipRenewalPriceEconomyUnits = 160;
    long long depositedCopper = 0;
    long long accountCapacityCopper = 0;
    long long accountRemainingCapacityCopper = 0;
    int trustScore = 0;
    std::string trustLabel;
    long long overdraftUsedCopper = 0;
    long long overdraftCeilingCopper = 0;
    long long overdraftRemainingCopper = 0;
};

struct GuildFinancePaymentResult
{
    bool success = false;
    long long walletPaidCopper = 0;
    long long overdraftDrawnCopper = 0;
    std::string failureReason;
};

struct GuildLoanTerms
{
    int weeks = 0;
    int baseRatePercent = 0;
    int longDurationSurchargePercent = 0;
    int effectiveRatePercent = 0;
    int minimumTrustScore = 0;
};

struct GuildLoanState
{
    bool active = false;
    std::string loanId;
    long long principalCopper = 0;
    long long originalTotalDueCopper = 0;
    long long extensionFeesCopper = 0;
    long long totalDueCopper = 0;
    long long repaidCopper = 0;
    long long remainingCopper = 0;
    int startDay = -1;
    int originalTermWeeks = 0;
    int extensionWeeks = 0;
    int dueDay = -1;
    int baseRatePercent = 0;
    int longDurationSurchargePercent = 0;
    int extensionRatePercent = 0;
    int totalCostRatePercent = 0;
    bool automaticRepayment = false;
    bool overdue = false;
};

struct GuildLoanCreateResult
{
    bool success = false;
    GuildLoanState loan;
    std::string failureReason;
};

struct GuildLoanExtensionResult
{
    bool success = false;
    int requestedWeeks = 0;
    int appliedRatePercent = 0;
    long long addedFeeCopper = 0;
    GuildLoanState loan;
    std::string failureReason;
};

struct GuildLoanDayResult
{
    bool hadActiveLoan = false;
    bool automaticPaymentAttempted = false;
    long long automaticPaymentCopper = 0;
    bool becameOverdue = false;
    bool fullyRepaid = false;
    std::vector<std::string> reportLines;
};

struct GuildDebtSituation
{
    bool overdue = false;
    int overdueDays = 0;
    long long totalDebtCopper = 0;
    long long immediatelyAvailableCopper = 0;
    bool likelyFinancialHardship = false;
    bool canCoverNow = false;
    bool newCreditRestricted = false;
    std::string statusLabel;
};

enum class GuildDebtRepaymentPriority
{
    OverdraftFirst,
    LoanFirst,
    Split
};

struct GuildDebtRepaymentResult
{
    bool success = false;
    long long totalPaidCopper = 0;
    long long overdraftPaidCopper = 0;
    long long loanPaidCopper = 0;
    std::string failureReason;
};

class GuildFinanceSystem
{
public:
    static const std::string& membershipId();
    static const std::string& membershipName();
    static int membershipRenewalPriceEconomyUnits();

    static bool isMember(const Player& player);
    static long long depositedBalanceCopper(const Player& player);
    static long long accountCapacityCopper(const Player& player);
    static long long accountRemainingCapacityCopper(const Player& player);
    static bool depositCopper(Player& player, long long amount, std::string* failureReason = nullptr);
    static bool withdrawCopper(Player& player, long long amount, std::string* failureReason = nullptr);

    static int financialTrustScore(const Player& player);
    static std::string financialTrustLabel(int score);
    static long long overdraftUsedCopper(const Player& player);
    static long long overdraftCeilingCopper(const Player& player);
    static long long overdraftRemainingCopper(const Player& player);
    static GuildFinancePaymentResult payAffiliatedService(
        Player& player,
        long long amount,
        const std::string& serviceKey,
        const std::string& serviceLabel,
        bool authorizeOverdraft
    );
    static bool repayOverdraftFromWallet(Player& player, long long amount, std::string* failureReason = nullptr);
    static bool repayOverdraftFromGuildBalance(Player& player, long long amount, std::string* failureReason = nullptr);

    static GuildLoanTerms loanTermsForWeeks(int weeks);
    static long long loanMaximumPrincipalCopper(const Player& player);
    static GuildLoanState activeLoan(const Player& player);
    static GuildLoanCreateResult createLoan(Player& player, long long principalCopper, int weeks, bool automaticRepayment);
    static bool repayLoanFromWallet(Player& player, long long amount, std::string* failureReason = nullptr);
    static bool repayLoanFromGuildBalance(Player& player, long long amount, std::string* failureReason = nullptr);
    static GuildLoanExtensionResult extendActiveLoan(Player& player, int extraWeeks);
    static GuildLoanDayResult processEndOfWorldDayLoan(Player& player);
    static GuildDebtSituation assessDebtSituation(const Player& player);

    static GuildDebtRepaymentResult repayDebtsFromWallet(Player& player, long long amount, GuildDebtRepaymentPriority priority);
    static GuildDebtRepaymentResult repayDebtsFromGuildBalance(Player& player, long long amount, GuildDebtRepaymentPriority priority);
    static GuildDebtRepaymentResult clearAllDebtsByCheat(Player& player);

    static GuildFinanceOverview overview(const Player& player);
};

#endif
