// EN: BossEndTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossEndTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossEndTurn.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

    std::string trimBossEndTurnLine(std::string line)
    {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
        {
            line.pop_back();
        }

        return line;
    }

    class ScopedBossEndTurnNarration
    {
    private:
        std::ostringstream current;
        std::vector<std::string> lines;
        int bossId = 0;
        std::string bossName;

        void flushLine()
        {
            std::string line = trimBossEndTurnLine(current.str());
            if (!line.empty())
            {
                lines.push_back(line);
            }
            current.str(std::string());
            current.clear();
        }

    public:
        explicit ScopedBossEndTurnNarration(const Boss& boss)
            : bossId(boss.getBossId()),
              bossName(boss.getName())
        {
        }

        ~ScopedBossEndTurnNarration()
        {
            flushLine();
            if (!lines.empty())
            {
                MessageScreen::show(
                    "FIN DE TOUR - " + bossName,
                    "combat.boss.end_turn." + std::to_string(bossId),
                    lines,
                    false
                );
            }
        }

        template <typename T>
        ScopedBossEndTurnNarration& operator<<(const T& value)
        {
            current << value;
            return *this;
        }

        using StreamManipulator = std::ostream& (*)(std::ostream&);

        ScopedBossEndTurnNarration& operator<<(StreamManipulator manipulator)
        {
            if (manipulator == static_cast<StreamManipulator>(std::endl<char, std::char_traits<char>>))
            {
                flushLine();
            }
            else
            {
                manipulator(current);
            }
            return *this;
        }

        ScopedBossEndTurnNarration(const ScopedBossEndTurnNarration&) = delete;
        ScopedBossEndTurnNarration& operator=(const ScopedBossEndTurnNarration&) = delete;
    };

    // EN: dealOngoingDamage declares or implements a focused behavior used by this module.
    // FR: dealOngoingDamage déclare ou implémente un comportement précis utilisé par ce module.
    void dealOngoingDamage(Entity& player, int damage, ScopedBossEndTurnNarration& narration)
    {
        if (damage <= 0)
        {
            return;
        }

        player.takeDamage(damage);
        narration << player.getName() << " subit " << damage << " dégâts supplémentaires." << std::endl;
        narration << player.getName() << " possède maintenant "
                  << player.getHp() << "/" << player.getMaxHp() << " PV." << std::endl;
        narration << std::endl;
    }
}

