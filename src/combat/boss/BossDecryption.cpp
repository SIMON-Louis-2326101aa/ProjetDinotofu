// EN: BossDecryption.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossDecryption.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/boss/BossDecryption.hpp"

#include "core/Console.hpp"

#include <iostream>

// EN: checkBossDecryption declares or implements a focused behavior used by this module.
// FR: checkBossDecryption déclare ou implémente un comportement précis utilisé par ce module.
void BossDecryption::checkBossDecryption(Boss& boss)
{
    if (!boss.mustDecryptStats() && !boss.shouldUnlockUltimate())
    {
        return;
    }

    bool unlockingUltimate = boss.shouldUnlockUltimate();

    std::cout << std::endl;

    if (boss.getBossId() == 1)
    {
        std::cout << "Très bien, humain..." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Je vois que je t'ai sous-estimé." << std::endl;
        std::cout << "La lumière qui me protège commence à se fissurer." << std::endl;
        std::cout << "Alors regarde bien... mais ne crois pas que cela suffira à me vaincre." << std::endl;
    }
    else if (boss.getBossId() == 2)
    {
        std::cout << "Hahaha..." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Tu as réussi à m'arracher assez de sang pour voir derrière le voile." << std::endl;
        std::cout << "Mais plus tu comprends ce que je suis, plus tu t'approches de la peur." << std::endl;
        std::cout << "Allez, humain. Lis mes statistiques... et désespère." << std::endl;
    }
    else if (boss.getBossId() == 3)
    {
        std::cout << "Analyse défensive compromise." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Le protocole de dissimulation perd en stabilité." << std::endl;
        std::cout << "Tu as atteint un seuil que peu d'êtres auraient dû franchir." << std::endl;
        std::cout << "Mes données deviennent visibles... mais mon armure, elle, tient encore." << std::endl;
    }
    else if (boss.getBossId() == 4)
    {
        std::cout << "Le sang noir de l'écho tombe sur le sol." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Aucune douleur. Aucun recul. Seulement un hurlement." << std::endl;
        std::cout << "La chasse commence vraiment." << std::endl;
    }
    else if (boss.getBossId() == 5)
    {
        std::cout << "La couronne de Grinka tombe de travers." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Son sourire disparaît." << std::endl;
        std::cout << "Elle ne ressemble plus à une voleuse. Elle ressemble à une reine qu'on vient d'humilier." << std::endl;
        std::cout << "Ajoutez les frais de colère." << std::endl;
    }
    else if (boss.getBossId() == 6)
    {
        std::cout << "L'avatar pose un genou au sol. Pendant un instant, tu crois voir une ouverture." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Puis l'ombre se relève avant lui." << std::endl;
        std::cout << "La couronne se fend. Quelque chose, derrière son visage, sourit." << std::endl;
    }
    else if (boss.getBossId() == 7)
    {
        std::cout << "Une fissure traverse les écailles du fragment draconique." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Thamarys n'est pas entier ici... mais même un fragment suffit à faire trembler l'air." << std::endl;
        std::cout << "Le souffle ancien commence à se former." << std::endl;
    }
    else if (boss.getBossId() == 8)
    {
        std::cout << "Les racines cessent de retenir leur colère." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Mojo ferme les yeux. La forêt, elle, les ouvre tous." << std::endl;
        std::cout << "La mémoire de tes gestes remonte depuis le sol." << std::endl;
    }
    else if (boss.getBossId() == 9)
    {
        std::cout << "Le reflet d'Inakari se brise en plusieurs sourires." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Chaque miroir prétend être le vrai." << std::endl;
        std::cout << "Une seule vérité respire derrière les mensonges." << std::endl;
    }
    else if (boss.getBossId() == 16)
    {
        std::cout << "Lexior pose son regard sur ce que tu répètes depuis trop longtemps." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Le registre de tes dix derniers combats s'ouvre sans ton accord." << std::endl;
        std::cout << "À partir de maintenant, même ton équipement peut être jugé." << std::endl;
    }
    else if (boss.getBossId() == 17)
    {
        std::cout << "La lune se fend entre rêve et cauchemar." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Luna et Onyrae ne parlent pas ensemble. Elles se superposent." << std::endl;
        std::cout << "Le sommeil devient une arme." << std::endl;
    }
    else if (boss.getBossId() == 18)
    {
        std::cout << "Le fragment élémentaire refuse de choisir une forme." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Feu, Terre, Eau et Vent atteignent le seuil critique ensemble." << std::endl;
        std::cout << "Deux ultimes deviennent possibles : fusion totale ou division en quatre formes." << std::endl;
    }
    else if (boss.getBossId() == 19)
    {
        std::cout << "Hitogami a assez regardé." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Son adaptation humaine s'éveille vraiment, et sa haine des races non humaines devient plus visible." << std::endl;
        std::cout << "Même battu, il pourrait se relever une fois." << std::endl;
    }
    else if (boss.getBossId() == 20)
    {
        std::cout << "Sérendys sourit comme si plusieurs futurs venaient de se présenter." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Son destin peut choisir la meilleure issue ou inverser le rare et le commun." << std::endl;
    }
    else if (boss.getBossId() == 21)
    {
        std::cout << "Le Gardien du Seuil fissure la limite devant toi." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Ses épreuves à points deviennent plus dangereuses après le seuil critique." << std::endl;
    }
    else if (boss.getBossId() == 22)
    {
        std::cout << "Le Roi Sans Salle tend la main vers un trône qui n'existe pas." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Si la salle a disparu, alors il régnera sur les ruines." << std::endl;
    }
    else if (boss.getBossId() == 23)
    {
        std::cout << "La Bête recule. Pour la première fois, elle semble chercher quelque chose." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Pas une sortie. Pas une proie. Un nom." << std::endl;
    }
    else if (boss.getBossId() == 24)
    {
        std::cout << "Aldebaroth cesse de contenir ce qui remonte du fond du combat." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Le démon primordial affaibli ne juge pas les triches. Il mange la part négative du monde." << std::endl;
    }
    else if (boss.getBossId() == 25)
    {
        std::cout << "Les jumeaux cessent de se synchroniser proprement." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Si une moitié tombe, cinq tours seulement resteront pour empêcher la reconstruction fatale." << std::endl;
    }
    else if (boss.getBossId() == 26)
    {
        std::cout << "L'écho fragmenté d'Obérion retire la politesse du duel." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Au seuil critique, arme et armure peuvent être refusées. Le corps doit répondre seul." << std::endl;
    }
    else if (boss.getBossId() == 27)
    {
        std::cout << "FireFlight regarde ses propres blessures, puis le ciel." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Bon. On va arrêter de faire semblant que ce combat est stable." << std::endl;
    }
    else if (boss.getBossId() == 28)
    {
        std::cout << "Le Souffle sans Visage inspire plus fort que le registre ne peut écrire." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Son ultime naît quand respirer devient un privilège." << std::endl;
    }
    else if (boss.getBossId() == 29)
    {
        std::cout << "La Marionnette laisse tomber plusieurs clous à la fois." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Chaque clou semble attendre une douleur à accrocher." << std::endl;
    }
    else if (boss.getBossId() == 30)
    {
        std::cout << "Moiran ne change pas de posture. Le destin, lui, change d'épaisseur." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Le seuil critique ouvre une fin qui reconnaît déjà tes détours." << std::endl;
    }
    else if (boss.getBossId() == 31)
    {
        std::cout << "Le Cerf des Âmes Égarées baisse lentement ses bois." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Les lanternes accrochées à lui s'allument sans feu." << std::endl;
    }
    else if (boss.getBossId() == 28)
    {
        std::cout << "Le Souffle sans Visage perd une partie de son silence." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Le registre comprend enfin que cette chose n'attaque pas le corps : elle attaque l'acte de respirer." << std::endl;
    }
    else if (boss.getBossId() == 29)
    {
        std::cout << "La Marionnette aux Mille Clous craque sous sa propre couture." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Chaque clou est une mémoire douloureuse. Chaque fil est une dette de mouvement." << std::endl;
    }
    else if (boss.getBossId() == 30)
    {
        std::cout << "Moiran relève la tête, comme si ton futur venait d'oser changer de page." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Le Destin ne déteste pas les tricheurs. Il les archive comme des preuves." << std::endl;
    }
    else if (boss.getBossId() == 31)
    {
        std::cout << "Le Cerf des Âmes Égarées baisse ses bois vers le sol." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Les morts ne parlent pas. Pourtant, leurs lanternes indiquent la direction du jugement." << std::endl;
    }

    else if (boss.getBossId() == 32)
    {
        std::cout << "Gorvald crache du sang au sol, puis sourit." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Bien. Maintenant, mon peuple peut regarder." << std::endl;
    }
    else if (boss.getBossId() == 33)
    {
        std::cout << "Serana observe son propre sang comme une insulte." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Tu as renversé du sang royal. Il faudra le remplacer." << std::endl;
    }
    else if (boss.getBossId() == 34)
    {
        std::cout << "Draiite grimpe lentement sur une toile invisible." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Tu viens enfin de voir la salle entière." << std::endl;
    }
    else if (boss.getBossId() == 35)
    {
        std::cout << "Le miroir derrière les Jumelles se fend en silence." << std::endl;
        Console::pauseSeconds(2);
        std::cout << "Maintenant, choisis ce que tu veux croire." << std::endl;
    }
    else
    {
        std::cout << "Le voile autour de " << boss.getName() << " se déchire." << std::endl;
        Console::pauseSeconds(2);

        std::cout << "Ses statistiques deviennent enfin lisibles." << std::endl;
    }

    std::cout << std::endl;

    Console::pauseSeconds(2);

    std::cout << "Décryptage en cours..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(2);

    boss.decryptStats();

    if (unlockingUltimate)
    {
        boss.unlockUltimate();
        std::cout << "Seuil critique franchi : l'ultime de l'entité vient de s'éveiller." << std::endl;
    }

    std::cout << "Décryptage terminé." << std::endl;
    std::cout << std::endl;

    boss.displayStats();

    std::cout << "Le combat vient de changer de rythme." << std::endl;
    std::cout << std::endl;
}
