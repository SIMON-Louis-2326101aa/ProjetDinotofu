// EN: CombatActions.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatActions.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/CombatActions.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/action/CombatPotion.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "core/Console.hpp"

#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"
#include "item/weapon/WeaponType.hpp"

#include <iostream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <initializer_list>


namespace
{
    struct AmmunitionChoice
    {
        std::string id;
        std::string label;
        int count;
        bool special;
    };

    std::string normalizeCombatActionText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool playerUsesChoiceBasedAmmunition(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return false;
        }

        return player.getEquippedWeapon().getType() == WeaponType::Bow;
    }

    std::vector<AmmunitionChoice> buildAmmunitionChoices(const Player& player)
    {
        std::string weaponName = normalizeCombatActionText(player.getEquippedWeapon().getName());

        std::string trainingId = "training_arrows";
        std::string trainingLabel = "Flèches d'entraînement";
        std::string specialId = "barbed_arrows";
        std::string specialLabel = "Flèches barbelées";
        std::string elementalId = "ash_arrows";
        std::string elementalLabel = "Flèches de cendre";
        std::string specialistId = "venom_arrows";
        std::string specialistLabel = "Flèches enduites de venin";

        if (weaponName.find("arbal") != std::string::npos || weaponName.find("carreau") != std::string::npos)
        {
            trainingId = "training_bolts";
            trainingLabel = "Carreaux d'entraînement";
            specialId = "piercing_bolts";
            specialLabel = "Carreaux perforants";
            elementalId = "frozen_bolts";
            elementalLabel = "Carreaux givrés";
            specialistId = "shock_bolts";
            specialistLabel = "Carreaux à pointe conductrice";
        }
        else if (weaponName.find("lancer") != std::string::npos || weaponName.find("couteau") != std::string::npos || weaponName.find("bandouli") != std::string::npos)
        {
            trainingId = "training_throwing_knives";
            trainingLabel = "Couteaux de lancer émoussés";
            specialId = "balanced_throwing_knives";
            specialLabel = "Couteaux de lancer équilibrés";
            elementalId = "conductive_knives";
            elementalLabel = "Couteaux conducteurs";
            specialistId = "smoke_knives";
            specialistLabel = "Couteaux fumigènes";
        }

        std::vector<AmmunitionChoice> choices;
        choices.push_back({trainingId, trainingLabel, player.getInventory().countMaterialById(trainingId), false});
        choices.push_back({specialId, specialLabel, player.getInventory().countMaterialById(specialId), true});
        choices.push_back({elementalId, elementalLabel, player.getInventory().countMaterialById(elementalId), true});
        choices.push_back({specialistId, specialistLabel, player.getInventory().countMaterialById(specialistId), true});
        return choices;
    }

    int magicCatalystSuccessChance(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return 70;
        }

        Weapon weapon = player.getEquippedWeapon();
        if (weapon.getType() != WeaponType::Staff)
        {
            return 68;
        }

        if (weapon.isBroken())
        {
            return 42;
        }

        int chance = 94;
        if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0)
        {
            int durabilityPercent = weapon.getDurability() * 100 / weapon.getMaxDurability();
            if (durabilityPercent <= 15) chance -= 26;
            else if (durabilityPercent <= 30) chance -= 16;
            else if (durabilityPercent <= 50) chance -= 7;
        }

        std::string weaponName = normalizeCombatActionText(weapon.getName());
        if (weaponName.find("instable") != std::string::npos)
        {
            chance -= 6;
        }

        if (chance < 35) chance = 35;
        if (chance > 98) chance = 98;
        return chance;
    }

    bool resolveMagicCatalystAttempt(Player* player, const std::string& spellName, Random& random)
    {
        if (player == nullptr)
        {
            return true;
        }

        int successChance = magicCatalystSuccessChance(*player);
        std::cout << "Canalisation : " << spellName << "." << std::endl;
        std::cout << "Stabilité du catalyseur : " << successChance << "%" << std::endl;

        if (player->hasEquippedWeapon())
        {
            Weapon weapon = player->getEquippedWeapon();
            if (weapon.getType() != WeaponType::Staff)
            {
                std::cout << "L'arme équipée n'est pas un vrai catalyseur : la formule répond moins bien." << std::endl;
            }
            else if (weapon.isBroken())
            {
                std::cout << "Le catalyseur est brisé : la magie passe par des fissures dangereuses." << std::endl;
            }
            else if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0 && weapon.getDurability() * 100 / weapon.getMaxDurability() <= 30)
            {
                std::cout << "Le catalyseur est presque épuisé : la réussite devient moins sûre." << std::endl;
            }
        }
        else
        {
            std::cout << "Aucun catalyseur équipé : le sort est forcé à mains nues." << std::endl;
        }

        if (random.between(1, 100) <= successChance)
        {
            return true;
        }

        std::cout << "Le flux se casse avant de former le sort. Rien ne se stabilise." << std::endl;
        std::cout << std::endl;
        return false;
    }

    void askAmmunitionChoiceIfNeeded(Entity& attacker)
    {
        Player* player = dynamic_cast<Player*>(&attacker);
        if (player == nullptr || !playerUsesChoiceBasedAmmunition(*player))
        {
            return;
        }

        std::vector<AmmunitionChoice> choices = buildAmmunitionChoices(*player);

        bool hasAvailableAmmunition = false;
        for (const AmmunitionChoice& choice : choices)
        {
            if (choice.count > 0)
            {
                hasAvailableAmmunition = true;
                break;
            }
        }

        std::cout << "========== CHOIX DES MUNITIONS ==========" << std::endl;
        std::cout << "Chaque tir consomme une munition. Choisis ce que tu veux utiliser." << std::endl;

        if (!hasAvailableAmmunition)
        {
            std::cout << "Aucune munition compatible." << std::endl;
            std::cout << "1 : Utiliser l'arme en défense d'urgence" << std::endl;
            std::cout << "0 : Annuler" << std::endl;
            std::cout << "=========================================" << std::endl;
            std::cout << "> ";

            int emergencyChoice = Console::askNumberBetween(
                0,
                1,
                "Choix invalide. Choisis 0 pour annuler ou 1 pour la défense d'urgence."
            );

            if (emergencyChoice == 0)
            {
                player->setNextAmmunitionChoice("__cancel_attack__");
                std::cout << "Tu baisses ton arme à distance : aucune munition, aucune attaque." << std::endl;
                std::cout << std::endl;
                return;
            }

            player->setNextAmmunitionChoice("__emergency_defense__");
            std::cout << "Tu utilises ton arme en défense d'urgence : pas de tir magique, seulement un coup de crosse/branche si l'ennemi se rapproche." << std::endl;
            std::cout << std::endl;
            return;
        }

        std::cout << "0 : Annuler" << std::endl;
        for (std::size_t i = 0; i < choices.size(); ++i)
        {
            std::cout << (i + 1) << " : " << choices[i].label
                      << " x" << choices[i].count;
            if (choices[i].special)
            {
                std::cout << " [spéciale]";
            }
            if (choices[i].count <= 0)
            {
                std::cout << " [indisponible]";
            }
            std::cout << std::endl;
        }
        std::cout << "=========================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            static_cast<int>(choices.size()),
            "Choix invalide. Choisis une munition affichée."
        );

        if (choice == 0)
        {
            player->setNextAmmunitionChoice("__cancel_attack__");
            std::cout << "Tir annulé : tu ne dépenses aucune munition." << std::endl;
            std::cout << std::endl;
            return;
        }

        const AmmunitionChoice& selected = choices[choice - 1];
        if (selected.count <= 0)
        {
            player->setNextAmmunitionChoice("__cancel_attack__");
            std::cout << "Tu n'as plus cette munition. Le tir est annulé pour éviter de gaspiller ton tour bêtement." << std::endl;
            std::cout << std::endl;
            return;
        }

        player->setNextAmmunitionChoice(selected.id);
        std::cout << "Munition choisie : " << selected.label << " x" << selected.count << "." << std::endl;
        std::cout << std::endl;
    }
}

