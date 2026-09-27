// EN: Split from Player.cpp to keep Player responsibilities maintainable.
// FR: Extrait de Player.cpp afin de garder des responsabilités Player maintenables.
#include "entity/Player.hpp"
#include "entity/player/PlayerUiSupport.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "core/VersionInfo.hpp"
#include "economy/Money.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "progression/TitleCatalog.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

void Player::beginChallengeCombatTracking()
{
    challengeCombatTrackingActive = true;
    challengeCombatConsumablesUsed = 0;
    challengeCombatSkillsUsed = 0;
    challengeCombatNonBasicAttacksUsed = 0;
    challengeCombatBasicAttacksUsed = 0;
    challengeCombatDefenseTurns = 0;
    challengeCombatTurnsTaken = 0;
    challengeCombatDamageTaken = 0;
    challengeCombatSummonActions = 0;
    challengeCombatPartySize = 1;
    challengeCombatAlivePartyCount = 1;
}

void Player::recordChallengeCombatAction(const std::string& actionKind)
{
    if (!challengeCombatTrackingActive) return;

    if (actionKind != "summon_attack" && actionKind != "summon_skill")
    {
        ++challengeCombatTurnsTaken;
    }

    if (actionKind == "consumable" || actionKind == "ally_consumable")
    {
        ++challengeCombatConsumablesUsed;
    }
    else if (actionKind == "skill")
    {
        ++challengeCombatSkillsUsed;
        ++challengeCombatNonBasicAttacksUsed;
    }
    else if (actionKind == "special_attack" || actionKind == "tactical_action" || actionKind == "summon_attack" || actionKind == "summon_skill")
    {
        ++challengeCombatNonBasicAttacksUsed;
        if (actionKind == "summon_skill" || actionKind == "tactical_action") ++challengeCombatSkillsUsed;
    }
    else if (actionKind == "basic_attack")
    {
        ++challengeCombatBasicAttacksUsed;
    }
    else if (actionKind == "defense")
    {
        ++challengeCombatDefenseTurns;
    }
}

void Player::recordChallengeSummonAction(int damageDone)
{
    if (!challengeCombatTrackingActive || damageDone <= 0) return;
    ++challengeCombatSummonActions;
    recordChallengeCombatAction("summon_attack");
}

void Player::applyChallengeCombatGroupSummary(
    int partySize,
    int alivePartyCount,
    int groupConsumablesUsed,
    int groupSkillsUsed,
    int groupNonBasicAttacksUsed,
    int groupBasicAttacksUsed,
    int groupDamageTaken,
    int groupSummonActions
)
{
    if (!challengeCombatTrackingActive) return;
    challengeCombatPartySize = std::max(1, partySize);
    challengeCombatAlivePartyCount = std::max(0, alivePartyCount);
    challengeCombatConsumablesUsed = std::max(0, groupConsumablesUsed);
    challengeCombatSkillsUsed = std::max(0, groupSkillsUsed);
    challengeCombatNonBasicAttacksUsed = std::max(0, groupNonBasicAttacksUsed);
    challengeCombatBasicAttacksUsed = std::max(0, groupBasicAttacksUsed);
    challengeCombatDamageTaken = std::max(0, groupDamageTaken);
    challengeCombatSummonActions = std::max(0, groupSummonActions);
}

int Player::getChallengeCombatConsumablesUsed() const { return challengeCombatConsumablesUsed; }
int Player::getChallengeCombatSkillsUsed() const { return challengeCombatSkillsUsed; }
int Player::getChallengeCombatNonBasicAttacksUsed() const { return challengeCombatNonBasicAttacksUsed; }
int Player::getChallengeCombatBasicAttacksUsed() const { return challengeCombatBasicAttacksUsed; }
int Player::getChallengeCombatDamageTaken() const { return challengeCombatDamageTaken; }
int Player::getChallengeCombatSummonActions() const { return challengeCombatSummonActions; }

bool Player::isChallengeCombatTrackingActive() const
{
    return challengeCombatTrackingActive;
}

