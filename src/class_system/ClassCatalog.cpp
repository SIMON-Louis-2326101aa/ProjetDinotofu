// EN: ClassCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ClassCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Central catalog for playable classes. It keeps the old direct index access, while adding category menus.
// Description : Catalogue central des classes jouables. Il conserve l'ancien accès par index direct tout en ajoutant les menus par catégories.

#include "class_system/ClassCatalog.hpp"

#include <algorithm>
#include <cctype>
#include "interface/menu/common/MessageScreen.hpp"

#include <vector>

namespace
{
    struct ClassTemplate
    {
        const char* name;
        int maxHp;
        int minDamage;
        int maxDamage;
        int criticalDamage;
        int healingPotionCount;
        int damagePotionCount;
        const char* role;
        ClassCategory category;
    };

    // EN: getClassTemplates declares or implements a focused behavior used by this module.
    // FR: getClassTemplates déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<ClassTemplate>& getClassTemplates()
    {
        static const std::vector<ClassTemplate> classes = {
            {"Chevalier", 200, 5, 20, 30, 4, 3, "équilibré, simple et fiable", ClassCategory::Melee},
            {"Assassin", 150, 11, 31, 48, 6, 2, "rapide, fragile, gros critique", ClassCategory::Melee},
            {"Colosse", 400, 4, 10, 18, 2, 1, "très tanky, lent, lourd", ClassCategory::Melee},
            {"Barbare", 260, 12, 28, 38, 3, 3, "brutal, violent, peu subtil", ClassCategory::Melee},
            {"Guerrier", 230, 9, 24, 34, 4, 3, "maîtrise des armes", ClassCategory::Melee},
            {"Moine", 175, 9, 25, 36, 5, 2, "esquive, discipline et poings", ClassCategory::Melee},
            {"Duelliste", 170, 12, 27, 43, 4, 2, "1v1, style et précision", ClassCategory::Melee},
            {"Berserker", 240, 14, 34, 46, 2, 4, "énormes dégâts, défense discutable", ClassCategory::Melee},
            {"Lancier", 210, 10, 27, 39, 4, 3, "portée, contrôle et contre-attaque", ClassCategory::Melee},
            {"Briseur lourd", 300, 15, 38, 58, 2, 2, "précision instable, dégâts énormes", ClassCategory::Melee},
            {"Gladiateur", 220, 13, 30, 44, 3, 3, "arène, riposte et pression constante", ClassCategory::Melee},
            {"Ravageur", 255, 16, 36, 52, 2, 4, "attaque frontale, casse-garde et risque", ClassCategory::Melee},
            {"Maître d'armes", 235, 11, 29, 42, 4, 3, "polyvalence martiale et lecture d'équipement", ClassCategory::Melee},
            {"Pugiliste", 185, 10, 27, 42, 5, 2, "poings, garde courte et tempo nerveux", ClassCategory::Melee},
            {"Faucheur", 205, 13, 32, 49, 3, 4, "grande lame, pression et finition risquée", ClassCategory::Melee},
            {"Sabreur rapide", 175, 13, 29, 47, 4, 2, "lame légère, initiative et critiques propres", ClassCategory::Melee},
            {"Chevalier bouclier", 285, 7, 20, 31, 5, 2, "protection frontale, contre et stabilité", ClassCategory::Melee},
            {"Hallebardier", 225, 11, 31, 45, 4, 3, "allonge, balayage et garde d'entrée", ClassCategory::Melee},
            {"Épéiste errant", 195, 12, 28, 44, 4, 3, "duel mobile, lecture et survie de voyage", ClassCategory::Melee},
            {"Cogneur de taverne", 210, 13, 30, 41, 4, 2, "bagarre sale, pression courte et résistance bancale", ClassCategory::Melee},
            {"Gardien de porte", 275, 8, 23, 35, 5, 2, "tenir un passage, empêcher le contournement", ClassCategory::Melee},
            {"Lame brisée", 180, 14, 32, 50, 3, 4, "risque, revanche et coups décisifs", ClassCategory::Melee},
            {"Martelier", 245, 15, 35, 54, 2, 3, "marteau, armure et brise-construction", ClassCategory::Melee},

            {"Archer", 160, 10, 29, 44, 5, 3, "distance, précision et mobilité", ClassCategory::Distance},
            {"Rôdeur", 180, 9, 27, 40, 5, 3, "distance, pistage et survie", ClassCategory::Distance},
            {"Arbalétrier", 175, 12, 32, 46, 3, 3, "tir lourd, plus lent mais puissant", ClassCategory::Distance},
            {"Chasseur", 190, 9, 24, 37, 4, 3, "traque, pièges et survie", ClassCategory::Distance},
            {"Lanceur de dagues", 150, 11, 26, 44, 5, 2, "distance courte, multi-projectiles", ClassCategory::Distance},
            {"Tireur", 165, 11, 31, 45, 3, 3, "visée, armes de trait et précision", ClassCategory::Distance},
            {"Pisteur", 175, 10, 28, 41, 5, 3, "lecture de traces, chasse et survie", ClassCategory::Distance},
            {"Frondeur tactique", 155, 9, 25, 39, 5, 2, "harcèlement, contrôle léger et économie de munitions", ClassCategory::Distance},
            {"Éclaireur d'élite", 170, 12, 30, 47, 4, 3, "initiative, vision et retrait propre", ClassCategory::Distance},
            {"Javelinier", 190, 11, 29, 43, 4, 3, "projectiles lourds, allonge et repli court", ClassCategory::Distance},
            {"Trappeur", 180, 8, 24, 38, 5, 3, "pièges, patience et contrôle de terrain", ClassCategory::Distance},
            {"Guetteur", 165, 9, 26, 41, 5, 2, "surveillance, alerte et tir opportuniste", ClassCategory::Distance},
            {"Messager armé", 170, 8, 25, 39, 6, 2, "mobilité, fuite propre et petites lames", ClassCategory::Distance},
            {"Arquebusier expérimental", 150, 14, 36, 55, 2, 4, "tir très lourd, préparation et raté coûteux", ClassCategory::Distance},
            {"Fauconnier", 165, 9, 25, 39, 5, 3, "vision aérienne, harcèlement et repérage", ClassCategory::Distance},
            {"Sentinelle de rempart", 185, 10, 28, 42, 4, 3, "tir défensif, garde de mur et patience", ClassCategory::Distance},
            {"Chasseur de reliques", 170, 11, 30, 46, 4, 3, "pièges propres, fouille et tir opportuniste", ClassCategory::Distance},
            {"Tireur nomade", 160, 12, 31, 48, 5, 3, "mobilité, précision et économie de ressources", ClassCategory::Distance},

            {"Mage", 140, 6, 29, 46, 4, 5, "magie classique, fragile mais dangereux", ClassCategory::Magic},
            {"Ensorceleur", 145, 8, 27, 42, 4, 4, "magie instinctive et explosive", ClassCategory::Magic},
            {"Sorcier", 160, 7, 25, 40, 4, 4, "pactes, malédictions et risques", ClassCategory::Magic},
            {"Magicien", 135, 6, 30, 48, 4, 5, "gros potentiel magique, fragile", ClassCategory::Magic},
            {"Pyromancien", 145, 8, 32, 50, 3, 5, "feu, burst et risques", ClassCategory::Magic},
            {"Cryomancien", 160, 6, 25, 38, 4, 4, "contrôle, ralentissement et survie", ClassCategory::Magic},
            {"Arcaniste", 150, 7, 31, 49, 4, 5, "magie pure et instable", ClassCategory::Magic},
            {"Occultiste", 155, 9, 29, 45, 3, 5, "ombres, pactes et dégâts étranges", ClassCategory::Magic},
            {"Mage fou", 140, 1, 36, 55, 3, 6, "résultats absurdes, parfois géniaux", ClassCategory::Magic},
            {"Mage flame", 150, 9, 34, 52, 3, 5, "flammes rapides et dégâts croissants", ClassCategory::Magic},
            {"Mage zodiacal", 155, 7, 33, 51, 4, 5, "sorts aléatoires liés aux signes", ClassCategory::Magic},
            {"Électromancien", 145, 8, 31, 50, 3, 5, "choc, équipement métallique et burst nerveux", ClassCategory::Magic},
            {"Gravemage", 155, 7, 28, 44, 4, 5, "runes lentes, défense magique et préparation", ClassCategory::Magic},
            {"Miragien", 140, 6, 26, 46, 4, 5, "illusions, erreurs de cible et duel mental", ClassCategory::Magic},
            {"Hydromancien", 155, 6, 25, 39, 5, 4, "eau, amortissement et soins mineurs", ClassCategory::Magic},
            {"Géomancien", 175, 6, 24, 37, 5, 4, "terre, résistance et terrain difficile", ClassCategory::Magic},
            {"Chronomancien", 135, 5, 28, 52, 4, 5, "tempo, retard et gros risque d'instabilité", ClassCategory::Magic},
            {"Démoniste", 150, 9, 30, 48, 3, 5, "malédictions, pactes et dégâts sombres", ClassCategory::Magic},
            {"Aéromancien", 145, 7, 27, 44, 4, 5, "vent, esquive et déplacement", ClassCategory::Magic},
            {"Lumomancien", 150, 6, 24, 38, 6, 4, "lumière, soin mineur et anti-morts", ClassCategory::Magic},
            {"Umbromancien", 145, 8, 30, 48, 3, 5, "ombre, affaiblissement et duel mental", ClassCategory::Magic},
            {"Cristallomancien", 160, 7, 26, 41, 5, 4, "barrières, éclats et terrain tranchant", ClassCategory::Magic},
            {"Sonomancien", 140, 6, 25, 45, 4, 5, "ondes, interruption et confusion légère", ClassCategory::Magic},
            {"Mage de sang", 155, 10, 31, 50, 3, 5, "sacrifice, vol de rythme et danger personnel", ClassCategory::Magic},
            {"Bibliomancien", 135, 5, 27, 47, 5, 5, "grimoires, préparation et savoir dangereux", ClassCategory::Magic},

            {"Invocateur", 160, 6, 22, 34, 5, 4, "alliés invoqués et stratégie", ClassCategory::Invocation},
            {"Nécromancien", 150, 7, 26, 42, 4, 5, "morts-vivants, sacrifices et contrôle", ClassCategory::Invocation},
            {"Dompteur", 185, 8, 23, 35, 5, 3, "compagnons, bêtes et adaptation", ClassCategory::Invocation},
            {"Pactisant", 175, 9, 28, 43, 4, 4, "contrats dangereux et pouvoirs empruntés", ClassCategory::Invocation},
            {"Marionnettiste", 155, 7, 24, 39, 4, 4, "contrôle d'alliés fragiles et fils dangereux", ClassCategory::Invocation},
            {"Totémiste", 190, 6, 21, 32, 5, 3, "totems, protection de zone et patience", ClassCategory::Invocation},
            {"Corbeaumancien", 150, 8, 27, 42, 4, 4, "familier, vision et petites malédictions", ClassCategory::Invocation},
            {"Dresseur spectral", 160, 7, 25, 41, 4, 4, "compagnons d'âme, peur et protection fragile", ClassCategory::Invocation},
            {"Gardien de familiers", 195, 7, 22, 34, 6, 3, "protège ses créatures et encaisse à leur place", ClassCategory::Invocation},
            {"Conjurateur de ruche", 150, 6, 24, 39, 4, 4, "petites invocations nombreuses et pression constante", ClassCategory::Invocation},
            {"Belluaire", 185, 8, 24, 36, 5, 3, "bêtes dressées, morsures et protection fragile", ClassCategory::Invocation},
            {"Porte-essaim", 150, 7, 25, 40, 4, 4, "nuées temporaires, gêne et pression d'usure", ClassCategory::Invocation},
            {"Reliquaire vivant", 170, 6, 23, 37, 5, 4, "artefacts invoqués, charges et danger latent", ClassCategory::Invocation},
            {"Gardien de totems", 205, 5, 20, 32, 6, 3, "totems résistants, zone lente et défense", ClassCategory::Invocation},
            {"Appelant draconique", 165, 8, 27, 43, 4, 5, "souffles mineurs, pacte d'écailles et risque", ClassCategory::Invocation},

            {"Barde", 170, 6, 18, 28, 5, 3, "support, charme et chaos social", ClassCategory::Support},
            {"Clerc", 220, 6, 17, 26, 6, 2, "soin, foi et protection", ClassCategory::Support},
            {"Prêtre", 205, 5, 18, 28, 7, 2, "soin, lumière et soutien", ClassCategory::Support},
            {"Paladin", 280, 8, 22, 34, 5, 3, "tank sacré, protection et lumière", ClassCategory::Support},
            {"Gardien", 320, 4, 14, 22, 4, 1, "protection pure, très défensif", ClassCategory::Support},
            {"Templier", 260, 7, 22, 33, 5, 3, "foi martiale et anti-magie", ClassCategory::Support},
            {"Tank sacré", 340, 4, 15, 24, 4, 1, "mur vivant, entrave et lumière", ClassCategory::Support},
            {"Oracle", 180, 5, 20, 33, 6, 3, "lecture du danger et soutien fragile", ClassCategory::Support},
            {"Protecteur", 300, 5, 17, 27, 5, 1, "garde d'allié, interception et stabilité", ClassCategory::Support},
            {"Médecin de terrain", 185, 5, 16, 25, 7, 2, "soins pratiques, pansements et survie", ClassCategory::Support},
            {"Infirmier de guilde", 175, 4, 15, 24, 8, 1, "soins sûrs, peu offensif, très stable", ClassCategory::Support},
            {"Porte-bannière", 210, 6, 19, 31, 6, 2, "moral, défense de groupe et présence", ClassCategory::Support},
            {"Juge novice", 190, 7, 21, 35, 5, 3, "sanction légère, protection et lecture des fautes", ClassCategory::Support},
            {"Chantre de bataille", 185, 6, 20, 32, 6, 3, "chants courts, moral et protection légère", ClassCategory::Support},
            {"Archiviste de terrain", 165, 5, 18, 29, 7, 3, "notes, identification et soutien prudent", ClassCategory::Support},
            {"Aumônier itinérant", 205, 5, 17, 27, 8, 2, "foi douce, soin et résistance mentale", ClassCategory::Support},
            {"Porte-lanterne", 180, 6, 19, 31, 6, 3, "lumière, anti-ombre et escorte", ClassCategory::Support},
            {"Sénéchal de guilde", 220, 5, 18, 30, 7, 2, "ordre, gestion de groupe et défense calme", ClassCategory::Support},

            {"Druide", 190, 7, 20, 30, 5, 3, "nature, survie et magie sauvage", ClassCategory::Hybrid},
            {"Chevalier runique", 240, 8, 26, 39, 4, 4, "arme lourde et magie gravée", ClassCategory::Hybrid},
            {"Ombrelame", 160, 12, 31, 50, 4, 3, "assassin magique, très agressif", ClassCategory::Hybrid},
            {"Mage-lame", 180, 9, 28, 43, 4, 4, "mélange arme réelle et magie", ClassCategory::Hybrid},
            {"Shaman", 180, 7, 24, 36, 5, 4, "esprits, rituels et soutien", ClassCategory::Hybrid},
            {"Chevalier draconique", 250, 10, 29, 43, 4, 3, "écailles, arme lourde et souffle mineur", ClassCategory::Hybrid},
            {"Rôdeur arcanique", 175, 10, 30, 45, 4, 4, "distance, magie légère et contrôle de terrain", ClassCategory::Hybrid},
            {"Moine solaire", 190, 9, 26, 40, 5, 3, "discipline, lumière et contre-pression", ClassCategory::Hybrid},
            {"Chaman de guerre", 205, 9, 26, 39, 5, 4, "rituels courts, arme simple et pression spirituelle", ClassCategory::Hybrid},
            {"Lame tellurique", 220, 10, 27, 41, 4, 3, "terre et lame, défense active et choc lourd", ClassCategory::Hybrid},
            {"Danseur lunaire", 165, 11, 29, 48, 5, 3, "esquive, rythme et magie lunaire légère", ClassCategory::Hybrid},
            {"Corsaire arcanique", 175, 12, 30, 47, 4, 4, "lame, tir court et ruse magique", ClassCategory::Hybrid},
            {"Pèlerin martial", 210, 8, 24, 36, 6, 3, "foi, voyage et endurance au contact", ClassCategory::Hybrid},
            {"Fauche-âme", 170, 13, 32, 51, 3, 5, "lame sombre, cible blessée et pacte instable", ClassCategory::Hybrid},
            {"Brise-rune", 215, 11, 29, 44, 4, 4, "arme réelle, anti-magie et choc de sceau", ClassCategory::Hybrid},
            {"Chevalier des brumes", 200, 10, 27, 42, 5, 4, "esquive voilée, défense et contre doux", ClassCategory::Hybrid},

            {"Artificier", 175, 8, 24, 36, 4, 4, "gadgets, précision et projectiles", ClassCategory::Craft},
            {"Forgeron", 185, 14, 28, 38, 3, 2, "force, forge et économie d'usure des kits", ClassCategory::Craft},
            {"Alchimiste", 165, 5, 18, 28, 6, 5, "potions, mélanges et effets instables", ClassCategory::Craft},
            {"Cuisinier de guilde", 175, 6, 19, 29, 6, 3, "rations, buffs légers et économie de consommables", ClassCategory::Craft},
            {"Cartographe", 160, 7, 22, 34, 5, 3, "lecture de zone, repérage et préparation", ClassCategory::Craft},
            {"Récupérateur", 180, 9, 23, 35, 4, 3, "loot propre, matériaux et bricolage rapide", ClassCategory::Craft},
            {"Runiste", 165, 7, 22, 36, 5, 4, "runes, catalyseurs et réparation magique", ClassCategory::Craft},
            {"Enchanteur", 155, 6, 21, 35, 5, 5, "améliorations d'équipement et instabilité utile", ClassCategory::Craft},
            {"Bricoleur de siège", 190, 10, 24, 34, 4, 3, "gros outils, pièges lourds et entretien", ClassCategory::Craft},
            {"Intendant", 180, 5, 17, 27, 7, 2, "stocks, économie de consommables et organisation", ClassCategory::Craft},
            {"Joaillier runique", 150, 5, 18, 30, 6, 5, "gemmes, stabilisation et économie de catalyseur", ClassCategory::Craft},
            {"Charpentier de guerre", 205, 9, 23, 34, 5, 3, "bois renforcé, barricades et réparation de terrain", ClassCategory::Craft},
            {"Scribe d'atelier", 155, 4, 16, 27, 7, 4, "plans, recettes et amélioration de connaissance", ClassCategory::Craft},
            {"Herboriste de combat", 160, 5, 17, 28, 8, 3, "plantes, remèdes et anti-poison", ClassCategory::Craft},
            {"Mécanicien de reliques", 170, 7, 22, 36, 5, 5, "objets anciens, charges et réparation risquée", ClassCategory::Craft}
        };

        return classes;
    }

