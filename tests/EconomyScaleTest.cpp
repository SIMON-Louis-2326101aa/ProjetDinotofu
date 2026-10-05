#include "economy/Money.hpp"
#include "economy/EconomyBalance.hpp"
#include "item/Inventory.hpp"
#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopPriceRules.hpp"
#include "quest/QuestCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/DifficultyMode.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"
#include "entity/Player.hpp"

#include <cassert>
#include <string>
#include <algorithm>
#include <vector>

int main()
{
    // Historical gameplay price/reward integers are economy units: 1 unit = 1 PF = 10 PC.
    assert(Money::COPPER_PER_ECONOMY_UNIT == 10);
    assert(Money::copperFromEconomyUnits(1) == 10);
    assert(Money::copperFromEconomyUnits(13) == 130);
    assert(Money::economyUnitsFromCopper(130) == 13);
    assert(Money::formatEconomyUnits(13) == "1 électrum, 3 fer");
    assert(Money::formatEconomyUnits(100) == "1 or");
    assert(Money::formatEconomyUnits(1000) == "1 platine");

    // V3.50.31: the starter purse is physical money, not an abstract PF budget.
    // Normal starts modestly with exactly 5 PF + 15 PC = 65 PC. The 15 copper pieces stay physical.
    const CoinBreakdown normalStarter = DifficultyRules::getStarterCoinStacks(DifficultyMode::Normal);
    assert(normalStarter.iron == 5);
    assert(normalStarter.copper == 15);
    assert(DifficultyRules::getStarterCopper(DifficultyMode::Normal) == 65);
    Inventory inventory;
    inventory.setCoinStacks(normalStarter);
    assert(inventory.getTotalCopper() == 65);
    assert(inventory.getEconomyUnits() == 6); // legacy PF view floors the 5 PC remainder.
    const CoinBreakdown starterStacks = inventory.getCoinStacks();
    assert(starterStacks.iron == 5);
    assert(starterStacks.copper == 15);
    assert(starterStacks.electrum == 0);
    assert(inventory.getWalletLine().find("5 fer") != std::string::npos);
    assert(inventory.getWalletLine().find("15 cuivres") != std::string::npos);
    assert(inventory.getWalletTotalLine() == "65 PC");
    assert(Money::socialStandingLabel(starterStacks) == "Monnaie courante");

    CoinBreakdown copperOnly;
    copperOnly.copper = 65;
    assert(Money::socialStandingLabel(copperOnly).find("Petite monnaie") != std::string::npos);
    CoinBreakdown goldCarrier;
    goldCarrier.gold = 1;
    assert(Money::socialStandingLabel(goldCarrier).find("notable") != std::string::npos);
    CoinBreakdown platinumCarrier;
    platinumCarrier.platinum = 1;
    assert(Money::socialStandingLabel(platinumCarrier).find("exceptionnelle") != std::string::npos);

    // Physical stacks are not silently normalized: 51 PO stay 51 PO in one stack.
    Inventory stackedGold;
    stackedGold.earnGold(51);
    assert(stackedGold.getCoinStacks().gold == 51);
    assert(stackedGold.getCoinStacks().platinum == 0);
    assert(stackedGold.getWalletLine().find("51 or") != std::string::npos);
    assert(stackedGold.spendGold(1));
    assert(stackedGold.getCoinStacks().gold == 50);

    // Spending keeps exact copper precision and only makes change when necessary.
    assert(inventory.spendEconomyUnits(3));
    assert(inventory.getTotalCopper() == 35);
    assert(inventory.getCoinStacks().iron == 2);
    assert(inventory.getCoinStacks().copper == 15);
    inventory.earnCopper(7);
    assert(inventory.getTotalCopper() == 42);
    assert(inventory.getCoinStacks().copper == 22);

    // Authored PF rewards are paid in sensible physical denominations without normalizing old stacks.
    Inventory rewardWallet;
    rewardWallet.earnEconomyUnits(13);
    assert(rewardWallet.getTotalCopper() == 130);
    assert(rewardWallet.getCoinStacks().electrum == 1);
    assert(rewardWallet.getCoinStacks().iron == 3);
    rewardWallet.earnEconomyUnits(100);
    assert(rewardWallet.getCoinStacks().gold == 1);
    assert(rewardWallet.getCoinStacks().electrum == 1);
    assert(rewardWallet.getCoinStacks().iron == 3);

    Inventory changeWallet;
    changeWallet.earnGold(1);
    assert(changeWallet.spendEconomyUnits(1));
    assert(changeWallet.getTotalCopper() == 990);
    assert(changeWallet.getCoinStacks().gold == 0);
    assert(changeWallet.getCoinStacks().electrum == 9);
    assert(changeWallet.getCoinStacks().iron == 9);


    // Voluntary guild exchange changes only the physical denomination, never total value.
    Inventory exchangeWallet;
    CoinBreakdown exchangeStacks;
    exchangeStacks.gold = 2;
    exchangeStacks.iron = 25;
    exchangeWallet.setCoinStacks(exchangeStacks);
    const long long exchangeTotal = exchangeWallet.getTotalCopper();
    assert(exchangeWallet.breakCoinsToLower(CoinType::Gold, 1));
    assert(exchangeWallet.getCoinStacks().gold == 1);
    assert(exchangeWallet.getCoinStacks().electrum == 10);
    assert(exchangeWallet.getTotalCopper() == exchangeTotal);
    assert(exchangeWallet.combineCoinsToHigher(CoinType::Iron, 2));
    assert(exchangeWallet.getCoinStacks().iron == 5);
    assert(exchangeWallet.getCoinStacks().electrum == 12);
    assert(exchangeWallet.getTotalCopper() == exchangeTotal);
    assert(!exchangeWallet.breakCoinsToLower(CoinType::Copper, 1));
    assert(!exchangeWallet.combineCoinsToHigher(CoinType::Platinum, 1));
    assert(Money::coinAbbreviation(CoinType::Copper) == "PC");
    assert(Money::coinAbbreviation(CoinType::Platinum) == "PP");
    assert(Money::coinScaleText().find("Pièce de cuivre (PC)") != std::string::npos);

    // V3.50.32: the exchange desk can target any denomination directly. One "lot" is the
    // smallest exact conversion between the selected source and destination denominations.
    Inventory customExchange;
    CoinBreakdown customStacks;
    customStacks.copper = 250;
    customStacks.iron = 25;
    customStacks.gold = 2;
    customExchange.setCoinStacks(customStacks);
    const long long customTotal = customExchange.getTotalCopper();
    assert(customExchange.convertCoinLots(CoinType::Copper, CoinType::Electrum, 2)); // 200 PC -> 2 PE
    assert(customExchange.getCoinStacks().copper == 50);
    assert(customExchange.getCoinStacks().electrum == 2);
    assert(customExchange.convertCoinLots(CoinType::Gold, CoinType::Copper, 1)); // 1 PO -> 1000 PC
    assert(customExchange.getCoinStacks().gold == 1);
    assert(customExchange.getCoinStacks().copper == 1050);
    assert(customExchange.getTotalCopper() == customTotal);
    assert(!customExchange.convertCoinLots(CoinType::Copper, CoinType::Copper, 1));
    assert(!customExchange.convertCoinLots(CoinType::Copper, CoinType::Platinum, 1)); // not enough PC for one PP

    Inventory idealWallet;
    CoinBreakdown awkwardStacks;
    awkwardStacks.copper = 12345;
    idealWallet.setCoinStacks(awkwardStacks);
    assert(idealWallet.compactCoinsToHighest());
    const CoinBreakdown compacted = idealWallet.getCoinStacks();
    assert(compacted.platinum == 1);
    assert(compacted.gold == 2);
    assert(compacted.electrum == 3);
    assert(compacted.iron == 4);
    assert(compacted.copper == 5);
    assert(idealWallet.getTotalCopper() == 12345);
    assert(idealWallet.flattenCoinsToCopper());
    assert(idealWallet.getCoinStacks().copper == 12345);
    assert(idealWallet.getCoinStacks().iron == 0);
    assert(idealWallet.getCoinStacks().platinum == 0);

    // Exact authored rewards preserve the declared physical pieces instead of compacting them.
    Inventory exactRewardWallet;
    CoinBreakdown exactReward;
    exactReward.copper = 17;
    exactReward.iron = 2;
    exactReward.gold = 3;
    exactRewardWallet.earnCoinStacks(exactReward);
    assert(exactRewardWallet.getCoinStacks().copper == 17);
    assert(exactRewardWallet.getCoinStacks().iron == 2);
    assert(exactRewardWallet.getCoinStacks().gold == 3);
    assert(exactRewardWallet.getCoinStacks().electrum == 0);
    assert(Money::coinStacksValueInCopper(exactReward) == 3037);

    Player platinumMilestonePlayer;
    CoinBreakdown platinumBuildWallet;
    platinumBuildWallet.copper = Money::COPPER_PER_PLATINUM;
    platinumMilestonePlayer.getInventory().setCoinStacks(platinumBuildWallet);
    assert(platinumMilestonePlayer.getInventory().convertCoinLots(CoinType::Copper, CoinType::Platinum, 1));
    platinumMilestonePlayer.refreshCurrencyTitles();
    assert(platinumMilestonePlayer.hasTitle("Premier éclat de platine"));

    // Economy API semantics are explicit before the V3.50.30 checkpoint audit:
    // vault purchase/upgrade values are PF economy units, while travel/inn/logistics values below are exact PC.
    assert(EconomyBalance::cityVaultPurchaseCost("valebrume") == 350);
    assert(Money::copperFromEconomyUnits(EconomyBalance::cityVaultPurchaseCost("valebrume")) == 3500);
    assert(EconomyBalance::innCommonBedCostCopper("valebrume", 1) == 100);
    assert(EconomyBalance::innSafeRoomCostCopper("valebrume", 1) == 192);
    assert(EconomyBalance::innWarmMealCostCopper("valebrume", 1) == 50);
    assert(EconomyBalance::estimatedTravelCopperCost(20) == 100);
    assert(EconomyBalance::cityChangeTaxCopper("valebrume", 20) > 0);
    assert(EconomyBalance::cityVaultMaterialTransferCostCopper("valebrume", "port_lanterne", 20, 5) > 0);
    assert(EconomyBalance::routeRewardBudgetCopperForDistance(20, 2) > 0);
    // A beginner with 65 PC can still buy a warm meal, but an inn bed already requires
    // earning money: the opening purse is deliberately modest rather than a comfort buffer.
    const long long normalStarterCopper = DifficultyRules::getStarterCopper(DifficultyMode::Normal);
    assert(EconomyBalance::innWarmMealCostCopper("valebrume", 1) < normalStarterCopper);
    assert(EconomyBalance::innCommonBedCostCopper("valebrume", 1) > normalStarterCopper);
    assert(EconomyBalance::innSafeRoomCostCopper("valebrume", 1) > normalStarterCopper);
    assert(Money::copperFromEconomyUnits(EconomyBalance::cityVaultPurchaseCost("valebrume")) > normalStarterCopper);

    // Merchant quote styles can be exact or rounded to a clean denomination. The charged amount
    // must match the displayed quote instead of hiding an arbitrary conversion.
    assert(!ShopPriceRules::usesRoundedBuyQuotes(ShopType::Consumable));
    assert(ShopPriceRules::usesRoundedBuyQuotes(ShopType::Weapon));
    assert(ShopPriceRules::roundBuyQuote(ShopType::Consumable, 85) == 85);
    assert(ShopPriceRules::roundBuyQuote(ShopType::Weapon, 85) == 90);
    assert(ShopPriceRules::roundBuyQuote(ShopType::Weapon, 165) == 170);
    assert(ShopPriceRules::roundBuyQuote(ShopType::Weapon, 205) == 200);

    // Audit every preview-shop listing: prices stay positive and resale cannot create free arbitrage.
    int maxShopPrice = 0;
    bool sawTrainingBow = false;
    bool sawRustySword = false;
    bool sawWornLeatherArmor = false;
    bool sawMinorHealingPotion = false;
    bool sawSurvivalRation = false;
    const std::vector<std::string> races = {"", "Humain", "Démon", "Kitsune"};
    const std::vector<std::string> classes = {"", "Forgeron", "Alchimiste", "Artificier"};
    for (const ShopInventory& shop : ShopCatalog::createAllPreviewShops())
    {
        for (const ShopItem& item : shop.getItems())
        {
            assert(item.getBuyPrice() > 0);
            assert(item.getSellPrice() >= 0);
            assert(item.getSellPrice() <= item.getBuyPrice());
            maxShopPrice = std::max(maxShopPrice, item.getBuyPrice());

            if (item.getId() == "training_bow")
            {
                sawTrainingBow = true;
                assert(item.getBuyPrice() <= 100);
            }
            else if (item.getId() == "rusty_sword")
            {
                sawRustySword = true;
                assert(item.getBuyPrice() <= 80);
            }
            else if (item.getId() == "worn_leather_armor")
            {
                sawWornLeatherArmor = true;
                assert(item.getBuyPrice() <= 100);
            }
            else if (item.getId() == "minor_healing_potion")
            {
                sawMinorHealingPotion = true;
                assert(item.getBuyPrice() <= 20);
            }
            else if (item.getId() == "survival_ration")
            {
                sawSurvivalRation = true;
                assert(item.getBuyPrice() <= 20);
            }

            for (const std::string& race : races)
            {
                for (const std::string& className : classes)
                {
                    const int buy = ShopPriceRules::applyBuyModifier(item.getBuyPrice(), race, className);
                    const int quotedBuy = ShopPriceRules::roundBuyQuote(shop.getType(), buy);
                    const int sell = ShopPriceRules::applySellModifier(item.getSellPrice(), race, className);
                    assert(buy > 0);
                    assert(quotedBuy > 0);
                    assert(sell >= 0);
                    if (sell > 0) assert(sell < quotedBuy);
                }
            }
        }
    }
    assert(maxShopPrice > 0 && maxShopPrice <= 2000);
    assert(sawTrainingBow);
    assert(sawRustySword);
    assert(sawWornLeatherArmor);
    assert(sawMinorHealingPotion);
    assert(sawSurvivalRation);

    // Beginner contracts must stay pocket-money scale: no early rat hunt or local errand should
    // suddenly pay literal PO/PP amounts. Values here are authored PF economy units.
    for (int sample = 0; sample < 64; ++sample)
    {
        for (const Quest& quest : QuestCatalog::createGuildBoard(1))
        {
            if (quest.rank.find("F") != std::string::npos)
            {
                assert(quest.rewardGold <= 20);
                if (quest.objectiveType == "service") assert(quest.rewardGold <= 5);
            }
        }

        for (const Quest& quest : QuestCatalog::createGuildBoard(2))
        {
            if (quest.rank.find("E") != std::string::npos) assert(quest.rewardGold <= 30);
        }
    }

    // Ordinary guild-board rewards stay on the same authored PF scale as shops at low/high levels.
    for (int level : {1, 5, 10, 25, 50})
    {
        for (int sample = 0; sample < 12; ++sample)
        {
            const std::vector<Quest> quests = QuestCatalog::createGuildBoard(level);
            assert(!quests.empty());
            for (const Quest& quest : quests)
            {
                assert(quest.rewardGold >= 0);
                assert(quest.rewardGold <= maxShopPrice * 2);
            }
        }
    }

    // V3.50.30 checkpoint: protect real purchasing power, not only isolated constants.
    // The broad bands intentionally leave room for future content while catching x10/x100 unit mistakes.
    struct PurchasingPowerCheckpoint
    {
        int level;
        int minAverageQuestReward;
        int maxAverageQuestReward;
        int maxSingleQuestReward;
    };

    const std::vector<PurchasingPowerCheckpoint> purchasingPowerCheckpoints = {
        {1, 3, 13, 25},
        {5, 8, 28, 55},
        {10, 12, 45, 80},
        {25, 25, 85, 150},
        {50, 45, 145, 260}
    };

    int previousAverageReward = -1;
    int previousAffordableListings = -1;
    int level10RepresentativeBudget = 0;
    int level50RepresentativeBudget = 0;

    for (const PurchasingPowerCheckpoint& checkpoint : purchasingPowerCheckpoints)
    {
        long long rewardTotal = 0;
        int rewardCount = 0;
        int largestReward = 0;

        for (int sample = 0; sample < 64; ++sample)
        {
            const std::vector<Quest> quests = QuestCatalog::createGuildBoard(checkpoint.level);
            assert(!quests.empty());
            for (const Quest& quest : quests)
            {
                assert(quest.rewardGold >= 0);
                rewardTotal += quest.rewardGold;
                ++rewardCount;
                largestReward = std::max(largestReward, quest.rewardGold);
            }
        }

        assert(rewardCount > 0);
        const int averageReward = static_cast<int>(rewardTotal / rewardCount);
        assert(averageReward >= checkpoint.minAverageQuestReward);
        assert(averageReward <= checkpoint.maxAverageQuestReward);
        assert(largestReward <= checkpoint.maxSingleQuestReward);
        assert(averageReward > previousAverageReward);

        // Representative purchasing power after four ordinary contracts, in exact copper.
        const long long representativeBudgetCopper = DifficultyRules::getStarterCopper(DifficultyMode::Normal)
            + Money::copperFromEconomyUnits(averageReward * 4LL);
        int affordableListings = 0;
        int totalListings = 0;
        for (const ShopInventory& shop : ShopCatalog::createAllPreviewShops())
        {
            for (const ShopItem& item : shop.getItems())
            {
                ++totalListings;
                if (Money::copperFromEconomyUnits(item.getBuyPrice()) <= representativeBudgetCopper)
                {
                    ++affordableListings;
                }
            }
        }

        assert(totalListings > 0);
        assert(affordableListings > 0);
        assert(affordableListings >= previousAffordableListings);
        assert(affordableListings < totalListings); // high-end goods must remain goals, even at level 50.

        if (checkpoint.level == 10) level10RepresentativeBudget = static_cast<int>(representativeBudgetCopper);
        if (checkpoint.level == 50) level50RepresentativeBudget = static_cast<int>(representativeBudgetCopper);
        previousAverageReward = averageReward;
        previousAffordableListings = affordableListings;
    }

    // The municipal vault stays a medium-term target: not a trivial early purchase, but reachable later.
    const int vaultCost = EconomyBalance::cityVaultPurchaseCost("valebrume");
    const long long vaultCostCopper = Money::copperFromEconomyUnits(vaultCost);
    assert(vaultCostCopper > level10RepresentativeBudget);
    assert(vaultCostCopper <= level50RepresentativeBudget);

    // Combat income follows the same scale: a normal humanoid is useful pocket money,
    // while a benchmark boss is meaningful without instantly buying the whole economy.
    Monster level1Humanoid("Auditeur novice", "humanoïde", Race::Humain, 1, 80, 4, 8, 12, 0, 0);
    Monster level50Humanoid("Auditeur vétéran", "humanoïde", Race::Humain, 50, 900, 30, 55, 80, 0, 0);
    const CombatReward level1CombatReward = CombatRewardSystem::calculateMonsterReward(level1Humanoid);
    const CombatReward level50CombatReward = CombatRewardSystem::calculateMonsterReward(level50Humanoid);
    assert(level1CombatReward.getEconomyUnits() > 0 && level1CombatReward.getEconomyUnits() <= 5);
    assert(level50CombatReward.getEconomyUnits() > level1CombatReward.getEconomyUnits());
    assert(level50CombatReward.getEconomyUnits() < vaultCost);

    Boss benchmarkBoss(5, "Boss étalon", "boss d'audit", 1200, 35, 65, 95, 1, 1, 3, 4);
    const CombatReward bossReward = CombatRewardSystem::calculateBossReward(benchmarkBoss, DifficultyMode::Normal, 0, 6);
    assert(bossReward.getEconomyUnits() > level50CombatReward.getEconomyUnits());
    assert(bossReward.getEconomyUnits() < vaultCost);

    return 0;
}
