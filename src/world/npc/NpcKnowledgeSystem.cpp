#include "world/npc/NpcKnowledgeSystem.hpp"
#include "entity/Player.hpp"
#include <algorithm>
#include <cctype>
#include <functional>

namespace
{
std::string low(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string roleForName(const std::string& npcName)
{
    const std::string value = low(npcName);
    if (value.find("bibli") != std::string::npos || value.find("archiv") != std::string::npos) return "bibliothécaire scribe";
    if (value.find("garde") != std::string::npos || value.find("capitaine") != std::string::npos) return "garde";
    if (value.find("forge") != std::string::npos || value.find("forger") != std::string::npos) return "forgeron";
    if (value.find("auberg") != std::string::npos) return "aubergiste";
    if (value.find("prêtre") != std::string::npos || value.find("pret") != std::string::npos) return "prêtre";
    if (value.find("guilde") != std::string::npos || value.find("maître") != std::string::npos || value.find("maitre") != std::string::npos) return "maître de guilde";
    if (value.find("march") != std::string::npos || value.find("vende") != std::string::npos) return "marchand";
    return "habitant";
}

struct ShareableFact
{
    std::string factType;
    std::string subjectId;
    std::string label;
    int confidence = 65;
    int evidenceLevel = 1;
};

bool convertHistoricalEvent(const PlayerHistoricalEvent& event, ShareableFact& out)
{
    if (event.category == "rival_return" || event.category == "rival_escape" || event.category == "rival_wound")
    {
        out.factType = "rival_seen";
        out.subjectId = event.subjectId;
        out.label = event.label;
        out.confidence = event.category == "rival_return" ? 82 : 72;
        out.evidenceLevel = 1;
        return true;
    }
    if (event.category == "exploration_language_trace")
    {
        out.factType = "discovered_writing";
        out.subjectId = event.subjectId;
        out.label = event.label;
        out.confidence = 78;
        out.evidenceLevel = 1;
        return true;
    }
    if (event.category == "attaque_locale" || event.category == "local_attack" || event.category == "defense_locale")
    {
        out.factType = "local_attack";
        out.subjectId = event.subjectId;
        out.label = event.label;
        out.confidence = 80;
        out.evidenceLevel = 1;
        return true;
    }
    if (event.category == "item_memory_enemy_signature")
    {
        out.factType = "unusual_monster";
        out.subjectId = event.subjectId;
        out.label = event.label;
        out.confidence = 66;
        out.evidenceLevel = 1;
        return true;
    }
    return false;
}
}

LivingNpcProfile NpcKnowledgeSystem::profileForNamedNpc(const std::string& npcName)
{
    LivingNpcProfile profile = LivingNpcProfileSystem::infer(npcName, roleForName(npcName), "Humain");
    const std::string value = low(npcName);

    auto setNamed = [&](const std::string& profession, const std::string& temperament, const std::string& network)
    {
        profile.profession = profession;
        profile.temperament = temperament;
        profile.informationNetwork = network;
    };

    if (value == "mira") setNamed("intendance", "pragmatique", "reseau_guilde");
    else if (value == "orren") setNamed("garde", "méfiant mais loyal", "reseau_garde");
    else if (value == "lysa") setNamed("soigneuse", "empathique", "reseau_temple");
    else if (value == "bram") setNamed("artisan", "franc", "reseau_artisans");
    else if (value == "soryn") setNamed("érudit", "méthodique", "reseau_savant");
    else if (value == "eda") setNamed("logistique", "rigoureuse", "reseau_commercial");
    else if (value == "nell") setNamed("messagère", "prudente", "reseau_guilde");
    else if (value.find("meron") != std::string::npos) setNamed("érudit", "méthodique", "reseau_savant");
    else if (value.find("prunigil") != std::string::npos) setNamed("contact", "observateur", "reseau_contacts");
    else if (value == "bob" || value == "maurice") setNamed("marchand itinérant", "sociable", "reseau_commercial");

    return profile;
}

int NpcKnowledgeSystem::effectiveConfidence(const NpcKnownFact& fact, int currentWorldDay)
{
    const int age = std::max(0, currentWorldDay - std::max(fact.firstLearnedDay, fact.lastReinforcedDay));
    int decay = 0;

    // Une preuve matérielle ou un fait vécu directement résiste bien mieux au temps qu'une rumeur.
    if (fact.evidenceLevel >= 3 || fact.sourceType == "preuve" || fact.sourceType == "interaction_directe")
    {
        decay = age / 40;
    }
    else if (fact.evidenceLevel >= 2 || fact.sourceType == "observation_directe" || fact.sourceType == "registre")
    {
        decay = age / 18;
    }
    else if (fact.sourceType == "temoignage_joueur")
    {
        decay = age / 4;
    }
    else
    {
        decay = age / 2;
    }

    // Des sources distinctes réellement entendues plusieurs fois ralentissent l'oubli sans créer une vérité magique.
    const int reinforcement = std::min(10, std::max(0, fact.timesHeard - 1) * 2);
    return std::clamp(fact.confidence - std::max(0, decay - reinforcement), 15, 100);
}

std::string NpcKnowledgeSystem::confidenceLabel(int confidence, int evidenceLevel)
{
    if (evidenceLevel >= 3 || confidence >= 95) return "fait vérifié";
    if (evidenceLevel >= 2 || confidence >= 80) return "témoignage solide";
    if (confidence >= 60) return "témoignage plausible";
    return "rumeur fragile";
}

std::string NpcKnowledgeSystem::sourceLabel(const std::string& sourceType)
{
    if (sourceType == "observation_directe") return "vu directement";
    if (sourceType == "preuve") return "preuve examinée";
    if (sourceType == "interaction_directe") return "vécu avec toi";
    if (sourceType == "temoignage_joueur") return "raconté par toi";
    if (sourceType == "registre") return "consigné dans un registre";
    if (sourceType == "rumeur") return "entendu par rumeur";
    if (sourceType == "rumeur_locale") return "relayé par un autre PNJ local";
    if (sourceType == "rumeur_interville") return "relayé depuis une autre ville";
    if (sourceType == "rapport_interville") return "rapport ou copie arrivé depuis une autre ville";
    return "source non précisée";
}

bool NpcKnowledgeSystem::hasContradictoryClaims(const Player& player, const std::string& npcName, const std::string& factType, const std::string& subjectId)
{
    std::string firstVariant;
    for (const NpcKnownFact& fact : player.getNpcKnownFactsFor(npcName, 0))
    {
        if (fact.factType != factType || fact.subjectId != subjectId) continue;
        const std::string variant = fact.claimVariant.empty() ? "default" : fact.claimVariant;
        if (firstVariant.empty()) firstVariant = variant;
        else if (variant != firstVariant) return true;
    }
    return false;
}

std::vector<std::string> NpcKnowledgeSystem::conversationMemoryLines(const Player& player, const std::string& npcName, int limit)
{
    std::vector<std::string> lines;
    const LivingNpcProfile profile = profileForNamedNpc(npcName);
    const std::vector<NpcKnownFact> facts = player.getNpcKnownFactsFor(npcName, std::max(1, limit));
    for (const NpcKnownFact& fact : facts)
    {
        std::string subject = fact.label.empty() ? fact.subjectId : fact.label;
        if (subject.size() > 82) subject = subject.substr(0, 79) + "...";
        const int currentConfidence = effectiveConfidence(fact, player.getWorldDaysElapsed());
        const int ageDays = std::max(0, player.getWorldDaysElapsed() - fact.lastReinforcedDay);
        std::string memoryLine = "Mémoire de " + npcName + " — " + confidenceLabel(currentConfidence, fact.evidenceLevel)
            + " (" + sourceLabel(fact.sourceType) + ", renforcée il y a " + std::to_string(ageDays) + " j) : " + subject;
        if (!fact.relayChannel.empty()) memoryLine += " [canal : " + fact.relayChannel + "]";
        if (hasContradictoryClaims(player, npcName, fact.factType, fact.subjectId))
            memoryLine += " [version contestée : " + (fact.claimVariant.empty() ? std::string("default") : fact.claimVariant) + "]";
        lines.push_back(memoryLine);
        lines.push_back(LivingNpcProfileSystem::reactionToKnownFact(profile, fact.factType, fact.subjectId.empty() ? "ce sujet" : fact.subjectId));
        if (static_cast<int>(lines.size()) >= limit * 2) break;
    }
    return lines;
}


std::vector<std::string> NpcKnowledgeSystem::spontaneousIntroLines(const Player& player, const std::string& npcName)
{
    const LivingNpcProfile profile = profileForNamedNpc(npcName);
    if (!profile.canInitiateConversation) return {};

    const std::size_t seed = std::hash<std::string>{}(npcName + "|" + std::to_string(player.getWorldDaysElapsed()));
    const std::vector<NpcKnownFact> facts = player.getNpcKnownFactsFor(npcName, 1);
    const bool hasMemory = !facts.empty();
    const int threshold = hasMemory ? 58 : 28;
    if (static_cast<int>(seed % 100) >= threshold) return {};

    std::vector<std::string> lines;
    if (hasMemory)
    {
        const NpcKnownFact& fact = facts.front();
        const int confidence = effectiveConfidence(fact, player.getWorldDaysElapsed());
        lines.push_back(npcName + " prend l'initiative avant que tu poses ta question.");
        lines.push_back(LivingNpcProfileSystem::reactionToKnownFact(profile, fact.factType, fact.subjectId.empty() ? fact.label : fact.subjectId));
        lines.push_back("Il parle à partir de ce qu'il croit savoir (" + confidenceLabel(confidence, fact.evidenceLevel) + "), pas d'un état omniscient du monde.");
        return lines;
    }

    lines.push_back(npcName + " engage brièvement la conversation de lui-même.");
    if (profile.profession == "garde") lines.push_back("« Si tu rapportes quelque chose d'important, donne-moi un lieu, une heure et un témoin. »");
    else if (profile.profession == "marchand") lines.push_back("« Les routes parlent avant les clients. Enfin... surtout quand quelqu'un revient entier pour raconter. »");
    else if (profile.profession == "érudit") lines.push_back("« Un détail exact vaut mieux qu'une histoire impressionnante. Qu'est-ce que tu as réellement observé ? »");
    else if (profile.profession == "aubergiste") lines.push_back("« Les gens racontent beaucoup ici. Je garde surtout ce qui revient par plusieurs bouches. »");
    else if (profile.profession == "religieux") lines.push_back("« Une parole peut aider, mais elle peut aussi déformer. Dis-moi ce que tu sais, pas ce que tu imagines. »");
    else lines.push_back("Son entrée en matière dépend de son humeur et de son métier plutôt que d'une réplique fixe répétée à chaque visite.");
    return lines;
}

bool NpcKnowledgeSystem::hasShareableRecentFact(const Player& player)
{
    const std::vector<PlayerHistoricalEvent>& events = player.getHistoricalEvents();
    ShareableFact converted;
    for (auto it = events.rbegin(); it != events.rend(); ++it)
    {
        if (convertHistoricalEvent(*it, converted)) return true;
    }
    return false;
}

std::vector<std::string> NpcKnowledgeSystem::shareMostRecentFact(Player& player, const std::string& npcName)
{
    const std::vector<PlayerHistoricalEvent>& events = player.getHistoricalEvents();
    ShareableFact converted;
    for (auto it = events.rbegin(); it != events.rend(); ++it)
    {
        if (!convertHistoricalEvent(*it, converted)) continue;
        player.rememberNpcFact(
            npcName,
            converted.factType,
            converted.subjectId.empty() ? it->id : converted.subjectId,
            converted.label.empty() ? it->label : converted.label,
            "temoignage_joueur",
            player.getName(),
            converted.confidence,
            converted.evidenceLevel
        );
        const LivingNpcProfile profile = profileForNamedNpc(npcName);
        return {
            "Tu transmets un fait récent à " + npcName + ".",
            "Source enregistrée : ton témoignage. Ce PNJ ne le traite pas automatiquement comme une vérité absolue.",
            "Crédibilité actuelle : " + confidenceLabel(converted.confidence, converted.evidenceLevel) + ".",
            LivingNpcProfileSystem::reactionToKnownFact(profile, converted.factType, converted.subjectId.empty() ? "ce sujet" : converted.subjectId)
        };
    }
    return {"Tu n'as aucun fait récent pertinent à transmettre ici."};
}
