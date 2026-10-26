// EN: LootGenerator.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: LootGenerator.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// English: Implements simple monster loot generation for the first playable reward pass.
// Français : Implémente une génération simple de loots de monstres pour la première version jouable des récompenses.

#include "combat/loot/LootGenerator.hpp"
#include "combat/ally/RecruitedAllyAftermathSystem.hpp"

#include "economy/Money.hpp"

#include "entity/Race.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/WeaponType.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/armor/ArmorType.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include "progression/DifficultyRules.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"

#include <string>
#include <algorithm>
#include <cctype>
#include <vector>


namespace
{
    void showLootScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines);
        }
    }

    std::string formatLootMaterialLine(const Material& material)
    {
        std::string line = material.getName();
        if (material.hasSpecialQuality())
        {
            line += " [" + material.getQualityLabel() + "]";
        }
        line += " x" + std::to_string(material.getQuantity()) + ".";
        return line;
    }

    void addLootLine(
        std::vector<std::string>& lines,
        const std::string& reason,
        const Material& material
    )
    {
        lines.push_back(reason + " : " + formatLootMaterialLine(material));
    }

    std::string toLower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    // EN: containsText declares or implements a focused behavior used by this module.
    // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
    bool containsText(const std::string& value, const std::string& searched)
    {
        return toLower(value).find(toLower(searched)) != std::string::npos;
    }

    bool monsterTextContains(const Monster& monster, const std::string& searched)
    {
        return containsText(monster.getName(), searched) || containsText(monster.getType(), searched);
    }

    bool isWolfLikeBeast(const Monster& monster)
    {
        return monsterTextContains(monster, "loup")
            || monsterTextContains(monster, "wolf")
            || monsterTextContains(monster, "meute")
            || monsterTextContains(monster, "lupin");
    }

    bool isBoarLikeBeast(const Monster& monster)
    {
        return monsterTextContains(monster, "sanglier")
            || monsterTextContains(monster, "verrat")
            || monsterTextContains(monster, "defense")
            || monsterTextContains(monster, "défense");
    }

    bool isRodentLikeBeast(const Monster& monster)
    {
        return monsterTextContains(monster, "rat")
            || monsterTextContains(monster, "rongeur")
            || monsterTextContains(monster, "souris");
    }

    bool isFangedPredatorBeast(const Monster& monster)
    {
        return monsterTextContains(monster, "chien")
            || monsterTextContains(monster, "renard")
            || monsterTextContains(monster, "chacal")
            || monsterTextContains(monster, "ours")
            || monsterTextContains(monster, "croc")
            || monsterTextContains(monster, "crocs")
            || monsterTextContains(monster, "prédateur")
            || monsterTextContains(monster, "predateur")
            || monsterTextContains(monster, "carnassier")
            || monsterTextContains(monster, "lunaire");
    }

    // EN: isIntelligentRace declares or implements a focused behavior used by this module.
    // FR: isIntelligentRace déclare ou implémente un comportement précis utilisé par ce module.
    bool isIntelligentRace(Race race)
    {
        switch (race)
        {
            case Race::Humain:
            case Race::SemiHumain:
            case Race::Elfe:
            case Race::ElfeNoir:
            case Race::Nain:
            case Race::Gnome:
            case Race::Halfelin:
            case Race::Tieffelin:
            case Race::Aasimar:
            case Race::Kitsune:
            case Race::Fee:
            case Race::SemiDragon:
            case Race::Gobelin:
            case Race::Hobgobelin:
            case Race::Orc:
            case Race::Demon:
            case Race::Ange:
                return true;

            default:
                return false;
        }
    }

    // EN: canLogicallyCarryMoney declares or implements a focused behavior used by this module.
    // FR: canLogicallyCarryMoney déclare ou implémente un comportement précis utilisé par ce module.
    bool canLogicallyCarryMoney(const Monster& monster)
    {
        if (isIntelligentRace(monster.getRace()))
        {
            return true;
        }

        return containsText(monster.getType(), "bandit")
            || containsText(monster.getType(), "voleur")
            || containsText(monster.getType(), "aventurier")
            || containsText(monster.getName(), "marchand");
    }

    // EN: canCarryLargePurse declares or implements a focused behavior used by this module.
    // FR: canCarryLargePurse déclare ou implémente un comportement précis utilisé par ce module.
    bool canCarryLargePurse(const Monster& monster)
    {
        return containsText(monster.getType(), "voleur")
            || containsText(monster.getType(), "bandit")
            || containsText(monster.getType(), "aventurier")
            || containsText(monster.getType(), "expérimenté")
            || containsText(monster.getType(), "experimente")
            || containsText(monster.getType(), "assassin")
            || monster.isElite();
    }

    // EN: giveCoinsIfLogical declares or implements a focused behavior used by this module.
    // FR: giveCoinsIfLogical déclare ou implémente un comportement précis utilisé par ce module.
    void giveCoinsIfLogical(Player& player, const Monster& monster, Random& random, std::vector<std::string>& lines)
    {
        if (!canLogicallyCarryMoney(monster))
        {
            return;
        }

        int smallCoinChance = monster.isElite() ? 100 : 88;

        if (random.between(1, 100) <= smallCoinChance)
        {
            int coins = random.between(3, 8) + monster.getLevel() * 2;
            player.getInventory().earnEconomyUnits(coins);
            player.refreshCurrencyTitles();
            lines.push_back(monster.getName() + " avait " + Money::formatEconomyUnits(coins) + " sur lui.");
        }

        int largePurseChance = canCarryLargePurse(monster) ? 8 : 1;

        if (monster.isElite())
        {
            largePurseChance += 6;
        }

        if (random.between(1, 100) <= largePurseChance)
        {
            int coins = random.between(55, 85) + monster.getLevel() * random.between(8, 12);
            player.getInventory().earnEconomyUnits(coins);
            player.refreshCurrencyTitles();
            lines.push_back("Belle trouvaille : une bourse plus lourde que prévu contient " + Money::formatEconomyUnits(coins) + ".");
        }
    }

    // EN: usesMonsterPurityScale declares or implements a focused behavior used by this module.
    // FR: usesMonsterPurityScale déclare ou implémente un comportement précis utilisé par ce module.
    bool usesMonsterPurityScale(const Material& material)
    {
        return material.getCategory() == "Matériau de monstre"
            || material.getCategory() == "Fragment de boss";
    }

    // EN: improveQualityByOneRank declares or implements a focused behavior used by this module.
    // FR: improveQualityByOneRank déclare ou implémente un comportement précis utilisé par ce module.
    void improveQualityByOneRank(Material& material)
    {
        if (material.getQuality() == "exceptional")
        {
            return;
        }

        if (usesMonsterPurityScale(material))
        {
            if (material.getQuality() == "impure")
            {
                material.setQuality("normal");
            }
            else if (material.getQuality() == "normal")
            {
                material.setQuality("pure");
            }
            else if (material.getQuality() == "pure")
            {
                material.setQuality("exceptional");
            }
            return;
        }

        if (material.getQuality() == "low")
        {
            material.setQuality("normal");
        }
        else if (material.getQuality() == "normal")
        {
            material.setQuality("high");
        }
        else if (material.getQuality() == "high")
        {
            material.setQuality("exceptional");
        }
    }

    // EN: playerUsesWeaponType declares or implements a focused behavior used by this module.
    // FR: playerUsesWeaponType déclare ou implémente un comportement précis utilisé par ce module.
    bool playerUsesWeaponType(const Player& player, WeaponType type)
    {
        return player.hasEquippedWeapon() && player.getEquippedWeapon().getType() == type;
    }



    // EN: isMeleeWeaponClass declares or implements a focused behavior used by this module.
    // FR: isMeleeWeaponClass déclare ou implémente un comportement précis utilisé par ce module.
    bool isMeleeWeaponClass(const std::string& cls)
    {
        return containsText(cls, "chevalier")
            || containsText(cls, "guerrier")
            || containsText(cls, "duelliste")
            || containsText(cls, "assassin")
            || containsText(cls, "barbare")
            || containsText(cls, "berserker")
            || containsText(cls, "ombrelame")
            || containsText(cls, "mage-lame");
    }

    // EN: isHeavyWeaponClass declares or implements a focused behavior used by this module.
    // FR: isHeavyWeaponClass déclare ou implémente un comportement précis utilisé par ce module.
    bool isHeavyWeaponClass(const std::string& cls)
    {
        return containsText(cls, "colosse")
            || containsText(cls, "briseur")
            || containsText(cls, "barbare")
            || containsText(cls, "berserker")
            || containsText(cls, "forgeron");
    }

    // EN: isBowWeaponClass declares or implements a focused behavior used by this module.
    // FR: isBowWeaponClass déclare ou implémente un comportement précis utilisé par ce module.
    bool isBowWeaponClass(const std::string& cls)
    {
        return containsText(cls, "archer")
            || containsText(cls, "rôdeur")
            || containsText(cls, "rodeur")
            || containsText(cls, "chasseur")
            || containsText(cls, "tireur")
            || containsText(cls, "arbalétrier")
            || containsText(cls, "arbaletrier");
    }

    // EN: isStaffWeaponClass declares or implements a focused behavior used by this module.
    // FR: isStaffWeaponClass déclare ou implémente un comportement précis utilisé par ce module.
    bool isStaffWeaponClass(const std::string& cls)
    {
        return containsText(cls, "mage")
            || containsText(cls, "sorcier")
            || containsText(cls, "magicien")
            || containsText(cls, "clerc")
            || containsText(cls, "prêtre")
            || containsText(cls, "pretre")
            || containsText(cls, "druide")
            || containsText(cls, "shaman")
            || containsText(cls, "alchimiste")
            || containsText(cls, "invocateur")
            || containsText(cls, "nécromancien")
            || containsText(cls, "necromancien");
    }

    // EN: classMastersEquippedWeapon declares or implements a focused behavior used by this module.
    // FR: classMastersEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool classMastersEquippedWeapon(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return containsText(player.getType(), "moine");
        }

        WeaponType weaponType = player.getEquippedWeapon().getType();
        std::string cls = player.getType();

        switch (weaponType)
        {
            case WeaponType::Sword:
            case WeaponType::Dagger:
                return isMeleeWeaponClass(cls) || containsText(cls, "lanceur de dagues");

            case WeaponType::Axe:
            case WeaponType::Hammer:
                return isHeavyWeaponClass(cls);

            case WeaponType::Spear:
                return containsText(cls, "lancier")
                    || containsText(cls, "chevalier")
                    || containsText(cls, "paladin")
                    || containsText(cls, "templier");

            case WeaponType::Bow:
                return isBowWeaponClass(cls);

            case WeaponType::Staff:
                return isStaffWeaponClass(cls);

            case WeaponType::BareHands:
                return containsText(cls, "moine");

            default:
                return false;
        }
    }

    // EN: weaponMasteryDegradationProtection declares or implements a focused behavior used by this module.
    // FR: weaponMasteryDegradationProtection déclare ou implémente un comportement précis utilisé par ce module.
    int weaponMasteryDegradationProtection(const Player& player)
    {
        if (!classMastersEquippedWeapon(player))
        {
            return 0;
        }

        int protection = 10;

        if (player.getLevel() >= 5)
        {
            protection += 5;
        }

        if (player.getLevel() >= 10)
        {
            protection += 5;
        }

        if (containsText(player.getType(), "guerrier")
            || containsText(player.getType(), "duelliste")
            // EN: containsText declares or implements a focused behavior used by this module.
            // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
            || containsText(player.getType(), "forgeron")
            || containsText(player.getType(), "archer")
            // EN: containsText declares or implements a focused behavior used by this module.
            // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
            || containsText(player.getType(), "tireur"))
        {
            protection += 4;
        }

        return protection;
    }

    // EN: weaponMasteryCleanRecoveryBonus declares or implements a focused behavior used by this module.
    // FR: weaponMasteryCleanRecoveryBonus déclare ou implémente un comportement précis utilisé par ce module.
    int weaponMasteryCleanRecoveryBonus(const Player& player)
    {
        if (!classMastersEquippedWeapon(player))
        {
            return 0;
        }

        int bonus = 4;

        if (player.getLevel() >= 5)
        {
            bonus += 3;
        }

        if (containsText(player.getType(), "assassin")
            || containsText(player.getType(), "duelliste")
            // EN: containsText declares or implements a focused behavior used by this module.
            // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
            || containsText(player.getType(), "chasseur")
            || containsText(player.getType(), "rôdeur")
            // EN: containsText declares or implements a focused behavior used by this module.
            // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
            || containsText(player.getType(), "rodeur"))
        {
            bonus += 4;
        }

        return bonus;
    }

    // EN: hasRecoveryHelp declares or implements a focused behavior used by this module.
    // FR: hasRecoveryHelp déclare ou implémente un comportement précis utilisé par ce module.
    bool hasRecoveryHelp(const Player& player, const std::string& id)
    {
        return player.getInventory().countMaterialById(id) > 0;
    }

    // EN: passiveCleanHarvestBonus declares or implements a focused behavior used by this module.
    // FR: passiveCleanHarvestBonus déclare ou implémente un comportement précis utilisé par ce module.
    int passiveCleanHarvestBonus(const Player& player, const Material& material)
    {
        int bonus = 0;

        if (hasRecoveryHelp(player, "precision_harvest_tools"))
        {
            bonus += 8;
        }

        if (hasRecoveryHelp(player, "clean_harvest_manual")
            && (material.getCategory() == "Plante" || material.getCategory() == "Matériau" || material.getCategory() == "Matériau rare"))
        {
            bonus += 7;
        }

        if (hasRecoveryHelp(player, "monster_dissection_guide")
            && (material.getCategory() == "Matériau de monstre" || material.getCategory() == "Fragment de boss"))
        {
            bonus += 10;
        }

        bonus += weaponMasteryCleanRecoveryBonus(player);

        return bonus;
    }

    // EN: passiveDegradationProtection declares or implements a focused behavior used by this module.
    // FR: passiveDegradationProtection déclare ou implémente un comportement précis utilisé par ce module.
    int passiveDegradationProtection(const Player& player, const Material& material)
    {
        int protection = 0;

        if (hasRecoveryHelp(player, "precision_harvest_tools"))
        {
            protection += 5;
        }

        if (hasRecoveryHelp(player, "preservation_vials")
            && (material.getCategory() == "Plante"
                || material.getCategory() == "Matériau magique"
                || material.getCategory() == "Matériau rare"
                || material.getCategory() == "Fragment de boss"
                || containsText(material.getName(), "Sang")
                // EN: containsText declares or implements a focused behavior used by this module.
                // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
                || containsText(material.getName(), "Braise")
                || containsText(material.getName(), "Résidu")
                // EN: containsText declares or implements a focused behavior used by this module.
                // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
                || containsText(material.getName(), "Noyau")))
        {
            protection += 12;
        }

        if (hasRecoveryHelp(player, "monster_dissection_guide") && material.getCategory() == "Matériau de monstre")
        {
            protection += 8;
        }

        protection += weaponMasteryDegradationProtection(player);

        return protection;
    }

    // EN: degradeQualityByOneRank declares or implements a focused behavior used by this module.
    // FR: degradeQualityByOneRank déclare ou implémente un comportement précis utilisé par ce module.
    void degradeQualityByOneRank(Material& material)
    {
        if (usesMonsterPurityScale(material))
        {
            if (material.getQuality() == "exceptional")
            {
                material.setQuality("pure");
            }
            else if (material.getQuality() == "pure")
            {
                material.setQuality("normal");
            }
            else if (material.getQuality() == "normal")
            {
                material.setQuality("impure");
            }
            return;
        }

        if (material.getQuality() == "exceptional")
        {
            material.setQuality("high");
        }
        else if (material.getQuality() == "high")
        {
            material.setQuality("normal");
        }
        else if (material.getQuality() == "normal")
        {
            material.setQuality("low");
        }
    }

    // EN: applyRecoveryStyleQualityEffect declares or implements a focused behavior used by this module.
    // FR: applyRecoveryStyleQualityEffect déclare ou implémente un comportement précis utilisé par ce module.
    void applyRecoveryStyleQualityEffect(Material& material, const Monster& monster, const Player& player, Random& random, std::vector<std::string>& lines)
    {
        if (material.getCategory() == "Outil" || material.getCategory() == "Livre" || material.getCategory() == "Renseignement")
        {
            return;
        }

        int degradationChance = 0;
        int cleanHarvestChance = 0;
        std::string reason;

        if (playerUsesWeaponType(player, WeaponType::Bow))
        {
            degradationChance += 18;
            reason = "impact de projectile";
        }

        if (playerUsesWeaponType(player, WeaponType::Hammer) || playerUsesWeaponType(player, WeaponType::Axe))
        {
            degradationChance += 12;
            reason = "choc trop brutal";
        }

        if (playerUsesWeaponType(player, WeaponType::Dagger) || playerUsesWeaponType(player, WeaponType::Sword))
        {
            cleanHarvestChance += 8;
        }

        if (playerUsesWeaponType(player, WeaponType::Spear))
        {
            cleanHarvestChance += 5;
            degradationChance += 5;
        }

        if (containsText(player.getType(), "mage")
            // EN: containsText declares or implements a focused behavior used by this module.
            // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
            || containsText(player.getType(), "sorcier")
            || containsText(player.getType(), "alchimiste")
            // EN: containsText declares or implements a focused behavior used by this module.
            // FR: containsText déclare ou implémente un comportement précis utilisé par ce module.
            || containsText(player.getType(), "explos"))
        {
            degradationChance += 10;
            reason = "résidus magiques instables";
        }

        if (containsText(monster.getType(), "fragile") || containsText(material.getName(), "Oreille"))
        {
            degradationChance += 6;
        }

        degradationChance = std::max(0, degradationChance - passiveDegradationProtection(player, material));
        cleanHarvestChance += passiveCleanHarvestBonus(player, material);

        if (degradationChance > 0 && random.between(1, 100) <= degradationChance)
        {
            std::string before = material.getQuality();
            degradeQualityByOneRank(material);

            if (before != material.getQuality())
            {
                if (reason.empty())
                {
                    reason = "récupération maladroite";
                }

                lines.push_back("Récupération abîmée (" + reason + ") : la qualité de " + material.getName() + " baisse d'un cran.");
            }

            return;
        }

        if (cleanHarvestChance > 0 && random.between(1, 100) <= cleanHarvestChance)
        {
            std::string before = material.getQuality();
            improveQualityByOneRank(material);

            if (before != material.getQuality())
            {
                std::string qualityLine = "Récupération précise";
                if (classMastersEquippedWeapon(player))
                {
                    qualityLine += " grâce à ta maîtrise de l'arme";
                }
                qualityLine += " : la qualité de " + material.getName() + " s'améliore d'un cran.";
                lines.push_back(qualityLine);
            }
        }
    }

    // EN: applyLootQuality declares or implements a focused behavior used by this module.
    // FR: applyLootQuality déclare ou implémente un comportement précis utilisé par ce module.
    Material applyLootQuality(Material material, const Monster& monster, const Player& player, Random& random, bool foundGoodLoot, std::vector<std::string>& lines)
    {
        if (material.getCategory() == "Outil" || material.getCategory() == "Livre" || material.getCategory() == "Renseignement")
        {
            return material;
        }

        int roll = random.between(1, 100);
        int highChance = foundGoodLoot ? 12 : 3;
        int lowChance = foundGoodLoot ? 12 : 35;
        int cleanRecoveryChance = foundGoodLoot ? 18 : 5;

        cleanRecoveryChance += passiveCleanHarvestBonus(player, material);
        lowChance = std::max(0, lowChance - passiveDegradationProtection(player, material));

        if (monster.isElite())
        {
            highChance += 8;
            lowChance -= 5;
            cleanRecoveryChance += 7;
        }

        if (monster.isEvolved())
        {
            highChance += 7;
            lowChance -= 6;
            cleanRecoveryChance += 8;
        }

        if (containsText(monster.getType(), "brûlé") || containsText(monster.getType(), "brule") || containsText(monster.getType(), "explosif"))
        {
            lowChance += 15;
            cleanRecoveryChance -= 8;
        }

        if (roll <= highChance)
        {
            if (usesMonsterPurityScale(material))
            {
                material.setQuality("pure");
            }
            else
            {
                material.setQuality("high");
            }
        }
        else if (roll >= 100 - lowChance)
        {
            if (usesMonsterPurityScale(material))
            {
                material.setQuality("impure");
            }
            else
            {
                material.setQuality("low");
            }
        }

        // Une récupération très propre peut améliorer la qualité d'un cran.
        // Pour l'instant, faute de journal détaillé des coups, on l'estime avec la qualité du loot trouvé,
        // le statut élite, et les profils qui risquent de brûler/exploser les composants.
        if (cleanRecoveryChance > 0 && random.between(1, 100) <= cleanRecoveryChance)
        {
            std::string before = material.getQuality();
            improveQualityByOneRank(material);

            if (before != material.getQuality())
            {
                std::string qualityLine = "Récupération propre";
                if (classMastersEquippedWeapon(player))
                {
                    qualityLine += " mieux contrôlée par ta maîtrise de l'arme";
                }
                qualityLine += " : la qualité de " + material.getName() + " s'améliore d'un cran.";
                lines.push_back(qualityLine);
            }
        }

        applyRecoveryStyleQualityEffect(material, monster, player, random, lines);

        return material;
    }


    bool askManualLootPickup(const std::string& name, const std::string& detail, const std::string& screenId)
    {
        MenuScreen screen("BUTIN TROUVÉ", screenId);
        screen.addLine(name);
        if (!detail.empty()) screen.addLine(detail);
        screen.addOption(1, "Récupérer", "Ajouter ce butin à l'inventaire.", true, screenId + ".take");
        screen.addOption(2, "Laisser sur place", "Ne pas récupérer cet objet.", true, screenId + ".leave");
        return TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis quoi faire de ce butin.") == 1;
    }

    bool shouldPickupLoot(Player& player, const std::string& categoryId, const std::string& stableId, const std::string& name, const std::string& detail)
    {
        const std::string policy = player.getLootPickupPolicy();
        if (policy == "manual")
        {
            return askManualLootPickup(name, detail, "combat.loot.manual_pickup");
        }
        if (policy == "filter")
        {
            if (player.isLootCategoryIgnored(categoryId)) return false;
            if (categoryId == "material" && !stableId.empty() && player.isLootMaterialIgnored(stableId)) return false;
        }
        return true;
    }

    bool isNotableMaterialLoot(const Material& material)
    {
        return material.getQuality() == "exceptional"
            || material.getQuality() == "pure"
            || material.getQuality() == "high"
            || material.getCategory() == "Fragment de boss"
            || material.getCategory() == "Matériau rare"
            || material.getCategory() == "Matériau magique";
    }

    bool addMaterialLootToPlayer(Player& player, const Material& material, std::vector<std::string>* lines = nullptr, const std::string& reason = "")
    {
        const std::string detail = formatLootMaterialLine(material);
        if (!shouldPickupLoot(player, "material", material.getId(), material.getName(), detail))
        {
            if (lines != nullptr)
            {
                lines->push_back((reason.empty() ? material.getName() : reason) + " : laissé sur place selon les préférences de butin.");
            }
            return false;
        }
        player.getInventory().addMaterial(material);
        player.recordMaterialCollected(material.getId(), material.getName(), material.getQuantity());
        if (isNotableMaterialLoot(material))
        {
            const std::string source = reason.empty() ? "Butin remarquable" : reason;
            player.recordCanonicalEvent(
                "butin_notable",
                "material:" + material.getId() + ":" + material.getQuality(),
                source + " — " + formatLootMaterialLine(material),
                material.getQuantity()
            );
        }
        return true;
    }

    // EN: giveExtraLoot declares or implements a focused behavior used by this module.
    // FR: giveExtraLoot déclare ou implémente un comportement précis utilisé par ce module.
    void giveExtraLoot(Player& player, Material material, const std::string& reason, std::vector<std::string>& lines)
    {
        if (addMaterialLootToPlayer(player, material, &lines, reason))
        {
            addLootLine(lines, reason, material);
        }
    }

    bool defeatedEnemyCanCarryEquipment(const Monster& monster)
    {
        if (monster.isInvocation())
        {
            return false;
        }

        if (isIntelligentRace(monster.getRace()))
        {
            return true;
        }

        return containsText(monster.getType(), "bandit")
            || containsText(monster.getType(), "voleur")
            || containsText(monster.getType(), "mercenaire")
            || containsText(monster.getType(), "garde")
            || containsText(monster.getType(), "aventurier")
            || containsText(monster.getName(), "soldat");
    }

    bool playerRaceMatchesMonsterEquipmentSize(const Player& player, const Monster& monster)
    {
        const CharacterRace playerRace = player.getRace();
        const Race monsterRace = monster.getRace();

        if (monsterRace == Race::Humain && playerRace == CharacterRace::Human) return true;
        if (monsterRace == Race::SemiHumain
            && (playerRace == CharacterRace::SemiHuman
                || playerRace == CharacterRace::SemiWolf
                || playerRace == CharacterRace::SemiFox
                || playerRace == CharacterRace::SemiDog
                || playerRace == CharacterRace::SemiCat
                || playerRace == CharacterRace::SemiLizard
                || playerRace == CharacterRace::SemiBird)) return true;
        if (monsterRace == Race::Elfe && playerRace == CharacterRace::Elf) return true;
        if (monsterRace == Race::ElfeNoir && playerRace == CharacterRace::DarkElf) return true;
        if (monsterRace == Race::Nain && playerRace == CharacterRace::Dwarf) return true;
        if (monsterRace == Race::Gnome && playerRace == CharacterRace::Gnome) return true;
        if (monsterRace == Race::Halfelin && playerRace == CharacterRace::Halfling) return true;
        if (monsterRace == Race::Tieffelin && playerRace == CharacterRace::Tiefling) return true;
        if (monsterRace == Race::Aasimar && playerRace == CharacterRace::Aasimar) return true;
        if (monsterRace == Race::Kitsune && playerRace == CharacterRace::Kitsune) return true;
        if (monsterRace == Race::Fee && playerRace == CharacterRace::Fairy) return true;
        if ((monsterRace == Race::SemiDragon || monsterRace == Race::Draconide) && playerRace == CharacterRace::HalfDragon) return true;
        if (monsterRace == Race::Orc && playerRace == CharacterRace::Orc) return true;
        if (monsterRace == Race::Demon && playerRace == CharacterRace::Demon) return true;

        return false;
    }

    int recoveredEquipmentDurability(Random& random, int maxDurability)
    {
        if (maxDurability <= 0)
        {
            return 0;
        }

        return std::max(1, maxDurability * random.between(1, 75) / 100);
    }

    void setWeaponDurabilityToRecoveredPercent(Weapon& weapon, Random& random)
    {
        if (weapon.isIndestructible())
        {
            return;
        }

        const int target = recoveredEquipmentDurability(random, weapon.getMaxDurability());
        weapon.loseDurability(std::max(0, weapon.getMaxDurability() - target));
    }

    void setArmorDurabilityToRecoveredPercent(Armor& armor, Random& random)
    {
        if (armor.isIndestructible())
        {
            return;
        }

        const int target = recoveredEquipmentDurability(random, armor.getMaxDurability());
        armor.loseDurability(std::max(0, armor.getMaxDurability() - target));
    }

    Weapon createRecoveredWeaponFromMonster(const Monster& monster, Random& random)
    {
        const int level = std::max(1, monster.getLevel());
        const std::string baseName = containsText(monster.getType(), "archer") || containsText(monster.getType(), "rôdeur") || containsText(monster.getType(), "rodeur")
            ? "Arc récupéré"
            : containsText(monster.getType(), "mage") || containsText(monster.getType(), "shaman") || containsText(monster.getType(), "chamane")
                ? "Bâton récupéré"
                : containsText(monster.getType(), "garde") || containsText(monster.getType(), "lancier")
                    ? "Lance récupérée"
                    : containsText(monster.getType(), "brute") || containsText(monster.getType(), "orc")
                        ? "Hache récupérée"
                        : "Arme récupérée";

        WeaponType type = WeaponType::Sword;
        if (baseName.find("Arc") != std::string::npos) type = WeaponType::Bow;
        else if (baseName.find("Bâton") != std::string::npos) type = WeaponType::Staff;
        else if (baseName.find("Lance") != std::string::npos) type = WeaponType::Spear;
        else if (baseName.find("Hache") != std::string::npos) type = WeaponType::Axe;

        Weapon weapon(
            baseName + " de " + monster.getName(),
            "Arme récupérée après combat. Elle porte des marques d'usage et sa durabilité dépend de l'état dans lequel elle a été ramassée.",
            24 + level * 7,
            type,
            std::max(1, level / 2),
            std::max(2, level + 3),
            std::max(1, level / 3),
            45 + level * 4
        );

        setWeaponDurabilityToRecoveredPercent(weapon, random);
        return weapon;
    }

    Armor createRecoveredArmorFromMonster(const Player& player, const Monster& monster, Random& random)
    {
        const int level = std::max(1, monster.getLevel());
        const bool sameSize = playerRaceMatchesMonsterEquipmentSize(player, monster);
        const bool heavy = containsText(monster.getType(), "garde")
            || containsText(monster.getType(), "soldat")
            || containsText(monster.getType(), "armure")
            || monster.isElite();

        Armor armor(
            sameSize ? "Armure récupérée adaptée" : "Armure récupérée non ajustée",
            sameSize
                ? "Armure récupérée sur une race compatible. Elle peut être équipée après réparation si son état le permet."
                : "Armure récupérée sur une morphologie différente. Mauvaise taille : non ajustée, elle sert surtout à la revente, au démontage ou à une retouche ultérieure.",
            28 + level * 8,
            heavy ? ArmorType::Chainmail : ArmorType::Leather,
            std::max(1, level),
            std::max(0, level / 3 + (heavy ? 1 : 0)),
            50 + level * 5
        );

        setArmorDurabilityToRecoveredPercent(armor, random);
        return armor;
    }

    int weaponTransportUnitCost(const Weapon& weapon)
    {
        switch (EquipmentWeightRules::getWeaponWeightClass(weapon))
        {
            case EquipmentWeightClass::Light: return 1;
            case EquipmentWeightClass::Heavy: return 3;
            case EquipmentWeightClass::Medium:
            default: return 2;
        }
    }

    int armorTransportUnitCost(const Armor& armor)
    {
        switch (EquipmentWeightRules::getArmorWeightClass(armor))
        {
            case EquipmentWeightClass::Light: return 1;
            case EquipmentWeightClass::Heavy: return 3;
            case EquipmentWeightClass::Medium:
            default: return 2;
        }
    }

    void tryGiveRecoveredEquipmentLoot(
        Player& player,
        const Monster& monster,
        Random& random,
        DifficultyMode difficulty,
        std::vector<std::string>& lines,
        int* remainingTransportUnits = nullptr)
    {
        if (!defeatedEnemyCanCarryEquipment(monster))
        {
            return;
        }

        int weaponChance = 5;
        int armorChance = 4;

        if (monster.isElite())
        {
            weaponChance += 4;
            armorChance += 3;
        }
        if (monster.isEvolved())
        {
            weaponChance += 2;
            armorChance += 2;
        }
        if (containsText(monster.getType(), "garde") || containsText(monster.getType(), "soldat") || containsText(monster.getType(), "mercenaire"))
        {
            weaponChance += 3;
            armorChance += 3;
        }

        switch (difficulty)
        {
            case DifficultyMode::Easy: weaponChance += 1; armorChance += 1; break;
            case DifficultyMode::Hard: weaponChance -= 1; break;
            case DifficultyMode::Nightmare: weaponChance -= 2; armorChance -= 1; break;
            case DifficultyMode::Lethal: weaponChance -= 3; armorChance -= 2; break;
            case DifficultyMode::Normal:
            default: break;
        }

        weaponChance = std::clamp(weaponChance, 2, 14);
        armorChance = std::clamp(armorChance, 1, 11);

        if ((remainingTransportUnits == nullptr || *remainingTransportUnits > 0)
            && random.between(1, 100) <= weaponChance)
        {
            Weapon weapon = createRecoveredWeaponFromMonster(monster, random);
            const int transportCost = weaponTransportUnitCost(weapon);
            const std::string dur = weapon.isIndestructible()
                ? "indestructible"
                : std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability());
            if (remainingTransportUnits != nullptr && *remainingTransportUnits < transportCost)
            {
                lines.push_back("Arme laissée sur place faute de capacité de transport ("
                    + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getWeaponWeightClass(weapon))
                    + ", coût " + std::to_string(transportCost) + " unité(s)) : " + weapon.getName() + ".");
            }
            else if (shouldPickupLoot(player, "weapon", "", weapon.getName(), "Durabilité : " + dur + "."))
            {
                player.getInventory().addWeapon(weapon);
                if (remainingTransportUnits != nullptr)
                {
                    *remainingTransportUnits = std::max(0, *remainingTransportUnits - transportCost);
                }
                lines.push_back("Équipement récupéré : " + weapon.getName() + " | "
                    + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getWeaponWeightClass(weapon))
                    + " | Durabilité : " + dur + ".");
                player.recordCanonicalEvent("butin_notable", "weapon:" + weapon.getName(), weapon.getName() + " récupérée sur " + monster.getName(), 1);
            }
            else
            {
                lines.push_back("Arme laissée sur place selon les préférences de butin : " + weapon.getName() + ".");
            }
        }

        if ((remainingTransportUnits == nullptr || *remainingTransportUnits > 0)
            && random.between(1, 100) <= armorChance)
        {
            Armor armor = createRecoveredArmorFromMonster(player, monster, random);
            const int transportCost = armorTransportUnitCost(armor);
            const std::string dur = armor.isIndestructible()
                ? "indestructible"
                : std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability());
            const bool enoughCapacity = remainingTransportUnits == nullptr || *remainingTransportUnits >= transportCost;
            const bool armorCollected = enoughCapacity
                && shouldPickupLoot(player, "armor", "", armor.getName(), "Durabilité : " + dur + ".");
            if (!enoughCapacity)
            {
                lines.push_back("Armure laissée sur place faute de capacité de transport ("
                    + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getArmorWeightClass(armor))
                    + ", coût " + std::to_string(transportCost) + " unité(s)) : " + armor.getName() + ".");
            }
            else if (armorCollected)
            {
                player.getInventory().addArmor(armor);
                if (remainingTransportUnits != nullptr)
                {
                    *remainingTransportUnits = std::max(0, *remainingTransportUnits - transportCost);
                }
                lines.push_back("Équipement récupéré : " + armor.getName() + " | "
                    + EquipmentWeightRules::getWeightLabel(EquipmentWeightRules::getArmorWeightClass(armor))
                    + " | Durabilité : " + dur + ".");
                player.recordCanonicalEvent("butin_notable", "armor:" + armor.getName(), armor.getName() + " récupérée sur " + monster.getName(), 1);
            }
            else
            {
                lines.push_back("Armure laissée sur place selon les préférences de butin : " + armor.getName() + ".");
            }
            if (armorCollected && armor.getDescription().find("non ajustée") != std::string::npos)
            {
                lines.push_back("Note : cette armure n'est pas à ta taille. Elle ne peut pas être équipée sans retouche ultérieure, mais garde une valeur de revente/démontage.");
            }
        }
    }

    enum class ExactPostKoLootKind
    {
        Material,
        Weapon,
        Armor,
        Coins
    };

    struct ExactPostKoLootCandidate
    {
        std::string id;
        std::string sourceMonster;
        std::string displayLabel;
        ExactPostKoLootKind kind = ExactPostKoLootKind::Material;
        int transportCost = 1;
        int priorityScore = 0;
        bool protectedFromAutomaticAbandon = false;
        Material material;
        Weapon weapon;
        Armor armor;
        CoinBreakdown coins;
    };

    int visibleMonsterRecoveryPriority(const Monster& monster)
    {
        return (monster.isElite() ? 30 : 0)
            + (monster.isEvolved() ? 18 : 0)
            + std::min(20, std::max(0, monster.getLevel()));
    }

    template <typename T>
    void applyExactItemRecoveryMetadata(
        ExactPostKoLootCandidate& candidate,
        const T& item,
        const Monster& monster)
    {
        candidate.priorityScore = item.getAutomaticRecoveryPriorityScore()
            + visibleMonsterRecoveryPriority(monster);
        candidate.protectedFromAutomaticAbandon = item.isProtectedFromAutomaticAbandon();
    }

    void addExactPostKoLootCandidateToInventory(
        Inventory& inventory,
        const ExactPostKoLootCandidate& candidate)
    {
        switch (candidate.kind)
        {
            case ExactPostKoLootKind::Material:
                inventory.addMaterial(candidate.material);
                break;
            case ExactPostKoLootKind::Weapon:
                inventory.addWeapon(candidate.weapon);
                break;
            case ExactPostKoLootKind::Armor:
                inventory.addArmor(candidate.armor);
                break;
            case ExactPostKoLootKind::Coins:
                inventory.earnCoinStacks(candidate.coins);
                break;
        }
    }

    void grantExactPostKoLootCandidate(
        Player& player,
        const ExactPostKoLootCandidate& candidate,
        std::vector<std::string>& lines)
    {
        switch (candidate.kind)
        {
            case ExactPostKoLootKind::Material:
                player.getInventory().addMaterial(candidate.material);
                break;
            case ExactPostKoLootKind::Weapon:
                player.getInventory().addWeapon(candidate.weapon);
                break;
            case ExactPostKoLootKind::Armor:
                player.getInventory().addArmor(candidate.armor);
                break;
            case ExactPostKoLootKind::Coins:
                player.getInventory().earnCoinStacks(candidate.coins);
                break;
        }

        std::string metadata;
        if (candidate.protectedFromAutomaticAbandon) metadata += " [priorité protégée]";
        lines.push_back("Ramené depuis " + candidate.sourceMonster + " : " + candidate.displayLabel
            + " | coût transport " + std::to_string(candidate.transportCost) + metadata + ".");
    }
}

