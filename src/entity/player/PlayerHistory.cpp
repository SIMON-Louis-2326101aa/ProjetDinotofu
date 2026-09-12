#include "entity/Player.hpp"

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
            "rivaux_potentiels", "destin_instable", "serments_pretes", "serments_rompus",
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

std::string Player::createRivalFromEnemy(const std::string& enemyName, const std::string& enemyFamily, int level, int maxHp, int attack, const std::string& reason)
{
    PlayerRivalRecord rival;
    rival.rivalId = makeHistoryId("rival");
    rival.enemyName = enemyName.empty() ? "Adversaire inconnu" : enemyName;
    rival.enemyFamily = enemyFamily;
    rival.originLocationId = currentCityId;
    rival.lastKnownLocationId = currentCityId;
    rival.rivalryReason = reason;
    rival.baseLevel = std::max(1, level);
    rival.currentLevel = rival.baseLevel;
    rival.baseMaxHp = std::max(1, maxHp);
    rival.baseAttack = std::max(1, attack);
    rival.firstSeenDay = worldDaysElapsed;
    rival.lastSeenDay = worldDaysElapsed;
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
    const int evolvedLevel = rival->baseLevel + rival->returns * 2 + rival->wounds;
    rival->currentLevel = evolvedLevel > 255 ? 255 : evolvedLevel;
    rival->lastSeenDay = worldDaysElapsed;
    if (!locationId.empty()) rival->lastKnownLocationId = locationId;
    recordHistoricalEvent("rival_return", rivalId, rival->enemyName + " réapparaît après avoir survécu.");
}

void Player::recordRivalWound(const std::string& rivalId, int amount)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive || amount <= 0) return;
    rival->wounds += amount;
    rival->lastSeenDay = worldDaysElapsed;
    recordHistoricalEvent("rival_wound", rivalId, rival->enemyName + " conserve une blessure de la rencontre.");
}

void Player::markRivalDefeated(const std::string& rivalId, const std::string& locationId)
{
    PlayerRivalRecord* rival = findMutableRival(rivalId);
    if (!rival || !rival->alive) return;
    rival->alive = false;
    rival->lastSeenDay = worldDaysElapsed;
    if (!locationId.empty()) rival->lastKnownLocationId = locationId;
    recordHistoricalEvent("rival_defeated", rivalId, rival->enemyName + " est définitivement vaincu.", true);
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
        rivalRecords.push_back(rival);
    }
}
