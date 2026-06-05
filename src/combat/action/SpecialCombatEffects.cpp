// EN: SpecialCombatEffects.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCombatEffects.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Centralizes special combat hooks for bosses and special characters.

#include "combat/action/SpecialCombatEffects.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

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

    void showSpecialCombatMessage(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MessageScreen::show(title, screenId, lines, false);
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

        showSpecialCombatMessage(
            "ÉVEIL INSTABLE",
            "combat.special.sanctus.awakening",
            {
                "La lumière de Sanctus se fend net.",
                reason,
                "Skuro ne remplace pas Sanctus : il remonte à travers lui.",
                "Éveil instable : attaques plus lourdes, défense plus agressive, mais coups moins fiables."
            }
        );
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

    showSpecialCombatMessage(
        "RENVOI D'ATLAS",
        "combat.special.atlas.reflect",
        {
            attacker.getName() + " frappe de toutes ses forces...",
            "Mais l'armure d'" + bossDefenseur->getName() + " absorbe l'impact.",
            "Une partie de la puissance est renvoyée : " + std::to_string(reflectedDamage) + " dégâts.",
            attacker.getName() + " possède maintenant " + std::to_string(attacker.getHp()) + "/" + std::to_string(attacker.getMaxHp()) + " PV."
        }
    );

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

    showSpecialCombatMessage(
        "VOL DE VIE DÉMONIAQUE",
        "combat.special.demon.lifesteal",
        {attackingBoss->getName() + " absorbe le sang de l'attaque et récupère " + std::to_string(healing) + " PV."}
    );
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
            showSpecialCombatMessage(
                "ESQUIVE SPÉCIALE",
                "combat.special.fireflight.dodge",
                {
                    "Fire Flight lit le mouvement au dernier instant.",
                    "Sous 50% PV, son instinct de commandant prend le relais : l'attaque est esquivée."
                }
            );
            return true;
        }
    }

    if (isName(defender, "Hestia") && random.between(1, 100) <= 18)
    {
        showSpecialCombatMessage(
            "ESQUIVE SPÉCIALE",
            "combat.special.hestia.dodge",
            {"Hestia ferme les yeux de peur... et esquive presque par accident."}
        );
        return true;
    }

    if (isName(defender, "Aoi") && random.between(1, 100) <= 16)
    {
        showSpecialCombatMessage(
            "PROTECTION KITSUNE",
            "combat.special.aoi.dodge",
            {
                "Aoi protège son cercle d'incantation avec une petite flamme kitsune.",
                "L'attaque glisse sur le sort au lieu de la toucher directement."
            }
        );
        return true;
    }

    if (isName(defender, "Matt (PRO)") && random.between(1, 100) <= 14)
    {
        showSpecialCombatMessage(
            "ESQUIVE SPÉCIALE",
            "combat.special.mattpro.dodge",
            {"Matt (PRO) anticipe le timing comme s'il connaissait déjà la frame exacte."}
        );
        return true;
    }

    if (isName(defender, "Trexof") && isUnderHalfHp(defender) && random.between(1, 100) <= 18)
    {
        showSpecialCombatMessage(
            "ESQUIVE SPÉCIALE",
            "combat.special.trexof.dodge",
            {"Trexof recule d'un pas : vieux réflexe de survie, piège évité."}
        );
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
            showSpecialCombatMessage(
                "COUP INSTABLE",
                "combat.special.sanctus.unstable_miss",
                {
                    "Sanctus frappe comme si Skuro tenait son bras...",
                    "Mais cette violence n'est pas encore stable, et le coup passe à côté."
                }
            );
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
        showSpecialCombatMessage(
            "COUP TROP LOURD",
            "combat.special.skuro.miss",
            {
                "Skuro abat son épée à deux mains avec une violence ridicule...",
                "Mais le coup est trop lourd, trop avide, et fend seulement l'air."
            }
        );
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
            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.skuro.ultimate",
                {
                    "Ultime spécial - Skuro : Fente de la matière.",
                    "Il ne cherche plus un adversaire. Il cherche l'endroit exact où le monde se coupe."
                }
            );
        }

        rawDamage += bonus;

        showSpecialCombatMessage(
            "EFFET SPÉCIAL - SKURO",
            "combat.special.skuro.bonus",
            {"Skuro trouve enfin la matière. Son tranchant réclame " + std::to_string(bonus) + " dégâts bruts supplémentaires."}
        );
    }

    if (isName(attacker, "Sanctus") && isAwakenedSanctus(attacker))
    {
        int bonus = random.between(12, 24);
        rawDamage += bonus;

        if (random.between(1, 100) <= 28)
        {
            critical = true;
        }

        showSpecialCombatMessage(
            "EFFET SPÉCIAL - SANCTUS",
            "combat.special.sanctus.awakened_bonus",
            {"La protection de Sanctus se retourne en sentence. Skuro pousse sous la lumière : +" + std::to_string(bonus) + " dégâts bruts."}
        );
    }

    if (isName(attacker, "Hestia"))
    {
        hestiaSpellCounter[attacker.getName()]++;
        int bonus = hestiaSpellCounter[attacker.getName()] * 4;

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            bonus += 30;
            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.hestia.ultimate",
                {
                    "Ultime spécial - Hestia : Dôme d'étoile oubliée.",
                    "Elle tremble, mais le sort ne tremble pas avec elle."
                }
            );
        }

        rawDamage += bonus;

        showSpecialCombatMessage(
            "EFFET SPÉCIAL - HESTIA",
            "combat.special.hestia.spell_growth",
            {
                "La magie d'Hestia résonne malgré sa peur.",
                "Chaque incantation, ratée ou non, renforce la suivante : +" + std::to_string(bonus) + " dégâts bruts."
            }
        );
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
            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.kanade.ultimate",
                {
                    "Kanadé hurle sur le ciel, et le treizième signe répond trop fort.",
                    "Ultime spécial - Kanadé : Constellation rageuse."
                }
            );
        }

        rawDamage += bonus;

        showSpecialCombatMessage(
            "EFFET SPÉCIAL - KANADÉ",
            "combat.special.kanade.zodiac",
            {
                "Kanadé râle contre son propre sort, puis le zodiaque répond quand même.",
                "Signe tiré : " + std::to_string(zodiacRoll) + "/13, bonus : +" + std::to_string(bonus) + " dégâts bruts."
            }
        );
    }

    if (isName(attacker, "Fail"))
    {
        int experiment = random.between(1, 100);

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            rawDamage += 36;
            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.fail.ultimate",
                {
                    "Ultime spécial - Fail : Expérience interdite.",
                    "Fail : Si ça explose, c'est que ça prouve quelque chose."
                }
            );
        }
        else if (experiment <= 20)
        {
            rawDamage += 20;
            showSpecialCombatMessage("EFFET SPÉCIAL - FAIL", "combat.special.fail.positive_experiment", {"Fail sourit : l'expérience est instable, donc parfaite. +20 dégâts bruts."});
        }
        else if (experiment <= 35)
        {
            rawDamage -= 8;
            if (rawDamage < 0)
            {
                rawDamage = 0;
            }
            showSpecialCombatMessage("EFFET SPÉCIAL - FAIL", "combat.special.fail.failed_experiment", {"Fail note quelque chose dans sa tête : l'expérience fonctionne... moins bien que prévu. -8 dégâts bruts."});
        }
    }

    if (isName(attacker, "Fire Flight") && isUnderHalfHp(attacker))
    {
        if (random.between(1, 100) <= 70)
        {
            critical = true;
            rawDamage += attacker.getCriticalDamage() / 2;

            showSpecialCombatMessage(
                "EFFET SPÉCIAL - FIRE FLIGHT",
                "combat.special.fireflight.critical_command",
                {
                    "Fire Flight passe sous le seuil critique.",
                    "Ses ordres deviennent plus froids, plus précis : critique renforcé."
                }
            );
        }
    }

    if (isName(attacker, "Louis"))
    {
        int bonus = random.between(2, 7);

        if (attackCounters[attacker.getName()] % 3 == 0)
        {
            bonus += 12;
            showSpecialCombatMessage("EFFET SPÉCIAL - LOUIS", "combat.special.louis.prepared_barrage", {"Louis déclenche une salve préparée : ce n'est pas très propre, mais ça part de partout."});
        }

        rawDamage += bonus;
        showSpecialCombatMessage("EFFET SPÉCIAL - LOUIS", "combat.special.louis.projectile_bonus", {"Louis ajuste son bricolage offensif : quelques projectiles de plus partent presque au bon endroit.", "Bonus brut : +" + std::to_string(bonus) + "."});
    }

    if (isName(attacker, "Hazak"))
    {
        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            critical = true;
            rawDamage += 32;
            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.hazak.ultimate",
                {
                    "Ultime spécial - Hazak : Contrat de silence.",
                    "Hazak : Le combat faisait trop de bruit."
                }
            );
        }
        else if (random.between(1, 100) <= 25)
        {
            critical = true;
            rawDamage += 10;
            showSpecialCombatMessage("EFFET SPÉCIAL - HAZAK", "combat.special.hazak.opening", {"Hazak ne cherche pas le spectacle. Il cherche l'ouverture. +10 dégâts bruts."});
        }
    }

    if (isName(attacker, "Trexof"))
    {
        int testRoll = random.between(1, 100);

        if (testRoll <= 22)
        {
            rawDamage += 14;
            showSpecialCombatMessage("EFFET SPÉCIAL - TREXOF", "combat.special.trexof.clean_test", {"Trexof repère une faille de garde et l'exploite proprement : +14 dégâts bruts."});
        }
        else if (testRoll <= 30)
        {
            rawDamage += 4;
            showSpecialCombatMessage("EFFET SPÉCIAL - TREXOF", "combat.special.trexof.angle_fix", {"Trexof corrige son angle au dernier instant : +4 dégâts bruts."});
        }
    }

    if (isName(attacker, "Matt (PRO)"))
    {
        if (random.between(1, 100) <= 20)
        {
            critical = true;
            rawDamage += 8;
            showSpecialCombatMessage("EFFET SPÉCIAL - MATT PRO", "combat.special.mattpro.controlled_critical", {"Matt (PRO) joue proprement, sans panique : critique contrôlé."});
        }
    }

    if (isName(attacker, "Mattzelda"))
    {
        rawDamage += random.between(6, 13);
        showSpecialCombatMessage("EFFET SPÉCIAL - MATTZELDA", "combat.special.mattzelda.heavy_joke", {"Mattzelda transforme sa blague en charge de colosse. Le coup pèse plus lourd que prévu."});
    }

    if (isName(attacker, "Aoi") && random.between(1, 100) <= 28)
    {
        int bonus = 9;

        if (shouldTriggerSpecialUltimate(attacker))
        {
            markSpecialUltimateUsed(attacker);
            bonus += 24;
            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.aoi.ultimate",
                {
                    "Ultime spécial - Aoi : Cercle des flammes kitsune.",
                    "Aoi baisse les yeux, puis laisse les flammes parler à sa place."
                }
            );
        }

        rawDamage += bonus;
        showSpecialCombatMessage("EFFET SPÉCIAL - AOI", "combat.special.aoi.kitsune_flame", {"Aoi ose libérer une flamme kitsune plus stable : +" + std::to_string(bonus) + " dégâts bruts."});
    }

    if (isName(attacker, "Hazak") && isGroupedWith(attacker, "Henrique"))
    {
        rawDamage += 6;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.hazak_henrique", {"Henrique garde le rythme de Hazak. L'ouverture devient plus propre : +6 dégâts bruts."});
    }

    if (isName(attacker, "Henrique") && isGroupedWith(attacker, "Hazak"))
    {
        rawDamage += 5;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.henrique_hazak", {"Hazak couvre l'angle mort d'Henrique : +5 dégâts bruts."});
    }

    if (isName(attacker, "Hazak") && isGroupedWith(attacker, "Hestia"))
    {
        rawDamage += 8;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.hazak_hestia", {"Hazak frappe plus froidement tant qu'Hestia est dans l'arène : +8 dégâts bruts."});
    }

    if (isName(attacker, "Aoi") && isGroupedWith(attacker, "Kanadé"))
    {
        rawDamage += 5;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.aoi_kanade", {"La rage de Kanadé stabilise la flamme d'Aoi : +5 dégâts bruts."});
    }

    if (isName(attacker, "Kanadé") && isGroupedWith(attacker, "Aoi"))
    {
        rawDamage += 5;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.kanade_aoi", {"Aoi protège l'incantation de Kanadé avec une flamme discrète : +5 dégâts bruts."});
    }

    if (isName(attacker, "Louis") && isGroupedWith(attacker, "Trexof"))
    {
        rawDamage += 4;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.louis_trexof", {"Trexof signale une trajectoire à Louis : +4 dégâts bruts."});
    }

    if (isName(attacker, "Trexof") && isGroupedWith(attacker, "Mattzelda"))
    {
        rawDamage += 4;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.trexof_mattzelda", {"Mattzelda attire l'attention assez longtemps pour que Trexof exploite une ouverture : +4 dégâts bruts."});
    }

    if (isName(attacker, "Fail") && isGroupedWith(attacker, "Hazak"))
    {
        rawDamage += 5;
        showSpecialCombatMessage("SYNERGIE SPÉCIALE", "combat.special.synergy.fail_hazak", {"Le contrat de non-agression force Fail à viser ailleurs que sur Hazak : +5 dégâts bruts utiles."});
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

        showSpecialCombatMessage(
            "PROJECTILES SECONDAIRES",
            "combat.special.louis.after_damage",
            {
                "Louis enchaîne avec " + std::to_string(extraProjectiles) + " projectile(s) secondaire(s).",
                defender.getName() + " subit " + std::to_string(extraDamage) + " dégâts supplémentaires."
            }
        );
    }

    if (isName(defender, "Hestia") && isGroupedWith(defender, "Hazak") && random.between(1, 100) <= 35)
    {
        int counterDamage = random.between(5, 12);
        attacker.takeDamage(counterDamage);

        showSpecialCombatMessage(
            "RIPOSTE SILENCIEUSE",
            "combat.special.hazak.hestia_counter",
            {
                "Hazak ne laisse pas l'impact sur Hestia devenir gratuit.",
                attacker.getName() + " subit " + std::to_string(counterDamage) + " dégâts de riposte silencieuse."
            }
        );
    }

    if (isName(attacker, "Kanadé") && isGroupedWith(attacker, "Aoi") && random.between(1, 100) <= 22)
    {
        int extraDamage = random.between(5, 10);
        defender.takeDamage(extraDamage);

        showSpecialCombatMessage(
            "FLAMME KITSUNE RETARDÉE",
            "combat.special.aoi.kanade_delayed_flame",
            {"Une flamme kitsune d'Aoi suit le signe de Kanadé et explose avec retard : " + std::to_string(extraDamage) + " dégâts supplémentaires."}
        );
    }

    if (isName(attacker, "Mattzelda") && random.between(1, 100) <= 25)
    {
        attacker.heal(6);

        showSpecialCombatMessage("SOUFFLE DE MATTZELDA", "combat.special.mattzelda.after_damage_heal", {"Mattzelda rigole après l'impact et reprend un peu son souffle : +6 PV."});
    }

    if (isName(attacker, "Trexof") && random.between(1, 100) <= 18)
    {
        int extraDamage = random.between(4, 9);
        defender.takeDamage(extraDamage);

        showSpecialCombatMessage(
            "SECOND TEST",
            "combat.special.trexof.after_damage",
            {"Trexof frappe une seconde fois la même ouverture : " + std::to_string(extraDamage) + " dégâts supplémentaires."}
        );
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

            showSpecialCombatMessage(
                "PROVOCATION INSTABLE",
                "combat.special.sanctus.awakened_provocation",
                {
                    "Sanctus retient Skuro de justesse, mais l'ennemi sent la menace.",
                    "Provocation : le regard revient sur lui."
                }
            );
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

            showSpecialCombatMessage(
                "DÔME D'HESTIA",
                "combat.special.hestia.revive",
                {
                    "Hestia disparaît presque derrière la peur...",
                    "Puis un dôme ancien se rallume tout seul, comme si quelque chose refusait de la laisser tomber.",
                    "Protection spéciale : retour unique à " + std::to_string(defender.getHp()) + "/" + std::to_string(defender.getMaxHp()) + " PV."
                }
            );
        }
    }

    if (isName(defender, "Henrique") && defender.isDead())
    {
        if (henriqueReviveUsed.find(defender.getName()) == henriqueReviveUsed.end())
        {
            henriqueReviveUsed.insert(defender.getName());
            defender.reviveWithHealthPercentage(45);

            showSpecialCombatMessage(
                "RETOUR D'HENRIQUE",
                "combat.special.henrique.revive",
                {
                    "Henrique tombe... puis se relève une fois de plus.",
                    "Sa capacité spéciale s'active : retour unique à " + std::to_string(defender.getHp()) + "/" + std::to_string(defender.getMaxHp()) + " PV."
                }
            );
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

            showSpecialCombatMessage(
                "ULTIME SPÉCIAL",
                "combat.special.sanctus.ultimate",
                {
                    "Ultime spécial - Sanctus : Rempart de croyance.",
                    "Sanctus attire la menace sur lui et transforme sa foi en posture de défense."
                }
            );
        }
    }

    if (isName(defender, "Mattzelda") && shouldTriggerSpecialUltimate(defender))
    {
        markSpecialUltimateUsed(defender);
        defender.heal(defender.getMaxHp() * 18 / 100);
        defender.startDefensePosture(30, 10, "Blague de colosse beaucoup trop solide");

        showSpecialCombatMessage(
            "ULTIME SPÉCIAL",
            "combat.special.mattzelda.ultimate",
            {
                "Ultime spécial - Mattzelda : Mur de blagues.",
                "Il rigole tellement fort que même les dégâts hésitent à continuer."
            }
        );
    }

    if (isName(defender, "Aoi") && isGroupedWith(defender, "Sanctus") && random.between(1, 100) <= 18)
    {
        defender.heal(5);
        showSpecialCombatMessage("SOUTIEN DE SANCTUS", "combat.special.sanctus.aoi_support", {"Sanctus détourne une partie de la menace : Aoi récupère 5 PV en gardant son cercle."});
    }

    if (isName(defender, "Kanadé") && isGroupedWith(defender, "Sanctus") && random.between(1, 100) <= 18)
    {
        defender.heal(5);
        showSpecialCombatMessage("SOUTIEN DE SANCTUS", "combat.special.sanctus.kanade_support", {"Sanctus encaisse juste assez pour que Kanadé garde sa colère utile : +5 PV."});
    }

    if (isName(defender, "Hestia") && isGroupedWith(defender, "Hazak") && random.between(1, 100) <= 25)
    {
        defender.startDefensePosture(22, 6, "Protection de Hazak autour d'Hestia");
        showSpecialCombatMessage("PROTECTION DE HAZAK", "combat.special.hazak.hestia_guard", {"Hazak déplace le combat autour d'Hestia. Sa prochaine garde devient plus sûre."});
    }

    if (isName(defender, "Louis") && isGroupedWith(defender, "Mattzelda") && random.between(1, 100) <= 18)
    {
        defender.heal(4);
        showSpecialCombatMessage("SOUTIEN DE MATTZELDA", "combat.special.mattzelda.louis_support", {"Mattzelda fait écran avec une blague beaucoup trop bruyante. Louis reprend 4 PV."});
    }

    if (isName(defender, "Aoi") && random.between(1, 100) <= 20)
    {
        showSpecialCombatMessage(
            "RÉFLEXE D'AOI",
            "combat.special.aoi.partial_guard",
            {
                "Aoi recule, timide, mais protège instinctivement ses incantations.",
                "Un cercle de protection incomplet tremble autour d'elle, sans parvenir à se fermer."
            }
        );
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
