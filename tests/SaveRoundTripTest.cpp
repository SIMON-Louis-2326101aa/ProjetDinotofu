#include "entity/Player.hpp"
#include "quest/Quest.hpp"
#include "save/SaveManager.hpp"
#include "save/SaveSchemaVersion.hpp"
#include "item/weapon/Weapon.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

int main()
{
    namespace fs = std::filesystem;

    fs::remove_all("assets/saves");

    Player original;
    original.setRace(CharacterRace::Demon);
    original.setLanguageKnowledgeLevel("gobelin", 2, false);
    original.setLanguageStudyProgress("gobelin", 43);
    Weapon rememberedWeapon(
        "Épée témoin",
        "Arme créée uniquement pour vérifier la mémoire persistante d'un exemplaire.",
        125,
        WeaponType::Sword,
        3,
        7,
        1,
        80
    );
    rememberedWeapon.setPersistentId("roundtrip_weapon_memory_001");
    original.getInventory().addWeapon(rememberedWeapon);
    original.recordHistoricalEvent(
        "item_memory_enemy_signature",
        rememberedWeapon.getPersistentId(),
        "L'Épée témoin a encaissé la signature d'un ennemi de test",
        true
    );

    original.rememberNpcFact(
        "Archiviste Meron",
        "discovered_writing",
        "archives_noyees_stela",
        "Stèle elfique retrouvée dans les Archives noyées",
        "temoignage_joueur",
        original.getName(),
        74,
        1,
        "stela_elfique",
        "bibliotheque"
    );

    Quest quest;
    quest.id = "roundtrip_foreign_contract";
    quest.title = "Contrat gobelin de test";
    quest.objective = "Comprendre le message gobelin.";
    quest.guildQuest = true;
    quest.accepted = true;
    quest.target = 1;
    quest.requiredLanguage = "gobelin";
    quest.requiredLanguageLevel = 2;
    quest.sourceLanguageText = "Grakka tik, vor nakka.";
    original.getQuestLog().getQuests().push_back(quest);

    const std::string rivalId = original.createRivalFromEnemy(
        "Hobgobelin balafré", "hobgobelin", 18, 240, 31,
        "a survécu à une embuscade", "calculateur", 47
    );
    original.recordRivalEscape(rivalId, "route_commerciale");
    original.recordRivalWound(rivalId, 2);
    original.recordRivalReturn(rivalId, "marche_sous_les_ponts");
    original.recordCanonicalEvent(
        "techniques_combinees_alliees",
        "Aline|Boros:Feu croisé",
        "Aline + Boros : Feu croisé",
        7
    );

    const std::string account = "roundtrip_test_account";
    assert(SaveManager::saveAccountSnapshot(account));
    assert(SaveManager::savePlayerSnapshot(original, account, DifficultyMode::Normal, DeathRuleMode::NonDefinitive));

    const std::vector<CharacterSaveSummary> summaries = SaveManager::listPlayableCharacters(account);
    assert(!summaries.empty());

    Player loaded;
    DifficultyMode difficulty = DifficultyMode::Easy;
    DeathRuleMode deathRule = DeathRuleMode::NonDefinitive;
    assert(SaveManager::loadPlayerSnapshot(summaries.front(), loaded, difficulty, deathRule));

    assert(loaded.getLanguageKnowledgeLevel("commun") >= 3);
    assert(loaded.getLanguageKnowledgeLevel("infernal") >= 3);
    assert(loaded.getLanguageKnowledgeLevel("gobelin") == 2);
    assert(loaded.getLanguageStudyProgress("gobelin") == 43);
    assert(loaded.getNpcKnownFacts().size() == 1);
    assert(loaded.getNpcKnownFacts().front().npcId == "Archiviste Meron");
    assert(loaded.getNpcKnownFacts().front().confidence == 74);
    assert(loaded.getNpcKnownFacts().front().claimVariant == "stela_elfique");
    assert(loaded.getNpcKnownFacts().front().relayChannel == "bibliotheque");

    const auto& rivals = loaded.getRivalRecords();
    assert(rivals.size() == 1);
    assert(rivals.front().enemyName == "Hobgobelin balafré");
    assert(rivals.front().temperament == "calculateur");
    assert(rivals.front().escapes == 1);
    assert(rivals.front().returns == 1);
    assert(rivals.front().wounds == 2);
    assert(rivals.front().emergenceScore == 47);
    assert(rivals.front().visibleMark.find("cicatrice") != std::string::npos || rivals.front().visibleMark.find("silhouette") != std::string::npos);
    assert(loaded.getCanonicalJournalCategoryTotal("techniques_combinees_alliees") >= 7);
    bool foundRememberedWeapon = false;
    for (const Weapon& weapon : loaded.getInventory().getWeapons())
    {
        if (weapon.getPersistentId() != "roundtrip_weapon_memory_001") continue;
        foundRememberedWeapon = true;
        assert(weapon.getName() == "Épée témoin");
        const std::vector<PlayerHistoricalEvent> itemMemories = loaded.getHistoricalEventsForSubject(weapon.getPersistentId(), 10);
        assert(itemMemories.size() == 1);
        assert(itemMemories.front().category == "item_memory_enemy_signature");
    }
    assert(foundRememberedWeapon);

    bool foundQuest = false;
    for (const Quest& loadedQuest : loaded.getQuestLog().getQuests())
    {
        if (loadedQuest.id != quest.id) continue;
        assert(loadedQuest.requiredLanguage == "gobelin");
        assert(loadedQuest.requiredLanguageLevel == 2);
        assert(loadedQuest.sourceLanguageText == quest.sourceLanguageText);
        foundQuest = true;
    }
    assert(foundQuest);

    // Simulate a V3.49.93-style save with neither NPC memory nor languageState sections.
    {
        std::ifstream input(summaries.front().path);
        std::ostringstream buffer;
        buffer << input.rdbuf();
        std::string legacy = buffer.str();
        const std::size_t start = legacy.find("  \"npcMemoryState\": {");
        const std::size_t next = legacy.find("  \"cheatState\": {", start);
        assert(start != std::string::npos && next != std::string::npos);
        legacy.erase(start, next - start);
        const std::string currentVersion = "\"saveVersion\": " + std::to_string(SaveSchemaVersion::Current);
        const std::size_t versionPos = legacy.find(currentVersion);
        assert(versionPos != std::string::npos);
        legacy.replace(versionPos, currentVersion.size(), "\"saveVersion\": 20");
        std::ofstream output(summaries.front().path, std::ios::trunc);
        output << legacy;
    }

    Player migrated;
    difficulty = DifficultyMode::Easy;
    deathRule = DeathRuleMode::NonDefinitive;
    assert(SaveManager::loadPlayerSnapshot(summaries.front(), migrated, difficulty, deathRule));
    assert(migrated.getLanguageKnowledgeLevel("commun") >= 3);
    assert(migrated.getLanguageKnowledgeLevel("infernal") >= 3);
    assert(migrated.getLanguageKnowledgeLevel("gobelin") == 0);
    assert(migrated.getNpcKnownFacts().empty());

    fs::remove_all("assets/saves");
    return 0;
}
