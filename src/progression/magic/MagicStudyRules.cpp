#include "progression/magic/MagicStudyRules.hpp"

#include "entity/Player.hpp"
#include "progression/TechniqueProgressionRules.hpp"
#include "progression/language/LanguageSystem.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalizeStudyText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    const std::vector<MagicStudyRequirement>& studyRequirements()
    {
        static const std::vector<MagicStudyRequirement> requirements = {
            {"basic_magic_manual", "learned_arcane_mark", "Marque élémentaire étudiée", 8, 45, 11, "", 0},
            {"arcane_binding_grimoire", "learned_arcane_binding", "Entrave arcanique étudiée", 15, 60, 12, "", 0},
            {"elemental_ward_grimoire", "learned_elemental_ward", "Voile élémentaire étudié", 25, 75, 14, "", 0},
            {"frost_needle_grimoire", "learned_frost_needle", "Aiguille de givre étudiée", 20, 70, 14, "nain", 2},
            {"mana_suture_grimoire", "learned_mana_suture", "Suture de mana étudiée", 30, 90, 16, "celeste", 2},
            {"occult_bramble_grimoire", "learned_occult_bramble", "Ronces occultes étudiées", 25, 80, 15, "elfique_noir", 2},
            {"resistance_rift_grimoire", "learned_resistance_rift", "Faille de résistance étudiée", 55, 120, 20, "anormal", 1}
        };
        return requirements;
    }

    const std::vector<MagicScrollRequirement>& scrollRequirements()
    {
        // Scrolls carry most of the formula themselves, so their barriers are lower than grimoires.
        // Advanced/foreign scripts still demand enough magical literacy to avoid wasting the paper.
        static const std::vector<MagicScrollRequirement> requirements = {
            {"étincelle arcanique", 1, 15, 8, "", 0},
            {"voile élémentaire", 5, 20, 9, "", 0},
            {"faille fragile", 25, 45, 14, "anormal", 1},
            {"braise errante", 8, 25, 10, "kitsune", 1},
            {"purification mineure", 3, 15, 8, "", 0},
            {"venin rampant", 10, 25, 10, "", 0},
            {"pas de verre", 12, 25, 11, "", 0},
            {"ronce-lien", 12, 25, 11, "elfique", 1}
        };
        return requirements;
    }

    bool hasSpecializedMagicPath(const Player& player)
    {
        return player.getMagicChannelingStyleLabel() != "Réserve mineure";
    }
}

namespace MagicStudyRules
{
    const MagicStudyRequirement* findStudyRequirement(const std::string& itemId)
    {
        for (const MagicStudyRequirement& requirement : studyRequirements())
        {
            if (requirement.itemId == itemId)
            {
                return &requirement;
            }
        }
        return nullptr;
    }

    bool isMagicStudyItem(const std::string& itemId)
    {
        return findStudyRequirement(itemId) != nullptr;
    }

    int magicKnowledgeScore(const Player& player)
    {
        const int mentalBase = std::max(1, (player.getAttributes().getIntelligence() + player.getAttributes().getWisdom()) / 2);
        int score = mentalBase;

        if (hasSpecializedMagicPath(player))
        {
            score += 3;
        }

        int studiedFormulae = 0;
        for (const std::string& skillId : player.getUnlockedActiveSkills())
        {
            if (skillId.rfind("learned_", 0) == 0)
            {
                ++studiedFormulae;
            }
        }
        score += std::min(10, studiedFormulae * 2);

        if (player.hasPassiveSkill("cautious_channeling") || player.hasPassiveSkill("arcane_channel_mastery"))
        {
            ++score;
        }

        return score;
    }

    std::string magicKnowledgeLabel(int score)
    {
        if (score >= 22) return "érudit";
        if (score >= 18) return "avancé";
        if (score >= 14) return "confirmé";
        if (score >= 10) return "notions solides";
        return "rudimentaire";
    }

