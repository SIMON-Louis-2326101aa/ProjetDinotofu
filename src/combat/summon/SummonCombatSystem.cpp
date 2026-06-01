// EN: SummonCombatSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonCombatSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Handles active summons during combat and lets summoner classes slowly turn duels into small group fights.

#include "combat/summon/SummonCombatSystem.hpp"

#include "combat/summon/SummonCatalog.hpp"
#include "combat/summon/SummonRules.hpp"
#include "combat/action/SpecialCombatEffects.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>


namespace
{
    std::map<std::string, int>& summonLinkPools()
    {
        static std::map<std::string, int> pools;
        return pools;
    }

    void showSummonNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = true
    )
    {
        MessageScreen::show(title, screenId, lines, waitAndClear);
    }

    void showSummonActionResult(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        showSummonNotice(title, screenId, lines, false);
    }

    std::string hpLine(const Entity& entity, int hpBefore)
    {
        return "PV " + entity.getName() + " : "
            + std::to_string(hpBefore)
            + " -> "
            + std::to_string(entity.getHp())
            + "/"
            + std::to_string(entity.getMaxHp())
            + ".";
    }

    std::string summonDurationText(const Summon& summon)
    {
        return "Durée : " + std::to_string(summon.getDurationTurns())
            + " tour(s) | maintien : " + std::to_string(summon.getMaintenanceCost())
            + " | slots : " + std::to_string(summon.getSlotCost());
    }

    int getSummonLinkRemaining(const std::string& ownerName);

    MenuOptionItemData makeSummonControlItemData(
        const Player& owner,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "summon_control";
        itemData.section = "Invocations";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = owner.getName();
        itemData.progress = "Lien disponible : " + std::to_string(getSummonLinkRemaining(owner.getName()));
        itemData.important = actionType == "summon_manual";
        return itemData;
    }

    MenuOptionItemData makeSummonActionItemData(
        const Summon& summon,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "Disponible",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "summon_action";
        itemData.section = "Ordre d'invocation";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = summon.getOwnerName();
        itemData.progress = "PV : " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp())
            + " | Durée : " + std::to_string(summon.getDurationTurns())
            + " | Lien : " + std::to_string(getSummonLinkRemaining(summon.getOwnerName()));
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeSummonTargetItemData(const Summon& summon, const Monster& monster)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = monster.isInvocation() ? "summon_target" : "enemy";
        itemData.section = "Cibles d'invocation";
        itemData.actionType = "target";
        itemData.name = monster.getName();
        itemData.detail = "Cible choisie par " + summon.getName();
        itemData.status = "PV : " + std::to_string(monster.getHp()) + "/" + std::to_string(monster.getMaxHp());
        itemData.owner = summon.getOwnerName();
        itemData.progress = "Invocation : " + summon.getName() + " | " + summonDurationText(summon);
        itemData.important = monster.getMaxHp() > 0 && monster.getHp() * 100 <= monster.getMaxHp() * 35;
        return itemData;
    }

    MenuOptionItemData makeTargetableSummonItemData(const Summon& summon)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "summon_target";
        itemData.section = "Invocations ciblables";
        itemData.actionType = "target";
        itemData.name = summon.getName();
        itemData.detail = "Invocation pouvant recevoir certains effets, attaques ennemies ou protections.";
        itemData.status = "PV : " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp());
        itemData.owner = summon.getOwnerName();
        itemData.progress = "Durée : " + std::to_string(summon.getDurationTurns())
            + " tour(s) | slots " + std::to_string(summon.getSlotCost())
            + " | maintien " + std::to_string(summon.getMaintenanceCost());
        itemData.important = summon.getMaxHp() > 0 && summon.getHp() * 100 <= summon.getMaxHp() * 35;
        return itemData;
    }

    // EN: calculateMaxSummonLink declares or implements a focused behavior used by this module.
    // FR: calculateMaxSummonLink déclare ou implémente un comportement précis utilisé par ce module.
    int calculateMaxSummonLink(const Player& owner)
    {
        int maxLink = 8 + owner.getLevel() / 4;
        std::string className = owner.getType();
        std::string raceName = owner.getRaceText();

        if (className.find("Mage") != std::string::npos
            || className.find("Alchimiste") != std::string::npos)
        {
            maxLink += 5;
        }

        if (className.find("Assassin") != std::string::npos
            || className.find("Rôdeur") != std::string::npos)
        {
            maxLink += 2;
        }

        if (raceName.find("Kitsune") != std::string::npos
            || raceName.find("Fée") != std::string::npos
            || raceName.find("Semi-dragon") != std::string::npos)
        {
            maxLink += 3;
        }

        if (owner.getName() == "Aoi"
            || owner.getName() == "Fail"
            || owner.getName() == "Kanadé"
            || owner.getName() == "Hestia")
        {
            maxLink += 4;
        }

        if (owner.getName() == "Hazak")
        {
            maxLink += 2;
        }

        return maxLink < 1 ? 1 : maxLink;
    }

    // EN: initializeSummonLinkPool declares or implements a focused behavior used by this module.
    // FR: initializeSummonLinkPool déclare ou implémente un comportement précis utilisé par ce module.
    void initializeSummonLinkPool(const Player& owner)
    {
        summonLinkPools()[owner.getName()] = calculateMaxSummonLink(owner);
    }

    // EN: getSummonLinkRemaining declares or implements a focused behavior used by this module.
    // FR: getSummonLinkRemaining déclare ou implémente un comportement précis utilisé par ce module.
    int getSummonLinkRemaining(const std::string& ownerName)
    {
        auto it = summonLinkPools().find(ownerName);

        if (it == summonLinkPools().end())
        {
            return 0;
        }

        return it->second;
    }

    // EN: spendSummonLink declares or implements a focused behavior used by this module.
    // FR: spendSummonLink déclare ou implémente un comportement précis utilisé par ce module.
    bool spendSummonLink(const Summon& summon)
    {
        int cost = summon.getMaintenanceCost();
        int& remaining = summonLinkPools()[summon.getOwnerName()];

        if (remaining < cost)
        {
            return false;
        }

        remaining -= cost;
        return true;
    }

    // EN: canKeepSummonActive declares or implements a focused behavior used by this module.
    // FR: canKeepSummonActive déclare ou implémente un comportement précis utilisé par ce module.
    bool canKeepSummonActive(Summon& summon)
    {
        if (spendSummonLink(summon))
        {
            return true;
        }

        showSummonNotice(
            "LIEN D'INVOCATION FAIBLE",
            "combat.summon.link.too_low",
            {
                summon.getName() + " vacille.",
                summon.getOwnerName() + " n'a plus assez de lien d'invocation pour la maintenir."
            }
        );
        summon.decreaseDuration();
        return false;
    }
}

