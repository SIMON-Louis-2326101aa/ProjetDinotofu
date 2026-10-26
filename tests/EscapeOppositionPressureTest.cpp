#include "combat/system/EscapeSystem.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"

#include <cassert>

int main()
{
    assert(EscapeSystem::escapeOpeningsRequired(1) == 1);
    assert(EscapeSystem::escapeOpeningsRequired(3) == 1);
    assert(EscapeSystem::escapeOpeningsRequired(4) == 1);
    assert(EscapeSystem::escapeOpeningsRequired(5) == 2);
    assert(EscapeSystem::escapeOpeningsRequired(8) == 3);

    // Même chance par ouverture, mais cinq opposants imposent deux réussites successives.
    // On cherche une graine où la première ouverture passe et la seconde non : le même fuyard
    // réussit donc face à un seul adversaire mais échoue face à cinq.
    bool foundSymmetricPressureExample = false;
    for (unsigned int seed = 1; seed < 5000 && !foundSymmetricPressureExample; ++seed)
    {
        Random oneOpponent(seed);
        Random fiveOpponents(seed);
        const bool vsOne = EscapeSystem::rollEscapeAgainstOpposition(oneOpponent, 45, 1);
        const bool vsFive = EscapeSystem::rollEscapeAgainstOpposition(fiveOpponents, 45, 5);
        if (vsOne && !vsFive) foundSymmetricPressureExample = true;
    }
    assert(foundSymmetricPressureExample);

    // Le gabarit compte aussi : petit/agile > profil lourd/massif, à pression égale.
    Monster nimble(
        "Fée éclaireuse", "Petite créature agile et légère", Race::Fee,
        5, 80, 4, 10, 15, 0, 0);
    Monster heavy(
        "Golem sentinelle", "Construction massive lourde et tank", Race::Construction,
        5, 280, 10, 22, 30, 0, 0);
    assert(EscapeSystem::monsterEscapeMobilityModifier(nimble)
        > EscapeSystem::monsterEscapeMobilityModifier(heavy));

    // Face à cinq opposants, une première ouverture réussie ne téléporte pas le monstre hors du
    // combat : elle crée une avance persistante qui augmente fortement la tentative suivante.
    bool foundProgressiveEscape = false;
    for (unsigned int seed = 1; seed < 10000 && !foundProgressiveEscape; ++seed)
    {
        Monster runner(
            "Rat de fuite", "Petite bête nerveuse agile", Race::Bete,
            4, 100, 3, 8, 12, 0, 0);
        runner.takeDamage(92); // <= 10 % PV : fuite faible-PV autorisée.
        Random random(seed);

        const MonsterEscapeAttemptResult first = EscapeSystem::attemptMonsterEscapeStep(
            runner, random, 5, 45, true);
        if (first.state != MonsterEscapeAttemptState::Progressed) continue;

        const MonsterEscapeAttemptResult second = EscapeSystem::attemptMonsterEscapeStep(
            runner, random, 5, 45, true);
        assert(second.progressBefore == 1);
        assert(second.retryBonusPercent >= 18);
        assert(second.effectiveChancePercent > first.effectiveChancePercent);
        if (second.state == MonsterEscapeAttemptState::Escaped)
        {
            foundProgressiveEscape = true;
        }
    }
    assert(foundProgressiveEscape);

    // Le joueur suit désormais la même logique progressive : une ouverture gagnée contre un groupe
    // reste valable au tour suivant et augmente la chance de la prochaine tentative.
    bool foundPlayerProgressiveEscape = false;
    for (unsigned int seed = 1; seed < 10000 && !foundPlayerProgressiveEscape; ++seed)
    {
        Player runner;
        runner.setRace(CharacterRace::Fairy);
        runner.resetEscapeOpeningProgress();
        Random random(seed);

        const MonsterEscapeAttemptResult first = EscapeSystem::attemptPlayerEscapeStep(
            runner, random, DifficultyMode::Normal, 5);
        if (first.state != MonsterEscapeAttemptState::Progressed) continue;

        const MonsterEscapeAttemptResult second = EscapeSystem::attemptPlayerEscapeStep(
            runner, random, DifficultyMode::Normal, 5);
        assert(second.progressBefore == 1);
        assert(second.retryBonusPercent >= 18);
        assert(second.effectiveChancePercent > first.effectiveChancePercent);
        if (second.state == MonsterEscapeAttemptState::Escaped)
        {
            foundPlayerProgressiveEscape = true;
        }
    }
    assert(foundPlayerProgressiveEscape);

    Player fairyRunner;
    fairyRunner.setRace(CharacterRace::Fairy);
    Player orcRunner;
    orcRunner.setRace(CharacterRace::Orc);
    assert(EscapeSystem::playerEscapeMobilityModifier(fairyRunner)
        > EscapeSystem::playerEscapeMobilityModifier(orcRunner));

    return 0;
}
