#include "adventure/content/BiomeAmbientEventSystem.hpp"
#include "adventure/content/BiomeLivingContentCatalog.hpp"

#include <cstdint>

namespace
{
std::uint32_t stableHash(const std::string& text)
{
    std::uint32_t value = 2166136261u;
    for (unsigned char c : text)
    {
        value ^= c;
        value *= 16777619u;
    }
    return value;
}
}

std::string BiomeAmbientEventSystem::journalKey(const std::string& biomeName, int worldDay, const std::string& eventId)
{
    return biomeName + "|jour=" + std::to_string(worldDay < 0 ? 0 : worldDay) + "|" + eventId;
}

BiomeAmbientEvent BiomeAmbientEventSystem::buildCurrentEvent(const std::string& biomeName, int worldDay, int dayProgressUnit)
{
    BiomeAmbientEvent event;
    if (!BiomeLivingContentCatalog::hasProfile(biomeName)) return event;

    const BiomeLivingContentProfile profile = BiomeLivingContentCatalog::forBiome(biomeName);
    const int safeDay = worldDay < 0 ? 0 : worldDay;
    const int safeUnit = dayProgressUnit < 0 ? 0 : dayProgressUnit;
    const std::uint32_t seed = stableHash(biomeName + "|" + std::to_string(safeDay) + "|" + std::to_string(safeUnit / 2));

    // Not every visit has to perform for the player. Silence and normality are
    // part of a living world too.
    if (seed % 100u >= 46u) return event;

    event.active = true;
    switch ((seed / 101u) % 5u)
    {
        case 0:
            event.id = "trace_sociale_fraiche";
            event.title = "Passage récent";
            event.lines = {
                "Micro-événement local : quelqu'un est passé ici récemment.",
                profile.socialTrace,
                "Cette trace peut aider à lire le terrain actuel, sans révéler le prochain événement."
            };
            event.explorationRollShift = -2;
            break;
        case 1:
            event.id = "terrain_deplace";
            event.title = "Terrain changé";
            event.lines = {
                "Micro-événement local : une partie du terrain n'est plus exactement dans l'état habituel.",
                profile.environmentalHazard,
                "Le changement est observable maintenant ; il ne prédit pas ce qui t'attend plus loin."
            };
            event.explorationRollShift = 3;
            break;
        case 2:
            event.id = "faune_active";
            event.title = "Vie neutre active";
            event.lines = {
                "Micro-événement local : la faune neutre laisse un comportement inhabituellement lisible.",
                profile.neutralLife,
                "Observer cette vie aide légèrement à éviter les mouvements les plus maladroits."
            };
            event.explorationRollShift = -1;
            break;
        case 3:
            event.id = "ressource_exposee";
            event.title = "Ressource exposée";
            event.lines = {
                "Micro-événement local : le terrain a récemment mis à nu des signes de ressources.",
                profile.resourceSign,
                "Ce n'est pas un butin gratuit : seulement un indice qui rend la fouille actuelle un peu plus favorable."
            };
            event.explorationRollShift = -2;
            break;
        default:
            event.id = "detail_inhabituel";
            event.title = "Détail inhabituel";
            event.lines = {
                "Micro-événement local : un détail ne colle pas complètement au reste du décor.",
                profile.unusualSign,
                "Tu le notes comme anomalie observée, pas comme prophétie ni comme preuve d'un danger précis."
            };
            event.explorationRollShift = 1;
            break;
    }
    return event;
}