void Player::finishChallengeCombatTracking(bool victory, bool bossFight, bool eliteFight, int defeatedEnemyCount)
{
    if (!challengeCombatTrackingActive)
    {
        return;
    }

    challengeCombatTrackingActive = false;
    if (!victory)
    {
        return;
    }

    std::vector<std::string> completedChallenges;
    std::vector<std::string> completedTitles;
    std::vector<std::string> heroCompletedQuests;
    std::vector<std::string> heroCompletedTitles;
    int heroExperienceReward = 0;
    int heroGoldReward = 0;
    int heroMarkReward = 0;

    for (Quest& quest : questLog.getQuests())
    {
        const bool heroChallenge = quest.origin == "Défi du Hero Villager";
        if ((!quest.guildChallenge && !heroChallenge)
            || !quest.accepted
            || quest.completed
            || quest.turnedIn
            || quest.failed)
        {
            continue;
        }

        bool success = false;
        int progressAmount = 0;
        if (quest.challengeCondition == "three_clean_victories" && challengeCombatConsumablesUsed == 0)
        {
            progressAmount = 1;
        }
        else if (quest.challengeCondition == "defend_then_win" && challengeCombatDefenseTurns >= 3)
        {
            success = true;
        }
        else if (quest.challengeCondition == "low_hp_victory" && getMaxHp() > 0 && getHp() * 100 <= getMaxHp() * 25)
        {
            success = true;
        }
        else if (quest.challengeCondition == "cursed_victory" && getActiveCurseCount() >= 1)
        {
            progressAmount = 1;
        }
        else if (quest.challengeCondition == "no_damage_victory" && challengeCombatDamageTaken == 0)
        {
            success = true;
        }
        else if (quest.challengeCondition == "basic_only_victory"
            && challengeCombatBasicAttacksUsed > 0
            && challengeCombatNonBasicAttacksUsed == 0
            && challengeCombatConsumablesUsed == 0)
        {
            success = true;
        }
        else if (quest.challengeCondition == "elite_no_consumable" && eliteFight && challengeCombatConsumablesUsed == 0)
        {
            success = true;
        }
        else if (quest.challengeCondition == "long_fight_victory" && challengeCombatTurnsTaken >= 6)
        {
            success = true;
        }
        else if (quest.challengeCondition == "boss_no_consumable" && bossFight && challengeCombatConsumablesUsed == 0)
        {
            success = true;
        }
        else if (quest.challengeCondition == "boss_no_consumable_skill"
            && bossFight
            && challengeCombatConsumablesUsed == 0
            && challengeCombatSkillsUsed == 0
            && challengeCombatNonBasicAttacksUsed == 0)
        {
            success = true;
        }
        else if (quest.challengeCondition == "triple_curse_victory" && getActiveCurseCount() >= 3)
        {
            success = true;
        }
        else if (quest.challengeCondition == "six_creatures")
        {
            progressAmount = std::max(1, defeatedEnemyCount);
        }
        else if (quest.challengeCondition == "group_all_survive"
            && challengeCombatPartySize >= 2
            && challengeCombatAlivePartyCount >= challengeCombatPartySize)
        {
            success = true;
        }
        else if (quest.challengeCondition == "group_basic_only"
            && challengeCombatPartySize >= 2
            && challengeCombatBasicAttacksUsed > 0
            && challengeCombatNonBasicAttacksUsed == 0
            && challengeCombatConsumablesUsed == 0)
        {
            success = true;
        }
        else if (quest.challengeCondition == "summon_support_victory"
            && challengeCombatSummonActions >= 1)
        {
            success = true;
        }
        else if (quest.challengeCondition == "group_no_consumable"
            && challengeCombatPartySize >= 2
            && challengeCombatConsumablesUsed == 0)
        {
            success = true;
        }

        if (success)
        {
            quest.progress = quest.target;
            quest.completed = true;
        }
        else if (progressAmount > 0)
        {
            quest.progress = std::min(quest.target, quest.progress + progressAmount);
            quest.completed = quest.progress >= quest.target;
        }

        if (!quest.completed)
        {
            continue;
        }

        std::string titleName;
        if (quest.challengeCondition == "boss_no_consumable_skill") titleName = "Seulement toi et le boss";
        else if (quest.challengeCondition == "boss_no_consumable") titleName = "Boss sans réserve";
        else if (quest.challengeCondition == "basic_only_victory") titleName = "À l'ancienne";
        else if (quest.challengeCondition == "no_damage_victory") titleName = "Pas une égratignure";
        else if (quest.challengeCondition == "triple_curse_victory") titleName = "Le quatrième problème";
        else if (quest.challengeCondition == "low_hp_victory") titleName = "Encore debout, malheureusement";
        else if (quest.challengeCondition == "three_clean_victories") titleName = "Trois victoires, zéro gorgée";
        else if (quest.challengeCondition == "defend_then_win") titleName = "Le mur qui répond";
        else if (quest.challengeCondition == "cursed_victory") titleName = "Victoire sous mauvaise influence";
        else if (quest.challengeCondition == "elite_no_consumable") titleName = "Élite sans fiole";
        else if (quest.challengeCondition == "long_fight_victory") titleName = "Ça devait être rapide";
        else if (quest.challengeCondition == "six_creatures") titleName = "Six problèmes de moins";
        else if (quest.challengeCondition == "group_all_survive") titleName = "Personne ne reste derrière";
        else if (quest.challengeCondition == "group_basic_only") titleName = "Escouade à l'ancienne";
        else if (quest.challengeCondition == "summon_support_victory") titleName = "Le groupe compte aussi les invoqués";
        else if (quest.challengeCondition == "group_no_consumable") titleName = "Groupe sans réserve";

        if (heroChallenge)
        {
            quest.turnedIn = true;
            heroExperienceReward += std::max(0, quest.rewardExperience);
            heroGoldReward += std::max(0, quest.rewardGold);
            heroMarkReward += std::max(0, quest.rewardMaterialQuantity);
            if (quest.challengeMarkReward > 0)
            {
                heroMarkReward = std::max(heroMarkReward, quest.challengeMarkReward);
            }
            heroCompletedQuests.push_back(quest.title);
            if (!titleName.empty() && grantTitle(titleName))
            {
                heroCompletedTitles.push_back(titleName);
            }
            continue;
        }

        completedChallenges.push_back(quest.title);

        if (!titleName.empty() && grantTitle(titleName))
        {
            completedTitles.push_back(titleName);
        }
    }

    if (!completedChallenges.empty())
    {
        std::vector<std::string> lines = {"Un ou plusieurs défis de guilde viennent d'être accomplis pendant ce combat."};
        for (const std::string& challengeName : completedChallenges)
        {
            lines.push_back("Défi accompli : " + challengeName + ".");
        }
        for (const std::string& titleName : completedTitles)
        {
            lines.push_back("Titre obtenu : " + titleName + ".");
        }
        lines.push_back("Les marques et les autres récompenses restent à récupérer auprès de la guilde.");
        PlayerUiSupport::showPlayerScreen("DÉFI RÉUSSI", "challenge.combat.completed", lines, false);
    }

    if (!heroCompletedQuests.empty())
    {
        if (heroExperienceReward > 0)
        {
            gainExperience(heroExperienceReward);
        }
        if (heroGoldReward > 0)
        {
            inventory.earnGold(heroGoldReward);
        }
        if (heroMarkReward > 0)
        {
            inventory.addMaterial(MaterialCatalog::createById("guild_challenge_mark", heroMarkReward));
        }

        std::vector<std::string> lines = {
            "L'air se plie brièvement derrière toi, comme si une silhouette avait attendu juste hors du regard.",
            "Le Hero Villager apparaît sans bruit, son armure de diamant bleu traversée par un éclat presque irréel."
        };
        for (const std::string& questName : heroCompletedQuests)
        {
            lines.push_back("Défi validé sur place : " + questName + ".");
        }
        lines.push_back("Hmmm... J'ai vu. Tu as réussi. La guilde n'a pas besoin de tamponner ce qui est déjà évident... Huuuh.");
        if (heroExperienceReward > 0) lines.push_back("Expérience reçue : " + std::to_string(heroExperienceReward) + ".");
        if (heroGoldReward > 0) lines.push_back("Récompense reçue : " + Money::formatGoldWithRaw(heroGoldReward) + ".");
        if (heroMarkReward > 0) lines.push_back("Marques de défi reçues : " + std::to_string(heroMarkReward) + ".");
        for (const std::string& titleName : heroCompletedTitles)
        {
            lines.push_back("Titre obtenu : " + titleName + ".");
        }
        lines.push_back("Avant même que tu répondes, sa silhouette se fragmente en carrés bleutés puis disparaît comme un mirage mal fixé.");
        PlayerUiSupport::showPlayerScreen("VALIDATION IMPOSSIBLE À EXPLIQUER", "challenge.hero.auto_turn_in", lines, false);
    }
}

