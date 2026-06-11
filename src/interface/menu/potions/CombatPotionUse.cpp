// EN: CombatPotionUse.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionUse.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionUse.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/system/DamageSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/DamageReport.hpp"

#include "interface/menu/CombatTargetMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include "item/Inventory.hpp"

#include "entity/Monster.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>


namespace
{
    std::string normalizePotionText(std::string value)
    {
        for (char& c : value)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return value;
    }

    bool potionNameContains(const Consumable& potion, const std::string& text)
    {
        return normalizePotionText(potion.getName()).find(normalizePotionText(text)) != std::string::npos;
    }

    void showPotionNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MessageScreen::show(title, screenId, lines);
    }



    void removeConsumedPotion(Player& player, int consumableIndex, const Consumable& potion)
    {
        if (player.getInventory().removeConsumable(consumableIndex))
        {
            player.recordConsumableUsed(potion.getName());
        }
    }

    bool playerLooksLikeNaturalCaster(const Player& player)
    {
        std::string className = normalizePotionText(player.getType());
        return className.find("mage") != std::string::npos
            || className.find("sorc") != std::string::npos
            || className.find("arcan") != std::string::npos
            || className.find("pyrom") != std::string::npos
            || className.find("cryo") != std::string::npos
            || className.find("occult") != std::string::npos
            || className.find("invoc") != std::string::npos
            || className.find("nécro") != std::string::npos
            || className.find("necro") != std::string::npos
            || className.find("pact") != std::string::npos
            || className.find("clerc") != std::string::npos
            || className.find("prêtre") != std::string::npos
            || className.find("pretre") != std::string::npos
            || className.find("paladin") != std::string::npos
            || className.find("druide") != std::string::npos
            || className.find("shaman") != std::string::npos
            || className.find("templier") != std::string::npos;
    }

    bool resolveScrollStability(Player& player, const Consumable& potion, Random& random, std::vector<std::string>& lines)
    {
        int successChance = playerLooksLikeNaturalCaster(player) ? 92 : 78;

        if (!playerLooksLikeNaturalCaster(player) && player.getLevel() <= 2)
        {
            successChance -= 8;
        }

        lines.push_back(player.getName() + " déchire " + potion.getName() + ".");
        lines.push_back("Stabilité du parchemin : " + std::to_string(successChance) + "%.");

        if (random.between(1, 100) <= successChance)
        {
            lines.push_back("Les runes tiennent assez longtemps pour prendre forme.");
            return true;
        }

        lines.push_back("Les runes brûlent trop vite : la magie se disperse avant de prendre forme.");
        return false;
    }

    bool applyCurativeStatusEffect(Player& player, const Consumable& potion, std::vector<std::string>& notes)
    {
        bool cured = false;

        if (potionNameContains(potion, "antidote"))
        {
            cured = player.curePoison();
            if (cured) notes.push_back("Le poison est neutralisé.");
        }
        else if (potionNameContains(potion, "anti-br") || potionNameContains(potion, "brûl") || potionNameContains(potion, "brul"))
        {
            cured = player.cureBurning();
            if (cured) notes.push_back("La brûlure est apaisée avant de continuer à ronger les chairs.");
        }
        else if (potionNameContains(potion, "givre") || potionNameContains(potion, "tiède") || potionNameContains(potion, "tiede"))
        {
            cured = player.cureFrost();
            if (cured) notes.push_back("Le froid quitte les articulations et les gestes redeviennent plus fluides.");
        }
        else if (potionNameContains(potion, "isolante") || potionNameContains(potion, "décharge") || potionNameContains(potion, "decharge"))
        {
            cured = player.cureShock();
            if (cured) notes.push_back("La conduction électrique est coupée avant de perturber un nouveau geste.");
        }
        else if (potionNameContains(potion, "défensive") || potionNameContains(potion, "defensive") || potionNameContains(potion, "précision") || potionNameContains(potion, "precision"))
        {
            cured = player.cureWeakening();
            if (!cured) cured = player.cureVulnerability();
            if (cured) notes.push_back("Le corps retrouve assez de stabilité pour refermer la faille active.");
        }

        return cured;
    }

    std::vector<int> chooseDistinctEffectIndexes(Random& random, int availableCount, int wantedCount)
    {
        std::vector<int> available;
        for (int i = 0; i < availableCount; ++i) available.push_back(i);

        std::vector<int> selected;
        wantedCount = std::max(0, std::min(wantedCount, availableCount));
        while (static_cast<int>(selected.size()) < wantedCount && !available.empty())
        {
            const int position = random.between(0, static_cast<int>(available.size()) - 1);
            selected.push_back(available[static_cast<std::size_t>(position)]);
            available.erase(available.begin() + position);
        }
        return selected;
    }

    Monster createWeakUnluckyEnemy(const Player& player, Random& random, int index)
    {
        const int level = std::max(1, player.getLevel() - 4);
        const int hp = 24 + level * 5;
        const int minDamage = std::max(1, 1 + level / 3);
        const int maxDamage = std::max(minDamage + 1, 4 + level);
        const int criticalDamage = std::max(maxDamage + 2, 7 + level * 2);

        const int variant = random.between(0, 2);
        if (variant == 0)
        {
            return Monster(
                "Petit slime malchanceux " + std::to_string(index),
                "Gelée faible attirée par la fiole",
                Race::Slime,
                level,
                hp,
                minDamage,
                maxDamage,
                criticalDamage,
                0,
                0
            );
        }
        if (variant == 1)
        {
            return Monster(
                "Gobelin tombé du mauvais sac " + std::to_string(index),
                "Renfort faible et très confus",
                Race::Gobelin,
                level,
                hp,
                minDamage,
                maxDamage,
                criticalDamage,
                0,
                0
            );
        }
        return Monster(
            "Rat attiré par la malchance " + std::to_string(index),
            "Petite bête opportuniste",
            Race::Bete,
            level,
            hp,
            minDamage,
            maxDamage,
            criticalDamage,
            0,
            0
        );
    }
}

