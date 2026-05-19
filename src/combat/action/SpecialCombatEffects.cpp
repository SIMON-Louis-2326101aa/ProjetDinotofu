// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Centralizes special combat hooks for bosses and special characters.

#include "combat/action/SpecialCombatEffects.hpp"

#include "core/Console.hpp"

#include "entity/Boss.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <string>

bool SpecialCombatEffects::atlasBlocksAttack(
    Entity& attacker,
    Entity& defender,
    int damage
)
{
    Boss* bossDefenseur = dynamic_cast<Boss*>(&defender);

    if (bossDefenseur == nullptr)
    {
        return false;
    }

    if (bossDefenseur->getBossId() != 3 || !bossDefenseur->isUltimateActive())
    {
        return false;
    }

    int reflectedDamage = damage / 3;

    attacker.takeDamage(reflectedDamage);

    std::cout << attacker.getName()
              << " frappe de toutes ses forces..."
              << std::endl;

    Console::pauseSeconds(1);

    std::cout << "Mais l'armure d'"
              << bossDefenseur->getName()
              << " absorbe l'impact."
              << std::endl;

    std::cout << "Une partie de la puissance est renvoyée à "
              << attacker.getName()
              << ", qui subit "
              << reflectedDamage
              << " dégâts."
              << std::endl;

    std::cout << std::endl;

    std::cout << attacker.getName()
              << " possède maintenant "
              << attacker.getHp()
              << "/"
              << attacker.getMaxHp()
              << " PV."
              << std::endl;

    std::cout << std::endl;

    return true;
}

void SpecialCombatEffects::applyDemonLifestealIfNeeded(
    Entity& attacker,
    int damageDealt
)
{
    Boss* attackingBoss = dynamic_cast<Boss*>(&attacker);

    if (attackingBoss == nullptr)
    {
        return;
    }

    if (attackingBoss->getBossId() != 2 || !attackingBoss->isUltimateActive())
    {
        return;
    }

    if (attackingBoss->getSpecialEffect() != 2
        && attackingBoss->getSpecialEffect() != 3)
    {
        return;
    }

    int healing = damageDealt * 50 / 100;

    if (healing <= 0)
    {
        return;
    }

    attackingBoss->heal(healing);

    std::cout << attackingBoss->getName()
              << " absorbe le sang de l'attaque et récupère "
              << healing
              << " PV."
              << std::endl;

    std::cout << std::endl;
}

bool SpecialCombatEffects::specialCharacterDodgesBeforeDamage(
    Entity& defender,
    Random& random
)
{
    if (isName(defender, "Fire Flight") && isUnderHalfHp(defender))
    {
        if (random.between(1, 100) <= 55)
        {
            std::cout << "Fire Flight lit le mouvement au dernier instant." << std::endl;
            std::cout << "Sous 50% PV, son instinct de commandant prend le relais : l'attaque est esquivée." << std::endl;
            std::cout << std::endl;
            return true;
        }
    }

    if (isName(defender, "Hestia") && random.between(1, 100) <= 18)
    {
        std::cout << "Hestia ferme les yeux de peur... et esquive presque par accident." << std::endl;
        std::cout << std::endl;
        return true;
    }

    return false;
}

bool SpecialCombatEffects::specialCharacterMissesBeforeDamage(
    Entity& attacker,
    Random& random
)
{
    if (!isName(attacker, "Skuro"))
    {
        return false;
    }

    if (random.between(1, 100) <= 35)
    {
        std::cout << "Skuro abat son épée à deux mains avec une violence ridicule..." << std::endl;
        std::cout << "Mais le coup est trop lourd, trop avide, et fend seulement l'air." << std::endl;
        std::cout << std::endl;
        return true;
    }

    return false;
}

