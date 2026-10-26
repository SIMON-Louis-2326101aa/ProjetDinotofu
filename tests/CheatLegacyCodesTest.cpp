#include "cheat/CheatManager.hpp"
#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "combat/CombatActions.hpp"
#include "core/Random.hpp"
#include "economy/GuildFinanceSystem.hpp"
#include "quest/Quest.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/DeathRuleMode.hpp"

#include <cassert>
#include <string>

int main()
{
    Player player;
    player.markAsAlteredByCheats();

    // Codes historiques reconnectés : ils doivent désormais produire un état moteur vérifiable.
    assert(CheatManager::tryActivateHiddenCode(player, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "iamnotgoblinfood"));
    assert(player.getCanonicalJournalCategoryTotal("cheat_effects") >= 1);

    const int effectsBeforeCook = player.getCanonicalJournalCategoryTotal("cheat_effects");
    assert(CheatManager::tryActivateHiddenCode(player, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "letmecook"));
    assert(player.getCanonicalJournalCategoryTotal("cheat_effects") > effectsBeforeCook);

    assert(CheatManager::tryActivateHiddenCode(player, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "whereismyplotarmor"));
    assert(player.hasBlessing("cheat_plot_armor"));

    assert(CheatManager::tryActivateHiddenCode(player, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "oopsallbosses"));
    assert(player.isBossUnlocked(27));

    player.takeDamage(7);
    const int damagedHp = player.getHp();
    assert(damagedHp < player.getMaxHp());
    assert(CheatManager::tryActivateHiddenCode(player, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "13023131302313"));
    assert(player.getHp() == player.getMaxHp());
    Monster target("Cible de test", "Brigand", Race::Humain, 3, 60, 2, 4, 6, 0, 0);
    Random random;
    CombatActions::executeAttack(player, target, random);
    assert(target.isDead());

    // talktothedev reste un secret non altérant sur un personnage propre.
    Player cleanSecretUser;
    assert(CheatManager::tryActivateHiddenCode(cleanSecretUser, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "talktothedev"));
    assert(!cleanSecretUser.isAlteredByCheats());
    assert(cleanSecretUser.getCanonicalJournalCategoryTotal("secret_code_usage") == 1);

    // randomizemyfate doit rester une vraie altération et conserver le niveau courant.
    Player fateUser;
    fateUser.setLoadedProgress(9, 0, fateUser.getMaxHp());
    fateUser.markAsAlteredByCheats();
    const int levelBefore = fateUser.getLevel();
    assert(CheatManager::tryActivateHiddenCode(fateUser, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "randomizemyfate"));
    assert(fateUser.isAlteredByCheats());
    assert(fateUser.getLevel() == levelBefore);
    assert(fateUser.getCanonicalJournalCategoryTotal("cheat_effects") >= 1);

    // Codes V3.51.40 validés : ils sont tous réellement branchés et restent des altérations.
    Player modernCheats;
    modernCheats.markAsAlteredByCheats();
    const int hpBeforePower = modernCheats.getMaxHp();
    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "ThatsALotOfPower"));
    assert(modernCheats.getMaxHp() > hpBeforePower);

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "icanfeelit"));
    assert(modernCheats.getCanonicalJournalCategoryTotal("mana_awakening") >= 1);

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "bornforit"));
    assert(modernCheats.isPassiveSkillUnlocked("inner_mantra_mastery"));
    assert(modernCheats.isPassiveSkillEnabled("inner_mantra_mastery"));

    const int materialTypesBefore = modernCheats.getInventory().getMaterialCount();
    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "fillmypocket"));
    assert(modernCheats.getInventory().getMaterialCount() >= materialTypesBefore + 8);

    const int dayBefore = modernCheats.getWorldDaysElapsed();
    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "justanotherday"));
    assert(modernCheats.getWorldDaysElapsed() == dayBefore + 1);

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "luckytofu"));
    assert(modernCheats.isPassiveSkillUnlocked("halfling_lucky_step"));
    assert(modernCheats.isPassiveSkillEnabled("halfling_lucky_step"));
    assert(!modernCheats.hasTitle("Luck is a skill"));

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "onemorechance"));
    assert(modernCheats.getCanonicalJournalCategoryTotal("cheat_spirit_rearm") >= 1);

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "trustmebro"));
    assert(modernCheats.getCanonicalJournalCategoryTotal("cheat_trustmebro") >= 1);

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "fearmeinstead"));
    assert(modernCheats.getCanonicalJournalCategoryTotal("cheat_fear_resistance") >= 1);

    assert(CheatManager::tryActivateHiddenCode(modernCheats, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "everybodylovesme"));
    assert(modernCheats.getCanonicalJournalCategoryTotal("reputation_locale_positive") >= 100);

    // whatdebt only makes sense now that real guild debts exist: it clears them without paying physical coins.
    Player debtUser;
    assert(debtUser.grantTitle("Aventurier"));
    debtUser.getInventory().earnCopper(1000);
    debtUser.activateLocalSubscription(
        GuildFinanceSystem::membershipId(),
        GuildFinanceSystem::membershipName(),
        7,
        GuildFinanceSystem::membershipRenewalPriceEconomyUnits()
    );
    std::string financeFailure;
    assert(GuildFinanceSystem::depositCopper(debtUser, 100, &financeFailure));
    for (int i = 0; i < 5; ++i)
    {
        Quest q;
        q.id = "cheat_debt_completed_" + std::to_string(i);
        q.guildQuest = true;
        q.turnedIn = true;
        debtUser.getQuestLog().getQuests().push_back(q);
    }
    assert(GuildFinanceSystem::createLoan(debtUser, 100, 1, false).success);
    const long long walletBeforeCredit = debtUser.getInventory().getTotalCopper();
    const GuildFinancePaymentResult debtCredit = GuildFinanceSystem::payAffiliatedService(
        debtUser,
        walletBeforeCredit + 25,
        "whatdebt_test_partner",
        "Service test avant whatdebt",
        true
    );
    assert(debtCredit.success);
    assert(GuildFinanceSystem::overdraftUsedCopper(debtUser) == 25);
    assert(GuildFinanceSystem::activeLoan(debtUser).active);
    const long long walletBeforeWhatDebt = debtUser.getInventory().getTotalCopper();
    debtUser.markAsAlteredByCheats();
    assert(CheatManager::tryActivateHiddenCode(debtUser, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "WhatDebt"));
    assert(GuildFinanceSystem::overdraftUsedCopper(debtUser) == 0);
    assert(!GuildFinanceSystem::activeLoan(debtUser).active);
    assert(debtUser.getInventory().getTotalCopper() == walletBeforeWhatDebt);

    return 0;
}
