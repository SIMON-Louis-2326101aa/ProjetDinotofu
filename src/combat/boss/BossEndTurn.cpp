// EN: BossEndTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossEndTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossEndTurn.hpp"

#include <iostream>

namespace
{
    // EN: dealOngoingDamage declares or implements a focused behavior used by this module.
    // FR: dealOngoingDamage déclare ou implémente un comportement précis utilisé par ce module.
    void dealOngoingDamage(Entity& player, int damage)
    {
        if (damage <= 0)
        {
            return;
        }

        player.takeDamage(damage);
        std::cout << player.getName() << " subit " << damage << " dégâts supplémentaires." << std::endl;
        std::cout << player.getName() << " possède maintenant "
                  << player.getHp() << "/" << player.getMaxHp() << " PV." << std::endl;
        std::cout << std::endl;
    }
}

bool BossEndTurn::handleBossEndTurn(
    Boss& boss,
    Entity& player
)
{
    if (boss.getBossId() == 6 && !boss.isUltimateActive())
    {
        int darkness = boss.getSpecialEffect() + 1;
        boss.setSpecialEffect(darkness);

        if (darkness % 3 == 0)
        {
            std::cout << "L'obscurité autour de l'avatar épaissit encore." << std::endl;
            std::cout << "Charges d'obscurité : " << darkness << std::endl;
            std::cout << std::endl;
        }
    }

    if (boss.isUltimateActive())
    {
        if (boss.getBossId() == 1)
        {
            int judgment = boss.getSpecialEffect();
            boss.heal(boss.getMaxHp() * (3 + judgment) / 100);

            std::cout << boss.getName() << " se régénère grâce au verdict lumineux." << std::endl;
            std::cout << "Les chaînes brillent encore autour de " << player.getName() << "." << std::endl;
            std::cout << std::endl;
        }
        else if (boss.getBossId() == 2)
        {
            if (boss.getSpecialEffect() == 1 || boss.getSpecialEffect() == 3)
            {
                int maxHpReduction = 4 + boss.getSpecialEffect() * 2;

                player.takeDamage(10 + boss.getSpecialEffect() * 2);
                player.reduceMaxHp(maxHpReduction);
                boss.heal(4 + boss.getSpecialEffect() * 3);

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
        else if (boss.getBossId() == 3)
        {
            int stored = boss.getSpecialEffect();
            std::cout << "Atlas transforme les coups encaissés en onde de rempart." << std::endl;
            dealOngoingDamage(player, 5 + stored * 5);
        }
        else if (boss.getBossId() == 4)
        {
            std::cout << "Des silhouettes de loups traversent la nuit close." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 4);
        }
        else if (boss.getBossId() == 5)
        {
            int debt = boss.getSpecialEffect() + 4;
            boss.setSpecialEffect(debt);
            std::cout << "Les collecteurs gobelins griffonnent de nouveaux frais sur le contrat." << std::endl;
            std::cout << "Dette gobeline actuelle : " << debt << std::endl;
            std::cout << std::endl;
        }
        else if (boss.getBossId() == 6)
        {
            std::cout << "La couronne fissurée pulse dans l'obscurité." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 7)
        {
            std::cout << "La brûlure draconique du souffle ancien résonne encore." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 8)
        {
            std::cout << "Les racines de Mojo resserrent brièvement le sol." << std::endl;
            dealOngoingDamage(player, 4 + boss.getSpecialEffect() * 3);
        }
        else if (boss.getBossId() == 9)
        {
            std::cout << "Un reflet retardataire d'Inakari te frappe depuis un angle impossible." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2);
        }


        else if (boss.getBossId() == 10)
        {
            std::cout << "Ton ombre reste clouée une seconde de trop au sol." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 11)
        {
            std::cout << "L'interface clignote. Une option que tu n'as pas choisie te regarde." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 12)
        {
            std::cout << "Une seconde morte se détache du cadran et te traverse." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect());
        }
        else if (boss.getBossId() == 13)
        {
            if (boss.getSpecialEffect() >= 100)
            {
                std::cout << "Le golem d'os et de sable martèle encore le sol." << std::endl;
                dealOngoingDamage(player, 9 + (boss.getSpecialEffect() - 100) * 3);
            }
            else
            {
                std::cout << "La berceuse des enterrés continue sous la poussière." << std::endl;
                dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2);
            }
        }
        else if (boss.getBossId() == 14)
        {
            std::cout << "Le champ de bataille imaginaire continue de rugir autour de Boros." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 15)
        {
            std::cout << "Un lien douloureux serre encore ton coeur." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 16)
        {
            std::cout << "Le verdict de Lexior continue de peser sur ton équipement." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 17)
        {
            std::cout << "Un cauchemar lunaire revient en écho derrière tes pensées." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 18)
        {
            std::cout << "Les quatre éléments vibrent encore en désaccord autour de toi." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 19)
        {
            std::cout << "Hitogami applique ce qu'il vient d'apprendre de toi." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 20)
        {
            std::cout << "Les probabilités restent inversées autour de Sérendys." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 21)
        {
            std::cout << "Le Seuil maintient sa pression jusqu'à ta prochaine réponse." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 22)
        {
            std::cout << "La cour invisible du Roi Sans Salle répète un ordre que personne n'aurait dû entendre." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 23)
        {
            std::cout << "La Bête change encore de silhouette au bord de ton regard." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 24)
        {
            std::cout << "L'abîme d'Aldebaroth continue de salir l'air." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 28)
        {
            std::cout << "Le Souffle sans Visage retient encore une partie de l'air." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 29)
        {
            std::cout << "Les fils de la Marionnette grincent autour de tes articulations." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 30)
        {
            std::cout << "Moiran laisse un futur possible saigner dans le présent." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 31)
        {
            std::cout << "Les lanternes du Cerf restent allumées autour de toi." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }

        else if (boss.getBossId() == 32)
        {
            std::cout << "La fureur royale de Gorvald continue de faire vibrer le sol." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 33)
        {
            std::cout << "Le banquet rouge n'est pas terminé : ton sang répond encore à Serana." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
            boss.heal(4 + boss.getSpecialEffect());
        }
        else if (boss.getBossId() == 34)
        {
            std::cout << "La soie noire resserre la pièce autour de toi." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2);
        }
        else if (boss.getBossId() == 35)
        {
            std::cout << "Un reflet de futur te frappe avec une seconde de retard." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2);
        }

        boss.reduceUltimate();
        if (boss.isUltimateActive())
        {
            std::cout << "Effet ultime encore actif : " << boss.getRemainingUltimateTurns()
                      << " tour(s) de boss restant(s)." << std::endl;
            std::cout << std::endl;
        }

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
                boss.setSpecialEffect(0);
            }
            else if (boss.getBossId() == 3)
            {
                std::cout << "L'armure d'Atlas s'ouvre à nouveau." << std::endl;
                std::cout << "Le dernier rempart retombe, mais ses fissures restent dangereuses." << std::endl;
            }
            else if (boss.getBossId() == 4)
            {
                std::cout << "La nuit de Lyknir recule, mais la meute garde ton odeur." << std::endl;
                boss.setSpecialEffect(1);
            }
            else if (boss.getBossId() == 5)
            {
                std::cout << "Le contrat royal se replie. La dette restante demeure inscrite en petit." << std::endl;
            }
            else if (boss.getBossId() == 6)
            {
                std::cout << "Le trône d'ombre s'effondre, mais la couronne continue de respirer." << std::endl;
            }
            else if (boss.getBossId() == 7)
            {
                std::cout << "Le souffle de Thamarys se dissipe, mais ses écailles ont appris." << std::endl;
            }
            else if (boss.getBossId() == 8)
            {
                std::cout << "La forêt relâche son étreinte. Mojo continue d'observer." << std::endl;
            }
            else if (boss.getBossId() == 9)
            {
                std::cout << "Les miroirs d'Inakari se fendent, sans disparaître complètement." << std::endl;
            }

            else if (boss.getBossId() == 10)
            {
                std::cout << "Le Jugement Silencieux referme sa main. Ton ombre retombe à sa place." << std::endl;
            }
            else if (boss.getBossId() == 11)
            {
                std::cout << "L'interface redevient lisible, mais tu sais qu'elle peut mentir." << std::endl;
            }
            else if (boss.getBossId() == 12)
            {
                std::cout << "L'aiguille morte reprend sa place. Certaines blessures restent en attente." << std::endl;
            }
            else if (boss.getBossId() == 13)
            {
                std::cout << "Les tombes se taisent, mais le sable continue de respirer." << std::endl;
            }
            else if (boss.getBossId() == 14)
            {
                std::cout << "L'hymne guerrier retombe en un grondement sourd." << std::endl;
            }
            else if (boss.getBossId() == 15)
            {
                std::cout << "Les fils invisibles se desserrent sans disparaître complètement." << std::endl;
            }
            else if (boss.getBossId() == 16)
            {
                std::cout << "Le procès de Lexior se referme, mais le verdict reste dans ta mémoire." << std::endl;
            }
            else if (boss.getBossId() == 17)
            {
                std::cout << "La lune se cache. Le rêve cesse de mordre." << std::endl;
            }
            else if (boss.getBossId() == 18)
            {
                std::cout << "Les quatre formes reviennent dans un seul fragment élémentaire." << std::endl;
            }
            else if (boss.getBossId() == 19)
            {
                std::cout << "Hitogami relâche la copie de ton rythme, mais il n'oublie pas." << std::endl;
            }
            else if (boss.getBossId() == 20)
            {
                std::cout << "Les probabilités cessent de mentir aussi fort." << std::endl;
            }
            else if (boss.getBossId() == 21)
            {
                std::cout << "Le Seuil se rouvre à moitié. L'épreuve n'est pas terminée." << std::endl;
            }
            else if (boss.getBossId() == 22)
            {
                std::cout << "Les murs invisibles du royaume tombent, mais le Roi cherche déjà une autre salle." << std::endl;
            }
            else if (boss.getBossId() == 23)
            {
                std::cout << "Le hurlement s'éteint. Le registre hésite encore à nommer ce qu'il vient de voir." << std::endl;
            }
            else if (boss.getBossId() == 24)
            {
                std::cout << "L'abîme d'Aldebaroth recule, mais une rancune reste dans l'air." << std::endl;
            }
            else if (boss.getBossId() == 25)
            {
                std::cout << "La création et la destruction cessent de se mordre pendant un instant." << std::endl;
            }
            else if (boss.getBossId() == 26)
            {
                std::cout << "L'étage imposé par Obérion se retire lentement." << std::endl;
            }
            else if (boss.getBossId() == 27)
            {
                std::cout << "Le patch temporaire expire. FireFlight sourit comme si le vrai test commençait seulement." << std::endl;
            }
            else if (boss.getBossId() == 28)
            {
                std::cout << "Le monde reprend son souffle, mais pas complètement." << std::endl;
            }
            else if (boss.getBossId() == 29)
            {
                std::cout << "Quelques fils cassent. D'autres attendent déjà de se retendre." << std::endl;
            }
            else if (boss.getBossId() == 30)
            {
                std::cout << "Le fil du Destin cesse de vibrer, sans jamais disparaître." << std::endl;
            }
            else if (boss.getBossId() == 31)
            {
                std::cout << "Les âmes égarées baissent leurs lanternes." << std::endl;
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
