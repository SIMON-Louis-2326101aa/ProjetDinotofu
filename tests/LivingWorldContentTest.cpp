#include "adventure/flavor/ExplorationLanguageTrace.hpp"
#include "adventure/flavor/ExplorationBiomeFlavor.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "world/npc/LivingNpcProfile.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "world/City.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "entity/Player.hpp"
#include "quest/QuestLog.hpp"
#include "quest/QuestCatalog.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <utility>

int main()
{
    Player player;
    assert(BiomeLivingContentCatalog::hasProfile("Archives noyées"));
    assert(BiomeLivingContentCatalog::hasProfile("Jardin des statues qui pleurent"));
    assert(ExplorationBiomeFlavor::dangerousSiteName("Jardin des statues qui pleurent").find("anges") != std::string::npos);
    assert(ExplorationBiomeFlavor::dangerousSiteWarning("Jardin des statues qui pleurent").find("statues") != std::string::npos);
    assert(ExplorationBiomeFlavor::environmentalObservation("Jardin des statues qui pleurent").find("observation") != std::string::npos || ExplorationBiomeFlavor::environmentalObservation("Jardin des statues qui pleurent").find("Observation") != std::string::npos);
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

    // Exploration test history keeps a long enough window that a four-question loop cannot repeat immediately.
    for (int i = 0; i < 12; ++i)
    {
        player.recordExplorationChallengeKey("challenge_" + std::to_string(i));
    }
    assert(player.getRecentExplorationChallengeKeys().size() == 10);
    assert(!player.wasExplorationChallengeRecentlySeen("challenge_0"));
    assert(!player.wasExplorationChallengeRecentlySeen("challenge_1"));
    assert(player.wasExplorationChallengeRecentlySeen("challenge_11"));

    // Low-level guild boards must not send field objectives into a biome whose own
    // minimum level is above the player. Services at the guild counter are exempt.
    const std::vector<std::pair<std::string, int>> biomeMinimumLevels = {
        {"Plaine sauvage", 1}, {"Route commerciale", 1}, {"Mares gélatineuses", 3},
        {"Forêt ancienne", 5}, {"Verger des lucioles de fer", 6}, {"Montagne froide", 7},
        {"Bocage aux lanternes", 8}, {"Quartier abandonné", 8}, {"Canaux de brume bleue", 9},
        {"Cimetière oublié", 10}, {"Foire abandonnée", 10}, {"Marais trouble", 12},
        {"Archives noyées", 12}, {"Marché sous les ponts", 12}, {"Ruines effondrées", 14},
        {"Mine sifflante", 14}, {"Jardin des statues qui pleurent", 14}, {"Temple des cloches fendues", 16},
        {"Falaises des drakes gris", 18}, {"Carrière des os blancs", 20}
    };
    for (int level = 1; level <= 12; ++level)
    {
        for (int sample = 0; sample < 30; ++sample)
        {
            const std::vector<Quest> offers = QuestCatalog::createGuildBoard(level);
            assert(!offers.empty());
            for (const Quest& offer : offers)
            {
                if (offer.objectiveType == "service") continue;
                for (const auto& [biomeName, minLevel] : biomeMinimumLevels)
                {
                    if (offer.location.find(biomeName) != std::string::npos)
                    {
                        assert(level >= minLevel);
                    }
                }
            }
        }
    }

    // A normal guild board should mix several objective families when the catalogue
    // offers them instead of producing six near-identical chores.
    for (int level : {1, 5, 10, 20, 35})
    {
        for (int sample = 0; sample < 30; ++sample)
        {
            const std::vector<Quest> offers = QuestCatalog::createGuildBoard(level);
            std::vector<std::string> types;
            int actionCount = 0;
            for (const Quest& offer : offers)
            {
                if (std::find(types.begin(), types.end(), offer.objectiveType) == types.end())
                {
                    types.push_back(offer.objectiveType);
                }
                if (offer.objectiveType == "combat" || offer.objectiveType == "exploration") actionCount++;
            }
            if (offers.size() >= 3) assert(types.size() >= 3);
            const int desiredActionMinimum = level >= 7 ? 3 : 2;
            const int requiredActionMinimum = std::min(desiredActionMinimum, std::max(1, static_cast<int>(offers.size()) - 1));
            assert(actionCount >= requiredActionMinimum);
        }
    }

    const City* valebrume = City::findById("valebrume");
    assert(valebrume != nullptr);
    QuestLog localityBoard;
    Quest farQuest;
    farQuest.id = "test_far_cliffs";
    farQuest.title = "Falaises lointaines";
    farQuest.objectiveType = "exploration";
    farQuest.location = "Falaises des drakes gris";
    Quest localQuest;
    localQuest.id = "test_local_plain";
    localQuest.title = "Plaine locale";
    localQuest.objectiveType = "exploration";
    localQuest.location = "Plaine sauvage";
    Quest counterQuest;
    counterQuest.id = "test_local_service";
    counterQuest.title = "Service au comptoir";
    counterQuest.objectiveType = "service";
    counterQuest.location = "Foire abandonnée / paperasse";
    localityBoard.getGuildBoardOffers() = {farQuest, localQuest, counterQuest};
    localityBoard.prioritizeGuildBoardForCity(*valebrume);
    assert(localityBoard.getGuildBoardOffers().front().id != "test_far_cliffs");
    assert(localityBoard.getGuildBoardOffers().back().id == "test_far_cliffs");

    Quest overdueQuest;
    overdueQuest.id = "test_overdue_client_request";
    overdueQuest.title = "Livraison oubliée";
    overdueQuest.client = "Prunigil le marchand";
    overdueQuest.objectiveType = "livraison";
    overdueQuest.accepted = true;
    overdueQuest.target = 1;
    overdueQuest.expiresAtDay = 1;
    assert(player.getQuestLog().addQuest(overdueQuest));
    assert(player.getQuestLog().expireOverdueQuests(2) == 1);
    QuestDeadlineSupport::synchronizeQuestConsequences(player);
    assert(player.npcKnowsFact("Prunigil le marchand", "quest_failed", overdueQuest.id));
    const int failedQuestMemoryCount = player.getCanonicalJournalCategoryTotal("quetes_echouees");
    assert(failedQuestMemoryCount == 1);
    QuestDeadlineSupport::synchronizeQuestConsequences(player);
    assert(player.getCanonicalJournalCategoryTotal("quetes_echouees") == failedQuestMemoryCount);
    const std::string failedReaction = LivingNpcProfileSystem::reactionToKnownFact(
        NpcKnowledgeSystem::profileForNamedNpc("Prunigil le marchand"), "quest_failed", overdueQuest.id);
    assert(!failedReaction.empty());
    assert(failedReaction.find("souvient") != std::string::npos || failedReaction.find("échec") != std::string::npos || failedReaction.find("affaire") != std::string::npos);

    QuestLog boardLog;
    boardLog.ensureGuildBoardReady(1, 0, 0);
    const std::size_t initialBoardSize = boardLog.getGuildBoardOffers().size();
    assert(initialBoardSize >= 6);
    assert(boardLog.removeGuildBoardOfferAt(0, 0));
    assert(boardLog.getGuildBoardOffers().size() == initialBoardSize - 1);
    assert(boardLog.getGuildBoardPendingReplacements() == 1);
    boardLog.ensureGuildBoardReady(1, 0, 0);
    assert(boardLog.getGuildBoardOffers().size() == initialBoardSize - 1);
    boardLog.ensureGuildBoardReady(1, 1, 0);
    assert(boardLog.getGuildBoardPendingReplacements() == 0);
    assert(boardLog.getGuildBoardOffers().size() == initialBoardSize);

    player.setLanguageKnowledgeLevel("gobelin", 2, false);
    player.setLanguageStudyProgress("gobelin", 45);
    std::vector<std::string> practiceNotes;
    assert(LanguageSystem::applyGuidedPractice(player, "gobelin", 2, &practiceNotes));
    assert(player.getLanguageStudyProgress("gobelin") > 45);
    assert(!practiceNotes.empty());
    return 0;
}
