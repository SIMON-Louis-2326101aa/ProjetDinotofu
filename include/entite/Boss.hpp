#ifndef BOSS_HPP
#define BOSS_HPP

#include "entite/Entite.hpp"

class Boss : public Entite
{
private:
    int idBoss;
    int ultimeRestant;
    int ultimeMax;
    int delaiUltime;
    int delaiUltimeMax;
    int effetSpecial;
    bool statsDecryptees;

public:
    Boss();

    Boss(
        int idBoss,
        const std::string& nom,
        const std::string& type,
        int pvMax,
        int degatsMin,
        int degatsMax,
        int degatsCrit,
        int potionsSoin,
        int potionsDegats,
        int ultimeMax,
        int delaiUltimeMax
    );

    int getIdBoss() const;

    int getUltimeRestant() const;
    int getUltimeMax() const;

    int getDelaiUltime() const;
    int getDelaiUltimeMax() const;

    int getEffetSpecial() const;
    void setEffetSpecial(int effet);

    bool peutUtiliserUltime() const;
    bool ultimeActif() const;

    void activerUltime();
    void reduireUltime();
    void reduireDelaiUltime();
    void reinitialiserDelaiUltime();

    bool statsVisibles() const override;
    bool doitDecrypterStats() const;
    void decrypterStats();

    void afficherStats() const override;
};

#endif