    void appendClassTemplateLines(std::vector<std::string>& lines, std::size_t displayedIndex, const ClassTemplate& currentClass)
    {
        lines.push_back(std::to_string(displayedIndex) + " : " + currentClass.name);
        lines.push_back("    Famille : " + classCategoryToText(currentClass.category));
        lines.push_back(std::string("    Rôle : ") + currentClass.role);
        lines.push_back("    PV : " + std::to_string(currentClass.maxHp));
        lines.push_back(
            "    Dégâts : " + std::to_string(currentClass.minDamage)
            + " - " + std::to_string(currentClass.maxDamage)
            + " | Critique : " + std::to_string(currentClass.criticalDamage)
        );
        lines.push_back("    Potions de soin : " + std::to_string(currentClass.healingPotionCount));
        lines.push_back("    Potions de dégâts : " + std::to_string(currentClass.damagePotionCount));
        lines.push_back("");
    }

    std::vector<const ClassTemplate*> getClassesByCategory(ClassCategory category)
    {
        std::vector<const ClassTemplate*> filteredClasses;

        for (const ClassTemplate& currentClass : getClassTemplates())
        {
            if (currentClass.category == category)
            {
                filteredClasses.push_back(&currentClass);
            }
        }

        return filteredClasses;
    }

    // EN: createPlayerClassFromTemplate declares or implements a focused behavior used by this module.
    // FR: createPlayerClassFromTemplate déclare ou implémente un comportement précis utilisé par ce module.
    PlayerClass createPlayerClassFromTemplate(const ClassTemplate& selectedClass)
    {
        return PlayerClass(
            selectedClass.name,
            selectedClass.maxHp,
            selectedClass.minDamage,
            selectedClass.maxDamage,
            selectedClass.criticalDamage,
            selectedClass.healingPotionCount,
            selectedClass.damagePotionCount
        );
    }

