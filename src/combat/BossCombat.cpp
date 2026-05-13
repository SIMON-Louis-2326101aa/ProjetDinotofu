#include "combat/BossCombat.hpp"

#include "core/Console.hpp"

#include <iostream>

void BossCombat::verifierDecryptageBoss(Boss& boss)
{
    if (!boss.doitDecrypterStats())
    {
        return;
    }

    std::cout << std::endl;

    if (boss.getIdBoss() == 1)
    {
        std::cout << "Très bien, humain..." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Je vois que je t'ai sous-estimé." << std::endl;
        std::cout << "La lumière qui me protège commence à se fissurer." << std::endl;
        std::cout << "Alors regarde bien... mais ne crois pas que cela suffira à me vaincre." << std::endl;
    }
    else if (boss.getIdBoss() == 2)
    {
        std::cout << "Hahaha..." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Tu as réussi à m'arracher assez de sang pour voir derrière le voile." << std::endl;
        std::cout << "Mais plus tu comprends ce que je suis, plus tu t'approches de la peur." << std::endl;
        std::cout << "Allez, humain. Lis mes statistiques... et désespère." << std::endl;
    }
    else if (boss.getIdBoss() == 3)
    {
        std::cout << "Analyse défensive compromise." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Le protocole de dissimulation perd en stabilité." << std::endl;
        std::cout << "Tu as atteint un seuil que peu d'êtres auraient dû franchir." << std::endl;
        std::cout << "Mes données deviennent visibles... mais mon armure, elle, tient encore." << std::endl;
    }
    else
    {
        std::cout << "Le voile autour de " << boss.getNom() << " se déchire." << std::endl;
        Console::pauseSecondes(2);

        std::cout << "Ses statistiques deviennent enfin lisibles." << std::endl;
    }

    std::cout << std::endl;

    Console::pauseSecondes(2);

    std::cout << "Décryptage en cours..." << std::endl;
    std::cout << std::endl;

    Console::pauseSecondes(2);

    boss.decrypterStats();

    std::cout << "Décryptage terminé." << std::endl;
    std::cout << std::endl;

    boss.afficherStats();

    std::cout << "Le combat vient de changer de rythme." << std::endl;
    std::cout << std::endl;
}

void BossCombat::executerUltimeBoss(
    Boss& boss,
    Entite& joueur,
    Random& random
)
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
            std::cout << boss.getNom()
                      << " libère une aura sombre qui dévore l'air autour de toi."
                      << std::endl;
            std::cout << "L'effet Corrosion t'est appliqué." << std::endl;
            std::cout << "Tes PV maximum vont lentement diminuer, et tu subiras des dégâts chaque tour." << std::endl;
        }
        else if (effet == 2)
        {
            std::cout << "L'arme de " << boss.getNom()
                      << " se teinte d'une couleur rouge sang."
                      << std::endl;
            std::cout << "L'effet Saignement t'est appliqué." << std::endl;
            std::cout << boss.getNom()
                      << " récupérera une partie des dégâts qu'il t'inflige."
                      << std::endl;
        }
        else
        {
            std::cout << boss.getNom()
                      << " libère une aura sombre, tandis que son arme devient rouge sang."
                      << std::endl;
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

bool BossCombat::gererFinTourBoss(
    Boss& boss,
    Entite& joueur
)
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
                          << joueur.getPv() << "/" << joueur.getPvMax() << " PV."
                          << std::endl;
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
            std::cout << joueur.getNom()
                      << " tente de bouger, mais les chaînes le maintiennent au sol."
                      << std::endl;
            std::cout << boss.getNom() << " conserve son tour." << std::endl;
            std::cout << std::endl;

            return false;
        }
    }

    return true;
}