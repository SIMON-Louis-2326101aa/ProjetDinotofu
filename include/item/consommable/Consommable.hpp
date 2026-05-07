#ifndef CONSOMMABLE_HPP
#define CONSOMMABLE_HPP

#include <string>

class Entite;

enum class TypeConsommable {
    Soin,
    BonusDegats
};

class Consommable {
private:
    std::string nom;
    TypeConsommable type;
    int valeur; // PV rendus ou bonus plat aux dégâts pour 1 attaque

public:
    Consommable();
    Consommable(const std::string& nom, TypeConsommable type, int valeur);

    const std::string& getNom() const;
    TypeConsommable getType() const;
    int getValeur() const;

    void setNom(const std::string& v);
    void setType(TypeConsommable t);
    void setValeur(int v);

    // Applique l'effet :
    // - Soin : modifie l'entité
    // - BonusDegats : ajoute à bonusDegatsTemp (pour la prochaine attaque)
    bool consommer(Entite& cible, int& bonusDegatsTemp) const;

    void afficher() const; // implémenté dans .cpp
};

#endif
