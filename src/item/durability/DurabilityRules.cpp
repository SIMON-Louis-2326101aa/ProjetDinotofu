// EN: DurabilityRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: DurabilityRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Lightweight durability and equipment-fit warnings used before the full material/size system.

#include "item/durability/DurabilityRules.hpp"

#include "item/armor/ArmorType.hpp"
#include "item/weapon/WeaponType.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    int durabilityPercent(int durability, int maxDurability)
    {
        if (maxDurability <= 0)
        {
            return 100;
        }
        durability = std::max(0, durability);
        return std::max(0, std::min(100, durability * 100 / maxDurability));
    }

    bool isSmallOrWingedRace(CharacterRace race)
    {
        return race == CharacterRace::Gnome
            || race == CharacterRace::Halfling
            || race == CharacterRace::Fairy
            || race == CharacterRace::SemiBird;
    }

    bool hasTailOrScales(CharacterRace race)
    {
        return race == CharacterRace::Kitsune
            || race == CharacterRace::SemiFox
            || race == CharacterRace::SemiCat
            || race == CharacterRace::SemiDog
            || race == CharacterRace::SemiWolf
            || race == CharacterRace::SemiLizard
            || race == CharacterRace::HalfDragon;
    }

    bool isLargeOrBrutalRace(CharacterRace race)
    {
        return race == CharacterRace::Orc
            || race == CharacterRace::HalfDragon
            || race == CharacterRace::Demon;
    }

    bool isLightFrameRace(CharacterRace race)
    {
        return race == CharacterRace::Fairy
            || race == CharacterRace::Gnome
            || race == CharacterRace::Halfling
            || race == CharacterRace::SemiBird;
    }
}

std::string DurabilityRules::weaponWearStateText(const Weapon& weapon)
{
    if (weapon.isIndestructible())
    {
        return "Usure : impossible à estimer, l'arme semble indestructible.";
    }

    if (weapon.isBroken())
    {
        return "Usure : cassée, réparation nécessaire avant usage sérieux.";
    }

    const int percent = durabilityPercent(weapon.getDurability(), weapon.getMaxDurability());
    if (percent <= 20)
    {
        return "Usure : critique, risque élevé de casse si le combat dure.";
    }
    if (percent <= 45)
    {
        return "Usure : fragile, une réparation préventive serait prudente.";
    }
    if (percent <= 75)
    {
        return "Usure : correcte, quelques marques commencent à apparaître.";
    }

    return "Usure : saine, l'arme répond encore bien.";
}

std::string DurabilityRules::armorWearStateText(const Armor& armor)
{
    if (armor.isIndestructible())
    {
        return "Usure : impossible à estimer, l'armure semble indestructible.";
    }

    if (armor.isBroken())
    {
        return "Usure : cassée, ses protections ne sont plus fiables.";
    }

    const int percent = durabilityPercent(armor.getDurability(), armor.getMaxDurability());
    if (percent <= 20)
    {
        return "Usure : critique, les attaches peuvent céder au mauvais moment.";
    }
    if (percent <= 45)
    {
        return "Usure : fragile, les sangles et plaques demandent une réparation.";
    }
    if (percent <= 75)
    {
        return "Usure : correcte, quelques points faibles sont visibles.";
    }

    return "Usure : saine, la protection tient encore bien.";
}

