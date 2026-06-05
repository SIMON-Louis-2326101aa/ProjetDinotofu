// EN: StoryCampaign.cpp implements the first structured story-mode data.
// FR: StoryCampaign.cpp implémente les premières données structurées du mode histoire.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "story/StoryCampaign.hpp"

#include "character/SpecialCharacterCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>
#include <vector>

namespace
{
    std::string normalizeStoryIdentityName(const std::string& name)
    {
        std::string normalized = name;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        const std::vector<std::pair<std::string, std::string>> replacements = {
            {"é", "e"}, {"è", "e"}, {"ê", "e"}, {"ë", "e"},
            {"à", "a"}, {"â", "a"}, {"ä", "a"},
            {"î", "i"}, {"ï", "i"},
            {"ô", "o"}, {"ö", "o"},
            {"ù", "u"}, {"û", "u"}, {"ü", "u"},
            {"ç", "c"}
        };

        for (const auto& replacement : replacements)
        {
            std::string::size_type position = 0;
            while ((position = normalized.find(replacement.first, position)) != std::string::npos)
            {
                normalized.replace(position, replacement.first.size(), replacement.second);
                position += replacement.second.size();
            }
        }

        std::string compact;
        for (unsigned char c : normalized)
        {
            if (std::isalnum(c))
            {
                compact += static_cast<char>(c);
            }
        }
        return compact;
    }

    bool getPlayableStorySpecialIdentity(const Player& player, SpecialCharacter& character)
    {
        if (!SpecialCharacterCatalog::findByName(player.getName(), character))
        {
            return false;
        }

        return character.canBePlayedWithSpecialDate() && !character.isPermanentlyNonPlayable();
    }

    std::string specialStoryRoleLine(const SpecialCharacter& character)
    {
        return character.getName() + " — " + character.getRaceText() + ", " + character.getNativeClass() + " : " + character.getCombatStyle();
    }

    std::vector<std::string> specialStoryHookLines(const Player& player)
    {
        SpecialCharacter character;
        if (!getPlayableStorySpecialIdentity(player, character))
        {
            return {};
        }

        const std::string key = normalizeStoryIdentityName(character.getName());

        if (key == "hazak")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Hazak ne marchait pas comme un simple volontaire de guilde : il avançait en surveillant déjà les angles morts, les ombres et les sorties possibles.",
                "La fumée blanche n'efface pas seulement une mission chez lui. Elle tente d'arracher une discipline d'assassin, des réflexes de survie et des noms qui devraient peser plus lourd que le silence.",
                "Même sans souvenir fiable, Hazak garde une sensation froide : s'il a été séparé du groupe, ce n'est peut-être pas par hasard."
            };
        }
        if (key == "mattzelda")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Mattzelda portait naturellement les charges que les autres trouvaient trop lourdes, comme si le monde testait toujours sa solidité.",
                "Quand la brume le prive de son équipement, elle ne lui retire pas sa présence : même vide, son corps se souvient qu'il peut faire mur.",
                "Le prologue de Mattzelda doit donc moins parler d'un héros perdu que d'un colosse obligé de survivre sans son poids habituel."
            };
        }
        if (key == "aoi")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Aoi sentait déjà les odeurs de feuilles mouillées, de métal froid et de magie trop nerveuse pour être naturelle.",
                "La brume avale son sac et sa lame, mais elle n'avale pas complètement la chaleur kitsune qui cherche à répondre sous la peau.",
                "Dans son prologue, la forêt blanche n'est pas seulement froide : elle devient une cage qui refuse que la flamme trouve un nom."
            };
        }
        if (key == "kanade")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Kanadé avançait avec cette impression désagréable que le ciel avait toujours un avis sur elle.",
                "La fumée ne fait pas taire son sang semi-draconique. Elle brouille seulement les signes, les constellations et les colères qui auraient pu l'aider à comprendre.",
                "Son prologue garde donc une tension particulière : même perdue, Kanadé n'est jamais totalement seule avec son propre ciel."
            };
        }
        if (key == "fail")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Fail traitait déjà les plans comme des propositions approximatives, ce qui inquiétait énormément les gens sérieux.",
                "La fumée blanche a beau essayer de tout ranger dans un silence parfait, elle tombe sur une magie de fée qui a rarement respecté les formes propres.",
                "Son prologue doit donc garder un côté instable : peur réelle, danger réel, mais impression que la brume elle-même n'est pas sûre de savoir comment le classer."
            };
        }
        if (key == "trexof")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Trexof observait déjà les limites du monde comme quelqu'un qui entend une fausse note dans une règle trop propre.",
                "La brume tente de le réduire à un survivant ordinaire, mais il garde un réflexe dangereux : analyser la faille au lieu de seulement la subir.",
                "Son prologue doit donner l'impression d'un survivant qui remarque les fausses notes avant les autres : pas invincible, mais immédiatement méfiant face à ce que le monde essaie de cacher."
            };
        }
        if (key == "skuro")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Skuro n'avait pas besoin de beaucoup parler pour que les autres sachent où il se trouvait : là où l'impact risquait d'être le plus brutal.",
                "Sans son arme lourde, il reste une menace incomplète, frustrée, presque silencieuse. La brume lui retire l'outil, pas l'envie de casser ce qui bloque la route.",
                "Son prologue prend donc la forme d'une retenue forcée : survivre sans pouvoir simplement frapper plus fort que le problème."
            };
        }
        if (key == "sanctus")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Sanctus portait déjà quelque chose de plus lourd qu'une armure : une croyance, une protection, une façon de juger le danger.",
                "Quand le blanc efface la route, il ne peut plus dire clairement quelle lumière il suivait. Il sait seulement qu'abandonner les autres sonnerait faux.",
                "Son prologue doit donc garder une blessure spirituelle : la foi existe encore, mais sa direction a été arrachée."
            };
        }
        if (key == "hestia")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Hestia semblait presque trop fragile pour la route, jusqu'au moment où la magie autour d'elle commençait à apprendre plus vite que prévu.",
                "La brume blanche ne sait pas quoi faire de cette fragilité étrange : elle retire les souvenirs, mais laisse une protection instinctive, comme un dôme sans nom.",
                "Son prologue doit donc être plus doux et plus inquiétant : Hestia survit parce que quelque chose en elle refuse de la laisser disparaître."
            };
        }
        if (key == "louis")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Louis vérifiait déjà trop de détails : la trajectoire d'un projectile, l'état d'un mécanisme, la place des alliés dans le chaos.",
                "La brume lui vole ses outils et ses repères, mais pas ce besoin presque naïf de chercher les autres avant de penser à lui-même.",
                "Son prologue garde donc une question centrale : qui devait-il protéger quand la fumée a séparé le groupe ?"
            };
        }
        if (key == "henrique")
        {
            return {
                "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
                "Avant la fumée, Henrique avançait comme quelqu'un qui considère tomber comme une formalité, pas comme une conclusion.",
                "La forêt blanche peut lui prendre sa route et ses souvenirs ; elle a plus de mal à lui prendre l'idée simple qu'il faut se relever.",
                "Son prologue doit donc insister sur l'obstination : Henrique ne sait plus pourquoi il était là, mais il refuse déjà que ce soit la fin."
            };
        }

        return {
            "Version personnalisée — personnage spécial jouable : " + specialStoryRoleLine(character),
            character.getName() + " n'entre pas dans l'histoire comme un personnage neutre.",
            "La fumée blanche efface les souvenirs, mais elle ne remplace pas complètement la race, la classe native, les réflexes et les cicatrices déjà inscrites dans cette identité.",
            "Le prologue garde donc le même squelette histoire, avec des lignes adaptées à ce personnage spécial."
        };
    }

    std::vector<std::string> specialMemoryLossAnchorLines(const Player& player)
    {
        SpecialCharacter character;
        if (!getPlayableStorySpecialIdentity(player, character))
        {
            return {};
        }

        const std::string key = normalizeStoryIdentityName(character.getName());
        if (key == "hazak")
        {
            return {"La fumée tente d'effacer la manière dont Hazak écoute les pas derrière lui. Elle échoue à moitié : le danger reste plus lisible que les souvenirs."};
        }
        if (key == "aoi")
        {
            return {"Une chaleur kitsune pulse une fois sous la peau d'Aoi. Pas assez pour brûler la brume. Assez pour prouver que tout n'a pas disparu."};
        }
        if (key == "kanade")
        {
            return {"Quelque chose au-dessus de Kanadé bouge derrière le blanc, comme une constellation qui refuse de donner son nom."};
        }
        if (key == "hestia")
        {
            return {"Autour d'Hestia, l'air forme presque un cercle protecteur avant de se briser. Même la fumée semble hésiter une seconde."};
        }
        if (key == "louis")
        {
            return {"Louis cherche ses outils par réflexe avant même de comprendre qu'ils ne sont plus là. Ses mains se souviennent mieux que sa tête."};
        }
        if (key == "trexof")
        {
            return {"Trexof remarque une répétition impossible dans les arbres. La mémoire tombe, mais le réflexe de reconnaître une fausse note reste accroché."};
        }
        if (key == "sanctus")
        {
            return {"Sanctus cherche une lumière précise dans la fumée. Il ne la retrouve pas, mais l'absence elle-même ressemble à un jugement."};
        }
        if (key == "skuro")
        {
            return {"Skuro serre une arme qui n'est plus là. La brume a pris le poids de la lame, pas le mouvement de la frappe."};
        }
        if (key == "mattzelda")
        {
            return {"Même sans équipement, Mattzelda garde une présence lourde dans la brume. Le sol semble se souvenir de lui avant qu'il ne se souvienne du chemin."};
        }
        if (key == "fail")
        {
            return {"Une étincelle de magie fée fait un détour absurde dans le blanc. Fail ne sait plus pourquoi, mais même la brume paraît contrariée."};
        }
        if (key == "henrique")
        {
            return {"Henrique oublie la route, puis l'ordre exact de la mission. Il n'oublie pas l'idée très simple de se relever."};
        }

        return {"Quelque chose dans l'identité spéciale de " + character.getName() + " résiste à la fumée : pas les souvenirs, mais les réflexes profonds."};
    }

    std::vector<std::string> specialArrivalLines(const Player& player)
    {
        SpecialCharacter character;
        if (!getPlayableStorySpecialIdentity(player, character))
        {
            return {};
        }

        return {
            "Le garde plisse les yeux en entendant ton nom. Il ne sait pas ce qu'il reconnaît : une rumeur, une race rare, une classe trop précise, ou seulement le ton d'une histoire qui a déjà commencé ailleurs.",
            "Le registre note ton identité comme cas spécial : " + specialStoryRoleLine(character) + ".",
            "Mira devra te traiter comme survivant de la fumée, mais aussi comme quelqu'un dont le passé risque de revenir avec plus de bruit qu'un simple nom perdu."
        };
    }

    std::vector<std::string> specialMiraLines(const Player& player)
    {
        SpecialCharacter character;
        if (!getPlayableStorySpecialIdentity(player, character))
        {
            return {};
        }

        return {
            "Mira baisse les yeux sur la ligne du registre où ton nom vient d'être écrit.",
            "Mira : « Les personnages comme " + character.getName() + " ne traversent pas une fumée blanche sans laisser de dettes derrière eux. Je ne sais pas encore lesquelles, donc on commence petit. »",
            "Mira : « Ta race, ta classe ou ta réputation ne suffiront pas à ouvrir les portes. Ici, même un nom spécial doit d'abord aider la ville à tenir. »"
        };
    }

    std::vector<std::string> specialChoiceLines(const Player& player, int choice, const std::map<std::string, std::vector<std::string>>& byKey)
    {
        SpecialCharacter character;
        if (!getPlayableStorySpecialIdentity(player, character))
        {
            return {};
        }

        const std::string key = normalizeStoryIdentityName(character.getName());
        const std::string lookup = key + ":" + std::to_string(choice);
        auto found = byKey.find(lookup);
        if (found != byKey.end())
        {
            return found->second;
        }

        auto generic = byKey.find("generic:" + std::to_string(choice));
        if (generic != byKey.end())
        {
            return generic->second;
        }

        return {};
    }

    std::vector<std::string> specialChapterTwoLinesByKey(const Player& player, const std::map<std::string, std::vector<std::string>>& byKey)
    {
        SpecialCharacter character;
        if (!getPlayableStorySpecialIdentity(player, character))
        {
            return {};
        }

        const std::string key = normalizeStoryIdentityName(character.getName());
        auto found = byKey.find(key);
        if (found != byKey.end())
        {
            return found->second;
        }

        auto generic = byKey.find("generic");
        if (generic != byKey.end())
        {
            return generic->second;
        }

        return {};
    }

    bool hasDefeatedBoss(const Player& player, int bossId)
    {
        const std::vector<int>& defeated = player.getDefeatedBossIds();
        return std::find(defeated.begin(), defeated.end(), bossId) != defeated.end();
    }

    int countTurnedInStorySupportQuests(const Player& player)
    {
        int count = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.turnedIn)
            {
                continue;
            }

            const std::string merged = quest.title + " " + quest.objective + " " + quest.location + " " + quest.objectiveType;
            const std::string lower = [&]() {
                std::string value = merged;
                std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return value;
            }();

            if (lower.find("chasse") != std::string::npos
                || lower.find("trace") != std::string::npos
                || lower.find("route") != std::string::npos
                || lower.find("sentier") != std::string::npos
                || lower.find("exploration") != std::string::npos
                || lower.find("mat") != std::string::npos
                || lower.find("nuisible") != std::string::npos
                || lower.find("patrouille") != std::string::npos)
            {
                ++count;
            }
        }
        return count;
    }

    int countTurnedInQuests(const Player& player)
    {
        int count = 0;
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.turnedIn)
            {
                ++count;
            }
        }
        return count;
    }

    bool hasTurnedInStoryQuest(const Player& player, const std::string& questId)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id == questId && quest.turnedIn && !quest.failed)
            {
                return true;
            }
        }
        return false;
    }

    int countTurnedInChapterOneMainRequests(const Player& player)
    {
        int count = 0;
        const std::vector<std::string> ids = {
            "story_ch1_mira_main",
            "story_ch1_orren_main",
            "story_ch1_lysa_main",
            "story_ch1_bram_main",
            "story_ch1_soryn_main"
        };

        for (const std::string& id : ids)
        {
            if (hasTurnedInStoryQuest(player, id))
            {
                ++count;
            }
        }
        return count;
    }

    int countTurnedInChapterTwoMainRequests(const Player& player)
    {
        int count = 0;
        const std::vector<std::string> ids = {
            "story_ch2_relay_briefing",
            "story_ch2_north_road_scout",
            "story_ch2_turned_marker",
            "story_ch2_relay_threat",
            "story_ch2_relay_signal",
            "story_ch2_first_rescue",
            "story_ch2_route_sack",
            "story_ch2_city_recovery",
            "story_ch2_cold_ink_trail",
            "story_ch2_route_rewrite",
            "story_ch2_short_route_counter",
            "story_ch2_black_knot_warning",
            "story_ch2_repair_downtime",
            "story_ch2_hidden_guardian_hint",
            "story_ch2_black_knot_seal",
            "story_ch2_black_knot_scars",
            "story_ch2_guarded_route"
        };

        for (const std::string& id : ids)
        {
            if (hasTurnedInStoryQuest(player, id))
            {
                ++count;
            }
        }
        return count;
    }

    std::string storyQuestShortState(const Player& player, const std::string& questId)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id != questId)
            {
                continue;
            }
            if (quest.failed) return "bloquée";
            if (quest.turnedIn) return "validée";
            if (quest.completed) return "à rendre";
            if (quest.accepted) return "en cours " + std::to_string(quest.progress) + "/" + std::to_string(quest.target);
            return "connue";
        }
        return "à débloquer";
    }

    std::string okMissing(bool value)
    {
        return value ? "OK" : "manquant";
    }

    std::string yesNo(bool value)
    {
        return value ? "oui" : "non";
    }
}

