// EN: BestiaryMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable bestiary menu with prepared categories and detailed entries.
// Français : Affiche le premier bestiaire utilisable avec catégories préparées et fiches détaillées.

#include "interface/menu/progression/BestiaryMenu.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/material/MaterialKnowledgeProgress.hpp"
#include "progression/material/MaterialExperimentLog.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    struct BestiaryPreviewEntry
    {
        std::string category;
        std::string name;
        std::string simpleDescription;
        std::string detailedDescription;
        std::string status;
        int encounters;
        int kills;
    };

    struct KnowledgeHints
    {
        std::string dangerRank;
        std::string habitat;
        std::string weaknesses;
        std::string resistances;
        std::string drops;
        std::string strategy;
    };

    std::string lowerCopy(const std::string& value)
    {
        std::string output = value;
        std::transform(output.begin(), output.end(), output.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return output;
    }

    bool containsText(const std::string& haystack, const std::string& needle)
    {
        return lowerCopy(haystack).find(lowerCopy(needle)) != std::string::npos;
    }

    KnowledgeHints buildKnowledgeHints(
        const std::string& category,
        const std::string& name,
        const std::string& description
    )
    {
        KnowledgeHints hints;
        hints.dangerRank = "Variable";
        hints.habitat = "Habitat encore mal confirmé.";
        hints.weaknesses = "Faiblesses non confirmées.";
        hints.resistances = "Résistances non confirmées.";
        hints.drops = "Butin à confirmer par observation, récolte ou achat d'information.";
        hints.strategy = "Observer le comportement avant de conclure.";

        const std::string probe = lowerCopy(category + " " + name + " " + description);

        if (containsText(probe, "slime") || containsText(probe, "gel"))
        {
            hints.dangerRank = "Faible à instable selon couleur";
            hints.habitat = "Mares gélatineuses, grottes humides, ruines chargées de résidus.";
            hints.weaknesses = "Impact net, froid ou feu selon couleur. Les slimes dorés et chromatiques demandent observation.";
            hints.resistances = "Saignement très faible ; poison souvent inutile sur gelées simples.";
            hints.drops = "Résidus de slime, gel coloré, parfois noyau instable ou matière brillante avalée.";
            hints.strategy = "Identifier la couleur avant d'utiliser une ressource rare.";
        }
        else if (containsText(probe, "gobelin") || containsText(probe, "hobgobelin"))
        {
            hints.dangerRank = "Faible seul, dangereux en bande";
            hints.habitat = "Routes, caches, campements de fortune, grottes proches des villes.";
            hints.weaknesses = "Pression rapide, dégâts de zone, neutralisation du shaman ou du chef.";
            hints.resistances = "Peu de résistance naturelle ; l'équipement change tout.";
            hints.drops = "Oreilles, insignes, pièces volées, cuir ou métal médiocre.";
            hints.strategy = "Ne pas laisser le soutien installer la bande.";
        }
        else if (containsText(probe, "loup") || containsText(probe, "bête") || containsText(probe, "sanglier") || containsText(probe, "renard"))
        {
            hints.dangerRank = "Faible à moyen";
            hints.habitat = "Plaines, forêts, routes isolées et lisières.";
            hints.weaknesses = "Entrave, feu maîtrisé, contre après charge.";
            hints.resistances = "Peu de résistance magique ; bon instinct contre les pièges évidents.";
            hints.drops = "Peau, crocs, fourrure, viande, cuir robuste selon propreté.";
            hints.strategy = "Rester groupé et éviter de finir blessé face à une meute.";
        }
        else if (containsText(probe, "squelette") || containsText(probe, "mort") || containsText(probe, "osseux") || containsText(probe, "ombre") || containsText(probe, "cimetière"))
        {
            hints.dangerRank = "Moyen, parfois trompeur";
            hints.habitat = "Cimetières, ruines, lieux froids ou chargés de mana mort.";
            hints.weaknesses = "Lumière, feu, coups lourds contre les os, purification.";
            hints.resistances = "Poison et saignement souvent très faibles.";
            hints.drops = "Os fissurés, fil d'ombre, poussière funéraire, fragments de sceau.";
            hints.strategy = "Chercher rupture ou purification plutôt que poison.";
        }
        else if (containsText(probe, "plante") || containsText(probe, "racine") || containsText(probe, "ronce"))
        {
            hints.dangerRank = "Moyen si terrain serré";
            hints.habitat = "Forêts anciennes, marais, ruines recouvertes.";
            hints.weaknesses = "Feu, coupe nette, gel sur fibres humides.";
            hints.resistances = "Poison peu utile ; entraves difficiles à retourner.";
            hints.drops = "Fibres, sève, graines, ronces occultes.";
            hints.strategy = "Libérer le mouvement avant de chercher le gros dégât.";
        }
        else if (containsText(probe, "bandit") || containsText(probe, "alchimiste") || containsText(probe, "mage renégat") || containsText(probe, "humain hostile") || containsText(probe, "chasseur de primes"))
        {
            hints.dangerRank = "Variable selon équipement";
            hints.habitat = "Routes commerciales, ruelles, camps improvisés et contrats douteux.";
            hints.weaknesses = "Désarmement, pression rapide sur les soutiens, interruption des fioles ou incantations.";
            hints.resistances = "Pas de résistance naturelle stable ; armure, potions et préparation font la différence.";
            hints.drops = "Insignes abîmés, cuir, fioles, poussière arcanique ou pièces volées.";
            hints.strategy = "Identifier le rôle : apothicaire, mage, chef ou tireur ne se gèrent pas dans le même ordre.";
        }
        else if (containsText(probe, "orc") || containsText(probe, "berserker") || containsText(probe, "colosse"))
        {
            hints.dangerRank = "Moyen à lourd";
            hints.habitat = "Campements robustes, zones de chasse, arènes et passages disputés.";
            hints.weaknesses = "Entrave, allonge, usure contrôlée et attaques qui ignorent partiellement l'armure.";
            hints.resistances = "Bonne résistance physique brute ; difficile à faire fuir au contact.";
            hints.drops = "Cuir épais, métal cabossé, trophées de guerre et matériaux de réparation.";
            hints.strategy = "Ne pas échanger coup pour coup sans garde : le rythme lent cache souvent un gros impact.";
        }
        else if (containsText(probe, "araignée") || containsText(probe, "insect") || containsText(probe, "venimeuse"))
        {
            hints.dangerRank = "Faible à dangereux si poison accumulé";
            hints.habitat = "Grottes, forêts humides, marais et coins non entretenus.";
            hints.weaknesses = "Feu contrôlé, attaques de zone, purification du poison.";
            hints.resistances = "Bon instinct d'esquive ; poison parfois moins efficace sur variantes déjà toxiques.";
            hints.drops = "Mandibules, venin, soie, chitine légère.";
            hints.strategy = "Ne pas attendre que le poison fasse boule de neige : traiter tôt ou finir vite.";
        }
        else if (containsText(probe, "dragon") || containsText(probe, "draconide") || containsText(probe, "kobold"))
        {
            hints.dangerRank = "Élevé quand l'âge augmente";
            hints.habitat = "Nids rocheux, montagnes, anciennes ruines et territoires gardés.";
            hints.weaknesses = "Faille de résistance, contrôle de souffle, éléments opposés selon lignée.";
            hints.resistances = "Écailles, souffle, fierté et résistance naturelle supérieure à la moyenne.";
            hints.drops = "Écailles, braises, griffes, fragments draconiques.";
            hints.strategy = "Préparer le combat : une victoire brutale peut abîmer les matériaux les plus précieux.";
        }
        else if (containsText(probe, "construction") || containsText(probe, "armure") || containsText(probe, "sentinelle") || containsText(probe, "gardien ancien"))
        {
            hints.dangerRank = "Lourd et défensif";
            hints.habitat = "Ruines, portes scellées, salles de garde et anciens ateliers.";
            hints.weaknesses = "Choc ciblé, marteau, fissures, corrosion ou brise-garde.";
            hints.resistances = "Poison et saignement quasi inutiles ; très bonne résistance aux petits coups.";
            hints.drops = "Fragments de métal, plaques, noyaux, anciennes pièces de mécanisme.";
            hints.strategy = "Chercher les fissures plutôt que de multiplier les entailles faibles.";
        }
        else if (containsText(probe, "élémentaire") || containsText(probe, "méphaïte") || containsText(probe, "braise") || containsText(probe, "anomalie"))
        {
            hints.dangerRank = "Instable";
            hints.habitat = "Failles magiques, ruines chargées, zones extrêmes ou expériences ratées.";
            hints.weaknesses = "Élément opposé, stabilisation, rupture de noyau ou voile élémentaire bien placé.";
            hints.resistances = "Résiste souvent à son propre élément ; certaines anomalies changent de réaction.";
            hints.drops = "Poussière arcanique, noyau instable, éclat élémentaire.";
            hints.strategy = "Lire la première réaction avant d'insister : l'instabilité punit les habitudes.";
        }
        else if (category == "Habitats / zones")
        {
            hints.dangerRank = "Zone";
            hints.habitat = "Lieu d'exploration avec familles dominantes, ressources et variantes rares.";
            hints.weaknesses = "La préparation dépend du biome : antidotes, anti-givre, arme lourde, feu ou voile élémentaire.";
            hints.resistances = "Une zone ne résiste pas, mais elle favorise certaines rencontres et pièges.";
            hints.drops = "Ressources locales, coffres, traces, contrats et matériaux liés aux familles présentes.";
            hints.strategy = "Lire la zone avant d'y entrer : les bons consommables valent parfois plus qu'un niveau.";
        }
        else if (containsText(probe, "brûlure") || containsText(probe, "poison") || containsText(probe, "givre") || containsText(probe, "choc") || containsText(probe, "saignement") || containsText(probe, "affaiblissement") || containsText(probe, "faille") || containsText(probe, "voile") || containsText(probe, "suture"))
        {
            hints.dangerRank = "Effet persistant";
            hints.habitat = "Sorts, munitions, potions, monstres élémentaires ou boss spécialisés.";
            hints.weaknesses = "Purification, potion adaptée, voile élémentaire ou fin naturelle de durée.";
            hints.resistances = "Race, classe, armure, matériau et catalyseur peuvent réduire ou amplifier l'effet.";
            hints.drops = "Pas un loot direct, mais une connaissance utile pour préparer les consommables.";
            hints.strategy = "Lire l'icône d'état : ignorer un effet faible peut coûter moins cher que gaspiller une potion.";
        }
        else if (category == "Boss")
        {
            hints.dangerRank = "Boss";
            hints.habitat = "Arène, territoire scellé ou lieu lié à sa légende.";
            hints.weaknesses = "Failles de phase à découvrir par dialogue, registre ou observation.";
            hints.resistances = "Résistances élevées ; changement de règles possible sous 50% PV.";
            hints.drops = "Fragment, invitation, matériau unique ou trace de boss.";
            hints.strategy = "Préparer potions, munitions et lecture de puissance avant d'entrer.";
        }
        else if (category == "Matériaux et plantes")
        {
            hints.dangerRank = "Ressource";
            hints.habitat = "Zone, monstre ou qualité de récolte associé.";
            hints.weaknesses = "Peut être brûlé, troué, écrasé ou souillé par une mauvaise méthode.";
            hints.resistances = "La haute qualité conserve mieux son intérêt en craft.";
            hints.drops = "Quantité et qualité dépendent du combat, de la récolte et parfois de la chance.";
            hints.strategy = "Choisir entre vente rapide et conservation pour recette rare.";
        }
        else if (category == "Objets rares")
        {
            hints.dangerRank = "Objet";
            hints.habitat = "Bibliothèque, marché noir, coffre, boss, troc ou récompense spéciale.";
            hints.weaknesses = "Usage limité, coût élevé, instabilité ou prérequis.";
            hints.resistances = "Garde sa valeur si utilisé dans le bon contexte.";
            hints.drops = "Obtention par achat, troc, exploration ou secret.";
            hints.strategy = "Vérifier si l'objet est jetable, apprenable, réparable ou unique.";
        }

        return hints;
    }

    // EN: getEntries declares or implements a focused behavior used by this module.
    // FR: getEntries déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<BestiaryPreviewEntry>& getEntries()
    {
        static const std::vector<BestiaryPreviewEntry> entries = {
            {"Races", "Humain", "Race commune, adaptable et ambitieuse.", "Les humains sont présents dans beaucoup de villes. Ils servent de base pour les aventuriers, bandits, chevaliers et mages.", "Connu", 0, 0},
            {"Races", "Elfe noir", "Race discrète, souvent liée aux ombres.", "Les elfes noirs peuvent être craints selon les régions. Hazak représente une version très dangereuse de cette race.", "Partiellement connu", 0, 0},
            {"Races", "Kitsune", "Race semi-humaine liée aux illusions et aux flammes.", "Les kitsunes gardent souvent un lien naturel avec la magie, la ruse et les flammes. Aoi en est un exemple spécial.", "Partiellement connu", 0, 0},
            {"Habitats / zones", "Plaine sauvage", "Biome ouvert pour les premiers pas.", "Plaines, hautes herbes, petits prédateurs et gobelins faibles. Les coffres visibles peuvent exister, mais les traces rares restent liées au risque et au niveau.", "Carte commune", 0, 0},
            {"Habitats / zones", "Route commerciale", "Passage de voyageurs, marchands et bandits.", "Zone idéale pour rencontrer humanoïdes, pilleurs, gobelins organisés, clients de guilde et objets volés. Les soutiens crédibles y apparaissent plus naturellement.", "Carte commune", 0, 0},
            {"Habitats / zones", "Mares gélatineuses", "Biome des slimes et réactions colorées.", "Les mares concentrent les variantes de slimes : vert, bleu, rouge, jaune, violet, ambré, doré, chromatique et formes anciennes. Le bon élément dépend toujours de la couleur.", "Carte commune", 0, 0},
            {"Habitats / zones", "Forêt ancienne", "Biome humide, végétal et plus sérieux.", "Racines, bêtes couvertes de mousse, plantes hostiles et esprits sylvestres. Les attaques trop brutales peuvent abîmer des composants végétaux précieux.", "Carte étudiée", 0, 0},
            {"Habitats / zones", "Montagne froide", "Biome de falaises et de froid.", "La montagne favorise le givre, les draconides froids, les bêtes lourdes et les constructions rocheuses. Anti-givre, voile élémentaire et armes solides y prennent plus de valeur.", "Carte étudiée", 0, 0},
            {"Habitats / zones", "Marais trouble", "Biome toxique et lent.", "Le marais attire slimes putrides, noyés, chamans de vase et mages sales. Antidotes, purification et gestion de la durée y sont essentiels.", "Carte dangereuse", 0, 0},
            {"Habitats / zones", "Cimetière oublié", "Biome des morts et des noms perdus.", "Morts-vivants, esprits, goules, oracles de tombe et ombres rares. Poison et saignement perdent souvent de la valeur ; lumière, feu et coups lourds en gagnent.", "Carte dangereuse", 0, 0},
            {"Habitats / zones", "Ruines effondrées", "Biome ancien, cassé et arcanique.", "Spectres, armures mortes, anomalies, sentinelles brisées et squelettes runiques. Les constructions y demandent des réponses différentes des chairs vivantes.", "Carte dangereuse", 0, 0},
            {"Entités hostiles / ennemis", "Gobelin peureux", "Petit ennemi rapide qui préfère parfois survivre.", "Un gobelin basique. Pas très noble, mais capable de gratter des dégâts et de fuir si le combat devient humiliant.", "Déjà rencontré dans les arènes libres", 1, 0},
            {"Entités hostiles / ennemis", "Famille gobeline", "Petits humanoïdes opportunistes avec plusieurs rôles.", "Les gobelins faibles servent de chair à canon, les frondeurs harcèlent, les gardes tiennent plus longtemps, les shamans peuvent soigner leurs alliés, et les hobgobelins organisent mieux les bandes. Un gobelin normal ne soigne pas par miracle : seuls les chamans et profils préparés savent maintenir une bande debout.", "Renseignement évolutif", 0, 0},
            {"Entités hostiles / ennemis", "Famille slime", "Gelées célèbres aux couleurs très lisibles.", "Vert = classique/régénération légère, bleu/blanc = froid, rouge = chaleur/brûlure, violet/noir = poison, jaune = choc, ambré = colle et gêne les mouvements, rose = rebond imprévisible, doré = attiré par les choses brillantes, chromatique/prisme/miroir = réaction instable. Les mares gélatineuses concentrent presque toutes les couleurs, mais certaines couleurs apparaissent aussi dans les lieux qui les attirent.", "Renseignement évolutif", 0, 0},
            {"Entités hostiles / ennemis", "Slime rouge", "Gelée chaude qui irrite la peau.", "Slime attiré par la chaleur : risque de brûlure légère, résiste mieux aux flammes faibles et supporte mal le froid propre.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime bleu", "Gelée froide et lente.", "Slime lié au froid : gêne les mouvements, peut installer du givre et craint souvent la chaleur soutenue.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime violet", "Gelée toxique des lieux humides.", "Slime de poison : très mauvais candidat pour les poisons classiques. Les antidotes et la distance restent précieux.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime jaune", "Gelée chargée d'étincelles.", "Le choc devient dangereux contre l'équipement métallique. Éviter d'empiler arme métal et armure métal contre lui si possible.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime ambré", "Gelée collante qui ralentit.", "Moins létal qu'un slime élémentaire pur, mais très pénible : il ouvre des fenêtres pour ses alliés.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime doré", "Gelée attirée par les objets brillants.", "Il peut justifier une trouvaille étrange, mais ne doit pas être vu comme une source fiable d'or. Sa valeur vient surtout du risque et de la rareté.", "Rare", 0, 0},
            {"Entités hostiles / ennemis", "Slime chromatique", "Gelée instable aux réactions difficiles.", "Il change de réaction selon l'impact reçu. L'observer vaut souvent mieux que brûler toutes ses ressources au premier tour.", "Rare instable", 0, 0},
            {"Entités hostiles / ennemis", "Monstres soigneurs crédibles", "Tous les ennemis ne savent pas boire ou lancer une potion.", "Un loup, un slime simple ou une bête sauvage ne comprend pas l'alchimie. Les soins ennemis doivent venir d'un shaman, chamane, apothicaire, oracle, mage ou monstre spécial avec une raison lore. Les IA alliées peuvent aussi soigner joueur ou allié si leur personnalité et leur inventaire s'y prêtent.", "Règle de cohérence", 0, 0},
            {"Entités hostiles / ennemis", "Loup affamé", "Prédateur opportuniste.", "Le loup teste les proies faibles. Son instinct le pousse surtout vers les proies faibles, isolées ou déjà blessées.", "Informations communes", 0, 0},
            {"Entités hostiles / ennemis", "Racine étrangleuse", "Plante qui gagne quand la cible cesse de bouger.", "Elle ne cherche pas seulement à blesser : elle ferme le terrain, use les tours et prépare les autres plantes.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Squelette archer rouillé", "Mort-vivant à distance, fragile mais pénible.", "Ses os supportent mal les coups lourds, mais le poison et le saignement ne changent presque rien. Le métal rouillé peut cacher de petits composants.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Oracle fissuré", "Support étrange qui lit mal l'avenir.", "Il peut soutenir, gêner ou annoncer des dangers. À éliminer tôt si le combat devient long.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Bandit apothicaire", "Humanoïde capable de soins crédibles.", "Contrairement à un loup ou un slime, il sait utiliser fioles et remèdes. Le focus soutien devient logique.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Chauve-souris des cavernes", "Créature rapide et fragile.", "Elle ne gagne pas par force brute : elle gêne, esquive et profite des endroits sombres. Les attaques précises ou de zone courte l'empêchent de tourner longtemps.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Rat géant", "Bête nuisible de bas niveau.", "Le rat géant sert de première menace sale : faible seul, plus dangereux si le joueur arrive déjà blessé ou empoisonné.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Kobold paniqué", "Petit draconide nerveux.", "Il n'a pas la puissance d'un vrai dragon, mais garde une logique de draconide : méfiant, territorial et parfois plus dangereux près d'un nid.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Araignée venimeuse", "Insectoïde toxique.", "Son poison force à choisir entre finir vite ou consommer une réponse. Les variantes toxiques résistent mieux aux poisons banals.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Sanglier sauvage", "Bête qui encaisse et charge.", "Il punit les joueurs trop confiants : esquive ou entrave avant impact, puis contre proprement après la charge.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Ours brun", "Bête massive et directe.", "L'ours ne demande pas une énigme : il demande de respecter sa force. Garde, distance, entrave et soins préparés valent mieux qu'un duel d'orgueil.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Squelette fissuré", "Mort-vivant fragile mais insensible aux mauvais outils.", "Ses os cassent sous les chocs lourds. Poison et saignement gaspillent souvent un tour.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Goule affamée", "Mort-vivant agressif.", "Plus organique qu'un squelette, mais toujours lié à la mort. Le feu et la purification restent de meilleures pistes que les poisons ordinaires.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Alchimiste renégat", "Humain dangereux avec fioles.", "Il peut faire plus qu'attaquer : affaiblir, empoisonner, brûler ou se sauver avec un remède. L'interrompre évite beaucoup de dégâts indirects.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Mage renégat", "Humain arcanique instable.", "Il dépend de sa canalisation et de son catalyseur. Lui laisser plusieurs tours revient souvent à accepter une altération ou une faille.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Orc mineur", "Combattant lourd encore lisible.", "Son niveau reste bas, mais son corps encaisse mieux qu'un gobelin. L'allonge et l'entrave évitent de subir son rythme.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Orc berserker", "Briseur sauvage dangereux blessé.", "Plus il prend de coups, plus l'échange devient risqué. Une faille ouverte doit être exploitée vite, pas entretenue par orgueil.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Chevalier corrompu", "Humain en armure devenu menace.", "L'armure réduit les petits coups, mais le choc, le marteau, la faille ou le brise-garde ouvrent de vraies fenêtres.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Méphaïte de braise", "Petit élémentaire de feu.", "Il supporte mal qu'on le traite comme une bête normale : feu contre feu déçoit, froid propre et voile élémentaire aident davantage.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Élémentaire instable", "Énergie condensée difficile à lire.", "Sa réaction peut changer selon l'élément et l'impact. Observer le premier échange évite de nourrir sa résistance.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Anomalie arcanique instable", "Erreur magique vivante.", "Elle n'obéit pas toujours aux catégories simples. La stabilité, le voile élémentaire et les attaques mesurées comptent plus que les automatismes.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Armure morte", "Construction hantée ou mort-vivant blindé.", "Poison et saignement sont presque inutiles. Cherche fissure, choc, marteau, purification ou rupture du noyau hanté.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Dragon mineur", "Jeune menace draconique.", "Même jeune, un dragon impose préparation : élément, souffle, écailles et butin fragile. Le battre salement peut gâcher ce qu'il laisse.", "Observation de terrain", 0, 0},
            {"Entités passives / alliées", "Aventurier errant", "Humain ou semi-humain généré par l'arène.", "Un adversaire sans légende connue, avec son propre nom, sa race et sa classe. Selon les rencontres, il peut devenir rival ou visage familier.", "Fiche archivée", 0, 0},
            {"Invocations", "Invocation mineure", "Renfort temporaire appelé par certaines classes.", "Les invocations suivent les mêmes règles globales de ciblage que les monstres, mais peuvent appartenir au joueur ou à l'ennemi.", "Fiche archivée", 0, 0},
            {"Invocations", "Esprit mineur", "Petit esprit stable et facile à maintenir.", "Invocation simple : faible coût de slot, maintien léger, attaque régulière et petite régénération. Elle sert de base aux invocateurs.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Serviteur osseux", "Invocation nécromantique solide.", "Le serviteur osseux peut sacrifier une partie de son ossature pour frapper plus fort. Son sacrifice final inflige une rupture dangereuse.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Flamme kitsune", "Flamme liée aux kitsunes.", "La Flamme kitsune brûle vite, frappe fort et reste fragile. Elle correspond particulièrement à Aoi et aux voies de feu liées aux pactes.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Bête arcanique", "Invocation lourde consommant plus de slots.", "La Bête arcanique occupe deux slots, encaisse mieux et se nourrit du mana ambiant. Elle transforme le duel en affrontement de groupe.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Expérience instable", "Créature dangereuse de laboratoire.", "L'Expérience instable tape fort, dure peu et peut exploser. Elle correspond aux expériences de Fail et aux risques de laboratoire.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Ombre récente", "Ombre née d'un lien violent.", "L'Ombre récente suit les traces de Hazak. Certaines ombres ne naissent qu'après un acte violent ou un lien trop sombre.", "Secret partiel", 0, 0},
            {"Invocations", "Éclat zodiacal", "Fragment magique lié aux signes.", "L'Éclat zodiacal tire un signe sur treize. Le treizième signe peut prolonger son existence et rappelle les anciens sorts zodiacaux liés à Kanadé.", "Fiche d'invocation", 0, 0},
            {"Boss", "???", "Nom inconnu.", "Un boss ne révèle son vrai nom que s'il le prononce, à l'entrée, à 50%, avant sa défaite, ou via des écrits crédibles trouvés avant.", "Identité verrouillée", 0, 0},
            // EN: "Matt declares or implements a focused behavior used by this module.
            // FR: "Matt déclare ou implémente un comportement précis utilisé par ce module.
            {"Personnages spéciaux", "Matt (PRO)", "Combattant silencieux et respectueux.", "Référence directe à Matt de Wii Sports. Il ne parle pas vraiment : il combat proprement, avec respect, et des bonus globaux légers.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Hazak", "Assassin elfe noir, sombre et efficace.", "Hazak prend les gens de haut, cherche la victoire et protège Hestia d'une violence trop traumatisante. Meilleur ami d'Henrique.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Sanctus / Skuro", "Protecteur pouvant vriller en exécuteur sombre.", "Sanctus protège, entrave et s'appuie sur un dieu de lumière. S'il prend trop cher ou inflige trop cher, Skuro peut ressortir.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Aoi", "Kitsune timide, mage de flammes et invocatrice de flammes.", "Aoi protège ses incantations et devient très dangereuse si on lui laisse le temps de préparer sa magie.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Kanadé", "Semi-dragonne rageuse aux sorts zodiacaux.", "Kanadé râle souvent, mais elle avance quand même. Ses sorts les plus étranges tournent autour des treize signes du zodiaque.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Hestia", "Humaine peureuse aux origines divines oubliées.", "Hestia était une divinité avant de perdre ses souvenirs. Elle préfère éviter le combat, mais possède une magie de protection anormalement haute.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Henrique", "Chevalier fonceur capable de revenir une fois.", "Henrique est le meilleur ami de Hazak. Il fonce dans le tas et possède une étrange capacité de retour à la vie.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Louis", "Artificier naïf qui cherche des amis.", "Louis vise bien, utilise plusieurs projectiles et reste fondamentalement sympathique malgré le chaos de l'arène.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Trexof", "Assassin humain et bêta-testeur principal.", "Trexof possède de légers bonus et fait partie du groupe proche de Mattzelda et Louis.", "Spécial", 0, 0},
            {"Matériaux et plantes", "Fleur bleue de montagne", "Plante calme et rare.", "Clin d'œil à la fleur bleue de Zelda BOTW. Elle sert aux remèdes, quêtes et secrets liés aux plantes.", "Renseignement de base", 0, 0},
            {"Matériaux et plantes", "Oreille de gobelin", "Composant commun de monstre.", "Matériau simple récupérable sur certains gobelins. Utile pour les premières ventes, les contrats et les crafts simples.", "Loot possible", 0, 0},
            {"Matériaux et plantes", "Peau de bête robuste", "Matériau de réparation épaisse.", "Sert déjà à fabriquer un kit moyen via rafistolage renforcé. Aussi recherchée pour les armures, les tanks et l'équipement de survie.", "Loot rare", 0, 0},
            {"Matériaux et plantes", "Fil d'ombre", "Composant sombre rare.", "Lié aux assassins, aux morts-vivants et aux ombres de Hazak. Les nécromanciens évitent d'en parler devant les guildes.", "Secret partiel", 0, 0},
            {"Matériaux et plantes", "Braise kitsune", "Braise magique instable.", "Composant lié à Aoi, aux flammes kitsune, à certaines potions avancées et aux invocations.", "Rare", 0, 0},
            {"Matériaux et plantes", "Fragment d'écaille draconique", "Fragment défensif rare.", "Matériau très solide, recherché pour protections rares, armures lourdes et crafts semi-dragons.", "Rare", 0, 0},
            {"Matériaux et plantes", "Noyau instable", "Cœur magique dangereux.", "Sert de base aux expériences de Fail, à l'alchimie risquée, aux objets explosifs et aux invocations instables.", "Dangereux", 0, 0},
            {"Matériaux et plantes", "Qualités de matériaux", "Les qualités modifient valeur et efficacité.", "Impur/faible qualité compte moins, normal compte normalement, pur/haute qualité compte plus et exceptionnel peut déclencher de petites particularités de craft.", "Renseignement commun", 0, 0},
            {"Effets et altérations", "Brûlure", "Dégât persistant de feu.", "La brûlure grignote les PV, menace les plantes et peut abîmer certains composants. Baume, voile élémentaire ou résistance au feu la rendent plus supportable.", "Effet connu", 0, 0},
            {"Effets et altérations", "Poison", "Altération lente et dangereuse.", "Très utile contre les chairs vivantes, beaucoup moins contre morts-vivants, slimes simples ou constructions. L'antidote reste le contre le plus direct.", "Effet connu", 0, 0},
            {"Effets et altérations", "Givre", "Froid qui ralentit et fragilise le rythme.", "Le givre punit les combattants qui comptent sur l'initiative. Les sources chaudes, anti-givre et protections élémentaires l'atténuent.", "Effet connu", 0, 0},
            {"Effets et altérations", "Choc", "Décharge amplifiée par le métal.", "Le choc devient plus sévère si arme et armure métalliques conduisent l'impact. Potion isolante et équipement adapté comptent beaucoup.", "Effet connu", 0, 0},
            {"Effets et altérations", "Saignement", "Perte de sang après entaille.", "Fort contre les êtres vivants, faible contre os, gelées et constructions. Les dagues et munitions barbelées l'exploitent bien.", "Effet connu", 0, 0},
            {"Effets et altérations", "Affaiblissement", "Force offensive réduite pendant quelques tours.", "L'affaiblissement ne tue pas vite, mais il protège énormément sur les longs combats. Purification ou attente peuvent le retirer.", "Effet étudié", 0, 0},
            {"Effets et altérations", "Faille ouverte", "Défense temporairement fissurée.", "La faille augmente les dégâts reçus. Elle se combine très bien avec une attaque lourde, mais doit être exploitée avant de se refermer.", "Effet étudié", 0, 0},
            {"Effets et altérations", "Voile élémentaire", "Protection courte contre plusieurs altérations.", "Réduit la force des brûlures, poisons, givre, choc et saignement. Excellent avant un boss ou une zone à élément dominant.", "Effet étudié", 0, 0},
            {"Effets et altérations", "Suture de mana", "Régénération courte par mana stabilisé.", "Ne remplace pas une potion d'urgence, mais soutient un combat long si le catalyseur reste fiable.", "Effet étudié", 0, 0},
            {"Objets rares", "Supports magiques", "Parchemins et grimoires ne servent pas au même usage.", "Un parchemin est un consommable à usage unique : un non-mage peut déclencher certains effets, et un mage peut aussi s'en servir. Un grimoire permet d'apprendre durablement un sort, mais demande classe compatible, niveau, catalyseur et stabilité. Certains sorts existent seulement en grimoire, d'autres seulement en parchemin.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Catalyseurs de mage", "La magie dépend aussi de l'objet qui la canalise.", "Un bâton, une baguette, une pierre ou un autre vrai catalyseur rend la magie plus stable. Un catalyseur brisé ou épuisé fait baisser la réussite. Une arme non magique peut forcer le sort à passer, mais avec moins de contrôle.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Purification mineure", "Parchemin sans grimoire courant.", "Cette magie existe surtout sur papier : elle arrache plusieurs altérations simples puis disparaît avec le support. Un mage peut l'utiliser aussi, même s'il sait déjà lancer d'autres sorts.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Suture de mana", "Grimoire sans parchemin courant.", "Sort étudiable par les vrais canalisateurs : il ne rend pas tous les PV immédiatement, mais pose une régénération courte. Mauvais catalyseur = réussite plus basse.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Ronces occultes", "Grimoire sans parchemin courant.", "Sort d'entrave sombre : ronces de mana, poison léger et affaiblissement. Il demande un vrai profil magique et un catalyseur correct.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Venin rampant", "Parchemin sans grimoire courant.", "Magie jetable : utilisable par mage ou non-mage, mais perdue à l'usage. Elle empoisonne et fatigue sans donner un sort durable au personnage.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Munitions spéciales", "Les armes à distance dépendent du type de munition.", "Arc = flèches, arbalète = carreaux, équipement de lancer = couteaux. Le joueur choisit la munition à chaque tir si plusieurs lots sont compatibles. Si aucune munition compatible n'existe, l'arme ne tire pas : elle ne sert qu'en défense d'urgence, sans devenir magique par hasard.", "Étude active", 0, 0},
            {"Races", "Spécialités de classes en combat", "Les classes commencent à se ressentir davantage.", "Assassin/ombrelame peuvent ouvrir un saignement, classes lourdes s'ancrent davantage, mages peuvent laisser une trace élémentaire faible, classes sacrées peuvent récupérer un peu sous pression. Les effets restent légers : la guilde enseigne d'abord les bases avant les vraies spécialisations.", "Étude active", 0, 0},
            {"Races", "Résistances et faiblesses", "Les éléments réagissent avec races, classes et matières.", "Le feu, le froid, le poison, le choc, le saignement et les vulnérabilités ne doivent pas être universels. Le métal attire mieux certains chocs, les plantes craignent davantage le feu, les corps organiques réagissent différemment au poison, et l'équipement porté peut aider ou aggraver l'effet.", "Étude active", 0, 0},
            {"Matériaux et plantes", "Dégradation de récolte", "La façon de combattre peut abîmer un composant.", "Flèches, feu, explosions ou armes lourdes peuvent produire des composants troués, brûlés ou écrasés. La maîtrise d'arme réduit peu à peu ces risques.", "Renseignement commun", 0, 0},
            {"Matériaux et plantes", "Récolte propre", "Une récolte bien faite peut améliorer une qualité.", "Une récolte propre peut faire passer impur vers normal, normal vers pur/haute qualité, puis exceptionnel. L'achat direct d'exceptionnel reste impossible.", "Renseignement commun", 0, 0},
            {"Boss", "??? - boss non révélé", "Identité verrouillée.", "Cette entrée volontairement floue rappelle que le nom d'un boss ne doit pas être ajouté gratuitement. Il faut une révélation directe ou un écrit crédible.", "Verrouillé", 0, 0},
            {"Boss", "Fitoria", "Ange de jugement lumineux.", "Fitoria juge, enchaîne, soigne de façon contrôlée et libère Sentence céleste après son passage sous les 50% PV.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Zelef", "Démon de corruption et de sang noir.", "Zelef peut voler de la vie, corrompre et laisser une Corrosion présente si le joueur perd. Le vaincre peut rendre ce qu'il a pris.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Atlas", "Protecteur universel déchu.", "Atlas encaisse avec ses plaques, fissures et contre-gardes. Son Dernier rempart transforme sa défense en menace.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Lyknir", "Écho de la Meute.", "Lyknir traque les habitudes du joueur. Son instinct de prédateur n'est pas moral : il lit une proie, pas une faute.", "Boss validé", 0, 0},
            {"Boss", "Grinka", "Reine gobeline de l'avarice.", "Grinka transforme le combat en dette, taxe et enchère gobeline. Si elle gagne, certaines pertes deviennent réelles jusqu'à revanche.", "Boss validé", 0, 0},
            {"Boss", "Fragment de Thamarys", "Fragment draconique.", "Thamarys possède des écailles à briser et un souffle d'origine. Le combattre revient à apprendre quelle défense draconique est encore debout.", "Boss validé", 0, 0},
            {"Boss", "Mojo", "Esprit mythique de la forêt.", "Mojo peut être vaincu brutalement ou apaisé. La victoire respectueuse ouvre une récompense et une lecture très différentes.", "Boss validé", 0, 0},
            {"Boss", "Reflet d'Inakari", "Reflet kitsune trompeur.", "Inakari ment avec élégance, propose de fausses récompenses et transforme les choix en miroirs dangereux.", "Boss validé", 0, 0},
            {"Boss", "L'Anomalie", "Erreur fière qui attaque l'interface.", "L'Anomalie corrompt les menus, invente de fausses sorties et réagit aux personnages altérés ou aux tentatives interdites.", "Boss finalisé", 0, 0},
            {"Boss", "Manifestation de Moiran", "Manifestation affaiblie du Destin.", "Moiran ne joue pas la chance : il observe la trajectoire. Les cheats, les morts et les lignes refusées deviennent des preuves.", "Boss finalisé", 0, 0},
            {"Boss", "Écho fragmenté d'Obérion", "Fragment approuvé de l'origine.", "Obérion ramène le combat à l'Origine nue : au-delà de 50%, l'arme et l'armure cessent d'être reconnues.", "Boss finalisé", 0, 0},
            {"Boss", "Avatar affaibli de FireFlight", "Créateur, aventurier, boss final.", "FireFlight est verrouillé derrière les invitations de boss. Après sa chute, il lance un test final de trois tours contre les souvenirs d'ultimes.", "Boss final", 0, 0},
            {"Divinités / lore", "FireFlight", "Créateur du monde et trace du créateur.", "FireFlight peut être personnage, boss, créateur et regard derrière le jeu. Ses dialogues changent avec les personnages spéciaux et les altérations.", "Lore sensible", 0, 0},
            {"Divinités / lore", "Moiran", "Destin créé par l'Ordre et le Temps.", "Moiran ferme des routes plutôt que de lancer des dés. Il réagit aux cheats, surtout aux cheats tentés en Léthal.", "Lore validé", 0, 0},
            {"Divinités / lore", "Obérion", "Dieu universel et père des primordiaux.", "Obérion complet dépasse le combat actuel. Seuls des fragments approuvés sont affrontables sans briser l'échelle du monde.", "Lore validé", 0, 0},
            {"Objets rares", "Invitations de boss", "Lettres nécessaires pour FireFlight.", "Chaque boss vaincu peut laisser une invitation. Quand toutes les lettres existent, l'entrée du boss final accepte enfin de s'ouvrir.", "Système boss final", 0, 0},
            {"Objets rares", "Fragments de boss avancés", "Matériaux uniques de boss.", "Fragments de nom perdu, miroir fendu, noyau de version instable, sceaux et traces divines nourrissent les crafts ou reliques majeures.", "Fiche archivée", 0, 0},
            {"Objets rares", "Particularités de craft", "Effets faibles nés de matériaux exceptionnels.", "Un objet crafté peut recevoir une petite particularité si plus de 50% de sa valeur de craft vient de matériaux exceptionnels. Les classes d'artisanat augmentent maintenant légèrement cette chance.", "Étude active", 0, 0},
            {"Habitats / zones", "Traces de territoire", "Les zones racontent parfois le monstre avant le combat.", "Griffures sur un arbre, os déplacés, gel anormal, suie récente ou silence soudain peuvent ajouter une entrée sans tuer la créature. Une trace ne donne pas tout : elle confirme surtout l'existence et le terrain probable.", "Méthode d'observation", 0, 0},
            {"Habitats / zones", "Cadavre étudié", "Une victoire n'est pas la seule manière d'apprendre.", "Un corps ancien, une mue, un nid abandonné ou une arme cassée peut révéler faiblesse, résistance ou butin possible. Les informations restent moins fiables qu'une vraie rencontre, mais elles aident à préparer la chasse.", "Méthode d'observation", 0, 0},
            {"Matériaux et plantes", "Qualité de dépouille", "La manière de vaincre change ce qui reste.", "Une dépouille brûlée, explosée ou percée trop violemment peut donner moins de composants propres. Une mise à mort contrôlée, une arme adaptée ou une récolte calme augmente les chances de matière propre.", "Méthode d'observation", 0, 0},
            {"Objets rares", "Pages de terrain annotées", "Certaines pages ne donnent pas un sort, mais une lecture du monde.", "Les pages trouvées ou achetées peuvent ajouter une fiche, améliorer une famille ou donner une piste de butin. Elles ne remplacent pas l'expérience, mais peuvent éviter de mourir idiot devant une faiblesse évidente.", "Renseignement de bibliothèque", 0, 0},
            {"Boss", "Rumeurs de boss", "Une rumeur ne suffit pas à révéler un nom.", "Le registre peut noter une silhouette, un titre ou un comportement sans donner l'identité complète. Le nom reste caché tant que le boss ne l'a pas offert lui-même ou qu'un écrit fiable ne le confirme pas.", "Renseignement incomplet", 0, 0}
        };

        return entries;
    }

    std::vector<BestiaryPreviewEntry> filterEntries(const std::string& category)
    {
        std::vector<BestiaryPreviewEntry> filtered;

        for (BestiaryPreviewEntry entry : getEntries())
        {
            entry.encounters += BestiaryRuntimeProgress::getEncounterCount(entry.name);
            entry.kills += BestiaryRuntimeProgress::getKillCount(entry.name);
            entry.status = BestiaryRuntimeProgress::getStatusFor(entry.name, entry.status);

            if (category == "Tout" || entry.category == category)
            {
                filtered.push_back(entry);
            }
        }

        for (const BestiaryRuntimeRecord& record : BestiaryRuntimeProgress::getRecords())
        {
            bool alreadyListed = false;

            for (const BestiaryPreviewEntry& entry : filtered)
            {
                if (entry.name == record.name)
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (!alreadyListed && (category == "Tout" || record.category == category))
            {
                filtered.push_back({
                    record.category,
                    record.name,
                    record.description,
                    record.description,
                    record.status,
                    record.encounters,
                    record.kills
                });
            }
        }

        for (const MaterialKnowledgeRecord& materialRecord : MaterialKnowledgeProgress::getRecords())
        {
            if (category != "Tout" && category != "Matériaux et plantes")
            {
                continue;
            }

            std::string label = materialRecord.name;

            if (!materialRecord.quality.empty() && materialRecord.quality != "normal")
            {
                label += " [" + materialRecord.quality + "]";
            }

            bool alreadyListed = false;

            for (const BestiaryPreviewEntry& entry : filtered)
            {
                if (entry.name == label)
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (!alreadyListed)
            {
                std::string detail = "Journal réel : " + label
                    + " a été récupéré "
                    + std::to_string(materialRecord.discoveredQuantity)
                    + " fois. Catégorie : "
                    + materialRecord.category
                    + ". Poids de craft observé : "
                    + std::to_string(materialRecord.bestQualityWeight)
                    + ".";

                filtered.push_back({
                    "Matériaux et plantes",
                    label,
                    "Matériau réellement découvert pendant l'aventure.",
                    detail,
                    "Découvert en jeu",
                    materialRecord.discoveredQuantity,
                    0
                });
            }
        }

        return filtered;
    }


    // EN: calculateKnowledgeLevel declares or implements a focused behavior used by this module.
    // FR: calculateKnowledgeLevel déclare ou implémente un comportement précis utilisé par ce module.
    int calculateKnowledgeLevel(const BestiaryPreviewEntry& entry)
    {
        const std::string statusLower = lowerCopy(entry.status);

        if (statusLower.find("verrouill") != std::string::npos
            || statusLower.find("identité verrouillée") != std::string::npos)
        {
            return 0;
        }

        int level = 0;

        if (statusLower.find("connu") != std::string::npos
            || statusLower.find("spécial") != std::string::npos
            || statusLower.find("renseignement") != std::string::npos
            || statusLower.find("document") != std::string::npos
            || statusLower.find("effet") != std::string::npos
            || statusLower.find("carte") != std::string::npos
            || statusLower.find("danger") != std::string::npos
            || statusLower.find("observation") != std::string::npos
            || statusLower.find("fiche") != std::string::npos
            || entry.encounters > 0)
        {
            level = 1;
        }

        if (entry.kills > 0
            || statusLower.find("combattu") != std::string::npos
            || statusLower.find("acheté") != std::string::npos
            || statusLower.find("lu") != std::string::npos
            || statusLower.find("étudi") != std::string::npos
            || statusLower.find("dangereuse") != std::string::npos
            || statusLower.find("valid") != std::string::npos)
        {
            level = 2;
        }

        if (entry.kills >= 3
            || statusLower.find("renseignement acheté") != std::string::npos
            || statusLower.find("spécial") != std::string::npos
            || statusLower.find("boss final") != std::string::npos
            || statusLower.find("lore valid") != std::string::npos)
        {
            level = 3;
        }

        return level;
    }

    std::string knowledgeLabel(const BestiaryPreviewEntry& entry)
    {
        int level = calculateKnowledgeLevel(entry);

        if (level == 0) return "0 - inconnu / verrouillé";
        if (level == 1) return "1 - existence confirmée";
        if (level == 2) return "2 - informations utiles";
        return "3 - fiche complète actuelle";
    }

    std::string knowledgeProgressHint(const BestiaryPreviewEntry& entry)
    {
        const int level = calculateKnowledgeLevel(entry);

        if (level <= 0)
        {
            return "Trouver une preuve crédible, rencontrer l'entité ou acheter un renseignement fiable.";
        }

        if (level == 1)
        {
            return "Encore 1 victoire, 3 rencontres environ, ou une page de bibliothèque peuvent rendre les infos utiles.";
        }

        if (level == 2)
        {
            return "Plusieurs observations, environ 3 victoires ou un renseignement spécialisé peuvent compléter la fiche.";
        }

        return "Fiche suffisamment complète pour la préparation actuelle.";
    }

    std::string displayNameForEntry(const BestiaryPreviewEntry& entry)
    {
        if (calculateKnowledgeLevel(entry) == 0)
        {
            return "???";
        }

        return entry.name;
    }

    MenuScreen buildEntryDetailScreen(const BestiaryPreviewEntry& entry)
    {
        int knowledgeLevel = calculateKnowledgeLevel(entry);
        KnowledgeHints hints = buildKnowledgeHints(entry.category, entry.name, entry.detailedDescription);

        MenuScreen screen("FICHE BESTIAIRE", "bestiary.entry.detail");
        screen.addLine("Catégorie : " + entry.category);
        screen.addLine("Nom : " + displayNameForEntry(entry));
        screen.addLine("Statut : " + entry.status);
        screen.addLine("Niveau de connaissance : " + knowledgeLabel(entry));
        screen.addLine("Progression : " + knowledgeProgressHint(entry));
        screen.addLine("Danger estimé : " + std::string(knowledgeLevel <= 0 ? "???" : hints.dangerRank));
        screen.addLine("Rencontres : " + std::to_string(entry.encounters));
        screen.addLine("Tués par le joueur : " + std::to_string(entry.kills));

        if (knowledgeLevel <= 0)
        {
            screen.addLine("Information verrouillée.");
            screen.addLine("Le nom, les détails et les indices importants restent cachés tant que ton personnage n'a pas découvert une preuve crédible.");
        }
        else
        {
            screen.addLine("Résumé : " + std::string(knowledgeLevel == 1 ? entry.simpleDescription : entry.detailedDescription));
            screen.addLine("Habitat / origine : " + hints.habitat);

            if (knowledgeLevel >= 2)
            {
                screen.addLine("Faiblesses probables : " + hints.weaknesses);
                screen.addLine("Résistances probables : " + hints.resistances);
            }
            else
            {
                screen.addLine("Faiblesses : encore imprécises.");
                screen.addLine("Résistances : encore imprécises.");
            }

            if (knowledgeLevel >= 3)
            {
                screen.addLine("Butin / ressource liée : " + hints.drops);
                screen.addLine("Conseil de chasse : " + hints.strategy);
            }
            else
            {
                screen.addLine("Butin / ressource liée : à confirmer.");
                screen.addLine("Conseil de chasse : continuer l'observation pour fiabiliser la fiche.");
            }
        }

        screen.addOption(0, "Continuer", "", true, "bestiary.entry.continue");
        return screen;
    }

    void displayEntryDetail(const BestiaryPreviewEntry& entry)
    {
        TerminalInterface::renderMenuScreen(buildEntryDetailScreen(entry));
    }

    void displayEntrySelectionList(const std::string& title, const std::vector<BestiaryPreviewEntry>& entries)
    {
        if (entries.empty())
        {
            MessageScreen::show(
                "BESTIAIRE",
                "bestiary.entry.empty",
                {"Aucune entrée préparée pour cette sélection."}
            );
            return;
        }

        const std::size_t itemsPerPage = 12;
        std::size_t pageIndex = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(entries.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, itemsPerPage);

            MenuScreen screen(title, "bestiary.entry.list");
            screen.addLine("Page " + std::to_string(pageIndex + 1) + " / " + std::to_string(totalPages));
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, entries.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const int localChoice = static_cast<int>(i - first + 1);
                const BestiaryPreviewEntry& entry = entries[i];
                KnowledgeHints hints = buildKnowledgeHints(entry.category, entry.name, entry.detailedDescription);

                std::string label = displayNameForEntry(entry)
                    + " | " + entry.category
                    + " | Connaissance : " + knowledgeLabel(entry)
                    + " | Danger : " + std::string(calculateKnowledgeLevel(entry) <= 0 ? "???" : hints.dangerRank)
                    + " | Tués : " + std::to_string(entry.kills);

                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "bestiary";
                itemData.section = entry.category;
                itemData.actionType = "inspect";
                itemData.name = displayNameForEntry(entry);
                itemData.detail = calculateKnowledgeLevel(entry) <= 0 ? "Information verrouillée" : entry.simpleDescription;
                itemData.status = knowledgeLabel(entry);
                itemData.progress = "Tués : " + std::to_string(entry.kills);
                itemData.owner = calculateKnowledgeLevel(entry) <= 0 ? "???" : hints.habitat;
                itemData.important = calculateKnowledgeLevel(entry) >= 2 || entry.kills > 0;

                screen.addOption(localChoice, label, "Inspecter cette fiche du bestiaire.", true, "bestiary.entry.select." + std::to_string(i), itemData);
            }

            if (pageIndex > 0)
            {
                screen.addOption(98, "Page précédente", "", true, "bestiary.entry.previous");
            }
            if (pageIndex + 1 < totalPages)
            {
                screen.addOption(99, "Page suivante", "", true, "bestiary.entry.next");
            }
            screen.addOption(0, "Retour", "", true, "bestiary.entry.back");
            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choix invalide."
            );

            Console::clear();

            if (choice == 0)
            {
                return;
            }

            if (choice == 98 && pageIndex > 0)
            {
                pageIndex--;
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                pageIndex++;
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice < 1 || choice > visibleCount)
            {
                MessageScreen::show(
                    "ENTRÉE NON AFFICHÉE",
                    "bestiary.entry.not_visible",
                    {
                        "Cette entrée n'est pas visible sur la page actuelle.",
                        "Utilise les options de page ou choisis une fiche affichée."
                    }
                );
                continue;
            }

            const std::size_t selectedIndex = first + static_cast<std::size_t>(choice - 1);
            displayEntryDetail(entries[selectedIndex]);
            Console::waitForEnter();
            Console::clear();
        }
    }

    // EN: displayEntryList declares or implements a focused behavior used by this module.
    // FR: displayEntryList déclare ou implémente un comportement précis utilisé par ce module.
    void displayEntryList(const std::string& category)
    {
        displayEntrySelectionList(category, filterEntries(category));
    }

    void displayKnowledgeLevelBrowser()
    {
        while (true)
        {
            MenuScreen screen("REGISTRE PAR CONNAISSANCE", "bestiary.knowledge_levels");
            screen.addOption(0, "Retour", "", true, "bestiary.knowledge.back");
            screen.addOption(1, "Entrées inconnues / verrouillées", "", true, "bestiary.knowledge.level0");
            screen.addOption(2, "Existence confirmée", "", true, "bestiary.knowledge.level1");
            screen.addOption(3, "Informations utiles", "", true, "bestiary.knowledge.level2");
            screen.addOption(4, "Fiches complètes actuelles", "", true, "bestiary.knowledge.level3");
            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choix invalide."
            );
            Console::clear();

            if (choice == 0)
            {
                return;
            }

            const int wantedLevel = choice - 1;
            std::vector<BestiaryPreviewEntry> filtered;

            for (const BestiaryPreviewEntry& entry : filterEntries("Tout"))
            {
                if (calculateKnowledgeLevel(entry) == wantedLevel)
                {
                    filtered.push_back(entry);
                }
            }

            std::string title;
            if (wantedLevel == 0) title = "Entrées inconnues / verrouillées";
            else if (wantedLevel == 1) title = "Existence confirmée";
            else if (wantedLevel == 2) title = "Informations utiles";
            else title = "Fiches complètes actuelles";

            displayEntrySelectionList(title, filtered);
            Console::clear();
        }
    }

    void showBestiaryInfoScreen(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }

    void displayHuntingNotebook()
    {
        std::vector<BestiaryPreviewEntry> entries = filterEntries("Tout");
        std::sort(entries.begin(), entries.end(), [](const BestiaryPreviewEntry& a, const BestiaryPreviewEntry& b) {
            const int scoreA = calculateKnowledgeLevel(a) * 100 + a.encounters + a.kills * 3;
            const int scoreB = calculateKnowledgeLevel(b) * 100 + b.encounters + b.kills * 3;

            if (scoreA == scoreB)
            {
                return a.name < b.name;
            }

            return scoreA < scoreB;
        });

        std::vector<std::string> lines;
        lines.push_back("Le carnet sert à choisir quoi observer ensuite au lieu d'ouvrir toute la liste.");
        lines.push_back("");
        lines.push_back("Fiches à compléter en priorité :");

        int shown = 0;
        for (const BestiaryPreviewEntry& entry : entries)
        {
            const int level = calculateKnowledgeLevel(entry);

            if (level >= 3)
            {
                continue;
            }

            lines.push_back("- " + displayNameForEntry(entry)
                + " | " + entry.category
                + " | " + knowledgeLabel(entry)
                + " | " + knowledgeProgressHint(entry));

            shown++;

            if (shown >= 8)
            {
                break;
            }
        }

        if (shown == 0)
        {
            lines.push_back("- Aucune priorité évidente : le registre est déjà très solide.");
        }

        lines.push_back("");
        lines.push_back("Pistes de terrain :");
        lines.push_back("- 1 rencontre confirme souvent l'existence.");
        lines.push_back("- 3 rencontres ou 1 victoire rendent la fiche plus utile.");
        lines.push_back("- Plusieurs victoires, notes de bibliothèque ou observations rares complètent la préparation.");
        lines.push_back("- Les boss gardent leur nom masqué sans révélation crédible.");

        showBestiaryInfoScreen("CARNET DE TRAQUE", "bestiary.hunting_notebook.detail", lines);
    }


    // EN: displayMaterialJournal declares or implements a focused behavior used by this module.
    // FR: displayMaterialJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterialJournal()
    {
        showBestiaryInfoScreen(
            "JOURNAL DES MATÉRIAUX",
            "bestiary.material_journal.detail",
            {
                "Ce journal complète le bestiaire : il explique comment un composant peut être trouvé, abîmé, amélioré ou utilisé.",
                "",
                "Qualités principales :",
                "- composants de monstre : impur / normal / pur / exceptionnel",
                "- plantes et matériaux classiques : faible qualité / normal / haute qualité / exceptionnel",
                "",
                "Règles utiles :",
                "- deux qualités différentes ne stackent pas ensemble ;",
                "- une action brutale peut dégrader un composant ;",
                "- une récolte propre peut améliorer la qualité d'un cran ;",
                "- les matériaux exceptionnels ne sont pas achetables directement ;",
                "- si plus de 50% de la valeur d'un craft vient d'exceptionnel, une particularité faible peut apparaître.",
                "",
                "Exemples suivis par le journal :",
                "- Oreille de gobelin : peut être trouée, brûlée ou propre selon le combat ;",
                "- Peau de bête robuste : utile aux kits et armures ;",
                "- Braise kitsune : composant magique sensible aux invocations ;",
                "- Noyau instable : puissant mais dangereux pour l'alchimie et les expériences."
            }
        );
    }

    // EN: displayCraftJournal declares or implements a focused behavior used by this module.
    // FR: displayCraftJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displayCraftJournal()
    {
        std::vector<std::string> lines;
        lines.push_back("Ce journal suit les fabrications réussies pendant la session actuelle.");
        lines.push_back("Il nourrit les savoir-faire liés à l'artisanat, l'alchimie, la forge et aux armes utilisées.");
        lines.push_back("");
        lines.push_back("Total fabriqué cette session : " + std::to_string(MaterialExperimentLog::getTotalCrafted()));

        const std::vector<CraftExperimentRecord>& records = MaterialExperimentLog::getCraftRecords();

        if (records.empty())
        {
            lines.push_back("Aucun craft suivi pour le moment.");
        }
        else
        {
            for (const CraftExperimentRecord& record : records)
            {
                lines.push_back("- " + record.recipeName + " : " + std::to_string(record.craftedCount) + " fabrication(s)");
            }
        }

        lines.push_back("");
        lines.push_back("Rappel : les matériaux exceptionnels majoritaires peuvent créer une particularité faible.");
        lines.push_back("Forgeron, Alchimiste et Artificier ont maintenant un léger avantage économique et artisanal.");

        showBestiaryInfoScreen("JOURNAL DU CRAFT", "bestiary.craft_journal.detail", lines);
    }

    // EN: displaySummonJournal declares or implements a focused behavior used by this module.
    // FR: displaySummonJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displaySummonJournal()
    {
        showBestiaryInfoScreen(
            "JOURNAL DES INVOCATIONS",
            "bestiary.summon_journal.detail",
            {
                "Les invocations utilisent maintenant une base plus claire : slots, maintien, durée, contrôle manuel et sacrifice.",
                "",
                "Règles observées :",
                "- slots d'invocation : certaines invocations lourdes occupent plus d'un slot ;",
                "- maintien : une invocation peut prolonger son lien au lieu d'attaquer ;",
                "- sacrifice : une invocation peut rompre son lien pour infliger une rupture ;",
                "- ciblage : les ennemis peuvent attaquer les invocations selon leur logique ;",
                "- conduite : instinctive ou guidée selon le lien avec l'invocateur.",
                "",
                "Notes incomplètes : certains rituels parlent encore de mana profond, de sacrifices avancés, d'ombres de Hazak et de formations de groupe."
            }
        );
    }

    // EN: displayInformationShopPreview declares or implements a focused behavior used by this module.
    // FR: displayInformationShopPreview déclare ou implémente un comportement précis utilisé par ce module.
    void displayInformationShopPreview()
    {
        showBestiaryInfoScreen(
            "ACHAT D'INFORMATIONS",
            "bestiary.info_shop.detail",
            {
                "La bibliothèque débloque des renseignements qui montent le niveau de connaissance du bestiaire.",
                "Les secrets, les boss et les noms importants devront toujours se mériter.",
                "",
                "Un nom de boss ne sera pas donné gratuitement si personne ne l'a prononcé."
            }
        );
    }


    void addCategoryCount(
        std::vector<std::pair<std::string, int>>& counts,
        const std::string& category
    )
    {
        for (std::pair<std::string, int>& count : counts)
        {
            if (count.first == category)
            {
                count.second++;
                return;
            }
        }

        counts.push_back({category, 1});
    }

    void displayKnowledgeSummary()
    {
        std::vector<BestiaryPreviewEntry> entries = filterEntries("Tout");
        int level0 = 0;
        int level1 = 0;
        int level2 = 0;
        int level3 = 0;
        int totalKills = 0;
        int totalEncounters = 0;
        std::vector<std::pair<std::string, int>> categoryCounts;

        for (const BestiaryPreviewEntry& entry : entries)
        {
            const int level = calculateKnowledgeLevel(entry);

            if (level <= 0) level0++;
            else if (level == 1) level1++;
            else if (level == 2) level2++;
            else level3++;

            totalKills += entry.kills;
            totalEncounters += entry.encounters;
            addCategoryCount(categoryCounts, entry.category);
        }

        std::vector<std::string> lines;
        lines.push_back("Entrées suivies : " + std::to_string(entries.size()));
        lines.push_back("Rencontres enregistrées : " + std::to_string(totalEncounters));
        lines.push_back("Tués enregistrés : " + std::to_string(totalKills));
        lines.push_back("");
        lines.push_back("Connaissance 0 / verrouillée : " + std::to_string(level0));
        lines.push_back("Connaissance 1 / existence : " + std::to_string(level1));
        lines.push_back("Connaissance 2 / utile : " + std::to_string(level2));
        lines.push_back("Connaissance 3 / complète actuelle : " + std::to_string(level3));
        lines.push_back("");
        lines.push_back("Répartition par catégorie :");

        for (const std::pair<std::string, int>& count : categoryCounts)
        {
            lines.push_back("- " + count.first + " : " + std::to_string(count.second));
        }

        std::vector<BestiaryRuntimeRecord> runtimeRecords = BestiaryRuntimeProgress::getRecords();
        std::sort(runtimeRecords.begin(), runtimeRecords.end(), [](const BestiaryRuntimeRecord& a, const BestiaryRuntimeRecord& b) {
            const int scoreA = a.encounters + a.kills * 3;
            const int scoreB = b.encounters + b.kills * 3;

            if (scoreA == scoreB)
            {
                return a.name < b.name;
            }

            return scoreA > scoreB;
        });

        lines.push_back("");
        lines.push_back("Découvertes réelles les plus observées :");

        if (runtimeRecords.empty())
        {
            lines.push_back("- Aucune trace réelle enregistrée dans cette session ou sauvegarde.");
        }
        else
        {
            const int limit = std::min(5, static_cast<int>(runtimeRecords.size()));

            for (int i = 0; i < limit; ++i)
            {
                const BestiaryRuntimeRecord& record = runtimeRecords[i];
                lines.push_back("- " + record.name
                    + " | " + record.category
                    + " | rencontres " + std::to_string(record.encounters)
                    + " | tués " + std::to_string(record.kills)
                    + " | " + record.status);
            }
        }

        showBestiaryInfoScreen("SYNTHÈSE DU BESTIAIRE", "bestiary.summary.detail", lines);
    }

    void displayTacticalIndex()
    {
        showBestiaryInfoScreen(
            "INDEX TACTIQUE",
            "bestiary.tactical_index.detail",
            {
                "Lecture rapide des grandes familles. Les fiches individuelles restent plus précises quand elles sont observées.",
                "",
                "Gelées / slimes",
                "- Saignement faible, poison souvent mauvais, couleur à identifier avant de lancer un sort rare.",
                "- Rouge : froid utile. Bleu/blanc : chaleur utile. Jaune : attention au métal. Violet/noir : antidotes utiles.",
                "",
                "Humanoïdes / gobelins / bandits",
                "- Peu de résistances naturelles, mais l'équipement et les rôles changent tout.",
                "- Les soutiens crédibles se focus : shaman, apothicaire, oracle, chef.",
                "",
                "Bêtes",
                "- Dangereuses sur cible isolée ou blessée. Entrave et contre après charge sont utiles.",
                "- Les matériaux se dégradent si le combat est trop brutal.",
                "",
                "Morts-vivants / os / ombres",
                "- Poison et saignement souvent faibles. Lumière, feu, rupture d'os ou purification sont plus fiables.",
                "- Les supports spectraux et oracles doivent être traités comme des soutiens, pas comme de simples sacs à PV.",
                "",
                "Constructions / armures mortes",
                "- Les entailles faibles déçoivent. Marteau, choc ciblé, faille, corrosion ou brise-garde gagnent en valeur.",
                "- Elles donnent souvent de bons composants si le combat ne les broie pas n'importe comment.",
                "",
                "Draconides / dragons",
                "- Même jeunes, ils demandent préparation : souffle, écailles et matériaux fragiles après combat.",
                "- Une victoire trop brutale peut donner moins de ressources propres.",
                "",
                "Plantes",
                "- Feu et coupe nette. Le poison est souvent décevant. L'entrave doit être cassée vite.",
                "",
                "Élémentaires / effets",
                "- Un élément résiste souvent à lui-même. Le métal amplifie le choc. Le voile élémentaire réduit plusieurs altérations.",
                "- Les anomalies peuvent punir les automatismes : observer le premier tour vaut parfois une potion économisée.",
                "",
                "Zones",
                "- Plaine = bases, Route = humanoïdes, Mares = slimes, Forêt = entraves, Montagne = froid/roche, Marais = poison, Cimetière = morts, Ruines = anomalies/constructions."
            }
        );
    }

}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
void BestiaryMenu::open()
{
    while (true)
    {
        MenuScreen screen("BESTIAIRE", "bestiary.hub");
        screen.addLine("Objet spécial de base : Bestiaire");
        screen.addLine("État du registre : catégories propres, entrées ???, niveaux de connaissance et traces sauvegardables.");
        screen.addOption(0, "Retour", "", true, "bestiary.back");
        screen.addOption(1, "Tout voir", "", true, "bestiary.all");
        screen.addOption(2, "Races", "", true, "bestiary.races");
        screen.addOption(3, "Entités hostiles / ennemis", "", true, "bestiary.hostiles");
        screen.addOption(4, "Entités passives / alliées", "", true, "bestiary.passives");
        screen.addOption(5, "Invocations", "", true, "bestiary.summons");
        screen.addOption(6, "Boss", "", true, "bestiary.bosses");
        screen.addOption(7, "Personnages spéciaux", "", true, "bestiary.special_characters");
        screen.addOption(8, "Matériaux et plantes", "", true, "bestiary.materials");
        screen.addOption(9, "Divinités / lore", "", true, "bestiary.lore");
        screen.addOption(10, "Objets rares", "", true, "bestiary.rare_items");
        screen.addOption(11, "Effets et altérations", "", true, "bestiary.effects");
        screen.addOption(12, "Habitats / zones", "", true, "bestiary.habitats");
        screen.addOption(13, "Journal des matériaux", "", true, "bestiary.material_journal");
        screen.addOption(14, "Journal des invocations", "", true, "bestiary.summon_journal");
        screen.addOption(15, "Acheter des informations communes", "", true, "bestiary.info_shop");
        screen.addOption(16, "Journal du craft", "", true, "bestiary.craft_journal");
        screen.addOption(17, "Index tactique", "", true, "bestiary.tactical_index");
        screen.addOption(18, "Synthèse du bestiaire", "", true, "bestiary.summary");
        screen.addOption(19, "Registre par niveau de connaissance", "", true, "bestiary.knowledge_levels");
        screen.addOption(20, "Carnet de traque", "", true, "bestiary.hunting_notebook");
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option affichée."
        );

        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1) displayEntryList("Tout");
        else if (choice == 2) displayEntryList("Races");
        else if (choice == 3) displayEntryList("Entités hostiles / ennemis");
        else if (choice == 4) displayEntryList("Entités passives / alliées");
        else if (choice == 5) displayEntryList("Invocations");
        else if (choice == 6) displayEntryList("Boss");
        else if (choice == 7) displayEntryList("Personnages spéciaux");
        else if (choice == 8) displayEntryList("Matériaux et plantes");
        else if (choice == 9) displayEntryList("Divinités / lore");
        else if (choice == 10) displayEntryList("Objets rares");
        else if (choice == 11) displayEntryList("Effets et altérations");
        else if (choice == 12) displayEntryList("Habitats / zones");
        else if (choice == 13) displayMaterialJournal();
        else if (choice == 14) displaySummonJournal();
        else if (choice == 15) displayInformationShopPreview();
        else if (choice == 16) displayCraftJournal();
        else if (choice == 17) displayTacticalIndex();
        else if (choice == 18) displayKnowledgeSummary();
        else if (choice == 19) displayKnowledgeLevelBrowser();
        else if (choice == 20) displayHuntingNotebook();
    }
}

