#include "entite/Joueur.hpp"

#include "objet/arme/CatalogueArmes.hpp"
#include "objet/armure/CatalogueArmures.hpp"
#include "objet/consommable/CatalogueConsommables.hpp"

#include <iostream>

Joueur::Joueur() : Entite()
{
    niveau = 1;
    experience = 0;
    indexArmeEquipee = -1;
}

Joueur::Joueur(
    const std::string& nom,
    const ClasseJoueur& classe
) : Entite(
        nom,
        classe.getNom(),
        classe.getPvMax(),
        classe.getDegatsMin(),
        classe.getDegatsMax(),
        classe.getDegatsCrit(),
        classe.getPotionsSoin(),
        classe.getPotionsDegats()
    )
{
    niveau = 1;
    experience = 0;
    indexArmeEquipee = -1;
}

int Joueur::getNiveau() const
{
    return niveau;
}

int Joueur::getExperience() const
{
    return experience;
}

Inventaire& Joueur::getInventaire()
{
    return inventaire;
}

const Inventaire& Joueur::getInventaire() const
{
    return inventaire;
}

int Joueur::getIndexArmeEquipee() const
{
    return indexArmeEquipee;
}

bool Joueur::aUneArmeEquipee() const
{
    return inventaire.possedeArme(indexArmeEquipee);
}

Arme Joueur::getArmeEquipee() const
{
    if (!aUneArmeEquipee())
    {
        return Arme();
    }

    return inventaire.getArme(indexArmeEquipee);
}

bool Joueur::equiperArme(int index)
{
    if (!inventaire.possedeArme(index))
    {
        return false;
    }

    indexArmeEquipee = index;
    return true;
}

void Joueur::desequiperArme()
{
    indexArmeEquipee = -1;
}

void Joueur::initialiserInventaireDeBase()
{
    inventaire.ajouterArme(CatalogueArmes::creerMainsNues());
    inventaire.ajouterArme(CatalogueArmes::creerEpeeRouillee());

    equiperArme(1);

    inventaire.ajouterArmure(CatalogueArmures::creerTenueSimple());
    inventaire.ajouterArmure(CatalogueArmures::creerArmureCuirUsee());

    for (int i = 0; i < potionsSoin; i++)
    {
        inventaire.ajouterConsommable(CatalogueConsommables::creerPotionSoinBasique());
    }

    for (int i = 0; i < potionsDegats; i++)
    {
        inventaire.ajouterConsommable(CatalogueConsommables::creerPotionDegatsBasique());
    }

    inventaire.gagnerOr(50);
}

void Joueur::gagnerExperience(int quantite)
{
    if (quantite <= 0)
    {
        return;
    }

    experience += quantite;

    while (experience >= 100)
    {
        experience -= 100;
        monterNiveau();
    }
}

void Joueur::monterNiveau()
{
    niveau++;

    pvMax += 20;
    pv = pvMax;

    degatsMin += 1;
    degatsMax += 2;
    degatsCrit += 3;

    std::cout << nom << " monte au niveau " << niveau << " !" << std::endl;
    std::cout << "Ses blessures se referment, et sa puissance augmente." << std::endl;
    std::cout << std::endl;
}

int Joueur::attaquer(Random& random, bool& esquive, bool& critique, int bonusDegats)
{
    int resultat = random.lancerD20();

    esquive = false;
    critique = false;

    int bonusMin = 0;
    int bonusMax = 0;
    int bonusCritique = 0;

    Arme* armeEquipee = inventaire.getArmeModifiable(indexArmeEquipee);

    if (armeEquipee != nullptr && !armeEquipee->estCassee())
    {
        bonusMin = armeEquipee->getBonusDegatsMin();
        bonusMax = armeEquipee->getBonusDegatsMax();
        bonusCritique = armeEquipee->getBonusCritique();
    }

    if (resultat <= 3)
    {
        esquive = true;
        return 0;
    }

    if (armeEquipee != nullptr)
    {
        armeEquipee->perdreDurabilite(1);
    }

    if (armeEquipee != nullptr && armeEquipee->estCassee())
    {
        std::cout << "L'arme de " << nom << " s'abîme sous le choc..." << std::endl;
        std::cout << armeEquipee->getNom() << " est maintenant cassée et ne donnera plus ses bonus." << std::endl;
        std::cout << std::endl;
    }

    if (resultat <= 16)
    {
        return random.entre(
            degatsMin + bonusMin,
            degatsMax + bonusMax
        ) + bonusDegats;
    }

    critique = true;
    return degatsCrit + bonusCritique + bonusDegats;
}

void Joueur::afficherStats() const
{
    std::cout << "===== JOUEUR =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Classe : " << type << std::endl;
    std::cout << "Niveau : " << niveau << std::endl;
    std::cout << "Expérience : " << experience << "/100" << std::endl;
    std::cout << "PV : " << pv << "/" << pvMax << std::endl;

    if (aUneArmeEquipee())
    {
        Arme arme = getArmeEquipee();

        std::cout << "Arme équipée : " << arme.getNom() << std::endl;

        if (arme.estIndestructible())
        {
            std::cout << "Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "Durabilité : " << arme.getDurabilite() << "/" << arme.getDurabiliteMax() << std::endl;
        }

        if (arme.estCassee())
        {
            std::cout << "État de l'arme : Cassée" << std::endl;
            std::cout << "Dégâts : " << degatsMin << " - " << degatsMax << std::endl;
            std::cout << "Critique : " << degatsCrit << std::endl;
        }
        else
        {
            std::cout << "Dégâts avec arme : "
                      << degatsMin + arme.getBonusDegatsMin()
                      << " - "
                      << degatsMax + arme.getBonusDegatsMax()
                      << std::endl;
            std::cout << "Critique avec arme : "
                      << degatsCrit + arme.getBonusCritique()
                      << std::endl;
        }
    }
    else
    {
        std::cout << "Arme équipée : Aucune" << std::endl;
        std::cout << "Dégâts : " << degatsMin << " - " << degatsMax << std::endl;
        std::cout << "Critique : " << degatsCrit << std::endl;
    }

    std::cout << "Potions de soin : "
              << inventaire.compterConsommables(TypeConsommable::Soin)
              << std::endl;

    std::cout << "Potions de rage : "
              << inventaire.compterConsommables(TypeConsommable::Degats)
              << std::endl;
    std::cout << "Or : " << inventaire.getOr() << " pièces" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << std::endl;
}

void Joueur::afficherInventaire() const
{
    inventaire.afficher();

    if (aUneArmeEquipee())
    {
        Arme arme = getArmeEquipee();

        std::cout << "Arme actuellement équipée : " << arme.getNom();

        if (!arme.estIndestructible())
        {
            std::cout << " (" << arme.getDurabilite() << "/" << arme.getDurabiliteMax() << ")";
        }

        if (arme.estCassee())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Aucune arme équipée." << std::endl;
    }

    std::cout << std::endl;
}