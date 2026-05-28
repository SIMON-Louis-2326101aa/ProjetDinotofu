// EN: BossUltimate.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossUltimate.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossUltimate.hpp"
#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

    template <typename... Parts>
    void appendUltimateLine(std::vector<std::string>& lines, const Parts&... parts)
    {
        std::ostringstream stream;
        (stream << ... << parts);
        lines.push_back(stream.str());
    }

    // EN: applyDirectDamage declares or implements a focused behavior used by this module.
    // FR: applyDirectDamage déclare ou implémente un comportement précis utilisé par ce module.

    void showBossUltimateLines(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines, false);
    }

    void displayUltimateStart(const Boss& boss)
    {
        std::vector<std::string> lines;
        lines.push_back("L'aura de l'entité change brutalement : un ultime vient de commencer.");

        if (boss.getMaxUltimateTurns() > 1)
        {
            lines.push_back("Durée estimée : " + std::to_string(boss.getMaxUltimateTurns()) + " tours de boss.");
            lines.push_back("Tant que l'effet reste actif, la fin de tour du boss peut déclencher une pression supplémentaire.");
        }
        else
        {
            lines.push_back("Effet estimé : décharge immédiate.");
        }

        showBossUltimateLines("ULTIME DE BOSS", "boss.ultimate.start", lines);
    }

    void applyDirectDamage(Entity& target, int damage, std::vector<std::string>* lines = nullptr)
    {
        if (damage <= 0)
        {
            return;
        }

        target.takeDamage(damage);

        if (lines != nullptr)
        {
            appendUltimateLine(*lines, target.getName(), " reçoit ", damage, " dégâts.");
            appendUltimateLine(*lines, target.getName(), " possède maintenant ", target.getHp(), "/", target.getMaxHp(), " PV.");
            return;
        }

        showBossUltimateLines(
            "IMPACT",
            "boss.ultimate.direct_damage",
            {
                target.getName() + " reçoit " + std::to_string(damage) + " dégâts.",
                target.getName() + " possède maintenant "
                    + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()) + " PV."
            }
        );
    }
}

