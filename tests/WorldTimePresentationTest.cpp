#include "entity/Player.hpp"

#include <cassert>
#include <string>

int main()
{
    Player player;

    assert(player.getCurrentWorldDayNumber() == 1);
    assert(player.formatWorldDateLine() == "Jour 1 (dimanche)");
    assert(player.formatWorldDayPartLine() == "Matin (1/5)");
    assert(player.formatWorldDateTimeLine() == "Jour 1 (dimanche) — Matin (1/5)");

    player.advanceWorldDayUnits(1);
    assert(player.formatWorldDayPartLine() == "Midi (2/5)");

    player.advanceWorldDayUnits(1);
    assert(player.formatWorldDayPartLine() == "Après-midi (3/5)");

    player.advanceWorldDayUnits(1);
    assert(player.formatWorldDayPartLine() == "Soir (4/5)");

    player.advanceWorldDayUnits(1);
    assert(player.formatWorldDayPartLine() == "Nuit (5/5)");

    player.advanceWorldDayUnits(1);
    assert(player.getCurrentWorldDayNumber() == 2);
    assert(player.formatWorldDateLine() == "Jour 2 (lundi)");
    assert(player.formatWorldDayPartLine() == "Matin (1/5)");

    const std::string change = player.formatWorldTimeChange(0, 4);
    assert(change.find("Nuit (5/5)") != std::string::npos);
    assert(change.find("Jour 2 (lundi)") != std::string::npos);
    assert(change.find("Matin (1/5)") != std::string::npos);

    return 0;
}