int StoryCampaign::suggestedChapterFromProgress(const Player& player)
{
    if (player.hasStorySkip())
    {
        return 99;
    }

    if (hasDefeatedBoss(player, 1) || player.getBossesKilled() >= 1 || player.getEnemiesKilled() >= 12 || player.getLevel() >= 4)
    {
        return 2;
    }

    if (player.hasStoryModeStarted() || player.getEnemiesKilled() >= 3 || player.getLevel() >= 2)
    {
        return 1;
    }

    return 0;
}

int StoryCampaign::suggestedCityDevelopmentFromProgress(const Player& player)
{
    int development = 0;
    if (player.hasStoryModeStarted()) development = std::max(development, 1);
    if (player.getEnemiesKilled() >= 8 || player.getLevel() >= 3) development = std::max(development, 2);
    if (hasDefeatedBoss(player, 1) || player.getBossesKilled() >= 1) development = std::max(development, 3);
    if (player.getEnemiesKilled() >= 25 || player.getLevel() >= 7) development = std::max(development, 4);
    if (player.hasStorySkip()) development = std::max(development, 9);
    return development;
}

bool StoryCampaign::canUnlockChapterTwo(const Player& player)
{
    if (player.hasStorySkip() || player.getStoryChapter() >= 2)
    {
        return true;
    }

    return StoryCampaign::canCompleteChapterOne(player);
}

int StoryCampaign::maxUnlockedChapter(const Player& player)
{
    if (player.hasStorySkip())
    {
        return 99;
    }

    int unlocked = 0;
    if (player.hasStoryModeStarted() || player.getStoryChapter() > 0 || player.getStoryStep() > 0)
    {
        unlocked = 1;
    }

    if (canUnlockChapterTwo(player) || player.getStoryChapter() >= 2)
    {
        unlocked = std::max(unlocked, 2);
    }

    return std::max(unlocked, player.getStoryChapter());
}

bool StoryCampaign::isChapterUnlocked(const Player& player, int chapter)
{
    if (chapter <= 0)
    {
        return false;
    }
    return maxUnlockedChapter(player) >= chapter;
}

StoryAccessSnapshot StoryCampaign::buildAccessSnapshot(const Player& player)
{
    StoryAccessSnapshot snapshot;
    snapshot.chapter = player.getStoryChapter();
    snapshot.step = player.getStoryStep();
    snapshot.cityDevelopment = player.getStoryCityDevelopmentLevel();

    if (snapshot.chapter <= 0)
    {
        snapshot.unlockedSystems = {
            "Bac à sable : tous les menus libres restent disponibles hors histoire.",
            "Mode histoire : prologue, quartier de départ, première route courte.",
            "Boutiques histoire : inventaire très limité, prix simples, peu de choix."
        };
        snapshot.limitedSystems = {
            "Guilde : quelques contrats F/E seulement dans la logique histoire.",
            "Exploration : abords de la ville, champs proches, pont court.",
            "Bestiaire : beaucoup d'entrées restent ??? dans la route histoire."
        };
        snapshot.lockedSystems = {
            "Boss avancés, grandes boutiques, quartiers riches, arènes spéciales.",
            "Routes lointaines, contrats de rang élevé, événements royaux.",
            "Sources profondes des monstres et vraie menace finale."
        };
        snapshot.mainObjectives = {
            "Se réveiller dans une ville qui manque de tout.",
            "Comprendre pourquoi les monstres reviennent trop près des murs.",
            "Trouver un premier référent fiable au lieu de courir partout."
        };
    }
    else if (snapshot.chapter == 1)
    {
        snapshot.unlockedSystems = {
            "Guilde locale très limitée : petits contrats de route, nuisibles, livraisons simples.",
            "Forge de fortune : réparation basique, pas encore grand artisanat.",
            "Herboristerie pauvre : potions simples et rations."
        };
        snapshot.limitedSystems = {
            "Exploration : sentier court, grange, vieux pont, lisière de bois.",
            "Boutiques : stocks faibles tant que les matériaux ne reviennent pas.",
            "Demandes de PNJ : elles prennent sens après avoir rencontré les premiers visages de la ville."
        };
        snapshot.lockedSystems = {
            "Boss hors ordre, grands biomes, quartiers de prestige, grosses routes marchandes.",
            "PNJ importants absents ou bloqués par la crise.",
            "Contrats de guilde au-dessus du rang local."
        };
        snapshot.mainObjectives = {
            "Nettoyer les menaces proches pour que la ville respire.",
            "Rapporter matériaux et preuves pour débloquer un vrai comptoir de guilde.",
            "Préparer la recherche d'un PNJ référent disparu au relais."
        };
    }
    else
    {
        snapshot.unlockedSystems = {
            "Chapitre 2 : route du relais silencieux, première vraie enquête, menace imposée, relais stabilisé, premier sauvetage et sacoche de routes.",
            "Développement de ville : demandes liées aux PNJ présentés, stocks moins ridicules, premières vraies priorités de forge/herboristerie et comptoirs relancés.",
            "Route du nord : après le signal, le joueur sauve Nell, exploite ses informations, suit l'encre froide et commence à comprendre que la route est réécrite.",
            "Économie histoire légère : les boutiques affichent maintenant pourquoi leurs stocks respirent ou restent pauvres selon la progression, avec un contre-registre de routes courtes.",
            "Temps de réparation : certaines étapes demandent maintenant de s'occuper utilement avec patrouilles, services et quêtes secondaires avant que la suite ne soit prête.",
            "Borne noire : la ville peut maintenant passer d'une rumeur de menace à un premier affrontement d'étape contre le verrou qui garde la route.",
            "Combat histoire : menaces imposées dans l'ordre, pas comme le bac à sable."
        };
        snapshot.limitedSystems = {
            "Boutiques : restent dépendantes des matériaux et des PNJ sauvés.",
            "Guilde : plus vivante, mais certaines branches se rebloquent si la route ou le relais tombe.",
            "Exploration : nouvelles zones, mais encore fermées si la ville n'a pas les ressources."
        };
        snapshot.lockedSystems = {
            "Grandes intrigues divines, vraie Source de l'Anomalie, FireFlight et fin globale.",
            "Légendes complètes : révélées progressivement par bibliothèque, PNJ et salles de boss.",
            "Routes de haut rang tant que l'histoire n'a pas justifié leur ouverture."
        };
        snapshot.mainObjectives = {
            "Stabiliser le relais avant que les routes ne se taisent complètement.",
            "Battre la première menace imposée par l'histoire, dans l'ordre prévu : les guetteurs sans feu.",
            "Faire revenir la première voix de la route : Nell la messagère, coincée derrière les caisses d'un convoi brisé.",
            "Exploiter la sacoche de Nell pour transformer le sauvetage en vraie piste de route.",
            "Faire réagir les comptoirs de la ville pour que les informations sauvées aient une conséquence concrète.",
            "Suivre l'encre froide de la Route commerciale afin de préparer la prochaine crise.",
            "Identifier ce qui réécrit les routes, puis installer un contre-registre pour que les comptoirs ne suivent plus les cartes corrompues.",
            "Repérer le nœud noir de la route avant d'en faire une vraie crise de chapitre.",
            "Occuper le temps des réparations avec des patrouilles et demandes secondaires utiles pour renforcer le personnage avant les sorties dangereuses.",
            "Comprendre qu'une présence connue par rumeur garde la borne noire, sans encore dévoiler son vrai nom.",
            "Briser le premier verrou de la borne noire sans encore révéler toute l'identité de ce qui contrôle la route.",
            "Lire les cicatrices du verrou pour transformer le combat gagné en preuves utiles.",
            "Organiser une route gardée afin que les comptoirs respirent sans croire que le danger a disparu.",
            "Utiliser les demandes des PNJ pour reconstruire : nourriture, forge, routes, soins, confiance."
        };
    }

    snapshot.sideObjectives = {
        "Tuer des monstres proches pour réduire la pression et récolter des matériaux.",
        "Parler d’abord aux référents, puis accepter des demandes proches quand la mission principale demande du développement.",
        "Aider certains clients amis déjà présentés afin qu'ils deviennent de vrais référents utiles.",
        "Inspecter le bestiaire et les matériaux pour comprendre d'où viennent les créatures."
    };

    snapshot.referentNpcs = {
        "Mira (intendante de quartier) : explique ce qui manque et ce qui débloque la ville.",
        "Orren (vieux garde / référent de route) : lit les chemins, les ponts et les disparitions.",
        "Lysa (soigneuse de fortune) : suit les blessés, les potions et les symptômes étranges.",
        "Bram (forgeron fatigué) : répare peu au début, puis dépend des matériaux rapportés.",
        "Soryn (archiviste) : raconte les légendes seulement quand le joueur a de quoi comprendre.",
        "Nell la messagère (survivante de route) : apparaît après le relais ; sa sacoche transforme le sauvetage en cartes, bons de convoi et piste d'encre froide.",
        "Eda (comptable des routes courtes) : devient un vrai contact de ville ; elle suit les retours réels, les stocks confirmés et les temps de réparation."
    };

    snapshot.intrigueThreads = {
        "Les monstres ne naissent pas tous naturellement : certains semblent poussés par une fissure du monde.",
        "La ville se développe parce que le joueur sécurise routes, matériaux et personnes clés.",
        "Certaines zones se rebloquent si une crise détruit une route, enlève un PNJ ou coupe un stock.",
        "L'encre froide ne copie pas seulement des noms : elle réécrit des chemins comme un registre tenu par une main invisible.",
        "Les boss de l'histoire doivent être affrontés dans un ordre narratif, même si le bac à sable reste libre.",
        "Certaines quêtes peuvent annoncer qu'un boss ou gardien doit tomber sans donner tout de suite son vrai nom, parce qu'il est connu par rumeurs avant d'être identifié.",
        "Le but final ne doit pas être nommé au début : il se révèle par traces, légendes, anomalies et invitations."
    };

    return snapshot;
}