    ClassOptionInfo toClassOptionInfo(const ClassTemplate& currentClass)
    {
        ClassOptionInfo info;
        info.name = currentClass.name;
        info.role = currentClass.role;
        info.categoryName = classCategoryToText(currentClass.category);
        info.maxHp = currentClass.maxHp;
        info.minDamage = currentClass.minDamage;
        info.maxDamage = currentClass.maxDamage;
        info.criticalDamage = currentClass.criticalDamage;
        info.healingPotionCount = currentClass.healingPotionCount;
        info.damagePotionCount = currentClass.damagePotionCount;
        return info;
    }
}

// EN: displayBasicClasses declares or implements a focused behavior used by this module.
// FR: displayBasicClasses déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> ClassCatalog::getBasicClassDisplayLines()
{
    std::vector<std::string> lines;
    const std::vector<ClassTemplate>& classes = getClassTemplates();

    for (std::size_t i = 0; i < classes.size(); ++i)
    {
        appendClassTemplateLines(lines, i + 1, classes[i]);
    }

    return lines;
}

void ClassCatalog::displayBasicClasses()
{
    MessageScreen::show("CLASSES", "catalog.classes.basic", getBasicClassDisplayLines(), false);
}

// EN: displayClassCategories declares or implements a focused behavior used by this module.
// FR: displayClassCategories déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> ClassCatalog::getClassCategoryDisplayLines()
{
    std::vector<std::string> lines;
    std::vector<ClassCategory> categories = getClassCategories();

    for (std::size_t i = 0; i < categories.size(); ++i)
    {
        ClassCategory category = categories[i];
        lines.push_back(
            std::to_string(i + 1) + " : " + classCategoryToText(category)
            + " (" + std::to_string(getPlayableClassCountByCategory(category)) + " classes)"
        );
    }

    return lines;
}

