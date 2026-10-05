#include "combat/system/MonsterPreparedActionSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <cctype>

namespace
{
std::string lower(std::string value)
{
    for (char& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return value;
}

bool containsAny(const std::string& value, const std::initializer_list<const char*>& needles)
{
    const std::string text = lower(value);
    for (const char* needle : needles)
    {
        if (text.find(lower(needle)) != std::string::npos) return true;
    }
    return false;
}
}

std::string MonsterPreparedActionSystem::familyForLabel(const std::string& label)
{
    if (containsAny(label, {"rituel", "serment", "sceau", "invocation", "prière", "priere"})) return "rituel";
    if (containsAny(label, {"souffle", "haleine", "brume ardente", "brume acide", "jet de feu", "jet de glace"})) return "souffle_elementaire";
    if (containsAny(label, {"plongeon", "piqué", "pique aérien", "pique aerien", "serres", "attaque plongeante"})) return "plongee_aerienne";
    if (containsAny(label, {"venin", "toxine", "dard", "crochet empoisonné", "crochet empoisonne"})) return "venin_prepare";
    if (containsAny(label, {"tir", "canon", "volée", "volee", "flèche", "fleche", "projectile"})) return "tir_lourd";
    if (containsAny(label, {"toile", "racine", "ronce", "liane", "entrave"})) return "entrave_massive";
    if (containsAny(label, {"cri", "hurlement", "meute", "ordre", "appel"})) return "cri_de_meute";
    if (containsAny(label, {"charge", "heurt", "marteau", "massue", "colosse", "écras", "ecras"})) return "charge_lourde";
    return "frappe_engagee";
}

std::string MonsterPreparedActionSystem::telegraphLineForLabel(const std::string& label)
{
    const std::string family = familyForLabel(label);
    if (family == "rituel") return "Télégraphe visible : gestes répétés, souffle retenu ou symbole maintenu ; le rituel demande de la continuité.";
    if (family == "souffle_elementaire") return "Télégraphe visible : gorge, thorax ou noyau élémentaire se charge ; le souffle est dangereux parce qu'il accumule sa pression avant d'être libéré.";
    if (family == "plongee_aerienne") return "Télégraphe visible : la créature prend de la hauteur ou verrouille ses ailes ; casser sa trajectoire avant le piqué évite l'impact plein.";
    if (family == "venin_prepare") return "Télégraphe visible : dard, crochets ou glandes restent exposés un instant ; la préparation cherche surtout à inoculer, pas à frapper fort.";
    if (family == "tir_lourd") return "Télégraphe visible : l'arme ou le membre de tir reste aligné trop longtemps ; la fenêtre vient avant le départ du projectile.";
    if (family == "entrave_massive") return "Télégraphe visible : fils, racines ou matière de contrôle sont rassemblés avant d'être projetés.";
    if (family == "cri_de_meute") return "Télégraphe visible : posture haute, souffle profond et attention du groupe convergent avant l'appel.";
    if (family == "charge_lourde") return "Télégraphe visible : appuis verrouillés et poids transféré vers l'avant ; casser l'élan reste possible avant l'impact.";
    return "Télégraphe visible : le geste est engagé assez longtemps pour être reconnu et contesté.";
}

std::string MonsterPreparedActionSystem::interruptHintForLabel(const std::string& label)
{
    const std::string family = familyForLabel(label);
    if (family == "rituel") return "Fenêtre : le rituel tolère mal une interruption précoce ; choc, givre, entrave ou environ 8 % de ses PV max en dégâts immédiats peuvent suffire.";
    if (family == "souffle_elementaire") return "Fenêtre : comprimer un souffle demande de tenir le rythme ; choc, givre, entrave ou environ 10 % des PV max peuvent casser la charge.";
    if (family == "plongee_aerienne") return "Fenêtre : la trajectoire reste solide mais lisible ; choc, givre, entrave ou environ 11 % des PV max peuvent briser le piqué avant l'impact.";
    if (family == "venin_prepare") return "Fenêtre : la préparation toxique est fragile ; choc, givre, entrave ou environ 6 % des PV max suffisent souvent à empêcher l'inoculation.";
    if (family == "tir_lourd") return "Fenêtre : dérégler la visée est plus facile que stopper une charge ; choc, givre, entrave ou environ 9 % des PV max peuvent casser le tir.";
    if (family == "entrave_massive") return "Fenêtre : la matière rassemblée est fragile ; choc, givre, entrave ou environ 7 % des PV max peuvent rompre la préparation.";
    if (family == "cri_de_meute") return "Fenêtre : le souffle et le rythme comptent ; choc, givre, entrave ou environ 8 % des PV max peuvent casser l'appel.";
    if (family == "charge_lourde") return "Fenêtre : l'élan est robuste ; choc, givre, entrave ou environ 12 % des PV max sont nécessaires pour stopper la charge par dégâts seuls.";
    return "Fenêtre : choc, givre, entrave ou environ 10 % des PV max en dégâts immédiats interrompent ce geste.";
}

MonsterPreparedActionResolution MonsterPreparedActionSystem::resolve(Monster& monster, Player& player)
{
    MonsterPreparedActionResolution result;
    if (!monster.hasPreparedSignature()) return result;

    result.hadPreparation = true;
    result.tier = std::max(1, monster.getPreparedSignatureTier());
    result.label = monster.getPreparedSignatureLabel();
    result.family = familyForLabel(result.label);

    if (result.family == "charge_lourde") result.damageInterruptPercent = 12;
    else if (result.family == "rituel") result.damageInterruptPercent = 8;
    else if (result.family == "souffle_elementaire") result.damageInterruptPercent = 10;
    else if (result.family == "plongee_aerienne") result.damageInterruptPercent = 11;
    else if (result.family == "venin_prepare") result.damageInterruptPercent = 6;
    else if (result.family == "tir_lourd") result.damageInterruptPercent = 9;
    else if (result.family == "entrave_massive") result.damageInterruptPercent = 7;
    else if (result.family == "cri_de_meute") result.damageInterruptPercent = 8;

    const int damageTakenSincePreparation = std::max(0, monster.getPreparedSignatureHpSnapshot() - monster.getHp());
    const int damageInterruptThreshold = std::max(4, monster.getMaxHp() * result.damageInterruptPercent / 100);

    if (monster.hasShock())
    {
        result.interrupted = true;
        result.interruptReason = "le choc a cassé la concentration";
    }
    else if (monster.hasFrost())
    {
        result.interrupted = true;
        result.interruptReason = "le givre a brisé l'élan";
    }
    else if (monster.hasEntanglement())
    {
        result.interrupted = true;
        result.interruptReason = "l'entrave a empêché le geste préparé";
    }
    else if (damageTakenSincePreparation >= damageInterruptThreshold)
    {
        result.interrupted = true;
        result.interruptReason = "la pression immédiate a infligé assez de dégâts pour casser la préparation";
    }

    if (result.interrupted)
    {
        monster.clearPreparedSignature();
        monster.applyVulnerability(1, 6 + result.tier * 2);
        return result;
    }

    result.resolved = true;
    const int base = std::max(6, monster.getMaxDamage() + monster.getLevel() / 3 + result.tier * 3);
    auto applyPreparedDamage = [&](int rawDamage)
    {
        const int actualDamage = DefensePostureSystem::reduceIncomingDamage(player, std::max(0, rawDamage));
        player.takeDamage(actualDamage);
        result.damage = actualDamage;
    };

    if (result.family == "charge_lourde")
    {
        result.damage = base + 4 + result.tier * 2;
        result.forcedReposition = true;
        result.effectLine = "L'impact casse les appuis et force à reprendre la position.";
        applyPreparedDamage(result.damage);
        player.applyWeakening(1, 6 + result.tier * 2);
        player.applyNextHitVulnerability(1, 6 + result.tier * 2);
    }
    else if (result.family == "souffle_elementaire")
    {
        result.damage = base + 2 + result.tier;
        result.effectLine = "Le souffle chargé frappe surtout par pression et exposition : il laisse le corps affaibli même sans déplacement forcé.";
        applyPreparedDamage(result.damage);
        player.applyWeakening(2, 5 + result.tier * 2);
        player.applyNextHitVulnerability(1, 3 + result.tier);
    }
    else if (result.family == "plongee_aerienne")
    {
        result.damage = base + 5 + result.tier;
        result.forcedReposition = true;
        result.effectLine = "Le piqué traverse la ligne de garde et force une reprise d'appui après l'impact.";
        applyPreparedDamage(result.damage);
        player.applyWeakening(1, 4 + result.tier);
        player.applyNextHitVulnerability(1, 8 + result.tier * 2);
    }
    else if (result.family == "venin_prepare")
    {
        result.damage = std::max(3, base - 4);
        result.effectLine = "La frappe directe reste modeste : le vrai danger vient du venin préparé qui continue d'agir après l'impact.";
        applyPreparedDamage(result.damage);
        player.applyPoison(2 + result.tier / 2, 2 + result.tier);
    }
    else if (result.family == "tir_lourd")
    {
        result.damage = base + 3;
        result.effectLine = "Le tir engagé ne pousse pas forcément, mais laisse une ouverture nette après l'esquive ou l'impact.";
        applyPreparedDamage(result.damage);
        player.applyNextHitVulnerability(1, 7 + result.tier * 2);
    }
    else if (result.family == "entrave_massive")
    {
        result.damage = std::max(4, base - 3);
        result.effectLine = "La matière préparée se referme sur les appuis : l'entrave devient la vraie menace, pas les dégâts bruts.";
        applyPreparedDamage(result.damage);
        player.applyEntanglement(1);
        player.applyWeakening(1, 4 + result.tier);
    }
    else if (result.family == "cri_de_meute")
    {
        result.damage = std::max(3, base - 5);
        result.effectLine = "Le cri secoue surtout le rythme : la prochaine pression ennemie devient plus dangereuse.";
        applyPreparedDamage(result.damage);
        player.applyWeakening(2, 5 + result.tier * 2);
        monster.applyPrecisionBoost(1, 1);
    }
    else if (result.family == "rituel")
    {
        result.damage = base;
        result.effectLine = "Le rituel aboutit sans déplacement forcé, mais laisse une faiblesse plus longue à exploiter par le groupe ennemi.";
        applyPreparedDamage(result.damage);
        player.applyWeakening(2, 6 + result.tier * 2);
        player.applyNextHitVulnerability(1, 4 + result.tier);
    }
    else
    {
        result.damage = base;
        result.forcedReposition = true;
        result.effectLine = "Le geste engagé rompt brièvement les appuis.";
        applyPreparedDamage(result.damage);
        player.applyWeakening(1, 5 + result.tier * 2);
        player.applyNextHitVulnerability(1, 5 + result.tier * 2);
    }

    monster.clearPreparedSignature();
    return result;
}