void LootGenerator::giveDefeatedWaveLoot(
    Player& player,
    const EnemyCombatQueue& wave,
    Random& random,
    DifficultyMode difficulty,
    int authorizedLootPercent
)
{
    if (wave.getDefeatedEnemyCount() <= 0)
    {
        return;
    }

    std::vector<std::string> lines;
    int lootChance = getLootChance(difficulty);
    bool atLeastOneLoot = false;
    authorizedLootPercent = std::clamp(authorizedLootPercent, 0, 100);

    for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
    {
        const Monster& monster = wave.getDefeatedEnemy(i);
        if (authorizedLootPercent < 100 && random.between(1, 100) > authorizedLootPercent)
        {
            continue;
        }
        if (tryGiveMonsterLoot(player, monster, random, difficulty, lootChance, lines))
        {
            atLeastOneLoot = true;
        }

        const std::size_t beforeEquipmentLines = lines.size();
        tryGiveRecoveredEquipmentLoot(player, monster, random, difficulty, lines);
        if (lines.size() > beforeEquipmentLines)
        {
            atLeastOneLoot = true;
        }
    }

    int progressedQuests = 0;
    for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
    {
        progressedQuests += player.getQuestLog().progressCombatQuestsForMonster(wave.getDefeatedEnemy(i));
    }

    int completedDeliveryQuests = player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());

    if (progressedQuests > 0 || completedDeliveryQuests > 0)
    {
        lines.push_back("Journal de quêtes mis à jour.");
    }

    if (authorizedLootPercent < 100)
    {
        lines.push_back("Part autorisée récupérable : environ " + std::to_string(authorizedLootPercent) + " %. Le reste est remis aux autorités ou laissé sous scellés.");
    }
    if (!atLeastOneLoot)
    {
        lines.push_back("Aucun matériau intéressant récupéré cette fois.");
    }

    showLootScreen("BUTIN", "combat.loot.wave.result", lines);
}



