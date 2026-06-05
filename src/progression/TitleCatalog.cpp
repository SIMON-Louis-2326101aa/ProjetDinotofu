// EN: TitleCatalog.cpp implements character title metadata, hidden trophy rules and completion checks.
// FR: TitleCatalog.cpp implémente les métadonnées des titres, les trophées cachés et les vérifications de complétion.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "progression/TitleCatalog.hpp"

#include <algorithm>

namespace
{
    using TitleCatalog::TitleDefinition;

    const std::string& genericEffectForCategory(const std::string& category)
    {
        static const std::string guild = "Réputation de guilde : peut influencer certains dialogues, contrôles de rang et réactions de comptoir.";
        static const std::string hunt = "Réputation de chasse : peut influencer des dialogues de guilde, pistes de bestiaire et contrats liés aux monstres.";
        static const std::string boss = "Réputation de boss : prouve un exploit majeur, peut ouvrir des dialogues rares et réactions de PNJ.";
        static const std::string economy = "Réputation économique : peut influencer des dialogues de marchands, registres, taxes ou missions de comptoir.";
        static const std::string city = "Réputation locale : peut changer certaines réactions de ville, rumeurs et demandes de guilde.";
        static const std::string biome = "Mémoire d'exploration : peut influencer des dialogues de route, biomes et rapports de terrain.";
        static const std::string bestiary = "Mémoire de registre : peut enrichir les dialogues de bestiaire, matériaux et dossiers de découverte.";
        static const std::string anomaly = "Affinité d'anomalie : peut modifier certains dialogues liés aux interfaces, hallucinations et erreurs.";
        static const std::string curse = "Trace de malédiction : peut influencer l'église, les diagnostics, les réactions de boss et les états spéciaux.";
        static const std::string coop = "Réputation de groupe : peut influencer les dialogues coop, partages de primes et réactions d'alliés.";
        static const std::string story = "Trace d'histoire : marque une étape narrative majeure sans forcément donner un bonus de combat brut.";
        static const std::string funny = "Titre surtout roleplay : peut déclencher des répliques drôles ou humiliantes, sans bonus direct prévu.";
        static const std::string completion = "Titre de complétion : sert de preuve ultime et peut être utilisé par des scènes ou dialogues de fin.";
        static const std::string system = "Titre de système : marque une action méta du personnage, avec effet très faible surtout interface/lore.";
        static const std::string generic = "Titre de réputation : effet principalement narratif, social ou contextuel.";

        if (category.find("Guilde") != std::string::npos) return guild;
        if (category.find("Chasse") != std::string::npos) return hunt;
        if (category.find("Boss") != std::string::npos) return boss;
        if (category.find("Économie") != std::string::npos || category.find("Economie") != std::string::npos) return economy;
        if (category.find("Ville") != std::string::npos) return city;
        if (category.find("Biomes") != std::string::npos) return biome;
        if (category.find("Bestiaire") != std::string::npos) return bestiary;
        if (category.find("Anomalie") != std::string::npos) return anomaly;
        if (category.find("Malédictions") != std::string::npos || category.find("Maledictions") != std::string::npos) return curse;
        if (category.find("Coop") != std::string::npos) return coop;
        if (category.find("Histoire") != std::string::npos) return story;
        if (category.find("Drôle") != std::string::npos || category.find("Drole") != std::string::npos) return funny;
        if (category.find("Complétion") != std::string::npos || category.find("Completion") != std::string::npos) return completion;
        if (category.find("Système") != std::string::npos || category.find("Systeme") != std::string::npos) return system;
        return generic;
    }

    TitleDefinition title(
        const std::string& name,
        const std::string& category,
        const std::string& unlockHint,
        bool visibleBeforeUnlock,
        const std::string& effect = "",
        bool requiredForUltimate = true
    )
    {
        TitleDefinition definition;
        definition.name = name;
        definition.category = category;
        definition.unlockHint = unlockHint;
        definition.effect = effect.empty() ? genericEffectForCategory(category) : effect;
        definition.visibleBeforeUnlock = visibleBeforeUnlock;
        definition.requiredForUltimate = requiredForUltimate;
        return definition;
    }