void CombatActions::executeAttack(
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

void CombatActions::executeBoostedAttack(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int damageBonus
)
{
    askAmmunitionChoiceIfNeeded(attacker);

    CombatAttack::executeBoostedAttack(
        attacker,
        defender,
        random,
        damageBonus
    );
}


void CombatActions::executeWeaponTechnique(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    int bonus = 5;
    std::string techniqueName = "Technique improvisée";

    Player* player = dynamic_cast<Player*>(&attacker);
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());

    if (player != nullptr && player->hasEquippedWeapon())
    {
        WeaponType weaponType = player->getEquippedWeapon().getType();

        if (weaponType == WeaponType::Dagger)
        {
            bonus = 7;
            techniqueName = "Entaille vive";
        }
        else if (weaponType == WeaponType::Spear)
        {
            bonus = 8;
            techniqueName = "Poussée de contrôle";
        }
        else if (weaponType == WeaponType::Bow)
        {
            bonus = 9;
            techniqueName = "Tir préparé";
        }
        else if (weaponType == WeaponType::Staff)
        {
            bonus = 8;
            techniqueName = "Canalisation prudente";
        }
        else if (weaponType == WeaponType::Axe || weaponType == WeaponType::Hammer)
        {
            bonus = 11;
            techniqueName = "Coup brise-garde";
        }
        else if (weaponType == WeaponType::Sword)
        {
            bonus = 7;
            techniqueName = "Coupe maîtrisée";
        }
        else if (weaponType == WeaponType::BareHands)
        {
            bonus = 3;
            techniqueName = "Frappe réflexe";
        }

        if (weaponType == WeaponType::Dagger && player->hasActiveSkill("chain_execution"))
        {
            bonus += 4;
            techniqueName += " / enchaînement appris";
            ElementalAffinitySystem::applyBleeding(defender, 1, 1 + std::max(1, player->getLevel() / 35));
            std::cout << "Le geste à la dague se prolonge en entaille courte." << std::endl;
        }
        else if (weaponType == WeaponType::Staff && player->hasActiveSkill("cautious_channeling"))
        {
            bonus += 3;
            attacker.startDefensePosture(8, 4, "Canalisation prudente");
            techniqueName += " / canalisation stable";
            std::cout << "Le bâton canalise sans exposer complètement son porteur." << std::endl;
        }
        else if (weaponType == WeaponType::BareHands && player->hasActiveSkill("reflex_counter"))
        {
            bonus += 2;
            attacker.startDefensePosture(10, 12, "Contre réflexe");
            techniqueName += " / contre prêt";
            std::cout << "Le corps garde assez d'élan pour répondre si l'ennemi rate." << std::endl;
        }
        else if (weaponType == WeaponType::Axe && player->hasActiveSkill("splitting_blow"))
        {
            bonus += 5;
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + std::max(1, player->getLevel() / 30));
            techniqueName += " / frappe fendue";
            std::cout << "La hache ouvre une trajectoire large qui mord après l'impact." << std::endl;
        }
        else if (weaponType == WeaponType::Hammer && player->hasActiveSkill("armor_crack"))
        {
            bonus += 5;
            ElementalAffinitySystem::applyShock(defender, 1);
            techniqueName += " / fracasse-garde";
            std::cout << "Le choc du marteau casse le rythme défensif adverse." << std::endl;
        }

        if (weaponType == WeaponType::Sword && player->hasPassiveSkill("blade_discipline"))
        {
            bonus += 3;
            techniqueName += " / discipline de lame";
            std::cout << "La lame reste dans un axe propre : le coup perd moins de force." << std::endl;
        }
        else if (weaponType == WeaponType::Spear && player->hasPassiveSkill("reach_control"))
        {
            bonus += 2;
            attacker.startDefensePosture(6, 5, "Contrôle d'allonge");
            techniqueName += " / allonge tenue";
            std::cout << "La lance garde une distance sûre après la poussée." << std::endl;
        }
    }

    if (className.find("assassin") != std::string::npos || className.find("ombrelame") != std::string::npos)
    {
        bonus += 6;
        techniqueName += " / angle mort";
    }
    else if (className.find("lancier") != std::string::npos)
    {
        bonus += 4;
        techniqueName += " / distance royale";
    }
    else if (className.find("archer") != std::string::npos || className.find("rôdeur") != std::string::npos || className.find("rodeur") != std::string::npos || className.find("arbal") != std::string::npos || className.find("tireur") != std::string::npos)
    {
        bonus += 5;
        techniqueName += " / visée de classe";
    }
    else if (className.find("mage") != std::string::npos || className.find("sorcier") != std::string::npos || className.find("arcaniste") != std::string::npos)
    {
        bonus += 7;
        techniqueName += " / surcharge arcanique";
    }
    else if (className.find("gardien") != std::string::npos || className.find("tank") != std::string::npos || className.find("colosse") != std::string::npos)
    {
        bonus -= 2;
        techniqueName += " / garde lourde";
        attacker.startDefensePosture(20, 5, "Technique de garde lourde");
    }
    else if (className.find("clerc") != std::string::npos || className.find("prêtre") != std::string::npos || className.find("pretre") != std::string::npos || className.find("paladin") != std::string::npos)
    {
        bonus += 2;
        techniqueName += " / frappe protectrice";
    }
    else if (className.find("alchim") != std::string::npos || className.find("artific") != std::string::npos)
    {
        bonus += 4;
        techniqueName += " / dosage instable";
    }
    else if (className.find("nécro") != std::string::npos || className.find("necro") != std::string::npos)
    {
        bonus += 3;
        techniqueName += " / dette des morts";
    }
    else if (className.find("invoc") != std::string::npos || className.find("dresseur") != std::string::npos)
    {
        bonus += 1;
        techniqueName += " / ordre de soutien";
        attacker.startDefensePosture(8, 3, "Ordre de soutien");
    }
    else if (className.find("duelliste") != std::string::npos || className.find("moine") != std::string::npos)
    {
        bonus += 4;
        techniqueName += " / lecture du rythme";
    }

    std::cout << attacker.getName() << " utilise : " << techniqueName << "." << std::endl;
    std::cout << "La technique dépend de l'arme équipée et de la classe : certaines formations imposent naturellement leur propre rythme." << std::endl;

    if (className.find("assassin") != std::string::npos || className.find("ombrelame") != std::string::npos)
    {
        ElementalAffinitySystem::applyBleeding(defender, 1, 2 + std::max(1, attacker.getMaxDamage() / 18));
        std::cout << "L'angle mort prépare un petit saignement si la cible survit." << std::endl;
    }
    else if (className.find("mage") != std::string::npos || className.find("sorcier") != std::string::npos || className.find("arcaniste") != std::string::npos)
    {
        int elementalRoll = random.between(1, 3);
        if (elementalRoll == 1)
        {
            ElementalAffinitySystem::applyBurning(defender, 1, 2 + std::max(1, attacker.getMaxDamage() / 20));
            std::cout << "La surcharge laisse une brûlure arcanique faible." << std::endl;
        }
        else if (elementalRoll == 2)
        {
            ElementalAffinitySystem::applyFrost(defender, 2);
            std::cout << "La surcharge refroidit la cible et prépare un ralentissement." << std::endl;
        }
        else
        {
            ElementalAffinitySystem::applyShock(defender, 2);
            std::cout << "La surcharge perturbe la cible avec une décharge instable." << std::endl;
        }
    }
    else if (className.find("clerc") != std::string::npos || className.find("prêtre") != std::string::npos || className.find("pretre") != std::string::npos || className.find("paladin") != std::string::npos)
    {
        int selfHeal = std::max(2, attacker.getMaxHp() / 25);
        attacker.heal(selfHeal);
        std::cout << "La frappe protectrice rend " << selfHeal << " PV au lanceur." << std::endl;
    }
    else if (className.find("alchim") != std::string::npos || className.find("artific") != std::string::npos)
    {
        int effectRoll = random.between(1, 3);
        if (effectRoll == 1)
        {
            ElementalAffinitySystem::applyPoison(defender, 2, 2 + std::max(1, attacker.getMaxDamage() / 18));
            std::cout << "Le dosage instable accroche un poison faible." << std::endl;
        }
        else if (effectRoll == 2)
        {
            ElementalAffinitySystem::applyShock(defender, 1);
            std::cout << "Le mécanisme bricolé produit une petite décharge." << std::endl;
        }
        else
        {
            attacker.startDefensePosture(10, 4, "Couverture d'artificier");
            std::cout << "Le bricolage crée une petite couverture défensive." << std::endl;
        }
    }
    else if (className.find("nécro") != std::string::npos || className.find("necro") != std::string::npos)
    {
        ElementalAffinitySystem::applyBleeding(defender, 2, 2 + std::max(1, attacker.getMaxDamage() / 20));
        std::cout << "La dette des morts laisse une trace sombre, proche d'un saignement." << std::endl;
    }
    else if (className.find("duelliste") != std::string::npos || className.find("moine") != std::string::npos)
    {
        attacker.startDefensePosture(8, 10, "Lecture du rythme");
        std::cout << "Le rythme lu prépare une petite fenêtre de contre." << std::endl;
    }
    else if (className.find("lancier") != std::string::npos)
    {
        attacker.startDefensePosture(12, 4, "Allonge de lancier");
        std::cout << "Le lancier garde l'ennemi à distance après sa technique." << std::endl;
    }

    std::cout << std::endl;

    executeBoostedAttack(attacker, defender, random, bonus);
}

