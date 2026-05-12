#include "combat/Combat.hpp"
#include "core/Console.hpp"
#include "classe/CatalogueClasses.hpp"
#include "boss/CatalogueBoss.hpp"
#include "entite/Joueur.hpp"

#include <iostream>

void Combat::lancerPvpDeuxJoueurs(Joueur& joueur1)
{
    std::cout << "À votre tour, Joueur 2." << std::endl;
    std::cout << "Quel est ton nom ?" << std::endl;
    std::cout << "> ";

    std::string nomJoueur2;
    std::cin >> nomJoueur2;

    Console::clear();

    std::cout << "Très bien, " << nomJoueur2 << "." << std::endl;
    std::cout << "Choisis ta classe et entre dans l'arène." << std::endl;
    std::cout << std::endl;

    CatalogueClasses::afficherClassesDeBase();

    std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
    std::cout << "> ";

    int choixClasse = Console::demanderNombreEntre(
        1,
        3,
        "Veuillez entrer un chiffre valide : 1, 2 ou 3."
    );

    Joueur joueur2(nomJoueur2, CatalogueClasses::creerClasseDeBase(choixClasse));

    joueur2.initialiserInventaireDeBase();

    Console::clear();

    std::cout << joueur2.getNom() << ", tes statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << "Le duel peut maintenant commencer." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    int tour = random.choisirPremierTour();

    std::cout << "Préparez-vous..." << std::endl;
    Console::pauseSecondes(2);
    std::cout << "Le combat commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!joueur1.estMort() && !joueur2.estMort())
    {
        bool tourTermine = false;

        if (tour == 1)
        {
            tourTermine = jouerTourHumain(joueur1, joueur2, SOIN_POTION, BONUS_POTION_DEGATS);

            if (tourTermine)
            {
                tour = 2;
            }
        }
        else
        {
            tourTermine = jouerTourHumain(joueur2, joueur1, SOIN_POTION, BONUS_POTION_DEGATS);

            if (tourTermine)
            {
                tour = 1;
            }
        }
    }

    afficherResultatCombat(joueur1, joueur2);
}

void Combat::lancerPvpIA(Joueur& joueur1)
{
    std::cout << "Préparation de l'IA..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << "Matt est entré dans l'arène." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    std::cout << joueur1.getNom() << ", choisis comment Matt obtiendra sa classe :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Classe aléatoire" << std::endl;
    std::cout << "2 : Choisir sa classe toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choixTypeClasse = Console::demanderNombreEntre(
        1,
        2,
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    int choixClasseIA;

    if (choixTypeClasse == 1)
    {
        choixClasseIA = random.entre(1, 3);
    }
    else
    {
        Console::clear();

        std::cout << "Choisis la classe que Matt devra utiliser." << std::endl;
        std::cout << "Pas très fair-play, mais l'arène accepte ce genre de petit caprice." << std::endl;
        std::cout << std::endl;

        CatalogueClasses::afficherClassesDeBase();

        std::cout << "Veuillez entrer uniquement le chiffre correspondant." << std::endl;
        std::cout << "> ";

        choixClasseIA = Console::demanderNombreEntre(
            1,
            3,
            "Veuillez entrer un chiffre valide : 1, 2 ou 3."
        );
    }

    Joueur ia("Matt", CatalogueClasses::creerClasseDeBase(choixClasseIA));

    ia.initialiserInventaireDeBase();

    Console::clear();

    std::cout << ia.getNom() << " a choisi la classe : " << ia.getType() << "." << std::endl;
    std::cout << "Ses statistiques ont été gravées dans l'arène." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    int tour = random.choisirPremierTour();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSecondes(2);
    std::cout << "Le combat contre " << ia.getNom() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!joueur1.estMort() && !ia.estMort())
    {
        bool tourTermine = false;

        if (tour == 1)
        {
            tourTermine = jouerTourHumain(joueur1, ia, SOIN_POTION, BONUS_POTION_DEGATS);

            if (tourTermine)
            {
                tour = 2;
            }
        }
        else
        {
            tourTermine = jouerTourIA(ia, joueur1, SOIN_POTION, BONUS_POTION_DEGATS);

            if (tourTermine)
            {
                tour = 1;
            }
        }
    }

    afficherResultatCombat(joueur1, ia);
}

