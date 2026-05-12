#include "objet/consommable/CatalogueConsommables.hpp"

Consommable CatalogueConsommables::creerPotionSoinBasique()
{
    return Consommable(
        "Potion de soin",
        "Referme les blessures et redonne un souffle de vie.",
        25,
        TypeConsommable::Soin,
        55
    );
}

Consommable CatalogueConsommables::creerPotionDegatsBasique()
{
    return Consommable(
        "Potion de rage",
        "Fait monter la puissance d'un coup, mais force à attaquer immédiatement.",
        30,
        TypeConsommable::Degats,
        25
    );
}

Consommable CatalogueConsommables::creerPotionSoinRenforcee()
{
    return Consommable(
        "Potion de soin renforcée",
        "Une potion plus rare, utilisée lorsque l'arène décide que le combat doit durer.",
        60,
        TypeConsommable::Soin,
        75
    );
}

Consommable CatalogueConsommables::creerPotionDegatsRenforcee()
{
    return Consommable(
        "Potion de rage supérieure",
        "Une rage plus pure, plus brutale, presque trop violente pour un simple humain.",
        70,
        TypeConsommable::Degats,
        45
    );
}