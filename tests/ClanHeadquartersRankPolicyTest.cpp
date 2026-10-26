#include "world/clan/ClanHeadquartersSystem.hpp"

#include "entity/Player.hpp"

#include <algorithm>
#include <cassert>
#include <string>

namespace
{
    std::string recruitLabel(const std::string& name, int rankIndex)
    {
        return name + " | race=Humain | profil=Éclaireur | trait=fiable | niveau_recrutement=8"
            + " | rang_origine_index=" + std::to_string(rankIndex)
            + " | part_base=18 | salaire=90 | profil_seulement=1";
    }

    void addRecruit(Player& player, const std::string& name, int rankIndex)
    {
        player.recordCanonicalEvent("pnj_recrutables_retenus", name, recruitLabel(name, rankIndex), 1);
        player.recordCanonicalEvent("recrues_engagees", name, name + " rejoint réellement le groupe", 1);
        std::string reason;
        assert(ClanHeadquartersSystem::assignRecruit(player, name, reason));
    }
}

int main()
{
    Player player;
    player.recordCanonicalEvent(
        "clan_hq_state",
        "built",
        "city=valebrume | built_day=0 | building_level=1 | beds=2 | quest_board=1 | counter=1",
        1
    );
    player.recordCanonicalEvent("clan_hq_building_expansions", "level_2", "Agrandissement général terminé : QG niveau 2", 1);
    player.recordCanonicalEvent("clan_hq_extra_beds", "bed_3", "Chambre 3", 1);
    player.recordCanonicalEvent("clan_hq_extra_beds", "bed_4", "Chambre 4", 1);
    player.recordCanonicalEvent("clan_hq_extra_beds", "bed_5", "Chambre 5", 1);
    player.recordCanonicalEvent("clan_hq_quest_board_upgrades", "level_2", "Tableau niveau 2", 1);
    player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Horloge initialisée", 1);

    // 4xE + 1xF => average 0.8, rounded to E. It is close to the best rank E,
    // therefore a five-member group may attempt average + 1 => D.
    addRecruit(player, "Aline", 1);
    addRecruit(player, "Boros", 1);
    addRecruit(player, "Celia", 1);
    addRecruit(player, "Daro", 1);
    addRecruit(player, "Ena", 0);
    assert(ClanHeadquartersSystem::residentRecruitNames(player).size() == 5);

    player.advanceWorldDays(3);
    const ClanHeadquartersPassiveReport report = ClanHeadquartersSystem::processPassiveActivity(player);
    assert(report.missionCycles == 1);
    assert(report.missionsCompleted == 5);

    int rankDLines = 0;
    int averageLines = 0;
    for (const std::string& line : report.lines)
    {
        if (line.find("contrat(s) rang D") != std::string::npos) ++rankDLines;
        if (line.find("plafond calculé sur la moyenne du groupe") != std::string::npos) ++averageLines;
    }
    assert(rankDLines == 5);
    assert(averageLines == 5);

    return 0;
}
