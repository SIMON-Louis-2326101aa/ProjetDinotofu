#pragma once

#include <string>
#include <vector>

class Player;

struct CrimeContactDialogueSnapshot
{
    bool known = false;
    bool available = false;
    bool ruptured = false;
    std::string contactName;
    std::vector<std::string> lines;
};

namespace CrimeContactDialogueSystem
{
    CrimeContactDialogueSnapshot build(const Player& player, const std::string& contactId);
}
