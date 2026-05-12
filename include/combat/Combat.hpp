#ifndef COMBAT_HPP
#define COMBAT_HPP

#include "entite/Joueur.hpp"
#include "entite/Boss.hpp"
#include "core/Random.hpp"

class Combat
{
private:
    Random random;

    static constexpr int SOIN_POTION = 55;
    static constexpr int SOIN_POTION_BOSS = 75;

    static constexpr int BONUS_POTION_DEGATS = 25;
    static constexpr int BONUS_POTION_DEGATS_BOSS = 45;

    void afficherMenuTour(const Entite& entite) const;
    void afficherManuelPotions(int soinPotion, int bonusPotionDegats) const;

    bool jouerTourHumain(Entite& attaquant, Entite& defenseur, int soinPotion, int bonusPotionDegats);
    bool jouerTourIA(Entite& ia, Entite& defenseur, int soinPotion, int bonusPotionDegats);
    bool jouerTourBoss(Boss& boss, Entite& joueur);

    int choisirActionIA(const Entite& ia);
    int choisirActionBoss(const Boss& boss);

    void executerAttaque(Entite& attaquant, Entite& defenseur);
    bool executerPotionDegats(Entite& attaquant, Entite& defenseur, int bonusPotionDegats);

    bool ouvrirInventaire(Joueur& joueur);
    bool equiperArmeDepuisInventaire(Joueur& joueur);

    bool atlasBloqueAttaque(Entite& attaquant, Entite& defenseur, int degats);
    void appliquerVolDeVieDemonSiBesoin(Entite& attaquant, int degatsInfliges);

    void verifierDecryptageBoss(Boss& boss);
    void executerUltimeBoss(Boss& boss, Entite& joueur);
    bool gererFinTourBoss(Boss& boss, Entite& joueur);

    void afficherPvApresAttaque(const Entite& defenseur) const;
    void afficherResultatCombat(const Entite& joueur1, const Entite& joueur2) const;

public:
    void lancerPvpDeuxJoueurs(Joueur& joueur1);
    void lancerPvpIA(Joueur& joueur1);
    void lancerPveBoss(Joueur& joueur1);
};

#endif
