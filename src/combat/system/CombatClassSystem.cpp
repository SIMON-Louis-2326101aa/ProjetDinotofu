// EN: CombatClassSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatClassSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/CombatClassSystem.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <initializer_list>


namespace
{
    bool containsAny(const std::string& value, std::initializer_list<const char*> terms)
    {
        for (const char* term : terms)
        {
            if (value.find(term) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }
}

std::string CombatClassSystem::normalizeClassText(const std::string& classText)
{
    std::string resultat = classText;

    std::transform(
        resultat.begin(),
        resultat.end(),
        resultat.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char caractere)
        {
            return static_cast<char>(std::tolower(caractere));
        }
    );

    return resultat;
}

// EN: getBaseEscapeChance declares or implements a focused behavior used by this module.
// FR: getBaseEscapeChance déclare ou implémente un comportement précis utilisé par ce module.
int CombatClassSystem::getBaseEscapeChance(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("assassin") != std::string::npos ||
        className.find("ombrelame") != std::string::npos ||
        className.find("moine") != std::string::npos ||
        className.find("duelliste") != std::string::npos ||
        className.find("pugiliste") != std::string::npos ||
        className.find("sabreur") != std::string::npos ||
        className.find("aéromancien") != std::string::npos ||
        className.find("aeromancien") != std::string::npos ||
        className.find("danseur lunaire") != std::string::npos ||
        className.find("messager arm") != std::string::npos ||
        className.find("légère") != std::string::npos ||
        className.find("legere") != std::string::npos)
    {
        return 72;
    }

    if (className.find("archer") != std::string::npos ||
        className.find("rôdeur") != std::string::npos ||
        className.find("rodeur") != std::string::npos ||
        className.find("chasseur") != std::string::npos ||
        className.find("lanceur de dagues") != std::string::npos ||
        className.find("tireur") != std::string::npos ||
        className.find("artificier") != std::string::npos ||
        className.find("javelinier") != std::string::npos ||
        className.find("trappeur") != std::string::npos ||
        className.find("guetteur") != std::string::npos)
    {
        return 62;
    }

    if (className.find("colosse") != std::string::npos ||
        className.find("gardien") != std::string::npos ||
        className.find("tank") != std::string::npos ||
        className.find("briseur lourd") != std::string::npos ||
        className.find("lourde") != std::string::npos)
    {
        return 28;
    }

    if (className.find("chevalier") != std::string::npos ||
        className.find("paladin") != std::string::npos ||
        className.find("templier") != std::string::npos ||
        className.find("clerc") != std::string::npos ||
        className.find("prêtre") != std::string::npos ||
        className.find("pretre") != std::string::npos)
    {
        return 48;
    }

    return 50;
}

// EN: getBaseDamageReductionPercentage declares or implements a focused behavior used by this module.
// FR: getBaseDamageReductionPercentage déclare ou implémente un comportement précis utilisé par ce module.
int CombatClassSystem::getBaseDamageReductionPercentage(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("assassin") != std::string::npos ||
        className.find("mage fou") != std::string::npos ||
        className.find("arcaniste") != std::string::npos ||
        className.find("légère") != std::string::npos ||
        className.find("legere") != std::string::npos)
    {
        return 0;
    }

    if (className.find("gardien") != std::string::npos
        || className.find("tank sac") != std::string::npos
        || className.find("chevalier bouclier") != std::string::npos
        || className.find("porte-bannière") != std::string::npos
        || className.find("porte-banniere") != std::string::npos)
    {
        return 16;
    }

    if (className.find("colosse") != std::string::npos ||
        className.find("paladin") != std::string::npos ||
        className.find("templier") != std::string::npos ||
        className.find("lourde") != std::string::npos)
    {
        return 11;
    }

    if (className.find("chevalier") != std::string::npos ||
        className.find("guerrier") != std::string::npos ||
        className.find("clerc") != std::string::npos ||
        className.find("prêtre") != std::string::npos ||
        className.find("pretre") != std::string::npos ||
        className.find("druide") != std::string::npos
        || className.find("géomancien") != std::string::npos
        || className.find("geomancien") != std::string::npos
        || className.find("hydromancien") != std::string::npos
        || className.find("lame tellurique") != std::string::npos
        || className.find("juge novice") != std::string::npos)
    {
        return 6;
    }

    return 0;
}