void Combat::lancerPveBoss(Joueur& joueur1)
{
    std::cout << "Vous sentez une aura maléfique autour de vous." << std::endl;
    Console::pauseSecondes(2);

    std::cout << "Par chance, votre âme est encore pure." << std::endl;
    std::cout << "À mesure que vous avancez, votre force change, comme si l'arène refusait de vous laisser mourir trop vite." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    ClasseJoueur classeEvoluee = CatalogueClasses::creerClasseEvolueeDepuisClasse(joueur1.getType());
    joueur1.appliquerClasse(classeEvoluee);

    std::cout << joueur1.getNom() << ", ta classe évolue en : " << joueur1.getType() << "." << std::endl;
    std::cout << "Tes PV et tes objets ont été renforcés pour ce combat." << std::endl;
    std::cout << "Tes dégâts, eux, restent bloqués : même l'arène semble avoir ses limites." << std::endl;
    std::cout << std::endl;

    joueur1.afficherStats();

    Console::pauseSecondes(3);

    std::cout << joueur1.getNom() << ", choisis le type d'apparition du boss :" << std::endl;
    std::cout << std::endl;
    std::cout << "1 : Boss aléatoire" << std::endl;
    std::cout << "2 : Choisir le boss toi-même" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";

    int choixTypeBoss = Console::demanderNombreEntre(
        1,
        2,
        "Veuillez entrer un chiffre valide : 1 ou 2."
    );

    int choixBoss;

    if (choixTypeBoss == 1)
    {
        choixBoss = random.entre(1, 3);
    }
    else
    {
        Console::clear();

        std::cout << "Sélectionne l'entité que tu veux exterminer :" << std::endl;
        std::cout << std::endl;

        CatalogueBoss::afficherBossDisponibles();

        std::cout << "Leurs statistiques resteront inconnues pour le moment." << std::endl;
        std::cout << "Tu auras besoin d'un bon instinct... ou d'une chance insolente." << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        choixBoss = Console::demanderNombreEntre(
            1,
            3,
            "Veuillez entrer un chiffre valide : 1, 2 ou 3."
        );
    }

    Boss boss = CatalogueBoss::creerBoss(choixBoss);

    Console::clear();

    std::cout << "Préparation du boss..." << std::endl;
    Console::pauseSecondes(2);

    std::cout << boss.getNom() << " est entré dans l'arène." << std::endl;
    std::cout << boss.getNom() << " est de type : " << boss.getType() << "." << std::endl;
    std::cout << "Fais attention : ce combat pourrait vraiment te coûter cher." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(3);

    int tour = random.choisirPremierTour();

    std::cout << "Prépare-toi..." << std::endl;
    Console::pauseSecondes(2);
    std::cout << "Le combat contre " << boss.getNom() << " commence maintenant." << std::endl;
    std::cout << std::endl;

    while (!joueur1.estMort() && !boss.estMort())
    {
        bool tourTermine = false;

        if (tour == 1)
        {
            tourTermine = jouerTourHumain(joueur1, boss, SOIN_POTION_BOSS, BONUS_POTION_DEGATS_BOSS);

            if (tourTermine)
            {
                verifierDecryptageBoss(boss);
                boss.reduireDelaiUltime();
                tour = 2;
            }
        }
        else
        {
            tourTermine = jouerTourBoss(boss, joueur1);

            if (tourTermine)
            {
                tour = 1;
            }
        }
    }

    afficherResultatCombat(joueur1, boss);
}

