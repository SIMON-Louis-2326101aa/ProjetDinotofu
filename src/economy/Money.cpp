// EN: Money.cpp formats Dinotofu monetary values.
// FR: Money.cpp formate les valeurs monétaires de Dinotofu.

#include "economy/Money.hpp"

#include <algorithm>
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
    return "Pièce de cuivre (PC) -> Pièce de fer (PF) -> Pièce d'électrum (PE) -> Pièce d'or (PO) -> Pièce de platine (PP) | 10 pièces d'un rang = 1 pièce du rang supérieur";
}

std::string Money::coinName(CoinType type)
{
    switch (type)
    {
        case CoinType::Copper: return "pièce de cuivre";
        case CoinType::Iron: return "pièce de fer";
        case CoinType::Electrum: return "pièce d'électrum";
        case CoinType::Gold: return "pièce d'or";
        case CoinType::Platinum: return "pièce de platine";
    }
    return "pièce";
}

std::string Money::coinAbbreviation(CoinType type)
{
    switch (type)
    {
        case CoinType::Copper: return "PC";
        case CoinType::Iron: return "PF";
        case CoinType::Electrum: return "PE";
        case CoinType::Gold: return "PO";
        case CoinType::Platinum: return "PP";
    }
    return "?";
}

long long Money::coinValueInCopper(CoinType type)
{
    switch (type)
    {
        case CoinType::Copper: return 1;
        case CoinType::Iron: return COPPER_PER_IRON;
        case CoinType::Electrum: return static_cast<long long>(COPPER_PER_IRON) * IRON_PER_ELECTRUM;
        case CoinType::Gold: return COPPER_PER_GOLD;
        case CoinType::Platinum: return COPPER_PER_PLATINUM;
    }
    return 1;
}

long long Money::coinStacksValueInCopper(const CoinBreakdown& stacks)
{
    return std::max(0LL, stacks.copper)
        + std::max(0LL, stacks.iron) * coinValueInCopper(CoinType::Iron)
        + std::max(0LL, stacks.electrum) * coinValueInCopper(CoinType::Electrum)
        + std::max(0LL, stacks.gold) * coinValueInCopper(CoinType::Gold)
        + std::max(0LL, stacks.platinum) * coinValueInCopper(CoinType::Platinum);
}

bool Money::hasLowerCoin(CoinType type)
{
    return type != CoinType::Copper;
}

bool Money::hasHigherCoin(CoinType type)
{
    return type != CoinType::Platinum;
}

CoinType Money::nextLowerCoin(CoinType type)
{
    switch (type)
    {
        case CoinType::Platinum: return CoinType::Gold;
        case CoinType::Gold: return CoinType::Electrum;
        case CoinType::Electrum: return CoinType::Iron;
        case CoinType::Iron: return CoinType::Copper;
        case CoinType::Copper: return CoinType::Copper;
    }
    return CoinType::Copper;
}

CoinType Money::nextHigherCoin(CoinType type)
{
    switch (type)
    {
        case CoinType::Copper: return CoinType::Iron;
        case CoinType::Iron: return CoinType::Electrum;
        case CoinType::Electrum: return CoinType::Gold;
        case CoinType::Gold: return CoinType::Platinum;
        case CoinType::Platinum: return CoinType::Platinum;
    }
    return CoinType::Platinum;
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

long long Money::copperFromEconomyUnits(long long economyUnits)
{
    if (economyUnits <= 0)
    {
        return 0;
    }
    return economyUnits * COPPER_PER_ECONOMY_UNIT;
}

long long Money::economyUnitsFromCopper(long long copperAmount)
{
    if (copperAmount <= 0)
    {
        return 0;
    }
    return copperAmount / COPPER_PER_ECONOMY_UNIT;
}

std::string Money::formatEconomyUnits(long long economyUnits)
{
    return formatCopper(copperFromEconomyUnits(economyUnits));
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

std::string Money::formatCoinStacks(const CoinBreakdown& stacks, bool includeZeroCoins)
{
    return formatBreakdown(stacks, includeZeroCoins);
}

std::string Money::formatWalletTotalFromCopper(long long copperAmount)
{
    if (copperAmount < 0)
    {
        copperAmount = 0;
    }

    return formatSeparatedNumber(copperAmount) + " PC";
}

std::string Money::formatCurrencyOverviewFromCopper(long long copperAmount)
{
    return formatWalletFromCopper(copperAmount);
}

std::string Money::socialStandingLabel(const CoinBreakdown& stacks)
{
    if (stacks.platinum > 0) return "Fortune exceptionnelle";
    if (stacks.gold > 0) return "Très aisé / allure de notable";
    if (stacks.electrum > 0) return "Aisé / respectable";
    if (stacks.iron > 0) return "Monnaie courante";
    if (stacks.copper > 0) return "Petite monnaie / moyens modestes";
    return "Sans monnaie visible";
}

std::string Money::socialStandingReaction(const CoinBreakdown& stacks)
{
    if (stacks.platinum > 0)
    {
        return "Une pièce de platine suffit à attirer les regards : on te suppose immensément riche, puissant ou lié aux hautes sphères.";
    }
    if (stacks.gold > 0)
    {
        return "Voir de l'or dans ta bourse te fait passer pour quelqu'un de très aisé ; certains peuvent te croire noble, marchand majeur ou aventurier important.";
    }
    if (stacks.electrum > 0)
    {
        return "L'électrum donne l'image de quelqu'un qui vit correctement et manipule déjà des sommes respectables.";
    }
    if (stacks.iron > 0)
    {
        return "Le fer est la monnaie quotidienne : rien de honteux, rien qui impressionne vraiment.";
    }
    if (stacks.copper > 0)
    {
        return "Une bourse composée uniquement de cuivre fait très modeste ; dans certains milieux, on peut vite te prendre pour un plouc ou quelqu'un de fauché.";
    }
    return "Une bourse vide parle d'elle-même : personne ne te prend pour un client fortuné.";
}
