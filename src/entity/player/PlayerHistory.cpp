#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <set>

namespace
{
    std::string makeHistoryId(const std::string& prefix)
    {
        static std::atomic<unsigned long long> counter{1};
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        return prefix + "-" + std::to_string(static_cast<unsigned long long>(now)) + "-" + std::to_string(counter.fetch_add(1));
    }

    bool shouldMigrateCanonicalCategory(const std::string& category)
    {
        static const std::set<std::string> important = {
            "rivaux_potentiels", "rivaux_confirmes", "rumeurs_rivaux", "destin_instable", "serments_pretes", "serments_rompus",
            "objets_avec_memoire", "cicatrices", "traumatismes", "heritage",
            "tombes", "techniques_combinees_alliees", "serments_fragilises"
        };
        return important.count(category) > 0;
    }
}

void Player::recordHistoricalEvent(const std::string& category, const std::string& subjectId, const std::string& label, bool resolved)
{
    if (category.empty() || subjectId.empty()) return;

    PlayerHistoricalEvent event;
    event.id = makeHistoryId("history");
    event.category = category;
    event.subjectId = subjectId;
    event.label = label.empty() ? subjectId : label;
    event.locationId = currentCityId;
    event.day = worldDaysElapsed;
    event.resolved = resolved;
    historicalEvents.push_back(event);

    if (historicalEvents.size() > 800)
    {
        auto removable = std::find_if(historicalEvents.begin(), historicalEvents.end(), [](const PlayerHistoricalEvent& e) {
            return e.resolved;
        });
        if (removable != historicalEvents.end()) historicalEvents.erase(removable);
    }
}

const std::vector<PlayerHistoricalEvent>& Player::getHistoricalEvents() const
{
    return historicalEvents;
}

std::vector<PlayerHistoricalEvent> Player::getHistoricalEventsForSubject(const std::string& subjectId, int limit) const
{
    std::vector<PlayerHistoricalEvent> out;
    for (auto it = historicalEvents.rbegin(); it != historicalEvents.rend(); ++it)
    {
        if (it->subjectId == subjectId)
        {
            out.push_back(*it);
            if (limit > 0 && static_cast<int>(out.size()) >= limit) break;
        }
    }
    return out;
}

void Player::resolveHistoricalEvent(const std::string& eventId)
{
    for (PlayerHistoricalEvent& event : historicalEvents)
    {
        if (event.id == eventId)
        {
            event.resolved = true;
            return;
        }
    }
}

void Player::setLoadedHistoricalEvents(const std::vector<PlayerHistoricalEvent>& events)
{
    historicalEvents.clear();
    for (PlayerHistoricalEvent event : events)
    {
        if (event.category.empty() || event.subjectId.empty()) continue;
        if (event.id.empty()) event.id = makeHistoryId("history");
        event.day = std::max(0, event.day);
        historicalEvents.push_back(event);
    }
}

void Player::migrateImportantCanonicalHistory()
{
    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (!shouldMigrateCanonicalCategory(record.category)) continue;
        const bool exists = std::any_of(historicalEvents.begin(), historicalEvents.end(), [&](const PlayerHistoricalEvent& e) {
            return e.category == record.category && e.subjectId == record.key && e.label == record.label;
        });
        if (!exists)
        {
            PlayerHistoricalEvent event;
            event.id = makeHistoryId("legacy");
            event.category = record.category;
            event.subjectId = record.key;
            event.label = record.label.empty() ? record.key : record.label;
            event.locationId = record.locationId;
            event.day = std::max(0, record.lastDay);
            event.resolved = false;
            historicalEvents.push_back(event);
        }
    }
}

std::string Player::createRivalFromEnemy(
    const std::string& enemyName,
    const std::string& enemyFamily,
    int level,
    int maxHp,
    int attack,
    const std::string& reason,
    const std::string& temperament,
    int emergenceScore
)
{
    PlayerRivalRecord rival;
    rival.rivalId = makeHistoryId("rival");
    rival.enemyName = enemyName.empty() ? "Adversaire inconnu" : enemyName;
    rival.enemyFamily = enemyFamily;
    rival.originLocationId = currentCityId;
    rival.lastKnownLocationId = currentCityId;
    rival.rivalryReason = reason;
    rival.temperament = temperament.empty() ? "survivant prudent" : temperament;
    rival.lastOutcome = "fuite fondatrice";
    rival.baseLevel = std::max(1, level);
    rival.currentLevel = rival.baseLevel;
    rival.baseMaxHp = std::max(1, maxHp);
    rival.baseAttack = std::max(1, attack);
    rival.firstSeenDay = worldDaysElapsed;
    rival.lastSeenDay = worldDaysElapsed;
    rival.emergenceScore = std::clamp(emergenceScore, 0, 100);
    rival.notoriety = 1;
    rivalRecords.push_back(rival);
    recordHistoricalEvent("rival_created", rival.rivalId, rival.enemyName + " devient un rival identifiable.");
    return rival.rivalId;
}

