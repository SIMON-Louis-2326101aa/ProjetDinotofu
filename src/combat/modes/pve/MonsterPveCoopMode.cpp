// EN: Cooperative PvE mode split from MonsterPveMode.cpp.
// FR: Mode PvE coopératif extrait de MonsterPveMode.cpp.
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "combat/modes/pve/PveWaveNarrativeSupport.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "combat/dialogue/EncounterDialogueSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/system/WaveCombatSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/InitiativeSystem.hpp"
#include "combat/group/TurnOrder.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/TurnManager.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/CombatDisplay.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "progression/blessing/BlessingSystem.hpp"
#include "combat/reward/CombatReward.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"
#include "core/Console.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <numeric>
#include <string>
#include <vector>

namespace
{
    int countAlivePlayers(const std::vector<Player*>& party)
    {
        int alive = 0;
        for (Player* player : party)
        {
            if (player != nullptr && !player->isDead())
            {
                ++alive;
            }
        }
        return alive;
    }

    struct CoopContribution
    {
        int turnsTaken = 0;
        int damageDealt = 0;
        int healingDone = 0;
        int damageTaken = 0;
        int supportActions = 0;
        bool wasDowned = false;
    };

    int scoreTargetThreat(Player& player, const CoopContribution& contribution)
    {
        int score = 10;

        if (player.isProvoking())
        {
            return 10000 + player.getProvocationTurns() * 100;
        }

        if (player.hasHealingThreat()) score += 85;
        score += std::min(120, contribution.healingDone / 2);
        score += std::min(90, contribution.damageDealt / 3);
        score += std::min(60, contribution.damageTaken / 4);

        if (player.getMaxHp() > 0)
        {
            int missingPercent = (player.getMaxHp() - player.getHp()) * 100 / player.getMaxHp();
            if (missingPercent >= 60) score += 35;
            else if (missingPercent >= 35) score += 20;
        }

        const std::string type = CombatClassSystem::normalizeClassText(player.getType());
        if (type.find("clerc") != std::string::npos || type.find("pretre") != std::string::npos || type.find("prêtre") != std::string::npos || type.find("alchimiste") != std::string::npos)
        {
            score += 35;
        }
        if (type.find("gardien") != std::string::npos || type.find("tank") != std::string::npos || type.find("colosse") != std::string::npos || player.isInDefensePosture())
        {
            score += 18;
        }

        return score;
    }

    Player* chooseAlivePlayerTarget(std::vector<Player*>& party, Random& random, const std::vector<CoopContribution>* contributions = nullptr)
    {
        std::vector<Player*> candidates;
        std::vector<int> scores;

        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr || player->isDead())
            {
                continue;
            }

