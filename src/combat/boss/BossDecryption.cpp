// EN: BossDecryption.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossDecryption.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossDecryption.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <string>
#include <vector>

namespace
{
    std::vector<std::string> decryptionLinesForBoss(const Boss& boss)
    {
        switch (boss.getBossId())
        {
            case 1:
                return {
                    "Très bien, humain...",
                    "Je vois que je t'ai sous-estimé.",
                    "La lumière qui me protège commence à se fissurer.",
                    "Alors regarde bien... mais ne crois pas que cela suffira à me vaincre."
                };
            case 2:
                return {
                    "Hahaha...",
                    "Tu as réussi à m'arracher assez de sang pour voir derrière le voile.",
                    "Mais plus tu comprends ce que je suis, plus tu t'approches de la peur.",
                    "Allez, humain. Lis mes statistiques... et désespère."
                };
            case 3:
                return {
                    "Analyse défensive compromise.",
                    "Le protocole de dissimulation perd en stabilité.",
                    "Tu as atteint un seuil que peu d'êtres auraient dû franchir.",
                    "Mes données deviennent visibles... mais mon armure, elle, tient encore."
                };
            case 4:
                return {
                    "Le sang noir de l'écho tombe sur le sol.",
                    "Aucune douleur. Aucun recul. Seulement un hurlement.",
                    "La chasse commence vraiment."
                };
            case 5:
                return {
                    "La couronne de Grinka tombe de travers.",
                    "Son sourire disparaît.",
                    "Elle ne ressemble plus à une voleuse. Elle ressemble à une reine qu'on vient d'humilier.",
                    "Ajoutez les frais de colère."
                };
            case 6:
                return {
                    "L'avatar pose un genou au sol. Pendant un instant, tu crois voir une ouverture.",
                    "Puis l'ombre se relève avant lui.",
                    "La couronne se fend. Quelque chose, derrière son visage, sourit."
                };
            case 7:
                return {
                    "Une fissure traverse les écailles du fragment draconique.",
                    "Thamarys n'est pas entier ici... mais même un fragment suffit à faire trembler l'air.",
                    "Le souffle ancien commence à se former."
                };
            case 8:
                return {
                    "Les racines cessent de retenir leur colère.",
                    "Mojo ferme les yeux. La forêt, elle, les ouvre tous.",
                    "La mémoire de tes gestes remonte depuis le sol."
                };
            case 9:
                return {
                    "Le reflet d'Inakari se brise en plusieurs sourires.",
                    "Chaque miroir prétend être le vrai.",
                    "Une seule vérité respire derrière les mensonges."
                };
            case 16:
                return {
                    "Lexior pose son regard sur ce que tu répètes depuis trop longtemps.",
                    "Le registre de tes dix derniers combats s'ouvre sans ton accord.",
                    "À partir de maintenant, même ton équipement peut être jugé."
                };
            case 17:
                return {
                    "La lune se fend entre rêve et cauchemar.",
                    "Luna et Onyrae ne parlent pas ensemble. Elles se superposent.",
                    "Le sommeil devient une arme."
                };
            case 18:
                return {
                    "Le fragment élémentaire refuse de choisir une forme.",
                    "Feu, Terre, Eau et Vent atteignent le seuil critique ensemble.",
                    "Deux ultimes deviennent possibles : fusion totale ou division en quatre formes."
                };
            case 19:
                return {
                    "Hitogami a assez regardé.",
                    "Son adaptation humaine s'éveille vraiment, et sa haine des races non humaines devient plus visible.",
                    "Même battu, il pourrait se relever une fois."
                };
            case 20:
                return {
                    "Sérendys sourit comme si plusieurs futurs venaient de se présenter.",
                    "Son destin peut choisir la meilleure issue ou inverser le rare et le commun."
                };
            case 21:
                return {
                    "Le Gardien du Seuil fissure la limite devant toi.",
                    "Ses épreuves à points deviennent plus dangereuses après le seuil critique."
                };
            case 22:
                return {
                    "Le Roi Sans Salle tend la main vers un trône qui n'existe pas.",
                    "Si la salle a disparu, alors il régnera sur les ruines."
                };
            case 23:
                return {
                    "La Bête recule. Pour la première fois, elle semble chercher quelque chose.",
                    "Pas une sortie. Pas une proie. Un nom."
                };
            case 24:
                return {
                    "Aldebaroth cesse de contenir ce qui remonte du fond du combat.",
                    "Le démon primordial affaibli ne juge pas les triches. Il mange la part négative du monde."
                };
            case 25:
                return {
                    "Les jumeaux cessent de se synchroniser proprement.",
                    "Si une moitié tombe, cinq tours seulement resteront pour empêcher la reconstruction fatale."
                };
            case 26:
                return {
                    "L'écho fragmenté d'Obérion retire la politesse du duel.",
                    "Au seuil critique, arme et armure peuvent être refusées. Le corps doit répondre seul."
                };
            case 27:
                return {
                    "FireFlight regarde ses propres blessures, puis le ciel.",
                    "Bon. On va arrêter de faire semblant que ce combat est stable."
                };
            case 28:
                return {
                    "Le Souffle sans Visage inspire plus fort que le registre ne peut écrire.",
                    "Son ultime naît quand respirer devient un privilège."
                };
            case 29:
                return {
                    "La Marionnette laisse tomber plusieurs clous à la fois.",
                    "Chaque clou semble attendre une douleur à accrocher."
                };
            case 30:
                return {
                    "Moiran ne change pas de posture. Le destin, lui, change d'épaisseur.",
                    "Le seuil critique ouvre une fin qui reconnaît déjà tes détours."
                };
            case 31:
                return {
                    "Le Cerf des Âmes Égarées baisse lentement ses bois.",
                    "Les lanternes accrochées à lui s'allument sans feu."
                };
            case 32:
                return {
                    "Gorvald crache du sang au sol, puis sourit.",
                    "Bien. Maintenant, mon peuple peut regarder."
                };
            case 33:
                return {
                    "Serana observe son propre sang comme une insulte.",
                    "Tu as renversé du sang royal. Il faudra le remplacer."
                };
            case 34:
                return {
                    "Draiite grimpe lentement sur une toile invisible.",
                    "Tu viens enfin de voir la salle entière."
                };
            case 35:
                return {
                    "Le miroir derrière les Jumelles se fend en silence.",
                    "Maintenant, choisis ce que tu veux croire."
                };
            default:
                return {
                    "Le voile autour de " + boss.getName() + " se déchire.",
                    "Ses statistiques deviennent enfin lisibles."
                };
        }
    }
}