void CombatActions::executeHeavyAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    int bonus = 10;

    if (className.find("berserker") != std::string::npos || className.find("barbare") != std::string::npos || className.find("briseur") != std::string::npos) bonus += 8;
    if (className.find("colosse") != std::string::npos || className.find("gardien") != std::string::npos) bonus += 4;
    if (className.find("assassin") != std::string::npos || className.find("archer") != std::string::npos) bonus -= 3;

    std::cout << attacker.getName() << " prépare une attaque lourde." << std::endl;
    std::cout << "Le coup demande de l'engagement : les profils lourds et berserkers y trouvent plus facilement leur force." << std::endl;
    std::cout << std::endl;

    executeBoostedAttack(attacker, defender, random, bonus);
}

void CombatActions::executeQuickAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    int bonus = -3;
    int chainChance = 55;

    if (className.find("assassin") != std::string::npos || className.find("duelliste") != std::string::npos || className.find("moine") != std::string::npos)
    {
        bonus += 3;
        chainChance += 20;
    }
    if (className.find("colosse") != std::string::npos || className.find("tank") != std::string::npos || className.find("briseur") != std::string::npos)
    {
        bonus -= 3;
        chainChance -= 18;
    }

    std::cout << attacker.getName() << " tente une attaque rapide." << std::endl;
    std::cout << "Les classes agiles enchaînent plus naturellement, les classes lourdes moins." << std::endl;
    std::cout << std::endl;

    executeBoostedAttack(attacker, defender, random, bonus);

    if (!defender.isDead() && random.between(1, 100) <= chainChance)
    {
        std::cout << "L'ouverture reste présente : second geste rapide !" << std::endl;
        std::cout << std::endl;
        executeBoostedAttack(attacker, defender, random, -5);
    }
}