            CoopContribution empty;
            const CoopContribution& contribution = (contributions != nullptr && i < contributions->size()) ? (*contributions)[i] : empty;
            candidates.push_back(player);
            scores.push_back(scoreTargetThreat(*player, contribution));
        }

        if (candidates.empty())
        {
            return nullptr;
        }

        int totalScore = std::accumulate(scores.begin(), scores.end(), 0);
        int roll = random.between(1, std::max(1, totalScore));
        int cursor = 0;

        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            cursor += scores[i];
            if (roll <= cursor)
            {
                return candidates[i];
            }
        }

        return candidates.back();
    }

    int sumActiveEnemyHp(const EnemyCombatQueue& wave)
    {
        int total = 0;
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i) total += std::max(0, wave.getActiveEnemy(i).getHp());
        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i) total += std::max(0, wave.getWaitingEnemy(i).getHp());
        return total;
    }

    std::vector<Summon> flattenCoopSummons(const std::vector<std::vector<Summon>>& partySummons)
    {
        std::vector<Summon> flattened;

        for (const std::vector<Summon>& summons : partySummons)
        {
            for (const Summon& summon : summons)
            {
                if (!summon.isDead() && !summon.isExpired())
                {
                    flattened.push_back(summon);
                }
            }
        }

        return flattened;
    }

    void displayPartyWaveCombatSnapshot(
        std::vector<Player*>& party,
        const EnemyCombatQueue& wave,
        const std::vector<std::vector<Summon>>& partySummons,
        const std::string& phase,
        int round,
        const std::string& actorName = ""
    )
    {
        std::vector<Entity*> entities;
        for (Player* player : party)
        {
            if (player != nullptr)
            {
                entities.push_back(player);
            }
        }

        std::vector<Summon> flattenedSummons = flattenCoopSummons(partySummons);
        GuiCombatStateSnapshot snapshot = CombatDisplay::buildWavePartySnapshot(
            entities,
            wave,
            flattenedSummons,
            "ÉTAT DU COMBAT COOP",
            phase,
            round
        );

        snapshot.currentActorName = actorName;
        CombatDisplay::displayCombatState(snapshot, false);
    }

    bool monsterCanUseHealingTools(const Monster& monster)
    {
        std::string profile = monster.getName() + " " + monster.getRaceText() + " " + monster.getType();
        std::transform(profile.begin(), profile.end(), profile.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

        return profile.find("soigneur") != std::string::npos
            || profile.find("prêtre") != std::string::npos
            || profile.find("pretre") != std::string::npos
            || profile.find("clerc") != std::string::npos
            || profile.find("chaman") != std::string::npos
            || profile.find("alchimiste") != std::string::npos
            || profile.find("sorcier") != std::string::npos
            || profile.find("mage putride") != std::string::npos
            || profile.find("chef bandit") != std::string::npos
            || profile.find("pilleur vétéran") != std::string::npos
            || profile.find("pilleur veteran") != std::string::npos;
    }

    int findMostInjuredMonsterAllyIndex(EnemyCombatQueue& wave, int healerIndex)
    {
        int bestIndex = -1;
        int bestPercent = 101;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (i == healerIndex || !wave.isActiveIndexValid(i))
            {
                continue;
            }

            Monster& ally = wave.getActiveEnemy(i);
            if (ally.isDead() || ally.getHp() >= ally.getMaxHp())
            {
                continue;
            }

            int percent = ally.getMaxHp() <= 0 ? 100 : ally.getHp() * 100 / ally.getMaxHp();
            if (percent < bestPercent)
            {
                bestPercent = percent;
                bestIndex = i;
            }
        }

        return bestIndex;
    }

    bool tryMonsterUseRareHealing(Monster& monster, EnemyCombatQueue& wave, int monsterIndex, Random& random)
    {
        if (monster.getHealingPotionCount() <= 0 || !monsterCanUseHealingTools(monster))
        {
            return false;
        }

        int healAmount = 35 + monster.getLevel() * 6;
        int allyIndex = findMostInjuredMonsterAllyIndex(wave, monsterIndex);

        if (allyIndex >= 0)
        {
            Monster& ally = wave.getActiveEnemy(allyIndex);
            int allyPercent = ally.getMaxHp() <= 0 ? 100 : ally.getHp() * 100 / ally.getMaxHp();
            int chance = allyPercent <= 35 ? 70 : 32;

            if (random.between(1, 100) <= chance)
            {
                monster.useHealingPotion(0);
                ally.heal(healAmount);
                monster.markHealingThreat();
                MessageScreen::show(
                    "SOIN ENNEMI",
                    "pve.monster.healing.ally",
                    {
                        monster.getName() + " utilise une potion/technique de soin sur " + ally.getName() + ".",
                        "Ce n'est pas un réflexe animal : seul un ennemi capable de comprendre le soin peut faire ça.",
                        ally.getName() + " récupère " + std::to_string(healAmount) + " PV et possède maintenant " + std::to_string(ally.getHp()) + "/" + std::to_string(ally.getMaxHp()) + " PV."
                    },
                    false
                );
                return true;
            }
        }

        if (monster.getHp() * 100 > monster.getMaxHp() * 35)
        {
            return false;
        }

        int selfChance = 18;
        if (random.between(1, 100) > selfChance)
        {
            return false;
        }

        monster.useHealingPotion(healAmount);
        MessageScreen::show(
            "SOIN ENNEMI",
            "pve.monster.healing.self",
            {
                monster.getName() + " utilise une potion de secours sur lui-même.",
                "Ce geste ne protège personne d'autre : c'est un pur réflexe de survie.",
                "PV actuels : " + std::to_string(monster.getHp()) + "/" + std::to_string(monster.getMaxHp()) + "."
            },
            false
        );
        return true;
    }

    void displayCoopPartyStatus(const std::vector<Player*>& party, const std::vector<bool>& wasDowned)
    {
        std::vector<std::string> lines;
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* player = party[i];
            if (player == nullptr)
            {
                continue;
            }

            std::string line = "J" + std::to_string(i + 1)
                + " [" + CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(i)) + "] - "
                + player->getName()
                + " : " + std::to_string(player->getHp()) + "/" + std::to_string(player->getMaxHp()) + " PV";
            if (player->isDead())
            {
                line += " [au sol]";
            }
            else if (i < wasDowned.size() && wasDowned[i])
            {
                line += " [a déjà chuté]";
            }
            lines.push_back(line);
        }

        MessageScreen::show("ÉTAT DU GROUPE", "pve.coop.party_status", lines, false);
    }

    std::vector<bool> extractDownedFlags(const std::vector<CoopContribution>& contributions)
    {
        std::vector<bool> flags;
        for (const CoopContribution& contribution : contributions) flags.push_back(contribution.wasDowned);
        return flags;
    }

    CombatReward buildIndividualCoopReward(
        const CombatReward& baseReward,
        const Player& player,
        const Player& sessionLeader,
        const CoopContribution& contribution
    )
    {
        int participation = contribution.turnsTaken > 0 ? 40 : 15;
        participation += std::min(35, contribution.damageDealt / 6);
        participation += std::min(25, contribution.healingDone / 5);
        participation += std::min(20, contribution.damageTaken / 7);
        participation += contribution.supportActions * 8;

        if (contribution.wasDowned)
        {
            participation = std::max(20, participation - 20);
        }

        int levelGap = sessionLeader.getLevel() - player.getLevel();
        if (levelGap >= 25) participation = std::min(participation, 35);
        else if (levelGap >= 15) participation = std::min(participation, 55);
        else if (levelGap >= 10) participation = std::min(participation, 75);

        participation = std::max(0, std::min(100, participation));
        return baseReward.getPercentage(participation);
    }


    bool hasAllyNeedingPotion(const std::vector<Player*>& party, const Player& healer)
    {
        for (Player* ally : party)
        {
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                return true;
            }
        }
        return false;
    }

    constexpr std::size_t PVE_PARTY_SUPPORT_PAGE_SIZE = 8;

    MenuOptionItemData makePvePartySupportData(
        const Player& healer,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "pve_party_support";
        itemData.section = "Soutien PvE coop";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = healer.getName();
        itemData.progress = "PV : " + std::to_string(healer.getHp()) + "/" + std::to_string(healer.getMaxHp());
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makePvePartyHealingTargetData(
        const Player& healer,
        const Player& target,
        std::size_t partyIndex
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "ally";
        itemData.section = "Cibles de soin PvE";
        itemData.actionType = "support";
        itemData.name = target.getName();
        itemData.detail = target.isDead() ? "Allié au sol à relever" : "Allié blessé à soigner";
        itemData.status = "PV : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp());
        itemData.owner = healer.getName();
        itemData.progress = "J" + std::to_string(partyIndex + 1)
            + " - " + CombatGroupBuilder::getFormationSlotLabel(static_cast<int>(partyIndex));
        itemData.important = target.isDead()
            || (target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 35);
        return itemData;
    }

    MenuOptionItemData makePvePartyPotionData(
        const Player& healer,
        const Consumable& potion,
        int inventoryIndex,
        int amount = 1
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "potion";
        itemData.section = "Potions de soutien PvE";
        itemData.actionType = "heal";
        itemData.name = potion.getName();
        itemData.quantity = std::to_string(std::max(1, amount));
        itemData.detail = potion.getDescription();
        itemData.status = "Soin : " + potion.getPowerDisplayText();
        itemData.price = "Valeur : " + std::to_string(potion.getValue()) + " or";
        itemData.stock = "Index inventaire : " + std::to_string(inventoryIndex + 1);
        itemData.owner = healer.getName();
        itemData.important = potion.getPower() >= 35;
        return itemData;
    }

    bool tryUseHealingPotionOnAlly(Player& healer, std::vector<Player*>& party, int& healingDone)
    {
        if (!hasAllyNeedingPotion(party, healer))
        {
            return false;
        }

        std::vector<int> potionIndices = CombatPotionUtils::getPotionIndices(
            healer,
            ConsumableType::Healing
        );

        if (potionIndices.empty())
        {
            return false;
        }

        MenuScreen supportScreen("SOUTIEN D'ÉQUIPE", "pve.party.support.choice");
        supportScreen.addSubtitle("Tour de " + healer.getName());
        supportScreen.addLine("Un allié peut recevoir une potion de soin avant l'action normale.");
        supportScreen.addOption(
            0,
            "Jouer normalement",
            "Ne consomme pas de potion.",
            true,
            "party.support.skip",
            makePvePartySupportData(healer, "skip", "Jouer normalement", "Ne consomme pas de potion.", "Action normale")
        );
        supportScreen.addOption(
            1,
            "Utiliser une potion de soin sur un allié",
            "Consomme le tour de soutien de " + healer.getName() + ".",
            true,
            "party.support.heal_ally",
            makePvePartySupportData(healer, "heal", "Potion de soutien", "Soigner ou relever un allié avant l'action normale.", "Consomme le tour", true)
        );
        int supportChoice = TerminalInterface::askMenuChoiceFromOptions(supportScreen, "Choisis une option affichée.");
        Console::clear();

        if (supportChoice == 0)
        {
            return false;
        }

        std::vector<Player*> targets;
        std::vector<std::size_t> targetPartyIndexes;
        for (std::size_t i = 0; i < party.size(); ++i)
        {
            Player* ally = party[i];
            if (ally != nullptr && ally != &healer && (ally->isDead() || ally->getHp() < ally->getMaxHp()))
            {
                targets.push_back(ally);
                targetPartyIndexes.push_back(i);
            }
        }

        if (targets.empty())
        {
            return false;
        }

        std::size_t targetPageIndex = 0;
        Player* target = nullptr;
        while (target == nullptr)
        {
            const std::size_t totalPages = PagedMenu::pageCount(targets.size(), PVE_PARTY_SUPPORT_PAGE_SIZE);
            if (targetPageIndex >= totalPages) targetPageIndex = totalPages - 1;
            const std::size_t firstIndex = PagedMenu::firstIndex(targetPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(targets.size(), targetPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);

            MenuScreen targetScreen("CHOIX DE L'ALLIÉ", "pve.party.support.target");
            targetScreen.addSubtitle("Potion de soutien de " + healer.getName());
            targetScreen.addLine("Alliés affichés : " + PagedMenu::rangeText(firstIndex, lastIndex, targets.size()));
            targetScreen.addLine("Choisis l'allié à soigner ou à relever.");
            targetScreen.addOption(
                0,
                "Annuler",
                "Retour au tour normal.",
                true,
                "party.support.target.cancel",
                makePvePartySupportData(healer, "cancel", "Annuler", "Retour au tour normal.", "Annulé")
            );

            for (std::size_t i = firstIndex; i < lastIndex; ++i)
            {
                Player* ally = targets[i];
                std::string label = ally->getName();
                if (ally->isDead())
                {
                    label += " [au sol]";
                }
                targetScreen.addOption(
                    static_cast<int>(i - firstIndex + 1),
                    label,
                    std::to_string(ally->getHp()) + "/" + std::to_string(ally->getMaxHp()) + " PV",
                    true,
                    "party.support.target",
                    makePvePartyHealingTargetData(healer, *ally, targetPartyIndexes[i])
                );
            }
            PagedMenu::addNavigationOptions(targetScreen, targetPageIndex, totalPages);

            int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une cible affichée.");
            Console::clear();

            if (targetChoice == 0)
            {
                return false;
            }
            if (targetChoice == 98 && targetPageIndex > 0)
            {
                --targetPageIndex;
                continue;
            }
            if (targetChoice == 99 && targetPageIndex + 1 < totalPages)
            {
                ++targetPageIndex;
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(targetChoice - 1);
            if (selectedIndex < targets.size() && selectedIndex < lastIndex)
            {
                target = targets[selectedIndex];
            }
        }

        std::size_t potionPageIndex = 0;
        int consumableIndex = -1;
        while (consumableIndex < 0)
        {
            std::vector<PotionStack> potionStacks = CombatPotionUtils::groupPotionIndices(healer, potionIndices);
            if (potionStacks.empty())
            {
                return false;
            }

            const std::size_t totalPages = PagedMenu::pageCount(potionStacks.size(), PVE_PARTY_SUPPORT_PAGE_SIZE);
            if (potionPageIndex >= totalPages) potionPageIndex = totalPages - 1;
            const std::size_t firstIndex = PagedMenu::firstIndex(potionPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);
            const std::size_t lastIndex = PagedMenu::lastIndexExclusive(potionStacks.size(), potionPageIndex, PVE_PARTY_SUPPORT_PAGE_SIZE);

            MenuScreen potionScreen("CHOIX DE LA POTION", "pve.party.support.potion");
            potionScreen.addSubtitle("Cible : " + target->getName());
            potionScreen.addLine("Piles affichées : " + PagedMenu::rangeText(firstIndex, lastIndex, potionStacks.size()));
            potionScreen.addLine("Choisis la potion de soin à utiliser.");
            potionScreen.addOption(
                0,
                "Annuler",
                "Ne consomme rien.",
                true,
                "party.support.potion.cancel",
                makePvePartySupportData(healer, "cancel", "Annuler", "Ne consomme rien.", "Annulé")
            );
            for (std::size_t i = firstIndex; i < lastIndex; ++i)
            {
                const PotionStack& stack = potionStacks[i];
                Consumable potion = healer.getInventory().getConsumable(stack.firstIndex);
                potionScreen.addOption(
                    static_cast<int>(i - firstIndex + 1),
                    CombatPotionUtils::stackLabel(potion.getName(), stack.amount),
                    "Soin : " + potion.getPowerDisplayText() + " | Quantité : " + std::to_string(stack.amount),
                    true,
                    "party.support.potion.healing",
                    makePvePartyPotionData(healer, potion, stack.firstIndex, stack.amount)
                );
            }
            PagedMenu::addNavigationOptions(potionScreen, potionPageIndex, totalPages);

            int potionChoice = TerminalInterface::askMenuChoiceFromOptions(potionScreen, "Choisis une potion affichée.");
            Console::clear();

            if (potionChoice == 0)
            {
                return false;
            }
            if (potionChoice == 98 && potionPageIndex > 0)
            {
                --potionPageIndex;
                continue;
            }
            if (potionChoice == 99 && potionPageIndex + 1 < totalPages)
            {
                ++potionPageIndex;
                continue;
            }

            const std::size_t selectedIndex = firstIndex + static_cast<std::size_t>(potionChoice - 1);
            if (selectedIndex < potionStacks.size() && selectedIndex < lastIndex)
            {
                consumableIndex = potionStacks[selectedIndex].firstIndex;
            }
        }
        if (!healer.getInventory().hasConsumable(consumableIndex))
        {
            MessageScreen::show(
                "POTION INTROUVABLE",
                "pve.party.support.potion.missing",
                {
                    "Cette potion n'est plus disponible.",
                    "Le soutien est annulé."
                },
                false
            );
            return false;
        }

        Consumable potion = healer.getInventory().getConsumable(consumableIndex);
        bool revivedTarget = target->isDead();
        if (revivedTarget)
        {
            target->reviveWithHealthPercentage(1);
            if (target->getHp() <= 0)
            {
                target->heal(1);
            }
        }
        int beforeHealHp = target->getHp();
        const int announcedHeal = potion.getHealingAmountForMaxHp(target->getMaxHp());
        target->heal(announcedHeal);
        healingDone += std::max(0, target->getHp() - beforeHealHp);
        healer.markHealingThreat();
        healer.recordChallengeCombatAction("ally_consumable");

        if (!healer.hasInfiniteConsumables())
        {
            healer.getInventory().removeConsumable(consumableIndex);
        }

        std::vector<std::string> resultLines;
        resultLines.push_back(healer.getName() + " devient soigneur ce tour-ci.");
        resultLines.push_back("Potion utilisée : " + potion.getName() + ".");
        resultLines.push_back("Cible : " + target->getName() + ".");
        if (revivedTarget)
        {
            resultLines.push_back(target->getName() + " est réveillé par la potion avant de récupérer ses forces.");
        }
        resultLines.push_back(target->getName() + " récupère " + std::to_string(target->getHp() - beforeHealHp) + " PV (soin annoncé : " + potion.getPowerDisplayText() + ").");
        resultLines.push_back("PV actuels : " + std::to_string(target->getHp()) + "/" + std::to_string(target->getMaxHp()) + ".");
        resultLines.push_back("Le tour de " + healer.getName() + " est consommé.");
        MessageScreen::show("SOUTIEN RÉUSSI", "pve.party.support.result", resultLines, false);
        return true;
    }

    void resolveLethalGroupDeathSaves(Player& player, Random& random)
    {
        if (!player.isDead())
        {
            return;
        }

        int green = 0;
        int red = 0;
        std::vector<std::string> lines;
        lines.push_back("Mort définitive coop : " + player.getName() + " est au sol.");
        lines.push_back("Les dés de survie commencent : 3 pastilles vertes pour revenir, 3 rouges pour disparaître.");

        while (green < 3 && red < 3)
        {
            int roll = random.between(1, 20);
            lines.push_back("Dé de survie : " + std::to_string(roll) + ".");

            if (roll == 20)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                lines.push_back("20 naturel : " + player.getName() + " se relève immédiatement à 1 PV et pourra rejouer.");
                MessageScreen::show("SURVIE EN MORT DÉFINITIVE COOP", "pve.coop.lethal_death_save.success_natural", lines, false);
                return;
            }

            if (roll == 1)
            {
                red += 2;
                lines.push_back("1 naturel : deux pastilles rouges apparaissent d'un coup.");
            }
            else if (roll >= 11)
            {
                ++green;
                lines.push_back("Pastille verte : " + std::to_string(green) + "/3.");
            }
            else
            {
                ++red;
                lines.push_back("Pastille rouge : " + std::to_string(red) + "/3.");
            }

            if (green >= 3)
            {
                player.reviveWithHealthPercentage(1);
                if (player.getHp() <= 0) player.heal(1);
                lines.push_back(player.getName() + " revient à 1 PV. La mort n'est pas comptée.");
                MessageScreen::show("SURVIE EN MORT DÉFINITIVE COOP", "pve.coop.lethal_death_save.success", lines, false);
                return;
            }
        }

        if (BlessingSystem::tryTriggerLethalSurvival(player))
        {
            lines.push_back(player.getName() + " reçoit trois pastilles rouges, mais toutes ses bénédictions se consument avant l'effacement.");
            lines.push_back("Le personnage revient à 1 PV, sans inventaire ni équipement, avec une marque irréversible.");
            MessageScreen::show("INTERVENTION DIVINE", "pve.coop.lethal_death_save.blessing", lines, false);
            DeathPenaltySystem::displayLethalSurvivalAnomaly();
            return;
        }

        player.recordDeath();
        lines.push_back(player.getName() + " reçoit trois pastilles rouges : mort définitive. Aucune bénédiction capable de briser le verdict n'a répondu.");
        MessageScreen::show("SURVIE EN MORT DÉFINITIVE COOP", "pve.coop.lethal_death_save.failure", lines, false);
    }
}
void MonsterPveMode::runTeam(
    std::vector<Player*>& party,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule
)
{
    if (party.empty() || party[0] == nullptr)
    {
        return;
    }

    Player& leader = *party[0];

    Console::clear();
    MessageScreen::show(
        "PvE COOP",
        "pve.coop.intro",
        {
            "Joueur principal : " + leader.getName() + ".",
            "Les données de voyage, niveau de session, événements et monstres suivent le joueur 1.",
            "Les récompenses resteront individuelles selon participation, chance et écart de niveau."
        },
        false
    );
    CombatGroupBuilder::displayFormationRules();

    WaveCombatSystem::displayWaveIntroduction();
    EnemyCombatQueue wave = WaveCombatSystem::createWaveForPlayer(leader, random, difficulty);

    WaveCombatSystem::displayFrontLineArrival(wave);
    PveWaveNarrativeSupport::recordWaveEncountersInBestiary(wave);
    PveWaveNarrativeSupport::recordWaveEncountersInJournal(leader, wave);
    EncounterDialogueSystem::display(leader, wave, random, "pve.coop");

    std::vector<int> initialHp;
    std::vector<CoopContribution> contributions(party.size());
    std::vector<std::vector<Summon>> partySummons(party.size());
    std::vector<SummonControlMode> summonControlModes(party.size(), SummonControlMode::Automatic);

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        initialHp.push_back(player != nullptr ? player->getHp() : 0);

        if (player != nullptr)
        {
            player->beginChallengeCombatTracking();
            partySummons[i] = SummonCombatSystem::createInitialSummonsFor(*player);
            SummonCombatSystem::displaySummonArrival(*player, partySummons[i]);
            summonControlModes[i] = SummonCombatSystem::askPlayerSummonControlMode(*player, partySummons[i]);
        }
    }

    bool escapeSucceeded = false;
    int round = 1;

    while (countAlivePlayers(party) > 0 && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        MessageScreen::show(
            "TOUR DE GROUPE " + std::to_string(round),
            "pve.coop.round." + std::to_string(round),
            {"L'initiative mélange joueurs, invocations et ennemis selon la Dextérité, la vitesse et un d20."},
            false
        );
        displayCoopPartyStatus(party, extractDownedFlags(contributions));
        displayPartyWaveCombatSnapshot(
            party,
            wave,
            partySummons,
            "Début du tour de groupe",
            round
        );

        InitiativeQueue initiative = InitiativeSystem::buildWaveQueue(party, wave, partySummons, random);
        MessageScreen::show(
            "ORDRE D'INITIATIVE",
            "pve.coop.initiative." + std::to_string(round),
            InitiativeSystem::buildDisplayLines(initiative),
            false
        );

        for (const InitiativeRoll& entry : initiative.getEntries())
        {
            if (!wave.hasEnemiesLeft() || escapeSucceeded || countAlivePlayers(party) <= 0)
            {
                break;
            }

            if (TurnOrder::isPlayer(entry.id))
            {
                const std::size_t i = static_cast<std::size_t>(std::max(0, entry.slotIndex));
                if (i >= party.size()) continue;
                Player* player = party[i];
                if (player == nullptr || player->isDead()) continue;

                MessageScreen::show(
                    "TOUR ALLIÉ",
                    "pve.coop.player_turn." + std::to_string(i + 1),
                    {"Tour de " + player->getName() + " [joueur " + std::to_string(i + 1) + "] — initiative " + std::to_string(entry.totalScore) + "."},
                    false
                );

                displayPartyWaveCombatSnapshot(
                    party,
                    wave,
                    partySummons,
                    "Action d'un joueur allié",
                    round,
                    player->getName()
                );

                int healingDoneThisTurn = 0;
                int enemyHpBeforeTurn = sumActiveEnemyHp(wave);
                bool finished = tryUseHealingPotionOnAlly(*player, party, healingDoneThisTurn);
                while (!finished && !player->isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
                {
                    finished = PlayerWaveCombatTurn::play(
                        *player,
                        wave,
                        random,
                        escapeSucceeded,
                        difficulty
                    );
                }

                if (finished)
                {
                    player->reduceClassSkillCooldown();
                    const int enemyHpAfterTurn = sumActiveEnemyHp(wave);
                    contributions[i].turnsTaken++;
                    contributions[i].damageDealt += std::max(0, enemyHpBeforeTurn - enemyHpAfterTurn);
                    contributions[i].healingDone += healingDoneThisTurn;
                    if (healingDoneThisTurn > 0) contributions[i].supportActions++;
                }
            }
            else if (TurnOrder::isSummonGroup(entry.id))
            {
                const std::size_t i = static_cast<std::size_t>(std::max(0, entry.slotIndex));
                if (i >= party.size() || i >= partySummons.size()) continue;
                Player* owner = party[i];
                if (owner == nullptr || owner->isDead() || !SummonCombatSystem::hasActiveSummons(partySummons[i])) continue;

                MessageScreen::show(
                    "TOUR DES INVOCATIONS",
                    "pve.coop.summon_turn." + std::to_string(i + 1),
                    {entry.label + " agissent à leur propre initiative : " + std::to_string(entry.totalScore) + "."},
                    false
                );
                const int before = sumActiveEnemyHp(wave);
                SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                    partySummons[i],
                    wave,
                    random,
                    summonControlModes[i]
                );
                const int summonDamage = std::max(0, before - sumActiveEnemyHp(wave));
                owner->recordChallengeSummonAction(summonDamage);
                if (summonDamage > 0)
                {
                    contributions[i].supportActions++;
                    contributions[i].damageDealt += summonDamage;
                }
            }
            else if (TurnOrder::isEnemy(entry.id))
            {
                const int enemyIndex = entry.slotIndex;
                if (!wave.isActiveIndexValid(enemyIndex)) continue;
                Monster& monster = wave.getActiveEnemy(enemyIndex);
                if (monster.isDead()) continue;

                Player* target = chooseAlivePlayerTarget(party, random, &contributions);
                if (target == nullptr) break;

                MessageScreen::show(
                    "TOUR ENNEMI",
                    "pve.coop.enemy_turn." + std::to_string(enemyIndex),
                    {"Tour de " + monster.getName() + " : cible " + target->getName() + " — initiative " + std::to_string(entry.totalScore) + "."},
                    false
                );
                if (!tryMonsterUseRareHealing(monster, wave, enemyIndex, random))
                {
                    const int targetHpBefore = target->getHp();
                    TurnManager::executeAttack(monster, *target, random);
                    for (std::size_t partyIndex = 0; partyIndex < party.size(); ++partyIndex)
                    {
                        if (party[partyIndex] == target)
                        {
                            contributions[partyIndex].damageTaken += std::max(0, targetHpBefore - target->getHp());
                            break;
                        }
                    }
                }

                for (std::size_t i = 0; i < party.size(); ++i)
                {
                    if (party[i] != nullptr && party[i]->isDead())
                    {
                        contributions[i].wasDowned = true;
                    }
                }
                Console::pauseSeconds(1);
            }
        }

        displayPartyWaveCombatSnapshot(
            party,
            wave,
            partySummons,
            "Après le tour d'initiative",
            round
        );

        wave.removeDeadAndReplace();
        ++round;
    }

    const int aliveAtCombatEnd = countAlivePlayers(party);
    int groupConsumables = 0;
    int groupSkills = 0;
    int groupNonBasic = 0;
    int groupBasic = 0;
    int groupDamageTaken = 0;
    int groupSummonActions = 0;
    int realPartySize = 0;
    for (Player* member : party)
    {
        if (member == nullptr) continue;
        ++realPartySize;
        groupConsumables += member->getChallengeCombatConsumablesUsed();
        groupSkills += member->getChallengeCombatSkillsUsed();
        groupNonBasic += member->getChallengeCombatNonBasicAttacksUsed();
        groupBasic += member->getChallengeCombatBasicAttacksUsed();
        groupDamageTaken += member->getChallengeCombatDamageTaken();
        groupSummonActions += member->getChallengeCombatSummonActions();
    }

    bool eliteDefeated = false;
    for (int defeatedIndex = 0; defeatedIndex < wave.getDefeatedEnemyCount(); ++defeatedIndex)
    {
        const Monster& defeated = wave.getDefeatedEnemy(defeatedIndex);
        if (defeated.isElite() || defeated.isEvolved())
        {
            eliteDefeated = true;
            break;
        }
    }

    const bool groupVictory = !escapeSucceeded && !wave.hasEnemiesLeft() && aliveAtCombatEnd > 0;
    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* member = party[i];
        if (member == nullptr || !member->isChallengeCombatTrackingActive()) continue;
        member->applyChallengeCombatGroupSummary(
            realPartySize,
            aliveAtCombatEnd,
            groupConsumables,
            groupSkills,
            groupNonBasic,
            groupBasic,
            groupDamageTaken,
            groupSummonActions
        );
        const bool participated = i < contributions.size() && contributions[i].turnsTaken > 0;
        member->finishChallengeCombatTracking(
            groupVictory && participated,
            false,
            eliteDefeated,
            wave.getDefeatedEnemyCount()
        );
    }

    if (escapeSucceeded)
    {
        MessageScreen::show(
            "FUITE DE GROUPE",
            "pve.coop.escape.success",
            {
                "Le groupe a ouvert une sortie.",
                "Chaque personnage récupère seulement une part de ce qu'il a réellement aidé à obtenir."
            },
            false
        );
    }

    if (countAlivePlayers(party) == 0 && wave.hasEnemiesLeft())
    {
        MessageScreen::show(
            "GROUPE AU SOL",
            "pve.coop.party_defeat",
            {"Tout le groupe est tombé."},
            false
        );

        for (Player* player : party)
        {
            if (player == nullptr) continue;

            player->recordDefeat();
            if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
            {
                resolveLethalGroupDeathSaves(*player, random);
            }
            else
            {
                player->recordDeath();
                player->reviveWithHealthPercentage(
                    DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
                );
            }
        }

        return;
    }

    PveWaveNarrativeSupport::displaySpecialDefeatDialogues(wave);
    CombatReward baseReward = escapeSucceeded
        ? CombatRewardSystem::calculatePlayerEscapeReward(wave, difficulty)
        : CombatRewardSystem::calculateWaveReward(wave, difficulty, leader, initialHp[0], round, random);

    MessageScreen::show(
        "RÉCOMPENSES INDIVIDUELLES COOP",
        "pve.coop.rewards.start",
        {"Les récompenses sont calculées selon la participation réelle de chaque personnage."},
        false
    );

    for (std::size_t i = 0; i < party.size(); ++i)
    {
        Player* player = party[i];
        if (player == nullptr)
        {
            continue;
        }

        if (player->isDead() && !DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            player->recordDeath();
            player->reviveWithHealthPercentage(
                DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
            );
            MessageScreen::show(
                "RÉVEIL DE FIN DE COMBAT",
                "pve.coop.reward.revive." + std::to_string(i + 1),
                {player->getName() + " est réveillé à la fin du combat. La mort est comptabilisée."},
                false
            );
        }

        if (player->isDead())
        {
            MessageScreen::show(
                "AUCUNE RÉCOMPENSE",
                "pve.coop.reward.down." + std::to_string(i + 1),
                {player->getName() + " reste au sol : aucune récompense supplémentaire après sa chute."},
                false
            );
            continue;
        }

        CombatReward individualReward = buildIndividualCoopReward(
            baseReward,
            *player,
            leader,
            contributions[i]
        );

        MessageScreen::show(
            "RÉCOMPENSE DE " + player->getName(),
            "pve.coop.reward.player." + std::to_string(i + 1),
            {"Résumé de participation et récompense individuelle."},
            false
        );
        CombatRewardSystem::displayReward(individualReward);
        CombatRewardSystem::giveRewardToPlayer(*player, individualReward);
        player->recordVictory();
        player->recordEnemyKills(wave.getDefeatedEnemyCount());
        PveWaveNarrativeSupport::recordWaveKillsInJournal(*player, wave);
        MessageScreen::show(
            "PARTICIPATION",
            "pve.coop.reward.participation." + std::to_string(i + 1),
            {
                "Tours joués : " + std::to_string(contributions[i].turnsTaken) + ".",
                "Dégâts infligés : " + std::to_string(contributions[i].damageDealt) + ".",
                "Soins effectués : " + std::to_string(contributions[i].healingDone) + ".",
                "Dégâts encaissés : " + std::to_string(contributions[i].damageTaken) + "."
            },
            false
        );
        LootGenerator::giveDefeatedWaveLoot(*player, wave, random, difficulty);
    }

    PveWaveNarrativeSupport::recordWaveKillsInBestiary(wave);
}
