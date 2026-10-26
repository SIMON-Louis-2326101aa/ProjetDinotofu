// EN: CombatActions.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatActions.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/CombatActions.hpp"
#include "combat/MagicMaterialRules.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/action/CombatPotion.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "progression/TechniqueProgressionRules.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"

#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "entity/Boss.hpp"
#include "item/weapon/WeaponType.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <iostream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <initializer_list>


namespace
{


    struct CombatOptionSpec
    {
        int number;
        std::string label;
        std::string hint;
        bool enabled;
        std::string actionId;
    };

    void showCombatActionMessage(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    )
    {
        MessageScreen::show(title, screenId, lines, waitAndClear);
    }

    int askStructuredCombatChoice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        const std::vector<CombatOptionSpec>& options,
        const std::string& invalidMessage = "Choix invalide."
    )
    {
        MenuScreen screen(title, screenId);
        screen.setChoiceInput("Choisis une action affichée.");

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        for (const CombatOptionSpec& option : options)
        {
            screen.addOption(option.number, option.label, option.hint, option.enabled, option.actionId);
        }

        return TerminalInterface::askMenuChoiceFromOptions(screen, invalidMessage);
    }

    void showCooldownApplied(const Entity& attacker, int turns)
    {
        showCombatActionMessage(
            "SOUFFLE DE COMPÉTENCE",
            "combat.class_skill.cooldown",
            {
                attacker.getName() + " doit reprendre son souffle.",
                "Récupération restante : " + std::to_string(turns) + " tour(s)."
            },
            false
        );
    }

    int canonicalRecordCount(const Player& player, const std::string& category, const std::string& key)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                total += record.count;
            }
        }
        return total;
    }

    bool consumePendingDivineExecution(Entity& attacker, Entity& defender)
    {
        Player* player = dynamic_cast<Player*>(&attacker);
        if (player == nullptr || dynamic_cast<Boss*>(&defender) != nullptr)
        {
            return false;
        }

        const int armed = canonicalRecordCount(*player, "cheat_effects", "divine_sequence_pending_execution");
        const int consumed = canonicalRecordCount(*player, "cheat_effects", "divine_sequence_execution_consumed");
        if (armed <= consumed || defender.isDead())
        {
            return false;
        }

        const int remainingHp = std::max(1, defender.getHp());
        defender.takeDamage(remainingHp);
        player->recordCanonicalEvent(
            "cheat_effects",
            "divine_sequence_execution_consumed",
            "Exécution divine consommée sur une cible hostile réelle",
            1
        );
        player->recordHistoricalEvent(
            "cheat_divine_execution",
            defender.getName(),
            "La séquence 13-02313-1302313 a exécuté une cible hostile ordinaire lors d'un vrai combat.",
            true
        );
        showCombatActionMessage(
            "SÉQUENCE DIVINE",
            "cheat.divine_sequence.execution",
            {
                "La séquence en attente reconnaît " + defender.getName() + ".",
                "La cible est abattue immédiatement.",
                "L'exécution a été consommée."
            },
            false
        );
        return true;
    }

    void showClassSkillResult(
        const std::string& skillName,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        std::vector<std::string> screenLines;
        screenLines.push_back("Compétence de classe : " + skillName + ".");
        screenLines.insert(screenLines.end(), lines.begin(), lines.end());

        showCombatActionMessage(
            "COMPÉTENCE DE CLASSE",
            screenId,
            screenLines,
            false
        );
    }
    struct AmmunitionChoice
    {
        std::string id;
        std::string label;
        int count;
        bool special;
    };

    std::string normalizeCombatActionText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool playerUsesChoiceBasedAmmunition(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return false;
        }

        return player.getEquippedWeapon().getType() == WeaponType::Bow;
    }

    std::vector<AmmunitionChoice> buildAmmunitionChoices(const Player& player)
    {
        std::string weaponName = normalizeCombatActionText(player.getEquippedWeapon().getName());

        std::string trainingId = "training_arrows";
        std::string trainingLabel = "Flèches d'entraînement";
        std::string specialId = "barbed_arrows";
        std::string specialLabel = "Flèches barbelées";
        std::string elementalId = "ash_arrows";
        std::string elementalLabel = "Flèches de cendre";
        std::string specialistId = "venom_arrows";
        std::string specialistLabel = "Flèches enduites de venin";

        if (weaponName.find("arbal") != std::string::npos || weaponName.find("carreau") != std::string::npos)
        {
            trainingId = "training_bolts";
            trainingLabel = "Carreaux d'entraînement";
            specialId = "piercing_bolts";
            specialLabel = "Carreaux perforants";
            elementalId = "frozen_bolts";
            elementalLabel = "Carreaux givrés";
            specialistId = "shock_bolts";
            specialistLabel = "Carreaux à pointe conductrice";
        }
        else if (weaponName.find("lancer") != std::string::npos || weaponName.find("couteau") != std::string::npos || weaponName.find("bandouli") != std::string::npos)
        {
            trainingId = "training_throwing_knives";
            trainingLabel = "Couteaux de lancer émoussés";
            specialId = "balanced_throwing_knives";
            specialLabel = "Couteaux de lancer équilibrés";
            elementalId = "conductive_knives";
            elementalLabel = "Couteaux conducteurs";
            specialistId = "smoke_knives";
            specialistLabel = "Couteaux fumigènes";
        }

        std::vector<AmmunitionChoice> choices;
        choices.push_back({trainingId, trainingLabel, player.getInventory().countMaterialById(trainingId), false});
        choices.push_back({specialId, specialLabel, player.getInventory().countMaterialById(specialId), true});
        choices.push_back({elementalId, elementalLabel, player.getInventory().countMaterialById(elementalId), true});
        choices.push_back({specialistId, specialistLabel, player.getInventory().countMaterialById(specialistId), true});
        return choices;
    }

    bool confirmLockedCombatMaterialUse(Player& player, const std::string& materialId, const std::string& label, const std::string& actionLabel)
    {
        const std::string key = Inventory::materialItemKeyById(materialId);
        if (!player.getInventory().isLockedItemKey(key)) return true;
        const int choice = askStructuredCombatChoice(
            "RESSOURCE VERROUILLÉE",
            "combat.inventory_lock.confirm",
            {
                "Ressource : " + label + ".",
                "Action : " + actionLabel + ".",
                "Cette ressource est verrouillée dans l'inventaire : une confirmation supplémentaire est obligatoire."
            },
            {
                {1, "Confirmer exceptionnellement", "Le verrou reste actif après cette utilisation.", true, "confirm_once"},
                {2, "Annuler", "Conserver la ressource verrouillée.", true, "cancel"}
            },
            "Choisis 1 ou 2."
        );
        return choice == 1;
    }

    bool confirmLockedCombatConsumableUse(Player& player, const Consumable& consumable, const std::string& actionLabel)
    {
        const std::string key = Inventory::consumableItemKey(consumable);
        if (!player.getInventory().isLockedItemKey(key)) return true;
        const int choice = askStructuredCombatChoice(
            "OBJET VERROUILLÉ",
            "combat.inventory_lock.consumable_confirm",
            {
                "Objet : " + consumable.getName() + ".",
                "Action : " + actionLabel + ".",
                "Cet objet est verrouillé : cette utilisation exige une confirmation supplémentaire."
            },
            {
                {1, "Confirmer exceptionnellement", "Le verrou restera actif après cette utilisation.", true, "confirm_once"},
                {2, "Annuler", "Conserver l'objet verrouillé.", true, "cancel"}
            },
            "Choisis 1 ou 2."
        );
        return choice == 1;
    }

    int magicCatalystSuccessChance(const Player& player)
    {
        if (!player.hasEquippedWeapon())
        {
            return 70;
        }

        Weapon weapon = player.getEquippedWeapon();
        if (weapon.getType() != WeaponType::Staff)
        {
            return 68;
        }

        if (weapon.isBroken())
        {
            return 42;
        }

        int chance = 94;
        if (!weapon.isIndestructible() && weapon.getMaxDurability() > 0)
        {
            int durabilityPercent = weapon.getDurability() * 100 / weapon.getMaxDurability();
            if (durabilityPercent <= 15) chance -= 26;
            else if (durabilityPercent <= 30) chance -= 16;
            else if (durabilityPercent <= 50) chance -= 7;
        }

        std::string weaponName = normalizeCombatActionText(weapon.getName());
        if (weaponName.find("instable") != std::string::npos)
        {
            chance -= 6;
        }

        if (chance < 35) chance = 35;
        if (chance > 98) chance = 98;
        return chance;
    }

    int applyStatusComboReactions(Entity& attacker, Entity& defender, std::vector<std::string>& notes)
    {
        int bonus = 0;

        if (defender.hasBurning() && defender.hasFrost())
        {
            defender.applyWeakening(2, 10);
            bonus += 2;
            notes.push_back("Réaction de statut : chaleur + givre créent un choc de vapeur qui gêne la cible.");
        }

        if (defender.hasPoison() && defender.hasBleeding())
        {
            defender.applyVulnerability(2, 9);
            bonus += 2;
            notes.push_back("Réaction de statut : poison + saignement rendent la blessure plus exploitable.");
        }

        if (defender.hasShock() && defender.hasVulnerability())
        {
            attacker.applyPrecisionBoost(2, 1);
            bonus += 1;
            notes.push_back("Réaction de statut : choc + vulnérabilité donnent une lecture plus propre du prochain geste.");
        }

        if (defender.hasWeakening() && defender.hasVulnerability())
        {
            bonus += 3;
            notes.push_back("Combo d'ouverture : cible affaiblie et vulnérable, la technique profite vraiment de la préparation.");
        }

        if (defender.hasFrost() && defender.hasShock())
        {
            defender.applyWeakening(1, 8);
            bonus += 1;
            notes.push_back("Réaction de contrôle : givre + choc cassent un peu le rythme adverse.");
        }

        return bonus;
    }

    int baseManaCostForSpell(const std::string& spellName)
    {
        if (spellName == "Marque élémentaire") return 9;
        if (spellName == "Entrave arcanique") return 12;
        if (spellName == "Voile élémentaire") return 13;
        if (spellName == "Surcharge contrôlée") return 19;
        if (spellName == "Faille de résistance") return 18;
        if (spellName == "Aiguille de givre") return 12;
        if (spellName == "Suture de mana") return 15;
        if (spellName == "Ronces occultes") return 14;
        return 12;
    }

    std::string magicTechniqueElementForPlayer(const Player& player)
    {
        const std::string className = normalizeCombatActionText(player.getType());
        if (className.find("pyrom") != std::string::npos || className.find("feu") != std::string::npos) return "feu";
        if (className.find("cryom") != std::string::npos || className.find("glace") != std::string::npos || className.find("givre") != std::string::npos) return "froid";
        if (className.find("electrom") != std::string::npos || className.find("électrom") != std::string::npos || className.find("foudre") != std::string::npos) return "foudre";
        if (className.find("hydrom") != std::string::npos || className.find("eau") != std::string::npos) return "eau";
        if (className.find("géom") != std::string::npos || className.find("geom") != std::string::npos || className.find("terre") != std::string::npos) return "terre";
        if (className.find("aérom") != std::string::npos || className.find("aerom") != std::string::npos || className.find("vent") != std::string::npos) return "vent";
        return "arcanique";
    }

    struct MaterialChannelingSelection
    {
        std::string materialId;
        std::string materialQuality;
        bool useRecipe = false;
        bool sacrificeMaterial = false;
        bool forcePureIncantation = false;
        std::vector<MagicMaterialRules::SpellMaterialSelectedRequirement> recipeSelection;
    };

    struct MaterialMenuChoice
    {
        std::string materialId;
        std::string materialQuality;
        bool sacrificeMaterial = false;
    };

    bool askSpellMaterialRecipeQualities(
        Player& player,
        const std::string& spellName,
        std::vector<MagicMaterialRules::SpellMaterialSelectedRequirement>& selection
    )
    {
        selection.clear();
        const MagicMaterialRules::SpellMaterialRecipe* recipe = MagicMaterialRules::findSpellMaterialRecipe(spellName);
        if (recipe == nullptr)
        {
            return false;
        }

        for (const MagicMaterialRules::SpellMaterialRequirement& requirement : recipe->requirements)
        {
            std::vector<std::string> qualities;
            for (const std::string& quality : MagicMaterialRules::availableQualityVariants(player.getInventory(), requirement.materialId))
            {
                if (player.getInventory().countMaterialByIdAndQuality(requirement.materialId, quality) >= requirement.quantity)
                {
                    qualities.push_back(quality);
                }
            }
            if (qualities.empty())
            {
                selection.clear();
                return false;
            }

            std::string chosenQuality;
            if (qualities.size() == 1)
            {
                chosenQuality = qualities.front();
            }
            else
            {
                std::vector<CombatOptionSpec> options;
                int optionNumber = 1;
                for (const std::string& quality : qualities)
                {
                    const int stock = player.getInventory().countMaterialByIdAndQuality(requirement.materialId, quality);
                    std::string hint = "Stock : x" + std::to_string(stock)
                        + " | " + MagicMaterialRules::formatChannelingProfilePreviewForQuality(requirement.materialId, quality, false);
                    hint += requirement.degradeInsteadOfConsume
                        ? " | cette qualité perdra un palier"
                        : " | cette qualité sera sacrifiée";
                    options.push_back({
                        optionNumber++,
                        MagicMaterialRules::qualityLabel(requirement.materialId, quality),
                        hint,
                        true,
                        "magic_recipe_quality"
                    });
                }
                const int cancelOption = optionNumber;
                options.push_back({
                    cancelOption,
                    "Revenir aux supports",
                    "Annule la préparation de la recette sans rien consommer.",
                    true,
                    "magic_recipe_quality_cancel"
                });

                const int choice = askStructuredCombatChoice(
                    "QUALITÉ D'INGRÉDIENT",
                    "combat.magic.recipe.quality",
                    {
                        "Recette : " + spellName + ".",
                        "Ingrédient : " + MagicMaterialRules::displayName(requirement.materialId)
                            + " x" + std::to_string(requirement.quantity) + ".",
                        requirement.degradeInsteadOfConsume
                            ? "Cet ingrédient sera dégradé, pas détruit immédiatement."
                            : "Cet ingrédient sera réellement sacrifié si le sort démarre."
                    },
                    options,
                    "Choisis une qualité disponible."
                );
                if (choice == cancelOption)
                {
                    selection.clear();
                    return false;
                }
                const int index = choice - 1;
                if (index < 0 || index >= static_cast<int>(qualities.size()))
                {
                    selection.clear();
                    return false;
                }
                chosenQuality = qualities[static_cast<std::size_t>(index)];
            }

            selection.push_back({
                requirement.materialId,
                chosenQuality,
                requirement.quantity,
                requirement.degradeInsteadOfConsume
            });
        }
        return MagicMaterialRules::canConsumeSpellMaterialRecipeSelection(player.getInventory(), selection);
    }

    MaterialChannelingSelection askMaterialChannelingSelection(
        Player& player,
        const std::string& spellName,
        const std::string& elementKey
    )
    {
        const std::vector<std::string> availableIds = MagicMaterialRules::availableMaterialIds(player.getInventory(), elementKey);
        const bool recipeDefined = MagicMaterialRules::findSpellMaterialRecipe(spellName) != nullptr;
        const bool recipeAvailable = MagicMaterialRules::hasSpellMaterialRecipeIngredients(player.getInventory(), spellName);
        const std::vector<std::string> missingRecipeMaterials = MagicMaterialRules::missingSpellMaterialIngredients(player.getInventory(), spellName);
        const int sacrificeUnlockLevel = TechniqueProgressionRules::genericTechniqueUnlockLevel("material_sacrifice");
        const bool sacrificeUnlocked = player.getLevel() >= sacrificeUnlockLevel;

        if (availableIds.empty() && !recipeDefined)
        {
            return {"", "", false, false, true, {}};
        }

        while (true)
        {
            std::vector<std::string> lines = {
                "Sort préparé : " + spellName + ".",
                "Affinité recherchée : " + MagicMaterialRules::elementDisplayName(elementKey) + ".",
                "Qualité et rareté modifient maintenant le coût, la stabilité et la puissance.",
                sacrificeUnlocked
                    ? "Technique : Sacrifice matériel disponible — détruit une unité entière pour un gros burst, mais reste moins rentable."
                    : "Technique : Sacrifice matériel verrouillée jusqu'au niveau " + std::to_string(sacrificeUnlockLevel) + ".",
                "La sélection ci-dessous ne consomme encore rien. Une confirmation séparée sera demandée."
            };
            std::vector<CombatOptionSpec> options;
            std::vector<MaterialMenuChoice> materialChoices;
            int optionNumber = 1;
            int recipeOption = -1;

            if (recipeDefined)
            {
                recipeOption = optionNumber;
                std::string recipeHint = "Plusieurs ingrédients : certains sont sacrifiés, d'autres seulement dégradés.";
                if (recipeAvailable)
                {
                    const MagicMaterialRules::MaterialChannelingProfile profile = MagicMaterialRules::recipeChannelingProfile(player.getInventory(), spellName);
                    recipeHint += " | mana : " + std::to_string(profile.manaPercent) + "%"
                        + " | stabilité : +" + std::to_string(profile.stabilityBonus)
                        + " | puissance : +" + std::to_string(profile.powerBoostPercent) + "%";
                }
                else
                {
                    recipeHint = "Incomplète — manque : ";
                    for (std::size_t i = 0; i < missingRecipeMaterials.size(); ++i)
                    {
                        if (i > 0) recipeHint += ", ";
                        recipeHint += missingRecipeMaterials[i];
                    }
                    recipeHint += ".";
                }
                options.push_back({
                    optionNumber++,
                    recipeAvailable ? "Recette dédiée" : "Recette dédiée (incomplète)",
                    recipeHint,
                    recipeAvailable,
                    "magic_material_recipe"
                });
            }

            const int firstMaterialOption = optionNumber;
            for (const std::string& id : availableIds)
            {
                const bool mineralFuel = MagicMaterialRules::isMineralFuelId(id);
                for (const std::string& quality : MagicMaterialRules::availableQualityVariants(player.getInventory(), id))
                {
                    std::string hint = MagicMaterialRules::formatMaterialUsePreviewForQuality(player.getInventory(), id, quality, mineralFuel);
                    hint += " | " + MagicMaterialRules::formatChannelingProfilePreviewForQuality(id, quality, false);
                    if (!elementKey.empty() && MagicMaterialRules::materialSupportsElement(id, elementKey))
                    {
                        hint += " | accord élémentaire";
                    }
                    else
                    {
                        hint += " | support générique";
                    }
                    options.push_back({
                        optionNumber++,
                        MagicMaterialRules::displayName(id) + " [" + MagicMaterialRules::qualityLabel(id, quality) + "]",
                        hint,
                        true,
                        "magic_material_single_quality"
                    });
                    materialChoices.push_back({id, quality, false});

                    if (mineralFuel && sacrificeUnlocked)
                    {
                        const int qualityCount = player.getInventory().countMaterialByIdAndQuality(id, quality);
                        std::string sacrificeHint = "Stock : x" + std::to_string(qualityCount);
                        sacrificeHint += " | " + MagicMaterialRules::formatChannelingProfilePreviewForQuality(id, quality, true);
                        sacrificeHint += " | toute la qualité restante de cette unité est convertie en puissance immédiate";
                        options.push_back({
                            optionNumber++,
                            "Sacrifice matériel — " + MagicMaterialRules::displayName(id) + " [" + MagicMaterialRules::qualityLabel(id, quality) + "]",
                            sacrificeHint,
                            true,
                            "magic_material_sacrifice"
                        });
                        materialChoices.push_back({id, quality, true});
                    }
                }
            }

            const int pureOption = optionNumber;
            options.push_back({
                pureOption,
                "Forcer sans matériau",
                "Préserve l'inventaire, mais bascule sur l'incantation pure de secours, plus coûteuse et moins stable.",
                true,
                "magic_material_fallback"
            });

            const int choice = askStructuredCombatChoice(
                "SUPPORT DE CANALISATION",
                "combat.magic.material.select",
                lines,
                options,
                "Choisis un support affiché."
            );

            if (choice == pureOption)
            {
                return {"", "", false, false, true, {}};
            }

            const bool selectedRecipe = recipeOption > 0 && choice == recipeOption;
            std::string selectedMaterial;
            std::string selectedMaterialQuality;
            bool selectedSacrifice = false;
            std::vector<MagicMaterialRules::SpellMaterialSelectedRequirement> selectedRecipeMaterials;
            std::vector<std::string> confirmLines;
            if (selectedRecipe)
            {
                if (!askSpellMaterialRecipeQualities(player, spellName, selectedRecipeMaterials))
                {
                    continue;
                }
                confirmLines = MagicMaterialRules::formatSpellMaterialRecipeSelection(selectedRecipeMaterials, spellName);
                const MagicMaterialRules::MaterialChannelingProfile profile = MagicMaterialRules::recipeChannelingProfileForSelection(selectedRecipeMaterials);
                confirmLines.insert(confirmLines.begin(),
                    "Profil : mana " + std::to_string(profile.manaPercent) + "% | stabilité +"
                    + std::to_string(profile.stabilityBonus) + " | puissance +" + std::to_string(profile.powerBoostPercent) + "%.");
                confirmLines.insert(confirmLines.begin(), "Cette recette sera engagée seulement si tu confirmes.");
            }
            else
            {
                const int materialIndex = choice - firstMaterialOption;
                if (materialIndex < 0 || materialIndex >= static_cast<int>(materialChoices.size()))
                {
                    continue;
                }
                const MaterialMenuChoice& selected = materialChoices[static_cast<std::size_t>(materialIndex)];
                selectedMaterial = selected.materialId;
                selectedMaterialQuality = selected.materialQuality;
                selectedSacrifice = selected.sacrificeMaterial;
                const bool mineralFuel = MagicMaterialRules::isMineralFuelId(selectedMaterial);
                confirmLines = {
                    selectedSacrifice
                        ? "Technique : Sacrifice matériel — " + MagicMaterialRules::qualityLabel(selectedMaterial, selectedMaterialQuality) + "."
                        : "Support choisi : " + MagicMaterialRules::displayName(selectedMaterial) + ".",
                    selectedSacrifice
                        ? "Stock de cette qualité : x" + std::to_string(player.getInventory().countMaterialByIdAndQuality(selectedMaterial, selectedMaterialQuality))
                        : MagicMaterialRules::formatMaterialUsePreviewForQuality(player.getInventory(), selectedMaterial, selectedMaterialQuality, mineralFuel),
                    selectedSacrifice
                        ? MagicMaterialRules::formatChannelingProfilePreviewForQuality(selectedMaterial, selectedMaterialQuality, true)
                        : MagicMaterialRules::formatChannelingProfilePreviewForQuality(selectedMaterial, selectedMaterialQuality, false),
                    selectedSacrifice
                        ? "Cette unité précise sera détruite immédiatement après engagement du mana, même si la stabilité échoue ensuite."
                        : (mineralFuel
                            ? "Ce minerai perdra seulement un palier de qualité lors d'une canalisation normale."
                            : "Une unité physique sera réellement consommée si la canalisation démarre.")
                };
            }

            const int confirmation = askStructuredCombatChoice(
                selectedSacrifice ? "CONFIRMER LE SACRIFICE" : "CONFIRMER LE SUPPORT",
                selectedSacrifice ? "combat.magic.material.sacrifice_confirm" : "combat.magic.material.confirm",
                confirmLines,
                {
                    {1, "Confirmer", "Le mana puis les ressources seront engagés au démarrage réel de la canalisation.", true, "confirm"},
                    {2, "Choisir autre chose", "Retourner à la liste sans rien consommer.", true, "change"},
                    {3, "Préserver les matériaux", "Forcer l'incantation pure de secours à la place.", true, "fallback"}
                },
                "Choisis 1, 2 ou 3."
            );

            if (confirmation == 2)
            {
                continue;
            }
            if (confirmation == 3)
            {
                return {"", "", false, false, true, {}};
            }

            if (selectedRecipe)
            {
                bool lockedIngredientRefused = false;
                for (const MagicMaterialRules::SpellMaterialSelectedRequirement& requirement : selectedRecipeMaterials)
                {
                    if (player.getInventory().isLockedItemKey(Inventory::materialItemKeyById(requirement.materialId))
                        && !confirmLockedCombatMaterialUse(player, requirement.materialId, MagicMaterialRules::displayName(requirement.materialId), "engager dans la recette de " + spellName))
                    {
                        lockedIngredientRefused = true;
                        break;
                    }
                }
                if (lockedIngredientRefused)
                {
                    continue;
                }
            }
            else if (!selectedMaterial.empty()
                && !confirmLockedCombatMaterialUse(player, selectedMaterial, MagicMaterialRules::displayName(selectedMaterial), selectedSacrifice ? "sacrifier" : "canaliser"))
            {
                continue;
            }

            if (selectedRecipe)
            {
                MaterialChannelingSelection result;
                result.useRecipe = true;
                result.recipeSelection = selectedRecipeMaterials;
                return result;
            }
            return {selectedMaterial, selectedMaterialQuality, false, selectedSacrifice, false, {}};
        }
    }

    bool resolveMagicCatalystAttempt(
        Player* player,
        const std::string& spellName,
        Random& random,
        int baseManaOverride = -1,
        const std::string& elementKey = "",
        bool interactiveMaterialChoice = true
    )
    {
        if (player == nullptr)
        {
            return true;
        }

        const std::string preferredStyle = player->getMagicChannelingStyleLabel();
        std::string activeStyle = preferredStyle;
        const int baseMana = baseManaOverride > 0 ? baseManaOverride : baseManaCostForSpell(spellName);
        int manaCost = baseMana;
        int successChance = magicCatalystSuccessChance(*player);
        int pendingPowerBoostPercent = 0;
        std::string materialToUse;
        std::string materialQualityToUse;
        bool materialUsesQualityFuel = false;
        bool sacrificeMaterial = false;
        bool useMaterialRecipe = false;
        std::vector<MagicMaterialRules::SpellMaterialSelectedRequirement> selectedRecipeMaterials;
        Weapon* catalystToWear = nullptr;
        std::vector<std::string> lines;
        lines.push_back("Sort : " + spellName);

        if (preferredStyle == "Minerais / matériaux")
        {
            MaterialChannelingSelection selection;
            if (interactiveMaterialChoice)
            {
                const std::string materialPolicy = player->getMagicMaterialSelectionPolicy();
                if (materialPolicy == "auto")
                {
                    const std::vector<std::string> availableIds = MagicMaterialRules::availableMaterialIds(player->getInventory(), elementKey);
                    std::string automaticId;
                    int bestRarity = 999;
                    for (const std::string& id : availableIds)
                    {
                        const bool locked = player->getInventory().isLockedItemKey(Inventory::materialItemKeyById(id));
                        if (!locked)
                        {
                            const int rarity = MagicMaterialRules::materialRarityTier(id);
                            if (automaticId.empty() || rarity < bestRarity)
                            {
                                automaticId = id;
                                bestRarity = rarity;
                            }
                        }
                    }
                    if (!automaticId.empty())
                    {
                        const std::string automaticQuality = MagicMaterialRules::nextMaterialQuality(player->getInventory(), automaticId);
                        if (confirmLockedCombatMaterialUse(*player, automaticId, MagicMaterialRules::displayName(automaticId), "canaliser le sort " + spellName))
                        {
                            selection.materialId = automaticId;
                            selection.materialQuality = automaticQuality;
                        }
                        else
                        {
                            selection = askMaterialChannelingSelection(*player, spellName, elementKey);
                        }
                    }
                    else
                    {
                        selection = askMaterialChannelingSelection(*player, spellName, elementKey);
                    }
                }
                else
                {
                    selection = askMaterialChannelingSelection(*player, spellName, elementKey);
                }
            }
            else
            {
                selection.materialId = MagicMaterialRules::findFirstAvailableMaterialId(player->getInventory(), elementKey);
                selection.materialQuality = selection.materialId.empty() ? "" : MagicMaterialRules::nextMaterialQuality(player->getInventory(), selection.materialId);
                selection.forcePureIncantation = selection.materialId.empty();
            }

            if (selection.forcePureIncantation)
            {
                activeStyle = "Incantation pure (secours)";
                manaCost = std::max(1, baseMana * 125 / 100);
                successChance = std::max(30, successChance - 8);
                lines.push_back("Aucun matériau n'est engagé : la formule est forcée par incantation pure.");
            }
            else if (selection.useRecipe)
            {
                selectedRecipeMaterials = selection.recipeSelection;
                const MagicMaterialRules::MaterialChannelingProfile profile = MagicMaterialRules::recipeChannelingProfileForSelection(selectedRecipeMaterials);
                manaCost = std::max(2, baseMana * profile.manaPercent / 100);
                useMaterialRecipe = true;
                successChance = std::min(97, successChance + profile.stabilityBonus);
                pendingPowerBoostPercent = profile.powerBoostPercent;
                lines.push_back("Voie : minerais / matériaux. La recette physique dédiée est préparée.");
                lines.push_back("Qualité combinée : mana " + std::to_string(profile.manaPercent) + "% | stabilité +"
                    + std::to_string(profile.stabilityBonus) + " | puissance +" + std::to_string(profile.powerBoostPercent) + "%.");
                const std::vector<std::string> recipeLines = MagicMaterialRules::formatSpellMaterialRecipeSelection(selectedRecipeMaterials, spellName);
                lines.insert(lines.end(), recipeLines.begin(), recipeLines.end());
            }
            else if (!selection.materialId.empty())
            {
                materialToUse = selection.materialId;
                materialQualityToUse = selection.materialQuality;
                sacrificeMaterial = selection.sacrificeMaterial;
                materialUsesQualityFuel = MagicMaterialRules::isMineralFuelId(materialToUse);
                const MagicMaterialRules::MaterialChannelingProfile profile = !materialQualityToUse.empty()
                    ? MagicMaterialRules::channelingProfileForQuality(materialToUse, materialQualityToUse, sacrificeMaterial)
                    : MagicMaterialRules::channelingProfile(player->getInventory(), materialToUse, sacrificeMaterial);
                manaCost = std::max(2, baseMana * profile.manaPercent / 100);
                successChance = std::min(97, successChance + profile.stabilityBonus);
                pendingPowerBoostPercent = profile.powerBoostPercent;

                if (sacrificeMaterial)
                {
                    activeStyle = "Sacrifice matériel";
                    lines.push_back("Technique : Sacrifice matériel. Toute la qualité restante d'une unité est convertie en puissance immédiate.");
                    lines.push_back("Le coût en mana reste volontairement bien supérieur à une canalisation matérielle normale.");
                }
                else
                {
                    lines.push_back("Voie : minerais / matériaux. Un support physique réel allège fortement la dépense de mana.");
                }

                lines.push_back("Profil du support : rareté " + MagicMaterialRules::materialRarityLabel(materialToUse)
                    + " | mana " + std::to_string(profile.manaPercent) + "% | stabilité +"
                    + std::to_string(profile.stabilityBonus) + " | puissance +" + std::to_string(profile.powerBoostPercent) + "%.");

                if (!elementKey.empty() && MagicMaterialRules::materialSupportsElement(materialToUse, elementKey))
                {
                    lines.push_back("Accord élémentaire : " + MagicMaterialRules::elementDisplayName(elementKey) + ".");
                }
                if (sacrificeMaterial)
                {
                    lines.push_back("Support sacrifié : " + MagicMaterialRules::displayName(materialToUse)
                        + " [" + MagicMaterialRules::qualityLabel(materialToUse, materialQualityToUse)
                        + "]. L'unité entière sera détruite si la canalisation démarre.");
                }
                else if (materialUsesQualityFuel)
                {
                    lines.push_back("Carburant minéral préparé : " + MagicMaterialRules::displayName(materialToUse) + ". Sa qualité sera dégradée d'un palier au lieu de disparaître immédiatement.");
                }
                else
                {
                    lines.push_back("Composant préparé : " + MagicMaterialRules::displayName(materialToUse) + ".");
                }
            }
            else
            {
                activeStyle = "Incantation pure (secours)";
                manaCost = std::max(1, baseMana * 125 / 100);
                successChance = std::max(30, successChance - 8);
                lines.push_back("Aucun matériau magique adapté : la formule doit être forcée par incantation pure.");
            }
        }
        else if (preferredStyle == "Baguette / catalyseur")
        {
            if (player->hasEquippedWeapon() && player->getEquippedWeapon().getType() == WeaponType::Staff)
            {
                Weapon* catalyst = player->getInventory().getMutableWeapon(player->getEquippedWeaponIndex());
                if (catalyst != nullptr && !catalyst->isBroken())
                {
                    manaCost = std::max(2, baseMana * 75 / 100);
                    successChance = std::min(97, successChance + 5);
                    catalystToWear = catalyst;
                    lines.push_back("Voie : baguette / catalyseur. L'incantation part plus vite et coûte moins de mana.");
                }
                else
                {
                    activeStyle = "Incantation pure (catalyseur brisé)";
                    manaCost = std::max(1, baseMana * 120 / 100);
                    successChance = std::max(25, successChance - 25);
                    lines.push_back("Le catalyseur est brisé : aucune réduction de mana ne s'applique et la formule doit être forcée directement.");
                }
            }
            else
            {
                activeStyle = "Incantation pure (sans catalyseur)";
                manaCost = std::max(1, baseMana * 120 / 100);
                successChance = std::max(30, successChance - 10);
                lines.push_back("Aucun vrai catalyseur équipé : la formule est récitée directement, plus lentement et plus coûteusement.");
            }
        }
        else
        {
            activeStyle = "Incantation pure";
            manaCost = std::max(2, baseMana * 120 / 100);
            successChance = std::min(96, successChance + 2);
            lines.push_back("Voie : incantation pure. Aucun support n'est requis, mais la formule est plus longue et plus gourmande.");
            lines.push_back("La puissance brute est légèrement supérieure si le sort se stabilise.");
        }

        const int manaCursePressure = player->getCursePressureForCategory("mana");
        if (manaCursePressure > 0 && random.between(1, 100) <= 10 + manaCursePressure * 4)
        {
            const int penalty = std::min(18, 4 + manaCursePressure * 2);
            successChance = std::max(25, successChance - penalty);
            lines.push_back(player->getKnownCursePressureForCategory("mana") > 0
                ? "La perturbation de mana déjà soupçonnée rend la canalisation plus capricieuse."
                : "Quelque chose tire dans le flux magique sans se nommer clairement.");
        }

        lines.push_back("Canalisation : " + activeStyle + ".");
        lines.push_back("Coût : " + std::to_string(manaCost) + " mana | Réserve avant sort : "
            + std::to_string(player->getMana()) + "/" + std::to_string(player->getMaxMana()) + ".");

        if (!player->spendMana(manaCost))
        {
            lines.push_back("La réserve est insuffisante : le sort ne peut pas être formé.");
            lines.push_back("Aucun composant ni point de durabilité n'est consommé tant que la canalisation ne démarre pas.");
            showCombatActionMessage("MANA INSUFFISANT", "combat.magic.mana.insufficient", lines, false);
            return false;
        }

        if (useMaterialRecipe && !player->hasInfiniteConsumables())
        {
            std::vector<std::string> recipeResultLines;
            if (!MagicMaterialRules::consumeSpellMaterialRecipeSelection(player->getInventory(), selectedRecipeMaterials, recipeResultLines))
            {
                // The confirmation path checks this before mana is spent. Reaching this case means the
                // inventory changed unexpectedly between selection and channeling, so refund the mana.
                player->restoreMana(manaCost);
                lines.push_back("La recette n'est plus complète au moment de la canalisation : le mana engagé est rendu et rien n'est lancé.");
                showCombatActionMessage("RECETTE INCOMPLÈTE", "combat.magic.material.recipe_missing", lines, false);
                return false;
            }
            lines.insert(lines.end(), recipeResultLines.begin(), recipeResultLines.end());
        }

        if (!materialToUse.empty() && !player->hasInfiniteConsumables())
        {
            if (sacrificeMaterial)
            {
                if (!player->getInventory().removeMaterialQuantityByIdAndQuality(materialToUse, materialQualityToUse, 1))
                {
                    player->restoreMana(manaCost);
                    lines.push_back("Le matériau de la qualité choisie n'est plus disponible : le mana engagé est rendu.");
                    showCombatActionMessage("SACRIFICE ANNULÉ", "combat.magic.material.sacrifice_missing", lines, false);
                    return false;
                }
                lines.push_back(
                    "Sacrifice matériel : " + MagicMaterialRules::displayName(materialToUse)
                    + " [" + MagicMaterialRules::qualityLabel(materialToUse, materialQualityToUse)
                    + "] est entièrement dissous en une seule poussée."
                );
            }
            else if (materialUsesQualityFuel)
            {
                std::string previousQuality;
                std::string resultingQuality;
                bool dissolved = false;
                const bool degraded = !materialQualityToUse.empty()
                    ? player->getInventory().degradeMaterialQualityByIdAndQuality(materialToUse, materialQualityToUse, previousQuality, resultingQuality, dissolved)
                    : player->getInventory().degradeMaterialQualityById(materialToUse, previousQuality, resultingQuality, dissolved);
                if (degraded)
                {
                    if (dissolved)
                    {
                        lines.push_back(
                            MagicMaterialRules::displayName(materialToUse)
                            + " était déjà à sa qualité minimale : le minerai se dissout complètement dans la canalisation."
                        );
                    }
                    else
                    {
                        lines.push_back(
                            MagicMaterialRules::displayName(materialToUse)
                            + " perd un palier de qualité : "
                            + MagicMaterialRules::qualityLabel(materialToUse, previousQuality)
                            + " -> "
                            + MagicMaterialRules::qualityLabel(materialToUse, resultingQuality)
                            + "."
                        );
                    }
                }
            }
            else
            {
                const bool consumed = !materialQualityToUse.empty()
                    ? player->getInventory().removeMaterialQuantityByIdAndQuality(materialToUse, materialQualityToUse, 1)
                    : player->getInventory().removeMaterialQuantityById(materialToUse, 1);
                if (consumed)
                {
                    lines.push_back("Le composant de qualité " + MagicMaterialRules::qualityLabel(materialToUse, materialQualityToUse) + " est consommé par la canalisation.");
                }
            }
        }

        if (catalystToWear != nullptr
            && !player->hasIndestructibleEquipment()
            && !catalystToWear->isIndestructible())
        {
            catalystToWear->loseDurability(1);
            lines.push_back("Le catalyseur perd 1 point de durabilité.");
        }

        lines.push_back("Stabilité : " + std::to_string(successChance) + "%.");
        const bool success = random.between(1, 100) <= successChance;
        lines.push_back(success ? "Le flux se stabilise." : "Le flux se casse avant de former le sort.");

        if (success)
        {
            if (activeStyle.find("Incantation pure") != std::string::npos)
            {
                player->applyPowerBoost(1, 10);
            }
            else if (pendingPowerBoostPercent > 0)
            {
                player->applyPowerBoost(1, pendingPowerBoostPercent);
                lines.push_back("Puissance canalisée : prochain impact magique +" + std::to_string(pendingPowerBoostPercent) + "%.");
            }
        }

        if (player->getMana() == 0)
        {
            const int faintRoll = random.between(1, 100);
            if (faintRoll <= 75)
            {
                const int faintTurns = random.between(1, 100) <= 22 ? 2 : 1;
                player->applyManaExhaustion(faintTurns);
                lines.push_back("La dernière goutte de mana disparaît. Les jambes lâchent : malaise pendant " + std::to_string(faintTurns) + " tour(s).");
            }
            else
            {
                lines.push_back("La réserve tombe exactement à zéro. Le corps vacille, mais tient encore debout cette fois.");
            }
        }

        showCombatActionMessage(
            success ? "CANALISATION RÉUSSIE" : "CANALISATION INSTABLE",
            success ? "combat.magic.channel.success" : "combat.magic.channel.failure",
            lines,
            false
        );

        return success;
    }

    void askAmmunitionChoiceIfNeeded(Entity& attacker)
    {
        Player* player = dynamic_cast<Player*>(&attacker);
        if (player == nullptr || !playerUsesChoiceBasedAmmunition(*player))
        {
            return;
        }

        std::vector<AmmunitionChoice> choices = buildAmmunitionChoices(*player);

        bool hasAvailableAmmunition = false;
        for (const AmmunitionChoice& choice : choices)
        {
            if (choice.count > 0)
            {
                hasAvailableAmmunition = true;
                break;
            }
        }

        if (!hasAvailableAmmunition)
        {
            int emergencyChoice = askStructuredCombatChoice(
                "CHOIX DES MUNITIONS",
                "combat.ammunition.empty",
                {
                    "Arme : " + player->getEquippedWeapon().getName(),
                    "Aucune munition compatible n'est disponible.",
                    "Tu peux annuler ou garder l'arme en défense d'urgence."
                },
                {
                    {1, "Utiliser l'arme en défense d'urgence", "Pas de tir magique : seulement une défense de proximité.", true, "emergency_defense"},
                    {0, "Annuler", "Ne dépense aucune munition et annule l'attaque.", true, "cancel"}
                },
                "Choix invalide. Choisis 0 ou 1."
            );

            if (emergencyChoice == 0)
            {
                player->setNextAmmunitionChoice("__cancel_attack__");
                showCombatActionMessage(
                    "TIR ANNULÉ",
                    "combat.ammunition.cancelled",
                    {"Tu baisses ton arme à distance.", "Aucune munition n'est dépensée."},
                    false
                );
                return;
            }

            player->setNextAmmunitionChoice("__emergency_defense__");
            showCombatActionMessage(
                "DÉFENSE D'URGENCE",
                "combat.ammunition.emergency_defense",
                {"Aucune munition compatible.", "L'arme reste prête pour un coup de crosse, de branche ou de garde si l'ennemi se rapproche."},
                false
            );
            return;
        }

        const std::string ammunitionPolicy = player->getAmmunitionSelectionPolicy();
        if (ammunitionPolicy == "favorite" || ammunitionPolicy == "auto")
        {
            int automaticIndex = -1;
            for (std::size_t i = 0; i < choices.size(); ++i)
            {
                const AmmunitionChoice& candidate = choices[i];
                if (candidate.count <= 0) continue;
                const bool favorite = player->getInventory().isFavoriteAmmunitionId(candidate.id);
                const bool locked = player->getInventory().isLockedItemKey(Inventory::materialItemKeyById(candidate.id));
                if (ammunitionPolicy == "favorite")
                {
                    if (favorite) { automaticIndex = static_cast<int>(i); break; }
                }
                else if (!locked)
                {
                    // Choices are ordered with the basic/standard ammunition first.
                    automaticIndex = static_cast<int>(i);
                    break;
                }
            }

            if (automaticIndex >= 0)
            {
                const AmmunitionChoice& selected = choices[static_cast<std::size_t>(automaticIndex)];
                if (confirmLockedCombatMaterialUse(*player, selected.id, selected.label, "utiliser comme munition"))
                {
                    player->setNextAmmunitionChoice(selected.id);
                    showCombatActionMessage(
                        ammunitionPolicy == "favorite" ? "MUNITION FAVORITE" : "MUNITION AUTOMATIQUE",
                        ammunitionPolicy == "favorite" ? "combat.ammunition.favorite" : "combat.ammunition.auto",
                        {
                            "Munition : " + selected.label,
                            "Quantité en réserve : " + std::to_string(selected.count),
                            ammunitionPolicy == "favorite"
                                ? "Préférence : favori disponible. Tu peux remettre Toujours demander dans l'inventaire."
                                : "Préférence : automatique prudent. Les munitions standard sont prioritaires."
                        },
                        false
                    );
                    return;
                }
            }
            // No valid favourite, or a protected automatic choice was refused: fall back to explicit asking.
        }

        std::vector<CombatOptionSpec> optionSpecs;
        optionSpecs.push_back({0, "Annuler", "Ne dépense aucune munition.", true, "cancel"});
        for (std::size_t i = 0; i < choices.size(); ++i)
        {
            const AmmunitionChoice& choice = choices[i];
            const std::string choiceKey = Inventory::materialItemKeyById(choice.id);
            const bool favorite = player->getInventory().isFavoriteAmmunitionId(choice.id);
            const bool locked = player->getInventory().isLockedItemKey(choiceKey);
            std::string label = (favorite ? "[FAVORI] " : "") + std::string(locked ? "[VERROUILLÉ] " : "") + choice.label + " x" + std::to_string(choice.count);
            std::string hint = choice.special ? "Munition spéciale." : "Munition standard.";
            if (favorite) hint += " Favori.";
            if (locked) hint += " Confirmation supplémentaire obligatoire.";
            if (choice.count <= 0)
            {
                hint += " Indisponible.";
            }
            optionSpecs.push_back({static_cast<int>(i + 1), label, hint, choice.count > 0, choice.id});
        }

        int choice = askStructuredCombatChoice(
            "CHOIX DES MUNITIONS",
            "combat.ammunition.choice",
            {
                "Arme : " + player->getEquippedWeapon().getName(),
                "Chaque tir consomme une munition.",
                "Les munitions indisponibles restent visibles mais verrouillées."
            },
            optionSpecs,
            "Choix invalide. Choisis une munition affichée."
        );

        if (choice == 0)
        {
            player->setNextAmmunitionChoice("__cancel_attack__");
            showCombatActionMessage(
                "TIR ANNULÉ",
                "combat.ammunition.cancelled",
                {"Tir annulé.", "Aucune munition n'est dépensée."},
                false
            );
            return;
        }

        const AmmunitionChoice& selected = choices[choice - 1];
        if (selected.count <= 0)
        {
            player->setNextAmmunitionChoice("__cancel_attack__");
            showCombatActionMessage(
                "MUNITION INDISPONIBLE",
                "combat.ammunition.unavailable",
                {"Cette munition n'est plus disponible.", "Le tir est annulé pour éviter de gaspiller ton tour."},
                false
            );
            return;
        }

        if (!confirmLockedCombatMaterialUse(*player, selected.id, selected.label, "utiliser comme munition"))
        {
            player->setNextAmmunitionChoice("__cancel_attack__");
            showCombatActionMessage("TIR ANNULÉ", "combat.ammunition.lock_cancelled", {"La munition verrouillée est conservée.", "Le tir est annulé."}, false);
            return;
        }

        player->setNextAmmunitionChoice(selected.id);
        showCombatActionMessage(
            "MUNITION CHOISIE",
            "combat.ammunition.selected",
            {
                "Munition : " + selected.label,
                "Quantité en réserve : " + std::to_string(selected.count),
                selected.special ? "Type : spéciale." : "Type : standard."
            },
            false
        );
    }
}

