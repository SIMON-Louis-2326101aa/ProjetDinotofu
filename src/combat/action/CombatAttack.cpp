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
    }

    void applyMonsterElementalStatusIfNeeded(Monster& attacker, Entity& defender, Random& random, int receivedDamage)
    {
        if (receivedDamage <= 0) return;

        const std::string raceText = attacker.getRaceText();
        const std::string typeText = attacker.getType();
        const std::string combined = raceText + " " + typeText + " " + attacker.getName();

        if (textContainsAny(combined, {"toxique", "putride", "venime", "poison"}) && random.between(1, 100) <= 28)
        {
            defender.applyPoison(2, 3 + attacker.getLevel() / 12);
            std::cout << attacker.getName() << " laisse un poison léger dans la blessure." << std::endl;
        }
        else if (textContainsAny(combined, {"brûl", "brule", "cendre", "feu", "rouge", "irritant"}) && random.between(1, 100) <= 22)
        {
            defender.applyBurning(2, 3 + attacker.getLevel() / 14);
            std::cout << attacker.getName() << " transmet une chaleur persistante." << std::endl;
        }
        else if (textContainsAny(combined, {"givre", "gel", "froid", "glace", "bleu"}) && random.between(1, 100) <= 22)
        {
            defender.applyFrost(1);
            std::cout << attacker.getName() << " ralentit sa cible avec un froid mordant." << std::endl;
        }
        else if (textContainsAny(combined, {"élect", "elect", "conduct", "orage"}) && random.between(1, 100) <= 20)
        {
            defender.applyShock(1);
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
