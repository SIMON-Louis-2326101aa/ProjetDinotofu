// EN: LegendTriggerSystem.cpp implements optional lore legend triggers for Dinotofu.
// FR: LegendTriggerSystem.cpp implémente les déclencheurs optionnels de légendes pour Dinotofu.
// English: Code identifiers are written in English; player-facing text can stay in French.
// Français : Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.

#include "lore/LegendTriggerSystem.hpp"

#include "interface/menu/common/MessageScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace
{

    std::set<std::string>& displayedTriggerIds()
    {
        static std::set<std::string> ids;
        return ids;
    }

    bool reserveTriggerDisplay(const std::string& id)
    {
        if (id.empty())
        {
            return false;
        }

        std::set<std::string>& ids = displayedTriggerIds();
        if (ids.find(id) != ids.end())
        {
            return false;
        }

        ids.insert(id);
        return true;
    }

    bool reserveAutomaticLegendDisplay(const std::string& id, const LegendArchiveEntry& entry)
    {
        if (entry.title.empty())
        {
            return reserveTriggerDisplay(id);
        }

        // EN: Auto triggers should not replay after the same save already learned/read this legend.
        // FR: Les déclenchements automatiques ne doivent pas rejouer si la sauvegarde connaît déjà cette légende.
        if (BestiaryRuntimeProgress::getEncounterCount(entry.title) > 0)
        {
            return false;
        }

        return reserveTriggerDisplay(id);
    }

    bool containsName(const std::vector<std::string>& names, const std::string& expectedName)
    {
        return std::find(names.begin(), names.end(), expectedName) != names.end();
    }

    bool hasLesBrasCasses(const std::vector<std::string>& names)
    {
        return containsName(names, "Hazak")
            && containsName(names, "Fail")
            && containsName(names, "Aoi")
            && containsName(names, "Kanadé")
            && containsName(names, "Sanctus");
    }

    void recordLegendTrace(const LegendArchiveEntry& entry)
    {
        BestiaryRuntimeProgress::recordEncounter(
            entry.title,
            "Légendes / contes",
            entry.shortDescription
        );
    }

    void showLegend(const LegendArchiveEntry& entry, const std::string& screenId, bool waitAndClear)
    {
        recordLegendTrace(entry);

        std::vector<std::string> lines;
        lines.push_back("Catégorie : " + entry.category);
        lines.push_back("Source : " + entry.source);
        lines.push_back("");

        for (const std::string& line : entry.lines)
        {
            lines.push_back(line);
        }

        MessageScreen::show(
            entry.title,
            screenId,
            lines,
            waitAndClear
        );
    }

    LegendArchiveEntry makeBrasCassesOrigin()
    {
        return LegendArchiveEntry{
            "bras_casses_origin",
            "Origine du nom des Bras cassés",
            "Groupes et héros",
            "Conte de taverne / rumeur d'aventurier",
            "La rumeur raconte pourquoi le groupe de Hazak porte un nom aussi peu glorieux pour des héros.",
            {
                "On raconte qu'avant de devenir un nom connu, Hazak a perdu un bras face à un orc.",
                "Pendant deux semaines, le groupe aurait continué à avancer avec un chef incomplet, trop fier pour admettre qu'il avait besoin d'aide.",
                "Puis le bras est revenu. Régénéré, reconstruit, ou arraché à une règle que personne n'a vraiment comprise.",
                "Depuis, les villages les appellent Les bras cassés : un peu pour le bras de Hazak, beaucoup parce qu'ils font n'importe quoi avant de réussir quand même.",
                "Héros, oui. Exemplaires, pas toujours. Mais quand un village tombe dans le chaos, on préfère souvent des bras cassés qui gagnent à des héros propres qui arrivent trop tard."
            }
        };
    }

    LegendArchiveEntry makeChildTale()
    {
        return LegendArchiveEntry{
            "child_tale_chaotic_heroes",
            "Conte pour enfant : les cinq héros trop bruyants",
            "Contes pour enfant",
            "Bibliothèque / rayon pour enfants",
            "Version adoucie et drôle des Bras cassés, racontée sans détail trop violent.",
            {
                "Dans la version pour enfants, cinq aventuriers arrivent toujours trop tard, trop fort, ou par la mauvaise porte.",
                "L'assassin demande le silence, le mage transforme le silence en expérience, la kitsune cache ses flammes, la semi-dragonne râle, et le protecteur répare ce qu'il peut.",
                "À la fin, le monstre tombe, le village survit, et personne n'arrive à expliquer pourquoi le plan a marché.",
                "La morale varie selon le conteur : certains disent qu'il faut respecter les règles, d'autres qu'il faut surtout sauver les gens avant de remplir les formulaires."
            }
        };
    }

    LegendArchiveEntry makeSpecialArenaRumor()
    {
        return LegendArchiveEntry{
            "special_arena_named_opponent",
            "Rumeur des adversaires spéciaux",
            "Déclencheurs et rumeurs",
            "Arène IA / gradins nerveux",
            "Une rumeur explique pourquoi certains noms ne doivent pas apparaître dans le registre avant d'avoir été vus.",
            {
                "Dans les gradins, personne ne prononce les grands noms trop vite.",
                "On dit que certains adversaires ne sont pas de simples tirages : ils portent déjà une histoire, un groupe, ou une dette que le registre ne veut pas révéler gratuitement.",
                "Quand l'arène confirme enfin l'un de ces profils, la fiche cesse d'être une silhouette et devient une trace réelle.",
                "La rumeur ne donne pas une faiblesse. Elle rappelle seulement qu'un nom spécial doit se gagner par rencontre, défi ou preuve crédible."
            }
        };
    }

    LegendArchiveEntry makeFriendlyTrioRumor()
    {
        return LegendArchiveEntry{
            "special_trio_friends_rumor",
            "Rumeur du trio qui transforme un test en bagarre",
            "Groupes et héros",
            "Arène / banc des spectateurs",
            "Une rumeur parle de Mattzelda, Louis et Trexof sans transformer leur rencontre en fiche gratuite.",
            {
                "Certains spectateurs prétendent avoir vu trois profils arriver ensemble : un mur qui plaisante, un artificier qui cherche des amis, et un testeur qui compte déjà les failles.",
                "Le trio ne ressemble pas à une compagnie officielle. Il ressemble plutôt à une mauvaise idée qui a appris à marcher droit pendant quelques minutes.",
                "La rumeur confirme seulement leur existence possible. Le registre gardera le reste tant que l'arène ne les aura pas vraiment placés devant toi.",
                "Dans Dinotofu, un nom spécial doit avoir une trace. Pas une révélation gratuite."
            }
        };
    }

    LegendArchiveEntry makeProtectedHestiaRumor()
    {
        return LegendArchiveEntry{
            "special_hestia_protected_rumor",
            "Rumeur de la mage qu'on ne laisse pas seule",
            "Groupes et héros",
            "Taverne / table trop silencieuse",
            "Une rumeur lie Hestia à Hazak et Sanctus sans dévoiler toute son histoire.",
            {
                "Les gens qui connaissent Hestia baissent la voix quand ils parlent d'elle.",
                "Ils disent qu'elle pourrait éviter presque tous les combats si la peur ne la poussait pas parfois en avant les yeux fermés.",
                "Hazak ne parle pas de compassion. Sanctus ne parle pas de peur. Pourtant, quand elle apparaît, il y a souvent quelqu'un entre elle et la violence.",
                "Le registre note cette protection comme une rumeur de groupe, pas comme une faiblesse tactique."
            }
        };
    }

    LegendArchiveEntry makeFireFlightCommandRumor()
    {
        return LegendArchiveEntry{
            "special_fireflight_command_rumor",
            "Rumeur du commandant qui ne comprend pas l'hostilité",
            "Groupes et héros",
            "Camp d'entraînement / salves lointaines",
            "Une rumeur présente Fire Flight comme meneur spécial avant ses vraies révélations de test final.",
            {
                "On raconte qu'un commandant observe parfois les combats comme s'il cherchait encore une raison de ne pas tirer.",
                "Il comprend les lignes, les salves et les positions. Il comprend beaucoup moins pourquoi des humains ou semi-humains choisissent d'être ennemis.",
                "Quand il donne un ordre, l'air se remplit de projectiles avant même qu'on sache si la diplomatie a officiellement échoué.",
                "Le registre garde cette rumeur séparée de son rôle final. Certaines vérités doivent arriver beaucoup plus tard."
            }
        };
    }


    LegendArchiveEntry makeCurseCounterRites()
    {
        return LegendArchiveEntry{
            "curse_counter_rites",
            "Les trois manières de rompre une trace",
            "Malédictions et rites",
            "Bibliothèque / rayon exorcisme",
            "Une note explique pourquoi certaines malédictions exigent autre chose qu'un exorcisme classique.",
            {
                "Les prêtres répètent souvent qu'une malédiction n'est pas toujours une maladie.",
                "Si elle vient d'un objet, il faut parfois isoler ou détruire l'objet source au lieu de purifier le porteur.",
                "Si elle vient d'un serment, il faut une vérité, un témoin ou une rupture symbolique : l'eau bénite seule ne suffit pas.",
                "Si elle vient d'une légende, la bibliothèque cherche la contre-version : l'histoire qui sait comment se terminer.",
                "Le registre garde cette règle sans donner de chiffres. Comprendre la famille d'une trace ne révèle pas encore son effet exact."
            }
        };
    }

    LegendArchiveEntry makeCurseCursedPatients()
    {
        return LegendArchiveEntry{
            "curse_cursed_patients",
            "Le patient n'est pas la malédiction",
            "Malédictions et PNJ",
            "Église / registre de Frère Calixte",
            "Une archive pose les bases des malédictions sur personnages non-joueurs.",
            {
                "Frère Calixte refuse d'écrire un nom de patient à côté d'un nom de malédiction avant diagnostic.",
                "Un habitant peut trembler, perdre le sommeil ou porter une aura étrange sans savoir ce qui l'atteint.",
                "L'église commence donc par des catégories vagues : santé, sommeil, esprit, présence sociale, équipement, corruption.",
                "Un mauvais diagnostic n'est pas inutile : il écarte des pistes et évite de traiter un innocent comme une énigme déjà résolue.",
                "Le registre rappelle surtout ceci : aider un PNJ maudit, ce n'est pas cliquer sur soigner. C'est comprendre ce qu'il porte."
            }
        };
    }

    LegendArchiveEntry makeLivingLibraryKeeper()
    {
        return LegendArchiveEntry{
            "legend_living_library_keeper",
            "Rumeur de la bibliothécaire qui classe les silences",
            "Déclencheurs et rumeurs",
            "Bibliothèque / comptoir des récits",
            "Une note explique que certains récits apparaissent par achat d'information, discussion ou retour après une rencontre marquante.",
            {
                "La bibliothécaire ne range pas seulement les livres : elle range aussi les choses que les aventuriers refusent d'avouer.",
                "Un combat trop étrange, un nom confirmé, une salle ancienne ou un PNJ bavard peut déplacer un récit du silence vers le registre.",
                "Elle prévient pourtant les joueurs pressés : acheter une rumeur ne doit pas donner la solution d'un boss.",
                "Une bonne archive donne une ambiance, une piste ou une mémoire. La faiblesse exacte, elle, se mérite sur le terrain."
            }
        };
    }

    LegendArchiveEntry makeTavernStorytellerRoute()
    {
        return LegendArchiveEntry{
            "legend_tavern_storyteller_route",
            "Rumeur du conteur qui change de table",
            "Déclencheurs et rumeurs",
            "Taverne / PNJ conteur",
            "Un conteur de passage annonce que certaines légendes pourront être racontées par PNJ plutôt que par menu fixe.",
            {
                "Dans certaines tavernes, un vieux conteur commence toujours par dire qu'il ne connaît rien.",
                "Puis il décrit une cicatrice, un emblème, une salle ou un groupe avec assez de détails pour que le registre se mette à gratter tout seul.",
                "Il ne raconte jamais tout. Un bon conteur garde une porte fermée pour que l'aventurier ait encore envie de l'ouvrir.",
                "Le système pourra plus tard utiliser ces PNJ comme déclencheurs doux : pas obligatoires, mais utiles pour donner vie au monde entre deux combats."
            }
        };
    }


    LegendArchiveEntry makeBreathingShelfRumor()
    {
        return LegendArchiveEntry{
            "legend_breathing_shelf",
            "Rumeur du rayonnage qui respire",
            "Déclencheurs et rumeurs",
            "Bibliothèque / étagère scellée",
            "Une étagère vivante rappelle que certains récits ne doivent sortir qu'après une vraie trace dans le monde.",
            {
                "Au fond de la bibliothèque, un rayonnage semble respirer quand un aventurier approche avec trop de questions.",
                "La bibliothécaire dit qu'il ne s'ouvre pas pour la curiosité, seulement pour les traces : un nom confirmé, une salle ancienne, une dette étrange ou un combat qui refuse de rester normal.",
                "Les pages y sont classées comme des créatures timides. Si on les force, elles deviennent fausses.",
                "La rumeur sert surtout de garde-fou : le lore doit surgir quand le voyage le mérite, pas quand le menu manque de décoration."
            }
        };
    }

    LegendArchiveEntry makeClosedDoorRumor()
    {
        return LegendArchiveEntry{
            "legend_closed_boss_door",
            "Note de la porte qui refuse le récit",
            "Règles du registre",
            "Salle ancienne / porte silencieuse",
            "Une note prévient que certaines salles de boss doivent parfois rester muettes pour garder leur mystère.",
            {
                "Toutes les portes anciennes ne parlent pas.",
                "Certaines savent qu'un avertissement trop précis devient une solution déguisée, et qu'une solution gratuite enlève la peur avant même le premier tour.",
                "Quand une porte reste silencieuse, ce n'est donc pas un bug du monde : c'est parfois sa manière de protéger le combat.",
                "Le registre notera ce silence plus tard, si le joueur survit assez longtemps pour lui donner un sens."
            }
        };
    }

    bool isSpecialArenaName(const std::string& name)
    {
        return name == "Matt (PRO)"
            || name == "Skuro"
            || name == "Sanctus"
            || name == "Hestia"
            || name == "Fire Flight"
            || name == "Louis"
            || name == "Hazak"
            || name == "Fail"
            || name == "Aoi"
            || name == "Kanadé"
            || name == "Trexof"
            || name == "Mattzelda"
            || name == "Henrique";
    }

    LegendArchiveEntry makeBossRoomGeneric(const Boss& boss)
    {
        return LegendArchiveEntry{
            "boss_room_generic_" + std::to_string(boss.getBossId()),
            "Légende murmurée avant l'arène",
            "Salles de boss",
            "Salle de boss / inscription abîmée",
            "Une salle ancienne laisse parfois une rumeur avant un combat important.",
            {
                "La pierre autour de l'arène ne donne pas toujours un nom.",
                "Parfois, elle donne seulement une peur ancienne, un avertissement, ou une phrase que personne n'a osé graver jusqu'au bout.",
                "L'entité devant toi n'est pas seulement une statistique. Elle a laissé assez de traces pour que le lieu se souvienne d'elle.",
                "Le registre note la rumeur, mais il refuse encore d'en faire une vérité complète."
            }
        };
    }

    LegendArchiveEntry makeBossAzelanosCrown()
    {
        return LegendArchiveEntry{
            "boss_azelanos_cracked_crown",
            "Légende de la couronne fissurée",
            "Salles de boss",
            "Salle de boss / couronne noire",
            "Une vieille salle raconte qu'un roi démon affaibli reste dangereux tant qu'il croit encore porter sa couronne.",
            {
                "Au centre de la salle, une marque circulaire ressemble moins à un trône qu'à une cicatrice.",
                "La légende dit qu'Azelanos n'a pas perdu sa couronne d'un seul coup : elle s'est fissurée chaque fois qu'un serviteur a cessé d'avoir peur.",
                "Pourtant, un fragment suffit parfois à faire plier les ombres autour de lui.",
                "Le registre classe cette trace comme avertissement : une forme affaiblie peut encore se comporter comme un roi si personne n'ose lui rappeler sa chute."
            }
        };
    }

    LegendArchiveEntry makeBossAnomalyFalseMenu()
    {
        return LegendArchiveEntry{
            "boss_anomaly_false_menu",
            "Rumeur du menu qui mentait",
            "Salles de boss",
            "Salle de boss / interface griffée",
            "Une rumeur prévient que certaines anomalies ne frappent pas seulement le personnage, mais aussi la confiance dans l'interface.",
            {
                "Sur une dalle, quelqu'un a gravé plusieurs numéros, puis les a barrés avec colère.",
                "La rumeur dit qu'une option peut garder son visage tout en changeant son intention, ou inversement.",
                "Les survivants ne conseillent pas de paniquer : ils conseillent de lire, relire, puis accepter que le monde puisse tricher avec style.",
                "Le registre ne donne pas la solution. Il rappelle seulement qu'une interface trop propre peut devenir suspecte quand l'Anomalie s'ennuie."
            }
        };
    }

    LegendArchiveEntry makeBossRoomSpecific(const Boss& boss)
    {
        switch (boss.getBossId())
        {
            case 4:
                return LegendArchiveEntry{
                    "boss_lyknir_pack_echo",
                    "Légende de la meute sans lune",
                    "Salles de boss",
                    "Salle de boss / griffures anciennes",
                    "Un écho ancien parle d'une meute qui n'avait plus besoin de voir la lune pour chasser.",
                    {
                        "Les griffures sur les murs ne sont pas placées au hasard.",
                        "Elles dessinent une meute qui tourne autour d'une proie déjà condamnée par la peur.",
                        "Une vieille phrase survit sous la poussière : le loup ne tue pas seulement avec ses crocs, il tue quand la proie croit être seule.",
                        "Le registre garde cette légende comme un avertissement, pas comme une faiblesse gratuite."
                    }
                };

            case 5:
                return LegendArchiveEntry{
                    "boss_grinka_tax_crown",
                    "Légende de la couronne des petites dettes",
                    "Salles de boss",
                    "Salle de boss / contrat gobelin",
                    "Une rumeur raconte que certaines reines ne volent pas l'or : elles inventent d'abord une raison de le prendre.",
                    {
                        "Un parchemin gobelin pend à un clou, couvert de taxes illisibles.",
                        "La légende dit qu'une reine peut posséder une salle entière sans jamais y construire un mur : il suffit que tout le monde lui doive quelque chose.",
                        "Chaque dette devient une marche. Chaque marche devient une couronne.",
                        "Le registre note surtout ceci : un gobelin organisé est plus dangereux qu'un gobelin courageux."
                    }
                };

            case 6:
                return makeBossAzelanosCrown();

            case 7:
                return LegendArchiveEntry{
                    "boss_thamarys_scale_fragment",
                    "Légende de l'écaille qui refuse de tomber",
                    "Salles de boss",
                    "Salle de boss / fragment draconique",
                    "Un récit ancien parle d'une écaille divine restée debout après le corps qui la portait.",
                    {
                        "Le sol garde des marques de griffes trop grandes pour cette arène.",
                        "Dans certains contes draconiques, une écaille n'est pas un bout d'armure : c'est une promesse que le corps refuse de trahir.",
                        "On dit qu'une vraie écaille ne cède pas au premier coup fort, mais au moment où le combattant comprend ce qu'il frappe.",
                        "La légende s'arrête là. Le reste doit se découvrir en survivant."
                    }
                };

            case 8:
                return LegendArchiveEntry{
                    "boss_mojo_forest_memory",
                    "Légende de la forêt qui juge sans parler",
                    "Salles de boss",
                    "Salle de boss / racines gravées",
                    "Une légende sylvestre rappelle que toutes les victoires contre la nature ne doivent pas être des massacres.",
                    {
                        "Des racines sortent du sol comme des lignes d'écriture.",
                        "Elles racontent une forêt qui ne voulait pas gagner, seulement savoir qui marchait encore avec respect.",
                        "Le conte ne dit pas qu'il faut être doux avec tout. Il dit seulement que certains lieux se souviennent de la manière dont on triomphe.",
                        "Le registre classe cette trace comme légende, pas comme ordre."
                    }
                };

            case 11:
                return makeBossAnomalyFalseMenu();

            case 27:
                return LegendArchiveEntry{
                    "boss_fireflight_creator_gate",
                    "Légende de la porte du créateur",
                    "Salles de boss",
                    "Salle finale / phrase impossible",
                    "Une rumeur très rare parle d'un créateur qui teste moins la force que la cohérence du voyage.",
                    {
                        "La salle ne ressemble pas vraiment à une salle.",
                        "Elle ressemble à une page qui hésite entre accueillir le joueur et corriger son personnage.",
                        "On raconte que le créateur ne demande pas seulement qui gagne, mais qui mérite que le monde accepte cette victoire.",
                        "Si la légende apparaît ici, ce n'est pas une récompense. C'est un rappel : la fin regarde tout ce qui l'a précédée."
                    }
                };

            default:
                return makeBossRoomGeneric(boss);
        }
    }

    const LegendArchiveEntry* findEntryById(const std::string& id, const std::vector<LegendArchiveEntry>& entries)
    {
        for (const LegendArchiveEntry& entry : entries)
        {
            if (entry.id == id)
            {
                return &entry;
            }
        }

        return nullptr;
    }
}