std::vector<std::string> DurabilityRules::describeWeaponUseWarnings(const Weapon& weapon, CharacterRace race)
{
    std::vector<std::string> lines;

    lines.push_back(weaponWearStateText(weapon));
    const int weaponPercent = durabilityPercent(weapon.getDurability(), weapon.getMaxDurability());
    if (!weapon.isIndestructible() && weaponPercent <= 20)
    {
        lines.push_back("Compatibilité : risque de rupture réel, surtout contre boss, automates, armures vivantes ou longs combats.");
        lines.push_back("Préparation : prévoir une arme secondaire ou un kit adapté avant une sortie de chasse/exploration.");
    }
    else if (!weapon.isIndestructible() && weaponPercent <= 45)
    {
        lines.push_back("Compatibilité : l'arme reste utilisable, mais chaque combat sérieux augmente le besoin de réparation.");
    }

    if (weapon.isBroken() || weapon.isIndestructible())
    {
        return lines;
    }

    switch (weapon.getType())
    {
        case WeaponType::Hammer:
        case WeaponType::Axe:
            if (isLightFrameRace(race))
            {
                lines.push_back("Compatibilité : arme lourde pour un gabarit léger, les combats longs fatigueront plus vite la prise.");
            }
            if (isLargeOrBrutalRace(race))
            {
                lines.push_back("Compatibilité : grande force naturelle, mais les impacts brutaux useront davantage le manche si l'entretien est ignoré.");
            }
            break;
        case WeaponType::Bow:
            if (race == CharacterRace::HalfDragon || race == CharacterRace::SemiLizard)
            {
                lines.push_back("Compatibilité : griffes/écailles possibles, la corde demande un contrôle plus propre pour éviter l'usure.");
            }
            else if (race == CharacterRace::SemiBird || race == CharacterRace::Elf || race == CharacterRace::SemiCat)
            {
                lines.push_back("Compatibilité : lecture fine des distances, bonne arme si l'entretien reste régulier.");
            }
            break;
        case WeaponType::Dagger:
            if (race == CharacterRace::SemiCat || race == CharacterRace::Kitsune || race == CharacterRace::SemiFox)
            {
                lines.push_back("Compatibilité : gestes rapides et discrets, bonne affinité naturelle avec les ouvertures courtes.");
            }
            break;
        case WeaponType::Staff:
            if (race == CharacterRace::Demon || race == CharacterRace::Tiefling || race == CharacterRace::Fairy || race == CharacterRace::Aasimar)
            {
                lines.push_back("Compatibilité : canalisation sensible, utile mais à surveiller si une malédiction touche le mana.");
            }
            break;
        case WeaponType::Spear:
            if (race == CharacterRace::SemiBird || race == CharacterRace::HalfDragon || race == CharacterRace::Orc)
            {
                lines.push_back("Compatibilité : bonne lecture de l'allonge, l'arme récompense les distances propres.");
            }
            break;
        default:
            break;
    }

    return lines;
}

std::vector<std::string> DurabilityRules::describeArmorFitWarnings(const Armor& armor, CharacterRace race)
{
    std::vector<std::string> lines;

    lines.push_back(armorWearStateText(armor));
    const int armorPercent = durabilityPercent(armor.getDurability(), armor.getMaxDurability());
    if (!armor.isIndestructible() && armorPercent <= 20)
    {
        lines.push_back("Ajustement : protection critique, les sangles peuvent céder avant que les PV ne préviennent clairement.");
        lines.push_back("Préparation : éviter une sortie longue sans réparation ou protection de rechange.");
    }
    else if (!armor.isIndestructible() && armorPercent <= 45)
    {
        lines.push_back("Ajustement : armure encore portable, mais réparation conseillée avant boss, donjon ou chasse longue.");
    }

    if (armor.isBroken() || armor.isIndestructible())
    {
        return lines;
    }

    switch (armor.getType())
    {
        case ArmorType::Plate:
            if (isSmallOrWingedRace(race))
            {
                lines.push_back("Ajustement racial : plaques lourdes délicates pour ce gabarit ou ces ailes. Une future armure taillée sur mesure serait préférable.");
            }
            if (hasTailOrScales(race))
            {
                lines.push_back("Ajustement racial : queue, écailles ou oreilles exigent des ouvertures propres, sinon l'usure des attaches augmente.");
            }
            break;
        case ArmorType::Chainmail:
            if (hasTailOrScales(race))
            {
                lines.push_back("Ajustement racial : la maille peut accrocher les écailles, fourrures ou appendices si elle n'est pas adaptée.");
            }
            break;
        case ArmorType::Leather:
            if (race == CharacterRace::SemiWolf || race == CharacterRace::SemiDog || race == CharacterRace::SemiCat || race == CharacterRace::SemiFox)
            {
                lines.push_back("Ajustement racial : cuir souple correct pour semi-humain, mais les sangles doivent éviter oreilles, queue et zones sensibles.");
            }
            if (isLargeOrBrutalRace(race))
            {
                lines.push_back("Ajustement racial : protection légère pour une morphologie puissante, risque d'usure plus rapide en combat frontal.");
            }
            break;
        case ArmorType::Cloth:
            if (race == CharacterRace::Fairy || race == CharacterRace::SemiBird)
            {
                lines.push_back("Ajustement racial : tissu facile à porter avec ailes/plumes, mais peu protecteur si l'ennemi accroche le corps.");
            }
            break;
        case ArmorType::Magical:
            lines.push_back("Ajustement magique : bonne base polyvalente, mais les malédictions peuvent perturber la réaction de l'armure.");
            break;
        default:
            break;
    }

    return lines;
}

