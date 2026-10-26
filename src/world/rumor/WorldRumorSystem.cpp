#include "world/rumor/WorldRumorSystem.hpp"

#include "entity/Player.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    int intField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos) return fallback;
        std::size_t start = pos + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-') ++end;
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end]))) ++end;
        if (end <= start) return fallback;
        try { return std::stoi(label.substr(start, end - start)); }
        catch (...) { return fallback; }
    }

    std::string stringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos) return fallback;
        const std::size_t start = pos + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos) end = label.size();
        if (end <= start) return fallback;
        return label.substr(start, end - start);
    }

    const PlayerJournalRecord* findRecord(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return &record;
        }
        return nullptr;
    }
}

std::string WorldRumorSystem::sourceTypeId(WorldRumorSourceType sourceType)
{
    switch (sourceType)
    {
        case WorldRumorSourceType::Tavern: return "tavern";
        case WorldRumorSourceType::Traveler: return "traveler";
        case WorldRumorSourceType::Guild: return "guild";
        case WorldRumorSourceType::Guard: return "guard";
        case WorldRumorSourceType::Archive: return "archive";
        case WorldRumorSourceType::Witness: return "witness";
        case WorldRumorSourceType::DirectObservation: return "direct";
        default: return "unknown";
    }
}

int WorldRumorSystem::sourceCredibilityBonus(WorldRumorSourceType sourceType)
{
    switch (sourceType)
    {
        case WorldRumorSourceType::DirectObservation: return 30;
        case WorldRumorSourceType::Archive: return 18;
        case WorldRumorSourceType::Guard: return 15;
        case WorldRumorSourceType::Guild: return 12;
        case WorldRumorSourceType::Witness: return 10;
        case WorldRumorSourceType::Traveler: return 3;
        case WorldRumorSourceType::Tavern: return -5;
        default: return -10;
    }
}

WorldRumorStatus WorldRumorSystem::statusFor(const Player& player, const std::string& rumorId)
{
    WorldRumorStatus status;
    status.rumorId = rumorId;
    const PlayerJournalRecord* rumor = findRecord(player, "world_rumors", rumorId);
    if (rumor != nullptr)
    {
        status.known = true;
        status.subjectId = stringField(rumor->label, "subject", "unknown");
        status.text = stringField(rumor->label, "text", rumor->label);
        status.lastSource = stringField(rumor->label, "source", "unknown");
        status.credibility = std::clamp(intField(rumor->label, "credibility", 0), 0, 100);
        status.evidenceLevel = std::clamp(intField(rumor->label, "evidence", 0), 0, 5);
        status.heardCount = std::max(0, rumor->count);
        status.lastDay = rumor->lastDay;
    }
    status.confirmed = findRecord(player, "world_rumor_confirmations", rumorId) != nullptr;
    status.contradicted = findRecord(player, "world_rumor_contradictions", rumorId) != nullptr;
    if (status.confirmed)
    {
        status.known = true;
        status.credibility = std::max(status.credibility, 90);
        status.evidenceLevel = std::max(status.evidenceLevel, 3);
    }
    return status;
}

void WorldRumorSystem::recordRumor(
    Player& player,
    const std::string& rumorId,
    const std::string& subjectId,
    const std::string& text,
    WorldRumorSourceType sourceType,
    int credibility,
    int evidenceLevel)
{
    if (rumorId.empty() || subjectId.empty() || text.empty()) return;
    const WorldRumorStatus previous = statusFor(player, rumorId);
    const int adjusted = std::clamp(credibility + sourceCredibilityBonus(sourceType), 0, 100);
    const int mergedCredibility = previous.known
        ? std::clamp(std::max(previous.credibility, (previous.credibility + adjusted * 2) / 3), 0, 100)
        : adjusted;
    const int mergedEvidence = std::max(previous.evidenceLevel, std::clamp(evidenceLevel, 0, 5));
    const std::string label = "subject=" + subjectId
        + " | text=" + text
        + " | source=" + sourceTypeId(sourceType)
        + " | credibility=" + std::to_string(mergedCredibility)
        + " | evidence=" + std::to_string(mergedEvidence);
    player.recordCanonicalEvent("world_rumors", rumorId, label, 1);
}

void WorldRumorSystem::confirmRumor(
    Player& player,
    const std::string& rumorId,
    const std::string& subjectId,
    const std::string& evidenceText,
    int evidenceLevel)
{
    if (rumorId.empty() || subjectId.empty()) return;
    recordRumor(player, rumorId, subjectId,
        evidenceText.empty() ? "preuve directe" : evidenceText,
        WorldRumorSourceType::DirectObservation,
        80,
        std::max(3, evidenceLevel));
    player.recordCanonicalEvent(
        "world_rumor_confirmations",
        rumorId,
        "subject=" + subjectId + " | evidence=" + (evidenceText.empty() ? "preuve directe" : evidenceText),
        1);
}

void WorldRumorSystem::contradictRumor(
    Player& player,
    const std::string& rumorId,
    const std::string& subjectId,
    const std::string& evidenceText)
{
    if (rumorId.empty() || subjectId.empty()) return;
    player.recordCanonicalEvent(
        "world_rumor_contradictions",
        rumorId,
        "subject=" + subjectId + " | evidence=" + (evidenceText.empty() ? "preuve contradictoire" : evidenceText),
        1);
}

std::vector<std::string> WorldRumorSystem::buildSummaryLines(const Player& player, const std::string& rumorId)
{
    const WorldRumorStatus status = statusFor(player, rumorId);
    if (!status.known) return {"Aucune rumeur crédible enregistrée pour ce sujet."};
    std::vector<std::string> lines;
    lines.push_back(status.text);
    lines.push_back("Crédibilité estimée : " + std::to_string(status.credibility) + "% | niveau de preuve : " + std::to_string(status.evidenceLevel) + ".");
    lines.push_back("Entendue/actualisée " + std::to_string(status.heardCount) + " fois | dernière source : " + status.lastSource + ".");
    if (status.confirmed) lines.push_back("Statut : au moins un élément direct a confirmé le noyau de cette rumeur.");
    if (status.contradicted) lines.push_back("Attention : des éléments contradictoires existent aussi. Une légende peut mélanger faits réels et déformations.");
    return lines;
}