    bool containsTitle(const std::vector<std::string>& titles, const std::string& titleName)
    {
        return std::find(titles.begin(), titles.end(), titleName) != titles.end();
    }
}

namespace TitleCatalog
{
    const std::vector<TitleDefinition>& availableTitleDefinitions()
    {
        static const std::vector<TitleDefinition> titles = {
            // Titres visibles : repères simples que le joueur peut comprendre sans spoiler un secret.
            title("Bienvenue dans Dinotofu", "Système", "Lancer le jeu avec un personnage chargé ou créé.", true, "Titre de session : prouve que le personnage a ouvert le registre du jeu. Effet très faible : peut déclencher de petites lignes d'accueil."),
            title("Voix du terminal", "Système", "Lancer le jeu depuis la version terminal.", true, "Titre de session : identité terminal/console assumée. Effet très faible : peut modifier certaines répliques d'interface ou de registre."),
            title("Une sorte de titre ?", "Système", "Obtenir son premier titre, quel qu'il soit.", true, "Et une sorte de description..."),
            title("Aventurier", "Guilde", "Réussir l'inscription officielle de la guilde.", true),
            title("Aventurier rang F", "Guilde", "Valider des contrats officiels après inscription.", true),
            title("Aventurier rang E", "Guilde", "Monter le dossier de guilde avec assez de contrats et de niveau.", true),
            title("Aventurier rang D", "Guilde", "Atteindre le rang D de guilde.", true),
            title("Aventurier rang C", "Guilde", "Atteindre le rang C de guilde.", true),
            title("Aventurier rang B", "Guilde", "Atteindre le rang B de guilde.", true),
            title("Aventurier rang A", "Guilde", "Atteindre le rang A de guilde.", true),
            title("Main fiable de la guilde", "Guilde", "Valider plusieurs contrats simples sans être un poids pour le panneau.", true),
            title("Aide de quartier", "Ville / guilde", "Rendre service aux habitants par des contrats sobres.", true),
            title("Sac discipliné", "Quêtes / inventaire", "Réussir une quête de tri de sac ou d'inventaire trop chargé sans perdre l'objet important.", false, "Influence faible : les intendants et marchands font davantage confiance à ton sens du tri."),
            title("Armurier qui écoute", "Quêtes / équipement", "Réussir une quête d'armure mal ajustée en respectant la morphologie d'une race ou sous-race.", false, "Influence faible : forgerons et semi-humains réagissent mieux à tes conseils d'ajustement."),
            title("Intendant de terrain", "Quêtes / inventaire", "Réussir une mission de préparation d'inventaire, équipement ou durabilité.", false, "Influence faible : la guilde te voit comme quelqu'un qui prépare son sac avant de mourir bêtement."),
            title("Chaîne tenue", "Quêtes de guilde", "Terminer une suite de demandes, délais ou priorités sans casser le fil de mission.", false, "Influence faible : le comptoir fait davantage confiance à ta gestion des priorités."),
            title("Partenaire fiable", "Coop", "Valider une mission liée à la participation ou aux récompenses de groupe.", false, "Influence faible : les alliés et la guilde remarquent que tu ne triches pas sur la participation."),
            title("Arme entretenue", "Équipement / durabilité", "Débloquer ou utiliser une habitude de soin d'arme avant une sortie risquée.", false, "Influence faible : forgerons et guildes remarquent que tu prépares ton arme avant qu'elle ne casse."),
            title("Armure qui respire", "Équipement / durabilité", "Respecter assez souvent l'ajustement d'une armure, surtout avec races ou sous-races.", false, "Influence faible : les artisans réagissent mieux à tes remarques d'ajustement."),
            title("Retour par le bon chemin", "Biomes / exploration", "Accumuler assez de routes, fuites ou explorations pour apprendre à revenir proprement.", false, "Influence faible : améliore surtout les dialogues de route et d'exploration."),
            title("Éclaireur de route", "Biomes / exploration", "Valider des missions de route, de piste ou d'exploration lisible.", true),
            title("Archiviste de terrain", "Bestiaire / matériaux", "Rapporter des observations utiles au registre.", true),
            title("Trieur de matériaux", "Bestiaire / matériaux", "Aider la guilde à classer des composants ou lots douteux.", true),
            title("Chasseur de guilde", "Chasse", "Valider une première chasse chiffrée officielle.", true),
            title("Chasseur confirmé", "Chasse", "Valider une chasse chiffrée de rang B ou plus.", true),
            title("Nettoyeur de nuisibles", "Chasse", "Valider une chasse contre rats, slimes, gobelins ou insectoïdes mineurs.", true),
            title("Pisteur de bêtes", "Chasse", "Valider une chasse contre loups, ours ou bêtes lourdes.", true),
            title("Gardien des tombes", "Chasse", "Valider une chasse de squelettes, goules, revenants ou morts-vivants.", true),
            title("Briseur d'automates", "Chasse", "Valider une chasse de golems, automates, armures vivantes, statues ou pantins animés.", true),
            title("Première chute officielle", "Drôle / survie", "Mourir au moins une fois en mode non définitif.", true, "Titre roleplay : rappelle que le personnage a déjà connu une vraie chute."),
            title("Survivant administratif", "Drôle / survie", "Revenir après une mort que le registre a dû noter.", true),

            // Titres secrets : volontairement nombreux et masqués pour préserver la découverte.
            title("Aventurier rang S", "Guilde", "Atteindre le rang S de guilde.", false),
            title("Aventurier rang SS", "Guilde", "Atteindre le rang SS de guilde.", false),
            title("Aventurier rang SSS", "Guilde", "Atteindre le rang SSS de guilde.", false),
            title("Aventurier - Héros mondial", "Guilde", "Atteindre le rang Héros mondial.", false),
            title("Aventurier - Légende", "Guilde", "Atteindre le rang Légende.", false),
            title("Aventurier - Rang divin", "Guilde", "Atteindre le rang Dieu.", false),
            title("Le registre n'a plus de rang", "Guilde", "Atteindre le rang maximum actuel de guilde.", false),
            title("Niveau maximum, problème minimum", "Guilde", "Atteindre le niveau maximum du personnage.", false, "Titre de progression : peut déclencher des réactions de PNJ qui comprennent que le niveau n'est plus le vrai problème."),
            title("Nom scellé par la guilde", "Guilde", "Valider une mission que le panneau préfère ne pas expliquer.", false),
            title("Sang-froid de rang S", "Guilde", "Terminer un contrat de rang S ou supérieur sans que la guilde le banalise.", false),
            title("Vétéran des contrats scellés", "Guilde", "Accumuler des contrats très dangereux ou confidentiels.", false),
            title("Mandataire des rois", "Guilde", "Réussir une mission de portée royale ou mondiale.", false),
            title("Ligne vivante du registre", "Guilde", "Approcher les contrats que les registres modifient eux-mêmes.", false),

            title("Millionnaire qui recompte", "Économie", "Posséder au moins 1 000 000 de pièces.", false),
            title("Banquier de l'impossible", "Économie", "Posséder au moins 10 000 000 de pièces.", false),
            title("Le coffre a peur de toi", "Économie", "Accumuler une richesse assez absurde pour inquiéter les marchands.", false, "Titre drôle et économique : peut provoquer des réactions de marchands ou de banquiers."),
            title("Contrôleur de prix", "Économie", "Corriger ou vérifier une mission liée aux prix, taxes, stocks ou monnaies.", false),
            title("Commis de crise", "Économie", "Aider une ville ou un comptoir pendant une tension économique réelle.", false),
            title("Livreur sous tension", "Économie", "Mener un convoi ou une livraison fragile malgré les routes et les coûts.", false),
            title("Marchand qui recompte", "Économie", "Démêler une facture, une monnaie ou un registre qui ne tombe pas juste.", false),
            title("Négociateur de crise", "Ville / guilde", "Résoudre une mission sociale ou politique sans transformer le problème en bagarre.", false),
            title("Main des réparations", "Ville / guilde", "Aider une ville abîmée avec des actions concrètes et modestes.", false),
            title("Rumeur calmée", "Ville / guilde", "Empêcher un panneau, une rumeur ou un témoin de déclencher une panique inutile.", false),
            title("Médiateur semi-humain", "Semi-humains / sous-races", "Désamorcer une mission liée aux races, sous-races ou malentendus sociaux.", false),
            title("Parole sans morsure", "Semi-humains / sous-races", "Faire respecter une différence raciale sans en faire un pouvoir gratuit.", false),

            title("Grand chasseur de guilde", "Chasse", "Valider une chasse chiffrée de rang S ou plus.", false),
            title("Ratier des caves", "Chasse", "Nettoyer assez de nuisibles pour que les caves respirent.", false),
            title("Gobelinophobe administratif", "Chasse", "Valider des contrats gobelins avec preuves de guilde.", false),
            title("Fléau des slimes", "Chasse", "Faire assez de nettoyage gélatineux pour marquer le registre.", false),
            title("Morsure rendue", "Chasse", "Valider des chasses de loups ou bêtes à crocs.", false),
            title("Tisseur coupé", "Chasse", "Nettoyer des nids ou fils insectoïdes sans y laisser la mission.", false),
            title("Os remis au silence", "Chasse", "Accumuler des contrats morts-vivants ou funéraires.", false),
            title("Traqueur de draconides", "Chasse", "Valider une chasse de draconides ou drakes.", false),
            title("Tueur de dragon", "Chasse", "Valider une chasse officielle visant des dragons.", false),
            title("Fléau infernal", "Chasse", "Valider une chasse visant des démons ou présences infernales.", false),
            title("Bourreau des nids impossibles", "Chasse", "Éteindre une source de monstres qui refuse les rapports normaux.", false),
            title("Ça compte comme entraînement", "Chasse", "Tuer au moins 10 monstres.", false),
            title("Nettoyeur professionnel", "Chasse", "Tuer au moins 100 monstres.", false),
            title("Le compteur a abandonné", "Chasse", "Tuer au moins 1 000 monstres.", false),
            title("Pourquoi ils étaient autant ?", "Chasse", "Tuer au moins 10 000 monstres.", false, "Titre drôle de massacre administratif : surtout utilisé pour réactions de guilde et registres."),
            title("Encore un slime ?", "Drôle / chasse", "Accumuler une réputation liée aux slimes ou aux nuisibles répétitifs.", false),

            title("Cartographe de biome", "Biomes / exploration", "Corriger ou explorer une mission où le terrain compte vraiment.", false),
            title("Marcheur de frontières", "Biomes / exploration", "Explorer une limite de biome ou de carte instable.", false),
            title("Pied sûr des ruines", "Biomes / exploration", "Revenir de ruines, carrières, caveaux ou ateliers dangereux.", false),
            title("Respiration de brume", "Biomes / exploration", "Survivre à une mission où la brume, l'eau ou les canaux troublent la route.", false),
            title("Corniche tenue", "Biomes / exploration", "Revenir d'une falaise, montagne ou route draconique sans y laisser le dossier.", false),
            title("Frontière refusée", "Biomes / exploration", "Stabiliser une région que les cartes refusent de garder.", false),

            title("Inspecteur de reliques", "Bestiaire / matériaux", "Classer une ressource rare, ancienne ou liée à un boss.", false),
            title("Lecteur de traces", "Bestiaire / matériaux", "Compléter un dossier de terrain sans révéler une faiblesse gratuite.", false),
            title("Catalogue vivant", "Bestiaire / matériaux", "Étudier une archive ou un bestiaire qui semble te lire en retour.", false),
            title("Main qui ne vend pas tout", "Bestiaire / matériaux", "Conserver assez de logique dans les matériaux rares pour ne pas les brader.", false),
            title("Œil des composants maudits", "Bestiaire / matériaux", "Identifier un lot dont la qualité ou la rareté ment.", false),

            title("Lecteur d'anomalies", "Anomalie", "Valider un contrat de guilde lié aux interfaces fausses, hallucinations ou anomalies.", false),
            title("Œil fissuré", "Anomalie", "Subir ou résoudre assez d'affichages impossibles pour apprendre à douter proprement.", false),
            title("Celui qui recompte les cibles", "Anomalie", "Survivre à des missions où le nombre d'ennemis ment.", false),
            title("Erreur impossible", "Anomalie", "Vaincre une source d'interface corrompue.", false),
            title("Erreur de surface", "Anomalie", "Vaincre la première émanation de l'Anomalie.", false),
            title("Source dissipée", "Anomalie", "Vaincre la Source stable de l'Anomalie.", false, "Titre secret : l'interface reconnaît que le joueur a brisé la source sans vraiment la tuer."),
            title("Texture qui respire", "Anomalie", "Voir la Source de l'Anomalie se dissiper dans les textures.", false),
            title("Menu qui répond", "Anomalie", "Être reconnu par une anomalie qui n'aurait pas dû lire le joueur.", false),
            title("Débogueur de cauchemar", "Anomalie", "Faire face à une corruption d'interface sans suivre ses provocations.", false),
            title("Je lis les menus maintenant", "Drôle / anomalie", "Survivre à une situation où l'interface mentait assez pour devenir traumatisante.", false),

            title("Exorcisé par revanche", "Malédictions", "Briser une malédiction de boss en rebattant sa source.", false),
            title("Porteur de trace", "Malédictions", "Continuer malgré une trace de boss ancrée hors combat.", false),
            title("Ancre brisée", "Malédictions", "Retirer une trace que l'église ne pouvait pas exorciser seule.", false),
            title("Sang noir rendu", "Malédictions", "Reprendre ce que Zelef avait marqué.", false),
            title("Proie qui mord", "Malédictions", "Revenir face à Lyknir avec la marque encore fraîche.", false),
            title("Rancune éteinte", "Malédictions", "Faire taire une rancune abyssale à sa source.", false),

            title("Tombeur de boss", "Boss", "Vaincre un boss officiel.", false),
            title("Briseur de chaînes célestes", "Boss", "Vaincre Fitoria.", false),
            title("Survivant du sang noir", "Boss", "Vaincre Zelef.", false),
            title("Fissure du dernier rempart", "Boss", "Vaincre Atlas.", false),
            title("Voleur de reine", "Boss", "Vaincre Grinka.", false),
            title("Couronne brisée", "Boss", "Vaincre l'Avatar d'Azelanos.", false),
            title("Écaille de tempête", "Boss", "Vaincre le Fragment de Thamarys.", false),
            title("Ami des forêts furieuses", "Boss", "Vaincre Mojo.", false),
            title("Renard du vrai reflet", "Boss", "Vaincre le Reflet d'Inakari.", false),
            title("Acquitté du silence", "Boss", "Vaincre le Jugement Silencieux.", false),
            title("Minute volée", "Boss", "Vaincre l'Horloge des Chuchotements.", false),
            title("Nom sous les os", "Boss", "Vaincre l'Enfant des Os Enfouis.", false),
            title("Survivant de la guerre", "Boss", "Vaincre l'Avatar de Boros.", false),
            title("Cœur non soumis", "Boss", "Vaincre l'Avatar d'Anastasia.", false),
            title("Justiciable debout", "Boss", "Vaincre l'Avatar de Lexior.", false),
            title("Éveillé du cauchemar", "Boss", "Vaincre le Fragment de Luna / Onyrae.", false),
            title("Main des quatre éléments", "Boss", "Vaincre le Fragment de Syvaranelya.", false),
            title("Humain hors du registre", "Boss", "Vaincre l'Écho d'Hitogami.", false),
            title("Chance retournée", "Boss", "Vaincre l'Écho de Sérendys.", false),
            title("Passe-seuil", "Boss", "Vaincre le Gardien du Seuil.", false),
            title("Sujet sans royaume", "Boss", "Vaincre le Roi Sans Salle.", false),
            title("Nom donné à l'inconnu", "Boss", "Vaincre la Bête qui n'avait pas de nom.", false),
            title("Entre création et ruine", "Boss", "Vaincre Asterion et Nihilon.", false),
            title("Écho d'univers", "Boss", "Vaincre l'Écho fragmenté d'Obérion.", false),
            title("Regard du créateur", "Boss", "Vaincre l'Avatar affaibli de FireFlight, considéré comme boss final actuel.", false),
            title("Respiration volée", "Boss", "Vaincre le Souffle sans Visage.", false),
            title("Clou retiré", "Boss", "Vaincre la Marionnette aux Mille Clous.", false),
            title("Destin écarté", "Boss", "Vaincre la Manifestation de Moiran.", false),
            title("Berger des âmes", "Boss", "Vaincre le Cerf des Âmes Égarées.", false),
            title("Briseur de horde", "Boss", "Vaincre Gorvald.", false),
            title("Sang royal refusé", "Boss", "Vaincre Serana.", false),
            title("Filet arraché", "Boss", "Vaincre Draiite.", false),
            title("Vérité sans miroir", "Boss", "Vaincre les Jumelles du Miroir Fendu.", false),
            title("Avant la dernière porte", "Boss", "Vaincre tous les boss disponibles sauf le boss final actuel.", false),
            title("Une invitation ?", "Boss", "Obtenir la preuve qu'il ne reste plus que le dernier boss actuel.", false, "Titre de seuil final : peut servir à ouvrir des dialogues, invitations ou avertissements de fin."),
            title("Celui qui a tout vaincu", "Boss", "Battre le boss final actuel après avoir vaincu les autres boss.", false),

            title("Compagnon de revanche", "Coop", "Participer à une victoire de boss qui libère une trace d'un allié.", false),
            title("Partage de prime propre", "Coop", "Recevoir une récompense de groupe avec vraie participation.", false),
            title("Dernier debout du groupe", "Coop", "Survivre à une victoire de groupe quand le combat a vraiment coûté cher.", false),

            title("Fin de chapitre", "Histoire", "Terminer le mode histoire principal lorsqu'il sera branché.", false),
            title("Et ce n'était que le début...", "Histoire", "Terminer le mode histoire et révéler que la suite commence seulement.", false),
            title("La fin ???", "Complétion", "Atteindre la vraie conclusion globale du jeu.", false, "Titre de conclusion : volontairement exclu du prérequis du titre ultime.", false),
            title("Complétionniste de l'impossible", "Complétion", "Posséder tous les titres requis sauf lui-même et La fin ???.", false, "Titre ultime : preuve que le personnage a presque tout accompli dans le registre.", false),

            title("J'ai glissé chef", "Drôle", "Tomber ou mourir d'une façon assez ridicule pour que le registre ait honte.", false),
            title("Promis c'était calculé", "Drôle", "Survivre à une situation qui ressemble beaucoup trop à une erreur.", false),
            title("Le plan était de survivre", "Drôle", "Gagner ou revenir d'un combat alors que le plan stratégique était flou.", false),
            title("Pas payé assez cher pour ça", "Drôle", "Accepter ou terminer une mission dont la récompense semblait trop petite pour le chaos.", false),
            title("Désolé la guilde", "Drôle", "Accumuler assez d'échecs, de dégâts ou d'anomalies pour embarrasser le comptoir.", false),
            title("Potion avant réflexion", "Drôle", "Abuser des consommables ou survivre grâce à une potion prise au dernier moment.", false),
            title("Inventaire trop lourd", "Drôle", "Accumuler assez d'objets ou matériaux pour que le sac devienne une menace.", false),
            title("Presque héroïque", "Drôle", "Faire quelque chose d'utile, mais pas assez propre pour que la guilde appelle ça héroïque.", false)
        };

        return titles;
    }