std::vector<Summon> SummonCombatSystem::createInitialSummonsFor(const Player& owner)
{
    if (!SummonRules::characterCanSummon(owner.getName(), owner.getType()))
    {
        return {};
    }

    initializeSummonLinkPool(owner);

    std::vector<Summon> summons = SummonCatalog::createStarterSummonsForClass(
        owner.getName(),
        owner.getType()
    );

    int maxSlots = SummonRules::getMaxSummonSlots(owner.getName(), owner.getType());

    while (calculateUsedSlots(summons) > maxSlots && !summons.empty())
    {
        summons.pop_back();
    }

    return summons;
}

// EN: hasActiveSummons declares or implements a focused behavior used by this module.
// FR: hasActiveSummons déclare ou implémente un comportement précis utilisé par ce module.
bool SummonCombatSystem::hasActiveSummons(const std::vector<Summon>& summons)
{
    for (const Summon& summon : summons)
    {
        if (!summon.isDead() && !summon.isExpired())
        {
            return true;
        }
    }

    return false;
}


// EN: calculateUsedSlots declares or implements a focused behavior used by this module.
// FR: calculateUsedSlots déclare ou implémente un comportement précis utilisé par ce module.
int SummonCombatSystem::calculateUsedSlots(const std::vector<Summon>& summons)
{
    int usedSlots = 0;

    for (const Summon& summon : summons)
    {
        if (!summon.isDead() && !summon.isExpired())
        {
            usedSlots += summon.getSlotCost();
        }
    }

    return usedSlots;
}

void SummonCombatSystem::displaySummonArrival(
    const Player& owner,
    const std::vector<Summon>& summons
)
{
    if (summons.empty())
    {
        return;
    }

    int maxSlots = SummonRules::getMaxSummonSlots(owner.getName(), owner.getType());
    std::vector<std::string> lines;
    lines.push_back(owner.getName() + " n'entre pas vraiment seul dans le combat.");
    lines.push_back("Slots d'invocation utilisés : " + std::to_string(calculateUsedSlots(summons)) + "/" + std::to_string(maxSlots) + ".");
    lines.push_back("Lien d'invocation disponible : " + std::to_string(getSummonLinkRemaining(owner.getName())) + " point(s) pour ce combat.");

    for (const Summon& summon : summons)
    {
        lines.push_back("- " + summon.getName() + " apparaît aux côtés de " + owner.getName() + " | " + summonDurationText(summon) + ".");
    }

    showSummonNotice("INVOCATIONS ENTRANTES", "combat.summon.arrival", lines);
}


