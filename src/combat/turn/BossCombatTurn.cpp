// EN: BossCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/BossCombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/BossCombat.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/ai/AIAction.hpp"

#include "core/Console.hpp"
#include "entity/Player.hpp"

#include <iostream>
#include <map>
#include <string>

namespace
{

    std::string getMostUsedEquipmentLabel(const Player& player)
    {
        std::map<std::string, int> counts;
        const std::vector<std::string>& usage = player.getRecentCombatEquipmentUsage();

        for (const std::string& label : usage)
        {
            if (!label.empty()) counts[label]++;
        }

        if (player.hasEquippedWeapon() || player.hasEquippedArmor())
        {
            std::string current;
            if (player.hasEquippedWeapon()) current += "Arme:" + player.getEquippedWeapon().getName();
            if (player.hasEquippedArmor())
            {
                if (!current.empty()) current += " | ";
                current += "Armure:" + player.getEquippedArmor().getName();
            }
            if (!current.empty()) counts[current] += 2;
        }

        std::string best = "ton équipement principal";
        int bestCount = 0;
        for (const auto& entry : counts)
        {
            if (entry.second > bestCount)
            {
                best = entry.first;
                bestCount = entry.second;
            }
        }

        return best;
    }

    // EN: applyAlteredHunterPressure declares or implements a focused behavior used by this module.
    // FR: applyAlteredHunterPressure déclare ou implémente un comportement précis utilisé par ce module.
    void applyAlteredHunterPressure(Boss& boss, Entity& player, Random& random)
    {
        Player* concretePlayer = dynamic_cast<Player*>(&player);
        if (concretePlayer == nullptr || !concretePlayer->isAlteredByCheats()) return;

        if ((boss.getBossId() == 11 || boss.getBossId() == 16 || boss.getBossId() == 26 || boss.getBossId() == 27) && random.between(1, 100) <= 35)
        {
            int punishment = 8 + concretePlayer->getLevel() / 2;
            if (concretePlayer->isGodModeEnabled()) punishment += 10;
            if (concretePlayer->hasInfiniteConsumables()) punishment += 6;
            if (concretePlayer->hasIndestructibleEquipment()) punishment += 6;
            if (concretePlayer->hasEquipmentProtection()) punishment += 6;
            if (concretePlayer->hasStorySkip()) punishment += 8;

            std::string hunterName = "Lexior";
            if (boss.getBossId() == 11) hunterName = "L'Anomalie";
            else if (boss.getBossId() == 26) hunterName = "Obérion";
            else if (boss.getBossId() == 27) hunterName = "FireFlight";

            std::cout << hunterName << " lit les altérations du personnage." << std::endl;
            std::cout << "Les codes ne te protègent pas ici. Ils servent de preuve." << std::endl;
            std::cout << "Réponse anti-altération : " << punishment << " dégâts." << std::endl;
            player.takeDamage(punishment);
            std::cout << player.getName() << " possède maintenant " << player.getHp() << "/" << player.getMaxHp() << " PV." << std::endl;
            std::cout << std::endl;
        }
    }
    // EN: isGoblinLikePlayer declares or implements a focused behavior used by this module.
    // FR: isGoblinLikePlayer déclare ou implémente un comportement précis utilisé par ce module.
    bool isGoblinLikePlayer(const Entity& player)
    {
        const Player* concretePlayer = dynamic_cast<const Player*>(&player);
        if (concretePlayer == nullptr) return false;
        std::string raceText = concretePlayer->getRaceText();
        return raceText.find("Gobelin") != std::string::npos || raceText.find("gobelin") != std::string::npos;
    }


    // EN: describeNamelessBeastVariant declares or implements a focused behavior used by this module.
    // FR: describeNamelessBeastVariant déclare ou implémente un comportement précis utilisé par ce module.
    void describeNamelessBeastVariant(Random& random)
    {
        int variant = random.between(1, 5);

        std::cout << "Le registre tente de décrire la créature." << std::endl;
        if (variant == 1)
        {
            std::cout << "Cette fois, elle avance sur quatre pattes trop longues, couverte de poils blancs et d'os apparents." << std::endl;
            std::cout << "Ses yeux ressemblent à des trous dans une page arrachée." << std::endl;
        }
        else if (variant == 2)
        {
            std::cout << "Cette fois, elle rampe comme une masse noire, avec des bois de cerf et une mâchoire de requin." << std::endl;
            std::cout << "Chaque souffle change la forme de son dos." << std::endl;
        }
        else if (variant == 3)
        {
            std::cout << "Cette fois, elle ressemble presque à un oiseau géant, sauf que ses ailes sont faites de doigts." << std::endl;
            std::cout << "Le sol refuse d'enregistrer ses empreintes." << std::endl;
        }
        else if (variant == 4)
        {
            std::cout << "Cette fois, elle porte une carapace de pierre humide et une queue qui semble venir d'un autre animal." << std::endl;
            std::cout << "Le bestiaire écrit puis efface son espèce trois fois de suite." << std::endl;
        }
        else
        {
            std::cout << "Cette fois, elle semble presque humaine de loin." << std::endl;
            std::cout << "Puis elle bouge, et ton cerveau abandonne cette comparaison." << std::endl;
        }
        std::cout << std::endl;
    }

