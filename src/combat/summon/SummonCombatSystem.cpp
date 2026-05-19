// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Handles active summons during combat and lets summoner classes slowly turn duels into small group fights.

#include "combat/summon/SummonCombatSystem.hpp"

#include "combat/summon/SummonCatalog.hpp"
#include "combat/summon/SummonRules.hpp"
#include "combat/action/SpecialCombatEffects.hpp"
#include "core/Console.hpp"

#include <iostream>

std::vector<Summon> SummonCombatSystem::createInitialSummonsFor(const Player& owner)
{
    if (!SummonRules::classCanSummon(owner.getType()) && owner.getName() != "Aoi")
    {
        return {};
    }

    return SummonCatalog::createStarterSummonsForClass(
        owner.getName(),
        owner.getType()
    );
}

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

void SummonCombatSystem::displaySummonArrival(
    const Player& owner,
    const std::vector<Summon>& summons
)
{
    if (summons.empty())
    {
        return;
    }

    std::cout << owner.getName()
              << " n'entre pas vraiment seul dans le combat."
              << std::endl;

    for (const Summon& summon : summons)
    {
        std::cout << "- "
                  << summon.getName()
                  << " apparaît aux côtés de "
                  << owner.getName()
                  << "."
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

        bool dodged = false;
        int damage = rollSummonDamage(summon, random, dodged);

        if (dodged)
        {
            std::cout << summon.getName()
                      << " attaque "
                      << target.getName()
                      << ", mais la cible esquive."
                      << std::endl;
        }
        else
        {
            target.takeDamage(damage);

            std::cout << summon.getName()
                      << " frappe "
                      << target.getName()
                      << " et inflige "
                      << damage
                      << " dégâts."
                      << std::endl;
        }

        summon.decreaseDuration();
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

        bool dodged = false;
        int damage = rollSummonDamage(summon, random, dodged);

        if (dodged)
        {
            std::cout << summon.getName()
                      << " attaque "
                      << target.getName()
                      << ", mais la cible esquive."
                      << std::endl;
        }
        else
        {
            target.takeDamage(damage);

            std::cout << summon.getName()
                      << " frappe "
                      << target.getName()
                      << " et inflige "
                      << damage
                      << " dégâts."
                      << std::endl;
        }

        summon.decreaseDuration();
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
    std::cout << "0 : Ne rien faire" << std::endl;
    std::cout << "1 : Attaquer " << target.getName() << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        1,
        "Veuillez entrer 0 ou 1."
    );

    if (choice == 0)
    {
        std::cout << summon.getName()
                  << " reste en retrait, mais le lien d'invocation continue de se consumer."
                  << std::endl;
        summon.decreaseDuration();
        return;
    }

    bool dodged = false;
    int damage = rollSummonDamage(summon, random, dodged);

    if (dodged)
    {
        std::cout << summon.getName()
                  << " attaque "
                  << target.getName()
                  << ", mais la cible esquive."
                  << std::endl;
    }
    else
    {
        target.takeDamage(damage);

        std::cout << summon.getName()
                  << " frappe "
                  << target.getName()
                  << " et inflige "
                  << damage
                  << " dégâts."
                  << std::endl;
    }

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

    bool dodged = false;
    int damage = rollSummonDamage(summon, random, dodged);

    if (dodged)
    {
        std::cout << summon.getName()
                  << " attaque "
                  << target.getName()
                  << ", mais la cible esquive."
                  << std::endl;
    }
    else
    {
        target.takeDamage(damage);

        std::cout << summon.getName()
                  << " frappe "
                  << target.getName()
                  << " et inflige "
                  << damage
                  << " dégâts."
                  << std::endl;
    }

    summon.decreaseDuration();
}

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
                  << " tour(s)"
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