void CombatActions::executeAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    executeBoostedAttack(
        attacker,
        defender,
        random,
        0
    );
}

void CombatActions::executeBoostedAttack(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int damageBonus
)
{
    if (consumePendingDivineExecution(attacker, defender))
    {
        return;
    }

    askAmmunitionChoiceIfNeeded(attacker);

    CombatAttack::executeBoostedAttack(
        attacker,
        defender,
        random,
        damageBonus
    );
}


bool CombatActions::prepareMagicTechnique(
    Player& player,
    const std::string& techniqueName,
    Random& random,
    int baseManaCost
)
{
    return resolveMagicCatalystAttempt(
        &player,
        techniqueName,
        random,
        std::max(1, baseManaCost),
        "",
        false
    );
}


bool CombatActions::prepareMagicTechniqueInteractive(
    Player& player,
    const std::string& techniqueName,
    Random& random,
    int baseManaCost
)
{
    return resolveMagicCatalystAttempt(
        &player,
        techniqueName,
        random,
        std::max(1, baseManaCost),
        magicTechniqueElementForPlayer(player),
        true
    );
}


bool CombatActions::hasCompatibleAmmunition(const Player& player)
{
    if (!playerUsesChoiceBasedAmmunition(player))
    {
        return true;
    }

    const std::vector<AmmunitionChoice> choices = buildAmmunitionChoices(player);
    return std::any_of(choices.begin(), choices.end(), [](const AmmunitionChoice& choice) {
        return choice.count > 0;
    });
}