void ClassCatalog::displayClassCategories()
{
    MessageScreen::show("CATÉGORIES DE CLASSES", "catalog.classes.categories", getClassCategoryDisplayLines(), false);
}

// EN: displayClassesByCategory declares or implements a focused behavior used by this module.
// FR: displayClassesByCategory déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> ClassCatalog::getClassDisplayLinesByCategory(ClassCategory category)
{
    std::vector<std::string> lines;
    std::vector<const ClassTemplate*> classes = getClassesByCategory(category);

    for (std::size_t i = 0; i < classes.size(); ++i)
    {
        appendClassTemplateLines(lines, i + 1, *classes[i]);
    }

    return lines;
}

void ClassCatalog::displayClassesByCategory(ClassCategory category)
{
    MessageScreen::show("CLASSES PAR CATÉGORIE", "catalog.classes.by_category", getClassDisplayLinesByCategory(category), false);
}

// EN: displayClassesByCategoryChoice declares or implements a focused behavior used by this module.
// FR: displayClassesByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
void ClassCatalog::displayClassesByCategoryChoice(int categoryChoice)
{
    displayClassesByCategory(getClassCategoryByChoice(categoryChoice));
}

// EN: getPlayableClassCount declares or implements a focused behavior used by this module.
// FR: getPlayableClassCount déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getPlayableClassCount()
{
    return static_cast<int>(getClassTemplates().size());
}