std::vector<std::string> StoryCampaign::buildLongIntroductionLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Avant la ville, avant la fumée blanche, il y avait une mission presque banale.",
        "Pas une légende. Pas une prophétie. Juste une sortie de guilde comme on en donne à des gens qui savent tenir une arme et rapporter des matériaux.",
        "Tu étais avec d'autres. Des voix familières, des sacs trop lourds, des plaisanteries de route, une promesse simple : exterminer quelques nuisibles, récolter ce qui pouvait servir, rentrer avant que le soir ne rende la forêt stupide.",
        "La mission devait être courte. Les traces étaient fraîches, les monstres signalés n'avaient rien d'un désastre, et personne n'avait demandé à la ville de fermer ses portes derrière vous.",
        "Puis les oiseaux se sont tus en même temps.",
        "Le chemin a changé de pente. Les marques de passage se sont répétées sur trois arbres différents. Un ami a juré qu'il avait déjà vu cette souche. Un autre a ri trop fort pour faire semblant de ne pas avoir peur."
    };

    const std::vector<std::string> specialLines = specialStoryHookLines(player);
    lines.insert(lines.end(), specialLines.begin(), specialLines.end());

    const std::vector<std::string> endLines = {
        "La fumée blanche est arrivée sans bruit. Pas comme du brouillard. Comme une chose qui savait exactement qui elle venait chercher.",
        "Elle n'a pas seulement séparé le groupe. Elle a commencé à retirer les liens entre les souvenirs : les noms des autres, leurs visages, la raison de la mission, puis la certitude même d'avoir marché avec eux.",
        "Tu te souviens de mains qui attrapent ton bras. Tu te souviens d'un cri qui aurait dû être important. Tu te souviens d'avoir voulu répondre. Ensuite, le blanc.",
        "Quand le joueur reprend vraiment la main, il ne reste plus que la forêt, la fumée et un prénom : " + player.getName() + ".",
        "Le personnage ne commence pas héros. Il commence survivant, amputé de son propre passé, avec l'impression qu'il vient d'abandonner quelqu'un sans savoir qui.",
        "Sans équipement, sans groupe, sans carte fiable, il doit sortir de la forêt. Si quelque chose attaque, il ne s'agit pas de gagner : seulement parer ou fuir.",
        "La sortie mène à un cadavre et à un paquet abîmé. Ce kit de départ n'est pas un cadeau héroïque : c'est ce qui reste d'une personne qui n'a pas eu la même chance.",
        "Au loin, une ville tient à peine debout. Les murs sont trop bas pour rassurer. Les lanternes sont accrochées trop près du sol. Les marchands parlent doucement, comme si les choses dehors pouvaient comprendre les prix.",
        "La guilde existe, mais elle n'a rien d'une grande institution. Pour l'instant, c'est un comptoir, deux bancs, un registre humide et des gens qui font semblant de savoir quoi faire.",
        "Ici, rien ne s'ouvre par simple décision de registre. Les choses s'ouvrent parce que quelqu'un est sauvé, parce qu'une route tient encore, parce qu'une menace tombe, parce que des matériaux reviennent vivants jusqu'aux portes.",
        "Les monstres ne sont pas seulement des obstacles. Certains ont l'air d'avoir fui quelque chose. D'autres semblent avoir été fabriqués par une peur plus ancienne que la ville.",
        "Le but final ne sera pas donné au début. Il commencera petit : retrouver son nom complet, comprendre la mission perdue, rouvrir une route, sauver un référent, puis découvrir contre quoi le monde se défend vraiment."
    };
    lines.insert(lines.end(), endLines.begin(), endLines.end());
    return lines;
}

std::vector<std::string> StoryCampaign::buildWhiteFogPrologueLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Prologue — Mission ordinaire",
        "La journée avait commencé avec des voix autour de toi.",
        "Pas des voix de rêve. Des voix proches. Des gens avec qui tu marchais depuis assez longtemps pour reconnaître leurs pas sans les regarder.",
        "Quelqu'un râlait parce que les sacs de récolte étaient déjà humides. Quelqu'un d'autre comptait les fioles vides en disant que cette fois, il ne porterait pas les organes de slime pour tout le monde.",
        "La mission n'avait rien d'une grande bataille : nettoyer une zone de nuisance, récupérer des matériaux, confirmer que le sentier restait praticable, puis rentrer avant la fermeture des portes.",
        "Tu avais un équipement. Un sac. Une raison d'être là. Et surtout, tu n'étais pas seul.",
        "Une voix te lance : « " + player.getName() + ", tu fermes la marche ? »",
        "Tu réponds quelque chose. Le souvenir garde le mouvement de ta bouche, mais pas les mots exacts."
    };

    const std::vector<std::string> specialLines = specialStoryHookLines(player);
    lines.insert(lines.end(), specialLines.begin(), specialLines.end());

    const std::vector<std::string> endLines = {
        "Puis les traces deviennent étranges.",
        "Une empreinte de monstre s'arrête au milieu du chemin. Une corde de balise pend à une branche, blanche de givre alors qu'il ne fait pas froid. Les oiseaux se taisent tous en même temps.",
        "Un de tes compagnons murmure qu'il faut rentrer. Un autre dit que ce n'est qu'un brouillard de vallée. Personne ne croit vraiment cette phrase.",
        "La fumée blanche arrive entre deux arbres.",
        "Elle n'avance pas avec le vent. Elle avance contre lui.",
        "Quelqu'un attrape ton bras. Quelqu'un crie ton prénom. Ou peut-être le sien. Tu n'arrives déjà plus à distinguer.",
        "La mission habituelle se brise là, dans un silence trop propre.",
        "Quand tu ouvres les yeux pour de bon, tu es seul.",
        "Le monde autour de toi n'a presque plus de couleur. Les troncs sont noirs, le sol est humide, et tout le reste disparaît dans une fumée blanche qui respire comme une chose vivante.",
        "Tes mains sont vides. Pas d'arme. Pas de sac. Pas de signe clair de ce que tu étais avant cette forêt.",
        "Il reste pourtant un mot, planté au milieu du vide : " + player.getName() + ".",
        "Tu ne sais pas si c'est ton prénom, un avertissement, ou le dernier morceau de quelqu'un qui te connaissait.",
        "Dans cette séquence, aucun vrai combat n'est gagné. Si quelque chose attaque, il faudra parer assez longtemps pour respirer, ou fuir assez vite pour rester vivant.",
        "La sortie de forêt mènera au premier équipement, mais pas comme un cadeau. Ici, même les objets de départ auront une histoire."
    };
    lines.insert(lines.end(), endLines.begin(), endLines.end());
    return lines;
}

std::vector<std::string> StoryCampaign::buildWhiteFogMemoryLossLines(const Player& player)
{
    std::vector<std::string> lines = {
        "La fumée blanche avance.",
        "Elle touche d'abord les souvenirs qui semblaient solides : la mission, le nombre de compagnons, le chemin du retour.",
        "Tu essaies de compter les autres. Un. Deux. Trois... Le quatrième devient une forme sans visage. Le cinquième n'a plus de voix. Ou peut-être qu'il n'y en avait jamais eu cinq.",
        "Un rire disparaît. Une main sur ton épaule disparaît. La phrase « on rentre ensemble » se coupe avant le dernier mot.",
        "Puis la fumée devient plus précise.",
        "Ton âge se brouille. Ton origine se replie sur elle-même. Même ton ancienne façon de tenir une arme devient une image sans muscle."
    };

    const std::vector<std::string> specialLines = specialMemoryLossAnchorLines(player);
    lines.insert(lines.end(), specialLines.begin(), specialLines.end());

    const std::vector<std::string> endLines = {
        "Tu essaies de penser à ta famille, à ta maison, à la guilde qui vous a envoyés. La fumée répond par un blanc parfait.",
        "Il reste une seule chose assez lourde pour ne pas partir tout de suite : " + player.getName() + ".",
        player.getName() + "... Tu répètes ce mot dans ta tête jusqu'à ce qu'il ressemble à une corde au-dessus du vide.",
        player.getName() + " : « C'est moi... non ? Oui. Je crois. »",
        player.getName() + " : « J'étais avec... avec qui ? »",
        "La forêt ne répond pas. Elle garde les noms des autres comme si elle les avait avalés avant toi.",
        player.getName() + " : « Je dois sortir. Après, je me souviendrai. Peut-être. »",
        "Le prénom est réel. Le reste devra être reconstruit."
    };
    lines.insert(lines.end(), endLines.begin(), endLines.end());
    return lines;
}

