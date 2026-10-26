#include "combat/anatomy/MonsterAnatomySystem.hpp"

#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalized(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& text, const std::vector<std::string>& needles)
    {
        const std::string probe = normalized(text);
        for (const std::string& needle : needles)
            if (probe.find(normalized(needle)) != std::string::npos) return true;
        return false;
    }

    bool hasUsefulReaction(const MonsterWeakPointProfile& profile)
    {
        return profile.revealTargetStats
            || profile.breakDefensePosture
            || profile.weakeningTurns > 0
            || profile.vulnerabilityTurns > 0
            || profile.nextHitVulnerabilityTurns > 0;
    }
}

MonsterWeakPointProfile MonsterAnatomySystem::weakPointProfile(const Monster& monster)
{
    MonsterWeakPointProfile out;
    const std::string identity = monster.getName() + " " + monster.getType() + " " + monster.getRaceText();

    // Explicit trap first: this one deliberately punishes a successful anatomical hit.
    if (containsAny(identity, {"squelette suicidaire explosif", "crâne piégé", "crane piege", "explosif toxique"}))
    {
        out.zoneId = "trapped_skull";
        out.zoneLabel = "crâne";
        out.reactiveHazard = true;
        out.selfDestructiveHazard = true;
        out.hazardDamage = std::max(5, monster.getLevel() / 3 + 5);
        out.poisonTurns = 3;
        out.poisonDamage = std::max(1, monster.getLevel() / 20 + 2);
        out.burningTurns = 2;
        out.burningDamage = std::max(1, monster.getLevel() / 22 + 2);
        out.illusionTurns = 2;
        out.hint = "Le crâne paraît anormalement chargé. Le frapper directement peut déclencher ce qu'il contient.";
        out.successLine = "Le crâne piégé rompt au lieu de simplement céder.";
        return out;
    }

    const bool armoredIdentity = containsAny(identity, {
        "cuirass", "carapace", "coquille", "armure", "blindé", "blinde", "plaqué", "plaque"
    });

    switch (monster.getRace())
    {
        case Race::MortVivant:
            if (containsAny(identity, {"squelette", "ossement", "armure morte"}))
            {
                out.zoneId = "joints";
                out.zoneLabel = "jointures";
                out.nextHitVulnerabilityTurns = 1;
                out.nextHitVulnerabilityPercent = 6;
                out.hint = "Les jointures et attaches semblent plus utiles à viser que les os les plus épais.";
                out.successLine = "Une attache cède et laisse le prochain impact entrer plus proprement.";
            }
            else
            {
                out.zoneId = "dead_tissue_anchor";
                out.zoneLabel = "attache nécrotique";
                out.weakeningTurns = 1;
                out.weakeningPercent = 5;
                out.hint = "Les chairs mortes tiennent encore par quelques attaches plus tendues que les autres.";
                out.successLine = "L'attache nécrotique lâche assez pour déséquilibrer le mort-vivant.";
            }
            break;

        case Race::Slime:
            out.zoneId = "core";
            out.zoneLabel = "noyau";
            out.revealTargetStats = true;
            out.vulnerabilityTurns = 1;
            out.vulnerabilityPercent = 5;
            out.hint = "La masse laisse parfois entrevoir un noyau plus cohérent.";
            out.successLine = "Le noyau est forcé à rester visible un instant.";
            break;

        case Race::Construction:
            out.zoneId = armoredIdentity ? "plate_joint" : "mechanical_joint";
            out.zoneLabel = armoredIdentity ? "jonction de plaque" : "articulation mécanique";
            out.revealTargetStats = true;
            out.weakeningTurns = 1;
            out.weakeningPercent = armoredIdentity ? 6 : 5;
            out.nextHitVulnerabilityTurns = 1;
            out.nextHitVulnerabilityPercent = armoredIdentity ? 9 : 7;
            out.breakDefensePosture = true;
            out.hint = armoredIdentity
                ? "Les plaques protègent la masse, mais leur jonction transmet directement les contraintes au mécanisme."
                : "Les articulations, rivets et axes mobiles sont plus vulnérables que la matière principale.";
            out.successLine = armoredIdentity
                ? "La jonction se décale : la plaque tient encore, mais le mécanisme derrière devient lisible."
                : "L'articulation prend du jeu et le mouvement de la construction devient moins propre.";
            break;

        case Race::Dragon:
            out.zoneId = "wing_root";
            out.zoneLabel = "base d'aile";
            out.weakeningTurns = 1;
            out.weakeningPercent = 6;
            out.nextHitVulnerabilityTurns = 1;
            out.nextHitVulnerabilityPercent = 6;
            out.hint = "Les écailles couvrent le tronc ; la base des ailes et leurs attaches restent plus difficiles à protéger complètement.";
            out.successLine = "La base d'aile encaisse mal l'angle et perturbe l'appui du dragon.";
            break;

        case Race::Draconide:
        case Race::SemiDragon:
            out.zoneId = "scale_seam";
            out.zoneLabel = "jointure d'écailles";
            out.nextHitVulnerabilityTurns = 1;
            out.nextHitVulnerabilityPercent = 7;
            out.hint = "Les écailles résistent bien de face, mais leurs jointures se chevauchent moins proprement en mouvement.";
            out.successLine = "La jointure d'écailles s'ouvre juste assez pour préparer le prochain coup.";
            break;

        case Race::Plante:
            out.zoneId = "root_collar";
            out.zoneLabel = "collet racinaire";
            out.weakeningTurns = 1;
            out.weakeningPercent = 7;
            out.breakDefensePosture = true;
            out.hint = "Le collet et les grosses attaches racinaires commandent une grande partie de la stabilité de la plante.";
            out.successLine = "Une attache racinaire se dérobe et la plante perd une partie de son ancrage.";
            break;

        case Race::Insectoide:
            if (armoredIdentity)
            {
                out.zoneId = "carapace_joint";
                out.zoneLabel = "joint de carapace";
                out.nextHitVulnerabilityTurns = 1;
                out.nextHitVulnerabilityPercent = 10;
                out.breakDefensePosture = true;
                out.hint = "La carapace encaisse de face ; ses jointures et la face interne des articulations sont moins épaisses.";
                out.successLine = "Le joint de carapace s'entrouvre et expose brièvement la protection interne.";
            }
            else
            {
                out.zoneId = "leg_joint";
                out.zoneLabel = "articulation de patte";
                out.weakeningTurns = 1;
                out.weakeningPercent = 5;
                out.hint = "Les articulations de pattes sont de petites cibles, mais elles contrôlent directement l'équilibre.";
                out.successLine = "Une articulation décroche et le déplacement devient moins assuré.";
            }
            break;

        case Race::Bete:
            if (armoredIdentity)
            {
                out.zoneId = "shell_hinge";
                out.zoneLabel = containsAny(identity, {"coquille", "escargot"}) ? "charnière de coquille" : "bord de protection";
                out.nextHitVulnerabilityTurns = 1;
                out.nextHitVulnerabilityPercent = 9;
                out.breakDefensePosture = true;
                out.hint = "La protection naturelle est épaisse, mais sa charnière et ses bords doivent rester mobiles.";
                out.successLine = "La protection naturelle se décale sans se briser et laisse une ouverture courte.";
            }
            else
            {
                out.zoneId = "tendon";
                out.zoneLabel = "tendon d'appui";
                out.weakeningTurns = 1;
                out.weakeningPercent = 4;
                out.hint = "Les tendons d'appui et changements de direction offrent de petites fenêtres plutôt qu'un vrai point faible fixe.";
                out.successLine = "Le tendon d'appui est pris au mauvais moment et la bête perd du rythme.";
            }
            break;

        case Race::Elementaire:
            out.zoneId = "focus_core";
            out.zoneLabel = "foyer élémentaire";
            out.revealTargetStats = true;
            out.vulnerabilityTurns = 1;
            out.vulnerabilityPercent = 6;
            out.hint = "La forme se reconstruit autour d'un foyer d'énergie plus stable que le reste du corps.";
            out.successLine = "Le foyer élémentaire se dévoile une fraction de seconde et la forme se disperse autour.";
            break;

        case Race::Esprit:
        case Race::Ange:
        case Race::Demon:
            out.zoneId = "spiritual_anchor";
            out.zoneLabel = "ancrage spirituel";
            out.revealTargetStats = true;
            out.vulnerabilityTurns = 1;
            out.vulnerabilityPercent = 5;
            out.hint = "La silhouette n'est pas entièrement matérielle ; certains points servent d'ancrage à sa forme.";
            out.successLine = "L'ancrage vacille et la forme devient brièvement plus facile à lire.";
            break;

        case Race::AnomalieArcanique:
            out.zoneId = "unstable_knot";
            out.zoneLabel = "nœud instable";
            out.revealTargetStats = true;
            out.vulnerabilityTurns = 1;
            out.vulnerabilityPercent = 7;
            out.reactiveHazard = true;
            out.hazardDamage = std::max(2, monster.getLevel() / 5 + 2);
            out.illusionTurns = 1;
            out.hint = "Un nœud d'énergie concentre l'anomalie, mais le toucher peut renvoyer une partie de l'instabilité.";
            out.successLine = "Le nœud se déforme et renvoie une brève décharge arcanique.";
            break;

        default:
            out.hint = "Aucune réaction anatomique spéciale évidente n'est connue pour cette cible.";
            break;
    }

    return out;
}

