// EN: SpecialCombatEffects.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCombatEffects.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
#include <vector>

namespace
{
    std::set<std::string>& awakenedSanctusNames()
    {
        static std::set<std::string> names;
        return names;
    }

    // EN: specialAttackCounters stores lightweight per-combat attack rhythm counters for special characters.
    // FR: specialAttackCounters conserve de petits compteurs de rythme d'attaque pour les personnages spéciaux.
    std::map<std::string, int>& specialAttackCounters()
    {
        static std::map<std::string, int> counters;
        return counters;
    }


    // EN: specialGroupAllies remembers relationship groups created for special adventurer encounters.
    // FR: specialGroupAllies mémorise les groupes relationnels créés pour les rencontres spéciales d'aventuriers.
    std::map<std::string, std::set<std::string>>& specialGroupAllies()
    {
        static std::map<std::string, std::set<std::string>> allies;
        return allies;
    }

    // EN: usedSpecialUltimates remembers which special character ultimate already triggered during this runtime.
    // FR: usedSpecialUltimates mémorise quel ultime de personnage spécial a déjà été déclenché pendant cette session.
    std::set<std::string>& usedSpecialUltimates()
    {
        static std::set<std::string> names;
        return names;
    }

    std::string normalizeCombatName(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            // EN: [] declares or implements a focused behavior used by this module.
            // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }


    // EN: isGroupedWith checks whether an entity was registered in the same special group as another character.
    // FR: isGroupedWith vérifie si une entité a été enregistrée dans le même groupe spécial qu'un autre personnage.
    bool isGroupedWith(const Entity& entity, const std::string& allyName)
    {
        const std::string currentName = normalizeCombatName(entity.getName());
        const std::string normalizedAlly = normalizeCombatName(allyName);
        auto iterator = specialGroupAllies().find(currentName);

        if (iterator == specialGroupAllies().end())
        {
            return false;
        }

        return iterator->second.find(normalizedAlly) != iterator->second.end();
    }

    // EN: hasUsedSpecialUltimate checks whether this special character already consumed its unique emergency ultimate.
    // FR: hasUsedSpecialUltimate vérifie si ce personnage spécial a déjà consommé son ultime d'urgence unique.
    bool hasUsedSpecialUltimate(const Entity& entity)
    {
        return usedSpecialUltimates().find(normalizeCombatName(entity.getName())) != usedSpecialUltimates().end();
    }

    // EN: markSpecialUltimateUsed marks the emergency ultimate as consumed for this special character.
    // FR: markSpecialUltimateUsed marque l'ultime d'urgence comme consommé pour ce personnage spécial.
    void markSpecialUltimateUsed(const Entity& entity)
    {
        usedSpecialUltimates().insert(normalizeCombatName(entity.getName()));
    }

    // EN: shouldTriggerSpecialUltimate determines whether a low-health special character can trigger its emergency ultimate.
    // FR: shouldTriggerSpecialUltimate détermine si un personnage spécial bas en PV peut déclencher son ultime d'urgence.
    bool shouldTriggerSpecialUltimate(const Entity& entity)
    {
        return entity.getHp() * 100 <= entity.getMaxHp() * 35 && !hasUsedSpecialUltimate(entity);
    }

    // EN: isAwakenedSanctus declares or implements a focused behavior used by this module.
    // FR: isAwakenedSanctus déclare ou implémente un comportement précis utilisé par ce module.
    bool isAwakenedSanctus(const Entity& entity)
    {
        return awakenedSanctusNames().find(normalizeCombatName(entity.getName())) != awakenedSanctusNames().end();
    }

    // EN: awakenSanctusIfNeeded declares or implements a focused behavior used by this module.
    // FR: awakenSanctusIfNeeded déclare ou implémente un comportement précis utilisé par ce module.
    void awakenSanctusIfNeeded(Entity& entity, const std::string& reason)
    {
        std::string normalized = normalizeCombatName(entity.getName());

        if (awakenedSanctusNames().find(normalized) != awakenedSanctusNames().end())
        {
            return;
        }

        awakenedSanctusNames().insert(normalized);
        entity.startProvocation(3);

        std::cout << "La lumière de Sanctus se fend net." << std::endl;
        std::cout << reason << std::endl;
        std::cout << "Skuro ne remplace pas Sanctus : il remonte à travers lui." << std::endl;
        std::cout << "Éveil instable : attaques plus lourdes, défense plus agressive, mais coups moins fiables." << std::endl;
        std::cout << std::endl;
    }
}

void SpecialCombatEffects::registerSpecialGroupContext(const std::vector<std::string>& names)
{
    std::vector<std::string> normalizedNames;

    for (const std::string& name : names)
    {
        normalizedNames.push_back(normalizeCombatName(name));
    }

    for (const std::string& name : normalizedNames)
    {
        std::set<std::string>& allies = specialGroupAllies()[name];

        for (const std::string& ally : normalizedNames)
        {
            if (ally != name)
            {
                allies.insert(ally);
            }
        }
    }
}

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