void BossUltimate::executeBossUltimate(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    boss.activateUltimate();
    displayUltimateStart(boss);

    std::vector<std::string> ultimateLines;

    if (boss.getBossId() == 1)
    {
        int judgment = boss.getSpecialEffect();
        int damage = 18 + judgment * 5 + random.between(0, 10);

        appendUltimateLine(ultimateLines, "Fitoria déploie ses ailes.");
        appendUltimateLine(ultimateLines, "La lumière ne descend pas du ciel. Elle tombe comme un verdict.");
        appendUltimateLine(ultimateLines, "Des chaînes traversent l'arène.");
        appendUltimateLine(ultimateLines, "Elles ne cherchent pas ton corps. Elles cherchent ta faute.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Sentence céleste.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(10 + judgment * 3);
        appendUltimateLine(ultimateLines, "La sentence nourrit Fitoria autant qu'elle te retient.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 2)
    {
        boss.setSpecialEffect(3);

        appendUltimateLine(ultimateLines, "Zelef enfonce sa main dans sa propre poitrine.");
        appendUltimateLine(ultimateLines, "Il arrache quelque chose qui bat encore.");
        appendUltimateLine(ultimateLines, "Le coeur n'est pas le sien. Ou peut-être qu'il l'a volé depuis longtemps.");
        appendUltimateLine(ultimateLines, "Le sang noir explose dans l'arène.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Coeur démoniaque.");
        ultimateLines.push_back("");

        applyDirectDamage(player, 22 + random.between(4, 16), &ultimateLines);
        appendUltimateLine(ultimateLines, "Corrosion et Saignement te rongent en même temps.");
        appendUltimateLine(ultimateLines, "Si Zelef gagne, une partie de ces PV maximum ne reviendra pas toute seule.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 4)
    {
        int markPower = 2 + random.between(0, 2);
        boss.setSpecialEffect(markPower);

        appendUltimateLine(ultimateLines, "La lumière disparaît. Pas lentement. Pas naturellement.");
        appendUltimateLine(ultimateLines, "Elle est dévorée.");
        appendUltimateLine(ultimateLines, "Tu n'entends plus qu'une chose : des pattes, partout.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Chasse sans lune.");
        ultimateLines.push_back("");

        int damage = random.between(boss.getMinDamage(), boss.getMaxDamage()) + 12 * markPower;
        applyDirectDamage(player, damage, &ultimateLines);

        appendUltimateLine(ultimateLines, "La Marque de proie brûle sur toi.");
        appendUltimateLine(ultimateLines, "La meute frappera plus fort tant que la nuit restera fermée.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 5)
    {
        int currentDebt = boss.getSpecialEffect();
        int debtBurst = currentDebt + random.between(18, 35);
        int damage = 18 + debtBurst / 2;

        appendUltimateLine(ultimateLines, "Grinka lève son parchemin.");
        appendUltimateLine(ultimateLines, "Les chiffres bougent seuls. Les lignes se multiplient.");
        appendUltimateLine(ultimateLines, "Les signatures apparaissent sans main pour les écrire.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Dette royale.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);

        boss.setSpecialEffect(currentDebt / 2);
        appendUltimateLine(ultimateLines, "Une partie de la dette brûle dans l'attaque, mais Grinka garde le reste dans son registre.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 6)
    {
        int darkness = boss.getSpecialEffect();
        int damage = 24 + darkness * 4 + random.between(8, 18);

        appendUltimateLine(ultimateLines, "L'avatar lève la main vers sa couronne brisée.");
        appendUltimateLine(ultimateLines, "Les ténèbres ne tombent pas du ciel. Elles montent du sol.");
        appendUltimateLine(ultimateLines, "Elles rampent sur tes jambes. Elles serrent ton torse. Elles cherchent ton nom.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Couronne des ténèbres.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);

        boss.setSpecialEffect(darkness * 2 / 5);
        appendUltimateLine(ultimateLines, "L'ombre se vide partiellement, mais la couronne garde assez de ténèbres pour continuer à grandir.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 3)
    {
        appendUltimateLine(ultimateLines, "Atlas plante ses pieds dans le sol.");
        appendUltimateLine(ultimateLines, "L'arène tremble, comme si elle venait de se souvenir du poids du ciel.");
        appendUltimateLine(ultimateLines, "Son armure se referme. Ses fissures brillent.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Dernier rempart.");
        ultimateLines.push_back("");
        boss.setSpecialEffect(boss.getSpecialEffect() + 2);
    }
    else if (boss.getBossId() == 7)
    {
        int adaptation = boss.getSpecialEffect();
        int damage = 30 + adaptation * 5 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Le Fragment de Thamarys inspire.");
        appendUltimateLine(ultimateLines, "L'air devient trop lourd pour être respiré.");
        appendUltimateLine(ultimateLines, "Le souffle qui arrive n'appartient à aucun élément unique.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Souffle de l'Origine.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Une brûlure draconique temporaire marque le combat.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 8)
    {
        int memory = boss.getSpecialEffect();
        bool respectful = memory <= 2 || player.isInDefensePosture();

        appendUltimateLine(ultimateLines, "Mojo ferme les yeux.");
        appendUltimateLine(ultimateLines, "Chaque racine devient une main. Chaque feuille devient un jugement.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "La forêt se souvient.");
        ultimateLines.push_back("");

        if (respectful)
        {
            applyDirectDamage(player, 14 + random.between(0, 8), &ultimateLines);
            boss.setSpecialEffect(0);
            appendUltimateLine(ultimateLines, "Les racines reculent avant de te broyer complètement.");
            appendUltimateLine(ultimateLines, "La forêt ne voit pas seulement un ennemi. Elle voit un passage.");
        }
        else
        {
            applyDirectDamage(player, 24 + memory * 4 + random.between(4, 14), &ultimateLines);
            appendUltimateLine(ultimateLines, "La forêt se souvient de chaque branche brisée.");
            appendUltimateLine(ultimateLines, "Tes mouvements deviennent plus lourds sous les racines.");
        }

        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 9)
    {
        int mirrors = boss.getSpecialEffect();
        int playerChoice = random.between(1, 3);

        appendUltimateLine(ultimateLines, "Inakari divise l'arène en neuf reflets.");
        appendUltimateLine(ultimateLines, "Chaque mensonge a une voix. Chaque voix prétend être la bonne.");
        appendUltimateLine(ultimateLines, "Un détail trahit pourtant le vrai : une seule ombre touche le sol.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Neuf reflets, une seule vérité.");
        ultimateLines.push_back("");

        if (playerChoice == 1)
        {
            applyDirectDamage(player, 18 + mirrors * 3 + random.between(0, 8), &ultimateLines);
            boss.setSpecialEffect(0);
            appendUltimateLine(ultimateLines, "Tu lis l'indice au dernier moment. Les illusions vacillent.");
        }
        else
        {
            applyDirectDamage(player, 28 + mirrors * 5 + random.between(3, 12), &ultimateLines);
            appendUltimateLine(ultimateLines, "Tu frappes un mensonge. Le vrai reflet te frappe depuis le côté.");
        }

        ultimateLines.push_back("");
    }

    else if (boss.getBossId() == 10)
    {
        int judgment = boss.getSpecialEffect();
        int damage = 24 + judgment * 5 + random.between(4, 14);

        appendUltimateLine(ultimateLines, "Le Jugement Silencieux lève une main sans bruit.");
        appendUltimateLine(ultimateLines, "Ton ombre se détache du sol comme si elle devait témoigner contre toi.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Sentence sans voix.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Plus tes gestes se répètent, plus le jugement trouvera une forme.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 11)
    {
        int corruption = boss.getSpecialEffect();
        int damage = 26 + corruption * 4 + random.between(5, 18);

        appendUltimateLine(ultimateLines, "Erreur critique. Le récit se réécrit.");
        appendUltimateLine(ultimateLines, "Tu n'es jamais vraiment né. Tu n'es qu'une variable instable.");
        appendUltimateLine(ultimateLines, "Les règles se déplacent.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Erreur critique : le récit se réécrit.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);

        if (random.between(1, 100) <= 35)
        {
            int healAmount = 18 + corruption * 3;
            boss.heal(healAmount);
            appendUltimateLine(ultimateLines, "PV de l'Anomalie : correction forcée de +", healAmount, ".");
        }

        appendUltimateLine(ultimateLines, "Pendant quelques tours, l'interface semblera mentir avant de se stabiliser.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 12)
    {
        int delayedWounds = boss.getSpecialEffect();
        int damage = 22 + delayedWounds + random.between(6, 16);

        appendUltimateLine(ultimateLines, "L'Horloge des Chuchotements s'arrête.");
        appendUltimateLine(ultimateLines, "Puis toutes les secondes que tu croyais avoir évitées reviennent ensemble.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Retour de la minute morte.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.setSpecialEffect(delayedWounds / 2);
        appendUltimateLine(ultimateLines, "Une partie des blessures retardées vient d'être payée.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 13)
    {
        int profanationState = boss.getSpecialEffect();
        bool golemAwakened = profanationState >= 100;
        int profanation = golemAwakened ? profanationState - 100 : profanationState;

        if (golemAwakened)
        {
            int damage = 34 + profanation * 6 + random.between(8, 20);

            appendUltimateLine(ultimateLines, "Le golem d'os et de sable ouvre sa cage thoracique.");
            appendUltimateLine(ultimateLines, "Au centre, l'Enfant dort presque, fusionné avec le coeur du colosse.");
            appendUltimateLine(ultimateLines, "Chaque tombe autour de toi se relève dans le même mouvement.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Tombeau du géant intérieur.");
            ultimateLines.push_back("");

            applyDirectDamage(player, damage, &ultimateLines);
            appendUltimateLine(ultimateLines, "Le second rythme du combat écrase l'arène sous des pas de sable et d'os.");
        }
        else
        {
            int damage = 20 + profanation * 5 + random.between(4, 14);

            appendUltimateLine(ultimateLines, "L'Enfant ouvre la bouche. Aucune voix ne sort.");
            appendUltimateLine(ultimateLines, "Pourtant, sous tes pieds, les morts chantent.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Berceuse des enterrés.");
            ultimateLines.push_back("");

            applyDirectDamage(player, damage, &ultimateLines);
            appendUltimateLine(ultimateLines, "La Profanation rend la berceuse plus lourde à chaque faute.");
        }

        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 14)
    {
        int warGauge = boss.getSpecialEffect();
        int damage = 30 + warGauge * 4 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "L'arme de Boros frappe le sol.");
        appendUltimateLine(ultimateLines, "Le bruit ne ressemble pas à du métal.");
        appendUltimateLine(ultimateLines, "Il ressemble à mille batailles qui recommencent en même temps.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Hymne du carnage.");
        ultimateLines.push_back("");

        if (player.isInDefensePosture())
        {
            damage = damage * 70 / 100;
            appendUltimateLine(ultimateLines, "Ta posture absorbe une partie du choc guerrier.");
        }

        applyDirectDamage(player, damage, &ultimateLines);

        if (warGauge >= 9)
        {
            boss.takeDamage(8 + warGauge);
            appendUltimateLine(ultimateLines, "La Jauge de guerre était si haute que l'avatar subit aussi le contrecoup du carnage.");
        }

        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 15)
    {
        int links = boss.getSpecialEffect();
        int deathMemory = 0;
        Player* concretePlayer = dynamic_cast<Player*>(&player);
        if (concretePlayer != nullptr)
        {
            deathMemory = concretePlayer->getDeaths();
            if (deathMemory > 12) deathMemory = 12;
        }

        int damage = 24 + links * 4 + deathMemory * 2 + random.between(4, 16);

        appendUltimateLine(ultimateLines, "Des fils invisibles s'enroulent autour de toi.");
        appendUltimateLine(ultimateLines, "Ils ne coupent pas la peau. Ils serrent ce que tu refuses de perdre.");
        appendUltimateLine(ultimateLines, "Anastasia sent aussi combien de fois tu t'es accroché à la vie.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Serment qui saigne.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Les soins paraissent plus lourds, comme si survivre avait désormais un prix émotionnel.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 16)
    {
        int verdict = boss.getSpecialEffect();
        int damage = 28 + verdict * 5 + random.between(6, 18);

        appendUltimateLine(ultimateLines, "Lexior prononce une phrase sans hausser la voix.");
        appendUltimateLine(ultimateLines, "Le combat cesse de ressembler à un duel. Il ressemble à un procès.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Verdict des actes répétés.");
        ultimateLines.push_back("");

        Player* concretePlayer = dynamic_cast<Player*>(&player);
        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            damage += 20;
            appendUltimateLine(ultimateLines, "Les altérations du personnage sont ajoutées au dossier.");
        }

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Tant que le verdict tient, l'équipement scellé répondra mal.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 17)
    {
        int dreamPressure = boss.getSpecialEffect();
        int damage = 24 + dreamPressure * 4 + random.between(6, 18);

        appendUltimateLine(ultimateLines, "La lune disparaît, puis revient derrière tes paupières.");
        appendUltimateLine(ultimateLines, "Luna apaise le monde. Onyrae transforme ce calme en piège.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Éclipse du rêve et du cauchemar.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        if (random.between(1, 100) <= 45)
        {
            boss.heal(16 + dreamPressure * 2);
            appendUltimateLine(ultimateLines, "Une partie du rêve revient dans le fragment au lieu de te laisser une ouverture.");
        }
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 18)
    {
        int instability = boss.getSpecialEffect();

        if (random.between(1, 100) <= 50)
        {
            int damage = 32 + instability * 5 + random.between(8, 22);
            appendUltimateLine(ultimateLines, "Les quatre éléments cessent de se contredire.");
            appendUltimateLine(ultimateLines, "Ils fusionnent en une seule pression impossible.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Fusion élémentaire supérieure.");
            ultimateLines.push_back("");
            applyDirectDamage(player, damage, &ultimateLines);
        }
        else
        {
            int totalDamage = 24 + instability * 4 + random.between(8, 20);
            int quarterDamage = totalDamage / 4;
            if (quarterDamage < 1) quarterDamage = 1;

            appendUltimateLine(ultimateLines, "Syvaranelya se sépare en quatre présences.");
            appendUltimateLine(ultimateLines, "Feu. Terre. Eau. Vent.");
            appendUltimateLine(ultimateLines, "Pendant un instant, le combat devient un faux 1 contre 4.");
            appendUltimateLine(ultimateLines, "Chaque partie ne porte qu'un quart du corps, mais chaque blessure résonne chez toutes.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Quadrature élémentaire.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Les dégâts sont divisés entre quatre formes : ", quarterDamage, " x4.");
            applyDirectDamage(player, quarterDamage * 4, &ultimateLines);
        }

        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 19)
    {
        int adaptation = boss.getSpecialEffect();
        int damage = 26 + adaptation * 5 + random.between(6, 18);

        appendUltimateLine(ultimateLines, "Hitogami ferme les yeux.");
        appendUltimateLine(ultimateLines, "Pendant une seconde, tu crois qu'il abandonne.");
        appendUltimateLine(ultimateLines, "Puis il reprend ta posture. Ton rythme. Ton souffle.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "L'humain recommence.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.setSpecialEffect(adaptation + 2);
        appendUltimateLine(ultimateLines, "L'écho humain a appris une partie de ta manière de survivre.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 20)
    {
        int luck = boss.getSpecialEffect();

        if (random.between(1, 100) <= 50)
        {
            int damage = 24 + luck * 4 + random.between(8, 22);
            appendUltimateLine(ultimateLines, "Trois futurs se dessinent devant Sérendys.");
            appendUltimateLine(ultimateLines, "Dans le premier, elle tombe. Dans le second, tu souris.");
            appendUltimateLine(ultimateLines, "Dans le troisième, elle efface les deux autres.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Le destin garde la meilleure issue.");
            ultimateLines.push_back("");
            applyDirectDamage(player, damage, &ultimateLines);
            if (random.between(1, 100) <= 45)
            {
                boss.heal(12 + luck * 2);
                appendUltimateLine(ultimateLines, "Le futur choisi lui rend aussi une partie de sa chance.");
            }
        }
        else
        {
            int damage = 18 + luck * 3 + random.between(6, 16);
            boss.setSpecialEffect(luck + 4);
            appendUltimateLine(ultimateLines, "Sérendys retourne une pièce qui ne devrait avoir que deux faces.");
            appendUltimateLine(ultimateLines, "Ce qui était rare devient commun. Ce qui était commun devient rare.");
            appendUltimateLine(ultimateLines, "Pendant deux tours, les effets absurdes arrivent trop facilement et les évidences deviennent capricieuses.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Inversion des probabilités.");
            ultimateLines.push_back("");
            applyDirectDamage(player, damage, &ultimateLines);
        }

        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 21)
    {
        int threshold = boss.getSpecialEffect();
        int damage = 28 + threshold * 4 + random.between(6, 18);

        appendUltimateLine(ultimateLines, "Le Gardien lève une main.");
        appendUltimateLine(ultimateLines, "L'espace derrière toi disparaît. Celui devant toi se ferme.");
        appendUltimateLine(ultimateLines, "Il ne reste qu'une seule question : étais-tu prêt ?");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Le Seuil se referme.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Le prochain verrou dépendra encore de la façon dont tu réponds à l'épreuve.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 22)
    {
        int authority = boss.getSpecialEffect();
        int damage = 30 + authority * 4 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Des murs invisibles se dressent.");
        appendUltimateLine(ultimateLines, "Un trône sans matière apparaît derrière le Roi Sans Salle.");
        appendUltimateLine(ultimateLines, "La cour est morte. Mais elle s'agenouille encore.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Couronnement des ruines.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(16 + authority * 2);
        appendUltimateLine(ultimateLines, "Pendant quelques tours, chaque ordre royal sera renforcé par les ruines.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 23)
    {
        int instinct = boss.getSpecialEffect();
        int damage = 34 + instinct * 5 + random.between(8, 24);

        appendUltimateLine(ultimateLines, "La Bête ouvre la gueule.");
        appendUltimateLine(ultimateLines, "Aucun son ne devrait sortir. Parce qu'aucun nom ne devrait l'appeler.");
        appendUltimateLine(ultimateLines, "Pourtant, le monde tremble.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Hurlement sans nom.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Le registre brouille une partie des informations. Cette Bête ne ressemblera peut-être pas à la prochaine.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 24)
    {
        int abyss = boss.getSpecialEffect();
        int damage = 32 + abyss * 5 + random.between(8, 24);

        appendUltimateLine(ultimateLines, "Aldebaroth sourit comme si le pire du monde venait de retrouver son propriétaire.");
        appendUltimateLine(ultimateLines, "La haine, la peur et la rancune se condensent autour de lui.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Abîme des fautes accumulées.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(18 + abyss * 2);
        appendUltimateLine(ultimateLines, "Il ne réagit pas aux altérations de triche. Ce n'est pas son domaine.");
        appendUltimateLine(ultimateLines, "Il se nourrit seulement du négatif laissé par le combat.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 25)
    {
        int twinState = boss.getSpecialEffect();
        int damage = 30 + twinState * 4 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Asterion crée une forme parfaite.");
        appendUltimateLine(ultimateLines, "Nihilon la détruit avant qu'elle ait le temps d'exister complètement.");
        appendUltimateLine(ultimateLines, "Le choc entre création et destruction traverse l'arène.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Cycle impossible.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.setSpecialEffect(twinState + 2);
        appendUltimateLine(ultimateLines, "Si l'un des deux tombe vraiment, il ne restera que cinq tours pour empêcher la reconstruction fatale.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 26)
    {
        int authority = boss.getSpecialEffect();
        int damage = 34 + authority * 5 + random.between(10, 26);

        appendUltimateLine(ultimateLines, "Obérion ne descend pas dans l'arène.");
        appendUltimateLine(ultimateLines, "Seul un écho fragmenté de son autorité accepte de te regarder.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Origine du monde étagé.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);

        Player* concretePlayer = dynamic_cast<Player*>(&player);
        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            appendUltimateLine(ultimateLines, "Les altérations sont pesées sans colère.");
            appendUltimateLine(ultimateLines, "Une loi ancienne refuse que les raccourcis remplacent l'existence.");
            applyDirectDamage(player, 18 + concretePlayer->getLethalCheatAttemptCount() * 8, &ultimateLines);
        }

        appendUltimateLine(ultimateLines, "L'écho peut forcer un duel nu : sans armure, sans arme, seulement le corps et la volonté.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 27)
    {
        int authority = boss.getSpecialEffect();
        bool devLimit = random.between(1, 100) <= 45;

        appendUltimateLine(ultimateLines, "Une fenêtre invisible s'ouvre dans l'air.");
        appendUltimateLine(ultimateLines, "FireFlight ne lève pas son arme. Il lève la main vers le monde.");
        ultimateLines.push_back("");

        if (devLimit)
        {
            appendUltimateLine(ultimateLines, "Commande développeur : Limite imposée.");
            appendUltimateLine(ultimateLines, "Tu voulais une victoire par automatisme. Je préfère une victoire méritée.");
            appendUltimateLine(ultimateLines, "Pendant quelques tours, la stratégie dominante devient moins fiable.");
            applyDirectDamage(player, 24 + authority * 3 + random.between(6, 16), &ultimateLines);
        }
        else
        {
            appendUltimateLine(ultimateLines, "Patch d'urgence.");
            appendUltimateLine(ultimateLines, "Je n'ai pas créé ce monde pour qu'il soit juste.");
            appendUltimateLine(ultimateLines, "Je l'ai créé pour qu'il survive à ceux qui essaient de le briser.");
            applyDirectDamage(player, 32 + authority * 4 + random.between(8, 24), &ultimateLines);
            boss.heal(20 + authority * 2);
        }

        appendUltimateLine(ultimateLines, "L'interface cligne une fois, comme si quelqu'un venait de sauvegarder une règle au-dessus de toi.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 28)
    {
        int breath = boss.getSpecialEffect();
        int damage = 30 + breath * 5 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Le Souffle sans Visage cesse d'inspirer.");
        appendUltimateLine(ultimateLines, "Pendant une seconde, tout l'air du combat appartient à l'entité.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Apnée du monde.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Les prochains tours semblent plus courts, comme si respirer demandait maintenant une action entière.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 29)
    {
        int nails = boss.getSpecialEffect();
        int damage = 28 + nails * 5 + random.between(8, 20);

        appendUltimateLine(ultimateLines, "Tous les fils se tendent d'un seul coup.");
        appendUltimateLine(ultimateLines, "La Marionnette n'avance pas vers toi : elle te tire vers elle.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Théâtre des Mille Clous.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(8 + nails * 2);
        appendUltimateLine(ultimateLines, "Chaque clou planté dans le rythme du combat recoud une partie de son corps.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 30)
    {
        int line = boss.getSpecialEffect();
        Player* concretePlayer = dynamic_cast<Player*>(&player);
        int damage = 32 + line * 5 + random.between(10, 24);

        appendUltimateLine(ultimateLines, "Moiran ferme les yeux.");
        appendUltimateLine(ultimateLines, "Toutes les routes possibles s'affichent une fraction de seconde.");
        appendUltimateLine(ultimateLines, "Puis il en coupe une.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Fil rompu du Destin.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            appendUltimateLine(ultimateLines, "Les altérations ne changent pas le destin : elles lui donnent seulement plus de preuves.");
            applyDirectDamage(player, 10 + concretePlayer->getLethalCheatAttemptCount() * 6, &ultimateLines);
        }
        appendUltimateLine(ultimateLines, "Pendant quelques tours, les choix qui semblaient sûrs deviennent suspects.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 31)
    {
        int souls = boss.getSpecialEffect();
        int damage = 30 + souls * 4 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Le Cerf baisse les bois.");
        appendUltimateLine(ultimateLines, "Les lanternes des morts s'allument une par une.");
        appendUltimateLine(ultimateLines, "Aucune voix ne crie. C'est pire : elles te reconnaissent.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Procession des Âmes Égarées.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(12 + souls * 2);
        appendUltimateLine(ultimateLines, "Le Cerf ne veut pas seulement gagner : il veut ramener les âmes perdues hors de ta route.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 28)
    {
        int breath = boss.getSpecialEffect();
        int damage = 30 + breath * 4 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Le Souffle sans Visage efface la distance entre inspiration et noyade.");
        appendUltimateLine(ultimateLines, "L'air devient un couloir trop étroit pour ton propre corps.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Asphyxie d'identité.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Pendant quelques tours, respirer ressemble à une action volontaire.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 29)
    {
        int nails = boss.getSpecialEffect();
        int damage = 28 + nails * 5 + random.between(8, 24);

        appendUltimateLine(ultimateLines, "La Marionnette aux Mille Clous cesse de bouger.");
        appendUltimateLine(ultimateLines, "Tous les fils, eux, continuent.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Théâtre des douleurs suspendues.");
        ultimateLines.push_back("");

        if (player.isInDefensePosture())
        {
            damage = damage * 80 / 100;
            appendUltimateLine(ultimateLines, "Ta posture retient une partie des fils, mais chaque clou tire dans une direction différente.");
        }

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(10 + nails);
        appendUltimateLine(ultimateLines, "Une partie de ta douleur revient dans la marionnette sous forme de réparation sale.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 30)
    {
        int line = boss.getSpecialEffect();
        int damage = 32 + line * 5 + random.between(10, 26);
        Player* concretePlayer = dynamic_cast<Player*>(&player);

        appendUltimateLine(ultimateLines, "Moiran ferme les yeux.");
        appendUltimateLine(ultimateLines, "Le combat continue. Mais quelque chose vient de se terminer ailleurs.");
        appendUltimateLine(ultimateLines, "Une version de toi tombe. Une autre n'a jamais existé.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "La Fin qui te reconnaît.");
        ultimateLines.push_back("");

        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            damage += 18;
            appendUltimateLine(ultimateLines, "Ta ligne de destinée porte une cicatrice d'altération. Moiran la suit du doigt.");
        }

        if (concretePlayer != nullptr && concretePlayer->getLethalCheatAttemptCount() > 0)
        {
            damage += concretePlayer->getLethalCheatAttemptCount() * 12;
            appendUltimateLine(ultimateLines, "Chemin refusé.");
            appendUltimateLine(ultimateLines, "Cette route n'a jamais été acceptée. Elle frappe maintenant avec ses intérêts.");
        }

        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Pendant deux tours, répéter exactement la même réponse devient plus dangereux.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 31)
    {
        int souls = boss.getSpecialEffect();
        int damage = 26 + souls * 4 + random.between(8, 20);

        appendUltimateLine(ultimateLines, "Le Cerf baisse ses bois.");
        appendUltimateLine(ultimateLines, "Les lanternes d'âmes s'ouvrent une à une, sans flamme.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Procession des âmes égarées.");
        ultimateLines.push_back("");

        if (player.isInDefensePosture())
        {
            damage = damage * 70 / 100;
            appendUltimateLine(ultimateLines, "Ta retenue calme une partie de la procession.");
        }
        else
        {
            appendUltimateLine(ultimateLines, "Chaque âme traverse ton corps comme si elle cherchait encore une sortie.");
        }

        applyDirectDamage(player, damage, &ultimateLines);
        boss.heal(8 + souls * 2);
        appendUltimateLine(ultimateLines, "Le Cerf récupère les regrets qui n'ont pas trouvé de tombe.");
        ultimateLines.push_back("");
    }

    else if (boss.getBossId() == 32)
    {
        int fury = boss.getSpecialEffect();
        int damage = 34 + fury * 5 + random.between(10, 26);

        appendUltimateLine(ultimateLines, "Gorvald lève sa hache comme s'il soulevait tout son royaume.");
        appendUltimateLine(ultimateLines, "Derrière lui, tu n'entends pas une armée. Tu entends une nation frapper son torse.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Couronne de sang orc.");
        ultimateLines.push_back("");

        if (player.isInDefensePosture())
        {
            damage = damage * 70 / 100;
            appendUltimateLine(ultimateLines, "Ta posture encaisse l'impact. Gorvald respecte la garde, mais pas la faiblesse.");
        }
        applyDirectDamage(player, damage, &ultimateLines);
        boss.setSpecialEffect(fury / 2);
        appendUltimateLine(ultimateLines, "Une partie de sa fureur royale se consume dans l'attaque.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 33)
    {
        int thirst = boss.getSpecialEffect();
        int damage = 30 + thirst * 5 + random.between(8, 24);

        appendUltimateLine(ultimateLines, "La lumière de l'arène devient rouge.");
        appendUltimateLine(ultimateLines, "Serana ne bondit pas. Elle invite. Ton sang répond avant toi.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Banquet de la nuit rouge.");
        ultimateLines.push_back("");

        applyDirectDamage(player, damage, &ultimateLines);
        int heal = 18 + thirst * 4;
        boss.heal(heal);
        appendUltimateLine(ultimateLines, "Serana récupère ", heal, " PV en transformant la blessure en repas royal.");
        appendUltimateLine(ultimateLines, "Tes soins semblent moins rassurants pendant quelques tours.");
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 34)
    {
        int web = boss.getSpecialEffect();
        int damage = 28 + web * 4 + random.between(8, 22);

        appendUltimateLine(ultimateLines, "Les murs disparaissent sous la soie.");
        appendUltimateLine(ultimateLines, "Le plafond descend. Le sol colle à tes pas.");
        ultimateLines.push_back("");
        appendUltimateLine(ultimateLines, "Royaume de soie noire.");
        ultimateLines.push_back("");

        if (player.isInDefensePosture())
        {
            damage = damage * 85 / 100;
            appendUltimateLine(ultimateLines, "Ta posture limite les morsures, mais la toile entre dans chaque angle mort.");
        }
        applyDirectDamage(player, damage, &ultimateLines);
        boss.setSpecialEffect(web + 2);
        appendUltimateLine(ultimateLines, "Toile de reine renforcée : ", boss.getSpecialEffect());
        ultimateLines.push_back("");
    }
    else if (boss.getBossId() == 35)
    {
        int mirror = boss.getSpecialEffect();
        bool secondUltimate = random.between(1, 100) <= 45;
        int damage = 32 + mirror * 5 + random.between(10, 26);

        if (secondUltimate)
        {
            appendUltimateLine(ultimateLines, "Ton reflet bouge avant toi.");
            appendUltimateLine(ultimateLines, "Il lève ton arme. Il porte ton visage. Mais son sourire ne t'appartient pas.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Reflet qui prend ta place.");
            boss.heal(14 + mirror * 2);
            damage += 10;
        }
        else
        {
            appendUltimateLine(ultimateLines, "Le miroir se fend encore.");
            appendUltimateLine(ultimateLines, "Dans chaque morceau, tu gagnes. Dans chaque morceau, tu perds.");
            ultimateLines.push_back("");
            appendUltimateLine(ultimateLines, "Futur dans le miroir.");
        }

        ultimateLines.push_back("");
        if (player.isInDefensePosture())
        {
            damage = damage * 80 / 100;
            appendUltimateLine(ultimateLines, "Tu refuses de suivre le premier reflet. Une partie du futur se casse avant de te toucher.");
        }
        applyDirectDamage(player, damage, &ultimateLines);
        appendUltimateLine(ultimateLines, "Les Jumelles te laissent avec une question : as-tu évité le mensonge, ou seulement préféré l'autre ?");
        ultimateLines.push_back("");
    }
    else
    {
        appendUltimateLine(ultimateLines, boss.getName(), " se met en position de défense.");
        appendUltimateLine(ultimateLines, "Son armure change de couleur et de matière.");
        appendUltimateLine(ultimateLines, "Sa résistance semble désormais presque impénétrable.");
        ultimateLines.push_back("");
    }

    if (!ultimateLines.empty())
    {
        showBossUltimateLines(
            "ULTIME — " + boss.getName(),
            "boss.ultimate.resolve." + std::to_string(boss.getBossId()),
            ultimateLines
        );
    }

    player.takeDamage(0);
}
