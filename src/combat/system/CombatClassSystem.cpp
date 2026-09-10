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
        className.find("voleur") != std::string::npos ||
        className.find("roublard") != std::string::npos ||
        className.find("brigand") != std::string::npos ||
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
        return 112;
    }

    if (className.find("voleur") != std::string::npos
        || className.find("roublard") != std::string::npos
        || className.find("brigand") != std::string::npos)
    {
        return 105;
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
        return 112;
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
        return 110;
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
        return 106;
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


int CombatClassSystem::getClassCriticalRollThreshold(const Entity& entity)
{
    const std::string className = normalizeClassText(entity.getType());

    if (containsAny(className, {
            "assassin", "ombrelame", "duelliste", "sabreur", "lanceur de dagues",
            "tireur", "archer", "éclaireur", "eclaireur", "danseur lunaire",
            "corsaire arcanique", "fauche-âme", "fauche-ame"
        }))
    {
        return 15;
    }

    if (containsAny(className, {
            "colosse", "gardien", "tank sac", "chevalier bouclier", "protecteur",
            "porte-bannière", "porte-banniere", "infirmier", "médecin", "medecin",
            "intendant", "aumônier", "aumonier"
        }))
    {
        return 17;
    }

    return 16;
}

std::string CombatClassSystem::getClassBalanceIdentityLine(const Entity& entity)
{
    const std::string className = normalizeClassText(entity.getType());

    if (className.find("éveillé") != std::string::npos || className.find("eveille") != std::string::npos || className.find("maître") != std::string::npos || className.find("maitre") != std::string::npos)
    {
        return "Profil évolué : la classe doit sentir un vrai cap franchi, mais dépend encore de son équipement, de son rôle et de ses fenêtres d'action.";
    }

    if (getClassCriticalRollThreshold(entity) <= 15)
    {
        return "Profil de classe : critique plus fréquent, mais la classe paie souvent ce rythme par fragilité ou dépendance au bon geste.";
    }

    if (getClassCriticalRollThreshold(entity) >= 17)
    {
        return "Profil de classe : critiques moins fréquents, mais meilleure tenue ou rôle défensif plus fiable.";
    }

    if (containsAny(className, {"pyromancien", "mage flame", "mage fou", "arcaniste", "démoniste", "demoniste", "chronomancien"}))
    {
        return "Profil de classe : dégâts magiques dangereux, mais dépendance à la fenêtre, au catalyseur et aux risques de canalisation.";
    }

    if (containsAny(className, {"barbare", "berserker", "ravageur", "briseur", "martelier", "faucheur"}))
    {
        return "Profil de classe : impact lourd et visible, mais précision, défense ou tempo plus exigeants.";
    }

    if (containsAny(className, {"clerc", "prêtre", "pretre", "barde", "oracle", "chantre", "support", "soutien"}))
    {
        return "Profil de classe : puissance directe plus basse, compensée par soutien, soins, lecture et stabilité.";
    }

    return "Profil de classe : rythme standard, pensé pour rester lisible sans extrême gratuit.";
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
        return 2;
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

    // EN: The affinity bonus must be felt, but it stays smaller than a real skill or rarity bonus.
    // FR: Le bonus d'affinité doit se ressentir, sans remplacer une vraie compétence ou rareté.
    const std::string className = normalizeClassText(entity.getType());
    int percent = 5;
    if (containsAny(className, {"maître d'armes", "maitre d'armes", "assassin", "ombrelame", "lanceur de dagues", "tireur", "archer", "pyromancien", "mage-lame", "chevalier runique"}))
    {
        percent = 8;
    }
    else if (containsAny(className, {"guerrier", "chevalier", "lancier", "berserker", "briseur", "forgeron", "runiste", "enchanteur", "druide"}))
    {
        percent = 6;
    }

    return std::max(2, currentDamage * percent / 100);
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
        return classContainsAny(className, {"assassin", "ombrelame", "voleur", "roublard", "brigand", "duelliste", "sabreur", "danseur lunaire", "lanceur de dagues", "messager arm", "pugiliste", "moine"});
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
            return 16;
        }
        if (classContainsAny(className, {"maître d'armes", "maitre d'armes", "duelliste", "tireur", "sabreur", "archer", "lancier"}))
        {
            return 14;
        }
        if (classContainsAny(className, {"briseur", "berserker", "colosse", "gardien", "forgeron"}))
        {
            return 8;
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
            return 109;
        }
        if (classContainsAny(className, {"maître d'armes", "maitre d'armes", "briseur", "berserker", "faucheur", "tireur", "pyromancien", "mage-lame", "chevalier runique"}))
        {
            return 110;
        }
        if (classContainsAny(className, {"gardien", "colosse", "support", "clerc", "prêtre", "pretre", "barde", "intendant"}))
        {
            return 103;
        }
        return 106;
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
        return "arme cohérente avec la classe : geste plus fiable, impact mieux transmis et bonus assez visible pour compter";
    }

    if (accuracy < 0 || damagePercent < 100)
    {
        return "arme maladroite pour la classe : précision instable, dégâts moins bien transmis et risque de gaspiller une bonne action";
    }

    return "arme utilisable sans vraie affinité ni gros malus";
}