void CombatActions::executeWeaponTechnique(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    int bonus = 5;
    std::string techniqueName = "Technique improvisée";

    Player* player = dynamic_cast<Player*>(&attacker);
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());

    std::vector<std::string> techniqueNotes;

    if (player != nullptr && player->hasEquippedWeapon())
    {
        WeaponType weaponType = player->getEquippedWeapon().getType();

        if (weaponType == WeaponType::Dagger)
        {
            bonus = 7;
            techniqueName = "Entaille vive";
        }
        else if (weaponType == WeaponType::Spear)
        {
            bonus = 8;
            techniqueName = "Poussée de contrôle";
        }
        else if (weaponType == WeaponType::Bow)
        {
            bonus = 9;
            techniqueName = "Tir préparé";
        }
        else if (weaponType == WeaponType::Staff)
        {
            bonus = 8;
            techniqueName = "Canalisation prudente";
        }
        else if (weaponType == WeaponType::Axe || weaponType == WeaponType::Hammer)
        {
            bonus = 11;
            techniqueName = "Coup brise-garde";
        }
        else if (weaponType == WeaponType::Sword)
        {
            bonus = 7;
            techniqueName = "Coupe maîtrisée";
        }
        else if (weaponType == WeaponType::BareHands)
        {
            bonus = 3;
            techniqueName = "Frappe réflexe";
        }

        if (weaponType == WeaponType::Dagger && player->hasActiveSkill("chain_execution"))
        {
            bonus += 4;
            techniqueName += " / enchaînement appris";
            ElementalAffinitySystem::applyBleeding(defender, 1, 1 + std::max(1, player->getLevel() / 35));
            techniqueNotes.push_back("Le geste à la dague se prolonge en entaille courte.");
        }
        else if (weaponType == WeaponType::Staff && player->hasActiveSkill("cautious_channeling"))
        {
            bonus += 3;
            attacker.startDefensePosture(8, 4, "Canalisation prudente");
            techniqueName += " / canalisation stable";
            techniqueNotes.push_back("Le bâton canalise sans exposer complètement son porteur.");
        }
        else if (weaponType == WeaponType::BareHands && player->hasActiveSkill("reflex_counter"))
        {
            bonus += 2;
            attacker.startDefensePosture(10, 12, "Contre réflexe");
            techniqueName += " / contre prêt";
            techniqueNotes.push_back("Le corps garde assez d'élan pour répondre si l'ennemi rate.");
        }
        else if (weaponType == WeaponType::Axe && player->hasActiveSkill("splitting_blow"))
        {
            bonus += 5;
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + std::max(1, player->getLevel() / 30));
            techniqueName += " / frappe fendue";
            techniqueNotes.push_back("La hache ouvre une trajectoire large qui mord après l'impact.");
        }
        else if (weaponType == WeaponType::Hammer && player->hasActiveSkill("armor_crack"))
        {
            bonus += 5;
            ElementalAffinitySystem::applyShock(defender, 1);
            techniqueName += " / fracasse-garde";
            techniqueNotes.push_back("Le choc du marteau casse le rythme défensif adverse.");
        }

        if (weaponType == WeaponType::Sword && player->hasPassiveSkill("blade_discipline"))
        {
            bonus += 3;
            techniqueName += " / discipline de lame";
            techniqueNotes.push_back("La lame reste dans un axe propre : le coup perd moins de force.");
        }
        else if (weaponType == WeaponType::Spear && player->hasPassiveSkill("reach_control"))
        {
            bonus += 2;
            attacker.startDefensePosture(6, 5, "Contrôle d'allonge");
            techniqueName += " / allonge tenue";
            techniqueNotes.push_back("La lance garde une distance sûre après la poussée.");
        }
    }

    if (className.find("assassin") != std::string::npos || className.find("ombrelame") != std::string::npos)
    {
        bonus += 6;
        techniqueName += " / angle mort";
    }
    else if (className.find("lancier") != std::string::npos)
    {
        bonus += 4;
        techniqueName += " / distance royale";
    }
    else if (className.find("archer") != std::string::npos || className.find("rôdeur") != std::string::npos || className.find("rodeur") != std::string::npos || className.find("arbal") != std::string::npos || className.find("tireur") != std::string::npos)
    {
        bonus += 5;
        techniqueName += " / visée de classe";
    }
    else if (className.find("mage") != std::string::npos || className.find("sorcier") != std::string::npos || className.find("arcaniste") != std::string::npos)
    {
        bonus += 7;
        techniqueName += " / surcharge arcanique";
    }
    else if (className.find("gardien") != std::string::npos || className.find("tank") != std::string::npos || className.find("colosse") != std::string::npos)
    {
        bonus -= 2;
        techniqueName += " / garde lourde";
        attacker.startDefensePosture(20, 5, "Technique de garde lourde");
    }
    else if (className.find("clerc") != std::string::npos || className.find("prêtre") != std::string::npos || className.find("pretre") != std::string::npos || className.find("paladin") != std::string::npos)
    {
        bonus += 2;
        techniqueName += " / frappe protectrice";
    }
    else if (className.find("alchim") != std::string::npos || className.find("artific") != std::string::npos)
    {
        bonus += 4;
        techniqueName += " / dosage instable";
    }
    else if (className.find("nécro") != std::string::npos || className.find("necro") != std::string::npos)
    {
        bonus += 3;
        techniqueName += " / dette des morts";
    }
    else if (className.find("invoc") != std::string::npos || className.find("dresseur") != std::string::npos)
    {
        bonus += 1;
        techniqueName += " / ordre de soutien";
        attacker.startDefensePosture(8, 3, "Ordre de soutien");
    }
    else if (className.find("duelliste") != std::string::npos || className.find("moine") != std::string::npos)
    {
        bonus += 4;
        techniqueName += " / lecture du rythme";
    }

    techniqueNotes.insert(techniqueNotes.begin(), "La technique dépend de l'arme équipée et de la classe : certaines formations imposent naturellement leur propre rythme.");

    const int comboBonus = applyStatusComboReactions(attacker, defender, techniqueNotes);
    if (comboBonus > 0)
    {
        bonus += comboBonus;
    }

    if (className.find("assassin") != std::string::npos || className.find("ombrelame") != std::string::npos)
    {
        ElementalAffinitySystem::applyBleeding(defender, 1, 2 + std::max(1, attacker.getMaxDamage() / 18));
        techniqueNotes.push_back("L'angle mort prépare un petit saignement si la cible survit.");
    }
    else if (className.find("mage") != std::string::npos || className.find("sorcier") != std::string::npos || className.find("arcaniste") != std::string::npos)
    {
        int elementalRoll = random.between(1, 3);
        if (elementalRoll == 1)
        {
            ElementalAffinitySystem::applyBurning(defender, 1, 2 + std::max(1, attacker.getMaxDamage() / 20));
            techniqueNotes.push_back("La surcharge laisse une brûlure arcanique faible.");
        }
        else if (elementalRoll == 2)
        {
            ElementalAffinitySystem::applyFrost(defender, 2);
            techniqueNotes.push_back("La surcharge refroidit la cible et prépare un ralentissement.");
        }
        else
        {
            ElementalAffinitySystem::applyShock(defender, 2);
            techniqueNotes.push_back("La surcharge perturbe la cible avec une décharge instable.");
        }
    }
    else if (className.find("clerc") != std::string::npos || className.find("prêtre") != std::string::npos || className.find("pretre") != std::string::npos || className.find("paladin") != std::string::npos)
    {
        int selfHeal = std::max(2, attacker.getMaxHp() / 25);
        attacker.heal(selfHeal);
        techniqueNotes.push_back("La frappe protectrice rend " + std::to_string(selfHeal) + " PV au lanceur.");
    }
    else if (className.find("alchim") != std::string::npos || className.find("artific") != std::string::npos)
    {
        int effectRoll = random.between(1, 3);
        if (effectRoll == 1)
        {
            ElementalAffinitySystem::applyPoison(defender, 2, 2 + std::max(1, attacker.getMaxDamage() / 18));
            techniqueNotes.push_back("Le dosage instable accroche un poison faible.");
        }
        else if (effectRoll == 2)
        {
            ElementalAffinitySystem::applyShock(defender, 1);
            techniqueNotes.push_back("Le mécanisme bricolé produit une petite décharge.");
        }
        else
        {
            attacker.startDefensePosture(10, 4, "Couverture d'artificier");
            techniqueNotes.push_back("Le bricolage crée une petite couverture défensive.");
        }
    }
    else if (className.find("nécro") != std::string::npos || className.find("necro") != std::string::npos)
    {
        ElementalAffinitySystem::applyBleeding(defender, 2, 2 + std::max(1, attacker.getMaxDamage() / 20));
        techniqueNotes.push_back("La dette des morts laisse une trace sombre, proche d'un saignement.");
    }
    else if (className.find("duelliste") != std::string::npos || className.find("moine") != std::string::npos)
    {
        attacker.startDefensePosture(8, 10, "Lecture du rythme");
        techniqueNotes.push_back("Le rythme lu prépare une petite fenêtre de contre.");
    }
    else if (className.find("lancier") != std::string::npos)
    {
        attacker.startDefensePosture(12, 4, "Allonge de lancier");
        techniqueNotes.push_back("Le lancier garde l'ennemi à distance après sa technique.");
    }

    showCombatActionMessage(
        "TECHNIQUE D'ARME",
        "combat.weapon_technique",
        [&]() {
            std::vector<std::string> lines;
            lines.push_back(attacker.getName() + " utilise : " + techniqueName + ".");
            lines.insert(lines.end(), techniqueNotes.begin(), techniqueNotes.end());
            lines.push_back("Bonus de technique : " + std::to_string(bonus) + ".");
            return lines;
        }(),
        false
    );

    executeBoostedAttack(attacker, defender, random, bonus);
}

