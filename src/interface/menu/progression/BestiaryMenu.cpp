// EN: BestiaryMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable bestiary menu with prepared categories and detailed entries.
// Français : Affiche le premier bestiaire utilisable avec catégories préparées et fiches détaillées.

#include "interface/menu/progression/BestiaryMenu.hpp"

#include "core/Console.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/material/MaterialKnowledgeProgress.hpp"
#include "progression/material/MaterialExperimentLog.hpp"

#include <iostream>
#include <string>
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

    // EN: getEntries declares or implements a focused behavior used by this module.
    // FR: getEntries déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<BestiaryPreviewEntry>& getEntries()
    {
        static const std::vector<BestiaryPreviewEntry> entries = {
            {"Races", "Humain", "Race commune, adaptable et ambitieuse.", "Les humains sont présents dans beaucoup de villes. Ils servent de base pour les aventuriers, bandits, chevaliers et mages.", "Connu", 0, 0},
            {"Races", "Elfe noir", "Race discrète, souvent liée aux ombres.", "Les elfes noirs peuvent être craints selon les régions. Hazak représente une version très dangereuse de cette race.", "Partiellement connu", 0, 0},
            {"Races", "Kitsune", "Race semi-humaine liée aux illusions et aux flammes.", "Les kitsunes pourront plus tard avoir des bonus de magie, de ruse et d'invocation. Aoi en est un exemple spécial.", "Partiellement connu", 0, 0},
            {"Entités hostiles / ennemis", "Gobelin peureux", "Petit ennemi rapide qui préfère parfois survivre.", "Un gobelin basique. Pas très noble, mais capable de gratter des dégâts et de fuir si le combat devient humiliant.", "Déjà rencontré dans les arènes libres", 1, 0},
            {"Entités hostiles / ennemis", "Famille gobeline", "Petits humanoïdes opportunistes avec plusieurs rôles.", "Les gobelins faibles servent de chair à canon, les frondeurs harcèlent, les gardes tiennent plus longtemps, les shamans peuvent soigner leurs alliés, et les hobgobelins organisent mieux les bandes. Un gobelin normal ne devient pas automatiquement soigneur : il faut un profil crédible.", "Renseignement évolutif", 0, 0},
            {"Entités hostiles / ennemis", "Famille slime", "Gelées célèbres aux couleurs très lisibles.", "Vert = classique/régénération légère, bleu/blanc = froid, rouge = chaleur/brûlure, violet/noir = poison, jaune = choc, ambré = colle et gêne les mouvements, rose = rebond imprévisible, doré = attiré par les choses brillantes, chromatique/prisme/miroir = réaction instable. Les mares gélatineuses concentrent presque toutes les couleurs, mais certaines couleurs apparaissent aussi dans les lieux qui les attirent.", "Renseignement évolutif", 0, 0},
            {"Entités hostiles / ennemis", "Monstres soigneurs crédibles", "Tous les ennemis ne savent pas boire ou lancer une potion.", "Un loup, un slime simple ou une bête sauvage ne comprend pas l'alchimie. Les soins ennemis doivent venir d'un shaman, chamane, apothicaire, oracle, mage ou monstre spécial avec une raison lore. Les IA alliées peuvent aussi soigner joueur ou allié si leur personnalité et leur inventaire s'y prêtent.", "Règle de cohérence", 0, 0},
            {"Entités hostiles / ennemis", "Loup affamé", "Prédateur opportuniste.", "Le loup teste les proies faibles. Plus tard, il pourra cibler les blessés ou les soigneurs selon son instinct.", "Informations communes", 0, 0},
            {"Entités passives / alliées", "Aventurier random", "Humain ou semi-humain généré par l'arène.", "Un adversaire non spécial, avec nom, race et classe tirés aléatoirement. Il peut devenir allié ou rival plus tard.", "Préparé", 0, 0},
            {"Invocations", "Invocation mineure", "Renfort temporaire appelé par certaines classes.", "Les invocations suivent les mêmes règles globales de ciblage que les monstres, mais peuvent appartenir au joueur ou à l'ennemi.", "Préparé", 0, 0},
            {"Invocations", "Esprit mineur", "Petit esprit stable et facile à maintenir.", "Invocation simple : faible coût de slot, maintien léger, attaque régulière et petite régénération. Elle sert de base aux invocateurs.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Serviteur osseux", "Invocation nécromantique solide.", "Le serviteur osseux peut sacrifier une partie de son ossature pour frapper plus fort. Son sacrifice final inflige une rupture dangereuse.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Flamme kitsune", "Flamme liée aux kitsunes.", "La Flamme kitsune brûle vite, frappe fort et reste fragile. Elle correspond particulièrement à Aoi et aux futures voies de feu/invocation.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Bête arcanique", "Invocation lourde consommant plus de slots.", "La Bête arcanique occupe deux slots, encaisse mieux et se nourrit du mana ambiant. Elle transforme davantage le duel en vrai combat de groupe.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Expérience instable", "Créature dangereuse de laboratoire.", "L'Expérience instable tape fort, dure peu et peut exploser. Elle correspond à Fail et aux mécaniques futures de risque expérimental.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Ombre récente", "Ombre née d'un lien violent.", "L'Ombre récente est prévue pour Hazak. Plus tard, elle pourra dépendre de vrais kills, sacrifices ou conditions sombres.", "Secret partiel", 0, 0},
            {"Invocations", "Éclat zodiacal", "Fragment magique lié aux signes.", "L'Éclat zodiacal tire un signe sur treize. Le treizième signe peut prolonger son existence et annonce les futurs sorts de Kanadé.", "Fiche d'invocation", 0, 0},
            {"Boss", "???", "Nom inconnu.", "Un boss ne révèle son vrai nom que s'il le prononce, à l'entrée, à 50%, avant sa défaite, ou via des écrits crédibles trouvés avant.", "Identité verrouillée", 0, 0},
            // EN: "Matt declares or implements a focused behavior used by this module.
            // FR: "Matt déclare ou implémente un comportement précis utilisé par ce module.
            {"Personnages spéciaux", "Matt (PRO)", "Combattant silencieux et respectueux.", "Référence directe à Matt de Wii Sports. Il ne parle pas vraiment : il combat proprement, avec respect, et des bonus globaux légers.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Hazak", "Assassin elfe noir, sombre et efficace.", "Hazak prend les gens de haut, cherche la victoire et protège Hestia d'une violence trop traumatisante. Meilleur ami d'Henrique.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Sanctus / Skuro", "Protecteur pouvant vriller en exécuteur sombre.", "Sanctus protège, entrave et s'appuie sur un dieu de lumière. S'il prend trop cher ou inflige trop cher, Skuro peut ressortir.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Aoi", "Kitsune timide, mage de flammes et future invocatrice.", "Aoi protège ses incantations et peut devenir très dangereuse si on lui laisse le temps de préparer sa magie.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Kanadé", "Semi-dragonne rageuse aux sorts zodiacaux.", "Kanadé râle souvent, mais elle avance quand même. Ses futurs sorts seront liés aux treize signes du zodiaque.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Hestia", "Humaine peureuse aux origines divines oubliées.", "Hestia était une divinité avant de perdre ses souvenirs. Elle préfère éviter le combat, mais possède une magie de protection anormalement haute.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Henrique", "Chevalier fonceur capable de revenir une fois.", "Henrique est le meilleur ami de Hazak. Il fonce dans le tas et possède une future capacité de retour à la vie.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Louis", "Artificier naïf qui cherche des amis.", "Louis vise bien, utilise plusieurs projectiles et reste fondamentalement sympathique malgré le chaos de l'arène.", "Spécial", 0, 0},
            {"Personnages spéciaux", "Trexof", "Assassin humain et bêta-testeur principal.", "Trexof possède de légers bonus et fait partie du groupe proche de Mattzelda et Louis.", "Spécial", 0, 0},
            {"Matériaux et plantes", "Fleur bleue de montagne", "Plante calme et rare.", "Clin d'œil à la fleur bleue de Zelda BOTW. Elle servira plus tard aux remèdes, quêtes et secrets liés aux plantes.", "Renseignement de base", 0, 0},
            {"Matériaux et plantes", "Oreille de gobelin", "Composant commun de monstre.", "Matériau simple récupérable sur certains gobelins. Utile pour les premières ventes ou crafts futurs.", "Loot possible", 0, 0},
            {"Matériaux et plantes", "Peau de bête robuste", "Matériau de réparation épaisse.", "Sert déjà à fabriquer un kit moyen via rafistolage renforcé. Plus tard : armures, tanks et équipement de survie.", "Loot rare", 0, 0},
            {"Matériaux et plantes", "Fil d'ombre", "Composant sombre rare.", "Lié aux assassins, aux morts-vivants et aux futures ombres de Hazak. Ne révèle pas encore tout le système de nécromancie.", "Secret partiel", 0, 0},
            {"Matériaux et plantes", "Braise kitsune", "Braise magique instable.", "Composant prévu pour Aoi, les flammes kitsune, certaines potions avancées et les invocations.", "Rare", 0, 0},
            {"Matériaux et plantes", "Fragment d'écaille draconique", "Fragment défensif rare.", "Matériau très solide, prévu pour protections rares, armures lourdes et crafts semi-dragons.", "Rare", 0, 0},
            {"Matériaux et plantes", "Noyau instable", "Cœur magique dangereux.", "Sert de base future aux expériences de Fail, à l'alchimie risquée, aux objets explosifs et aux invocations instables.", "Dangereux", 0, 0},
            {"Matériaux et plantes", "Qualités de matériaux", "Les qualités modifient valeur et efficacité.", "Impur/faible qualité compte moins, normal compte normalement, pur/haute qualité compte plus et exceptionnel peut déclencher de petites particularités de craft.", "Renseignement commun", 0, 0},
            {"Objets rares", "Munitions spéciales", "Les armes à distance dépendent du type de munition.", "Arc = flèches, arbalète = carreaux, équipement de lancer = couteaux. Le joueur choisit la munition à chaque tir si plusieurs lots sont compatibles. Si aucune munition compatible n'existe, l'arme ne tire pas : elle ne sert qu'en défense d'urgence, sans devenir magique par hasard.", "Système actif", 0, 0},
            {"Races", "Spécialités de classes en combat", "Les classes commencent à se ressentir davantage.", "Assassin/ombrelame peuvent ouvrir un saignement, classes lourdes s'ancrent davantage, mages peuvent laisser une trace élémentaire faible, classes sacrées peuvent récupérer un peu sous pression. Les chiffres restent légers pour éviter de casser l'équilibrage avant le système complet de compétences.", "Système actif", 0, 0},
            {"Matériaux et plantes", "Dégradation de récolte", "La façon de combattre peut abîmer un composant.", "Flèches, feu, explosions ou armes lourdes peuvent produire des composants troués, brûlés ou écrasés. La maîtrise d'arme réduira plus tard ces risques.", "Renseignement commun", 0, 0},
            {"Matériaux et plantes", "Récolte propre", "Une récolte bien faite peut améliorer une qualité.", "Une récolte propre peut faire passer impur vers normal, normal vers pur/haute qualité, puis exceptionnel. L'achat direct d'exceptionnel reste impossible.", "Renseignement commun", 0, 0},
            {"Boss", "??? - boss non révélé", "Identité verrouillée.", "Cette entrée volontairement floue rappelle que le nom d'un boss ne doit pas être ajouté gratuitement. Il faut une révélation directe ou un écrit crédible.", "Verrouillé", 0, 0},
            {"Boss", "Fitoria", "Ange de jugement lumineux.", "Fitoria juge, enchaîne, soigne de façon contrôlée et libère Sentence céleste après son passage sous les 50% PV.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Zelef", "Démon de corruption et de sang noir.", "Zelef peut voler de la vie, corrompre et laisser une Corrosion présente si le joueur perd. Le vaincre plus tard peut rendre ce qu'il a pris.", "Révélé par le registre initial", 1, 0},
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
            {"Divinités / lore", "FireFlight", "Créateur du monde et trace du développeur.", "FireFlight peut être personnage, boss, créateur et regard derrière le jeu. Ses dialogues changent avec les personnages spéciaux et les altérations.", "Lore sensible", 0, 0},
            {"Divinités / lore", "Moiran", "Destin créé par l'Ordre et le Temps.", "Moiran ferme des routes plutôt que de lancer des dés. Il réagit aux cheats, surtout aux cheats tentés en Léthal.", "Lore validé", 0, 0},
            {"Divinités / lore", "Obérion", "Dieu universel et père des primordiaux.", "Obérion complet dépasse le combat actuel. Seuls des fragments approuvés sont affrontables sans briser l'échelle du monde.", "Lore validé", 0, 0},
            {"Objets rares", "Invitations de boss", "Lettres nécessaires pour FireFlight.", "Chaque boss vaincu peut laisser une invitation. Quand toutes les lettres existent, l'entrée du boss final accepte enfin de s'ouvrir.", "Système boss final", 0, 0},
            {"Objets rares", "Fragments de boss avancés", "Matériaux uniques de boss.", "Fragments de nom perdu, miroir fendu, noyau de version instable, sceaux et traces divines serviront plus tard aux crafts ou reliques majeures.", "Préparé", 0, 0},
            {"Objets rares", "Particularités de craft", "Effets faibles nés de matériaux exceptionnels.", "Un objet crafté peut recevoir une petite particularité si plus de 50% de sa valeur de craft vient de matériaux exceptionnels. Les classes d'artisanat augmentent maintenant légèrement cette chance.", "Système actif", 0, 0}
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
        if (entry.status.find("Verrouillé") != std::string::npos
            || entry.status.find("Identité verrouillée") != std::string::npos)
        {
            return 0;
        }

        int level = 0;

        if (entry.status.find("Connu") != std::string::npos
            || entry.status.find("Spécial") != std::string::npos
            || entry.status.find("Renseignement") != std::string::npos
            || entry.encounters > 0)
        {
            level = 1;
        }

        if (entry.kills > 0
            || entry.status.find("Combattu") != std::string::npos
            || entry.status.find("acheté") != std::string::npos
            || entry.status.find("lu") != std::string::npos)
        {
            level = 2;
        }

        if (entry.kills >= 3
            || entry.status.find("Renseignement acheté") != std::string::npos
            || entry.status.find("Spécial") != std::string::npos)
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

    std::string displayNameForEntry(const BestiaryPreviewEntry& entry)
    {
        if (calculateKnowledgeLevel(entry) == 0)
        {
            return "???";
        }

        return entry.name;
    }

    // EN: displayEntryDetail declares or implements a focused behavior used by this module.
    // FR: displayEntryDetail déclare ou implémente un comportement précis utilisé par ce module.
    void displayEntryDetail(const BestiaryPreviewEntry& entry)
    {
        int knowledgeLevel = calculateKnowledgeLevel(entry);

        std::cout << "========== FICHE BESTIAIRE ==========" << std::endl;
        std::cout << "Catégorie : " << entry.category << std::endl;
        std::cout << "Nom : " << displayNameForEntry(entry) << std::endl;
        std::cout << "Statut : " << entry.status << std::endl;
        std::cout << "Niveau de connaissance : " << knowledgeLabel(entry) << std::endl;
        std::cout << "Rencontres : " << entry.encounters << std::endl;
        std::cout << "Tués par le joueur : " << entry.kills << std::endl;
        std::cout << std::endl;

        if (knowledgeLevel <= 0)
        {
            std::cout << "Information verrouillée." << std::endl;
            std::cout << "Le nom, les détails et les indices importants restent cachés tant que ton personnage n'a pas découvert une preuve crédible." << std::endl;
        }
        else if (knowledgeLevel == 1)
        {
            std::cout << entry.simpleDescription << std::endl;
            std::cout << "Détails complets non confirmés. Rencontre, combat, lecture ou achat d'information nécessaire." << std::endl;
        }
        else
        {
            std::cout << entry.detailedDescription << std::endl;
        }

        std::cout << "====================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: displayEntryList declares or implements a focused behavior used by this module.
    // FR: displayEntryList déclare ou implémente un comportement précis utilisé par ce module.
    void displayEntryList(const std::string& category)
    {
        std::vector<BestiaryPreviewEntry> entries = filterEntries(category);

        if (entries.empty())
        {
            std::cout << "Aucune entrée préparée pour cette catégorie." << std::endl;
            std::cout << std::endl;
            return;
        }

        while (true)
        {
            std::cout << "========== " << category << " ==========" << std::endl;
            std::cout << "0 : Retour" << std::endl;

            for (int i = 0; i < static_cast<int>(entries.size()); ++i)
            {
                std::cout << i + 1 << " : " << displayNameForEntry(entries[i])
                          << " | Connaissance : " << knowledgeLabel(entries[i])
                          << " | Statut : " << entries[i].status
                          << " | Tués : " << entries[i].kills
                          << std::endl;
            }

            std::cout << "================================" << std::endl;
            std::cout << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(
                0,
                static_cast<int>(entries.size()),
                "Choix invalide."
            );

            Console::clear();

            if (choice == 0)
            {
                return;
            }

            displayEntryDetail(entries[choice - 1]);
            Console::waitForEnter();
            Console::clear();
        }
    }


    // EN: displayMaterialJournal declares or implements a focused behavior used by this module.
    // FR: displayMaterialJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterialJournal()
    {
        std::cout << "========== JOURNAL DES MATÉRIAUX ==========" << std::endl;
        std::cout << "Ce journal complète le bestiaire : il explique comment un composant peut être trouvé, abîmé, amélioré ou utilisé." << std::endl;
        std::cout << std::endl;
        std::cout << "Qualités principales :" << std::endl;
        std::cout << "- composants de monstre : impur / normal / pur / exceptionnel" << std::endl;
        std::cout << "- plantes et matériaux classiques : faible qualité / normal / haute qualité / exceptionnel" << std::endl;
        std::cout << std::endl;
        std::cout << "Règles utiles :" << std::endl;
        std::cout << "- deux qualités différentes ne stackent pas ensemble ;" << std::endl;
        std::cout << "- une action brutale peut dégrader un composant ;" << std::endl;
        std::cout << "- une récolte propre peut améliorer la qualité d'un cran ;" << std::endl;
        std::cout << "- les matériaux exceptionnels ne sont pas achetables directement ;" << std::endl;
        std::cout << "- si plus de 50% de la valeur d'un craft vient d'exceptionnel, une particularité faible peut apparaître." << std::endl;
        std::cout << std::endl;
        std::cout << "Exemples suivis par le journal :" << std::endl;
        std::cout << "- Oreille de gobelin : peut être trouée, brûlée ou propre selon le combat ;" << std::endl;
        std::cout << "- Peau de bête robuste : utile aux kits et armures ;" << std::endl;
        std::cout << "- Braise kitsune : composant magique sensible aux invocations ;" << std::endl;
        std::cout << "- Noyau instable : puissant mais dangereux pour l'alchimie et les expériences." << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: displayCraftJournal declares or implements a focused behavior used by this module.
    // FR: displayCraftJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displayCraftJournal()
    {
        std::cout << "========== JOURNAL DU CRAFT ==========" << std::endl;
        std::cout << "Ce journal suit les fabrications réussies pendant la session actuelle." << std::endl;
        std::cout << "Il servira plus tard aux compétences passives/actives d'artisanat, d'alchimie, de forge et d'armes utilisées." << std::endl;
        std::cout << std::endl;
        std::cout << "Total fabriqué cette session : " << MaterialExperimentLog::getTotalCrafted() << std::endl;

        const std::vector<CraftExperimentRecord>& records = MaterialExperimentLog::getCraftRecords();

        if (records.empty())
        {
            std::cout << "Aucun craft suivi pour le moment." << std::endl;
        }
        else
        {
            for (const CraftExperimentRecord& record : records)
            {
                std::cout << "- " << record.recipeName << " : " << record.craftedCount << " fabrication(s)" << std::endl;
            }
        }

        std::cout << std::endl;
        std::cout << "Rappel : les matériaux exceptionnels majoritaires peuvent créer une particularité faible." << std::endl;
        std::cout << "Forgeron, Alchimiste et Artificier ont maintenant un léger avantage économique et artisanal." << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: displaySummonJournal declares or implements a focused behavior used by this module.
    // FR: displaySummonJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displaySummonJournal()
    {
        std::cout << "========== JOURNAL DES INVOCATIONS ==========" << std::endl;
        std::cout << "Les invocations utilisent maintenant une base plus claire : slots, maintien, durée, contrôle manuel et sacrifice." << std::endl;
        std::cout << std::endl;
        std::cout << "Systèmes actifs :" << std::endl;
        std::cout << "- slots d'invocation : certaines invocations lourdes occupent plus d'un slot ;" << std::endl;
        std::cout << "- maintien : une invocation peut prolonger son lien au lieu d'attaquer ;" << std::endl;
        std::cout << "- sacrifice : une invocation peut rompre son lien pour infliger une rupture ;" << std::endl;
        std::cout << "- ciblage : les ennemis peuvent attaquer les invocations selon leur logique ;" << std::endl;
        std::cout << "- contrôle : automatique ou manuel selon le choix du joueur." << std::endl;
        std::cout << std::endl;
        std::cout << "À venir : mana complet, sacrifices avancés, ombres de Hazak après vrais kills, IA alliée et slots de groupe complets." << std::endl;
        std::cout << "=============================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: displayInformationShopPreview declares or implements a focused behavior used by this module.
    // FR: displayInformationShopPreview déclare ou implémente un comportement précis utilisé par ce module.
    void displayInformationShopPreview()
    {
        std::cout << "========== ACHAT D'INFORMATIONS ==========" << std::endl;
        std::cout << "La bibliothèque débloque maintenant des renseignements qui montent le niveau de connaissance du bestiaire." << std::endl;
        std::cout << "Les secrets, les boss et les noms importants devront toujours se mériter." << std::endl;
        std::cout << std::endl;
        std::cout << "Un nom de boss ne sera pas donné gratuitement si personne ne l'a prononcé." << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << std::endl;
    }
}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
void BestiaryMenu::open()
{
    while (true)
    {
        std::cout << "================ BESTIAIRE ================" << std::endl;
        std::cout << "Objet spécial de base : Bestiaire" << std::endl;
        std::cout << "Version actuelle : catégories propres, entrées ???, niveaux de connaissance et progression sauvegardable." << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Tout voir" << std::endl;
        std::cout << "2 : Races" << std::endl;
        std::cout << "3 : Entités hostiles / ennemis" << std::endl;
        std::cout << "4 : Entités passives / alliées" << std::endl;
        std::cout << "5 : Invocations" << std::endl;
        std::cout << "6 : Boss" << std::endl;
        std::cout << "7 : Personnages spéciaux" << std::endl;
        std::cout << "8 : Matériaux et plantes" << std::endl;
        std::cout << "9 : Divinités / lore" << std::endl;
        std::cout << "10 : Objets rares" << std::endl;
        std::cout << "11 : Journal des matériaux" << std::endl;
        std::cout << "12 : Journal des invocations" << std::endl;
        std::cout << "13 : Acheter des informations communes" << std::endl;
        std::cout << "14 : Journal du craft" << std::endl;
        std::cout << "===========================================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(
            0,
            14,
            "Choix invalide. Entre un chiffre entre 0 et 14."
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
        else if (choice == 11) displayMaterialJournal();
        else if (choice == 12) displaySummonJournal();
        else if (choice == 13) displayInformationShopPreview();
        else if (choice == 14) displayCraftJournal();
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

    std::cout << "========== BESTIAIRE ==========" << std::endl;
    std::cout << "Recherche : " << entryName << std::endl;
    std::cout << std::endl;
    std::cout << "Entrée inconnue ou pas encore découverte." << std::endl;
    std::cout << "Si ton personnage découvre cette chose plus tard, le bestiaire pourra afficher son nom, sa description, ses rencontres et ses tués." << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}