std::vector<std::string> StoryCampaign::buildWhiteFogFirstReactionLines(const Player& player, int choice)
{
    static const std::map<std::string, std::vector<std::string>> linesByChoice = {
        {"hazak:1", {"Réflexe spécial — Hazak avance lentement, mais son corps choisit surtout les angles morts. La prudence ressemble moins à de la peur qu'à une chasse inversée."}},
        {"hazak:2", {"Réflexe spécial — Hazak court, puis corrige aussitôt sa trajectoire. La panique veut du bruit ; son instinct refuse de lui en donner."}},
        {"hazak:3", {"Réflexe spécial — Hazak appelle une seule fois. Quand la fumée répond avec sa voix, il sait déjà qu'il vient de repérer un piège."}},
        {"mattzelda:1", {"Réflexe spécial — Mattzelda avance sans se presser, comme si chaque pas devait vérifier que le sol peut encore porter quelqu'un."}},
        {"mattzelda:2", {"Réflexe spécial — Mattzelda court droit devant. La brume le ralentit, mais elle n'arrive pas à lui donner l'air fragile."}},
        {"mattzelda:3", {"Réflexe spécial — Mattzelda appelle assez fort pour faire vibrer les branches. La réponse copiée paraît plus petite que lui."}},
        {"aoi:1", {"Réflexe spécial — Aoi suit les arbres moins blancs en cherchant la moindre odeur de chaleur. Même perdue, elle ne marche jamais totalement seule avec le froid."}},
        {"aoi:2", {"Réflexe spécial — Aoi court ; une chaleur kitsune remonte dans ses jambes, aussitôt étouffée par le blanc qui colle aux feuilles."}},
        {"aoi:3", {"Réflexe spécial — Aoi appelle, puis sa voix tremble quand la fumée répond trop bien. Une flamme minuscule veut naître, sans encore trouver de nom."}},
        {"kanade:1", {"Réflexe spécial — Kanadé avance en guettant le ciel, même si la fumée a avalé les constellations. Elle déteste l'idée d'être observée sans pouvoir répondre."}},
        {"kanade:2", {"Réflexe spécial — Kanadé court comme si elle voulait distancer la forêt entière. Le ciel ne donne aucun signe, ce qui l'énerve presque plus que la peur."}},
        {"kanade:3", {"Réflexe spécial — Kanadé appelle ; la réponse copiée sonne faux, comme une étoile mal dessinée. Son sang semi-draconique gronde sans explication."}},
        {"fail:1", {"Réflexe spécial — Fail avance lentement et la fumée semble hésiter sur la forme du piège. Même perdue, sa magie refuse les lignes trop droites."}},
        {"fail:2", {"Réflexe spécial — Fail court, trébuche presque, puis évite une racine qu'il n'avait aucune raison d'avoir vue. La brume n'aime pas les accidents utiles."}},
        {"fail:3", {"Réflexe spécial — Fail appelle ; quand la fumée imite sa voix, l'écho paraît une demi-seconde vexé de ne pas être le plus étrange des deux."}},
        {"trexof:1", {"Réflexe spécial — Trexof avance lentement, mais compte les arbres qui se répètent. La peur existe ; l'analyse prend quand même la priorité."}},
        {"trexof:2", {"Réflexe spécial — Trexof court, puis remarque que la forêt corrige sa trajectoire comme une main trop sûre d'elle. Ça ne rassure pas. Ça confirme quelque chose."}},
        {"trexof:3", {"Réflexe spécial — Trexof appelle et écoute la copie de sa voix. Le délai de réponse est trop propre. Quelque chose imite mal la vie."}},
        {"skuro:1", {"Réflexe spécial — Skuro avance lentement avec une retenue presque violente. Sans arme lourde, chaque pas ressemble à une insulte qu'il garde pour plus tard."}},
        {"skuro:2", {"Réflexe spécial — Skuro court droit devant comme s'il voulait traverser la brume par impact. La forêt plie moins que prévu, mais elle a senti la menace."}},
        {"skuro:3", {"Réflexe spécial — Skuro appelle. Quand la fumée répond, ses mains cherchent déjà quoi briser dans la direction du mensonge."}},
        {"sanctus:1", {"Réflexe spécial — Sanctus avance lentement, cherchant une direction qui ressemble à une promesse. La lumière ne répond pas, mais l'abandon sonne faux."}},
        {"sanctus:2", {"Réflexe spécial — Sanctus court sans appeler ça une fuite. Il protège encore quelque chose, même s'il ne sait plus qui marchait derrière lui."}},
        {"sanctus:3", {"Réflexe spécial — Sanctus appelle avec une autorité blessée. La fumée copie le son, mais pas le poids du serment."}},
        {"hestia:1", {"Réflexe spécial — Hestia avance doucement. L'air autour d'elle se tend parfois comme un voile qui apprend encore à protéger."}},
        {"hestia:2", {"Réflexe spécial — Hestia court et manque presque de tomber. Une force sans nom la redresse avant que ses propres souvenirs ne comprennent pourquoi."}},
        {"hestia:3", {"Réflexe spécial — Hestia appelle d'une voix trop fragile pour cette forêt. La réponse copiée hésite, comme si la fumée n'osait pas la toucher trop fort."}},
        {"louis:1", {"Réflexe spécial — Louis avance lentement en cherchant des repères pratiques : pente, humidité, angle des branches. Ses souvenirs tombent, sa méthode reste."}},
        {"louis:2", {"Réflexe spécial — Louis court, puis regarde malgré lui où des alliés auraient dû passer. Même seul, il calcule encore une retraite de groupe."}},
        {"louis:3", {"Réflexe spécial — Louis appelle, puis s'arrête net devant l'écho. Une bonne imitation n'est pas un ami, et il le comprend trop vite."}},
        {"henrique:1", {"Réflexe spécial — Henrique avance lentement, mâchoire serrée. Il ne sait plus pourquoi il faut continuer, mais s'arrêter lui paraît déjà ridicule."}},
        {"henrique:2", {"Réflexe spécial — Henrique court comme s'il avait décidé que la forêt n'aurait pas le dernier mot. La brume gagne du terrain, pas l'argument."}},
        {"henrique:3", {"Réflexe spécial — Henrique appelle ; la copie répond. Il oublie presque la peur pour une idée simple : si quelque chose ment, on lui rentre dedans plus tard."}},
        {"generic:1", {"Réflexe spécial — ton identité résiste un peu à la brume : tu avances avec une façon de bouger que la fumée n'a pas réussi à effacer."}},
        {"generic:2", {"Réflexe spécial — la course réveille des instincts trop anciens pour être des souvenirs. La fumée prend la carte, pas le corps."}},
        {"generic:3", {"Réflexe spécial — quand la fumée répond avec ta voix, quelque chose en toi reconnaît le piège avant de retrouver le passé."}}
    };

    return specialChoiceLines(player, choice, linesByChoice);
}

std::vector<std::string> StoryCampaign::buildWhiteFogEncounterReactionLines(const Player& player, int choice)
{
    static const std::map<std::string, std::vector<std::string>> linesByChoice = {
        {"hazak:1", {"Réaction spéciale — Hazak encaisse le choc en tournant l'épaule. Même sans lame, son corps cherche déjà le point faible de ce qui l'attaque."}},
        {"hazak:2", {"Réaction spéciale — Hazak fuit sans tourner le dos plus longtemps que nécessaire. C'est une retraite, pas une panique."}},
        {"mattzelda:1", {"Réaction spéciale — Mattzelda bloque avec une lourdeur presque absurde. La créature comprend qu'il faudra plus qu'un premier impact."}},
        {"mattzelda:2", {"Réaction spéciale — Mattzelda fuit, mais chaque pas garde l'impression d'un mur qui se déplace plutôt que d'un homme qui cède."}},
        {"aoi:1", {"Réaction spéciale — Aoi pare et une chaleur sèche traverse brièvement ses avant-bras. Pas une vraie flamme. Un souvenir de flamme."}},
        {"aoi:2", {"Réaction spéciale — Aoi fuit entre les troncs avec une souplesse kitsune que la peur ne parvient pas à voler."}},
        {"kanade:1", {"Réaction spéciale — Kanadé pare, furieuse de ne pas pouvoir appeler ce qui gronde au-dessus d'elle. La colère tient presque lieu d'armure."}},
        {"kanade:2", {"Réaction spéciale — Kanadé fuit en jurant contre le ciel absent. Même la brume paraît éviter de répondre."}},
        {"fail:1", {"Réaction spéciale — Fail pare et une étincelle imprévisible détourne une partie du choc. Ce n'est pas propre, mais c'est vivant."}},
        {"fail:2", {"Réaction spéciale — Fail fuit par un angle improbable. La créature semble perdre une seconde à comprendre pourquoi cette trajectoire marche."}},
        {"trexof:1", {"Réaction spéciale — Trexof pare en mémorisant le rythme d'attaque. Il ne peut pas gagner maintenant, mais il classe déjà le danger."}},
        {"trexof:2", {"Réaction spéciale — Trexof fuit après avoir repéré le moment exact où la brume ralentit la poursuite. Ce détail reviendra sûrement plus tard."}},
        {"skuro:1", {"Réaction spéciale — Skuro pare trop brutalement pour quelqu'un sans arme. La créature recule autant par surprise que par impact."}},
        {"skuro:2", {"Réaction spéciale — Skuro fuit avec une colère dense. La forêt vient seulement de gagner du temps, pas le droit de se croire supérieure."}},
        {"sanctus:1", {"Réaction spéciale — Sanctus bloque en se plaçant comme s'il protégeait encore quelqu'un derrière lui. Même vide, le geste garde un sens."}},
        {"sanctus:2", {"Réaction spéciale — Sanctus fuit en gardant le corps entre le danger et une absence. Il ne sait plus qui manque. Il protège quand même."}},
        {"hestia:1", {"Réaction spéciale — Hestia lève les bras trop tard, mais l'air se durcit juste assez. Quelque chose en elle apprend la peur très vite."}},
        {"hestia:2", {"Réaction spéciale — Hestia fuit et le voile invisible autour d'elle se referme par réflexe. La brume n'a pas réussi à la prendre entièrement."}},
        {"louis:1", {"Réaction spéciale — Louis pare en calculant déjà comment il aurait couvert un allié. Le geste arrive sans l'allié, et ça fait presque plus mal que le choc."}},
        {"louis:2", {"Réaction spéciale — Louis fuit vers une ligne plus praticable, pas seulement loin du monstre. Même amnésique, il cherche une sortie utile."}},
        {"henrique:1", {"Réaction spéciale — Henrique pare, recule, puis reste debout par principe. La créature a frappé. Elle n'a pas convaincu."}},
        {"henrique:2", {"Réaction spéciale — Henrique fuit parce qu'il le faut, mais son regard promet déjà une revanche à quelque chose qui ne connaît même pas son nom."}},
        {"generic:1", {"Réaction spéciale — ton identité ancienne transforme la parade en geste presque naturel, malgré la mémoire arrachée."}},
        {"generic:2", {"Réaction spéciale — tu fuis avec des réflexes que tu ne sais plus expliquer. La brume n'a pas tout emporté."}}
    };

    return specialChoiceLines(player, choice, linesByChoice);
}

