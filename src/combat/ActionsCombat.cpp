#include "combat/ActionsCombat.hpp"

#include "combat/SystemeDegats.hpp"

#include "core/Console.hpp"

#include "entite/Joueur.hpp"
#include "entite/Boss.hpp"

#include "objet/consommable/Consommable.hpp"
#include "objet/consommable/TypeConsommable.hpp"

#include <iostream>

void ActionsCombat::executerAttaque(
    Entite& attaquant,
    Entite& defenseur,
    Random& random
)
{
    bool esquive = false;
    bool critique = false;

    int degatsBruts = attaquant.attaquer(random, esquive, critique);

    if (esquive)
    {
        std::cout << attaquant.getNom() << " attaque, mais " << defenseur.getNom()
                  << " esquive au dernier moment." << std::endl;
        std::cout << std::endl;
        return;
    }

    if (atlasBloqueAttaque(attaquant, defenseur, degatsBruts))
    {
        return;
    }

    if (critique)
    {
        std::cout << attaquant.getNom()
                  << " frappe avec une violence monstrueuse et inflige "
                  << degatsBruts
                  << " dégâts bruts critiques."
                  << std::endl;
    }
    else
    {
        std::cout << attaquant.getNom()
                  << " attaque et inflige "
                  << degatsBruts
                  << " dégâts bruts."
                  << std::endl;
    }

    int degatsRecus = SystemeDegats::appliquerProtectionArmure(defenseur, degatsBruts);

    defenseur.recevoirDegats(degatsRecus);
    appliquerVolDeVieDemonSiBesoin(attaquant, degatsRecus);

    std::cout << defenseur.getNom() << " reçoit " << degatsRecus << " dégâts." << std::endl;
    std::cout << defenseur.getNom() << " possède maintenant "
              << defenseur.getPv() << "/" << defenseur.getPvMax() << " PV."
              << std::endl;
    std::cout << std::endl;
}

