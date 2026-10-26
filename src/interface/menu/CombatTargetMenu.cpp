// EN: CombatTargetMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatTargetMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatTargetMenu.hpp"

#include "combat/TurnManager.hpp"
#include "combat/CombatActions.hpp"
#include "combat/action/CombatAttack.hpp"
#include "combat/system/ObservationSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "progression/TechniqueProgressionRules.hpp"

#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"

#include <string>
#include <vector>
#include <algorithm>

namespace
{
    // EN: findForcedTargetIndex declares or implements a focused behavior used by this module.
    // FR: findForcedTargetIndex déclare ou implémente un comportement précis utilisé par ce module.
    int findForcedTargetIndex(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& enemy = wave.getActiveEnemy(i);

            if (enemy.isProvoking() || enemy.hasHealingThreat())
            {
                return i;
            }
        }

        return -1;
    }

    MenuOptionItemData buildEnemyTargetItemData(const Monster& enemy)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = enemy.isInvocation() ? "summon" : "enemy";
        itemData.section = "Cibles";
        itemData.actionType = "target";
        itemData.name = enemy.getName();
        itemData.status = "PV : " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp());
        itemData.owner = enemy.getRaceText();

        if (enemy.isInvocation())
        {
            itemData.detail = "Invocation ennemie active";
        }
        else if (enemy.isEvolved())
        {
            itemData.detail = "Variation évoluée";
        }
        else if (enemy.isElite())
        {
            itemData.detail = "Élite";
        }
        else
        {
            itemData.detail = "Cible active";
        }

        if (enemy.isProvoking())
        {
            itemData.progress = "Provocation active";
            itemData.important = true;
        }
        else if (enemy.hasHealingThreat())
        {
            itemData.progress = "Soigneur marqué";
            itemData.important = true;
        }
        else
        {
            itemData.important = enemy.getMaxHp() > 0 && enemy.getHp() * 100 <= enemy.getMaxHp() * 35;
        }

        return itemData;
    }


    MenuOptionItemData buildSelectedTargetActionData(
        const Monster& target,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = target.isInvocation() ? "summon" : "enemy";
        itemData.section = "Cible sélectionnée";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = "PV : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp());
        itemData.owner = target.getName();

        if (target.isProvoking())
        {
            itemData.progress = "Provocation active";
            itemData.important = true;
        }
        else if (target.hasHealingThreat())
        {
            itemData.progress = "Soigneur marqué";
            itemData.important = true;
        }
        else
        {
            itemData.important = important;
        }

        return itemData;
    }
}

bool CombatTargetMenu::openForAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    std::string* resolvedTargetName,
    std::string* resolvedTargetId,
    bool* targetForcedByCombatRules
)
{
    if (resolvedTargetName != nullptr) resolvedTargetName->clear();
    if (resolvedTargetId != nullptr) resolvedTargetId->clear();
    if (targetForcedByCombatRules != nullptr) *targetForcedByCombatRules = false;
    return openTargetMenu(
        player,
        wave,
        random,
        false,
        0,
        resolvedTargetName,
        resolvedTargetId,
        targetForcedByCombatRules
    );
}

bool CombatTargetMenu::openForDamagePotion(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int potionDamageBonus
)
{
    return openForBoostedAttack(
        player,
        wave,
        random,
        potionDamageBonus
    );
}

bool CombatTargetMenu::openForBoostedAttack(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    int damageBonus
)
{
    return openTargetMenu(
        player,
        wave,
        random,
        true,
        damageBonus
    );
}