void Combat::afficherMenuTour(const Entite& entite) const
{
    std::cout << "Tour de " << entite.getNom() << std::endl;
    std::cout << std::endl;

    std::cout << "Choisis une option parmi :" << std::endl;
    std::cout << "0 : Stats                 1 : Attaque" << std::endl;
    std::cout << "2 : Potion de soin        3 : Potion de dégâts" << std::endl;
    std::cout << "4 : Manuel de potions     5 : Passer son tour" << std::endl;
    std::cout << "6 : Inventaire            7 : Équiper une arme" << std::endl;
    std::cout << std::endl;
    std::cout << "> ";
}

void Combat::afficherManuelPotions(int soinPotion, int bonusPotionDegats) const
{
    std::cout << "Un vieux maître d'arène t'explique rapidement ton équipement :" << std::endl;
    std::cout << std::endl;
    std::cout << "- Les potions de soin rendent " << soinPotion << " PV, puis terminent ton tour." << std::endl;
    std::cout << "- Les potions de dégâts ajoutent " << bonusPotionDegats << " dégâts à ton attaque." << std::endl;
    std::cout << "- Une potion de dégâts est consommée même si ton attaque échoue." << std::endl;
    std::cout << std::endl;
}

bool Combat::jouerTourHumain(Entite& attaquant, Entite& defenseur, int soinPotion, int bonusPotionDegats)
{
    afficherMenuTour(attaquant);

    int option = Console::demanderNombreEntre(
        0,
        7,
        "Option invalide. Choisis un chiffre entre 0 et 7."
    );

    Console::clear();

    if (option == 0)
    {
        attaquant.afficherStats();
        return false;
    }

    if (option == 1)
    {
        executerAttaque(attaquant, defenseur);
        return true;
    }

    if (option == 2)
    {
        return executerPotionSoin(attaquant, soinPotion);
    }

    if (option == 3)
    {
        return executerPotionDegats(attaquant, defenseur, bonusPotionDegats);
    }

    if (option == 4)
    {
        afficherManuelPotions(soinPotion, bonusPotionDegats);
        return false;
    }

    if (option == 6)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur == nullptr)
        {
            std::cout << attaquant.getNom() << " n'a pas d'inventaire accessible." << std::endl;
            std::cout << std::endl;
            return false;
        }

        ouvrirInventaire(*joueur);
        return false;
    }   

    if (option == 7)
    {
        Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

        if (joueur == nullptr)
        {   
            std::cout << attaquant.getNom() << " ne peut pas équiper d'arme." << std::endl;
            std::cout << std::endl;
            return false;
        }

        return equiperArmeDepuisInventaire(*joueur);
    }

    std::cout << attaquant.getNom() << " hésite, oublie quoi faire, et met fin à son tour..." << std::endl;
    std::cout << std::endl;
    return true;
}

bool Combat::jouerTourIA(Entite& ia, Entite& defenseur, int soinPotion, int bonusPotionDegats)
{
    std::cout << "Tour de " << ia.getNom() << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    int option = choisirActionIA(ia);

    if (option == 0)
    {
        ia.afficherStats();
        return false;
    }

    if (option == 1)
    {
        executerAttaque(ia, defenseur);
        return true;
    }

    if (option == 2)
    {
        return executerPotionSoin(ia, soinPotion);
    }

    if (option == 3)
    {
        return executerPotionDegats(ia, defenseur, bonusPotionDegats);
    }

    std::cout << ia.getNom() << " bug mentalement, fixe le vide, et passe son tour." << std::endl;
    std::cout << std::endl;
    return true;
}

bool Combat::jouerTourBoss(Boss& boss, Entite& joueur)
{
    std::cout << "Tour de " << boss.getNom() << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    int option = choisirActionBoss(boss);

    if (option == 0)
    {
        boss.afficherStats();
        return false;
    }

    if (option == 1)
    {
        executerAttaque(boss, joueur);
        return gererFinTourBoss(boss, joueur);
    }

    if (option == 2)
    {
        if (boss.utiliserPotionSoin(boss.getPvMax() * 10 / 100))
        {
            std::cout << boss.getNom() << " récupère une partie de sa vitalité." << std::endl;
            std::cout << std::endl;
        }

        return gererFinTourBoss(boss, joueur);
    }

    if (option == 3)
    {
        executerPotionDegats(boss, joueur, 50);
        return gererFinTourBoss(boss, joueur);
    }

    if (option == 4)
    {
        executerUltimeBoss(boss, joueur);
        return gererFinTourBoss(boss, joueur);
    }

    std::cout << boss.getNom() << " reste immobile, comme s'il observait déjà ta fin." << std::endl;
    std::cout << std::endl;

    return gererFinTourBoss(boss, joueur);
}

