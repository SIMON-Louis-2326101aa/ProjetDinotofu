#include "story/HestiaArcSystem.hpp"

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalize(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

HestiaRescueScenario HestiaArcSystem::rescueScenario(bool skeletonVariant)
{
    HestiaRescueScenario s;
    if (skeletonVariant)
    {
        s.variantId = "hestia_cellar_skeletons";
        s.defenders = "Squelettes dans le sous-sol abandonné";
        s.eliteThreat = "Squelette suicidaire explosif";
        s.warnings = {
            "Le crâne du suicidaire est piégé : le frapper directement peut déclencher l'explosion.",
            "Explosion : hallucination, poison et brûlure possibles.",
            "Une infiltration reste possible, mais les couloirs morts-vivants rendent la discrétion difficile."
        };
    }
    else
    {
        s.variantId = "hestia_cellar_goblins";
        s.defenders = "Gobelins occupant le sous-sol abandonné";
        s.eliteThreat = "Gobelin ninja expérimenté";
        s.warnings = {
            "Le ninja peut couper les routes de sortie et punir une avance trop bruyante.",
            "Une infiltration est possible, mais exige d'éviter les postes gobelins et de libérer Hestia sans alerte générale."
        };
    }
    return s;
}

bool HestiaArcSystem::completeMainStoryRescue(Player& player, const std::string& initialResidence, bool playerHasHome)
{
    PlayerHestiaStoryState& h = player.getMutableHestiaStoryState();
    if (h.rescued) return false;
    h.rescued = true;
    h.materialized = true;
    h.trust = 1;
    h.awakeningStage = 0;
    h.lastVisitDay = player.getWorldDaysElapsed();
    h.nextSmallFindDay = h.lastVisitDay + 2;
    h.sacrificeConsumed = false;

    if (!moveResidence(player, initialResidence, playerHasHome))
        h.residence = "guild_diane";

    player.recordCanonicalEvent("hestia_arc", "rescue", "Hestia sauvée du sous-sol abandonné et ajoutée aux proches du joueur", 1);
    return true;
}

bool HestiaArcSystem::moveResidence(Player& player, const std::string& residence, bool playerHasHome)
{
    PlayerHestiaStoryState& h = player.getMutableHestiaStoryState();
    if (!h.rescued || !h.materialized) return false;
    if (residence == "player_home")
    {
        if (!playerHasHome || !player.getHomeState().owned) return false;
        h.residence = residence;
        player.recordCanonicalEvent("hestia_residence", residence, "Hestia change de lieu de vie", 1);
        return true;
    }
    if (residence == "active_party" || residence == "guild_diane")
    {
        h.residence = residence;
        player.recordCanonicalEvent("hestia_residence", residence, "Hestia change de lieu de vie", 1);
        return true;
    }
    return false;
}

std::vector<std::string> HestiaArcSystem::visit(Player& player, int currentDay, Random& random)
{
    PlayerHestiaStoryState& h = player.getMutableHestiaStoryState();
    if (!h.rescued || !h.materialized) return {"Hestia n'est pas disponible."};
    h.lastVisitDay = std::max(0, currentDay);
    h.trust = std::clamp(h.trust + random.between(3, 7), 0, 100);
    player.recordCanonicalEvent("hestia_visites", h.residence.empty() ? "unknown" : h.residence, "Moment volontaire passé avec Hestia", 1);
    std::vector<std::string> lines;
    if (h.residence == "guild_diane")
        lines.push_back("Hestia reste près de Diane à l'accueil de la guilde. Elle parle peu, mais ta visite la rassure.");
    else if (h.residence == "player_home")
        lines.push_back("Hestia semble heureuse de te voir rentrer. Elle peut cohabiter avec ton conjoint sans problème.");
    else
        lines.push_back("Hestia reste près de toi. Elle essaie encore d'aider même quand elle ne sait pas vraiment comment.");
    lines.push_back("Confiance d'Hestia : " + std::to_string(h.trust) + "/100.");
    return lines;
}

HestiaSmallFind HestiaArcSystem::maybeBringSmallFind(Player& player, int currentDay, Random& random)
{
    HestiaSmallFind out;
    PlayerHestiaStoryState& h = player.getMutableHestiaStoryState();
    if (!h.rescued || !h.materialized || h.residence != "player_home") return out;
    if (h.nextSmallFindDay >= 0 && currentDay < h.nextSmallFindDay) return out;
    if (random.between(1, 100) > 42)
    {
        h.nextSmallFindDay = currentDay + 2;
        return out;
    }

    out.occurred = true;
    const int roll = random.between(1, 5);
    if (roll == 1) { out.copper = random.between(1, 8); out.label = "quelques petites pièces trouvées sans importance"; player.getInventory().earnCopper(out.copper); }
    else if (roll == 2) { out.materialId = "bitter_healing_leaf"; out.materialQuantity = 1; out.label = "une feuille amère de soin"; player.getInventory().addMaterial(MaterialCatalog::createById(out.materialId, 1)); }
    else if (roll == 3) { out.materialId = "rusted_metal_fragment"; out.materialQuantity = 1; out.label = "un fragment de métal rouillé"; player.getInventory().addMaterial(MaterialCatalog::createById(out.materialId, 1)); }
    else if (roll == 4) { out.materialId = "slime_residue"; out.materialQuantity = 1; out.label = "un résidu de slime qu'elle trouvait joli"; player.getInventory().addMaterial(MaterialCatalog::createById(out.materialId, 1)); }
    else { out.label = "un caillou étrangement lisse"; }

    h.nextSmallFindDay = currentDay + random.between(2, 4);
    out.lines.push_back("Hestia t'attend avec " + out.label + ". Elle a l'air beaucoup trop fière de sa trouvaille.");
    player.recordCanonicalEvent("hestia_trouvailles", out.materialId.empty() ? "petit_objet" : out.materialId, out.label, 1);
    return out;
}

std::string HestiaArcSystem::wellbeingLabel(const Player& player, int currentDay)
{
    (void)currentDay;
    const PlayerHestiaStoryState& h = player.getHestiaStoryState();
    if (!h.rescued) return "inconnue";
    if (!h.materialized) return "non matérialisée";
    if (h.residence == "guild_diane") return "en sécurité auprès de Diane";
    if (h.residence == "player_home") return "installée tranquillement à la maison";
    if (h.residence == "active_party") return "présente avec toi";
    return "en sécurité";
}

bool HestiaArcSystem::setAwakeningStage(Player& player, int stage)
{
    PlayerHestiaStoryState& h = player.getMutableHestiaStoryState();
    if (!h.rescued || !h.materialized) return false;
    const int normalized = std::clamp(stage, 0, 3);
    if (normalized <= h.awakeningStage) return false;
    h.awakeningStage = normalized;
    player.recordCanonicalEvent("hestia_eveils_lumiere", std::to_string(normalized), "Grande magie divine/lumineuse d'Hestia", 1);
    if (normalized >= 3)
    {
        h.materialized = false;
        h.residence = "returned_to_heavens";
    }
    return true;
}

int HestiaArcSystem::combatAptitudeScore(const Player& player, const std::string& discipline)
{
    const PlayerHestiaStoryState& h = player.getHestiaStoryState();
    if (!h.rescued) return -100;
    const std::string d = normalize(discipline);
    if (d.find("divin") != std::string::npos || d.find("lumi") != std::string::npos)
        return -4 + 16 * h.awakeningStage; // starts bad during readaptation, then becomes absurdly gifted.
    if (d.find("mag") != std::string::npos)
        return -4 + 7 * h.awakeningStage;
    return -4;
}

bool HestiaArcSystem::canSacrificeForPlayer(const Player& player, bool hestiaActiveInParty)
{
    const PlayerHestiaStoryState& h = player.getHestiaStoryState();
    return h.rescued && h.materialized && h.awakeningStage >= 1 && !h.sacrificeConsumed
        && hestiaActiveInParty && h.residence == "active_party";
}

bool HestiaArcSystem::consumeSacrificeForPlayer(Player& player, bool hestiaActiveInParty)
{
    if (!canSacrificeForPlayer(player, hestiaActiveInParty)) return false;
    PlayerHestiaStoryState& h = player.getMutableHestiaStoryState();
    h.sacrificeConsumed = true;
    h.materialized = false;
    h.residence = "sacrificed_for_player";
    player.recordCanonicalEvent("hestia_arc", "sacrifice", "Hestia a téléporté le joueur en sécurité au prix de sa matérialisation", 1);
    return true;
}
