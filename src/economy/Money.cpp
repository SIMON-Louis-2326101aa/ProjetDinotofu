// EN: Money.cpp formats Dinotofu monetary values.
// FR: Money.cpp formate les valeurs monétaires de Dinotofu.

#include "economy/Money.hpp"

#include <cstdlib>
#include <sstream>
#include <string>

namespace
{
    void appendCoin(std::vector<std::string>& parts, long long amount, const std::string& singular, const std::string& plural, bool includeZero = false)
    {
        if (amount < 0)
        {
            amount = 0;
        }
        if (amount == 0 && !includeZero)
        {
            return;
        }

        parts.push_back(Money::formatSeparatedNumber(amount) + " " + (amount == 1 ? singular : plural));
    }
}

std::string Money::coinScaleText()
{
    return "1 platine = 10 or | 1 or = 10 électrum | 1 électrum = 10 fer | 1 fer = 10 cuivre";
}

CoinBreakdown Money::breakdownFromGold(int goldAmount)
{
    CoinBreakdown breakdown;
    if (goldAmount < 0)
    {
        goldAmount = 0;
    }

    breakdown.platinum = goldAmount / GOLD_PER_PLATINUM;
    breakdown.gold = goldAmount % GOLD_PER_PLATINUM;
    return breakdown;
}

CoinBreakdown Money::breakdownFromCopper(int copperAmount)
{
    return breakdownFromCopper(static_cast<long long>(copperAmount));
}

CoinBreakdown Money::breakdownFromCopper(long long copperAmount)
{
    CoinBreakdown breakdown;
    if (copperAmount < 0)
    {
        copperAmount = 0;
    }

    breakdown.platinum = copperAmount / COPPER_PER_PLATINUM;
    copperAmount %= COPPER_PER_PLATINUM;
    breakdown.gold = copperAmount / COPPER_PER_GOLD;
    copperAmount %= COPPER_PER_GOLD;
    const long long copperPerElectrum = static_cast<long long>(COPPER_PER_IRON) * IRON_PER_ELECTRUM;
    breakdown.electrum = copperAmount / copperPerElectrum;
    copperAmount %= copperPerElectrum;
    breakdown.iron = copperAmount / COPPER_PER_IRON;
    breakdown.copper = copperAmount % COPPER_PER_IRON;
    return breakdown;
}

std::string Money::formatSeparatedNumber(long long value)
{
    const bool negative = value < 0;
    if (negative)
    {
        value = -value;
    }

    std::string raw = std::to_string(value);
    std::string out;
    int group = 0;
    for (auto it = raw.rbegin(); it != raw.rend(); ++it)
    {
        if (group == 3)
        {
            out.insert(out.begin(), ',');
            group = 0;
        }
        out.insert(out.begin(), *it);
        ++group;
    }

    if (negative)
    {
        out.insert(out.begin(), '-');
    }
    return out;
}

std::string Money::formatBreakdown(const CoinBreakdown& breakdown, bool includeZeroCoins)
{
    std::vector<std::string> parts;
    appendCoin(parts, breakdown.platinum, "platine", "platines", includeZeroCoins);
    appendCoin(parts, breakdown.gold, "or", "or", includeZeroCoins);
    appendCoin(parts, breakdown.electrum, "électrum", "électrum", includeZeroCoins);
    appendCoin(parts, breakdown.iron, "fer", "fer", includeZeroCoins);
    appendCoin(parts, breakdown.copper, "cuivre", "cuivres", includeZeroCoins);

    if (parts.empty())
    {
        return includeZeroCoins ? "0 platine | 0 or | 0 électrum | 0 fer | 0 cuivre" : "0 cuivre";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size(); ++i)
    {
        if (i > 0)
        {
            out << (includeZeroCoins ? " | " : ", ");
        }
        out << parts[i];
    }
    return out.str();
}

long long Money::copperFromGold(int goldAmount)
{
    if (goldAmount <= 0)
    {
        return 0;
    }
    return static_cast<long long>(goldAmount) * COPPER_PER_GOLD;
}

std::string Money::formatGold(int goldAmount)
{
    return formatBreakdown(breakdownFromGold(goldAmount));
}

std::string Money::formatGoldWithRaw(int goldAmount)
{
    if (goldAmount < 0)
    {
        goldAmount = 0;
    }
    return formatGold(goldAmount) + " (total : " + formatSeparatedNumber(goldAmount) + " po)";
}

std::string Money::formatCopper(int copperAmount)
{
    return formatCopper(static_cast<long long>(copperAmount));
}

std::string Money::formatCopper(long long copperAmount)
{
    return formatBreakdown(breakdownFromCopper(copperAmount));
}

std::string Money::formatWalletFromCopper(long long copperAmount)
{
    return formatBreakdown(breakdownFromCopper(copperAmount), true);
}

std::string Money::formatWalletTotalFromCopper(long long copperAmount)
{
    if (copperAmount < 0)
    {
        copperAmount = 0;
    }

    const long long wholeGold = copperAmount / COPPER_PER_GOLD;
    const long long remainder = copperAmount % COPPER_PER_GOLD;
    std::string goldText = formatSeparatedNumber(wholeGold);
    if (remainder > 0)
    {
        std::string decimals = std::to_string(remainder);
        while (decimals.size() < 3)
        {
            decimals.insert(decimals.begin(), '0');
        }
        while (!decimals.empty() && decimals.back() == '0')
        {
            decimals.pop_back();
        }
        goldText += "." + decimals;
    }

    return goldText + " po équivalentes (= " + formatSeparatedNumber(copperAmount) + " cuivre)";
}

std::string Money::formatCurrencyOverviewFromCopper(long long copperAmount)
{
    return formatWalletFromCopper(copperAmount) + " | Total converti : " + formatWalletTotalFromCopper(copperAmount);
}
