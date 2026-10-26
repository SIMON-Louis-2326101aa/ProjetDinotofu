#include "entity/Player.hpp"
#include "progression/TitleCatalog.hpp"
#include "save/SaveManager.hpp"
#include "tutorial/TutorialSystem.hpp"

#include <cassert>
#include <filesystem>
#include <string>

int main()
{
    namespace fs = std::filesystem;
    fs::remove_all("assets/saves");

    const std::string account = "account_aware_ultimate_title_test";
    assert(SaveManager::saveAccountSnapshot(account));

    Player player;
    player.setOwnershipMetadata(account, account);

    for (const auto& definition : TitleCatalog::availableTitleDefinitions())
    {
        if (!definition.requiredForUltimate) continue;
        if (definition.name == TutorialSystem::startedTitle() || definition.name == TutorialSystem::completedTitle()) continue;
        if (definition.ultimateAlternativeGroup == "home_financing_route" && definition.name == "Plus de propriétaire") continue;
        player.grantTitle(definition.name);
    }

    // The two tutorial trophies are account-only and are still missing here.
    assert(!player.hasTitle("Complétionniste de l'impossible"));

    assert(SaveManager::mergeAccountUnlockedTitles(
        account,
        {TutorialSystem::startedTitle(), TutorialSystem::completedTitle()},
        TutorialSystem::dummyName()
    ));

    // Re-granting an already owned session title refreshes account-aware completion.
    assert(!player.grantTitle("Bienvenue dans Dinotofu"));
    assert(player.hasTitle("Complétionniste de l'impossible"));

    // Either route of an explicit alternative group satisfies that group.
    std::vector<std::string> owned;
    for (const auto& definition : TitleCatalog::availableTitleDefinitions())
    {
        if (!definition.requiredForUltimate) continue;
        if (definition.name == "Plus de propriétaire") continue;
        owned.push_back(definition.name);
    }
    assert(TitleCatalog::hasAllUltimatePrerequisites(owned));

    fs::remove_all("assets/saves");
    return 0;
}
