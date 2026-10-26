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
        bool requiredForUltimate = true,
        const std::string& ultimateAlternativeGroup = ""
    )
    {
        TitleDefinition definition;
        definition.name = name;
        definition.category = category;
        definition.unlockHint = unlockHint;
        definition.effect = effect.empty() ? genericEffectForCategory(category) : effect;
        definition.visibleBeforeUnlock = visibleBeforeUnlock;
        definition.requiredForUltimate = requiredForUltimate;
        definition.ultimateAlternativeGroup = ultimateAlternativeGroup;
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
            title("Rencontre avec notre cobaye", "Système / tutoriel", "Lancer le tutoriel avec Jean-Michel Test au moins une fois.", true, "Trophée de compte : prouve que tu as rencontré le cobaye officiel. Aucun bonus de combat ; Jean-Michel a déjà assez de problèmes comme ça."),
            title("J'ai lu le manuel", "Système / tutoriel", "Terminer le tutoriel de Jean-Michel Test.", true, "Trophée de compte : confirme que les bases de la ville, des contrats, de l'exploration et du combat ont été vues."),
            title("Ami du petit Dinotofu", "Système / compagnon", "Consulter le compagnon Dinotofu au moins une fois.", true, "Titre de confort : le petit Dinotofu sait désormais que tu viens parfois demander conseil. Aucun bonus de combat direct.", false),
            title("Œil de serrure", "Combat / observation", "Avec un profil discret, répéter l'observation tactique jusqu'à apprendre à lire serrures, pièges et failles.", false, "Titre d'apprentissage : marque l'acquisition de la lecture des serrures et failles. Il reste hors complétion ultime car son accès dépend du profil joué.", false),
            title("Une sorte de titre ?", "Système", "Obtenir son premier titre, quel qu'il soit.", true, "Et une sorte de description..."),
            title("On ne dira rien...", "Système / altération", "Altérer volontairement au moins un personnage du compte avec un code de triche officiel.", false, "Trophée spécial de compte : le registre a parfaitement vu ce qui s'est passé. Il reste le seul trophée de compte accordé par la triche et ne compte jamais pour la complétion ultime.", false),
            title("Aventurier", "Guilde", "Réussir l'inscription officielle de la guilde.", true),
            title("Aventurier rang F", "Guilde", "Valider des contrats officiels après inscription.", true),
            title("Aventurier rang E", "Guilde", "Monter le dossier de guilde avec assez de contrats et de niveau.", true),
            title("Aventurier rang D", "Guilde", "Atteindre le rang D de guilde.", true),
            title("Un bon entraînement", "Guilde / entraînement", "Battre Torvald dans un 1v1 amical de guilde.", true, "Reconnaissance d'entraînement : Torvald sait que tu tiens un duel propre et peut réagir autrement aux bases d'équipe."),
            title("On recrute !", "Guilde / clan", "Battre Torvald puis accepter d'ouvrir légalement ton équipe aux candidats.", true, "Titre actif de recrutement : augmente les occasions de croiser des PNJ recrutables jusqu'au groupe solo complet de 6 personnes (joueur + 5 recrues)."),
            title("Anti clan", "Guilde / clan", "Battre Torvald puis choisir une posture de groupe fermé.", true, "Titre actif de fermeture : bloque volontairement les rencontres de PNJ recrutables tant qu'il est équipé."),
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

            title("Premier éclat de platine", "Économie", "Obtenir et posséder au moins une pièce de platine pour la première fois.", false, "Titre de seuil monétaire : le platine attire immédiatement l’attention et peut colorer les réactions de marchands, notables ou aventuriers."),
            title("Millionnaire qui recompte", "Économie", "Posséder au moins 1 000 000 de pièces.", false),
            title("Banquier de l'impossible", "Économie", "Posséder au moins 10 000 000 de pièces.", false),
            title("Le coffre a peur de toi", "Économie", "Accumuler une richesse assez absurde pour inquiéter les marchands.", false, "Titre drôle et économique : peut provoquer des réactions de marchands ou de banquiers."),
            title("Contrôleur de prix", "Économie", "Corriger ou vérifier une mission liée aux prix, taxes, stocks ou monnaies.", false),
            title("Commis de crise", "Économie", "Aider une ville ou un comptoir pendant une tension économique réelle.", false),
            title("Livreur sous tension", "Économie", "Mener un convoi ou une livraison fragile malgré les routes et les coûts.", false),
            title("Marchand qui recompte", "Économie", "Démêler une facture, une monnaie ou un registre qui ne tombe pas juste.", false),
            title("Aide de comptoir", "Économie / commerce", "Rendre un premier service propre à Prunigil.", true, "Confiance marchande légère : Prunigil et certains vendeurs reconnaissent une aide déjà formée."),
            title("Apprenti de Prunigil", "Économie / commerce", "Accumuler plusieurs services réussis au comptoir de Prunigil.", true, "Confiance marchande : peut ouvrir des recommandations et quelques vendeurs temporaires."),
            title("Vérificateur de registre", "Économie / commerce", "Traiter assez de contrats, comptes et documents sensibles pour être reconnu comme vérificateur.", false, "Confiance marchande avancée : davantage de vendeurs temporaires et de suites de dossiers peuvent apparaître."),
            title("Mandataire du comptoir", "Économie / commerce", "Atteindre le plus haut palier actuel de confiance de Prunigil.", false, "Confiance marchande élevée : les vendeurs itinérants les plus prudents acceptent les recommandations de Prunigil."),
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
            title("Triplement maudit", "Malédictions / défi", "Porter au moins trois malédictions actives en même temps.", false, "Titre de défi maudit : les exorcistes, la guilde et certains vendeurs remarquent que trois problèmes surnaturels cohabitent déjà sur toi."),
            title("Le quatrième problème", "Malédictions / défi", "Remporter un combat en portant au moins trois malédictions actives.", false, "Titre de défi : tu étais déjà triplement maudit, mais l'adversaire a quand même réussi à devenir le problème supplémentaire."),

            title("Pas une égratignure", "Défis de guilde", "Remporter un combat sans subir le moindre dégât.", false, "Marque d'exploit : peut provoquer des réactions de guilde, sans bonus de combat direct."),
            title("À l'ancienne", "Défis de guilde", "Remporter un combat uniquement avec des attaques simples, sans compétence ni consommable.", false),
            title("Boss sans réserve", "Défis de guilde / boss", "Vaincre un boss sans utiliser de consommable.", false),
            title("Seulement toi et le boss", "Défis de guilde / boss", "Vaincre un boss sans consommable, compétence de classe ni technique d'arme.", false),
            title("Encore debout, malheureusement", "Défis de guilde / drôle", "Remporter un combat avec au plus un quart de ses PV maximum.", false),
            title("Trois victoires, zéro gorgée", "Défis de guilde", "Remporter trois combats propres sans utiliser de consommable.", false),
            title("Le mur qui répond", "Défis de guilde", "Défendre plusieurs fois puis remporter le combat.", false),
            title("Victoire sous mauvaise influence", "Défis de guilde / malédictions", "Remporter plusieurs combats en portant au moins une malédiction.", false),
            title("Élite sans fiole", "Défis de guilde / élite", "Vaincre une créature élite ou un mini-boss sans consommable.", false),
            title("Ça devait être rapide", "Défis de guilde / drôle", "Remporter un combat ayant duré au moins six tours d'action.", false),
            title("Six problèmes de moins", "Défis de guilde", "Éliminer six créatures dans le cadre d'un même défi.", false),
            title("Survivant des caisses", "Défis marchands", "Remporter le défi amical de Bob et Maurice avec uniquement le contenu des caisses.", false),
            title("Témoin du marchand bleu", "Légendes / marchands", "Rencontrer le vendeur héroïque dont les rumeurs paraissaient trop absurdes.", false),
            title("Les deux du même comptoir", "Marchands / drôle", "Rencontrer Bob et Maurice et comprendre qu'ils ne voyageront jamais séparément.", false),
            title("Personne ne reste derrière", "Défis de groupe", "Remporter un combat collectif avec tous les aventuriers encore debout.", false),
            title("Escouade à l'ancienne", "Défis de groupe", "Remporter un combat collectif uniquement avec des attaques simples et de la défense.", false),
            title("Le groupe compte aussi les invoqués", "Défis de groupe / invocations", "Gagner un combat où une invocation alliée a réellement participé aux dégâts.", false),
            title("Groupe sans réserve", "Défis de groupe", "Remporter un combat collectif sans aucun consommable utilisé par l'équipe.", false),
            title("Porte-marque de la guilde", "Défis de guilde", "Échanger assez de Marques de défi pour recevoir la reconnaissance officielle du panneau.", false),
            title("Le troisième avis n'était pas demandé", "Marchands / drôle", "Terminer la première longue suite commerciale de Bob et Maurice.", false),

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

            // Titres ajoutés lors de l’audit rétrospectif V3.51.40.
            // Les titres de parcours général restent visibles lorsqu’ils n’annoncent pas une découverte majeure.
            title("Bon pour accord", "Économie / guilde", "Obtenir son premier prêt de guilde.", true),
            title("La confiance se mérite", "Économie / guilde", "Atteindre un niveau très élevé de confiance financière auprès de la guilde.", true),
            title("Bon payeur", "Économie / guilde", "Rembourser plusieurs prêts intégralement sans retard.", true),
            title("Avant l'échéance", "Économie / guilde", "Solder intégralement un prêt avant sa date limite.", true),
            title("Jusqu'au dernier cuivre", "Économie / guilde", "Rembourser une dette importante malgré une bourse presque vide.", false),
            title("Dans le rouge", "Économie / guilde", "Approcher son plafond de découvert puis revenir à zéro légitimement.", true),
            title("Dette réglée", "Économie / guilde", "Sortir d'une situation avec prêt et découvert actifs en même temps.", true),
            title("On peut s'arranger", "Économie / guilde", "Négocier une prolongation avant échéance puis rembourser entièrement.", true),
            title("La parole vaut de l'or", "Économie / guilde", "Respecter plusieurs contrats financiers consécutifs.", true),
            title("Sous clé", "Économie / coffre", "Acheter et utiliser le coffre-fort personnel.", true),
            title("Capital confiance", "Économie / guilde", "Atteindre le plafond maximal de prêt sans défaut de paiement.", false),
            title("Toute ma fortune sur moi", "Économie / drôle", "Transporter une fortune importante sans la sécuriser dans le coffre-fort.", false, "Titre surtout roleplay : les marchands et voleurs ont de bonnes raisons de te regarder.", false),
            title("Chaque pièce compte", "Économie", "Avancer longtemps avec très peu d'argent sans dette ni triche.", false),

            title("Personne derrière", "Compagnons", "Ramener une recrue mise KO jusqu'aux soins au lieu de l'abandonner.", true),
            title("Ce n'était pas qu'un contrat", "Compagnons", "Terminer la quête personnelle d'une recrue engagée.", false),
            title("Vieille équipe", "Compagnons", "Conserver longtemps le même noyau de compagnons.", true),
            title("On te ramènera", "Compagnons", "Faire soigner ou récupérer plusieurs compagnons tombés au lieu de les remplacer.", true),
            title("La facture est réglée", "Compagnons / économie", "Rembourser intégralement une dette importante liée aux soins d'un compagnon.", false),
            title("À vos ordres", "Commandement", "Mener plusieurs combats avec une équipe sous son commandement.", true),
            title("Chef de terrain", "Commandement", "Réussir plusieurs rotations tactiques comme chef.", true),
            title("Tout le monde rentre", "Commandement / compagnons", "Gagner un combat difficile sans perte définitive de compagnon.", false),
            title("Je couvre tes arrières", "Compagnons", "Sauver plusieurs fois un compagnon qui aurait dû tomber.", false),
            title("Pas besoin de le répéter", "Commandement / loyauté", "Atteindre une obéissance et une loyauté très élevées avec un compagnon.", false),
            title("Tu avais raison", "Commandement", "Désobéir à un ordre puis être reconnu comme ayant évité une mauvaise décision.", false, "Titre contextuel : récompense un jugement réellement justifié, pas la désobéissance gratuite.", false),
            title("Mauvais ordre", "Commandement", "Survivre après avoir refusé un ordre objectivement catastrophique.", false, "Titre contextuel rare : dépend d'un ordre réellement mauvais et de ses conséquences.", false),
            title("Dernier rempart", "Commandement / survie", "Rester seul actif pendant que l'équipe est hors combat et tenir assez longtemps.", false),
            title("Relève assurée", "Commandement / réserve", "Gagner grâce à un membre entré depuis la réserve.", true),
            title("Le chef tombe, l'équipe reste", "Commandement / survie", "Gagner après le KO du chef de l'équipe.", false),
            title("Je prends la suite", "Commandement", "Assumer temporairement le commandement après la chute du chef.", false),

            title("Comme un seul homme", "Duo / relations", "Réussir plusieurs techniques duo.", true),
            title("Sans un mot", "Duo / relations", "Réussir une technique duo avec une maîtrise très élevée.", false),
            title("Je savais que tu serais là", "Duo / relations", "Être sauvé plusieurs fois par le même compagnon.", false),
            title("Jusqu'au bout", "Duo / relations", "Voir un compagnon rester à tes côtés malgré une situation très défavorable.", false),
            title("Confiance aveugle", "Duo / relations", "Atteindre une relation et une loyauté exceptionnelles.", false),
            title("Mieux à deux", "Duo / relations", "Battre une cible importante grâce à une technique combinée.", false),
            title("Parfaitement synchronisés", "Duo / relations", "Réussir une technique duo à une fiabilité proche du maximum.", false),

            title("Tout est une arme", "Combat tactique", "Exploiter plusieurs types de matériaux, pièges ou enduits tactiques distincts.", false),
            title("J'avais prévu le coup", "Combat tactique", "Interrompre plusieurs familles de techniques ennemies préparées.", false),
            title("Casseur de rythme", "Combat tactique", "Interrompre plusieurs types de charges, rituels ou attaques lourdes.", false),
            title("Maître de sa voie", "Combat / maîtrise", "Pousser une vraie technique jusqu'à sa maîtrise maximale.", true),
            title("Pas besoin de force brute", "Combat tactique", "Gagner un combat difficile surtout grâce au contrôle, aux ouvertures et à la préparation.", false),

            title("Pas encore.", "Mort / survie", "Réussir l'unique résistance spirituelle d'une première mort létale.", false),
            title("Trois fois non", "Mort / survie", "Obtenir les trois réussites spirituelles avant les trois échecs.", false),
            title("Deuxième souffle", "Mort / survie", "Revenir d'une situation létale grâce à un allié survivant.", false),
            title("Ils m'ont ramené", "Mort / survie", "Être sauvé après que l'équipe a continué le combat sans toi.", false),
            title("Le dernier témoin", "Mort / survie", "Être le seul survivant d'un combat majeur.", false),
            title("Revenu de loin", "Mort / survie", "Survivre à une résolution létale avec une faible probabilité de sauvetage.", false),
            title("Pas aujourd'hui", "Mort / survie", "Survivre légitimement à plusieurs situations où la mort définitive était réellement possible.", false),
            title("On me croyait mort", "Mort / survie", "Revenir après une disparition ou un KO prolongé puis poursuivre l'aventure.", false),
            title("Je leur dois la vie", "Mort / relations", "Être sauvé d'une issue létale grâce à un compagnon précis.", false),

            title("Je le sens...", "Mana / progression", "Éveiller son mana légitimement après une vraie quête dédiée.", true),
            title("Premier écho", "Mana / progression", "Atteindre la première étape réelle de maîtrise du mana.", true),
            title("Résonance", "Mana / progression", "Atteindre une maîtrise avancée du système Écho / Résonance.", false),
            title("À sec", "Mana / survie", "Gagner un combat après être tombé à zéro mana.", false),
            title("Plus rien dans les veines", "Mana / survie", "Survivre à un épuisement de mana sévère.", false),
            title("Pas né pour ça", "Progression / hybride", "Devenir compétent en mêlée avec un personnage initialement pur mage.", false),
            title("Finalement, peut-être", "Progression / hybride", "Devenir réellement compétent dans une discipline initialement faible.", false),
            title("Deux voies, un seul combattant", "Progression / hybride", "Construire un véritable style hybride plutôt que simplement changer d'équipement.", false),

            title("Je crois avoir compris...", "Langues", "Réussir une interaction importante avec une compréhension intermédiaire d'une langue étrangère.", true),
            title("Sans traducteur", "Langues", "Comprendre complètement une rencontre étrangère grâce à une langue apprise.", true),
            title("Les mots ouvrent des portes", "Langues", "Résoudre ou débloquer une situation grâce à une langue apprise.", true),
            title("Polyglotte", "Langues", "Atteindre un niveau conversationnel ou courant dans plusieurs langues étrangères.", true),
            title("Ça ressemblait à du gobelin", "Langues / drôle", "Comprendre pour la première fois une partie seulement d'un texte étranger.", false),

            title("De bouche à oreille", "Rumeurs / mémoire", "Voir un fait lié au personnage voyager réellement jusqu'à une autre ville.", false),
            title("On raconte que...", "Rumeurs / mémoire", "Devenir le sujet d'une rumeur locale crédible.", false),
            title("J'ai entendu autre chose", "Rumeurs / mémoire", "Découvrir deux versions contradictoires du même événement.", false),
            title("La vérité voyage lentement", "Rumeurs / mémoire", "Faire corriger progressivement une rumeur par des preuves crédibles.", false),
            title("Tout se sait", "Rumeurs / mémoire", "Voir une action importante devenir connue dans plusieurs villes sans omniscience artificielle.", false),

            title("Curieux de nature", "Biomes / exploration", "Déclencher des interactions propres à plusieurs biomes différents.", true),
            title("Rien n'est décoratif", "Biomes / exploration", "Interagir avec de nombreux éléments environnementaux uniques.", false),
            title("Là où personne ne regarde", "Biomes / exploration", "Découvrir un micro-événement particulièrement rare.", false),
            title("J'ai laissé des traces", "Biomes / exploration", "Accumuler des événements d'exploration réellement mémorisés.", false),
            title("Plus loin encore", "Biomes / exploration", "Découvrir de nombreux biomes avancés.", false),

            title("On se connaît, non ?", "Rivaux", "Recroiser pour la première fois un rival vivant déjà rencontré.", false),
            title("Cicatrices partagées", "Rivaux", "Retrouver un rival portant une marque issue d'un ancien affrontement.", false),
            title("Il revient encore...", "Rivaux", "Voir le même rival revenir plusieurs fois.", false),
            title("Cette fois, c'est fini", "Rivaux", "Éliminer définitivement un rival ayant survécu à plusieurs rencontres.", false),
            title("Maintenant je te connais", "Boss / rivaux", "Vaincre un adversaire adaptatif après plusieurs rencontres où il a appris de toi.", false),
            title("Tu m'avais oublié ?", "Boss / rivaux", "Revenir vaincre une cible qui t'avait auparavant mis KO.", false),
            title("J'avais dit qu'il tomberait", "Boss / rivaux", "Vaincre une cible majeure après plusieurs défaites face à elle.", false),
            title("Plus gros que prévu", "Boss / chasse", "Vaincre un boss significativement au-dessus de l'estimation initiale.", false),
            title("Mission accomplie, techniquement", "Boss / drôle", "Voir son équipe achever le boss après son propre KO, sans clear personnel.", false),
            title("À mon tour", "Boss / réserve", "Entrer depuis la réserve contre un boss et jouer un rôle décisif.", false),
            title("Chasseur de chasseurs", "Boss / chasse", "Éliminer un ennemi spécialisé dans la traque du joueur.", false),
            title("Une vieille affaire", "Rivaux", "Mettre fin à une rivalité persistante après une longue histoire commune.", false),

            title("Cette lame a une histoire", "Équipement / mémoire", "Accumuler plusieurs souvenirs sur le même exemplaire précis d'une arme.", false),
            title("Pas qu'un bout de métal", "Équipement / mémoire", "Voir un équipement recevoir un surnom émergent lié à son histoire.", false),
            title("Je ne la remplacerai pas", "Équipement / mémoire", "Conserver et utiliser le même équipement pendant une très longue période.", false),
            title("Lié à l'acier", "Équipement / mémoire", "Construire une vraie histoire persistante autour d'un objet lié.", false),
            title("C'est bien la mienne", "Équipement / mémoire", "Perdre puis récupérer exactement le même équipement grâce à son identité persistante.", false),
            title("Rends-moi ça.", "Équipement / mémoire", "Récupérer un bien réellement volé par Grinka ou par un rival voleur retrouvé plus tard.", false),
            title("Ça valait le coup ?", "Équipement / enchantement", "Perdre réellement un équipement lors d'un échec critique d'enchantement.", false),
            title("Pas cette fois", "Équipement / enchantement", "Voir un sceau anti-casse sauver un équipement qui aurait dû être détruit.", false),
            title("Trophée de guerre", "Équipement / JcJ", "Obtenir en JcJ dangereux l'équipement exact d'un adversaire.", false),
            title("Marché conclu", "Équipement / échange", "Effectuer un vrai échange transactionnel entre personnages.", true),

            // V3.57.01 — consolidation des systèmes apparus depuis la dernière grande passe titres.
            title("On a un QG !", "Clan / QG", "Achever la construction du premier QG de clan.", true),
            title("Ce n'est plus une cabane", "Clan / QG", "Agrandir le QG jusqu'au niveau 2.", true),
            title("Ça devient sérieux", "Clan / QG", "Atteindre le QG niveau 3 avec ses vrais prérequis.", false),
            title("Il reste une chambre ?", "Clan / QG", "Porter la capacité du QG à au moins cinq lits.", false),
            title("Le tableau travaille aussi", "Clan / QG", "Installer le tableau de quêtes du clan niveau 2.", true),
            title("La caisse du patron", "Clan / QG", "Retirer pour la première fois un vrai versement de la caisse de missions.", false),
            title("Une tournée pour le clan !", "Clan / QG", "Profiter pour la première fois du bar / de la salle commune du QG.", false),
            title("On avait dit petite fête", "Clan / QG", "Organiser une première fête réelle au QG.", false),
            title("Comme à la maison", "Clan / QG", "Installer le premier niveau de confort et décoration du QG.", false),
            title("Ça va forger.", "Clan / artisanat", "Construire l'atelier de forge niveau 1 du QG.", true),
            title("Forgeron maison", "Clan / artisanat", "Recruter le premier forgeron permanent du clan.", true),
            title("C'est en forgeant...", "Clan / artisanat", "Faire progresser le forgeron du clan jusqu'au niveau d'artisan 5 par de vraies commandes.", false),
            title("Chacun sa spécialité", "Clan / artisanat", "Faire choisir au forgeron du clan sa première spécialisation permanente.", false),
            title("Prix d'ami", "Clan / artisanat", "Payer une vraie réparation bénéficiant de la remise du forgeron du clan.", false),

            title("Enfin chez moi", "Foyer", "Obtenir son premier logement principal.", true),
            title("Le loyer, cette quête hebdomadaire", "Foyer", "Payer trois loyers réels de son logement principal.", false, "Trophée de foyer : valide le parcours locatif régulier.", true, "home_financing_route"),
            title("Plus de propriétaire", "Foyer", "Acheter définitivement son logement principal.", false, "Trophée de foyer : valide le parcours d'achat direct.", true, "home_financing_route"),
            title("Mon lit, mes règles", "Foyer", "Dormir au moins une fois dans son propre logement.", true),
            title("J'ai dit oui", "Foyer / mariage", "Valider un mariage réel après les prérequis prévus.", false, "Titre de relation : la mécanique complète de mariage doit être jouable avant qu'il ne devienne un prérequis ultime.", false),
            title("Une place pour deux", "Foyer / mariage", "Installer réellement son conjoint PNJ au foyer.", false, "Titre de foyer vivant : gardé hors complétion ultime tant que la route mariage complète n'est pas ouverte.", false),
            title("Jusqu'à ce que la sauvegarde nous sépare", "Foyer / mariage", "Valider un mariage avec un conjoint PNJ.", false, "Titre de mariage PNJ : il pourra compter pour l'ultime lorsque toute la route mariage/divorce sera honnêtement jouable.", false),
            title("Jusqu'à ce que le ping nous sépare", "Foyer / mariage / coop", "Valider un mariage joueur-joueur.", false, "Titre coopératif : volontairement hors complétion ultime car il dépend d'un autre joueur humain.", false),

            title("Dossier : retrouvée", "Histoire / Hestia", "Terminer réellement le sauvetage de la personne disparue et ramener Hestia.", false),
            title("Une colocataire de plus", "Histoire / Hestia", "Proposer à Hestia de vivre dans le logement du joueur.", false, "Titre de relation : hors complétion ultime tant qu'une mort définitive d'alliée peut couper cette route sans alternative équivalente.", false),
            title("Elle apprend... doucement", "Histoire / Hestia", "Faire participer Hestia à au moins cinq vrais entraînements ou combats.", false, "Titre de progression de compagne : non requis tant que la mort définitive peut interrompre ce parcours.", false),
            title("Un gant un peu trop attachant", "Histoire / Hestia", "Recevoir l'artefact d'âme lié à Hestia.", false, "Titre d'artefact : non requis tant que la route Hestia peut être interrompue définitivement.", false),
            title("Deux formes valent mieux qu'une", "Histoire / Hestia", "Débloquer puis choisir la seconde forme de l'artefact d'Hestia.", false, "Titre d'artefact avancé : non requis tant que la route Hestia peut être interrompue définitivement.", false),
            title("Elle ne m'a pas laissé mourir", "Histoire / Hestia", "Être réellement sauvé par le sacrifice d'Hestia.", false, "Titre secret de sacrifice : événement exceptionnel et potentiellement incompatible avec d'autres destins d'Hestia.", false),
            title("C'était ma première vraie amie", "Histoire / Hestia", "Construire avec Hestia une confiance réelle à travers plusieurs visites et moments vécus.", false, "Titre secret de relation : une trace émotionnelle, pas un simple seuil affiché. Non requis tant qu'une alternative de mémoire n'existe pas en cas de mort définitive.", false),

            title("Sur la piste", "Primes / traque", "Faire réellement progresser l'enquête d'une prime ciblée.", true),
            title("Viens sans faire d'histoire", "Primes / capture", "Obtenir la reddition d'une cible et la capturer vivante.", false),
            title("Il fallait vraiment en arriver là ?", "Primes / capture", "Capturer vivante une cible après confrontation.", false),
            title("Pas assez loin", "Primes / traque", "Capturer une cible après qu'elle a réellement changé de ville.", false),
            title("Tu peux courir...", "Primes / traque", "Voir une même cible accumuler au moins trois tentatives d'évasion avant résolution.", false),
            title("J'avais gardé la piste", "Primes / traque", "Exploiter avec succès une piste inter-ville persistante laissée par une cible.", false),

            title("Évidemment, ce n'était pas fini.", "Quêtes / suites", "Terminer sa première phase 2 obligatoire.", true),
            title("Puisqu'on y est...", "Quêtes / suites", "Accepter puis terminer une phase 2 facultative.", true),
            title("Facteur officiel", "Quêtes / inter-ville", "Terminer un premier contrat rémunéré de transmission d'informations inter-ville.", true),
            title("Ce n'est pas une lettre, ça.", "Quêtes / inter-ville", "Terminer un premier transport rémunéré de matériaux inter-ville.", true),
            title("Avec correspondance", "Quêtes / inter-ville", "Terminer un contrat inter-ville comportant plusieurs relais réels.", false),

            title("J'ai touché de l'herbe", "Ville / événements", "Participer activement à la fête des moissons.", false),
            title("On barricade d'abord", "Ville / événements", "Participer aux préparatifs d'une mobilisation contre un raid annoncé.", false),
            title("Le convoi est arrivé... à peu près", "Économie / convois", "Résoudre puis clôturer un premier incident commercial jouable.", false),
            title("Il manquait quelque chose au chargement", "Économie / convois", "Faire déboucher l'enquête d'un incident commercial sur une vraie prime séparée.", false),

            // V3.51.51 — stockage, encombrement, sacs et morphologies.
            // Ces titres décrivent des styles de jeu parfois incompatibles entre eux : aucun n'est requis pour la complétion ultime.
            title("Enfin de la place", "Économie / stockage", "Acheter son premier coffre municipal.", true, "Trophée logistique : marque l'accès au stockage municipal local.", false),
            title("On pousse les murs", "Économie / stockage", "Améliorer un coffre municipal pour la première fois.", true, "Trophée logistique : marque le premier agrandissement d'un coffre.", false),
            title("Chaque chose à sa place", "Économie / stockage", "Installer l'amélioration Place optimisée sur un coffre de niveau 2 ou plus.", true, "Trophée logistique : récompense l'optimisation physique du rangement.", false),
            title("Optimisation maximale", "Économie / stockage", "Cumuler Place optimisée et Rangement anticipé sur un coffre niveau 5.", false, "Trophée logistique : le coffre utilise exactement 25 % de place en moins qu'à rangement brut.", false),
            title("Plein à craquer", "Inventaire / encombrement", "Atteindre au moins 100 % de la capacité portée.", true, "Trophée d'encombrement, sans bonus direct.", false),
            title("Ça rentre encore.", "Inventaire / encombrement", "Atteindre au moins 200 % de la capacité portée.", false, "Trophée drôle : le personnage doit s'alléger avant de pouvoir repartir.", false),
            title("Mule de compétition", "Inventaire / encombrement", "Terminer une exploration avec plus de 150 % de charge.", false, "Trophée de transport extrême.", false),
            title("Je garde tout", "Inventaire / encombrement", "Revenir en ville presque immobilisé sans avoir abandonné son butin en chemin.", false, "Trophée de collection obstinée.", false),
            title("Grand ménage", "Inventaire / stockage", "Décharger d'un coup la grande majorité d'un inventaire très rempli.", false, "Trophée de rangement.", false),
            title("Ça respire enfin", "Inventaire / stockage", "Passer d'une charge supérieure à 150 % à moins de 50 % grâce à une vente ou un stockage réel.", false, "Trophée de rangement radical.", false),
            title("Pas un espace de perdu", "Économie / stockage", "Remplir réellement un coffre municipal à 100 % de sa capacité effective.", false, "Trophée de stockage compact.", false),
            title("Conservateur", "Économie / stockage", "Remplir à 100 % un coffre municipal de niveau 5.", false, "Trophée prestigieux de collection et de conservation.", false),
            title("Réseau de stockage", "Économie / stockage", "Posséder des coffres municipaux dans au moins trois villes différentes.", false, "Trophée logistique multi-ville.", false),
            title("Empire logistique", "Économie / stockage", "Posséder des coffres municipaux dans au moins cinq villes différentes.", false, "Trophée logistique avancé.", false),
            title("Partout chez moi", "Économie / stockage", "Posséder un coffre dans un grand nombre de villes principales.", false, "Trophée de réseau de stockage étendu.", false),
            title("Collectionneur itinérant", "Économie / stockage", "Posséder plusieurs coffres municipaux de niveau 5 dans des villes différentes.", false, "Trophée de collection multi-ville.", false),
            title("Armurier malgré moi", "Équipement / collection", "Stocker au moins 100 armes différentes ou exemplaires conservés dans ses coffres.", false, "Trophée de collection d'armes.", false),
            title("Une pour chaque occasion", "Équipement / collection", "Stocker au moins 50 armures dans ses coffres.", false, "Trophée de collection d'armures.", false),
            title("Ça peut toujours servir", "Bestiaire / matériaux", "Accumuler une très grande réserve de matériaux dans ses coffres.", false, "Trophée de collection de matériaux.", false),
            title("Réserve stratégique", "Bestiaire / matériaux", "Stocker une grande diversité de familles de matériaux.", false, "Trophée de préparation matérielle.", false),
            title("Sous le matelas", "Économie / stockage", "Stocker une très grosse fortune en monnaie physique dans un coffre.", false, "Trophée futur de monnaie physique stockée ; le coffre doit réellement contenir les pièces.", false),
            title("Je préfère le tangible", "Économie / monnaie", "Conserver une très grande fortune sous forme de pièces physiques plutôt que de soldes abstraits.", false, "Trophée de monnaie physique.", false),
            title("Ça pèse moins comme ça", "Économie / monnaie", "Réduire fortement le nombre de pièces physiques transportées par un change vers de grosses dénominations.", false, "Trophée de change et de logistique monétaire.", false),
            title("Prêt pour le long voyage", "Inventaire / sacs", "Acheter le sac à dos de capacité maximale actuelle.", true, "Trophée de progression du sac.", false),
            title("Même ça, j'ai rempli", "Inventaire / sacs", "Atteindre 100 % de capacité avec le plus grand sac disponible.", false, "Trophée d'encombrement avancé.", false),
            title("Collection de sacs", "Inventaire / sacs", "Acheter successivement tous les paliers de sacs disponibles.", false, "Trophée de progression logistique.", false),
            title("Voyageur organisé", "Biomes / exploration", "Réaliser une exploration en restant sous 50 % d'encombrement malgré les trouvailles.", false, "Trophée de préparation légère.", false),
            title("Préparation impeccable", "Biomes / exploration", "Enchaîner plusieurs explorations sans jamais dépasser 85 % de capacité.", false, "Trophée de discipline logistique.", false),
            title("Minimaliste", "Biomes / exploration", "Partir en exploration avec moins de 10 % de la capacité d'inventaire utilisée.", true, "Trophée de départ volontairement léger.", false),
            title("L'essentiel seulement", "Biomes / exploration", "Partir sous 10 % et terminer la sortie sans dépasser 50 % de charge.", false, "Trophée de minimalisme tenu pendant toute une sortie.", false),
            title("Je voyage léger", "Biomes / exploration", "Enchaîner plusieurs sorties en restant sous 50 % de charge.", false, "Trophée de constance légère.", false),
            title("Un corps façonné", "Morphologie / progression", "Achever volontairement une transformation durable de morphologie par un parcours long et cohérent.", false, "Trophée de transformation corporelle volontaire.", false),
            title("Plus solide qu'avant", "Morphologie / progression", "Passer volontairement d'une morphologie Fine à Standard.", false, "Trophée de transformation vers une carrure plus solide.", false),
            title("Taillé pour porter", "Morphologie / progression", "Passer volontairement d'une morphologie Standard à Robuste.", false, "Trophée de transformation robuste.", false),
            title("Plus léger qu'avant", "Morphologie / progression", "Passer volontairement de Robuste à Standard.", false, "Trophée de transformation vers une carrure plus légère.", false),
            title("Affûté", "Morphologie / progression", "Passer volontairement de Standard à Fine.", false, "Trophée de transformation fine.", false),
            title("Silhouette travaillée", "Morphologie / progression", "Atteindre volontairement une morphologie Fine après un entraînement et une alimentation suivis.", false, "Trophée de transformation longue et volontaire.", false),
            title("Plume au vent", "Morphologie / mobilité", "Conserver une morphologie Fine et démontrer durablement une excellente mobilité.", false, "Trophée de mobilité légère.", false),
            title("Affûté comme une lame", "Morphologie / mobilité", "Devenir volontairement Fin puis atteindre un haut niveau de mobilité en situation réelle.", false, "Trophée de mobilité travaillée.", false),
            title("Ça flotte presque", "Morphologie / inventaire", "Être naturellement de petit gabarit, de morphologie Fine et voyager sous 25 % de charge.", false, "Trophée de légèreté extrême.", false),
            title("Bête de somme", "Morphologie / inventaire", "Combiner une morphologie Robuste avec le plus grand sac disponible.", false, "Trophée de capacité de portage.", false),
            title("Tout sur le dos", "Morphologie / inventaire", "Être Robuste et porter plus de 150 % de capacité sans atteindre 200 %.", false, "Trophée de portage extrême.", false),
            title("Déménageur", "Inventaire / équipement", "Transporter une très grosse quantité d'équipement lourd en une seule sortie sans atteindre 200 %.", false, "Trophée de logistique lourde.", false),
            title("L'art du minimum", "Morphologie / exploration", "Avec une morphologie Fine, enchaîner des sorties avec une charge très faible.", false, "Trophée de minimalisme durable.", false),
            title("Il m'a rendu service ?", "Drôle / rivaux", "Se faire voler par un fuyard alors qu'on était déjà extrêmement surchargé.", false, "Trophée ironique : le vol a techniquement allégé le sac.", false),

            title("Ma parole me lie", "Serments", "Prononcer son premier vrai serment.", true),
            title("Parole tenue", "Serments", "Respecter longtemps un serment alors que le rompre aurait été avantageux.", false),
            title("Parjure", "Serments", "Rompre volontairement un serment.", false, "Trace sociale négative : certains personnages pourront réagir à un serment volontairement brisé.", false),
            title("Deux serments, un choix", "Serments", "Être réellement forcé de sacrifier un serment pour en respecter un autre.", false, "Titre de dilemme : volontairement non requis pour la complétion car les serments peuvent être exclusifs.", false),

            title("Enfant du pays", "Ville / réputation", "Atteindre une réputation locale exceptionnelle dans une ville.", true),
            title("Persona non grata", "Ville / réputation", "Subir un vrai refus de service à cause d'une réputation locale très mauvaise.", false, "Trace négative de réputation : non requise pour la complétion.", false),
            title("Racheter ses erreurs", "Ville / réputation", "Revenir d'une réputation très négative par des réparations légitimes.", false),
            title("Deux villes, deux visages", "Ville / réputation", "Être très apprécié dans une ville et franchement mal vu dans une autre.", false),
            title("Tout le monde me connaît", "Monde / réputation", "Atteindre une très haute réputation générale ou multi-locale.", false),
            title("Pas partout en bien", "Monde / réputation", "Être célèbre avec des réputations très différentes selon les groupes et les villes.", false),
            title("Une histoire à raconter", "Monde / mémoire", "Accumuler de nombreux événements historiques uniques sur le même personnage.", true),
            title("On parle de toi", "Monde / rumeurs", "Devenir le sujet de plusieurs rumeurs indépendantes.", false),
            title("Je connais quelqu'un", "Monde / relations", "Disposer de contacts crédibles dans plusieurs organisations.", false),
            title("Aucun camp", "Monde / factions", "Rester indépendant malgré plusieurs propositions sérieuses de recrutement.", false, "Titre de parcours alternatif : non requis pour la complétion.", false),
            title("Plusieurs vies", "Monde / progression", "Traverser de nombreux systèmes majeurs sur le même personnage.", false),

            title("Le prix de la puissance", "Magie / matériaux", "Sacrifier volontairement un matériau exceptionnel dans une vraie canalisation.", false),
            title("À la bonne qualité", "Magie / matériaux", "Réussir une recette avancée avec des qualités de matériaux choisies précisément.", true),
            title("Ça devait marcher...", "Magie / drôle", "Perdre légitimement un support lors d'un échec de stabilité.", false),
            title("Le bâton est mort, pas moi", "Magie / catalyseurs", "Réussir une incantation de secours avec un catalyseur brisé.", false),
            title("Sans catalyseur", "Magie / maîtrise", "Réussir plusieurs incantations pures difficiles sans catalyseur.", false),
            title("Je l'ai enfin compris", "Magie / grimoires", "Étudier jusqu'au bout un grimoire exigeant langue, niveau et temps.", false),
            title("Trois voies, une seule magie", "Magie / maîtrise", "Expérimenter légitimement les trois approches magiques accessibles au personnage.", false),

            title("Sous contrat", "Emploi / équipes", "Signer son premier contrat avec une équipe externe.", true),
            title("Employé modèle", "Emploi / équipes", "Réussir plusieurs missions employeur sans avertissement disciplinaire.", true),
            title("Mise à pied", "Emploi / équipes", "Être réellement placé sur le banc pour raisons disciplinaires.", false, "Trace disciplinaire négative : non requise pour la complétion.", false),
            title("Vous êtes viré.", "Emploi / drôle", "Se faire réellement licencier d'une organisation.", false, "Titre négatif/humoristique : non requis pour la complétion.", false),
            title("Sans rancune", "Emploi / équipes", "Terminer proprement plusieurs contrats auprès d'organisations différentes.", false),

            title("La marque n'a pas suffi", "FireFlight", "Survivre jusqu'au bout à une marque mortelle de FireFlight.", false),
            title("Toujours debout", "FireFlight", "Survivre à une phase où FireFlight impose réellement des règles mortelles.", false),
            title("Tout le monde respire", "FireFlight", "Faire traverser à tout le groupe une marque collective sans mort définitive.", false),

            title("Canal interdit", "REAPERS-13", "Utiliser légitimement le Protocol 13 après avoir obtenu une connaissance crédible des Reapers.", false, "Titre secret lié à un canal classifié. Son existence reste cachée par défaut.", false),
            title("Signal reçu", "REAPERS-13", "Recevoir une vraie réponse à une balise Protocol 13.", false, "Titre secret Reaper : non requis pour la complétion générale.", false),
            title("Sous observation", "REAPERS-13", "Attirer officiellement l'attention des Reapers sans être encore recruté.", false, "Titre secret Reaper : non requis pour la complétion générale.", false),
            title("Ils t'ont choisi", "REAPERS-13", "Être sélectionné officiellement par les Reapers.", false, "Titre secret Reaper : non requis pour la complétion générale.", false),
            title("La treizième ligne", "REAPERS-13", "Intégrer officiellement la 13th-Line.", false, "Grade/faction Reaper : non requis pour la complétion générale.", false),
            title("Echo dans l'ombre", "REAPERS-13", "Être accepté dans l'Echo Unit.", false, "Grade/faction Reaper : non requis pour la complétion générale.", false),
            title("Operational-Reaper", "REAPERS-13", "Obtenir officiellement le grade Operational Reaper.", false, "Grade officiel Reaper, jamais accordé par simple puissance brute.", false),
            title("Reaper-Warden", "REAPERS-13", "Obtenir officiellement le grade Reapers Warden.", false, "Grade officiel extrêmement élevé, non requis pour la complétion générale.", false),
            title("Black Flag", "REAPERS-13", "Intégrer l'unité principale du Reaper-13 si elle conserve ce nom officiel.", false, "Titre d'unité classifié et non requis pour la complétion générale.", false),
            title("La fuite est terminée", "REAPERS-13", "Capturer une cible majeure après une longue traque Reaper.", false),
            title("Pas pour la gloire", "REAPERS-13", "Terminer plusieurs opérations Reaper en privilégiant l'objectif à la récompense personnelle.", false),
            title("Observer avant d'agir", "REAPERS-13", "Réussir une mission dangereuse après une vraie phase de renseignement.", false),
            title("Aucune erreur", "REAPERS-13", "Terminer une opération Reaper difficile sans perte, fuite de cible ni victime collatérale.", false),
            title("Cible prioritaire", "REAPERS-13 / secret", "Devenir une cible prioritaire du clan après un acte exceptionnellement hostile envers sa hiérarchie.", false, "Titre extrêmement secret ; il ne remplace jamais le grade Reaper-13.", false),

            title("Luck is a skill", "Chance / secret", "Prouver légitimement, sur plusieurs situations réellement improbables, que la chance n'était peut-être pas qu'un accident.", false),
            title("Je savais que ça marcherait", "Chance / secret", "Réussir une action significative avec une probabilité réellement très faible.", false),
            title("C'était prévu.", "Chance / secret", "Accumuler plusieurs événements favorables très improbables au cours d'une même aventure.", false),
            title("Pourquoi ça marche ?", "Système / secret", "Déclencher légitimement une combinaison très improbable de plusieurs systèmes.", false),
            title("Rien à signaler", "Système / intégrité", "Atteindre une très longue progression sans jamais altérer le personnage.", false),
            title("Pur jusqu'au bout", "Système / intégrité", "Atteindre une étape majeure de fin de progression sans aucune triche altérante.", false),

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

    std::string catalogVersion()
    {
        return "V3.57.02";
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
        std::vector<std::string> validatedAlternativeGroups;
        for (const TitleDefinition& definition : availableTitleDefinitions())
        {
            if (!definition.requiredForUltimate)
            {
                continue;
            }

            if (definition.ultimateAlternativeGroup.empty())
            {
                if (!containsTitle(ownedTitles, definition.name)) return false;
                continue;
            }

            if (std::find(validatedAlternativeGroups.begin(), validatedAlternativeGroups.end(), definition.ultimateAlternativeGroup)
                != validatedAlternativeGroups.end())
            {
                continue;
            }

            bool oneAlternativeOwned = false;
            for (const TitleDefinition& alternative : availableTitleDefinitions())
            {
                if (alternative.requiredForUltimate
                    && alternative.ultimateAlternativeGroup == definition.ultimateAlternativeGroup
                    && containsTitle(ownedTitles, alternative.name))
                {
                    oneAlternativeOwned = true;
                    break;
                }
            }
            if (!oneAlternativeOwned) return false;
            validatedAlternativeGroups.push_back(definition.ultimateAlternativeGroup);
        }
        return true;
    }
}