std::vector<std::string> StoryCampaign::buildWhiteFogKitReactionLines(const Player& player, int choice)
{
    static const std::map<std::string, std::vector<std::string>> linesByChoice = {
        {"hazak:1", {"Réaction spéciale — Hazak ferme les yeux du mort avec une précision silencieuse. Ce respect-là ne contredit pas la mort ; il la regarde en face."}},
        {"hazak:2", {"Réaction spéciale — Hazak prend seulement l'utile. La culpabilité viendra si elle survit à la route."}},
        {"hazak:3", {"Réaction spéciale — Hazak fouille vite, mais pas au hasard. Même pressé, il laisse moins de traces qu'un survivant ordinaire."}},
        {"mattzelda:1", {"Réaction spéciale — Mattzelda ferme les yeux du mort avec une douceur qui contraste presque trop avec sa carrure."}},
        {"mattzelda:2", {"Réaction spéciale — Mattzelda prend le nécessaire et le cale comme s'il portait déjà la prochaine porte sur ses épaules."}},
        {"mattzelda:3", {"Réaction spéciale — Mattzelda fouille vite, mais le paquet paraît soudain minuscule entre ses mains."}},
        {"aoi:1", {"Réaction spéciale — Aoi ferme les yeux du mort. Une chaleur très faible passe dans ses doigts, comme une prière que personne ne lui a apprise."}},
        {"aoi:2", {"Réaction spéciale — Aoi prend le nécessaire, puis s'excuse presque trop bas pour être entendue. Les feuilles autour d'elle cessent de trembler."}},
        {"aoi:3", {"Réaction spéciale — Aoi fouille vite. Sa queue ou son instinct kitsune cherche déjà une sortie avant que le cœur accepte la scène."}},
        {"kanade:1", {"Réaction spéciale — Kanadé ferme les yeux du mort avec mauvaise humeur, comme si le ciel aurait dû s'en charger et avait encore raté son travail."}},
        {"kanade:2", {"Réaction spéciale — Kanadé prend l'utile sans cérémonie. Survivre d'abord, donner un sens ensuite."}},
        {"kanade:3", {"Réaction spéciale — Kanadé fouille vite ; une colère sourde lui promet que ce cadavre ne sera pas juste un décor oublié."}},
        {"fail:1", {"Réaction spéciale — Fail ferme les yeux du mort avec un sérieux rare. Pendant une seconde, même sa magie arrête de faire semblant de rire."}},
        {"fail:2", {"Réaction spéciale — Fail prend le nécessaire et murmure qu'un plan viable commence parfois par du vol de survie très mal assumé."}},
        {"fail:3", {"Réaction spéciale — Fail fouille vite, trouve presque une logique dans le chaos, puis décide que ce n'est pas le moment d'en être fier."}},
        {"trexof:1", {"Réaction spéciale — Trexof ferme les yeux du mort, puis observe le paquet comme une pièce posée là par quelqu'un qui voulait provoquer un choix."}},
        {"trexof:2", {"Réaction spéciale — Trexof prend seulement l'utile et note mentalement ce que la forêt a choisi de laisser. Un cadeau trop propre n'est jamais neutre."}},
        {"trexof:3", {"Réaction spéciale — Trexof fouille vite, mais l'étrangeté du paquet le dérange presque autant que le cadavre."}},
        {"skuro:1", {"Réaction spéciale — Skuro ferme les yeux du mort avec une rudesse contrôlée. Il respecte mieux les corps que les obstacles."}},
        {"skuro:2", {"Réaction spéciale — Skuro prend le nécessaire comme on arrache une chance au sol. La forêt ne lui offrira rien gratuitement."}},
        {"skuro:3", {"Réaction spéciale — Skuro fouille vite. S'il avait son arme, la peur autour de ce fossé aurait probablement une forme plus cassée."}},
        {"sanctus:1", {"Réaction spéciale — Sanctus ferme les yeux du mort comme un rite incomplet. Il ne retrouve pas les mots sacrés, mais le geste suffit pour l'instant."}},
        {"sanctus:2", {"Réaction spéciale — Sanctus prend le nécessaire en silence. La survie n'efface pas la dette ; elle la reporte."}},
        {"sanctus:3", {"Réaction spéciale — Sanctus fouille vite, puis détourne les yeux comme si quelqu'un jugeait encore la scène au-dessus de lui."}},
        {"hestia:1", {"Réaction spéciale — Hestia ferme les yeux du mort et l'air devient doux une seconde. La protection en elle ne sait pas ressusciter. Elle sait seulement entourer."}},
        {"hestia:2", {"Réaction spéciale — Hestia prend le nécessaire avec des mains tremblantes. Le voile autour d'elle se resserre, honteux et protecteur."}},
        {"hestia:3", {"Réaction spéciale — Hestia fouille vite, trop vite pour son cœur. Pourtant quelque chose en elle refuse de la laisser s'effondrer ici."}},
        {"louis:1", {"Réaction spéciale — Louis ferme les yeux du mort, puis vérifie le paquet avec méthode. Il aurait voulu demander l'avis du groupe. Le silence répond."}},
        {"louis:2", {"Réaction spéciale — Louis prend l'utile et range chaque chose comme si une mauvaise organisation pouvait tuer quelqu'un plus tard."}},
        {"louis:3", {"Réaction spéciale — Louis fouille vite, mais ses mains trient quand même. Même la panique n'arrive pas à le rendre complètement brouillon."}},
        {"henrique:1", {"Réaction spéciale — Henrique ferme les yeux du mort avec une promesse simple : quelqu'un sortira de cette forêt debout."}},
        {"henrique:2", {"Réaction spéciale — Henrique prend le nécessaire sans se chercher d'excuse. Il faudra vivre assez longtemps pour mériter mieux."}},
        {"henrique:3", {"Réaction spéciale — Henrique fouille vite, puis se redresse comme si la honte elle-même venait de lui demander un duel."}},
        {"generic:1", {"Réaction spéciale — le respect du mort réveille une ancienne manière d'agir que ton nom seul n'explique pas encore."}},
        {"generic:2", {"Réaction spéciale — tu prends l'utile avec un instinct précis, comme si une ancienne vie connaissait déjà le prix d'une seconde chance."}},
        {"generic:3", {"Réaction spéciale — même en fouillant vite, ton identité spéciale laisse une trace particulière dans la scène : la brume n'a pas fabriqué un survivant ordinaire."}}
    };

    return specialChoiceLines(player, choice, linesByChoice);
}

std::vector<std::string> StoryCampaign::buildChapterOneArrivalLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 1 — La ville qui tient à peine",
        "La forêt se termine sans prévenir.",
        "Un fossé, une route de terre, trois lanternes trop basses, puis des murs de bois renforcés par tout ce que les habitants ont pu trouver : planches, tôles, os de bêtes, morceaux de charrette.",
        "La ville n'a pas l'air sauvée. Elle a l'air de retarder sa chute.",
        "Un garde te pointe du menton au lieu de lever son arme. Il a les yeux d'un homme qui a déjà vu trop de gens sortir de la brume sans comprendre pourquoi.",
        "Le garde : « Ton nom ? »",
        player.getName() + " : « ..." + player.getName() + ". Je crois. »",
        "Le garde : « Tu crois ? »",
        player.getName() + " : « C'est la seule chose qui reste quand j'essaie de penser au reste. »"
    };

    const std::vector<std::string> specialLines = specialArrivalLines(player);
    lines.insert(lines.end(), specialLines.begin(), specialLines.end());

    lines.push_back("Le garde ne rit pas. Dans cette ville, oublier son passé n'est apparemment pas assez rare pour mériter une blague.");
    lines.push_back("Il t'indique une femme près d'un registre humide : Mira, l'intendante de quartier. Si quelqu'un peut transformer un survivant perdu en aide utile, c'est elle.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterOneMiraLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Mira ne te demande pas d'où tu viens. Elle regarde d'abord tes mains, tes bottes, le paquet abîmé, puis la direction de la forêt.",
        "Mira : « Encore la fumée blanche... »",
        player.getName() + " : « Vous savez ce que c'est ? »",
        "Mira : « Non. Les gens qui savent vraiment ne reviennent pas pour expliquer. Mais je sais ce qu'elle laisse derrière elle : des survivants avec un prénom, des monstres trop proches, et des routes qui n'obéissent plus aux cartes. »"
    };

    const std::vector<std::string> specialLines = specialMiraLines(player);
    lines.insert(lines.end(), specialLines.begin(), specialLines.end());

    const std::vector<std::string> endLines = {
        player.getName() + " : « Je dois faire quoi ? »",
        "Mira : « D'abord ? Ne pas mourir. Ensuite, aider la ville à tenir. On manque de tout : nourriture, cuir, os propres, herbes, preuves, bras fiables. »",
        "Elle pousse le registre vers toi. La page des grandes quêtes est presque vide. Les rangs élevés sont barrés. Les routes longues sont fermées. Même les boutiques ne vendent que le minimum.",
        "Mira : « Tu veux des réponses ? Alors commence par nous rapporter de quoi garder les portes ouvertes. Les réponses arrivent rarement avant les vivants. »"
    };
    lines.insert(lines.end(), endLines.begin(), endLines.end());
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterOneMissionLines(const Player& player)
{
    return {
        "Mission principale — Faire respirer les murs",
        "Objectif 1 : faire connaissance avec Mira, qui présente la ville, les noms et les professions utiles.",
        "Objectif 2 : accomplir la première quête principale non refusable : rencontrer Orren, Lysa, Bram et Soryn pendant le tour de ville.",
        "Objectif 3 : notifier Mira que tout le monde a été rencontré, pour qu'elle ajoute sa propre demande et les quatre demandes des référents.",
        "Objectif 4 : retourner parler à Orren, Lysa, Bram et Soryn via PNJ notables, de la part de Mira, afin que chacun ajoute sa quête principale.",
        "Objectif 5 : terminer et rendre les cinq demandes principales, puis notifier Mira une dernière fois.",
        "Objectif narratif : comprendre que les monstres ne viennent pas seulement pour attaquer. Certains fuient quelque chose, d'autres semblent avoir été poussés vers les murs.",
        "Progression actuelle de " + player.getName() + " : " + player.getStoryProgressLabel(),
        "Déblocage futur : quand Mira aura reçu le bilan final des cinq demandes principales, le chapitre 2 ouvrira la route du relais silencieux."
    };
}


bool StoryCampaign::canCompleteChapterOne(const Player& player)
{
    if (player.hasStorySkip() || player.getStoryChapter() >= 2)
    {
        return true;
    }

    const bool miraHasGivenMainRequests = player.getStoryStep() >= 5;
    const bool allMainRequestsTurnedIn = countTurnedInChapterOneMainRequests(player) >= 5;
    return miraHasGivenMainRequests && allMainRequestsTurnedIn;
}

std::vector<std::string> StoryCampaign::buildChapterOneProgressLines(const Player& player)
{
    const int supportQuests = countTurnedInStorySupportQuests(player);
    const int totalTurnedIn = countTurnedInQuests(player);
    const bool miraMet = player.getStoryStep() >= 3 || player.getStoryChapter() >= 2;
    const bool firstReferentTourDone = player.getStoryStep() >= 4 || player.getStoryChapter() >= 2;
    const bool miraNotifiedAfterTour = player.getStoryStep() >= 5 || player.getStoryChapter() >= 2;
    const int mainRequestsDone = countTurnedInChapterOneMainRequests(player);

    return {
        "Validation du chapitre 1 — Mira ne cherche plus des statistiques générales : elle suit une chaîne de quêtes principales.",
        "Mira rencontrée : " + okMissing(miraMet) + " — elle présente la ville, les noms et les professions importantes.",
        "Tour de ville : " + okMissing(firstReferentTourDone) + " — Orren, Lysa, Bram et Soryn doivent être rencontrés avant leurs demandes.",
        "Mira notifiée après le tour : " + okMissing(miraNotifiedAfterTour) + " — c'est elle qui transforme les besoins en quêtes principales non refusables.",
        "Demandes principales rendues : " + okMissing(mainRequestsDone >= 5) + " — " + std::to_string(mainRequestsDone) + "/5 validées auprès de Mira, Orren, Lysa, Bram et Soryn.",
        "Aide libre déjà faite : " + std::to_string(supportQuests) + " demande(s) de soutien utile(s) | Quêtes rendues au total : " + std::to_string(totalTurnedIn) + ".",
        StoryCampaign::canCompleteChapterOne(player)
            ? "Conclusion : Mira peut ouvrir la suite. Le relais silencieux devient une vraie priorité."
            : "Conclusion : demande à Mira ce qu'il reste à faire, puis suis Quêtes > Quête principale et PNJ notables."
    };
}

std::vector<std::string> StoryCampaign::buildChapterOneReferentIntroLines(const Player& player)
{
    return {
        "Première quête principale — Faire le tour de la ville",
        "Mira ne te lance pas vers des demandes sorties de nulle part.",
        "Elle commence par parler de la ville, citer les noms importants et expliquer pourquoi chaque profession compte encore.",
        "Avant de savoir qui a besoin de cuir, d'herbes, de preuves ou de routes, tu dois rencontrer les visages qui tiennent encore la ville debout.",
        "Cette étape n'est pas une faveur optionnelle : la ville ne confiera pas la suite à quelqu'un qui ne connaît même pas les personnes à aider.",
        "Référents à rencontrer :",
        "- Mira (intendante de quartier) : priorités, murs, validation de la suite.",
        "- Orren (vieux garde / référent de route) : chemins, ponts, disparitions.",
        "- Lysa (soigneuse de fortune) : blessés, potions, symptômes étranges.",
        "- Bram (forgeron fatigué) : réparations, outils, matériaux.",
        "- Soryn (archiviste) : traces, légendes, indices à ne pas lire trop tôt.",
        "Progression actuelle de " + player.getName() + " : " + player.getStoryProgressLabel()
    };
}

