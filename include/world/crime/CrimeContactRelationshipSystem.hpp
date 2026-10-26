#pragma once

#include <string>
#include <vector>

class Player;

struct CrimeContactRelationshipAssessment
{
    bool known = false;
    bool available = false;
    bool ruptured = false;
    int reliability = 50;
    int favorBalance = 0; // > 0: the contact owes the player; < 0: the player owes the contact.
    int rivalry = 0;
    int cooperation = 0;
    int pressure = 0;
    std::string reliabilityLabel;
    std::string debtLabel;
    std::string relationshipLabel;
};

struct CrimeContactInteractionResult
{
    bool changed = false;
    bool ruptureStarted = false;
    bool ruptureEnded = false;
    int reliabilityDelta = 0;
    int favorDelta = 0;
    int rivalryDelta = 0;
    int ruptureUntilDay = -1;
    std::vector<std::string> lines;
};

namespace CrimeContactRelationshipSystem
{
    CrimeContactRelationshipAssessment assess(const Player& player, const std::string& contactId);

    CrimeContactInteractionResult recordReliableDeal(
        Player& player,
        const std::string& contactId,
        int contactFavorOwedToPlayer = 0,
        const std::string& reason = ""
    );

    CrimeContactInteractionResult recordPlayerDebt(
        Player& player,
        const std::string& contactId,
        int amount,
        const std::string& reason = ""
    );

    CrimeContactInteractionResult settlePlayerDebt(
        Player& player,
        const std::string& contactId,
        int amount,
        const std::string& reason = ""
    );

    CrimeContactInteractionResult recordBrokenDeal(
        Player& player,
        const std::string& contactId,
        int severity,
        const std::string& reason = ""
    );

    // Returns a contact the player already knows. Never reveals an undiscovered network member.
    std::string chooseKnownAlternateRelay(const Player& player, const std::string& contactId);

    // Used when the original contact itself is unavailable: pick another already-known and currently
    // usable member of the same network without requiring the unavailable contact to relay anything.
    std::string chooseKnownReplacementContact(const Player& player, const std::string& contactId);
}