bool CombatTargetMenu::openForClassSkill(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    std::string* resolvedTargetName,
    std::string* resolvedTargetId
)
{
    if (resolvedTargetName != nullptr) resolvedTargetName->clear();
    if (resolvedTargetId != nullptr) resolvedTargetId->clear();
    if (!wave.hasActiveEnemies())
    {
        MessageScreen::show(
            "COMPÉTENCE DE VAGUE",
            "combat.wave.class_skill.no_target",
            {"Aucun adversaire actif ne peut recevoir une compétence."},
            false
        );
        return false;
    }

    if (!player.isClassSkillReady())
    {
        MessageScreen::show(
            "COMPÉTENCE INDISPONIBLE",
            "combat.wave.class_skill.cooldown_locked",
            {
                "Les compétences actives ont une récupération réelle.",
                "Récupération restante : " + std::to_string(player.getClassSkillCooldownTurns()) + " tour(s)."
            },
            false
        );
        return false;
    }

    const int level = player.getLevel();
    const bool rangedWeapon = player.hasEquippedWeapon() && player.getEquippedWeapon().getType() == WeaponType::Bow;
    const bool magicProfile = player.getMagicChannelingStyleLabel() != "Réserve mineure";
    const bool rangedAmmunitionAvailable = !rangedWeapon || CombatActions::hasCompatibleAmmunition(player);
    const int chainUnlockLevel = TechniqueProgressionRules::chainUnlockLevel(magicProfile);
    const int tripleChainLevel = TechniqueProgressionRules::tripleChainUnlockLevel();
    const int quadrupleChainLevel = TechniqueProgressionRules::quadrupleChainUnlockLevel();
    const int battlefieldMasteryLevel = TechniqueProgressionRules::genericMasteryUnlockLevel("battlefield_mastery");
    const int veteranRallyLevel = TechniqueProgressionRules::genericMasteryUnlockLevel("veteran_rally");
    const int masterExecutionLevel = TechniqueProgressionRules::genericMasteryUnlockLevel("master_execution");
    const int perfectGuardBreakLevel = TechniqueProgressionRules::genericMasteryUnlockLevel("perfect_guard_break");
    const int firstTechniqueLevel = TechniqueProgressionRules::genericTechniqueUnlockLevel("safe_rally");
    const std::string chainName = rangedWeapon
        ? (level >= quadrupleChainLevel ? "Quadruple tir" : (level >= tripleChainLevel ? "Triple tir" : "Double tir"))
        : (magicProfile
            ? (level >= quadrupleChainLevel ? "Résonance canalisée" : "Écho canalisé")
            : (level >= quadrupleChainLevel ? "Quadruple coup" : (level >= tripleChainLevel ? "Triple coup" : "Double coup")));

    if (level < firstTechniqueLevel)
    {
        MessageScreen::show(
            "TECHNIQUES À APPRENDRE",
            "combat.wave.class_skill.none_unlocked",
            {
                player.getName() + " n'a pas encore assez de pratique pour une technique active fiable.",
                "Ralliement prudent, geste simple de survie, devient accessible au niveau " + std::to_string(firstTechniqueLevel) + ".",
                "Le niveau maximal est 255 : les techniques fortes sont désormais étalées sur une vraie progression au lieu d'être concentrées au début."
            },
            false
        );
        return false;
    }

    MenuScreen screen("COMPÉTENCE ACTIVE", "combat.wave.class_skill.selector");
    screen.addSubtitle("Techniques réellement maîtrisées par " + player.getName());
    screen.addLine("Les techniques se débloquent progressivement. Une technique absente de cette liste n'est pas encore maîtrisée.");
    screen.addLine("Toutes consomment le tour et déclenchent un cooldown.");
    if (level >= quadrupleChainLevel)
    {
        screen.addLine("Maîtrise tardive : les enchaînements universels ont atteint leur quatrième impulsion.");
    }
    if (level >= perfectGuardBreakLevel)
    {
        screen.addLine("Maîtrise de niveau 240 : les techniques de rupture les plus anciennes ont atteint leur forme experte.");
    }
    screen.addBackOption("Retour", "combat.wave.class_skill.back");
    if (level >= chainUnlockLevel) screen.addOption(1, chainName, rangedWeapon
        ? "2 tirs, 3 au niveau " + std::to_string(tripleChainLevel) + ", puis 4 au niveau " + std::to_string(quadrupleChainLevel) + ". Chaque tir consomme sa propre munition ; la salve s'arrête si les projectiles manquent. Recharge 3 tours." + (rangedAmmunitionAvailable ? "" : " Aucune munition compatible disponible.")
        : (magicProfile
            ? "Deux impulsions, trois au niveau " + std::to_string(tripleChainLevel) + ", puis quatre au niveau " + std::to_string(quadrupleChainLevel) + " : coût, support et risque suivent toujours ta voie magique. Recharge 3 tours."
            : "2 coups, 3 au niveau " + std::to_string(tripleChainLevel) + ", puis 4 au niveau " + std::to_string(quadrupleChainLevel) + ". Recharge 3 tours."),
        rangedAmmunitionAvailable, "combat.wave.skill.chain_target");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("line_sweep")) screen.addOption(2,
        level >= battlefieldMasteryLevel ? (rangedWeapon ? "Balayage de tirs maîtrisé" : "Balayage de ligne maîtrisé") : (rangedWeapon ? "Balayage de tirs" : "Balayage de ligne"), "Frappe tous les ennemis actifs, dégâts plus faibles par cible." + std::string(rangedWeapon ? " Chaque cible demande une munition ; le balayage s'arrête si le carquois se vide." : "") + " Recharge 4 tours." + (rangedAmmunitionAvailable ? "" : " Aucune munition compatible disponible."), rangedAmmunitionAvailable, "combat.wave.skill.line_sweep");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("random_ricochet")) screen.addOption(3, rangedWeapon ? "Salve imprévisible" : "Ricochets imprévisibles", "3 frappes sur des ennemis actifs aléatoires. Peut retomber sur la même cible." + std::string(rangedWeapon ? " Chaque projectile est réel et la salve s'arrête si les munitions manquent." : "") + " Recharge 4 tours." + (rangedAmmunitionAvailable ? "" : " Aucune munition compatible disponible."), rangedAmmunitionAvailable, "combat.wave.skill.random_ricochet");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("guard_break")) screen.addOption(4, level >= perfectGuardBreakLevel ? "Brise-garde parfait" : "Brise-garde", "Choisis une cible : dégâts moyens, affaiblissement et vulnérabilité. Recharge 3 tours.", true, "combat.wave.skill.guard_break");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("safe_rally")) screen.addOption(5, level >= veteranRallyLevel ? "Ralliement vétéran" : "Ralliement prudent", "Ne cible personne : récupère un peu, passe en garde et prépare l'équipe. Recharge 4 tours.", true, "combat.wave.skill.safe_rally");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("environment_move")) screen.addOption(6, "Manœuvre d'environnement", "Utilise le terrain/la torche/la poussière : contrôle léger sur une cible ou sur la ligne. Recharge 4 tours.", true, "combat.wave.skill.environment_move");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("stop_hit")) screen.addOption(7, rangedWeapon ? "Tir d'arrêt" : "Coup d'arrêt", "Choisis une cible : faibles dégâts et affaiblissement pour casser son prochain rythme. Recharge 3 tours.", true, "combat.wave.skill.stop_hit");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("opportunist_execute")) screen.addOption(8, level >= masterExecutionLevel ? "Exécution maîtresse" : "Exécution opportuniste", "Choisis une cible : très forte si elle est blessée ou déjà altérée, sinon pose une ouverture. Recharge 4 tours.", true, "combat.wave.skill.opportunist_execute");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("unstable_mix")) screen.addOption(9, "Mélange instable", "Lance deux fioles improvisées sur des cibles aléatoires : poison, choc, brûlure ou saignement. Recharge 5 tours.", true, "combat.wave.skill.unstable_mix");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("line_break")) screen.addOption(10, "Rupture de ligne", "Choisis une cible : frappe principale puis impact secondaire sur une autre cible si possible. Recharge 4 tours.", true, "combat.wave.skill.line_break");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("field_suture")) screen.addOption(11, "Suture de fortune", "Ne cible personne : soin immédiat modéré + régénération courte, jamais un full heal. Recharge 5 tours.", true, "combat.wave.skill.field_suture");
    if (level >= TechniqueProgressionRules::genericTechniqueUnlockLevel("focus_signal")) screen.addOption(12, "Signal de focus", "Choisis une cible : peu de dégâts, mais grosse ouverture pour les alliés et prochaines frappes. Recharge 2 tours.", true, "combat.wave.skill.focus_signal");

    int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    auto finishCooldown = [&](int turns) {
        player.startClassSkillCooldown(turns);
        MessageScreen::show(
            "RÉCUPÉRATION",
            "combat.wave.class_skill.cooldown_started",
            {
                "Compétence active utilisée.",
                "Récupération restante : " + std::to_string(turns) + " tour(s)."
            },
            false
        );
    };

    if (choice == 1)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        if (resolvedTargetName != nullptr) *resolvedTargetName = wave.getActiveEnemy(targetIndex).getName();
        if (resolvedTargetId != nullptr) *resolvedTargetId = wave.getActiveEnemy(targetIndex).getCombatInstanceId();

        MessageScreen::show(
            chainName,
            "combat.wave.class_skill.chain_target.start",
            {
                player.getName() + (rangedWeapon ? " verrouille sa ligne de tir et prépare plusieurs projectiles." : (magicProfile ? " retient son souffle et force plusieurs impulsions dans la même canalisation." : " verrouille une cible et prépare plusieurs frappes courtes.")),
                rangedWeapon ? "Chaque impact demande une munition réelle." : (magicProfile ? "La répétition magique dépense du mana : elle n'est pas l'équivalent gratuit d'un Triple coup." : "L'enchaînement devient triple seulement avec assez de maîtrise.")
            },
            false
        );

        int strikes = player.getLevel() >= quadrupleChainLevel ? 4 : (player.getLevel() >= tripleChainLevel ? 3 : 2);
        if (magicProfile)
        {
            // A magical chain is a real spell sequence, not a renamed martial multi-hit.
            // The base pressure is intentionally high, then each casting path applies its own
            // material/catalyst/incantation discount or surcharge through CombatActions.
            const int baseManaCost = strikes >= 4 ? 40 : (strikes == 3 ? 28 : 18);
            if (!CombatActions::prepareMagicTechniqueInteractive(player, "Écho canalisé", random, baseManaCost))
            {
                MessageScreen::show(
                    "ÉCHO INTERROMPU",
                    "combat.wave.class_skill.chain_target.interrupted",
                    {
                        "L'enchaînement magique n'a pas réussi à former toutes ses impulsions.",
                        "La tentative a tout de même cassé le rythme de canalisation."
                    },
                    false
                );
                finishCooldown(std::max(2, 2 + player.getMagicRecoveryAdjustment()));
                return true;
            }
        }

        for (int strike = 1; strike <= strikes; ++strike)
        {
            if (rangedWeapon && !CombatActions::hasCompatibleAmmunition(player))
            {
                MessageScreen::show(
                    "SALVE INTERROMPUE",
                    "combat.wave.class_skill.chain_target.no_ammo",
                    {"Plus aucune munition compatible : les projectiles restants ne sont pas remplacés par des coups de crosse."},
                    false
                );
                break;
            }
            if (!wave.isActiveIndexValid(targetIndex))
            {
                break;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            if (target.isDead())
            {
                break;
            }

            MessageScreen::show(
                "COUP " + std::to_string(strike) + "/" + std::to_string(strikes),
                "combat.wave.class_skill.chain_target.hit",
                {"Cible : " + target.getName() + "."},
                false
            );
            CombatActions::executeBoostedAttack(player, target, random, -2 + player.getLevel() / 12);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
        }

        finishCooldown(magicProfile
            ? std::max(2, 3 + player.getMagicRecoveryAdjustment())
            : 3);
        return true;
    }

    if (choice == 2)
    {
        const int originalActiveCount = wave.getActiveEnemyCount();
        MessageScreen::show(
            "BALAYAGE DE LIGNE",
            "combat.wave.class_skill.line_sweep.start",
            {
                player.getName() + " attaque la première ligne au lieu de tunnel une seule cible.",
                "Les ennemis actifs sont touchés une fois, avec des dégâts réduits."
            },
            false
        );

        for (int i = 0; i < originalActiveCount && i < wave.getActiveEnemyCount(); ++i)
        {
            if (rangedWeapon && !CombatActions::hasCompatibleAmmunition(player))
            {
                MessageScreen::show("BALAYAGE INTERROMPU", "combat.wave.class_skill.line_sweep.no_ammo", {"Le carquois ne contient plus de projectile compatible : le balayage s'arrête proprement."}, false);
                break;
            }
            Monster& target = wave.getActiveEnemy(i);
            if (target.isDead())
            {
                continue;
            }
            const int masteryBonus = player.getLevel() >= battlefieldMasteryLevel ? 4 : 0;
            CombatActions::executeBoostedAttack(player, target, random, -6 + masteryBonus + player.getLevel() / 18);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
        }

        finishCooldown(player.getLevel() >= battlefieldMasteryLevel ? 3 : 4);
        return true;
    }

    if (choice == 3)
    {
        MessageScreen::show(
            "RICOCHETS IMPRÉVISIBLES",
            "combat.wave.class_skill.random_ricochet.start",
            {
                player.getName() + " laisse le geste rebondir entre les ouvertures disponibles.",
                "Trois frappes partent sur des cibles actives aléatoires."
            },
            false
        );

        for (int strike = 1; strike <= 3 && wave.hasActiveEnemies(); ++strike)
        {
            if (rangedWeapon && !CombatActions::hasCompatibleAmmunition(player))
            {
                MessageScreen::show("SALVE INTERROMPUE", "combat.wave.class_skill.random_ricochet.no_ammo", {"Plus aucune munition compatible : la salve s'arrête au lieu de transformer les tirs restants en coups de crosse."}, false);
                break;
            }
            const int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
            Monster& target = wave.getActiveEnemy(targetIndex);
            MessageScreen::show(
                "RICOCHET " + std::to_string(strike) + "/3",
                "combat.wave.class_skill.random_ricochet.hit",
                {"La frappe part vers : " + target.getName() + "."},
                false
            );
            CombatActions::executeBoostedAttack(player, target, random, -4 + player.getLevel() / 16);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
        }

        finishCooldown(4);
        return true;
    }


    if (choice == 4)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        if (resolvedTargetName != nullptr) *resolvedTargetName = targetName;
        if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
        MessageScreen::show(
            "BRISE-GARDE",
            "combat.wave.class_skill.guard_break.start",
            {
                player.getName() + " cherche à casser le rythme de " + targetName + ".",
                "Cette compétence ne sert pas seulement à taper : elle ouvre une fenêtre tactique pour le groupe."
            },
            false
        );
        const int ruptureDuration = player.getLevel() >= perfectGuardBreakLevel ? 3 : 2;
        target.applyWeakening(ruptureDuration, 10 + player.getLevel() / 10);
        target.applyVulnerability(ruptureDuration, 12 + player.getLevel() / 12);
        CombatActions::executeBoostedAttack(player, target, random, -3 + player.getLevel() / 18);
        ThreatSystem::consumeForcedTargetIfNeeded(target);
        wave.removeDeadAndReplace();
        player.recordCanonicalEvent("competences_actives_utilisees", "Brise-garde", "Affaiblissement/vulnérabilité posés sur " + targetName, 1);
        finishCooldown(player.getLevel() >= perfectGuardBreakLevel ? 2 : 3);
        return true;
    }

    if (choice == 5)
    {
        const int healAmount = std::max(4, player.getMaxHp() / 9 + player.getLevel() / 5);
        const int hpBefore = player.getHp();
        if (player.getHp() < player.getMaxHp())
        {
            player.heal(healAmount);
        }
        const bool veteranRally = player.getLevel() >= veteranRallyLevel;
        player.startDefensePosture(veteranRally ? 18 : 14, veteranRally ? 7 : 5, veteranRally ? "Ralliement vétéran" : "Ralliement prudent");
        if (veteranRally)
        {
            player.applyRegeneration(2, std::max(2, player.getMaxHp() / 40));
        }
        MessageScreen::show(
            veteranRally ? "RALLIEMENT VÉTÉRAN" : "RALLIEMENT PRUDENT",
            "combat.wave.class_skill.safe_rally.start",
            {
                player.getName() + " recule d'un pas, reprend son souffle et donne un signal clair.",
                "PV récupérés : " + std::to_string(std::max(0, player.getHp() - hpBefore)) + ".",
                veteranRally
                    ? "Maîtrise vétéran : garde renforcée et courte régénération pendant 2 tours."
                    : "Garde courte active : dégâts subis réduits et petite chance de contre."
            },
            false
        );
        player.recordCanonicalEvent("competences_actives_utilisees", "Ralliement prudent", "Soin léger + garde courte", 1);
        finishCooldown(4);
        return true;
    }

    if (choice == 6)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        if (resolvedTargetName != nullptr) *resolvedTargetName = targetName;
        if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
        const int controlRoll = random.between(1, 3);
        std::vector<std::string> lines;
        lines.push_back(player.getName() + " exploite ce qu'il a sous la main au lieu d'attaquer bêtement.");
        if (controlRoll == 1)
        {
            target.applyWeakening(2, 8 + player.getLevel() / 14);
            lines.push_back("Poussière, recul, déséquilibre : " + targetName + " est affaibli pendant 2 tours.");
        }
        else if (controlRoll == 2)
        {
            target.applyVulnerability(2, 9 + player.getLevel() / 16);
            lines.push_back("Une ouverture est forcée : " + targetName + " devient plus vulnérable pendant 2 tours.");
        }
        else
        {
            const int originalActiveCount = wave.getActiveEnemyCount();
            for (int i = 0; i < originalActiveCount && i < wave.getActiveEnemyCount(); ++i)
            {
                Monster& lineTarget = wave.getActiveEnemy(i);
                lineTarget.applyWeakening(1, 6 + player.getLevel() / 18);
            }
            lines.push_back("La ligne ennemie perd un peu son rythme pendant 1 tour.");
        }
        MessageScreen::show("MANŒUVRE D'ENVIRONNEMENT", "combat.wave.class_skill.environment_move.start", lines, false);
        CombatActions::executeBoostedAttack(player, target, random, -6 + player.getLevel() / 22);
        ThreatSystem::consumeForcedTargetIfNeeded(target);
        wave.removeDeadAndReplace();
        player.recordCanonicalEvent("competences_actives_utilisees", "Manœuvre d'environnement", "Contrôle de terrain utilisé contre " + targetName, 1);
        finishCooldown(4);
        return true;
    }


    if (choice == 7)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        if (resolvedTargetName != nullptr) *resolvedTargetName = targetName;
        if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
        MessageScreen::show(
            "COUP D'ARRÊT",
            "combat.wave.class_skill.stop_hit.start",
            {
                player.getName() + " ne cherche pas le gros dégât : il cherche à casser le prochain mouvement de " + targetName + ".",
                "Contrôle court : affaiblissement + choc ou givre selon l'ouverture."
            },
            false
        );
        target.applyWeakening(2, 12 + player.getLevel() / 14);
        if (random.between(1, 100) <= 55)
        {
            ElementalAffinitySystem::applyShock(target, 1 + (player.getLevel() >= 25 ? 1 : 0));
        }
        else
        {
            ElementalAffinitySystem::applyFrost(target, 1 + (player.getLevel() >= 30 ? 1 : 0));
        }
        CombatActions::executeBoostedAttack(player, target, random, -7 + player.getLevel() / 24);
        ThreatSystem::consumeForcedTargetIfNeeded(target);
        wave.removeDeadAndReplace();
        player.recordCanonicalEvent("competences_actives_utilisees", "Coup d'arrêt", "Contrôle court posé sur " + targetName, 1);
        finishCooldown(3);
        return true;
    }

    if (choice == 8)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        if (resolvedTargetName != nullptr) *resolvedTargetName = targetName;
        if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
        const int executionThreshold = player.getLevel() >= masterExecutionLevel ? 50 : 40;
        const bool weakenedTarget = target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * executionThreshold;
        const bool alteredTarget = target.hasPoison() || target.hasBleeding() || target.hasBurning() || target.hasFrost() || target.hasShock() || target.hasWeakening() || target.hasVulnerability();
        const int masteryExecutionBonus = player.getLevel() >= masterExecutionLevel ? 4 : 0;
        const int bonus = (weakenedTarget || alteredTarget) ? 8 + masteryExecutionBonus + player.getLevel() / 10 : -4 + player.getLevel() / 30;
        std::vector<std::string> lines;
        lines.push_back(player.getName() + " cherche une finition, pas une attaque neutre.");
        if (weakenedTarget || alteredTarget)
        {
            lines.push_back(player.getLevel() >= masterExecutionLevel
                ? "La cible est déjà blessée ou altérée : l'exécution maîtresse exploite une fenêtre plus large et frappe plus fort."
                : "La cible est déjà blessée ou altérée : l'exécution opportuniste frappe fort.");
        }
        else
        {
            lines.push_back("La cible n'est pas encore ouverte : la frappe sera moins forte mais laissera une vulnérabilité.");
            target.applyVulnerability(2, 10 + player.getLevel() / 15);
        }
        MessageScreen::show("EXÉCUTION OPPORTUNISTE", "combat.wave.class_skill.opportunist_execute.start", lines, false);
        CombatActions::executeBoostedAttack(player, target, random, bonus);
        if (!target.isDead() && (weakenedTarget || alteredTarget))
        {
            ElementalAffinitySystem::applyBleeding(target, 2, 2 + player.getLevel() / 20);
        }
        ThreatSystem::consumeForcedTargetIfNeeded(target);
        wave.removeDeadAndReplace();
        player.recordCanonicalEvent("competences_actives_utilisees", "Exécution opportuniste", "Frappe conditionnelle sur " + targetName, 1);
        finishCooldown(4);
        return true;
    }

    if (choice == 9)
    {
        MessageScreen::show(
            "MÉLANGE INSTABLE",
            "combat.wave.class_skill.unstable_mix.start",
            {
                player.getName() + " improvise avec deux fioles de terrain.",
                "Les cibles sont aléatoires : cette compétence sert à créer des réactions, pas à contrôler parfaitement le combat."
            },
            false
        );
        for (int vial = 1; vial <= 2 && wave.hasActiveEnemies(); ++vial)
        {
            const int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int roll = random.between(1, 4);
            std::string effect;
            if (roll == 1)
            {
                ElementalAffinitySystem::applyPoison(target, 3, 2 + player.getLevel() / 22);
                effect = "poison";
            }
            else if (roll == 2)
            {
                ElementalAffinitySystem::applyShock(target, 2);
                effect = "choc";
            }
            else if (roll == 3)
            {
                ElementalAffinitySystem::applyBurning(target, 2, 2 + player.getLevel() / 24);
                effect = "brûlure";
            }
            else
            {
                ElementalAffinitySystem::applyBleeding(target, 3, 2 + player.getLevel() / 24);
                effect = "saignement";
            }
            MessageScreen::show(
                "FIOLE " + std::to_string(vial) + "/2",
                "combat.wave.class_skill.unstable_mix.vial",
                {"Cible : " + targetName + ".", "Effet appliqué : " + effect + "."},
                false
            );
            CombatActions::executeBoostedAttack(player, target, random, -8 + player.getLevel() / 28);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
        }
        player.recordCanonicalEvent("competences_actives_utilisees", "Mélange instable", "Deux fioles aléatoires lancées en combat", 1);
        finishCooldown(5);
        return true;
    }

    if (choice == 10)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        if (resolvedTargetName != nullptr) *resolvedTargetName = targetName;
        if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
        MessageScreen::show(
            "RUPTURE DE LIGNE",
            "combat.wave.class_skill.line_break.start",
            {
                player.getName() + " ouvre la ligne ennemie au lieu de tunnel une seule frappe.",
                "La cible principale prend l'impact, puis une autre cible peut être touchée si la ligne tient encore."
            },
            false
        );
        target.applyWeakening(2, 8 + player.getLevel() / 16);
        CombatActions::executeBoostedAttack(player, target, random, -3 + player.getLevel() / 18);
        ThreatSystem::consumeForcedTargetIfNeeded(target);
        wave.removeDeadAndReplace();

        if (wave.getActiveEnemyCount() >= 2)
        {
            int secondaryIndex = random.between(0, wave.getActiveEnemyCount() - 1);
            if (secondaryIndex == targetIndex && wave.getActiveEnemyCount() > 1)
            {
                secondaryIndex = (secondaryIndex + 1) % wave.getActiveEnemyCount();
            }
            Monster& secondary = wave.getActiveEnemy(secondaryIndex);
            const std::string secondaryName = secondary.getName();
            secondary.applyVulnerability(1, 7 + player.getLevel() / 22);
            MessageScreen::show(
                "IMPACT SECONDAIRE",
                "combat.wave.class_skill.line_break.secondary",
                {
                    "La rupture se propage vers : " + secondaryName + ".",
                    "L'impact secondaire est plus léger mais crée une faille courte."
                },
                false
            );
            CombatActions::executeBoostedAttack(player, secondary, random, -9 + player.getLevel() / 30);
            ThreatSystem::consumeForcedTargetIfNeeded(secondary);
            wave.removeDeadAndReplace();
        }

        player.recordCanonicalEvent("competences_actives_utilisees", "Rupture de ligne", "Frappe principale + pression secondaire", 1);
        finishCooldown(4);
        return true;
    }

    if (choice == 11)
    {
        const int hpBefore = player.getHp();
        const int immediateHeal = std::max(5, player.getMaxHp() / 8 + player.getLevel() / 6);
        if (player.getHp() < player.getMaxHp())
        {
            player.heal(immediateHeal);
        }
        const int regenPerTurn = std::max(2, player.getMaxHp() / 30 + player.getLevel() / 18);
        player.applyRegeneration(2, regenPerTurn);
        player.startDefensePosture(8, 3, "Suture de fortune");
        MessageScreen::show(
            "SUTURE DE FORTUNE",
            "combat.wave.class_skill.field_suture.start",
            {
                player.getName() + " arrache quelques secondes pour refermer le pire.",
                "Soin immédiat : +" + std::to_string(std::max(0, player.getHp() - hpBefore)) + " PV.",
                "Régénération courte : " + std::to_string(regenPerTurn) + " PV pendant 2 tours.",
                "Ce n'est pas un soin d'infirmerie : ça stabilise, mais ça ne remplace pas les vrais soins."
            },
            false
        );
        player.recordCanonicalEvent("competences_actives_utilisees", "Suture de fortune", "Soin modéré + régénération courte", 1);
        finishCooldown(5);
        return true;
    }

    if (choice == 12)
    {
        int targetIndex = chooseTarget(wave);
        Console::clear();
        if (targetIndex < 0 || !wave.isActiveIndexValid(targetIndex))
        {
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        if (resolvedTargetName != nullptr) *resolvedTargetName = targetName;
        if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
        target.applyVulnerability(3, 18 + player.getLevel() / 12);
        target.applyWeakening(1, 6 + player.getLevel() / 24);
        MessageScreen::show(
            "SIGNAL DE FOCUS",
            "combat.wave.class_skill.focus_signal.start",
            {
                player.getName() + " désigne une faille claire sur " + targetName + ".",
                "Peu de dégâts directs, mais une vulnérabilité forte reste ouverte pendant 3 tours.",
                "Cette option sert surtout à préparer les alliés, les réactions et les finitions."
            },
            false
        );
        CombatActions::executeBoostedAttack(player, target, random, -10 + player.getLevel() / 35);
        ThreatSystem::consumeForcedTargetIfNeeded(target);
        wave.removeDeadAndReplace();
        player.recordCanonicalEvent("competences_actives_utilisees", "Signal de focus", "Ouverture forte posée sur " + targetName, 1);
        finishCooldown(2);
        return true;
    }

    return false;
}