    std::vector<std::string> studyBlockers(const Player& player, const MagicStudyRequirement& requirement)
    {
        std::vector<std::string> blockers;

        if (!hasSpecializedMagicPath(player))
        {
            blockers.push_back("voie magique insuffisante : le texte peut être lu, mais pas stabilisé comme technique personnelle");
        }
        if (player.getLevel() < requirement.minimumLevel)
        {
            blockers.push_back("niveau " + std::to_string(requirement.minimumLevel) + " requis (actuel : " + std::to_string(player.getLevel()) + ")");
        }
        if (player.getMaxMana() < requirement.minimumMaxMana)
        {
            blockers.push_back("réserve maximale de " + std::to_string(requirement.minimumMaxMana) + " mana requise (actuelle : " + std::to_string(player.getMaxMana()) + ")");
        }

        const int knowledge = magicKnowledgeScore(player);
        if (knowledge < requirement.minimumKnowledge)
        {
            blockers.push_back("connaissances magiques " + std::to_string(requirement.minimumKnowledge) + " requises (actuelles : " + std::to_string(knowledge) + ", " + magicKnowledgeLabel(knowledge) + ")");
        }

        if (!requirement.languageId.empty()
            && !LanguageSystem::understands(player, requirement.languageId, requirement.minimumLanguageLevel))
        {
            blockers.push_back(
                "langue : " + LanguageSystem::displayName(requirement.languageId)
                + " niveau " + LanguageSystem::knowledgeLabel(requirement.minimumLanguageLevel)
                + " requis (actuel : " + LanguageSystem::knowledgeLabel(LanguageSystem::getKnowledgeLevel(player, requirement.languageId)) + ")"
            );
        }

        return blockers;
    }

    std::vector<std::string> studyRequirementLines(const Player& player, const std::string& itemId)
    {
        std::vector<std::string> lines;
        const MagicStudyRequirement* requirement = findStudyRequirement(itemId);
        if (requirement == nullptr)
        {
            return lines;
        }

        const int knowledge = magicKnowledgeScore(player);
        const auto status = [](bool ok) { return std::string(ok ? "[OK] " : "[À ATTEINDRE] "); };
        lines.push_back("Prérequis pour transformer ce texte en technique personnelle :");
        lines.push_back(status(hasSpecializedMagicPath(player)) + "voie magique spécialisée");
        lines.push_back(status(player.getLevel() >= requirement->minimumLevel)
            + "niveau " + std::to_string(requirement->minimumLevel)
            + " (actuel : " + std::to_string(player.getLevel()) + ")");
        lines.push_back(status(player.getMaxMana() >= requirement->minimumMaxMana)
            + "mana maximal " + std::to_string(requirement->minimumMaxMana)
            + " (actuel : " + std::to_string(player.getMaxMana()) + ")");
        lines.push_back(status(knowledge >= requirement->minimumKnowledge)
            + "connaissances magiques " + std::to_string(requirement->minimumKnowledge)
            + " (actuelles : " + std::to_string(knowledge) + ", " + magicKnowledgeLabel(knowledge) + ")");
        if (!requirement->languageId.empty())
        {
            const int currentLanguageLevel = LanguageSystem::getKnowledgeLevel(player, requirement->languageId);
            const bool languageOk = LanguageSystem::understands(player, requirement->languageId, requirement->minimumLanguageLevel);
            lines.push_back(status(languageOk)
                + LanguageSystem::displayName(requirement->languageId) + " : "
                + LanguageSystem::knowledgeLabel(requirement->minimumLanguageLevel) + " requis"
                + " (actuel : " + LanguageSystem::knowledgeLabel(currentLanguageLevel) + ")");
        }
        lines.push_back("Le livre n'est pas consommé si ces prérequis ne sont pas encore réunis.");
        return lines;
    }