std::vector<LegendArchiveEntry> LegendTriggerSystem::getArchiveEntries()
{
    return {
        makeBrasCassesOrigin(),
        makeChildTale(),
        makeSpecialArenaRumor(),
        makeFriendlyTrioRumor(),
        makeProtectedHestiaRumor(),
        makeFireFlightCommandRumor(),
        makeCurseCounterRites(),
        makeCurseCursedPatients(),
        makeLivingLibraryKeeper(),
        makeTavernStorytellerRoute(),
        makeBreathingShelfRumor(),
        makeClosedDoorRumor(),
        LegendArchiveEntry{
            "boss_lyknir_pack_echo",
            "Légende de la meute sans lune",
            "Salles de boss",
            "Salle de boss / griffures anciennes",
            "Un écho ancien parle d'une meute qui n'avait plus besoin de voir la lune pour chasser.",
            {
                "Les griffures sur les murs ne sont pas placées au hasard.",
                "Elles dessinent une meute qui tourne autour d'une proie déjà condamnée par la peur.",
                "Une vieille phrase survit sous la poussière : le loup ne tue pas seulement avec ses crocs, il tue quand la proie croit être seule.",
                "Le registre garde cette légende comme un avertissement, pas comme une faiblesse gratuite."
            }
        },
        LegendArchiveEntry{
            "boss_grinka_tax_crown",
            "Légende de la couronne des petites dettes",
            "Salles de boss",
            "Salle de boss / contrat gobelin",
            "Une rumeur raconte que certaines reines ne volent pas l'or : elles inventent d'abord une raison de le prendre.",
            {
                "Un parchemin gobelin pend à un clou, couvert de taxes illisibles.",
                "La légende dit qu'une reine peut posséder une salle entière sans jamais y construire un mur : il suffit que tout le monde lui doive quelque chose.",
                "Chaque dette devient une marche. Chaque marche devient une couronne.",
                "Le registre note surtout ceci : un gobelin organisé est plus dangereux qu'un gobelin courageux."
            }
        },
        makeBossAzelanosCrown(),
        LegendArchiveEntry{
            "boss_thamarys_scale_fragment",
            "Légende de l'écaille qui refuse de tomber",
            "Salles de boss",
            "Salle de boss / fragment draconique",
            "Un récit ancien parle d'une écaille divine restée debout après le corps qui la portait.",
            {
                "Le sol garde des marques de griffes trop grandes pour cette arène.",
                "Dans certains contes draconiques, une écaille n'est pas un bout d'armure : c'est une promesse que le corps refuse de trahir.",
                "On dit qu'une vraie écaille ne cède pas au premier coup fort, mais au moment où le combattant comprend ce qu'il frappe.",
                "La légende s'arrête là. Le reste doit se découvrir en survivant."
            }
        },
        LegendArchiveEntry{
            "boss_mojo_forest_memory",
            "Légende de la forêt qui juge sans parler",
            "Salles de boss",
            "Salle de boss / racines gravées",
            "Une légende sylvestre rappelle que toutes les victoires contre la nature ne doivent pas être des massacres.",
            {
                "Des racines sortent du sol comme des lignes d'écriture.",
                "Elles racontent une forêt qui ne voulait pas gagner, seulement savoir qui marchait encore avec respect.",
                "Le conte ne dit pas qu'il faut être doux avec tout. Il dit seulement que certains lieux se souviennent de la manière dont on triomphe.",
                "Le registre classe cette trace comme légende, pas comme ordre."
            }
        },
        makeBossAnomalyFalseMenu(),
        LegendArchiveEntry{
            "boss_fireflight_creator_gate",
            "Légende de la porte du créateur",
            "Salles de boss",
            "Salle finale / phrase impossible",
            "Une rumeur très rare parle d'un créateur qui teste moins la force que la cohérence du voyage.",
            {
                "La salle ne ressemble pas vraiment à une salle.",
                "Elle ressemble à une page qui hésite entre accueillir le joueur et corriger son personnage.",
                "On raconte que le créateur ne demande pas seulement qui gagne, mais qui mérite que le monde accepte cette victoire.",
                "Si la légende apparaît ici, ce n'est pas une récompense. C'est un rappel : la fin regarde tout ce qui l'a précédée."
            }
        },
        LegendArchiveEntry{
            "legend_camp_abandoned_pot",
            "Rumeur de la marmite encore tiède",
            "Déclencheurs et rumeurs",
            "Campement abandonné / feu couvert de cendres",
            "Une petite rumeur de route explique qu'un camp vide peut être plus inquiétant qu'un camp hostile.",
            {
                "La marmite ne bout plus, mais elle n'est pas froide.",
                "Dans les récits de caravane, ce détail suffit à faire taire les plus bruyants : quelqu'un est parti trop vite, ou quelqu'un attend que tu poses la mauvaise question.",
                "Les vieux pisteurs ne disent pas de fuir. Ils disent de compter les bols, les empreintes et les sacs encore fermés.",
                "Le registre classe cette scène comme trace de terrain : petite, mais assez nette pour mériter prudence."
            }
        },
        LegendArchiveEntry{
            "legend_bridge_three_tolls",
            "Conte du pont aux trois péages",
            "Contes pour enfant",
            "Bibliothèque / histoire de route",
            "Un conte drôle transforme les taxes de pont, les gobelins et les mauvais négociateurs en leçon de survie.",
            {
                "Le premier péage demandait une pièce. Le second demandait deux pièces. Le troisième demandait le sac entier, parce que le gobelin avait appris les mathématiques chez Grinka.",
                "Un enfant demanda pourquoi le héros ne passait pas à la nage. Le conteur répondit que les rivières taxent aussi, mais avec des dents.",
                "La morale change selon la région : négocie avant le pont, combats après l'embuscade, et ne donne jamais ton meilleur couteau à un gobelin qui sait écrire.",
                "Le registre garde le conte parce qu'il explique mieux la route commerciale que certains rapports officiels."
            }
        },
        LegendArchiveEntry{
            "legend_class_wall_of_names",
            "Mur des classes trop nombreuses",
            "Règles du registre",
            "Salle d'inscription / plaques de bois",
            "Une note justifie pourquoi les classes sont rangées par familles plutôt qu'affichées en un seul mur illisible.",
            {
                "Le mur d'inscription a déjà essayé d'afficher toutes les classes d'un coup.",
                "Trois apprentis se sont perdus avant d'avoir choisi une arme, un mage a signé dans la mauvaise case, et un forgeron a demandé si le mur pouvait être réparé avec du cuir.",
                "Depuis, la guilde range les profils par familles : contact, distance, magie, invocation, soutien, hybride et artisanat.",
                "Ce n'est pas moins complet. C'est juste plus humain pour ceux qui veulent revenir en arrière avant de choisir leur avenir."
            }
        },
        LegendArchiveEntry{
            "legend_guild_cook_first_soup",
            "Conte de la soupe qui sauva une expédition",
            "Groupes et héros",
            "Cuisine de guilde / louche cabossée",
            "Une histoire donne de la valeur aux classes artisanales et aux préparations avant le combat.",
            {
                "Personne ne voulait engager le cuisinier. Il ne portait ni épée rare, ni grimoire, ni armure brillante.",
                "Puis la neige a bloqué le col, les potions ont gelé, et le héros le plus puissant du groupe a failli s'évanouir avant même de voir le monstre.",
                "Le cuisinier a sorti une soupe épaisse, trop salée, presque insultante. Tout le monde a survécu assez longtemps pour atteindre l'abri.",
                "Depuis, certains vétérans disent qu'une ration réussie vaut parfois mieux qu'un critique magnifique lancé trop tard."
            }
        },
        LegendArchiveEntry{
            "legend_recycler_three_nails",
            "Rumeur du récupérateur aux trois clous",
            "Déclencheurs et rumeurs",
            "Atelier de guilde / caisse de rebut",
            "Une rumeur valorise les matériaux faibles, les crafts sales et les personnages qui gardent tout.",
            {
                "Le récupérateur jurait pouvoir réparer une armure avec trois clous, une lanière et l'orgueil d'un chevalier.",
                "Il avait tort pour l'orgueil : il en fallait deux doses.",
                "Mais l'armure a tenu jusqu'à la porte suivante, ce qui suffit parfois à transformer une blague en métier.",
                "Le registre rappelle ici qu'un matériau médiocre n'est pas inutile. Il attend juste une bonne mauvaise idée."
            }
        },
        LegendArchiveEntry{
            "legend_prudent_storyteller",
            "Rumeur du conteur prudent",
            "Déclencheurs et rumeurs",
            "Bibliothèque / note de conteur",
            "Un rappel méta-lore : une légende doit apparaître quand elle a du sens, pas parce qu'un menu existe.",
            {
                "Les vieux conteurs ont une règle simple : une bonne légende ne se répète pas à chaque porte.",
                "Elle attend une salle marquée, un PNJ qui a vraiment quelque chose à dire, ou une rumeur assez étrange pour survivre au bruit de la taverne.",
                "Le registre préfère une trace rare qui reste en tête à dix textes forcés qu'on clique sans lire.",
                "Si rien ne mérite d'être raconté, le silence fait aussi partie du monde."
            }
        },
        LegendArchiveEntry{
            "legend_optional_lore_rule",
            "Note : légendes sans obligation",
            "Déclencheurs et rumeurs",
            "Bibliothèque / règle de lecture",
            "Le lore doit enrichir l'aventure sans bloquer le joueur qui veut juste avancer.",
            {
                "Une légende peut donner du contexte, une ambiance, une peur ou une piste.",
                "Elle ne doit pas devenir un péage obligatoire avant chaque combat, ni révéler gratuitement les secrets d'un boss.",
                "Le joueur peut donc l'ignorer pendant l'action, puis revenir la lire plus tard depuis le bestiaire.",
                "C'est pour ça que le registre existe : garder la mémoire du monde sans ralentir la partie."
            }
        },
        LegendArchiveEntry{
            "rare_boss_room_rule",
            "Règle des légendes de salle",
            "Règles du registre",
            "Bibliothèque / notes de registre",
            "Principe du système : les salles et PNJ peuvent raconter une légende, mais pas systématiquement.",
            {
                "Les légendes de Dinotofu doivent rester des moments rares.",
                "Un PNJ, une bibliothèque, une salle de boss ou une rumeur peut en raconter une, mais le jeu ne doit pas forcer une lecture avant chaque combat.",
                "Le joueur doit pouvoir ignorer le lore sans casser sa progression, puis revenir le lire quand il en a envie.",
                "Le registre classe donc ces récits comme traces optionnelles, pas comme objectifs obligatoires."
            }
        },
        LegendArchiveEntry{
            "legend_hammer_that_remembered",
            "Conte du marteau qui se souvenait trop",
            "Contes pour enfant",
            "Bibliothèque / rayon artisanat",
            "Petit conte autour des armes, de l'usure et des réparations qui coûtent moins cher que l'orgueil.",
            {
                "Un apprenti jurait que son marteau était incassable, parce qu'il n'avait jamais encore frappé quelque chose de plus dur que lui.",
                "Le vieux forgeron le laissa partir. Le soir même, le marteau revint en deux morceaux et l'apprenti en une seule leçon.",
                "Depuis, les enfants apprennent qu'un outil courageux reste un outil : il faut l'entretenir avant de lui demander de sauver une vie.",
                "La morale plaît aux forgerons, beaucoup moins aux héros qui préfèrent appeler ça une malédiction."
            }
        },
        LegendArchiveEntry{
            "legend_bookshelf_bites_back",
            "Rumeur de l'étagère qui rend les coups",
            "Déclencheurs et rumeurs",
            "Ruines effondrées / archive ouverte",
            "Une rumeur justifie les constructions étranges et les archives hostiles ajoutées au contenu.",
            {
                "Dans certaines ruines, les livres ne sont pas protégés par des gardes. Ils sont les gardes.",
                "Un aventurier trop pressé aurait frappé une étagère pour l'ouvrir plus vite. Elle lui aurait rendu la politesse avec trois volumes et une écharde.",
                "Les érudits appellent cela une défense mémorielle. Les guerriers appellent cela un meuble énervé.",
                "Le registre conseille de lire la pièce avant de frapper le mobilier."
            }
        },
        LegendArchiveEntry{
            "legend_false_toll_smile",
            "Rumeur du péager qui souriait trop",
            "Déclencheurs et rumeurs",
            "Route commerciale / borne cassée",
            "Une rumeur de route explique l'arrivée de nouveaux faux péagers et bandits administratifs.",
            {
                "Il portait un tampon, une corde et un sourire beaucoup trop officiel pour être honnête.",
                "Il réclamait une taxe de passage, une taxe de retour, puis une taxe de contestation de taxe.",
                "Le marchand paya la première. L'aventurier répondit à la seconde. Le pont n'avait jamais été aussi calme après cela.",
                "La guilde classe ce genre d'individu comme menace sociale avant menace martiale, ce qui ne l'empêche pas de mordre."
            }
        },
        LegendArchiveEntry{
            "legend_silent_silver_slime",
            "Note sur le slime d'argent silencieux",
            "Déclencheurs et rumeurs",
            "Mares gélatineuses / surface trop lisse",
            "Une note de terrain donne une ambiance aux slimes rares sans révéler une solution parfaite.",
            {
                "Les chasseurs disent qu'un slime normal fait au moins le bruit d'un sac mouillé quand il approche.",
                "Celui-là ne fait rien. Pas un clapotis, pas une bulle, pas même un bruit de digestion.",
                "On ne sait pas s'il absorbe les sons ou s'il déteste simplement prévenir ses proies.",
                "Le registre recommande de surveiller l'eau quand elle devient trop polie pour être naturelle."
            }
        },
        LegendArchiveEntry{
            "legend_mosquito_queen_fever",
            "Rumeur de la reine moustique de fièvre",
            "Déclencheurs et rumeurs",
            "Marais trouble / moustiquaire trouée",
            "Un récit court pour donner de la présence aux insectoïdes rares du marais.",
            {
                "Les anciens du marais ne craignent pas le bourdonnement. Ils craignent le moment où tous les bourdonnements s'accordent.",
                "On dit alors qu'une reine passe, et que même les fioles bien bouchées commencent à transpirer.",
                "La rumeur ne dit pas de fuir le marais. Elle dit de compter ses antidotes avant de trouver le nid.",
                "Ceux qui oublient reviennent souvent avec plus de fièvre que d'héroïsme."
            }
        },
        LegendArchiveEntry{
            "legend_archive_field_worker",
            "Note de l'archiviste de terrain",
            "Règles du registre",
            "Comptoir de guilde / carnet taché",
            "Une note méta-lore explique pourquoi les classes de connaissance ont une place dans l'aventure.",
            {
                "L'archiviste de terrain n'a pas besoin de tuer plus fort que les autres pour être utile.",
                "Il sait reconnaître une trace, dater une morsure, lire un symbole et éviter qu'un groupe confonde une faiblesse avec une superstition.",
                "Dans un monde où les monstres évoluent, oublier d'écrire peut tuer autant qu'oublier de parer.",
                "Le registre considère donc le savoir comme une ressource, pas comme une décoration."
            }
        }
    };
}