    if (isName(defender, "Aoi") && random.between(1, 100) <= 16)
    {
        std::cout << "Aoi protège son cercle d'incantation avec une petite flamme kitsune." << std::endl;
        std::cout << "L'attaque glisse sur le sort au lieu de la toucher directement." << std::endl;
        std::cout << std::endl;
        return true;
    }

    if (isName(defender, "Matt (PRO)") && random.between(1, 100) <= 14)
    {
        std::cout << "Matt (PRO) anticipe le timing comme s'il connaissait déjà la frame exacte." << std::endl;
        std::cout << std::endl;
        return true;
    }

    if (isName(defender, "Trexof") && isUnderHalfHp(defender) && random.between(1, 100) <= 18)
    {
        std::cout << "Trexof recule d'un pas : réflexe de bêta testeur, bug évité." << std::endl;
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
    if (isName(attacker, "Sanctus") && isAwakenedSanctus(attacker))
    {
        if (random.between(1, 100) <= 20)
        {
            std::cout << "Sanctus frappe comme si Skuro tenait son bras..." << std::endl;
            std::cout << "Mais cette violence n'est pas encore stable, et le coup passe à côté." << std::endl;
            std::cout << std::endl;
            return true;
        }

        return false;
    }

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
    std::map<std::string, int>& attackCounters = specialAttackCounters();
    attackCounters[attacker.getName()]++;

    if (isName(attacker, "Skuro"))
    {
        int bonus = random.between(18, 34);

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            bonus += 38;
            critical = true;
            std::cout << "Ultime spécial - Skuro : Fente de la matière." << std::endl;
            std::cout << "Il ne cherche plus un adversaire. Il cherche l'endroit exact où le monde se coupe." << std::endl;
        }

        rawDamage += bonus;

        std::cout << "Skuro trouve enfin la matière. Son tranchant réclame "
                  << bonus
                  << " dégâts bruts supplémentaires."
                  << std::endl;
    }

    if (isName(attacker, "Sanctus") && isAwakenedSanctus(attacker))
    {
        int bonus = random.between(12, 24);
        rawDamage += bonus;

        if (random.between(1, 100) <= 28)
        {
            critical = true;
        }

        std::cout << "La protection de Sanctus se retourne en sentence. Skuro pousse sous la lumière : +"
                  << bonus
                  << " dégâts bruts."
                  << std::endl;
    }

    if (isName(attacker, "Hestia"))
    {
        hestiaSpellCounter[attacker.getName()]++;
        int bonus = hestiaSpellCounter[attacker.getName()] * 4;

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            bonus += 30;
            std::cout << "Ultime spécial - Hestia : Dôme d'étoile oubliée." << std::endl;
            std::cout << "Elle tremble, mais le sort ne tremble pas avec elle." << std::endl;
        }

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

        if (zodiacRoll == 13 || shouldTriggerSpecialUltimate(attacker))
        {
            if (shouldTriggerSpecialUltimate(attacker))
            {
                markSpecialUltimateUsed(attacker);
            }

            bonus += 22;
            std::cout << "Kanadé hurle sur le ciel, et le treizième signe répond trop fort." << std::endl;
            std::cout << "Ultime spécial - Kanadé : Constellation rageuse." << std::endl;
        }

        rawDamage += bonus;

