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

#include <iostream>
#include <map>


namespace
{
    std::map<std::string, int>& summonLinkPools()
    {
        static std::map<std::string, int> pools;
        return pools;
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

        std::cout << summon.getName()
                  << " vacille : "
                  << summon.getOwnerName()
                  << " n'a plus assez de lien d'invocation pour la maintenir."
                  << std::endl;
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

    std::cout << owner.getName()
              << " n'entre pas vraiment seul dans le combat."
              << std::endl;
    std::cout << "Slots d'invocation utilisés : "
              << calculateUsedSlots(summons)
              << "/"
              << maxSlots
              << std::endl;
    std::cout << "Lien d'invocation disponible : "
              << getSummonLinkRemaining(owner.getName())
              << " point(s) pour ce combat."
              << std::endl;

    for (const Summon& summon : summons)
    {
        std::cout << "- "
                  << summon.getName()
                  << " apparaît aux côtés de "
                  << owner.getName()
                  << " | slots : "
                  << summon.getSlotCost()
                  << " | maintien : "
                  << summon.getMaintenanceCost()
                  << " | durée : "
                  << summon.getDurationTurns()
                  << " tour(s)."
                  << std::endl;
    }

    std::cout << std::endl;
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

    std::cout << "Comment veux-tu gérer les invocations de "
              << owner.getName()
              << " ?"
              << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Attaque automatique" << std::endl;
    std::cout << "    Les invocations attaquent seules après ton tour." << std::endl;
    std::cout << std::endl;
    std::cout << "2 : Contrôle manuel" << std::endl;
    std::cout << "    Tu joues aussi tes invocations quand ton tour est terminé." << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        1,
        2,
        "Veuillez entrer 1 pour automatique ou 2 pour manuel."
    );

    std::cout << std::endl;

    if (choice == 2)
    {
        std::cout << "Contrôle manuel activé pour tes invocations." << std::endl;
        std::cout << "Elles suivront tes ordres tant que le lien d'invocation tient." << std::endl;
        std::cout << std::endl;
        return SummonControlMode::Manual;
    }

    std::cout << "Attaque automatique activée pour tes invocations." << std::endl;
    std::cout << "Elles agiront seules après ton tour." << std::endl;
    std::cout << std::endl;

    return SummonControlMode::Automatic;
}

void SummonCombatSystem::playSummonTurnsAgainstEntity(
    std::vector<Summon>& summons,
    Entity& target,
    Random& random
)
{
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

    std::cout << "========== TOUR DES INVOCATIONS ==========" << std::endl;

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

    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;

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

    std::cout << "========== TOUR DES INVOCATIONS ==========" << std::endl;

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

    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;

    removeInactiveSummons(summons);
}

void SummonCombatSystem::playManualSummonTurnAgainstEntity(
    Summon& summon,
    Entity& target,
    Random& random
)
{
    std::cout << summon.getName()
              << " attend ton ordre."
              << " Durée restante : "
              << summon.getDurationTurns()
              << " tour(s)."
              << std::endl;
    std::cout << "Lien restant pour "
              << summon.getOwnerName()
              << " : "
              << getSummonLinkRemaining(summon.getOwnerName())
              << " point(s)."
              << std::endl;
    std::cout << "0 : Ne rien faire" << std::endl;
    std::cout << "1 : Attaquer " << target.getName() << std::endl;

    int maxChoice = 1;

    if (hasSpecialAbility(summon))
    {
        std::cout << "2 : Technique d'invocation" << std::endl;
        maxChoice = 2;
    }

    std::cout << "3 : Maintenir le lien (+1 tour, pas d'attaque)" << std::endl;
    std::cout << "4 : Sacrifier l'invocation" << std::endl;
    maxChoice = 4;

    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        maxChoice,
        "Veuillez entrer un choix disponible."
    );

    if (choice == 0)
    {
        std::cout << summon.getName()
                  << " reste en retrait, mais le lien d'invocation continue de se consumer."
                  << std::endl;
        summon.decreaseDuration();
        return;
    }

    if (choice == 2)
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

    std::cout << summon.getName()
              << " attend ton ordre."
              << " Durée restante : "
              << summon.getDurationTurns()
              << " tour(s)."
              << std::endl;
    std::cout << std::endl;

    wave.displayActiveEnemies();

    std::cout << "0 : Ne rien faire" << std::endl;

    if (hasSpecialAbility(summon))
    {
        std::cout << "Astuce : choisis une cible, puis l'invocation pourra utiliser sa technique spéciale." << std::endl;
    }

    std::cout << "Choisis une cible pour l'invocation." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        wave.getActiveEnemyCount(),
        "Veuillez entrer 0 ou le numéro d'une cible active."
    );

    if (choice == 0)
    {
        std::cout << summon.getName()
                  << " reste en retrait, mais le lien d'invocation continue de se consumer."
                  << std::endl;
        summon.decreaseDuration();
        return;
    }

    Monster& target = wave.getActiveEnemy(choice - 1);

    if (hasSpecialAbility(summon))
    {
        std::cout << "1 : Attaque normale" << std::endl;
        std::cout << "2 : Technique d'invocation" << std::endl;
        std::cout << "3 : Maintenir le lien (+1 tour, pas d'attaque)" << std::endl;
        std::cout << "4 : Sacrifier l'invocation" << std::endl;
        std::cout << "> ";

        int actionChoice = Console::askNumberBetween(
            1,
            4,
            "Veuillez choisir une action disponible."
        );

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
        std::cout << summon.getName()
                  << " tente de renforcer le lien, mais la réserve d'invocation est vide."
                  << std::endl;
        summon.decreaseDuration();
        return false;
    }

    summon.extendDuration(1);

    std::cout << summon.getName()
              << " concentre son lien au lieu d'attaquer."
              << std::endl;
    std::cout << "Coût de maintien théorique : "
              << cost
              << " point(s) de lien consommés."
              << std::endl;
    std::cout << "Durée restante : "
              << summon.getDurationTurns()
              << " tour(s)."
              << std::endl;

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
        std::cout << summon.getName()
                  << " refuse d'être sacrifiée : son lien n'est pas de cette nature."
                  << std::endl;
        summon.decreaseDuration();
        return;
    }

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

    std::cout << summon.getName()
              << " rompt volontairement son lien et se sacrifie."
              << std::endl;
    std::cout << target.getName()
              << " subit "
              << damage
              << " dégâts de rupture d'invocation."
              << std::endl;
}