void LegendTriggerSystem::displayArchiveEntry(const std::string& id)
{
    const std::vector<LegendArchiveEntry> entries = getArchiveEntries();
    const LegendArchiveEntry* entry = findEntryById(id, entries);

    if (entry == nullptr)
    {
        MessageScreen::show(
            "LÉGENDE INTROUVABLE",
            "legend.archive.missing",
            {
                "Cette entrée n'existe pas encore dans les archives.",
                "Elle pourra être ajoutée plus tard si le monde garde une trace assez nette."
            }
        );
        return;
    }

    showLegend(*entry, "legend.archive." + id, true);
}

void LegendTriggerSystem::maybeDisplayBossRoomLegend(const Boss& boss, Random& random)
{
    int chance = 14;

    if (boss.getBossId() == 4 || boss.getBossId() == 5 || boss.getBossId() == 6 || boss.getBossId() == 7 || boss.getBossId() == 8 || boss.getBossId() == 11)
    {
        chance = boss.getBossId() == 11 ? 24 : 22;
    }
    else if (boss.getBossId() == 27)
    {
        chance = 30;
    }

    if (random.between(1, 100) > chance)
    {
        return;
    }

    LegendArchiveEntry entry = makeBossRoomSpecific(boss);
    const std::string triggerId = "boss_room." + entry.id;

    if (!reserveAutomaticLegendDisplay(triggerId, entry))
    {
        return;
    }

    showLegend(
        entry,
        "legend.trigger.boss_room." + std::to_string(boss.getBossId()),
        false
    );
}

