// EN: Shared quest presentation helpers used by several quest menu modules.
// FR: Helpers communs de présentation des quêtes utilisés par plusieurs modules.

#include "interface/menu/quest/QuestPresentationSupport.hpp"

#include <algorithm>
#include <cctype>
#include <vector>

namespace QuestPresentationSupport
{
std::string lowerQuestDialogueText(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}
bool questDialogueContainsAny(const Quest& quest, const std::vector<std::string>& needles)
{
    const std::string combined = lowerQuestDialogueText(
        quest.title + " " + quest.objective + " " + quest.objectiveType + " " + quest.targetFamily + " " + quest.location
    );

    for (const std::string& needle : needles)
    {
        if (combined.find(lowerQuestDialogueText(needle)) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}
bool isCountedHuntQuest(const Quest& quest)
{
    return quest.objectiveType == "combat"
        && quest.target > 1
        && questDialogueContainsAny(quest, {"chasse", "chasser", "traquer", "nettoyer", "objectif chiffré"});
}
bool isArtificialHuntQuest(const Quest& quest)
{
    return isCountedHuntQuest(quest)
        && questDialogueContainsAny(quest, {"construction", "automate", "golem", "armure", "sentinelle", "mannequin", "pantin", "artificielle"});
}
std::string questKindText(const Quest& quest)
    {
        if (quest.guildChallenge) return "Défi temporaire de guilde";
        if (quest.origin == "Défi du Hero Villager") return "Défi héroïque à validation directe";
        return quest.guildQuest ? "Contrat officiel de guilde" : "Demande informelle de PNJ";
    }
std::string questProgressMethodText(const Quest& quest)
    {
        if (quest.objectiveType == "service")
        {
            if (questDialogueContainsAny(quest, {"tri de sac", "inventaire trop", "sac trop"}))
            {
                return "Va à la guilde et choisis Traiter un service de guilde : l'épreuve consiste à trier un inventaire selon poids, valeur, fragilité et utilité.";
            }
            if (questDialogueContainsAny(quest, {"armure mal ajustée", "armure mal ajustee", "sangles", "morphologie"}))
            {
                return "Va à la guilde et choisis Traiter un service de guilde : l'épreuve consiste à adapter l'équipement à la morphologie de la race ou sous-race.";
            }
            return quest.guildQuest
                ? "Va à la guilde et choisis Traiter un service de guilde."
                : "Retourne parler au PNJ concerné pour confirmer le service.";
        }

        if (quest.objectiveType == "combat")
        {
            if (isCountedHuntQuest(quest))
            {
                if (isArtificialHuntQuest(quest))
                {
                    return "Lance des combats contre automates, golems, armures vivantes, statues ou pantins animés : chaque cible artificielle compatible vaincue compte.";
                }
                return "Lance des combats correspondant à la cible/famille indiquée : chaque monstre compatible vaincu compte jusqu'au nombre demandé.";
            }
            return "Lance des combats correspondant à la cible/famille indiquée.";
        }

        if (quest.objectiveType == "exploration" || quest.objectiveType == "bestiaire")
        {
            return "Passe par Exploration et choisis la zone conseillée ; les traces et découvertes font avancer le journal.";
        }

        if (quest.objectiveType == "livraison")
        {
            if (!quest.requiredMaterialId.empty())
            {
                return "Récupère le matériau demandé, puis rends la quête au contact.";
            }

            return "Passe par Exploration dans la zone conseillée ; cette livraison se traite comme une sortie de terrain.";
        }

        return "Suis la cible et le lieu conseillés, puis reviens voir le contact.";
    }
}
