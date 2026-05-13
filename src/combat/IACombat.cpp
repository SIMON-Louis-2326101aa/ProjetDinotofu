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

    if (ia.getPv() <= 60 && soinDisponible)
    {
        return 2;
    }

    int option = random.entre(1, 100);

    if (ia.getPv() <= 60 && !soinDisponible)
    {
        if (degatsDisponible && option <= 25)
        {
            return 3;
        }

        return 1;
    }

    if (degatsDisponible && option <= 20)
    {
        return 3;
    }

    if (option <= 85)
    {
        return 1;
    }

    return 5;
}

int IACombat::choisirActionBoss(const Boss& boss, Random& random)
{
    if (boss.getPv() <= (boss.getPvMax() / 4) && boss.getPotionsSoin() > 0)
    {
        return 2;
    }

    if (boss.peutUtiliserUltime())
    {
        return 4;
    }

    bool degatsDisponible = boss.getPotionsDegats() > 0;

    int option = random.entre(1, 100);

    if (degatsDisponible && option <= 20)
    {
        return 3;
    }

    if (option <= 90)
    {
        return 1;
    }

    return 5;
}