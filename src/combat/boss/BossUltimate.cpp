// EN: BossUltimate.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossUltimate.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossUltimate.hpp"
#include "entity/Player.hpp"

#include <iostream>

namespace
{
    // EN: applyDirectDamage declares or implements a focused behavior used by this module.
    // FR: applyDirectDamage déclare ou implémente un comportement précis utilisé par ce module.
    void applyDirectDamage(Entity& target, int damage)
    {
        if (damage <= 0)
        {
            return;
        }

        target.takeDamage(damage);
        std::cout << target.getName() << " reçoit " << damage << " dégâts." << std::endl;
        std::cout << target.getName() << " possède maintenant "
                  << target.getHp() << "/" << target.getMaxHp() << " PV." << std::endl;
        std::cout << std::endl;
    }
}

void BossUltimate::executeBossUltimate(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    boss.activateUltimate();

    if (boss.getBossId() == 1)
    {
        int judgment = boss.getSpecialEffect();
        int damage = 18 + judgment * 5 + random.between(0, 10);

        std::cout << "Fitoria déploie ses ailes." << std::endl;
        std::cout << "La lumière ne descend pas du ciel. Elle tombe comme un verdict." << std::endl;
        std::cout << "Des chaînes traversent l'arène." << std::endl;
        std::cout << "Elles ne cherchent pas ton corps. Elles cherchent ta faute." << std::endl;
        std::cout << std::endl;
        std::cout << "Sentence céleste." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.heal(10 + judgment * 3);
        std::cout << "La sentence nourrit Fitoria autant qu'elle te retient." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 2)
    {
        boss.setSpecialEffect(3);

        std::cout << "Zelef enfonce sa main dans sa propre poitrine." << std::endl;
        std::cout << "Il arrache quelque chose qui bat encore." << std::endl;
        std::cout << "Le coeur n'est pas le sien. Ou peut-être qu'il l'a volé depuis longtemps." << std::endl;
        std::cout << "Le sang noir explose dans l'arène." << std::endl;
        std::cout << std::endl;
        std::cout << "Coeur démoniaque." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, 22 + random.between(4, 16));
        std::cout << "Corrosion et Saignement te rongent en même temps." << std::endl;
        std::cout << "Si Zelef gagne, une partie de ces PV maximum ne reviendra pas toute seule." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 4)
    {
        int markPower = 2 + random.between(0, 2);
        boss.setSpecialEffect(markPower);

        std::cout << "La lumière disparaît. Pas lentement. Pas naturellement." << std::endl;
        std::cout << "Elle est dévorée." << std::endl;
        std::cout << "Tu n'entends plus qu'une chose : des pattes, partout." << std::endl;
        std::cout << std::endl;
        std::cout << "Chasse sans lune." << std::endl;
        std::cout << std::endl;

        int damage = random.between(boss.getMinDamage(), boss.getMaxDamage()) + 12 * markPower;
        applyDirectDamage(player, damage);

        std::cout << "La Marque de proie brûle sur toi." << std::endl;
        std::cout << "La meute frappera plus fort tant que la nuit restera fermée." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 5)
    {
        int currentDebt = boss.getSpecialEffect();
        int debtBurst = currentDebt + random.between(18, 35);
        int damage = 18 + debtBurst / 2;

        std::cout << "Grinka lève son parchemin." << std::endl;
        std::cout << "Les chiffres bougent seuls. Les lignes se multiplient." << std::endl;
        std::cout << "Les signatures apparaissent sans main pour les écrire." << std::endl;
        std::cout << std::endl;
        std::cout << "Dette royale." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);

        boss.setSpecialEffect(currentDebt / 2);
        std::cout << "Une partie de la dette brûle dans l'attaque, mais Grinka garde le reste dans son registre." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 6)
    {
        int darkness = boss.getSpecialEffect();
        int damage = 24 + darkness * 4 + random.between(8, 18);

        std::cout << "L'avatar lève la main vers sa couronne brisée." << std::endl;
        std::cout << "Les ténèbres ne tombent pas du ciel. Elles montent du sol." << std::endl;
        std::cout << "Elles rampent sur tes jambes. Elles serrent ton torse. Elles cherchent ton nom." << std::endl;
        std::cout << std::endl;
        std::cout << "Couronne des ténèbres." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);

        boss.setSpecialEffect(darkness * 2 / 5);
        std::cout << "L'ombre se vide partiellement, mais la couronne garde assez de ténèbres pour continuer à grandir." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 3)
    {
        std::cout << "Atlas plante ses pieds dans le sol." << std::endl;
        std::cout << "L'arène tremble, comme si elle venait de se souvenir du poids du ciel." << std::endl;
        std::cout << "Son armure se referme. Ses fissures brillent." << std::endl;
        std::cout << std::endl;
        std::cout << "Dernier rempart." << std::endl;
        std::cout << std::endl;
        boss.setSpecialEffect(boss.getSpecialEffect() + 2);
    }
    else if (boss.getBossId() == 7)
    {
        int adaptation = boss.getSpecialEffect();
        int damage = 30 + adaptation * 5 + random.between(8, 22);

        std::cout << "Le Fragment de Thamarys inspire." << std::endl;
        std::cout << "L'air devient trop lourd pour être respiré." << std::endl;
        std::cout << "Le souffle qui arrive n'appartient à aucun élément unique." << std::endl;
        std::cout << std::endl;
        std::cout << "Souffle de l'Origine." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Une brûlure draconique temporaire marque le combat." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 8)
    {
        int memory = boss.getSpecialEffect();
        bool respectful = memory <= 2 || player.isInDefensePosture();

        std::cout << "Mojo ferme les yeux." << std::endl;
        std::cout << "Chaque racine devient une main. Chaque feuille devient un jugement." << std::endl;
        std::cout << std::endl;
        std::cout << "La forêt se souvient." << std::endl;
        std::cout << std::endl;

        if (respectful)
        {
            applyDirectDamage(player, 14 + random.between(0, 8));
            boss.setSpecialEffect(0);
            std::cout << "Les racines reculent avant de te broyer complètement." << std::endl;
            std::cout << "La forêt ne voit pas seulement un ennemi. Elle voit un passage." << std::endl;
        }
        else
        {
            applyDirectDamage(player, 24 + memory * 4 + random.between(4, 14));
            std::cout << "La forêt se souvient de chaque branche brisée." << std::endl;
            std::cout << "Tes mouvements deviennent plus lourds sous les racines." << std::endl;
        }

        std::cout << std::endl;
    }
    else if (boss.getBossId() == 9)
    {
        int mirrors = boss.getSpecialEffect();
        int playerChoice = random.between(1, 3);

        std::cout << "Inakari divise l'arène en neuf reflets." << std::endl;
        std::cout << "Chaque mensonge a une voix. Chaque voix prétend être la bonne." << std::endl;
        std::cout << "Un détail trahit pourtant le vrai : une seule ombre touche le sol." << std::endl;
        std::cout << std::endl;
        std::cout << "Neuf reflets, une seule vérité." << std::endl;
        std::cout << std::endl;

        if (playerChoice == 1)
        {
            applyDirectDamage(player, 18 + mirrors * 3 + random.between(0, 8));
            boss.setSpecialEffect(0);
            std::cout << "Tu lis l'indice au dernier moment. Les illusions vacillent." << std::endl;
        }
        else
        {
            applyDirectDamage(player, 28 + mirrors * 5 + random.between(3, 12));
            std::cout << "Tu frappes un mensonge. Le vrai reflet te frappe depuis le côté." << std::endl;
        }

        std::cout << std::endl;
    }

    else if (boss.getBossId() == 10)
    {
        int judgment = boss.getSpecialEffect();
        int damage = 24 + judgment * 5 + random.between(4, 14);

        std::cout << "Le Jugement Silencieux lève une main sans bruit." << std::endl;
        std::cout << "Ton ombre se détache du sol comme si elle devait témoigner contre toi." << std::endl;
        std::cout << std::endl;
        std::cout << "Sentence sans voix." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Plus tes gestes se répètent, plus le jugement trouvera une forme." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 11)
    {
        int corruption = boss.getSpecialEffect();
        int damage = 26 + corruption * 4 + random.between(5, 18);

        std::cout << "Erreur critique. Le récit se réécrit." << std::endl;
        std::cout << "Tu n'es jamais vraiment né. Tu n'es qu'une variable instable." << std::endl;
        std::cout << "Les règles se déplacent." << std::endl;
        std::cout << std::endl;
        std::cout << "Erreur critique : le récit se réécrit." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);

        if (random.between(1, 100) <= 35)
        {
            int healAmount = 18 + corruption * 3;
            boss.heal(healAmount);
            std::cout << "PV de l'Anomalie : correction forcée de +" << healAmount << "." << std::endl;
        }

        std::cout << "Pendant quelques tours, l'interface semblera mentir avant de se stabiliser." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 12)
    {
        int delayedWounds = boss.getSpecialEffect();
        int damage = 22 + delayedWounds + random.between(6, 16);

        std::cout << "L'Horloge des Chuchotements s'arrête." << std::endl;
        std::cout << "Puis toutes les secondes que tu croyais avoir évitées reviennent ensemble." << std::endl;
        std::cout << std::endl;
        std::cout << "Retour de la minute morte." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.setSpecialEffect(delayedWounds / 2);
        std::cout << "Une partie des blessures retardées vient d'être payée." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 13)
    {
        int profanationState = boss.getSpecialEffect();
        bool golemAwakened = profanationState >= 100;
        int profanation = golemAwakened ? profanationState - 100 : profanationState;

        if (golemAwakened)
        {
            int damage = 34 + profanation * 6 + random.between(8, 20);

            std::cout << "Le golem d'os et de sable ouvre sa cage thoracique." << std::endl;
            std::cout << "Au centre, l'Enfant dort presque, fusionné avec le coeur du colosse." << std::endl;
            std::cout << "Chaque tombe autour de toi se relève dans le même mouvement." << std::endl;
            std::cout << std::endl;
            std::cout << "Tombeau du géant intérieur." << std::endl;
            std::cout << std::endl;

            applyDirectDamage(player, damage);
            std::cout << "Le second rythme du combat écrase l'arène sous des pas de sable et d'os." << std::endl;
        }
        else
        {
            int damage = 20 + profanation * 5 + random.between(4, 14);

            std::cout << "L'Enfant ouvre la bouche. Aucune voix ne sort." << std::endl;
            std::cout << "Pourtant, sous tes pieds, les morts chantent." << std::endl;
            std::cout << std::endl;
            std::cout << "Berceuse des enterrés." << std::endl;
            std::cout << std::endl;

            applyDirectDamage(player, damage);
            std::cout << "La Profanation rend la berceuse plus lourde à chaque faute." << std::endl;
        }

        std::cout << std::endl;
    }
    else if (boss.getBossId() == 14)
    {
        int warGauge = boss.getSpecialEffect();
        int damage = 30 + warGauge * 4 + random.between(8, 22);

        std::cout << "L'arme de Boros frappe le sol." << std::endl;
        std::cout << "Le bruit ne ressemble pas à du métal." << std::endl;
        std::cout << "Il ressemble à mille batailles qui recommencent en même temps." << std::endl;
        std::cout << std::endl;
        std::cout << "Hymne du carnage." << std::endl;
        std::cout << std::endl;

        if (player.isInDefensePosture())
        {
            damage = damage * 70 / 100;
            std::cout << "Ta posture absorbe une partie du choc guerrier." << std::endl;
        }

        applyDirectDamage(player, damage);

        if (warGauge >= 9)
        {
            boss.takeDamage(8 + warGauge);
            std::cout << "La Jauge de guerre était si haute que l'avatar subit aussi le contrecoup du carnage." << std::endl;
        }

        std::cout << std::endl;
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

        std::cout << "Des fils invisibles s'enroulent autour de toi." << std::endl;
        std::cout << "Ils ne coupent pas la peau. Ils serrent ce que tu refuses de perdre." << std::endl;
        std::cout << "Anastasia sent aussi combien de fois tu t'es accroché à la vie." << std::endl;
        std::cout << std::endl;
        std::cout << "Serment qui saigne." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Les soins paraissent plus lourds, comme si survivre avait désormais un prix émotionnel." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 16)
    {
        int verdict = boss.getSpecialEffect();
        int damage = 28 + verdict * 5 + random.between(6, 18);

        std::cout << "Lexior prononce une phrase sans hausser la voix." << std::endl;
        std::cout << "Le combat cesse de ressembler à un duel. Il ressemble à un procès." << std::endl;
        std::cout << std::endl;
        std::cout << "Verdict des actes répétés." << std::endl;
        std::cout << std::endl;

        Player* concretePlayer = dynamic_cast<Player*>(&player);
        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            damage += 20;
            std::cout << "Les altérations du personnage sont ajoutées au dossier." << std::endl;
        }

        applyDirectDamage(player, damage);
        std::cout << "Tant que le verdict tient, l'équipement scellé répondra mal." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 17)
    {
        int dreamPressure = boss.getSpecialEffect();
        int damage = 24 + dreamPressure * 4 + random.between(6, 18);

        std::cout << "La lune disparaît, puis revient derrière tes paupières." << std::endl;
        std::cout << "Luna apaise le monde. Onyrae transforme ce calme en piège." << std::endl;
        std::cout << std::endl;
        std::cout << "Éclipse du rêve et du cauchemar." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        if (random.between(1, 100) <= 45)
        {
            boss.heal(16 + dreamPressure * 2);
            std::cout << "Une partie du rêve revient dans le fragment au lieu de te laisser une ouverture." << std::endl;
        }
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 18)
    {
        int instability = boss.getSpecialEffect();

        if (random.between(1, 100) <= 50)
        {
            int damage = 32 + instability * 5 + random.between(8, 22);
            std::cout << "Les quatre éléments cessent de se contredire." << std::endl;
            std::cout << "Ils fusionnent en une seule pression impossible." << std::endl;
            std::cout << std::endl;
            std::cout << "Fusion élémentaire supérieure." << std::endl;
            std::cout << std::endl;
            applyDirectDamage(player, damage);
        }
        else
        {
            int totalDamage = 24 + instability * 4 + random.between(8, 20);
            int quarterDamage = totalDamage / 4;
            if (quarterDamage < 1) quarterDamage = 1;

            std::cout << "Syvaranelya se sépare en quatre présences." << std::endl;
            std::cout << "Feu. Terre. Eau. Vent." << std::endl;
            std::cout << "Pendant un instant, le combat devient un faux 1 contre 4." << std::endl;
            std::cout << "Chaque partie ne porte qu'un quart du corps, mais chaque blessure résonne chez toutes." << std::endl;
            std::cout << std::endl;
            std::cout << "Quadrature élémentaire." << std::endl;
            std::cout << std::endl;
            std::cout << "Les dégâts sont divisés entre quatre formes : " << quarterDamage << " x4." << std::endl;
            applyDirectDamage(player, quarterDamage * 4);
        }

        std::cout << std::endl;
    }
    else if (boss.getBossId() == 19)
    {
        int adaptation = boss.getSpecialEffect();
        int damage = 26 + adaptation * 5 + random.between(6, 18);

        std::cout << "Hitogami ferme les yeux." << std::endl;
        std::cout << "Pendant une seconde, tu crois qu'il abandonne." << std::endl;
        std::cout << "Puis il reprend ta posture. Ton rythme. Ton souffle." << std::endl;
        std::cout << std::endl;
        std::cout << "L'humain recommence." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.setSpecialEffect(adaptation + 2);
        std::cout << "L'écho humain a appris une partie de ta manière de survivre." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 20)
    {
        int luck = boss.getSpecialEffect();

        if (random.between(1, 100) <= 50)
        {
            int damage = 24 + luck * 4 + random.between(8, 22);
            std::cout << "Trois futurs se dessinent devant Sérendys." << std::endl;
            std::cout << "Dans le premier, elle tombe. Dans le second, tu souris." << std::endl;
            std::cout << "Dans le troisième, elle efface les deux autres." << std::endl;
            std::cout << std::endl;
            std::cout << "Le destin garde la meilleure issue." << std::endl;
            std::cout << std::endl;
            applyDirectDamage(player, damage);
            if (random.between(1, 100) <= 45)
            {
                boss.heal(12 + luck * 2);
                std::cout << "Le futur choisi lui rend aussi une partie de sa chance." << std::endl;
            }
        }
        else
        {
            int damage = 18 + luck * 3 + random.between(6, 16);
            boss.setSpecialEffect(luck + 4);
            std::cout << "Sérendys retourne une pièce qui ne devrait avoir que deux faces." << std::endl;
            std::cout << "Ce qui était rare devient commun. Ce qui était commun devient rare." << std::endl;
            std::cout << "Pendant deux tours, les effets absurdes arrivent trop facilement et les évidences deviennent capricieuses." << std::endl;
            std::cout << std::endl;
            std::cout << "Inversion des probabilités." << std::endl;
            std::cout << std::endl;
            applyDirectDamage(player, damage);
        }

        std::cout << std::endl;
    }
    else if (boss.getBossId() == 21)
    {
        int threshold = boss.getSpecialEffect();
        int damage = 28 + threshold * 4 + random.between(6, 18);

        std::cout << "Le Gardien lève une main." << std::endl;
        std::cout << "L'espace derrière toi disparaît. Celui devant toi se ferme." << std::endl;
        std::cout << "Il ne reste qu'une seule question : étais-tu prêt ?" << std::endl;
        std::cout << std::endl;
        std::cout << "Le Seuil se referme." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Le prochain verrou dépendra encore de la façon dont tu réponds à l'épreuve." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 22)
    {
        int authority = boss.getSpecialEffect();
        int damage = 30 + authority * 4 + random.between(8, 22);

        std::cout << "Des murs invisibles se dressent." << std::endl;
        std::cout << "Un trône sans matière apparaît derrière le Roi Sans Salle." << std::endl;
        std::cout << "La cour est morte. Mais elle s'agenouille encore." << std::endl;
        std::cout << std::endl;
        std::cout << "Couronnement des ruines." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.heal(16 + authority * 2);
        std::cout << "Pendant quelques tours, chaque ordre royal sera renforcé par les ruines." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 23)
    {
        int instinct = boss.getSpecialEffect();
        int damage = 34 + instinct * 5 + random.between(8, 24);

        std::cout << "La Bête ouvre la gueule." << std::endl;
        std::cout << "Aucun son ne devrait sortir. Parce qu'aucun nom ne devrait l'appeler." << std::endl;
        std::cout << "Pourtant, le monde tremble." << std::endl;
        std::cout << std::endl;
        std::cout << "Hurlement sans nom." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Le registre brouille une partie des informations. Cette Bête ne ressemblera peut-être pas à la prochaine." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 24)
    {
        int abyss = boss.getSpecialEffect();
        int damage = 32 + abyss * 5 + random.between(8, 24);

        std::cout << "Aldebaroth sourit comme si le pire du monde venait de retrouver son propriétaire." << std::endl;
        std::cout << "La haine, la peur et la rancune se condensent autour de lui." << std::endl;
        std::cout << std::endl;
        std::cout << "Abîme des fautes accumulées." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.heal(18 + abyss * 2);
        std::cout << "Il ne réagit pas aux altérations de triche. Ce n'est pas son domaine." << std::endl;
        std::cout << "Il se nourrit seulement du négatif laissé par le combat." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 25)
    {
        int twinState = boss.getSpecialEffect();
        int damage = 30 + twinState * 4 + random.between(8, 22);

        std::cout << "Asterion crée une forme parfaite." << std::endl;
        std::cout << "Nihilon la détruit avant qu'elle ait le temps d'exister complètement." << std::endl;
        std::cout << "Le choc entre création et destruction traverse l'arène." << std::endl;
        std::cout << std::endl;
        std::cout << "Cycle impossible." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.setSpecialEffect(twinState + 2);
        std::cout << "Si l'un des deux tombe vraiment, il ne restera que cinq tours pour empêcher la reconstruction fatale." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 26)
    {
        int authority = boss.getSpecialEffect();
        int damage = 34 + authority * 5 + random.between(10, 26);

        std::cout << "Obérion ne descend pas dans l'arène." << std::endl;
        std::cout << "Seul un écho fragmenté de son autorité accepte de te regarder." << std::endl;
        std::cout << std::endl;
        std::cout << "Origine du monde étagé." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);

        Player* concretePlayer = dynamic_cast<Player*>(&player);
        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            std::cout << "Les altérations sont pesées sans colère." << std::endl;
            std::cout << "Une loi ancienne refuse que les raccourcis remplacent l'existence." << std::endl;
            applyDirectDamage(player, 18 + concretePlayer->getLethalCheatAttemptCount() * 8);
        }

        std::cout << "L'écho peut forcer un duel nu : sans armure, sans arme, seulement le corps et la volonté." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 27)
    {
        int authority = boss.getSpecialEffect();
        bool devLimit = random.between(1, 100) <= 45;

        std::cout << "Une fenêtre invisible s'ouvre dans l'air." << std::endl;
        std::cout << "FireFlight ne lève pas son arme. Il lève la main vers le monde." << std::endl;
        std::cout << std::endl;

        if (devLimit)
        {
            std::cout << "Commande développeur : Limite imposée." << std::endl;
            std::cout << "Tu voulais une victoire par automatisme. Je préfère une victoire méritée." << std::endl;
            std::cout << "Pendant quelques tours, la stratégie dominante devient moins fiable." << std::endl;
            applyDirectDamage(player, 24 + authority * 3 + random.between(6, 16));
        }
        else
        {
            std::cout << "Patch d'urgence." << std::endl;
            std::cout << "Je n'ai pas créé ce monde pour qu'il soit juste." << std::endl;
            std::cout << "Je l'ai créé pour qu'il survive à ceux qui essaient de le briser." << std::endl;
            applyDirectDamage(player, 32 + authority * 4 + random.between(8, 24));
            boss.heal(20 + authority * 2);
        }

        std::cout << "L'interface cligne une fois, comme si quelqu'un venait de sauvegarder une règle au-dessus de toi." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 28)
    {
        int breath = boss.getSpecialEffect();
        int damage = 30 + breath * 5 + random.between(8, 22);

        std::cout << "Le Souffle sans Visage cesse d'inspirer." << std::endl;
        std::cout << "Pendant une seconde, tout l'air du combat appartient à l'entité." << std::endl;
        std::cout << std::endl;
        std::cout << "Apnée du monde." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Les prochains tours semblent plus courts, comme si respirer demandait maintenant une action entière." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 29)
    {
        int nails = boss.getSpecialEffect();
        int damage = 28 + nails * 5 + random.between(8, 20);

        std::cout << "Tous les fils se tendent d'un seul coup." << std::endl;
        std::cout << "La Marionnette n'avance pas vers toi : elle te tire vers elle." << std::endl;
        std::cout << std::endl;
        std::cout << "Théâtre des Mille Clous." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.heal(8 + nails * 2);
        std::cout << "Chaque clou planté dans le rythme du combat recoud une partie de son corps." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 30)
    {
        int line = boss.getSpecialEffect();
        Player* concretePlayer = dynamic_cast<Player*>(&player);
        int damage = 32 + line * 5 + random.between(10, 24);

        std::cout << "Moiran ferme les yeux." << std::endl;
        std::cout << "Toutes les routes possibles s'affichent une fraction de seconde." << std::endl;
        std::cout << "Puis il en coupe une." << std::endl;
        std::cout << std::endl;
        std::cout << "Fil rompu du Destin." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            std::cout << "Les altérations ne changent pas le destin : elles lui donnent seulement plus de preuves." << std::endl;
            applyDirectDamage(player, 10 + concretePlayer->getLethalCheatAttemptCount() * 6);
        }
        std::cout << "Pendant quelques tours, les choix qui semblaient sûrs deviennent suspects." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 31)
    {
        int souls = boss.getSpecialEffect();
        int damage = 30 + souls * 4 + random.between(8, 22);

        std::cout << "Le Cerf baisse les bois." << std::endl;
        std::cout << "Les lanternes des morts s'allument une par une." << std::endl;
        std::cout << "Aucune voix ne crie. C'est pire : elles te reconnaissent." << std::endl;
        std::cout << std::endl;
        std::cout << "Procession des Âmes Égarées." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        boss.heal(12 + souls * 2);
        std::cout << "Le Cerf ne veut pas seulement gagner : il veut ramener les âmes perdues hors de ta route." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 28)
    {
        int breath = boss.getSpecialEffect();
        int damage = 30 + breath * 4 + random.between(8, 22);

        std::cout << "Le Souffle sans Visage efface la distance entre inspiration et noyade." << std::endl;
        std::cout << "L'air devient un couloir trop étroit pour ton propre corps." << std::endl;
        std::cout << std::endl;
        std::cout << "Asphyxie d'identité." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        std::cout << "Pendant quelques tours, respirer ressemble à une action volontaire." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 29)
    {
        int nails = boss.getSpecialEffect();
        int damage = 28 + nails * 5 + random.between(8, 24);

        std::cout << "La Marionnette aux Mille Clous cesse de bouger." << std::endl;
        std::cout << "Tous les fils, eux, continuent." << std::endl;
        std::cout << std::endl;
        std::cout << "Théâtre des douleurs suspendues." << std::endl;
        std::cout << std::endl;

        if (player.isInDefensePosture())
        {
            damage = damage * 80 / 100;
            std::cout << "Ta posture retient une partie des fils, mais chaque clou tire dans une direction différente." << std::endl;
        }

        applyDirectDamage(player, damage);
        boss.heal(10 + nails);
        std::cout << "Une partie de ta douleur revient dans la marionnette sous forme de réparation sale." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 30)
    {
        int line = boss.getSpecialEffect();
        int damage = 32 + line * 5 + random.between(10, 26);
        Player* concretePlayer = dynamic_cast<Player*>(&player);

        std::cout << "Moiran ferme les yeux." << std::endl;
        std::cout << "Le combat continue. Mais quelque chose vient de se terminer ailleurs." << std::endl;
        std::cout << "Une version de toi tombe. Une autre n'a jamais existé." << std::endl;
        std::cout << std::endl;
        std::cout << "La Fin qui te reconnaît." << std::endl;
        std::cout << std::endl;

        if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats())
        {
            damage += 18;
            std::cout << "Ta ligne de destinée porte une cicatrice d'altération. Moiran la suit du doigt." << std::endl;
        }

        if (concretePlayer != nullptr && concretePlayer->getLethalCheatAttemptCount() > 0)
        {
            damage += concretePlayer->getLethalCheatAttemptCount() * 12;
            std::cout << "Chemin refusé." << std::endl;
            std::cout << "Cette route n'a jamais été acceptée. Elle frappe maintenant avec ses intérêts." << std::endl;
        }

        applyDirectDamage(player, damage);
        std::cout << "Effet narratif : pendant deux tours, répéter exactement la même réponse devient plus dangereux." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 31)
    {
        int souls = boss.getSpecialEffect();
        int damage = 26 + souls * 4 + random.between(8, 20);

        std::cout << "Le Cerf baisse ses bois." << std::endl;
        std::cout << "Les lanternes d'âmes s'ouvrent une à une, sans flamme." << std::endl;
        std::cout << std::endl;
        std::cout << "Procession des âmes égarées." << std::endl;
        std::cout << std::endl;

        if (player.isInDefensePosture())
        {
            damage = damage * 70 / 100;
            std::cout << "Ta retenue calme une partie de la procession." << std::endl;
        }
        else
        {
            std::cout << "Chaque âme traverse ton corps comme si elle cherchait encore une sortie." << std::endl;
        }

        applyDirectDamage(player, damage);
        boss.heal(8 + souls * 2);
        std::cout << "Le Cerf récupère les regrets qui n'ont pas trouvé de tombe." << std::endl;
        std::cout << std::endl;
    }

    else if (boss.getBossId() == 32)
    {
        int fury = boss.getSpecialEffect();
        int damage = 34 + fury * 5 + random.between(10, 26);

        std::cout << "Gorvald lève sa hache comme s'il soulevait tout son royaume." << std::endl;
        std::cout << "Derrière lui, tu n'entends pas une armée. Tu entends une nation frapper son torse." << std::endl;
        std::cout << std::endl;
        std::cout << "Couronne de sang orc." << std::endl;
        std::cout << std::endl;

        if (player.isInDefensePosture())
        {
            damage = damage * 70 / 100;
            std::cout << "Ta posture encaisse l'impact. Gorvald respecte la garde, mais pas la faiblesse." << std::endl;
        }
        applyDirectDamage(player, damage);
        boss.setSpecialEffect(fury / 2);
        std::cout << "Une partie de sa fureur royale se consume dans l'attaque." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 33)
    {
        int thirst = boss.getSpecialEffect();
        int damage = 30 + thirst * 5 + random.between(8, 24);

        std::cout << "La lumière de l'arène devient rouge." << std::endl;
        std::cout << "Serana ne bondit pas. Elle invite. Ton sang répond avant toi." << std::endl;
        std::cout << std::endl;
        std::cout << "Banquet de la nuit rouge." << std::endl;
        std::cout << std::endl;

        applyDirectDamage(player, damage);
        int heal = 18 + thirst * 4;
        boss.heal(heal);
        std::cout << "Serana récupère " << heal << " PV en transformant la blessure en repas royal." << std::endl;
        std::cout << "Tes soins semblent moins rassurants pendant quelques tours." << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 34)
    {
        int web = boss.getSpecialEffect();
        int damage = 28 + web * 4 + random.between(8, 22);

        std::cout << "Les murs disparaissent sous la soie." << std::endl;
        std::cout << "Le plafond descend. Le sol colle à tes pas." << std::endl;
        std::cout << std::endl;
        std::cout << "Royaume de soie noire." << std::endl;
        std::cout << std::endl;

        if (player.isInDefensePosture())
        {
            damage = damage * 85 / 100;
            std::cout << "Ta posture limite les morsures, mais la toile entre dans chaque angle mort." << std::endl;
        }
        applyDirectDamage(player, damage);
        boss.setSpecialEffect(web + 2);
        std::cout << "Toile de reine renforcée : " << boss.getSpecialEffect() << std::endl;
        std::cout << std::endl;
    }
    else if (boss.getBossId() == 35)
    {
        int mirror = boss.getSpecialEffect();
        bool secondUltimate = random.between(1, 100) <= 45;
        int damage = 32 + mirror * 5 + random.between(10, 26);

        if (secondUltimate)
        {
            std::cout << "Ton reflet bouge avant toi." << std::endl;
            std::cout << "Il lève ton arme. Il porte ton visage. Mais son sourire ne t'appartient pas." << std::endl;
            std::cout << std::endl;
            std::cout << "Reflet qui prend ta place." << std::endl;
            boss.heal(14 + mirror * 2);
            damage += 10;
        }
        else
        {
            std::cout << "Le miroir se fend encore." << std::endl;
            std::cout << "Dans chaque morceau, tu gagnes. Dans chaque morceau, tu perds." << std::endl;
            std::cout << std::endl;
            std::cout << "Futur dans le miroir." << std::endl;
        }

        std::cout << std::endl;
        if (player.isInDefensePosture())
        {
            damage = damage * 80 / 100;
            std::cout << "Tu refuses de suivre le premier reflet. Une partie du futur se casse avant de te toucher." << std::endl;
        }
        applyDirectDamage(player, damage);
        std::cout << "Les Jumelles te laissent avec une question : as-tu évité le mensonge, ou seulement préféré l'autre ?" << std::endl;
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