void CombatActions::executeHeavyAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    int bonus = 10;

    if (className.find("berserker") != std::string::npos || className.find("barbare") != std::string::npos || className.find("briseur") != std::string::npos) bonus += 8;
    if (className.find("colosse") != std::string::npos || className.find("gardien") != std::string::npos) bonus += 4;
    if (className.find("assassin") != std::string::npos || className.find("archer") != std::string::npos) bonus -= 3;

    showCombatActionMessage(
        "ATTAQUE LOURDE",
        "combat.attack.heavy",
        {
            attacker.getName() + " prépare une attaque lourde.",
            "Le coup demande de l'engagement : les profils lourds et berserkers y trouvent plus facilement leur force.",
            "Bonus d'attaque : " + std::to_string(bonus) + "."
        },
        false
    );

    executeBoostedAttack(attacker, defender, random, bonus);
}

void CombatActions::executeQuickAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    int bonus = -3;
    int chainChance = 55;

    if (className.find("assassin") != std::string::npos || className.find("duelliste") != std::string::npos || className.find("moine") != std::string::npos)
    {
        bonus += 3;
        chainChance += 20;
    }

    Player* trainedPlayer = dynamic_cast<Player*>(&attacker);
    if (trainedPlayer != nullptr && trainedPlayer->hasPassiveSkill("footwork_drill"))
    {
        bonus += 1;
        chainChance += 8;
        attacker.startDefensePosture(5, 5, "Duel d'appuis");
    }
    if (className.find("colosse") != std::string::npos || className.find("tank") != std::string::npos || className.find("briseur") != std::string::npos)
    {
        bonus -= 3;
        chainChance -= 18;
    }

    showCombatActionMessage(
        "ATTAQUE RAPIDE",
        "combat.attack.quick",
        {
            attacker.getName() + " tente une attaque rapide.",
            "Les classes agiles enchaînent plus naturellement, les classes lourdes moins.",
            "Chance d'enchaînement : " + std::to_string(chainChance) + "%.",
            "Bonus d'attaque : " + std::to_string(bonus) + "."
        },
        false
    );

    executeBoostedAttack(attacker, defender, random, bonus);

    if (!defender.isDead() && random.between(1, 100) <= chainChance)
    {
        showCombatActionMessage(
            "ENCHAÎNEMENT RAPIDE",
            "combat.attack.quick.chain",
            {"L'ouverture reste présente : second geste rapide !"},
            false
        );
        executeBoostedAttack(attacker, defender, random, -5);
    }
}