int CombatClassSystem::getOutgoingDamagePercent(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("gardien") != std::string::npos
        || className.find("tank sac") != std::string::npos
        || className.find("chevalier bouclier") != std::string::npos
        || className.find("porte-bannière") != std::string::npos
        || className.find("porte-banniere") != std::string::npos
        || className.find("intendant") != std::string::npos)
    {
        return 82;
    }

    if (className.find("colosse") != std::string::npos
        || className.find("paladin") != std::string::npos)
    {
        return 92;
    }

    if (className.find("assassin") != std::string::npos
        || className.find("ombrelame") != std::string::npos
        || className.find("lanceur de dagues") != std::string::npos)
    {
        return 116;
    }

    if (className.find("duelliste") != std::string::npos)
    {
        return 108;
    }

    if (className.find("berserker") != std::string::npos
        || className.find("briseur lourd") != std::string::npos
        || className.find("barbare") != std::string::npos
        || className.find("faucheur") != std::string::npos
        || className.find("ravageur") != std::string::npos)
    {
        return 115;
    }

    if (className.find("duelliste") != std::string::npos
        || className.find("guerrier") != std::string::npos
        || className.find("chevalier runique") != std::string::npos
        || className.find("mage-lame") != std::string::npos
        || className.find("sabreur") != std::string::npos
        || className.find("lame tellurique") != std::string::npos
        || className.find("chaman de guerre") != std::string::npos)
    {
        return 106;
    }

    if (className.find("pyromancien") != std::string::npos
        || className.find("mage flame") != std::string::npos
        || className.find("mage fou") != std::string::npos
        || className.find("électromancien") != std::string::npos
        || className.find("electromancien") != std::string::npos
        || className.find("démoniste") != std::string::npos
        || className.find("demoniste") != std::string::npos
        || className.find("chronomancien") != std::string::npos)
    {
        return 113;
    }

    if (className.find("mage") != std::string::npos
        || className.find("sorcier") != std::string::npos
        || className.find("ensorceleur") != std::string::npos
        || className.find("arcaniste") != std::string::npos
        || className.find("occultiste") != std::string::npos
        || className.find("hydromancien") != std::string::npos
        || className.find("géomancien") != std::string::npos
        || className.find("geomancien") != std::string::npos
        || className.find("aéromancien") != std::string::npos
        || className.find("aeromancien") != std::string::npos
        || className.find("runiste") != std::string::npos
        || className.find("enchanteur") != std::string::npos)
    {
        return 108;
    }

    if (className.find("clerc") != std::string::npos
        || className.find("prêtre") != std::string::npos
        || className.find("pretre") != std::string::npos
        || className.find("barde") != std::string::npos
        || className.find("alchimiste") != std::string::npos
        || className.find("médecin") != std::string::npos
        || className.find("medecin") != std::string::npos
        || className.find("infirmier") != std::string::npos
        || className.find("intendant") != std::string::npos)
    {
        return 94;
    }

    return 100;
}

int CombatClassSystem::getOutgoingFlatBonus(const Entity& entity)
{
    std::string className = normalizeClassText(entity.getType());

    if (className.find("lancier") != std::string::npos || className.find("javelinier") != std::string::npos)
    {
        return 3;
    }

    if (className.find("archer") != std::string::npos
        || className.find("rôdeur") != std::string::npos
        || className.find("rodeur") != std::string::npos
        || className.find("arbal") != std::string::npos
        || className.find("tireur") != std::string::npos
        || className.find("guetteur") != std::string::npos
        || className.find("trappeur") != std::string::npos)
    {
        return 2;
    }

    if (className.find("forgeron") != std::string::npos)
    {
        return 3;
    }

    if (className.find("arbal") != std::string::npos || className.find("tireur") != std::string::npos)
    {
        return 4;
    }

    if (className.find("assassin") != std::string::npos
        || className.find("ombrelame") != std::string::npos
        || className.find("lanceur de dagues") != std::string::npos)
    {
        return 3;
    }

    if (className.find("moine") != std::string::npos
        || className.find("pugiliste") != std::string::npos
        || className.find("sabreur") != std::string::npos
        || className.find("duelliste") != std::string::npos)
    {
        return 2;
    }

    return 0;
}