std::vector<std::string> StoryCampaign::buildChapterOneReferentTourLines(const Player& player)
{
    (void)player;

    return {
        "Mira t'accompagne au moins jusqu'au seuil des bons bâtiments. Elle ne laisse pas la ville te transformer en coursier anonyme.",
        "Mira (intendante de quartier) pose le cadre : rien ne sort des murs sans raison, rien ne rentre sans être compté.",
        "Orren (vieux garde / référent de route) montre une carte trop raturée : certaines routes ne sont pas fermées, elles mentent.",
        "Lysa (soigneuse de fortune) garde des compresses propres dans une boîte trop petite. Elle demande surtout des plantes simples et des signes fiables.",
        "Bram (forgeron fatigué) ne promet pas des miracles. Il promet seulement que le métal cassera moins vite si on lui rapporte de quoi travailler.",
        "Soryn (archiviste) referme un registre dès que tu approches. Il parlera des légendes quand les preuves éviteront de transformer chaque rumeur en vérité.",
        "Le premier tour est fait. Les référents existent maintenant comme personnes, pas comme noms dans une liste.",
        "Mira : « Voilà. Maintenant quand quelqu'un te demande quelque chose, tu sauras au moins pourquoi il n'a pas dormi. »",
        "Prochaine étape : retourne notifier Mira. C'est elle qui ajoutera sa propre demande principale et t'enverra vers les quatre autres de sa part."
    };
}

std::vector<std::string> StoryCampaign::buildStoryTransitionRulesLines(const Player& player)
{
    (void)player;
    return {
        "Règles de transition histoire / bac à sable",
        "Depuis le bac à sable vers l'histoire : le jeu prévient clairement que la route histoire recommence au même niveau pour tous.",
        "Accepter signifie reset du personnage vers le départ histoire : niveau 1, progression neuve, inventaire vidé avant prologue.",
        "Depuis l'histoire vers le bac à sable : deux idées existent.",
        "1) Abandonner le monde à sa destinée : sortie réelle de la route histoire, à réserver à une confirmation forte plus tard.",
        "2) Créer un clone pour la session d'aujourd'hui : personnage éphémère, non sauvegardé, supprimé en fin de session ou au prochain nettoyage s'il reste une trace.",
        "Après la fin complète de l'histoire : le vrai personnage bascule automatiquement en bac à sable, avec tous les systèmes ouverts pour continuer à vivre, explorer, farmer, finir les titres et tester le monde."
    };
}

std::vector<std::string> StoryCampaign::buildStoryCompletionLines(const Player& player)
{
    return {
        "Après la fin du mode histoire",
        "Quand l'histoire principale sera terminée, " + player.getName() + " ne sera pas bloqué sur un écran de fin.",
        "Le personnage entrera automatiquement en bac à sable / mode libre.",
        "Tout ce qui aura été justifié par l'histoire pourra rester ouvert : boutiques développées, routes, boss libres, titres, exploration, quêtes, systèmes avancés.",
        "But : permettre au joueur de continuer à vivre dans le monde après la conclusion, au lieu de perdre son personnage.",
        "Le titre de fin et les trophées ultimes pourront alors servir de traces de conclusion, mais pas de cadenas qui arrêtent le jeu."
    };
}

std::vector<std::string> StoryCampaign::buildChapterOneLines(const Player& player)
{
    (void)player;
    return {
        "Chapitre 1 — La ville qui tient à peine",
        "Départ : sortie de la forêt blanche, kit récupéré sur un cadavre, peu de boutiques, peu de contrats, très peu d'informations fiables.",
        "La mission principale n'est pas encore de sauver le monde. C'est de prouver que la ville peut survivre une semaine de plus.",
        "Objectif principal proposé : survivre au prologue, atteindre la ville, sécuriser les abords immédiats, rapporter des preuves de monstres proches, puis débloquer le vrai comptoir de guilde.",
        "Objectifs de soutien : rencontrer les premiers référents, puis aider avec rations, matériaux de réparation, peaux, os, venin contrôlé et herbes simples.",
        "Intrigue : les monstres arrivent trop près des murs et certains portent des traces qui ne correspondent pas à leur espèce.",
        "Verrous : les routes longues, boss avancés et boutiques complètes restent fermés en mode histoire.",
        "PNJ référent mis en avant : Mira, intendante de quartier. Elle ne combat pas, mais elle sait ce que la ville peut construire avec ce que tu rapportes."
    };
}

std::vector<std::string> StoryCampaign::buildChapterTwoLines(const Player& player)
{
    return {
        "Chapitre 2 — Le relais silencieux",
        "Déclenchement conseillé : niveau 4 environ, une première vraie pression de monstres réglée, ou un premier boss/événement proche validé.",
        "État actuel : chapitre suggéré par progression = " + std::to_string(suggestedChapterFromProgress(player)) + " | Chapitre sauvegardé = " + std::to_string(player.getStoryChapter()) + ".",
        "Le relais du nord ne répond plus. Les caravanes ne reviennent pas. Les cartes indiquent encore la route, mais les gardes disent qu'elle paraît plus longue qu'avant.",
        "Mission principale : comprendre pourquoi les bornes ont été retournées, utiliser la Route commerciale comme vraie zone d'enquête, affronter la première patrouille qui verrouille le relais, faire répondre le relais, sauver Nell, exploiter sa sacoche, identifier la main invisible qui réécrit les chemins, puis contenir le premier verrou de la borne noire.",
        "Demandes de soutien possibles : la quête principale peut demander explicitement de retourner voir des PNJ déjà présentés pour fournir matériaux, nourriture, cartes, confiance ou stocks de comptoir.",
        "Menace d'étape : pas encore une grande menace finale, mais une présence imposée par l'histoire : les guetteurs sans feu, affrontés dans l'ordre au relais.",
        "Conséquence : après l'affrontement, le signal réactivé, le premier sauvetage, la sacoche de Nell, le contre-registre et les cicatrices du verrou, la ville gagne des routes courtes surveillées plutôt qu'une simple victoire de façade.",
        "Reblocage possible : si une crise coupe le relais ou blesse un PNJ clé, certaines boutiques/contrats se ferment temporairement."
    };
}


std::vector<std::string> StoryCampaign::buildChapterTwoMissionLines(const Player& player)
{
    return {
        "Mission principale — Le relais silencieux",
        "Objectif 1 : écouter le briefing de Mira et Orren autour du registre du relais.",
        "Objectif 2 : reconnaître la Route commerciale depuis la sortie nord, car la route semble plus longue que sur les cartes.",
        "Objectif 3 : rapporter une preuve précise à Soryn : borne retournée, trace contradictoire ou signe de monstre poussé vers les murs.",
        "Objectif 4 : affronter les guetteurs sans feu, première vraie menace imposée du relais. Ce n'est pas un contrat libre à esquiver.",
        "Objectif 5 : réactiver un signal simple du relais avec Mira, Orren, Bram et Soryn, pour que la route réponde avant d'être considérée comme vraiment tenue.",
        "Objectif 6 : suivre le premier appel rendu possible par ce signal et sauver Nell la messagère d'un convoi brisé.",
        "Objectif 7 : exploiter la sacoche de routes de Nell pour transformer le sauvetage en vraie piste.",
        "Objectif 8 : distribuer les informations aux comptoirs pour que la ville gagne des conséquences visibles : stocks, rumeurs, contrats et confiance.",
        "Objectif 9 : suivre l'encre froide de la route sur deux scènes de Route commerciale plus scénarisées.",
        "Objectif 10 : identifier ce qui réécrit la route, sans encore révéler tout l'antagoniste derrière le phénomène.",
        "Objectif 11 : installer un contre-registre de routes courtes pour que les comptoirs n'obéissent pas aux cartes corrompues.",
        "Objectif 12 : repérer le nœud noir, première annonce de la prochaine vraie crise du chapitre 2.",
        "Objectif 13 : tenir pendant les travaux, car la ville doit réparer, préparer les soins et stabiliser ses comptoirs avant la sortie dangereuse.",
        "Objectif 14 : confirmer qu'une présence garde la borne noire, sans donner son vrai nom trop tôt.",
        "Objectif 15 : briser le premier verrou de la borne noire, une menace d'étape encore désignée par preuves plutôt que par identité complète.",
        "Objectif 16 : lire les cicatrices du verrou avec Soryn, Nell et Orren pour comprendre ce qui a cédé sans écrire trop vite un faux nom dans le registre.",
        "Objectif 17 : organiser une route gardée : petits départs, marques de retour, stocks surveillés et relais secondaires, afin que la ville ne confonde pas une victoire avec une route sauvée.",
        "Progression actuelle de " + player.getName() + " : " + player.getStoryProgressLabel(),
        "Quêtes principales visibles : Quêtes > Quête principale. Les actions utiles passent par Exploration, PNJ notables et rendu auprès du bon contact."
    };
}

std::vector<std::string> StoryCampaign::buildChapterTwoProgressLines(const Player& player)
{
    const int done = countTurnedInChapterTwoMainRequests(player);
    std::string conclusion;
    if (player.getStoryStep() >= 18)
    {
        conclusion = "Conclusion actuelle : la route gardée tient pour les premiers retours, mais le vrai nom derrière la borne noire reste encore hors du registre.";
    }
    else if (player.getStoryStep() >= 17)
    {
        conclusion = "Conclusion actuelle : les cicatrices du verrou sont classées. La ville doit maintenant organiser une route gardée au lieu de célébrer trop vite.";
    }
    else if (player.getStoryStep() >= 16)
    {
        conclusion = "Conclusion actuelle : le premier verrou de la borne noire a cédé. Il faut lire ce qu'il a laissé avant de décider ce que la route a vraiment perdu.";
    }
    else if (player.getStoryStep() >= 15)
    {
        conclusion = "Conclusion actuelle : la ville sait assez de choses pour viser le premier verrou de la borne noire sans révéler le vrai nom de la menace.";
    }
    else if (player.getStoryStep() >= 14)
    {
        conclusion = "Conclusion actuelle : les réparations ont tenu assez longtemps. Il faut maintenant identifier ce qui garde la borne noire sans lui donner un nom trop tôt.";
    }
    else if (player.getStoryStep() >= 13)
    {
        conclusion = "Conclusion actuelle : le nœud noir est repéré. Mira demande de tenir le quartier pendant les réparations au lieu de courir trop vite vers la crise.";
    }
    else if (player.getStoryStep() >= 12)
    {
        conclusion = "Conclusion actuelle : le contre-registre tient. Il faut maintenant aller voir ce que la route tente de cacher au nord.";
    }
    else if (player.getStoryStep() >= 11)
    {
        conclusion = "Conclusion actuelle : la route est bien réécrite. Il faut protéger les comptoirs avec une règle plus fiable que les cartes corrompues.";
    }
    else if (player.getStoryStep() >= 10)
    {
        conclusion = "Conclusion actuelle : l'encre froide donne une vraie piste. Il faut identifier le procédé qui réécrit la route.";
    }
    else if (player.getStoryStep() >= 9)
    {
        conclusion = "Conclusion actuelle : les comptoirs respirent. Il faut suivre les deux traces d'encre froide sur la Route commerciale.";
    }
    else if (player.getStoryStep() >= 8)
    {
        conclusion = "Conclusion actuelle : la sacoche parle. Il faut maintenant transformer ses informations en stocks, rumeurs et confiance locale.";
    }
    else if (player.getStoryStep() >= 7)
    {
        conclusion = "Conclusion actuelle : Nell est sauvée. Sa sacoche doit être exploitée avant de prétendre comprendre la route.";
    }
    else if (player.getStoryStep() >= 6)
    {
        conclusion = "Conclusion actuelle : le relais répond. Il faut maintenant suivre le premier appel et prouver que la route peut ramener des vivants.";
    }
    else if (player.getStoryStep() >= 5)
    {
        conclusion = "Conclusion actuelle : les guetteurs sont repoussés. Il reste à faire répondre le relais, pas seulement à le traverser.";
    }
    else if (player.getStoryStep() >= 4)
    {
        conclusion = "Conclusion actuelle : la route a assez menti. La prochaine étape est l'affrontement imposé des guetteurs sans feu.";
    }
    else
    {
        conclusion = "Conclusion actuelle : continuer le relais silencieux demande exploration, preuve et rendu auprès des référents.";
    }

    return {
        "Validation du chapitre 2 — première enquête de route.",
        "Briefing Mira/Orren : " + storyQuestShortState(player, "story_ch2_relay_briefing") + ".",
        "Reconnaissance de la Route commerciale : " + storyQuestShortState(player, "story_ch2_north_road_scout") + ".",
        "Preuve de borne retournée : " + storyQuestShortState(player, "story_ch2_turned_marker") + ".",
        "Menace imposée — Les guetteurs sans feu : " + storyQuestShortState(player, "story_ch2_relay_threat") + ".",
        "Signal du relais — Le relais doit répondre : " + storyQuestShortState(player, "story_ch2_relay_signal") + ".",
        "Premier sauvetage — La voix derrière les caisses : " + storyQuestShortState(player, "story_ch2_first_rescue") + ".",
        "Sacoche de routes — La sacoche qui parle : " + storyQuestShortState(player, "story_ch2_route_sack") + ".",
        "Conséquences de ville — Les comptoirs rouvrent un œil : " + storyQuestShortState(player, "story_ch2_city_recovery") + ".",
        "Route scénarisée — L'encre froide de la route : " + storyQuestShortState(player, "story_ch2_cold_ink_trail") + ".",
        "Enquête — La carte qui se réécrit : " + storyQuestShortState(player, "story_ch2_route_rewrite") + ".",
        "Ville/économie — Le contre-registre des routes courtes : " + storyQuestShortState(player, "story_ch2_short_route_counter") + ".",
        "Alerte — Le nœud noir au bout du relais : " + storyQuestShortState(player, "story_ch2_black_knot_warning") + ".",
        "Réparations — Tenir pendant les travaux : " + storyQuestShortState(player, "story_ch2_repair_downtime") + ".",
        "Piste de menace — La chose qui garde la borne : " + storyQuestShortState(player, "story_ch2_hidden_guardian_hint") + ".",
        "Boss d'étape — Le verrou de la borne : " + storyQuestShortState(player, "story_ch2_black_knot_seal") + ".",
        "Après-crise — Les cicatrices du verrou : " + storyQuestShortState(player, "story_ch2_black_knot_scars") + ".",
        "Ville/route — Une route à garder ouverte : " + storyQuestShortState(player, "story_ch2_guarded_route") + ".",
        "Étapes principales rendues : " + std::to_string(done) + "/17.",
        conclusion
    };
}

