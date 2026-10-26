#include "story/HestiaMissingPersonQuestSystem.hpp"

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "entity/Race.hpp"
#include "story/HestiaArcSystem.hpp"

#include <algorithm>
#include <cstdint>

namespace
{
    std::uint32_t stableHash(const std::string& text)
    {
        std::uint32_t value = 2166136261u;
        for (unsigned char c : text)
        {
            value ^= static_cast<std::uint32_t>(c);
            value *= 16777619u;
        }
        return value;
    }

    Quest* findQuest(Player& player)
    {
        for (Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == HestiaMissingPersonQuestSystem::QuestId)
                return &quest;
        }
        return nullptr;
    }

    int scaledLevel(const Player& player)
    {
        return std::clamp(player.getLevel(), 5, 18);
    }
}

int HestiaMissingPersonQuestSystem::completedQuestCount(const Player& player)
{
    int count = 0;
    for (const Quest& quest : player.getQuestLog().getQuests())
    {
        if (quest.turnedIn && !quest.failed && quest.id != QuestId)
            ++count;
    }
    return count;
}

HestiaMissingPersonAvailability HestiaMissingPersonQuestSystem::availability(const Player& player)
{
    HestiaMissingPersonAvailability out;
    out.completedQuestCount = completedQuestCount(player);

    if (!player.hasStoryModeStarted())
    {
        out.reason = "L'histoire principale doit avoir réellement commencé.";
        return out;
    }
    if (player.getHestiaStoryState().rescued)
    {
        out.reason = "La personne disparue a déjà été retrouvée.";
        return out;
    }
    if (questExists(player))
    {
        out.available = true;
        out.reason = "La disparition est déjà inscrite dans le journal.";
        return out;
    }
    if (out.completedQuestCount < RequiredCompletedQuests)
    {
        out.reason = "Cette disparition ne remonte pas encore jusqu'au joueur : "
            + std::to_string(out.completedQuestCount) + "/" + std::to_string(RequiredCompletedQuests)
            + " quêtes validées.";
        return out;
    }

    out.available = true;
    out.reason = "Après assez de services rendus, Diane confie enfin une disparition plus ancienne et mal classée.";
    return out;
}

bool HestiaMissingPersonQuestSystem::questExists(const Player& player)
{
    for (const Quest& quest : player.getQuestLog().getQuests())
        if (quest.id == QuestId) return true;
    return false;
}

bool HestiaMissingPersonQuestSystem::questActive(const Player& player)
{
    for (const Quest& quest : player.getQuestLog().getQuests())
    {
        if (quest.id == QuestId)
            return quest.accepted && !quest.completed && !quest.turnedIn && !quest.failed;
    }
    return false;
}

Quest HestiaMissingPersonQuestSystem::createQuest(const Player& player)
{
    Quest quest;
    quest.id = QuestId;
    quest.rank = player.getLevel() >= 12 ? "B" : "C";
    quest.title = "Une personne disparue";
    quest.origin = "Quête principale";
    quest.client = "Diane";
    quest.location = "Quartier abandonné";
    quest.objective = "Retrouver une personne portée disparue après qu'une ancienne piste de la guilde a refait surface.";
    quest.objectiveType = "exploration";
    quest.targetFamily = "Disparition / sous-sol abandonné";
    quest.rewardExperience = 0;
    quest.rewardGold = 0;
    quest.rewardNote = "La vraie conséquence est narrative : retrouver la personne et décider où elle sera mise en sécurité.";
    quest.progress = 0;
    quest.target = 1;
    quest.guildQuest = false;
    quest.accepted = true;
    quest.expiresAtDay = -1;
    quest.hideFutureSteps = true;
    return quest;
}

bool HestiaMissingPersonQuestSystem::syncQuestIfAvailable(Player& player)
{
    const HestiaMissingPersonAvailability state = availability(player);
    if (!state.available || questExists(player) || player.getHestiaStoryState().rescued)
        return false;

    Quest quest = createQuest(player);
    if (!player.getQuestLog().addQuest(quest))
        return false;

    player.recordCanonicalEvent(
        "hestia_arc",
        "missing_person_report",
        "Diane a confié une ancienne disparition sans révéler l'identité réelle ni l'importance de la personne recherchée",
        1
    );
    return true;
}

bool HestiaMissingPersonQuestSystem::usesSkeletonVariant(const Player& player)
{
    // Stable per character without adding a save-only flag. The player never sees this calculation.
    return (stableHash(player.getName() + "|hestia_missing_person") & 1u) != 0u;
}