void LootGenerator::givePostKoRecoveredWaveLoot(
    Player& player,
    const EnemyCombatQueue& wave,
    Random& random,
    DifficultyMode difficulty,
    int transportCapacityUnits,
    const std::string& contextId,
    const std::string& battlefieldLabel)
{
    const int defeatedCount = std::max(0, wave.getDefeatedEnemyCount());
    transportCapacityUnits = std::max(0, transportCapacityUnits);

    std::vector<std::string> lines;
    lines.push_back("Le butin n'est pas téléporté dans ton inventaire : les objets sont d'abord générés exactement, puis l'équipe choisit ce qu'elle peut réellement transporter.");
    const std::string capacityLabel = transportCapacityUnits <= 0 ? "aucune"
        : transportCapacityUnits <= 2 ? "très limitée"
        : transportCapacityUnits <= 4 ? "limitée"
        : transportCapacityUnits <= 7 ? "correcte"
        : "confortable";
    lines.push_back("Capacité de récupération du groupe après ton extraction : " + capacityLabel + ".");
    lines.push_back("Les objets Quête/Personnel/Unique et les priorités explicites passent avant le loot ordinaire ; à priorité égale, le groupe favorise ce qui tient réellement dans la capacité restante.");

    if (transportCapacityUnits <= 0 || defeatedCount <= 0)
    {
        if (defeatedCount > 0)
        {
            lines.push_back("Personne n'avait assez de capacité restante pour ramener du butin en plus du joueur blessé.");
            player.recordCanonicalEvent(
                "butin_post_ko_transport",
                contextId + ":aucun_transport",
                "Butin physique abandonné faute de capacité de transport après le KO du joueur.",
                defeatedCount);
        }
        showLootScreen("BUTIN POST-KO", "combat.loot.post_ko.none", lines);
        return;
    }

    const int lootChance = getLootChance(difficulty);
    std::vector<ExactPostKoLootCandidate> exactCandidates;
    int exactCandidateSerial = 0;
    for (int enemyIndex = 0; enemyIndex < wave.getDefeatedEnemyCount(); ++enemyIndex)
    {
        const Monster& monster = wave.getDefeatedEnemy(enemyIndex);

        // Generate the actual loot first in an isolated copy. This preserves the normal loot
        // RNG/pickup rules without granting anything to the unconscious player's real inventory.
        Player staging = player;
        staging.getInventory().clearAll();
        std::vector<std::string> ignoredLines;
        tryGiveMonsterLoot(staging, monster, random, difficulty, lootChance, ignoredLines);
        tryGiveRecoveredEquipmentLoot(staging, monster, random, difficulty, ignoredLines);

        const Inventory& stagedInventory = staging.getInventory();
        for (const Material& material : stagedInventory.getMaterials())
        {
            ExactPostKoLootCandidate candidate;
            candidate.id = "material:" + std::to_string(enemyIndex) + ":" + std::to_string(exactCandidateSerial++);
            candidate.sourceMonster = monster.getName();
            candidate.displayLabel = material.getName() + " x" + std::to_string(material.getQuantity());
            candidate.kind = ExactPostKoLootKind::Material;
            candidate.transportCost = 1;
            candidate.material = material;
            applyExactItemRecoveryMetadata(candidate, material, monster);
            exactCandidates.push_back(candidate);
        }
        for (const Weapon& weapon : stagedInventory.getWeapons())
        {
            ExactPostKoLootCandidate candidate;
            candidate.id = "weapon:" + std::to_string(enemyIndex) + ":" + weapon.getPersistentId() + ":" + std::to_string(exactCandidateSerial++);
            candidate.sourceMonster = monster.getName();
            candidate.displayLabel = weapon.getName();
            candidate.kind = ExactPostKoLootKind::Weapon;
            candidate.transportCost = weaponTransportUnitCost(weapon);
            candidate.weapon = weapon;
            applyExactItemRecoveryMetadata(candidate, weapon, monster);
            exactCandidates.push_back(candidate);
        }
        for (const Armor& armor : stagedInventory.getArmors())
        {
            ExactPostKoLootCandidate candidate;
            candidate.id = "armor:" + std::to_string(enemyIndex) + ":" + armor.getPersistentId() + ":" + std::to_string(exactCandidateSerial++);
            candidate.sourceMonster = monster.getName();
            candidate.displayLabel = armor.getName();
            candidate.kind = ExactPostKoLootKind::Armor;
            candidate.transportCost = armorTransportUnitCost(armor);
            candidate.armor = armor;
            applyExactItemRecoveryMetadata(candidate, armor, monster);
            exactCandidates.push_back(candidate);
        }

        const CoinBreakdown coins = stagedInventory.getCoinStacks();
        const long long totalCoinCount = coins.copper + coins.iron + coins.electrum + coins.gold + coins.platinum;
        if (totalCoinCount > 0)
        {
            ExactPostKoLootCandidate candidate;
            candidate.id = "coins:" + std::to_string(enemyIndex) + ":" + std::to_string(exactCandidateSerial++);
            candidate.sourceMonster = monster.getName();
            candidate.displayLabel = Money::formatCoinStacks(coins, true);
            candidate.kind = ExactPostKoLootKind::Coins;
            candidate.transportCost = 1;
            candidate.priorityScore = visibleMonsterRecoveryPriority(monster);
            candidate.coins = coins;
            exactCandidates.push_back(candidate);
        }
    }

    std::vector<PostKoRecoveryCandidate> recoveryCandidates;
    recoveryCandidates.reserve(exactCandidates.size());
    for (const ExactPostKoLootCandidate& candidate : exactCandidates)
    {
        recoveryCandidates.push_back({
            candidate.id,
            candidate.transportCost,
            candidate.priorityScore,
            candidate.protectedFromAutomaticAbandon
        });
    }

    const PostKoRecoverySelection selection = RecruitedAllyAftermathSystem::selectPostKoRecoveryCandidates(
        recoveryCandidates, transportCapacityUnits);

    auto isRecovered = [&](const std::string& id) {
        return std::find(selection.recoveredIds.begin(), selection.recoveredIds.end(), id) != selection.recoveredIds.end();
    };

    int recoveredExactObjects = 0;
    int deferredProtectedObjects = 0;
    Inventory deferredProtectedInventory;
    for (const ExactPostKoLootCandidate& candidate : exactCandidates)
    {
        if (isRecovered(candidate.id))
        {
            grantExactPostKoLootCandidate(player, candidate, lines);
            ++recoveredExactObjects;
        }
        else
        {
            if (candidate.protectedFromAutomaticAbandon)
            {
                ++deferredProtectedObjects;
                addExactPostKoLootCandidateToInventory(deferredProtectedInventory, candidate);
            }
            lines.push_back("Laissé sur place faute de capacité : " + candidate.displayLabel
                + " (" + candidate.sourceMonster + ", coût " + std::to_string(candidate.transportCost) + ")"
                + (candidate.protectedFromAutomaticAbandon ? " — objet protégé mais physiquement impossible à emporter maintenant." : "."));
        }
    }

    if (exactCandidates.empty())
    {
        lines.push_back("Les dépouilles fouillées ne contenaient finalement aucun bien récupérable selon les règles de loot actuelles.");
    }
    else if (selection.remainingCarryUnits <= 0)
    {
        lines.push_back("Le groupe repart chargé au maximum raisonnable pour cette extraction.");
    }
    else
    {
        lines.push_back("Le groupe conserve encore " + std::to_string(selection.remainingCarryUnits) + " unité(s) de marge après le tri exact.");
    }
    if (deferredProtectedObjects > 0)
    {
        const std::string cacheId = player.createBattlefieldRecoveryCache(
            contextId,
            battlefieldLabel.empty() ? "zone du combat" : battlefieldLabel,
            deferredProtectedInventory);
        lines.push_back(std::to_string(deferredProtectedObjects) + " objet(s) protégé(s) restent physiquement sur le terrain dans une piste de récupération persistante.");
        if (!cacheId.empty())
            lines.push_back("Piste enregistrée : " + cacheId + ". Tu pourras tenter d'y retourner depuis les lieux de la ville concernée.");
    }

    player.recordCanonicalEvent(
        "butin_post_ko_transport",
        contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
        "Butin exact physiquement ramené après KO | objets=" + std::to_string(recoveredExactObjects)
            + "/" + std::to_string(exactCandidates.size())
            + " | capacite=" + std::to_string(selection.usedCarryUnits)
            + "/" + std::to_string(transportCapacityUnits),
        recoveredExactObjects);

    showLootScreen("BUTIN RAPPORTÉ PAR L'ÉQUIPE", "combat.loot.post_ko.wave", lines);
}

