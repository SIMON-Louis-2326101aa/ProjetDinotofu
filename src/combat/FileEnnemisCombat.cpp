#include "combat/FileEnnemisCombat.hpp"

#include <iostream>

FileEnnemisCombat::FileEnnemisCombat()
{
}

void FileEnnemisCombat::ajouterEnnemiEnAttente(const Monstre& monstre)
{
    ennemisEnAttente.push_back(monstre);
}

void FileEnnemisCombat::initialiserPremiereLigne()
{
    while (peutAjouterEnnemiActif() && !ennemisEnAttente.empty())
    {
        faireEntrerProchainEnnemi();
    }
}

bool FileEnnemisCombat::aEncoreDesEnnemis() const
{
    return !ennemisActifs.empty() || !ennemisEnAttente.empty();
}

bool FileEnnemisCombat::aDesEnnemisActifs() const
{
    return !ennemisActifs.empty();
}

bool FileEnnemisCombat::aDesEnnemisEnAttente() const
{
    return !ennemisEnAttente.empty();
}

int FileEnnemisCombat::getNombreEnnemisActifs() const
{
    return static_cast<int>(ennemisActifs.size());
}

int FileEnnemisCombat::getNombreEnnemisEnAttente() const
{
    return static_cast<int>(ennemisEnAttente.size());
}

int FileEnnemisCombat::getNombreTotalEnnemisRestants() const
{
    return getNombreEnnemisActifs() + getNombreEnnemisEnAttente();
}

bool FileEnnemisCombat::indexActifValide(int index) const
{
    return index >= 0 && index < getNombreEnnemisActifs();
}

Monstre& FileEnnemisCombat::getEnnemiActif(int index)
{
    return ennemisActifs[index];
}

const Monstre& FileEnnemisCombat::getEnnemiActif(int index) const
{
    return ennemisActifs[index];
}

void FileEnnemisCombat::retirerEnnemiActif(int index)
{
    if (!indexActifValide(index))
    {
        return;
    }

    ennemisActifs.erase(ennemisActifs.begin() + index);

    if (peutAjouterEnnemiActif() && !ennemisEnAttente.empty())
    {
        faireEntrerProchainEnnemi();
    }
}

void FileEnnemisCombat::retirerMortsEtRemplacer()
{
    int i = 0;

    while (i < getNombreEnnemisActifs())
    {
        if (ennemisActifs[i].estMort())
        {
            std::cout << ennemisActifs[i].getNom()
                      << " disparaît de la première ligne."
                      << std::endl;

            retirerEnnemiActif(i);
            continue;
        }

        ++i;
    }
}

void FileEnnemisCombat::afficherEnnemisActifs() const
{
    std::cout << "========== ENNEMIS ACTIFS ==========" << std::endl;

    if (ennemisActifs.empty())
    {
        std::cout << "Aucun ennemi actif." << std::endl;
    }
    else
    {
        for (int i = 0; i < static_cast<int>(ennemisActifs.size()); ++i)
        {
            const Monstre& monstre = ennemisActifs[i];

            std::cout << i + 1
                      << " : "
                      << monstre.getNom()
                      << " | "
                      << monstre.getPv()
                      << "/"
                      << monstre.getPvMax()
                      << " PV"
                      << std::endl;
        }
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}

void FileEnnemisCombat::afficherFileResume() const
{
    std::cout << "========== ÉTAT DE LA VAGUE ==========" << std::endl;
    std::cout << "Ennemis actifs : " << getNombreEnnemisActifs() << std::endl;
    std::cout << "Ennemis en attente : " << getNombreEnnemisEnAttente() << std::endl;
    std::cout << "Total restant : " << getNombreTotalEnnemisRestants() << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;
}

bool FileEnnemisCombat::peutAjouterEnnemiActif() const
{
    return getNombreEnnemisActifs() < NOMBRE_MAX_ENNEMIS_ACTIFS;
}

void FileEnnemisCombat::faireEntrerProchainEnnemi()
{
    if (ennemisEnAttente.empty())
    {
        return;
    }

    Monstre prochain = ennemisEnAttente.front();
    ennemisEnAttente.erase(ennemisEnAttente.begin());

    ennemisActifs.push_back(prochain);

    std::cout << prochain.getNom()
              << " entre dans la première ligne."
              << std::endl;
    std::cout << std::endl;
}