// EN: PlayerWaveCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: PlayerWaveCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"

#include "combat/system/EscapeSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/CombatTargetMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"
#include "interface/menu/CombatRoleMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "diagnostic/RuntimeLog.hpp"
#include "item/weapon/WeaponType.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>


namespace
{
    struct TacticalLantern
    {
        std::string id;
        std::string name;
        bool mycelium = false;
    };

    struct PreparedWeaponCoating
    {
        std::string id;
        std::string name;
        std::string label;
    };

    std::string normalizeTacticalText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool playerClassContainsAny(const Player& player, const std::vector<std::string>& needles)
    {
        const std::string classText = normalizeTacticalText(player.getType());
        for (const std::string& needle : needles)
        {
            if (classText.find(normalizeTacticalText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    bool playerHasShadowStepAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "voleur", "assassin", "roublard", "rôdeur", "rodeur",
            "duelliste", "ninja", "éclaireur", "eclaireur"
        })
            || player.hasActiveSkill("shadow_step")
            || player.hasPassiveSkill("semi_cat_reflexes")
            || player.hasPassiveSkill("semi_fox_cunning");
    }

    bool playerHasRogueTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "voleur", "assassin", "roublard", "brigand", "ninja",
            "éclaireur", "eclaireur", "trappeur"
        })
            || player.hasPassiveSkill("semi_fox_cunning")
            || player.hasPassiveSkill("semi_cat_reflexes");
    }

    bool playerHasGuardianTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "chevalier", "gardien", "paladin", "colosse", "tank",
            "protecteur", "templier", "soldat", "sentinelle"
        })
            || player.hasPassiveSkill("steady_guard")
            || player.hasPassiveSkill("living_rampart");
    }

    bool playerHasArcaneTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "mage", "sorcier", "sorcière", "sorciere", "alchimiste",
            "artificier", "occultiste", "chaman", "chamane", "kitsune"
        })
            || player.hasPassiveSkill("careful_dosage")
            || player.hasPassiveSkill("cautious_channeling")
            || player.hasActiveSkill("arcane_impulse");
    }

    bool playerHasSupportTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "prêtre", "pretre", "soigneur", "paladin", "druide",
            "chaman", "chamane", "barde", "protecteur", "moine"
        })
            || player.hasActiveSkill("learned_mana_suture")
            || player.hasPassiveSkill("rally_breath")
            || player.hasPassiveSkill("vigor_sign_mastery");
    }

    bool playerHasSkirmisherTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "archer", "rôdeur", "rodeur", "chasseur", "artificier",
            "tireur", "éclaireur", "eclaireur", "mercenaire"
        })
            || player.hasActiveSkill("tracking_mark")
            || player.hasActiveSkill("prepared_volley")
            || player.hasPassiveSkill("ranger_eye");
    }


    bool playerHasWarriorBurstAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "guerrier", "barbare", "berserker", "gladiateur", "orc",
            "bagarreur", "champion", "ravageur", "soldat", "mercenaire"
        })
            || player.hasPassiveSkill("orcish_forced_march")
            || player.hasPassiveSkill("momentum_striker")
            || player.hasPassiveSkill("war_cry_caller");
    }

    bool playerHasCommanderTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "barde", "chef", "capitaine", "commandant", "stratège", "stratege",
            "mercenaire", "chevalier", "paladin", "seigneur", "noble"
        })
            || player.hasPassiveSkill("war_cry_caller")
            || player.hasPassiveSkill("rally_breath")
            || player.hasPassiveSkill("formation_breaker");
    }

    bool playerHasNatureTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "druide", "chaman", "chamane", "rôdeur", "rodeur", "forestier",
            "kitsune", "trappeur", "chasseur", "invocateur", "herboriste"
        })
            || player.hasPassiveSkill("semi_wolf_tracking")
            || player.hasPassiveSkill("semi_fox_cunning")
            || player.hasPassiveSkill("elven_fine_perception");
    }

    bool playerHasArtificeTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "artificier", "alchimiste", "forgeron", "ingénieur", "ingenieur",
            "bricoleur", "mécano", "mecano", "artisan", "inventeur"
        })
            || player.hasPassiveSkill("careful_dosage")
            || player.hasPassiveSkill("field_weapon_crafter")
            || player.hasPassiveSkill("tactical_toolbox");
    }

    bool playerHasSacredTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "prêtre", "pretre", "paladin", "templier", "clerc", "moine",
            "saint", "sanctus", "lumière", "lumiere", "protecteur"
        })
            || player.hasPassiveSkill("battle_suture_mastery")
            || player.hasPassiveSkill("vigor_sign_mastery")
            || player.hasPassiveSkill("rampart_oath_mastery");
    }

    bool playerHasMonkTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "moine", "pugiliste", "martial", "ascète", "ascete",
            "danseur lunaire", "duelliste", "bretteur", "bagarreur"
        })
            || player.hasPassiveSkill("footwork_drill")
            || player.hasPassiveSkill("semi_cat_reflexes")
            || player.hasPassiveSkill("vigor_sign_mastery");
    }

    bool playerHasIllusionTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "illusion", "illusionniste", "kitsune", "mage", "sorcier", "sorcière", "sorciere",
            "occultiste", "voleur", "roublard", "barde", "umbromancien"
        })
            || player.hasPassiveSkill("semi_fox_cunning")
            || player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler")
            || player.hasPassiveSkill("shadow_step_mastery") || player.hasPassiveSkill("shadow_stepper");
    }

    bool playerHasSummonerTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "invocateur", "invocatrice", "conjurateur", "conjuratrice", "nécromancien", "necromancien",
            "dresseur", "dompteur", "chaman", "chamane", "conjurateur de ruche", "démoniste", "demoniste",
            "fauche-âme", "fauche ame", "occultiste"
        })
            || player.hasPassiveSkill("breath_totem_mastery")
            || player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler")
            || player.hasActiveSkill("arcane_impulse");
    }

    bool playerHasBloodTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "vampire", "fauche-âme", "fauche ame", "démoniste", "demoniste", "occultiste",
            "berserker", "barbare", "nécromancien", "necromancien", "sanguin", "sang"
        })
            || player.hasPassiveSkill("rage_control_mastery")
            || player.hasPassiveSkill("anchor_breaker")
            || player.hasPassiveSkill("scar_tissue");
    }

    bool playerHasMedicOrCraftTechniqueAffinity(const Player& player)
    {
        return playerHasSupportTechniqueAffinity(player)
            || playerHasArtificeTechniqueAffinity(player)
            || playerClassContainsAny(player, {
                "médecin", "medecin", "apothicaire", "herboriste", "cuisinier", "cuisinière", "cuisiniere"
            })
            || player.hasPassiveSkill("careful_dosage")
            || player.hasPassiveSkill("field_weapon_crafter");
    }

    bool playerHasElementalistTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "élémentaliste", "elementaliste", "mage", "sorcier", "sorcière", "sorciere",
            "artificier", "alchimiste", "chaman", "chamane", "kitsune", "draconique", "dragon",
            "pyromancien", "cryomancien", "électromancien", "electromancien"
        })
            || player.hasPassiveSkill("wild_spark")
            || player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler")
            || player.hasPassiveSkill("workshop_bomb_mastery") || player.hasPassiveSkill("workshop_bomber")
            || player.hasActiveSkill("arcane_impulse");
    }

    bool playerHasBardTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "barde", "chanteur", "chanteuse", "musicien", "musicienne",
            "chef", "capitaine", "commandant", "stratège", "stratege", "paladin", "noble"
        })
            || player.hasPassiveSkill("war_cry_caller")
            || player.hasPassiveSkill("battle_order_mastery")
            || player.hasPassiveSkill("rally_breath");
    }

    bool playerHasBeastTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "barbare", "berserker", "druide", "chaman", "chamane", "rôdeur", "rodeur",
            "chasseur", "dompteur", "dresseur", "loup", "félidé", "felide", "orc", "sauvage"
        })
            || player.hasPassiveSkill("semi_wolf_tracking")
            || player.hasPassiveSkill("semi_cat_reflexes")
            || player.hasPassiveSkill("rage_control_mastery")
            || player.hasPassiveSkill("blood_pact_mastery");
    }


    bool weaponIsLightBladeForDance(const Weapon& weapon)
    {
        const std::string name = normalizeTacticalText(weapon.getName());
        return weapon.getType() == WeaponType::Dagger
            || name.find("dague") != std::string::npos
            || name.find("couteau") != std::string::npos
            || name.find("courte") != std::string::npos
            || name.find("rapière") != std::string::npos
            || name.find("rapiere") != std::string::npos
            || name.find("sabre léger") != std::string::npos
            || name.find("sabre leger") != std::string::npos
            || name.find("lame légère") != std::string::npos
            || name.find("lame legere") != std::string::npos;
    }

    int countLightWeaponsForBladeDance(const Player& player)
    {
        int count = 0;
        if (player.hasEquippedWeapon() && weaponIsLightBladeForDance(player.getEquippedWeapon()))
        {
            ++count;
        }
        for (const Weapon& weapon : player.getInventory().getWeapons())
        {
            if (weaponIsLightBladeForDance(weapon))
            {
                ++count;
            }
            if (count >= 2)
            {
                return count;
            }
        }
        return count;
    }

    bool playerHasDuelTechniqueAffinity(const Player& player)
    {
        return playerClassContainsAny(player, {
            "duelliste", "bretteur", "escrimeur", "samouraï", "samourai", "ronin",
            "assassin", "voleur", "roublard", "moine", "gladiateur", "lame"
        })
            || player.hasPassiveSkill("shadow_step_mastery") || player.hasPassiveSkill("shadow_stepper")
            || player.hasPassiveSkill("inner_mantra_mastery")
            || player.hasPassiveSkill("rogue_feint_mastery") || player.hasPassiveSkill("rogue_feinter")
            || player.hasPassiveSkill("blade_discipline");
    }

    bool playerHasWardenTechniqueAffinity(const Player& player)
    {
        return playerHasGuardianTechniqueAffinity(player)
            || playerHasSupportTechniqueAffinity(player)
            || playerHasSacredTechniqueAffinity(player)
            || playerClassContainsAny(player, {
                "sentinelle", "protecteur", "gardien", "chevalier", "paladin", "clerc", "templier", "colosse"
            })
            || player.hasPassiveSkill("emergency_warder")
            || player.hasPassiveSkill("steel_prayer_mastery");
    }

    bool tacticalMonsterProfileContainsAny(const Monster& monster, const std::vector<std::string>& needles)
    {
        const std::string profileText = normalizeTacticalText(monster.getName() + " " + monster.getType() + " " + monster.getRaceText());
        for (const std::string& needle : needles)
        {
            if (profileText.find(normalizeTacticalText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    bool tacticalMonsterIsDedicatedCaller(const Monster& monster)
    {
        return tacticalMonsterProfileContainsAny(monster, {
            "rameuteur", "rameuteuse", "rameut", "hurleur", "hurleuse", "corneur", "corne",
            "crieur", "crieuse", "guetteur", "guetteuse", "éclaireur", "eclaireur",
            "sentinelle", "alarme", "tambour", "chef", "capitaine", "sergent", "alpha",
            "reine", "matriarche", "nid", "scribe", "chaman", "chamane", "shaman"
        });
    }

    bool tacticalMonsterIsCommonBandRace(const Monster& monster)
    {
        return tacticalMonsterProfileContainsAny(monster, {
            "gobelin", "loup", "chien", "meute", "kobold", "rat", "nuisible",
            "insecte", "insectoïde", "insectoide", "araignée", "araignee", "slime"
        });
    }

    bool tacticalMonsterMayTryToSignal(const Monster& monster)
    {
        if (monster.isDead() || monster.isInvocation())
        {
            return false;
        }

        if (monster.wasSpawnedByReinforcementCall() && !monster.canUseWeakenedReinforcementSignal())
        {
            return false;
        }

        return tacticalMonsterIsDedicatedCaller(monster)
            || monster.canUseWeakenedReinforcementSignal()
            || tacticalMonsterIsCommonBandRace(monster);
    }

    bool waveHasSignalToCut(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (tacticalMonsterMayTryToSignal(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    std::string describeSignalTargetHint(const Monster& monster)
    {
        if (monster.canUseWeakenedReinforcementSignal())
        {
            return "Second signal fragile : dangereux seulement si la ligne s'effondre vraiment.";
        }

        if (tacticalMonsterIsDedicatedCaller(monster))
        {
            return "Vrai appelant : son souffle, son regard ou son ordre peut rameuter la ligne.";
        }

        if (tacticalMonsterIsCommonBandRace(monster))
        {
            return "Instinct de bande : il peut chercher les siens seulement s'il survit trop longtemps blessé.";
        }

        return "Aucun signal net.";
    }

    int chooseSignalTarget(EnemyCombatQueue& wave, const std::string& screenId)
    {
        std::vector<int> signalTargets;
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (tacticalMonsterMayTryToSignal(wave.getActiveEnemy(index)))
            {
                signalTargets.push_back(index);
            }
        }

        if (signalTargets.empty())
        {
            MessageScreen::show(
                "AUCUN SIGNAL",
                screenId + ".empty",
                {"Aucun souffle, cri ou ordre de renfort n'est assez net pour être coupé."},
                false
            );
            return -1;
        }

        MenuScreen targetScreen("SIGNAL À COUPER", screenId + ".target");
        targetScreen.addLine("Choisis l'appelant, le chef ou la créature de bande dont tu veux briser le rythme.");
        targetScreen.addBackOption("Retour", screenId + ".back");

        for (int option = 0; option < static_cast<int>(signalTargets.size()); ++option)
        {
            const int enemyIndex = signalTargets[option];
            const Monster& enemy = wave.getActiveEnemy(enemyIndex);
            targetScreen.addOption(
                option + 1,
                enemy.getName() + " - PV " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()),
                describeSignalTargetHint(enemy),
                true,
                screenId + ".signal_target." + std::to_string(option + 1)
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return -1;
        }

        const int mappedIndex = choice - 1;
        if (mappedIndex < 0 || mappedIndex >= static_cast<int>(signalTargets.size()))
        {
            MessageScreen::show(
                "CIBLE INTROUVABLE",
                screenId + ".invalid",
                {"La cible choisie n'est plus assez lisible dans la ligne active."},
                false
            );
            return -1;
        }

        return signalTargets[mappedIndex];
    }

    bool consumeTacticalLantern(Player& player, TacticalLantern& lantern)
    {
        if (player.getInventory().countMaterialById("fire_lantern") > 0
            && player.getInventory().removeMaterialQuantityById("fire_lantern", 1))
        {
            lantern.id = "fire_lantern";
            lantern.name = "lanterne de feu";
            lantern.mycelium = false;
            return true;
        }

        if (player.getInventory().countMaterialById("mycelium_lantern") > 0
            && player.getInventory().removeMaterialQuantityById("mycelium_lantern", 1))
        {
            lantern.id = "mycelium_lantern";
            lantern.name = "lanterne de mycélium";
            lantern.mycelium = true;
            return true;
        }

        return false;
    }

    int canonicalEventCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.count;
            }
        }
        return 0;
    }

    int totalTacticalActionCount(const Player& player)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "actions_tactiques_combat")
            {
                total += std::max(0, record.count);
            }
        }
        return total;
    }

    int tacticalMasteryLevel(const Player& player, const std::string& actionKey)
    {
        return player.getActiveSkillMasteryLevel(actionKey);
    }

    std::string passiveMasteryIdForAction(const std::string& actionKey)
    {
        if (actionKey == "observation_active") return "terrain_reader";
        if (actionKey == "preparation_arme") return "field_weapon_crafter";
        if (actionKey == "brise_garde") return "guard_breaker";
        if (actionKey == "posture_soutien") return "support_rhythm";
        if (actionKey == "rupture_formation") return "formation_breaker";
        if (actionKey == "coupe_signal") return "signal_cut_awareness";
        if (actionKey == "lecture_corps") return "body_reader";
        if (actionKey == "chaine_etats") return "status_conductor";
        if (actionKey == "marque_proie") return "prey_marker";
        if (actionKey == "retrait_controle") return "controlled_retreat";
        if (actionKey == "lecture_menace") return "threat_reader";
        if (actionKey == "balayage_bas") return "low_sweeper";
        if (actionKey == "rupture_concentration") return "focus_breaker";
        if (actionKey == "rupture_allonge") return "reach_breaker";
        if (actionKey == "perce_rempart") return "shield_piercer";
        if (actionKey == "rupture_ancrage") return "anchor_breaker";
        if (actionKey == "souffle_ralliement") return "rally_breath";
        if (actionKey == "frappe_elan") return "momentum_striker";
        if (actionKey == "voile_urgence") return "emergency_warder";
        if (actionKey == "etincelle_instable") return "wild_spark";
        if (actionKey == "cri_guerre") return "war_cry_caller";
        if (actionKey == "pas_ombre") return "shadow_step_mastery";
        if (actionKey == "coup_arc") return "arc_sweep_mastery";
        if (actionKey == "signe_vigueur") return "vigor_sign_mastery";
        if (actionKey == "feinte_sournoise") return "rogue_feint_mastery";
        if (actionKey == "suture_bataille") return "battle_suture_mastery";
        if (actionKey == "canalisation_arcanique") return "arcane_channel_mastery";
        if (actionKey == "serment_rempart") return "rampart_oath_mastery";
        if (actionKey == "tir_arret") return "stopping_shot_mastery";
        if (actionKey == "rage_maitrisee") return "rage_control_mastery";
        if (actionKey == "ordre_bataille") return "battle_order_mastery";
        if (actionKey == "totem_souffle") return "breath_totem_mastery";
        if (actionKey == "bombe_atelier") return "workshop_bomb_mastery";
        if (actionKey == "priere_acier") return "steel_prayer_mastery";
        if (actionKey == "mantra_interieur") return "inner_mantra_mastery";
        if (actionKey == "image_trompeuse") return "trick_image_mastery";
        if (actionKey == "lien_invocation") return "summoning_link_mastery";
        if (actionKey == "pacte_sanguin") return "blood_pact_mastery";
        if (actionKey == "remede_fortune") return "field_remedy_mastery";
        if (actionKey == "lame_elementaire") return "elemental_blade_mastery";
        if (actionKey == "cercle_protecteur") return "protective_circle_mastery";
        if (actionKey == "fleche_entravante") return "binding_shot_mastery";
        if (actionKey == "chant_revigorant") return "inspiring_chant_mastery";
        if (actionKey == "instinct_bete") return "beast_instinct_mastery";
        if (actionKey == "danse_lame") return "blade_dance_mastery";
        return "";
    }

    bool tacticalActionUsesWeaponTempo(const std::string& actionKey)
    {
        return actionKey == "coup_de_pied"
            || actionKey == "exploitation_ouverture"
            || actionKey == "preparation_arme"
            || actionKey == "brise_garde"
            || actionKey == "lecture_corps"
            || actionKey == "chaine_etats"
            || actionKey == "marque_proie"
            || actionKey == "balayage_bas"
            || actionKey == "rupture_concentration"
            || actionKey == "rupture_allonge"
            || actionKey == "perce_rempart"
            || actionKey == "frappe_elan"
            || actionKey == "pas_ombre"
            || actionKey == "coup_arc"
            || actionKey == "feinte_sournoise"
            || actionKey == "tir_arret"
            || actionKey == "rage_maitrisee"
            || actionKey == "priere_acier"
            || actionKey == "lame_elementaire"
            || actionKey == "fleche_entravante"
            || actionKey == "instinct_bete"
            || actionKey == "danse_lame";
    }

    bool tacticalActionUsesArmorTempo(const std::string& actionKey)
    {
        return actionKey == "posture_soutien"
            || actionKey == "rupture_formation"
            || actionKey == "retrait_controle"
            || actionKey == "lecture_menace"
            || actionKey == "souffle_ralliement"
            || actionKey == "voile_urgence"
            || actionKey == "cri_guerre"
            || actionKey == "signe_vigueur"
            || actionKey == "suture_bataille"
            || actionKey == "serment_rempart"
            || actionKey == "ordre_bataille"
            || actionKey == "totem_souffle"
            || actionKey == "mantra_interieur"
            || actionKey == "pacte_sanguin"
            || actionKey == "remede_fortune"
            || actionKey == "cercle_protecteur"
            || actionKey == "chant_revigorant";
    }

    int tacticalLoadoutSynergyModifier(const Player& player, const std::string& actionKey)
    {
        int modifier = 0;
        if (player.hasEquippedWeapon() && tacticalActionUsesWeaponTempo(actionKey))
        {
            const Weapon weapon = player.getEquippedWeapon();
            const bool bonus = CombatClassSystem::hasWeaponAffinity(player, weapon.getType(), weapon.getName());
            const bool malus = CombatClassSystem::getWeaponHandlingAccuracyAdjustment(player, weapon.getType(), weapon.getName()) < 0
                || CombatClassSystem::getWeaponHandlingDamagePercent(player, weapon.getType(), weapon.getName()) < 100;
            if (bonus) modifier += 2;
            if (malus) modifier -= 3;
        }
        if (player.hasEquippedArmor() && tacticalActionUsesArmorTempo(actionKey))
        {
            const Armor armor = player.getEquippedArmor();
            const bool bonus = CombatClassSystem::hasArmorAffinity(player, armor.getType(), armor.getName());
            const bool malus = CombatClassSystem::getArmorHandlingDamageReductionAdjustment(player, armor.getType(), armor.getName(), 24) < 0
                || CombatClassSystem::getArmorHandlingEscapeAdjustment(player, armor.getType(), armor.getName()) < 0;
            if (bonus) modifier += 1;
            if (malus) modifier -= 2;
        }
        return std::clamp(modifier, -3, 3);
    }

    std::string tacticalLoadoutSynergyLine(const Player& player, const std::string& actionKey)
    {
        const int modifier = tacticalLoadoutSynergyModifier(player, actionKey);
        if (modifier >= 2)
        {
            return " Build : équipement cohérent, la classe transmet mieux ce geste.";
        }
        if (modifier > 0)
        {
            return " Build : petit soutien d'équipement cohérent.";
        }
        if (modifier <= -2)
        {
            return " Build : équipement contradictoire, le geste perd en propreté malgré la maîtrise.";
        }
        if (modifier < 0)
        {
            return " Build : léger frottement d'équipement.";
        }
        return "";
    }

    int tacticalPassiveMasteryLevel(const Player& player, const std::string& actionKey)
    {
        const std::string passiveId = passiveMasteryIdForAction(actionKey);
        if (passiveId.empty() || !player.hasPassiveSkill(passiveId))
        {
            return 0;
        }
        return player.getPassiveMasteryLevelFromAction(actionKey);
    }

    int tacticalMasterySoftBonus(const Player& player, const std::string& actionKey)
    {
        // Les paliers doivent se sentir : un bon niveau donne un vrai cran de dégâts/protection/contrôle.
        // Le passif activé ajoute du soutien, mais ne lance jamais l'actif et ne supprime jamais coût, tour ou risque.
        const int activeLevel = tacticalMasteryLevel(player, actionKey);
        const int passiveLevel = tacticalPassiveMasteryLevel(player, actionKey);
        const int activeBonus = activeLevel <= 0 ? 0 : 1 + activeLevel;
        const int passiveBonus = passiveLevel <= 0 ? 0 : 1 + passiveLevel;
        const int loadoutBonus = tacticalLoadoutSynergyModifier(player, actionKey);
        return std::clamp(activeBonus + passiveBonus + loadoutBonus, -4, 20);
    }

    int tacticalMasteryChanceBonus(const Player& player, const std::string& actionKey)
    {
        // Sert aux chances secondaires. Le bonus devient visible à mi-maîtrise, mais reste plafonné pour éviter le 100% facile.
        const int activeBonus = tacticalMasteryLevel(player, actionKey) * 3;
        const int passiveBonus = tacticalPassiveMasteryLevel(player, actionKey) * 2;
        const int loadoutBonus = tacticalLoadoutSynergyModifier(player, actionKey) * 2;
        return std::clamp(activeBonus + passiveBonus + loadoutBonus, -8, 40);
    }

    int tacticalMasteryDurationBonus(const Player& player, const std::string& actionKey)
    {
        // Une bonne maîtrise peut prolonger un effet court de façon sensible, jamais transformer un actif en aura permanente.
        const int activeLevel = tacticalMasteryLevel(player, actionKey);
        const int passiveLevel = tacticalPassiveMasteryLevel(player, actionKey);
        int bonus = 0;
        if (activeLevel >= 3) ++bonus;
        if (activeLevel >= 6) ++bonus;
        if (passiveLevel >= 2) ++bonus;
        return std::min(3, bonus);
    }

    std::string tacticalMasteryShortResultLine(const Player& player, const std::string& actionKey)
    {
        const int level = tacticalMasteryLevel(player, actionKey);
        const int passiveLevel = tacticalPassiveMasteryLevel(player, actionKey);
        const std::string passivePart = passiveLevel > 0
            ? " Passif actif : niveau " + std::to_string(passiveLevel) + ", le réflexe soutient le geste sans choisir l'action à ta place."
            : "";
        const std::string loadoutPart = tacticalLoadoutSynergyLine(player, actionKey);
        if (level <= 0)
        {
            return "Maîtrise : geste encore brut, aucun bonus stable." + passivePart + loadoutPart;
        }
        if (level >= 8)
        {
            return "Maîtrise : impact fort, l'actif garde son coût mais peut changer le tour quand il est bien choisi." + passivePart + loadoutPart;
        }
        if (level >= 4)
        {
            return "Maîtrise : impact visible, meilleure stabilité et effet secondaire plus crédible." + passivePart + loadoutPart;
        }
        return "Maîtrise : petite habitude, le mouvement devient déjà moins anecdotique." + passivePart + loadoutPart;
    }

    std::string tacticalMasteryLine(const Player& player, const std::string& actionKey)
    {
        std::string line = "Maîtrise active : " + player.getActiveSkillMasteryLabel(actionKey)
            + " ; " + player.getActiveSkillMasteryProgressHint(actionKey)
            + " ; effet : " + player.getActiveSkillMasteryEffectHint(actionKey) + ".";
        const int passiveLevel = tacticalPassiveMasteryLevel(player, actionKey);
        if (passiveLevel > 0)
        {
            line += " Passif activé : " + player.getPassiveMasteryLabelFromAction(actionKey)
                + ", soutien léger appliqué au geste.";
        }
        const std::string loadoutLine = tacticalLoadoutSynergyLine(player, actionKey);
        if (!loadoutLine.empty())
        {
            line += loadoutLine;
        }
        return line;
    }

    std::string tacticalMasteryMenuHint(const Player& player, const std::string& actionKey)
    {
        std::string hint = " Maîtrise : " + player.getActiveSkillMasteryLabel(actionKey)
            + ", " + player.getActiveSkillMasteryProgressHint(actionKey);
        const int passiveLevel = tacticalPassiveMasteryLevel(player, actionKey);
        if (passiveLevel > 0)
        {
            hint += ", passif actif " + std::to_string(passiveLevel);
        }
        const int loadoutModifier = tacticalLoadoutSynergyModifier(player, actionKey);
        if (loadoutModifier > 0)
        {
            hint += ", build +" + std::to_string(loadoutModifier);
        }
        else if (loadoutModifier < 0)
        {
            hint += ", build " + std::to_string(loadoutModifier);
        }
        return hint + ".";
    }

    int tacticalMasteryCooldownReduction(const Player& player, const std::string& actionKey)
    {
        // Une maîtrise haute aide le rythme, mais ne supprime pas le cooldown.
        // Un passif de maîtrise activé peut aider un cran, jamais rendre la technique gratuite.
        const int activeLevel = tacticalMasteryLevel(player, actionKey);
        const int passiveLevel = tacticalPassiveMasteryLevel(player, actionKey);
        const int activeReduction = (activeLevel >= 6 ? 1 : 0) + (activeLevel >= 9 ? 1 : 0);
        const int passiveReduction = passiveLevel >= 4 ? 1 : 0;
        const int loadoutModifier = tacticalLoadoutSynergyModifier(player, actionKey);
        const int loadoutReduction = loadoutModifier >= 3 ? 1 : 0;
        return std::min(2, activeReduction + passiveReduction + loadoutReduction);
    }

    void updateTacticalLearning(Player& player, const std::string& actionKey)
    {
        const int actionCount = canonicalEventCount(player, "actions_tactiques_combat", actionKey);
        const int totalCount = totalTacticalActionCount(player);

        if (actionKey == "observation_active" && actionCount >= 3)
        {
            player.unlockPassiveSkill("terrain_reader", "Lecture du terrain");
        }

        if (actionKey == "preparation_arme" && actionCount >= 3)
        {
            player.unlockPassiveSkill("field_weapon_crafter", "Préparateur d'arme de terrain");
        }

        if (totalCount >= 5)
        {
            player.unlockPassiveSkill("combat_improviser", "Improvisateur de combat");
        }

        if (actionKey == "brise_garde" && actionCount >= 3)
        {
            player.unlockPassiveSkill("guard_breaker", "Casseur de garde");
        }

        if (actionKey == "posture_soutien" && actionCount >= 3)
        {
            player.unlockPassiveSkill("support_rhythm", "Rythme de soutien");
        }

        if (actionKey == "rupture_formation" && actionCount >= 3)
        {
            player.unlockPassiveSkill("formation_breaker", "Briseur de formation");
        }

        if (actionKey == "coupe_signal" && actionCount >= 3)
        {
            player.unlockPassiveSkill("signal_cut_awareness", "Lecture coupe-signal");
        }

        if (actionKey == "lecture_corps" && actionCount >= 3)
        {
            player.unlockPassiveSkill("body_reader", "Lecture des corps");
        }

        if (actionKey == "chaine_etats" && actionCount >= 3)
        {
            player.unlockPassiveSkill("status_conductor", "Conducteur d'états");
        }

        if (actionKey == "marque_proie" && actionCount >= 3)
        {
            player.unlockPassiveSkill("prey_marker", "Marqueur de proie");
        }

        if (actionKey == "retrait_controle" && actionCount >= 3)
        {
            player.unlockPassiveSkill("controlled_retreat", "Sens du retrait");
        }

        if (actionKey == "lecture_menace" && actionCount >= 3)
        {
            player.unlockPassiveSkill("threat_reader", "Lecture des menaces");
        }

        if (actionKey == "balayage_bas" && actionCount >= 3)
        {
            player.unlockPassiveSkill("low_sweeper", "Lecture du balayage bas");
        }

        if (actionKey == "rupture_concentration" && actionCount >= 3)
        {
            player.unlockPassiveSkill("focus_breaker", "Briseur de concentration");
        }

        if (actionKey == "rupture_allonge" && actionCount >= 3)
        {
            player.unlockPassiveSkill("reach_breaker", "Briseur d'allonge");
        }

        if (actionKey == "perce_rempart" && actionCount >= 3)
        {
            player.unlockPassiveSkill("shield_piercer", "Perce-rempart");
        }

        if (actionKey == "rupture_ancrage" && actionCount >= 3)
        {
            player.unlockPassiveSkill("anchor_breaker", "Briseur d'ancrage");
        }

        if (actionKey == "souffle_ralliement" && actionCount >= 3)
        {
            player.unlockPassiveSkill("rally_breath", "Souffle rallié");
        }

        if (actionKey == "frappe_elan" && actionCount >= 3)
        {
            player.unlockPassiveSkill("momentum_striker", "Élan canalisé");
        }

        if (actionKey == "voile_urgence" && actionCount >= 3)
        {
            player.unlockPassiveSkill("emergency_warder", "Réflexe de voile");
        }

        if (actionKey == "etincelle_instable" && actionCount >= 3)
        {
            player.unlockPassiveSkill("wild_spark", "Instabilité apprivoisée");
        }

        if (actionKey == "cri_guerre" && actionCount >= 3)
        {
            player.unlockPassiveSkill("war_cry_caller", "Voix de guerre");
        }

        if (actionKey == "pas_ombre" && actionCount >= 3 && playerHasShadowStepAffinity(player))
        {
            player.unlockPassiveSkill("shadow_step_mastery", "Maîtrise du pas de l'ombre");
        }

        if (actionKey == "coup_arc" && actionCount >= 3)
        {
            player.unlockPassiveSkill("arc_sweep_mastery", "Amplitude contrôlée");
        }

        if (actionKey == "signe_vigueur" && actionCount >= 3)
        {
            player.unlockPassiveSkill("vigor_sign_mastery", "Souffle de vigueur");
        }

        if (actionKey == "feinte_sournoise" && actionCount >= 3 && playerHasRogueTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("rogue_feint_mastery", "Angle de feinte");
        }

        if (actionKey == "suture_bataille" && actionCount >= 3 && playerHasSupportTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("battle_suture_mastery", "Gestes de suture");
        }

        if (actionKey == "canalisation_arcanique" && actionCount >= 3 && playerHasArcaneTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("arcane_channel_mastery", "Canalisation stabilisée");
        }

        if (actionKey == "serment_rempart" && actionCount >= 3 && playerHasGuardianTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("rampart_oath_mastery", "Tenue du rempart");
        }

        if (actionKey == "tir_arret" && actionCount >= 3 && playerHasSkirmisherTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("stopping_shot_mastery", "Œil d'arrêt");
        }

        if (actionKey == "rage_maitrisee" && actionCount >= 3 && playerHasWarriorBurstAffinity(player))
        {
            player.unlockPassiveSkill("rage_control_mastery", "Rage canalisée");
        }

        if (actionKey == "ordre_bataille" && actionCount >= 3 && playerHasCommanderTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("battle_order_mastery", "Voix de bataille");
        }

        if (actionKey == "totem_souffle" && actionCount >= 3 && playerHasNatureTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("breath_totem_mastery", "Ancrage du souffle");
        }

        if (actionKey == "bombe_atelier" && actionCount >= 3 && playerHasArtificeTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("workshop_bomb_mastery", "Bricolage explosif");
        }

        if (actionKey == "priere_acier" && actionCount >= 3 && playerHasSacredTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("steel_prayer_mastery", "Foi d'acier");
        }

        if (actionKey == "mantra_interieur" && actionCount >= 3 && playerHasMonkTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("inner_mantra_mastery", "Souffle intérieur");
        }

        if (actionKey == "image_trompeuse" && actionCount >= 3 && playerHasIllusionTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("trick_image_mastery", "Angle trompeur");
        }

        if (actionKey == "lien_invocation" && actionCount >= 3 && playerHasSummonerTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("summoning_link_mastery", "Lien stabilisé");
        }

        if (actionKey == "pacte_sanguin" && actionCount >= 3 && playerHasBloodTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("blood_pact_mastery", "Sang discipliné");
        }

        if (actionKey == "remede_fortune" && actionCount >= 3 && playerHasMedicOrCraftTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("field_remedy_mastery", "Gestes de terrain");
        }

        if (actionKey == "lame_elementaire" && actionCount >= 3 && playerHasElementalistTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("elemental_blade_mastery", "Maîtrise élémentaire");
        }

        if (actionKey == "cercle_protecteur" && actionCount >= 3 && playerHasWardenTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("protective_circle_mastery", "Garde circulaire");
        }

        if (actionKey == "fleche_entravante" && actionCount >= 3 && playerHasSkirmisherTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("binding_shot_mastery", "Trait entravant");
        }

        if (actionKey == "chant_revigorant" && actionCount >= 3 && playerHasBardTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("inspiring_chant_mastery", "Voix revigorante");
        }

        if (actionKey == "instinct_bete" && actionCount >= 3 && playerHasBeastTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("beast_instinct_mastery", "Instinct canalisé");
        }

        if (actionKey == "danse_lame" && actionCount >= 3 && playerHasDuelTechniqueAffinity(player))
        {
            player.unlockPassiveSkill("blade_dance_mastery", "Rythme de lame");
        }

        if (totalCount >= 9)
        {
            player.unlockPassiveSkill("tactical_toolbox", "Boîte à outils tactique");
        }
    }

    bool hasWeaponCoatingMaterial(const Player& player)
    {
        return player.getInventory().countMaterialById("slime_residue") > 0
            || player.getInventory().countMaterialById("arcane_dust") > 0
            || player.getInventory().countMaterialById("amber_tempering_oil") > 0
            || player.getInventory().countMaterialById("echoing_resin") > 0
            || player.getInventory().countMaterialById("moonlit_salt") > 0
            || player.getInventory().countMaterialById("venom_arrows") > 0;
    }

    PreparedWeaponCoating consumeWeaponCoatingMaterial(Player& player)
    {
        struct CoatingChoice
        {
            std::string id;
            std::string name;
            std::string label;
        };

        const std::vector<CoatingChoice> choices = {
            {"slime_residue", "résidu de slime", "colle toxique"},
            {"arcane_dust", "poussière arcanique", "étincelle arcanique"},
            {"amber_tempering_oil", "huile de trempe ambrée", "huile d'atelier"},
            {"echoing_resin", "résine d'écho", "lecture vibrante"},
            {"moonlit_salt", "sel lunaire", "morsure froide"},
            {"venom_arrows", "flèche enduite de venin", "venin recyclé"}
        };

        for (const CoatingChoice& choice : choices)
        {
            if (player.getInventory().countMaterialById(choice.id) > 0
                && player.getInventory().removeMaterialQuantityById(choice.id, 1))
            {
                return {choice.id, choice.name, choice.label};
            }
        }

        return {"", "", ""};
    }

    bool hasImprovisedTrapMaterial(const Player& player)
    {
        return player.getInventory().countMaterialById("rusted_metal_fragment") > 0
            || player.getInventory().countMaterialById("worn_leather_piece") > 0
            || player.getInventory().countMaterialById("cracked_bone") > 0
            || player.getInventory().countMaterialById("slime_residue") > 0
            || player.getInventory().countMaterialById("arcane_dust") > 0;
    }

    std::string consumeImprovisedTrapMaterial(Player& player)
    {
        struct MaterialChoice
        {
            std::string id;
            std::string label;
        };

        const std::vector<MaterialChoice> choices = {
            {"rusted_metal_fragment", "fragment de métal rouillé"},
            {"worn_leather_piece", "morceau de cuir usé"},
            {"cracked_bone", "os fissuré"},
            {"slime_residue", "résidu de slime"},
            {"arcane_dust", "poussière arcanique"}
        };

        for (const MaterialChoice& choice : choices)
        {
            if (player.getInventory().countMaterialById(choice.id) > 0
                && player.getInventory().removeMaterialQuantityById(choice.id, 1))
            {
                return choice.label;
            }
        }

        return "";
    }

    bool hasExploitableOpening(const Monster& target)
    {
        const bool lowHp = target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 45;
        return lowHp
            || target.hasPoison()
            || target.hasBleeding()
            || target.hasBurning()
            || target.hasFrost()
            || target.hasShock()
            || target.hasWeakening()
            || target.hasVulnerability();
    }

    bool waveHasExploitableOpening(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (hasExploitableOpening(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    bool waveHasFormationToBreak(const EnemyCombatQueue& wave)
    {
        if (wave.getActiveEnemyCount() >= 2)
        {
            return true;
        }

        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (wave.getActiveEnemy(index).isInDefensePosture())
            {
                return true;
            }
        }

        return false;
    }


    int countCombatStatuses(const Monster& monster)
    {
        int count = 0;
        if (monster.hasPoison()) ++count;
        if (monster.hasBleeding()) ++count;
        if (monster.hasBurning()) ++count;
        if (monster.hasFrost()) ++count;
        if (monster.hasShock()) ++count;
        if (monster.hasWeakening()) ++count;
        if (monster.hasVulnerability()) ++count;
        if (monster.hasNextHitVulnerability()) ++count;
        return count;
    }

    bool hasStatusChain(const Monster& monster)
    {
        if (monster.isDead())
        {
            return false;
        }

        return countCombatStatuses(monster) >= 2
            || (monster.hasPoison() && monster.hasBleeding())
            || (monster.hasBurning() && monster.hasFrost())
            || (monster.hasShock() && (monster.hasVulnerability() || monster.hasNextHitVulnerability()))
            || (monster.hasWeakening() && (monster.hasVulnerability() || monster.hasNextHitVulnerability()));
    }

    bool waveHasStatusChain(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (hasStatusChain(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    bool isLowOrMobileTarget(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const std::string archetype = normalizeTacticalText(profile.archetype);
        return profile.incomingAccuracyModifier <= -8
            || archetype.find("nuisible") != std::string::npos
            || archetype.find("chauve-souris") != std::string::npos
            || archetype.find("fée") != std::string::npos
            || archetype.find("insecto") != std::string::npos
            || archetype.find("araignée") != std::string::npos
            || archetype.find("kobold") != std::string::npos
            || archetype.find("slime bondissant") != std::string::npos
            || archetype.find("duelliste") != std::string::npos;
    }

    bool isFocusOrSupportTarget(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const std::string archetype = normalizeTacticalText(profile.archetype);
        return archetype.find("soigneur") != std::string::npos
            || archetype.find("lanceur") != std::string::npos
            || archetype.find("porte-malédiction") != std::string::npos
            || archetype.find("fée") != std::string::npos
            || archetype.find("spectral") != std::string::npos
            || archetype.find("énergie") != std::string::npos
            || archetype.find("alchimiste") != std::string::npos
            || archetype.find("champignon") != std::string::npos
            || archetype.find("cristal") != std::string::npos;
    }

    bool waveHasLowOrMobileTarget(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (isLowOrMobileTarget(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    bool waveHasFocusOrSupportTarget(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (isFocusOrSupportTarget(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    bool isReachOrBacklineTarget(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const std::string archetype = normalizeTacticalText(profile.archetype);
        return archetype.find("harceleur") != std::string::npos
            || archetype.find("tireur") != std::string::npos
            || archetype.find("allonge") != std::string::npos
            || archetype.find("lanceur") != std::string::npos
            || archetype.find("duelliste") != std::string::npos
            || tacticalMonsterProfileContainsAny(monster, {"archer", "arbalétrier", "arbaletrier", "tireur", "lancier", "piquier", "hallebardier", "javelot", "sarbacane"});
    }

    bool waveHasReachOrBacklineTarget(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (isReachOrBacklineTarget(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    bool isShieldOrArmorTarget(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const std::string archetype = normalizeTacticalText(profile.archetype);
        return monster.isInDefensePosture()
            || archetype.find("porte-bouclier") != std::string::npos
            || archetype.find("gardien") != std::string::npos
            || archetype.find("construction") != std::string::npos
            || archetype.find("carapace") != std::string::npos
            || profile.physicalDamageModifierPercent <= -12
            || tacticalMonsterProfileContainsAny(monster, {"bouclier", "rempart", "armure", "carapace", "coquille", "golem", "sentinelle", "blindé", "blinde"});
    }

    bool waveHasShieldOrArmorTarget(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (isShieldOrArmorTarget(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    bool isOccultAnchorTarget(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const std::string archetype = normalizeTacticalText(profile.archetype);
        return archetype.find("porte-malédiction") != std::string::npos
            || archetype.find("nécrotique") != std::string::npos
            || archetype.find("mort-vivant") != std::string::npos
            || archetype.find("spectral") != std::string::npos
            || archetype.find("serment sacré") != std::string::npos
            || archetype.find("énergie") != std::string::npos
            || tacticalMonsterProfileContainsAny(monster, {"maudit", "maudite", "malédiction", "malediction", "nécro", "necro", "spectre", "âme", "ame", "rituel", "totem", "relique", "crypte"});
    }

    bool waveHasOccultAnchorTarget(const EnemyCombatQueue& wave)
    {
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (isOccultAnchorTarget(wave.getActiveEnemy(index)))
            {
                return true;
            }
        }
        return false;
    }

    std::string describeStatusChainHint(const Monster& monster)
    {
        std::vector<std::string> pieces;
        if (monster.hasPoison()) pieces.push_back("poison");
        if (monster.hasBleeding()) pieces.push_back("saignement");
        if (monster.hasBurning()) pieces.push_back("brûlure");
        if (monster.hasFrost()) pieces.push_back("givre");
        if (monster.hasShock()) pieces.push_back("choc");
        if (monster.hasWeakening()) pieces.push_back("affaiblissement");
        if (monster.hasVulnerability() || monster.hasNextHitVulnerability()) pieces.push_back("faille");

        if (pieces.empty())
        {
            return "Aucune chaîne de statut lisible.";
        }

        std::string hint = "États visibles : ";
        for (std::size_t index = 0; index < pieces.size(); ++index)
        {
            if (index > 0)
            {
                hint += index + 1 == pieces.size() ? " et " : ", ";
            }
            hint += pieces[index];
        }
        hint += ".";
        return hint;
    }

    int chooseStatusChainTarget(EnemyCombatQueue& wave, const std::string& screenId)
    {
        std::vector<int> targets;
        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            if (hasStatusChain(wave.getActiveEnemy(index)))
            {
                targets.push_back(index);
            }
        }

        if (targets.empty())
        {
            MessageScreen::show(
                "AUCUNE CHAÎNE",
                screenId + ".empty",
                {"Aucun ennemi ne porte assez d'états pour être forcé à céder maintenant."},
                false
            );
            return -1;
        }

        MenuScreen targetScreen("CHAÎNE D'ÉTATS", screenId + ".target");
        targetScreen.addLine("Choisis la cible dont tu veux faire réagir les blessures, brûlures, venins ou failles.");
        targetScreen.addBackOption("Retour", screenId + ".back");

        for (int option = 0; option < static_cast<int>(targets.size()); ++option)
        {
            const int enemyIndex = targets[option];
            const Monster& enemy = wave.getActiveEnemy(enemyIndex);
            targetScreen.addOption(
                option + 1,
                enemy.getName() + " - PV " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()),
                describeStatusChainHint(enemy),
                true,
                screenId + ".chain_target." + std::to_string(option + 1)
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return -1;
        }

        const int mappedIndex = choice - 1;
        if (mappedIndex < 0 || mappedIndex >= static_cast<int>(targets.size()))
        {
            MessageScreen::show(
                "CIBLE INTROUVABLE",
                screenId + ".invalid",
                {"La cible choisie n'a plus de chaîne lisible."},
                false
            );
            return -1;
        }

        return targets[mappedIndex];
    }


    std::string describeBodyTargetHint(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        const int physicalModifier = profile.physicalDamageModifierPercent;
        std::string hint;
        if (physicalModifier >= 20)
        {
            hint = "Corps fragile : difficile à cadrer parfois, mais le choc traverse vite si l'angle est bon.";
        }
        else if (physicalModifier >= 8)
        {
            hint = "Gabarit sensible : une vraie touche peut faire céder l'appui ou la chair.";
        }
        else if (physicalModifier <= -18)
        {
            hint = "Matière dure : vise jointure, noyau, ventre ou fissure plutôt que la masse.";
        }
        else if (physicalModifier < 0)
        {
            hint = "Corps résistant : mieux vaut chercher une faille qu'empiler les impacts simples.";
        }
        else
        {
            hint = "Corps lisible : une observation du rythme peut quand même ouvrir une zone faible.";
        }

        if (!profile.durabilityLine.empty())
        {
            hint += " " + profile.durabilityLine;
        }
        return hint;
    }

    int chooseBodyTarget(EnemyCombatQueue& wave, const std::string& screenId)
    {
        if (!wave.hasActiveEnemies())
        {
            MessageScreen::show(
                "AUCUNE CIBLE",
                screenId + ".empty",
                {"Aucun corps ennemi n'est assez proche pour chercher une faille."},
                false
            );
            return -1;
        }

        MenuScreen targetScreen("FAILLE DE CORPS", screenId + ".target");
        targetScreen.addLine("Choisis la cible dont tu veux lire la matière, les appuis ou le point faible.");
        targetScreen.addBackOption("Retour", screenId + ".back");

        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            const Monster& enemy = wave.getActiveEnemy(index);
            targetScreen.addOption(
                index + 1,
                enemy.getName() + " - PV " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()),
                describeBodyTargetHint(enemy),
                true,
                screenId + ".body_target." + std::to_string(index + 1)
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return -1;
        }

        const int targetIndex = choice - 1;
        if (!wave.isActiveIndexValid(targetIndex))
        {
            MessageScreen::show(
                "CIBLE INTROUVABLE",
                screenId + ".invalid",
                {"La cible choisie n'est plus dans la ligne active."},
                false
            );
            return -1;
        }

        return targetIndex;
    }

    int chooseTacticalTarget(EnemyCombatQueue& wave, const std::string& screenId)
    {
        if (!wave.hasActiveEnemies())
        {
            MessageScreen::show(
                "AUCUNE CIBLE",
                screenId + ".empty",
                {"Aucun adversaire actif ne peut être ciblé pour l'instant."},
                false
            );
            return -1;
        }

        MenuScreen targetScreen("CIBLE TACTIQUE", screenId + ".target");
        targetScreen.addLine("Choisis sur qui appliquer l'action tactique.");
        targetScreen.addBackOption("Retour", screenId + ".back");

        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            const Monster& enemy = wave.getActiveEnemy(index);
            targetScreen.addOption(
                index + 1,
                enemy.getName() + " - PV " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()),
                "Cible active dans la vague.",
                true,
                screenId + ".target." + std::to_string(index + 1)
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return -1;
        }

        const int targetIndex = choice - 1;
        if (!wave.isActiveIndexValid(targetIndex))
        {
            MessageScreen::show(
                "CIBLE INTROUVABLE",
                screenId + ".invalid",
                {"La cible choisie n'est plus dans la ligne active."},
                false
            );
            return -1;
        }

        return targetIndex;
    }

    bool openTacticalActionMenu(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        const int lanternCount = player.getInventory().countMaterialById("fire_lantern")
            + player.getInventory().countMaterialById("mycelium_lantern");
        const bool hasLantern = lanternCount > 0;
        const bool hasEnemy = wave.hasActiveEnemies();
        const bool hasOpening = hasEnemy && waveHasExploitableOpening(wave);
        const bool hasTrapMaterial = hasImprovisedTrapMaterial(player);
        const bool hasWeaponPreparation = player.hasEquippedWeapon() && hasWeaponCoatingMaterial(player);
        const bool hasFormation = hasEnemy && waveHasFormationToBreak(wave);
        const bool hasSignal = hasEnemy && waveHasSignalToCut(wave);
        const bool hasStatusCombo = hasEnemy && waveHasStatusChain(wave);
        const bool hasMobileTarget = hasEnemy && waveHasLowOrMobileTarget(wave);
        const bool hasFocusTarget = hasEnemy && waveHasFocusOrSupportTarget(wave);
        const bool hasReachTarget = hasEnemy && waveHasReachOrBacklineTarget(wave);
        const bool hasShieldTarget = hasEnemy && waveHasShieldOrArmorTarget(wave);
        const bool hasOccultTarget = hasEnemy && waveHasOccultAnchorTarget(wave);
        const bool canShadowStep = playerHasShadowStepAffinity(player);
        const bool canRogueFeint = playerHasRogueTechniqueAffinity(player);
        const bool canGuardianOath = playerHasGuardianTechniqueAffinity(player);
        const bool canArcaneChannel = playerHasArcaneTechniqueAffinity(player);
        const bool canBattleSuture = playerHasSupportTechniqueAffinity(player);
        const bool canStoppingShot = playerHasSkirmisherTechniqueAffinity(player);
        const bool canMasteredRage = playerHasWarriorBurstAffinity(player);
        const bool canBattleOrder = playerHasCommanderTechniqueAffinity(player);
        const bool canBreathTotem = playerHasNatureTechniqueAffinity(player);
        const bool canWorkshopBomb = playerHasArtificeTechniqueAffinity(player);
        const bool canSteelPrayer = playerHasSacredTechniqueAffinity(player);
        const bool canInnerMantra = playerHasMonkTechniqueAffinity(player);
        const bool canTrickImage = playerHasIllusionTechniqueAffinity(player);
        const bool canSummoningLink = playerHasSummonerTechniqueAffinity(player);
        const bool canBloodPact = playerHasBloodTechniqueAffinity(player);
        const bool canFieldRemedy = playerHasMedicOrCraftTechniqueAffinity(player);
        const bool canElementalBlade = playerHasElementalistTechniqueAffinity(player);
        const bool canProtectiveCircle = playerHasWardenTechniqueAffinity(player);
        const bool canBindingShot = playerHasSkirmisherTechniqueAffinity(player);
        const bool canInspiringChant = playerHasBardTechniqueAffinity(player);
        const bool canBeastInstinct = playerHasBeastTechniqueAffinity(player);
        const bool canBladeDance = playerHasDuelTechniqueAffinity(player);

        MenuScreen screen("ACTIONS TACTIQUES", "wave.combat.tactical_actions");
        screen.addSubtitle("Objets, terrain, observation active et ouvertures");
        screen.addLine("Chaque geste prend le tour, mais change la façon dont la ligne respire.");
        screen.addLine("Les actifs restent choisis. Les usages réussis montent leur maîtrise, et certains passifs naissent après 3 essais.");
        screen.addLine("Les paliers donnent surtout fiabilité, précision, souffle ou petit rythme : pas de bonus absurde trop tôt.");
        screen.addBackOption("Retour", "wave.combat.tactical_actions.back");
        screen.addOption(1, "Lancer une lanterne sur une cible", "Consomme une lanterne. Dégâts légers et effet de zone selon le type." + tacticalMasteryMenuHint(player, "lanterne_cible"), hasLantern && hasEnemy, "wave.combat.tactical_actions.lantern_target");
        screen.addOption(2, "Jeter une lanterne au sol", "Consomme une lanterne. Gêne plusieurs ennemis actifs et prépare le terrain." + tacticalMasteryMenuHint(player, "lanterne_sol"), hasLantern && hasEnemy, "wave.combat.tactical_actions.lantern_floor");
        screen.addOption(3, "Coup de pied / repousser", "Petit dégât, affaiblit brièvement une cible et force un mauvais appui." + tacticalMasteryMenuHint(player, "coup_de_pied"), hasEnemy, "wave.combat.tactical_actions.kick");
        screen.addOption(4, "Poussière / objet au sol", "Gêne une cible sans consommer de ressource spéciale." + tacticalMasteryMenuHint(player, "poussiere"), hasEnemy, "wave.combat.tactical_actions.dust");
        screen.addOption(5, "Observer activement", "Révèle la cible et applique une petite vulnérabilité tactique." + tacticalMasteryMenuHint(player, "observation_active"), hasEnemy, "wave.combat.tactical_actions.observe");
        screen.addOption(6, "Exploiter une ouverture", "Transforme blessure/statut/vulnérabilité en réaction tactique. Disponible si une cible est ouverte." + tacticalMasteryMenuHint(player, "exploitation_ouverture"), hasOpening, "wave.combat.tactical_actions.exploit_opening");
        screen.addOption(7, "Piège improvisé de terrain", "Consomme un petit matériau et gêne la ligne ennemie avec un bricolage rapide, distinct d'une vraie bombe d'atelier." + tacticalMasteryMenuHint(player, "piege_artisan"), hasEnemy && hasTrapMaterial, "wave.combat.tactical_actions.craft_trap");
        screen.addOption(8, "Enduire / fusionner vite l'arme", "Consomme un composant pour préparer l'arme et appliquer un effet de terrain sur une cible." + tacticalMasteryMenuHint(player, "preparation_arme"), hasEnemy && hasWeaponPreparation, "wave.combat.tactical_actions.weapon_coating");
        screen.addOption(9, "Brise-garde", "Contrôle court : dégâts faibles, vulnérabilité et affaiblissement. Plus utile contre les cibles déjà solides." + tacticalMasteryMenuHint(player, "brise_garde"), hasEnemy, "wave.combat.tactical_actions.guard_break");
        screen.addOption(10, "Tenir la ligne / couvrir", "Action de soutien : garde, provocation courte et bonus défensif sans taper plus fort." + tacticalMasteryMenuHint(player, "posture_soutien"), true, "wave.combat.tactical_actions.support_stance");
        screen.addOption(11, "Casser la formation", "Perturbe plusieurs ennemis, retire certaines postures et prépare une vraie reprise de tempo." + tacticalMasteryMenuHint(player, "rupture_formation"), hasFormation, "wave.combat.tactical_actions.break_formation");
        screen.addOption(12, "Couper le signal", "Bouscule un appelant ou une race de bande avant que l'appel ne parte vraiment." + tacticalMasteryMenuHint(player, "coupe_signal"), hasSignal, "wave.combat.tactical_actions.cut_signal");
        screen.addOption(13, "Viser une faille de corps", "Cherche une jointure, un noyau, un ventre mou ou une zone fragile selon la matière." + tacticalMasteryMenuHint(player, "lecture_corps"), hasEnemy, "wave.combat.tactical_actions.body_weakpoint");
        screen.addOption(14, "Forcer une chaîne d'états", "Fait réagir plusieurs blessures, venins, brûlures ou failles déjà présents sur une cible." + tacticalMasteryMenuHint(player, "chaine_etats"), hasStatusCombo, "wave.combat.tactical_actions.status_chain");
        screen.addOption(15, "Marquer une proie", "Désigne une cible, révèle sa lecture et prépare le prochain vrai impact contre elle." + tacticalMasteryMenuHint(player, "marque_proie"), hasEnemy, "wave.combat.tactical_actions.mark_prey");
        screen.addOption(16, "Sens du retrait", "Recule proprement : garde courte, souffle repris et ligne moins facile à punir." + tacticalMasteryMenuHint(player, "retrait_controle"), true, "wave.combat.tactical_actions.controlled_retreat");
        screen.addOption(17, "Lire la menace", "Repère le rôle dangereux d'une cible et prépare la réponse adaptée." + tacticalMasteryMenuHint(player, "lecture_menace"), hasEnemy, "wave.combat.tactical_actions.threat_read");
        screen.addOption(18, "Lecture du balayage bas", "Geste large contre les petites cibles, ailes basses, pattes ou appuis nerveux." + tacticalMasteryMenuHint(player, "balayage_bas"), hasMobileTarget, "wave.combat.tactical_actions.low_sweep");
        screen.addOption(19, "Étouffer la concentration", "Bouscule un soigneur, lanceur, porteur d'effet ou profil instable avant son prochain geste." + tacticalMasteryMenuHint(player, "rupture_concentration"), hasFocusTarget, "wave.combat.tactical_actions.focus_break");
        screen.addOption(20, "Rompre l'allonge", "Casse la distance d'un tireur, lancier, duelliste ou lanceur trop bien placé." + tacticalMasteryMenuHint(player, "rupture_allonge"), hasReachTarget, "wave.combat.tactical_actions.reach_break");
        screen.addOption(21, "Percer le rempart", "Cherche un angle contre bouclier, carapace, armure lourde ou corps verrouillé." + tacticalMasteryMenuHint(player, "perce_rempart"), hasShieldTarget, "wave.combat.tactical_actions.shield_pierce");
        screen.addOption(22, "Rompre l'ancrage occulte", "Perturbe une marque, une aura, un mort-vivant, un spectre ou une énergie instable." + tacticalMasteryMenuHint(player, "rupture_ancrage"), hasOccultTarget, "wave.combat.tactical_actions.anchor_break");
        screen.addOption(23, "Souffle rallié", "Resserre ton rythme : garde, précision et moral sans chercher un dégât immédiat." + tacticalMasteryMenuHint(player, "souffle_ralliement"), true, "wave.combat.tactical_actions.rally_breath");
        screen.addOption(24, "Élan canalisé", "Attaque vive : dégâts directs puis souffle offensif si le mouvement traverse vraiment." + tacticalMasteryMenuHint(player, "frappe_elan"), hasEnemy, "wave.combat.tactical_actions.momentum_strike");
        screen.addOption(25, "Réflexe de voile", "Protection courte : garde, résistance élémentaire et petite régénération de survie." + tacticalMasteryMenuHint(player, "voile_urgence"), true, "wave.combat.tactical_actions.emergency_ward");
        screen.addOption(26, "Instabilité apprivoisée", "Geste magique ou alchimique : petit impact et effet élémentaire imprévisible." + tacticalMasteryMenuHint(player, "etincelle_instable"), hasEnemy, "wave.combat.tactical_actions.wild_spark");
        screen.addOption(27, "Voix de guerre", "Buff de tempo : puissance, précision et garde courte avant la prochaine mêlée." + tacticalMasteryMenuHint(player, "cri_guerre"), true, "wave.combat.tactical_actions.war_cry");
        screen.addOption(28, "Pas de l'ombre", "Technique agile : voleur, assassin, roublard, rôdeur, duelliste ou appuis naturellement félins/renards." + tacticalMasteryMenuHint(player, "pas_ombre"), canShadowStep, "wave.combat.tactical_actions.shadow_step");
        screen.addOption(29, "Coup en arc", "Attaque large : touche jusqu'à deux ennemis actifs avec moins de précision mais plus de présence." + tacticalMasteryMenuHint(player, "coup_arc"), hasEnemy, "wave.combat.tactical_actions.arc_sweep");
        screen.addOption(30, "Signe de vigueur", "Soutien personnel : souffle, soin lent et purge légère d'un mauvais état." + tacticalMasteryMenuHint(player, "signe_vigueur"), true, "wave.combat.tactical_actions.vigor_sign");
        screen.addOption(31, "Feinte sournoise", "Technique de lames basses : faux angle, petite entaille et ouverture discrète." + tacticalMasteryMenuHint(player, "feinte_sournoise"), hasEnemy && canRogueFeint, "wave.combat.tactical_actions.rogue_feint");
        screen.addOption(32, "Suture de bataille", "Technique de soutien : referme un peu, stabilise et retire parfois un mauvais état." + tacticalMasteryMenuHint(player, "suture_bataille"), canBattleSuture, "wave.combat.tactical_actions.battle_suture");
        screen.addOption(33, "Canalisation arcanique", "Technique de canalisateur : prépare le prochain geste magique ou alchimique sans frapper tout de suite." + tacticalMasteryMenuHint(player, "canalisation_arcanique"), canArcaneChannel, "wave.combat.tactical_actions.arcane_channel");
        screen.addOption(34, "Serment du rempart", "Technique de front : attire les regards, durcit la garde et tient la ligne." + tacticalMasteryMenuHint(player, "serment_rempart"), canGuardianOath, "wave.combat.tactical_actions.rampart_oath");
        screen.addOption(35, "Tir d'arrêt", "Technique de pisteur : projectile ou geste précis pour stopper une avance." + tacticalMasteryMenuHint(player, "tir_arret"), hasEnemy && canStoppingShot, "wave.combat.tactical_actions.stopping_shot");
        screen.addOption(36, "Rage maîtrisée", "Technique de front : frappe dure puis canalise l'élan au lieu de le laisser déborder." + tacticalMasteryMenuHint(player, "rage_maitrisee"), hasEnemy && canMasteredRage, "wave.combat.tactical_actions.mastered_rage");
        screen.addOption(37, "Ordre de bataille", "Technique de commandement : rythme, précision et tenue de ligne sans frapper directement." + tacticalMasteryMenuHint(player, "ordre_bataille"), canBattleOrder, "wave.combat.tactical_actions.battle_order");
        screen.addOption(38, "Totem de souffle", "Technique de nature : ancre courte, souffle lent et protection vivante." + tacticalMasteryMenuHint(player, "totem_souffle"), canBreathTotem, "wave.combat.tactical_actions.breath_totem");
        screen.addOption(39, "Bombe d'atelier", "Technique d'artisan : projectile bricolé, effet instable et vrai impact de terrain." + tacticalMasteryMenuHint(player, "bombe_atelier"), hasEnemy && canWorkshopBomb, "wave.combat.tactical_actions.workshop_bomb");
        screen.addOption(40, "Prière d'acier", "Technique sacrée : protection, souffle et purification courte sous pression." + tacticalMasteryMenuHint(player, "priere_acier"), canSteelPrayer, "wave.combat.tactical_actions.steel_prayer");
        screen.addOption(41, "Mantra intérieur", "Technique martiale : souffle, précision, garde et purge d'un mauvais rythme." + tacticalMasteryMenuHint(player, "mantra_interieur"), canInnerMantra, "wave.combat.tactical_actions.inner_mantra");
        screen.addOption(42, "Image trompeuse", "Technique d'illusion : laisse une fausse ouverture et revient par un angle plus sûr." + tacticalMasteryMenuHint(player, "image_trompeuse"), hasEnemy && canTrickImage, "wave.combat.tactical_actions.trick_image");
        screen.addOption(43, "Lien d'invocation", "Technique de lien : ancre une présence alliée ou spectrale autour de la ligne." + tacticalMasteryMenuHint(player, "lien_invocation"), canSummoningLink, "wave.combat.tactical_actions.summoning_link");
        screen.addOption(44, "Pacte sanguin", "Technique risquée : paie un peu de chair pour obtenir élan, morsure et souffle noir." + tacticalMasteryMenuHint(player, "pacte_sanguin"), hasEnemy && canBloodPact, "wave.combat.tactical_actions.blood_pact");
        screen.addOption(45, "Remède de fortune", "Technique de terrain : stabilise le corps avec soin, dosage et gestes rapides." + tacticalMasteryMenuHint(player, "remede_fortune"), canFieldRemedy, "wave.combat.tactical_actions.field_remedy");
        screen.addOption(46, "Lame élémentaire", "Technique élémentaire : charge l'arme ou la main puis frappe avec une couleur instable." + tacticalMasteryMenuHint(player, "lame_elementaire"), hasEnemy && canElementalBlade, "wave.combat.tactical_actions.elemental_blade");
        screen.addOption(47, "Cercle protecteur", "Technique de garde : trace un cercle court qui protège le corps et calme la ligne." + tacticalMasteryMenuHint(player, "cercle_protecteur"), canProtectiveCircle, "wave.combat.tactical_actions.protective_circle");
        screen.addOption(48, "Flèche entravante", "Technique de tireur : cloue un appui, une aile ou une fuite avec un tir précis." + tacticalMasteryMenuHint(player, "fleche_entravante"), hasEnemy && canBindingShot, "wave.combat.tactical_actions.binding_shot");
        screen.addOption(49, "Chant revigorant", "Technique de présence : relève le souffle, nettoie la peur et remet du rythme." + tacticalMasteryMenuHint(player, "chant_revigorant"), canInspiringChant, "wave.combat.tactical_actions.inspiring_chant");
        screen.addOption(50, "Instinct de bête", "Technique sauvage : laisse parler les réflexes, gagne l'élan et mord une ouverture." + tacticalMasteryMenuHint(player, "instinct_bete"), hasEnemy && canBeastInstinct, "wave.combat.tactical_actions.beast_instinct");
        screen.addOption(51, "Danse de lame", "Technique de duel : 2 coups avec une arme légère, 3 coups si deux armes légères sont prêtes." + tacticalMasteryMenuHint(player, "danse_lame"), hasEnemy && canBladeDance, "wave.combat.tactical_actions.blade_dance");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (!hasEnemy)
        {
            MessageScreen::show(
                "AUCUN ADVERSAIRE",
                "wave.combat.tactical_actions.no_enemy",
                {"Il n'y a aucune cible active pour utiliser une action tactique."},
                false
            );
            return false;
        }

        if (choice == 1)
        {
            TacticalLantern lantern;
            if (!consumeTacticalLantern(player, lantern))
            {
                MessageScreen::show(
                    "AUCUNE LANTERNE",
                    "wave.combat.tactical_actions.no_lantern_target",
                    {"Tu n'as plus de lanterne utilisable dans l'inventaire."},
                    false
                );
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.lantern_target");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int masteryBonus = tacticalMasterySoftBonus(player, "lanterne_cible");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "lanterne_cible");
            const int damage = std::max(3, player.getLevel() / 3 + random.between(3, 7) + masteryBonus);
            target.takeDamage(damage);

            if (lantern.mycelium)
            {
                target.applyWeakening(2 + masteryDuration, 12 + masteryBonus);
                target.applyVulnerability(2, 8 + masteryBonus);
                target.revealStats();
            }
            else
            {
                target.applyBurning(2 + masteryDuration, std::max(2, player.getLevel() / 16 + 2 + masteryBonus / 2));
                target.applyWeakening(1 + masteryDuration, 8 + masteryBonus);
            }

            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "lanterne_cible", "Lanterne lancée sur une cible", 1);
            updateTacticalLearning(player, "lanterne_cible");

            MessageScreen::show(
                "LANTERNE LANCÉE",
                "wave.combat.tactical_actions.lantern_target.result",
                {
                    player.getName() + " lance une " + lantern.name + " sur " + targetName + ".",
                    "Dégâts directs : " + std::to_string(damage) + ".",
                    lantern.mycelium
                        ? "Le mycélium trouble la cible : faiblesse, vulnérabilité et observation révélée."
                        : "Les flammes brûlent la cible et réduisent brièvement sa pression offensive.",
                    tacticalMasteryShortResultLine(player, "lanterne_cible")
                },
                false
            );
            return true;
        }

        if (choice == 2)
        {
            TacticalLantern lantern;
            if (!consumeTacticalLantern(player, lantern))
            {
                MessageScreen::show(
                    "AUCUNE LANTERNE",
                    "wave.combat.tactical_actions.no_lantern_floor",
                    {"Tu n'as plus de lanterne utilisable dans l'inventaire."},
                    false
                );
                return false;
            }

            const int activeCount = wave.getActiveEnemyCount();
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " jette une " + lantern.name + " au sol pour contrôler l'espace.");

            for (int index = 0; index < activeCount && index < wave.getActiveEnemyCount(); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                const int masteryBonus = tacticalMasterySoftBonus(player, "lanterne_sol");
                const int masteryDuration = tacticalMasteryDurationBonus(player, "lanterne_sol");
                const int damage = (lantern.mycelium ? random.between(1, 3) : random.between(2, 5)) + std::max(0, masteryBonus / 2);
                enemy.takeDamage(damage);
                if (lantern.mycelium)
                {
                    enemy.applyWeakening(2 + masteryDuration, 9 + masteryBonus);
                    enemy.applyVulnerability(1, 5 + masteryBonus);
                    enemy.revealStats();
                }
                else
                {
                    enemy.applyBurning(1 + masteryDuration, 2 + masteryBonus / 2);
                    enemy.applyWeakening(2 + masteryDuration, 7 + masteryBonus);
                }
                lines.push_back("- " + enemy.getName() + " subit " + std::to_string(damage) + " dégâts et perd en stabilité.");
            }

            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "lanterne_sol", "Lanterne jetée au sol", 1);
            updateTacticalLearning(player, "lanterne_sol");
            lines.push_back(tacticalMasteryShortResultLine(player, "lanterne_sol"));

            MessageScreen::show(
                "TERRAIN PERTURBÉ",
                "wave.combat.tactical_actions.lantern_floor.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 3)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.kick");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool drilled = player.hasPassiveSkill("footwork_drill");
            const int masteryBonus = tacticalMasterySoftBonus(player, "coup_de_pied");
            const int damage = std::max(1, player.getLevel() / 4 + random.between(1, drilled ? 6 : 4) + masteryBonus);
            target.takeDamage(damage);
            target.applyWeakening(drilled ? 2 : 1, (drilled ? 12 : 7) + masteryBonus);
            if (drilled)
            {
                target.applyVulnerability(1, 5);
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "coup_de_pied", "Coup de pied tactique", 1);
            updateTacticalLearning(player, "coup_de_pied");

            MessageScreen::show(
                "COUP DE PIED",
                "wave.combat.tactical_actions.kick.result",
                {
                    player.getName() + " repousse " + targetName + " d'un coup sec.",
                    "Dégâts : " + std::to_string(damage) + ".",
                    drilled ? "L'entraînement au jeu de jambes rend le mouvement plus propre." : "Ce n'est pas élégant, mais ça change le rythme.",
                    tacticalMasteryShortResultLine(player, "coup_de_pied")
                },
                false
            );
            return true;
        }

        if (choice == 4)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.dust");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int masteryBonus = tacticalMasterySoftBonus(player, "poussiere");
            const int vulnerabilityChance = 35 + tacticalMasteryChanceBonus(player, "poussiere");
            target.applyWeakening(2, 10 + masteryBonus);
            if (random.between(1, 100) <= vulnerabilityChance)
            {
                target.applyVulnerability(1, 6 + masteryBonus);
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            player.recordCanonicalEvent("actions_tactiques_combat", "poussiere", "Poussière tactique", 1);
            updateTacticalLearning(player, "poussiere");

            MessageScreen::show(
                "POUSSIÈRE DANS LES YEUX",
                "wave.combat.tactical_actions.dust.result",
                {
                    player.getName() + " utilise le sol contre " + targetName + ".",
                    "La cible est gênée pendant un court moment.",
                    "C'est une petite action, mais elle évite le spam attaque normale.",
                    tacticalMasteryShortResultLine(player, "poussiere")
                },
                false
            );
            return true;
        }

        if (choice == 5)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.observe");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool rogueReader = playerClassContainsAny(player, {"voleur", "roublard", "brigand", "assassin", "rôdeur", "rodeur", "trappeur"});
            const bool trained = player.hasActiveSkill("combat_observation")
                || player.hasPassiveSkill("combat_observation")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("lock_reader");
            target.revealStats();
            target.applyVulnerability(trained ? 2 : 1, trained ? 10 : 6);
            player.recordCanonicalEvent("actions_tactiques_combat", "observation_active", "Observation active", 1);
            if (rogueReader)
            {
                player.recordCanonicalEvent("actions_tactiques_combat", "lecture_voleur", "Lecture de voleur", 1);
            }
            updateTacticalLearning(player, "observation_active");

            std::vector<std::string> observeLines = {
                player.getName() + " lit les mouvements de " + targetName + ".",
                trained ? "Ton entraînement d'observation transforme l'analyse en vraie ouverture." : "La cible est révélée et légèrement plus facile à punir.",
                "Cette action consomme le tour, contrairement au simple menu d'information."
            };

            const std::vector<std::string> profileLines = MonsterBehaviorProfileCatalog::buildObservationLines(
                target,
                trained || rogueReader
            );
            observeLines.insert(observeLines.end(), profileLines.begin(), profileLines.end());

            if (rogueReader && canonicalEventCount(player, "actions_tactiques_combat", "lecture_voleur") >= 2)
            {
                if (player.unlockPassiveSkill("lock_reader", "Lecture des serrures et failles"))
                {
                    observeLines.push_back("Apprentissage : ton profil discret transforme l'observation en lecture de serrures, pièges et failles.");
                    player.grantTitle("Œil de serrure");
                }
                else
                {
                    observeLines.push_back("Lecture de voleur : tu confirmes les failles, pièges et gestes suspects plus vite qu'un profil classique.");
                }
            }

            MessageScreen::show(
                "OBSERVATION ACTIVE",
                "wave.combat.tactical_actions.observe.result",
                observeLines,
                false
            );
            return true;
        }

        if (choice == 6)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.exploit_opening");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            if (!hasExploitableOpening(target))
            {
                MessageScreen::show(
                    "AUCUNE OUVERTURE",
                    "wave.combat.tactical_actions.exploit_opening.no_opening",
                    {
                        targetName + " n'a pas encore de blessure, statut ou faille assez visible.",
                        "Prépare d'abord une ouverture avec observation, poussière, lanterne, brise-garde ou une compétence."
                    },
                    false
                );
                return false;
            }

            const bool expert = player.hasPassiveSkill("terrain_reader") || player.hasPassiveSkill("combat_improviser");
            int damage = std::max(3, player.getLevel() / 3 + random.between(3, expert ? 10 : 7));
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " exploite l'ouverture visible sur " + targetName + ".");

            if (target.hasBurning() && target.hasFrost())
            {
                damage += 8 + player.getLevel() / 9;
                target.applyWeakening(2, 14 + player.getLevel() / 16);
                lines.push_back("Réaction : choc thermique. La cible perd brutalement son rythme.");
            }
            else if (target.hasPoison() && target.hasBleeding())
            {
                damage += 6 + player.getLevel() / 10;
                target.applyVulnerability(2, 12 + player.getLevel() / 18);
                lines.push_back("Réaction : plaie infectée. La blessure devient une vraie faille.");
            }
            else if (target.hasShock() && target.hasVulnerability())
            {
                damage += 7 + player.getLevel() / 11;
                ElementalAffinitySystem::applyShock(target, 1);
                lines.push_back("Réaction : conduite forcée. Le choc se propage dans l'ouverture.");
            }
            else if (target.hasWeakening() && target.hasVulnerability())
            {
                damage += 5 + player.getLevel() / 12;
                target.applyWeakening(1, 8 + player.getLevel() / 20);
                lines.push_back("Réaction : brèche punie. La cible encaisse mal le repositionnement.");
            }
            else if (target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 45)
            {
                damage += 4 + player.getLevel() / 14;
                ElementalAffinitySystem::applyBleeding(target, 2, std::max(1, player.getLevel() / 25 + 1));
                lines.push_back("Réaction : cible blessée. Le coup vise une zone déjà fragilisée.");
            }
            else
            {
                target.applyVulnerability(1, 6 + player.getLevel() / 22);
                lines.push_back("Réaction simple : l'altération existante est transformée en petite vulnérabilité.");
            }

            if (expert)
            {
                damage += 3;
                lines.push_back("Lecture du terrain : bonus léger grâce à l'expérience tactique accumulée.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "exploitation_ouverture", "Ouverture exploitée en combat", 1);
            updateTacticalLearning(player, "exploitation_ouverture");
            lines.push_back("Dégâts tactiques : " + std::to_string(damage) + ".");

            MessageScreen::show(
                "OUVERTURE EXPLOITÉE",
                "wave.combat.tactical_actions.exploit_opening.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 7)
        {
            const std::string materialName = consumeImprovisedTrapMaterial(player);
            if (materialName.empty())
            {
                MessageScreen::show(
                    "MATÉRIAU MANQUANT",
                    "wave.combat.tactical_actions.craft_trap.no_material",
                    {
                        "Il te faut au moins un petit composant : métal rouillé, cuir usé, os fissuré, résidu de slime ou poussière arcanique.",
                        "L'idée est d'ouvrir un premier vrai usage artisan en combat sans créer un bouton gratuit."
                    },
                    false
                );
                return false;
            }

            const bool improvisor = player.hasPassiveSkill("combat_improviser");
            const int masteryBonus = tacticalMasterySoftBonus(player, "piege_artisan");
            const int activeCount = wave.getActiveEnemyCount();
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " transforme un " + materialName + " en piège de fortune.");
            lines.push_back("Ce n'est pas une bombe magique : c'est du bricolage sale, utile pour casser la routine.");

            for (int index = 0; index < activeCount && index < wave.getActiveEnemyCount(); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                const int damage = std::max(1, player.getLevel() / 5 + random.between(1, improvisor ? 6 : 4) + std::max(0, masteryBonus / 2));
                enemy.takeDamage(damage);
                enemy.applyWeakening(improvisor ? 2 : 1, (improvisor ? 12 : 8) + masteryBonus);
                if (index == 0 || improvisor)
                {
                    enemy.applyVulnerability(1, improvisor ? 8 : 5);
                }
                lines.push_back("- " + enemy.getName() + " est gêné : " + std::to_string(damage) + " dégâts, stabilité réduite.");
            }

            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "piege_artisan", "Piège improvisé d'artisan", 1);
            updateTacticalLearning(player, "piege_artisan");
            lines.push_back(tacticalMasteryShortResultLine(player, "piege_artisan"));

            MessageScreen::show(
                "PIÈGE IMPROVISÉ",
                "wave.combat.tactical_actions.craft_trap.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 8)
        {
            if (!player.hasEquippedWeapon())
            {
                MessageScreen::show(
                    "ARME MANQUANTE",
                    "wave.combat.tactical_actions.weapon_coating.no_weapon",
                    {"Il faut une arme équipée pour faire une préparation rapide."},
                    false
                );
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.weapon_coating");
            if (targetIndex < 0)
            {
                return false;
            }

            PreparedWeaponCoating coating = consumeWeaponCoatingMaterial(player);
            if (coating.id.empty())
            {
                MessageScreen::show(
                    "COMPOSANT MANQUANT",
                    "wave.combat.tactical_actions.weapon_coating.no_component",
                    {
                        "Il te faut au moins un composant compatible : résidu de slime, poussière arcanique, huile ambrée, résine d'écho, sel lunaire ou venin déjà préparé.",
                        "La fusion rapide reste volontairement légère : utile en combat, mais pas gratuite."
                    },
                    false
                );
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const WeaponType weaponType = player.getEquippedWeapon().getType();
            const bool daggerWork = weaponType == WeaponType::Dagger;
            const bool bowWork = weaponType == WeaponType::Bow;
            const bool staffWork = weaponType == WeaponType::Staff;
            const bool prepared = player.hasPassiveSkill("field_weapon_crafter") || player.hasPassiveSkill("tactical_toolbox");
            const int masteryBonus = tacticalMasterySoftBonus(player, "preparation_arme");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "preparation_arme");
            int damage = std::max(2, player.getLevel() / 4 + random.between(2, prepared ? 8 : 5) + masteryBonus);
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " prépare rapidement son arme avec " + coating.name + ".");
            lines.push_back("Procédé : " + coating.label + ". C'est une fusion de terrain, pas un enchantement permanent.");

            if (coating.id == "slime_residue" || coating.id == "venom_arrows")
            {
                ElementalAffinitySystem::applyPoison(target, (prepared ? 4 : 3) + masteryDuration, 2 + player.getLevel() / 22 + masteryBonus / 2);
                target.applyWeakening(1 + masteryDuration, (prepared ? 10 : 6) + masteryBonus);
                lines.push_back("Effet : la lame colle une toxine faible dans l'ouverture.");
            }
            else if (coating.id == "arcane_dust")
            {
                ElementalAffinitySystem::applyShock(target, (prepared ? 3 : 2) + masteryDuration);
                target.applyVulnerability(1, (prepared ? 9 : 5) + masteryBonus);
                lines.push_back("Effet : la poussière accroche une étincelle instable sur la cible.");
            }
            else if (coating.id == "amber_tempering_oil")
            {
                player.applyPowerBoost(2, prepared ? 14 : 9);
                player.applyPrecisionBoost(2, prepared ? 2 : 1);
                damage += (prepared ? 4 : 2) + std::max(0, masteryBonus / 2);
                lines.push_back("Effet : l'huile stabilise le prochain rythme offensif du personnage.");
            }
            else if (coating.id == "echoing_resin")
            {
                target.revealStats();
                target.applyVulnerability(2, (prepared ? 11 : 7) + masteryBonus);
                player.applyPrecisionBoost(2, 1 + std::min(1, masteryBonus / 3));
                lines.push_back("Effet : la résine transmet les vibrations et révèle mieux les failles.");
            }
            else if (coating.id == "moonlit_salt")
            {
                ElementalAffinitySystem::applyFrost(target, (prepared ? 3 : 2) + masteryDuration);
                target.applyWeakening(1 + masteryDuration, (prepared ? 10 : 6) + masteryBonus);
                lines.push_back("Effet : le sel lunaire mord les articulations et ralentit la cible.");
            }

            if (daggerWork)
            {
                ElementalAffinitySystem::applyBleeding(target, 2, std::max(1, player.getLevel() / 24 + 1));
                damage += 2;
                lines.push_back("Synergie dague : la préparation devient une dague sale, proche de l'idée de dague empoisonnée.");
            }
            else if (bowWork)
            {
                damage += 1;
                target.applyVulnerability(1, 5);
                lines.push_back("Synergie distance : le projectile préparé garde mieux son effet sur l'impact.");
            }
            else if (staffWork)
            {
                player.applyElementalWard(2, prepared ? 14 : 8);
                lines.push_back("Synergie catalyseur : une partie du composant sert aussi de garde élémentaire temporaire.");
            }

            if (prepared)
            {
                damage += 2;
                lines.push_back("Maîtrise artisanale : la préparation est plus stable grâce à l'habitude gagnée en combat.");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "preparation_arme"));

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "preparation_arme", "Arme préparée en combat", 1);
            player.recordCanonicalEvent("artisanat_combat", coating.id, coating.name, 1);
            updateTacticalLearning(player, "preparation_arme");
            lines.push_back("Dégâts de finition : " + std::to_string(damage) + ".");

            MessageScreen::show(
                "ARME PRÉPARÉE",
                "wave.combat.tactical_actions.weapon_coating.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 9)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.guard_break");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool trained = player.hasPassiveSkill("guard_breaker")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"chevalier", "guerrier", "briseur", "colosse", "paladin", "soldat"});
            const bool wasDefending = target.isInDefensePosture();
            const std::string brokenPostureLabel = target.getDefensePostureLabel();
            const bool heavyTarget = target.getMaxHp() >= player.getMaxHp()
                || target.hasVulnerability()
                || target.hasWeakening()
                || wasDefending;
            const int masteryBonus = tacticalMasterySoftBonus(player, "brise_garde");
            int damage = std::max(1, player.getLevel() / 5 + random.between(1, trained ? 6 : 4) + masteryBonus);
            if (heavyTarget)
            {
                damage += 2;
            }

            if (wasDefending)
            {
                target.clearDefensePosture();
                damage += trained ? 2 : 1;
            }

            const int weaknessChance = std::min(80, (trained ? 42 : 28) + (wasDefending ? 18 : 0) + (heavyTarget ? 8 : 0) + tacticalMasteryChanceBonus(player, "brise_garde"));
            const bool nextHitWeakness = random.between(1, 100) <= weaknessChance;
            if (nextHitWeakness)
            {
                target.applyNextHitVulnerability(2, (trained ? 20 : 14) + masteryBonus);
            }

            target.takeDamage(damage);
            target.applyWeakening(trained ? 2 : 1, (trained ? 13 : 8) + masteryBonus);
            target.applyVulnerability(heavyTarget ? 2 : 1, (trained ? 10 : 6) + masteryBonus);
            if (trained && random.between(1, 100) <= 35 + tacticalMasteryChanceBonus(player, "bombe_atelier"))
            {
                ElementalAffinitySystem::applyBleeding(target, 1, std::max(1, player.getLevel() / 28 + 1));
            }

            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "brise_garde", "Brise-garde tactique", 1);
            if (wasDefending)
            {
                player.recordCanonicalEvent("actions_tactiques_combat", "garde_cassee", "Posture défensive ennemie cassée", 1);
            }
            if (nextHitWeakness)
            {
                player.recordCanonicalEvent("actions_tactiques_combat", "faille_prochain_coup", "Faiblesse créée pour le prochain coup subi", 1);
            }
            updateTacticalLearning(player, "brise_garde");

            std::vector<std::string> guardBreakLines;
            guardBreakLines.push_back(player.getName() + " cherche la garde de " + targetName + " au lieu de frapper au hasard.");
            guardBreakLines.push_back("Dégâts contrôlés : " + std::to_string(damage) + ".");
            if (wasDefending)
            {
                guardBreakLines.push_back("Garde cassée : " + targetName + " perd sa posture " + brokenPostureLabel + ".");
            }
            else
            {
                guardBreakLines.push_back("Aucune posture défensive active à casser, mais l'impact force quand même une mauvaise position.");
            }
            guardBreakLines.push_back(trained ? "Lecture entraînée : la faille tient plus longtemps et peut saigner légèrement." : "La cible est affaiblie et plus facile à punir pendant un court moment.");
            guardBreakLines.push_back(nextHitWeakness ? "Faiblesse ouverte : le prochain coup subi par la cible mordra plus fort." : "La faille tient mal : pas de faiblesse spéciale sur le prochain coup.");
            guardBreakLines.push_back(heavyTarget ? "Cible solide ou déjà ouverte : le geste trouve un meilleur angle." : "Cible normale : effet court, utile surtout pour préparer un prochain tour.");
            guardBreakLines.push_back(tacticalMasteryShortResultLine(player, "brise_garde"));
            RuntimeLog::recordScreen("BRISE-GARDE", "wave.combat.tactical_actions.guard_break.result", guardBreakLines);

            MessageScreen::show(
                "BRISE-GARDE",
                "wave.combat.tactical_actions.guard_break.result",
                guardBreakLines,
                false
            );
            return true;
        }

        if (choice == 10)
        {
            const bool supportTrained = player.hasPassiveSkill("support_rhythm")
                || player.hasPassiveSkill("steady_guard")
                || player.hasPassiveSkill("living_rampart")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"support", "paladin", "protecteur", "gardien", "chevalier", "sénéchal", "senechal", "infirmier"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "posture_soutien");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "posture_soutien");
            player.startProvocation((supportTrained ? 3 : 2) + masteryDuration);
            player.applyElementalWard(2 + masteryDuration, (supportTrained ? 16 : 10) + masteryBonus);
            player.applyPrecisionBoost(2, (supportTrained ? 2 : 1) + std::min(1, masteryBonus / 3));
            if (supportTrained)
            {
                player.applyRegeneration(2, std::max(1, player.getMaxHp() / 40));
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "posture_soutien", "Tenir la ligne", 1);
            updateTacticalLearning(player, "posture_soutien");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " se place pour couvrir la ligne au lieu d'attaquer.");
            lines.push_back("Effet : provocation courte, garde élémentaire et précision légère.");
            if (supportTrained)
            {
                lines.push_back("Rythme de soutien : la posture tient mieux et rend un peu de souffle.");
            }
            else
            {
                lines.push_back("La ligne tient mieux : tu gagnes le temps qu'une attaque directe n'aurait pas donné.");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "posture_soutien"));

            MessageScreen::show(
                "TENIR LA LIGNE",
                "wave.combat.tactical_actions.support_stance.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 11)
        {
            const bool trained = player.hasPassiveSkill("formation_breaker")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"chevalier", "guerrier", "briseur", "soldat", "mercenaire", "stratège", "stratege", "paladin"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "rupture_formation");
            int affected = 0;
            int brokenPostures = 0;
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " cherche la ligne ennemie plutôt qu'une cible isolée.");

            for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                const bool wasDefending = enemy.isInDefensePosture();
                const std::string postureLabel = enemy.getDefensePostureLabel();
                const int damage = std::max(1, player.getLevel() / 7 + random.between(1, trained ? 4 : 3) + std::max(0, masteryBonus / 2));

                enemy.takeDamage(damage);
                enemy.applyWeakening(1, (trained ? 9 : 6) + masteryBonus);

                if (wasDefending)
                {
                    enemy.clearDefensePosture();
                    enemy.applyNextHitVulnerability(1, (trained ? 14 : 9) + masteryBonus);
                    ++brokenPostures;
                    lines.push_back("- " + enemy.getName() + " perd sa posture " + postureLabel + ".");
                }
                else if (random.between(1, 100) <= (trained ? 45 : 28) + tacticalMasteryChanceBonus(player, "rupture_formation"))
                {
                    enemy.applyVulnerability(1, (trained ? 7 : 5) + masteryBonus);
                    lines.push_back("- " + enemy.getName() + " recule et laisse une petite faille.");
                }
                else
                {
                    lines.push_back("- " + enemy.getName() + " garde sa place, mais perd un peu de stabilité.");
                }

                ++affected;
            }

            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "rupture_formation", "Formation ennemie cassée", 1);
            if (brokenPostures > 0)
            {
                player.recordCanonicalEvent("actions_tactiques_combat", "postures_groupe_cassees", "Postures ennemies cassées en groupe", brokenPostures);
            }
            updateTacticalLearning(player, "rupture_formation");

            lines.push_back("Cibles perturbées : " + std::to_string(affected) + ".");
            lines.push_back("Postures cassées : " + std::to_string(brokenPostures) + ".");
            lines.push_back(trained ? "Lecture de formation : la rupture ouvre mieux les ennemis coordonnés." : "La ligne ennemie respire moins bien après ce choc de rythme.");
            lines.push_back(tacticalMasteryShortResultLine(player, "rupture_formation"));

            MessageScreen::show(
                "FORMATION CASSÉE",
                "wave.combat.tactical_actions.break_formation.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 12)
        {
            const int targetIndex = chooseSignalTarget(wave, "wave.combat.tactical_actions.cut_signal");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool dedicated = tacticalMonsterIsDedicatedCaller(target);
            const bool weakenedCaller = target.canUseWeakenedReinforcementSignal();
            const bool commonBand = tacticalMonsterIsCommonBandRace(target);
            const bool trained = player.hasPassiveSkill("signal_cut_awareness") || player.hasPassiveSkill("signal_cutter")
                || player.hasPassiveSkill("formation_breaker")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"voleur", "roublard", "assassin", "rôdeur", "rodeur", "mercenaire", "stratège", "stratege", "éclaireur", "eclaireur"});

            int cooldown = 1;
            if (weakenedCaller)
            {
                cooldown = trained ? 5 : 4;
            }
            else if (dedicated)
            {
                cooldown = trained ? 4 : 3;
            }
            else if (commonBand)
            {
                cooldown = trained ? 3 : 2;
            }
            else
            {
                cooldown = trained ? 2 : 1;
            }

            const int masteryBonus = tacticalMasterySoftBonus(player, "coupe_signal");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "coupe_signal");
            const int damage = std::max(1, player.getLevel() / 8 + random.between(1, trained ? 4 : 3) + std::max(0, masteryBonus / 2));
            target.takeDamage(damage);
            target.resetReinforcementLowHpTurns();
            target.setReinforcementEntryCooldown(cooldown + std::min(1, masteryDuration));
            target.applyWeakening((trained ? 2 : 1) + masteryDuration, (trained ? 10 : 6) + masteryBonus);
            if (dedicated || weakenedCaller || random.between(1, 100) <= (trained ? 45 : 25))
            {
                target.applyVulnerability(1 + masteryDuration, (trained ? 8 : 5) + masteryBonus);
            }

            if ((dedicated || weakenedCaller) && random.between(1, 100) <= (trained ? 42 : 24))
            {
                target.applyNextHitVulnerability(1 + masteryDuration, (trained ? 12 : 8) + masteryBonus);
            }

            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "coupe_signal", "Signal ennemi étouffé", 1);
            updateTacticalLearning(player, "coupe_signal");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " coupe la trajectoire de " + targetName + " avant que le signal ne porte.");
            lines.push_back("Impact court : " + std::to_string(damage) + " dégâts.");
            if (weakenedCaller)
            {
                lines.push_back("Le second appel reste possible, mais son rythme se perd pendant " + std::to_string(cooldown) + " tours.");
            }
            else if (dedicated)
            {
                lines.push_back("Le signal du rameuteur se brise pendant " + std::to_string(cooldown) + " tours.");
            }
            else if (commonBand)
            {
                lines.push_back("La panique ne devient pas un vrai appel : l'instinct repart à zéro.");
            }
            else
            {
                lines.push_back("Aucun appel net n'était prêt, mais la cible perd son souffle et sa place.");
            }
            lines.push_back(trained
                ? "Lecture propre : tu frappes le souffle, les yeux ou le geste qui devait prévenir les autres."
                : "Geste brusque : suffisant pour retarder l'appel, mais pas encore assez fin pour tout fermer.");
            lines.push_back(tacticalMasteryShortResultLine(player, "coupe_signal"));

            MessageScreen::show(
                "SIGNAL COUPÉ",
                "wave.combat.tactical_actions.cut_signal.result",
                lines,
                false
            );
            return true;
        }


        if (choice == 13)
        {
            const int targetIndex = chooseBodyTarget(wave, "wave.combat.tactical_actions.body_weakpoint");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const std::string archetype = normalizeTacticalText(profile.archetype);
            const int physicalModifier = profile.physicalDamageModifierPercent;
            const bool fragileBody = physicalModifier >= 8;
            const bool veryFragileBody = physicalModifier >= 20;
            const bool hardBody = physicalModifier <= -8;
            const bool veryHardBody = physicalModifier <= -18;
            const bool slimeBody = archetype.find("slime") != std::string::npos;
            const bool spectralBody = archetype.find("spectral") != std::string::npos;
            const bool trained = player.hasPassiveSkill("body_reader")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("guard_breaker")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"voleur", "roublard", "assassin", "rôdeur", "rodeur", "chasseur", "archer", "artisan", "forgeron", "mercenaire", "moine"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "lecture_corps");
            const int masteryChance = tacticalMasteryChanceBonus(player, "lecture_corps");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "lecture_corps");
            int successChance = 56 + (trained ? 14 : 0) + masteryChance;
            if (target.areStatsVisible())
            {
                successChance += 8;
            }
            if (target.hasWeakening() || target.hasVulnerability() || target.hasNextHitVulnerability())
            {
                successChance += 8;
            }
            if (target.isInDefensePosture())
            {
                successChance += hardBody ? 6 : 3;
            }
            if (veryFragileBody)
            {
                successChance -= 8;
            }
            else if (fragileBody)
            {
                successChance -= 4;
            }
            if (veryHardBody)
            {
                successChance -= 7;
            }
            else if (hardBody)
            {
                successChance -= 4;
            }
            successChance = std::clamp(successChance, 30, 88);

            const bool success = random.between(1, 100) <= successChance;
            int damage = std::max(2, player.getLevel() / 6 + random.between(2, trained ? 8 : 6) + masteryBonus);
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " cherche la vraie faille de " + targetName + ".");

            if (success)
            {
                if (fragileBody)
                {
                    damage += std::max(2, player.getLevel() / 9 + physicalModifier / 8);
                    target.applyWeakening(1 + masteryDuration, (trained ? 8 : 5) + masteryBonus);
                    lines.push_back("L'angle touche une zone qui encaisse mal le choc.");
                }
                if (hardBody)
                {
                    damage = std::max(1, damage / (veryHardBody ? 2 : 1));
                    target.applyNextHitVulnerability(1, trained ? 18 : 12);
                    target.applyVulnerability(1, trained ? 7 : 4);
                    lines.push_back("La matière ne cède pas vraiment, mais une jointure reste ouverte.");
                    if (target.isInDefensePosture())
                    {
                        const std::string postureLabel = target.getDefensePostureLabel();
                        target.clearDefensePosture();
                        lines.push_back("La posture " + postureLabel + " perd son point d'appui.");
                    }
                }
                if (slimeBody)
                {
                    target.revealStats();
                    target.applyVulnerability(2, trained ? 11 : 8);
                    lines.push_back("Le noyau se laisse lire une seconde dans la masse.");
                }
                if (spectralBody)
                {
                    target.applyVulnerability(2, trained ? 9 : 6);
                    target.applyWeakening(1, trained ? 7 : 4);
                    lines.push_back("La forme se fixe juste assez pour être punie.");
                }
                if (!fragileBody && !hardBody && !slimeBody && !spectralBody)
                {
                    target.applyNextHitVulnerability(1 + masteryDuration, (trained ? 12 : 8) + masteryBonus);
                    lines.push_back("La lecture ne casse rien, mais elle prépare un meilleur prochain coup.");
                }
                target.takeDamage(damage);
                lines.push_back("Impact précis : " + std::to_string(damage) + " dégâts.");
                lines.push_back(trained
                    ? "Lecture des corps : ton regard trouve plus vite les jointures, noyaux et appuis faibles."
                    : "Lecture brute : l'angle existe, mais il demande encore de l'habitude.");
            }
            else
            {
                damage = std::max(1, damage / 3);
                target.takeDamage(damage);
                target.applyWeakening(1, 3);
                lines.push_back("Le geste accroche mal la faille cherchée.");
                lines.push_back("Éraflure : " + std::to_string(damage) + " dégâts, mais la cible perd un peu de rythme.");
                if (profile.incomingAccuracyModifier <= -8 && random.between(1, 100) <= 35)
                {
                    player.applyWeakening(1, 4);
                    lines.push_back(targetName + " se dérobe assez bas pour te voler un appui.");
                }
            }

            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "lecture_corps", "Faille de corps visée", 1);
            updateTacticalLearning(player, "lecture_corps");

            lines.push_back(tacticalMasteryShortResultLine(player, "lecture_corps"));

            MessageScreen::show(
                "FAILLE DE CORPS",
                "wave.combat.tactical_actions.body_weakpoint.result",
                lines,
                false
            );
            return true;
        }


        if (choice == 14)
        {
            const int targetIndex = chooseStatusChainTarget(wave, "wave.combat.tactical_actions.status_chain");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool trained = player.hasPassiveSkill("status_conductor")
                || player.hasPassiveSkill("tactical_toolbox")
                || player.hasPassiveSkill("field_weapon_crafter")
                || playerClassContainsAny(player, {"alchimiste", "mage", "sorcier", "sorcière", "sorciere", "artisan", "voleur", "assassin", "rôdeur", "rodeur"});

            const int statusCount = countCombatStatuses(target);
            const int masteryBonus = tacticalMasterySoftBonus(player, "chaine_etats");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "chaine_etats");
            int damage = std::max(3, player.getLevel() / 5 + statusCount * 2 + random.between(1, trained ? 8 : 5) + masteryBonus);
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " force les états déjà présents sur " + targetName + " à se répondre.");

            if (target.hasPoison() && target.hasBleeding())
            {
                damage += std::max(2, player.getLevel() / 10 + 3);
                target.applyVulnerability(2 + masteryDuration, (trained ? 12 : 8) + masteryBonus);
                lines.push_back("Le venin trouve les plaies ouvertes et la défense cède plus vite.");
            }
            if (target.hasBurning() && target.hasFrost())
            {
                damage += std::max(2, player.getLevel() / 12 + 2);
                target.applyWeakening(2 + masteryDuration, (trained ? 13 : 9) + masteryBonus);
                lines.push_back("Le chaud et le froid se disputent la chair et cassent le rythme.");
            }
            if (target.hasShock() && (target.hasVulnerability() || target.hasNextHitVulnerability()))
            {
                damage += std::max(2, player.getLevel() / 9 + 2);
                target.applyNextHitVulnerability(1, trained ? 16 : 11);
                lines.push_back("Le choc cherche la faille déjà ouverte et la garde tremble encore.");
            }
            if (target.hasWeakening() && (target.hasVulnerability() || target.hasNextHitVulnerability()))
            {
                target.applyBleeding(1, std::max(1, player.getLevel() / 30 + 1));
                lines.push_back("La cible manque assez d'appui pour rouvrir une blessure courte.");
            }
            if (statusCount >= 3)
            {
                target.applyWeakening(1, trained ? 10 : 6);
                target.applyVulnerability(1, trained ? 9 : 5);
                lines.push_back("Trop d'états se superposent : la ligne de défense devient sale.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "chaine_etats", "Chaîne d'états forcée", 1);
            updateTacticalLearning(player, "chaine_etats");

            lines.push_back("Réaction forcée : " + std::to_string(damage) + " dégâts.");
            lines.push_back(tacticalMasteryShortResultLine(player, "chaine_etats"));
            lines.push_back(trained
                ? "Le geste ne rajoute pas seulement une blessure : il fait parler celles qui existaient déjà."
                : "La réaction prend, mais demande encore une meilleure lecture du bon moment.");

            MessageScreen::show(
                "CHAÎNE D'ÉTATS",
                "wave.combat.tactical_actions.status_chain.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 15)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.mark_prey");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const bool trained = player.hasPassiveSkill("prey_marker")
                || player.hasPassiveSkill("body_reader")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"rôdeur", "rodeur", "chasseur", "archer", "assassin", "voleur", "éclaireur", "eclaireur", "mercenaire"});
            const bool elusive = profile.incomingAccuracyModifier <= -8;
            const bool hard = profile.physicalDamageModifierPercent <= -8;
            const bool fragile = profile.physicalDamageModifierPercent >= 10;
            const int masteryBonus = tacticalMasterySoftBonus(player, "marque_proie");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "marque_proie");

            int damage = std::max(1, player.getLevel() / 8 + random.between(1, trained ? 5 : 3) + std::max(0, masteryBonus / 2));
            target.revealStats();
            target.applyNextHitVulnerability(1 + masteryDuration, (trained ? 18 : 12) + masteryBonus);
            target.applyVulnerability(1, (trained ? 8 : 5) + masteryBonus);
            if (elusive)
            {
                target.applyWeakening(1 + masteryDuration, (trained ? 8 : 5) + masteryBonus);
            }
            if (hard)
            {
                target.applyNextHitVulnerability(1 + masteryDuration, (trained ? 22 : 14) + masteryBonus);
            }
            if (fragile)
            {
                damage += std::max(1, player.getLevel() / 12 + 1);
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "marque_proie", "Proie marquée", 1);
            updateTacticalLearning(player, "marque_proie");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " marque " + targetName + " pour le prochain vrai impact.");
            lines.push_back("Entaille de repère : " + std::to_string(damage) + " dégâts.");
            if (elusive)
            {
                lines.push_back("La marque suit une cible mobile au lieu de poursuivre son corps au hasard.");
            }
            if (hard)
            {
                lines.push_back("La marque ne traverse pas la matière, elle montre où reprendre l'angle.");
            }
            if (fragile)
            {
                lines.push_back("Le petit corps supporte mal même cette entaille courte.");
            }
            lines.push_back(trained
                ? "Le regard reste dessus : la prochaine frappe trouve mieux son chemin."
                : "La marque est visible, mais il faudra encore mieux choisir le moment.");
            lines.push_back(tacticalMasteryShortResultLine(player, "marque_proie"));

            MessageScreen::show(
                "PROIE MARQUÉE",
                "wave.combat.tactical_actions.mark_prey.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 16)
        {
            const bool trained = player.hasPassiveSkill("controlled_retreat")
                || player.hasPassiveSkill("support_rhythm")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"tank", "chevalier", "gardien", "paladin", "mercenaire", "moine", "support", "soigneur"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "retrait_controle");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "retrait_controle");
            player.applyGuardBoost((trained ? 2 : 1) + masteryDuration, (trained ? 18 : 12) + masteryBonus);
            player.applyPrecisionBoost(1 + (masteryDuration >= 2 ? 1 : 0), (trained ? 7 : 4) + std::max(0, masteryBonus / 2));
            if (player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 40)
            {
                player.applyRegeneration(1 + std::min(1, masteryDuration), std::max(1, player.getLevel() / 18 + 2 + masteryBonus / 3));
            }

            const bool brokeEntanglement = player.hasEntanglement() && (trained || masteryBonus >= 3) && player.cureEntanglement();

            int affected = 0;
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " recule juste assez pour reprendre l'air sans quitter la ligne.");
            if (brokeEntanglement)
            {
                lines.push_back("Contre-entrave : le retrait casse les fils, racines ou prises qui allaient voler le tour.");
            }

            for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(enemy);
                const bool predator = normalizeTacticalText(profile.archetype).find("prédateur") != std::string::npos
                    || normalizeTacticalText(profile.archetype).find("meute") != std::string::npos
                    || normalizeTacticalText(profile.archetype).find("sournois") != std::string::npos;
                if (predator || enemy.hasPrecisionBoost() || enemy.hasPowerBoost())
                {
                    enemy.applyWeakening(1 + masteryDuration, (trained ? 8 : 5) + masteryBonus);
                    ++affected;
                }
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "retrait_controle", "Sens du retrait", 1);
            updateTacticalLearning(player, "retrait_controle");

            lines.push_back(trained
                ? "Le recul est propre : garde, respiration et prochain geste se replacent ensemble."
                : "Le recul suffit à fermer un angle, mais demande encore du calme.");
            if (affected > 0)
            {
                lines.push_back("Poursuivants gênés : " + std::to_string(affected) + ".");
            }
            if (player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 40)
            {
                lines.push_back("Le souffle repris stabilise un peu les blessures immédiates.");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "retrait_controle"));

            MessageScreen::show(
                "RETRAIT CONTRÔLÉ",
                "wave.combat.tactical_actions.controlled_retreat.result",
                lines,
                false
            );
            return true;
        }

        if (choice == 17)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.threat_read");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const std::string archetype = normalizeTacticalText(profile.archetype);
            const bool trained = player.hasPassiveSkill("threat_reader")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("prey_marker")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"voleur", "assassin", "rôdeur", "rodeur", "mercenaire", "stratège", "stratege", "chasseur"});

            target.revealStats();
            const int masteryBonus = tacticalMasterySoftBonus(player, "lecture_menace");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "lecture_menace");
            int pressure = (trained ? 10 : 6) + masteryBonus;
            int damage = std::max(1, player.getLevel() / 12 + random.between(1, trained ? 4 : 2) + std::max(0, masteryBonus / 2));
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " lit le rôle de " + targetName + " au lieu de suivre seulement son arme.");
            lines.push_back("Profil lu : " + profile.archetype + ".");

            if (tacticalMonsterIsDedicatedCaller(target) || target.canUseWeakenedReinforcementSignal())
            {
                target.resetReinforcementLowHpTurns();
                target.setReinforcementEntryCooldown(trained ? 3 : 2);
                lines.push_back("Le souffle du signal est forcé à se perdre dans la mêlée.");
                pressure += 4;
            }
            if (archetype.find("soigneur") != std::string::npos || archetype.find("lanceur") != std::string::npos)
            {
                target.applyWeakening(2 + masteryDuration, (trained ? 12 : 8) + masteryBonus);
                lines.push_back("La main utile tremble avant de retrouver son rythme.");
                pressure += 3;
            }
            if (archetype.find("porte-bouclier") != std::string::npos || archetype.find("gardien") != std::string::npos)
            {
                target.applyNextHitVulnerability(1 + masteryDuration, (trained ? 16 : 10) + masteryBonus);
                lines.push_back("La garde ne tombe pas, mais un bord reste lisible.");
                pressure += 2;
            }
            if (archetype.find("berserker") != std::string::npos || archetype.find("chargeur") != std::string::npos)
            {
                target.applyWeakening(1 + masteryDuration, (trained ? 11 : 7) + masteryBonus);
                lines.push_back("L'élan brutal est lu avant de partir complètement.");
                pressure += 2;
            }
            if (archetype.find("porte-malédiction") != std::string::npos || archetype.find("énergie") != std::string::npos || archetype.find("cristal") != std::string::npos)
            {
                target.applyVulnerability(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);
                lines.push_back("La source instable montre une ligne où reprendre l'impact.");
                pressure += 2;
            }

            target.applyVulnerability(1 + masteryDuration, pressure);
            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.applyPrecisionBoost(1 + std::min(1, masteryDuration), (trained ? 5 : 3) + std::max(0, masteryBonus / 3));
            player.recordCanonicalEvent("actions_tactiques_combat", "lecture_menace", "Menace lue", 1);
            updateTacticalLearning(player, "lecture_menace");

            lines.push_back("Entaille de lecture : " + std::to_string(damage) + " dégâts.");
            lines.push_back(trained
                ? "La menace est cadrée : le prochain choix devrait arriver plus net."
                : "La menace est comprise, même si le geste reste encore prudent.");
            MessageScreen::show("MENACE LUE", "wave.combat.tactical_actions.threat_read.result", lines, false);
            return true;
        }

        if (choice == 18)
        {
            const bool trained = player.hasPassiveSkill("low_sweeper")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("combat_improviser")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"moine", "guerrier", "lancier", "chevalier", "mercenaire", "paysan"});

            int affected = 0;
            int totalDamage = 0;
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " balaie bas pour couper pattes, ailes basses et appuis nerveux.");
            for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                if (!isLowOrMobileTarget(enemy))
                {
                    continue;
                }

                const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(enemy);
                const bool veryElusive = profile.incomingAccuracyModifier <= -12;
                const int damage = std::max(1, player.getLevel() / 14 + random.between(1, trained ? 4 : 2));
                enemy.takeDamage(damage);
                enemy.applyWeakening(1, trained ? 8 : 5);
                if (veryElusive)
                {
                    enemy.applyVulnerability(1, trained ? 7 : 4);
                }
                ++affected;
                totalDamage += damage;
            }

            wave.removeDeadAndReplace();
            if (affected <= 0)
            {
                MessageScreen::show("AUCUN APPUI BAS", "wave.combat.tactical_actions.low_sweep.empty", {"Aucune cible assez basse ou nerveuse ne donne un bon angle de balayage."}, false);
                return false;
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "balayage_bas", "Lecture du balayage bas", 1);
            updateTacticalLearning(player, "balayage_bas");
            lines.push_back("Cibles gênées : " + std::to_string(affected) + ".");
            lines.push_back("Dégâts dispersés : " + std::to_string(totalDamage) + ".");
            lines.push_back(trained
                ? "Le balayage ne poursuit pas les petites cibles : il coupe l'endroit où elles veulent passer."
                : "Le geste gêne la ligne basse, mais peut encore gagner en précision.");
            MessageScreen::show("BALAYAGE BAS", "wave.combat.tactical_actions.low_sweep.result", lines, false);
            return true;
        }

        if (choice == 19)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.focus_break");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const std::string archetype = normalizeTacticalText(profile.archetype);
            const bool naturalTarget = isFocusOrSupportTarget(target);
            const bool trained = player.hasPassiveSkill("focus_breaker")
                || player.hasPassiveSkill("status_conductor")
                || player.hasPassiveSkill("terrain_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"mage", "sorcier", "prêtre", "pretre", "paladin", "anti-mage", "voleur", "assassin"});

            int damage = std::max(1, player.getLevel() / 11 + random.between(1, trained ? 5 : 3));
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " coupe la respiration ou le geste de " + targetName + ".");

            if (naturalTarget)
            {
                target.applyWeakening(2, trained ? 14 : 9);
                target.applyNextHitVulnerability(1, trained ? 14 : 9);
                lines.push_back("La concentration se froisse avant de redevenir dangereuse.");
                if (archetype.find("champignon") != std::string::npos)
                {
                    target.applyBurning(1, std::max(1, player.getLevel() / 28 + 1));
                    lines.push_back("Les spores se rétractent sous une chaleur courte.");
                }
                if (archetype.find("cristal") != std::string::npos)
                {
                    target.applyVulnerability(1, trained ? 11 : 7);
                    lines.push_back("La résonance perd une note et laisse une fissure claire.");
                }
            }
            else
            {
                target.applyWeakening(1, trained ? 8 : 5);
                damage = std::max(1, damage / 2);
                lines.push_back("La cible n'a pas de vrai rituel à couper, mais le geste perturbe quand même son rythme.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "rupture_concentration", "Concentration brisée", 1);
            updateTacticalLearning(player, "rupture_concentration");

            lines.push_back("Pression directe : " + std::to_string(damage) + " dégâts.");
            lines.push_back(trained
                ? "La rupture tombe au bon moment : la cible devra reconstruire son geste."
                : "La rupture fonctionne, mais demande encore une meilleure lecture du souffle.");
            MessageScreen::show("CONCENTRATION BRISÉE", "wave.combat.tactical_actions.focus_break.result", lines, false);
            return true;
        }

        if (choice == 20)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.reach_break");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const std::string archetype = normalizeTacticalText(profile.archetype);
            const bool naturalTarget = isReachOrBacklineTarget(target);
            const bool trained = player.hasPassiveSkill("reach_breaker")
                || player.hasPassiveSkill("low_sweeper")
                || player.hasPassiveSkill("controlled_retreat")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"lancier", "rodeur", "rôdeur", "archer", "duelliste", "mercenaire", "moine"});

            int damage = std::max(1, player.getLevel() / 10 + random.between(1, trained ? 6 : 3));
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " ferme la distance de " + targetName + " avant que l'allonge soit vraiment utile.");
            if (naturalTarget)
            {
                target.applyWeakening(2, trained ? 13 : 8);
                target.applyVulnerability(1, trained ? 9 : 5);
                player.applyPrecisionBoost(1, trained ? 4 : 2);
                lines.push_back("Le bras long, le tir ou la pointe perd son espace propre.");
                if (archetype.find("harceleur") != std::string::npos || archetype.find("tireur") != std::string::npos)
                {
                    target.applyNextHitVulnerability(1, trained ? 14 : 9);
                    lines.push_back("La trajectoire de recul devient lisible pour la prochaine reprise.");
                }
            }
            else
            {
                damage = std::max(1, damage / 2);
                target.applyWeakening(1, trained ? 7 : 4);
                lines.push_back("La cible n'a pas une vraie allonge à briser, mais son espace se ferme quand même.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "rupture_allonge", "Allonge rompue", 1);
            updateTacticalLearning(player, "rupture_allonge");
            lines.push_back("Impact de fermeture : " + std::to_string(damage) + " dégâts.");
            lines.push_back(trained ? "La distance est cassée proprement : l'ennemi devra reconstruire son angle." : "La fermeture gêne, mais reste encore brute.");
            MessageScreen::show("ALLONGE ROMPUE", "wave.combat.tactical_actions.reach_break.result", lines, false);
            return true;
        }

        if (choice == 21)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.shield_pierce");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const bool naturalTarget = isShieldOrArmorTarget(target);
            const bool wasDefending = target.isInDefensePosture();
            const std::string postureLabel = target.getDefensePostureLabel();
            const bool trained = player.hasPassiveSkill("shield_piercer")
                || player.hasPassiveSkill("guard_breaker")
                || player.hasPassiveSkill("body_reader")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"marteau", "hache", "chevalier", "guerrier", "paladin", "forgeron", "artisan"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "perce_rempart");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "perce_rempart");
            int damage = std::max(1, player.getLevel() / 9 + random.between(1, trained ? 5 : 3) + std::max(0, masteryBonus / 2));
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " cherche le bord du rempart de " + targetName + " plutôt que son centre.");
            if (naturalTarget)
            {
                target.applyNextHitVulnerability(2 + masteryDuration, (trained ? 22 : 14) + masteryBonus);
                target.applyVulnerability(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);
                if (wasDefending)
                {
                    target.clearDefensePosture();
                    damage += trained ? 3 : 1;
                    lines.push_back("La posture " + postureLabel + " se défait sur un angle trop serré.");
                }
                if (profile.physicalDamageModifierPercent <= -12)
                {
                    lines.push_back("La matière ne cède pas, mais une jointure reste ouverte.");
                }
                if (trained && random.between(1, 100) <= 30)
                {
                    target.applyWeakening(1, 8);
                }
            }
            else
            {
                damage = std::max(1, damage / 2);
                target.applyNextHitVulnerability(1, trained ? 9 : 5);
                lines.push_back("Aucun vrai rempart ne bloque la cible, le geste sert surtout à préparer la suite.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "perce_rempart", "Rempart percé", 1);
            updateTacticalLearning(player, "perce_rempart");
            lines.push_back("Choc ciblé : " + std::to_string(damage) + " dégâts.");
            lines.push_back(trained ? "Le prochain angle est beaucoup plus clair." : "L'ouverture existe, mais il faut encore apprendre à la reprendre vite.");
            lines.push_back(tacticalMasteryShortResultLine(player, "perce_rempart"));
            MessageScreen::show("REMPART PERCÉ", "wave.combat.tactical_actions.shield_pierce.result", lines, false);
            return true;
        }

        if (choice == 22)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.anchor_break");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const std::string archetype = normalizeTacticalText(profile.archetype);
            const bool naturalTarget = isOccultAnchorTarget(target);
            const bool trained = player.hasPassiveSkill("anchor_breaker")
                || player.hasPassiveSkill("focus_breaker")
                || player.hasPassiveSkill("status_conductor")
                || player.hasPassiveSkill("tactical_toolbox")
                || playerClassContainsAny(player, {"mage", "prêtre", "pretre", "paladin", "occultiste", "exorciste", "chasseur"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "rupture_ancrage");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "rupture_ancrage");
            int damage = std::max(1, player.getLevel() / 11 + random.between(1, trained ? 6 : 3) + masteryBonus);
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " frappe le lien invisible autour de " + targetName + ".");
            if (naturalTarget)
            {
                target.applyVulnerability(2, (trained ? 14 : 9) + masteryBonus);
                target.applyWeakening(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);
                player.applyElementalWard(1 + masteryDuration, (trained ? 12 : 7) + masteryBonus);
                lines.push_back("L'aura se décale et la cible tient moins bien son rythme.");
                if (archetype.find("mort-vivant") != std::string::npos || archetype.find("spectral") != std::string::npos)
                {
                    target.applyBurning(1, std::max(1, player.getLevel() / 32 + 1));
                    lines.push_back("La forme froide accroche une chaleur courte.");
                }
                if (archetype.find("porte-malédiction") != std::string::npos)
                {
                    target.applyNextHitVulnerability(1 + masteryDuration, (trained ? 16 : 10) + masteryBonus);
                    lines.push_back("La marque noire revient vers sa source et ouvre le prochain impact.");
                }
                if (archetype.find("énergie") != std::string::npos || archetype.find("cristal") != std::string::npos)
                {
                    target.applyShock(1 + masteryDuration);
                    lines.push_back("La résonance saute une note et se mord elle-même.");
                }
            }
            else
            {
                damage = std::max(1, damage / 2);
                target.applyVulnerability(1, trained ? 7 : 4);
                lines.push_back("La cible n'a pas d'ancrage net, mais son souffle se décale un instant.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "rupture_ancrage", "Ancrage rompu", 1);
            updateTacticalLearning(player, "rupture_ancrage");
            lines.push_back("Rupture courte : " + std::to_string(damage) + " dégâts.");
            lines.push_back(trained ? "L'ancrage tremble vraiment : le prochain rythme est plus facile à salir." : "Le lien bouge, mais il faudra répéter ce genre de lecture.");
            lines.push_back(tacticalMasteryShortResultLine(player, "rupture_ancrage"));
            MessageScreen::show("ANCRAGE ROMPU", "wave.combat.tactical_actions.anchor_break.result", lines, false);
            return true;
        }

        if (choice == 23)
        {
            const bool trained = player.hasPassiveSkill("rally_breath")
                || player.hasPassiveSkill("support_rhythm")
                || player.hasPassiveSkill("controlled_retreat")
                || playerClassContainsAny(player, {"barde", "paladin", "chevalier", "commandant", "chef", "support", "gardien"});

            const bool lowHp = player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 45;
            const int masteryBonus = tacticalMasterySoftBonus(player, "souffle_ralliement");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "souffle_ralliement");
            player.applyGuardBoost((trained ? 2 : 1) + masteryDuration, (trained ? 16 : 10) + masteryBonus);
            player.applyPrecisionBoost(1 + (masteryDuration >= 2 ? 1 : 0), (trained ? 3 : 2) + std::max(0, masteryBonus / 3));
            if (trained || lowHp)
            {
                player.applyPowerBoost(1 + std::min(1, masteryDuration), (trained ? 12 : 7) + masteryBonus);
            }
            if (lowHp)
            {
                player.applyRegeneration(2 + std::min(1, masteryDuration), std::max(1, player.getLevel() / 28 + 2 + masteryBonus / 3));
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "souffle_ralliement", "Souffle rallié", 1);
            updateTacticalLearning(player, "souffle_ralliement");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " inspire, baisse l'épaule et reprend le centre de la ligne.");
            lines.push_back("La garde se resserre et le prochain geste devient plus sûr.");
            if (trained)
            {
                lines.push_back("Le souffle ne sert pas qu'à tenir : il donne aussi l'élan pour répondre.");
            }
            if (lowHp)
            {
                lines.push_back("La survie force le corps à gratter un peu de souffle en plus.");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "souffle_ralliement"));
            MessageScreen::show("SOUFFLE DE RALLIEMENT", "wave.combat.tactical_actions.rally_breath.result", lines, false);
            return true;
        }

        if (choice == 24)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.momentum_strike");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const bool trained = player.hasPassiveSkill("momentum_striker")
                || player.hasPassiveSkill("prey_marker")
                || player.hasPassiveSkill("battle_instinct")
                || playerClassContainsAny(player, {"guerrier", "assassin", "duelliste", "moine", "barbare", "lancier", "bretteur"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "frappe_elan");
            int damage = std::max(3, player.getLevel() / 5 + random.between(4, trained ? 11 : 8) + masteryBonus);
            if (target.hasNextHitVulnerability() || target.hasVulnerability())
            {
                damage += trained ? 5 : 3;
            }
            if (profile.incomingAccuracyModifier <= -7)
            {
                damage = std::max(2, damage - 2);
            }

            target.takeDamage(damage);
            if (!target.isDead())
            {
                if (trained && random.between(1, 100) <= 35 + tacticalMasteryChanceBonus(player, "bombe_atelier"))
                {
                    target.applyBleeding(1, std::max(1, player.getLevel() / 34 + 1));
                }
                else if (random.between(1, 100) <= 35)
                {
                    target.applyWeakening(1, 4 + player.getLevel() / 36);
                }
            }
            player.applyPowerBoost(1, trained ? 12 : 7);
            player.applyPrecisionBoost(1, trained ? 3 : 1);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "frappe_elan", "Élan canalisé", 1);
            updateTacticalLearning(player, "frappe_elan");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " traverse la distance et frappe " + targetName + " sans casser son propre rythme.");
            lines.push_back("Dégâts : " + std::to_string(damage) + ".");
            lines.push_back(trained ? "L'élan reste dans le corps : la reprise offensive arrive plus naturellement." : "Le coup porte, et l'élan laisse une réponse possible derrière.");
            lines.push_back(tacticalMasteryShortResultLine(player, "frappe_elan"));
            MessageScreen::show("FRAPPE D'ÉLAN", "wave.combat.tactical_actions.momentum_strike.result", lines, false);
            return true;
        }

        if (choice == 25)
        {
            const bool trained = player.hasPassiveSkill("emergency_warder")
                || player.hasPassiveSkill("learned_elemental_ward")
                || player.hasPassiveSkill("minor_fire_resistance")
                || player.hasPassiveSkill("minor_cold_resistance")
                || playerClassContainsAny(player, {"mage", "prêtre", "pretre", "paladin", "druide", "chaman", "soigneur", "protecteur"});
            const bool veryLowHp = player.getMaxHp() > 0 && player.getHp() * 100 <= player.getMaxHp() * 35;

            const int masteryBonus = tacticalMasterySoftBonus(player, "voile_urgence");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "voile_urgence");
            player.applyGuardBoost((trained ? 2 : 1) + masteryDuration, (trained ? 18 : 11) + masteryBonus);
            player.applyElementalWard((trained ? 2 : 1) + masteryDuration, (trained ? 20 : 12) + masteryBonus);
            player.applyRegeneration((veryLowHp ? 3 : 2) + masteryDuration, std::max(1, player.getLevel() / 30 + (trained ? 3 : 2) + masteryBonus / 2));
            if (trained)
            {
                player.applyPrecisionBoost(1, 1);
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "voile_urgence", "Réflexe de voile", 1);
            updateTacticalLearning(player, "voile_urgence");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " ramène un voile court contre la peau et les coutures de l'équipement.");
            lines.push_back("La garde tient mieux, les éléments mordent moins fort et le souffle revient par à-coups.");
            if (veryLowHp)
            {
                lines.push_back("Le corps prend ce qu'il peut : la régénération dure un peu plus longtemps.");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "voile_urgence"));
            MessageScreen::show("VOILE D'URGENCE", "wave.combat.tactical_actions.emergency_ward.result", lines, false);
            return true;
        }

        if (choice == 26)
        {
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.wild_spark");
            if (targetIndex < 0)
            {
                return false;
            }

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool trained = player.hasPassiveSkill("wild_spark")
                || player.hasPassiveSkill("careful_dosage")
                || player.hasPassiveSkill("learned_arcane_mark")
                || playerClassContainsAny(player, {"mage", "alchimiste", "artificier", "sorcier", "occultiste", "kitsune"});
            const int masteryBonus = tacticalMasterySoftBonus(player, "etincelle_instable");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "etincelle_instable");
            const int roll = random.between(1, trained || masteryBonus >= 3 ? 5 : 4);
            int damage = std::max(2, player.getLevel() / 7 + random.between(2, trained ? 8 : 6) + masteryBonus);
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " libère une étincelle mal domptée vers " + targetName + ".");

            if (roll == 1)
            {
                target.applyBurning(2 + masteryDuration, std::max(1, player.getLevel() / 30 + 2 + masteryBonus / 2));
                lines.push_back("La chaleur accroche la cible et refuse de s'éteindre tout de suite.");
            }
            else if (roll == 2)
            {
                target.applyFrost(1 + masteryDuration);
                target.applyWeakening(1, 5 + player.getLevel() / 35 + masteryBonus);
                lines.push_back("Le froid casse le tempo avant de se dissiper.");
            }
            else if (roll == 3)
            {
                target.applyShock(1 + masteryDuration);
                lines.push_back("Le choc saute dans les nerfs ou la matière conductrice.");
            }
            else if (roll == 4)
            {
                target.applyPoison(2 + masteryDuration, std::max(1, player.getLevel() / 34 + 1 + masteryBonus / 2));
                lines.push_back("Un résidu sale reste dans la plaie comme une fiole mal fermée.");
            }
            else
            {
                damage += std::max(2, player.getLevel() / 12);
                target.applyVulnerability(1, 6 + player.getLevel() / 34);
                lines.push_back("L'étincelle se concentre au dernier moment et ouvre une ligne plus nette.");
            }

            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "etincelle_instable", "Instabilité apprivoisée", 1);
            updateTacticalLearning(player, "etincelle_instable");
            player.recordChallengeCombatAction("skill");
            lines.push_back("Impact instable : " + std::to_string(damage) + " dégâts.");
            lines.push_back(tacticalMasteryShortResultLine(player, "etincelle_instable"));
            MessageScreen::show("ÉTINCELLE INSTABLE", "wave.combat.tactical_actions.wild_spark.result", lines, false);
            return true;
        }

        if (choice == 27)
        {
            const bool trained = player.hasPassiveSkill("war_cry_caller")
                || player.hasPassiveSkill("rally_breath")
                || player.hasPassiveSkill("veteran_rhythm")
                || playerClassContainsAny(player, {"guerrier", "barbare", "paladin", "chef", "soldat", "mercenaire", "barde"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "cri_guerre");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "cri_guerre");
            player.applyPowerBoost(2 + masteryDuration, (trained ? 14 : 9) + masteryBonus);
            player.applyPrecisionBoost(2, (trained ? 3 : 2) + std::min(1, masteryBonus / 3));
            player.applyGuardBoost(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);

            int shaken = 0;
            for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                if (!enemy.isDead() && random.between(1, 100) <= (trained ? 28 : 18) + tacticalMasteryChanceBonus(player, "cri_guerre"))
                {
                    enemy.applyWeakening(1, 3 + player.getLevel() / 40 + masteryBonus);
                    ++shaken;
                }
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "cri_guerre", "Voix de guerre", 1);
            updateTacticalLearning(player, "cri_guerre");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " pousse un cri court, pas pour faire joli, mais pour imposer le rythme.");
            lines.push_back("La prochaine mêlée gagne en puissance, précision et tenue.");
            if (shaken > 0)
            {
                lines.push_back(std::to_string(shaken) + " adversaire(s) perdent brièvement leur assurance.");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "cri_guerre"));
            MessageScreen::show("CRI DE GUERRE", "wave.combat.tactical_actions.war_cry.result", lines, false);
            return true;
        }

        if (choice == 28)
        {
            if (!playerHasShadowStepAffinity(player))
            {
                MessageScreen::show(
                    "APPUI INADAPTÉ",
                    "wave.combat.tactical_actions.shadow_step.locked",
                    {
                        "Ce pas demande un corps ou une formation capable de sortir de l'axe sans bruit.",
                        "Un profil agile, sournois ou naturellement vif pourra le travailler plus tard."
                    },
                    false
                );
                return false;
            }

            const bool trained = player.hasPassiveSkill("shadow_step_mastery") || player.hasPassiveSkill("shadow_stepper")
                || player.hasPassiveSkill("controlled_retreat")
                || player.hasPassiveSkill("semi_cat_reflexes")
                || player.hasPassiveSkill("semi_fox_cunning")
                || playerClassContainsAny(player, {"voleur", "assassin", "roublard", "rôdeur", "rodeur", "duelliste", "ninja", "éclaireur", "eclaireur"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "pas_ombre");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "pas_ombre");
            player.applyGuardBoost(1 + masteryDuration, (trained ? 15 : 9) + masteryBonus);
            player.applyPrecisionBoost(2, (trained ? 4 : 2) + std::min(2, masteryBonus / 3));
            if (trained || random.between(1, 100) <= 35 + tacticalMasteryChanceBonus(player, "pas_ombre"))
            {
                player.applyPowerBoost(1, (trained ? 8 : 4) + masteryBonus);
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "pas_ombre", "Pas de l'ombre", 1);
            updateTacticalLearning(player, "pas_ombre");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "PAS DE L'OMBRE",
                "wave.combat.tactical_actions.shadow_step.result",
                {
                    player.getName() + " quitte l'axe évident et revient avec une respiration plus basse.",
                    "Le corps offre moins de prise, et le prochain angle devient plus net.",
                    trained ? "Le déplacement ressemble moins à une fuite qu'à une promesse de riposte." : "Le geste reste prudent, mais il casse déjà la lecture adverse.",
                    tacticalMasteryShortResultLine(player, "pas_ombre")
                },
                false
            );
            return true;
        }

        if (choice == 29)
        {
            const bool trained = player.hasPassiveSkill("arc_sweep_mastery") || player.hasPassiveSkill("arc_sweeper")
                || player.hasPassiveSkill("momentum_striker")
                || player.hasPassiveSkill("blade_discipline")
                || playerClassContainsAny(player, {"guerrier", "barbare", "lancier", "bretteur", "samouraï", "samourai", "moine"});
            const int masteryBonus = tacticalMasterySoftBonus(player, "coup_arc");
            const int masteryChance = tacticalMasteryChanceBonus(player, "coup_arc");
            const int hits = std::min(2 + (tacticalMasteryLevel(player, "coup_arc") >= 8 ? 1 : 0), wave.getActiveEnemyCount());
            int totalDamage = 0;
            std::vector<std::string> lines;
            lines.push_back(player.getName() + " dessine un arc large pour occuper plus qu'une seule cible.");

            for (int index = 0; index < hits; ++index)
            {
                Monster& target = wave.getActiveEnemy(index);
                const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
                int damage = std::max(2, player.getLevel() / 6 + random.between(3, trained ? 9 : 6) + masteryBonus);
                if (profile.incomingAccuracyModifier <= -10)
                {
                    damage = std::max(1, damage - 2);
                }
                if (target.hasNextHitVulnerability() || target.hasVulnerability())
                {
                    damage += 2;
                }
                target.takeDamage(damage);
                totalDamage += damage;
                if (!target.isDead() && random.between(1, 100) <= (trained ? 30 : 18) + masteryChance)
                {
                    target.applyBleeding(1, std::max(1, player.getLevel() / 36 + 1));
                }
                ThreatSystem::consumeForcedTargetIfNeeded(target);
                lines.push_back(target.getName() + " reçoit " + std::to_string(damage) + " dégâts dans le mouvement.");
            }

            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "coup_arc", "Coup en arc", 1);
            updateTacticalLearning(player, "coup_arc");
            player.recordChallengeCombatAction("skill");
            lines.push_back("Dégâts totaux : " + std::to_string(totalDamage) + ".");
            lines.push_back(tacticalMasteryShortResultLine(player, "coup_arc"));
            MessageScreen::show("COUP EN ARC", "wave.combat.tactical_actions.arc_sweep.result", lines, false);
            return true;
        }

        if (choice == 30)
        {
            const bool trained = player.hasPassiveSkill("vigor_sign_mastery")
                || player.hasPassiveSkill("emergency_warder")
                || player.hasPassiveSkill("learned_mana_suture")
                || playerClassContainsAny(player, {"prêtre", "pretre", "paladin", "druide", "soigneur", "chaman", "moine"});

            const int masteryBonus = tacticalMasterySoftBonus(player, "signe_vigueur");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "signe_vigueur");
            player.applyRegeneration((trained ? 3 : 2) + masteryDuration, std::max(2, player.getLevel() / 28 + (trained ? 3 : 2) + masteryBonus / 2));
            player.applyGuardBoost(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);
            player.applyElementalWard(1 + masteryDuration, (trained ? 12 : 7) + masteryBonus);

            std::vector<std::string> purged;
            if (player.hasWeakening() && player.cureWeakening())
            {
                purged.push_back("affaiblissement");
            }
            else if (player.hasPoison() && player.curePoison())
            {
                purged.push_back("poison");
            }
            else if (player.hasBleeding() && player.cureBleeding())
            {
                purged.push_back("saignement");
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "signe_vigueur", "Signe de vigueur", 1);
            updateTacticalLearning(player, "signe_vigueur");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " trace un signe court et force le corps à reprendre prise.");
            lines.push_back("Un souffle lent revient, avec une garde et une résistance plus propres.");
            if (!purged.empty())
            {
                lines.push_back("Le signe arrache aussi un mauvais état : " + purged.front() + ".");
            }
            lines.push_back(tacticalMasteryShortResultLine(player, "signe_vigueur"));
            MessageScreen::show("SIGNE DE VIGUEUR", "wave.combat.tactical_actions.vigor_sign.result", lines, false);
            return true;
        }

        if (choice == 31)
        {
            if (!playerHasRogueTechniqueAffinity(player))
            {
                MessageScreen::show("GESTE TROP ÉVIDENT", "wave.combat.tactical_actions.rogue_feint.locked", {"Cette feinte demande une main basse, des appuis discrets et l'habitude des angles sales."}, false);
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.rogue_feint");
            if (targetIndex < 0) return false;

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool trained = player.hasPassiveSkill("rogue_feint_mastery") || player.hasPassiveSkill("rogue_feinter") || player.hasPassiveSkill("shadow_step_mastery") || player.hasPassiveSkill("shadow_stepper") || player.hasPassiveSkill("semi_fox_cunning");
            const int masteryBonus = tacticalMasterySoftBonus(player, "feinte_sournoise");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "feinte_sournoise");
            int damage = std::max(2, player.getLevel() / 7 + random.between(2, trained ? 8 : 5) + masteryBonus);
            if (target.hasNextHitVulnerability() || target.hasVulnerability())
            {
                damage += trained ? 4 : 2;
            }

            target.takeDamage(damage);
            target.applyNextHitVulnerability(2 + masteryDuration, (trained ? 16 : 10) + masteryBonus);
            target.applyWeakening(1 + masteryDuration, (trained ? 8 : 5) + masteryBonus);
            target.revealStats();
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.applyPrecisionBoost(1, trained ? 3 : 2);
            player.recordCanonicalEvent("actions_tactiques_combat", "feinte_sournoise", "Feinte sournoise", 1);
            updateTacticalLearning(player, "feinte_sournoise");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "FEINTE SOURNOISE",
                "wave.combat.tactical_actions.rogue_feint.result",
                {
                    player.getName() + " laisse croire à un angle propre, puis glisse le geste ailleurs.",
                    targetName + " reçoit " + std::to_string(damage) + " dégâts et garde une ouverture sale.",
                    trained ? "La feinte revient naturellement dans la main." : "Le geste reste risqué, mais la cible a perdu une lecture.",
                    tacticalMasteryShortResultLine(player, "feinte_sournoise")
                },
                false
            );
            return true;
        }

        if (choice == 32)
        {
            if (!playerHasSupportTechniqueAffinity(player))
            {
                MessageScreen::show("GESTE MAL CANALISÉ", "wave.combat.tactical_actions.battle_suture.locked", {"Il faut un vrai instinct de soutien pour refermer une blessure sans lâcher la ligne."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("battle_suture_mastery") || player.hasPassiveSkill("vigor_sign_mastery") || player.hasActiveSkill("learned_mana_suture");
            const int masteryBonus = tacticalMasterySoftBonus(player, "suture_bataille");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "suture_bataille");
            const int healAmount = std::max(3, player.getLevel() / 8 + (trained ? 8 : 5) + masteryBonus);
            player.heal(healAmount);
            player.applyRegeneration((trained ? 3 : 2) + masteryDuration, std::max(1, player.getLevel() / 30 + (trained ? 3 : 2) + masteryBonus / 2));
            player.applyGuardBoost(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);

            std::vector<std::string> purged;
            if (player.hasBleeding() && player.cureBleeding()) purged.push_back("saignement");
            else if (player.hasPoison() && player.curePoison()) purged.push_back("poison");
            else if (player.hasBurning() && player.cureBurning()) purged.push_back("brûlure");

            player.recordCanonicalEvent("actions_tactiques_combat", "suture_bataille", "Suture de bataille", 1);
            updateTacticalLearning(player, "suture_bataille");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " serre la plaie, cale le souffle et refuse de céder la ligne.");
            lines.push_back("Soin immédiat : " + std::to_string(healAmount) + " PV.");
            if (!purged.empty()) lines.push_back("Le geste nettoie aussi : " + purged.front() + ".");
            lines.push_back(tacticalMasteryShortResultLine(player, "suture_bataille"));
            MessageScreen::show("SUTURE DE BATAILLE", "wave.combat.tactical_actions.battle_suture.result", lines, false);
            return true;
        }

        if (choice == 33)
        {
            if (!playerHasArcaneTechniqueAffinity(player))
            {
                MessageScreen::show("MANA INSTABLE", "wave.combat.tactical_actions.arcane_channel.locked", {"Sans canalisation, le geste partirait en poussière avant d'avoir une forme."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler") || player.hasPassiveSkill("wild_spark") || player.hasPassiveSkill("cautious_channeling");
            const int masteryBonus = tacticalMasterySoftBonus(player, "canalisation_arcanique");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "canalisation_arcanique");
            player.applyPowerBoost(2 + masteryDuration, (trained ? 16 : 10) + masteryBonus);
            player.applyPrecisionBoost(2, (trained ? 4 : 2) + std::min(2, masteryBonus / 3));
            player.applyElementalWard(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);

            player.recordCanonicalEvent("actions_tactiques_combat", "canalisation_arcanique", "Canalisation arcanique", 1);
            updateTacticalLearning(player, "canalisation_arcanique");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "CANALISATION ARCANIQUE",
                "wave.combat.tactical_actions.arcane_channel.result",
                {
                    player.getName() + " serre le flux au lieu de le jeter tout de suite.",
                    "Le prochain geste gagne en puissance et en précision, avec un voile élémentaire court.",
                    trained ? "La canalisation garde une forme nette." : "Le flux tremble encore, mais il ne se perd pas.",
                    tacticalMasteryShortResultLine(player, "canalisation_arcanique")
                },
                false
            );
            return true;
        }

        if (choice == 34)
        {
            if (!playerHasGuardianTechniqueAffinity(player))
            {
                MessageScreen::show("LIGNE TROP LÉGÈRE", "wave.combat.tactical_actions.rampart_oath.locked", {"Ce serment demande une présence capable de faire hésiter une ligne entière."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("rampart_oath_mastery") || player.hasPassiveSkill("living_rampart") || player.hasPassiveSkill("steady_guard");
            const int masteryBonus = tacticalMasterySoftBonus(player, "serment_rempart");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "serment_rempart");
            player.startProvocation((trained ? 3 : 2) + masteryDuration);
            player.applyGuardBoost(2 + masteryDuration, (trained ? 22 : 15) + masteryBonus);
            player.applyRegeneration(1 + masteryDuration, std::max(1, player.getLevel() / 32 + (trained ? 3 : 2) + masteryBonus / 2));

            player.recordCanonicalEvent("actions_tactiques_combat", "serment_rempart", "Serment du rempart", 1);
            updateTacticalLearning(player, "serment_rempart");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "SERMENT DU REMPART",
                "wave.combat.tactical_actions.rampart_oath.result",
                {
                    player.getName() + " plante sa présence devant la ligne.",
                    "Les regards reviennent vers lui, la garde se durcit et le souffle tient mieux.",
                    trained ? "Le rempart ne tremble presque pas." : "Le geste attire le danger, mais il tient.",
                    tacticalMasteryShortResultLine(player, "serment_rempart")
                },
                false
            );
            return true;
        }

        if (choice == 35)
        {
            if (!playerHasSkirmisherTechniqueAffinity(player))
            {
                MessageScreen::show("DISTANCE MAL LUE", "wave.combat.tactical_actions.stopping_shot.locked", {"Ce tir demande l'habitude des trajectoires, des fuites et des approches cassées."}, false);
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.stopping_shot");
            if (targetIndex < 0) return false;

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const bool trained = player.hasPassiveSkill("stopping_shot_mastery") || player.hasPassiveSkill("ranger_eye") || player.hasActiveSkill("tracking_mark");
            const int masteryBonus = tacticalMasterySoftBonus(player, "tir_arret");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "tir_arret");
            int damage = std::max(2, player.getLevel() / 6 + random.between(3, trained ? 9 : 6) + masteryBonus);
            if (profile.incomingAccuracyModifier <= -10)
            {
                damage = std::max(1, damage - 1);
                target.applyWeakening(1, trained ? 8 : 5);
            }
            else
            {
                target.applyNextHitVulnerability(1, trained ? 12 : 8);
            }

            target.takeDamage(damage);
            if (!target.isDead() && random.between(1, 100) <= (trained ? 35 : 22) + tacticalMasteryChanceBonus(player, "tir_arret"))
            {
                target.applyBleeding(1, std::max(1, player.getLevel() / 38 + 1));
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "tir_arret", "Tir d'arrêt", 1);
            updateTacticalLearning(player, "tir_arret");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "TIR D'ARRÊT",
                "wave.combat.tactical_actions.stopping_shot.result",
                {
                    player.getName() + " coupe l'approche avant qu'elle ne devienne dangereuse.",
                    targetName + " reçoit " + std::to_string(damage) + " dégâts et perd un peu de ligne.",
                    trained ? "La trajectoire était préparée avant même le mouvement adverse." : "Le tir force au moins une mauvaise reprise.",
                    tacticalMasteryShortResultLine(player, "tir_arret")
                },
                false
            );
            return true;
        }

        if (choice == 36)
        {
            if (!playerHasWarriorBurstAffinity(player))
            {
                MessageScreen::show("RAGE MAL TENUE", "wave.combat.tactical_actions.mastered_rage.locked", {"Ce geste demande l'habitude d'encaisser, de pousser et de retenir l'élan avant qu'il ne morde son porteur."}, false);
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.mastered_rage");
            if (targetIndex < 0) return false;

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool trained = player.hasPassiveSkill("rage_control_mastery") || player.hasPassiveSkill("momentum_striker") || player.hasPassiveSkill("war_cry_caller");
            const bool wounded = player.getHp() * 2 <= player.getMaxHp();
            int damage = std::max(4, player.getLevel() / 4 + random.between(5, trained ? 13 : 9));
            if (wounded)
            {
                damage += trained ? 5 : 3;
            }

            target.takeDamage(damage);
            if (!target.isDead())
            {
                if (wounded || random.between(1, 100) <= (trained ? 45 : 30))
                {
                    target.applyBleeding(1, std::max(1, player.getLevel() / 34 + 1));
                }
                else
                {
                    target.applyWeakening(1, trained ? 8 : 5);
                }
            }
            player.applyPowerBoost(1, trained ? 14 : 9);
            player.applyGuardBoost(1, wounded ? (trained ? 12 : 8) : (trained ? 8 : 5));
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "rage_maitrisee", "Rage maîtrisée", 1);
            updateTacticalLearning(player, "rage_maitrisee");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "RAGE MAÎTRISÉE",
                "wave.combat.tactical_actions.mastered_rage.result",
                {
                    player.getName() + " laisse monter la rage juste assez pour frapper sans perdre la ligne.",
                    targetName + " reçoit " + std::to_string(damage) + " dégâts.",
                    wounded ? "La blessure rend l'élan plus mordant, mais la garde tient." : "L'élan reste contenu dans les appuis."
                },
                false
            );
            return true;
        }

        if (choice == 37)
        {
            if (!playerHasCommanderTechniqueAffinity(player))
            {
                MessageScreen::show("ORDRE FLOU", "wave.combat.tactical_actions.battle_order.locked", {"Un ordre lancé sans présence se perd dans le bruit des armes."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("battle_order_mastery") || player.hasPassiveSkill("war_cry_caller") || player.hasPassiveSkill("rally_breath");
            player.applyPrecisionBoost(2, trained ? 4 : 2);
            player.applyPowerBoost(2, trained ? 10 : 6);
            player.applyGuardBoost(1, trained ? 10 : 6);

            int unsettled = 0;
            for (int index = 0; index < wave.getActiveEnemyCount() && unsettled < (trained ? 2 : 1); ++index)
            {
                Monster& enemy = wave.getActiveEnemy(index);
                if (!enemy.isDead())
                {
                    enemy.applyWeakening(1, trained ? 6 : 4);
                    unsettled++;
                }
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "ordre_bataille", "Ordre de bataille", 1);
            updateTacticalLearning(player, "ordre_bataille");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " donne un ordre court, assez sec pour remettre les appuis en place.");
            lines.push_back("Le rythme gagne en précision, en garde et en élan.");
            if (unsettled > 0) lines.push_back(std::to_string(unsettled) + " adversaire(s) perdent brièvement leur assurance.");
            MessageScreen::show("ORDRE DE BATAILLE", "wave.combat.tactical_actions.battle_order.result", lines, false);
            return true;
        }

        if (choice == 38)
        {
            if (!playerHasNatureTechniqueAffinity(player))
            {
                MessageScreen::show("SOUFFLE MUET", "wave.combat.tactical_actions.breath_totem.locked", {"La nature ne répond pas à une main qui ne sait pas écouter le sol."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("breath_totem_mastery") || player.hasPassiveSkill("semi_wolf_tracking") || player.hasPassiveSkill("elven_fine_perception");
            player.applyRegeneration(trained ? 3 : 2, std::max(1, player.getLevel() / 28 + (trained ? 3 : 2)));
            player.applyElementalWard(2, trained ? 14 : 9);
            player.applyPrecisionBoost(1, trained ? 3 : 1);

            int rooted = 0;
            if (hasEnemy)
            {
                for (int index = 0; index < wave.getActiveEnemyCount() && rooted < (trained ? 2 : 1); ++index)
                {
                    Monster& enemy = wave.getActiveEnemy(index);
                    const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(enemy);
                    if (!enemy.isDead() && profile.incomingAccuracyModifier <= -5)
                    {
                        enemy.applyWeakening(1, trained ? 7 : 4);
                        rooted++;
                    }
                }
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "totem_souffle", "Totem de souffle", 1);
            updateTacticalLearning(player, "totem_souffle");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " pose un repère bas, presque invisible, et cale son souffle dessus.");
            lines.push_back("Le corps récupère lentement et un voile vivant amortit les éléments.");
            if (rooted > 0) lines.push_back("Des racines de fortune gênent " + std::to_string(rooted) + " trajectoire(s) nerveuse(s).");
            MessageScreen::show("TOTEM DE SOUFFLE", "wave.combat.tactical_actions.breath_totem.result", lines, false);
            return true;
        }

        if (choice == 39)
        {
            if (!playerHasArtificeTechniqueAffinity(player))
            {
                MessageScreen::show("BRICOLAGE DANGEREUX", "wave.combat.tactical_actions.workshop_bomb.locked", {"Sans main d'atelier, la bombe partirait probablement du mauvais côté."}, false);
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.workshop_bomb");
            if (targetIndex < 0) return false;

            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const bool trained = player.hasPassiveSkill("workshop_bomb_mastery") || player.hasPassiveSkill("workshop_bomber") || player.hasPassiveSkill("careful_dosage") || player.hasPassiveSkill("field_weapon_crafter");
            const int masteryBonus = tacticalMasterySoftBonus(player, "bombe_atelier");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "bombe_atelier");
            int damage = std::max(3, player.getLevel() / 5 + random.between(4, trained ? 12 : 8) + masteryBonus);
            const int roll = random.between(1, 4);
            std::string effectLine;
            if (roll == 1)
            {
                target.applyBurning(2 + masteryDuration, std::max(1, player.getLevel() / 30 + 2 + masteryBonus / 2));
                effectLine = "Une mèche mord l'air et laisse une brûlure courte.";
            }
            else if (roll == 2)
            {
                target.applyPoison(2 + masteryDuration, std::max(1, player.getLevel() / 34 + 1 + masteryBonus / 2));
                effectLine = "Une poudre sale colle à la peau et continue de ronger.";
            }
            else if (roll == 3)
            {
                target.applyShock(1 + masteryDuration);
                target.applyWeakening(1 + masteryDuration, (trained ? 9 : 6) + masteryBonus);
                effectLine = "Le choc coupe le geste et fait trembler les appuis.";
            }
            else
            {
                target.applyNextHitVulnerability(1, (trained ? 15 : 10) + masteryBonus);
                effectLine = "L'explosion ouvre un angle bref pour le prochain impact.";
            }
            if (trained && random.between(1, 100) <= 35 + tacticalMasteryChanceBonus(player, "bombe_atelier"))
            {
                damage += 3;
            }
            target.takeDamage(damage);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "bombe_atelier", "Bombe d'atelier", 1);
            updateTacticalLearning(player, "bombe_atelier");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "BOMBE D'ATELIER",
                "wave.combat.tactical_actions.workshop_bomb.result",
                {
                    player.getName() + " lance un bricolage instable qui éclate au ras de la ligne.",
                    targetName + " reçoit " + std::to_string(damage) + " dégâts.",
                    effectLine
                },
                false
            );
            return true;
        }

        if (choice == 40)
        {
            if (!playerHasSacredTechniqueAffinity(player))
            {
                MessageScreen::show("PRIÈRE SANS ANCRE", "wave.combat.tactical_actions.steel_prayer.locked", {"Cette prière demande une foi, une discipline ou une protection assez nette pour tenir sous les coups."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("steel_prayer_mastery") || player.hasPassiveSkill("battle_suture_mastery") || player.hasPassiveSkill("rampart_oath_mastery");
            player.applyGuardBoost(2, trained ? 18 : 12);
            player.applyElementalWard(2, trained ? 16 : 10);
            player.applyRegeneration(1, std::max(1, player.getLevel() / 32 + (trained ? 3 : 2)));
            if (trained)
            {
                player.startProvocation(1);
            }

            std::vector<std::string> purged;
            if (player.hasWeakening() && player.cureWeakening()) purged.push_back("affaiblissement");
            else if (player.hasShock() && player.cureShock()) purged.push_back("choc");
            else if (player.hasFrost() && player.cureFrost()) purged.push_back("givre");
            else if (player.hasBurning() && player.cureBurning()) purged.push_back("brûlure");

            player.recordCanonicalEvent("actions_tactiques_combat", "priere_acier", "Prière d'acier", 1);
            updateTacticalLearning(player, "priere_acier");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " ferme la main comme sur une lame froide et murmure une prière courte.");
            lines.push_back("La garde devient plus lourde, le souffle revient et les éléments glissent moins bien.");
            if (!purged.empty()) lines.push_back("La prière arrache aussi : " + purged.front() + ".");
            MessageScreen::show("PRIÈRE D'ACIER", "wave.combat.tactical_actions.steel_prayer.result", lines, false);
            return true;
        }

        if (choice == 41)
        {
            if (!playerHasMonkTechniqueAffinity(player))
            {
                MessageScreen::show("SOUFFLE DISPERSÉ", "wave.combat.tactical_actions.inner_mantra.locked", {"Ce mantra demande une discipline de corps, d'appuis ou de duel assez stable."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("inner_mantra_mastery") || player.hasPassiveSkill("footwork_drill") || player.hasPassiveSkill("vigor_sign_mastery");
            player.applyPrecisionBoost(2, trained ? 4 : 2);
            player.applyGuardBoost(2, trained ? 13 : 8);
            player.applyPowerBoost(1, trained ? 8 : 4);
            player.applyRegeneration(1, std::max(1, player.getLevel() / 36 + (trained ? 3 : 1)));

            std::vector<std::string> purged;
            if (player.hasShock() && player.cureShock()) purged.push_back("choc");
            else if (player.hasFrost() && player.cureFrost()) purged.push_back("givre");
            else if (player.hasWeakening() && player.cureWeakening()) purged.push_back("affaiblissement");

            player.recordCanonicalEvent("actions_tactiques_combat", "mantra_interieur", "Mantra intérieur", 1);
            updateTacticalLearning(player, "mantra_interieur");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " cale sa respiration au fond du ventre et laisse le bruit du combat glisser autour.");
            lines.push_back("Le prochain geste devient plus propre : précision, garde et élan court.");
            if (!purged.empty()) lines.push_back("Le souffle chasse aussi : " + purged.front() + ".");
            MessageScreen::show("MANTRA INTÉRIEUR", "wave.combat.tactical_actions.inner_mantra.result", lines, false);
            return true;
        }

        if (choice == 42)
        {
            if (!playerHasIllusionTechniqueAffinity(player))
            {
                MessageScreen::show("MIROIR MUET", "wave.combat.tactical_actions.trick_image.locked", {"Sans ruse, magie ou art de détourner le regard, l'image ne tient pas."}, false);
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.trick_image");
            if (targetIndex < 0) return false;

            Monster& target = wave.getActiveEnemy(targetIndex);
            const bool trained = player.hasPassiveSkill("trick_image_mastery") || player.hasPassiveSkill("semi_fox_cunning") || player.hasPassiveSkill("shadow_step_mastery") || player.hasPassiveSkill("shadow_stepper") || player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler");
            player.applyGuardBoost(1, trained ? 12 : 7);
            player.applyPrecisionBoost(2, trained ? 4 : 2);
            target.applyWeakening(1, trained ? 10 : 6);
            target.applyNextHitVulnerability(1, trained ? 14 : 9);
            if (trained && random.between(1, 100) <= 35 + tacticalMasteryChanceBonus(player, "bombe_atelier"))
            {
                target.revealStats();
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "image_trompeuse", "Image trompeuse", 1);
            updateTacticalLearning(player, "image_trompeuse");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "IMAGE TROMPEUSE",
                "wave.combat.tactical_actions.trick_image.result",
                {
                    player.getName() + " laisse une fausse présence absorber le regard de " + target.getName() + ".",
                    "La cible frappe un angle qui n'existe déjà plus.",
                    trained ? "Le mirage tient assez longtemps pour révéler un détail utile." : "L'illusion reste courte, mais elle ouvre une vraie reprise."
                },
                false
            );
            return true;
        }

        if (choice == 43)
        {
            if (!playerHasSummonerTechniqueAffinity(player))
            {
                MessageScreen::show("LIEN SANS RÉPONSE", "wave.combat.tactical_actions.summoning_link.locked", {"Aucune présence ne répond à un lien jeté sans pacte, ruche, invocation ou rite."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("summoning_link_mastery") || player.hasPassiveSkill("breath_totem_mastery") || player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler");
            player.applyElementalWard(2, trained ? 14 : 9);
            player.applyGuardBoost(1, trained ? 12 : 7);
            player.applyRegeneration(1, std::max(1, player.getLevel() / 34 + (trained ? 2 : 1)));

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " serre un lien invisible autour de la ligne, comme si une présence restait juste derrière l'épaule.");
            lines.push_back("Le corps encaisse mieux et le souffle revient par petites prises.");
            if (hasEnemy)
            {
                Monster& target = wave.getActiveEnemy(random.between(0, wave.getActiveEnemyCount() - 1));
                const int damage = std::max(2, player.getLevel() / 8 + random.between(2, trained ? 8 : 5));
                target.takeDamage(damage);
                target.applyWeakening(1, trained ? 7 : 4);
                ThreatSystem::consumeForcedTargetIfNeeded(target);
                lines.push_back("La présence liée griffe " + target.getName() + " : " + std::to_string(damage) + " dégâts et un souffle troublé.");
                wave.removeDeadAndReplace();
            }

            player.recordCanonicalEvent("actions_tactiques_combat", "lien_invocation", "Lien d'invocation", 1);
            updateTacticalLearning(player, "lien_invocation");
            player.recordChallengeCombatAction("skill");
            MessageScreen::show("LIEN D'INVOCATION", "wave.combat.tactical_actions.summoning_link.result", lines, false);
            return true;
        }

        if (choice == 44)
        {
            if (!playerHasBloodTechniqueAffinity(player))
            {
                MessageScreen::show("PACTE REFUSÉ", "wave.combat.tactical_actions.blood_pact.locked", {"Ce genre de prix demande un corps habitué à la rage, au sang ou aux pactes sombres."}, false);
                return false;
            }

            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.blood_pact");
            if (targetIndex < 0) return false;

            Monster& target = wave.getActiveEnemy(targetIndex);
            const bool trained = player.hasPassiveSkill("blood_pact_mastery") || player.hasPassiveSkill("rage_control_mastery") || player.hasPassiveSkill("scar_tissue");
            const int masteryBonus = tacticalMasterySoftBonus(player, "pacte_sanguin");
            const int masteryDuration = tacticalMasteryDurationBonus(player, "pacte_sanguin");
            int selfCost = std::max(1, player.getMaxHp() / (trained ? 28 : 22));
            selfCost = std::min(selfCost, std::max(0, player.getHp() - 1));
            if (selfCost <= 0)
            {
                MessageScreen::show("SANG TROP COURT", "wave.combat.tactical_actions.blood_pact.too_low", {"Le corps refuse de payer davantage. Le pacte doit attendre un souffle de plus."}, false);
                return false;
            }
            player.takeDamage(selfCost);
            player.applyPowerBoost(2 + std::min(1, masteryDuration), (trained ? 18 : 12) + masteryBonus);
            player.applyRegeneration(2 + std::min(1, masteryDuration), std::max(1, player.getLevel() / 32 + (trained ? 3 : 2) + masteryBonus / 3));
            const int damage = std::max(4, player.getLevel() / 4 + random.between(4, trained ? 13 : 9) + masteryBonus);
            target.takeDamage(damage);
            target.applyBleeding(2, std::max(1, player.getLevel() / 34 + 1));
            if (trained || random.between(1, 100) <= 35)
            {
                target.applyVulnerability(1 + masteryDuration, (trained ? 10 : 6) + masteryBonus);
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();

            player.recordCanonicalEvent("actions_tactiques_combat", "pacte_sanguin", "Pacte sanguin", 1);
            updateTacticalLearning(player, "pacte_sanguin");
            player.recordChallengeCombatAction("skill");

            MessageScreen::show(
                "PACTE SANGUIN",
                "wave.combat.tactical_actions.blood_pact.result",
                {
                    player.getName() + " paie " + std::to_string(selfCost) + " PV pour forcer le sang à répondre.",
                    target.getName() + " reçoit " + std::to_string(damage) + " dégâts et une plaie qui insiste.",
                    "Le prix laisse aussi une vigueur sombre qui remonte dans les bras."
                },
                false
            );
            return true;
        }

        if (choice == 45)
        {
            if (!playerHasMedicOrCraftTechniqueAffinity(player))
            {
                MessageScreen::show("MAINS MAL ASSURÉES", "wave.combat.tactical_actions.field_remedy.locked", {"Un remède improvisé demande soin, dosage, atelier ou vraie habitude de terrain."}, false);
                return false;
            }

            const bool trained = player.hasPassiveSkill("field_remedy_mastery") || player.hasPassiveSkill("battle_suture_mastery") || player.hasPassiveSkill("careful_dosage") || player.hasPassiveSkill("steel_prayer_mastery");
            const int healAmount = std::max(3, player.getMaxHp() / (trained ? 10 : 14));
            player.heal(healAmount);
            player.applyRegeneration(trained ? 2 : 1, std::max(1, player.getLevel() / 30 + (trained ? 2 : 1)));
            player.applyGuardBoost(1, trained ? 9 : 5);

            std::vector<std::string> purged;
            if (player.hasPoison() && player.curePoison()) purged.push_back("poison");
            else if (player.hasBleeding() && player.cureBleeding()) purged.push_back("saignement");
            else if (player.hasBurning() && player.cureBurning()) purged.push_back("brûlure");
            else if (player.hasWeakening() && player.cureWeakening()) purged.push_back("affaiblissement");

            player.recordCanonicalEvent("actions_tactiques_combat", "remede_fortune", "Remède de fortune", 1);
            updateTacticalLearning(player, "remede_fortune");
            player.recordChallengeCombatAction("skill");

            std::vector<std::string> lines;
            lines.push_back(player.getName() + " trouve assez de calme pour serrer une plaie, doser vite et reprendre le fil du combat.");
            lines.push_back("Soin immédiat : +" + std::to_string(healAmount) + " PV, puis un souffle court continue de tenir.");
            if (!purged.empty()) lines.push_back("Le remède nettoie aussi : " + purged.front() + ".");
            MessageScreen::show("REMÈDE DE FORTUNE", "wave.combat.tactical_actions.field_remedy.result", lines, false);
            return true;
        }

        if (choice == 46)
        {
            if (!playerHasElementalistTechniqueAffinity(player))
            {
                MessageScreen::show("ÉLÉMENT MUET", "wave.combat.tactical_actions.elemental_blade.locked", {"Il faut une affinité magique, alchimique ou élémentaire pour tenir cette couleur dans l'arme."}, false);
                return false;
            }
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.elemental_blade");
            if (targetIndex < 0) return false;
            Monster& target = wave.getActiveEnemy(targetIndex);
            const int mastery = tacticalMasteryLevel(player, "lame_elementaire");
            const int masteryBonus = tacticalMasterySoftBonus(player, "lame_elementaire");
            const bool trained = mastery > 0 || player.hasPassiveSkill("elemental_blade_mastery") || player.hasPassiveSkill("wild_spark") || player.hasPassiveSkill("arcane_channel_mastery") || player.hasPassiveSkill("arcane_channeler");
            const int damage = std::max(4, player.getLevel() / 4 + random.between(4, (trained ? 12 : 8) + masteryBonus));
            target.takeDamage(damage);
            const int element = random.between(1, 4);
            std::string effectLine;
            if (element == 1) { target.applyBurning(2, std::max(1, player.getLevel() / 30 + 2 + masteryBonus / 2)); effectLine = "La lame laisse une chaleur accrochée à la plaie."; }
            else if (element == 2) { target.applyFrost(1); target.applyWeakening(1, (trained ? 8 : 5) + masteryBonus); effectLine = "Le froid durcit mal l'appui de la cible."; }
            else if (element == 3) { target.applyShock(1); target.applyNextHitVulnerability(1, (trained ? 10 : 6) + masteryBonus); effectLine = "Une étincelle ouvre le prochain angle."; }
            else { target.applyPoison(2, std::max(1, player.getLevel() / 34 + 1 + masteryBonus / 2)); effectLine = "La couleur tourne au venin et s'insinue dans la blessure."; }
            if (trained) player.applyPowerBoost(1, 8 + masteryBonus);
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "lame_elementaire", "Lame élémentaire", 1);
            updateTacticalLearning(player, "lame_elementaire");
            player.recordChallengeCombatAction("skill");
            MessageScreen::show("LAME ÉLÉMENTAIRE", "wave.combat.tactical_actions.elemental_blade.result", {
                player.getName() + " fait courir une couleur vive le long du geste.",
                target.getName() + " reçoit " + std::to_string(damage) + " dégâts.",
                effectLine,
                tacticalMasteryLine(player, "lame_elementaire")
            }, false);
            return true;
        }

        if (choice == 47)
        {
            if (!playerHasWardenTechniqueAffinity(player))
            {
                MessageScreen::show("CERCLE BANCAL", "wave.combat.tactical_actions.protective_circle.locked", {"Un cercle protecteur demande garde, foi, soutien ou vraie habitude de tenir une ligne."}, false);
                return false;
            }
            const int mastery = tacticalMasteryLevel(player, "cercle_protecteur");
            const int masteryBonus = tacticalMasterySoftBonus(player, "cercle_protecteur");
            const bool trained = mastery > 0 || player.hasPassiveSkill("protective_circle_mastery") || player.hasPassiveSkill("steel_prayer_mastery") || player.hasPassiveSkill("rampart_oath_mastery");
            player.applyGuardBoost(2, (trained ? 18 : 12) + masteryBonus);
            player.applyElementalWard(2, (trained ? 16 : 10) + masteryBonus);
            player.applyRegeneration(1, std::max(1, player.getLevel() / 36 + (trained ? 2 : 1) + masteryBonus / 3));
            if (trained && player.hasNextHitVulnerability()) player.cureVulnerability();
            player.recordCanonicalEvent("actions_tactiques_combat", "cercle_protecteur", "Cercle protecteur", 1);
            updateTacticalLearning(player, "cercle_protecteur");
            player.recordChallengeCombatAction("skill");
            MessageScreen::show("CERCLE PROTECTEUR", "wave.combat.tactical_actions.protective_circle.result", {
                player.getName() + " trace une limite courte autour de ses appuis.",
                "Le corps encaisse mieux, les éléments glissent davantage et le souffle revient par petites prises.",
                tacticalMasteryLine(player, "cercle_protecteur")
            }, false);
            return true;
        }

        if (choice == 48)
        {
            if (!playerHasSkirmisherTechniqueAffinity(player))
            {
                MessageScreen::show("TRAJECTOIRE INCERTAINE", "wave.combat.tactical_actions.binding_shot.locked", {"Ce tir demande un oeil de pisteur, de chasseur, d'archer ou d'éclaireur."}, false);
                return false;
            }
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.binding_shot");
            if (targetIndex < 0) return false;
            Monster& target = wave.getActiveEnemy(targetIndex);
            const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(target);
            const int mastery = tacticalMasteryLevel(player, "fleche_entravante");
            const int masteryBonus = tacticalMasterySoftBonus(player, "fleche_entravante");
            const bool trained = mastery > 0 || player.hasPassiveSkill("binding_shot_mastery") || player.hasPassiveSkill("stopping_shot_mastery") || player.hasPassiveSkill("ranger_eye");
            const int damage = std::max(3, player.getLevel() / 5 + random.between(3, (trained ? 10 : 7) + masteryBonus));
            target.takeDamage(damage);
            target.applyWeakening(1, (trained ? 10 : 6) + masteryBonus);
            target.applyNextHitVulnerability(1, (trained ? 9 : 5) + masteryBonus);
            if (profile.incomingAccuracyModifier < -8 || normalizeTacticalText(profile.archetype).find("aérien") != std::string::npos)
            {
                target.applyFrost(1);
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "fleche_entravante", "Flèche entravante", 1);
            updateTacticalLearning(player, "fleche_entravante");
            player.recordChallengeCombatAction("skill");
            MessageScreen::show("FLÈCHE ENTRAVANTE", "wave.combat.tactical_actions.binding_shot.result", {
                player.getName() + " vise moins la chair que l'appui qui allait fuir.",
                target.getName() + " reçoit " + std::to_string(damage) + " dégâts et perd un peu de liberté de mouvement.",
                tacticalMasteryLine(player, "fleche_entravante")
            }, false);
            return true;
        }

        if (choice == 49)
        {
            if (!playerHasBardTechniqueAffinity(player))
            {
                MessageScreen::show("VOIX SANS PORTÉE", "wave.combat.tactical_actions.inspiring_chant.locked", {"Ce chant demande présence, commandement, musique ou vraie habitude de tenir les autres debout."}, false);
                return false;
            }
            const int mastery = tacticalMasteryLevel(player, "chant_revigorant");
            const int masteryBonus = tacticalMasterySoftBonus(player, "chant_revigorant");
            const bool trained = mastery > 0 || player.hasPassiveSkill("inspiring_chant_mastery") || player.hasPassiveSkill("battle_order_mastery") || player.hasPassiveSkill("rally_breath");
            player.applyPrecisionBoost(2, (trained ? 4 : 2) + masteryBonus / 2);
            player.applyPowerBoost(1, (trained ? 12 : 7) + masteryBonus);
            player.applyRegeneration(2, std::max(1, player.getLevel() / 35 + (trained ? 2 : 1) + masteryBonus / 3));
            std::vector<std::string> purged;
            if (player.hasWeakening() && player.cureWeakening()) purged.push_back("affaiblissement");
            else if (player.hasShock() && player.cureShock()) purged.push_back("choc");
            else if (player.hasFrost() && player.cureFrost()) purged.push_back("givre");
            player.recordCanonicalEvent("actions_tactiques_combat", "chant_revigorant", "Chant revigorant", 1);
            updateTacticalLearning(player, "chant_revigorant");
            player.recordChallengeCombatAction("skill");
            std::vector<std::string> lines = {
                player.getName() + " pose une phrase courte, assez ferme pour redonner un axe au combat.",
                "Le rythme donne précision, puissance courte et souffle régulier.",
                tacticalMasteryLine(player, "chant_revigorant")
            };
            if (!purged.empty()) lines.push_back("La voix chasse aussi : " + purged.front() + ".");
            MessageScreen::show("CHANT REVIGORANT", "wave.combat.tactical_actions.inspiring_chant.result", lines, false);
            return true;
        }

        if (choice == 50)
        {
            if (!playerHasBeastTechniqueAffinity(player))
            {
                MessageScreen::show("INSTINCT FERMÉ", "wave.combat.tactical_actions.beast_instinct.locked", {"Il faut une affinité sauvage, une rage maîtrisée ou un vrai flair de chasse pour suivre cette impulsion."}, false);
                return false;
            }
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.beast_instinct");
            if (targetIndex < 0) return false;
            Monster& target = wave.getActiveEnemy(targetIndex);
            const int mastery = tacticalMasteryLevel(player, "instinct_bete");
            const int masteryBonus = tacticalMasterySoftBonus(player, "instinct_bete");
            const bool trained = mastery > 0 || player.hasPassiveSkill("beast_instinct_mastery") || player.hasPassiveSkill("semi_wolf_tracking") || player.hasPassiveSkill("rage_control_mastery");
            player.applyPrecisionBoost(1, (trained ? 4 : 2) + masteryBonus / 2);
            player.applyPowerBoost(1, (trained ? 12 : 7) + masteryBonus);
            const int damage = std::max(4, player.getLevel() / 4 + random.between(4, (trained ? 13 : 9) + masteryBonus));
            target.takeDamage(damage);
            if (target.hasBleeding() || trained || random.between(1, 100) <= 45)
            {
                target.applyBleeding(2, std::max(1, player.getLevel() / 30 + 1 + masteryBonus / 3));
            }
            else
            {
                target.applyWeakening(1, 6);
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "instinct_bete", "Instinct de bête", 1);
            updateTacticalLearning(player, "instinct_bete");
            player.recordChallengeCombatAction("skill");
            MessageScreen::show("INSTINCT DE BÊTE", "wave.combat.tactical_actions.beast_instinct.result", {
                player.getName() + " cesse de trop réfléchir et suit la première faille vivante.",
                target.getName() + " reçoit " + std::to_string(damage) + " dégâts dans un angle brutal.",
                tacticalMasteryLine(player, "instinct_bete")
            }, false);
            return true;
        }

        if (choice == 51)
        {
            if (!playerHasDuelTechniqueAffinity(player))
            {
                MessageScreen::show("PAS TROP LOURD", "wave.combat.tactical_actions.blade_dance.locked", {"Cette danse demande duel, lame légère, corps entraîné ou vraie ruse de mêlée."}, false);
                return false;
            }
            const int targetIndex = chooseTacticalTarget(wave, "wave.combat.tactical_actions.blade_dance");
            if (targetIndex < 0) return false;
            Monster& target = wave.getActiveEnemy(targetIndex);
            const int lightWeaponCount = countLightWeaponsForBladeDance(player);
            if (lightWeaponCount <= 0)
            {
                MessageScreen::show("DANSE IMPOSSIBLE", "wave.combat.tactical_actions.blade_dance.no_light_weapon", {
                    "Danse de lame demande au moins une arme légère équipée ou présente dans l'inventaire.",
                    "Avec une seule arme légère, la danse fait 2 coups ; avec deux armes légères, elle monte à 3 coups."
                }, false);
                return false;
            }
            const int mastery = tacticalMasteryLevel(player, "danse_lame");
            const int masteryBonus = tacticalMasterySoftBonus(player, "danse_lame");
            const bool trained = mastery > 0 || player.hasPassiveSkill("blade_dance_mastery") || player.hasPassiveSkill("shadow_step_mastery") || player.hasPassiveSkill("shadow_stepper") || player.hasPassiveSkill("inner_mantra_mastery") || player.hasPassiveSkill("rogue_feint_mastery") || player.hasPassiveSkill("rogue_feinter");
            const int strikeCount = lightWeaponCount >= 2 ? 3 : 2;
            int totalDamage = 0;
            std::vector<std::string> hitLines;
            for (int strike = 1; strike <= strikeCount; ++strike)
            {
                if (target.isDead()) break;
                const int rollMax = (trained ? 7 : 4) + masteryBonus + (strike == 1 ? 1 : 0);
                const int strikeDamage = std::max(2, player.getLevel() / (6 + strike) + random.between(2, std::max(3, rollMax)));
                target.takeDamage(strikeDamage);
                totalDamage += strikeDamage;
                hitLines.push_back("Coupe " + std::to_string(strike) + "/" + std::to_string(strikeCount) + " : " + std::to_string(strikeDamage) + " dégâts.");
            }
            target.applyNextHitVulnerability(1, (trained ? 10 : 6) + masteryBonus + (strikeCount >= 3 ? 2 : 0));
            if (trained || random.between(1, 100) <= 35)
            {
                player.applyGuardBoost(1, (trained ? 8 : 5) + masteryBonus);
            }
            ThreatSystem::consumeForcedTargetIfNeeded(target);
            wave.removeDeadAndReplace();
            player.recordCanonicalEvent("actions_tactiques_combat", "danse_lame", "Danse de lame", 1);
            updateTacticalLearning(player, "danse_lame");
            player.recordChallengeCombatAction("skill");
            std::vector<std::string> lines;
            lines.push_back(strikeCount >= 3
                ? player.getName() + " lance une vraie Danse de lame à trois coupes, portée par deux armes légères."
                : player.getName() + " n'a qu'une arme légère prête : la Danse de lame reste limitée à deux coupes.");
            lines.insert(lines.end(), hitLines.begin(), hitLines.end());
            lines.push_back(target.getName() + " subit " + std::to_string(totalDamage) + " dégâts au total et laisse un angle de reprise.");
            lines.push_back(tacticalMasteryLine(player, "danse_lame"));
            MessageScreen::show("DANSE DE LAME", "wave.combat.tactical_actions.blade_dance.result", lines, false);
            return true;
        }

        return false;
    }
}

bool PlayerWaveCombatTurn::play(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random,
    bool& escapeSucceeded,
    DifficultyMode difficulty,
    bool teamOrdersAvailable,
    const std::function<bool()>& openTeamOrders
)
{
    if (player.hasEntanglement() && player.hasPassiveSkill("church_oath_roots"))
    {
        const int rootChance = std::clamp(30 + player.getLevel() / 6 + player.getActiveSkillMasteryLevel("retrait_controle") * 3, 30, 58);
        if (random.between(1, 100) <= rootChance && player.cureEntanglement())
        {
            MessageScreen::show(
                "SERMENT DES RACINES",
                "wave.combat.player.entangled_oath_roots",
                {
                    player.getName() + " sent les fils ou racines avant qu'ils ne ferment complètement le tour.",
                    "L'entrave est brisée, mais ce n'est pas gratuit : le serment donne un vrai contre-jeu, pas une immunité."
                },
                false
            );
            player.recordCanonicalEvent("serments_eglise", "racines_contre_entrave", "Serment des Racines a brisé une entrave", 1);
        }
    }

    if (player.consumeEntanglementTurn())
    {
        MessageScreen::show(
            "ENTRAVÉ",
            "wave.combat.player.entangled_skip",
            {
                player.getName() + " perd son tour : des fils, racines ou liens verrouillent le mouvement.",
                "Un ordre clair à un allié peut parfois compenser, mais le corps principal ne peut pas agir cette fois."
            },
            false
        );
        return true;
    }

    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(
            wave.getActiveEnemy(i),
            random
        );
    }

    const MenuScreen turnScreen = CombatMenu::buildTurnScreen(player, teamOrdersAvailable);
    int choice = TerminalInterface::askMenuChoiceFromOptions(
        turnScreen,
        "Choix invalide. Entre un chiffre entre 0 et 10."
    );

    Console::clear();

    if (choice == 0)
    {
        return openWaveInterface(player, wave, difficulty, teamOrdersAvailable, openTeamOrders);
    }

    if (choice == 1)
    {
        MenuScreen attackScreen("ACTION OFFENSIVE", "wave.combat.attack_selector");
        attackScreen.addSubtitle("Tour de " + player.getName());
        attackScreen.addLine("Choisis entre une attaque ciblée classique ou une compétence active avec cooldown.");
        attackScreen.addBackOption("Retour", "wave.combat.attack.back");
        attackScreen.addOption(1, "Attaque ciblée", "Choisir un ennemi actif puis attaquer normalement.", true, "wave.combat.attack.targeted");
        attackScreen.addOption(2, "Compétence active", "Multi-coups, multi-cibles ou cible aléatoire selon la technique choisie.", true, "wave.combat.attack.class_skill");

        int attackChoice = TerminalInterface::askMenuChoiceFromOptions(
            attackScreen,
            "Choix invalide."
        );

        Console::clear();

        if (attackChoice == 0)
        {
            return false;
        }

        bool used = false;
        if (attackChoice == 1)
        {
            used = CombatTargetMenu::openForAttack(
                player,
                wave,
                random
            );
            if (used)
            {
                player.recordChallengeCombatAction("basic_attack");
            }
        }
        else if (attackChoice == 2)
        {
            used = CombatTargetMenu::openForClassSkill(
                player,
                wave,
                random
            );
            if (used)
            {
                player.recordChallengeCombatAction("skill");
            }
        }

        return used;
    }

    if (choice == 2)
    {
        const bool used = CombatPotionMenu::openQuickHealing(player);
        if (used)
        {
            player.recordChallengeCombatAction("consumable");
        }
        return used;
    }

    if (choice == 3)
    {
        const bool used = CombatPotionMenu::openAgainstWave(
            player,
            wave,
            random,
            PVE_POTION_DAMAGE_BONUS
        );
        if (used)
        {
            player.recordChallengeCombatAction("consumable");
        }
        return used;
    }

    if (choice == 4)
    {
        EquipmentMenu::open(player);
        return false;
    }

    if (choice == 5)
    {
        return InventoryMenu::open(player);
    }

    if (choice == 6)
    {
        DefensePostureSystem::enterDefensePosture(player);
        player.recordChallengeCombatAction("defense");
        return true;
    }

    if (choice == 7)
    {
        MessageScreen::show(
            "TOUR PASSÉ",
            "wave.combat.wait",
            {
                player.getName() + " choisit de ne rien faire ce tour-ci.",
                "Parfois, survivre commence par attendre le bon moment."
            },
            false
        );

        player.recordChallengeCombatAction("wait");
        return true;
    }

    if (choice == 8)
    {
        escapeSucceeded = EscapeSystem::playerAttemptsEscape(
            player,
            random,
            difficulty,
            wave.getTotalRemainingEnemyCount()
        );
        player.recordChallengeCombatAction("escape");
        return true;
    }

    if (choice == 9)
    {
        if (teamOrdersAvailable && openTeamOrders)
        {
            openTeamOrders();
            return false;
        }

        MessageScreen::show(
            "CONSIGNES D'ÉQUIPE",
            "wave.combat.team_orders.unavailable",
            {"Aucune recrue ou IA alliée stable n'attend de consigne dans ce combat."},
            false
        );
        return false;
    }

    if (choice == 10)
    {
        const bool used = openTacticalActionMenu(player, wave, random);
        if (used)
        {
            player.recordChallengeCombatAction("tactical_action");
        }
        return used;
    }

    return false;
}

bool PlayerWaveCombatTurn::openWaveInterface(
    Player& player,
    EnemyCombatQueue& wave,
    DifficultyMode difficulty,
    bool teamOrdersAvailable,
    const std::function<bool()>& openTeamOrders
)
{
    MenuScreen screen("INTERFACE DE VAGUE", "wave.combat.interface");
    screen.addSubtitle(player.getName() + " face à " + std::to_string(wave.getTotalRemainingEnemyCount()) + " adversaire(s) restant(s)");
    screen.addBackOption("Retour", "wave.interface.back");
    screen.addOption(1, "Voir l'état du combat", "Adversaires actifs et résumé de la file.", true, "wave.interface.state");
    screen.addOption(2, "Voir mes statistiques", "Ouvre les statistiques du personnage.", true, "wave.interface.stats");
    screen.addOption(3, "Résumé équipement", "Affichage simple de l'équipement.", true, "wave.interface.equipment");
    screen.addOption(4, "Compétences de rôle", "Actions et rappels liés au rôle.", true, "wave.interface.role");
    screen.addOption(5, "Observer / analyser les adversaires", "Relit la vague active.", true, "wave.interface.observe");
    screen.addOption(6, "Voir un adversaire dans le bestiaire", "Choisir une entrée parmi les ennemis actifs.", true, "wave.interface.bestiary");
    screen.addOption(7, "Consignes aux alliés", teamOrdersAvailable ? "Consignes ciblées ou de groupe, sans consommer le tour." : "Indisponible sans allié stable.", teamOrdersAvailable, teamOrdersAvailable ? "wave.interface.team_directives" : "wave.interface.team_directives_unavailable");
    screen.addOption(8, "Contrôle des invocations", "Rappel des ordres actuels.", true, "wave.interface.summons");
    screen.addOption(9, "Journal bêta local", "Affiche le fichier de logs combat/exploration à envoyer au dev.", true, "wave.interface.beta_log");

    int interfaceChoice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");

    Console::clear();

    if (interfaceChoice == 0)
    {
        return false;
    }

    if (interfaceChoice == 1)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 2)
    {
        StatisticsMenu::open(player, difficulty, false);
        return false;
    }

    if (interfaceChoice == 3)
    {
        player.displaySimpleEquipment();
        return false;
    }

    if (interfaceChoice == 4)
    {
        return CombatRoleMenu::open(player);
    }

    if (interfaceChoice == 5)
    {
        wave.displayActiveEnemies();
        wave.displayQueueSummary();
        return false;
    }

    if (interfaceChoice == 6)
    {
        if (!wave.hasActiveEnemies())
        {
            MessageScreen::show(
                "BESTIAIRE",
                "wave.interface.bestiary_empty",
                {"Aucun adversaire actif à consulter dans le bestiaire."},
                false
            );
            return false;
        }

        MenuScreen targetScreen("BESTIAIRE DE COMBAT", "wave.interface.bestiary_target");
        targetScreen.addLine("Choisis l'adversaire à rechercher dans le bestiaire.");
        targetScreen.addBackOption("Retour", "wave.interface.bestiary.back");

        for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
        {
            targetScreen.addOption(
                index + 1,
                wave.getActiveEnemy(index).getName(),
                "Consulter ce que tu sais déjà sur cette créature.",
                true,
                "wave.interface.bestiary.target"
            );
        }

        int targetChoice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choix invalide.");

        Console::clear();

        if (targetChoice == 0)
        {
            return false;
        }

        BestiaryMenu::displayObjectEntry(
            wave.getActiveEnemy(targetChoice - 1).getName()
        );

        return false;
    }

    if (interfaceChoice == 7)
    {
        if (teamOrdersAvailable && openTeamOrders)
        {
            openTeamOrders();
            return false;
        }

        MessageScreen::show(
            "CONSIGNES AUX ALLIÉS",
            "wave.interface.team_directives_unavailable",
            {"Aucun allié stable n'attend de consigne sur ce champ de bataille."},
            false
        );
        return false;
    }

    if (interfaceChoice == 8)
    {
        MessageScreen::show(
            "CONTRÔLE DES INVOCATIONS",
            "wave.interface.summons_order",
            {
                "Tes invocations suivent l'ordre donné au début du combat.",
                "Changer cet ordre au milieu du chaos demande une ouverture que tu n'as pas encore."
            },
            false
        );
        return false;
    }

    if (interfaceChoice == 9)
    {
        MessageScreen::show(
            "JOURNAL BÊTA LOCAL",
            "wave.interface.beta_log",
            {
                "Le journal local note les écrans importants, les combats, l'exploration et les menus traversés.",
                "Fichier à envoyer au dev en cas de retour précis : " + RuntimeLog::currentLogPath(),
                "Il sert surtout à comprendre ce qui s'est passé sans demander au testeur de tout raconter à la main."
            },
            false
        );
        return false;
    }

    return false;
}