std::vector<Monster> HestiaMissingPersonQuestSystem::buildDefenderWave(const Player& player)
{
    std::vector<Monster> monsters;
    const int level = scaledLevel(player);

    if (usesSkeletonVariant(player))
    {
        const int minions = std::clamp(2 + player.getLevel() / 7, 3, 5);
        for (int i = 0; i < minions; ++i)
        {
            monsters.emplace_back(
                "Squelette de cave " + std::to_string(i + 1),
                "Mort-vivant de sous-sol",
                Race::MortVivant,
                std::max(4, level - 1),
                58 + level * 8,
                5 + level,
                10 + level * 2,
                15 + level * 2,
                0,
                0
            );
        }
        monsters.emplace_back(
            "Squelette suicidaire explosif",
            "Mort-vivant explosif toxique et hallucinogène — crâne piégé",
            Race::MortVivant,
            level + 2,
            82 + level * 11,
            7 + level,
            14 + level * 2,
            22 + level * 3,
            0,
            1,
            false,
            true,
            false,
            true
        );
    }
    else
    {
        const int minions = std::clamp(2 + player.getLevel() / 7, 3, 5);
        for (int i = 0; i < minions; ++i)
        {
            monsters.emplace_back(
                "Gobelin de cave " + std::to_string(i + 1),
                "Gobelin de bande",
                Race::Gobelin,
                std::max(4, level - 1),
                52 + level * 7,
                5 + level,
                10 + level * 2,
                15 + level * 2,
                1,
                0
            );
        }
        monsters.emplace_back(
            "Gobelin ninja du sous-sol",
            "Gobelin ninja expérimenté, rapide et chargé de couper la sortie",
            Race::Gobelin,
            level + 2,
            78 + level * 10,
            8 + level,
            15 + level * 2,
            23 + level * 3,
            1,
            1,
            false,
            true,
            true,
            true
        );
    }

    return monsters;
}

int HestiaMissingPersonQuestSystem::stealthChancePercent(const Player& player)
{
    // Deliberately difficult: stealth is a valid alternative, not the free solution.
    int chance = 18;
    chance += std::clamp(player.getAttributes().getDexterity(), 0, 30);
    chance += std::clamp(player.getLevel() / 2, 0, 12);
    if (player.hasTitle("Aventurier")) chance += 4;
    if (usesSkeletonVariant(player)) chance -= 5; // silent corridors, but fewer living distractions.
    return std::clamp(chance, 18, 62);
}

HestiaStealthAttempt HestiaMissingPersonQuestSystem::attemptStealth(Player& player, Random& random)
{
    HestiaStealthAttempt out;
    out.chancePercent = stealthChancePercent(player);
    out.roll = random.between(1, 100);
    out.success = out.roll <= out.chancePercent;

    out.lines.push_back("Tu cherches d'abord les angles morts, les portes secondaires et les moments où la cave se tait.");
    out.lines.push_back("Discrétion estimée : " + std::to_string(out.chancePercent) + " %.");
    if (out.success)
    {
        out.lines.push_back("Tu passes sans déclencher l'alerte générale et atteins la pièce fermée avant que les gardes comprennent ce qui manque.");
        player.recordCanonicalEvent("hestia_arc", "rescue_stealth", "Sauvetage de la personne disparue par infiltration", 1);
    }
    else
    {
        out.lines.push_back("Un bruit, une silhouette ou un mauvais timing suffit : l'alerte part avant que tu atteignes la pièce.");
        player.recordCanonicalEvent("hestia_arc", "stealth_failed", "Infiltration du sous-sol détectée", 1);
    }
    return out;
}

bool HestiaMissingPersonQuestSystem::completeRescue(Player& player, const std::string& initialResidence, bool playerHasHome)
{
    if (!HestiaArcSystem::completeMainStoryRescue(player, initialResidence, playerHasHome))
        return false;

    Quest* quest = findQuest(player);
    if (quest != nullptr)
    {
        quest->progress = std::max(1, quest->target);
        quest->completed = true;
        quest->turnedIn = true;
        quest->failed = false;
        quest->expiresAtDay = -1;
    }
    player.getQuestLog().refreshLinkedQuestProgress();
    return true;
}

std::vector<std::string> HestiaMissingPersonQuestSystem::introLines(const Player& player)
{
    const int done = completedQuestCount(player);
    return {
        "Après " + std::to_string(done) + " quêtes validées, Diane ressort un vieux dossier qui n'avait jamais mené nulle part.",
        "La fiche ne parle ni de divinité, ni de lumière, ni de destin : seulement d'une personne disparue et d'une piste récemment revenue du Quartier abandonné.",
        "Une cave condamnée semble avoir été rouverte. Quelqu'un ou quelque chose l'occupe encore."
    };
}

std::vector<std::string> HestiaMissingPersonQuestSystem::approachLines(const Player& player)
{
    const HestiaRescueScenario scenario = HestiaArcSystem::rescueScenario(usesSkeletonVariant(player));
    std::vector<std::string> lines = {
        "Présence repérée : " + scenario.defenders + ".",
        "Menace particulière : " + scenario.eliteThreat + ".",
        "Deux approches restent crédibles : entrer de force ou tenter une infiltration difficile."
    };
    lines.insert(lines.end(), scenario.warnings.begin(), scenario.warnings.end());
    return lines;
}
