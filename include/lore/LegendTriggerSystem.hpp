// EN: LegendTriggerSystem.hpp defines optional lore legend triggers for Dinotofu.
// FR: LegendTriggerSystem.hpp définit les déclencheurs optionnels de légendes pour Dinotofu.
// English: Code identifiers are written in English; player-facing text can stay in French.
// Français : Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_LORE_LEGENDTRIGGERSYSTEM_HPP
#define INCLUDE_LORE_LEGENDTRIGGERSYSTEM_HPP

#include "core/Random.hpp"
#include "entity/Boss.hpp"

#include <string>
#include <vector>

struct LegendArchiveEntry
{
    std::string id;
    std::string title;
    std::string category;
    std::string source;
    std::string shortDescription;
    std::vector<std::string> lines;
};

class LegendTriggerSystem
{
public:
    // EN: Returns all readable legend snippets already safe for the archive.
    // FR: Renvoie les récits lisibles déjà sûrs pour l'archive.
    static std::vector<LegendArchiveEntry> getArchiveEntries();

    // EN: Displays a specific archive entry if it exists.
    // FR: Affiche une entrée d'archive précise si elle existe.
    static void displayArchiveEntry(const std::string& id);

    // EN: Rarely displays a short boss-room legend before the fight.
    // FR: Affiche rarement une courte légende de salle de boss avant le combat.
    static void maybeDisplayBossRoomLegend(const Boss& boss, Random& random);

    // EN: Rarely displays a short special group legend before a PvE group encounter.
    // FR: Affiche rarement une courte légende de groupe spécial avant une rencontre PvE de groupe.
    static void maybeDisplaySpecialGroupLegend(const std::vector<std::string>& names, Random& random);

    // EN: Rarely displays an arena rumor when a special AI opponent is encountered.
    // FR: Affiche rarement une rumeur d'arène quand un adversaire IA spécial apparaît.
    static void maybeDisplaySpecialOpponentLegend(const std::string& name, Random& random);

    // EN: Rarely displays a soft library/tavern lore whisper after the matching archive is unlocked.
    // FR: Affiche rarement une rumeur douce de bibliothèque/taverne après déblocage de l'archive correspondante.
    static void maybeDisplayLibraryLoreWhisper(Random& random);
};

#endif
