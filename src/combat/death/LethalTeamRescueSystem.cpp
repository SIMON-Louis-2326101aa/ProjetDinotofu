#include "combat/death/LethalTeamRescueSystem.hpp"

#include "core/Random.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

namespace
{
    std::string lowerCopy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& normalized, std::initializer_list<const char*> tokens)
    {
        for (const char* token : tokens)
        {
            if (normalized.find(token) != std::string::npos) return true;
        }
        return false;
    }

    bool recruitCanPerformLethalRescue(const RecruitedAllyCombatSupport& ally)
    {
        if (ally.currentHp <= 0) return false;
        const std::string profile = lowerCopy(ally.job + " " + ally.trait + " " + ally.motivation);
        return containsAny(profile, {
            "soigneur", "guerisseur", "guérisseur", "medecin", "médecin", "infirmier",
            "pretre", "prêtre", "clerc", "druide", "shaman", "chaman", "aumonier", "aumônier",
            "mage d'appui", "lumomancien", "herboriste", "herboriste de combat"
        });
    }

    int recruitRescueStrength(const RecruitedAllyCombatSupport& ally)
    {
        int strength = 0;
        const std::string profile = lowerCopy(ally.job + " " + ally.trait + " " + ally.motivation);
        if (containsAny(profile, {"medecin", "médecin", "infirmier", "soigneur", "guerisseur", "guérisseur"})) strength += 8;
        else if (containsAny(profile, {"pretre", "prêtre", "clerc", "druide", "shaman", "chaman", "aumonier", "aumônier", "mage d'appui", "lumomancien"})) strength += 6;
        else if (containsAny(profile, {"herboriste"})) strength += 4;

        // La maîtrise létale dépend réellement de l'expérience du soigneur, pas seulement de son métier.
        strength += std::max(0, ally.currentRankIndex) * 2;
        strength += RecruitedAllyCombatRoster::maturityTier(ally) * 2;
        strength += std::clamp(ally.supportKitQuality, 0, 10);
        strength += std::max(0, ally.estimatedLevel) / 12;
        return strength;
    }

    std::vector<const RecruitedAllyCombatSupport*> uniqueLivingRecruits(
        const std::vector<RecruitedAllyCombatSupport>& active,
        const std::vector<RecruitedAllyCombatSupport>& reserve)
    {
        std::vector<const RecruitedAllyCombatSupport*> result;
        std::unordered_set<std::string> seen;
        auto append = [&](const std::vector<RecruitedAllyCombatSupport>& source) {
            for (const auto& ally : source)
            {
                if (ally.name.empty() || ally.currentHp <= 0 || !seen.insert(ally.name).second) continue;
                result.push_back(&ally);
            }
        };
        append(active);
        append(reserve);
        return result;
    }

    std::string rescueEventLabel(const LethalTeamRescueResult& result, const std::string& contextId)
    {
        std::ostringstream label;
        label << "contexte=" << contextId
              << " | sauveteur=" << (result.rescuerName.empty() ? "aucun" : result.rescuerName)
              << " | methode=" << result.rescueMethod
              << " | chance=" << result.survivalChancePercent
              << " | jet=" << result.roll;
        return label.str();
    }
}

LethalTeamRescueAssessment LethalTeamRescueSystem::assessRecruitedTeam(
    const std::vector<RecruitedAllyCombatSupport>& activeAllies,
    const std::vector<RecruitedAllyCombatSupport>& reserveAllies)
{
    LethalTeamRescueAssessment assessment;
    const auto living = uniqueLivingRecruits(activeAllies, reserveAllies);
    if (living.empty()) return assessment;

    assessment.teamPresent = true;

    const RecruitedAllyCombatSupport* best = nullptr;
    int bestStrength = -1;
    int capableCount = 0;
    for (const RecruitedAllyCombatSupport* ally : living)
    {
        if (!recruitCanPerformLethalRescue(*ally)) continue;
        ++capableCount;
        const int strength = recruitRescueStrength(*ally);
        if (strength > bestStrength)
        {
            bestStrength = strength;
            best = ally;
        }
    }

    if (best != nullptr)
    {
        assessment.hasCapableRescuer = true;
        assessment.rescuerName = best->name;
        assessment.survivalChancePercent = std::clamp(33 + bestStrength * 3 / 5 + std::max(0, capableCount - 1) * 2, 33, 66);
        assessment.revivedHealthPercent = std::clamp(8 + assessment.survivalChancePercent / 5, 12, 21);
        assessment.rescueMethod = "soin/reanimation de terrain";
        return assessment;
    }

    // Une équipe incapable de réanimer peut encore avoir une très faible chance de stabiliser,
    // transporter ou trouver un secours avant que la mort devienne irréversible.
    assessment.rescuerName = living.front()->name;
    assessment.survivalChancePercent = 15;
    assessment.revivedHealthPercent = 5;
    assessment.rescueMethod = "stabilisation/transport d'urgence";
    return assessment;
}

