#ifndef RAPPORT_DEGATS_HPP
#define RAPPORT_DEGATS_HPP

struct RapportDegats
{
    int degatsBruts;
    int degatsAbsorbesArmure;
    int degatsReduitsClasse;
    int degatsRecus;
    bool armureUtilisee;
    bool armureCasseePendantImpact;

    RapportDegats()
        : degatsBruts(0),
          degatsAbsorbesArmure(0),
          degatsReduitsClasse(0),
          degatsRecus(0),
          armureUtilisee(false),
          armureCasseePendantImpact(false)
    {
    }
};

#endif