std::vector<std::string> StoryCampaign::buildChapterTwoBriefingLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — Premier briefing du relais silencieux",
        "Mira pose le registre au milieu de la table. Orren ne s'assoit pas : il reste debout, comme si une chaise pouvait l'empêcher de repartir assez vite.",
        "Sur la carte, le relais du nord n'est pas loin. Sur les récits des gardes, il recule chaque fois qu'on l'approche.",
        "Orren : « Les bornes ne disparaissent pas. Elles changent de côté. C'est pire. Une route qui se trompe toute seule finit par ramener les gens au mauvais endroit. »",
        "Mira : « Tu ne pars pas chercher une gloire. Tu pars chercher une contradiction assez nette pour qu'on sache quoi fermer, quoi ouvrir, et qui prévenir. »",
        player.getName() + " reçoit une consigne simple : reconnaître la Route commerciale depuis la sortie nord, puis revenir avec autre chose qu'une impression."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Le relais réagit aussi à ton identité particulière :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale suivante : La route qui s'allonge.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoRelaySignalLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — Le relais doit répondre",
        "Les guetteurs sans feu ont reculé, mais Orren refuse d'appeler ça une route sûre.",
        "Orren : « Une route sans réponse redevient un piège dès que le prochain groupe panique. Il faut un signe simple, visible, répétable. »",
        "Bram fournit une cloche basse récupérée sur une carriole cassée. Lysa insiste pour placer une marque près de l'abri le moins exposé. Soryn exige que la preuve soit inscrite proprement dans le registre.",
        "Mira te confie la dernière étape du relais : faire sonner quelque chose qui prouve aux prochaines patrouilles que la route n'est plus seulement un endroit où l'on disparaît."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale prête à compléter : Le relais doit répondre.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoFirstRescueLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — La voix derrière les caisses",
        "La cloche basse du relais répond à peine depuis quelques heures quand un garde revient avec le visage pâle.",
        "Garde : « On a entendu trois coups depuis le nord. Pas une attaque. Un signal de vivant. »",
        "Orren déplie la route sans parler. Soryn note déjà l'heure. Mira ne demande pas si tu veux y aller : cette fois, la route vient de réclamer quelqu'un.",
        "Le signal mène vers un convoi brisé. Entre deux caisses renversées, Nell la messagère tient encore debout, blessée, mais assez lucide pour protéger une sacoche de routes.",
        "Objectif : dégager le passage, repousser ce qui rôde autour du convoi, puis ramener Nell assez vivante pour qu'elle parle."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale colore aussi ce sauvetage :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale prête : La voix derrière les caisses.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoRouteSackLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — La sacoche qui parle",
        "Nell pose sa sacoche sur la table comme si elle déposait quelqu'un de vivant.",
        "Dedans : trois cartes froissées, deux bons de convoi, un nom de halte rayé deux fois, et une marque d'encre froide qui ne sèche pas.",
        "Nell : « Les routes courtes ne sont pas toutes mortes. Mais quelqu'un les force à se contredire. Si vous suivez la mauvaise ligne, vous revenez au même relais avec une heure de moins dans le corps. »",
        "Objectif : trier la sacoche avec Nell, Mira, Orren et Soryn pour choisir une piste exploitable au lieu de partir sur dix rumeurs."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale réagit aussi à la sacoche :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale prête à compléter : La sacoche qui parle.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoCityRecoveryLines(const Player& player)
{
    (void)player;
    return {
        "Chapitre 2 — Les comptoirs rouvrent un œil",
        "Mira refuse de laisser la sacoche de Nell dormir dans un coffre. Une information qui ne circule pas ne sauve personne.",
        "Herboristerie : Lysa note quels paniers peuvent revenir par route courte, donc les plantes communes deviennent moins absurdes à obtenir.",
        "Forge : Bram repère deux itinéraires où les plaques et sangles de convoi peuvent être récupérées sans envoyer une caravane entière au suicide.",
        "Guilde : le maître accepte d'afficher des contrats de route un peu plus sérieux, tant qu'ils restent près du relais et pas vers les grands biomes.",
        "Relais : Orren et Nell ajoutent une règle simple : chaque départ doit laisser un signe de retour, même si le groupe pense revenir vite.",
        "Conséquence : les boutiques et PNJ peuvent maintenant commenter le palier de ville et justifier de meilleurs stocks de terrain sans ouvrir toute la route trop tôt."
    };
}

std::vector<std::string> StoryCampaign::buildChapterTwoColdInkTrailLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — L'encre froide de la route",
        "La marque de la sacoche ne correspond à aucune encre de Soryn. Elle ne tache pas les doigts, mais refroidit le papier autour d'elle.",
        "Première trace : la carte indique une halte qui existe encore sur le terrain, mais dont le nom a été gratté dans trois registres différents.",
        "Deuxième trace : une ligne de route revient vers le relais alors qu'elle devrait traverser un pont court. Orren appelle ça une boucle. Soryn appelle ça une preuve.",
        "Objectif : suivre ces deux traces sur la Route commerciale pour ouvrir la prochaine vraie crise du chapitre 2."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale lit aussi quelque chose dans l'encre froide :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : L'encre froide de la route.");
    return lines;
}


std::vector<std::string> StoryCampaign::buildChapterTwoRouteRewriteLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — La carte qui se réécrit",
        "Soryn ne pose pas l'encre froide sur sa table. Il la pose sur une assiette de fer, comme si le papier pouvait contaminer le bois.",
        "Soryn : « Une encre normale raconte ce qu'une main a écrit. Celle-ci corrige ce qu'une route a vécu. »",
        "Nell compare trois bons de convoi. Les numéros ne changent pas, mais les destinations se déplacent d'une ligne à l'autre quand personne ne regarde directement.",
        "Orren résume sans poésie : quelqu'un ou quelque chose ne bloque pas les routes. Il les réécrit après le passage des vivants.",
        "Objectif : classer le phénomène avec Soryn et Nell, afin de passer d'une rumeur d'encre à une menace nommable : une main invisible dans les cartes."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale réagit aussi à cette route réécrite :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : La carte qui se réécrit.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoShortRouteCounterLines(const Player& player)
{
    (void)player;
    return {
        "Chapitre 2 — Le contre-registre des routes courtes",
        "Mira refuse que la ville continue d'utiliser les mêmes cartes que l'encre froide sait corriger.",
        "Eda, une comptable de routes courtes, sort des ficelles, des jetons de bois et un vieux registre de dettes de convoi.",
        "Eda : « Si la carte ment, on ne suit plus la carte seule. On suit les retours, les marques, les stocks réellement revenus et les témoins encore vivants. »",
        "Herboristerie : les paniers de plantes sont marqués par provenance réelle, pas seulement par destination prévue.",
        "Forge : Bram note quels clous, plaques et sangles reviennent par trajet confirmé.",
        "Guilde : les contrats de route courte affichent maintenant un avertissement quand leur destination dépend d'une carte non vérifiée.",
        "Objectif : installer ce contre-registre pour donner une conséquence économique concrète à l'enquête, sans ouvrir toutes les boutiques d'un coup."
    };
}

std::vector<std::string> StoryCampaign::buildChapterTwoBlackKnotWarningLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — Le nœud noir au bout du relais",
        "Le contre-registre révèle une chose simple et moche : toutes les erreurs ne partent pas du relais. Elles y reviennent.",
        "Nell pointe trois trajets courts. Orren pointe trois disparitions. Soryn pointe trois corrections d'encre froide. Les lignes forment un nœud autour d'une ancienne borne noire.",
        "Mira : « On ne va pas y envoyer une caravane. On va y envoyer quelqu'un qui sait revenir avec une preuve, pas avec une légende de plus. »",
        "Sur la route, les traces deviennent plus propres au lieu de devenir plus sauvages. C'est presque pire : la prochaine crise n'a pas l'air affamée. Elle a l'air organisée.",
        "Objectif : reconnaître le nœud noir, repousser ce qui surveille l'approche et revenir avant que la route n'apprenne ton trajet."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale sent aussi le danger du nœud noir :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : Le nœud noir au bout du relais.");
    return lines;
}


std::vector<std::string> StoryCampaign::buildChapterTwoRepairDowntimeLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — Tenir pendant les réparations",
        "Le nœud noir est repéré, mais Mira interdit de foncer dessus pendant que la ville pose ses protections.",
        "Bram doit renforcer deux ferrures de porte, Lysa doit préparer les trousses de retour, Eda doit recopier les routes courtes à partir des stocks réels, et Orren doit vérifier les relais secondaires.",
        "Mira : « Si tu pars maintenant, tu arrives fatigué, mal préparé, avec une ville qui ne sait pas te récupérer. Alors tu t'occupes utilement. »",
        "Objectif : faire avancer trois actions utiles pendant les réparations : patrouille courte, service de comptoir, quête secondaire, exploration proche ou demande de PNJ.",
        "Mira veut que tu reviennes avec des gestes utiles dans les jambes, pas avec une impatience héroïque et des portes encore ouvertes derrière toi."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale change aussi cette attente :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : Tenir pendant les travaux.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoHiddenGuardianHintLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — La chose qui garde la borne",
        "Les réparations tiennent. Le contre-registre ne ment pas. Et justement, c'est là que le registre devient inquiétant.",
        "Eda montre une colonne presque vide : tous les retours confirmés évitent un même point, comme si les voyageurs savaient inconsciemment qu'il ne faut pas écrire ce nom.",
        "Soryn refuse encore d'appeler ça une bête. Orren refuse d'appeler ça un simple barrage. Nell, elle, dit seulement : « Sur les routes, quand tout le monde évite le même silence, c'est qu'il y a quelqu'un au milieu. »",
        "Mira ne donne pas le vrai nom. Pas encore. Elle inscrit seulement une désignation de terrain : la chose qui garde la borne.",
        "Objectif : recouper les témoignages, identifier la menace assez précisément pour préparer l'affrontement, sans révéler trop tôt son vrai nom ni sa nature complète.",
        "Soryn : « Si le registre demande un jour de s'en débarrasser, ce ne sera pas pour frapper une ombre au hasard. Ce sera parce que trop de rumeurs et trop de preuves auront déjà désigné le même silence. »"
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale sent aussi cette présence :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : La chose qui garde la borne.");
    return lines;
}


