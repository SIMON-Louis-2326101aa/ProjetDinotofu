// EN: Centralized quest deadline expiration support shared by quest submenus.
// FR: Support centralisé d'expiration des délais partagé par les sous-menus de quêtes.

#include "interface/menu/quest/QuestDeadlineSupport.hpp"

#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "quest/Quest.hpp"

namespace QuestDeadlineSupport
{

    void synchronizeQuestConsequences(Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (!quest.accepted || !quest.failed || quest.id.empty())
            {
                continue;
            }

            const std::string contact = quest.client.empty() ? std::string("Guilde locale") : quest.client;
            if (player.npcKnowsFact(contact, "quest_failed", quest.id))
            {
                continue;
            }

            player.rememberNpcFact(
                contact,
                "quest_failed",
                quest.id,
                "Affaire non menée à temps : " + quest.title,
                "registre_local",
                "journal_quetes",
                100,
                3
            );
            player.recordCanonicalEvent("quetes_echouees", quest.id, quest.title);
        }
    }

    void expireOverdueQuestDeadlines(Player& player, const std::string& screenId, bool notify)
    {
        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
        synchronizeQuestConsequences(player);
        if (expired <= 0 || !notify)
        {
            return;
        }

        MessageScreen::show(
            "DÉLAI DÉPASSÉ",
            screenId + ".deadline_expired",
            {
                std::to_string(expired) + " quête" + (expired > 1 ? "s" : "") + " vient d'être archivée pour délai dépassé.",
                "Les délais restent volontairement généreux, mais les commandes urgentes, services de comptoir et livraisons ne peuvent pas attendre indéfiniment.",
                "Tu peux les retrouver dans le journal, filtre : rendues / archivées."
            }
        );
    }
}