SummonControlMode SummonCombatSystem::askPlayerSummonControlMode(
    const Player& owner,
    const std::vector<Summon>& summons
)
{
    if (summons.empty())
    {
        return SummonControlMode::Automatic;
    }

    MenuScreen screen("CONTRÔLE DES INVOCATIONS", "combat.summon.control_mode");
    screen.addLine("Invocateur : " + owner.getName());
    screen.addLine("Invocations actives : " + std::to_string(summons.size()));
    screen.addOption(
        1,
        "Attaque automatique",
        "Les invocations attaquent seules après ton tour.",
        true,
        "combat.summon.mode.automatic",
        makeSummonControlItemData(owner, "summon_auto", "Attaque automatique", "Les invocations choisissent leur cible sans ouvrir de sous-menu.", "Simple")
    );
    screen.addOption(
        2,
        "Contrôle manuel",
        "Tu joues aussi tes invocations quand ton tour est terminé.",
        true,
        "combat.summon.mode.manual",
        makeSummonControlItemData(owner, "summon_manual", "Contrôle manuel", "Ouvre des ordres détaillés pour chaque invocation active.", "Plus précis")
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Choisis 1 pour automatique ou 2 pour manuel."
    );
    Console::clear();

    if (choice == 2)
    {
        showSummonNotice(
            "CONTRÔLE MANUEL",
            "combat.summon.control_mode.manual",
            {
                "Contrôle manuel activé pour tes invocations.",
                "Elles suivront tes ordres tant que le lien d'invocation tient."
            }
        );
        return SummonControlMode::Manual;
    }

    showSummonNotice(
        "CONTRÔLE AUTOMATIQUE",
        "combat.summon.control_mode.automatic",
        {
            "Tes invocations choisissent leurs cibles selon leur instinct.",
            "Elles agiront seules après ton tour."
        }
    );

    return SummonControlMode::Automatic;
}

void SummonCombatSystem::playSummonTurnsAgainstEntity(
    std::vector<Summon>& summons,
    Entity& target,
    Random& random
)
{
    if (hasActiveSummons(summons))
    {
        showSummonActionResult(
            "TOUR DES INVOCATIONS",
            "combat.summon.auto.entity.start",
            {
                "Mode : automatique.",
                "Cible principale : " + target.getName() + "."
            }
        );
    }

    for (Summon& summon : summons)
    {
        if (target.isDead())
        {
            break;
        }

        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        if (!canKeepSummonActive(summon))
        {
            continue;
        }

        if (hasSpecialAbility(summon) && random.between(1, 100) <= 30)
        {
            executeSummonSpecialAbility(summon, target, random);
        }
        else
        {
            executeSummonAttack(summon, target, random);
            summon.decreaseDuration();
        }
    }

    removeInactiveSummons(summons);
}

void SummonCombatSystem::playSummonTurnsAgainstWave(
    std::vector<Summon>& summons,
    EnemyCombatQueue& wave,
    Random& random
)
{
    if (hasActiveSummons(summons))
    {
        showSummonActionResult(
            "TOUR DES INVOCATIONS",
            "combat.summon.auto.wave.start",
            {
                "Mode : automatique.",
                "Les invocations choisissent une cible active dans la vague."
            }
        );
    }

    for (Summon& summon : summons)
    {
        if (!wave.hasEnemiesLeft())
        {
            break;
        }

        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        if (!canKeepSummonActive(summon))
        {
            continue;
        }

        if (!wave.hasActiveEnemies())
        {
            wave.initializeFrontLine();
        }

        if (!wave.hasActiveEnemies())
        {
            break;
        }

        int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
        Monster& target = wave.getActiveEnemy(targetIndex);

        if (hasSpecialAbility(summon) && random.between(1, 100) <= 30)
        {
            executeSummonSpecialAbility(summon, target, random);
        }
        else
        {
            executeSummonAttack(summon, target, random);
            summon.decreaseDuration();
        }
        wave.removeDeadAndReplace();
    }

    removeInactiveSummons(summons);
}


void SummonCombatSystem::playPlayerSummonTurnsAgainstEntity(
    std::vector<Summon>& summons,
    Entity& target,
    Random& random,
    SummonControlMode controlMode
)
{
    if (controlMode == SummonControlMode::Automatic)
    {
        playSummonTurnsAgainstEntity(summons, target, random);
        return;
    }

    showSummonActionResult(
        "TOUR DES INVOCATIONS",
        "combat.summon.manual.entity.start",
        {
            "Mode : contrôle manuel.",
            "Chaque invocation encore liée peut recevoir un ordre."
        }
    );

    for (Summon& summon : summons)
    {
        if (target.isDead())
        {
            break;
        }

        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        if (!canKeepSummonActive(summon))
        {
            continue;
        }

        playManualSummonTurnAgainstEntity(summon, target, random);
    }

    showSummonActionResult(
        "FIN DU TOUR DES INVOCATIONS",
        "combat.summon.manual.entity.end",
        {"Les ordres manuels de ce tour sont terminés."}
    );

    removeInactiveSummons(summons);
}

void SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
    std::vector<Summon>& summons,
    EnemyCombatQueue& wave,
    Random& random,
    SummonControlMode controlMode
)
{
    if (controlMode == SummonControlMode::Automatic)
    {
        playSummonTurnsAgainstWave(summons, wave, random);
        return;
    }

    showSummonActionResult(
        "TOUR DES INVOCATIONS",
        "combat.summon.manual.wave.start",
        {
            "Mode : contrôle manuel.",
            "Chaque invocation peut choisir une cible active dans la vague."
        }
    );

    for (Summon& summon : summons)
    {
        if (!wave.hasEnemiesLeft())
        {
            break;
        }

        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        if (!canKeepSummonActive(summon))
        {
            continue;
        }

        playManualSummonTurnAgainstWave(summon, wave, random);
        wave.removeDeadAndReplace();
    }

    showSummonActionResult(
        "FIN DU TOUR DES INVOCATIONS",
        "combat.summon.manual.wave.end",
        {"Les ordres manuels contre la vague sont terminés."}
    );

    removeInactiveSummons(summons);
}