MonsterWeakPointHitResult MonsterAnatomySystem::resolveWeakPointReaction(Monster& monster, Player& player, Random& random)
{
    (void)random;
    MonsterWeakPointHitResult out;
    const MonsterWeakPointProfile profile = weakPointProfile(monster);
    if (!profile.reactiveHazard && !hasUsefulReaction(profile)) return out;

    out.triggered = true;

    if (!monster.isDead())
    {
        if (profile.revealTargetStats) monster.revealStats();
        if (profile.weakeningTurns > 0)
            monster.applyWeakening(profile.weakeningTurns, std::max(1, profile.weakeningPercent));
        if (profile.vulnerabilityTurns > 0)
            monster.applyVulnerability(profile.vulnerabilityTurns, std::max(1, profile.vulnerabilityPercent));
        if (profile.nextHitVulnerabilityTurns > 0)
            monster.applyNextHitVulnerability(profile.nextHitVulnerabilityTurns, std::max(1, profile.nextHitVulnerabilityPercent));
        if (profile.breakDefensePosture && monster.isInDefensePosture())
        {
            const std::string postureLabel = monster.getDefensePostureLabel();
            monster.clearDefensePosture();
            out.lines.push_back("La faille casse l'appui de la posture " + postureLabel + ".");
        }

        if (!profile.successLine.empty()) out.lines.push_back(profile.successLine);
    }

    if (profile.reactiveHazard)
    {
        out.playerDamage = std::max(1, profile.hazardDamage);
        player.takeDamage(out.playerDamage);
        if (profile.poisonTurns > 0) player.applyPoison(profile.poisonTurns, std::max(1, profile.poisonDamage));
        if (profile.burningTurns > 0) player.applyBurning(profile.burningTurns, std::max(1, profile.burningDamage));
        if (profile.illusionTurns > 0) player.applyIllusion(profile.illusionTurns);

        if (profile.zoneId == "trapped_skull")
            out.lines.push_back("La faille était un piège : le " + profile.zoneLabel + " éclate au contact.");
        else
            out.lines.push_back("La " + profile.zoneLabel + " réagit brutalement au contact.");

        out.lines.push_back(player.getName() + " encaisse " + std::to_string(out.playerDamage) + " dégâts en retour.");
        if (profile.poisonTurns > 0) out.lines.push_back("Des résidus toxiques empoisonnent la zone touchée.");
        if (profile.burningTurns > 0) out.lines.push_back("La déflagration laisse des brûlures actives.");
        if (profile.illusionTurns > 0) out.lines.push_back("Le contrecoup provoque aussi des hallucinations pendant un court moment.");
    }

    if (profile.selfDestructiveHazard && !monster.isDead())
    {
        monster.takeDamage(monster.getHp());
        out.targetDestroyedByReaction = monster.isDead();
        if (out.targetDestroyedByReaction)
            out.lines.push_back(monster.getName() + " est détruit par sa propre charge.");
    }

    return out;
}