    MagicStudyOutcome attemptStudy(Player& player, const std::string& itemId, std::vector<std::string>& notes)
    {
        notes.clear();
        const MagicStudyRequirement* requirement = findStudyRequirement(itemId);
        if (requirement == nullptr)
        {
            return MagicStudyOutcome::NotApplicable;
        }

        const int canonicalLevel = TechniqueProgressionRules::magicStudyUnlockLevel(requirement->skillId);
        if (canonicalLevel != requirement->minimumLevel)
        {
            notes.push_back("Le grimoire semble incohérent avec les règles de progression et refuse de se stabiliser.");
            return MagicStudyOutcome::Blocked;
        }

        if (player.isActiveSkillUnlocked(requirement->skillId))
        {
            notes.push_back("Formule déjà comprise : " + requirement->skillName + ".");
            notes.push_back("La relecture sert surtout à consolider les annotations déjà connues.");
            return MagicStudyOutcome::AlreadyKnown;
        }

        const std::vector<std::string> blockers = studyBlockers(player, *requirement);
        if (!blockers.empty())
        {
            notes.push_back("Étude incomplète : tu possèdes le texte, mais tu ne peux pas encore en faire une technique fiable.");
            for (const std::string& blocker : blockers)
            {
                notes.push_back("- " + blocker + ".");
            }
            notes.push_back("Le livre reste dans l'inventaire : tu pourras le relire plus tard sans le racheter.");
            return MagicStudyOutcome::Blocked;
        }

        player.unlockActiveSkill(requirement->skillId, requirement->skillName);
        notes.push_back("Étude réussie : " + requirement->skillName + ".");
        notes.push_back(
            "Compréhension magique : " + std::to_string(magicKnowledgeScore(player))
            + " (" + magicKnowledgeLabel(magicKnowledgeScore(player)) + ")."
        );
        if (!requirement->languageId.empty())
        {
            notes.push_back("Le passage en " + LanguageSystem::displayName(requirement->languageId) + " a été compris correctement.");
        }
        return MagicStudyOutcome::Learned;
    }

    const MagicScrollRequirement* findScrollRequirement(const std::string& scrollName)
    {
        const std::string normalized = normalizeStudyText(scrollName);
        for (const MagicScrollRequirement& requirement : scrollRequirements())
        {
            if (normalized.find(normalizeStudyText(requirement.nameFragment)) != std::string::npos)
            {
                return &requirement;
            }
        }
        return nullptr;
    }

    std::vector<std::string> scrollUseBlockers(const Player& player, const std::string& scrollName)
    {
        std::vector<std::string> blockers;
        const MagicScrollRequirement* requirement = findScrollRequirement(scrollName);
        if (requirement == nullptr)
        {
            return blockers;
        }

        if (player.getLevel() < requirement->minimumLevel)
        {
            blockers.push_back("niveau " + std::to_string(requirement->minimumLevel) + " requis (actuel : " + std::to_string(player.getLevel()) + ")");
        }
        if (player.getMaxMana() < requirement->minimumMaxMana)
        {
            blockers.push_back("réserve maximale de " + std::to_string(requirement->minimumMaxMana) + " mana requise (actuelle : " + std::to_string(player.getMaxMana()) + ")");
        }

        const int knowledge = magicKnowledgeScore(player);
        if (knowledge < requirement->minimumKnowledge)
        {
            blockers.push_back("connaissances magiques " + std::to_string(requirement->minimumKnowledge) + " requises (actuelles : " + std::to_string(knowledge) + ")");
        }

        if (!requirement->languageId.empty()
            && !LanguageSystem::understands(player, requirement->languageId, requirement->minimumLanguageLevel))
        {
            blockers.push_back(
                "lecture du tracé en " + LanguageSystem::displayName(requirement->languageId)
                + " insuffisante (" + LanguageSystem::knowledgeLabel(requirement->minimumLanguageLevel) + " requis)"
            );
        }
        return blockers;
    }

    bool canUseScroll(const Player& player, const std::string& scrollName)
    {
        return scrollUseBlockers(player, scrollName).empty();
    }
}