        std::cout << "Kanadé râle contre son propre sort, puis le zodiaque répond quand même." << std::endl;
        std::cout << "Signe tiré : " << zodiacRoll << "/13, bonus : +" << bonus << " dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Fail"))
    {
        int experiment = random.between(1, 100);

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            rawDamage += 36;
            std::cout << "Ultime spécial - Fail : Prototype interdit." << std::endl;
            std::cout << "Fail : Si ça explose, c'est que ça prouve quelque chose." << std::endl;
        }
        else if (experiment <= 20)
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
        int bonus = random.between(2, 7);

        if (attackCounters[attacker.getName()] % 3 == 0)
        {
            bonus += 12;
            std::cout << "Louis déclenche une salve préparée : ce n'est pas très propre, mais ça part de partout." << std::endl;
        }

        rawDamage += bonus;
        std::cout << "Louis ajuste son bricolage offensif : quelques projectiles de plus partent presque au bon endroit." << std::endl;
    }

    if (isName(attacker, "Hazak"))
    {
        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            critical = true;
            rawDamage += 32;
            std::cout << "Ultime spécial - Hazak : Contrat de silence." << std::endl;
            std::cout << "Hazak : Le combat faisait trop de bruit." << std::endl;
        }
        else if (random.between(1, 100) <= 25)
        {
            critical = true;
            rawDamage += 10;
            std::cout << "Hazak ne cherche pas le spectacle. Il cherche l'ouverture. +10 dégâts bruts." << std::endl;
        }
    }

    if (isName(attacker, "Trexof"))
    {
        int testRoll = random.between(1, 100);

        if (testRoll <= 22)
        {
            rawDamage += 14;
            std::cout << "Trexof repère une faille d'équilibrage et l'exploite proprement : +14 dégâts bruts." << std::endl;
        }
        else if (testRoll <= 30)
        {
            rawDamage += 4;
            std::cout << "Trexof corrige son angle en plein test : +4 dégâts bruts." << std::endl;
        }
    }

    if (isName(attacker, "Matt (PRO)"))
    {
        if (random.between(1, 100) <= 20)
        {
            critical = true;
            rawDamage += 8;
            std::cout << "Matt (PRO) joue proprement, sans panique : critique contrôlé." << std::endl;
        }
    }

    if (isName(attacker, "Mattzelda"))
    {
        rawDamage += random.between(6, 13);
        std::cout << "Mattzelda transforme sa blague en charge de colosse. Le coup pèse plus lourd que prévu." << std::endl;
    }

    if (isName(attacker, "Aoi") && random.between(1, 100) <= 28)
    {
        int bonus = 9;

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            bonus += 24;
            std::cout << "Ultime spécial - Aoi : Cercle des flammes kitsune." << std::endl;
            std::cout << "Aoi baisse les yeux, puis laisse les flammes parler à sa place." << std::endl;
        }

        rawDamage += bonus;
        std::cout << "Aoi ose libérer une flamme kitsune plus stable : +" << bonus << " dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Hazak") && isGroupedWith(attacker, "Henrique"))
    {
        rawDamage += 6;
        std::cout << "Henrique garde le rythme de Hazak. L'ouverture devient plus propre : +6 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Henrique") && isGroupedWith(attacker, "Hazak"))
    {
        rawDamage += 5;
        std::cout << "Hazak couvre l'angle mort d'Henrique : +5 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Hazak") && isGroupedWith(attacker, "Hestia"))
    {
        rawDamage += 8;
        std::cout << "Hazak frappe plus froidement tant qu'Hestia est dans l'arène : +8 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Aoi") && isGroupedWith(attacker, "Kanadé"))
    {
        rawDamage += 5;
        std::cout << "La rage de Kanadé stabilise la flamme d'Aoi : +5 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Kanadé") && isGroupedWith(attacker, "Aoi"))
    {
        rawDamage += 5;
        std::cout << "Aoi protège l'incantation de Kanadé avec une flamme discrète : +5 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Louis") && isGroupedWith(attacker, "Trexof"))
    {
        rawDamage += 4;
        std::cout << "Trexof signale une trajectoire à Louis : +4 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Trexof") && isGroupedWith(attacker, "Mattzelda"))
    {
        rawDamage += 4;
        std::cout << "Mattzelda attire l'attention assez longtemps pour que Trexof teste une ouverture : +4 dégâts bruts." << std::endl;
    }

    if (isName(attacker, "Fail") && isGroupedWith(attacker, "Hazak"))
    {
        rawDamage += 5;
        std::cout << "Le contrat de non-agression force Fail à viser ailleurs que sur Hazak : +5 dégâts bruts utiles." << std::endl;
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

    if (isName(defender, "Hestia") && isGroupedWith(defender, "Hazak") && random.between(1, 100) <= 35)
    {
        int counterDamage = random.between(5, 12);
        attacker.takeDamage(counterDamage);

        std::cout << "Hazak ne laisse pas l'impact sur Hestia devenir gratuit." << std::endl;
        std::cout << attacker.getName() << " subit " << counterDamage << " dégâts de riposte silencieuse." << std::endl;
        std::cout << std::endl;
    }

    if (isName(attacker, "Kanadé") && isGroupedWith(attacker, "Aoi") && random.between(1, 100) <= 22)
    {
        int extraDamage = random.between(5, 10);
        defender.takeDamage(extraDamage);

        std::cout << "Une flamme kitsune d'Aoi suit le signe de Kanadé et explose avec retard : "
                  << extraDamage
                  << " dégâts supplémentaires."
                  << std::endl;
        std::cout << std::endl;
    }

    if (isName(attacker, "Mattzelda") && random.between(1, 100) <= 25)
    {
        attacker.heal(6);

        std::cout << "Mattzelda rigole après l'impact et reprend un peu son souffle : +6 PV." << std::endl;
        std::cout << std::endl;
    }

    if (isName(attacker, "Trexof") && random.between(1, 100) <= 18)
    {
        int extraDamage = random.between(4, 9);
        defender.takeDamage(extraDamage);

        std::cout << "Trexof valide un second test sur la même ouverture : "
                  << extraDamage
                  << " dégâts supplémentaires."
                  << std::endl;
        std::cout << std::endl;
    }

    if (isName(attacker, "Sanctus") && defender.getMaxHp() > 0)
    {
        int violentDealtThreshold = defender.getMaxHp() * 40 / 100;

        if (receivedDamage >= violentDealtThreshold)
        {
            awakenSanctusIfNeeded(
                attacker,
                "Il vient de protéger quelqu'un avec un coup beaucoup trop proche d'une exécution."
            );
        }
        else if (isAwakenedSanctus(attacker) && random.between(1, 100) <= 25)
        {
            attacker.startProvocation(2);

            std::cout << "Sanctus retient Skuro de justesse, mais l'ennemi sent la menace." << std::endl;
            std::cout << "Provocation : le regard revient sur lui." << std::endl;
            std::cout << std::endl;
        }
    }
}

