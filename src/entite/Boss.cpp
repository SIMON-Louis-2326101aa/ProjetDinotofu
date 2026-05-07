#include "entite/Boss.hpp"

#include <iostream>

Boss::Boss() : Entite()
{
    idBoss = 0;
    ultimeRestant = 0;
    ultimeMax = 0;
    delaiUltime = 0;
    delaiUltimeMax = 0;
    effetSpecial = 0;
    statsDecryptees = false;
}

Boss::Boss(
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
) : Entite(
        nom,
        type,
        pvMax,
        degatsMin,
        degatsMax,
        degatsCrit,
        potionsSoin,
        potionsDegats
    )
{
    this->idBoss = idBoss;

    this->ultimeRestant = 0;
    this->ultimeMax = ultimeMax;

    this->delaiUltime = delaiUltimeMax;
    this->delaiUltimeMax = delaiUltimeMax;

    this->effetSpecial = 0;
    this->statsDecryptees = false;
}

int Boss::getIdBoss() const
{
    return idBoss;
}

int Boss::getUltimeRestant() const
{
    return ultimeRestant;
}

int Boss::getUltimeMax() const
{
    return ultimeMax;
}

int Boss::getDelaiUltime() const
{
    return delaiUltime;
}

int Boss::getDelaiUltimeMax() const
{
    return delaiUltimeMax;
}

int Boss::getEffetSpecial() const
{
    return effetSpecial;
}

void Boss::setEffetSpecial(int effet)
{
    effetSpecial = effet;
}

bool Boss::peutUtiliserUltime() const
{
    return ultimeRestant <= 0 && delaiUltime <= 0 && pv <= (pvMax / 2);
}

bool Boss::ultimeActif() const
{
    return ultimeRestant > 0;
}

void Boss::activerUltime()
{
    ultimeRestant = ultimeMax;
}

void Boss::reduireUltime()
{
    if (ultimeRestant > 0)
    {
        ultimeRestant--;
    }
}

void Boss::reduireDelaiUltime()
{
    if (ultimeRestant <= 0 && delaiUltime > 0)
    {
        delaiUltime--;
    }
}

void Boss::reinitialiserDelaiUltime()
{
    delaiUltime = delaiUltimeMax;
    effetSpecial = 0;
}

bool Boss::statsVisibles() const
{
    return statsDecryptees;
}

bool Boss::doitDecrypterStats() const
{
    return !statsDecryptees && pv <= (pvMax / 2);
}

void Boss::decrypterStats()
{
    statsDecryptees = true;
}

void Boss::afficherStats() const
{
    if (!statsDecryptees)
    {
        std::cout << "Tentative de décryptage des statistiques de l'entité échouée." << std::endl;
        std::cout << "Sa puissance brouille encore toute lecture fiable." << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "===== STATISTIQUES DE L'ENTITÉ =====" << std::endl;
    std::cout << "Nom : " << nom << std::endl;
    std::cout << "Type d'entité : " << type << std::endl;
    std::cout << "PV : " << pv << "/" << pvMax << std::endl;
    std::cout << "Dégâts : " << degatsMin << " - " << degatsMax << std::endl;
    std::cout << "Critique : " << degatsCrit << std::endl;
    std::cout << "Potions de soin : " << potionsSoin << std::endl;
    std::cout << "Potions de dégâts : " << potionsDegats << std::endl;
    std::cout << "Ultime restant : " << ultimeRestant << std::endl;
    std::cout << "Délai ultime : " << delaiUltime << std::endl;

    if (effetSpecial > 0)
    {
        std::cout << "Effet spécial actif : " << effetSpecial << std::endl;
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}