PlayerRivalRecord* Player::findMutableRival(const std::string& rivalId)
{
    for (PlayerRivalRecord& rival : rivalRecords) if (rival.rivalId == rivalId) return &rival;
    return nullptr;
}

const PlayerRivalRecord* Player::findRival(const std::string& rivalId) const
{
    for (const PlayerRivalRecord& rival : rivalRecords) if (rival.rivalId == rivalId) return &rival;
    return nullptr;
}

const std::vector<PlayerRivalRecord>& Player::getRivalRecords() const
{
    return rivalRecords;
}

void Player::recordRivalEscape(const std::string& rivalId, const std::string& locationId)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive) return;
    ++rival->escapes;
    rival->notoriety = std::min(100, rival->notoriety + 1);
    rival->lastOutcome = "nouvelle fuite";
    rival->lastSeenDay = worldDaysElapsed;
    if (!locationId.empty()) rival->lastKnownLocationId = locationId;
    recordHistoricalEvent("rival_escape", rivalId, rival->enemyName + " survit et s'échappe.");
}

void Player::recordRivalReturn(const std::string& rivalId, const std::string& locationId)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive) return;
    ++rival->returns;
    ++rival->encounters;
    rival->notoriety = std::min(100, rival->notoriety + 2);
    rival->lastOutcome = "retour confirmé";
    const int evolvedLevel = rival->baseLevel + rival->returns * 2 + rival->wounds;
    rival->currentLevel = evolvedLevel > 255 ? 255 : evolvedLevel;
    rival->lastSeenDay = worldDaysElapsed;
    if (!locationId.empty()) rival->lastKnownLocationId = locationId;
    recordCanonicalEvent("rumeurs_rivaux", rivalId, rival->enemyName + " a été reconnu lors d'un retour réel", 1);
    recordHistoricalEvent("rival_return", rivalId, rival->enemyName + " réapparaît après avoir survécu.");
}

void Player::recordRivalWound(const std::string& rivalId, int amount)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive || amount <= 0) return;
    rival->wounds += amount;
    rival->notoriety = std::min(100, rival->notoriety + amount);
    rival->lastOutcome = "blessure conservée";
    if (rival->visibleMark.empty())
    {
        rival->visibleMark = rival->wounds >= 2
            ? "cicatrices visibles laissées par plusieurs rencontres"
            : "cicatrice encore visible de sa fuite";
    }
    else if (rival->wounds >= 3)
    {
        rival->visibleMark = "silhouette marquée par plusieurs blessures anciennes";
    }
    rival->lastSeenDay = worldDaysElapsed;
    recordHistoricalEvent("rival_wound", rivalId, rival->enemyName + " conserve une blessure de la rencontre.");
}

bool Player::rivalHasStolenProperty(const std::string& rivalId) const
{
    const PlayerRivalRecord* rival = findRival(rivalId);
    if (!rival) return false;
    return rival->stolenCopper > 0 || rival->stolenMaterialQuantity > 0;
}

void Player::recordRivalStolenCopper(const std::string& rivalId, long long copperAmount)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive || copperAmount <= 0) return;
    rival->stolenCopper += copperAmount;
    rival->lastOutcome = "fuite avec bien volé";
    rival->notoriety = std::min(100, rival->notoriety + 4);
    recordCanonicalEvent("vols_rivaux", rivalId, rival->enemyName + " s'est enfui avec une partie de ta bourse", 1);
    recordHistoricalEvent("rival_theft", rivalId, rival->enemyName + " s'enfuit avec un bien réellement volé.");
}