bool CombatActions::executeClassSkill(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    Player* player = dynamic_cast<Player*>(&attacker);
    int level = player != nullptr ? player->getLevel() : 1;

    auto hasAny = [&](std::initializer_list<const char*> terms) {
        for (const char* term : terms)
        {
            if (className.find(term) != std::string::npos) return true;
        }
        return false;
    };

    auto hasRangedWeapon = [&]() {
        if (player == nullptr || !player->hasEquippedWeapon()) return false;
        std::string weaponName = normalizeCombatActionText(player->getEquippedWeapon().getName());
        return player->getEquippedWeapon().getType() == WeaponType::Bow
            || weaponName.find("arbal") != std::string::npos
            || weaponName.find("lancer") != std::string::npos
            || weaponName.find("bandouli") != std::string::npos;
    };

    auto startCooldown = [&](int turns) {
        attacker.startClassSkillCooldown(turns);
        std::cout << "Souffle de compétence : " << turns << " tour(s) de récupération." << std::endl;
        std::cout << std::endl;
    };

    if (!attacker.isClassSkillReady())
    {
        std::cout << "La compétence de classe n'est pas encore revenue." << std::endl;
        std::cout << "Récupération restante : " << attacker.getClassSkillCooldownTurns() << " tour(s)." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::cout << attacker.getName() << " cherche une compétence de classe utilisable maintenant." << std::endl;

    if (hasAny({"assassin", "ombrelame", "lanceur de dagues"}) && level >= 3)
    {
        std::cout << "========== COMPÉTENCE ASSASSIN ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Pas dans l'angle mort (niv. 3, recharge 3 tours)" << std::endl;
        if (level >= 8) std::cout << "2 : Disparition basse (niv. 8, recharge 4 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 8 ? 2 : 1, "Choix invalide.");
        if (choice == 0) return false;
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Disparition basse." << std::endl;
            std::cout << "Tu recules dans un angle mort, puis tu frappes en laissant une blessure plus longue." << std::endl;
            attacker.startDefensePosture(14, 18, "Disparition basse");
            ElementalAffinitySystem::applyBleeding(defender, 4, 3 + std::max(1, level / 14));
            executeBoostedAttack(attacker, defender, random, 4 + level / 18);
            startCooldown(4);
            return true;
        }

        std::cout << "Compétence de classe : Pas dans l'angle mort." << std::endl;
        std::cout << "Tu sacrifies la puissance brute pour préparer une blessure qui continue de travailler." << std::endl;
        ElementalAffinitySystem::applyBleeding(defender, 3, 2 + std::max(1, level / 18));
        executeBoostedAttack(attacker, defender, random, 2 + level / 20);
        startCooldown(3);
        return true;
    }

    if (hasAny({"gardien", "tank", "colosse"}) && level >= 3)
    {
        std::cout << "========== COMPÉTENCE DÉFENSIVE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Rempart provoquant (niv. 3, recharge 3 tours)" << std::endl;
        if (level >= 8) std::cout << "2 : Ancrage total (niv. 8, recharge 4 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 8 ? 2 : 1, "Choix invalide.");
        if (choice == 0) return false;
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Ancrage total." << std::endl;
            std::cout << "Tu abandonnes presque l'offense pour forcer l'ennemi à te gérer." << std::endl;
            attacker.startDefensePosture(35, 12, "Ancrage total");
            executeBoostedAttack(attacker, defender, random, -5);
            startCooldown(4);
            return true;
        }

        std::cout << "Compétence de classe : Rempart provoquant." << std::endl;
        std::cout << "Tu prends de la place, tu frappes moins fort, mais tu rends ta présence impossible à ignorer." << std::endl;
        attacker.startDefensePosture(22, 7, "Rempart provoquant");
        executeBoostedAttack(attacker, defender, random, -2);
        startCooldown(3);
        return true;
    }

    if (hasAny({"clerc", "prêtre", "pretre", "paladin"}) && level >= 3)
    {
        std::cout << "========== COMPÉTENCE SACRÉE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Serment de survie (niv. 3, recharge 3 tours)" << std::endl;
        if (level >= 7) std::cout << "2 : Prière purificatrice (niv. 7, recharge 5 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 7 ? 2 : 1, "Choix invalide.");
        if (choice == 0) return false;
        int heal = std::max(5, attacker.getMaxHp() / 12 + level / 5);
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Prière purificatrice." << std::endl;
            int cured = 0;
            if (attacker.cureBurning()) cured++;
            if (attacker.curePoison()) cured++;
            if (attacker.cureFrost()) cured++;
            if (attacker.cureShock()) cured++;
            if (attacker.cureBleeding()) cured++;
            if (attacker.cureWeakening()) cured++;
            if (attacker.cureVulnerability()) cured++;
            attacker.heal(heal + level / 3);
            attacker.startDefensePosture(10, 3, "Prière purificatrice");
            std::cout << attacker.getName() << " récupère " << (heal + level / 3) << " PV et purifie " << cured << " effet(s)." << std::endl;
            startCooldown(5);
            return true;
        }

        std::cout << "Compétence de classe : Serment de survie." << std::endl;
        std::cout << "Ce tour sert surtout à tenir : soin personnel et garde légère." << std::endl;
        attacker.heal(heal);
        attacker.startDefensePosture(12, 4, "Serment de survie");
        std::cout << attacker.getName() << " récupère " << heal << " PV." << std::endl;
        startCooldown(3);
        return true;
    }

    if (hasAny({"mage", "magicien", "ensorceleur", "sorcier", "arcaniste", "pyromancien", "cryomancien", "occultiste", "invocateur", "nécromancien", "necromancien", "pactisant", "clerc", "prêtre", "pretre", "paladin", "druide", "shaman", "templier", "mage-lame", "chevalier runique"})
        && (level >= 4
            || (player != nullptr && (player->hasActiveSkill("learned_arcane_mark")
                || player->hasActiveSkill("learned_arcane_binding")
                || player->hasActiveSkill("learned_elemental_ward")
                || player->hasActiveSkill("learned_resistance_rift")
                || player->hasActiveSkill("learned_frost_needle")
                || player->hasActiveSkill("learned_mana_suture")
                || player->hasActiveSkill("learned_occult_bramble")))))
    {
        bool canArcaneMark = level >= 4 || (player != nullptr && player->hasActiveSkill("learned_arcane_mark"));
        bool canArcaneBinding = level >= 6 || (player != nullptr && player->hasActiveSkill("learned_arcane_binding"));
        bool canElementalWard = level >= 8 || (player != nullptr && player->hasActiveSkill("learned_elemental_ward"));
        bool canControlledOverload = level >= 9;
        bool canResistanceRift = level >= 11 || (player != nullptr && player->hasActiveSkill("learned_resistance_rift"));
        bool canFrostNeedle = player != nullptr && player->hasActiveSkill("learned_frost_needle");
        bool canManaSuture = player != nullptr && player->hasActiveSkill("learned_mana_suture");
        bool canOccultBramble = player != nullptr && player->hasActiveSkill("learned_occult_bramble");

        std::cout << "========== COMPÉTENCE ARCANIQUE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        if (canArcaneMark) std::cout << "1 : Marque élémentaire (recharge 3 tours)" << std::endl;
        if (canArcaneBinding) std::cout << "2 : Entrave arcanique (recharge 4 tours)" << std::endl;
        if (canElementalWard) std::cout << "3 : Voile élémentaire (recharge 4 tours)" << std::endl;
        if (canControlledOverload) std::cout << "4 : Surcharge contrôlée (niv. 9, recharge 5 tours)" << std::endl;
        if (canResistanceRift) std::cout << "5 : Faille de résistance (recharge 5 tours)" << std::endl;
        if (canFrostNeedle) std::cout << "6 : Aiguille de givre étudiée (grimoire, recharge 4 tours)" << std::endl;
        if (canManaSuture) std::cout << "7 : Suture de mana étudiée (grimoire, recharge 5 tours)" << std::endl;
        if (canOccultBramble) std::cout << "8 : Ronces occultes étudiées (grimoire, recharge 4 tours)" << std::endl;
        std::cout << "> ";

        int skillChoice = 0;
        while (true)
        {
            skillChoice = Console::askNumberBetween(0, 8, "Choix invalide.");
            if (skillChoice == 0) return false;
            if (skillChoice == 1 && canArcaneMark) break;
            if (skillChoice == 2 && canArcaneBinding) break;
            if (skillChoice == 3 && canElementalWard) break;
            if (skillChoice == 4 && canControlledOverload) break;
            if (skillChoice == 5 && canResistanceRift) break;
            if (skillChoice == 6 && canFrostNeedle) break;
            if (skillChoice == 7 && canManaSuture) break;
            if (skillChoice == 8 && canOccultBramble) break;
            std::cout << "Ce sort n'est pas encore assez compris par ton personnage." << std::endl;
            std::cout << "> ";
        }

        if (skillChoice == 8)
        {
            std::cout << "Compétence arcanique : Ronces occultes." << std::endl;
            std::cout << "Ce grimoire ne lance pas une boule d'énergie : il force des liens sombres à sortir du sol autour de la cible." << std::endl;
            if (!resolveMagicCatalystAttempt(player, "Ronces occultes", random))
            {
                startCooldown(2);
                return true;
            }
            defender.applyWeakening(3, 12 + level / 10);
            ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 20);
            if (random.between(1, 100) <= 35)
            {
                ElementalAffinitySystem::applyFrost(defender, 1);
                std::cout << "Les ronces freinent aussi l'élan de la cible." << std::endl;
            }
            executeBoostedAttack(attacker, defender, random, 1 + level / 28);
            startCooldown(4);
            return true;
        }

        if (skillChoice == 7)
        {
            std::cout << "Compétence arcanique : Suture de mana." << std::endl;
            std::cout << "Ce sort vient d'un grimoire : il ne soigne pas d'un coup, il referme lentement les blessures." << std::endl;
            if (!resolveMagicCatalystAttempt(player, "Suture de mana", random))
            {
                startCooldown(2);
                return true;
            }
            int healPerTurn = std::max(4, attacker.getMaxHp() / 18 + level / 8);
            attacker.applyRegeneration(3, healPerTurn);
            attacker.startDefensePosture(6, 2, "Suture de mana");
            std::cout << "La suture commence à se refermer autour des plaies." << std::endl;
            startCooldown(5);
            return true;
        }

        if (skillChoice == 6)
        {
            std::cout << "Compétence arcanique : Aiguille de givre." << std::endl;
            std::cout << "Ce sort vient d'un grimoire précis : il n'a pas encore de parchemin courant fiable." << std::endl;
            if (!resolveMagicCatalystAttempt(player, "Aiguille de givre", random))
            {
                startCooldown(2);
                return true;
            }
            ElementalAffinitySystem::applyFrost(defender, 3);
            if (random.between(1, 100) <= 35)
            {
                defender.applyWeakening(2, 8 + level / 12);
                std::cout << "Le froid mord assez juste pour raidir le prochain geste." << std::endl;
            }
            executeBoostedAttack(attacker, defender, random, 2 + level / 26);
            startCooldown(4);
            return true;
        }

        if (skillChoice == 5)
        {
            std::cout << "Compétence arcanique : Faille de résistance." << std::endl;
            std::cout << "Tu ne cherches pas seulement à blesser : tu ouvres une brèche que les prochains impacts pourront exploiter." << std::endl;
            std::cout << "1 : Faille ardente" << std::endl;
            std::cout << "2 : Faille de givre" << std::endl;
            std::cout << "3 : Faille conductrice" << std::endl;
            std::cout << "> ";
            int choice = Console::askNumberBetween(1, 3, "Choix invalide.");
            if (!resolveMagicCatalystAttempt(player, "Faille de résistance", random))
            {
                startCooldown(2);
                return true;
            }
            defender.applyVulnerability(3, 16 + level / 10);
            if (choice == 1) ElementalAffinitySystem::applyBurning(defender, 3, 3 + level / 18);
            else if (choice == 2) ElementalAffinitySystem::applyFrost(defender, 3);
            else ElementalAffinitySystem::applyShock(defender, 2);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            startCooldown(5);
            return true;
        }

        if (skillChoice == 4)
        {
            std::cout << "Compétence arcanique : Surcharge contrôlée." << std::endl;
            std::cout << "1 : Feu concentré" << std::endl;
            std::cout << "2 : Givre concentré" << std::endl;
            std::cout << "3 : Choc concentré" << std::endl;
            std::cout << "4 : Poison concentré" << std::endl;
            std::cout << "> ";
            int choice = Console::askNumberBetween(1, 4, "Choix invalide.");
            if (!resolveMagicCatalystAttempt(player, "Surcharge contrôlée", random))
            {
                startCooldown(2);
                return true;
            }
            if (choice == 1) ElementalAffinitySystem::applyBurning(defender, 4, 3 + level / 16);
            else if (choice == 2) ElementalAffinitySystem::applyFrost(defender, 4);
            else if (choice == 3) ElementalAffinitySystem::applyShock(defender, 3);
            else ElementalAffinitySystem::applyPoison(defender, 4, 3 + level / 18);
            executeBoostedAttack(attacker, defender, random, 4 + level / 22);
            startCooldown(5);
            return true;
        }

        if (skillChoice == 3)
        {
            std::cout << "Compétence arcanique : Voile élémentaire." << std::endl;
            std::cout << "Tu replies la magie contre toi plutôt que de la projeter : les altérations élémentaires auront plus de mal à s'accrocher." << std::endl;
            if (!resolveMagicCatalystAttempt(player, "Voile élémentaire", random))
            {
                startCooldown(2);
                return true;
            }
            attacker.applyElementalWard(4, 18 + level / 8);
            attacker.startDefensePosture(8, 3, "Voile élémentaire");
            startCooldown(4);
            return true;
        }

        if (skillChoice == 2)
        {
            std::cout << "Compétence arcanique : Entrave arcanique." << std::endl;
            std::cout << "La magie ne cherche pas seulement à brûler ou choquer : elle tire directement sur le souffle de la cible." << std::endl;
            if (!resolveMagicCatalystAttempt(player, "Entrave arcanique", random))
            {
                startCooldown(2);
                return true;
            }
            defender.applyWeakening(3, 14 + level / 8);
            if (random.between(1, 100) <= 45)
            {
                ElementalAffinitySystem::applyShock(defender, 1);
                std::cout << "L'entrave grésille assez pour troubler le prochain geste." << std::endl;
            }
            else
            {
                ElementalAffinitySystem::applyFrost(defender, 1);
                std::cout << "L'entrave refroidit le rythme plutôt que le corps." << std::endl;
            }
            std::cout << std::endl;
            executeBoostedAttack(attacker, defender, random, 2 + level / 28);
            startCooldown(4);
            return true;
        }

        std::cout << "Compétence arcanique : Marque élémentaire." << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Braise faible" << std::endl;
        std::cout << "2 : Givre bref" << std::endl;
        std::cout << "3 : Décharge instable" << std::endl;
        std::cout << "4 : Venin arcanique" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, 4, "Choix invalide.");
        if (choice == 0) return false;
        if (!resolveMagicCatalystAttempt(player, "Marque élémentaire", random))
        {
            startCooldown(2);
            return true;
        }
        if (choice == 1) { ElementalAffinitySystem::applyBurning(defender, 3, 2 + level / 20); std::cout << "Une braise faible s'accroche à la cible." << std::endl; }
        else if (choice == 2) { ElementalAffinitySystem::applyFrost(defender, 3); std::cout << "Un froid bref ralentit la cible." << std::endl; }
        else if (choice == 3) { ElementalAffinitySystem::applyShock(defender, 2); std::cout << "Une décharge instable trouble le prochain geste adverse." << std::endl; }
        else { ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 22); std::cout << "Un venin arcanique faible entre dans la blessure." << std::endl; }
        std::cout << std::endl;
        executeBoostedAttack(attacker, defender, random, 1 + level / 30);
        startCooldown(3);
        return true;
    }

    if (hasAny({"archer", "rôdeur", "rodeur", "tireur"}) && level >= 4)
    {
        if (!hasRangedWeapon())
        {
            std::cout << "Cette compétence demande une vraie arme à distance équipée." << std::endl << std::endl;
            return false;
        }

        std::cout << "========== COMPÉTENCE DISTANCE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Tir de couverture (niv. 4, recharge 3 tours)" << std::endl;
        if (level >= 8) std::cout << "2 : Tir d'arrêt (niv. 8, recharge 4 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 8 ? 2 : 1, "Choix invalide.");
        if (choice == 0) return false;
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Tir d'arrêt." << std::endl;
            std::cout << "Le tir cherche à casser l'élan, pas seulement à blesser." << std::endl;
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 18);
            ElementalAffinitySystem::applyFrost(defender, 2);
            executeBoostedAttack(attacker, defender, random, 6 + level / 16);
            startCooldown(4);
            return true;
        }

        std::cout << "Compétence de classe : Tir de couverture." << std::endl;
        std::cout << "Le tir vise moins la létalité immédiate que le contrôle de la cible." << std::endl;
        ElementalAffinitySystem::applyFrost(defender, 1);
        executeBoostedAttack(attacker, defender, random, 4 + level / 18);
        startCooldown(3);
        return true;
    }

    if (hasAny({"berserker", "barbare", "briseur"}) && level >= 4)
    {
        std::cout << "========== COMPÉTENCE BRUTALE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Rage assumée (niv. 4, recharge 3 tours)" << std::endl;
        if (level >= 8) std::cout << "2 : Coup de rupture (niv. 8, recharge 4 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 8 ? 2 : 1, "Choix invalide.");
        if (choice == 0) return false;
        int bonus = 6 + level / 12;
        int recoil = std::max(1, attacker.getMaxHp() / 40);
        if (attacker.getHp() * 100 <= attacker.getMaxHp() * 50)
        {
            bonus += 7;
            std::cout << "La blessure nourrit la rage : bonus renforcé." << std::endl;
        }
        int cooldownTurns = 3;
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Coup de rupture." << std::endl;
            bonus += 8;
            recoil = std::max(2, attacker.getMaxHp() / 25);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 16);
            cooldownTurns = 4;
        }
        else
        {
            std::cout << "Compétence de classe : Rage assumée." << std::endl;
        }
        attacker.takeDamage(recoil);
        std::cout << "Le corps encaisse " << recoil << " PV de contrecoup avant de rendre la violence." << std::endl;
        executeBoostedAttack(attacker, defender, random, bonus);
        startCooldown(cooldownTurns);
        return true;
    }

    if (hasAny({"alchim", "artific", "bricoleur"}) && level >= 4)
    {
        std::cout << "========== COMPÉTENCE EXPÉRIMENTALE ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Mélange improvisé (niv. 4, recharge 3 tours)" << std::endl;
        if (level >= 7) std::cout << "2 : Fiole tactique (niv. 7, recharge 4 tours)" << std::endl;
        if (level >= 9) std::cout << "3 : Poudre de fragilisation (niv. 9, recharge 5 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 9 ? 3 : (level >= 7 ? 2 : 1), "Choix invalide.");
        if (choice == 0) return false;
        if (choice == 3)
        {
            std::cout << "Compétence de classe : Poudre de fragilisation." << std::endl;
            std::cout << "La poudre n'explose presque pas : elle s'accroche aux points faibles et rend le prochain échange dangereux." << std::endl;
            defender.applyVulnerability(3, 14 + level / 12);
            if (random.between(1, 100) <= 55) ElementalAffinitySystem::applyPoison(defender, 2, 2 + level / 24);
            else ElementalAffinitySystem::applyShock(defender, 1);
            executeBoostedAttack(attacker, defender, random, 2 + level / 26);
            startCooldown(5);
            return true;
        }
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Fiole tactique." << std::endl;
            std::cout << "1 : Acide/poison" << std::endl;
            std::cout << "2 : Fumée défensive" << std::endl;
            std::cout << "3 : Étincelle instable" << std::endl;
            std::cout << "> ";
            int vial = Console::askNumberBetween(1, 3, "Choix invalide.");
            if (vial == 1) ElementalAffinitySystem::applyPoison(defender, 4, 3 + level / 18);
            else if (vial == 2) attacker.startDefensePosture(22, 8, "Fumée tactique");
            else ElementalAffinitySystem::applyShock(defender, 3);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            startCooldown(4);
            return true;
        }

        std::cout << "Compétence de classe : Mélange improvisé." << std::endl;
        int roll = random.between(1, 4);
        if (roll == 1) { ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 20); std::cout << "Le mélange pique et empoisonne la cible." << std::endl; }
        else if (roll == 2) { ElementalAffinitySystem::applyBurning(defender, 2, 2 + level / 22); std::cout << "Le mélange chauffe trop vite et brûle la cible." << std::endl; }
        else if (roll == 3) { ElementalAffinitySystem::applyShock(defender, 2); std::cout << "Un petit mécanisme produit une décharge." << std::endl; }
        else { attacker.startDefensePosture(14, 6, "Écran expérimental"); std::cout << "Le mélange produit surtout un écran défensif." << std::endl; }
        std::cout << std::endl;
        executeBoostedAttack(attacker, defender, random, 2 + level / 25);
        startCooldown(3);
        return true;
    }

    if (hasAny({"invoc", "dresseur", "nécro", "necro"}) && level >= 5)
    {
        std::cout << "========== COMPÉTENCE D'APPEL ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Ordre spectral (niv. 5, recharge 3 tours)" << std::endl;
        if (level >= 9) std::cout << "2 : Lien de meute / ombre (niv. 9, recharge 4 tours)" << std::endl;
        std::cout << "> ";
        int choice = Console::askNumberBetween(0, level >= 9 ? 2 : 1, "Choix invalide.");
        if (choice == 0) return false;
        if (choice == 2)
        {
            std::cout << "Compétence de classe : Lien de meute / ombre." << std::endl;
            std::cout << "Ta présence impose une pression de groupe, même sans invocation déjà visible." << std::endl;
            ElementalAffinitySystem::applyFrost(defender, 3);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 20);
            attacker.startDefensePosture(12, 5, "Lien de meute / ombre");
            executeBoostedAttack(attacker, defender, random, 5 + level / 20);
            startCooldown(4);
            return true;
        }

        std::cout << "Compétence de classe : Ordre spectral." << std::endl;
        std::cout << "Même sans invocation active, ta présence force un appui invisible et froid." << std::endl;
        ElementalAffinitySystem::applyFrost(defender, 2);
        attacker.startDefensePosture(8, 3, "Ordre spectral");
        executeBoostedAttack(attacker, defender, random, 3 + level / 24);
        startCooldown(3);
        return true;
    }

    std::cout << "Aucune technique de classe stable ne répond pour ce profil." << std::endl;
    std::cout << "Les maîtres de guilde disent que les premières techniques fiables apparaissent souvent entre les niveaux 3 et 5." << std::endl;
    std::cout << "Tu improvises donc une attaque simple." << std::endl << std::endl;
    executeAttack(attacker, defender, random);
    return true;
}