// EN: displayObjectEntry declares or implements a focused behavior used by this module.
// FR: displayObjectEntry déclare ou implémente un comportement précis utilisé par ce module.
void BestiaryMenu::displayObjectEntry(const std::string& entryName)
{
    for (BestiaryPreviewEntry entry : getEntries())
    {
        if (entry.name == entryName)
        {
            entry.encounters += BestiaryRuntimeProgress::getEncounterCount(entry.name);
            entry.kills += BestiaryRuntimeProgress::getKillCount(entry.name);
            entry.status = BestiaryRuntimeProgress::getStatusFor(entry.name, entry.status);
            displayEntryDetail(entry);
            return;
        }
    }

    for (const BestiaryRuntimeRecord& record : BestiaryRuntimeProgress::getRecords())
    {
        if (record.name == entryName)
        {
            displayEntryDetail({
                record.category,
                record.name,
                record.description,
                record.description,
                record.status,
                record.encounters,
                record.kills
            });
            return;
        }
    }

    MenuScreen screen("BESTIAIRE", "bestiary.object.unknown");
    screen.addLine("Recherche : " + entryName);
    screen.addLine("Entrée inconnue ou pas encore découverte.");
    screen.addLine("Quand ton personnage découvrira cette chose, le bestiaire pourra afficher son nom, sa description, ses rencontres et ses tués.");
    screen.addOption(0, "Continuer", "", true, "bestiary.object.unknown.continue");
    TerminalInterface::renderMenuScreen(screen);
}
