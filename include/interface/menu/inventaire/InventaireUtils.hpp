#ifndef INVENTAIRE_UTILS_HPP
#define INVENTAIRE_UTILS_HPP

#include "entite/Joueur.hpp"

#include "objet/arme/Arme.hpp"
#include "objet/armure/Armure.hpp"
#include "objet/consommable/TypeConsommable.hpp"

#include <string>
#include <vector>

struct GroupeConsommable
{
    int premierIndex;
    int quantite;
    std::string nom;
    TypeConsommable type;
    int puissance;

    GroupeConsommable();
};

class InventaireUtils
{
public:
    static std::string typeConsommableVersTexte(TypeConsommable type);

    static std::string durabiliteArmeTexte(const Arme& arme);
    static std::string durabiliteArmureTexte(const Armure& armure);

    static std::vector<GroupeConsommable> grouperConsommables(const Joueur& joueur);
};

#endif