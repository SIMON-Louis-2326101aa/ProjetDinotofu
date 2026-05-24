// EN: CombatAttack.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatAttack.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/action/CombatAttack.hpp"

#include "combat/DamageReport.hpp"
#include "combat/system/DamageSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/action/SpecialCombatEffects.hpp"
#include "entity/Player.hpp"
#include "entity/Boss.hpp"
#include "entity/Monster.hpp"
#include "item/weapon/WeaponType.hpp"

#include <iostream>
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

    void applyAmmunitionStatusIfNeeded(Player& attacker, Entity& defender, int receivedDamage)
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
            defender.applyBleeding(2, 3 + attacker.getLevel() / 25);
            std::cout << "La munition ouvre une blessure qui saignera sur les prochains tours." << std::endl;
        }
        else if (ammoId == "piercing_bolts")
        {
            int piercingDamage = 3 + attacker.getLevel() / 20;
            defender.takeDamage(piercingDamage);
            std::cout << "Le carreau perforant traverse une protection et ajoute " << piercingDamage << " dégâts directs." << std::endl;
        }
        else if (ammoId == "ash_arrows")
        {
            defender.applyBurning(2, 4 + attacker.getLevel() / 30);
            std::cout << "La flèche de cendre accroche une brûlure faible mais persistante." << std::endl;
        }
        else if (ammoId == "frozen_bolts")
        {
            defender.applyFrost(2);
            std::cout << "Le carreau givré ralentit la cible : le froid restera un court instant." << std::endl;
        }
        else if (ammoId == "conductive_knives")
        {
            defender.applyShock(2);
            int shockDamage = 2 + attacker.getLevel() / 35;
            Player* defendingPlayer = dynamic_cast<Player*>(&defender);
            if (defendingPlayer != nullptr)
            {
                int metalPieces = countPlayerMetalEquipment(*defendingPlayer);
                if (metalPieces > 0)
                {
                    shockDamage += 3 * metalPieces;
                    std::cout << "Le métal équipé amplifie la conduction électrique." << std::endl;
                }
            }
            defender.takeDamage(shockDamage);
            std::cout << "Le couteau conducteur ajoute " << shockDamage << " dégâts électriques immédiats." << std::endl;
        }
        else if (ammoId == "venom_arrows")
        {
            defender.applyPoison(3, 2 + attacker.getLevel() / 18);
            std::cout << "Le venin de la flèche s'accroche : poison léger sur plusieurs tours." << std::endl;
        }
        else if (ammoId == "shock_bolts")
        {
            defender.applyShock(2);
            int shockDamage = 4 + attacker.getLevel() / 24;
            Player* defendingPlayer = dynamic_cast<Player*>(&defender);
            if (defendingPlayer != nullptr)
            {
                int metalPieces = countPlayerMetalEquipment(*defendingPlayer);
                shockDamage += 4 * metalPieces;
                if (metalPieces > 0)
                {
                    std::cout << "Le carreau conducteur mord dans le métal équipé." << std::endl;
                }
            }
            defender.takeDamage(shockDamage);
            std::cout << "La pointe conductrice ajoute " << shockDamage << " dégâts électriques." << std::endl;
        }
        else if (ammoId == "smoke_knives")
        {
            defender.applyFrost(1);
            attacker.startDefensePosture(10, 8, "Écran de fumée court");
            std::cout << "Le couteau fumigène gêne la cible et donne une petite fenêtre défensive au lanceur." << std::endl;
        }
    }

    void applyMonsterElementalStatusIfNeeded(Monster& attacker, Entity& defender, Random& random, int receivedDamage)
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
                defender.applyPoison(2, 2 + attacker.getLevel() / 14);
                std::cout << attacker.getName() << " change de couleur et laisse un poison instable." << std::endl;
            }
            else if (effectRoll == 2)
            {
                defender.applyBurning(2, 2 + attacker.getLevel() / 16);
                std::cout << attacker.getName() << " pulse rouge et accroche une chaleur anormale." << std::endl;
            }
            else if (effectRoll == 3)
            {
                defender.applyFrost(2);
                std::cout << attacker.getName() << " devient pâle et ralentit la cible." << std::endl;
            }
            else
            {
                defender.applyShock(2);
                std::cout << attacker.getName() << " vibre comme du verre chargé d'électricité." << std::endl;
            }
        }
        else if (textContainsAny(combined, {"ambré", "ambre", "collant", "poisseux"}) && random.between(1, 100) <= 34)
        {
            defender.applyFrost(1);
            std::cout << attacker.getName() << " colle à la cible : ce n'est pas du givre, mais les mouvements deviennent lourds." << std::endl;
        }
        else if (textContainsAny(combined, {"toxique", "putride", "venime", "poison", "violet", "noir", "vaseux"}) && random.between(1, 100) <= 28)
        {
            defender.applyPoison(2, 3 + attacker.getLevel() / 12);
            std::cout << attacker.getName() << " laisse un poison léger dans la blessure." << std::endl;
        }
        else if (textContainsAny(combined, {"brûl", "brule", "cendre", "feu", "rouge", "irritant", "chaud"}) && random.between(1, 100) <= 24)
        {
            defender.applyBurning(2, 3 + attacker.getLevel() / 14);
            std::cout << attacker.getName() << " transmet une chaleur persistante." << std::endl;
        }
        else if (textContainsAny(combined, {"givre", "gel", "froid", "glace", "bleu", "blanc"}) && random.between(1, 100) <= 24)
        {
            defender.applyFrost(2);
            std::cout << attacker.getName() << " ralentit sa cible avec un froid mordant." << std::endl;
        }
        else if (textContainsAny(combined, {"élect", "elect", "conduct", "orage", "jaune", "vibrant", "chromatique"}) && random.between(1, 100) <= 22)
        {
            defender.applyShock(2);
            int shockDamage = 2 + attacker.getLevel() / 16;
            Player* defendingPlayer = dynamic_cast<Player*>(&defender);
            if (defendingPlayer != nullptr)
            {
                int metalPieces = countPlayerMetalEquipment(*defendingPlayer);
                if (metalPieces > 0)
                {
                    shockDamage += 3 * metalPieces;
                    std::cout << "L'équipement métallique attire une partie de la décharge." << std::endl;
                }
            }
            defender.takeDamage(shockDamage);
            std::cout << attacker.getName() << " ajoute " << shockDamage << " dégâts électriques." << std::endl;
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
        std::cout << attacker.getName() << " ne peut pas agir : les effets en cours l'ont mis au sol." << std::endl << std::endl;
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
        std::cout << attacker.getName()
                  << " attaque, mais "
                  << defender.getName()
                  << " esquive au dernier moment."
                  << std::endl;
        std::cout << std::endl;
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

        std::string classFocus = CombatClassSystem::normalizeClassText(attacker.getType());

        const int classLevel = attackingPlayerIdentity->getLevel();

        if (classLevel >= 5
            && (classFocus.find("guerrier") != std::string::npos
                || classFocus.find("chevalier") != std::string::npos
                || classFocus.find("duelliste") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            rawDamage += 2 + classLevel / 30;
            std::cout << "Technique apprise : enchaînement simple, le geste s'enchaîne mieux grâce à l'expérience." << std::endl;
        }

        if (classLevel >= 8
            && (classFocus.find("assassin") != std::string::npos
                || classFocus.find("ombrelame") != std::string::npos)
            && random.between(1, 100) <= 16)
        {
            defender.applyBleeding(1, 1 + classLevel / 40);
            rawDamage += 1;
            std::cout << "Technique apprise : incision discrète, une blessure courte s'ajoute à la frappe." << std::endl;
        }

        if (classLevel >= 10
            && (classFocus.find("gardien") != std::string::npos
                || classFocus.find("colosse") != std::string::npos
                || classFocus.find("paladin") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            attacker.startDefensePosture(10, 2, "Posture apprise de rempart");
            std::cout << "Technique apprise : rempart court, le combattant finit son attaque en garde." << std::endl;
        }

        if (classLevel >= 12
            && (classFocus.find("mage") != std::string::npos
                || classFocus.find("arcaniste") != std::string::npos
                || classFocus.find("sorcier") != std::string::npos)
            && random.between(1, 100) <= 15)
        {
            int roll = random.between(1, 4);
            if (roll == 1) defender.applyBurning(1, 1 + classLevel / 32);
            else if (roll == 2) defender.applyPoison(1, 1 + classLevel / 32);
            else if (roll == 3) defender.applyFrost(1);
            else defender.applyShock(1);
            std::cout << "Technique apprise : trace élémentaire, la magie suit le geste physique." << std::endl;
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
            std::cout << "Technique apprise : tir cadré, le bonus existe seulement car l'arme équipée le permet." << std::endl;
        }

        if ((classFocus.find("assassin") != std::string::npos
            || classFocus.find("ombrelame") != std::string::npos
            || classFocus.find("lanceur de dagues") != std::string::npos)
            && random.between(1, 100) <= (critical ? 45 : 18))
        {
            defender.applyBleeding(2, 2 + attackingPlayerIdentity->getLevel() / 35);
            std::cout << "Spécialité furtive : la frappe cherche une veine et prépare un saignement." << std::endl;
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
            std::cout << "Spécialité sacrée : l'effort referme légèrement les blessures du combattant." << std::endl;
        }

        if ((classFocus.find("gardien") != std::string::npos
            || classFocus.find("colosse") != std::string::npos
            || classFocus.find("tank") != std::string::npos)
            && random.between(1, 100) <= 24)
        {
            attacker.startDefensePosture(8, 3, "Ancrage de classe lourde");
            std::cout << "Spécialité lourde : le combattant s'ancre après son coup." << std::endl;
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
                defender.applyBurning(1, 2 + attackingPlayerIdentity->getLevel() / 28);
                std::cout << "Spécialité magique : une braise instable reste accrochée à la cible." << std::endl;
            }
            else if (effectRoll == 2)
            {
                defender.applyFrost(1);
                std::cout << "Spécialité magique : un froid bref gêne le mouvement adverse." << std::endl;
            }
            else
            {
                defender.applyShock(1);
                std::cout << "Spécialité magique : une perturbation électrique traverse l'impact." << std::endl;
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
            std::cout << "Spécialité à distance : le tir est cadré par l'arme équipée, pas par la classe seule." << std::endl;
        }

        if ((classFocus.find("lancier") != std::string::npos
            || classFocus.find("spear") != std::string::npos
            || classFocus.find("garde") != std::string::npos)
            && attackingPlayerIdentity->hasEquippedWeapon()
            && attackingPlayerIdentity->getEquippedWeapon().getType() == WeaponType::Spear
            && random.between(1, 100) <= 22)
        {
            attacker.startDefensePosture(6, 2, "Garde de lance");
            std::cout << "Spécialité de lance : la portée garde l'adversaire à distance après l'impact." << std::endl;
        }

        if ((classFocus.find("barbare") != std::string::npos
            || classFocus.find("berserker") != std::string::npos
            || classFocus.find("briseur") != std::string::npos)
            && attacker.getHp() * 100 <= attacker.getMaxHp() * 55
            && random.between(1, 100) <= 28)
        {
            rawDamage = rawDamage * 112 / 100 + 2;
            std::cout << "Spécialité brutale : plus le combattant est blessé, plus il force l'impact." << std::endl;
        }

        if ((classFocus.find("alchimiste") != std::string::npos
            || classFocus.find("artificier") != std::string::npos
            || classFocus.find("bricoleur") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            int effectRoll = random.between(1, 4);
            if (effectRoll == 1)
            {
                defender.applyPoison(1, 1 + attackingPlayerIdentity->getLevel() / 35);
                std::cout << "Spécialité d'artisanat : un résidu expérimental empoisonne légèrement la cible." << std::endl;
            }
            else if (effectRoll == 2)
            {
                defender.applyBurning(1, 1 + attackingPlayerIdentity->getLevel() / 35);
                std::cout << "Spécialité d'artisanat : une étincelle chimique reste sur la blessure." << std::endl;
            }
            else
            {
                rawDamage += 2;
                std::cout << "Spécialité d'artisanat : le coup vise une faiblesse matérielle." << std::endl;
            }
        }

        if ((classFocus.find("invoc") != std::string::npos
            || classFocus.find("nécro") != std::string::npos
            || classFocus.find("necro") != std::string::npos)
            && random.between(1, 100) <= 18)
        {
            defender.applyFrost(1);
            std::cout << "Spécialité d'invocateur : l'attaque laisse une pression froide, comme une présence derrière la cible." << std::endl;
        }

        if (rawDamage != beforeSpecialityDamage)
        {
            std::cout << "La spécialité de classe modifie l'impact de l'attaque."
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
            std::cout << attackingMonster->getName() << " harcèle à distance au lieu de charger bêtement." << std::endl;
        }

        if (textContainsAny(monsterFocus, {"garde", "sentinelle", "armure", "golem", "construction"}) && random.between(1, 100) <= 24)
        {
            attacker.startDefensePosture(10, 2, "Garde monstrueuse");
            std::cout << attackingMonster->getName() << " protège son noyau/faiblesse après avoir frappé." << std::endl;
        }

        if (textContainsAny(monsterFocus, {"loup", "prédateur", "predateur", "moustique", "sangsue"})
            && defender.getHp() * 100 <= defender.getMaxHp() * 45
            && random.between(1, 100) <= 30)
        {
            rawDamage += 3 + attackingMonster->getLevel() / 10;
            std::cout << attackingMonster->getName() << " sent la proie blessée et devient plus agressif." << std::endl;
        }

        if (textContainsAny(monsterFocus, {"champignon hurleur", "tambour", "oracle"}) && random.between(1, 100) <= 18)
        {
            rawDamage += 1;
            std::cout << attackingMonster->getName() << " perturbe le rythme du combat : son rôle de support se ressent même seul." << std::endl;
        }

        if (textContainsAny(attackingMonster->getName() + " " + typeText, {"shaman", "chamane", "oracle", "apothicaire"})
            && random.between(1, 100) <= 26)
        {
            int selfHeal = std::max(2, 4 + attackingMonster->getLevel() / 8);
            attacker.heal(selfHeal);
            std::cout << attackingMonster->getName() << " gaspille une fiole ou une prière mineure pour rester debout." << std::endl;
        }

        if (raceText.find("Gobelin") != std::string::npos && defender.getHp() * 2 <= defender.getMaxHp())
        {
            rawDamage += 3;
            std::cout << attackingMonster->getName() << " sent la faiblesse et frappe comme un pillard opportuniste." << std::endl;
        }
        else if (raceText.find("Bête") != std::string::npos || typeText.find("Prédateur") != std::string::npos || typeText.find("rapide") != std::string::npos)
        {
            if (random.between(1, 100) <= 25)
            {
                rawDamage += 4;
                std::cout << attackingMonster->getName() << " profite de sa vitesse naturelle." << std::endl;
            }
        }
        else if (raceText.find("Mort-vivant") != std::string::npos)
        {
            if (random.between(1, 100) <= 20)
            {
                rawDamage += 5;
                std::cout << "Une force froide accompagne le coup du mort-vivant." << std::endl;
            }
        }
        else if (raceText.find("Slime") != std::string::npos)
        {
            if (typeText.find("toxique") != std::string::npos || typeText.find("putride") != std::string::npos)
            {
                rawDamage += 4;
                std::cout << attackingMonster->getName() << " laisse une matière nocive sur la blessure." << std::endl;
            }
            else if (typeText.find("froide") != std::string::npos || typeText.find("gelée froide") != std::string::npos)
            {
                rawDamage += 3;
                std::cout << attackingMonster->getName() << " refroidit brutalement l'impact." << std::endl;
            }
            else if (typeText.find("brillante") != std::string::npos || typeText.find("or") != std::string::npos)
            {
                if (random.between(1, 100) <= 35)
                {
                    rawDamage += 5;
                    std::cout << attackingMonster->getName() << " attaque en protégeant ce qui brille en lui." << std::endl;
                }
            }
            else if (typeText.find("bond") != std::string::npos || attackingMonster->getName().find("rose") != std::string::npos)
            {
                if (random.between(1, 100) <= 35)
                {
                    rawDamage += 4;
                    std::cout << attackingMonster->getName() << " rebondit dans un angle absurde avant l'impact." << std::endl;
                }
            }
            else if (typeText.find("coll") != std::string::npos || attackingMonster->getName().find("ambr") != std::string::npos)
            {
                rawDamage += 2;
                std::cout << attackingMonster->getName() << " rend l'esquive pénible avec sa matière collante." << std::endl;
            }
            else if (random.between(1, 100) <= 18)
            {
                rawDamage += 2;
                std::cout << attackingMonster->getName() << " rebondit et rend le choc moins prévisible." << std::endl;
            }
        }
        else if (raceText.find("Plante") != std::string::npos)
        {
            if (random.between(1, 100) <= 25)
            {
                rawDamage += 3;
                std::cout << attackingMonster->getName() << " accroche sa cible avec des fibres végétales." << std::endl;
            }
        }
        else if (raceText.find("Insectoïde") != std::string::npos)
        {
            if (random.between(1, 100) <= 28)
            {
                rawDamage += 4;
                std::cout << attackingMonster->getName() << " pique dans un angle difficile à protéger." << std::endl;
            }
        }
        else if (raceText.find("Élémentaire") != std::string::npos || raceText.find("Démon") != std::string::npos)
        {
            if (random.between(1, 100) <= 22)
            {
                rawDamage += 6;
                std::cout << attackingMonster->getName() << " libère une surcharge instable." << std::endl;
            }
        }
        else if (raceText.find("Orc") != std::string::npos || raceText.find("Hobgobelin") != std::string::npos)
        {
            if (attackingMonster->getHp() * 2 <= attackingMonster->getMaxHp())
            {
                rawDamage += 6;
                std::cout << attackingMonster->getName() << " devient plus violent en étant blessé." << std::endl;
            }
            else if (random.between(1, 100) <= 22)
            {
                rawDamage += 3;
                std::cout << attackingMonster->getName() << " impose sa force brute." << std::endl;
            }
        }
        else if (raceText.find("Construction") != std::string::npos)
        {
            rawDamage += 2;
            attacker.startDefensePosture(12, 2, "Carapace de construction");
            std::cout << attackingMonster->getName() << " frappe lourdement et reste difficile à entamer." << std::endl;
        }
        else if (raceText.find("Anomalie") != std::string::npos)
        {
            int anomalyRoll = random.between(1, 4);
            rawDamage += anomalyRoll * 2;
            if (anomalyRoll == 4)
            {
                defender.applyShock(1);
                std::cout << "L'anomalie déforme l'impact et laisse une perturbation électrique." << std::endl;
            }
            else
            {
                std::cout << "L'anomalie rend les dégâts difficiles à prévoir." << std::endl;
            }
        }
        else if (raceText.find("Dragon") != std::string::npos || raceText.find("Draconide") != std::string::npos)
        {
            rawDamage = rawDamage * 108 / 100 + 2;
            std::cout << "La puissance draconique rend le coup plus lourd." << std::endl;
        }

        if (rawDamage < 1)
        {
            rawDamage = 1;
        }

        if (rawDamage != beforeMonsterSpecialityDamage && attackingMonster->isElite())
        {
            std::cout << "Son statut d'élite rend cette spécialité encore plus inquiétante." << std::endl;
        }
    }

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

    if (critical)
    {
        std::cout << attacker.getName()
                  << " frappe avec une violence monstrueuse et inflige "
                  << rawDamage
                  << " dégâts bruts critiques."
                  << std::endl;
    }
    else if (damageBonus > 0)
    {
        std::cout << attacker.getName()
                  << " attaque avec une puissance renforcée et inflige "
                  << rawDamage
                  << " dégâts bruts."
                  << std::endl;
    }
    else
    {
        std::cout << attacker.getName()
                  << " attaque et inflige "
                  << rawDamage
                  << " dégâts bruts."
                  << std::endl;
    }

    DamageReport rapport = DamageSystem::calculateReceivedDamage(
        defender,
        rawDamage
    );

    DamageSystem::displayDamageReport(
        defender,
        rapport
    );

    rapport.receivedDamage = DefensePostureSystem::reduceIncomingDamage(
        defender,
        rapport.receivedDamage
    );

    defender.takeDamage(rapport.receivedDamage);

    Player* attackingPlayer = dynamic_cast<Player*>(&attacker);
    if (attackingPlayer != nullptr)
    {
        applyAmmunitionStatusIfNeeded(*attackingPlayer, defender, rapport.receivedDamage);
    }

    if (attackingMonster != nullptr)
    {
        applyMonsterElementalStatusIfNeeded(*attackingMonster, defender, random, rapport.receivedDamage);
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
        std::cout << attackingPlayer->getName()
                  << " se blesse en frappant à mains nues et perd "
                  << recoilDamage
                  << " PV."
                  << std::endl;
    }

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

    std::cout << defender.getName()
              << " reçoit "
              << rapport.receivedDamage
              << " dégâts."
              << std::endl;

    std::cout << defender.getName()
              << " possède maintenant "
              << defender.getHp()
              << "/"
              << defender.getMaxHp()
              << " PV."
              << std::endl;

    std::cout << std::endl;
}
