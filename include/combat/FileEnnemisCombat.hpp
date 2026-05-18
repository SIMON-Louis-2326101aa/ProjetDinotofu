#ifndef FILE_ENNEMIS_COMBAT_HPP
#define FILE_ENNEMIS_COMBAT_HPP

#include "entite/Monstre.hpp"

#include <vector>

class FileEnnemisCombat
{
public:
    static constexpr int NOMBRE_MAX_ENNEMIS_ACTIFS = 3;

    FileEnnemisCombat();

    void ajouterEnnemiEnAttente(const Monstre& monstre);
    void initialiserPremiereLigne();

    bool aEncoreDesEnnemis() const;
    bool aDesEnnemisActifs() const;
    bool aDesEnnemisEnAttente() const;

    int getNombreEnnemisActifs() const;
    int getNombreEnnemisEnAttente() const;
    int getNombreTotalEnnemisRestants() const;

    bool indexActifValide(int index) const;

    Monstre& getEnnemiActif(int index);
    const Monstre& getEnnemiActif(int index) const;

    void retirerEnnemiActif(int index);
    void retirerMortsEtRemplacer();

    void afficherEnnemisActifs() const;
    void afficherFileResume() const;

private:
    std::vector<Monstre> ennemisActifs;
    std::vector<Monstre> ennemisEnAttente;

    bool peutAjouterEnnemiActif() const;
    void faireEntrerProchainEnnemi();
};

#endif