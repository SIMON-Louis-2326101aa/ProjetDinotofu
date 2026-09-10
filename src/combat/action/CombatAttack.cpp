// EN: CombatAttack.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatAttack.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/action/CombatAttack.hpp"

#include "combat/DamageReport.hpp"
#include "combat/system/DamageSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/action/SpecialCombatEffects.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "entity/Player.hpp"
#include "entity/Boss.hpp"
#include "entity/Monster.hpp"
#include "item/weapon/WeaponType.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <sstream>
#include <ostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>


namespace
{
    std::string normalizeAttackText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool textContainsAny(const std::string& text, const std::vector<std::string>& needles)
    {
        std::string normalized = normalizeAttackText(text);
        for (const std::string& needle : needles)
        {
            if (normalized.find(normalizeAttackText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }


    std::vector<std::string> splitCapturedCombatLines(const std::string& text)
    {
        std::vector<std::string> lines;
        std::istringstream stream(text);
        std::string line;
        while (std::getline(stream, line))
        {
            if (!line.empty())
            {
                lines.push_back(line);
            }
        }
        return lines;
    }


    bool playerUsesShortReachWeapon(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return true;
        }

        const Weapon weapon = player.getEquippedWeapon();
        const std::string weaponName = normalizeAttackText(weapon.getName());
        if (weapon.getType() == WeaponType::Bow || weapon.getType() == WeaponType::Spear || weapon.getType() == WeaponType::Staff)
        {
            return false;
        }
        return weapon.getType() == WeaponType::Dagger
            || weapon.getType() == WeaponType::BareHands
            || weaponName.find("dague") != std::string::npos
            || weaponName.find("couteau") != std::string::npos
            || weaponName.find("courte") != std::string::npos
            || weaponName.find("poing") != std::string::npos
            || weaponName.find("griffe") != std::string::npos;
    }

    void showCapturedCombatLines(
        const std::string& title,
        const std::string& screenId,
        const std::string& capturedText
    )
    {
        std::vector<std::string> lines = splitCapturedCombatLines(capturedText);
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, true);
        }
    }

    int countPlayerMetalEquipment(const Player& player)
    {
        int metalPieces = 0;
        if (player.hasEquippedWeapon())
        {
            std::string weaponName = player.getEquippedWeapon().getName();
            if (textContainsAny(weaponName, {"fer", "acier", "métal", "metal", "conducteur"}))
            {
                metalPieces++;
            }
        }
        if (player.hasEquippedArmor())
        {
            std::string armorName = player.getEquippedArmor().getName();
            if (textContainsAny(armorName, {"fer", "acier", "métal", "metal", "plaques", "lourde"}))
            {
                metalPieces++;
            }
        }
        return metalPieces;
    }


    int applyStatusComboReactionsIfNeeded(
        Entity& attacker,
        Entity& defender,
        Random& random,
        int currentDamage,
        std::ostream& output
    )
    {
        int bonusDamage = 0;
        const std::string classFocus = normalizeAttackText(attacker.getType());
        const int safeLevel = std::max(1, (attacker.getMaxDamage() + attacker.getCriticalDamage()) / 10);
        const int hpPressure = defender.getMaxHp() > 0 ? defender.getHp() * 100 / defender.getMaxHp() : 100;

        if (defender.hasBurning() && defender.hasPoison() && random.between(1, 100) <= 34)
        {
            const int reactionDamage = std::max(2, currentDamage / 8 + safeLevel / 6);
            bonusDamage += reactionDamage;
            defender.applyWeakening(2, 8 + safeLevel / 6);
            output << "Réaction de statut : chaleur + poison. La toxine s'agite, ajoute "
                   << reactionDamage << " dégât(s) et affaiblit la cible." << std::endl;
        }

        if (defender.hasFrost() && defender.hasShock() && random.between(1, 100) <= 38)
        {
            const int reactionDamage = std::max(2, currentDamage / 10 + safeLevel / 8);
            bonusDamage += reactionDamage;
            defender.applyVulnerability(2, 6 + safeLevel / 10);
            output << "Réaction de statut : givre + choc. La cible conduit mal son propre mouvement, +"
                   << reactionDamage << " dégât(s) et petite vulnérabilité." << std::endl;
        }

        if (defender.hasBleeding() && defender.hasVulnerability() && random.between(1, 100) <= 42)
        {
            const int reactionDamage = std::max(2, currentDamage / 9 + safeLevel / 7);
            bonusDamage += reactionDamage;
            output << "Réaction de statut : blessure + faille. L'attaque exploite une ouverture déjà créée, +"
                   << reactionDamage << " dégât(s)." << std::endl;
        }

        if (defender.hasWeakening()
            && (classFocus.find("gardien") != std::string::npos
                || classFocus.find("tank") != std::string::npos
                || classFocus.find("colosse") != std::string::npos
                || classFocus.find("paladin") != std::string::npos)
            && random.between(1, 100) <= 35)
        {
            attacker.startDefensePosture(12, 4, "Contre-rythme défensif");
            output << "Combo de rôle : la cible est affaiblie, le profil défensif transforme l'échange en garde active." << std::endl;
        }

        if ((defender.hasPoison() || defender.hasBleeding())
            && (classFocus.find("voleur") != std::string::npos
                || classFocus.find("roublard") != std::string::npos
                || classFocus.find("brigand") != std::string::npos
                || classFocus.find("assassin") != std::string::npos
                || classFocus.find("ombrelame") != std::string::npos)
            && random.between(1, 100) <= 36)
        {
            const int reactionDamage = std::max(1, 2 + safeLevel / 9);
            bonusDamage += reactionDamage;
            defender.applyVulnerability(1, 7 + safeLevel / 12);
            output << "Combo de rôle : profil furtif sur cible déjà blessée/empoisonnée, +"
                   << reactionDamage << " dégât(s) et faille courte." << std::endl;
        }

        if ((defender.hasBurning() || defender.hasFrost() || defender.hasShock() || defender.hasPoison())
            && (classFocus.find("mage") != std::string::npos
                || classFocus.find("arcan") != std::string::npos
                || classFocus.find("sorc") != std::string::npos
                || classFocus.find("mancien") != std::string::npos
                || classFocus.find("démoniste") != std::string::npos
                || classFocus.find("demoniste") != std::string::npos
                || classFocus.find("runiste") != std::string::npos)
            && random.between(1, 100) <= 30)
        {
            const int reactionDamage = std::max(2, 2 + safeLevel / 8);
            bonusDamage += reactionDamage;
            output << "Combo de rôle : le profil magique lit l'altération active et la fait résonner, +"
                   << reactionDamage << " dégât(s)." << std::endl;
        }

        if (hpPressure <= 35 && defender.hasVulnerability() && random.between(1, 100) <= 28)
        {
            const int finisherDamage = std::max(1, 2 + safeLevel / 10);
            bonusDamage += finisherDamage;
            output << "Pression de fin de combat : cible très blessée + vulnérable, l'impact gagne "
                   << finisherDamage << " dégât(s)." << std::endl;
        }

        return bonusDamage;
    }