LethalTeamRescueResult LethalTeamRescueSystem::tryRecruitedTeamRescue(
    Player& player,
    const std::vector<RecruitedAllyCombatSupport>& activeAllies,
    const std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    const std::string& contextId)
{
    LethalTeamRescueResult result;
    static_cast<LethalTeamRescueAssessment&>(result) = assessRecruitedTeam(activeAllies, reserveAllies);
    if (!player.isDead() || !result.teamPresent || result.survivalChancePercent <= 0) return result;

    result.roll = random.between(1, 100);
    result.survived = result.roll <= result.survivalChancePercent;
    if (result.survived)
    {
        player.reviveWithHealthPercentage(result.revivedHealthPercent);
        if (player.getHp() <= 0) player.heal(1);
        player.recordCanonicalEvent(
            "survies_lethales_equipe",
            contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
            rescueEventLabel(result, contextId),
            1
        );
    }
    else
    {
        player.recordCanonicalEvent(
            "tentatives_sauvetage_lethal_equipe",
            contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
            rescueEventLabel(result, contextId),
            1
        );
    }
    return result;
}

bool LethalTeamRescueSystem::humanPlayerCanPerformLethalRescue(const Player& player)
{
    if (player.isDead()) return false;
    const std::string type = lowerCopy(player.getType());
    return containsAny(type, {
        "pretre", "prêtre", "clerc", "medecin", "médecin", "infirmier", "druide",
        "shaman", "chaman", "aumonier", "aumônier", "paladin", "lumomancien", "herboriste"
    });
}

int LethalTeamRescueSystem::humanPlayerRescueStrength(const Player& player)
{
    if (!humanPlayerCanPerformLethalRescue(player)) return 0;
    const std::string type = lowerCopy(player.getType());
    int strength = player.getLevel() / 10;
    if (containsAny(type, {"medecin", "médecin", "infirmier", "pretre", "prêtre", "clerc"})) strength += 8;
    else if (containsAny(type, {"druide", "shaman", "chaman", "aumonier", "aumônier", "lumomancien", "herboriste"})) strength += 6;
    else if (containsAny(type, {"paladin"})) strength += 5;
    return strength;
}

LethalTeamRescueResult LethalTeamRescueSystem::tryHumanPartyRescue(
    Player& downedPlayer,
    const std::vector<Player*>& party,
    Random& random,
    const std::string& contextId)
{
    LethalTeamRescueResult result;
    if (!downedPlayer.isDead()) return result;

    Player* fallback = nullptr;
    Player* best = nullptr;
    int bestStrength = -1;
    int capableCount = 0;
    for (Player* member : party)
    {
        if (member == nullptr || member == &downedPlayer || member->isDead()) continue;
        result.teamPresent = true;
        if (fallback == nullptr) fallback = member;
        if (!humanPlayerCanPerformLethalRescue(*member)) continue;
        ++capableCount;
        const int strength = humanPlayerRescueStrength(*member);
        if (strength > bestStrength)
        {
            bestStrength = strength;
            best = member;
        }
    }

    if (!result.teamPresent) return result;
    if (best != nullptr)
    {
        result.hasCapableRescuer = true;
        result.rescuerName = best->getName();
        result.survivalChancePercent = std::clamp(33 + bestStrength + std::max(0, capableCount - 1) * 2, 33, 66);
        result.revivedHealthPercent = std::clamp(8 + result.survivalChancePercent / 5, 12, 21);
        result.rescueMethod = "soin/reanimation par coequipier";
    }
    else
    {
        result.rescuerName = fallback != nullptr ? fallback->getName() : "un coequipier";
        result.survivalChancePercent = 15;
        result.revivedHealthPercent = 5;
        result.rescueMethod = "transport/stabilisation sans specialiste";
    }

    result.roll = random.between(1, 100);
    result.survived = result.roll <= result.survivalChancePercent;
    if (result.survived)
    {
        downedPlayer.reviveWithHealthPercentage(result.revivedHealthPercent);
        if (downedPlayer.getHp() <= 0) downedPlayer.heal(1);
        downedPlayer.recordCanonicalEvent(
            "survies_lethales_equipe",
            contextId + ":jour_" + std::to_string(downedPlayer.getWorldDaysElapsed()),
            rescueEventLabel(result, contextId),
            1
        );
    }
    else
    {
        downedPlayer.recordCanonicalEvent(
            "tentatives_sauvetage_lethal_equipe",
            contextId + ":jour_" + std::to_string(downedPlayer.getWorldDaysElapsed()),
            rescueEventLabel(result, contextId),
            1
        );
    }
    return result;
}