void LootGenerator::givePostKoRecoveredBossLoot(
    Player& player,
    const Boss& boss,
    Random& random,
    DifficultyMode difficulty,
    int transportCapacityUnits,
    const std::string& contextId)
{
    transportCapacityUnits = std::max(0, transportCapacityUnits);
    if (transportCapacityUnits <= 0)
    {
        player.recordCanonicalEvent(
            "butin_post_ko_transport",
            contextId + ":boss_abandonne",
            "Le boss est tombé pendant l'inconscience du joueur, mais aucun survivant ne pouvait ramener son fragment en plus de l'extraction.",
            1);
        return;
    }

    player.recordCanonicalEvent(
        "butin_post_ko_transport",
        contextId + ":boss_fragment",
        "Le groupe a priorisé le fragment portable de " + boss.getName() + " pendant l'extraction du joueur.",
        1);
    MessageScreen::show(
        "BUTIN DE BOSS TRANSPORTÉ",
        "combat.loot.post_ko.boss_transport",
        {
            "Le fragment de boss ne rejoint pas magiquement ton sac.",
            "Un survivant l'a physiquement récupéré puis transporté avec le groupe jusqu'à ton réveil.",
            "La place disponible après ton extraction suffisait encore pour ce lot portable de boss."
        },
        false);
    giveDefeatedBossLoot(player, boss, random, difficulty);
}