void SummonCombatSystem::playManualSummonTurnAgainstEntity(
    Summon& summon,
    Entity& target,
    Random& random
)
{
    MenuScreen screen("ORDRE D'INVOCATION", "combat.summon.manual.entity");
    screen.addLine(summon.getName() + " attend ton ordre.");
    screen.addLine(summonDurationText(summon));
    screen.addLine("Lien restant pour " + summon.getOwnerName() + " : " + std::to_string(getSummonLinkRemaining(summon.getOwnerName())) + " point(s).");
    screen.addBackOption("Ne rien faire", "combat.summon.manual.wait");
    screen.addOption(
        1,
        "Attaquer " + target.getName(),
        "Attaque normale de l'invocation.",
        true,
        "combat.summon.manual.attack",
        makeSummonActionItemData(summon, "attack", "Attaque normale", "Frappe directe sur " + target.getName() + ".", "Dégâts " + std::to_string(summon.getMinDamage()) + "-" + std::to_string(summon.getMaxDamage()), true)
    );

    if (hasSpecialAbility(summon))
    {
        screen.addOption(
            2,
            "Technique d'invocation",
            "Utiliser l'effet spécial de cette invocation.",
            true,
            "combat.summon.manual.special",
            makeSummonActionItemData(summon, "special", "Technique d'invocation", "Utilise l'effet spécial disponible sans révéler de faiblesse cachée.", "Technique disponible", true)
        );
    }

    screen.addOption(
        3,
        "Maintenir le lien",
        "+1 tour, pas d'attaque.",
        true,
        "combat.summon.manual.maintain",
        makeSummonActionItemData(summon, "support", "Maintenir le lien", "Consomme le lien pour prolonger la durée de l'invocation.", "+1 tour")
    );
    screen.addOption(
        4,
        "Sacrifier l'invocation",
        "Rupture violente du lien contre la cible.",
        summon.canBeSacrificed(),
        "combat.summon.manual.sacrifice",
        makeSummonActionItemData(summon, "danger", "Sacrifier l'invocation", "Détruit l'invocation pour infliger un effet violent à la cible.", summon.canBeSacrificed() ? "Dangereux" : "Impossible", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Choisis une action disponible pour cette invocation."
    );
    Console::clear();

    if (choice == 0)
    {
        showSummonNotice(
            "INVOCATION EN RETRAIT",
            "combat.summon.manual.wait.result",
            {summon.getName() + " reste en retrait, mais le lien d'invocation continue de se consumer."}
        );
        summon.decreaseDuration();
        return;
    }

    if (choice == 2 && hasSpecialAbility(summon))
    {
        executeSummonSpecialAbility(summon, target, random);
        return;
    }

    if (choice == 3)
    {
        maintainSummonLink(summon);
        return;
    }

    if (choice == 4)
    {
        sacrificeSummonAgainstEntity(summon, target, random);
        return;
    }

    executeSummonAttack(summon, target, random);
    summon.decreaseDuration();
}

void SummonCombatSystem::playManualSummonTurnAgainstWave(
    Summon& summon,
    EnemyCombatQueue& wave,
    Random& random
)
{
    if (!wave.hasActiveEnemies())
    {
        wave.initializeFrontLine();
    }

    if (!wave.hasActiveEnemies())
    {
        return;
    }

    constexpr std::size_t itemsPerPage = 6;
    std::size_t pageIndex = 0;
    const int activeEnemyCount = wave.getActiveEnemyCount();
    const std::size_t totalPages = PagedMenu::pageCount(static_cast<std::size_t>(activeEnemyCount), itemsPerPage);
    int selectedEnemyIndex = -1;

    while (selectedEnemyIndex < 0)
    {
        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(static_cast<std::size_t>(activeEnemyCount), pageIndex, itemsPerPage);
        std::vector<int> visibleEnemyIndexes;

        MenuScreen targetScreen("CIBLE DE L'INVOCATION", "combat.summon.manual.wave.target");
        targetScreen.addLine(summon.getName() + " attend ton ordre.");
        targetScreen.addLine(summonDurationText(summon));
        targetScreen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, static_cast<std::size_t>(activeEnemyCount)));
        if (hasSpecialAbility(summon))
        {
            targetScreen.addLine("Astuce : choisis une cible, puis l'invocation pourra utiliser sa technique spéciale.");
        }
        PagedMenu::addNavigationOptions(targetScreen, pageIndex, totalPages);

        for (std::size_t i = first; i < last; ++i)
        {
            Monster& monster = wave.getActiveEnemy(static_cast<int>(i));
            const int visibleNumber = static_cast<int>(visibleEnemyIndexes.size()) + 1;
            visibleEnemyIndexes.push_back(static_cast<int>(i));

            targetScreen.addOption(
                visibleNumber,
                monster.getName(),
                std::to_string(monster.getHp()) + "/" + std::to_string(monster.getMaxHp()) + " PV",
                !monster.isDead(),
                "combat.summon.manual.wave.target." + std::to_string(i),
                makeSummonTargetItemData(summon, monster)
            );
        }

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            targetScreen,
            "Choisis une cible affichée, 98/99 pour naviguer, ou 0 pour revenir."
        );
        Console::clear();

        if (choice == 0)
        {
            showSummonNotice(
                "INVOCATION EN RETRAIT",
                "combat.summon.manual.wave.wait.result",
                {summon.getName() + " reste en retrait, mais le lien d'invocation continue de se consumer."}
            );
            summon.decreaseDuration();
            return;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        if (choice >= 1 && choice <= static_cast<int>(visibleEnemyIndexes.size()))
        {
            selectedEnemyIndex = visibleEnemyIndexes[choice - 1];
        }
    }

    Monster& target = wave.getActiveEnemy(selectedEnemyIndex);

    if (hasSpecialAbility(summon))
    {
        MenuScreen actionScreen("ACTION DE L'INVOCATION", "combat.summon.manual.wave.action");
        actionScreen.addLine("Invocation : " + summon.getName());
        actionScreen.addLine("Cible : " + target.getName() + " | " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()) + " PV");
        actionScreen.addOption(
            1,
            "Attaque normale",
            "Frappe directe de l'invocation.",
            true,
            "combat.summon.manual.wave.attack",
            makeSummonActionItemData(summon, "attack", "Attaque normale", "Frappe directe sur " + target.getName() + ".", "Dégâts " + std::to_string(summon.getMinDamage()) + "-" + std::to_string(summon.getMaxDamage()), true)
        );
        actionScreen.addOption(
            2,
            "Technique d'invocation",
            "Utilise l'effet spécial disponible.",
            true,
            "combat.summon.manual.wave.special",
            makeSummonActionItemData(summon, "special", "Technique d'invocation", "Utilise l'effet spécial disponible sans spoiler la cible.", "Technique disponible", true)
        );
        actionScreen.addOption(
            3,
            "Maintenir le lien",
            "+1 tour, pas d'attaque.",
            true,
            "combat.summon.manual.wave.maintain",
            makeSummonActionItemData(summon, "support", "Maintenir le lien", "Prolonge l'invocation au lieu de frapper cette cible.", "+1 tour")
        );
        actionScreen.addOption(
            4,
            "Sacrifier l'invocation",
            "Rupture violente du lien contre la cible.",
            summon.canBeSacrificed(),
            "combat.summon.manual.wave.sacrifice",
            makeSummonActionItemData(summon, "danger", "Sacrifier l'invocation", "Détruit l'invocation pour une rupture violente contre " + target.getName() + ".", summon.canBeSacrificed() ? "Dangereux" : "Impossible", true)
        );

        int actionChoice = TerminalInterface::askMenuChoiceFromOptions(
            actionScreen,
            "Choisis une action disponible."
        );
        Console::clear();

        if (actionChoice == 2)
        {
            executeSummonSpecialAbility(summon, target, random);
            return;
        }

        if (actionChoice == 3)
        {
            maintainSummonLink(summon);
            return;
        }

        if (actionChoice == 4)
        {
            sacrificeSummonAgainstEntity(summon, target, random);
            return;
        }
    }

    executeSummonAttack(summon, target, random);
    summon.decreaseDuration();
}