bool CombatClassSystem::hasArmorAffinity(
    const Entity& entity,
    ArmorType armorType,
    const std::string& armorName
)
{
    const std::string className = normalizeClassText(entity.getType());
    const std::string armor = normalizeClassText(armorName);

    switch (armorType)
    {
        case ArmorType::Cloth:
            return classContainsAny(className, {"mage", "mancien", "sorcier", "arcaniste", "occultiste", "démoniste", "demoniste", "prêtre", "pretre", "clerc", "barde", "oracle", "bibliomancien", "moine", "pugiliste"})
                || armor.find("robe") != std::string::npos;
        case ArmorType::Leather:
            return isSwiftWeaponClass(className) || isRangedWeaponClass(className) || isCraftWeaponClass(className)
                || classContainsAny(className, {"chasseur", "rôdeur", "rodeur", "trappeur", "éclaireur", "eclaireur", "voleur", "assassin"});
        case ArmorType::Chainmail:
            return isMartialWeaponClass(className) || isSupportWeaponClass(className) || isHeavyWeaponClass(className)
                || classContainsAny(className, {"sentinelle", "garde", "paladin", "templier"});
        case ArmorType::Plate:
            return isHeavyWeaponClass(className) || classContainsAny(className, {"chevalier", "paladin", "templier", "gardien", "colosse", "tank", "martelier"});
        case ArmorType::Magical:
            return isMagicalWeaponClass(className) || isSupportWeaponClass(className) || classContainsAny(className, {"mage-lame", "chevalier runique", "runiste", "enchanteur"});
        case ArmorType::Unknown:
        default:
            return false;
    }
}

namespace
{
    bool isArmorClearlyAwkwardForClass(const std::string& className, ArmorType armorType, const std::string& armorName)
    {
        const bool swift = isSwiftWeaponClass(className);
        const bool ranged = isRangedWeaponClass(className);
        const bool heavy = isHeavyWeaponClass(className);
        const bool martial = isMartialWeaponClass(className);
        const bool magical = isMagicalWeaponClass(className);
        const bool support = isSupportWeaponClass(className);
        const bool craft = isCraftWeaponClass(className);
        const std::string armor = armorName;

        if (armorType == ArmorType::Plate)
        {
            return swift || ranged || (magical && !martial) || classContainsAny(className, {"moine", "pugiliste", "barde", "danseur"});
        }
        if (armorType == ArmorType::Cloth)
        {
            return (heavy && !magical) || classContainsAny(className, {"colosse", "briseur", "martelier", "tank"});
        }
        if (armorType == ArmorType::Magical)
        {
            return heavy && !magical && !support && !craft && armor.find("harnais") == std::string::npos;
        }
        if (armorType == ArmorType::Leather)
        {
            return classContainsAny(className, {"colosse", "tank sacré", "tank sacre", "gardien de porte"});
        }
        return false;
    }
}

int CombatClassSystem::getArmorHandlingDamageReductionAdjustment(
    const Entity& entity,
    ArmorType armorType,
    const std::string& armorName,
    int rawDamage
)
{
    const std::string className = normalizeClassText(entity.getType());
    const std::string armor = normalizeClassText(armorName);

    if (hasArmorAffinity(entity, armorType, armorName))
    {
        int bonus = 1;
        if (armorType == ArmorType::Plate || armorType == ArmorType::Chainmail)
        {
            bonus = classContainsAny(className, {"gardien", "colosse", "tank", "chevalier bouclier", "paladin", "templier"}) ? 3 : 2;
        }
        else if (armorType == ArmorType::Magical)
        {
            bonus = classContainsAny(className, {"mage", "mancien", "runiste", "oracle", "bibliomancien"}) ? 2 : 1;
        }
        else if (armorType == ArmorType::Leather && rawDamage <= 24)
        {
            bonus = 2;
        }
        return bonus;
    }

    if (isArmorClearlyAwkwardForClass(className, armorType, armor))
    {
        if (armorType == ArmorType::Plate)
        {
            return -2;
        }
        if (armorType == ArmorType::Cloth)
        {
            return -2;
        }
        return -1;
    }

    return 0;
}

int CombatClassSystem::getArmorHandlingEscapeAdjustment(
    const Entity& entity,
    ArmorType armorType,
    const std::string& armorName
)
{
    const std::string className = normalizeClassText(entity.getType());
    const std::string armor = normalizeClassText(armorName);

    if (hasArmorAffinity(entity, armorType, armorName))
    {
        if (armorType == ArmorType::Leather && (isSwiftWeaponClass(className) || isRangedWeaponClass(className))) return 4;
        if (armorType == ArmorType::Cloth && (isMagicalWeaponClass(className) || classContainsAny(className, {"moine", "pugiliste"}))) return 3;
        if (armorType == ArmorType::Plate && isHeavyWeaponClass(className)) return 1;
        return 0;
    }

    if (isArmorClearlyAwkwardForClass(className, armorType, armor))
    {
        if (armorType == ArmorType::Plate) return -8;
        if (armorType == ArmorType::Cloth && isHeavyWeaponClass(className)) return -2;
        return -4;
    }

    return 0;
}

std::string CombatClassSystem::getArmorHandlingLabel(
    const Entity& entity,
    ArmorType armorType,
    const std::string& armorName
)
{
    const int reductionAdjustment = getArmorHandlingDamageReductionAdjustment(entity, armorType, armorName, 24);
    const int escapeAdjustment = getArmorHandlingEscapeAdjustment(entity, armorType, armorName);

    if (reductionAdjustment > 0 || escapeAdjustment > 0)
    {
        return "armure cohérente avec la classe : la protection se place mieux, la mobilité reste plus naturelle et le rôle défensif se ressent";
    }
    if (reductionAdjustment < 0 || escapeAdjustment < 0)
    {
        return "armure maladroite pour la classe : protection mal exploitée, mobilité gênée ou mauvais compromis pour le rôle";
    }
    return "armure utilisable sans vraie affinité ni gros malus de classe";
}
