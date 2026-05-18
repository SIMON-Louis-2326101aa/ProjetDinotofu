#include "combat/boss/BossUltime.hpp"

#include <iostream>

void BossUltime::executerUltimeBoss(
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