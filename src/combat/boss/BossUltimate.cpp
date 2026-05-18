// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossUltimate.hpp"

#include <iostream>

void BossUltimate::executeBossUltimate(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    boss.activateUltimate();

    if (boss.getBossId() == 1)
    {
        std::cout << boss.getName() << " déploie de grandes ailes dans son dos." << std::endl;
        std::cout << "Des chaînes de lumière s'emparent de ton corps et t'immobilisent." << std::endl;
        std::cout << "Tant que ces chaînes existeront, l'arène refusera de te rendre ton tour." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 2)
    {
        int effet = random.between(1, 3);
        boss.setSpecialEffect(effet);

        if (effet == 1)
        {
            std::cout << boss.getName()
                      << " libère une aura sombre qui dévore l'air autour de toi."
                      << std::endl;
            std::cout << "L'effet Corrosion t'est appliqué." << std::endl;
            std::cout << "Tes PV maximum vont lentement diminuer, et tu subiras des dégâts chaque tour." << std::endl;
        }
        else if (effet == 2)
        {
            std::cout << "L'arme de " << boss.getName()
                      << " se teinte d'une couleur rouge sang."
                      << std::endl;
            std::cout << "L'effet Saignement t'est appliqué." << std::endl;
            std::cout << boss.getName()
                      << " récupérera une partie des dégâts qu'il t'inflige."
                      << std::endl;
        }
        else
        {
            std::cout << boss.getName()
                      << " libère une aura sombre, tandis que son arme devient rouge sang."
                      << std::endl;
            std::cout << "Les effets Corrosion et Saignement te sont appliqués en même temps." << std::endl;
            std::cout << "Tes PV maximum diminuent, et chaque attaque réussie le régénère." << std::endl;
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << boss.getName() << " se met en position de défense." << std::endl;
        std::cout << "Son armure change de couleur et de matière." << std::endl;
        std::cout << "Sa résistance semble désormais presque impénétrable." << std::endl;
        std::cout << std::endl;
    }

    player.takeDamage(0);
}