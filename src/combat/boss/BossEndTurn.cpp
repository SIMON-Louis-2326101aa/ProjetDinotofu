// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossEndTurn.hpp"

#include <iostream>

bool BossEndTurn::handleBossEndTurn(
    Boss& boss,
    Entity& player
)
{
    if (boss.isUltimateActive())
    {
        if (boss.getBossId() == 1)
        {
            boss.heal(boss.getMaxHp() * 5 / 100);

            std::cout << boss.getName() << " se régénère grâce à des esprits lumineux." << std::endl;
            std::cout << "Les chaînes brillent encore autour de " << player.getName() << "." << std::endl;
            std::cout << std::endl;
        }
        else if (boss.getBossId() == 2)
        {
            if (boss.getSpecialEffect() == 1 || boss.getSpecialEffect() == 3)
            {
                int maxHpReduction = boss.getMaxHp() * 2 / 100;

                player.takeDamage(10);
                player.reduceMaxHp(maxHpReduction);

                std::cout << player.getName() << " subit les dégâts de Corrosion." << std::endl;
                std::cout << "Ses PV maximum diminuent de " << maxHpReduction << "." << std::endl;
                std::cout << player.getName() << " possède maintenant "
                          << player.getHp()
                          << "/"
                          << player.getMaxHp()
                          << " PV."
                          << std::endl;
                std::cout << std::endl;
            }
        }

        boss.reduceUltimate();

        if (!boss.isUltimateActive())
        {
            if (boss.getBossId() == 1)
            {
                std::cout << "Les ailes de " << boss.getName() << " se rétractent." << std::endl;
                std::cout << "Les chaînes de lumière disparaissent enfin." << std::endl;
            }
            else if (boss.getBossId() == 2)
            {
                std::cout << "L'aura sombre de " << boss.getName() << " s'affaiblit." << std::endl;
                std::cout << "Les miasmes quittent lentement l'arène." << std::endl;
            }
            else
            {
                std::cout << "L'armure d'" << boss.getName() << " perd son éclat anormal." << std::endl;
                std::cout << "Ses matériaux semblent revenir à leur état initial." << std::endl;
            }

            std::cout << std::endl;

            boss.resetUltimateCooldown();
            return true;
        }

        if (boss.getBossId() == 1)
        {
            std::cout << player.getName()
                      << " tente de bouger, mais les chaînes le maintiennent au sol."
                      << std::endl;
            std::cout << boss.getName() << " conserve son tour." << std::endl;
            std::cout << std::endl;

            return false;
        }
    }

    return true;
}