// EN: maintainSummonLink declares or implements a focused behavior used by this module.
// FR: maintainSummonLink déclare ou implémente un comportement précis utilisé par ce module.
bool SummonCombatSystem::maintainSummonLink(Summon& summon)
{
    int cost = summon.getMaintenanceCost();

    if (!spendSummonLink(summon))
    {
        showSummonNotice(
            "LIEN VIDE",
            "combat.summon.maintain.failed",
            {
                summon.getName() + " tente de renforcer le lien, mais la réserve d'invocation est vide.",
                "La durée de l'invocation diminue normalement."
            }
        );
        summon.decreaseDuration();
        return false;
    }

    summon.extendDuration(1);

    showSummonNotice(
        "LIEN MAINTENU",
        "combat.summon.maintain.result",
        {
            summon.getName() + " concentre son lien au lieu d'attaquer.",
            "Coût de maintien théorique : " + std::to_string(cost) + " point(s) de lien consommés.",
            "Durée restante : " + std::to_string(summon.getDurationTurns()) + " tour(s)."
        }
    );

    return true;
}

void SummonCombatSystem::sacrificeSummonAgainstEntity(
    Summon& summon,
    Entity& target,
    Random& random
)
{
    if (!summon.canBeSacrificed())
    {
        showSummonNotice(
            "SACRIFICE REFUSÉ",
            "combat.summon.sacrifice.refused",
            {
                summon.getName() + " refuse d'être sacrifiée : son lien n'est pas de cette nature.",
                "La durée de l'invocation diminue normalement."
            }
        );
        summon.decreaseDuration();
        return;
    }

    int hpBefore = target.getHp();
    int damage = random.between(summon.getMinDamage() + summon.getSlotCost() * 3, summon.getMaxDamage() + summon.getMaintenanceCost() * 6);

    if (summon.getName() == "Expérience instable")
    {
        damage += random.between(8, 18);
    }
    else if (summon.getName() == "Serviteur osseux" || summon.getName() == "Ombre récente")
    {
        damage += random.between(4, 10);
    }

    target.takeDamage(damage);
    summon.markSacrificed();

    showSummonNotice(
        "SACRIFICE D'INVOCATION",
        "combat.summon.sacrifice.result",
        {
            summon.getName() + " rompt volontairement son lien et se sacrifie.",
            target.getName() + " subit " + std::to_string(damage) + " dégâts de rupture d'invocation.",
            "PV cible : " + std::to_string(hpBefore) + " -> " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()) + "."
        }
    );
}