// EN: checkBossDecryption declares or implements a focused behavior used by this module.
// FR: checkBossDecryption déclare ou implémente un comportement précis utilisé par ce module.
void BossDecryption::checkBossDecryption(Boss& boss)
{
    if (!boss.mustDecryptStats() && !boss.shouldUnlockUltimate())
    {
        return;
    }

    const bool unlockingUltimate = boss.shouldUnlockUltimate();
    const std::string baseScreenId = "boss.decryption." + std::to_string(boss.getBossId());

    MessageScreen::show(
        "SEUIL CRITIQUE",
        baseScreenId + ".threshold",
        decryptionLinesForBoss(boss),
        false
    );

    Console::pauseSeconds(1);

    MessageScreen::show(
        "DÉCRYPTAGE",
        baseScreenId + ".start",
        {"Décryptage en cours..."},
        false
    );

    Console::pauseSeconds(1);

    boss.decryptStats();

    std::vector<std::string> resultLines;

    if (unlockingUltimate)
    {
        boss.unlockUltimate();
        resultLines.push_back("Seuil critique franchi : l'ultime de l'entité vient de s'éveiller.");
    }

    resultLines.push_back("Décryptage terminé.");

    MessageScreen::show(
        "DÉCRYPTAGE TERMINÉ",
        baseScreenId + ".complete",
        resultLines,
        false
    );

    boss.displayStats();

    MessageScreen::show(
        "RYTHME DU COMBAT",
        baseScreenId + ".tempo",
        {"Le combat vient de changer de rythme."},
        false
    );
}