bool CombatClassSystem::hasWeaponAffinity(
    const Entity& entity,
    WeaponType weaponType,
    const std::string& weaponName
)
{
    const std::string className = normalizeClassText(entity.getType());
    const std::string weapon = normalizeClassText(weaponName);

    switch (weaponType)
    {
        case WeaponType::Sword:
            return containsAny(className, {"épéiste", "epeiste", "chevalier", "guerrier", "duelliste", "paladin", "templier", "mage-lame", "sabreur", "faucheur", "lame tellurique", "juge novice"});
        case WeaponType::Dagger:
            return containsAny(className, {"assassin", "ombrelame", "voleur", "lanceur de dagues", "dague", "danseur lunaire", "messager arm"});
        case WeaponType::Spear:
            return containsAny(className, {"lancier", "javelinier", "garde", "chevalier", "sentinelle"});
        case WeaponType::Bow:
            return containsAny(className, {"archer", "rôdeur", "rodeur", "chasseur", "tireur", "arbal", "trappeur", "guetteur", "messager arm"})
                || weapon.find("arbal") != std::string::npos;
        case WeaponType::Staff:
            return containsAny(className, {"mage", "sorcier", "arcaniste", "clerc", "prêtre", "pretre", "druide", "invoc", "nécro", "necro", "occultiste", "mancien", "démoniste", "demoniste", "runiste", "enchanteur", "oracle"});
        case WeaponType::Axe:
        case WeaponType::Hammer:
            return containsAny(className, {"berserker", "barbare", "briseur", "colosse", "forgeron", "gardien", "orc", "faucheur", "siège", "siege", "lame tellurique"});
        case WeaponType::BareHands:
            return containsAny(className, {"moine", "pugiliste", "bagarreur"});
        default:
            return false;
    }
}

int CombatClassSystem::getWeaponAffinityDamageBonus(
    const Entity& entity,
    WeaponType weaponType,
    const std::string& weaponName,
    int currentDamage
)
{
    if (currentDamage <= 0 || !hasWeaponAffinity(entity, weaponType, weaponName))
    {
        return 0;
    }

    // EN: Very small mastery bonus: enough to reward coherent equipment, not enough to force a meta.
    // FR: Très léger bonus de maîtrise : il récompense l'équipement cohérent sans imposer une méta.
    return std::max(1, currentDamage * 3 / 100);
}

std::string CombatClassSystem::getWeaponAffinityLabel(
    const Entity& entity,
    WeaponType weaponType,
    const std::string& weaponName
)
{
    if (!hasWeaponAffinity(entity, weaponType, weaponName))
    {
        return "";
    }

    switch (weaponType)
    {
        case WeaponType::Sword: return "la classe sait manier ce type de lame";
        case WeaponType::Dagger: return "la classe exploite bien les armes courtes";
        case WeaponType::Spear: return "la classe profite naturellement de l'allonge";
        case WeaponType::Bow: return "la classe sait garder une vraie ligne de tir";
        case WeaponType::Staff: return "la classe canalise mieux avec ce support";
        case WeaponType::Axe:
        case WeaponType::Hammer: return "la classe transforme mieux la force brute";
        case WeaponType::BareHands: return "la classe sait se battre sans arme lourde";
        default: return "";
    }
}

namespace
{
    bool classContainsAny(const std::string& className, std::initializer_list<const char*> terms)
    {
        return containsAny(className, terms);
    }

