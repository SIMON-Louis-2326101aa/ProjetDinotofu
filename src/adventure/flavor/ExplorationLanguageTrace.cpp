#include "adventure/flavor/ExplorationLanguageTrace.hpp"
#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"

ExplorationLanguageTrace ExplorationLanguageTraceCatalog::forBiome(const std::string& name)
{
    if (name == "Archives noyées") return {"elfique", "« Selae nyr vael. Thiren os ara. »", "Les rayonnages inférieurs étaient déjà condamnés avant l'inondation ; chercher les sceaux bleus plutôt que les portes.", "Une annotation court sur le bord d'une page sèche.", 2};
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return {"kitsune", "« Hoshi wa uso o terasu. Kage wa namae o mamoru. »", "Les lanternes révèlent le mensonge, mais les ombres protègent les vrais noms.", "Une courte maxime est gravée derrière un torii.", 2};
    if (name == "Temple des cloches fendues") return {"celeste", "« Aurel ven, sonna vera, nomen tace. »", "Fais sonner la cloche vraie et tais le nom du mort.", "Une consigne rituelle est gravée sur le bronze fendu.", 2};
    if (name == "Marché sous les ponts") return {"gobelin", "« Grik vel, nar tik. Rakka dosh. »", "Deux coups brefs signalent un marchand sûr ; trois annoncent un guetteur.", "Un graffiti minuscule est caché sous une poutre.", 1};
    if (name == "Carrière des os blancs") return {"orc", "« Gor makh. Drah sten. Urg ven. »", "Ne frappe pas la pierre blanche après le troisième écho.", "Des marques de taille forment une phrase grossière.", 1};
    if (name == "Confluence du Mana pur") return {"anormal", "« // trois flux // quatrième mémoire // ne pas nommer // »", "Le motif avertit qu'un quatrième courant n'apparaît que lorsqu'on tente de le mesurer.", "Les signes changent légèrement quand tu détournes le regard.", 1};
    if (name == "Cimetière oublié") return {"spirituel", "« [nom gardé] [terre rendue] [ne pas suivre la voix] »", "Le message conseille de rendre aux tombes ce qui leur appartient et d'ignorer les voix qui imitent les proches.", "Un souvenir qui n'est pas le tien accompagne une pierre sans nom.", 1};
    if (name == "Archipel des îles flottantes") return {"draconique", "« Kraav siir ven. Tharun el, nor vae. »", "Les courants ascendants les plus sûrs se trouvent sous les ombres des grands îlots, pas sous leurs bords.", "Une balise ancienne porte des griffures régulières.", 2};
    if (name == "Bois de la Corruption") return {"infernal", "« Vel noss, dorakh tir. Shaal ven ara. »", "La corruption suit les pactes rompus plus facilement que le sang versé.", "Une phrase a été brûlée dans une écorce noire.", 2};
    if (name == "Bosquet des Fées du Mana") return {"feerique", "« Aeli thim, nimae ril. Sor vael. »", "Une offrande donnée sans demande ouvre un passage ; une faveur exigée le referme.", "Des mots apparaissent dans le pollen quand la lumière baisse.", 2};
    return {};
}

bool ExplorationLanguageTraceCatalog::hasTrace(const std::string& biomeName)
{
    return !forBiome(biomeName).languageId.empty();
}

std::string ExplorationLanguageTraceCatalog::renderForPlayer(const Player& player, const ExplorationLanguageTrace& trace)
{
    if (trace.languageId.empty()) return {};
    const int level = LanguageSystem::getKnowledgeLevel(player, trace.languageId);
    const std::string language = LanguageSystem::displayName(trace.languageId);
    if (level <= 0)
        return trace.contextHint + " Tu identifies seulement qu'il s'agit de " + language + ". Texte : " + trace.sourceText;
    if (level < trace.requiredLevel)
        return trace.contextHint + " Tu reconnais du " + language + " et quelques fragments, mais pas assez pour garantir le sens. Texte : " + trace.sourceText;
    return trace.contextHint + " Tu lis le " + language + " : « " + trace.translatedMeaning + " »";
}
