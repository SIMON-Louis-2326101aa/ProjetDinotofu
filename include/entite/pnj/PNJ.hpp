#ifndef PNJ_HPP
#define PNJ_HPP

#include <string>
#include <vector>
#include "entite/Entite.hpp"

class Arme;
class Consommable;

// Rôle simple pour orienter le comportement (dialogues, boutique, quêtes, etc.)
enum class RolePNJ {
    Neutre,
    Marchand,
    DonneurDeQuete,
    Histoire
};

struct OffreArme {
    Arme* ref;     // pointeur vers un objet possédé par le PNJ
    int prix;
};

struct OffreConso {
    Consommable* ref;
    int prix;
};

class PNJ : public Entite {
private:
    RolePNJ role;
    std::vector<std::string> dialogues;        // répliques simples
    std::vector<Arme> stockArmes;              // inventaire du PNJ (propriété)
    std::vector<Consommable> stockConsos;
    std::vector<OffreArme> offresArmes;        // vues/prix pour la boutique
    std::vector<OffreConso> offresConsos;

public:
    PNJ();
    PNJ(const std::string& nom, RolePNJ role);

    RolePNJ getRole() const;
    void setRole(RolePNJ r);

    // Dialogues
    void ajouterDialogue(const std::string& ligne);
    const std::vector<std::string>& getDialogues() const;

    // Boutique — ajoute un item au stock + crée une offre
    void ajouterOffreArme(const Arme& arme, int prix);
    void ajouterOffreConsommable(const Consommable& conso, int prix);

    const std::vector<OffreArme>& getOffresArmes() const;
    const std::vector<OffreConso>& getOffresConsos() const;

    // Vendre à un joueur : renvoie true si achat effectué
    bool vendreArme(int indexOffre, class Joueur& acheteur);
    bool vendreConsommable(int indexOffre, class Joueur& acheteur);

    // Affichages (implémentés dans .cpp)
    void afficherDialogues() const;
    void afficherBoutique() const;
};

#endif