    const TitleDefinition* findTitleDefinition(const std::string& titleName)
    {
        const std::vector<TitleDefinition>& titles = availableTitleDefinitions();
        for (const TitleDefinition& definition : titles)
        {
            if (definition.name == titleName)
            {
                return &definition;
            }
        }
        return nullptr;
    }

    std::string unlockHintFor(const std::string& titleName)
    {
        const TitleDefinition* definition = findTitleDefinition(titleName);
        return definition ? definition->unlockHint : "Obtention précise non répertoriée dans le catalogue actuel.";
    }

    std::string effectFor(const std::string& titleName)
    {
        const TitleDefinition* definition = findTitleDefinition(titleName);
        return definition ? definition->effect : "Effet non répertorié : titre conservé comme trophée historique du personnage.";
    }

    bool isVisibleBeforeUnlock(const std::string& titleName)
    {
        const TitleDefinition* definition = findTitleDefinition(titleName);
        return definition ? definition->visibleBeforeUnlock : false;
    }

    bool hasAllUltimatePrerequisites(const std::vector<std::string>& ownedTitles)
    {
        for (const TitleDefinition& definition : availableTitleDefinitions())
        {
            if (!definition.requiredForUltimate)
            {
                continue;
            }

            if (!containsTitle(ownedTitles, definition.name))
            {
                return false;
            }
        }
        return true;
    }
}
