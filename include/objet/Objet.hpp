#ifndef OBJET_HPP
#define OBJET_HPP

#include <string>

class Objet
{
protected:
    std::string nom;
    std::string description;
    int valeur;

public:
    Objet();

    Objet(
        const std::string& nom,
        const std::string& description,
        int valeur
    );

    virtual ~Objet() = default;

    std::string getNom() const;
    std::string getDescription() const;
    int getValeur() const;

    virtual void afficher() const;
};

#endif