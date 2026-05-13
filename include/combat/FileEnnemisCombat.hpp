#ifndef FILE_ENNEMIS_COMBAT_HPP
#define FILE_ENNEMIS_COMBAT_HPP

#include "entite/Monstre.hpp"

#include <vector>

class FileEnnemisCombat
{
private:
    static constexpr int MAX_FILE = 10;
    static constexpr int MAX_ACTIFS = 3;

    std::vector<Monstre> fileAttente;
    std::vector<Monstre> ennemisActifs;

public:
    FileEnnemisCombat();

    bool ajouterDansFile(const Monstre& monstre);
    void remplirActifs();

    bool tousMorts() const;
    bool aEncoreDesEnnemis() const;

    int getNombreEnnemisActifs() const;
    int getNombreEnFile() const;
    int getNombreTotalEnnemis() const;

    Monstre& getEnnemiActif(int index);
    const Monstre& getEnnemiActif(int index) const;

    bool indexActifValide(int index) const;

    bool retirerEnnemiActif(int index);
    void retirerMortsEtRemplacer();

    void afficherEnnemisActifs() const;
    void afficherFileResume() const;
};

#endif