bool CombatPotionUse::useHealingPotion(
    Player& player,
    int consumableIndex,
    const Consumable& potion
)
{
    if (!player.getInventory().hasConsumable(consumableIndex))
    {
        showPotionNotice(
            "POTION INTROUVABLE",
            "combat.potion.missing",
            {"Cette potion n'est plus disponible.", "Le sac a déjà changé depuis l'affichage précédent."}
        );
        return false;
    }

    const int hpBefore = player.getHp();
    std::vector<std::string> notes;
    bool curedStatus = applyCurativeStatusEffect(player, potion, notes);
    const int announcedHeal = potion.getHealingAmountForMaxHp(player.getMaxHp());
    player.heal(announcedHeal);
    ThreatSystem::markSelfHealingAction(player);

    if (!player.hasInfiniteConsumables())
    {
        removeConsumedPotion(player, consumableIndex, potion);
    }

    std::vector<std::string> lines;
    lines.push_back(player.getName() + " boit " + potion.getName() + ".");
    lines.push_back("Soin annoncé : " + potion.getPowerDisplayText() + ".");
    lines.push_back("Soin réel : +" + std::to_string(player.getHp() - hpBefore) + " PV.");
    lines.push_back("PV : " + std::to_string(hpBefore) + " -> " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".");
    for (const std::string& note : notes)
    {
        lines.push_back(note);
    }
    if (!curedStatus && player.hasActiveCombatStatus())
    {
        lines.push_back("La potion soigne les PV, mais elle n'est pas adaptée aux statuts encore actifs.");
    }

    showPotionNotice("POTION CURATIVE", "combat.potion.heal.result", lines);
    return true;
}