bool CombatActions::executeAIClassSkill(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    if (!attacker.isClassSkillReady())
    {
        return false;
    }

    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    Player* player = dynamic_cast<Player*>(&attacker);
    Monster* monster = dynamic_cast<Monster*>(&attacker);
    Boss* boss = dynamic_cast<Boss*>(&attacker);

    int level = 1;
    if (player != nullptr)
    {
        level = player->getLevel();
    }
    else if (monster != nullptr)
    {
        level = monster->getLevel();
    }
    else if (boss != nullptr)
    {
        level = 12;
    }
    else
    {
        level = std::max(1, (attacker.getMaxDamage() + attacker.getCriticalDamage()) / 12);
    }

    std::string profile = normalizeCombatActionText(attacker.getName() + " " + attacker.getType());

    auto hasAny = [&](std::initializer_list<const char*> terms) {
        for (const char* term : terms)
        {
            if (className.find(term) != std::string::npos) return true;
        }
        return false;
    };

    auto finishAIClassSkill = [&](int cooldown) {
        attacker.startClassSkillCooldown(cooldown);
        std::cout << attacker.getName() << " reprend son souffle après cette technique." << std::endl;
        std::cout << std::endl;
    };

    auto advancedProfile = [&]() {
        return profile.find("elite") != std::string::npos
            || profile.find("élite") != std::string::npos
            || profile.find("veteran") != std::string::npos
            || profile.find("vétéran") != std::string::npos
            || profile.find("chef") != std::string::npos
            || profile.find("champion") != std::string::npos
            || profile.find("oracle") != std::string::npos
            || profile.find("ancien") != std::string::npos
            || profile.find("hazak") != std::string::npos
            || profile.find("skuro") != std::string::npos
            || profile.find("kanade") != std::string::npos
            || profile.find("aoi") != std::string::npos
            || profile.find("sanctus") != std::string::npos
            || profile.find("fail") != std::string::npos
            || profile.find("hestia") != std::string::npos
            || profile.find("fire flight") != std::string::npos
            || profile.find("louis") != std::string::npos
            || profile.find("mattzelda") != std::string::npos
            || profile.find("trexof") != std::string::npos
            || profile.find("henrique") != std::string::npos;
    };

    bool knowsAdvancedMove = advancedProfile();

    if (hasAny({"assassin", "ombrelame", "lanceur de dagues"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 55)
        {
            std::cout << attacker.getName() << " s'abaisse dans un angle mort et laisse une blessure plus longue." << std::endl;
            attacker.startDefensePosture(12, 14, "Disparition basse");
            ElementalAffinitySystem::applyBleeding(defender, 4, 3 + std::max(1, level / 16));
            executeBoostedAttack(attacker, defender, random, 4 + level / 20);
            finishAIClassSkill(4);
        }
        else
        {
            std::cout << attacker.getName() << " disparaît brièvement de l'angle de vision." << std::endl;
            ElementalAffinitySystem::applyBleeding(defender, 3, 2 + std::max(1, level / 18));
            executeBoostedAttack(attacker, defender, random, 2 + level / 22);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"gardien", "tank", "colosse"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 50)
        {
            std::cout << attacker.getName() << " s'ancre au sol et transforme sa défense en mur vivant." << std::endl;
            attacker.startDefensePosture(32, 10, "Ancrage total");
            executeBoostedAttack(attacker, defender, random, -5);
            finishAIClassSkill(4);
        }
        else
        {
            std::cout << attacker.getName() << " impose sa présence et verrouille sa posture." << std::endl;
            attacker.startDefensePosture(20, 6, "Rempart instinctif");
            executeBoostedAttack(attacker, defender, random, -2);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"clerc", "prêtre", "pretre", "paladin"}))
    {
        int heal = std::max(4, attacker.getMaxHp() / 14 + level / 6);
        if (level >= 7 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            int cured = 0;
            if (attacker.cureBurning()) cured++;
            if (attacker.curePoison()) cured++;
            if (attacker.cureFrost()) cured++;
            if (attacker.cureShock()) cured++;
            if (attacker.cureBleeding()) cured++;
            if (attacker.cureWeakening()) cured++;
            if (attacker.cureVulnerability()) cured++;
            attacker.heal(heal + level / 4);
            attacker.startDefensePosture(12, 4, "Prière purificatrice");
            std::cout << attacker.getName() << " purifie " << cured << " effet(s) et récupère " << (heal + level / 4) << " PV." << std::endl;
            finishAIClassSkill(5);
        }
        else
        {
            std::cout << attacker.getName() << " murmure un serment et tient la ligne." << std::endl;
            attacker.heal(heal);
            attacker.startDefensePosture(10, 3, "Serment discret");
            std::cout << attacker.getName() << " récupère " << heal << " PV." << std::endl;
            finishAIClassSkill(4);
        }
        return true;
    }

    if (hasAny({"mage", "magicien", "ensorceleur", "sorcier", "arcaniste", "pyromancien", "cryomancien", "occultiste", "invocateur", "nécromancien", "necromancien", "pactisant", "clerc", "prêtre", "pretre", "paladin", "druide", "shaman", "templier", "mage-lame", "chevalier runique"}))
    {
        int hpPercent = attacker.getMaxHp() > 0 ? attacker.getHp() * 100 / attacker.getMaxHp() : 100;
        if (level >= 9 && hpPercent <= 45 && knowsAdvancedMove && random.between(1, 100) <= 30)
        {
            std::cout << attacker.getName() << " recoud une blessure avec un fil de mana tremblant." << std::endl;
            attacker.applyRegeneration(3, std::max(4, attacker.getMaxHp() / 22 + level / 10));
            attacker.startDefensePosture(6, 2, "Suture de mana");
            finishAIClassSkill(5);
            return true;
        }

        if (level >= 8 && hpPercent <= 55 && random.between(1, 100) <= (knowsAdvancedMove ? 48 : 28))
        {
            std::cout << attacker.getName() << " referme un voile élémentaire autour de son corps." << std::endl;
            attacker.applyElementalWard(3, 16 + level / 10);
            attacker.startDefensePosture(8, 3, "Voile élémentaire");
            finishAIClassSkill(4);
            return true;
        }

        if (level >= 11 && knowsAdvancedMove && random.between(1, 100) <= 34)
        {
            std::cout << attacker.getName() << " ouvre une faille magique au lieu de chercher le coup direct." << std::endl;
            defender.applyVulnerability(3, 15 + level / 12);
            int element = random.between(1, 3);
            if (element == 1) ElementalAffinitySystem::applyBurning(defender, 3, 3 + level / 20);
            else if (element == 2) ElementalAffinitySystem::applyFrost(defender, 3);
            else ElementalAffinitySystem::applyShock(defender, 2);
            executeBoostedAttack(attacker, defender, random, 3 + level / 26);
            finishAIClassSkill(5);
            return true;
        }

        if (level >= 6 && random.between(1, 100) <= (knowsAdvancedMove ? 38 : 24))
        {
            std::cout << attacker.getName() << " serre la trame magique autour du souffle adverse." << std::endl;
            defender.applyWeakening(3, 12 + level / 10);
            if (random.between(1, 100) <= 50) ElementalAffinitySystem::applyShock(defender, 1);
            else ElementalAffinitySystem::applyFrost(defender, 1);
            executeBoostedAttack(attacker, defender, random, 2 + level / 30);
            finishAIClassSkill(4);
            return true;
        }

        std::cout << attacker.getName() << " condense une marque élémentaire." << std::endl;
        int element = random.between(1, 4);
        if (element == 1) ElementalAffinitySystem::applyBurning(defender, 3, 2 + level / 22);
        else if (element == 2) ElementalAffinitySystem::applyFrost(defender, 3);
        else if (element == 3) ElementalAffinitySystem::applyShock(defender, 2);
        else ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 24);
        if (level >= 9 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            int secondElement = random.between(1, 3);
            if (secondElement == 1) ElementalAffinitySystem::applyBurning(defender, 2, 2 + level / 24);
            else if (secondElement == 2) ElementalAffinitySystem::applyShock(defender, 1);
            else ElementalAffinitySystem::applyFrost(defender, 2);
            executeBoostedAttack(attacker, defender, random, 4 + level / 24);
            finishAIClassSkill(5);
        }
        else
        {
            executeBoostedAttack(attacker, defender, random, 1 + level / 32);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"archer", "rôdeur", "rodeur", "tireur"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            std::cout << attacker.getName() << " vise l'élan adverse et cherche à clouer le rythme." << std::endl;
            ElementalAffinitySystem::applyFrost(defender, 2);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 20);
            executeBoostedAttack(attacker, defender, random, 5 + level / 18);
            finishAIClassSkill(4);
        }
        else
        {
            std::cout << attacker.getName() << " choisit un tir de contrôle plutôt qu'un tir gratuit." << std::endl;
            ElementalAffinitySystem::applyFrost(defender, 1);
            executeBoostedAttack(attacker, defender, random, 3 + level / 20);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"berserker", "barbare", "briseur"}))
    {
        int recoil = std::max(1, attacker.getMaxHp() / 45);
        std::cout << attacker.getName() << " laisse la rage guider le prochain impact." << std::endl;
        attacker.takeDamage(recoil);
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 50)
        {
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 18);
            executeBoostedAttack(attacker, defender, random, 8 + level / 12);
            finishAIClassSkill(4);
        }
        else
        {
            executeBoostedAttack(attacker, defender, random, 5 + level / 14);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"alchim", "artific", "bricoleur"}))
    {
        std::cout << attacker.getName() << " libère un mélange préparé à la va-vite." << std::endl;
        int roll = random.between(1, 4);
        if (roll == 1) ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 22);
        else if (roll == 2) ElementalAffinitySystem::applyBurning(defender, 2, 2 + level / 24);
        else if (roll == 3) ElementalAffinitySystem::applyShock(defender, 2);
        else attacker.startDefensePosture(12, 5, "Écran expérimental");
        if (level >= 9 && knowsAdvancedMove && random.between(1, 100) <= 35)
        {
            std::cout << "Une poudre fine trouve les fissures de la défense adverse." << std::endl;
            defender.applyVulnerability(3, 13 + level / 14);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            finishAIClassSkill(5);
        }
        else if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            attacker.startDefensePosture(10, 4, "Fiole tactique");
            executeBoostedAttack(attacker, defender, random, 4 + level / 24);
            finishAIClassSkill(4);
        }
        else
        {
            executeBoostedAttack(attacker, defender, random, 2 + level / 28);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"invoc", "dresseur", "nécro", "necro"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            std::cout << attacker.getName() << " renforce un lien invisible avant de donner l'ordre." << std::endl;
            ElementalAffinitySystem::applyFrost(defender, 2);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 22);
            attacker.startDefensePosture(12, 5, "Lien de meute");
            executeBoostedAttack(attacker, defender, random, 4 + level / 22);
            finishAIClassSkill(4);
        }
        else
        {
            std::cout << attacker.getName() << " donne un ordre à quelque chose que l'on distingue à peine." << std::endl;
            ElementalAffinitySystem::applyFrost(defender, 2);
            attacker.startDefensePosture(8, 3, "Ordre spectral");
            executeBoostedAttack(attacker, defender, random, 2 + level / 24);
            finishAIClassSkill(3);
        }
        return true;
    }

    return false;
}

bool CombatActions::executeHealingPotion(
    Entity& entity,
    int potionHealAmount
)
{
    return CombatPotion::executeHealingPotion(
        entity,
        potionHealAmount
    );
}

bool CombatActions::executeDamagePotion(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionDamageBonus
)
{
    return CombatPotion::executeDamagePotion(
        attacker,
        defender,
        random,
        potionDamageBonus
    );
}
