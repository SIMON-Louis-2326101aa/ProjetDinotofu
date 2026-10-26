#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

static std::string recruitLabel(const std::string& name, const std::string& job, int level, int rank, bool profileOnly = false)
{
    return name + " | race=Humain | profil=" + job + " | trait=calme | niveau_recrutement=" + std::to_string(level)
        + " | rang_origine_index=" + std::to_string(rank) + " | part_base=18 | remise_fidelite=2"
        + (profileOnly ? " | profil_seulement=1" : "");
}

static bool contains(const std::vector<RecruitedAllyCombatSupport>& allies, const std::string& name)
{
    return std::any_of(allies.begin(), allies.end(), [&](const auto& ally) { return ally.name == name; });
}

int main()
{
    static_assert(RecruitedAllyCombatRoster::MaxSoloTeamMembers == 6);
    static_assert(RecruitedAllyCombatRoster::MaxRecruitedTeamMembers == 5);
    static_assert(RecruitedAllyCombatRoster::MaxActiveRecruitsWithPlayer == 2);
    static_assert(RecruitedAllyCombatRoster::MaxActiveRecruitsWithoutPlayer == 3);

    Player player;
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Aline", recruitLabel("Aline", "Soigneur", 20, 2), 1);
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Boros", recruitLabel("Boros", "Gardien", 26, 3), 1);
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Celia", recruitLabel("Celia", "Archer", 18, 2), 1);
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Doran", recruitLabel("Doran", "Assassin", 30, 3), 1);
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Eren", recruitLabel("Eren", "Lancier", 22, 2), 1);

    // Recruit XP is now real progression input: HQ missions can advance it slowly, real adventures much faster.
    player.recordCanonicalEvent("xp_recrues", "Aline", "Aline gagne de l'expérience réelle", 5000);

    // Faye n'est qu'un profil retenu : elle ne doit jamais entrer en combat ni en réserve.
    player.recordCanonicalEvent("pnj_recrutables_retenus", "Faye", recruitLabel("Faye", "Mage d'appui", 40, 4, true), 1);

    for (const char* name : {"Aline", "Boros", "Celia", "Doran"})
        player.recordCanonicalEvent("recrues_equipees_toggle", name, std::string(name) + " équipé", 1);

    player.recordCanonicalEvent("ordre_manuel_recrues", "mode", "manuel", 1);
    player.recordCanonicalEvent("ordre_manuel_recrues", "slot_1", "Celia", 1);
    player.recordCanonicalEvent("ordre_manuel_recrues", "slot_2", "Aline", 1);

    auto active = RecruitedAllyCombatRoster::collectActive(player);
    auto reserve = RecruitedAllyCombatRoster::collectReserve(player);
    assert(active.size() == 2);
    assert(active[0].name == "Celia");
    assert(active[1].name == "Aline");
    assert(active[1].estimatedLevel > 20);
    assert(reserve.size() == 3);
    assert(!contains(active, "Faye"));
    assert(!contains(reserve, "Faye"));
    assert(active[0].normalizedShare > 0);
    assert(active[1].normalizedShare > 0);

    // Une recrue indisponible sort de la ligne : la sauvegarde ne fait pas apparaître un simple CV à sa place.
    player.recordCanonicalEvent("recrues_infirmerie_sejours", "Celia", "Celia | pret_jour=5", 1);
    active = RecruitedAllyCombatRoster::collectActive(player);
    reserve = RecruitedAllyCombatRoster::collectReserve(player);
    assert(active.size() == 2); // Aline + meilleur autre toggle encore actif (Doran/Boros)
    assert(active[0].name == "Aline");
    assert(active[1].name == "Doran" || active[1].name == "Boros");
    assert(!contains(reserve, "Faye"));

    // Une fois réellement engagée, Faye devient membre valide, mais la cohorte de voyage reste plafonnée à 5 recrues.
    player.recordCanonicalEvent("recrues_engagees", "Faye", "Faye rejoint réellement le groupe", 1);
    reserve = RecruitedAllyCombatRoster::collectReserve(player);
    assert(active.size() + reserve.size() <= static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxRecruitedTeamMembers));

    // V3.51.44 : une entrée depuis la réserve peut verrouiller exactement UNE fenêtre d'action.
    // Le marqueur est transitoire : le premier contrôle le consomme, le suivant laisse agir normalement.
    RecruitedAllyCombatSupport entering;
    entering.name = "Test réserve";
    assert(!entering.enteredFromReserveThisPhase);
    RecruitedAllyCombatRoster::markReserveEntryDelay(entering);
    assert(entering.enteredFromReserveThisPhase);
    assert(RecruitedAllyCombatRoster::consumeReserveEntryDelay(entering));
    assert(!entering.enteredFromReserveThisPhase);
    assert(!RecruitedAllyCombatRoster::consumeReserveEntryDelay(entering));

    // V3.51.52: a temporary municipal guard can fight but must never dilute player/recruit loot shares.
    RecruitedAllyCombatSupport paidRecruit;
    paidRecruit.name = "Recrue rémunérée";
    paidRecruit.requestedShare = 18;
    paidRecruit.currentRankIndex = 2;
    paidRecruit.estimatedLevel = 20;
    std::vector<RecruitedAllyCombatSupport> rewardTeam{paidRecruit};
    const int playerShareWithoutGuard = RecruitedAllyCombatRoster::normalizeShares(rewardTeam);
    const int recruitShareWithoutGuard = rewardTeam.front().normalizedShare;

    RecruitedAllyCombatSupport municipalGuard;
    municipalGuard.name = "Garde municipal";
    municipalGuard.requestedShare = 99; // Deliberately huge: must still be ignored.
    municipalGuard.currentRankIndex = 5;
    municipalGuard.estimatedLevel = 99;
    municipalGuard.temporaryNoReward = true;
    rewardTeam.push_back(municipalGuard);
    const int playerShareWithGuard = RecruitedAllyCombatRoster::normalizeShares(rewardTeam);
    assert(playerShareWithGuard == playerShareWithoutGuard);
    assert(rewardTeam[0].normalizedShare == recruitShareWithoutGuard);
    assert(rewardTeam[1].normalizedShare == 0);

    return 0;
}
