#include "combat/boss/BossFinTour.hpp"

#include <iostream>

bool BossFinTour::gererFinTourBoss(
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
                          << joueur.getPv()
                          << "/"
                          << joueur.getPvMax()
                          << " PV."
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