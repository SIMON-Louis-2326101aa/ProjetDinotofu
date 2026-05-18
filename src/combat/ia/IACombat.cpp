#include "combat/ia/IACombat.hpp"

#include "entite/Joueur.hpp"

#include "objet/Inventaire.hpp"
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

int IACombat::calculerPourcentagePv(const Entite& entite)
{
    if (entite.getPvMax() <= 0)
    {
        return 0;
    }

    return entite.getPv() * 100 / entite.getPvMax();
}

ActionIA IACombat::choisirActionIA(const Entite& ia, Random& random)
{
    bool soinDisponible = peutUtiliserPotionSoin(ia);
    bool degatsDisponible = peutUtiliserPotionDegats(ia);

    int pourcentagePv = calculerPourcentagePv(ia);
    int tirage = random.entre(1, 100);

    if (pourcentagePv <= 30)
    {
        return choisirActionIAEnDanger(
            soinDisponible,
            degatsDisponible,
            tirage
        );
    }

    if (pourcentagePv <= 60)
    {
        return choisirActionIAMiVie(
            soinDisponible,
            degatsDisponible,
            tirage
        );
    }

    return choisirActionIAStable(
        degatsDisponible,
        tirage
    );
}

ActionIA IACombat::choisirActionBoss(const Boss& boss, Random& random)
{
    bool soinDisponible = boss.getPotionsSoin() > 0;
    bool degatsDisponible = boss.getPotionsDegats() > 0;
    bool ultimeDisponible = boss.peutUtiliserUltime();

    int pourcentagePv = calculerPourcentagePv(boss);
    int tirage = random.entre(1, 100);

    if (pourcentagePv <= 25)
    {
        return choisirActionBossEnDanger(
            soinDisponible,
            degatsDisponible,
            ultimeDisponible,
            tirage
        );
    }

    if (ultimeDisponible)
    {
        return choisirActionBossUltimeDisponible(
            degatsDisponible,
            tirage
        );
    }

    return choisirActionBossStable(
        degatsDisponible,
        tirage
    );
}

ActionIA IACombat::choisirActionIAEnDanger(
    bool soinDisponible,
    bool degatsDisponible,
    int tirage
)
{
    if (soinDisponible && tirage <= 60)
    {
        return ActionIA::PotionSoin;
    }

    if (degatsDisponible && tirage <= 75)
    {
        return ActionIA::PotionDegats;
    }

    if (tirage <= 97)
    {
        return ActionIA::Attaquer;
    }

    return ActionIA::PasserTour;
}

ActionIA IACombat::choisirActionIAMiVie(
    bool soinDisponible,
    bool degatsDisponible,
    int tirage
)
{
    if (soinDisponible && tirage <= 30)
    {
        return ActionIA::PotionSoin;
    }

    if (degatsDisponible && tirage <= 48)
    {
        return ActionIA::PotionDegats;
    }

    if (tirage <= 96)
    {
        return ActionIA::Attaquer;
    }

    return ActionIA::PasserTour;
}

ActionIA IACombat::choisirActionIAStable(
    bool degatsDisponible,
    int tirage
)
{
    if (degatsDisponible && tirage <= 18)
    {
        return ActionIA::PotionDegats;
    }

    if (tirage <= 96)
    {
        return ActionIA::Attaquer;
    }

    return ActionIA::PasserTour;
}

ActionIA IACombat::choisirActionBossEnDanger(
    bool soinDisponible,
    bool degatsDisponible,
    bool ultimeDisponible,
    int tirage
)
{
    if (soinDisponible && tirage <= 35)
    {
        return ActionIA::PotionSoin;
    }

    if (ultimeDisponible && tirage <= 75)
    {
        return ActionIA::Ultime;
    }

    if (degatsDisponible && tirage <= 87)
    {
        return ActionIA::PotionDegats;
    }

    return ActionIA::Attaquer;
}

ActionIA IACombat::choisirActionBossUltimeDisponible(
    bool degatsDisponible,
    int tirage
)
{
    if (tirage <= 55)
    {
        return ActionIA::Ultime;
    }

    if (degatsDisponible && tirage <= 70)
    {
        return ActionIA::PotionDegats;
    }

    return ActionIA::Attaquer;
}

ActionIA IACombat::choisirActionBossStable(
    bool degatsDisponible,
    int tirage
)
{
    if (degatsDisponible && tirage <= 18)
    {
        return ActionIA::PotionDegats;
    }

    return ActionIA::Attaquer;
}