void Player::recordRivalStolenMaterial(const std::string& rivalId, const Material& material, int quantity)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive || quantity <= 0) return;

    // One compact material stack per rival is enough for the first real theft layer. If the rival
    // already carries another material type, keep the existing stash instead of overwriting it.
    if (rival->stolenMaterialQuantity > 0 && rival->stolenMaterialId != material.getId()) return;

    rival->stolenMaterialId = material.getId();
    rival->stolenMaterialName = material.getName();
    rival->stolenMaterialQuality = material.getQuality();
    rival->stolenMaterialQuantity += quantity;
    rival->lastOutcome = "fuite avec bien volé";
    rival->notoriety = std::min(100, rival->notoriety + 4);
    recordCanonicalEvent("vols_rivaux", rivalId, rival->enemyName + " s'est enfui avec " + material.getName(), quantity);
    recordHistoricalEvent("rival_theft", rivalId, rival->enemyName + " s'enfuit avec " + material.getName() + ".");
}

bool Player::recoverRivalStolenProperty(const std::string& rivalId)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival) return false;

    bool recovered = false;
    if (rival->stolenCopper > 0)
    {
        inventory.earnCopper(rival->stolenCopper);
        recordCanonicalEvent("biens_voles_recuperes", rivalId, rival->enemyName + " rend la partie de bourse volée", 1);
        rival->stolenCopper = 0;
        recovered = true;
    }

    if (rival->stolenMaterialQuantity > 0 && !rival->stolenMaterialId.empty())
    {
        inventory.addMaterial(MaterialCatalog::createById(
            rival->stolenMaterialId,
            rival->stolenMaterialQuantity,
            rival->stolenMaterialQuality.empty() ? "normal" : rival->stolenMaterialQuality));
        recordCanonicalEvent(
            "biens_voles_recuperes",
            rivalId,
            rival->enemyName + " rend " + (rival->stolenMaterialName.empty() ? rival->stolenMaterialId : rival->stolenMaterialName),
            rival->stolenMaterialQuantity);
        rival->stolenMaterialId.clear();
        rival->stolenMaterialName.clear();
        rival->stolenMaterialQuality = "normal";
        rival->stolenMaterialQuantity = 0;
        recovered = true;
    }

    if (recovered)
    {
        rival->lastOutcome = "bien volé récupéré";
        recordHistoricalEvent("rival_theft_recovered", rivalId, "Le bien volé par " + rival->enemyName + " a été récupéré.", true);
        grantTitle("Rends-moi ça.");
    }
    return recovered;
}

void Player::markRivalDefeated(const std::string& rivalId, const std::string& locationId)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive) return;
    rival->alive = false;
    rival->lastOutcome = "vaincu définitivement";
    rival->notoriety = std::min(100, rival->notoriety + 3);
    rival->lastSeenDay = worldDaysElapsed;
    if (!locationId.empty()) rival->lastKnownLocationId = locationId;
    recordHistoricalEvent("rival_defeated", rivalId, rival->enemyName + " est définitivement vaincu.", true);
    recoverRivalStolenProperty(rivalId);
    for (const PlayerVaultTheftCase& theftCase : getVaultTheftCases())
    {
        if (!theftCase.resolved && theftCase.identifiedRivalId == rivalId)
        {
            recoverVaultTheftCaseProperty(theftCase.caseId);
            break;
        }
    }
}

void Player::setLoadedRivalRecords(const std::vector<PlayerRivalRecord>& rivals)
{
    rivalRecords.clear();
    for (PlayerRivalRecord rival : rivals)
    {
        if (rival.rivalId.empty()) rival.rivalId = makeHistoryId("rival");
        if (rival.enemyName.empty()) continue;
        rival.baseLevel = std::max(1, rival.baseLevel);
        rival.currentLevel = std::max(rival.baseLevel, rival.currentLevel);
        rival.baseMaxHp = std::max(1, rival.baseMaxHp);
        rival.baseAttack = std::max(1, rival.baseAttack);
        if (rival.temperament.empty()) rival.temperament = "survivant prudent";
        rival.emergenceScore = std::clamp(rival.emergenceScore, 0, 100);
        rival.notoriety = std::clamp(rival.notoriety, 0, 100);
        rival.stolenCopper = std::max<long long>(0, rival.stolenCopper);
        rival.stolenMaterialQuantity = std::max(0, rival.stolenMaterialQuantity);
        if (rival.stolenMaterialId.empty())
        {
            rival.stolenMaterialQuantity = 0;
            rival.stolenMaterialName.clear();
            rival.stolenMaterialQuality = "normal";
        }
        if (rival.stolenMaterialQuality.empty()) rival.stolenMaterialQuality = "normal";
        rivalRecords.push_back(rival);
    }
}