// EN: removeInactiveSummons declares or implements a focused behavior used by this module.
// FR: removeInactiveSummons déclare ou implémente un comportement précis utilisé par ce module.
void SummonCombatSystem::removeInactiveSummons(std::vector<Summon>& summons)
{
    std::vector<std::string> lines;
    int index = 0;

    while (index < static_cast<int>(summons.size()))
    {
        if (summons[index].isDead())
        {
            lines.push_back(summons[index].getName() + " disparaît après avoir été détruit.");
            summons.erase(summons.begin() + index);
            continue;
        }

        if (summons[index].isExpired())
        {
            lines.push_back(summons[index].getName() + " se dissipe : son lien d'invocation arrive à sa limite.");
            summons.erase(summons.begin() + index);
            continue;
        }

        ++index;
    }

    if (!lines.empty())
    {
        lines.push_back("Invocations encore actives : " + std::to_string(summons.size()) + ".");
        showSummonActionResult(
            "LIENS D'INVOCATION",
            "combat.summon.cleanup",
            lines
        );
    }
}


// EN: hasTargetableSummons declares or implements a focused behavior used by this module.
// FR: hasTargetableSummons déclare ou implémente un comportement précis utilisé par ce module.
bool SummonCombatSystem::hasTargetableSummons(const std::vector<Summon>& summons)
{
    return hasActiveSummons(summons);
}

int SummonCombatSystem::chooseRandomTargetableSummonIndex(
    const std::vector<Summon>& summons,
    Random& random
)
{
    std::vector<int> validIndexes;

    for (int i = 0; i < static_cast<int>(summons.size()); ++i)
    {
        if (!summons[i].isDead() && !summons[i].isExpired())
        {
            validIndexes.push_back(i);
        }
    }

    if (validIndexes.empty())
    {
        return -1;
    }

    int randomIndex = random.between(0, static_cast<int>(validIndexes.size()) - 1);
    return validIndexes[randomIndex];
}


int SummonCombatSystem::chooseStrategicTargetableSummonIndex(
    const std::vector<Summon>& summons,
    const Entity& attacker,
    Random& random
)
{
    std::vector<int> validIndexes;

    for (int i = 0; i < static_cast<int>(summons.size()); ++i)
    {
        if (!summons[i].isDead() && !summons[i].isExpired())
        {
            validIndexes.push_back(i);
        }
    }

    if (validIndexes.empty())
    {
        return -1;
    }

    std::string attackerName = attacker.getName();
    std::string attackerType = attacker.getType();

    if (attackerName.find("Loup") != std::string::npos
        || attackerType.find("Assassin") != std::string::npos)
    {
        int bestIndex = validIndexes.front();

        for (int index : validIndexes)
        {
            if (summons[index].getHp() < summons[bestIndex].getHp())
            {
                bestIndex = index;
            }
        }

        return bestIndex;
    }

    if (attackerName == "Hazak"
        || attackerName == "Skuro"
        || attackerName == "Trexof")
    {
        int bestIndex = validIndexes.front();

        for (int index : validIndexes)
        {
            if (summons[index].getMaxDamage() > summons[bestIndex].getMaxDamage())
            {
                bestIndex = index;
            }
        }

        return bestIndex;
    }

    if (attackerName == "Sanctus" || attackerType.find("Colosse") != std::string::npos)
    {
        int bestIndex = validIndexes.front();

        for (int index : validIndexes)
        {
            if (summons[index].getMaxHp() > summons[bestIndex].getMaxHp())
            {
                bestIndex = index;
            }
        }

        return bestIndex;
    }

    return chooseRandomTargetableSummonIndex(summons, random);
}

