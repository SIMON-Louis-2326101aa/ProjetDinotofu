#pragma once

class Player;

// Hooks shared with the main quest/location menu while the implementation
// lives in QuestTeamMenu.cpp.
void maybeShowTorvaldRankDIntro(Player& player);
void openFieldObservationDesk(Player& player);
void openMercenaryCounter(Player& player);
void openTorvaldGuildTrainer(Player& player);
void openInfirmaryServiceMenu(Player& player);