// EN: chooseTarget declares or implements a focused behavior used by this module.
// FR: chooseTarget déclare ou implémente un comportement précis utilisé par ce module.
int CombatTargetMenu::chooseTarget(const EnemyCombatQueue& wave)
{
    MenuScreen screen("CHOIX DE CIBLE", "combat.target_select");
    screen.addLine("Choisis une cible active.");
    screen.addLine("Les ennemis en attente ne peuvent pas encore être ciblés.");

    for (const std::string& summaryLine : wave.getQueueSummaryLines())
    {
        screen.addFooterLine(summaryLine);
    }

    screen.addBackOption("Retour au menu principal", "combat.target.back");

    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        const Monster& enemy = wave.getActiveEnemy(i);
        std::string hint = std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()) + " PV";

        if (enemy.isInvocation())
        {
            hint += " | invocation";
        }
        else if (enemy.isEvolved())
        {
            hint += " | variation évoluée";
        }
        else if (enemy.isElite())
        {
            hint += " | élite";
        }

        if (enemy.isProvoking())
        {
            hint += " | provocation";
        }

        if (enemy.hasHealingThreat())
        {
            hint += " | soigneur marqué";
        }

        screen.addOption(
            i + 1,
            enemy.getName(),
            hint,
            !enemy.isDead(),
            "combat.target.select." + std::to_string(i),
            buildEnemyTargetItemData(enemy)
        );
    }

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Cible invalide. Choisis une cible affichée, ou 0 pour revenir."
    );

    if (choice == 0)
    {
        return -1;
    }

    return choice - 1;
}