// EN: displayTargetableSummons declares or implements a focused behavior used by this module.
// FR: displayTargetableSummons déclare ou implémente un comportement précis utilisé par ce module.
void SummonCombatSystem::displayTargetableSummons(const std::vector<Summon>& summons)
{
    MenuScreen screen("INVOCATIONS CIBLABLES", "combat.summon.targetable.list");
    bool hasVisibleSummon = false;

    for (int i = 0; i < static_cast<int>(summons.size()); ++i)
    {
        const Summon& summon = summons[i];

        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        hasVisibleSummon = true;
        screen.addOption(
            i + 1,
            summon.getName(),
            std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp())
                + " PV | durée " + std::to_string(summon.getDurationTurns())
                + " tour(s) | slots " + std::to_string(summon.getSlotCost())
                + " | maintien " + std::to_string(summon.getMaintenanceCost()),
            true,
            "combat.summon.targetable." + std::to_string(i),
            makeTargetableSummonItemData(summon)
        );
    }

    if (!hasVisibleSummon)
    {
        screen.addLine("Aucune invocation ciblable.");
    }
    else
    {
        screen.addLine("Ces invocations peuvent être visées par les ennemis ou par certains effets.");
    }

    screen.setDisplayOnlyInput("Liste affichée sans saisie obligatoire.");
    TerminalInterface::renderMenuScreen(screen, false);
}

void SummonCombatSystem::entityAttacksSummon(
    Entity& attacker,
    Summon& summon,
    Random& random
)
{
    bool dodged = false;
    bool critical = false;
    int hpBefore = summon.getHp();

    int damage = attacker.attack(random, dodged, critical);

    if (dodged)
    {
        showSummonActionResult(
            "ATTAQUE SUR INVOCATION",
            "combat.summon.enemy_attack.dodged",
            {
                attacker.getName() + " vise " + summon.getName() + ".",
                "Résultat : l'invocation esquive."
            }
        );
        return;
    }

    if (SpecialCombatEffects::specialCharacterMissesBeforeDamage(
        attacker,
        random
    ))
    {
        return;
    }

    SpecialCombatEffects::applySpecialCharacterAttackBonus(
        attacker,
        random,
        damage,
        critical
    );

    summon.takeDamage(damage);

    std::vector<std::string> lines;
    lines.push_back(attacker.getName() + " frappe l'invocation " + summon.getName() + ".");
    lines.push_back(
        "Dégâts" + std::string(critical ? " critiques" : "") + " : "
        + std::to_string(damage) + "."
    );
    lines.push_back(
        "PV invocation : " + std::to_string(hpBefore) + " -> "
        + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp()) + "."
    );

    if (summon.isDead())
    {
        lines.push_back(summon.getName() + " se fissure sous le choc. Son lien avec le monde vacille.");
    }

    showSummonActionResult(
        "ATTAQUE SUR INVOCATION",
        "combat.summon.enemy_attack.result",
        lines
    );
}


// EN: hasSpecialAbility declares or implements a focused behavior used by this module.
// FR: hasSpecialAbility déclare ou implémente un comportement précis utilisé par ce module.
bool SummonCombatSystem::hasSpecialAbility(const Summon& summon)
{
    return summon.getName() == "Flamme kitsune"
        || summon.getName() == "Serviteur osseux"
        || summon.getName() == "Expérience instable"
        || summon.getName() == "Bête arcanique"
        || summon.getName() == "Esprit mineur"
        || summon.getName() == "Ombre récente"
        || summon.getName() == "Éclat zodiacal"
        || summon.getName() == "Totem gardien"
        || summon.getName() == "Corbeau familier"
        || summon.getName() == "Éclat de miroir";
}

void SummonCombatSystem::executeSummonAttack(
    Summon& summon,
    Entity& target,
    Random& random
)
{
    bool dodged = false;
    int hpBefore = target.getHp();
    int damage = rollSummonDamage(summon, random, dodged);

    if (dodged)
    {
        showSummonActionResult(
            "ATTAQUE D'INVOCATION",
            "combat.summon.attack.dodged",
            {
                summon.getName() + " attaque " + target.getName() + ".",
                "Résultat : la cible esquive."
            }
        );
        return;
    }

    target.takeDamage(damage);

    showSummonActionResult(
        "ATTAQUE D'INVOCATION",
        "combat.summon.attack.result",
        {
            summon.getName() + " frappe " + target.getName() + ".",
            "Dégâts infligés : " + std::to_string(damage) + ".",
            hpLine(target, hpBefore)
        }
    );
}