bool BossEndTurn::handleBossEndTurn(
    Boss& boss,
    Entity& player
)
{
    ScopedBossEndTurnNarration endTurnNarration(boss);

    if (boss.getBossId() == 6 && !boss.isUltimateActive())
    {
        int darkness = boss.getSpecialEffect() + 1;
        boss.setSpecialEffect(darkness);

        if (darkness % 3 == 0)
        {
            endTurnNarration << "L'obscurité autour de l'avatar épaissit encore." << std::endl;
            endTurnNarration << "Charges d'obscurité : " << darkness << std::endl;
            endTurnNarration << std::endl;
        }
    }

    if (boss.isUltimateActive())
    {
        if (boss.getBossId() == 1)
        {
            int judgment = boss.getSpecialEffect();
            boss.heal(boss.getMaxHp() * (3 + judgment) / 100);

            endTurnNarration << boss.getName() << " se régénère grâce au verdict lumineux." << std::endl;
            endTurnNarration << "Les chaînes brillent encore autour de " << player.getName() << "." << std::endl;
            endTurnNarration << std::endl;
        }
        else if (boss.getBossId() == 2)
        {
            if (boss.getSpecialEffect() == 1 || boss.getSpecialEffect() == 3)
            {
                int maxHpReduction = 4 + boss.getSpecialEffect() * 2;

                player.takeDamage(10 + boss.getSpecialEffect() * 2);
                player.reduceMaxHp(maxHpReduction);
                boss.heal(4 + boss.getSpecialEffect() * 3);

                endTurnNarration << player.getName() << " subit les dégâts de Corrosion." << std::endl;
                endTurnNarration << "Ses PV maximum diminuent de " << maxHpReduction << "." << std::endl;
                endTurnNarration << player.getName() << " possède maintenant "
                          << player.getHp()
                          << "/"
                          << player.getMaxHp()
                          << " PV."
                          << std::endl;
                endTurnNarration << std::endl;
            }
        }
        else if (boss.getBossId() == 3)
        {
            int stored = boss.getSpecialEffect();
            endTurnNarration << "Atlas transforme les coups encaissés en onde de rempart." << std::endl;
            dealOngoingDamage(player, 5 + stored * 5, endTurnNarration);
        }
        else if (boss.getBossId() == 4)
        {
            endTurnNarration << "Des silhouettes de loups traversent la nuit close." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 4, endTurnNarration);
        }
        else if (boss.getBossId() == 5)
        {
            int debt = boss.getSpecialEffect() + 4;
            boss.setSpecialEffect(debt);
            endTurnNarration << "Les collecteurs gobelins griffonnent de nouveaux frais sur le contrat." << std::endl;
            endTurnNarration << "Dette gobeline actuelle : " << debt << std::endl;
            endTurnNarration << std::endl;
        }
        else if (boss.getBossId() == 6)
        {
            endTurnNarration << "La couronne fissurée pulse dans l'obscurité." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 7)
        {
            endTurnNarration << "La brûlure draconique du souffle ancien résonne encore." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 8)
        {
            endTurnNarration << "Les racines de Mojo resserrent brièvement le sol." << std::endl;
            dealOngoingDamage(player, 4 + boss.getSpecialEffect() * 3, endTurnNarration);
        }
        else if (boss.getBossId() == 9)
        {
            endTurnNarration << "Un reflet retardataire d'Inakari te frappe depuis un angle impossible." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2, endTurnNarration);
        }


        else if (boss.getBossId() == 10)
        {
            endTurnNarration << "Ton ombre reste clouée une seconde de trop au sol." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 11)
        {
            endTurnNarration << "L'interface clignote. Une option que tu n'as pas choisie te regarde." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 12)
        {
            endTurnNarration << "Une seconde morte se détache du cadran et te traverse." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect(), endTurnNarration);
        }
        else if (boss.getBossId() == 13)
        {
            if (boss.getSpecialEffect() >= 100)
            {
                endTurnNarration << "Le golem d'os et de sable martèle encore le sol." << std::endl;
                dealOngoingDamage(player, 9 + (boss.getSpecialEffect() - 100) * 3, endTurnNarration);
            }
            else
            {
                endTurnNarration << "La berceuse des enterrés continue sous la poussière." << std::endl;
                dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2, endTurnNarration);
            }
        }
        else if (boss.getBossId() == 14)
        {
            endTurnNarration << "Le champ de bataille imaginaire continue de rugir autour de Boros." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 15)
        {
            endTurnNarration << "Un lien douloureux serre encore ton coeur." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 16)
        {
            endTurnNarration << "Le verdict de Lexior continue de peser sur ton équipement." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 17)
        {
            endTurnNarration << "Un cauchemar lunaire revient en écho derrière tes pensées." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 18)
        {
            endTurnNarration << "Les quatre éléments vibrent encore en désaccord autour de toi." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 19)
        {
            endTurnNarration << "Hitogami applique ce qu'il vient d'apprendre de toi." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 20)
        {
            endTurnNarration << "Les probabilités restent inversées autour de Sérendys." << std::endl;
            dealOngoingDamage(player, 5 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 21)
        {
            endTurnNarration << "Le Seuil maintient sa pression jusqu'à ta prochaine réponse." << std::endl;
            dealOngoingDamage(player, 6 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 22)
        {
            endTurnNarration << "La cour invisible du Roi Sans Salle répète un ordre que personne n'aurait dû entendre." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 23)
        {
            endTurnNarration << "La Bête change encore de silhouette au bord de ton regard." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 24)
        {
            endTurnNarration << "L'abîme d'Aldebaroth continue de salir l'air." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 28)
        {
            endTurnNarration << "Le Souffle sans Visage retient encore une partie de l'air." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 29)
        {
            endTurnNarration << "Les fils de la Marionnette grincent autour de tes articulations." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 30)
        {
            endTurnNarration << "Moiran laisse un futur possible saigner dans le présent." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 31)
        {
            endTurnNarration << "Les lanternes du Cerf restent allumées autour de toi." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }

        else if (boss.getBossId() == 32)
        {
            endTurnNarration << "La fureur royale de Gorvald continue de faire vibrer le sol." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 33)
        {
            endTurnNarration << "Le banquet rouge n'est pas terminé : ton sang répond encore à Serana." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
            boss.heal(4 + boss.getSpecialEffect());
        }
        else if (boss.getBossId() == 34)
        {
            endTurnNarration << "La soie noire resserre la pièce autour de toi." << std::endl;
            dealOngoingDamage(player, 7 + boss.getSpecialEffect() * 2, endTurnNarration);
        }
        else if (boss.getBossId() == 35)
        {
            endTurnNarration << "Un reflet de futur te frappe avec une seconde de retard." << std::endl;
            dealOngoingDamage(player, 8 + boss.getSpecialEffect() * 2, endTurnNarration);
        }

        boss.reduceUltimate();
        if (boss.isUltimateActive())
        {
            endTurnNarration << "Effet ultime encore actif : " << boss.getRemainingUltimateTurns()
                      << " tour(s) de boss restant(s)." << std::endl;
            endTurnNarration << std::endl;
        }

        if (!boss.isUltimateActive())
        {
            if (boss.getBossId() == 1)
            {
                endTurnNarration << "Les ailes de " << boss.getName() << " se rétractent." << std::endl;
                endTurnNarration << "Les chaînes de lumière disparaissent enfin." << std::endl;
            }
            else if (boss.getBossId() == 2)
            {
                endTurnNarration << "L'aura sombre de " << boss.getName() << " s'affaiblit." << std::endl;
                endTurnNarration << "Les miasmes quittent lentement l'arène." << std::endl;
                boss.setSpecialEffect(0);
            }
            else if (boss.getBossId() == 3)
            {
                endTurnNarration << "L'armure d'Atlas s'ouvre à nouveau." << std::endl;
                endTurnNarration << "Le dernier rempart retombe, mais ses fissures restent dangereuses." << std::endl;
            }
            else if (boss.getBossId() == 4)
            {
                endTurnNarration << "La nuit de Lyknir recule, mais la meute garde ton odeur." << std::endl;
                boss.setSpecialEffect(1);
            }
            else if (boss.getBossId() == 5)
            {
                endTurnNarration << "Le contrat royal se replie. La dette restante demeure inscrite en petit." << std::endl;
            }
            else if (boss.getBossId() == 6)
            {
                endTurnNarration << "Le trône d'ombre s'effondre, mais la couronne continue de respirer." << std::endl;
            }
            else if (boss.getBossId() == 7)
            {
                endTurnNarration << "Le souffle de Thamarys se dissipe, mais ses écailles ont appris." << std::endl;
            }
            else if (boss.getBossId() == 8)
            {
                endTurnNarration << "La forêt relâche son étreinte. Mojo continue d'observer." << std::endl;
            }
            else if (boss.getBossId() == 9)
            {
                endTurnNarration << "Les miroirs d'Inakari se fendent, sans disparaître complètement." << std::endl;
            }

            else if (boss.getBossId() == 10)
            {
                endTurnNarration << "Le Jugement Silencieux referme sa main. Ton ombre retombe à sa place." << std::endl;
            }
            else if (boss.getBossId() == 11)
            {
                endTurnNarration << "L'interface redevient lisible, mais tu sais qu'elle peut mentir." << std::endl;
            }
            else if (boss.getBossId() == 12)
            {
                endTurnNarration << "L'aiguille morte reprend sa place. Certaines blessures restent en attente." << std::endl;
            }
            else if (boss.getBossId() == 13)
            {
                endTurnNarration << "Les tombes se taisent, mais le sable continue de respirer." << std::endl;
            }
            else if (boss.getBossId() == 14)
            {
                endTurnNarration << "L'hymne guerrier retombe en un grondement sourd." << std::endl;
            }
            else if (boss.getBossId() == 15)
            {
                endTurnNarration << "Les fils invisibles se desserrent sans disparaître complètement." << std::endl;
            }
            else if (boss.getBossId() == 16)
            {
                endTurnNarration << "Le procès de Lexior se referme, mais le verdict reste dans ta mémoire." << std::endl;
            }
            else if (boss.getBossId() == 17)
            {
                endTurnNarration << "La lune se cache. Le rêve cesse de mordre." << std::endl;
            }
            else if (boss.getBossId() == 18)
            {
                endTurnNarration << "Les quatre formes reviennent dans un seul fragment élémentaire." << std::endl;
            }
            else if (boss.getBossId() == 19)
            {
                endTurnNarration << "Hitogami relâche la copie de ton rythme, mais il n'oublie pas." << std::endl;
            }
            else if (boss.getBossId() == 20)
            {
                endTurnNarration << "Les probabilités cessent de mentir aussi fort." << std::endl;
            }
            else if (boss.getBossId() == 21)
            {
                endTurnNarration << "Le Seuil se rouvre à moitié. L'épreuve n'est pas terminée." << std::endl;
            }
            else if (boss.getBossId() == 22)
            {
                endTurnNarration << "Les murs invisibles du royaume tombent, mais le Roi cherche déjà une autre salle." << std::endl;
            }
            else if (boss.getBossId() == 23)
            {
                endTurnNarration << "Le hurlement s'éteint. Le registre hésite encore à nommer ce qu'il vient de voir." << std::endl;
            }
            else if (boss.getBossId() == 24)
            {
                endTurnNarration << "L'abîme d'Aldebaroth recule, mais une rancune reste dans l'air." << std::endl;
            }
            else if (boss.getBossId() == 25)
            {
                endTurnNarration << "La création et la destruction cessent de se mordre pendant un instant." << std::endl;
            }
            else if (boss.getBossId() == 26)
            {
                endTurnNarration << "L'étage imposé par Obérion se retire lentement." << std::endl;
            }
            else if (boss.getBossId() == 27)
            {
                endTurnNarration << "Le patch temporaire expire. FireFlight sourit comme si le vrai test commençait seulement." << std::endl;
            }
            else if (boss.getBossId() == 28)
            {
                endTurnNarration << "Le monde reprend son souffle, mais pas complètement." << std::endl;
            }
            else if (boss.getBossId() == 29)
            {
                endTurnNarration << "Quelques fils cassent. D'autres attendent déjà de se retendre." << std::endl;
            }
            else if (boss.getBossId() == 30)
            {
                endTurnNarration << "Le fil du Destin cesse de vibrer, sans jamais disparaître." << std::endl;
            }
            else if (boss.getBossId() == 31)
            {
                endTurnNarration << "Les âmes égarées baissent leurs lanternes." << std::endl;
            }
            else
            {
                endTurnNarration << "L'armure d'" << boss.getName() << " perd son éclat anormal." << std::endl;
                endTurnNarration << "Ses matériaux semblent revenir à leur état initial." << std::endl;
            }

            endTurnNarration << std::endl;

            boss.resetUltimateCooldown();
            return true;
        }

        if (boss.getBossId() == 1)
        {
            endTurnNarration << player.getName()
                      << " tente de bouger, mais les chaînes le maintiennent au sol."
                      << std::endl;
            endTurnNarration << boss.getName() << " conserve son tour." << std::endl;
            endTurnNarration << std::endl;

            return false;
        }
    }

    return true;
}
