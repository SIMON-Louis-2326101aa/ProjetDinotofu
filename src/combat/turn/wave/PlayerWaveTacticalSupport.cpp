// EN: Shared affinities, targeting and mastery helpers for wave tactical actions.
// FR: Helpers partagés d’affinités, ciblage et maîtrise des actions tactiques de vague.

#include "combat/turn/wave/PlayerWaveTacticalSupport.hpp"

#include "combat/system/CombatClassSystem.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/armor/Armor.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace PlayerWaveTacticalSupport
{


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

}
