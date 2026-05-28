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

    void showCapturedCombatLines(
        const std::string& title,
        const std::string& screenId,
        const std::string& capturedText
    )
    {
        std::vector<std::string> lines = splitCapturedCombatLines(capturedText);
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, false);
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

    bool dodged = false;
    bool critical = false;

    int rawDamage = attacker.attack(
        random,
        dodged,
        critical,
        damageBonus
    );

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

    std::ostringstream preparationBuffer;

    Player* attackingPlayerIdentity = dynamic_cast<Player*>(&attacker);
    Monster* attackingMonster = dynamic_cast<Monster*>(&attacker);

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
            preparationBuffer << "Technique apprise : enchaînement simple, le geste s'enchaîne mieux grâce à l'expérience." << std::endl;
        }

        if (classLevel >= 8
            && (classFocus.find("assassin") != std::string::npos
                || classFocus.find("ombrelame") != std::string::npos)
            && random.between(1, 100) <= 16)
        {
            ElementalAffinitySystem::applyBleeding(defender, 1, 1 + classLevel / 40);
            rawDamage += 1;
            preparationBuffer << "Technique apprise : incision discrète, une blessure courte s'ajoute à la frappe." << std::endl;
        }

        if (classLevel >= 10
            && (classFocus.find("gardien") != std::string::npos
                || classFocus.find("colosse") != std::string::npos
                || classFocus.find("paladin") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            attacker.startDefensePosture(10, 2, "Posture apprise de rempart");
            preparationBuffer << "Technique apprise : rempart court, le combattant finit son attaque en garde." << std::endl;
        }

        if (classLevel >= 12
            && (classFocus.find("mage") != std::string::npos
                || classFocus.find("arcaniste") != std::string::npos
                || classFocus.find("sorcier") != std::string::npos)
            && random.between(1, 100) <= 15)
        {
            int roll = random.between(1, 4);
            if (roll == 1) ElementalAffinitySystem::applyBurning(defender, 1, 1 + classLevel / 32);
            else if (roll == 2) ElementalAffinitySystem::applyPoison(defender, 1, 1 + classLevel / 32);
            else if (roll == 3) ElementalAffinitySystem::applyFrost(defender, 1);
            else ElementalAffinitySystem::applyShock(defender, 1);
            preparationBuffer << "Technique apprise : trace élémentaire, la magie suit le geste physique." << std::endl;
        }

        if (classLevel >= 15
            && (classFocus.find("rôdeur") != std::string::npos
                || classFocus.find("rodeur") != std::string::npos
                || classFocus.find("archer") != std::string::npos
                || classFocus.find("tireur") != std::string::npos)
            && attackingPlayerIdentity->hasEquippedWeapon()
            && (attackingPlayerIdentity->getEquippedWeapon().getType() == WeaponType::Bow
                || textContainsAny(attackingPlayerIdentity->getEquippedWeapon().getName(), {"arbal", "lancer", "bandoulière", "bandouliere"}))
            && random.between(1, 100) <= 20)
        {
            rawDamage += 3 + classLevel / 28;
            preparationBuffer << "Technique apprise : tir cadré, le bonus existe seulement car l'arme équipée le permet." << std::endl;
        }

        if ((classFocus.find("assassin") != std::string::npos
            || classFocus.find("ombrelame") != std::string::npos
            || classFocus.find("lanceur de dagues") != std::string::npos)
            && random.between(1, 100) <= (critical ? 45 : 18))
        {
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + attackingPlayerIdentity->getLevel() / 35);
            preparationBuffer << "Spécialité furtive : la frappe cherche une veine et prépare un saignement." << std::endl;
        }

        if ((classFocus.find("paladin") != std::string::npos
            || classFocus.find("clerc") != std::string::npos
            || classFocus.find("prêtre") != std::string::npos
            || classFocus.find("pretre") != std::string::npos)
            && attacker.getHp() * 2 <= attacker.getMaxHp()
            && random.between(1, 100) <= 30)
        {
            int selfHeal = std::max(1, 3 + attackingPlayerIdentity->getLevel() / 18);
            attacker.heal(selfHeal);
            preparationBuffer << "Spécialité sacrée : l'effort referme légèrement les blessures du combattant." << std::endl;
        }

        if ((classFocus.find("gardien") != std::string::npos
            || classFocus.find("colosse") != std::string::npos
            || classFocus.find("tank") != std::string::npos)
            && random.between(1, 100) <= 24)
        {
            attacker.startDefensePosture(8, 3, "Ancrage de classe lourde");
            preparationBuffer << "Spécialité lourde : le combattant s'ancre après son coup." << std::endl;
        }

        if ((classFocus.find("mage") != std::string::npos
            || classFocus.find("arcaniste") != std::string::npos
            || classFocus.find("sorcier") != std::string::npos
            || classFocus.find("pyromancien") != std::string::npos)
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
            || classFocus.find("tireur") != std::string::npos)
            && attackingPlayerIdentity->hasEquippedWeapon()
            && attackingPlayerIdentity->getEquippedWeapon().getType() == WeaponType::Bow
            && random.between(1, 100) <= 26)
        {
            rawDamage += std::max(1, 2 + attackingPlayerIdentity->getLevel() / 25);
            preparationBuffer << "Spécialité à distance : le tir suit enfin la ligne de l'arme équipée." << std::endl;
        }

        if ((classFocus.find("lancier") != std::string::npos
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
            || classFocus.find("briseur") != std::string::npos)
            && attacker.getHp() * 100 <= attacker.getMaxHp() * 55
            && random.between(1, 100) <= 28)
        {
            rawDamage = rawDamage * 112 / 100 + 2;
            preparationBuffer << "Spécialité brutale : plus le combattant est blessé, plus il force l'impact." << std::endl;
        }

        if ((classFocus.find("alchimiste") != std::string::npos
            || classFocus.find("artificier") != std::string::npos
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
            || classFocus.find("necro") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            ElementalAffinitySystem::applyFrost(defender, 1);
            preparationBuffer << "Spécialité d'invocateur : l'attaque laisse une pression froide, comme une présence derrière la cible." << std::endl;
        }

        if (rawDamage != beforeSpecialityDamage)
        {
            preparationBuffer << "La spécialité de classe change l'impact de l'attaque."
                      << std::endl;
        }
    }

    if (attackingMonster != nullptr)
    {
        int beforeMonsterSpecialityDamage = rawDamage;
        const std::string raceText = attackingMonster->getRaceText();
        const std::string typeText = attackingMonster->getType();
        const std::string monsterFocus = attackingMonster->getName() + " " + typeText + " " + raceText;

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

    if (critical)
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

    defender.takeDamage(rapport.receivedDamage);

    std::ostringstream followUpBuffer;

    Player* attackingPlayer = dynamic_cast<Player*>(&attacker);
    if (attackingPlayer != nullptr)
    {
        applyAmmunitionStatusIfNeeded(*attackingPlayer, defender, rapport.receivedDamage, followUpBuffer);
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
        false
    );
}