bool SummonCombatSystem::executeSummonSpecialAbility(
    Summon& summon,
    Entity& target,
    Random& random
)
{
    int hpBefore = target.getHp();

    if (summon.getName() == "Flamme kitsune")
    {
        int damage = random.between(summon.getMinDamage() + 3, summon.getMaxDamage() + 8);
        target.takeDamage(damage);
        summon.decreaseDuration();

        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.kitsune_flame",
            {
                "La Flamme kitsune danse autour de la cible.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s) brûlants.",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Serviteur osseux")
    {
        int selfHpBefore = summon.getHp();
        int selfDamage = summon.getMaxHp() / 4;
        int damage = random.between(summon.getMinDamage() + 4, summon.getMaxDamage() + 7);

        if (selfDamage < 1)
        {
            selfDamage = 1;
        }

        summon.takeDamage(selfDamage);
        target.takeDamage(damage);
        summon.decreaseDuration();

        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.bone_servant",
            {
                "Le Serviteur osseux sacrifie une partie de son ossature.",
                target.getName() + " reçoit " + std::to_string(damage) + " dégât(s).",
                "PV invocation : " + std::to_string(selfHpBefore) + " -> " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp()) + ".",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Expérience instable")
    {
        int damage = random.between(summon.getMinDamage() + 10, summon.getMaxDamage() + 18);
        target.takeDamage(damage);
        summon.takeDamage(summon.getMaxHp());
        summon.setDurationTurns(0);

        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.unstable_experiment",
            {
                "L'Expérience instable éclate dans un rire impossible.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s).",
                "L'invocation se détruit en rompant son propre lien.",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Bête arcanique")
    {
        int summonHpBefore = summon.getHp();
        int damage = random.between(summon.getMinDamage() + 2, summon.getMaxDamage() + 5);
        target.takeDamage(damage);
        summon.heal(4);
        summon.decreaseDuration();

        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.arcane_beast",
            {
                "La Bête arcanique mord et se nourrit du mana ambiant.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s).",
                "PV invocation : " + std::to_string(summonHpBefore) + " -> " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp()) + ".",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Esprit mineur")
    {
        int summonHpBefore = summon.getHp();
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage() + 3);
        target.takeDamage(damage);
        summon.heal(2);
        summon.decreaseDuration();

        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.minor_spirit",
            {
                "L'Esprit mineur pulse doucement avant l'impact.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s).",
                "PV invocation : " + std::to_string(summonHpBefore) + " -> " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp()) + ".",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Ombre récente")
    {
        int summonHpBefore = summon.getHp();
        int damage = random.between(summon.getMinDamage() + 5, summon.getMaxDamage() + 11);
        target.takeDamage(damage);
        std::vector<std::string> lines;

        if (random.between(1, 100) <= 35)
        {
            summon.heal(3);
            lines.push_back("L'Ombre récente mord dans une faille et se recompose légèrement.");
        }
        else
        {
            summon.decreaseDuration();
            lines.push_back("L'Ombre récente frappe sans bruit, puis son contour vacille.");
        }

        lines.push_back(target.getName() + " subit " + std::to_string(damage) + " dégât(s) d'ombre.");
        lines.push_back("PV invocation : " + std::to_string(summonHpBefore) + " -> " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp()) + ".");
        lines.push_back(hpLine(target, hpBefore));

        showSummonActionResult("TECHNIQUE D'INVOCATION", "combat.summon.special.recent_shadow", lines);
        return true;
    }

    if (summon.getName() == "Éclat zodiacal")
    {
        int sign = random.between(1, 13);
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage()) + sign;
        target.takeDamage(damage);
        std::vector<std::string> lines;

        if (sign == 13)
        {
            summon.setDurationTurns(summon.getDurationTurns() + 1);
            lines.push_back("Le treizième signe répond : l'éclat tient un tour de plus.");
        }
        else
        {
            summon.decreaseDuration();
        }

        lines.push_back("Signe tiré : " + std::to_string(sign) + "/13.");
        lines.push_back(target.getName() + " subit " + std::to_string(damage) + " dégât(s).");
        lines.push_back(hpLine(target, hpBefore));

        showSummonActionResult("TECHNIQUE D'INVOCATION", "combat.summon.special.zodiac_shard", lines);
        return true;
    }

    if (summon.getName() == "Totem gardien")
    {
        int summonHpBefore = summon.getHp();
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage() + 2);
        target.takeDamage(damage);
        summon.heal(5);
        summon.decreaseDuration();
        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.guardian_totem",
            {
                "Le Totem gardien absorbe une partie du choc et pulse vers la cible.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s).",
                "PV invocation : " + std::to_string(summonHpBefore) + " -> " + std::to_string(summon.getHp()) + "/" + std::to_string(summon.getMaxHp()) + ".",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Corbeau familier")
    {
        int damage = random.between(summon.getMinDamage() + 1, summon.getMaxDamage() + 5);
        target.takeDamage(damage);
        summon.decreaseDuration();
        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.familiar_raven",
            {
                "Le Corbeau familier pique les yeux de la cible et ouvre une fenêtre d'attaque.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s) précis.",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    if (summon.getName() == "Éclat de miroir")
    {
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage() + 8);
        target.takeDamage(damage);
        if (random.between(1, 100) <= 40) summon.extendDuration(1); else summon.decreaseDuration();
        showSummonActionResult(
            "TECHNIQUE D'INVOCATION",
            "combat.summon.special.mirror_shard",
            {
                "L'Éclat de miroir renvoie une image brisée de l'attaque à venir.",
                target.getName() + " subit " + std::to_string(damage) + " dégât(s) instables.",
                "Durée restante : " + std::to_string(summon.getDurationTurns()) + " tour(s).",
                hpLine(target, hpBefore)
            }
        );
        return true;
    }

    executeSummonAttack(summon, target, random);
    summon.decreaseDuration();
    return false;
}

int SummonCombatSystem::rollSummonDamage(
    const Summon& summon,
    Random& random,
    bool& dodged
)
{
    int roll = random.rollD20();

    dodged = roll <= 3;

    if (dodged)
    {
        return 0;
    }

    return random.between(
        summon.getMinDamage(),
        summon.getMaxDamage()
    );
}
