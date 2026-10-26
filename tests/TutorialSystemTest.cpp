#include "tutorial/TutorialSystem.hpp"

#include "progression/TitleCatalog.hpp"
#include "save/SaveManager.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    bool contains(const std::vector<std::string>& values, const std::string& value)
    {
        return std::find(values.begin(), values.end(), value) != values.end();
    }
}

int main()
{
    namespace fs = std::filesystem;
    fs::remove_all("assets/saves");

    assert(TutorialSystem::dummyName() == "Jean-Michel Test");
    assert(TutorialSystem::startedTitle() == "Rencontre avec notre cobaye");
    assert(TutorialSystem::completedTitle() == "J'ai lu le manuel");

    const auto* started = TitleCatalog::findTitleDefinition(TutorialSystem::startedTitle());
    const auto* completed = TitleCatalog::findTitleDefinition(TutorialSystem::completedTitle());
    assert(started != nullptr);
    assert(completed != nullptr);
    assert(started->visibleBeforeUnlock);
    assert(completed->visibleBeforeUnlock);
    assert(started->requiredForUltimate);
    assert(completed->requiredForUltimate);

    const std::string account = "tutorial_system_test";
    assert(SaveManager::saveAccountSnapshot(account));

    assert(SaveManager::mergeAccountUnlockedTitles(account, {TutorialSystem::startedTitle()}, TutorialSystem::dummyName()));
    auto titles = SaveManager::getAccountUnlockedTitles(account);
    assert(contains(titles, TutorialSystem::startedTitle()));
    assert(!contains(titles, TutorialSystem::completedTitle()));

    assert(SaveManager::mergeAccountUnlockedTitles(account, {TutorialSystem::completedTitle()}, TutorialSystem::dummyName()));
    titles = SaveManager::getAccountUnlockedTitles(account);
    assert(contains(titles, TutorialSystem::startedTitle()));
    assert(contains(titles, TutorialSystem::completedTitle()));

    const auto details = SaveManager::getAccountUnlockedTitleDetails(account);
    int tutorialRecords = 0;
    for (const auto& detail : details)
    {
        if (detail.name == TutorialSystem::startedTitle() || detail.name == TutorialSystem::completedTitle())
        {
            ++tutorialRecords;
            assert(detail.characterName == TutorialSystem::dummyName());
            assert(!detail.unlockedAt.empty());
        }
    }
    assert(tutorialRecords == 2);

    // Re-merging an already unlocked title must never duplicate the account trophy.
    assert(SaveManager::mergeAccountUnlockedTitles(account, {TutorialSystem::completedTitle()}, TutorialSystem::dummyName()));
    const auto repeated = SaveManager::getAccountUnlockedTitleDetails(account);
    int completedRecords = 0;
    for (const auto& detail : repeated)
    {
        if (detail.name == TutorialSystem::completedTitle()) ++completedRecords;
    }
    assert(completedRecords == 1);

    fs::remove_all("assets/saves");
    return 0;
}