bool CombatPotionUse::useSelectedPotion(
    Player& player,
    int consumableIndex,
    ConsumableType type,
    Entity* target,
    EnemyCombatQueue* wave,
    Random& random,
    int potionDamageBonus
)
{
    if (!player.getInventory().hasConsumable(consumableIndex))
    {
        showPotionNotice(
            "POTION INTROUVABLE",
            "combat.potion.missing",
            {"Cette potion n'est plus disponible.", "Le sac a déjà changé depuis l'affichage précédent."}
        );
        return false;
    }

    Consumable potion = player.getInventory().getConsumable(consumableIndex);

    if (type == ConsumableType::Healing)
    {
        return useHealingPotion(player, consumableIndex, potion);
    }

    if (type == ConsumableType::Damage)
    {
        int totalBonus = potion.getPower();

        if (potionDamageBonus > 0)
        {
            totalBonus = potion.getPower();
        }

        if (wave != nullptr)
        {
            bool attackLaunched = CombatTargetMenu::openForBoostedAttack(
                player,
                *wave,
                random,
                totalBonus
            );

            if (attackLaunched && !player.hasInfiniteConsumables())
            {
                removeConsumedPotion(player, consumableIndex, potion);
            }

            return attackLaunched;
        }

        if (target != nullptr)
        {
            if (!player.hasInfiniteConsumables())
            {
                removeConsumedPotion(player, consumableIndex, potion);
            }

            showPotionNotice(
                "POTION OFFENSIVE",
                "combat.potion.damage.launch",
                {
                    player.getName() + " utilise " + potion.getName() + ".",
                    "Cible : " + target->getName(),
                    "Bonus d'attaque : +" + std::to_string(totalBonus)
                }
            );

            CombatAttack::executeBoostedAttack(
                player,
                *target,
                random,
                totalBonus
            );

            return true;
        }

        showPotionNotice(
            "AUCUNE CIBLE",
            "combat.potion.damage.no_target",
            {"Aucune cible offensive disponible.", "Choisis une cible valide ou une autre action."}
        );
        return false;
    }

    if (type == ConsumableType::Buff)
    {
        if (potionNameContains(potion, "lucky potion"))
        {
            if (!player.hasInfiniteConsumables())
            {
                removeConsumedPotion(player, consumableIndex, potion);
            }

            const int effectCount = random.between(3, 5);
            const std::vector<int> effects = chooseDistinctEffectIndexes(random, 6, effectCount);
            std::vector<std::string> lines = {
                player.getName() + " boit la Lucky Potion.",
                "La fiole choisit " + std::to_string(effectCount) + " bonus sans demander l'avis de personne.",
                "Durée annoncée : trois tours."
            };

            for (int effect : effects)
            {
                if (effect == 0)
                {
                    const int power = random.between(18, 30);
                    player.applyPowerBoost(4, power);
                    lines.push_back("- Élan chanceux : dégâts infligés +" + std::to_string(power) + "%.");
                }
                else if (effect == 1)
                {
                    const int rollBonus = random.between(1, 3);
                    player.applyPrecisionBoost(4, rollBonus);
                    lines.push_back("- Précision chanceuse : +" + std::to_string(rollBonus) + " aux jets d'attaque.");
                }
                else if (effect == 2)
                {
                    const int guard = random.between(12, 22);
                    player.applyGuardBoost(4, guard);
                    lines.push_back("- Protection chanceuse : dégâts reçus -" + std::to_string(guard) + "%.");
                }
                else if (effect == 3)
                {
                    const int ward = random.between(18, 30);
                    player.applyElementalWard(4, ward);
                    lines.push_back("- Voile favorable : résistance élémentaire +" + std::to_string(ward) + "%.");
                }
                else if (effect == 4)
                {
                    const int regeneration = std::max(2, player.getMaxHp() * random.between(4, 7) / 100);
                    player.applyRegeneration(3, regeneration);
                    lines.push_back("- Régénération chanceuse : +" + std::to_string(regeneration) + " PV au début des prochains tours.");
                }
                else
                {
                    int cured = 0;
                    if (player.cureBurning()) cured++;
                    if (player.curePoison()) cured++;
                    if (player.cureBleeding()) cured++;
                    if (player.cureFrost()) cured++;
                    if (player.cureShock()) cured++;
                    if (cured > 0)
                    {
                        lines.push_back("- Coup de chance purificateur : " + std::to_string(cured) + " altération(s) retirée(s).");
                    }
                    else
                    {
                        const int before = player.getHp();
                        player.heal(std::max(1, player.getMaxHp() / 10));
                        lines.push_back("- Aucun mal à retirer : la chance rend plutôt " + std::to_string(player.getHp() - before) + " PV.");
                    }
                }
            }

            showPotionNotice("LUCKY POTION", "combat.potion.lucky.result", lines);
            return true;
        }

        if (!player.hasInfiniteConsumables())
        {
            removeConsumedPotion(player, consumableIndex, potion);
        }

        const int hpBefore = player.getHp();
        int stabilisation = std::max(1, potion.getPower() / 3);
        std::vector<std::string> notes;
        bool curedStatus = applyCurativeStatusEffect(player, potion, notes);
        bool elementalWard = potionNameContains(potion, "voile")
            || potionNameContains(potion, "élémentaire")
            || potionNameContains(potion, "elementaire")
            || potionNameContains(potion, "isolante")
            || potionNameContains(potion, "givre");

        player.heal(stabilisation);
        DefensePostureSystem::enterDefensePosture(player);
        if (elementalWard)
        {
            int wardPower = potionNameContains(potion, "voile") ? potion.getPower() : std::max(8, potion.getPower() / 2);
            player.applyElementalWard(3, wardPower);
        }

        std::vector<std::string> lines;
        lines.push_back(player.getName() + " utilise " + potion.getName() + ".");
        lines.push_back("Stabilisation : +" + std::to_string(stabilisation) + " PV.");
        lines.push_back("PV : " + std::to_string(hpBefore) + " -> " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + ".");
        lines.push_back("Posture défensive immédiate.");
        if (elementalWard)
        {
            lines.push_back("Un voile court rend les altérations élémentaires moins mordantes.");
        }
        for (const std::string& note : notes)
        {
            lines.push_back(note);
        }
        if (!curedStatus)
        {
            lines.push_back("L'effet reste court, mais ton corps gagne assez de stabilité pour tenir.");
        }

        showPotionNotice("POTION DE BUFF", "combat.potion.buff.result", lines);
        return true;
    }

    if (type == ConsumableType::Debuff)
    {
        Entity* debuffTarget = target;

        if (debuffTarget == nullptr && wave != nullptr && wave->hasActiveEnemies())
        {
            debuffTarget = &wave->getActiveEnemy(0);
        }

        if (debuffTarget == nullptr)
        {
            showPotionNotice(
                "AUCUNE CIBLE",
                "combat.potion.debuff.no_target",
                {"Aucune cible n'est assez proche pour recevoir cette fiole.", "Choisis une autre action ou une cible valide."}
            );
            return false;
        }


        if (potionNameContains(potion, "unlucky potion"))
        {
            if (!player.hasInfiniteConsumables())
            {
                removeConsumedPotion(player, consumableIndex, potion);
            }

            if (wave != nullptr && random.between(1, 100) <= 10)
            {
                for (int i = 1; i <= 3; ++i)
                {
                    wave->addWaitingEnemy(createWeakUnluckyEnemy(player, random, i));
                }
                wave->initializeFrontLine();

                showPotionNotice(
                    "UNLUCKY POTION",
                    "combat.potion.unlucky.spawn",
                    {
                        player.getName() + " lance l'Unlucky Potion sur " + debuffTarget->getName() + ".",
                        "La fiole éclate dans le mauvais sens.",
                        "Au lieu de maudire uniquement la cible, elle attire exactement trois ennemis faibles.",
                        "Ils rejoignent la première ligne ou attendent qu'une place se libère."
                    }
                );
                return true;
            }

            const int effectCount = random.between(3, 5);
            const std::vector<int> effects = chooseDistinctEffectIndexes(random, 7, effectCount);
            std::vector<std::string> lines = {
                player.getName() + " lance l'Unlucky Potion sur " + debuffTarget->getName() + ".",
                "La cible reçoit " + std::to_string(effectCount) + " effets négatifs aléatoires.",
                "Durée annoncée : trois tours."
            };

            const int statusDamage = std::max(2, potion.getPower() / 10);
            for (int effect : effects)
            {
                if (effect == 0)
                {
                    ElementalAffinitySystem::applyBurning(*debuffTarget, 3, statusDamage);
                    lines.push_back("- Brûlure persistante.");
                }
                else if (effect == 1)
                {
                    ElementalAffinitySystem::applyPoison(*debuffTarget, 3, statusDamage);
                    lines.push_back("- Poison persistant.");
                }
                else if (effect == 2)
                {
                    ElementalAffinitySystem::applyBleeding(*debuffTarget, 3, std::max(1, statusDamage - 1));
                    lines.push_back("- Saignement persistant.");
                }
                else if (effect == 3)
                {
                    ElementalAffinitySystem::applyFrost(*debuffTarget, 4);
                    lines.push_back("- Givre et gestes ralentis.");
                }
                else if (effect == 4)
                {
                    ElementalAffinitySystem::applyShock(*debuffTarget, 4);
                    lines.push_back("- Choc électrique perturbant.");
                }
                else if (effect == 5)
                {
                    debuffTarget->applyWeakening(4, random.between(16, 26));
                    lines.push_back("- Affaiblissement des dégâts infligés.");
                }
                else
                {
                    debuffTarget->applyVulnerability(4, random.between(14, 22));
                    lines.push_back("- Faille augmentant les dégâts reçus.");
                }
            }

            showPotionNotice("UNLUCKY POTION", "combat.potion.unlucky.result", lines);
            return true;
        }

        if (!player.hasInfiniteConsumables())
        {
            removeConsumedPotion(player, consumableIndex, potion);
        }

        int hpBefore = debuffTarget->getHp();
        int rawDamage = potion.getPower();
        DamageReport report = DamageSystem::calculateReceivedDamage(*debuffTarget, rawDamage);
        DamageSystem::displayDamageReport(*debuffTarget, report);
        debuffTarget->takeDamage(report.receivedDamage);
        bool fragilityPotion = potionNameContains(potion, "fragilis") || potionNameContains(potion, "faille");
        if (fragilityPotion)
        {
            debuffTarget->applyVulnerability(3, 14 + std::max(0, potion.getPower() / 14));
        }
        else
        {
            debuffTarget->applyWeakening(3, 12 + std::max(0, potion.getPower() / 12));
        }

        std::vector<std::string> lines;
        lines.push_back(player.getName() + " lance " + potion.getName() + " sur " + debuffTarget->getName() + ".");
        lines.push_back("PV cible : " + std::to_string(hpBefore) + " -> " + std::to_string(debuffTarget->getHp()) + "/" + std::to_string(debuffTarget->getMaxHp()) + ".");
        lines.push_back("Dégâts reçus : " + std::to_string(report.receivedDamage) + ".");
        lines.push_back(fragilityPotion
            ? "La cible garde une faille ouverte pendant plusieurs tours."
            : "La cible est affaiblie pendant plusieurs tours.");

        showPotionNotice("POTION DE DEBUFF", "combat.potion.debuff.result", lines);

        if (wave != nullptr)
        {
            wave->removeDeadAndReplace();
        }

        return true;
    }

    if (type == ConsumableType::Special)
    {
        bool isScroll = potionNameContains(potion, "parchemin");

        if (isScroll)
        {
            Entity* spellTarget = target;
            if (spellTarget == nullptr && wave != nullptr && wave->hasActiveEnemies())
            {
                spellTarget = &wave->getActiveEnemy(0);
            }

            bool defensiveScroll = potionNameContains(potion, "voile");
            bool purificationScroll = potionNameContains(potion, "purification");
            bool selfTargetScroll = defensiveScroll || purificationScroll;
            if (!selfTargetScroll && spellTarget == nullptr)
            {
                showPotionNotice(
                    "AUCUNE CIBLE",
                    "combat.potion.scroll.no_target",
                    {"Aucune cible n'est assez proche pour recevoir le sort du parchemin.", "Garde-le pour une situation plus lisible."}
                );
                return false;
            }

            if (!player.hasInfiniteConsumables())
            {
                removeConsumedPotion(player, consumableIndex, potion);
            }

            std::vector<std::string> scrollLines;
            if (!resolveScrollStability(player, potion, random, scrollLines))
            {
                showPotionNotice("PARCHEMIN INSTABLE", "combat.potion.scroll.failed", scrollLines);
                return true;
            }

            if (purificationScroll)
            {
                int cured = 0;
                if (player.cureBurning()) cured++;
                if (player.curePoison()) cured++;
                if (player.cureFrost()) cured++;
                if (player.cureShock()) cured++;
                if (player.cureBleeding()) cured++;

                if (cured == 0)
                {
                    player.applyElementalWard(2, std::max(8, potion.getPower() / 3));
                    scrollLines.push_back("Les runes ne trouvent aucun mal à arracher, alors elles laissent une protection faible.");
                }
                else
                {
                    scrollLines.push_back("La purification arrache " + std::to_string(cured) + " altération(s) au corps.");
                }
                showPotionNotice("PARCHEMIN DE PURIFICATION", "combat.potion.scroll.purification", scrollLines);
                return true;
            }

            if (defensiveScroll)
            {
                player.applyElementalWard(3, potion.getPower());
                DefensePostureSystem::enterDefensePosture(player);
                scrollLines.push_back("Le voile s'accroche au corps : protection élémentaire courte et posture défensive.");
                showPotionNotice("PARCHEMIN DÉFENSIF", "combat.potion.scroll.defense", scrollLines);
                return true;
            }

            if (potionNameContains(potion, "braise"))
            {
                int hpBefore = spellTarget->getHp();
                DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, std::max(6, potion.getPower() - 4));
                DamageSystem::displayDamageReport(*spellTarget, report);
                spellTarget->takeDamage(report.receivedDamage);
                ElementalAffinitySystem::applyBurning(*spellTarget, 3, 2 + potion.getPower() / 26);

                if (random.between(1, 100) <= 18)
                {
                    ElementalAffinitySystem::applyBurning(player, 1, 1);
                    scrollLines.push_back("La braise revient lécher la main qui l'a libérée.");
                }

                scrollLines.push_back("La braise errante mord " + spellTarget->getName() + ", puis cherche une autre faim.");
                scrollLines.push_back("PV cible : " + std::to_string(hpBefore) + " -> " + std::to_string(spellTarget->getHp()) + "/" + std::to_string(spellTarget->getMaxHp()) + ".");
                scrollLines.push_back("Dégâts reçus : " + std::to_string(report.receivedDamage) + ".");
                showPotionNotice("PARCHEMIN DE BRAISE", "combat.potion.scroll.ember", scrollLines);
                if (wave != nullptr) wave->removeDeadAndReplace();
                return true;
            }

            if (potionNameContains(potion, "venin"))
            {
                int hpBefore = spellTarget->getHp();
                DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, std::max(5, potion.getPower() / 3));
                DamageSystem::displayDamageReport(*spellTarget, report);
                spellTarget->takeDamage(report.receivedDamage);
                ElementalAffinitySystem::applyPoison(*spellTarget, 4, 2 + potion.getPower() / 24);
                spellTarget->applyWeakening(2, 10 + std::max(1, potion.getPower() / 16));
                if (random.between(1, 100) <= 25)
                {
                    ElementalAffinitySystem::applyBleeding(*spellTarget, 1, 1);
                    scrollLines.push_back("Le venin trouve une ouverture et laisse une trace rouge sombre.");
                }
                scrollLines.push_back("Le venin rampant mord " + spellTarget->getName() + " et fatigue son prochain échange.");
                scrollLines.push_back("PV cible : " + std::to_string(hpBefore) + " -> " + std::to_string(spellTarget->getHp()) + "/" + std::to_string(spellTarget->getMaxHp()) + ".");
                scrollLines.push_back("Dégâts reçus : " + std::to_string(report.receivedDamage) + ".");
                showPotionNotice("PARCHEMIN DE VENIN", "combat.potion.scroll.venom", scrollLines);
                if (wave != nullptr) wave->removeDeadAndReplace();
                return true;
            }

            if (potionNameContains(potion, "faille"))
            {
                int hpBefore = spellTarget->getHp();
                DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, std::max(8, potion.getPower() / 2));
                DamageSystem::displayDamageReport(*spellTarget, report);
                spellTarget->takeDamage(report.receivedDamage);
                spellTarget->applyVulnerability(3, 14 + std::max(1, potion.getPower() / 10));
                int element = random.between(1, 3);
                if (element == 1) ElementalAffinitySystem::applyBurning(*spellTarget, 2, 2 + potion.getPower() / 24);
                else if (element == 2) ElementalAffinitySystem::applyFrost(*spellTarget, 2);
                else ElementalAffinitySystem::applyShock(*spellTarget, 1);
                scrollLines.push_back("La faille mord la défense de " + spellTarget->getName() + ".");
                scrollLines.push_back("PV cible : " + std::to_string(hpBefore) + " -> " + std::to_string(spellTarget->getHp()) + "/" + std::to_string(spellTarget->getMaxHp()) + ".");
                scrollLines.push_back("Dégâts reçus : " + std::to_string(report.receivedDamage) + ".");
                showPotionNotice("PARCHEMIN DE FAILLE", "combat.potion.scroll.rift", scrollLines);
                if (wave != nullptr) wave->removeDeadAndReplace();
                return true;
            }

            int hpBefore = spellTarget->getHp();
            DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, potion.getPower());
            DamageSystem::displayDamageReport(*spellTarget, report);
            spellTarget->takeDamage(report.receivedDamage);
            if (random.between(1, 100) <= 45) ElementalAffinitySystem::applyShock(*spellTarget, 1);
            else ElementalAffinitySystem::applyBurning(*spellTarget, 2, 2);
            scrollLines.push_back("L'étincelle arcanique frappe " + spellTarget->getName() + ".");
            scrollLines.push_back("PV cible : " + std::to_string(hpBefore) + " -> " + std::to_string(spellTarget->getHp()) + "/" + std::to_string(spellTarget->getMaxHp()) + ".");
            scrollLines.push_back("Dégâts reçus : " + std::to_string(report.receivedDamage) + ".");
            showPotionNotice("PARCHEMIN ARCANISTE", "combat.potion.scroll.arcane", scrollLines);
            if (wave != nullptr) wave->removeDeadAndReplace();
            return true;
        }

        if (potionNameContains(potion, "fumée") || potionNameContains(potion, "fumee"))
        {
            if (!player.hasInfiniteConsumables())
            {
                removeConsumedPotion(player, consumableIndex, potion);
            }

            DefensePostureSystem::enterDefensePosture(player);
            player.clearProvocation();
            showPotionNotice(
                "FUMÉE TACTIQUE",
                "combat.potion.smoke.result",
                {
                    player.getName() + " brise une " + potion.getName() + ".",
                    "La fumée ne téléporte pas et ne garantit pas la fuite, mais elle casse la pression immédiate.",
                    "Effet actuel : posture défensive et provocation annulée."
                }
            );
            return true;
        }
    }

    showPotionNotice(
        "ACTION INACCESSIBLE",
        "combat.potion.unavailable",
        {"Cette option est inaccessible dans ce combat.", "Choisis une action compatible avec la situation actuelle."}
    );
    return false;
}