    void applyAmmunitionStatusIfNeeded(Player& attacker, Entity& defender, int receivedDamage, std::ostream& output)
    {
        if (receivedDamage <= 0)
        {
            attacker.clearLastConsumedAmmunition();
            return;
        }

        std::string ammoId = attacker.getLastConsumedAmmunition();
        attacker.clearLastConsumedAmmunition();

        if (ammoId.empty() || ammoId == "__emergency_defense__")
        {
            return;
        }

        if (ammoId == "barbed_arrows" || ammoId == "balanced_throwing_knives")
        {
            ElementalAffinitySystem::applyBleeding(defender, 2, 3 + attacker.getLevel() / 25);
            output << "La munition ouvre une blessure qui saignera sur les prochains tours." << std::endl;
        }
        else if (ammoId == "piercing_bolts")
        {
            int piercingDamage = 3 + attacker.getLevel() / 20;
            defender.takeDamage(piercingDamage);
            output << "Le carreau perforant traverse une protection et ajoute " << piercingDamage << " dégâts directs." << std::endl;
        }
        else if (ammoId == "ash_arrows")
        {
            ElementalAffinitySystem::applyBurning(defender, 2, 4 + attacker.getLevel() / 30);
            output << "La flèche de cendre accroche une brûlure faible mais persistante." << std::endl;
        }
        else if (ammoId == "frozen_bolts")
        {
            ElementalAffinitySystem::applyFrost(defender, 2);
            output << "Le carreau givré ralentit la cible : le froid restera un court instant." << std::endl;
        }
        else if (ammoId == "conductive_knives")
        {
            ElementalAffinitySystem::applyShock(defender, 2);
            int shockDamage = 2 + attacker.getLevel() / 35;
            Player* defendingPlayer = dynamic_cast<Player*>(&defender);
            if (defendingPlayer != nullptr)
            {
                int metalPieces = countPlayerMetalEquipment(*defendingPlayer);
                if (metalPieces > 0)
                {
                    shockDamage += 3 * metalPieces;
                    output << "Le métal équipé amplifie la conduction électrique." << std::endl;
                }
            }
            defender.takeDamage(shockDamage);
            output << "Le couteau conducteur ajoute " << shockDamage << " dégâts électriques immédiats." << std::endl;
        }
        else if (ammoId == "venom_arrows")
        {
            ElementalAffinitySystem::applyPoison(defender, 3, 2 + attacker.getLevel() / 18);
            output << "Le venin de la flèche s'accroche : poison léger sur plusieurs tours." << std::endl;
        }
        else if (ammoId == "shock_bolts")
        {
            ElementalAffinitySystem::applyShock(defender, 2);
            int shockDamage = 4 + attacker.getLevel() / 24;
            Player* defendingPlayer = dynamic_cast<Player*>(&defender);
            if (defendingPlayer != nullptr)
            {
                int metalPieces = countPlayerMetalEquipment(*defendingPlayer);
                shockDamage += 4 * metalPieces;
                if (metalPieces > 0)
                {
                    output << "Le carreau conducteur mord dans le métal équipé." << std::endl;
                }
            }
            defender.takeDamage(shockDamage);
            output << "La pointe conductrice ajoute " << shockDamage << " dégâts électriques." << std::endl;
        }
        else if (ammoId == "smoke_knives")
        {
            ElementalAffinitySystem::applyFrost(defender, 1);
            attacker.startDefensePosture(10, 8, "Écran de fumée court");
            output << "Le couteau fumigène gêne la cible et donne une petite fenêtre défensive au lanceur." << std::endl;
        }
    }

    void applyMonsterElementalStatusIfNeeded(Monster& attacker, Entity& defender, Random& random, int receivedDamage, std::ostream& output)
    {
        if (receivedDamage <= 0) return;

        const std::string raceText = attacker.getRaceText();
        const std::string typeText = attacker.getType();
        const std::string combined = raceText + " " + typeText + " " + attacker.getName();

        if (textContainsAny(combined, {"chromatique", "prisme", "miroir"}) && random.between(1, 100) <= 32)
        {
            int effectRoll = random.between(1, 4);
            if (effectRoll == 1)
            {
                ElementalAffinitySystem::applyPoison(defender, 2, 2 + attacker.getLevel() / 14);
                output << attacker.getName() << " change de couleur et laisse un poison instable." << std::endl;
            }
            else if (effectRoll == 2)
            {
                ElementalAffinitySystem::applyBurning(defender, 2, 2 + attacker.getLevel() / 16);
                output << attacker.getName() << " pulse rouge et accroche une chaleur anormale." << std::endl;
            }
            else if (effectRoll == 3)
            {
                ElementalAffinitySystem::applyFrost(defender, 2);
                output << attacker.getName() << " devient pâle et ralentit la cible." << std::endl;
            }
            else
            {
                ElementalAffinitySystem::applyShock(defender, 2);
                output << attacker.getName() << " vibre comme du verre chargé d'électricité." << std::endl;
            }
        }
        else if (textContainsAny(combined, {"ambré", "ambre", "collant", "poisseux"}) && random.between(1, 100) <= 34)
        {
            ElementalAffinitySystem::applyFrost(defender, 1);
            output << attacker.getName() << " colle à la cible : ce n'est pas du givre, mais les mouvements deviennent lourds." << std::endl;
        }
        else if (textContainsAny(combined, {"toxique", "putride", "venime", "poison", "violet", "noir", "vaseux"}) && random.between(1, 100) <= 28)
        {
            ElementalAffinitySystem::applyPoison(defender, 2, 3 + attacker.getLevel() / 12);
            output << attacker.getName() << " laisse un poison léger dans la blessure." << std::endl;
        }
        else if (textContainsAny(combined, {"brûl", "brule", "cendre", "feu", "rouge", "irritant", "chaud"}) && random.between(1, 100) <= 24)
        {
            ElementalAffinitySystem::applyBurning(defender, 2, 3 + attacker.getLevel() / 14);
            output << attacker.getName() << " transmet une chaleur persistante." << std::endl;
        }
        else if (textContainsAny(combined, {"givre", "gel", "froid", "glace", "bleu", "blanc"}) && random.between(1, 100) <= 24)
        {
            ElementalAffinitySystem::applyFrost(defender, 2);
            output << attacker.getName() << " ralentit sa cible avec un froid mordant." << std::endl;
        }
        else if (textContainsAny(combined, {"élect", "elect", "conduct", "orage", "jaune", "vibrant", "chromatique"}) && random.between(1, 100) <= 22)
        {
            ElementalAffinitySystem::applyShock(defender, 2);
            int shockDamage = 2 + attacker.getLevel() / 16;
            Player* defendingPlayer = dynamic_cast<Player*>(&defender);
            if (defendingPlayer != nullptr)
            {
                int metalPieces = countPlayerMetalEquipment(*defendingPlayer);
                if (metalPieces > 0)
                {
                    shockDamage += 3 * metalPieces;
                    output << "L'équipement métallique attire une partie de la décharge." << std::endl;
                }
            }
            defender.takeDamage(shockDamage);
            output << attacker.getName() << " ajoute " << shockDamage << " dégâts électriques." << std::endl;
        }
    }
}

void CombatAttack::executeAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    executeBoostedAttack(
        attacker,
        defender,
        random,
        0
    );
}