void LootGenerator::giveDefeatedBossLoot(
    Player& player,
    const Boss& boss,
    Random& random,
    DifficultyMode difficulty
)
{
    std::vector<std::string> lines;

    int fragmentQuantity = 1;

    if (difficulty == DifficultyMode::Hard && random.between(1, 100) <= 25)
    {
        fragmentQuantity++;
    }
    else if (difficulty == DifficultyMode::Nightmare && random.between(1, 100) <= 35)
    {
        fragmentQuantity++;
    }
    else if (difficulty == DifficultyMode::Lethal && random.between(1, 100) <= 45)
    {
        fragmentQuantity++;
    }

    Material bossFragment;

    if (boss.getBossId() == 1)
    {
        bossFragment = MaterialCatalog::createFitoriaFeather(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(2));
        lines.push_back("La lumière retombe en poussière arcanique x2.");
    }
    else if (boss.getBossId() == 2)
    {
        bossFragment = MaterialCatalog::createZelefDemonBlood(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createUnstableCore(1));
        lines.push_back("Un noyau instable bat encore dans les restes du démon.");
    }
    else if (boss.getBossId() == 3)
    {
        bossFragment = MaterialCatalog::createAtlasBrokenPlate(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createDraconicScaleFragment(1));
        lines.push_back("Une résistance presque draconique reste accrochée à la plaque.");
    }
    else if (boss.getBossId() == 4)
    {
        bossFragment = MaterialCatalog::createLyknirHuntShard(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createWolfFang(2));
        lines.push_back("La meute laisse derrière elle des crocs marqués par l'écho.");
    }
    else if (boss.getBossId() == 5)
    {
        bossFragment = MaterialCatalog::createGrinkaAvariceCoin(fragmentQuantity);
        player.getInventory().earnEconomyUnits(random.between(35, 85));
        player.refreshCurrencyTitles();
        lines.push_back("Quelques pièces tombent du sac royal avant que les collecteurs ne les récupèrent.");
    }
    else if (boss.getBossId() == 6)
    {
        bossFragment = MaterialCatalog::createAzelanosDarkCrownShard(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createUnstableCore(1));
        lines.push_back("Un noyau instable palpite dans les cendres de la couronne noire.");
    }
    else if (boss.getBossId() == 7)
    {
        bossFragment = MaterialCatalog::createThamarysOriginScale(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createDraconicScaleFragment(2));
        lines.push_back("Des fragments d'écailles ordinaires tombent autour de l'écaille d'origine.");
    }
    else if (boss.getBossId() == 8)
    {
        bossFragment = MaterialCatalog::createMojoAncientSeed(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBitterHealingLeaf(2));
        lines.push_back("La forêt laisse quelques feuilles médicinales près de la graine ancienne.");
    }
    else if (boss.getBossId() == 9)
    {
        bossFragment = MaterialCatalog::createInakariMirrorShard(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createKitsuneEmber(1));
        lines.push_back("Une braise kitsune continue de danser dans le reflet brisé.");
    }
    else if (boss.getBossId() == 10)
    {
        bossFragment = MaterialCatalog::createSilentJudgmentSeal(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createShadowThread(1));
        lines.push_back("Un fil d'ombre reste accroché au sceau muet.");
    }
    else if (boss.getBossId() == 11)
    {
        bossFragment = MaterialCatalog::createAnomalyGlitchFragment(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createVariationResidue(2));
        lines.push_back("Le fragment glitch laisse aussi deux résidus de variation.");
    }
    else if (boss.getBossId() == 36)
    {
        bossFragment = MaterialCatalog::createAnomalyGlitchFragment(fragmentQuantity + 1);
        player.getInventory().addMaterial(MaterialCatalog::createVariationResidue(3));
        lines.push_back("La Source ne laisse pas un cadavre : seulement des textures mortes et trois résidus de variation.");
    }
    else if (boss.getBossId() == 12)
    {
        bossFragment = MaterialCatalog::createDeadMinuteGear(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(2));
        lines.push_back("De la poussière arcanique tombe entre deux secondes mortes.");
    }
    else if (boss.getBossId() == 13)
    {
        bossFragment = MaterialCatalog::createBuriedBoneLullaby(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createCrackedBone(3));
        lines.push_back("Des os fissurés restent dans le sable après le silence.");
    }
    else if (boss.getBossId() == 14)
    {
        bossFragment = MaterialCatalog::createBorosWarMark(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBattleTornBadge(2));
        lines.push_back("Deux insignes abîmés portent encore l'écho du champ de bataille.");
    }
    else if (boss.getBossId() == 15)
    {
        bossFragment = MaterialCatalog::createAnastasiaBoundHeart(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(1));
        lines.push_back("Une poussière douce et lourde tombe du coeur lié.");
    }
    else if (boss.getBossId() == 16)
    {
        bossFragment = MaterialCatalog::createLexiorJusticeSplinter(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBossIdentityScrap(1));
        lines.push_back("Un fragment de registre reste coincé dans le verdict.");
    }
    else if (boss.getBossId() == 17)
    {
        bossFragment = MaterialCatalog::createLunarDreamFragment(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(2));
        lines.push_back("La poussière arcanique tombe comme de la neige dans un rêve.");
    }
    else if (boss.getBossId() == 18)
    {
        bossFragment = MaterialCatalog::createElementalFusionCore(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createUnstableCore(1));
        lines.push_back("Un noyau instable vibre encore entre quatre éléments.");
    }
    else if (boss.getBossId() == 19)
    {
        bossFragment = MaterialCatalog::createHumanWillFragment(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBattleTornBadge(1));
        lines.push_back("Un insigne abîmé reste près d'une volonté qui refusait de tomber.");
    }
    else if (boss.getBossId() == 20)
    {
        bossFragment = MaterialCatalog::createConsciousLuckShard(fragmentQuantity);
        player.getInventory().earnEconomyUnits(random.between(20, 70));
        player.refreshCurrencyTitles();
        lines.push_back("Quelques pièces tombent du bon côté, comme si le hasard avait souri une dernière fois.");
    }
    else if (boss.getBossId() == 21)
    {
        bossFragment = MaterialCatalog::createProgressionSeal(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBossIdentityScrap(1));
        lines.push_back("Un fragment de registre reste collé au sceau du seuil.");
    }
    else if (boss.getBossId() == 22)
    {
        bossFragment = MaterialCatalog::createAbsentThroneFragment(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createRustedMetalFragment(2));
        lines.push_back("Deux éclats de métal rouillé tombent d'une couronne qui n'a plus de salle.");
    }
    else if (boss.getBossId() == 23)
    {
        bossFragment = MaterialCatalog::createLostNameFragment(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBeastHide(2));
        lines.push_back("La bête laisse une peau impossible à classer. Elle ne ressemble déjà plus à celle du combat.");
    }
    else if (boss.getBossId() == 24)
    {
        bossFragment = MaterialCatalog::createAldebarothAbyssResidue(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createShadowThread(2));
        lines.push_back("Deux fils d'ombre restent collés au résidu d'abîme.");
    }
    else if (boss.getBossId() == 25)
    {
        bossFragment = MaterialCatalog::createTwinParadoxShard(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createUnstableCore(2));
        lines.push_back("Deux noyaux instables vibrent entre création et destruction.");
    }
    else if (boss.getBossId() == 26)
    {
        bossFragment = MaterialCatalog::createOberionOriginThread(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBossIdentityScrap(2));
        lines.push_back("Deux fragments de registre tentent de suivre le fil d'origine.");
    }
    else if (boss.getBossId() == 27)
    {
        bossFragment = MaterialCatalog::createUnstableVersionCore(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createVariationResidue(3));
        lines.push_back("Trois résidus de variation tombent comme des lignes supprimées.");
    }
    else if (boss.getBossId() == 28)
    {
        bossFragment = MaterialCatalog::createFacelessBreath(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createShadowThread(2));
        lines.push_back("Deux fils d'ombre restent suspendus dans un souffle qui n'a jamais eu de bouche.");
    }
    else if (boss.getBossId() == 29)
    {
        bossFragment = MaterialCatalog::createPuppetNail(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createRustedMetalFragment(3));
        lines.push_back("Trois éclats métalliques tombent avec les clous de la marionnette.");
    }
    else if (boss.getBossId() == 30)
    {
        bossFragment = MaterialCatalog::createMoiranFateThread(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBossIdentityScrap(2));
        lines.push_back("Deux fragments de registre essaient de suivre une destinée qui se déplace encore.");
    }
    else if (boss.getBossId() == 31)
    {
        bossFragment = MaterialCatalog::createLostSoulAntler(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBeastHide(1));
        lines.push_back("Une peau silencieuse reste près de l'éclat d'âme perdue.");
    }

    else if (boss.getBossId() == 32)
    {
        bossFragment = MaterialCatalog::createGorvaldRoyalBlood(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createRustedMetalFragment(2));
        lines.push_back("Deux éclats de hache tribale tombent près du sang royal.");
    }
    else if (boss.getBossId() == 33)
    {
        bossFragment = MaterialCatalog::createSeranaRoyalFang(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createShadowThread(1));
        lines.push_back("Un fil d'ombre reste froid malgré le sang rouge.");
    }
    else if (boss.getBossId() == 34)
    {
        bossFragment = MaterialCatalog::createDraiiteBlackSilk(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createSlimeResidue(1));
        lines.push_back("Un résidu collant reste accroché à la soie noire.");
    }
    else if (boss.getBossId() == 35)
    {
        bossFragment = MaterialCatalog::createBrokenMirrorShard(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createBossIdentityScrap(2));
        lines.push_back("Deux fragments de registre reflètent deux vérités différentes.");
    }
    else
    {
        bossFragment = MaterialCatalog::createUnstableVersionCore(fragmentQuantity);
        player.getInventory().addMaterial(MaterialCatalog::createVariationResidue(3));
        lines.push_back("Trois résidus de variation tombent comme des lignes supprimées.");
    }

    // Les fragments principaux de boss sont de vrais objets uniques pour la logique de
    // récupération : ils ne doivent pas être traités comme un lot banal si l'équipe doit
    // choisir quoi transporter après un KO. Cela reste distinct d'une liaison à l'âme.
    bossFragment.setUniqueRecoveryItem(true);
    bossFragment.setRecoveryPriority(100);
    bossFragment.setQuality("pure");

    if (random.between(1, 100) <= 12)
    {
        bossFragment.setQuality("exceptional");
        lines.push_back("Récupération parfaite : le fragment de boss est exceptionnel.");
    }

    addMaterialLootToPlayer(player, bossFragment);

    lines.push_back(
        boss.getName()
        + " laisse un fragment unique : "
        + bossFragment.getName()
        + " x"
        + std::to_string(bossFragment.getQuantity())
        + "."
    );
    lines.push_back("Ces fragments ne forment pas une relique complète, mais leur présence pèse déjà dans ton inventaire.");

    showLootScreen("BUTIN DE BOSS", "combat.loot.boss.result", lines);
}