void SpecialCombatEffects::applySpecialCharacterAfterReceivingDamage(
    Entity& defender,
    int receivedDamage,
    Random& random
)
{
    static std::set<std::string> henriqueReviveUsed;
    static std::set<std::string> hestiaDomeUsed;

    if (isName(defender, "Hestia") && defender.isDead())
    {
        if (hestiaDomeUsed.find(defender.getName()) == hestiaDomeUsed.end())
        {
            hestiaDomeUsed.insert(defender.getName());
            defender.reviveWithHealthPercentage(35);

            std::cout << "Hestia disparaît presque derrière la peur..." << std::endl;
            std::cout << "Puis un dôme ancien se rallume tout seul, comme si quelque chose refusait de la laisser tomber." << std::endl;
            std::cout << "Protection spéciale : retour unique à "
                      << defender.getHp()
                      << "/"
                      << defender.getMaxHp()
                      << " PV."
                      << std::endl;
            std::cout << std::endl;
        }
    }

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
            awakenSanctusIfNeeded(
                defender,
                "Il a encaissé un choc trop brutal pour rester seulement un bouclier."
            );
        }

        if (shouldTriggerSpecialUltimate(defender))
        {
            markSpecialUltimateUsed(defender);
            defender.startProvocation(3);
            defender.startDefensePosture(35, 18, "Mur sacré de Sanctus");

            std::cout << "Ultime spécial - Sanctus : Rempart de croyance." << std::endl;
            std::cout << "Sanctus attire la menace sur lui et transforme sa foi en posture de défense." << std::endl;
            std::cout << std::endl;
        }
    }

    if (isName(defender, "Mattzelda") && shouldTriggerSpecialUltimate(defender))
    {
        markSpecialUltimateUsed(defender);
        defender.heal(defender.getMaxHp() * 18 / 100);
        defender.startDefensePosture(30, 10, "Blague de colosse beaucoup trop solide");

        std::cout << "Ultime spécial - Mattzelda : Mur de blagues." << std::endl;
        std::cout << "Il rigole tellement fort que même les dégâts hésitent à continuer." << std::endl;
        std::cout << std::endl;
    }

    if (isName(defender, "Aoi") && isGroupedWith(defender, "Sanctus") && random.between(1, 100) <= 18)
    {
        defender.heal(5);
        std::cout << "Sanctus détourne une partie de la menace : Aoi récupère 5 PV en gardant son cercle." << std::endl;
        std::cout << std::endl;
    }

    if (isName(defender, "Kanadé") && isGroupedWith(defender, "Sanctus") && random.between(1, 100) <= 18)
    {
        defender.heal(5);
        std::cout << "Sanctus encaisse juste assez pour que Kanadé garde sa colère utile : +5 PV." << std::endl;
        std::cout << std::endl;
    }

    if (isName(defender, "Hestia") && isGroupedWith(defender, "Hazak") && random.between(1, 100) <= 25)
    {
        defender.startDefensePosture(22, 6, "Protection de Hazak autour d'Hestia");
        std::cout << "Hazak déplace le combat autour d'Hestia. Sa prochaine garde devient plus sûre." << std::endl;
        std::cout << std::endl;
    }

    if (isName(defender, "Louis") && isGroupedWith(defender, "Mattzelda") && random.between(1, 100) <= 18)
    {
        defender.heal(4);
        std::cout << "Mattzelda fait écran avec une blague beaucoup trop bruyante. Louis reprend 4 PV." << std::endl;
        std::cout << std::endl;
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
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    std::transform(
        expected.begin(),
        expected.end(),
        expected.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    return current == expected;
}

// EN: isUnderHalfHp declares or implements a focused behavior used by this module.
// FR: isUnderHalfHp déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCombatEffects::isUnderHalfHp(const Entity& entity)
{
    return entity.getHp() * 2 <= entity.getMaxHp();
}
