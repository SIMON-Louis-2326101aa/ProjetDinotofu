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

    if (boss.getBossId() == 4 || boss.getBossId() == 5 || boss.getBossId() == 7 || boss.getBossId() == 8)
    {
        chance = 22;
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

    if (containsName(names, "Hazak") && random.between(1, 100) <= 12)
    {
        LegendArchiveEntry entry = makeBrasCassesOrigin();
        if (reserveAutomaticLegendDisplay("hazak." + entry.id, entry))
        {
            showLegend(entry, "legend.trigger.group.hazak_arm_rumor", false);
        }
    }
}
