#include "entity/Player.hpp"

#include <algorithm>

namespace
{
    int clampConfidence(int value)
    {
        return std::clamp(value, 0, 100);
    }

    int clampEvidence(int value)
    {
        return std::clamp(value, 0, 3);
    }
}

void Player::rememberNpcFact(
    const std::string& npcId,
    const std::string& factType,
    const std::string& subjectId,
    const std::string& label,
    const std::string& sourceType,
    const std::string& sourceId,
    int confidence,
    int evidenceLevel,
    const std::string& claimVariant,
    const std::string& relayChannel
)
{
    if (npcId.empty() || factType.empty() || subjectId.empty()) return;

    confidence = clampConfidence(confidence);
    evidenceLevel = clampEvidence(evidenceLevel);
    const std::string normalizedVariant = claimVariant.empty() ? "default" : claimVariant;

    auto weakenContradictoryClaimsIfProven = [&]() {
        if (evidenceLevel < 3) return;
        for (NpcKnownFact& other : npcKnownFacts)
        {
            if (other.npcId != npcId || other.factType != factType || other.subjectId != subjectId) continue;
            if (other.claimVariant == normalizedVariant) continue;
            if (other.evidenceLevel >= evidenceLevel) continue;

            const int reduction = other.evidenceLevel <= 0 ? 28 : (other.evidenceLevel == 1 ? 20 : 10);
            other.confidence = std::max(15, other.confidence - reduction);
        }
    };

    for (NpcKnownFact& fact : npcKnownFacts)
    {
        if (fact.npcId != npcId || fact.factType != factType || fact.subjectId != subjectId || fact.claimVariant != normalizedVariant) continue;

        fact.timesHeard = std::max(1, fact.timesHeard + 1);
        fact.lastReinforcedDay = worldDaysElapsed;
        if (!label.empty()) fact.label = label;
        if (!currentCityId.empty()) fact.locationId = currentCityId;
        if (!relayChannel.empty()) fact.relayChannel = relayChannel;

        // Hearing the exact same source repeatedly cannot manufacture certainty.
        // A stronger source/evidence can genuinely reinforce the memory.
        const bool strongerSource = evidenceLevel > fact.evidenceLevel
            || (!sourceId.empty() && sourceId != fact.sourceId);
        if (strongerSource)
        {
            fact.confidence = std::max(fact.confidence, confidence);
            fact.evidenceLevel = std::max(fact.evidenceLevel, evidenceLevel);
            fact.sourceType = sourceType.empty() ? fact.sourceType : sourceType;
            fact.sourceId = sourceId.empty() ? fact.sourceId : sourceId;
        }
        else
        {
            fact.confidence = std::max(fact.confidence, std::min(100, confidence));
        }
        weakenContradictoryClaimsIfProven();
        return;
    }

    NpcKnownFact fact;
    fact.npcId = npcId;
    fact.factType = factType;
    fact.subjectId = subjectId;
    fact.label = label.empty() ? subjectId : label;
    fact.sourceType = sourceType.empty() ? "inconnue" : sourceType;
    fact.sourceId = sourceId;
    fact.locationId = currentCityId;
    fact.claimVariant = normalizedVariant;
    fact.relayChannel = relayChannel;
    fact.firstLearnedDay = worldDaysElapsed;
    fact.lastReinforcedDay = worldDaysElapsed;
    fact.confidence = confidence;
    fact.evidenceLevel = evidenceLevel;
    fact.timesHeard = 1;
    weakenContradictoryClaimsIfProven();
    npcKnownFacts.push_back(fact);

    // Avoid unbounded growth while keeping direct/proven memories preferentially.
    if (npcKnownFacts.size() > 1200)
    {
        auto removable = std::find_if(npcKnownFacts.begin(), npcKnownFacts.end(), [](const NpcKnownFact& value) {
            return value.evidenceLevel <= 1 && value.confidence < 70;
        });
        if (removable != npcKnownFacts.end()) npcKnownFacts.erase(removable);
    }
}

const std::vector<NpcKnownFact>& Player::getNpcKnownFacts() const
{
    return npcKnownFacts;
}

std::vector<NpcKnownFact> Player::getNpcKnownFactsFor(const std::string& npcId, int limit) const
{
    std::vector<NpcKnownFact> result;
    for (auto it = npcKnownFacts.rbegin(); it != npcKnownFacts.rend(); ++it)
    {
        if (it->npcId != npcId) continue;
        result.push_back(*it);
        if (limit > 0 && static_cast<int>(result.size()) >= limit) break;
    }
    return result;
}

bool Player::npcKnowsFact(const std::string& npcId, const std::string& factType, const std::string& subjectId) const
{
    return std::any_of(npcKnownFacts.begin(), npcKnownFacts.end(), [&](const NpcKnownFact& fact) {
        return fact.npcId == npcId && fact.factType == factType && fact.subjectId == subjectId;
    });
}

void Player::setLoadedNpcKnownFacts(const std::vector<NpcKnownFact>& facts)
{
    npcKnownFacts.clear();
    for (NpcKnownFact fact : facts)
    {
        if (fact.npcId.empty() || fact.factType.empty() || fact.subjectId.empty()) continue;
        fact.confidence = clampConfidence(fact.confidence);
        fact.evidenceLevel = clampEvidence(fact.evidenceLevel);
        fact.firstLearnedDay = std::max(0, fact.firstLearnedDay);
        fact.lastReinforcedDay = std::max(fact.firstLearnedDay, fact.lastReinforcedDay);
        fact.timesHeard = std::max(1, fact.timesHeard);
        if (fact.claimVariant.empty()) fact.claimVariant = "default";
        npcKnownFacts.push_back(fact);
    }
}