void LegendTriggerSystem::maybeDisplaySpecialGroupLegend(const std::vector<std::string>& names, Random& random)
{
    if (names.empty())
    {
        return;
    }

    if (hasLesBrasCasses(names))
    {
        if (random.between(1, 100) <= 35)
        {
            LegendArchiveEntry entry = makeBrasCassesOrigin();
            if (reserveAutomaticLegendDisplay("group." + entry.id, entry))
            {
                showLegend(entry, "legend.trigger.group.bras_casses_origin", false);
            }
        }
        return;
    }

    if (containsName(names, "Mattzelda") && containsName(names, "Louis") && containsName(names, "Trexof") && random.between(1, 100) <= 22)
    {
        LegendArchiveEntry entry = makeFriendlyTrioRumor();
        if (reserveAutomaticLegendDisplay("trio." + entry.id, entry))
        {
            showLegend(entry, "legend.trigger.group.friendly_trio", false);
        }
        return;
    }

    if (containsName(names, "Hazak") && containsName(names, "Hestia") && random.between(1, 100) <= 24)
    {
        LegendArchiveEntry entry = makeProtectedHestiaRumor();
        if (reserveAutomaticLegendDisplay("hestia." + entry.id, entry))
        {
            showLegend(entry, "legend.trigger.group.protected_hestia", false);
        }
        return;
    }

    if (containsName(names, "Fire Flight") && random.between(1, 100) <= 16)
    {
        LegendArchiveEntry entry = makeFireFlightCommandRumor();
        if (reserveAutomaticLegendDisplay("fireflight." + entry.id, entry))
        {
            showLegend(entry, "legend.trigger.group.fireflight_command", false);
        }
        return;
    }

    if (containsName(names, "Hazak") && random.between(1, 100) <= 12)
    {
        LegendArchiveEntry entry = makeBrasCassesOrigin();
        if (reserveAutomaticLegendDisplay("hazak." + entry.id, entry))
        {
            showLegend(entry, "legend.trigger.group.hazak_arm_rumor", false);
        }
    }
}

