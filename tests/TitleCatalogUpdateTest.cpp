#include "progression/TitleCatalog.hpp"
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

    assert(TitleCatalog::catalogVersion() == "V3.57.02");
    const auto& titles = TitleCatalog::availableTitleDefinitions();
    assert(titles.size() == 419);

    const char* tutorialTitles[] = {
        "Rencontre avec notre cobaye",
        "J'ai lu le manuel"
    };
    for (const char* titleName : tutorialTitles)
    {
        const auto* definition = TitleCatalog::findTitleDefinition(titleName);
        assert(definition != nullptr);
        assert(definition->requiredForUltimate);
    }

    const char* profileOrOptionalTitles[] = {
        "Ami du petit Dinotofu",
        "Œil de serrure"
    };
    for (const char* titleName : profileOrOptionalTitles)
    {
        const auto* definition = TitleCatalog::findTitleDefinition(titleName);
        assert(definition != nullptr);
        assert(!definition->requiredForUltimate);
    }

    const char* v35702Titles[] = {
        "On a un QG !", "Ça devient sérieux", "Prix d'ami",
        "Enfin chez moi", "Mon lit, mes règles", "Jusqu'à ce que la sauvegarde nous sépare",
        "Jusqu'à ce que le ping nous sépare",
        "Dossier : retrouvée", "C'était ma première vraie amie",
        "Sur la piste", "Viens sans faire d'histoire", "J'avais gardé la piste",
        "Évidemment, ce n'était pas fini.", "Puisqu'on y est...",
        "Facteur officiel", "Ce n'est pas une lettre, ça.", "Avec correspondance",
        "J'ai touché de l'herbe", "On barricade d'abord",
        "Le convoi est arrivé... à peu près", "Il manquait quelque chose au chargement"
    };
    for (const char* titleName : v35702Titles)
    {
        assert(TitleCatalog::findTitleDefinition(titleName) != nullptr);
    }

    const auto* rentRoute = TitleCatalog::findTitleDefinition("Le loyer, cette quête hebdomadaire");
    const auto* ownershipRoute = TitleCatalog::findTitleDefinition("Plus de propriétaire");
    assert(rentRoute != nullptr && ownershipRoute != nullptr);
    assert(rentRoute->requiredForUltimate && ownershipRoute->requiredForUltimate);
    assert(rentRoute->ultimateAlternativeGroup == "home_financing_route");
    assert(ownershipRoute->ultimateAlternativeGroup == "home_financing_route");

    const auto* cheatTrophy = TitleCatalog::findTitleDefinition("On ne dira rien...");
    assert(cheatTrophy != nullptr);
    assert(!cheatTrophy->requiredForUltimate);

    const auto* reaperSignal = TitleCatalog::findTitleDefinition("Canal interdit");
    assert(reaperSignal != nullptr);
    assert(!reaperSignal->visibleBeforeUnlock);

    // Reaper-13 appartient au chef existant : ce n'est pas un titre de progression du joueur.
    assert(TitleCatalog::findTitleDefinition("Reaper-13") == nullptr);
    assert(TitleCatalog::findTitleDefinition("Cible prioritaire") != nullptr);

    // Inventory / storage / morphology title family added in V3.51.51.
    const char* expectedLifestyleTitles[] = {
        "Enfin de la place", "On pousse les murs", "Chaque chose à sa place",
        "Optimisation maximale", "Plein à craquer", "Ça rentre encore.",
        "Minimaliste", "Conservateur", "Réseau de stockage",
        "Plus léger qu'avant", "Taillé pour porter", "Silhouette travaillée",
        "Déménageur", "Sous le matelas", "Prêt pour le long voyage"
    };
    for (const char* titleName : expectedLifestyleTitles)
    {
        const auto* definition = TitleCatalog::findTitleDefinition(titleName);
        assert(definition != nullptr);
        assert(!definition->requiredForUltimate);
    }

    Player player;
    assert(!player.areHiddenTitlesRevealed());
    player.setRevealHiddenTitles(true);
    assert(player.areHiddenTitlesRevealed());

    const std::string account = "title_catalog_update_test";
    player.setOwnershipMetadata(account, account);
    assert(SaveManager::saveAccountSnapshot(account));
    assert(SaveManager::savePlayerSnapshot(player, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));

    const auto summaries = SaveManager::listPlayableCharacters(account);
    assert(!summaries.empty());
    Player loaded;
    DifficultyMode loadedDifficulty = DifficultyMode::Easy;
    DeathRuleMode loadedDeathRule = DeathRuleMode::NonDefinitive;
    assert(SaveManager::loadPlayerSnapshot(summaries.front(), loaded, loadedDifficulty, loadedDeathRule));
    assert(loaded.areHiddenTitlesRevealed());

    fs::remove_all("assets/saves");
    return 0;
}