    bool isSwiftWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"assassin", "ombrelame", "duelliste", "sabreur", "danseur lunaire", "lanceur de dagues", "messager arm", "pugiliste", "moine"});
    }

    bool isRangedWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"archer", "rôdeur", "rodeur", "chasseur", "arbal", "tireur", "trappeur", "guetteur", "pisteur", "frondeur", "fauconnier", "javelinier"});
    }

    bool isHeavyWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"barbare", "berserker", "briseur", "colosse", "gardien", "martelier", "forgeron", "ravageur", "lame tellurique", "chevalier bouclier", "tank"});
    }

    bool isMartialWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"chevalier", "guerrier", "gladiateur", "maître d'armes", "maitre d'armes", "paladin", "templier", "lancier", "hallebardier", "épéiste", "epeiste", "faucheur"});
    }

    bool isMagicalWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"mage", "mancien", "sorcier", "ensorceleur", "arcaniste", "occultiste", "démoniste", "demoniste", "invoc", "nécro", "necro", "runiste", "enchanteur", "oracle", "bibliomancien"});
    }

    bool isSupportWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"clerc", "prêtre", "pretre", "barde", "médecin", "medecin", "infirmier", "intendant", "aumônier", "aumonier", "archiviste"});
    }

    bool isCraftWeaponClass(const std::string& className)
    {
        return classContainsAny(className, {"artificier", "forgeron", "alchimiste", "cuisinier", "cartographe", "récupérateur", "recuperateur", "runiste", "enchanteur", "bricoleur", "mécanicien", "mecanicien"});
    }

    bool isWeaponClearlyAwkward(const std::string& className, WeaponType weaponType, const std::string& weaponName)
    {
        const bool swift = isSwiftWeaponClass(className);
        const bool ranged = isRangedWeaponClass(className);
        const bool heavy = isHeavyWeaponClass(className);
        const bool martial = isMartialWeaponClass(className);
        const bool magical = isMagicalWeaponClass(className);
        const bool support = isSupportWeaponClass(className);
        const bool craft = isCraftWeaponClass(className);

        if (weaponType == WeaponType::BareHands)
        {
            return !classContainsAny(className, {"moine", "pugiliste", "bagarreur", "cogneur"});
        }

        if (magical && !martial && !craft)
        {
            return weaponType == WeaponType::Axe || weaponType == WeaponType::Hammer || weaponType == WeaponType::Bow;
        }

        if (swift)
        {
            return weaponType == WeaponType::Axe || weaponType == WeaponType::Hammer || weaponType == WeaponType::Staff;
        }

        if (ranged)
        {
            return weaponType == WeaponType::Hammer || weaponType == WeaponType::Axe || (weaponType == WeaponType::Staff && weaponName.find("arbal") == std::string::npos);
        }

        if ((heavy || martial) && !magical)
        {
            return weaponType == WeaponType::Staff || weaponType == WeaponType::Bow;
        }

        if (support && !martial && !magical)
        {
            return weaponType == WeaponType::Axe || weaponType == WeaponType::Hammer;
        }

        return false;
    }
}

int CombatClassSystem::getWeaponHandlingAccuracyAdjustment(
    const Entity& entity,
    WeaponType weaponType,
    const std::string& weaponName
)
{
    const std::string className = normalizeClassText(entity.getType());
    const std::string weapon = normalizeClassText(weaponName);

    if (hasWeaponAffinity(entity, weaponType, weaponName))
    {
        if (classContainsAny(className, {"assassin", "ombrelame", "lanceur de dagues"}))
        {
            return 18;
        }
        if (classContainsAny(className, {"maître d'armes", "maitre d'armes", "duelliste", "tireur", "sabreur", "archer", "lancier"}))
        {
            return 16;
        }
        return 10;
    }

    if (isWeaponClearlyAwkward(className, weaponType, weapon))
    {
        if (weaponType == WeaponType::BareHands)
        {
            return -18;
        }
        return -12;
    }

    return 0;
}

int CombatClassSystem::getWeaponHandlingDamagePercent(
    const Entity& entity,
    WeaponType weaponType,
    const std::string& weaponName
)
{
    const std::string className = normalizeClassText(entity.getType());
    const std::string weapon = normalizeClassText(weaponName);

    if (hasWeaponAffinity(entity, weaponType, weaponName))
    {
        if (classContainsAny(className, {"assassin", "ombrelame", "lanceur de dagues"}))
        {
            return 110;
        }
        if (classContainsAny(className, {"maître d'armes", "maitre d'armes", "briseur", "berserker", "faucheur", "tireur", "pyromancien", "mage-lame", "chevalier runique"}))
        {
            return 108;
        }
        return 104;
    }

    if (isWeaponClearlyAwkward(className, weaponType, weapon))
    {
        if (weaponType == WeaponType::BareHands)
        {
            return 78;
        }
        return 88;
    }

    return 100;
}

std::string CombatClassSystem::getWeaponHandlingLabel(
    const Entity& entity,
    WeaponType weaponType,
    const std::string& weaponName
)
{
    const int accuracy = getWeaponHandlingAccuracyAdjustment(entity, weaponType, weaponName);
    const int damagePercent = getWeaponHandlingDamagePercent(entity, weaponType, weaponName);

    if (accuracy > 0 || damagePercent > 100)
    {
        return "arme cohérente avec la classe : geste plus fiable et impact mieux transmis";
    }

    if (accuracy < 0 || damagePercent < 100)
    {
        return "arme maladroite pour la classe : précision instable et dégâts moins bien transmis";
    }

    return "arme utilisable sans vraie affinité ni gros malus";
}

