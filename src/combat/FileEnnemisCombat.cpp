#include "combat/FileEnnemisCombat.hpp"

#include <iostream>

FileEnnemisCombat::FileEnnemisCombat()
{
}

bool FileEnnemisCombat::ajouterDansFile(const Monstre& monstre)
{
    if (static_cast<int>(fileAttente.size() + ennemisActifs.size()) >= MAX_FILE)
    {
        return false;
    }

    fileAttente.push_back(monstre);
    return true;
}

void FileEnnemisCombat::remplirActifs()
{
    while (static_cast<int>(ennemisActifs.size()) < MAX_ACTIFS && !fileAttente.empty())
    {
        ennemisActifs.push_back(fileAttente.front());
        fileAttente.erase(fileAttente.begin());
    }
}

bool FileEnnemisCombat::tousMorts() const
{
    return ennemisActifs.empty() && fileAttente.empty();
}

bool FileEnnemisCombat::aEncoreDesEnnemis() const
{
    return !tousMorts();
}

int FileEnnemisCombat::getNombreEnnemisActifs() const
{
    return static_cast<int>(ennemisActifs.size());
}

int FileEnnemisCombat::getNombreEnFile() const
{
    return static_cast<int>(fileAttente.size());
}

int FileEnnemisCombat::getNombreTotalEnnemis() const
{
    return static_cast<int>(ennemisActifs.size() + fileAttente.size());
}

Monstre& FileEnnemisCombat::getEnnemiActif(int index)
{
    return ennemisActifs.at(index);
}

const Monstre& FileEnnemisCombat::getEnnemiActif(int index) const
{
    return ennemisActifs.at(index);
}

bool FileEnnemisCombat::indexActifValide(int index) const
{
    return index >= 0 && index < static_cast<int>(ennemisActifs.size());
}

bool FileEnnemisCombat::retirerEnnemiActif(int index)
{
    if (!indexActifValide(index))
    {
        return false;
    }

    ennemisActifs.erase(ennemisActifs.begin() + index);

    if (!fileAttente.empty())
    {
        Monstre nouveau = fileAttente.front();
        fileAttente.erase(fileAttente.begin());

        std::cout << nouveau.getNom()
                  << " surgit depuis l'arrière de la vague et prend sa place."
                  << std::endl;

        ennemisActifs.push_back(nouveau);
    }

    std::cout << std::endl;

    return true;
}

void FileEnnemisCombat::retirerMortsEtRemplacer()
{
    for (int i = static_cast<int>(ennemisActifs.size()) - 1; i >= 0; --i)
    {
        if (ennemisActifs[i].estMort())
        {
            std::cout << ennemisActifs[i].getNom() << " tombe au sol." << std::endl;

            ennemisActifs.erase(ennemisActifs.begin() + i);

            if (!fileAttente.empty())
            {
                Monstre nouveau = fileAttente.front();
                fileAttente.erase(fileAttente.begin());

                std::cout << nouveau.getNom()
                          << " surgit depuis l'arrière de la vague et prend sa place."
                          << std::endl;

                ennemisActifs.push_back(nouveau);
            }

            std::cout << std::endl;
        }
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

            std::cout << i << " : " << monstre.getNom();

            if (monstre.statsVisibles())
            {
                std::cout << " | PV : "
                          << monstre.getPv()
                          << "/"
                          << monstre.getPvMax();
            }
            else
            {
                std::cout << " | PV : ???";
            }

            std::cout << " | Race : " << monstre.getRaceTexte();

            if (monstre.estInvocation())
            {
                std::cout << " | Invocation";
            }
            else if (monstre.estElite())
            {
                std::cout << " | Élite";
            }

            std::cout << std::endl;
        }
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}

void FileEnnemisCombat::afficherFileResume() const
{
    std::cout << "Ennemis actifs : " << ennemisActifs.size() << "/3" << std::endl;
    std::cout << "Ennemis en attente : " << fileAttente.size() << std::endl;
    std::cout << "Total restant : " << getNombreTotalEnnemis() << std::endl;
    std::cout << std::endl;
}