#include "adventure/flavor/ExplorationLanguageTrace.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "world/npc/LivingNpcProfile.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "entity/Player.hpp"
#include <cassert>
#include <string>

int main()
{
    Player player;
    assert(BiomeLivingContentCatalog::hasProfile("Archives noyées"));
    assert(BiomeLivingContentCatalog::hasProfile("Plaine sauvage"));
    const std::vector<std::string> livingObservations = BiomeLivingContentCatalog::buildCurrentObservationLines("Archives noyées", 12);
    assert(livingObservations.size() == 3);
    for (const std::string& line : livingObservations) assert(!line.empty());

    bool foundAmbientEvent = false;
    for (int day = 0; day < 30 && !foundAmbientEvent; ++day)
    {
        const BiomeAmbientEvent ambient = BiomeAmbientEventSystem::buildCurrentEvent("Archives noyées", day, 0);
        if (!ambient.active) continue;
        foundAmbientEvent = true;
        assert(!ambient.id.empty());
        assert(!ambient.title.empty());
        assert(!ambient.lines.empty());
        assert(!BiomeAmbientEventSystem::journalKey("Archives noyées", day, ambient.id).empty());
    }
    assert(foundAmbientEvent);
    bool foundPlainEvent = false;
    for (int day = 0; day < 30 && !foundPlainEvent; ++day)
    {
        const BiomeAmbientEvent ambient = BiomeAmbientEventSystem::buildCurrentEvent("Plaine sauvage", day, 0);
        foundPlainEvent = ambient.active;
    }
    assert(foundPlainEvent);

    const auto trace = ExplorationLanguageTraceCatalog::forBiome("Archives noyées");
    assert(trace.languageId == "elfique");
    assert(ExplorationLanguageTraceCatalog::hasTrace("Archives noyées"));
    assert(!ExplorationLanguageTraceCatalog::hasTrace("Plaine sauvage"));
    const std::string unread = ExplorationLanguageTraceCatalog::renderForPlayer(player, trace);
    assert(unread.find("Texte") != std::string::npos || unread.find("fragments") != std::string::npos);
    player.setLanguageKnowledgeLevel("elfique", 2, false);
    const std::string read = ExplorationLanguageTraceCatalog::renderForPlayer(player, trace);
    assert(read.find("rayonnages") != std::string::npos);

    const LivingNpcProfile guard = LivingNpcProfileSystem::infer("Marek", "Garde de porte", "Humain");
    assert(guard.profession == "garde");
    assert(guard.temperament == "discipliné");
    assert(guard.informationNetwork == "reseau_garde");
    const std::string reaction = LivingNpcProfileSystem::reactionToKnownFact(guard, "rival_seen", "Rokk");
    assert(reaction.find("confirmer") != std::string::npos);

    const LivingNpcProfile goblinMerchant = LivingNpcProfileSystem::infer("Tik", "Marchand", "Gobelin");
    assert(goblinMerchant.nativeLanguage == "gobelin");
    assert(goblinMerchant.profession == "marchand");

    const LivingNpcProfile mira = NpcKnowledgeSystem::profileForNamedNpc("Mira");
    assert(mira.profession == "intendance");
    assert(mira.informationNetwork == "reseau_guilde");
    const LivingNpcProfile nell = NpcKnowledgeSystem::profileForNamedNpc("Nell");
    assert(nell.profession == "messagère");
    assert(nell.informationNetwork == "reseau_guilde");
    const LivingNpcProfile bob = NpcKnowledgeSystem::profileForNamedNpc("Bob");
    assert(bob.profession == "marchand itinérant");
    assert(bob.informationNetwork == "reseau_commercial");

    player.rememberNpcFact("Archiviste Meron", "discovered_writing", "trace_1", "Une stèle elfique", "temoignage_joueur", player.getName(), 72, 1);
    assert(player.npcKnowsFact("Archiviste Meron", "discovered_writing", "trace_1"));
    const std::vector<std::string> memoryLines = NpcKnowledgeSystem::conversationMemoryLines(player, "Archiviste Meron", 2);
    assert(!memoryLines.empty());
    assert(memoryLines.front().find("témoignage") != std::string::npos || memoryLines.front().find("plausible") != std::string::npos);
    const NpcKnownFact remembered = player.getNpcKnownFacts().front();
    const int confidenceNow = NpcKnowledgeSystem::effectiveConfidence(remembered, player.getWorldDaysElapsed());
    const int confidenceMuchLater = NpcKnowledgeSystem::effectiveConfidence(remembered, player.getWorldDaysElapsed() + 80);
    assert(confidenceMuchLater < confidenceNow);

    player.setLanguageKnowledgeLevel("gobelin", 2, false);
    player.setLanguageStudyProgress("gobelin", 45);
    std::vector<std::string> practiceNotes;
    assert(LanguageSystem::applyGuidedPractice(player, "gobelin", 2, &practiceNotes));
    assert(player.getLanguageStudyProgress("gobelin") > 45);
    assert(!practiceNotes.empty());
    return 0;
}
