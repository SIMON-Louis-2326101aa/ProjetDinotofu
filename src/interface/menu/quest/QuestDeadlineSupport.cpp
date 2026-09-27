// EN: Centralized quest deadline expiration support shared by quest submenus.
// FR: Support centralisé d'expiration des délais partagé par les sous-menus de quêtes.

#include "interface/menu/quest/QuestDeadlineSupport.hpp"

#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"

namespace QuestDeadlineSupport
{
    void expireOverdueQuestDeadlines(Player& player, const std::string& screenId, bool notify)
    {
        const int expired = player.getQuestLog().expireOverdueQuests(player.getWorldDaysElapsed());
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