// EN: getClassCategoryCount declares or implements a focused behavior used by this module.
// FR: getClassCategoryCount déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getClassCategoryCount()
{
    return static_cast<int>(getClassCategories().size());
}

// EN: getPlayableClassCountByCategory declares or implements a focused behavior used by this module.
// FR: getPlayableClassCountByCategory déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getPlayableClassCountByCategory(ClassCategory category)
{
    return static_cast<int>(getClassesByCategory(category).size());
}

// EN: getPlayableClassCountByCategoryChoice declares or implements a focused behavior used by this module.
// FR: getPlayableClassCountByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getPlayableClassCountByCategoryChoice(int categoryChoice)
{
    return getPlayableClassCountByCategory(getClassCategoryByChoice(categoryChoice));
}

std::vector<std::string> ClassCatalog::getPlayableClassNames()
{
    std::vector<std::string> names;

    for (const ClassTemplate& currentClass : getClassTemplates())
    {
        names.push_back(currentClass.name);
    }

    return names;
}

std::vector<ClassCategory> ClassCatalog::getClassCategories()
{
    return {
        ClassCategory::Melee,
        ClassCategory::Distance,
        ClassCategory::Magic,
        ClassCategory::Invocation,
        ClassCategory::Support,
        ClassCategory::Hybrid,
        ClassCategory::Craft
    };
}