void SpecialCombatEffects::applySpecialCharacterAttackBonus(
    Entity& attacker,
    Random& random,
    int& rawDamage,
    bool& critical
)
{
    static std::map<std::string, int> hestiaSpellCounter;

    if (isName(attacker, "Skuro"))
    {
        int bonus = random.between(18, 34);
        rawDamage += bonus;

        std::cout << "Skuro trouve enfin la matière. Son tranchant réclame "
                  << bonus
                  << " dégâts bruts supplémentaires."
                  << std::endl;
    }

    if (isName(attacker, "Hestia"))
    {
        hestiaSpellCounter[attacker.getName()]++;
        int bonus = hestiaSpellCounter[attacker.getName()] * 4;
        rawDamage += bonus;

        std::cout << "La magie d'Hestia résonne malgré sa peur."
                  << std::endl;
        std::cout << "Chaque incantation, ratée ou non, renforce la suivante : +"
                  << bonus
                  << " dégâts bruts."
                  << std::endl;
    }

    if (isName(attacker, "Kanadé"))
    {
        int zodiacRoll = random.between(1, 13);
        int bonus = zodiacRoll + random.between(0, 8);
        rawDamage += bonus;

        std::cout << "Kanadé râle contre son propre sort, puis le zodiaque répond quand même." << std::endl;
        std::cout << "Signe tiré : " << zodiacRoll << "/13, bonus : +" << bonus << " dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Fail"))
    {
        int experiment = random.between(1, 100);

        if (experiment <= 20)
        {
            rawDamage += 20;
            std::cout << "Fail sourit : l'expérience est instable, donc parfaite. +20 dégâts bruts." << std::endl;
        }
        else if (experiment <= 35)
        {
            rawDamage -= 8;
            if (rawDamage < 0)
            {
                rawDamage = 0;
            }
            std::cout << "Fail note quelque chose dans sa tête : l'expérience fonctionne... moins bien que prévu. -8 dégâts bruts." << std::endl;
        }
    }

    if (isName(attacker, "Fire Flight") && isUnderHalfHp(attacker))
    {
        if (random.between(1, 100) <= 70)
        {
            critical = true;
            rawDamage += attacker.getCriticalDamage() / 2;

            std::cout << "Fire Flight passe sous le seuil critique." << std::endl;
            std::cout << "Ses ordres deviennent plus froids, plus précis : critique renforcé." << std::endl;
        }
    }

    if (isName(attacker, "Louis"))
    {
        rawDamage += random.between(2, 7);
        std::cout << "Louis ajuste son bricolage offensif : quelques projectiles de plus partent presque au bon endroit." << std::endl;
    }

    if (isName(attacker, "Hazak"))
    {
        if (random.between(1, 100) <= 25)
        {
            critical = true;
            rawDamage += 10;
            std::cout << "Hazak ne cherche pas le spectacle. Il cherche l'ouverture. +10 dégâts bruts." << std::endl;
        }
    }

    if (rawDamage < 0)
    {
        rawDamage = 0;
    }
}

void SpecialCombatEffects::applySpecialCharacterAfterDamage(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int receivedDamage
)
{
    if (receivedDamage <= 0)
    {
        return;
    }

    if (isName(attacker, "Louis"))
    {
        int extraProjectiles = random.between(1, 3);
        int extraDamage = extraProjectiles * random.between(2, 4);

        defender.takeDamage(extraDamage);

        std::cout << "Louis enchaîne avec "
                  << extraProjectiles
                  << " projectile(s) secondaire(s). "
                  << defender.getName()
                  << " subit "
                  << extraDamage
                  << " dégâts supplémentaires."
                  << std::endl;
        std::cout << std::endl;
    }

    if (isName(attacker, "Sanctus") && receivedDamage >= 45)
    {
        attacker.startProvocation(2);

        std::cout << "Sanctus reste immobile après ce coup beaucoup trop violent." << std::endl;
        std::cout << "Pendant une seconde, son regard n'a plus rien de protecteur." << std::endl;
        std::cout << "Quelque chose ressemblant à Skuro vient de bouger sous la surface." << std::endl;
        std::cout << "Provocation : l'ennemi comprend qu'il doit répondre à Sanctus." << std::endl;
        std::cout << std::endl;
    }
}

void SpecialCombatEffects::applySpecialCharacterAfterReceivingDamage(
    Entity& defender,
    int receivedDamage,
    Random& random
)
{
    static std::set<std::string> henriqueReviveUsed;

    if (isName(defender, "Henrique") && defender.isDead())
    {
        if (henriqueReviveUsed.find(defender.getName()) == henriqueReviveUsed.end())
        {
            henriqueReviveUsed.insert(defender.getName());
            defender.reviveWithHealthPercentage(45);

            std::cout << "Henrique tombe... puis se relève une fois de plus." << std::endl;
            std::cout << "Sa capacité spéciale s'active : retour unique à "
                      << defender.getHp()
                      << "/"
                      << defender.getMaxHp()
                      << " PV."
                      << std::endl;
            std::cout << std::endl;
        }
    }

    if (isName(defender, "Sanctus"))
    {
        int heavyHitThreshold = defender.getMaxHp() * 35 / 100;

        if (receivedDamage >= heavyHitThreshold && receivedDamage > 0)
        {
            defender.startProvocation(2);

            std::cout << "Sanctus encaisse un choc trop brutal." << std::endl;
            std::cout << "La protection vacille. Skuro gratte contre les murs de sa propre âme." << std::endl;
            std::cout << "Transformation future préparée : Sanctus pourra basculer si la pression devient trop forte." << std::endl;
            std::cout << "Provocation : Sanctus attire la pression sur lui pour protéger les autres." << std::endl;
            std::cout << std::endl;
        }
    }

    if (isName(defender, "Aoi") && random.between(1, 100) <= 20)
    {
        std::cout << "Aoi recule, timide, mais protège instinctivement ses incantations." << std::endl;
        std::cout << "Son prochain système de protection magique sera branché avec les sorts." << std::endl;
        std::cout << std::endl;
    }
}

bool SpecialCombatEffects::isName(
    const Entity& entity,
    const std::string& expectedName
)
{
    std::string current = entity.getName();
    std::string expected = expectedName;

    std::transform(
        current.begin(),
        current.end(),
        current.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    std::transform(
        expected.begin(),
        expected.end(),
        expected.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    return current == expected;
}

bool SpecialCombatEffects::isUnderHalfHp(const Entity& entity)
{
    return entity.getHp() * 2 <= entity.getMaxHp();
}