bool CombatTargetMenu::openTargetMenu(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool boostedAttack,
    int damageBonus,
    std::string* resolvedTargetName,
    std::string* resolvedTargetId,
    bool* targetForcedByCombatRules
)
{
    while (wave.hasEnemiesLeft())
    {
        int targetIndex = chooseTarget(wave);

        Console::clear();

        if (targetIndex == -1)
        {
            return false;
        }

        if (!wave.isActiveIndexValid(targetIndex))
        {
            MessageScreen::show(
                "CIBLE INDISPONIBLE",
                "combat.target.unavailable",
                {"Cette cible n'est plus disponible."},
                true
            );
            return false;
        }

        int forcedTargetIndex = findForcedTargetIndex(wave);

        if (forcedTargetIndex >= 0 && forcedTargetIndex != targetIndex)
        {
            const Monster& forcedTarget = wave.getActiveEnemy(forcedTargetIndex);

            std::vector<std::string> lines;

            if (forcedTarget.isProvoking())
            {
                lines.push_back(forcedTarget.getName() + " bloque la ligne.");
                lines.push_back("Sa provocation t'empêche d'ignorer sa présence.");
            }
            else
            {
                lines.push_back(forcedTarget.getName() + " vient de soigner un allié.");
                lines.push_back("Ton attention se fixe sur le soigneur.");
            }

            MessageScreen::show(
                "CIBLE FORCÉE",
                "combat.target.forced",
                lines,
                true
            );
            return false;
        }

        Monster& target = wave.getActiveEnemy(targetIndex);

        bool stayOnThisTarget = true;

        while (stayOnThisTarget && !target.isDead())
        {
            MenuScreen targetScreen("CIBLE SÉLECTIONNÉE", "combat.target_actions");
            targetScreen.addLine("Attaquant : " + player.getName() + " | PV " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
            targetScreen.addLine("Cible : " + target.getName());
            targetScreen.addLine("Race : " + target.getRaceText());
            targetScreen.addLine("PV : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()));

            if (target.isInvocation())
            {
                targetScreen.addLine("Statut : Invocation");
            }
            else if (target.isElite())
            {
                targetScreen.addLine("Statut : Élite");
            }
            else
            {
                targetScreen.addLine("Statut : Ennemi standard");
            }

            if (target.isProvoking())
            {
                targetScreen.addLine("Priorité : provocation active, cette cible attire forcément l'attention.");
            }
            else if (target.hasHealingThreat())
            {
                targetScreen.addLine("Priorité : soigneur marqué, l'aggro tactique est temporairement fixée.");
            }

            for (const std::string& summaryLine : wave.getQueueSummaryLines())
            {
                targetScreen.addFooterLine(summaryLine);
            }

            targetScreen.addOption(
                0,
                "Retour au menu principal",
                "",
                true,
                "combat.target.back"
            );

            if (boostedAttack)
            {
                targetScreen.addOption(
                    1,
                    "Utiliser la potion de rage sur cette cible",
                    "Consomme l'effet préparé et attaque immédiatement.",
                    true,
                    "combat.target.use_rage_potion",
                    buildSelectedTargetActionData(
                        target,
                        "attack",
                        "Potion de rage",
                        "Attaque renforcée sur la cible sélectionnée.",
                        true
                    )
                );
            }
            else
            {
                targetScreen.addOption(
                    1,
                    "Attaquer cette cible",
                    "Consomme l'action du tour.",
                    true,
                    "combat.target.attack",
                    buildSelectedTargetActionData(
                        target,
                        "attack",
                        "Attaquer",
                        "Attaque simple sur la cible sélectionnée.",
                        true
                    )
                );
            }

            targetScreen.addOption(
                2,
                "Inspecter cette cible",
                "Voir les informations de combat connues.",
                true,
                "combat.target.inspect",
                buildSelectedTargetActionData(
                    target,
                    "inspect",
                    "Inspecter",
                    "Affiche les statistiques connues puis revient à cette cible."
                )
            );
            targetScreen.addOption(
                3,
                "Choisir une autre cible",
                "Retourner à la liste des cibles actives.",
                true,
                "combat.target.change",
                buildSelectedTargetActionData(
                    target,
                    "target",
                    "Changer de cible",
                    "Revient à la liste des cibles sans consommer l'action."
                )
            );
            int choice = TerminalInterface::askMenuChoiceFromOptions(
                targetScreen,
                "Choix invalide. Entre un chiffre entre 0 et 3."
            );

            Console::clear();

            if (choice == 0)
            {
                return false;
            }

            if (choice == 1)
            {
                if (resolvedTargetName != nullptr) *resolvedTargetName = target.getName();
                if (resolvedTargetId != nullptr) *resolvedTargetId = target.getCombatInstanceId();
                if (targetForcedByCombatRules != nullptr)
                {
                    const int forcedIndex = findForcedTargetIndex(wave);
                    *targetForcedByCombatRules = forcedIndex >= 0 && forcedIndex == targetIndex;
                }
                if (boostedAttack)
                {
                    CombatAttack::executeBoostedAttack(
                        player,
                        target,
                        random,
                        damageBonus
                    );

                    ThreatSystem::consumeForcedTargetIfNeeded(target);
                    wave.removeDeadAndReplace();

                    return true;
                }

                TurnManager::executeAttack(player, target, random);
                ThreatSystem::consumeForcedTargetIfNeeded(target);

                wave.removeDeadAndReplace();

                return true;
            }

            if (choice == 2)
            {
                ObservationSystem::displayTerminalStats(target);
            }

            if (choice == 3)
            {
                stayOnThisTarget = false;
            }
        }
    }

    return false;
}