std::vector<std::string> StoryCampaign::buildChapterTwoBlackKnotSealLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — Le verrou de la borne",
        "Soryn n'a pas écrit de nom dans le registre. Il a écrit une forme : un verrou, pas encore une personne, pas encore un monstre, mais quelque chose qui décide qui a le droit de passer.",
        "Orren pose deux marques de craie sur la carte. La première indique où revenir. La seconde indique où ne pas tomber.",
        "Mira : « On ne va pas chercher une victoire propre. On va chercher une preuve que la borne peut saigner, céder ou reculer. »",
        "Nell ajoute une règle simple : si la route commence à raccourcir au mauvais moment, il faut frapper le point fixe, pas courir derrière la distance.",
        "Objectif : affronter le verrou de la borne noire comme une menace d'étape non nommée, repousser sa garde et revenir faire classer la preuve auprès d'Orren."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale réagit avant l'affrontement :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : Le verrou de la borne.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoBlackKnotScarsLines(const Player& player)
{
    std::vector<std::string> lines = {
        "Chapitre 2 — Les cicatrices du verrou",
        "Le verrou a cédé, mais personne n'applaudit. Orren garde une main sur la carte, comme si la route pouvait encore mordre à travers le papier.",
        "Soryn gratte un peu d'encre noire sur une lame de bois. Elle n'est plus froide. Elle pulse faiblement, puis s'éteint dès que Nell prononce le nom d'une halte réelle.",
        "Mira : « On ne lui donne pas un nom parce qu'on a peur d'un silence. On classe les preuves, pas les frissons. »",
        "Objectif : relire les marques laissées par le verrou, comparer encre, traces et témoignages, puis rendre la preuve auprès de Soryn.",
        "Conséquence : la menace reste partiellement inconnue, mais la ville sait maintenant que la borne noire peut être blessée et que les retours réels l'affaiblissent."
    };

    const std::vector<std::string> specialLines = buildChapterTwoSpecialThreatLines(player);
    if (!specialLines.empty())
    {
        lines.push_back("Ton identité spéciale relit aussi les marques du combat :");
        lines.insert(lines.end(), specialLines.begin(), specialLines.end());
    }

    lines.push_back("Quête principale : Les cicatrices du verrou.");
    return lines;
}

std::vector<std::string> StoryCampaign::buildChapterTwoGuardedRouteLines(const Player& player)
{
    (void)player;
    return {
        "Chapitre 2 — Une route à garder ouverte",
        "La borne noire a reculé une fois. Ça ne suffit pas à rouvrir les grands départs. Mira refuse de vendre une victoire comme une sécurité.",
        "Orren choisit trois trajets courts, Nell choisit deux messagers capables de revenir même sans gloire, et Eda impose une règle : aucun stock n'est compté tant que quelqu'un de vivant ne l'a pas confirmé.",
        "Bram prépare des marques de métal simples pour les caisses de retour. Lysa ajoute des paquets de soin aux premiers convois, pas pour les héros : pour ceux qui devront rentrer blessés.",
        "Objectif : organiser une route gardée autour des preuves obtenues, valider les premiers retours et stabiliser les conséquences en ville sans prétendre que la crise est terminée.",
        "Conséquence : les boutiques et comptoirs peuvent respirer un peu mieux, mais la suite du chapitre devra découvrir qui profite vraiment des routes réécrites.",
        "Quête principale : Une route à garder ouverte."
    };
}

std::vector<std::string> StoryCampaign::buildChapterTwoSpecialThreatLines(const Player& player)
{
    static const std::map<std::string, std::vector<std::string>> linesByKey = {
        {"hazak", {"Hazak remarque que les guetteurs n'ont pas seulement attendu : ils ont choisi des angles de mort trop propres pour des monstres ordinaires.", "Le relais silencieux ressemble moins à une route perdue qu'à une embuscade entretenue par quelqu'un qui sait penser."}},
        {"mattzelda", {"Mattzelda sent la route résister sous ses pas. Si même lui a l'impression de pousser contre le chemin, ce n'est pas une simple fatigue.", "Le relais a besoin d'un poids stable : quelque chose ou quelqu'un qui prouve qu'on peut encore tenir ici."}},
        {"aoi", {"Aoi sent une odeur de cendre froide là où aucune torche n'a brûlé. La chaleur kitsune en elle n'aime pas ce silence.", "Le relais devra répondre par un signe vivant, pas seulement par une carte corrigée."}},
        {"kanade", {"Kanadé regarde le ciel au-dessus de la route. Les signes sont brouillés, mais pas vides : quelque chose les force à mentir.", "Si le relais doit répondre, il faudra un signal assez clair pour traverser même un ciel mal disposé."}},
        {"fail", {"Fail trouve la route presque trop bien rangée dans son absurdité. Quand le chaos paraît discipliné, il devient franchement suspect.", "Le signal du relais aura au moins le mérite de contrarier ce silence beaucoup trop sérieux."}},
        {"trexof", {"Trexof repère une répétition dans les bornes, puis une autre dans les traces. La route ne ment pas au hasard : elle suit une règle cachée.", "Réactiver le relais devient plus qu'un service : c'est planter un repère dans une route qui déteste les repères."}},
        {"skuro", {"Skuro n'aime pas les routes qui refusent l'impact. Les guetteurs ont reculé, mais le silence donne encore l'impression d'avoir des dents.", "Faire répondre le relais, pour lui, revient presque à planter un clou dans la gueule du problème."}},
        {"sanctus", {"Sanctus ressent dans le relais un serment interrompu. Pas une malédiction claire, plutôt une promesse que quelqu'un a cessé d'honorer.", "Le signal devra faire plus que guider : il devra rappeler que cette route protège encore les vivants."}},
        {"hestia", {"Hestia trouve le relais plus triste qu'effrayant. L'endroit semble attendre qu'on lui dise qu'il a encore le droit d'abriter quelqu'un.", "Quand le signal revient, la protection autour d'elle se calme, comme si le lieu respirait un peu mieux."}},
        {"louis", {"Louis compte les distances, les angles et les points d'appui. La route ment, mais pas assez pour empêcher une méthode de tenir.", "Le signal du relais devient une solution simple : un repère que même la panique peut comprendre."}},
        {"henrique", {"Henrique ne cherche pas à philosopher sur la route. Elle ment, donc il faut lui imposer une réponse plus têtue qu'elle.", "Le relais doit sonner parce que quelqu'un devra l'entendre et se dire qu'il n'est pas encore trop tard."}},
        {"generic", {"Ton identité spéciale réagit au relais comme à un vieux danger mal nommé. La route n'a pas seulement besoin d'être ouverte : elle doit redevenir fiable."}}
    };

    return specialChapterTwoLinesByKey(player, linesByKey);
}

std::vector<std::string> StoryCampaign::buildSandboxRulesLines(const Player& player)
{
    (void)player;
    return {
        "Bac à sable / mode libre",
        "Le bac à sable garde les systèmes accessibles pour jouer librement : boss, quêtes, boutiques, arènes, exploration, titres, cheats, etc.",
        "Le mode histoire, lui, ne doit pas tout ouvrir dès le début. Il verrouille volontairement les menus par logique narrative.",
        "Différence importante : un boss peut exister dans le bac à sable sans être disponible dans l'histoire au même moment.",
        "Même chose pour les boutiques : la forge complète peut exister techniquement, mais l'histoire peut n'autoriser qu'une forge pauvre tant que Bram n'a pas ses matériaux.",
        "Cette séparation permet de continuer à développer les fonctionnalités sans casser l'immersion du début d'histoire.",
        "Le menu d'entrée du mode histoire est volontairement séparé : Nouvelle histoire, Continuer, Sélectionner le chapitre.",
        "Nouvelle histoire recommence au début commun : niveau 1, aucun confort, prologue de la fumée blanche.",
        "Continuer lance automatiquement la prochaine étape dans l'ordre, sans demander au joueur de choisir un chapitre à chaque fois.",
        "Sélectionner le chapitre ne montre que les chapitres déjà débloqués comme sélectionnables ; les autres restent verrouillés ou masqués.",
        "Depuis un personnage déjà joué en bac à sable, commencer l'histoire demande une confirmation : le personnage repart au niveau 1, sans son confort, car tout le monde traverse la même fumée blanche.",
        "Depuis l'histoire, le joueur peut créer un clone de session pour s'amuser en bac à sable sans sauvegarde réelle.",
        "À la fin complète de l'histoire, le vrai personnage bascule automatiquement en bac à sable pour continuer sa vie avec tout ce qui a été débloqué."
    };
}

std::vector<std::string> StoryCampaign::buildDevelopmentLines(const Player& player)
{
    StoryAccessSnapshot snapshot = buildAccessSnapshot(player);
    std::vector<std::string> lines;
    lines.push_back("Développement actuel de la ville : palier " + std::to_string(snapshot.cityDevelopment) + ".");
    lines.push_back("Chapitre sauvegardé : " + std::to_string(snapshot.chapter) + " | Étape : " + std::to_string(snapshot.step) + ".");
    lines.push_back("");
    lines.push_back("Débloqué / ouvert :");
    for (const std::string& line : snapshot.unlockedSystems) lines.push_back("- " + line);
    lines.push_back("");
    lines.push_back("Limité / fragile :");
    for (const std::string& line : snapshot.limitedSystems) lines.push_back("- " + line);
    lines.push_back("");
    lines.push_back("Encore fermé dans l'histoire :");
    for (const std::string& line : snapshot.lockedSystems) lines.push_back("- " + line);
    return lines;
}

std::vector<std::string> StoryCampaign::buildReferentNpcLines(const Player& player)
{
    StoryAccessSnapshot snapshot = buildAccessSnapshot(player);
    std::vector<std::string> lines = {
        "Clients amis / référents prévus",
        "Ces PNJ ne sont pas forcément des alliés de combat. Certains servent de repères, de donneurs de missions ou de verrous narratifs."
    };
    for (const std::string& line : snapshot.referentNpcs) lines.push_back("- " + line);
    return lines;
}

std::vector<std::string> StoryCampaign::buildIntrigueLines(const Player& player)
{
    StoryAccessSnapshot snapshot = buildAccessSnapshot(player);
    std::vector<std::string> lines = {
        "Intrigues suivies prévues",
        "Le but final ne doit pas être donné directement au début. Chaque intrigue apporte une partie du vrai problème."
    };
    for (const std::string& line : snapshot.intrigueThreads) lines.push_back("- " + line);
    return lines;
}

std::vector<std::string> StoryCampaign::buildNextObjectiveLines(const Player& player)
{
    StoryAccessSnapshot snapshot = buildAccessSnapshot(player);
    std::vector<std::string> lines;
    lines.push_back("Progression histoire sauvegardée : chapitre " + std::to_string(player.getStoryChapter()) + ", étape " + std::to_string(player.getStoryStep()) + ".");
    lines.push_back("Chapitre suggéré par le personnage : " + std::to_string(suggestedChapterFromProgress(player)) + ".");
    lines.push_back("Chapitre 2 prêt selon progression : " + yesNo(canUnlockChapterTwo(player)) + ".");
    lines.push_back("Chapitre maximum sélectionnable : " + std::to_string(maxUnlockedChapter(player)) + ".");
    lines.push_back("");
    lines.push_back("Objectifs principaux :");
    for (const std::string& line : snapshot.mainObjectives) lines.push_back("- " + line);
    lines.push_back("");
    lines.push_back("Objectifs secondaires utiles au développement :");
    for (const std::string& line : snapshot.sideObjectives) lines.push_back("- " + line);
    return lines;
}