// EN: removeInactiveSummons declares or implements a focused behavior used by this module.
// FR: removeInactiveSummons déclare ou implémente un comportement précis utilisé par ce module.
void SummonCombatSystem::removeInactiveSummons(std::vector<Summon>& summons)
{
    int index = 0;

    while (index < static_cast<int>(summons.size()))
    {
        if (summons[index].isDead())
        {
            std::cout << summons[index].getName()
                      << " disparaît après avoir été détruit."
                      << std::endl;

            summons.erase(summons.begin() + index);
            continue;
        }

        if (summons[index].isExpired())
        {
            std::cout << summons[index].getName()
                      << " se dissipe : son lien d'invocation arrive à sa limite."
                      << std::endl;

            summons.erase(summons.begin() + index);
            continue;
        }

        ++index;
    }

    if (!summons.empty())
    {
        std::cout << std::endl;
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
    std::cout << "========== INVOCATIONS CIBLABLES ==========" << std::endl;

    bool hasVisibleSummon = false;

    for (int i = 0; i < static_cast<int>(summons.size()); ++i)
    {
        const Summon& summon = summons[i];

        if (summon.isDead() || summon.isExpired())
        {
            continue;
        }

        hasVisibleSummon = true;

        std::cout << i + 1
                  << " : "
                  << summon.getName()
                  << " | "
                  << summon.getHp()
                  << "/"
                  << summon.getMaxHp()
                  << " PV | durée : "
                  << summon.getDurationTurns()
                  << " tour(s) | slots : "
                  << summon.getSlotCost()
                  << " | maintien : "
                  << summon.getMaintenanceCost()
                  << std::endl;
    }

    if (!hasVisibleSummon)
    {
        std::cout << "Aucune invocation ciblable." << std::endl;
    }

    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;
}

void SummonCombatSystem::entityAttacksSummon(
    Entity& attacker,
    Summon& summon,
    Random& random
)
{
    bool dodged = false;
    bool critical = false;

    int damage = attacker.attack(random, dodged, critical);

    if (dodged)
    {
        std::cout << attacker.getName()
                  << " attaque "
                  << summon.getName()
                  << ", mais l'invocation esquive."
                  << std::endl;
        std::cout << std::endl;
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

    std::cout << attacker.getName()
              << " frappe l'invocation "
              << summon.getName()
              << " et inflige "
              << damage
              << " dégâts";

    if (critical)
    {
        std::cout << " critiques";
    }

    std::cout << "." << std::endl;

    if (summon.isDead())
    {
        std::cout << summon.getName()
                  << " se fissure sous le choc. Son lien avec le monde vacille."
                  << std::endl;
    }

    std::cout << std::endl;
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
    int damage = rollSummonDamage(summon, random, dodged);

    if (dodged)
    {
        std::cout << summon.getName()
                  << " attaque "
                  << target.getName()
                  << ", mais la cible esquive."
                  << std::endl;
        return;
    }

    target.takeDamage(damage);

    std::cout << summon.getName()
              << " frappe "
              << target.getName()
              << " et inflige "
              << damage
              << " dégâts."
              << std::endl;
}

bool SummonCombatSystem::executeSummonSpecialAbility(
    Summon& summon,
    Entity& target,
    Random& random
)
{
    if (summon.getName() == "Flamme kitsune")
    {
        int damage = random.between(summon.getMinDamage() + 3, summon.getMaxDamage() + 8);
        target.takeDamage(damage);
        summon.decreaseDuration();

        std::cout << "La Flamme kitsune danse autour de la cible."
                  << std::endl;
        std::cout << target.getName()
                  << " subit "
                  << damage
                  << " dégâts brûlants."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Serviteur osseux")
    {
        int selfDamage = summon.getMaxHp() / 4;
        int damage = random.between(summon.getMinDamage() + 4, summon.getMaxDamage() + 7);

        if (selfDamage < 1)
        {
            selfDamage = 1;
        }

        summon.takeDamage(selfDamage);
        target.takeDamage(damage);
        summon.decreaseDuration();

        std::cout << "Le Serviteur osseux sacrifie une partie de son ossature."
                  << std::endl;
        std::cout << target.getName()
                  << " reçoit "
                  << damage
                  << " dégâts. L'invocation perd "
                  << selfDamage
                  << " PV."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Expérience instable")
    {
        int damage = random.between(summon.getMinDamage() + 10, summon.getMaxDamage() + 18);
        target.takeDamage(damage);
        summon.takeDamage(summon.getMaxHp());
        summon.setDurationTurns(0);

        std::cout << "L'Expérience instable éclate dans un rire impossible."
                  << std::endl;
        std::cout << target.getName()
                  << " subit "
                  << damage
                  << " dégâts, mais l'invocation se détruit."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Bête arcanique")
    {
        int damage = random.between(summon.getMinDamage() + 2, summon.getMaxDamage() + 5);
        target.takeDamage(damage);
        summon.heal(4);
        summon.decreaseDuration();

        std::cout << "La Bête arcanique mord et se nourrit du mana ambiant."
                  << std::endl;
        std::cout << target.getName()
                  << " subit "
                  << damage
                  << " dégâts. L'invocation récupère quelques PV."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Esprit mineur")
    {
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage() + 3);
        target.takeDamage(damage);
        summon.heal(2);
        summon.decreaseDuration();

        std::cout << "L'Esprit mineur pulse doucement avant l'impact."
                  << std::endl;
        std::cout << target.getName()
                  << " subit "
                  << damage
                  << " dégâts."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Ombre récente")
    {
        int damage = random.between(summon.getMinDamage() + 5, summon.getMaxDamage() + 11);
        target.takeDamage(damage);

        if (random.between(1, 100) <= 35)
        {
            summon.heal(3);
            std::cout << "L'Ombre récente mord dans une faille et se recompose légèrement." << std::endl;
        }
        else
        {
            summon.decreaseDuration();
            std::cout << "L'Ombre récente frappe sans bruit, puis son contour vacille." << std::endl;
        }

        std::cout << target.getName()
                  << " subit "
                  << damage
                  << " dégâts d'ombre."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Éclat zodiacal")
    {
        int sign = random.between(1, 13);
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage()) + sign;
        target.takeDamage(damage);

        if (sign == 13)
        {
            summon.setDurationTurns(summon.getDurationTurns() + 1);
            std::cout << "Le treizième signe répond : l'éclat tient un tour de plus." << std::endl;
        }
        else
        {
            summon.decreaseDuration();
        }

        std::cout << "L'Éclat zodiacal tire le signe "
                  << sign
                  << "/13 et inflige "
                  << damage
                  << " dégâts."
                  << std::endl;
        return true;
    }

    if (summon.getName() == "Totem gardien")
    {
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage() + 2);
        target.takeDamage(damage);
        summon.heal(5);
        summon.decreaseDuration();
        std::cout << "Le Totem gardien absorbe une partie du choc et pulse vers la cible." << std::endl;
        std::cout << target.getName() << " subit " << damage << " dégâts. Le totem consolide son ancrage." << std::endl;
        return true;
    }

    if (summon.getName() == "Corbeau familier")
    {
        int damage = random.between(summon.getMinDamage() + 1, summon.getMaxDamage() + 5);
        target.takeDamage(damage);
        summon.decreaseDuration();
        std::cout << "Le Corbeau familier pique les yeux de la cible et ouvre une fenêtre d'attaque." << std::endl;
        std::cout << target.getName() << " subit " << damage << " dégâts précis." << std::endl;
        return true;
    }

    if (summon.getName() == "Éclat de miroir")
    {
        int damage = random.between(summon.getMinDamage(), summon.getMaxDamage() + 8);
        target.takeDamage(damage);
        if (random.between(1, 100) <= 40) summon.extendDuration(1); else summon.decreaseDuration();
        std::cout << "L'Éclat de miroir renvoie une image brisée de l'attaque future." << std::endl;
        std::cout << target.getName() << " subit " << damage << " dégâts instables." << std::endl;
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