void CombatAttack::executeBoostedAttack(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int damageBonus
)
{
    attacker.processStatusTickAtTurnStart();
    if (attacker.isDead())
    {
        MessageScreen::show(
            "ACTION IMPOSSIBLE",
            "combat.attack.status_blocked",
            {
                attacker.getName() + " ne peut pas agir : les effets en cours l'ont mis au sol."
            }
        );
        return;
    }

    Player* flyingCheckPlayer = dynamic_cast<Player*>(&attacker);
    if (defender.hasFlight() && flyingCheckPlayer != nullptr && playerUsesShortReachWeapon(*flyingCheckPlayer))
    {
        const bool skyOath = flyingCheckPlayer->hasPassiveSkill("church_oath_open_sky");
        const int skyChance = skyOath ? std::clamp(28 + flyingCheckPlayer->getLevel() / 5, 28, 48) : 0;
        if (!skyOath || random.between(1, 100) > skyChance)
        {
            MessageScreen::show(
                "CIBLE EN VOL",
                "combat.attack.flying_short_reach_blocked",
                {
                    defender.getName() + " garde assez de hauteur pour rendre les armes courtes inutiles.",
                    skyOath
                        ? "Serment du Ciel ouvert : tu lis une ouverture, mais pas assez cette fois. Une arme courte ne devient pas soudainement une lance."
                        : "Pendant le Vol, il faut une arme d'allonge, un tir, un sort, une entrave ou attendre que la cible redescende."
                },
                false
            );
            DefensePostureSystem::tryCounterAfterMiss(defender, attacker, random);
            return;
        }

        damageBonus += 1 + flyingCheckPlayer->getLevel() / 40;
        MessageScreen::show(
            "SERMENT DU CIEL",
            "combat.attack.flying_short_reach_oath_gap",
            {
                flyingCheckPlayer->getName() + " attend que " + defender.getName() + " baisse d'une aile au lieu de frapper dans le vide.",
                "L'arme courte touche seulement grâce à une vraie fenêtre de Vol : l'avantage reste rare, lisible et moins fiable qu'une allonge."
            },
            false
        );
    }

    if (defender.hasIllusion() && flyingCheckPlayer != nullptr)
    {
        int illusionTrapChance = 66;
        if (flyingCheckPlayer->hasPassiveSkill("threat_reader")) illusionTrapChance -= 10;
        if (flyingCheckPlayer->hasPassiveSkill("body_reader")) illusionTrapChance -= 8;
        if (flyingCheckPlayer->hasPassiveSkill("terrain_reader")) illusionTrapChance -= 8;
        if (flyingCheckPlayer->hasPassiveSkill("trick_image_mastery") || flyingCheckPlayer->hasPassiveSkill("semi_fox_cunning")) illusionTrapChance -= 6;
        if (flyingCheckPlayer->hasPassiveSkill("church_oath_silence")) illusionTrapChance -= 8;
        if (flyingCheckPlayer->hasPassiveSkill("church_oath_memory")) illusionTrapChance -= 4;
        if (flyingCheckPlayer->hasPassiveSkill("church_oath_broken_mirror")) illusionTrapChance -= 12;
        if (flyingCheckPlayer->hasPrecisionBoost()) illusionTrapChance -= 6;
        illusionTrapChance = std::clamp(illusionTrapChance, 28, 66);

        if (random.between(1, 100) <= illusionTrapChance)
        {
            const int backlash = std::max(1, flyingCheckPlayer->getLevel() / 8 + random.between(1, 4));
            attacker.takeDamage(backlash);
            std::vector<std::string> lines;
            lines.push_back(attacker.getName() + " frappe un reflet au lieu du vrai corps de " + defender.getName() + ".");
            lines.push_back("Le contrecoup revient dans le geste : -" + std::to_string(backlash) + " PV.");
            lines.push_back("Les illusions durent peu, mais punissent les coups précipités.");
            if (illusionTrapChance < 66)
            {
                lines.push_back("Lecture partielle : tes passifs/bonus d'observation réduisent le piège, mais ne donnent pas une vérité divine gratuite.");
                if (flyingCheckPlayer->hasPassiveSkill("church_oath_broken_mirror"))
                {
                    lines.push_back("Serment du Miroir brisé : même rompu par le mauvais choix, le reflet a été lu assez tard pour limiter l'erreur future.");
                }
            }
            MessageScreen::show(
                "ILLUSION FRAPPÉE",
                "combat.attack.illusion_wrong_target",
                lines,
                false
            );
            DefensePostureSystem::tryCounterAfterMiss(defender, attacker, random);
            return;
        }

        if (illusionTrapChance < 66)
        {
            MessageScreen::show(
                "REFLET LU",
                "combat.attack.illusion_read",
                {
                    attacker.getName() + " prend le temps de lire les appuis au lieu de frapper le premier reflet.",
                    "L'information vient de traces, d'expérience ou de passifs de lecture : l'illusion reste dangereuse, mais elle n'est pas devinée gratuitement."
                },
                false
            );
        }
    }

    bool dodged = false;
    bool critical = false;

    int rawDamage = attacker.attack(
        random,
        dodged,
        critical,
        damageBonus
    );

    Monster* defendingMonsterBeforeAccuracy = dynamic_cast<Monster*>(&defender);
    const int targetAccuracyModifier = defendingMonsterBeforeAccuracy != nullptr
        ? MonsterBehaviorProfileCatalog::getIncomingAccuracyModifier(*defendingMonsterBeforeAccuracy)
        : 0;

    Player* attackingPlayerBeforeDodge = dynamic_cast<Player*>(&attacker);
    if (dodged
        && attackingPlayerBeforeDodge != nullptr
        && attackingPlayerBeforeDodge->hasEquippedWeapon())
    {
        const Weapon equippedWeapon = attackingPlayerBeforeDodge->getEquippedWeapon();
        if (!equippedWeapon.isBroken() && !attackingPlayerBeforeDodge->hasBossEquipmentSeal())
        {
            const int accuracyAdjustment = CombatClassSystem::getWeaponHandlingAccuracyAdjustment(
                attacker,
                equippedWeapon.getType(),
                equippedWeapon.getName()
            );

            if (accuracyAdjustment > 0 && random.between(1, 100) <= accuracyAdjustment)
            {
                dodged = false;
                critical = false;
                rawDamage = random.between(attacker.getMinDamage(), attacker.getMaxDamage()) + damageBonus;
                MessageScreen::show(
                    "MAÎTRISE D'ARME",
                    "combat.attack.weapon_accuracy_recovered",
                    {
                        "Le geste rate presque, mais la maîtrise de " + equippedWeapon.getName() + " corrige la trajectoire.",
                        CombatClassSystem::getWeaponHandlingLabel(attacker, equippedWeapon.getType(), equippedWeapon.getName()) + "."
                    },
                    false
                );
            }
        }
    }

    if (dodged
        && defendingMonsterBeforeAccuracy != nullptr
        && targetAccuracyModifier > 0
        && random.between(1, 100) <= std::min(45, targetAccuracyModifier))
    {
        dodged = false;
        critical = false;
        rawDamage = random.between(attacker.getMinDamage(), attacker.getMaxDamage()) + damageBonus;
        MessageScreen::show(
            "CIBLE IMPOSANTE",
            "combat.attack.large_target_recovered",
            {
                defender.getName() + " tente d'effacer sa ligne, mais son corps laisse encore une fenêtre.",
                MonsterBehaviorProfileCatalog::getAccuracyLine(*defendingMonsterBeforeAccuracy)
            },
            false
        );
    }

    if (dodged)
    {
        MessageScreen::show(
            "ATTAQUE ESQUIVÉE",
            "combat.attack.dodged",
            {
                attacker.getName() + " attaque, mais " + defender.getName() + " esquive au dernier moment."
            }
        );
        DefensePostureSystem::tryCounterAfterMiss(defender, attacker, random);
        return;
    }

    if (defendingMonsterBeforeAccuracy != nullptr
        && targetAccuracyModifier < 0
        && random.between(1, 100) <= std::min(42, -targetAccuracyModifier))
    {
        std::ostringstream missReactionBuffer;
        MonsterBehaviorProfileCatalog::applyEvasiveMissReaction(
            *defendingMonsterBeforeAccuracy,
            attacker,
            random,
            missReactionBuffer
        );

        std::vector<std::string> missLines = {
            defender.getName() + " sort de la trajectoire avant que le coup ne ferme l'espace.",
            MonsterBehaviorProfileCatalog::getAccuracyLine(*defendingMonsterBeforeAccuracy)
        };
        const std::vector<std::string> reactionLines = splitCapturedCombatLines(missReactionBuffer.str());
        missLines.insert(missLines.end(), reactionLines.begin(), reactionLines.end());

        MessageScreen::show(
            "CIBLE DIFFICILE",
            "combat.attack.small_target_slipped",
            missLines,
            false
        );
        DefensePostureSystem::tryCounterAfterMiss(defender, attacker, random);
        return;
    }

    if (attackingPlayerBeforeDodge != nullptr)
    {
        const int precisionPressure = attackingPlayerBeforeDodge->getCursePressureForCategory("precision");
        if (precisionPressure > 0 && random.between(1, 100) <= std::min(34, 4 + precisionPressure * 4))
        {
            std::vector<std::string> lines;
            if (attackingPlayerBeforeDodge->getKnownCursePressureForCategory("precision") > 0)
            {
                lines.push_back("Malédiction diagnostiquée : la catégorie précision brouille le geste au pire moment.");
                lines.push_back("L'effet exact reste volontairement flou tant que la lecture n'est pas totale.");
            }
            else
            {
                lines.push_back(attacker.getName() + " prépare son attaque, puis son geste se désaxe sans raison claire.");
                lines.push_back("Quelque chose cloche, mais le statut ne donne encore qu'une trace inconnue.");
            }
            MessageScreen::show("GESTE TROUBLÉ", "combat.attack.curse_precision_miss", lines, false);
            DefensePostureSystem::tryCounterAfterMiss(defender, attacker, random);
            return;
        }
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
        rawDamage,
        critical
    );

    Player* attackingPlayerIdentity = dynamic_cast<Player*>(&attacker);
    Monster* attackingMonster = dynamic_cast<Monster*>(&attacker);

    if (attackingPlayerIdentity != nullptr
        && attackingPlayerIdentity->hasEquippedWeapon())
    {
        const Weapon equippedWeapon = attackingPlayerIdentity->getEquippedWeapon();
        if (!equippedWeapon.isBroken() && !attackingPlayerIdentity->hasBossEquipmentSeal())
        {
            const int accuracyAdjustment = CombatClassSystem::getWeaponHandlingAccuracyAdjustment(
                attacker,
                equippedWeapon.getType(),
                equippedWeapon.getName()
            );

            if (accuracyAdjustment < 0 && random.between(1, 100) <= -accuracyAdjustment)
            {
                MessageScreen::show(
                    "ARME MAL ADAPTÉE",
                    "combat.attack.weapon_accuracy_penalty",
                    {
                        equippedWeapon.getName() + " répond mal au style de " + attacker.getType() + ".",
                        "Le geste se désaxe et l'attaque manque sa vraie fenêtre.",
                        CombatClassSystem::getWeaponHandlingLabel(attacker, equippedWeapon.getType(), equippedWeapon.getName()) + "."
                    },
                    false
                );
                DefensePostureSystem::tryCounterAfterMiss(defender, attacker, random);
                return;
            }
        }
    }

    std::ostringstream preparationBuffer;

    if (attackingPlayerIdentity != nullptr)
    {
        int damagePercent = CombatClassSystem::getOutgoingDamagePercent(attacker);
        int flatBonus = CombatClassSystem::getOutgoingFlatBonus(attacker);
        int beforeSpecialityDamage = rawDamage;

        rawDamage = rawDamage * damagePercent / 100 + flatBonus;

        if (rawDamage < 1)
        {
            rawDamage = 1;
        }

        if (attackingPlayerIdentity->hasEquippedWeapon())
        {
            Weapon equippedWeapon = attackingPlayerIdentity->getEquippedWeapon();

            if (!equippedWeapon.isBroken() && !attackingPlayerIdentity->hasBossEquipmentSeal())
            {
                const int weaponHandlingDamagePercent = CombatClassSystem::getWeaponHandlingDamagePercent(
                    attacker,
                    equippedWeapon.getType(),
                    equippedWeapon.getName()
                );

                if (weaponHandlingDamagePercent != 100)
                {
                    const int beforeWeaponHandlingDamage = rawDamage;
                    rawDamage = std::max(1, rawDamage * weaponHandlingDamagePercent / 100);
                    const int diff = rawDamage - beforeWeaponHandlingDamage;
                    if (diff > 0)
                    {
                        preparationBuffer << "L'arme suit le style : +" << diff
                                          << " dégât(s), l'arme transmet mieux la force du style." << std::endl;
                    }
                    else if (diff < 0)
                    {
                        preparationBuffer << "L'arme gêne le style : " << diff
                                          << " dégât(s), l'arme est moins adaptée au style." << std::endl;
                    }
                }

                int affinityBonus = CombatClassSystem::getWeaponAffinityDamageBonus(
                    attacker,
                    equippedWeapon.getType(),
                    equippedWeapon.getName(),
                    rawDamage
                );

                if (affinityBonus > 0)
                {
                    rawDamage += affinityBonus;
                    std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(
                        attacker,
                        equippedWeapon.getType(),
                        equippedWeapon.getName()
                    );
                    preparationBuffer << "Affinité arme/classe : +" << affinityBonus
                              << " dégât(s), " << affinityLabel << "." << std::endl;
                }

                const std::string weaponName = equippedWeapon.getName();

                if (textContainsAny(weaponName, {"lame de fer runique", "fer runique"}))
                {
                    if (defender.isInDefensePosture() && random.between(1, 100) <= 35)
                    {
                        int guardBreakDamage = std::max(1, 2 + attackingPlayerIdentity->getLevel() / 35);
                        rawDamage += guardBreakDamage;
                        defender.clearDefensePosture();
                        preparationBuffer << "Effet latent : la rainure runique brise la garde et ajoute "
                                          << guardBreakDamage << " dégât(s)." << std::endl;
                    }
                    else if (random.between(1, 100) <= 14)
                    {
                        rawDamage += 1;
                        preparationBuffer << "Effet latent : la rune stabilise légèrement l'impact." << std::endl;
                    }
                }
                else if (textContainsAny(weaponName, {"dague d'ambre vive", "ambre vive"}))
                {
                    bool targetFragilized = defender.hasBleeding()
                        || defender.hasPoison()
                        || defender.getHp() * 100 <= defender.getMaxHp() * 55;

                    if (targetFragilized && random.between(1, 100) <= 38)
                    {
                        ElementalAffinitySystem::applyBleeding(defender, 2, 2 + attackingPlayerIdentity->getLevel() / 40);
                        rawDamage += 1;
                        preparationBuffer << "Effet latent : l'ambre accroche une blessure déjà fragile." << std::endl;
                    }
                    else if (random.between(1, 100) <= 12)
                    {
                        ElementalAffinitySystem::applyBleeding(defender, 1, 1);
                        preparationBuffer << "Effet latent : l'ambre laisse une coupure fine." << std::endl;
                    }
                }
                else if (textContainsAny(weaponName, {"arc long cendré", "cendré", "cendre"}))
                {
                    std::string ammoId = attackingPlayerIdentity->getLastConsumedAmmunition();
                    if (ammoId == "ash_arrows" || ammoId == "fire_bolts" || ammoId == "burning_arrows")
                    {
                        ElementalAffinitySystem::applyBurning(defender, 2, 2 + attackingPlayerIdentity->getLevel() / 35);
                        rawDamage += 1;
                        preparationBuffer << "Effet latent : l'arc cendré canalise mieux la munition brûlante." << std::endl;
                    }
                    else if (random.between(1, 100) <= 16)
                    {
                        ElementalAffinitySystem::applyBurning(defender, 1, 1);
                        preparationBuffer << "Effet latent : une poussière chaude reste sur la plaie." << std::endl;
                    }
                }
                else if (textContainsAny(weaponName, {"sceptre canalisateur", "canalisateur"}))
                {
                    std::string classFocusPreview = CombatClassSystem::normalizeClassText(attacker.getType());
                    bool magicalUser = classFocusPreview.find("mage") != std::string::npos
                        || classFocusPreview.find("arcan") != std::string::npos
                        || classFocusPreview.find("sorc") != std::string::npos
                        || classFocusPreview.find("pyrom") != std::string::npos
                        || classFocusPreview.find("invoc") != std::string::npos
                        || classFocusPreview.find("clerc") != std::string::npos
                        || classFocusPreview.find("prêtre") != std::string::npos
                        || classFocusPreview.find("pretre") != std::string::npos;

                    if (magicalUser && random.between(1, 100) <= 24)
                    {
                        int focusDamage = std::max(1, 2 + attackingPlayerIdentity->getLevel() / 45);
                        rawDamage += focusDamage;
                        preparationBuffer << "Effet latent : le catalyseur stabilise le geste magique et ajoute "
                                          << focusDamage << " dégât(s)." << std::endl;
                    }
                    else if (!magicalUser && random.between(1, 100) <= 8)
                    {
                        rawDamage += 1;
                        preparationBuffer << "Effet latent : le sceptre répond faiblement malgré une prise maladroite." << std::endl;
                    }
                }
            }
        }

        std::string classFocus = CombatClassSystem::normalizeClassText(attacker.getType());

        const int classLevel = attackingPlayerIdentity->getLevel();

        if (classLevel >= 5
            && (classFocus.find("guerrier") != std::string::npos
                || classFocus.find("chevalier") != std::string::npos
                || classFocus.find("duelliste") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            rawDamage += 2 + classLevel / 30;
            preparationBuffer << "L'enchaînement simple trouve naturellement sa suite." << std::endl;
        }

        if (classLevel >= 8
            && (classFocus.find("assassin") != std::string::npos
                || classFocus.find("ombrelame") != std::string::npos)
            && random.between(1, 100) <= 16)
        {
            ElementalAffinitySystem::applyBleeding(defender, 1, 1 + classLevel / 40);
            rawDamage += 1;
            preparationBuffer << "L'incision discrète laisse une blessure courte derrière la frappe." << std::endl;
        }

        if (classLevel >= 10
            && (classFocus.find("gardien") != std::string::npos
                || classFocus.find("colosse") != std::string::npos
                || classFocus.find("paladin") != std::string::npos
                || classFocus.find("chevalier bouclier") != std::string::npos
                || classFocus.find("porte-bannière") != std::string::npos
                || classFocus.find("porte-banniere") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            attacker.startDefensePosture(10, 2, "Posture apprise de rempart");
            preparationBuffer << "Le combattant finit son attaque en rempart court." << std::endl;
        }

        if (classLevel >= 12
            && (classFocus.find("mage") != std::string::npos
                || classFocus.find("arcaniste") != std::string::npos
                || classFocus.find("sorcier") != std::string::npos
                || classFocus.find("mancien") != std::string::npos
                || classFocus.find("démoniste") != std::string::npos
                || classFocus.find("demoniste") != std::string::npos
                || classFocus.find("runiste") != std::string::npos
                || classFocus.find("enchanteur") != std::string::npos
                || classFocus.find("chaman de guerre") != std::string::npos)
            && random.between(1, 100) <= 15)
        {
            int roll = random.between(1, 4);
            if (roll == 1) ElementalAffinitySystem::applyBurning(defender, 1, 1 + classLevel / 32);
            else if (roll == 2) ElementalAffinitySystem::applyPoison(defender, 1, 1 + classLevel / 32);
            else if (roll == 3) ElementalAffinitySystem::applyFrost(defender, 1);
            else ElementalAffinitySystem::applyShock(defender, 1);
            preparationBuffer << "Une trace élémentaire suit le geste physique." << std::endl;
        }

        if (classLevel >= 15
            && (classFocus.find("rôdeur") != std::string::npos
                || classFocus.find("rodeur") != std::string::npos
                || classFocus.find("archer") != std::string::npos
                || classFocus.find("tireur") != std::string::npos
                || classFocus.find("javelinier") != std::string::npos
                || classFocus.find("trappeur") != std::string::npos
                || classFocus.find("guetteur") != std::string::npos
                || classFocus.find("messager arm") != std::string::npos)
            && attackingPlayerIdentity->hasEquippedWeapon()
            && (attackingPlayerIdentity->getEquippedWeapon().getType() == WeaponType::Bow
                || textContainsAny(attackingPlayerIdentity->getEquippedWeapon().getName(), {"arbal", "lancer", "bandoulière", "bandouliere"}))
            && random.between(1, 100) <= 20)
        {
            rawDamage += 3 + classLevel / 28;
            preparationBuffer << "Le tir cadré suit la ligne exacte de l'arme." << std::endl;
        }

        if ((classFocus.find("assassin") != std::string::npos
            || classFocus.find("ombrelame") != std::string::npos
            || classFocus.find("lanceur de dagues") != std::string::npos
            || classFocus.find("sabreur") != std::string::npos
            || classFocus.find("danseur lunaire") != std::string::npos
            || classFocus.find("messager arm") != std::string::npos)
            && random.between(1, 100) <= (critical ? 45 : 18))
        {
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + attackingPlayerIdentity->getLevel() / 35);
            preparationBuffer << "Spécialité furtive : la frappe cherche une veine et prépare un saignement." << std::endl;
        }

        if ((classFocus.find("paladin") != std::string::npos
            || classFocus.find("clerc") != std::string::npos
            || classFocus.find("prêtre") != std::string::npos
            || classFocus.find("pretre") != std::string::npos
            || classFocus.find("juge novice") != std::string::npos
            || classFocus.find("infirmier") != std::string::npos)
            && attacker.getHp() * 2 <= attacker.getMaxHp()
            && random.between(1, 100) <= 30)
        {
            int selfHeal = std::max(1, 3 + attackingPlayerIdentity->getLevel() / 18);
            attacker.heal(selfHeal);
            preparationBuffer << "Spécialité sacrée : l'effort referme légèrement les blessures du combattant." << std::endl;
        }

        if ((classFocus.find("gardien") != std::string::npos
            || classFocus.find("colosse") != std::string::npos
            || classFocus.find("chevalier bouclier") != std::string::npos
            || classFocus.find("porte-bannière") != std::string::npos
            || classFocus.find("porte-banniere") != std::string::npos
            || classFocus.find("tank") != std::string::npos)
            && random.between(1, 100) <= 24)
        {
            attacker.startDefensePosture(8, 3, "Ancrage de classe lourde");
            preparationBuffer << "Spécialité lourde : le combattant s'ancre après son coup." << std::endl;
        }

        if ((classFocus.find("mage") != std::string::npos
            || classFocus.find("arcaniste") != std::string::npos
            || classFocus.find("sorcier") != std::string::npos
            || classFocus.find("pyromancien") != std::string::npos
            || classFocus.find("hydromancien") != std::string::npos
            || classFocus.find("géomancien") != std::string::npos
            || classFocus.find("geomancien") != std::string::npos
            || classFocus.find("chronomancien") != std::string::npos
            || classFocus.find("démoniste") != std::string::npos
            || classFocus.find("demoniste") != std::string::npos
            || classFocus.find("aéromancien") != std::string::npos
            || classFocus.find("aeromancien") != std::string::npos
            || classFocus.find("runiste") != std::string::npos
            || classFocus.find("enchanteur") != std::string::npos)
            && random.between(1, 100) <= 20)
        {
            int effectRoll = random.between(1, 3);
            if (effectRoll == 1)
            {
                ElementalAffinitySystem::applyBurning(defender, 1, 2 + attackingPlayerIdentity->getLevel() / 28);
                preparationBuffer << "Spécialité magique : une braise instable reste accrochée à la cible." << std::endl;
            }
            else if (effectRoll == 2)
            {
                ElementalAffinitySystem::applyFrost(defender, 1);
                preparationBuffer << "Spécialité magique : un froid bref gêne le mouvement adverse." << std::endl;
            }
            else
            {
                ElementalAffinitySystem::applyShock(defender, 1);
                preparationBuffer << "Spécialité magique : une perturbation électrique traverse l'impact." << std::endl;
            }
        }

        if ((classFocus.find("rôdeur") != std::string::npos
            || classFocus.find("rodeur") != std::string::npos
            || classFocus.find("archer") != std::string::npos
            || classFocus.find("tireur") != std::string::npos
            || classFocus.find("trappeur") != std::string::npos
            || classFocus.find("guetteur") != std::string::npos
            || classFocus.find("messager arm") != std::string::npos)
            && attackingPlayerIdentity->hasEquippedWeapon()
            && attackingPlayerIdentity->getEquippedWeapon().getType() == WeaponType::Bow
            && random.between(1, 100) <= 26)
        {
            rawDamage += std::max(1, 2 + attackingPlayerIdentity->getLevel() / 25);
            preparationBuffer << "Spécialité à distance : le tir suit enfin la ligne de l'arme équipée." << std::endl;
        }

        if ((classFocus.find("lancier") != std::string::npos
            || classFocus.find("javelinier") != std::string::npos
            || classFocus.find("spear") != std::string::npos
            || classFocus.find("garde") != std::string::npos)
            && attackingPlayerIdentity->hasEquippedWeapon()
            && attackingPlayerIdentity->getEquippedWeapon().getType() == WeaponType::Spear
            && random.between(1, 100) <= 22)
        {
            attacker.startDefensePosture(6, 2, "Garde de lance");
            preparationBuffer << "Spécialité de lance : la portée garde l'adversaire à distance après l'impact." << std::endl;
        }

        if ((classFocus.find("barbare") != std::string::npos
            || classFocus.find("berserker") != std::string::npos
            || classFocus.find("briseur") != std::string::npos
            || classFocus.find("faucheur") != std::string::npos
            || classFocus.find("lame tellurique") != std::string::npos)
            && attacker.getHp() * 100 <= attacker.getMaxHp() * 55
            && random.between(1, 100) <= 28)
        {
            rawDamage = rawDamage * 112 / 100 + 2;
            preparationBuffer << "Spécialité brutale : plus le combattant est blessé, plus il force l'impact." << std::endl;
        }

        if ((classFocus.find("alchimiste") != std::string::npos
            || classFocus.find("artificier") != std::string::npos
            || classFocus.find("runiste") != std::string::npos
            || classFocus.find("enchanteur") != std::string::npos
            || classFocus.find("intendant") != std::string::npos
            || classFocus.find("bricoleur") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            int effectRoll = random.between(1, 4);
            if (effectRoll == 1)
            {
                ElementalAffinitySystem::applyPoison(defender, 1, 1 + attackingPlayerIdentity->getLevel() / 35);
                preparationBuffer << "Spécialité d'artisanat : un résidu expérimental empoisonne légèrement la cible." << std::endl;
            }
            else if (effectRoll == 2)
            {
                ElementalAffinitySystem::applyBurning(defender, 1, 1 + attackingPlayerIdentity->getLevel() / 35);
                preparationBuffer << "Spécialité d'artisanat : une étincelle chimique reste sur la blessure." << std::endl;
            }
            else
            {
                rawDamage += 2;
                preparationBuffer << "Spécialité d'artisanat : le coup vise une faiblesse matérielle." << std::endl;
            }
        }

        if ((classFocus.find("invoc") != std::string::npos
            || classFocus.find("nécro") != std::string::npos
            || classFocus.find("necro") != std::string::npos
            || classFocus.find("dresseur spectral") != std::string::npos
            || classFocus.find("gardien de familiers") != std::string::npos
            || classFocus.find("conjurateur") != std::string::npos
            || classFocus.find("corbeaumancien") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            ElementalAffinitySystem::applyFrost(defender, 1);
            preparationBuffer << "Spécialité d'invocateur : l'attaque laisse une pression froide, comme une présence derrière la cible." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_guarded_flame")
            && random.between(1, 100) <= 16)
        {
            ElementalAffinitySystem::applyBurning(defender, 1, 1 + attackingPlayerIdentity->getLevel() / 45);
            preparationBuffer << "Serment de la Flamme gardée : la chaleur reste assez tenue pour marquer sans consumer gratuitement." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_shadow")
            && (critical || defender.hasBleeding() || random.between(1, 100) <= 14))
        {
            ElementalAffinitySystem::applyBleeding(defender, 1, 1 + attackingPlayerIdentity->getLevel() / 50);
            preparationBuffer << "Serment des Ombres franches : l'angle trouvé devient une coupure discrète, pas une trahison automatique." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_hunter")
            && attackingMonster == nullptr
            && defendingMonsterBeforeAccuracy != nullptr
            && random.between(1, 100) <= 18)
        {
            rawDamage += std::max(1, 1 + attackingPlayerIdentity->getLevel() / 45);
            preparationBuffer << "Serment du Chasseur : la frappe suit une trace réelle de la cible au lieu de deviner sa faiblesse." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_blood")
            && attacker.getHp() * 100 <= attacker.getMaxHp() * 55
            && random.between(1, 100) <= 20)
        {
            rawDamage += std::max(1, 2 + attackingPlayerIdentity->getLevel() / 35);
            preparationBuffer << "Serment du Sang : la douleur paie un impact plus net, sans annuler le danger." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_shield")
            && attacker.getHp() * 100 <= attacker.getMaxHp() * 70
            && random.between(1, 100) <= 16)
        {
            attacker.startDefensePosture(7, 2, "Serment du Bouclier");
            preparationBuffer << "Serment du Bouclier : le coup finit en garde courte, pour rappeler que la promesse porte aussi la ligne." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_king")
            && random.between(1, 100) <= 12)
        {
            rawDamage += 1;
            preparationBuffer << "Serment du Roi : la présence rend l'attaque plus assumée, surtout quand des alliés apprendront à suivre." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_silence")
            && (defender.hasIllusion() || random.between(1, 100) <= 10))
        {
            rawDamage += 1;
            preparationBuffer << "Serment du Silence : le calme retire du bruit au geste et aide à ne pas répondre aux provocations." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_broken_mirror")
            && defender.hasIllusion()
            && random.between(1, 100) <= 18)
        {
            rawDamage += std::max(1, 1 + attackingPlayerIdentity->getLevel() / 50);
            preparationBuffer << "Serment du Miroir brisé : le coup vise une incohérence de reflet, pas une vérité offerte gratuitement." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_open_sky")
            && defender.hasFlight()
            && random.between(1, 100) <= 16)
        {
            rawDamage += 1;
            preparationBuffer << "Serment du Ciel ouvert : l'attaque attend une baisse d'aile au lieu de gaspiller l'élan." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_roots")
            && defender.hasEntanglement()
            && random.between(1, 100) <= 18)
        {
            rawDamage += std::max(1, 1 + attackingPlayerIdentity->getLevel() / 55);
            preparationBuffer << "Serment des Racines : la frappe suit une tension de fil ou de racine déjà visible, sans inventer une faiblesse gratuite." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_witness")
            && (defender.hasVulnerability() || defender.hasWeakening() || defender.hasIllusion())
            && random.between(1, 100) <= 15)
        {
            rawDamage += 1;
            preparationBuffer << "Serment du Témoin : le coup s'appuie sur un signe déjà vu - posture, rumeur, faille ou reflet - pas sur une vérité divine." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_scars")
            && attacker.getMaxHp() > 0
            && attacker.getHp() * 100 <= attacker.getMaxHp() * 45
            && random.between(1, 100) <= 17)
        {
            rawDamage += std::max(1, 1 + attackingPlayerIdentity->getLevel() / 45);
            attacker.startDefensePosture(4, 1, "Serment des Cicatrices");
            preparationBuffer << "Serment des Cicatrices : la douleur vécue serre le geste et laisse une garde courte, pas une envie de se blesser pour rien." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_legacy")
            && (attackingPlayerIdentity->hasPassiveSkill("church_oath_memory") || attackingPlayerIdentity->hasPassiveSkill("church_oath_broken_trace"))
            && random.between(1, 100) <= 10)
        {
            rawDamage += 1;
            preparationBuffer << "Serment de l'Héritage : une trace déjà inscrite donne du poids au coup, comme un objet ou un nom qui refuse de disparaître." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_bound_forge")
            && attackingPlayerIdentity->hasEquippedWeapon())
        {
            const Weapon oathWeapon = attackingPlayerIdentity->getEquippedWeapon();
            if (!oathWeapon.isBroken()
                && CombatClassSystem::hasWeaponAffinity(attacker, oathWeapon.getType(), oathWeapon.getName())
                && random.between(1, 100) <= 14)
            {
                const int forgeDamage = std::max(1, 1 + attackingPlayerIdentity->getLevel() / 55);
                rawDamage += forgeDamage;
                attackingPlayerIdentity->recordCanonicalEvent("objets_avec_memoire", "arme_liee:" + oathWeapon.getName(), oathWeapon.getName() + " a porté un coup cohérent avec son serment", 1);
                preparationBuffer << "Serment de la Forge liée : " << oathWeapon.getName() << " répond à une classe qui sait vraiment la tenir. La trace compte pour les futurs objets avec mémoire." << std::endl;
            }
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_bonds")
            && (attackingPlayerIdentity->hasPassiveSkill("church_oath_king") || attackingPlayerIdentity->hasPassiveSkill("battle_order_mastery") || attackingPlayerIdentity->getCanonicalJournalCategoryTotal("participation_recrues") > 0)
            && random.between(1, 100) <= 13)
        {
            rawDamage += 1;
            attacker.startDefensePosture(3, 1, "Serment des Liens");
            attackingPlayerIdentity->recordCanonicalEvent("techniques_combinees_alliees", "lien_en_combat", "Le Serment des Liens a soutenu une action de groupe", 1);
            preparationBuffer << "Serment des Liens : l'attaque garde une place pour les alliés et les futurs combos de groupe, sans jouer leur tour à leur place." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_rivals")
            && defendingMonsterBeforeAccuracy != nullptr
            && (critical || defender.hasVulnerability() || defender.hasBleeding() || defender.hasWeakening())
            && random.between(1, 100) <= 13)
        {
            rawDamage += std::max(1, 1 + attackingPlayerIdentity->getLevel() / 60);
            attackingPlayerIdentity->recordCanonicalEvent("rivaux_potentiels", "marque_rivale:" + defendingMonsterBeforeAccuracy->getName(), defendingMonsterBeforeAccuracy->getName() + " a été marqué par le Serment des Rivaux", 1);
            preparationBuffer << "Serment des Rivaux : le coup n'invente pas un ennemi juré, il marque une faille déjà vécue que l'adversaire pourrait porter plus tard." << std::endl;
        }

        if (attackingPlayerIdentity->hasPassiveSkill("church_oath_unstable_fate")
            && (attackingPlayerIdentity->hasPassiveSkill("church_oath_memory") || attackingPlayerIdentity->hasPassiveSkill("church_oath_broken_trace") || attackingPlayerIdentity->getCanonicalJournalCategoryTotal("rivaux_potentiels") > 0 || attackingPlayerIdentity->getCanonicalJournalCategoryTotal("objets_avec_memoire") > 0)
            && random.between(1, 100) <= (attacker.getHp() * 100 <= std::max(1, attacker.getMaxHp()) * 35 ? 15 : 8))
        {
            if (random.between(1, 100) <= 55)
            {
                rawDamage += 1;
                preparationBuffer << "Serment du Destin instable : une trace ancienne déplace légèrement l'impact, sans garantir la victoire." << std::endl;
            }
            else
            {
                attacker.startDefensePosture(2, 1, "Destin instable");
                preparationBuffer << "Serment du Destin instable : le fil ne donne pas plus de force, mais resserre une garde minuscule au bon moment." << std::endl;
            }
            attackingPlayerIdentity->recordCanonicalEvent("destin_instable", "oscillation_combat", "Le Serment du Destin instable a oscillé pendant une action", 1);
        }

        const int attackPressure = attackingPlayerIdentity->getCursePressureForCategory("attack");
        if (attackPressure > 0)
        {
            const int beforeCurseDamage = rawDamage;
            const int percent = std::max(72, 100 - attackPressure * 4);
            rawDamage = std::max(1, rawDamage * percent / 100);
            if (rawDamage < beforeCurseDamage)
            {
                if (attackingPlayerIdentity->getKnownCursePressureForCategory("attack") > 0)
                {
                    preparationBuffer << "Malédiction diagnostiquée : la catégorie attaque alourdit l'impact sans révéler sa valeur exacte." << std::endl;
                }
                else
                {
                    preparationBuffer << "Quelque chose retient l'impact, comme si la force arrivait avec un temps de retard." << std::endl;
                }
            }
        }

        if (rawDamage != beforeSpecialityDamage)
        {
            preparationBuffer << "La manière de combattre change l'impact."
                      << std::endl;
        }
    }

    if (attackingMonster != nullptr)
    {
        int beforeMonsterSpecialityDamage = rawDamage;
        const std::string raceText = attackingMonster->getRaceText();
        const std::string typeText = attackingMonster->getType();
        const std::string monsterFocus = attackingMonster->getName() + " " + typeText + " " + raceText;

        const int signatureBonusDamage = MonsterBehaviorProfileCatalog::applySignaturePreImpact(
            *attackingMonster,
            defender,
            random,
            rawDamage,
            preparationBuffer
        );
        if (signatureBonusDamage > 0)
        {
            rawDamage += signatureBonusDamage;
        }

        if (textContainsAny(monsterFocus, {"frondeur", "tireur", "archer"}) && random.between(1, 100) <= 22)
        {
            rawDamage += 2 + attackingMonster->getLevel() / 12;
            preparationBuffer << attackingMonster->getName() << " harcèle à distance au lieu de charger bêtement." << std::endl;
        }

        if (textContainsAny(monsterFocus, {"garde", "sentinelle", "armure", "golem", "construction"}) && random.between(1, 100) <= 24)
        {
            attacker.startDefensePosture(10, 2, "Garde monstrueuse");
            preparationBuffer << attackingMonster->getName() << " protège son noyau/faiblesse après avoir frappé." << std::endl;
        }

        if (textContainsAny(monsterFocus, {"loup", "prédateur", "predateur", "moustique", "sangsue"})
            && defender.getHp() * 100 <= defender.getMaxHp() * 45
            && random.between(1, 100) <= 30)
        {
            rawDamage += 3 + attackingMonster->getLevel() / 10;
            preparationBuffer << attackingMonster->getName() << " sent la proie blessée et devient plus agressif." << std::endl;
        }

        if (textContainsAny(monsterFocus, {"champignon hurleur", "tambour", "oracle"}) && random.between(1, 100) <= 18)
        {
            rawDamage += 1;
            preparationBuffer << attackingMonster->getName() << " perturbe le rythme du combat : son rôle de support se ressent même seul." << std::endl;
        }

        if (textContainsAny(attackingMonster->getName() + " " + typeText, {"shaman", "chamane", "oracle", "apothicaire"})
            && random.between(1, 100) <= 26)
        {
            int selfHeal = std::max(2, 4 + attackingMonster->getLevel() / 8);
            attacker.heal(selfHeal);
            preparationBuffer << attackingMonster->getName() << " gaspille une fiole ou une prière mineure pour rester debout." << std::endl;
        }

        if (raceText.find("Gobelin") != std::string::npos && defender.getHp() * 2 <= defender.getMaxHp())
        {
            rawDamage += 3;
            preparationBuffer << attackingMonster->getName() << " sent la faiblesse et frappe comme un pillard opportuniste." << std::endl;
        }
        else if (raceText.find("Bête") != std::string::npos || typeText.find("Prédateur") != std::string::npos || typeText.find("rapide") != std::string::npos)
        {
            if (random.between(1, 100) <= 25)
            {
                rawDamage += 4;
                preparationBuffer << attackingMonster->getName() << " profite de sa vitesse naturelle." << std::endl;
            }
        }
        else if (raceText.find("Mort-vivant") != std::string::npos)
        {
            if (random.between(1, 100) <= 20)
            {
                rawDamage += 5;
                preparationBuffer << "Une force froide accompagne le coup du mort-vivant." << std::endl;
            }
        }
        else if (raceText.find("Slime") != std::string::npos)
        {
            if (typeText.find("toxique") != std::string::npos || typeText.find("putride") != std::string::npos)
            {
                rawDamage += 4;
                preparationBuffer << attackingMonster->getName() << " laisse une matière nocive sur la blessure." << std::endl;
            }
            else if (typeText.find("froide") != std::string::npos || typeText.find("gelée froide") != std::string::npos)
            {
                rawDamage += 3;
                preparationBuffer << attackingMonster->getName() << " refroidit brutalement l'impact." << std::endl;
            }
            else if (typeText.find("brillante") != std::string::npos || typeText.find("or") != std::string::npos)
            {
                if (random.between(1, 100) <= 35)
                {
                    rawDamage += 5;
                    preparationBuffer << attackingMonster->getName() << " attaque en protégeant ce qui brille en lui." << std::endl;
                }
            }
            else if (typeText.find("bond") != std::string::npos || attackingMonster->getName().find("rose") != std::string::npos)
            {
                if (random.between(1, 100) <= 35)
                {
                    rawDamage += 4;
                    preparationBuffer << attackingMonster->getName() << " rebondit dans un angle absurde avant l'impact." << std::endl;
                }
            }
            else if (typeText.find("coll") != std::string::npos || attackingMonster->getName().find("ambr") != std::string::npos)
            {
                rawDamage += 2;
                preparationBuffer << attackingMonster->getName() << " rend l'esquive pénible avec sa matière collante." << std::endl;
            }
            else if (random.between(1, 100) <= 18)
            {
                rawDamage += 2;
                preparationBuffer << attackingMonster->getName() << " rebondit et rend le choc moins prévisible." << std::endl;
            }
        }
        else if (raceText.find("Plante") != std::string::npos)
        {
            if (random.between(1, 100) <= 25)
            {
                rawDamage += 3;
                preparationBuffer << attackingMonster->getName() << " accroche sa cible avec des fibres végétales." << std::endl;
            }
        }
        else if (raceText.find("Insectoïde") != std::string::npos)
        {
            if (random.between(1, 100) <= 28)
            {
                rawDamage += 4;
                preparationBuffer << attackingMonster->getName() << " pique dans un angle difficile à protéger." << std::endl;
            }
        }
        else if (raceText.find("Élémentaire") != std::string::npos || raceText.find("Démon") != std::string::npos)
        {
            if (random.between(1, 100) <= 22)
            {
                rawDamage += 6;
                preparationBuffer << attackingMonster->getName() << " libère une surcharge instable." << std::endl;
            }
        }
        else if (raceText.find("Orc") != std::string::npos || raceText.find("Hobgobelin") != std::string::npos)
        {
            if (attackingMonster->getHp() * 2 <= attackingMonster->getMaxHp())
            {
                rawDamage += 6;
                preparationBuffer << attackingMonster->getName() << " devient plus violent en étant blessé." << std::endl;
            }
            else if (random.between(1, 100) <= 22)
            {
                rawDamage += 3;
                preparationBuffer << attackingMonster->getName() << " impose sa force brute." << std::endl;
            }
        }
        else if (raceText.find("Construction") != std::string::npos)
        {
            rawDamage += 2;
            attacker.startDefensePosture(12, 2, "Carapace de construction");
            preparationBuffer << attackingMonster->getName() << " frappe lourdement et reste difficile à entamer." << std::endl;
        }
        else if (raceText.find("Anomalie") != std::string::npos)
        {
            int anomalyRoll = random.between(1, 4);
            rawDamage += anomalyRoll * 2;
            if (anomalyRoll == 4)
            {
                ElementalAffinitySystem::applyShock(defender, 1);
                preparationBuffer << "L'anomalie déforme l'impact et laisse une perturbation électrique." << std::endl;
            }
            else
            {
                preparationBuffer << "L'anomalie rend les dégâts difficiles à prévoir." << std::endl;
            }
        }
        else if (raceText.find("Dragon") != std::string::npos || raceText.find("Draconide") != std::string::npos)
        {
            rawDamage = rawDamage * 108 / 100 + 2;
            preparationBuffer << "La puissance draconique rend le coup plus lourd." << std::endl;
        }

        if (rawDamage < 1)
        {
            rawDamage = 1;
        }

        if (rawDamage != beforeMonsterSpecialityDamage && attackingMonster->isElite())
        {
            preparationBuffer << "Son statut d'élite rend cette spécialité encore plus inquiétante." << std::endl;
        }
    }

    const int comboBonusDamage = applyStatusComboReactionsIfNeeded(attacker, defender, random, rawDamage, preparationBuffer);
    if (comboBonusDamage > 0)
    {
        rawDamage += comboBonusDamage;
        preparationBuffer << "Les altérations se répondent et déforment le rythme de l'impact." << std::endl;
    }

    if (rawDamage < 1)
    {
        rawDamage = 1;
    }

    showCapturedCombatLines(
        "PRÉPARATION DE L'ATTAQUE",
        "combat.attack.preparation",
        preparationBuffer.str()
    );

    if (SpecialCombatEffects::specialCharacterDodgesBeforeDamage(
        defender,
        random
    ))
    {
        return;
    }

    if (SpecialCombatEffects::atlasBlocksAttack(
        attacker,
        defender,
        rawDamage
    ))
    {
        return;
    }

    std::ostringstream impactBuffer;

    if (attackingMonster != nullptr)
    {
        impactBuffer << MonsterBehaviorProfileCatalog::buildAttackImpactLine(
            *attackingMonster,
            rawDamage,
            critical,
            damageBonus > 0
        ) << std::endl;
    }
    else if (critical)
    {
        impactBuffer << attacker.getName()
                  << " frappe avec une violence monstrueuse et inflige "
                  << rawDamage
                  << " dégâts bruts critiques."
                  << std::endl;
    }
    else if (damageBonus > 0)
    {
        impactBuffer << attacker.getName()
                  << " attaque avec une puissance renforcée et inflige "
                  << rawDamage
                  << " dégâts bruts."
                  << std::endl;
    }
    else
    {
        impactBuffer << attacker.getName()
                  << " attaque et inflige "
                  << rawDamage
                  << " dégâts bruts."
                  << std::endl;
    }

    DamageReport rapport = DamageSystem::calculateReceivedDamage(
        defender,
        rawDamage
    );

    for (const std::string& line : DamageSystem::buildDamageReportLines(defender, rapport))
    {
        impactBuffer << line << std::endl;
    }

    showCapturedCombatLines(
        "IMPACT DE L'ATTAQUE",
        "combat.attack.impact",
        impactBuffer.str()
    );

    rapport.receivedDamage = DefensePostureSystem::reduceIncomingDamage(
        defender,
        rapport.receivedDamage
    );

    Player* defendingPlayerCurseCheck = dynamic_cast<Player*>(&defender);
    if (defendingPlayerCurseCheck != nullptr && rapport.receivedDamage > 0)
    {
        const int defensePressure = defendingPlayerCurseCheck->getCursePressureForCategory("defense")
            + defendingPlayerCurseCheck->getCursePressureForCategory("health") / 2;
        if (defensePressure > 0)
        {
            const int beforeCurseDamage = rapport.receivedDamage;
            rapport.receivedDamage = std::max(1, rapport.receivedDamage * std::min(145, 100 + defensePressure * 3) / 100);
            if (rapport.receivedDamage > beforeCurseDamage)
            {
                if (defendingPlayerCurseCheck->getKnownCursePressureForCategory("defense") > 0
                    || defendingPlayerCurseCheck->getKnownCursePressureForCategory("health") > 0)
                {
                    MessageScreen::show(
                        "PRESSION MAUDITE",
                        "combat.attack.curse_defense_known",
                        {"Malédiction diagnostiquée : la protection du corps répond moins bien à l'impact."},
                        false
                    );
                }
                else
                {
                    MessageScreen::show(
                        "PRESSION INCONNUE",
                        "combat.attack.curse_defense_unknown",
                        {"Le corps encaisse étrangement mal, sans que la raison soit encore claire."},
                        false
                    );
                }
            }
        }
    }

    defender.takeDamage(rapport.receivedDamage);

    std::ostringstream followUpBuffer;

    Player* attackingPlayer = dynamic_cast<Player*>(&attacker);
    if (attackingPlayer != nullptr)
    {
        applyAmmunitionStatusIfNeeded(*attackingPlayer, defender, rapport.receivedDamage, followUpBuffer);

        Monster* defendingMonsterAfterImpact = dynamic_cast<Monster*>(&defender);
        if (defendingMonsterAfterImpact != nullptr)
        {
            MonsterBehaviorProfileCatalog::applyIncomingHitReaction(
                *defendingMonsterAfterImpact,
                attacker,
                random,
                rapport.receivedDamage,
                followUpBuffer
            );
        }
    }

    if (attackingMonster != nullptr)
    {
        applyMonsterElementalStatusIfNeeded(*attackingMonster, defender, random, rapport.receivedDamage, followUpBuffer);
    }

    Boss* defendingBoss = dynamic_cast<Boss*>(&defender);

    if (attackingPlayer != nullptr
        && attackingPlayer->hasEquippedWeapon()
        && attackingPlayer->getEquippedWeapon().getType() == WeaponType::BareHands
        && defendingBoss == nullptr
        && defender.getMaxHp() > 60)
    {
        int recoilDamage = random.between(1, 2);
        attackingPlayer->takeDamage(recoilDamage);
        followUpBuffer << attackingPlayer->getName()
                  << " se blesse en frappant à mains nues et perd "
                  << recoilDamage
                  << " PV."
                  << std::endl;
    }

    showCapturedCombatLines(
        "EFFETS APRÈS IMPACT",
        "combat.attack.follow_up",
        followUpBuffer.str()
    );

    SpecialCombatEffects::applySpecialCharacterAfterReceivingDamage(
        defender,
        rapport.receivedDamage,
        random
    );

    SpecialCombatEffects::applyDemonLifestealIfNeeded(
        attacker,
        rapport.receivedDamage
    );

    SpecialCombatEffects::applySpecialCharacterAfterDamage(
        attacker,
        defender,
        random,
        rapport.receivedDamage
    );

    MessageScreen::show(
        "RÉSULTAT DE L'ATTAQUE",
        "combat.attack.result",
        {
            defender.getName() + " reçoit " + std::to_string(rapport.receivedDamage) + " dégât(s).",
            defender.getName() + " possède maintenant " + std::to_string(defender.getHp()) + "/" + std::to_string(defender.getMaxHp()) + " PV."
        },
        true
    );
}
