// EN: CombatActions.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatActions.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/CombatActions.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/action/CombatPotion.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "core/Console.hpp"

#include "entity/Player.hpp"
#include "item/weapon/WeaponType.hpp"

#include <iostream>
#include <vector>
#include <cctype>


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

        if (weaponName.find("arbal") != std::string::npos || weaponName.find("carreau") != std::string::npos)
        {
            trainingId = "training_bolts";
            trainingLabel = "Carreaux d'entraînement";
            specialId = "piercing_bolts";
            specialLabel = "Carreaux perforants";
            elementalId = "frozen_bolts";
            elementalLabel = "Carreaux givrés";
        }
        else if (weaponName.find("lancer") != std::string::npos || weaponName.find("couteau") != std::string::npos || weaponName.find("bandouli") != std::string::npos)
        {
            trainingId = "training_throwing_knives";
            trainingLabel = "Couteaux de lancer émoussés";
            specialId = "balanced_throwing_knives";
            specialLabel = "Couteaux de lancer équilibrés";
            elementalId = "conductive_knives";
            elementalLabel = "Couteaux conducteurs";
        }

        std::vector<AmmunitionChoice> choices;
        choices.push_back({trainingId, trainingLabel, player.getInventory().countMaterialById(trainingId), false});
        choices.push_back({specialId, specialLabel, player.getInventory().countMaterialById(specialId), true});
        choices.push_back({elementalId, elementalLabel, player.getInventory().countMaterialById(elementalId), true});
        return choices;
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
    else if (className.find("clerc") != std::string::npos || className.find("prêtre") != std::string::npos || className.find("pretre") != std::string::npos)
    {
        bonus += 2;
        techniqueName += " / frappe protectrice";
    }

    std::cout << attacker.getName() << " utilise : " << techniqueName << "." << std::endl;
    std::cout << "La technique dépend de l'arme équipée et donne un premier vrai feeling de gameplay." << std::endl;
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
    std::cout << "Le coup dépend maintenant davantage de la classe : lourd et berserker le sentent plus que les classes fines." << std::endl;
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