bool ActionsCombat::executerPotionSoin(
    Entite& entite,
    int soinPotion
)
{
    Joueur* joueur = dynamic_cast<Joueur*>(&entite);

    if (joueur != nullptr)
    {
        Consommable potion;

        if (!joueur->getInventaire().utiliserPremierConsommable(TypeConsommable::Soin, potion))
        {
            std::cout << joueur->getNom() << " fouille son inventaire..." << std::endl;
            std::cout << "Mais aucune potion de soin n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }

        joueur->soigner(potion.getPuissance());

        std::cout << joueur->getNom() << " utilise : " << potion.getNom() << "." << std::endl;
        std::cout << "Ses blessures se referment, et il récupère "
                  << potion.getPuissance() << " PV." << std::endl;
        std::cout << joueur->getNom() << " possède maintenant "
                  << joueur->getPv() << "/" << joueur->getPvMax() << " PV." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (entite.utiliserPotionSoin(soinPotion))
    {
        std::cout << entite.getNom() << " utilise une potion de soin." << std::endl;
        std::cout << "Sa vitalité revient lentement." << std::endl;
        std::cout << std::endl;

        return true;
    }

    std::cout << entite.getNom() << " n'a plus aucune potion de soin." << std::endl;
    std::cout << std::endl;

    return false;
}

bool ActionsCombat::executerPotionDegats(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusPotionDegats
)
{
    int bonusUtilise = bonusPotionDegats;

    Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

    if (joueur != nullptr)
    {
        Consommable potion;

        if (!joueur->getInventaire().utiliserPremierConsommable(TypeConsommable::Degats, potion))
        {
            std::cout << joueur->getNom() << " cherche une potion de rage dans son inventaire..." << std::endl;
            std::cout << "Mais aucune potion offensive n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }

        bonusUtilise = potion.getPuissance();

        std::cout << joueur->getNom() << " utilise : " << potion.getNom() << "." << std::endl;
    }
    else
    {
        if (!attaquant.consommerPotionDegats())
        {
            std::cout << attaquant.getNom()
                      << " cherche une potion offensive, mais sa rage est déjà épuisée."
                      << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }
    }

    std::cout << attaquant.getNom() << " sent ses forces monter d'un coup." << std::endl;
    std::cout << "Une rage brutale s'empare de lui..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    bool esquive = false;
    bool critique = false;

    int degatsBruts = attaquant.attaquer(random, esquive, critique, bonusUtilise);

    if (esquive)
    {
        std::cout << attaquant.getNom()
                  << " attaque, mais sa puissance le paralyse un court instant."
                  << std::endl;
        std::cout << defenseur.getNom()
                  << " évite l'assaut sans subir de dégâts."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (atlasBloqueAttaque(attaquant, defenseur, degatsBruts))
    {
        return true;
    }

    if (critique)
    {
        std::cout << "La rage de " << attaquant.getNom() << " explose dans l'arène." << std::endl;
        std::cout << "Il inflige " << degatsBruts << " dégâts bruts monstrueux." << std::endl;
    }
    else
    {
        std::cout << attaquant.getNom()
                  << " attaque avec une puissance dévastatrice et inflige "
                  << degatsBruts
                  << " dégâts bruts."
                  << std::endl;
    }

    int degatsRecus = SystemeDegats::appliquerProtectionArmure(defenseur, degatsBruts);

    defenseur.recevoirDegats(degatsRecus);
    appliquerVolDeVieDemonSiBesoin(attaquant, degatsRecus);

    std::cout << defenseur.getNom() << " reçoit " << degatsRecus << " dégâts." << std::endl;
    std::cout << defenseur.getNom() << " possède maintenant "
              << defenseur.getPv() << "/" << defenseur.getPvMax() << " PV."
              << std::endl;
    std::cout << std::endl;

    return true;
}

bool ActionsCombat::atlasBloqueAttaque(
    Entite& attaquant,
    Entite& defenseur,
    int degats
)
{
    Boss* bossDefenseur = dynamic_cast<Boss*>(&defenseur);

    if (bossDefenseur == nullptr)
    {
        return false;
    }

    if (bossDefenseur->getIdBoss() != 3 || !bossDefenseur->ultimeActif())
    {
        return false;
    }

    int degatsRenvoi = degats / 3;

    attaquant.recevoirDegats(degatsRenvoi);

    std::cout << attaquant.getNom() << " frappe de toutes ses forces..." << std::endl;
    Console::pauseSecondes(1);

    std::cout << "Mais l'armure d'" << bossDefenseur->getNom() << " absorbe l'impact." << std::endl;
    std::cout << "Une partie de la puissance est renvoyée à " << attaquant.getNom()
              << ", qui subit " << degatsRenvoi << " dégâts." << std::endl;
    std::cout << std::endl;

    std::cout << attaquant.getNom() << " possède maintenant "
              << attaquant.getPv() << "/" << attaquant.getPvMax() << " PV."
              << std::endl;
    std::cout << std::endl;

    return true;
}

void ActionsCombat::appliquerVolDeVieDemonSiBesoin(
    Entite& attaquant,
    int degatsInfliges
)
{
    Boss* bossAttaquant = dynamic_cast<Boss*>(&attaquant);

    if (bossAttaquant == nullptr)
    {
        return;
    }

    if (bossAttaquant->getIdBoss() != 2 || !bossAttaquant->ultimeActif())
    {
        return;
    }

    if (bossAttaquant->getEffetSpecial() != 2 && bossAttaquant->getEffetSpecial() != 3)
    {
        return;
    }

    int soin = degatsInfliges * 50 / 100;

    if (soin <= 0)
    {
        return;
    }

    bossAttaquant->soigner(soin);

    std::cout << bossAttaquant->getNom()
              << " absorbe le sang de l'attaque et récupère "
              << soin
              << " PV."
              << std::endl;
    std::cout << std::endl;
}