void LegendTriggerSystem::maybeDisplaySpecialOpponentLegend(const std::string& name, Random& random)
{
    if (!isSpecialArenaName(name))
    {
        return;
    }

    if (random.between(1, 100) > 18)
    {
        return;
    }

    LegendArchiveEntry entry = makeSpecialArenaRumor();
    if (!reserveAutomaticLegendDisplay("arena." + entry.id, entry))
    {
        return;
    }

    showLegend(entry, "legend.trigger.arena.special_opponent", true);
}

void LegendTriggerSystem::maybeDisplayLibraryLoreWhisper(Random& random)
{
    const bool storytellerRouteKnown = BestiaryRuntimeProgress::getEncounterCount("Rumeur du conteur qui change de table") > 0
        || BestiaryRuntimeProgress::getEncounterCount("Rumeur de la bibliothécaire qui classe les silences") > 0;

    if (!storytellerRouteKnown)
    {
        return;
    }

    if (random.between(1, 100) > 14)
    {
        return;
    }

    LegendArchiveEntry entry = random.between(1, 100) <= 55
        ? makeBreathingShelfRumor()
        : makeClosedDoorRumor();

    if (!reserveAutomaticLegendDisplay("library." + entry.id, entry))
    {
        return;
    }

    showLegend(entry, "legend.trigger.library.whisper", true);
}