int Combat::choisirActionIA(const Entite& ia)
{
    if (ia.getPv() <= 60 && ia.getPotionsSoin() > 0)
    {
        return 2;
    }

    int option = random.entre(0, 5);

    if (option == 2 || option == 4)
    {
        option = 1;
    }

    if (option == 3 && ia.getPotionsDegats() <= 0)
    {
        option = 1;
    }

    if (option == 5)
    {
        option = random.entre(0, 5);

        if (option == 2 || option == 4)
        {
            option = 1;
        }
    }

    return option;
}

int Combat::choisirActionBoss(const Boss& boss)
{
    if (boss.getPv() <= (boss.getPvMax() / 4) && boss.getPotionsSoin() > 0)
    {
        return 2;
    }

    if (boss.peutUtiliserUltime())
    {
        return 4;
    }

    int option = random.entre(0, 5);

    if (option == 2 || option == 4)
    {
        option = 1;
    }

    if (option == 3 && boss.getPotionsDegats() <= 0)
    {
        option = 1;
    }

    return option;
}

void Combat::executerAttaque(Entite& attaquant, Entite& defenseur)
{
    bool esquive = false;
    bool critique = false;

    int degats = attaquant.attaquer(random, esquive, critique);

    if (esquive)
    {
        std::cout << attaquant.getNom() << " attaque, mais " << defenseur.getNom()
                  << " esquive au dernier moment." << std::endl;
        std::cout << std::endl;
        return;
    }

    if (atlasBloqueAttaque(attaquant, defenseur, degats))
    {
        return;
    }

    defenseur.recevoirDegats(degats);
    appliquerVolDeVieDemonSiBesoin(attaquant, degats);

    if (critique)
    {
        std::cout << attaquant.getNom() << " frappe avec une violence monstrueuse et inflige "
                  << degats << " dégâts critiques." << std::endl;
    }
    else
    {
        std::cout << attaquant.getNom() << " attaque et inflige "
                  << degats << " dégâts." << std::endl;
    }

    afficherPvApresAttaque(defenseur);
}

bool Combat::executerPotionSoin(Entite& entite, int soinPotion)
{
    Joueur* joueur = dynamic_cast<Joueur*>(&entite);

    if (joueur != nullptr)
    {
        Consommable potion;

        if (!joueur->getInventaire().utiliserPremierConsommable(TypeConsommable::Soin, potion))
        {
            std::cout << joueur->getNom() << " fouille son inventaire..." << std::endl;
            std::cout << "Mais aucune potion de soin n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;
            return false;
        }

        joueur->soigner(potion.getPuissance());

        std::cout << joueur->getNom() << " utilise : " << potion.getNom() << "." << std::endl;
        std::cout << "Ses blessures se referment, et il récupère "
                  << potion.getPuissance() << " PV." << std::endl;
        std::cout << joueur->getNom() << " possède maintenant "
                  << joueur->getPv() << "/" << joueur->getPvMax() << " PV." << std::endl;
        std::cout << std::endl;

        return true;
    }

    if (entite.utiliserPotionSoin(soinPotion))
    {
        std::cout << entite.getNom() << " utilise une potion de soin." << std::endl;
        std::cout << "Sa vitalité revient lentement." << std::endl;
        std::cout << std::endl;
        return true;
    }

    std::cout << entite.getNom() << " n'a plus aucune potion de soin." << std::endl;
    std::cout << std::endl;

    return false;
}

