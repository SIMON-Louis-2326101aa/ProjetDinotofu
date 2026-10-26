#include "cheat/CheatManager.hpp"
#include "entity/Player.hpp"
#include "progression/TitleCatalog.hpp"
#include "save/SaveManager.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    bool contains(const std::vector<std::string>& values, const std::string& expected)
    {
        return std::find(values.begin(), values.end(), expected) != values.end();
    }

    const AccountUnlockedTitleInfo* findTitleDetail(const std::vector<AccountUnlockedTitleInfo>& values, const std::string& expected)
    {
        const auto it = std::find_if(values.begin(), values.end(), [&](const AccountUnlockedTitleInfo& value) {
            return value.name == expected;
        });
        return it == values.end() ? nullptr : &(*it);
    }
}

int main()
{
    namespace fs = std::filesystem;
    fs::remove_all("assets/saves");

    const std::string account = "account_title_progress_test";
    assert(SaveManager::saveAccountSnapshot(account));

    Player legitimate;
    legitimate.setOwnershipMetadata(account, account);
    assert(legitimate.grantTitle("Aventurier"));
    assert(legitimate.hasTitle("Aventurier"));
    assert(legitimate.hasTitle("Une sorte de titre ?"));
    assert(SaveManager::savePlayerSnapshot(legitimate, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));

    std::vector<std::string> accountTitles = SaveManager::getAccountUnlockedTitles(account);
    assert(contains(accountTitles, "Aventurier"));
    assert(contains(accountTitles, "Une sorte de titre ?"));
    std::vector<AccountUnlockedTitleInfo> titleDetails = SaveManager::getAccountUnlockedTitleDetails(account);
    const AccountUnlockedTitleInfo* adventurerDetail = findTitleDetail(titleDetails, "Aventurier");
    assert(adventurerDetail != nullptr);
    assert(!adventurerDetail->unlockedAt.empty());
    assert(adventurerDetail->characterName == legitimate.getName());

    // First alteration grants the dedicated local marker and freezes normal title progression.
    legitimate.markAsAlteredByCheats();
    assert(legitimate.isAlteredByCheats());
    assert(legitimate.hasTitle("On ne dira rien..."));
    const TitleCatalog::TitleDefinition* alteredMarker = TitleCatalog::findTitleDefinition("On ne dira rien...");
    assert(alteredMarker != nullptr);
    assert(!alteredMarker->requiredForUltimate);
    assert(!legitimate.grantTitle("Pas une égratignure"));

    // A future cheat that explicitly grants base titles may still grant them to this character only.
    assert(legitimate.grantCharacterOnlyTitleFromCheat("Pas une égratignure"));
    assert(legitimate.hasTitle("Pas une égratignure"));
    assert(SaveManager::savePlayerSnapshot(legitimate, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));

    accountTitles = SaveManager::getAccountUnlockedTitles(account);
    assert(contains(accountTitles, "Aventurier"));
    assert(!contains(accountTitles, "Pas une égratignure"));
    assert(contains(accountTitles, "On ne dira rien..."));

    // Account trophies survive deletion of the character that originally unlocked them.
    const auto summaries = SaveManager::listPlayableCharacters(account);
    assert(!summaries.empty());
    assert(SaveManager::deletePlayableCharacter(summaries.front()));
    accountTitles = SaveManager::getAccountUnlockedTitles(account);
    assert(contains(accountTitles, "Aventurier"));
    titleDetails = SaveManager::getAccountUnlockedTitleDetails(account);
    adventurerDetail = findTitleDetail(titleDetails, "Aventurier");
    assert(adventurerDetail != nullptr);
    assert(!adventurerDetail->unlockedAt.empty());
    assert(adventurerDetail->characterName == legitimate.getName());

    // A new legitimate character can continue the same account collection.
    Player second;
    second.setOwnershipMetadata(account, account);
    assert(second.grantTitle("Un bon entraînement"));
    assert(SaveManager::savePlayerSnapshot(second, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));
    accountTitles = SaveManager::getAccountUnlockedTitles(account);
    assert(contains(accountTitles, "Aventurier"));
    assert(contains(accountTitles, "Un bon entraînement"));

    // Protocol 13 stays diegetic only if the character already has a credible Reaper trace.
    Player protocolUser;
    protocolUser.setOwnershipMetadata(account, account);
    protocolUser.recordCanonicalEvent("reapers13_knowledge", "npc_rumor", "Un PNJ fiable a parlé des Reapers", 1);
    const bool protocolActivated = CheatManager::tryActivateHiddenCode(
        protocolUser,
        DifficultyMode::Normal,
        DeathRuleMode::NonDefinitive,
        "DINOTOFU:13-02313"
    );
    assert(protocolActivated);
    assert(!protocolUser.isAlteredByCheats());
    assert(protocolUser.getCanonicalJournalCategoryTotal("protocoles_secrets") == 1);
    assert(!protocolUser.hasTitle("On ne dira rien..."));

    // The same character cannot transmit Protocol 13 twice.
    assert(CheatManager::tryActivateHiddenCode(protocolUser, DifficultyMode::Normal, DeathRuleMode::NonDefinitive, "dinotofu:13-02313"));
    assert(protocolUser.getCanonicalJournalCategoryTotal("protocoles_secrets") == 1);

    // Metagaming the forbidden channel without any in-world Reaper knowledge alters the character.
    Player metagameProtocolUser;
    metagameProtocolUser.setOwnershipMetadata(account, account);
    std::istringstream protocolInput("1\n");
    std::streambuf* previousInput = std::cin.rdbuf(protocolInput.rdbuf());
    const bool metagameProtocolActivated = CheatManager::tryActivateHiddenCode(
        metagameProtocolUser,
        DifficultyMode::Normal,
        DeathRuleMode::NonDefinitive,
        "DINOTOFU:13-02313"
    );
    std::cin.rdbuf(previousInput);
    std::cin.clear();
    assert(metagameProtocolActivated);
    assert(metagameProtocolUser.isAlteredByCheats());
    assert(metagameProtocolUser.hasTitle("On ne dira rien..."));
    assert(metagameProtocolUser.getCanonicalJournalCategoryTotal("protocoles_secrets") == 1);

    fs::remove_all("assets/saves");
    return 0;
}
