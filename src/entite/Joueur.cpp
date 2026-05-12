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
    indexArmureEquipee = -1;
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
    indexArmureEquipee = -1;
}

int Joueur::getBonusPvArmureEquipee() const
{
    if (!aUneArmureEquipee())
    {
        return 0;
    }

    Armure armure = getArmureEquipee();

    if (armure.estCassee())
    {
        return 0;
    }

    return armure.getBonusPvMax();
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

int Joueur::getIndexArmureEquipee() const
{
    return indexArmureEquipee;
}

bool Joueur::aUneArmureEquipee() const
{
    return inventaire.possedeArmure(indexArmureEquipee);
}

Armure Joueur::getArmureEquipee() const
{
    if (!aUneArmureEquipee())
    {
        return Armure();
    }

    return inventaire.getArmure(indexArmureEquipee);
}

bool Joueur::equiperArmure(int index)
{
    if (!inventaire.possedeArmure(index))
    {
        return false;
    }

    int ancienBonusPv = getBonusPvArmureEquipee();

    indexArmureEquipee = index;

    int nouveauBonusPv = getBonusPvArmureEquipee();
    int differencePvMax = nouveauBonusPv - ancienBonusPv;

    pvMax += differencePvMax;
    pv += differencePvMax;

    if (pvMax < 1)
    {
        pvMax = 1;
    }

    if (pv < 1)
    {
        pv = 1;
    }

    if (pv > pvMax)
    {
        pv = pvMax;
    }

    return true;
}

void Joueur::desequiperArmure()
{
    int ancienBonusPv = getBonusPvArmureEquipee();

    indexArmureEquipee = -1;

    pvMax -= ancienBonusPv;

    if (pvMax < 1)
    {
        pvMax = 1;
    }

    if (pv > pvMax)
    {
        pv = pvMax;
    }
}

void Joueur::initialiserInventaireDeBase()
{
    inventaire.ajouterArme(CatalogueArmes::creerMainsNues());
    inventaire.ajouterArme(CatalogueArmes::creerEpeeRouillee());

    equiperArme(1);

    inventaire.ajouterArmure(CatalogueArmures::creerTenueSimple());
    inventaire.ajouterArmure(CatalogueArmures::creerArmureCuirUsee());

    equiperArmure(0);

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
    std::cout << "===== STATS JOUEUR =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Classe : " << type << std::endl;
    std::cout << "Niveau : " << niveau << std::endl;
    std::cout << "Expérience : " << experience << "/100" << std::endl;
    std::cout << "PV : " << pv << "/" << pvMax << std::endl;
    std::cout << "Dégâts de base : " << degatsMin << " - " << degatsMax << std::endl;
    std::cout << "Critique de base : " << degatsCrit << std::endl;

    std::cout << "Potions de soin : "
              << inventaire.compterConsommables(TypeConsommable::Soin)
              << std::endl;

    std::cout << "Potions de rage : "
              << inventaire.compterConsommables(TypeConsommable::Degats)
              << std::endl;

    std::cout << "========================" << std::endl;
    std::cout << std::endl;
}

void Joueur::afficherInventaire() const
{
    inventaire.afficher();
    afficherEquipementSimple();
}

void Joueur::afficherEquipementSimple() const
{
    std::cout << "===== ÉQUIPEMENT =====" << std::endl;

    if (aUneArmeEquipee())
    {
        Arme arme = getArmeEquipee();

        std::cout << "Arme équipée : " << arme.getNom();

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
        std::cout << "Arme équipée : Aucune" << std::endl;
    }

    if (aUneArmureEquipee())
    {
        Armure armure = getArmureEquipee();

        std::cout << "Armure équipée : " << armure.getNom();

        if (!armure.estIndestructible())
        {
            std::cout << " (" << armure.getDurabilite() << "/" << armure.getDurabiliteMax() << ")";
        }

        if (armure.estCassee())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Armure équipée : Aucune" << std::endl;
    }

    std::cout << "Or : " << inventaire.getOr() << " pièces" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << std::endl;
}

void Joueur::afficherEquipementDetaille() const
{
    std::cout << "========== ÉQUIPEMENT DÉTAILLÉ ==========" << std::endl;
    std::cout << std::endl;

    if (aUneArmeEquipee())
    {
        Arme arme = getArmeEquipee();

        std::cout << "=== Arme équipée ===" << std::endl;
        std::cout << "Nom : " << arme.getNom() << std::endl;
        std::cout << "Description : " << arme.getDescription() << std::endl;
        std::cout << "Bonus dégâts : +"
                  << arme.getBonusDegatsMin()
                  << " à +"
                  << arme.getBonusDegatsMax()
                  << std::endl;
        std::cout << "Bonus critique : +" << arme.getBonusCritique() << std::endl;

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
            std::cout << "État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }
    }
    else
    {
        std::cout << "=== Arme équipée ===" << std::endl;
        std::cout << "Aucune arme équipée." << std::endl;
    }

    std::cout << std::endl;

    if (aUneArmureEquipee())
    {
        Armure armure = getArmureEquipee();

        std::cout << "=== Armure équipée ===" << std::endl;
        std::cout << "Nom : " << armure.getNom() << std::endl;
        std::cout << "Description : " << armure.getDescription() << std::endl;
        std::cout << "Bonus PV max : +" << armure.getBonusPvMax() << std::endl;
        std::cout << "Réduction dégâts : " << armure.getReductionDegats() << std::endl;

        if (armure.estIndestructible())
        {
            std::cout << "Durabilité : Indestructible" << std::endl;
        }
        else
        {
            std::cout << "Durabilité : " << armure.getDurabilite() << "/" << armure.getDurabiliteMax() << std::endl;
        }

        if (armure.estCassee())
        {
            std::cout << "État : Cassée, ses bonus ne s'appliquent plus." << std::endl;
        }
        else
        {
            std::cout << "État : Utilisable" << std::endl;
        }
    }
    else
    {
        std::cout << "=== Armure équipée ===" << std::endl;
        std::cout << "Aucune armure équipée." << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Or : " << inventaire.getOr() << " pièces" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}