std::vector<std::string> DurabilityRules::describeWeaponMaintenanceAdvice(const Weapon& weapon, CharacterRace race)
{
    std::vector<std::string> lines;
    lines.push_back("Entretien conseillé :");

    if (weapon.isIndestructible())
    {
        lines.push_back("- Aucun entretien courant : l'arme semble ignorer l'usure normale.");
        return lines;
    }

    if (weapon.isBroken())
    {
        lines.push_back("- Priorité absolue : réparation complète avant combat sérieux.");
        lines.push_back("- Un forgeron devrait vérifier si la qualité exceptionnelle n'a pas été dégradée par la casse.");
        return lines;
    }

    const int percent = durabilityPercent(weapon.getDurability(), weapon.getMaxDurability());
    if (percent <= 20)
    {
        lines.push_back("- Réparation urgente : éviter boss, longs combats et tests risqués tant que l'arme est critique.");
    }
    else if (percent <= 45)
    {
        lines.push_back("- Réparation préventive : l'arme peut servir, mais un mauvais combat peut la faire basculer.");
    }
    else if (percent <= 75)
    {
        lines.push_back("- Contrôle simple : nettoyer, resserrer et surveiller après quelques combats.");
    }
    else
    {
        lines.push_back("- Entretien léger : nettoyage et contrôle de routine suffisent pour l'instant.");
    }

    switch (weapon.getType())
    {
        case WeaponType::Sword:
            lines.push_back("- Lame : affûtage régulier et contrôle de garde après les ennemis blindés.");
            break;
        case WeaponType::Dagger:
            lines.push_back("- Dague : affûtage court, gaine propre et pointe vérifiée pour les attaques précises.");
            break;
        case WeaponType::Axe:
            lines.push_back("- Hache : vérifier le manche et le coin de fixation après impacts lourds.");
            break;
        case WeaponType::Hammer:
            lines.push_back("- Marteau : surveiller fissures du manche et déséquilibre de la tête.");
            break;
        case WeaponType::Spear:
            lines.push_back("- Lance : contrôler la hampe, la pointe et les attaches avant les combats longs.");
            break;
        case WeaponType::Staff:
            lines.push_back("- Bâton : nettoyer le catalyseur et vérifier les perturbations de mana/enchantement.");
            break;
        case WeaponType::Bow:
            lines.push_back("- Arc : corde, branches et tension à vérifier avant chaque expédition.");
            break;
        default:
            lines.push_back("- Arme spéciale : inspection manuelle conseillée, surtout si son type exact est mal classé.");
            break;
    }

    if (weapon.getEnchantmentCount() > 0)
    {
        lines.push_back("- Enchantement : bonus utile, mais l'entretien doit éviter de perturber les runes ou effets actifs.");
    }

    if (percent <= 45)
    {
        lines.push_back("- Matière : noter si l'usure vient d'impacts, corrosion, chaleur, froid ou magie avant de réparer au hasard.");
        lines.push_back("- Préparation terrain : contre automates/golems, privilégier contrôle de fixation ; en marais, sécher/nettoyer vite ; en montagne froide, vérifier fissures et métal fragilisé.");
    }

    if (isLightFrameRace(race) && (weapon.getType() == WeaponType::Axe || weapon.getType() == WeaponType::Hammer))
    {
        lines.push_back("- Morphologie : prise renforcée conseillée pour éviter fatigue et perte de contrôle sur arme lourde.");
    }
    if ((race == CharacterRace::SemiLizard || race == CharacterRace::HalfDragon) && weapon.getType() == WeaponType::Bow)
    {
        lines.push_back("- Morphologie : attention aux griffes/écailles proches de la corde.");
    }

    return lines;
}