bool Combat::executerPotionDegats(Entite& attaquant, Entite& defenseur, int bonusPotionDegats)
{
    int bonusUtilise = bonusPotionDegats;

    Joueur* joueur = dynamic_cast<Joueur*>(&attaquant);

    if (joueur != nullptr)
    {
        Consommable potion;

        if (!joueur->getInventaire().utiliserPremierConsommable(TypeConsommable::Degats, potion))
        {
            std::cout << joueur->getNom() << " cherche une potion de rage dans son inventaire..." << std::endl;
            std::cout << "Mais aucune potion de dégâts n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bonusUtilise = potion.getPuissance();

        std::cout << joueur->getNom() << " utilise : " << potion.getNom() << "." << std::endl;
    }
    else
    {
        if (!attaquant.consommerPotionDegats())
        {
            std::cout << attaquant.getNom() << " cherche une potion de dégâts, mais sa rage est déjà épuisée." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;
            return false;
        }
    }

    std::cout << attaquant.getNom() << " sent ses forces monter d'un coup." << std::endl;
    std::cout << "Une rage brutale s'empare de lui..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(1);

    bool esquive = false;
    bool critique = false;

    int degats = attaquant.attaquer(random, esquive, critique, bonusUtilise);

    if (esquive)
    {
        std::cout << attaquant.getNom() << " attaque, mais sa puissance le paralyse un court instant." << std::endl;
        std::cout << defenseur.getNom() << " évite l'assaut sans subir de dégâts." << std::endl;
        std::cout << std::endl;
        return true;
    }

    if (atlasBloqueAttaque(attaquant, defenseur, degats))
    {
        return true;
    }

    defenseur.recevoirDegats(degats);
    appliquerVolDeVieDemonSiBesoin(attaquant, degats);

    if (critique)
    {
        std::cout << "La rage de " << attaquant.getNom() << " explose dans l'arène." << std::endl;
        std::cout << "Il inflige " << degats << " dégâts monstrueux." << std::endl;
    }
    else
    {
        std::cout << attaquant.getNom() << " attaque avec une puissance dévastatrice et inflige "
                  << degats << " dégâts." << std::endl;
    }

    afficherPvApresAttaque(defenseur);
    return true;
}

bool Combat::ouvrirInventaire(Joueur& joueur)
{
    joueur.afficherInventaire();
    return false;
}

bool Combat::equiperArmeDepuisInventaire(Joueur& joueur)
{
    if (joueur.getInventaire().getNombreArmes() <= 0)
    {
        std::cout << joueur.getNom() << " n'a aucune arme à équiper." << std::endl;
        std::cout << std::endl;
        return false;
    }

    joueur.getInventaire().afficherArmes();

    std::cout << "Choisis l'arme à équiper." << std::endl;
    std::cout << "Entre son numéro, ou -1 pour annuler." << std::endl;
    std::cout << "> ";

    int choix;
    std::cin >> choix;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');

        std::cout << "Choix invalide. L'équipement est annulé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (choix == -1)
    {
        std::cout << "Changement d'arme annulé." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (!joueur.equiperArme(choix))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Arme armeEquipee = joueur.getArmeEquipee();

    std::cout << joueur.getNom() << " équipe : " << armeEquipee.getNom() << "." << std::endl;

    if (armeEquipee.estCassee())
    {
        std::cout << "Attention : cette arme est cassée, elle ne donnera aucun bonus." << std::endl;
    }

    std::cout << std::endl;

    return false;
}

bool Combat::atlasBloqueAttaque(Entite& attaquant, Entite& defenseur, int degats)
{
    Boss* bossDefenseur = dynamic_cast<Boss*>(&defenseur);

    if (bossDefenseur == nullptr)
    {
        return false;
    }

    if (bossDefenseur->getIdBoss() != 3 || !bossDefenseur->ultimeActif())
    {
        return false;
    }

    int degatsRenvoi = degats / 3;

    attaquant.recevoirDegats(degatsRenvoi);

    std::cout << attaquant.getNom() << " frappe de toutes ses forces..." << std::endl;
    Console::pauseSecondes(1);

    std::cout << "Mais l'armure d'" << bossDefenseur->getNom() << " absorbe l'impact." << std::endl;
    std::cout << "Une partie de la puissance est renvoyée à " << attaquant.getNom()
              << ", qui subit " << degatsRenvoi << " dégâts." << std::endl;
    std::cout << std::endl;

    std::cout << attaquant.getNom() << " possède maintenant "
              << attaquant.getPv() << "/" << attaquant.getPvMax() << " PV." << std::endl;
    std::cout << std::endl;

    return true;
}

void Combat::appliquerVolDeVieDemonSiBesoin(Entite& attaquant, int degatsInfliges)
{
    Boss* bossAttaquant = dynamic_cast<Boss*>(&attaquant);

    if (bossAttaquant == nullptr)
    {
        return;
    }

    if (bossAttaquant->getIdBoss() != 2 || !bossAttaquant->ultimeActif())
    {
        return;
    }

    if (bossAttaquant->getEffetSpecial() != 2 && bossAttaquant->getEffetSpecial() != 3)
    {
        return;
    }

    int soin = degatsInfliges * 50 / 100;

    if (soin <= 0)
    {
        return;
    }

    bossAttaquant->soigner(soin);

    std::cout << bossAttaquant->getNom() << " absorbe le sang de l'attaque et récupère "
              << soin << " PV." << std::endl;
    std::cout << std::endl;
}

void Combat::verifierDecryptageBoss(Boss& boss)
{
    if (boss.doitDecrypterStats())
    {
        std::cout << "Très bien humain..." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Je vois que je t'ai sous-estimé." << std::endl;
        std::cout << "Je me nomme " << boss.getNom() << ", et j'appartiens à la classe : " << boss.getType() << "." << std::endl;
        std::cout << std::endl;

        Console::pauseSecondes(3);

        std::cout << "Pour récompenser tes efforts, je t'autorise à accéder à mes statistiques." << std::endl;
        std::cout << "Décryptage en cours..." << std::endl;
        std::cout << std::endl;

        Console::pauseSecondes(3);

        boss.decrypterStats();

        std::cout << "Décryptage terminé." << std::endl;
        std::cout << std::endl;

        boss.afficherStats();

        std::cout << "Maintenant que tu sais tout ça, je ne vais plus être si clément." << std::endl;
        std::cout << std::endl;
    }
}

void Combat::executerUltimeBoss(Boss& boss, Entite& joueur)
{
    boss.activerUltime();

    if (boss.getIdBoss() == 1)
    {
        std::cout << boss.getNom() << " déploie de grandes ailes dans son dos." << std::endl;
        std::cout << "Des chaînes de lumière s'emparent de ton corps et t'immobilisent." << std::endl;
        std::cout << "Tant que ces chaînes existeront, l'arène refusera de te rendre ton tour." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getIdBoss() == 2)
    {
        int effet = random.entre(1, 3);
        boss.setEffetSpecial(effet);

        if (effet == 1)
        {
            std::cout << boss.getNom() << " libère une aura sombre qui dévore l'air autour de toi." << std::endl;
            std::cout << "L'effet Corrosion t'est appliqué." << std::endl;
            std::cout << "Tes PV maximum vont lentement diminuer, et tu subiras des dégâts chaque tour." << std::endl;
        }
        else if (effet == 2)
        {
            std::cout << "L'arme de " << boss.getNom() << " se teinte d'une couleur rouge sang." << std::endl;
            std::cout << "L'effet Saignement t'est appliqué." << std::endl;
            std::cout << boss.getNom() << " récupérera une partie des dégâts qu'il t'inflige." << std::endl;
        }
        else
        {
            std::cout << boss.getNom() << " libère une aura sombre, tandis que son arme devient rouge sang." << std::endl;
            std::cout << "Les effets Corrosion et Saignement te sont appliqués en même temps." << std::endl;
            std::cout << "Tes PV maximum diminuent, et chaque attaque réussie le régénère." << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << boss.getNom() << " se met en position de défense." << std::endl;
        std::cout << "Son armure change de couleur et de matière." << std::endl;
        std::cout << "Sa résistance semble désormais presque impénétrable." << std::endl;
        std::cout << std::endl;
    }

    joueur.recevoirDegats(0);
}

bool Combat::gererFinTourBoss(Boss& boss, Entite& joueur)
{
    if (boss.ultimeActif())
    {
        if (boss.getIdBoss() == 1)
        {
            boss.soigner(boss.getPvMax() * 5 / 100);

            std::cout << boss.getNom() << " se régénère grâce à des esprits lumineux." << std::endl;
            std::cout << "Les chaînes brillent encore autour de " << joueur.getNom() << "." << std::endl;
            std::cout << std::endl;
        }
        else if (boss.getIdBoss() == 2)
        {
            if (boss.getEffetSpecial() == 1 || boss.getEffetSpecial() == 3)
            {
                int reductionPvMax = boss.getPvMax() * 2 / 100;

                joueur.recevoirDegats(10);
                joueur.reduirePvMax(reductionPvMax);

                std::cout << joueur.getNom() << " subit les dégâts de Corrosion." << std::endl;
                std::cout << "Ses PV maximum diminuent de " << reductionPvMax << "." << std::endl;
                std::cout << joueur.getNom() << " possède maintenant "
                          << joueur.getPv() << "/" << joueur.getPvMax() << " PV." << std::endl;
                std::cout << std::endl;
            }
        }

        boss.reduireUltime();

        if (!boss.ultimeActif())
        {
            if (boss.getIdBoss() == 1)
            {
                std::cout << "Les ailes de " << boss.getNom() << " se rétractent." << std::endl;
                std::cout << "Les chaînes de lumière disparaissent enfin." << std::endl;
            }
            else if (boss.getIdBoss() == 2)
            {
                std::cout << "L'aura sombre de " << boss.getNom() << " s'affaiblit." << std::endl;
                std::cout << "Les miasmes quittent lentement l'arène." << std::endl;
            }
            else
            {
                std::cout << "L'armure d'" << boss.getNom() << " perd son éclat anormal." << std::endl;
                std::cout << "Ses matériaux semblent revenir à leur état initial." << std::endl;
            }

            std::cout << std::endl;

            boss.reinitialiserDelaiUltime();
            return true;
        }

        if (boss.getIdBoss() == 1)
        {
            std::cout << joueur.getNom() << " tente de bouger, mais les chaînes le maintiennent au sol." << std::endl;
            std::cout << boss.getNom() << " conserve son tour." << std::endl;
            std::cout << std::endl;

            return false;
        }
    }

    return true;
}

void Combat::afficherPvApresAttaque(const Entite& defenseur) const
{
    if (defenseur.statsVisibles())
    {
        std::cout << defenseur.getNom() << " possède maintenant "
                  << defenseur.getPv() << "/" << defenseur.getPvMax() << " PV." << std::endl;
    }
    else
    {
        std::cout << "Les PV de " << defenseur.getNom() << " restent impossibles à lire." << std::endl;
    }

    std::cout << std::endl;
}

void Combat::afficherResultatCombat(const Entite& joueur1, const Entite& joueur2) const
{
    Console::pauseSecondes(2);

    std::cout << std::endl;

    if (joueur1.estMort())
    {
        std::cout << joueur1.getNom() << " a été mis à terre..." << std::endl;
        std::cout << joueur2.getNom() << " remporte donc la partie, ainsi que le respect de l'arène." << std::endl;
    }
    else
    {
        std::cout << joueur2.getNom() << " a été mis à terre..." << std::endl;
        std::cout << joueur1.getNom() << " remporte donc la partie, ainsi que le respect de l'arène." << std::endl;
    }

    std::cout << std::endl;
}