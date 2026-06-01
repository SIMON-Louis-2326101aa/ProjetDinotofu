// EN: LootGenerator.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: LootGenerator.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// English: Implements simple monster loot generation for the first playable reward pass.
// Français : Implémente une génération simple de loots de monstres pour la première version jouable des récompenses.

#include "combat/loot/LootGenerator.hpp"

#include "entity/Race.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/WeaponType.hpp"
#include "progression/DifficultyRules.hpp"
#include "interface/menu/common/MessageScreen.hpp"

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
            player.getInventory().earnGold(coins);
            lines.push_back(monster.getName() + " avait " + std::to_string(coins) + " pièce(s) sur lui.");
        }

        int largePurseChance = canCarryLargePurse(monster) ? 8 : 1;

        if (monster.isElite())
        {
            largePurseChance += 6;
        }

        if (random.between(1, 100) <= largePurseChance)
        {
            int coins = random.between(55, 85) + monster.getLevel() * random.between(8, 12);
            player.getInventory().earnGold(coins);
            lines.push_back("Belle trouvaille : une bourse plus lourde que prévu contient " + std::to_string(coins) + " pièce(s).");
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

    // EN: giveExtraLoot declares or implements a focused behavior used by this module.
    // FR: giveExtraLoot déclare ou implémente un comportement précis utilisé par ce module.
    void giveExtraLoot(Player& player, Material material, const std::string& reason, std::vector<std::string>& lines)
    {
        player.getInventory().addMaterial(material);
        addLootLine(lines, reason, material);
    }
}

void LootGenerator::giveDefeatedWaveLoot(
    Player& player,
    const EnemyCombatQueue& wave,
    Random& random,
    DifficultyMode difficulty
)
{
    if (wave.getDefeatedEnemyCount() <= 0)
    {
        return;
    }

    std::vector<std::string> lines;
    int lootChance = getLootChance(difficulty);
    bool atLeastOneLoot = false;

    for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
    {
        const Monster& monster = wave.getDefeatedEnemy(i);
        if (tryGiveMonsterLoot(player, monster, random, difficulty, lootChance, lines))
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

    if (!atLeastOneLoot)
    {
        lines.push_back("Aucun matériau intéressant récupéré cette fois.");
    }

    showLootScreen("BUTIN", "combat.loot.wave.result", lines);
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
        player.getInventory().earnGold(random.between(35, 85));
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
        player.getInventory().earnGold(random.between(20, 70));
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

    bossFragment.setQuality("pure");

    if (random.between(1, 100) <= 12)
    {
        bossFragment.setQuality("exceptional");
        lines.push_back("Récupération parfaite : le fragment de boss est exceptionnel.");
    }

    player.getInventory().addMaterial(bossFragment);

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
            loot = MaterialCatalog::createWolfFang(quantity);
            break;

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
            if (random.between(1, 100) <= 35)
            {
                loot = MaterialCatalog::createBattleTornBadge(quantity);
            }
            else
            {
                loot = MaterialCatalog::createRustedMetalFragment(quantity);
            }
            break;

        case Race::MortVivant:
            loot = MaterialCatalog::createCrackedBone(quantity);
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
        case Race::SemiDragon:
            loot = MaterialCatalog::createArcaneDust(quantity);
            break;

        default:
            loot = MaterialCatalog::createWornLeatherPiece(quantity);
            break;
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

    if ((monster.getRace() == Race::Fee || containsText(monster.getName(), "Fail") || containsText(monster.getType(), "mage"))
        && random.between(1, 100) <= 22)
    {
        giveExtraLoot(player, applyLootQuality(MaterialCatalog::createUnstableCore(1), monster, player, random, true, lines), "Résidu magique condensé", lines);
    }

    giveCoinsIfLogical(player, monster, random, lines);

    loot = applyLootQuality(loot, monster, player, random, foundGoodLoot, lines);
    player.getInventory().addMaterial(loot);

    if (!foundGoodLoot)
    {
        addLootLine(lines, "Butin minimal récupéré sur " + monster.getName(), loot);
    }
    else
    {
        addLootLine(lines, monster.getName() + " laisse quelque chose derrière lui", loot);
    }

    return true;
}