// EN: getClassCategoryByChoice declares or implements a focused behavior used by this module.
// FR: getClassCategoryByChoice déclare ou implémente un comportement précis utilisé par ce module.
ClassCategory ClassCatalog::getClassCategoryByChoice(int categoryChoice)
{
    std::vector<ClassCategory> categories = getClassCategories();

    if (categoryChoice < 1 || categoryChoice > static_cast<int>(categories.size()))
    {
        categoryChoice = 1;
    }

    return categories[categoryChoice - 1];
}

std::string ClassCatalog::getClassCategoryNameByChoice(int categoryChoice)
{
    return classCategoryToText(getClassCategoryByChoice(categoryChoice));
}

std::vector<ClassOptionInfo> ClassCatalog::getClassOptionsByCategoryChoice(int categoryChoice)
{
    std::vector<ClassOptionInfo> options;
    ClassCategory category = getClassCategoryByChoice(categoryChoice);

    for (const ClassTemplate* currentClass : getClassesByCategory(category))
    {
        options.push_back(toClassOptionInfo(*currentClass));
    }

    return options;
}

// EN: createBaseClass declares or implements a focused behavior used by this module.
// FR: createBaseClass déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createBaseClass(int choice)
{
    const std::vector<ClassTemplate>& classes = getClassTemplates();

    if (choice < 1 || choice > static_cast<int>(classes.size()))
    {
        choice = 1;
    }

    return createPlayerClassFromTemplate(classes[choice - 1]);
}