    // EN: executeNewBossPassiveAction declares or implements a focused behavior used by this module.
    // FR: executeNewBossPassiveAction déclare ou implémente un comportement précis utilisé par ce module.
    void executeNewBossPassiveAction(Boss& boss, Entity& player, Random& random)
    {
        if (boss.getBossId() == 1 && random.between(1, 100) <= 30)
        {
            int judgment = boss.getSpecialEffect() + 1;
            if (judgment > 5) judgment = 5;
            boss.setSpecialEffect(judgment);

            std::cout << "Fitoria ne lève pas son arme. Elle lève les yeux vers toi." << std::endl;
            std::cout << "La lumière ne te frappe pas encore. Elle te mesure." << std::endl;
            std::cout << "Jugement lumineux : " << judgment << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 2 && random.between(1, 100) <= 35)
        {
            int corruption = boss.getSpecialEffect();
            if (corruption < 1) corruption = 1;
            boss.setSpecialEffect(corruption);

            int damage = 5 + random.between(0, 5);
            player.takeDamage(damage);
            boss.heal(damage / 2);

            std::cout << "Le sang de Zelef touche le sol." << std::endl;
            std::cout << "Il ne coule pas. Il rampe vers toi." << std::endl;
            std::cout << player.getName() << " subit " << damage << " dégâts de corruption légère." << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 3 && random.between(1, 100) <= 35)
        {
            int fissures = boss.getSpecialEffect() + 1;
            if (fissures > 5) fissures = 5;
            boss.setSpecialEffect(fissures);

            std::cout << "Une plaque de l'armure d'Atlas se fissure." << std::endl;
            std::cout << "Chaque faille retire un peu de défense, mais prépare une réponse plus lourde." << std::endl;
            std::cout << "Fissures visibles : " << fissures << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 4 && random.between(1, 100) <= 35)
        {
            int mark = boss.getSpecialEffect() + 1;
            if (mark > 5) mark = 5;
            boss.setSpecialEffect(mark);

            std::cout << "L'Écho de Lyknir baisse la tête." << std::endl;
            std::cout << "Ses yeux ne regardent plus ton corps. Ils regardent ta fuite." << std::endl;
            std::cout << "Marque de proie : " << mark << std::endl;
            std::cout << std::endl;

            if (mark >= 3)
            {
                int damage = 8 + mark * 3;
                player.takeDamage(damage);
                std::cout << "La meute mord depuis les angles morts et inflige " << damage << " dégâts." << std::endl;
                std::cout << player.getName() << " possède maintenant "
                          << player.getHp() << "/" << player.getMaxHp() << " PV." << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 5 && random.between(1, 100) <= 40)
        {
            int addedDebt = random.between(8, 18);
            int debt = boss.getSpecialEffect() + addedDebt;
            boss.setSpecialEffect(debt);

            std::cout << "Grinka claque des doigts." << std::endl;
            std::cout << "Des collecteurs gobelins surgissent avec des sacs plus grands qu'eux." << std::endl;
            std::cout << "Ils ne savent pas se battre. Ils savent facturer." << std::endl;
            std::cout << "Dette gobeline actuelle : " << debt << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 6 && random.between(1, 100) <= 30)
        {
            int darkness = boss.getSpecialEffect() + random.between(1, 2);
            boss.setSpecialEffect(darkness);

            std::cout << "L'ombre derrière l'avatar ne suit pas ses mouvements." << std::endl;
            std::cout << "Elle grandit." << std::endl;
            std::cout << "Charges d'obscurité : " << darkness << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 7 && random.between(1, 100) <= 35)
        {
            int scales = boss.getSpecialEffect() + 1;
            if (scales > 5) scales = 5;
            boss.setSpecialEffect(scales);

            std::cout << "Les écailles du Fragment de Thamarys changent lentement de reflet." << std::endl;
            std::cout << "Le dragon apprend la forme de tes attaques." << std::endl;
            std::cout << "Adaptation draconique : " << scales << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 8 && random.between(1, 100) <= 40)
        {
            int memory = boss.getSpecialEffect();

            if (player.isInDefensePosture())
            {
                memory -= 1;
                if (memory < 0) memory = 0;
                std::cout << "Mojo observe ta posture défensive." << std::endl;
                std::cout << "La forêt ne voit pas une provocation. Elle voit de la retenue." << std::endl;
            }
            else
            {
                memory += 1;
                if (memory > 6) memory = 6;
                std::cout << "Les feuilles frémissent autour de Mojo." << std::endl;
                std::cout << "La forêt grave un nouveau souvenir de ce combat." << std::endl;
            }

            boss.setSpecialEffect(memory);
            std::cout << "Mémoire de la forêt : " << memory << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 9 && random.between(1, 100) <= 38)
        {
            int mirrors = boss.getSpecialEffect() + 1;
            if (mirrors > 5) mirrors = 5;
            boss.setSpecialEffect(mirrors);

            std::cout << "Inakari rit sans ouvrir la bouche." << std::endl;
            std::cout << "Un reflet de plus apparaît dans l'arène." << std::endl;
            std::cout << "Reflets menteurs : " << mirrors << std::endl;
            std::cout << std::endl;
        }


        if (boss.getBossId() == 10 && random.between(1, 100) <= 38)
        {
            int judgment = boss.getSpecialEffect() + 1;
            if (judgment > 6) judgment = 6;
            boss.setSpecialEffect(judgment);

            std::cout << "Le Jugement Silencieux ne parle pas." << std::endl;
            std::cout << "Pourtant, ton ombre semble répondre à sa place." << std::endl;
            std::cout << "Charges de jugement muet : " << judgment << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 11 && random.between(1, 100) <= 45)
        {
            int corruption = boss.getSpecialEffect() + random.between(1, 2);
            if (corruption > 9) corruption = 9;
            boss.setSpecialEffect(corruption);

            std::cout << "========== C0M8@T ==========" << std::endl;
            std::cout << "1 : ▓▒░█§╬?//" << std::endl;
            std::cout << "2 : @@@_ERREUR_MÉMOIRE" << std::endl;
            std::cout << "3 : TU N'ÉTAIS PAS CENSÉ VOIR ÇA" << std::endl;
            std::cout << "L'Anomalie remplace le texte, mais pas toujours les règles." << std::endl;
            std::cout << "Corruption d'interface : " << corruption << std::endl;
            std::cout << std::endl;

            if (random.between(1, 100) <= 22)
            {
                int healAmount = 10 + corruption * 2;
                boss.heal(healAmount);
                std::cout << "Correction..." << std::endl;
                std::cout << "PV de l'Anomalie corrigés de +" << healAmount << "." << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 12 && random.between(1, 100) <= 40)
        {
            int delayedWounds = boss.getSpecialEffect() + random.between(2, 5);
            if (delayedWounds > 24) delayedWounds = 24;
            boss.setSpecialEffect(delayedWounds);

            std::cout << "L'Horloge des Chuchotements avance sans bouger." << std::endl;
            std::cout << "Une blessure que tu n'as pas encore reçue vient d'être enregistrée." << std::endl;
            std::cout << "Blessures retardées : " << delayedWounds << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 13)
        {
            int profanation = boss.getSpecialEffect();
            bool golemAwakened = profanation >= 100;

            if (!golemAwakened && boss.getHp() <= boss.getMaxHp() / 4)
            {
                boss.setSpecialEffect(100 + profanation);
                golemAwakened = true;

                std::cout << "Le sable s'ouvre sous les os." << std::endl;
                std::cout << "Un golem d'os et de sable surgit derrière l'Enfant." << std::endl;
                std::cout << "L'Enfant ne grimpe pas dedans. Il disparaît dans son coeur." << std::endl;
                std::cout << "Le rythme du combat change complètement." << std::endl;
                std::cout << "Un second ultime vient d'être débloqué." << std::endl;
                std::cout << std::endl;
            }
            else if (random.between(1, 100) <= 40)
            {
                int baseProfanation = golemAwakened ? profanation - 100 : profanation;
                if (player.isInDefensePosture())
                {
                    baseProfanation -= 1;
                    if (baseProfanation < 0) baseProfanation = 0;
                    std::cout << "L'Enfant des Os voit ta retenue. Les tombes restent calmes, pour l'instant." << std::endl;
                }
                else
                {
                    baseProfanation += 1;
                    if (baseProfanation > 8) baseProfanation = 8;
                    std::cout << "Un os craque sous tes pas. Le lieu se souvient de l'affront." << std::endl;
                }

                boss.setSpecialEffect((golemAwakened ? 100 : 0) + baseProfanation);
                std::cout << "Profanation : " << baseProfanation << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 14 && random.between(1, 100) <= 42)
        {
            int warGauge = boss.getSpecialEffect() + random.between(1, 3);
            if (warGauge > 12) warGauge = 12;
            boss.setSpecialEffect(warGauge);

            std::cout << "L'avatar de Boros sourit, pas par plaisir : par reconnaissance." << std::endl;
            std::cout << "Le combat devient enfin un vrai champ de bataille." << std::endl;
            std::cout << "Jauge de guerre : " << warGauge << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 15 && random.between(1, 100) <= 42)
        {
            int links = boss.getSpecialEffect() + 1;
            if (links > 10) links = 10;
            boss.setSpecialEffect(links);

            std::cout << "Anastasia parle doucement, mais ses mots serrent quelque chose en toi." << std::endl;
            std::cout << "Elle ne frappe pas seulement le corps. Elle touche ce que tu refuses de perdre." << std::endl;
            std::cout << "Liens douloureux : " << links << std::endl;
            std::cout << std::endl;
        }


        if (boss.getBossId() == 16)
        {
            Player* concretePlayer = dynamic_cast<Player*>(&player);
            if (concretePlayer != nullptr && boss.getHp() <= boss.getMaxHp() / 2 && !concretePlayer->hasBossEquipmentSeal())
            {
                std::string sealedLabel = getMostUsedEquipmentLabel(*concretePlayer);
                concretePlayer->activateBossEquipmentSeal("Verdict de Lexior : " + sealedLabel + " ne répond plus correctement.");
                std::cout << "Lexior ouvre un registre que tu n'as jamais rempli volontairement." << std::endl;
                std::cout << "Il y retrouve le stuff le plus utilisé dans tes dix derniers combats... et celui que tu portes maintenant." << std::endl;
                std::cout << "Verdict : " << sealedLabel << " est scellé pour ce combat." << std::endl;
                std::cout << std::endl;
            }

            if (random.between(1, 100) <= 42)
            {
                int verdict = boss.getSpecialEffect() + 1;
                if (verdict > 8) verdict = 8;
                boss.setSpecialEffect(verdict);
                std::cout << "Lexior compte tes répétitions, tes soins forcés, tes coups inutiles et tes esquives heureuses." << std::endl;
                std::cout << "Charges de verdict : " << verdict << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 17 && random.between(1, 100) <= 42)
        {
            int dreamPressure = boss.getSpecialEffect() + random.between(1, 2);
            if (dreamPressure > 10) dreamPressure = 10;
            boss.setSpecialEffect(dreamPressure);
            std::cout << "La lune se reflète dans un sol qui n'existe pas." << std::endl;
            std::cout << "Onyrae transforme une pensée en cauchemar, puis Luna la recouvre de silence." << std::endl;
            std::cout << "Pression rêve/cauchemar : " << dreamPressure << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 18 && random.between(1, 100) <= 45)
        {
            int elementalState = boss.getSpecialEffect() + 1;
            if (elementalState > 12) elementalState = 12;
            boss.setSpecialEffect(elementalState);
            std::cout << "Syvaranelya change de peau élémentaire." << std::endl;
            std::cout << "Feu, Terre, Eau et Vent ne se remplacent pas : ils s'empilent." << std::endl;
            std::cout << "Instabilité élémentaire : " << elementalState << std::endl;
            std::cout << std::endl;
        }


        if (boss.getBossId() == 19)
        {
            Player* concretePlayer = dynamic_cast<Player*>(&player);
            int adaptation = boss.getSpecialEffect();

            if (random.between(1, 100) <= 45)
            {
                adaptation += 1;

                std::cout << "Hitogami t'observe avec une normalité presque insultante." << std::endl;
                std::cout << "Les monstres naissent forts. Les humains apprennent à le devenir." << std::endl;

                if (concretePlayer != nullptr && concretePlayer->getRace() == CharacterRace::Human)
                {
                    std::cout << "Face à un humain, son regard devient presque compatissant." << std::endl;
                    std::cout << "Tu n'es pas spécial parce que tu tombes. Tu es spécial parce que tu te relèves." << std::endl;
                    if (adaptation > 0) adaptation -= 1;
                }
                else if (isGoblinLikePlayer(player))
                {
                    std::cout << "Son expression se durcit en voyant une race gobeline." << std::endl;
                    std::cout << "Il ne cherche même pas à cacher son mépris : pour lui, ce combat doit être purgé." << std::endl;
                    adaptation += 2;
                    player.takeDamage(8 + random.between(0, 8));
                }
                else
                {
                    std::cout << "Pour toute race non humaine, sa patience devient plus froide." << std::endl;
                    std::cout << "Il ne hait pas seulement ton corps. Il hait ce que ton existence contredit." << std::endl;
                    adaptation += 1;
                }

                if (adaptation > 10) adaptation = 10;
                boss.setSpecialEffect(adaptation);
                std::cout << "Adaptation humaine : " << adaptation << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 20 && random.between(1, 100) <= 45)
        {
            int luck = boss.getSpecialEffect() + random.between(1, 3);
            if (luck > 12) luck = 12;
            boss.setSpecialEffect(luck);

            std::cout << "Sérendys lance un dé qu'elle ne regarde même pas tomber." << std::endl;
            std::cout << "Oh... cette version-là était presque belle." << std::endl;
            if (random.between(1, 100) <= 30)
            {
                std::cout << "Elle relance une issue qui ne l'arrangeait pas." << std::endl;
                boss.heal(8 + luck);
            }
            std::cout << "Chance consciente : " << luck << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 21 && random.between(1, 100) <= 38)
        {
            int threshold = boss.getSpecialEffect();
            int score = 0;

            std::cout << "Le Gardien du Seuil ne pose pas une question." << std::endl;
            std::cout << "Il agit, puis il juge la réaction que tu avais déjà préparée." << std::endl;

            for (int i = 1; i <= 3; ++i)
            {
                int situation = random.between(1, 4);
                int roll = random.between(1, 100);

                if (situation == 1)
                {
                    std::cout << "Situation " << i << " : le Gardien prépare une frappe lourde." << std::endl;
                    if (player.isInDefensePosture()) roll += 35;
                }
                else if (situation == 2)
                {
                    std::cout << "Situation " << i << " : le Gardien referme son armure et tente de récupérer le rythme." << std::endl;
                    if (!player.isInDefensePosture()) roll += 15;
                }
                else if (situation == 3)
                {
                    std::cout << "Situation " << i << " : le Gardien ouvre un verrou qui punit les automatismes." << std::endl;
                    roll += random.between(-10, 20);
                }
                else
                {
                    std::cout << "Situation " << i << " : le Gardien attend que tu paniques." << std::endl;
                    if (player.isInDefensePosture()) roll += 20;
                }

                if (roll >= 65)
                {
                    score += 2;
                    std::cout << "Lecture correcte. +2 points." << std::endl;
                }
                else if (roll >= 45)
                {
                    score += 1;
                    std::cout << "Lecture moyenne. +1 point." << std::endl;
                }
                else
                {
                    score -= 1;
                    std::cout << "Mauvaise réaction. -1 point." << std::endl;
                }
            }

            if (score >= 4)
            {
                threshold -= 3;
                if (threshold < 0) threshold = 0;
                std::cout << "Score correct : ta prochaine attaque réussie devrait frapper comme un double impact." << std::endl;
                std::cout << "[Effet simplifié actuel] Le verrou du boss baisse fortement." << std::endl;
            }
            else
            {
                threshold += 4;
                std::cout << "Score insuffisant : la prochaine sanction du Gardien devrait peser comme un double coup." << std::endl;
                std::cout << "[Effet simplifié actuel] Le verrou du boss augmente fortement." << std::endl;
            }

            if (threshold > 12) threshold = 12;
            boss.setSpecialEffect(threshold);
            std::cout << "Verrou du Seuil : " << threshold << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 22 && random.between(1, 100) <= 42)
        {
            int authority = boss.getSpecialEffect() + random.between(1, 2);
            if (authority > 12) authority = 12;
            boss.setSpecialEffect(authority);

            std::cout << "Le Roi Sans Salle donne un ordre à une cour qui n'existe plus." << std::endl;
            if (random.between(1, 100) <= 50)
            {
                std::cout << "Ordre royal : Défendez le trône absent." << std::endl;
                boss.heal(8 + authority);
            }
            else
            {
                std::cout << "Ordre royal : Exécutez l'intrus." << std::endl;
                player.takeDamage(6 + authority);
            }
            std::cout << "Autorité fantôme : " << authority << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 23 && random.between(1, 100) <= 50)
        {
            int instinct = boss.getSpecialEffect() + random.between(1, 3);
            if (instinct > 14) instinct = 14;
            boss.setSpecialEffect(instinct);

            describeNamelessBeastVariant(random);
            std::cout << "Nom : ???" << std::endl;
            std::cout << "Espèce : erreur" << std::endl;
            std::cout << "La Bête réagit par instinct pur, comme si elle était différente à chaque rencontre." << std::endl;
            if (player.isInDefensePosture())
            {
                std::cout << "Elle tourne autour de ta garde au lieu de la briser tout de suite." << std::endl;
            }
            else
            {
                player.takeDamage(7 + instinct);
                std::cout << "Elle te mord avant que le registre finisse sa phrase." << std::endl;
            }
            std::cout << "Instinct sans nom : " << instinct << std::endl;
            std::cout << std::endl;
        }


        if (boss.getBossId() == 24 && random.between(1, 100) <= 44)
        {
            int abyss = boss.getSpecialEffect() + random.between(1, 3);
            if (abyss > 14) abyss = 14;
            boss.setSpecialEffect(abyss);

            std::cout << "Aldebaroth laisse remonter ce que le combat a de plus laid." << std::endl;
            std::cout << "Chaque hésitation devient rancune. Chaque blessure devient dette émotionnelle." << std::endl;
            if (player.isInDefensePosture())
            {
                std::cout << "Ta garde ne l'intéresse pas : il ne cherche pas l'ouverture, il cherche la fissure intérieure." << std::endl;
            }
            else
            {
                player.takeDamage(7 + abyss);
                std::cout << "Le négatif s'accroche à toi et inflige " << 7 + abyss << " dégâts." << std::endl;
            }
            std::cout << "Pression démoniaque : " << abyss << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 25)
        {
            int twinState = boss.getSpecialEffect();
            bool reconstructionCountdown = twinState >= 100;

            if (!reconstructionCountdown && boss.getHp() <= boss.getMaxHp() / 2)
            {
                boss.setSpecialEffect(100 + 5);
                std::cout << "Asterion vacille. Nihilon rit." << std::endl;
                std::cout << "Ou peut-être l'inverse. Le registre n'arrive pas à décider lequel vient de tomber." << std::endl;
                std::cout << "Il reste 5 tours pour briser l'autre moitié avant la reconstruction fatale." << std::endl;
                std::cout << std::endl;
            }
            else if (reconstructionCountdown)
            {
                int remaining = twinState - 100;
                remaining--;
                if (remaining <= 0)
                {
                    std::cout << "La moitié détruite se reconstruit autour de l'autre." << std::endl;
                    std::cout << "Création et destruction se referment sur toi." << std::endl;
                    std::cout << "Désintégration conceptuelle." << std::endl;
                    player.takeDamage(player.getHp());
                    std::cout << player.getName() << " tombe à 0 PV." << std::endl;
                    std::cout << std::endl;
                }
                else
                {
                    boss.setSpecialEffect(100 + remaining);
                    std::cout << "Compte à rebours des jumeaux : " << remaining << " tours avant reconstruction fatale." << std::endl;
                    std::cout << std::endl;
                }
            }
            else if (random.between(1, 100) <= 40)
            {
                twinState += random.between(1, 2);
                if (twinState > 12) twinState = 12;
                boss.setSpecialEffect(twinState);
                std::cout << "Asterion ajoute. Nihilon retire." << std::endl;
                std::cout << "Le résultat n'est pas zéro : c'est une pression impossible." << std::endl;
                std::cout << "Instabilité jumelle : " << twinState << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 26)
        {
            Player* concretePlayer = dynamic_cast<Player*>(&player);
            int authority = boss.getSpecialEffect();

            if (boss.getHp() <= boss.getMaxHp() / 2 && concretePlayer != nullptr && !concretePlayer->hasBossEquipmentSeal())
            {
                concretePlayer->activateBossEquipmentSeal("Écho d'Obérion : armure et arme refusées. Le combat continue au poing.");
                if (concretePlayer->hasEquippedWeapon()) concretePlayer->unequipWeapon();
                if (concretePlayer->hasEquippedArmor()) concretePlayer->unequipArmor();
                std::cout << "Obérion passe le seuil critique sans colère." << std::endl;
                std::cout << "À partir de maintenant, l'armure ne répond plus. L'arme quitte ta main." << std::endl;
                std::cout << "Il ne reste que le poing, le souffle, et la preuve que tu peux exister sans outil." << std::endl;
                std::cout << std::endl;
            }

            if (random.between(1, 100) <= 42)
            {
                authority += random.between(1, 2);
                if (authority > 15) authority = 15;
                boss.setSpecialEffect(authority);
                std::cout << "L'écho d'Obérion superpose plusieurs étages du monde pendant une seconde." << std::endl;
                std::cout << "Ton corps comprend qu'il n'affronte pas la vraie divinité, seulement une règle tombée trop bas." << std::endl;
                std::cout << "Autorité primordiale fragmentée : " << authority << std::endl;
                std::cout << std::endl;
            }
        }

        if (boss.getBossId() == 27 && random.between(1, 100) <= 45)
        {
            int authority = boss.getSpecialEffect() + random.between(1, 2);
            if (authority > 14) authority = 14;
            boss.setSpecialEffect(authority);

            std::cout << "FireFlight regarde une règle invisible dans l'air." << std::endl;
            std::cout << "Tu appelles ça une faille. Moi, j'appelle ça une décision de conception." << std::endl;
            if (random.between(1, 100) <= 35)
            {
                std::cout << "========== MENU COMBAT ==========" << std::endl;
                std::cout << "1 : Attaquer" << std::endl;
                std::cout << "2 : Demander au développeur d'être gentil" << std::endl;
                std::cout << "3 : Ouvrir un ticket de bug contre toi-même" << std::endl;
                std::cout << "4 : Faire semblant que ce choix est équilibré" << std::endl;
                std::cout << "FireFlight touche l'interface, mais contrairement à l'Anomalie, il sait exactement ce qu'il fait." << std::endl;
            }
            if (player.isInDefensePosture())
            {
                std::cout << "Trop de stabilité défensive détectée : patch de pression appliqué." << std::endl;
                player.takeDamage(6 + authority);
            }
            else
            {
                std::cout << "Build instable : FireFlight améliore temporairement son prochain test." << std::endl;
            }
            std::cout << "Autorité du créateur limitée : " << authority << std::endl;
            std::cout << std::endl;
        }


        if (boss.getBossId() == 28 && random.between(1, 100) <= 45)
        {
            int breath = boss.getSpecialEffect() + random.between(1, 3);
            if (breath > 14) breath = 14;
            boss.setSpecialEffect(breath);

            std::cout << "Le Souffle sans Visage inspire sans bouche." << std::endl;
            std::cout << "L'air quitte brièvement tes poumons, comme si quelqu'un avait oublié de te dessiner un visage." << std::endl;
            if (!player.isInDefensePosture())
            {
                int damage = 7 + breath;
                player.takeDamage(damage);
                std::cout << "Asphyxie muette : " << damage << " dégâts." << std::endl;
            }
            else
            {
                std::cout << "Ta posture limite la panique, mais pas le froid qui remonte dans ta gorge." << std::endl;
            }
            std::cout << "Pression respiratoire : " << breath << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 29 && random.between(1, 100) <= 43)
        {
            int nails = boss.getSpecialEffect() + random.between(1, 2);
            if (nails > 12) nails = 12;
            boss.setSpecialEffect(nails);

            std::cout << "La Marionnette aux Mille Clous tire sur des fils que personne ne tient." << std::endl;
            std::cout << "Un clou tombe au sol. Puis un autre. Puis tu comprends qu'ils comptent tes erreurs." << std::endl;
            if (player.isInDefensePosture())
            {
                std::cout << "Ta garde bloque le geste, mais pas la tension des fils autour de tes membres." << std::endl;
            }
            else
            {
                int damage = 6 + nails;
                player.takeDamage(damage);
                std::cout << "Un fil te force à avancer dans la mauvaise direction : " << damage << " dégâts." << std::endl;
            }
            std::cout << "Clous actifs : " << nails << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 30 && random.between(1, 100) <= 45)
        {
            Player* concretePlayer = dynamic_cast<Player*>(&player);
            int line = boss.getSpecialEffect() + random.between(1, 2);
            if (concretePlayer != nullptr && concretePlayer->isAlteredByCheats()) line += 2;
            if (concretePlayer != nullptr && concretePlayer->getLethalCheatAttemptCount() > 0) line += 2;
            if (line > 15) line = 15;
            boss.setSpecialEffect(line);

            std::cout << "Moiran ne prédit pas ton prochain coup. Il regarde la route entière." << std::endl;
            std::cout << "Un fil invisible se tend entre ton choix passé et ta prochaine erreur." << std::endl;
            if (concretePlayer != nullptr && concretePlayer->getLethalCheatAttemptCount() > 0)
            {
                std::cout << "Le Destin reconnaît la tentative de cheat en Léthal : même les chemins interdits laissent des traces." << std::endl;
                player.takeDamage(8 + concretePlayer->getLethalCheatAttemptCount() * 4);
            }
            std::cout << "Ligne de destinée : " << line << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 31 && random.between(1, 100) <= 42)
        {
            int souls = boss.getSpecialEffect() + random.between(1, 2);
            if (souls > 12) souls = 12;
            boss.setSpecialEffect(souls);

            std::cout << "Le Cerf des Âmes Égarées avance sans bruit." << std::endl;
            std::cout << "Entre ses bois pendent des lanternes qui ne contiennent pas de feu, mais des souvenirs." << std::endl;
            if (player.isInDefensePosture())
            {
                boss.heal(6 + souls);
                std::cout << "Il respecte ta retenue et recule, mais les âmes autour de lui le referment." << std::endl;
            }
            else
            {
                int damage = 7 + souls;
                player.takeDamage(damage);
                std::cout << "Une âme perdue traverse ta poitrine : " << damage << " dégâts." << std::endl;
            }
            std::cout << "Âmes égarées : " << souls << std::endl;
            std::cout << std::endl;
        }


        if (boss.getBossId() == 32 && random.between(1, 100) <= 45)
        {
            int fury = boss.getSpecialEffect() + random.between(1, 3);
            if (player.isInDefensePosture()) fury += 1;
            if (fury > 16) fury = 16;
            boss.setSpecialEffect(fury);

            std::cout << "Gorvald frappe le sol du manche de sa hache." << std::endl;
            std::cout << "Un roi orc ne mesure pas seulement la force : il mesure si tu oses rester debout." << std::endl;
            if (player.isInDefensePosture())
            {
                std::cout << "Ta posture l'intéresse. Il ne la respecte que si elle tient au prochain choc." << std::endl;
            }
            std::cout << "Fureur royale : " << fury << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 33 && random.between(1, 100) <= 45)
        {
            int thirst = boss.getSpecialEffect() + random.between(1, 2);
            if (thirst > 15) thirst = 15;
            boss.setSpecialEffect(thirst);

            std::cout << "Serana avance comme si l'arène était son salon." << std::endl;
            std::cout << "Ne tremble pas. Le sang a toujours meilleur goût quand il croit encore choisir." << std::endl;
            if (player.getHp() < player.getMaxHp() / 2)
            {
                int bite = 6 + thirst;
                player.takeDamage(bite);
                boss.heal(4 + thirst / 2);
                std::cout << "Sang appelé : " << bite << " dégâts, et la reine récupère une partie de la blessure." << std::endl;
            }
            std::cout << "Soif royale : " << thirst << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 34 && random.between(1, 100) <= 45)
        {
            int web = boss.getSpecialEffect() + random.between(1, 3);
            if (player.isInDefensePosture()) web += 1;
            if (web > 16) web = 16;
            boss.setSpecialEffect(web);

            std::cout << "Draiite ne court pas. Elle n'en a pas besoin." << std::endl;
            std::cout << "Un fil invisible vient de choisir une direction à ta place." << std::endl;
            if (!player.isInDefensePosture())
            {
                int poison = 5 + web;
                player.takeDamage(poison);
                std::cout << "Morsure venimeuse préparée par la toile : " << poison << " dégâts." << std::endl;
            }
            else
            {
                std::cout << "Ta posture tient, mais rester immobile donne du temps à la toile." << std::endl;
            }
            std::cout << "Toile de reine : " << web << std::endl;
            std::cout << std::endl;
        }

        if (boss.getBossId() == 35 && random.between(1, 100) <= 45)
        {
            int mirror = boss.getSpecialEffect() + random.between(1, 2);
            if (mirror > 15) mirror = 15;
            boss.setSpecialEffect(mirror);

            std::cout << "Les Jumelles parlent ensemble, mais une seule phrase est vraie." << std::endl;
            if (random.between(1, 100) <= 50)
            {
                std::cout << "La prochaine attaque sera faible. La prochaine attaque sera mortelle." << std::endl;
                if (!player.isInDefensePosture())
                {
                    int cut = 6 + mirror;
                    player.takeDamage(cut);
                    std::cout << "Tu suis le mauvais reflet : " << cut << " dégâts." << std::endl;
                }
                else
                {
                    std::cout << "Tu ne choisis aucun reflet. La posture réduit le mensonge." << std::endl;
                }
            }
            else
            {
                std::cout << "Un reflet corrompu copie ton hésitation et nourrit les Jumelles." << std::endl;
                boss.heal(5 + mirror);
            }
            std::cout << "Vérité fendue : " << mirror << std::endl;
            std::cout << std::endl;
        }

        applyAlteredHunterPressure(boss, player, random);
    }
}

bool BossCombatTurn::play(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    std::cout << "Tour de " << boss.getName() << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    executeNewBossPassiveAction(boss, player, random);

    AIAction action = CombatAI::chooseBossAction(boss, random);

    if (action == AIAction::Attack)
    {
        CombatActions::executeAttack(boss, player, random);
        return BossCombat::handleBossEndTurn(boss, player);
    }

    if (action == AIAction::HealingPotion)
    {
        bool potionUsed = boss.useHealingPotion(boss.getMaxHp() * 10 / 100);

        if (potionUsed)
        {
            std::cout << boss.getName() << " récupère une partie de sa vitalité." << std::endl;
            std::cout << std::endl;
        }
        else
        {
            std::cout << boss.getName()
                      << " cherche une source de régénération, mais rien ne répond."
                      << std::endl;
            std::cout << std::endl;

            CombatActions::executeAttack(boss, player, random);
        }

        return BossCombat::handleBossEndTurn(boss, player);
    }

    if (action == AIAction::DamagePotion)
    {
        bool potionUsed = CombatActions::executeDamagePotion(
            boss,
            player,
            random,
            50
        );

        if (!potionUsed)
        {
            CombatActions::executeAttack(boss, player, random);
        }

        return BossCombat::handleBossEndTurn(boss, player);
    }

    if (action == AIAction::Ultimate)
    {
        BossCombat::executeBossUltimate(boss, player, random);
        return BossCombat::handleBossEndTurn(boss, player);
    }

    std::cout << boss.getName()
              << " reste immobile, comme s'il observait déjà ta fin."
              << std::endl;
    std::cout << std::endl;

    return BossCombat::handleBossEndTurn(boss, player);
}