std::vector<std::string> DurabilityRules::describeArmorMaintenanceAdvice(const Armor& armor, CharacterRace race)
{
    std::vector<std::string> lines;
    lines.push_back("Entretien conseillé :");

    if (armor.isIndestructible())
    {
        lines.push_back("- Aucun entretien courant : l'armure semble ignorer l'usure normale.");
        return lines;
    }

    if (armor.isBroken())
    {
        lines.push_back("- Priorité absolue : réparation complète avant de compter sur cette protection.");
        lines.push_back("- Vérifier attaches, doublure et plaques : une armure cassée peut blesser son porteur.");
        return lines;
    }

    const int percent = durabilityPercent(armor.getDurability(), armor.getMaxDurability());
    if (percent <= 20)
    {
        lines.push_back("- Réparation urgente : éviter les combats frontaux et les boss tant que les attaches sont critiques.");
    }
    else if (percent <= 45)
    {
        lines.push_back("- Réparation préventive : les sangles ou plaques commencent à devenir peu fiables.");
    }
    else if (percent <= 75)
    {
        lines.push_back("- Contrôle simple : resserrer les attaches et vérifier les points de frottement.");
    }
    else
    {
        lines.push_back("- Entretien léger : nettoyage, séchage et vérification de routine suffisent pour l'instant.");
    }

    switch (armor.getType())
    {
        case ArmorType::Cloth:
            lines.push_back("- Tissu : sécher vite, recoudre tôt, éviter feu/acide si possible.");
            break;
        case ArmorType::Leather:
            lines.push_back("- Cuir : graisser, assouplir les sangles et éviter qu'il sèche ou craque.");
            break;
        case ArmorType::Chainmail:
            lines.push_back("- Maille : retirer rouille, anneaux tordus et accroches dangereuses.");
            break;
        case ArmorType::Plate:
            lines.push_back("- Plaques : vérifier rivets, charnières, mobilité et doublure intérieure.");
            break;
        case ArmorType::Magical:
            lines.push_back("- Magique : contrôler la stabilité des runes avant toute réparation physique lourde.");
            break;
        default:
            lines.push_back("- Armure spéciale : inspection manuelle conseillée, surtout si son type exact est mal classé.");
            break;
    }

    if (armor.getEnchantmentCount() > 0)
    {
        lines.push_back("- Enchantement : réparer doucement autour des effets actifs pour éviter une réaction instable.");
    }

    if (percent <= 45)
    {
        lines.push_back("- Matière : vérifier si la faiblesse vient du cuir, des anneaux, des rivets, des plaques ou d'une couture morphologique.");
        lines.push_back("- Préparation terrain : humidité, sel, acide, froid, chaleur ou poussière de ruines peuvent empirer une usure déjà fragile.");
    }

    if (hasTailOrScales(race))
    {
        lines.push_back("- Morphologie : vérifier ouvertures de queue, oreilles, cornes, écailles ou fourrure avant départ.");
    }
    if (race == CharacterRace::SemiBird || race == CharacterRace::Fairy)
    {
        lines.push_back("- Morphologie : ne jamais bloquer les ailes ou l'équilibre aérien avec des attaches trop serrées.");
    }

    return lines;
}