// EN: createClassByCategoryChoice declares or implements a focused behavior used by this module.
// FR: createClassByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createClassByCategoryChoice(int categoryChoice, int classChoice)
{
    ClassCategory category = getClassCategoryByChoice(categoryChoice);
    std::vector<const ClassTemplate*> classes = getClassesByCategory(category);

    if (classes.empty())
    {
        return createBaseClass(1);
    }

    if (classChoice < 1 || classChoice > static_cast<int>(classes.size()))
    {
        classChoice = 1;
    }

    return createPlayerClassFromTemplate(*classes[classChoice - 1]);
}

// EN: createClassByName declares or implements a focused behavior used by this module.
// FR: createClassByName déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createClassByName(const std::string& className)
{
    return createClassFromNormalizedName(normalizeClassName(className));
}

// EN: createEvolvedClassFromClass declares or implements a focused behavior used by this module.
// FR: createEvolvedClassFromClass déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createEvolvedClassFromClass(const std::string& currentClass)
{
    std::string normalizedName = normalizeClassName(currentClass);

    if (normalizedName == normalizeClassName("Chevalier"))
    {
        return PlayerClass("Chevalier éveillé", 300, 9, 28, 42, 5, 3);
    }

    if (normalizedName == normalizeClassName("Assassin")
        || normalizedName == normalizeClassName("Classe Légère")
        || normalizedName == normalizeClassName("Roublard"))
    {
        return PlayerClass("Assassin éveillé", 210, 13, 34, 55, 6, 2);
    }

    if (normalizedName == normalizeClassName("Colosse")
        || normalizedName == normalizeClassName("Classe Lourde"))
    {
        return PlayerClass("Colosse éveillé", 500, 7, 18, 30, 2, 3);
    }

    if (normalizedName == normalizeClassName("Mage flame")
        || normalizedName == normalizeClassName("Pyromancien"))
    {
        return createClassByName("Arcaniste");
    }

    if (normalizedName == normalizeClassName("Tank sacré")
        || normalizedName == normalizeClassName("Sanctus"))
    {
        return createClassByName("Paladin");
    }

    if (normalizedName == normalizeClassName("Lancier"))
    {
        return PlayerClass("Maître de la hampe", 290, 12, 32, 48, 4, 3);
    }

    if (normalizedName == normalizeClassName("Archer")
        || normalizedName == normalizeClassName("Rôdeur")
        || normalizedName == normalizeClassName("Chasseur"))
    {
        return PlayerClass("Tireur éveillé", 230, 12, 34, 50, 4, 3);
    }

    if (normalizedName == normalizeClassName("Mage")
        || normalizedName == normalizeClassName("Magicien")
        || normalizedName == normalizeClassName("Ensorceleur")
        || normalizedName == normalizeClassName("Sorcier"))
    {
        return PlayerClass("Mage éveillé", 210, 9, 36, 58, 4, 5);
    }

    PlayerClass baseClass = createClassByName(currentClass);
    return PlayerClass(
        baseClass.getName() + " éveillé",
        baseClass.getMaxHp() + 70,
        baseClass.getMinDamage() + 2,
        baseClass.getMaxDamage() + 6,
        baseClass.getCriticalDamage() + 10,
        baseClass.getHealingPotionCount(),
        baseClass.getDamagePotionCount()
    );
}

// EN: createClassFromNormalizedName declares or implements a focused behavior used by this module.
// FR: createClassFromNormalizedName déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createClassFromNormalizedName(const std::string& normalizedName)
{
    const std::vector<ClassTemplate>& classes = getClassTemplates();

    for (const ClassTemplate& currentClass : classes)
    {
        if (normalizeClassName(currentClass.name) == normalizedName)
        {
            return createPlayerClassFromTemplate(currentClass);
        }
    }

    return createBaseClass(1);
}

std::string ClassCatalog::normalizeClassName(const std::string& className)
{
    std::string normalized = className;

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    return normalized;
}