// EN: getLootChance declares or implements a focused behavior used by this module.
// FR: getLootChance déclare ou implémente un comportement précis utilisé par ce module.
int LootGenerator::getLootChance(DifficultyMode difficulty)
{
    return DifficultyRules::getLootChancePercentage(difficulty);
}

int LootGenerator::getLootQuantity(
    Random& random,
    DifficultyMode difficulty,
    const Monster& monster
)
{
    int quantity = 1;

    if (monster.isElite())
    {
        quantity++;
    }

    if (monster.isEvolved())
    {
        quantity++;
    }

    if (monster.getLevel() >= 4 && random.between(1, 100) <= 35)
    {
        quantity++;
    }

    switch (difficulty)
    {
        case DifficultyMode::Easy:
        case DifficultyMode::Normal:
        case DifficultyMode::Hard:
        case DifficultyMode::Nightmare:
        case DifficultyMode::Lethal:
        default:
            if (random.between(1, 100) <= DifficultyRules::getLootQuantityBonusChance(difficulty))
            {
                quantity++;
            }
            break;
    }

    return quantity;
}

bool LootGenerator::tryGiveMonsterLoot(
    Player& player,
    const Monster& monster,
    Random& random,
    DifficultyMode difficulty,
    int lootChance,
    std::vector<std::string>& lines
)
{
    bool foundGoodLoot = random.between(1, 100) <= lootChance;
    int quantity = foundGoodLoot ? getLootQuantity(random, difficulty, monster) : 1;
    Material loot;

    switch (monster.getRace())
    {
        case Race::Gobelin:
            loot = MaterialCatalog::createGoblinEar(quantity);
            break;

        case Race::Bete:
        {
            if (isWolfLikeBeast(monster) && random.between(1, 100) <= 70)
            {
                loot = MaterialCatalog::createWolfFang(quantity);
            }
            else if (isBoarLikeBeast(monster) && random.between(1, 100) <= 65)
            {
                loot = MaterialCatalog::createBoarTusk(quantity);
            }
            else if (isRodentLikeBeast(monster) && random.between(1, 100) <= 60)
            {
                loot = MaterialCatalog::createRodentTooth(quantity);
            }
            else if (isFangedPredatorBeast(monster) && random.between(1, 100) <= 55)
            {
                loot = MaterialCatalog::createPredatorFang(quantity);
            }
            else
            {
                loot = MaterialCatalog::createBeastHide(quantity);
            }
            break;
        }

        case Race::Humain:
        case Race::SemiHumain:
        case Race::Elfe:
        case Race::ElfeNoir:
        case Race::Nain:
        case Race::Gnome:
        case Race::Halfelin:
        case Race::Tieffelin:
        case Race::Hobgobelin:
        case Race::Orc:
        {
            const int humanoidLootRoll = random.between(1, 100);
            if (humanoidLootRoll <= 18)
            {
                loot = MaterialCatalog::createBattleTornBadge(quantity);
            }
            else if (humanoidLootRoll <= 55)
            {
                loot = MaterialCatalog::createRustedMetalFragment(quantity);
            }
            else if (humanoidLootRoll <= 78)
            {
                loot = MaterialCatalog::createWeakRepairKit(1);
            }
            else
            {
                loot = MaterialCatalog::createArcaneDust(quantity);
            }
            break;
        }

        case Race::MortVivant:
        case Race::Esprit:
            loot = random.between(1, 100) <= 72 ? MaterialCatalog::createCrackedBone(quantity) : MaterialCatalog::createShadowThread(quantity);
            break;

        case Race::Plante:
            loot = MaterialCatalog::createBitterHealingLeaf(quantity);
            break;

        case Race::Slime:
            loot = MaterialCatalog::createSlimeResidue(quantity);
            break;

        case Race::Demon:
        case Race::Aasimar:
        case Race::Fee:
            loot = MaterialCatalog::createArcaneDust(quantity);
            break;

        case Race::SemiDragon:
        case Race::Dragon:
        case Race::Draconide:
            loot = random.between(1, 100) <= 65 ? MaterialCatalog::createDraconicScaleFragment(quantity) : MaterialCatalog::createArcaneDust(quantity);
            break;

        case Race::Elementaire:
        case Race::AnomalieArcanique:
        case Race::Aberration:
            loot = random.between(1, 100) <= 55 ? MaterialCatalog::createUnstableCore(quantity) : MaterialCatalog::createArcaneDust(quantity);
            break;

        case Race::Insectoide:
            loot = random.between(1, 100) <= 55 ? MaterialCatalog::createWornLeatherPiece(quantity) : MaterialCatalog::createSlimeResidue(quantity);
            break;

        case Race::Construction:
            loot = random.between(1, 100) <= 70 ? MaterialCatalog::createRustedMetalFragment(quantity) : MaterialCatalog::createRunicIronShard(quantity);
            break;

        default:
        {
            const int genericLootRoll = random.between(1, 100);
            if (genericLootRoll <= 28)
            {
                loot = MaterialCatalog::createWornLeatherPiece(quantity);
            }
            else if (genericLootRoll <= 58)
            {
                loot = MaterialCatalog::createBitterHealingLeaf(quantity);
            }
            else if (genericLootRoll <= 82)
            {
                loot = MaterialCatalog::createSlimeResidue(quantity);
            }
            else
            {
                loot = MaterialCatalog::createArcaneDust(quantity);
            }
            break;
        }
    }

    if (monster.isElite() && random.between(1, 100) <= 20)
    {
        Material extra = MaterialCatalog::createWeakRepairKit(1);
        giveExtraLoot(player, extra, monster.getName() + " possédait aussi un outil intact", lines);
    }

    if (monster.isEvolved() && random.between(1, 100) <= 28)
    {
        Material extra = MaterialCatalog::createArcaneDust(1);
        extra.setQuality(random.between(1, 100) <= 20 ? "exceptional" : "pure");
        giveExtraLoot(player, extra, "Résidu de variation récupéré sur la créature évoluée", lines);
    }

    if (canLogicallyCarryMoney(monster) && random.between(1, 100) <= 4)
    {
        giveExtraLoot(player, MaterialCatalog::createPrecisionHarvestTools(1), "Matériel de terrain récupéré", lines);
    }

    if ((containsText(monster.getType(), "alchim") || containsText(monster.getType(), "mage") || monster.getRace() == Race::Fee)
        && random.between(1, 100) <= 5)
    {
        giveExtraLoot(player, MaterialCatalog::createPreservationVials(1), "Sacoche fragile récupérée", lines);
    }

    if ((monster.getRace() == Race::Bete || containsText(monster.getType(), "bête")) && random.between(1, 100) <= 35)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createBeastHide(1), monster, player, random, true, lines), "Dépouille exploitable", lines);
    }

    if ((monster.getRace() == Race::ElfeNoir || containsText(monster.getName(), "Hazak") || containsText(monster.getType(), "assassin"))
        && random.between(1, 100) <= 25)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createShadowThread(1), monster, player, random, true, lines), "Trace d'ombre récupérée", lines);
    }

    if ((monster.getRace() == Race::Kitsune || containsText(monster.getName(), "Aoi")) && random.between(1, 100) <= 30)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createKitsuneEmber(1), monster, player, random, true, lines), "Braise instable laissée au sol", lines);
    }

    if ((monster.getRace() == Race::SemiDragon || containsText(monster.getName(), "Kanad")) && random.between(1, 100) <= 30)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createDraconicScaleFragment(1), monster, player, random, true, lines), "Fragment solide arraché au choc", lines);
    }

    if ((monster.getRace() == Race::Fee || containsText(monster.getName(), "Fail") || containsText(monster.getType(), "mage") || containsText(monster.getType(), "mancien") || containsText(monster.getType(), "anomalie"))
        && random.between(1, 100) <= 22)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createUnstableCore(1), monster, player, random, true, lines), "Résidu magique condensé", lines);
    }

    if ((monster.getRace() == Race::Construction || containsText(monster.getType(), "rune")) && random.between(1, 100) <= 26)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createRunicIronShard(1), monster, player, random, true, lines), "Éclat runique récupéré dans la carcasse", lines);
    }

    if ((monster.getRace() == Race::AnomalieArcanique || containsText(monster.getType(), "variation") || containsText(monster.getType(), "anomalie")) && random.between(1, 100) <= 24)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createVariationResidue(1), monster, player, random, true, lines), "Résidu de variation stabilisé", lines);
    }

    giveCoinsIfLogical(player, monster, random, lines);

    loot = applyLootQuality(loot, monster, player, random, foundGoodLoot, lines);
    const std::string lootReason = !foundGoodLoot
        ? "Butin minimal récupéré sur " + monster.getName()
        : monster.getName() + " laisse quelque chose derrière lui";
    const bool collected = addMaterialLootToPlayer(player, loot, &lines, lootReason);
    if (collected)
    {
        addLootLine(lines, lootReason, loot);
    }

    return collected;
}
