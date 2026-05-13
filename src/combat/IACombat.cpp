#include "combat/IACombat.hpp"

#include "entite/Joueur.hpp"
#include "objet/consommable/TypeConsommable.hpp"

bool IACombat::peutUtiliserPotionSoin(const Entite& entite)
{
    const Joueur* joueur = dynamic_cast<const Joueur*>(&entite);

    if (joueur != nullptr)
    {
        return joueur->getInventaire().compterConsommables(TypeConsommable::Soin) > 0;
    }

    return entite.getPotionsSoin() > 0;
}

bool IACombat::peutUtiliserPotionDegats(const Entite& entite)
{
    const Joueur* joueur = dynamic_cast<const Joueur*>(&entite);

    if (joueur != nullptr)
    {
        return joueur->getInventaire().compterConsommables(TypeConsommable::Degats) > 0;
    }

    return entite.getPotionsDegats() > 0;
}

int IACombat::choisirActionIA(const Entite& ia, Random& random)
{
    bool soinDisponible = peutUtiliserPotionSoin(ia);
    bool degatsDisponible = peutUtiliserPotionDegats(ia);

    int pourcentagePv = ia.getPv() * 100 / ia.getPvMax();
    int tirage = random.entre(1, 100);

    if (pourcentagePv <= 30)
    {
        if (soinDisponible && tirage <= 55)
        {
            return 2;
        }

        if (degatsDisponible && tirage <= 70)
        {
            return 3;
        }

        if (tirage <= 97)
        {
            return 1;
        }

        return 5;
    }

    if (pourcentagePv <= 60)
    {
        if (soinDisponible && tirage <= 30)
        {
            return 2;
        }

        if (degatsDisponible && tirage <= 45)
        {
            return 3;
        }

        if (tirage <= 95)
        {
            return 1;
        }

        return 5;
    }

    if (degatsDisponible && tirage <= 18)
    {
        return 3;
    }

    if (tirage <= 94)
    {
        return 1;
    }

    return 5;
}

int IACombat::choisirActionBoss(const Boss& boss, Random& random)
{
    bool soinDisponible = boss.getPotionsSoin() > 0;
    bool degatsDisponible = boss.getPotionsDegats() > 0;
    bool ultimeDisponible = boss.peutUtiliserUltime();

    int pourcentagePv = boss.getPv() * 100 / boss.getPvMax();
    int tirage = random.entre(1, 100);

    if (pourcentagePv <= 25)
    {
        if (soinDisponible && tirage <= 35)
        {
            return 2;
        }

        if (ultimeDisponible && tirage <= 70)
        {
            return 4;
        }

        if (degatsDisponible && tirage <= 82)
        {
            return 3;
        }

        return 1;
    }

    if (ultimeDisponible)
    {
        if (tirage <= 50)
        {
            return 4;
        }

        if (degatsDisponible && tirage <= 65)
        {
            return 3;
        }

        return 1;
    }

    if (degatsDisponible && tirage <= 18)
    {
        return 3;
    }

    return 1;
}