bool CombatActions::executeClassSkill(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    Player* player = dynamic_cast<Player*>(&attacker);
    int level = player != nullptr ? player->getLevel() : 1;

    auto hasAny = [&](std::initializer_list<const char*> terms) {
        for (const char* term : terms)
        {
            if (className.find(term) != std::string::npos) return true;
        }
        return false;
    };

    auto hasRangedWeapon = [&]() {
        if (player == nullptr || !player->hasEquippedWeapon()) return false;
        std::string weaponName = normalizeCombatActionText(player->getEquippedWeapon().getName());
        return player->getEquippedWeapon().getType() == WeaponType::Bow
            || weaponName.find("arbal") != std::string::npos
            || weaponName.find("lancer") != std::string::npos
            || weaponName.find("bandouli") != std::string::npos;
    };

    auto startCooldown = [&](int turns) {
        attacker.startClassSkillCooldown(turns);
        showCooldownApplied(attacker, turns);
    };

    if (!attacker.isClassSkillReady())
    {
        showCombatActionMessage(
            "COMPÉTENCE INDISPONIBLE",
            "combat.class_skill.cooldown_locked",
            {
                "La compétence de classe n'est pas encore revenue.",
                "Récupération restante : " + std::to_string(attacker.getClassSkillCooldownTurns()) + " tour(s)."
            },
            false
        );
        return false;
    }

    if (player != nullptr && player->hasActiveSkill("combat_observation"))
    {
        int observationChoice = askStructuredCombatChoice(
            "COMPÉTENCE GÉNÉRALE",
            "combat.class_skill.general_selector",
            {
                "Tu connais Observation tactique.",
                "Elle utilise la même récupération que les compétences de classe : c'est un vrai choix de tour, pas un bonus gratuit."
            },
            {
                {1, "Observation tactique", "Lire la cible, créer une faille courte, puis garder une petite posture.", true, "combat_observation"},
                {2, "Techniques de classe", "Ouvrir les compétences propres à la classe actuelle.", true, "class_skills"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );

        if (observationChoice == 0)
        {
            return false;
        }

        if (observationChoice == 1)
        {
            std::vector<std::string> observationLines;
            observationLines.push_back(attacker.getName() + " observe " + defender.getName() + " au lieu de se jeter directement sur les PV.");
            observationLines.push_back("PV observés : " + std::to_string(defender.getHp()) + "/" + std::to_string(defender.getMaxHp()) + ".");
            observationLines.push_back("Type aperçu : " + defender.getType() + ".");

            std::vector<std::string> statusHints;
            if (defender.hasBurning()) statusHints.push_back("brûlure");
            if (defender.hasPoison()) statusHints.push_back("poison");
            if (defender.hasFrost()) statusHints.push_back("givre");
            if (defender.hasShock()) statusHints.push_back("choc");
            if (defender.hasBleeding()) statusHints.push_back("saignement");
            if (defender.hasWeakening()) statusHints.push_back("affaiblissement");
            if (defender.hasVulnerability()) statusHints.push_back("vulnérabilité");
            if (defender.isInDefensePosture()) statusHints.push_back("posture défensive");

            if (statusHints.empty())
            {
                observationLines.push_back("Aucun statut évident : l'observation force une première faille légère.");
            }
            else
            {
                std::string joined;
                for (std::size_t i = 0; i < statusHints.size(); ++i)
                {
                    if (i > 0) joined += ", ";
                    joined += statusHints[i];
                }
                observationLines.push_back("État visible : " + joined + ".");
                observationLines.push_back("Les prochains coups peuvent exploiter cette information, surtout avec les réactions de statuts.");
            }

            const bool trainedFieldReader = player->hasPassiveSkill("field_observer");
            defender.applyVulnerability(trainedFieldReader ? 3 : 2, 10 + level / 10 + (trainedFieldReader ? 4 : 0));
            if (trainedFieldReader)
            {
                observationLines.push_back("Lecture de terrain : tes observations hors combat rendent la faille plus stable.");
            }
            if (defender.hasWeakening() || defender.hasFrost() || defender.hasShock())
            {
                attacker.startDefensePosture(trainedFieldReader ? 15 : 12, trainedFieldReader ? 10 : 8, "Lecture tactique");
                observationLines.push_back("La cible a déjà un rythme troublé : tu gardes aussi une petite posture de lecture.");
            }
            else
            {
                defender.applyWeakening(trainedFieldReader ? 2 : 1, 6 + level / 14 + (trainedFieldReader ? 2 : 0));
                observationLines.push_back("La lecture impose une gêne courte : moins spectaculaire qu'un gros coup, mais utile pour préparer un combo.");
            }

            showClassSkillResult(
                "Observation tactique",
                "combat.class_skill.general.observation.result",
                observationLines
            );
            startCooldown(3);
            return true;
        }
    }

    if (hasAny({"voleur", "roublard", "brigand"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("opportunist_hit"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE VOLEUR",
            "combat.class_skill.thief",
            {attacker.getName() + " cherche une ouverture sale, pratique ou simplement rentable."},
            {
                {1, "Coup opportuniste", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("opportunist_hit")) + ", recharge 3 tours. Vulnérabilité courte.", true, "opportunist_hit"},
                {2, "Poudre d'échappée", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("escape_powder")) + ", recharge 4 tours. Défense + affaiblissement.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("escape_powder"), "escape_powder"},
                {3, "Frappe aux poches", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("pocket_strike")) + ", recharge 5 tours. Blessure + gêne durable.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("pocket_strike"), "pocket_strike"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;

        if (choice == 3)
        {
            showClassSkillResult(
                "Frappe aux poches",
                "combat.class_skill.thief.pocket_strike.result",
                {"Le coup ne vole pas encore d'objet réel : il vise les attaches, la concentration et les réflexes de la cible."}
            );
            defender.applyWeakening(3, 12 + level / 10);
            defender.applyVulnerability(2, 10 + level / 14);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 18);
            executeBoostedAttack(attacker, defender, random, 3 + level / 22);
            startCooldown(5);
            return true;
        }

        if (choice == 2)
        {
            showClassSkillResult(
                "Poudre d'échappée",
                "combat.class_skill.thief.escape_powder.result",
                {"Le voleur ne cherche pas seulement à taper : il brouille la lecture du tour et prépare une sortie."}
            );
            attacker.startDefensePosture(18, 16, "Poudre d'échappée");
            defender.applyWeakening(2, 10 + level / 12);
            if (random.between(1, 100) <= 45)
            {
                ElementalAffinitySystem::applyPoison(defender, 2, 2 + level / 24);
            }
            executeBoostedAttack(attacker, defender, random, 1 + level / 30);
            startCooldown(4);
            return true;
        }

        showClassSkillResult(
            "Coup opportuniste",
            "combat.class_skill.thief.opportunist_hit.result",
            {"Le voleur attaque l'ouverture, pas les PV : la cible devient plus fragile pendant un court instant."}
        );
        defender.applyVulnerability(2, 9 + level / 12);
        if (random.between(1, 100) <= 35)
        {
            ElementalAffinitySystem::applyBleeding(defender, 1, 2 + level / 24);
        }
        executeBoostedAttack(attacker, defender, random, 2 + level / 24);
        startCooldown(3);
        return true;
    }

    if (hasAny({"assassin", "ombrelame", "lanceur de dagues"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("shadow_step"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE ASSASSIN",
            "combat.class_skill.assassin",
            {attacker.getName() + " cherche une ouverture d'assassin."},
            {
                {1, "Pas dans l'angle mort", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("shadow_step")) + ", recharge 3 tours. Saignement préparé.", true, "shadow_step"},
                {2, "Disparition basse", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("low_disappearance")) + ", recharge 4 tours. Défense + blessure longue.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("low_disappearance"), "low_disappearance"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        if (choice == 2)
        {
            showClassSkillResult(
                "Disparition basse",
                "combat.class_skill.assassin.low_disappearance.result",
                {"Tu recules dans un angle mort, puis tu frappes en laissant une blessure plus longue."}
            );
            attacker.startDefensePosture(14, 18, "Disparition basse");
            ElementalAffinitySystem::applyBleeding(defender, 4, 3 + std::max(1, level / 14));
            executeBoostedAttack(attacker, defender, random, 4 + level / 18);
            startCooldown(4);
            return true;
        }

        showClassSkillResult(
            "Pas dans l'angle mort",
            "combat.class_skill.assassin.shadow_step.result",
            {"Tu sacrifies la puissance brute pour préparer une blessure qui continue de travailler."}
        );
        ElementalAffinitySystem::applyBleeding(defender, 3, 2 + std::max(1, level / 18));
        executeBoostedAttack(attacker, defender, random, 2 + level / 20);
        startCooldown(3);
        return true;
    }

    if (hasAny({"gardien", "tank", "colosse"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("taunting_wall"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE DÉFENSIVE",
            "combat.class_skill.defense",
            {attacker.getName() + " choisit une posture de protection."},
            {
                {1, "Rempart provoquant", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("taunting_wall")) + ", recharge 3 tours. Défense et menace.", true, "taunting_wall"},
                {2, "Ancrage total", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("total_anchor")) + ", recharge 4 tours. Défense très forte.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("total_anchor"), "total_anchor"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        if (choice == 2)
        {
            showClassSkillResult(
                "Ancrage total",
                "combat.class_skill.defense.total_anchor.result",
                {"Tu abandonnes presque l'offense pour forcer l'ennemi à te gérer."}
            );
            attacker.startDefensePosture(35, 12, "Ancrage total");
            executeBoostedAttack(attacker, defender, random, -5);
            startCooldown(4);
            return true;
        }

        showClassSkillResult(
            "Rempart provoquant",
            "combat.class_skill.defense.taunting_wall.result",
            {"Tu prends de la place, tu frappes moins fort, mais tu rends ta présence impossible à ignorer."}
        );
        attacker.startDefensePosture(22, 7, "Rempart provoquant");
        executeBoostedAttack(attacker, defender, random, -2);
        startCooldown(3);
        return true;
    }

    if (hasAny({"clerc", "prêtre", "pretre", "paladin"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("survival_oath"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE SACRÉE",
            "combat.class_skill.sacred",
            {attacker.getName() + " rassemble une énergie protectrice."},
            {
                {1, "Serment de survie", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("survival_oath")) + ", recharge 3 tours. Soin personnel + garde légère.", true, "survival_oath"},
                {2, "Prière purificatrice", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("purifying_prayer")) + ", recharge 5 tours. Soin + purification.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("purifying_prayer"), "purifying_prayer"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        int heal = std::max(5, attacker.getMaxHp() / 12 + level / 5);
        if (choice == 2)
        {
            showClassSkillResult(
                "Prière purificatrice",
                "combat.class_skill.sacred.purifying_prayer.result",
                {"La lumière cherche d'abord les effets dangereux avant de refermer les blessures."}
            );
            int cured = 0;
            if (attacker.cureBurning()) cured++;
            if (attacker.curePoison()) cured++;
            if (attacker.cureFrost()) cured++;
            if (attacker.cureShock()) cured++;
            if (attacker.cureBleeding()) cured++;
            if (attacker.cureWeakening()) cured++;
            if (attacker.cureVulnerability()) cured++;
            attacker.heal(heal + level / 3);
            attacker.startDefensePosture(10, 3, "Prière purificatrice");
            showCombatActionMessage(
                "PRIÈRE PURIFICATRICE",
                "combat.class_skill.sacred.purifying_prayer.heal",
                {
                    attacker.getName() + " récupère " + std::to_string(heal + level / 3) + " PV.",
                    "Effets purifiés : " + std::to_string(cured) + "."
                },
                false
            );
            startCooldown(5);
            return true;
        }

        showClassSkillResult(
            "Serment de survie",
            "combat.class_skill.sacred.survival_oath.result",
            {"Ce tour sert surtout à tenir : soin personnel et garde légère."}
        );
        attacker.heal(heal);
        attacker.startDefensePosture(12, 4, "Serment de survie");
        showCombatActionMessage(
            "SERMENT DE SURVIE",
            "combat.class_skill.sacred.survival_oath.heal",
            {attacker.getName() + " récupère " + std::to_string(heal) + " PV."},
            false
        );
        startCooldown(3);
        return true;
    }

    if (hasAny({"mage", "magicien", "ensorceleur", "sorcier", "arcaniste", "pyromancien", "cryomancien", "occultiste", "invocateur", "nécromancien", "necromancien", "pactisant", "clerc", "prêtre", "pretre", "paladin", "druide", "shaman", "templier", "mage-lame", "chevalier runique"})
        && (level >= TechniqueProgressionRules::classTechniqueUnlockLevel("arcane_mark")
            || (player != nullptr && ((player->hasActiveSkill("learned_arcane_mark") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_arcane_mark"))
                || (player->hasActiveSkill("learned_arcane_binding") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_arcane_binding"))
                || (player->hasActiveSkill("learned_elemental_ward") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_elemental_ward"))
                || (player->hasActiveSkill("learned_resistance_rift") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_resistance_rift"))
                || (player->hasActiveSkill("learned_frost_needle") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_frost_needle"))
                || (player->hasActiveSkill("learned_mana_suture") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_mana_suture"))
                || (player->hasActiveSkill("learned_occult_bramble") && level >= TechniqueProgressionRules::magicStudyUnlockLevel("learned_occult_bramble"))))))
    {
        auto startMagicCooldown = [&](int baseTurns) {
            const int adjustment = player != nullptr ? player->getMagicRecoveryAdjustment() : 0;
            startCooldown(std::max(2, baseTurns + adjustment));
        };

        auto learnedAtRequiredLevel = [&](const std::string& skillId) {
            return player != nullptr
                && player->hasActiveSkill(skillId)
                && level >= TechniqueProgressionRules::magicStudyUnlockLevel(skillId);
        };

        bool canArcaneMark = level >= TechniqueProgressionRules::classTechniqueUnlockLevel("arcane_mark") || learnedAtRequiredLevel("learned_arcane_mark");
        bool canArcaneBinding = level >= TechniqueProgressionRules::classTechniqueUnlockLevel("arcane_binding") || learnedAtRequiredLevel("learned_arcane_binding");
        bool canElementalWard = level >= TechniqueProgressionRules::classTechniqueUnlockLevel("elemental_ward") || learnedAtRequiredLevel("learned_elemental_ward");
        bool canControlledOverload = level >= TechniqueProgressionRules::classTechniqueUnlockLevel("controlled_overload");
        bool canResistanceRift = level >= TechniqueProgressionRules::classTechniqueUnlockLevel("resistance_rift") || learnedAtRequiredLevel("learned_resistance_rift");
        bool canFrostNeedle = learnedAtRequiredLevel("learned_frost_needle");
        bool canManaSuture = learnedAtRequiredLevel("learned_mana_suture");
        bool canOccultBramble = learnedAtRequiredLevel("learned_occult_bramble");

        int skillChoice = askStructuredCombatChoice(
            "COMPÉTENCE ARCANIQUE",
            "combat.class_skill.arcane",
            {
                attacker.getName() + " choisit une formule arcanique.",
                "Les sorts non compris restent visibles mais verrouillés."
            },
            {
                {1, "Marque élémentaire", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("arcane_mark")) + " ou grimoire dès niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_arcane_mark")) + ", recharge 3 tours.", canArcaneMark, "arcane_mark"},
                {2, "Entrave arcanique", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("arcane_binding")) + " ou grimoire dès niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_arcane_binding")) + ", recharge 4 tours.", canArcaneBinding, "arcane_binding"},
                {3, "Voile élémentaire", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("elemental_ward")) + " ou grimoire dès niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_elemental_ward")) + ", recharge 4 tours.", canElementalWard, "elemental_ward"},
                {4, "Surcharge contrôlée", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("controlled_overload")) + ", recharge 5 tours.", canControlledOverload, "controlled_overload"},
                {5, "Faille de résistance", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("resistance_rift")) + " ou grimoire dès niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_resistance_rift")) + ", recharge 5 tours.", canResistanceRift, "resistance_rift"},
                {6, "Aiguille de givre étudiée", "Grimoire, niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_frost_needle")) + ", recharge 4 tours.", canFrostNeedle, "frost_needle"},
                {7, "Suture de mana étudiée", "Grimoire, niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_mana_suture")) + ", recharge 5 tours.", canManaSuture, "mana_suture"},
                {8, "Ronces occultes étudiées", "Grimoire, niv. " + std::to_string(TechniqueProgressionRules::magicStudyUnlockLevel("learned_occult_bramble")) + ", recharge 4 tours.", canOccultBramble, "occult_bramble"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (skillChoice == 0) return false;

        if (skillChoice == 8)
        {
            showClassSkillResult(
                "Ronces occultes",
                "combat.class_skill.arcane.dark_roots.result",
                {"Ce grimoire ne lance pas une boule d'énergie : il force des liens sombres à sortir du sol autour de la cible."}
            );
            if (!resolveMagicCatalystAttempt(player, "Ronces occultes", random, -1, "terre"))
            {
                startMagicCooldown(2);
                return true;
            }
            defender.applyWeakening(3, 12 + level / 10);
            ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 20);
            if (random.between(1, 100) <= 35)
            {
                ElementalAffinitySystem::applyFrost(defender, 1);
                showCombatActionMessage(
                    "RONCES OCCULTES",
                    "combat.class_skill.arcane.dark_roots.slow",
                    {"Les ronces freinent aussi l'élan de la cible."},
                    false
                );
            }
            executeBoostedAttack(attacker, defender, random, 1 + level / 28);
            startMagicCooldown(4);
            return true;
        }

        if (skillChoice == 7)
        {
            showClassSkillResult(
                "Suture de mana",
                "combat.class_skill.arcane.mana_suture.result",
                {"Ce sort vient d'un grimoire : il ne soigne pas d'un coup, il referme lentement les blessures."}
            );
            if (!resolveMagicCatalystAttempt(player, "Suture de mana", random, -1, "arcanique"))
            {
                startMagicCooldown(2);
                return true;
            }
            int healPerTurn = std::max(4, attacker.getMaxHp() / 18 + level / 8);
            attacker.applyRegeneration(3, healPerTurn);
            attacker.startDefensePosture(6, 2, "Suture de mana");
            showCombatActionMessage(
                "SUTURE DE MANA",
                "combat.class_skill.arcane.mana_suture.heal",
                {"La suture commence à se refermer autour des plaies."},
                false
            );
            startMagicCooldown(5);
            return true;
        }

        if (skillChoice == 6)
        {
            showClassSkillResult(
                "Aiguille de givre",
                "combat.class_skill.arcane.frost_needle.result",
                {"Ce sort vient d'un grimoire précis : il n'a pas encore de parchemin courant fiable."}
            );
            if (!resolveMagicCatalystAttempt(player, "Aiguille de givre", random, -1, "froid"))
            {
                startMagicCooldown(2);
                return true;
            }
            ElementalAffinitySystem::applyFrost(defender, 3);
            if (random.between(1, 100) <= 35)
            {
                defender.applyWeakening(2, 8 + level / 12);
                showCombatActionMessage(
                    "AIGUILLE DE GIVRE",
                    "combat.class_skill.arcane.frost_needle.slow",
                    {"Le froid mord assez juste pour raidir le prochain geste."},
                    false
                );
            }
            executeBoostedAttack(attacker, defender, random, 2 + level / 26);
            startMagicCooldown(4);
            return true;
        }

        if (skillChoice == 5)
        {
            int choice = askStructuredCombatChoice(
                "FAILLE DE RÉSISTANCE",
                "combat.class_skill.arcane.rift",
                {
                    "Tu ne cherches pas seulement à blesser : tu ouvres une brèche que les prochains impacts pourront exploiter.",
                    "Choisis la faille à appliquer."
                },
                {
                    {1, "Faille ardente", "Brèche vulnérable au feu.", true, "fire_rift"},
                    {2, "Faille de givre", "Brèche vulnérable au froid.", true, "frost_rift"},
                    {3, "Faille conductrice", "Brèche vulnérable au choc.", true, "shock_rift"}
                }
            );
            const std::string riftElement = choice == 1 ? "feu" : (choice == 2 ? "froid" : "foudre");
            if (!resolveMagicCatalystAttempt(player, "Faille de résistance", random, -1, riftElement))
            {
                startMagicCooldown(2);
                return true;
            }
            defender.applyVulnerability(3, 16 + level / 10);
            if (choice == 1) ElementalAffinitySystem::applyBurning(defender, 3, 3 + level / 18);
            else if (choice == 2) ElementalAffinitySystem::applyFrost(defender, 3);
            else ElementalAffinitySystem::applyShock(defender, 2);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            startMagicCooldown(5);
            return true;
        }

        if (skillChoice == 4)
        {
            int choice = askStructuredCombatChoice(
                "SURCHARGE CONTRÔLÉE",
                "combat.class_skill.arcane.overload",
                {"Choisis l'élément concentré dans la surcharge."},
                {
                    {1, "Feu concentré", "Brûlure arcanique.", true, "fire_overload"},
                    {2, "Givre concentré", "Ralentissement par le froid.", true, "frost_overload"},
                    {3, "Choc concentré", "Décharge instable.", true, "shock_overload"},
                    {4, "Poison concentré", "Venin arcanique.", true, "poison_overload"}
                }
            );
            const std::string overloadElement = choice == 1 ? "feu" : (choice == 2 ? "froid" : (choice == 3 ? "foudre" : "arcanique"));
            if (!resolveMagicCatalystAttempt(player, "Surcharge contrôlée", random, -1, overloadElement))
            {
                startMagicCooldown(2);
                return true;
            }
            if (choice == 1) ElementalAffinitySystem::applyBurning(defender, 4, 3 + level / 16);
            else if (choice == 2) ElementalAffinitySystem::applyFrost(defender, 4);
            else if (choice == 3) ElementalAffinitySystem::applyShock(defender, 3);
            else ElementalAffinitySystem::applyPoison(defender, 4, 3 + level / 18);
            executeBoostedAttack(attacker, defender, random, 4 + level / 22);
            startMagicCooldown(5);
            return true;
        }

        if (skillChoice == 3)
        {
            showClassSkillResult(
                "Voile élémentaire",
                "combat.class_skill.arcane.elemental_veil.result",
                {"Tu replies la magie contre toi plutôt que de la projeter : les altérations élémentaires auront plus de mal à s'accrocher."}
            );
            if (!resolveMagicCatalystAttempt(player, "Voile élémentaire", random, -1, "arcanique"))
            {
                startMagicCooldown(2);
                return true;
            }
            attacker.applyElementalWard(4, 18 + level / 8);
            attacker.startDefensePosture(8, 3, "Voile élémentaire");
            startMagicCooldown(4);
            return true;
        }

        if (skillChoice == 2)
        {
            showClassSkillResult(
                "Entrave arcanique",
                "combat.class_skill.arcane.arcane_shackle.result",
                {"La magie ne cherche pas seulement à brûler ou choquer : elle tire directement sur le souffle de la cible."}
            );
            if (!resolveMagicCatalystAttempt(player, "Entrave arcanique", random, -1, "arcanique"))
            {
                startMagicCooldown(2);
                return true;
            }
            defender.applyWeakening(3, 14 + level / 8);
            if (random.between(1, 100) <= 45)
            {
                ElementalAffinitySystem::applyShock(defender, 1);
                showCombatActionMessage("ENTRAVE ARCANIQUE", "combat.class_skill.arcane.arcane_shackle.shock", {"L'entrave grésille assez pour troubler le prochain geste."}, false);
            }
            else
            {
                ElementalAffinitySystem::applyFrost(defender, 1);
                showCombatActionMessage("ENTRAVE ARCANIQUE", "combat.class_skill.arcane.arcane_shackle.frost", {"L'entrave refroidit le rythme plutôt que le corps."}, false);
            }
            /* Écran structuré déjà affiché. */
            executeBoostedAttack(attacker, defender, random, 2 + level / 28);
            startMagicCooldown(4);
            return true;
        }

        int choice = askStructuredCombatChoice(
            "MARQUE ÉLÉMENTAIRE",
            "combat.class_skill.arcane.mark",
            {"Choisis la marque élémentaire à poser."},
            {
                {1, "Braise faible", "Applique une petite brûlure.", true, "mark_fire"},
                {2, "Givre bref", "Ralentit la cible.", true, "mark_frost"},
                {3, "Décharge instable", "Trouble le prochain geste adverse.", true, "mark_shock"},
                {4, "Venin arcanique", "Empoisonne faiblement la cible.", true, "mark_poison"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        const std::string markElement = choice == 1 ? "feu" : (choice == 2 ? "froid" : (choice == 3 ? "foudre" : "arcanique"));
        if (!resolveMagicCatalystAttempt(player, "Marque élémentaire", random, -1, markElement))
        {
            startMagicCooldown(2);
            return true;
        }
        std::string markNote;
        if (choice == 1) { ElementalAffinitySystem::applyBurning(defender, 3, 2 + level / 20); markNote = "Une braise faible s'accroche à la cible."; }
        else if (choice == 2) { ElementalAffinitySystem::applyFrost(defender, 3); markNote = "Un froid bref ralentit la cible."; }
        else if (choice == 3) { ElementalAffinitySystem::applyShock(defender, 2); markNote = "Une décharge instable trouble le prochain geste adverse."; }
        else { ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 22); markNote = "Un venin arcanique faible entre dans la blessure."; }
        showCombatActionMessage("MARQUE ÉLÉMENTAIRE", "combat.class_skill.arcane.elemental_mark.result", {markNote}, false);
        executeBoostedAttack(attacker, defender, random, 1 + level / 30);
        startMagicCooldown(3);
        return true;
    }

    if (hasAny({"archer", "rôdeur", "rodeur", "tireur"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("cover_shot"))
    {
        if (!hasRangedWeapon())
        {
            showCombatActionMessage(
                "COMPÉTENCE DISTANCE IMPOSSIBLE",
                "combat.class_skill.ranged.no_weapon",
                {"Cette compétence demande une vraie arme à distance équipée."},
                false
            );
            return false;
        }

        int choice = askStructuredCombatChoice(
            "COMPÉTENCE DISTANCE",
            "combat.class_skill.ranged",
            {attacker.getName() + " prépare une technique à distance."},
            {
                {1, "Tir de couverture", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("cover_shot")) + ", recharge 3 tours. Contrôle la cible.", true, "cover_shot"},
                {2, "Tir d'arrêt", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("stopping_shot")) + ", recharge 4 tours. Casse l'élan.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("stopping_shot"), "stopping_shot"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        if (choice == 2)
        {
            showClassSkillResult(
                "Tir d'arrêt",
                "combat.class_skill.ranged.stopping_shot.result",
                {"Le tir cherche à casser l'élan, pas seulement à blesser."}
            );
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 18);
            ElementalAffinitySystem::applyFrost(defender, 2);
            executeBoostedAttack(attacker, defender, random, 6 + level / 16);
            startCooldown(4);
            return true;
        }

        showClassSkillResult(
            "Tir de couverture",
            "combat.class_skill.ranged.cover_shot.result",
            {"Le tir vise moins la létalité immédiate que le contrôle de la cible."}
        );
        ElementalAffinitySystem::applyFrost(defender, 1);
        executeBoostedAttack(attacker, defender, random, 4 + level / 18);
        startCooldown(3);
        return true;
    }

    if (hasAny({"berserker", "barbare", "briseur"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("assumed_rage"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE BRUTALE",
            "combat.class_skill.brutal",
            {attacker.getName() + " canalise une violence directe."},
            {
                {1, "Rage assumée", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("assumed_rage")) + ", recharge 3 tours. Bonus de dégâts avec contrecoup.", true, "assumed_rage"},
                {2, "Coup de rupture", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("rupture_blow")) + ", recharge 4 tours. Frappe plus lourde et saignement.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("rupture_blow"), "rupture_blow"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        int bonus = 6 + level / 12;
        int recoil = std::max(1, attacker.getMaxHp() / 40);
        if (attacker.getHp() * 100 <= attacker.getMaxHp() * 50)
        {
            bonus += 7;
            showCombatActionMessage(
                "RAGE RENFORCÉE",
                "combat.class_skill.brutal.rage_bonus",
                {"La blessure nourrit la rage : bonus renforcé."},
                false
            );
        }
        int cooldownTurns = 3;
        if (choice == 2)
        {
            showClassSkillResult(
                "Coup de rupture",
                "combat.class_skill.brutal.rupture_blow.result",
                {"La violence cherche la rupture plutôt que le simple impact."}
            );
            bonus += 8;
            recoil = std::max(2, attacker.getMaxHp() / 25);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 16);
            cooldownTurns = 4;
        }
        else
        {
            showClassSkillResult(
                "Rage assumée",
                "combat.class_skill.brutal.assumed_rage.result",
                {"La violence est acceptée, mais elle réclame un contrecoup."}
            );
        }
        attacker.takeDamage(recoil);
        showCombatActionMessage(
            "CONTRECOUP BRUTAL",
            "combat.class_skill.brutal.recoil",
            {"Le corps encaisse " + std::to_string(recoil) + " PV de contrecoup avant de rendre la violence."},
            false
        );
        executeBoostedAttack(attacker, defender, random, bonus);
        startCooldown(cooldownTurns);
        return true;
    }

    if (hasAny({"alchim", "artific", "bricoleur"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("improvised_mix"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE EXPÉRIMENTALE",
            "combat.class_skill.experimental",
            {attacker.getName() + " choisit un procédé instable."},
            {
                {1, "Mélange improvisé", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("improvised_mix")) + ", recharge 3 tours. Effet aléatoire.", true, "improvised_mix"},
                {2, "Fiole tactique", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("tactical_vial")) + ", recharge 4 tours. Choix d'effet.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("tactical_vial"), "tactical_vial"},
                {3, "Poudre de fragilisation", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("fragility_powder")) + ", recharge 5 tours. Vulnérabilité.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("fragility_powder"), "fragility_powder"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        if (choice == 3)
        {
            showClassSkillResult(
                "Poudre de fragilisation",
                "combat.class_skill.experimental.fragility_powder.result",
                {"La poudre n'explose presque pas : elle s'accroche aux points faibles et rend le prochain échange dangereux."}
            );
            defender.applyVulnerability(3, 14 + level / 12);
            if (random.between(1, 100) <= 55) ElementalAffinitySystem::applyPoison(defender, 2, 2 + level / 24);
            else ElementalAffinitySystem::applyShock(defender, 1);
            executeBoostedAttack(attacker, defender, random, 2 + level / 26);
            startCooldown(5);
            return true;
        }
        if (choice == 2)
        {
            int vial = askStructuredCombatChoice(
                "FIOLE TACTIQUE",
                "combat.class_skill.experimental.vial",
                {"Choisis la fiole à briser pendant l'action."},
                {
                    {1, "Acide / poison", "Empoisonne la cible.", true, "acid_vial"},
                    {2, "Fumée défensive", "Renforce la posture défensive.", true, "smoke_vial"},
                    {3, "Étincelle instable", "Applique un choc.", true, "spark_vial"}
                }
            );
            if (vial == 1) ElementalAffinitySystem::applyPoison(defender, 4, 3 + level / 18);
            else if (vial == 2) attacker.startDefensePosture(22, 8, "Fumée tactique");
            else ElementalAffinitySystem::applyShock(defender, 3);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            startCooldown(4);
            return true;
        }

        int roll = random.between(1, 4);
        std::string mixNote;
        if (roll == 1) { ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 20); mixNote = "Le mélange pique et empoisonne la cible."; }
        else if (roll == 2) { ElementalAffinitySystem::applyBurning(defender, 2, 2 + level / 22); mixNote = "Le mélange chauffe trop vite et brûle la cible."; }
        else if (roll == 3) { ElementalAffinitySystem::applyShock(defender, 2); mixNote = "Un petit mécanisme produit une décharge."; }
        else { attacker.startDefensePosture(14, 6, "Écran expérimental"); mixNote = "Le mélange produit surtout un écran défensif."; }
        showClassSkillResult(
            "Mélange improvisé",
            "combat.class_skill.experimental.improvised_mix.result",
            {mixNote}
        );
        executeBoostedAttack(attacker, defender, random, 2 + level / 25);
        startCooldown(3);
        return true;
    }

    if (hasAny({"invoc", "dresseur", "nécro", "necro"}) && level >= TechniqueProgressionRules::classTechniqueUnlockLevel("spectral_order"))
    {
        int choice = askStructuredCombatChoice(
            "COMPÉTENCE D'APPEL",
            "combat.class_skill.call",
            {attacker.getName() + " appelle une pression de groupe."},
            {
                {1, "Ordre spectral", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("spectral_order")) + ", recharge 3 tours. Appui invisible et froid.", true, "spectral_order"},
                {2, "Lien de meute / ombre", "Niv. " + std::to_string(TechniqueProgressionRules::classTechniqueUnlockLevel("pack_shadow_link")) + ", recharge 4 tours. Pression renforcée.", level >= TechniqueProgressionRules::classTechniqueUnlockLevel("pack_shadow_link"), "pack_shadow_link"},
                {0, "Retour", "Ne consomme pas le tour.", true, "back"}
            }
        );
        if (choice == 0) return false;
        if (choice == 2)
        {
            showClassSkillResult(
                "Lien de meute / ombre",
                "combat.class_skill.call.pack_shadow_link.result",
                {"Ta présence impose une pression de groupe, même sans invocation déjà visible."}
            );
            ElementalAffinitySystem::applyFrost(defender, 3);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 20);
            attacker.startDefensePosture(12, 5, "Lien de meute / ombre");
            executeBoostedAttack(attacker, defender, random, 5 + level / 20);
            startCooldown(4);
            return true;
        }

        showClassSkillResult(
            "Ordre spectral",
            "combat.class_skill.call.spectral_order.result",
            {"Même sans invocation active, ta présence force un appui invisible et froid."}
        );
        ElementalAffinitySystem::applyFrost(defender, 2);
        attacker.startDefensePosture(8, 3, "Ordre spectral");
        executeBoostedAttack(attacker, defender, random, 3 + level / 24);
        startCooldown(3);
        return true;
    }

    showCombatActionMessage(
        "COMPÉTENCE INSTABLE",
        "combat.class_skill.unavailable.fallback",
        {
            "Aucune technique de classe stable ne répond pour ce profil.",
            "Les maîtres de guilde disent que les premières techniques fiables apparaissent plutôt vers les niveaux 8 à 12, selon la classe et la simplicité du geste.",
            "Tu improvises donc une attaque simple."
        },
        false
    );
    executeAttack(attacker, defender, random);
    return true;
}

bool CombatActions::executeAIClassSkill(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    if (!attacker.isClassSkillReady())
    {
        return false;
    }

    std::string className = CombatClassSystem::normalizeClassText(attacker.getType());
    Player* player = dynamic_cast<Player*>(&attacker);
    Monster* monster = dynamic_cast<Monster*>(&attacker);
    Boss* boss = dynamic_cast<Boss*>(&attacker);

    int level = 1;
    if (player != nullptr)
    {
        level = player->getLevel();
    }
    else if (monster != nullptr)
    {
        level = monster->getLevel();
    }
    else if (boss != nullptr)
    {
        level = 12;
    }
    else
    {
        level = std::max(1, (attacker.getMaxDamage() + attacker.getCriticalDamage()) / 12);
    }

    std::string profile = normalizeCombatActionText(attacker.getName() + " " + attacker.getType());

    auto hasAny = [&](std::initializer_list<const char*> terms) {
        for (const char* term : terms)
        {
            if (className.find(term) != std::string::npos) return true;
        }
        return false;
    };

    auto finishAIClassSkill = [&](int cooldown) {
        attacker.startClassSkillCooldown(cooldown);
        showCombatActionMessage(
            "RÉCUPÉRATION IA",
            "combat.ai.class_skill.cooldown",
            {
                attacker.getName() + " reprend son souffle après cette technique.",
                "Récupération restante : " + std::to_string(cooldown) + " tour(s)."
            },
            false
        );
    };

    auto advancedProfile = [&]() {
        return profile.find("elite") != std::string::npos
            || profile.find("élite") != std::string::npos
            || profile.find("veteran") != std::string::npos
            || profile.find("vétéran") != std::string::npos
            || profile.find("chef") != std::string::npos
            || profile.find("champion") != std::string::npos
            || profile.find("oracle") != std::string::npos
            || profile.find("ancien") != std::string::npos
            || profile.find("hazak") != std::string::npos
            || profile.find("skuro") != std::string::npos
            || profile.find("kanade") != std::string::npos
            || profile.find("aoi") != std::string::npos
            || profile.find("sanctus") != std::string::npos
            || profile.find("fail") != std::string::npos
            || profile.find("hestia") != std::string::npos
            || profile.find("fire flight") != std::string::npos
            || profile.find("louis") != std::string::npos
            || profile.find("mattzelda") != std::string::npos
            || profile.find("trexof") != std::string::npos
            || profile.find("henrique") != std::string::npos;
    };

    bool knowsAdvancedMove = advancedProfile();

    if (hasAny({"voleur", "roublard", "brigand"}))
    {
        if (level >= 10 && knowsAdvancedMove && random.between(1, 100) <= 42)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.thief.pocket", {attacker.getName() + " frappe les attaches et le rythme plutôt que de chercher seulement les dégâts.", "Effet : affaiblissement, vulnérabilité et saignement léger."}, false);
            defender.applyWeakening(3, 11 + level / 12);
            defender.applyVulnerability(2, 9 + level / 16);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 20);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            finishAIClassSkill(5);
            return true;
        }

        if (level >= 7 && random.between(1, 100) <= (knowsAdvancedMove ? 44 : 28))
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.thief.powder", {attacker.getName() + " jette une poudre courte pour brouiller la lecture du tour.", "Effet : posture défensive et affaiblissement."}, false);
            attacker.startDefensePosture(16, 14, "Poudre d'échappée");
            defender.applyWeakening(2, 9 + level / 14);
            if (random.between(1, 100) <= 40) ElementalAffinitySystem::applyPoison(defender, 2, 2 + level / 26);
            executeBoostedAttack(attacker, defender, random, 1 + level / 32);
            finishAIClassSkill(4);
            return true;
        }

        showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.thief.opportunist", {attacker.getName() + " vise une petite ouverture au lieu d'une grosse frappe.", "Effet : vulnérabilité courte."}, false);
        defender.applyVulnerability(2, 8 + level / 14);
        if (random.between(1, 100) <= 35) ElementalAffinitySystem::applyBleeding(defender, 1, 2 + level / 26);
        executeBoostedAttack(attacker, defender, random, 2 + level / 26);
        finishAIClassSkill(3);
        return true;
    }

    if (hasAny({"assassin", "ombrelame", "lanceur de dagues"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 55)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.assassin.advanced", {attacker.getName() + " s'abaisse dans un angle mort et laisse une blessure plus longue.", "Effets : posture défensive et saignement prolongé."}, false);
            attacker.startDefensePosture(12, 14, "Disparition basse");
            ElementalAffinitySystem::applyBleeding(defender, 4, 3 + std::max(1, level / 16));
            executeBoostedAttack(attacker, defender, random, 4 + level / 20);
            finishAIClassSkill(4);
        }
        else
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.assassin.basic", {attacker.getName() + " disparaît brièvement de l'angle de vision.", "Effet : saignement préparé avant l'attaque."}, false);
            ElementalAffinitySystem::applyBleeding(defender, 3, 2 + std::max(1, level / 18));
            executeBoostedAttack(attacker, defender, random, 2 + level / 22);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"gardien", "tank", "colosse"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 50)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.defense.advanced", {attacker.getName() + " s'ancre au sol et transforme sa défense en mur vivant.", "Effet : posture défensive majeure."}, false);
            attacker.startDefensePosture(32, 10, "Ancrage total");
            executeBoostedAttack(attacker, defender, random, -5);
            finishAIClassSkill(4);
        }
        else
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.defense.basic", {attacker.getName() + " impose sa présence et verrouille sa posture.", "Effet : défense et menace renforcées."}, false);
            attacker.startDefensePosture(20, 6, "Rempart instinctif");
            executeBoostedAttack(attacker, defender, random, -2);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"clerc", "prêtre", "pretre", "paladin"}))
    {
        int heal = std::max(4, attacker.getMaxHp() / 14 + level / 6);
        if (level >= 7 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            int cured = 0;
            if (attacker.cureBurning()) cured++;
            if (attacker.curePoison()) cured++;
            if (attacker.cureFrost()) cured++;
            if (attacker.cureShock()) cured++;
            if (attacker.cureBleeding()) cured++;
            if (attacker.cureWeakening()) cured++;
            if (attacker.cureVulnerability()) cured++;
            attacker.heal(heal + level / 4);
            attacker.startDefensePosture(12, 4, "Prière purificatrice");
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.sacred.purify", {attacker.getName() + " purifie " + std::to_string(cured) + " effet(s).", "Soin reçu : " + std::to_string(heal + level / 4) + " PV."}, false);
            finishAIClassSkill(5);
        }
        else
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.sacred.oath", {attacker.getName() + " murmure un serment et tient la ligne.", "Effet : soin personnel et garde discrète."}, false);
            attacker.heal(heal);
            attacker.startDefensePosture(10, 3, "Serment discret");
            showCombatActionMessage("SOIN IA", "combat.ai.class_skill.sacred.heal", {attacker.getName() + " récupère " + std::to_string(heal) + " PV."}, false);
            finishAIClassSkill(4);
        }
        return true;
    }

    if (hasAny({"mage", "magicien", "ensorceleur", "sorcier", "arcaniste", "pyromancien", "cryomancien", "occultiste", "invocateur", "nécromancien", "necromancien", "pactisant", "clerc", "prêtre", "pretre", "paladin", "druide", "shaman", "templier", "mage-lame", "chevalier runique"}))
    {
        int hpPercent = attacker.getMaxHp() > 0 ? attacker.getHp() * 100 / attacker.getMaxHp() : 100;
        if (level >= 9 && hpPercent <= 45 && knowsAdvancedMove && random.between(1, 100) <= 30)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.arcane.suture", {attacker.getName() + " recoud une blessure avec un fil de mana tremblant.", "Effet : régénération et garde légère."}, false);
            attacker.applyRegeneration(3, std::max(4, attacker.getMaxHp() / 22 + level / 10));
            attacker.startDefensePosture(6, 2, "Suture de mana");
            finishAIClassSkill(5);
            return true;
        }

        if (level >= 8 && hpPercent <= 55 && random.between(1, 100) <= (knowsAdvancedMove ? 48 : 28))
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.arcane.ward", {attacker.getName() + " referme un voile élémentaire autour de son corps.", "Effet : résistance élémentaire temporaire."}, false);
            attacker.applyElementalWard(3, 16 + level / 10);
            attacker.startDefensePosture(8, 3, "Voile élémentaire");
            finishAIClassSkill(4);
            return true;
        }

        if (level >= 11 && knowsAdvancedMove && random.between(1, 100) <= 34)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.arcane.rift", {attacker.getName() + " ouvre une faille magique au lieu de chercher le coup direct.", "Effet : vulnérabilité et altération élémentaire."}, false);
            defender.applyVulnerability(3, 15 + level / 12);
            int element = random.between(1, 3);
            if (element == 1) ElementalAffinitySystem::applyBurning(defender, 3, 3 + level / 20);
            else if (element == 2) ElementalAffinitySystem::applyFrost(defender, 3);
            else ElementalAffinitySystem::applyShock(defender, 2);
            executeBoostedAttack(attacker, defender, random, 3 + level / 26);
            finishAIClassSkill(5);
            return true;
        }

        if (level >= 6 && random.between(1, 100) <= (knowsAdvancedMove ? 38 : 24))
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.arcane.binding", {attacker.getName() + " serre la trame magique autour du souffle adverse.", "Effet : affaiblissement et perturbation."}, false);
            defender.applyWeakening(3, 12 + level / 10);
            if (random.between(1, 100) <= 50) ElementalAffinitySystem::applyShock(defender, 1);
            else ElementalAffinitySystem::applyFrost(defender, 1);
            executeBoostedAttack(attacker, defender, random, 2 + level / 30);
            finishAIClassSkill(4);
            return true;
        }

        showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.arcane.mark", {attacker.getName() + " condense une marque élémentaire.", "Effet : altération élémentaire appliquée avant l'attaque."}, false);
        int element = random.between(1, 4);
        if (element == 1) ElementalAffinitySystem::applyBurning(defender, 3, 2 + level / 22);
        else if (element == 2) ElementalAffinitySystem::applyFrost(defender, 3);
        else if (element == 3) ElementalAffinitySystem::applyShock(defender, 2);
        else ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 24);
        if (level >= 9 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            int secondElement = random.between(1, 3);
            if (secondElement == 1) ElementalAffinitySystem::applyBurning(defender, 2, 2 + level / 24);
            else if (secondElement == 2) ElementalAffinitySystem::applyShock(defender, 1);
            else ElementalAffinitySystem::applyFrost(defender, 2);
            executeBoostedAttack(attacker, defender, random, 4 + level / 24);
            finishAIClassSkill(5);
        }
        else
        {
            executeBoostedAttack(attacker, defender, random, 1 + level / 32);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"archer", "rôdeur", "rodeur", "tireur"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.ranged.stop", {attacker.getName() + " vise l'élan adverse et cherche à clouer le rythme.", "Effet : ralentissement et saignement."}, false);
            ElementalAffinitySystem::applyFrost(defender, 2);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 20);
            executeBoostedAttack(attacker, defender, random, 5 + level / 18);
            finishAIClassSkill(4);
        }
        else
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.ranged.cover", {attacker.getName() + " choisit un tir de contrôle plutôt qu'un tir gratuit.", "Effet : contrôle léger avant l'impact."}, false);
            ElementalAffinitySystem::applyFrost(defender, 1);
            executeBoostedAttack(attacker, defender, random, 3 + level / 20);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"berserker", "barbare", "briseur"}))
    {
        int recoil = std::max(1, attacker.getMaxHp() / 45);
        showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.brutal.rage", {attacker.getName() + " laisse la rage guider le prochain impact.", "Effet : contrecoup personnel avant l'attaque."}, false);
        attacker.takeDamage(recoil);
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 50)
        {
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 18);
            executeBoostedAttack(attacker, defender, random, 8 + level / 12);
            finishAIClassSkill(4);
        }
        else
        {
            executeBoostedAttack(attacker, defender, random, 5 + level / 14);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"alchim", "artific", "bricoleur"}))
    {
        showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.experimental.mix", {attacker.getName() + " libère un mélange préparé à la va-vite.", "Effet : résultat instable selon la fiole."}, false);
        int roll = random.between(1, 4);
        if (roll == 1) ElementalAffinitySystem::applyPoison(defender, 3, 2 + level / 22);
        else if (roll == 2) ElementalAffinitySystem::applyBurning(defender, 2, 2 + level / 24);
        else if (roll == 3) ElementalAffinitySystem::applyShock(defender, 2);
        else attacker.startDefensePosture(12, 5, "Écran expérimental");
        if (level >= 9 && knowsAdvancedMove && random.between(1, 100) <= 35)
        {
            showCombatActionMessage("POUDRE IA", "combat.ai.class_skill.experimental.powder", {"Une poudre fine trouve les fissures de la défense adverse.", "Effet : vulnérabilité temporaire."}, false);
            defender.applyVulnerability(3, 13 + level / 14);
            executeBoostedAttack(attacker, defender, random, 3 + level / 24);
            finishAIClassSkill(5);
        }
        else if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            attacker.startDefensePosture(10, 4, "Fiole tactique");
            executeBoostedAttack(attacker, defender, random, 4 + level / 24);
            finishAIClassSkill(4);
        }
        else
        {
            executeBoostedAttack(attacker, defender, random, 2 + level / 28);
            finishAIClassSkill(3);
        }
        return true;
    }

    if (hasAny({"invoc", "dresseur", "nécro", "necro"}))
    {
        if (level >= 8 && knowsAdvancedMove && random.between(1, 100) <= 45)
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.call.link", {attacker.getName() + " renforce un lien invisible avant de donner l'ordre.", "Effet : pression de meute et garde."}, false);
            ElementalAffinitySystem::applyFrost(defender, 2);
            ElementalAffinitySystem::applyBleeding(defender, 2, 2 + level / 22);
            attacker.startDefensePosture(12, 5, "Lien de meute");
            executeBoostedAttack(attacker, defender, random, 4 + level / 22);
            finishAIClassSkill(4);
        }
        else
        {
            showCombatActionMessage("TECHNIQUE IA", "combat.ai.class_skill.call.order", {attacker.getName() + " donne un ordre à quelque chose que l'on distingue à peine.", "Effet : froid spectral et garde."}, false);
            ElementalAffinitySystem::applyFrost(defender, 2);
            attacker.startDefensePosture(8, 3, "Ordre spectral");
            executeBoostedAttack(attacker, defender, random, 2 + level / 24);
            finishAIClassSkill(3);
        }
        return true;
    }

    return false;
}

bool CombatActions::executeHealingPotion(
    Entity& entity,
    int potionHealAmount
)
{
    if (Player* player = dynamic_cast<Player*>(&entity))
    {
        const int potionIndex = player->getInventory().findFirstConsumable(ConsumableType::Healing);
        if (potionIndex >= 0)
        {
            const Consumable potion = player->getInventory().getConsumable(potionIndex);
            if (!confirmLockedCombatConsumableUse(*player, potion, "boire cette potion de soin"))
            {
                return false;
            }
        }
    }

    return CombatPotion::executeHealingPotion(
        entity,
        potionHealAmount
    );
}

bool CombatActions::executeDamagePotion(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionDamageBonus
)
{
    if (Player* player = dynamic_cast<Player*>(&attacker))
    {
        const int potionIndex = player->getInventory().findFirstConsumable(ConsumableType::Damage);
        if (potionIndex >= 0)
        {
            const Consumable potion = player->getInventory().getConsumable(potionIndex);
            if (!confirmLockedCombatConsumableUse(*player, potion, "utiliser cette potion offensive"))
            {
                return false;
            }
        }
    }

    return CombatPotion::executeDamagePotion(
        attacker,
        defender,
        random,
        potionDamageBonus
    );
}
