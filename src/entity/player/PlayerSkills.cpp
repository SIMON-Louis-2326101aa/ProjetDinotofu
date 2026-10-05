// EN: Split from Player.cpp to keep Player responsibilities maintainable.
// FR: Extrait de Player.cpp afin de garder des responsabilités Player maintenables.
#include "entity/Player.hpp"
#include "entity/player/PlayerUiSupport.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "core/VersionInfo.hpp"
#include "economy/Money.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "progression/TitleCatalog.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

namespace
{



    int countEnvironmentalResiliencePassives(const Player& player)
    {
        int count = 0;

        const std::vector<std::string> passiveIds = {
            "temperature_adaptation",
            "minor_fire_resistance",
            "infernal_fire_resistance",
            "minor_cold_resistance",
            "semi_lizard_scales",
            "dragon_weather_blood",
            "orcish_forced_march",
            "cautious_pathing",
            "threat_route_planner",
            "temperature_drill",
            "terrain_drill"
        };

        for (const std::string& passiveId : passiveIds)
        {
            if (player.hasPassiveSkill(passiveId))
            {
                ++count;
            }
        }

        return count;
    }

    bool containsSkillId(const std::vector<std::string>& skills, const std::string& skillId)
    {
        return std::find(skills.begin(), skills.end(), skillId) != skills.end();
    }

    bool isChurchOathSkillId(const std::string& skillId)
    {
        return skillId.rfind("church_oath_", 0) == 0;
    }

    bool isChurchOathContractId(const std::string& skillId)
    {
        return isChurchOathSkillId(skillId) && skillId != "church_oath_broken_trace";
    }

    int canonicalKeyCount(const Player& player, const std::string& category, const std::string& key)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key && record.count > 0)
            {
                total += record.count;
            }
        }
        return total;
    }

    bool isActiveChurchOathContract(const Player& player, const std::string& skillId)
    {
        if (!isChurchOathContractId(skillId) || !player.isPassiveSkillUnlocked(skillId))
        {
            return false;
        }

        const int sworn = canonicalKeyCount(player, "serments_eglise", skillId);
        const int broken = canonicalKeyCount(player, "serments_rompus", skillId);
        if (sworn == 0 && broken == 0)
        {
            // Compatibility with very old saves where an oath was stored only as an unlocked passive.
            return true;
        }
        return sworn > broken;
    }

    std::vector<int> activeMasteryThresholdsForMax(int maxLevel)
    {
        const std::vector<int> allThresholds = {3, 6, 10, 15, 22, 30, 40, 55, 75, 100};
        maxLevel = std::clamp(maxLevel, 1, static_cast<int>(allThresholds.size()));
        return std::vector<int>(allThresholds.begin(), allThresholds.begin() + maxLevel);
    }

    std::vector<int> passiveMasteryThresholdsForMax(int maxLevel)
    {
        const std::vector<int> allThresholds = {3, 8, 16, 30, 55};
        maxLevel = std::clamp(maxLevel, 1, static_cast<int>(allThresholds.size()));
        return std::vector<int>(allThresholds.begin(), allThresholds.begin() + maxLevel);
    }

    int activeMasteryMaxLevelForAction(const std::string& actionKey)
    {
        // 10 reste le maximum du maximum. Les actions simples ou très spécialisées s'arrêtent avant.
        // Les petits gestes de terrain ne justifient pas dix paliers : ils deviennent surtout plus propres.
        if (actionKey == "lanterne_cible"
            || actionKey == "lanterne_sol"
            || actionKey == "coup_de_pied"
            || actionKey == "poussiere"
            || actionKey == "retrait_controle"
            || actionKey == "souffle_ralliement"
            || actionKey == "voile_urgence"
            || actionKey == "signe_vigueur"
            || actionKey == "suture_bataille"
            || actionKey == "remede_fortune"
            || actionKey == "image_trompeuse"
            || actionKey == "mantra_interieur")
        {
            return 4;
        }

        // Actions tactiques de lecture, contrôle ou préparation : assez de marge pour sentir l'habitude.
        if (actionKey == "observation_active"
            || actionKey == "exploitation_ouverture"
            || actionKey == "piege_artisan"
            || actionKey == "preparation_arme"
            || actionKey == "brise_garde"
            || actionKey == "posture_soutien"
            || actionKey == "rupture_formation"
            || actionKey == "coupe_signal"
            || actionKey == "lecture_corps"
            || actionKey == "chaine_etats"
            || actionKey == "marque_proie"
            || actionKey == "lecture_menace"
            || actionKey == "balayage_bas"
            || actionKey == "rupture_concentration"
            || actionKey == "rupture_allonge"
            || actionKey == "perce_rempart"
            || actionKey == "rupture_ancrage"
            || actionKey == "frappe_elan"
            || actionKey == "etincelle_instable"
            || actionKey == "cri_guerre"
            || actionKey == "bombe_atelier"
            || actionKey == "totem_souffle"
            || actionKey == "pacte_sanguin"
            || actionKey == "lien_invocation"
            || actionKey == "cercle_protecteur"
            || actionKey == "chant_revigorant")
        {
            return 6;
        }

        // Techniques plus profondes ou plus ouvertes : plafond haut, mais toujours progressif.
        if (actionKey == "pas_ombre"
            || actionKey == "coup_arc"
            || actionKey == "feinte_sournoise"
            || actionKey == "canalisation_arcanique"
            || actionKey == "serment_rempart"
            || actionKey == "tir_arret"
            || actionKey == "rage_maitrisee"
            || actionKey == "ordre_bataille"
            || actionKey == "priere_acier"
            || actionKey == "lame_elementaire"
            || actionKey == "fleche_entravante"
            || actionKey == "instinct_bete")
        {
            return 8;
        }
        if (actionKey == "danse_lame")
        {
            return 10;
        }
        return 6;
    }

    int passiveMasteryMaxLevelForAction(const std::string& actionKey)
    {
        // 5 reste le maximum du maximum. Les réflexes simples ne doivent pas inventer des paliers inutiles.
        if (actionKey == "lanterne_cible"
            || actionKey == "lanterne_sol"
            || actionKey == "coup_de_pied"
            || actionKey == "poussiere"
            || actionKey == "retrait_controle"
            || actionKey == "souffle_ralliement"
            || actionKey == "voile_urgence"
            || actionKey == "signe_vigueur"
            || actionKey == "suture_bataille"
            || actionKey == "remede_fortune"
            || actionKey == "image_trompeuse"
            || actionKey == "mantra_interieur")
        {
            return 2;
        }
        if (actionKey == "observation_active"
            || actionKey == "exploitation_ouverture"
            || actionKey == "piege_artisan"
            || actionKey == "preparation_arme"
            || actionKey == "brise_garde"
            || actionKey == "posture_soutien"
            || actionKey == "rupture_formation"
            || actionKey == "coupe_signal"
            || actionKey == "lecture_corps"
            || actionKey == "chaine_etats"
            || actionKey == "marque_proie"
            || actionKey == "lecture_menace"
            || actionKey == "balayage_bas"
            || actionKey == "rupture_concentration"
            || actionKey == "rupture_allonge"
            || actionKey == "perce_rempart"
            || actionKey == "rupture_ancrage"
            || actionKey == "frappe_elan"
            || actionKey == "etincelle_instable"
            || actionKey == "cri_guerre"
            || actionKey == "bombe_atelier"
            || actionKey == "totem_souffle"
            || actionKey == "pacte_sanguin"
            || actionKey == "lien_invocation"
            || actionKey == "cercle_protecteur"
            || actionKey == "chant_revigorant")
        {
            return 3;
        }
        if (actionKey == "pas_ombre"
            || actionKey == "coup_arc"
            || actionKey == "feinte_sournoise"
            || actionKey == "canalisation_arcanique"
            || actionKey == "serment_rempart"
            || actionKey == "tir_arret"
            || actionKey == "rage_maitrisee"
            || actionKey == "ordre_bataille"
            || actionKey == "priere_acier"
            || actionKey == "lame_elementaire"
            || actionKey == "fleche_entravante"
            || actionKey == "instinct_bete")
        {
            return 4;
        }
        if (actionKey == "danse_lame")
        {
            return 5;
        }
        return 3;
    }

    int masteryLevelFromSuccessfulUses(int uses, int maxLevel)
    {
        const std::vector<int> thresholds = activeMasteryThresholdsForMax(maxLevel);
        int level = 0;
        for (int threshold : thresholds)
        {
            if (uses >= threshold) ++level;
        }
        return std::clamp(level, 0, static_cast<int>(thresholds.size()));
    }

    std::string masteryRankLabel(int masteryLevel, int maxLevel)
    {
        if (masteryLevel >= maxLevel) return maxLevel >= 9 ? "maîtrise parfaite" : "maîtrise complète";
        if (masteryLevel >= 8) return "maîtrise experte";
        if (masteryLevel >= 6) return "maîtrise solide";
        if (masteryLevel >= 4) return "maîtrise stable";
        if (masteryLevel >= 2) return "maîtrise correcte";
        if (masteryLevel >= 1) return "faible maîtrise";
        return "essai";
    }

    int nextMasteryThresholdFromSuccessfulUses(int uses, int maxLevel)
    {
        const std::vector<int> thresholds = activeMasteryThresholdsForMax(maxLevel);
        for (int threshold : thresholds)
        {
            if (uses < threshold)
            {
                return threshold;
            }
        }
        return -1;
    }

    std::string masteryProgressHintFromUses(int uses, int maxLevel)
    {
        const int nextThreshold = nextMasteryThresholdFromSuccessfulUses(uses, maxLevel);
        if (nextThreshold < 0)
        {
            return "palier maximal atteint pour cette compétence";
        }
        return "prochain palier à " + std::to_string(nextThreshold)
            + " usages réussis, plafond local " + std::to_string(maxLevel) + "/10";
    }

    std::string masteryEffectHint(int masteryLevel, int maxLevel)
    {
        if (masteryLevel >= maxLevel) return "maîtrise complète locale : la compétence change vraiment la décision de tour, avec impact net sans supprimer coût, tour ou risque";
        if (masteryLevel >= 8) return "geste expert : vraie montée d'impact, meilleur rythme, effet secondaire beaucoup plus crédible";
        if (masteryLevel >= 6) return "geste solide : bonus visible de puissance, protection ou contrôle ; la compétence doit se ressentir dans le combat";
        if (masteryLevel >= 4) return "geste stable : l'effet commence vraiment à se sentir, sans devenir gratuit";
        if (masteryLevel >= 2) return "geste compris : bonus perceptible de précision, souffle ou contrôle selon l'actif";
        if (masteryLevel >= 1) return "première habitude : l'actif reste choisi, mais il cesse d'être juste symbolique";
        return "aucun bonus stable : c'est encore une vraie tentative";
    }

    int passiveMasteryLevelFromSuccessfulUses(int uses, int maxLevel)
    {
        const std::vector<int> thresholds = passiveMasteryThresholdsForMax(maxLevel);
        int level = 0;
        for (int threshold : thresholds)
        {
            if (uses >= threshold) ++level;
        }
        return std::clamp(level, 0, static_cast<int>(thresholds.size()));
    }

    std::string passiveMasteryRankLabel(int passiveLevel, int maxLevel)
    {
        if (passiveLevel >= maxLevel) return maxLevel >= 5 ? "passif maîtrisé" : "passif complet";
        if (passiveLevel >= 4) return "passif fiable";
        if (passiveLevel >= 3) return "passif ancré";
        if (passiveLevel >= 2) return "passif naturel";
        if (passiveLevel >= 1) return "passif naissant";
        return "passif en essai";
    }

    int nextPassiveMasteryThresholdFromSuccessfulUses(int uses, int maxLevel)
    {
        const std::vector<int> thresholds = passiveMasteryThresholdsForMax(maxLevel);
        for (int threshold : thresholds)
        {
            if (uses < threshold)
            {
                return threshold;
            }
        }
        return -1;
    }

    std::string passiveMasteryProgressHintFromUses(int uses, int maxLevel)
    {
        const int nextThreshold = nextPassiveMasteryThresholdFromSuccessfulUses(uses, maxLevel);
        if (nextThreshold < 0)
        {
            return "niveau passif maximal atteint pour cette maîtrise";
        }
        if (uses < 3)
        {
            return "déblocage à 3 essais réussis, plafond local " + std::to_string(maxLevel) + "/5";
        }
        return "prochain niveau passif à " + std::to_string(nextThreshold)
            + " usages réussis, plafond local " + std::to_string(maxLevel) + "/5";
    }

    std::string passiveMasteryEffectHint(int passiveLevel, int maxLevel)
    {
        if (passiveLevel >= maxLevel) return "réflexe complet local : soutien vraiment utile quand l'actif lié est choisi, sans jouer à ta place";
        if (passiveLevel >= 4) return "réaction régulière : meilleure lecture hors combat ou sous pression, bonus visible mais contrôlé";
        if (passiveLevel >= 3) return "réaction ancrée : le soutien pèse dans le résultat sans devenir une action gratuite";
        if (passiveLevel >= 2) return "habitude naturelle : gain clair de lecture ou de stabilité";
        if (passiveLevel >= 1) return "réflexe naissant : effet simple, déjà perceptible et pas seulement cosmétique";
        return "pas encore débloqué : il faut réussir 3 essais";
    }

    int countRecentEquipmentUsageContaining(const std::vector<std::string>& usage, const std::string& token)
    {
        int count = 0;
        for (const std::string& entry : usage)
        {
            if (entry.find(token) != std::string::npos)
            {
                ++count;
            }
        }
        return count;
    }

    int countRecentFullLoadoutUsage(const std::vector<std::string>& usage)
    {
        int count = 0;
        for (const std::string& entry : usage)
        {
            if (entry.find("Arme:") != std::string::npos && entry.find("Armure:") != std::string::npos)
            {
                ++count;
            }
        }
        return count;
    }

    bool hasRecentFullLoadoutUsage(const std::vector<std::string>& usage, int requiredCount)
    {
        return countRecentFullLoadoutUsage(usage) >= requiredCount;
    }

    std::string playerSkillDisplayName(const std::string& skillId)
    {
        if (skillId == "night_vision") return "Vision nocturne";
        if (skillId == "dwarven_mine_sense") return "Sens des galeries";
        if (skillId == "elven_fine_perception") return "Perception elfique";
        if (skillId == "halfling_lucky_step") return "Pas chanceux";
        if (skillId == "dragon_weather_blood") return "Sang draconique d'endurance";
        if (skillId == "temperature_adaptation") return "Adaptation aux températures";
        if (skillId == "minor_fire_resistance") return "Résistance légère au feu";
        if (skillId == "infernal_fire_resistance") return "Résistance infernale au feu";
        if (skillId == "minor_cold_resistance") return "Résistance légère au froid";
        if (skillId == "fire_vulnerability") return "Faiblesse aux flammes";
        if (skillId == "semi_wolf_tracking") return "Flair de meute";
        if (skillId == "semi_fox_cunning") return "Ruse de renard";
        if (skillId == "semi_dog_loyal_scent") return "Flair loyal";
        if (skillId == "semi_cat_reflexes") return "Réflexes félins";
        if (skillId == "semi_lizard_scales") return "Écailles tempérées";
        if (skillId == "semi_bird_open_sky") return "Vue des hauteurs";
        if (skillId == "orcish_forced_march") return "Marche forcée orque";
        if (skillId == "fairy_mana_sense") return "Sens féerique du mana";
        if (skillId == "survival_breath") return "Souffle de survie";
        if (skillId == "ranger_eye") return "Œil de rôdeur";
        if (skillId == "steady_guard") return "Garde stable";
        if (skillId == "living_rampart") return "Rempart vivant";
        if (skillId == "sure_hand") return "Main sûre";
        if (skillId == "careful_dosage") return "Dosage prudent";
        if (skillId == "chain_execution") return "Enchaînement";
        if (skillId == "reflex_counter") return "Contre réflexe";
        if (skillId == "cautious_channeling") return "Canalisation prudente";
        if (skillId == "shadow_step") return "Pas de l'ombre";
        if (skillId == "arcane_impulse") return "Élan arcanique";
        if (skillId == "prepared_volley") return "Salve préparée";
        if (skillId == "tracking_mark") return "Marque de pisteur";
        if (skillId == "blade_discipline") return "Discipline de lame";
        if (skillId == "splitting_blow") return "Frappe fendue";
        if (skillId == "armor_crack") return "Fracasse-garde";
        if (skillId == "reach_control") return "Contrôle d'allonge";
        if (skillId == "battle_instinct") return "Instinct de bataille";
        if (skillId == "veteran_rhythm") return "Rythme de vétéran";
        if (skillId == "scar_tissue") return "Peau des survivants";
        if (skillId == "escape_reader") return "Lecture de fuite";
        if (skillId == "boss_memory") return "Mémoire de boss";
        if (skillId == "armor_habit") return "Habitude d'armure";
        if (skillId == "loadout_memory") return "Mémoire d'équipement";
        if (skillId == "field_maintenance") return "Entretien de terrain";
        if (skillId == "curse_anchor_awareness") return "Conscience d'ancrage";
        if (skillId == "material_sorting_habit") return "Tri des composants";
        if (skillId == "equipment_identity") return "Identité d'équipement";
        if (skillId == "guild_route_memory") return "Mémoire de route de guilde";
        if (skillId == "bestiary_family_reader") return "Lecture des familles";
        if (skillId == "weapon_care_habit") return "Soin d'arme";
        if (skillId == "armor_fit_memory") return "Mémoire d'ajustement";
        if (skillId == "cautious_pathing") return "Pas prudent";
        if (skillId == "threat_route_planner") return "Plan de route dangereux";
        if (skillId == "environmental_resilience") return "Résilience environnementale";
        if (skillId == "temperature_drill") return "Habitude chaleur / froid";
        if (skillId == "terrain_drill") return "Habitude terrain difficile";
        if (skillId == "footwork_drill") return "Appuis entraînés";
        if (skillId == "combat_observation") return "Observation tactique";
        if (skillId == "field_observer") return "Lecture de terrain";
        if (skillId == "contract_reader") return "Lecture de contrat";
        if (skillId == "guard_breaker") return "Casseur de garde";
        if (skillId == "support_rhythm") return "Rythme de soutien";
        if (skillId == "formation_breaker") return "Briseur de formation";
        if (skillId == "signal_cut_awareness" || skillId == "signal_cutter") return "Lecture coupe-signal";
        if (skillId == "body_reader") return "Lecture des corps";
        if (skillId == "status_conductor") return "Conducteur d'états";
        if (skillId == "prey_marker") return "Marqueur de proie";
        if (skillId == "controlled_retreat") return "Sens du retrait";
        if (skillId == "threat_reader") return "Lecture des menaces";
        if (skillId == "low_sweeper") return "Lecture du balayage bas";
        if (skillId == "focus_breaker") return "Briseur de concentration";
        if (skillId == "reach_breaker") return "Briseur d'allonge";
        if (skillId == "shield_piercer") return "Perce-rempart";
        if (skillId == "anchor_breaker") return "Briseur d'ancrage";
        if (skillId == "rally_breath") return "Souffle rallié";
        if (skillId == "momentum_striker") return "Élan canalisé";
        if (skillId == "emergency_warder") return "Réflexe de voile";
        if (skillId == "wild_spark") return "Instabilité apprivoisée";
        if (skillId == "war_cry_caller") return "Voix de guerre";
        if (skillId == "arc_sweep") return "Coup en arc";
        if (skillId == "vigor_sign") return "Signe de vigueur";
        if (skillId == "rogue_feint") return "Feinte sournoise";
        if (skillId == "battle_suture") return "Suture de bataille";
        if (skillId == "arcane_channel") return "Canalisation arcanique";
        if (skillId == "rampart_oath") return "Serment du rempart";
        if (skillId == "stopping_shot") return "Tir d'arrêt";
        if (skillId == "mastered_rage") return "Rage maîtrisée";
        if (skillId == "battle_order") return "Ordre de bataille";
        if (skillId == "breath_totem") return "Totem de souffle";
        if (skillId == "workshop_bomb") return "Bombe d'atelier";
        if (skillId == "steel_prayer") return "Prière d'acier";
        if (skillId == "inner_mantra") return "Mantra intérieur";
        if (skillId == "trick_image") return "Image trompeuse";
        if (skillId == "summoning_link") return "Lien d'invocation";
        if (skillId == "blood_pact") return "Pacte sanguin";
        if (skillId == "field_remedy") return "Remède de fortune";
        if (skillId == "elemental_blade") return "Lame élémentaire";
        if (skillId == "protective_circle") return "Cercle protecteur";
        if (skillId == "binding_shot") return "Flèche entravante";
        if (skillId == "inspiring_chant") return "Chant revigorant";
        if (skillId == "beast_instinct") return "Instinct de bête";
        if (skillId == "blade_dance") return "Danse de lame";
        if (skillId == "shadow_step_mastery" || skillId == "shadow_stepper") return "Maîtrise du pas de l'ombre";
        if (skillId == "arc_sweep_mastery" || skillId == "arc_sweeper") return "Amplitude contrôlée";
        if (skillId == "vigor_sign_mastery") return "Souffle de vigueur";
        if (skillId == "rogue_feint_mastery" || skillId == "rogue_feinter") return "Angle de feinte";
        if (skillId == "battle_suture_mastery") return "Gestes de suture";
        if (skillId == "arcane_channel_mastery" || skillId == "arcane_channeler") return "Canalisation stabilisée";
        if (skillId == "rampart_oath_mastery") return "Tenue du rempart";
        if (skillId == "stopping_shot_mastery") return "Œil d'arrêt";
        if (skillId == "rage_control_mastery") return "Rage canalisée";
        if (skillId == "battle_order_mastery") return "Voix de bataille";
        if (skillId == "breath_totem_mastery") return "Ancrage du souffle";
        if (skillId == "workshop_bomb_mastery" || skillId == "workshop_bomber") return "Bricolage explosif";
        if (skillId == "steel_prayer_mastery") return "Foi d'acier";
        if (skillId == "inner_mantra_mastery") return "Souffle intérieur";
        if (skillId == "trick_image_mastery") return "Angle trompeur";
        if (skillId == "summoning_link_mastery") return "Lien stabilisé";
        if (skillId == "blood_pact_mastery") return "Sang discipliné";
        if (skillId == "field_remedy_mastery") return "Gestes de terrain";
        if (skillId == "elemental_blade_mastery") return "Maîtrise élémentaire";
        if (skillId == "protective_circle_mastery") return "Garde circulaire";
        if (skillId == "binding_shot_mastery") return "Trait entravant";
        if (skillId == "inspiring_chant_mastery") return "Voix revigorante";
        if (skillId == "beast_instinct_mastery") return "Instinct canalisé";
        if (skillId == "blade_dance_mastery") return "Rythme de lame";
        if (skillId == "learned_arcane_mark") return "Marque élémentaire étudiée";
        if (skillId == "learned_arcane_binding") return "Entrave arcanique étudiée";
        if (skillId == "learned_elemental_ward") return "Voile élémentaire étudié";
        if (skillId == "learned_resistance_rift") return "Faille de résistance étudiée";
        if (skillId == "learned_frost_needle") return "Aiguille de givre étudiée";
        if (skillId == "learned_mana_suture") return "Suture de mana étudiée";
        if (skillId == "learned_occult_bramble") return "Ronces occultes étudiées";
        if (skillId == "church_oath_shield") return "Serment du Bouclier";
        if (skillId == "church_oath_blood") return "Serment du Sang";
        if (skillId == "church_oath_hunter") return "Serment du Chasseur";
        if (skillId == "church_oath_king") return "Serment du Roi";
        if (skillId == "church_oath_guarded_flame") return "Serment de la Flamme gardée";
        if (skillId == "church_oath_shadow") return "Serment des Ombres franches";
        if (skillId == "church_oath_pilgrim") return "Serment du Pèlerin";
        if (skillId == "church_oath_memory") return "Serment de Mémoire";
        if (skillId == "church_oath_silence") return "Serment du Silence";
        if (skillId == "church_oath_open_sky") return "Serment du Ciel ouvert";
        if (skillId == "church_oath_roots") return "Serment des Racines";
        if (skillId == "church_oath_broken_mirror") return "Serment du Miroir brisé";
        if (skillId == "church_oath_witness") return "Serment du Témoin";
        if (skillId == "church_oath_scars") return "Serment des Cicatrices";
        if (skillId == "church_oath_legacy") return "Serment de l'Héritage";
        if (skillId == "church_oath_bound_forge") return "Serment de la Forge liée";
        if (skillId == "church_oath_bonds") return "Serment des Liens";
        if (skillId == "church_oath_rivals") return "Serment des Rivaux";
        if (skillId == "church_oath_unstable_fate") return "Serment du Destin instable";
        if (skillId == "church_oath_broken_trace") return "Trace de serment rompu";
        return skillId;
    }

    std::string playerSkillDescription(const std::string& skillId)
    {
        if (skillId == "night_vision") return "Passif racial : tes yeux s'habituent mieux aux zones sombres et aux détails cachés.";
        if (skillId == "temperature_adaptation") return "Passif racial : meilleure tolérance aux zones de température pénible, sans remplacer une vraie tenue de survie.";
        if (skillId == "minor_fire_resistance") return "Passif racial : petite résistance au feu/chaleur. Elle aide aussi contre les brûlures de combat, mais reste insuffisante face aux zones extrêmes.";
        if (skillId == "infernal_fire_resistance") return "Passif racial : forte résistance aux flammes ordinaires et à la chaleur, efficace contre les brûlures de combat et les biomes chauds non extrêmes.";
        if (skillId == "minor_cold_resistance") return "Passif racial : meilleure tenue au froid et au givre, utile en exploration froide et contre les ralentissements de froid.";
        if (skillId == "fire_vulnerability") return "Faiblesse raciale : les flammes s'accrochent plus facilement. Les races ailées/fragiles doivent se méfier des brûlures.";
        if (skillId == "semi_wolf_tracking") return "Passif semi-humain : flair de meute, très utile sur routes sauvages, forêt et recherches de traces.";
        if (skillId == "semi_fox_cunning") return "Passif semi-humain : ruse de renard, utile pour les détours secondaires et les sorties de nuit prudentes.";
        if (skillId == "semi_dog_loyal_scent") return "Passif semi-humain : flair loyal, bon pour escortes, protection et recherche de personnes.";
        if (skillId == "semi_cat_reflexes") return "Passif semi-humain : réflexes félins, vision et appuis plus sûrs quand la nuit complique tout.";
        if (skillId == "semi_lizard_scales") return "Passif semi-humain : écailles tempérées, chaleur un peu mieux supportée mais froid plus pénible.";
        if (skillId == "semi_bird_open_sky") return "Passif semi-humain : vue des hauteurs, excellente lecture des zones ouvertes mais vulnérabilité aux flammes.";
        if (skillId == "survival_breath") return "Passif : petit instinct de survie débloqué par l'expérience. Ton corps apprend à encaisser l'instant où tout bascule.";
        if (skillId == "ranger_eye") return "Passif : meilleure lecture des ouvertures à distance, gagné en utilisant régulièrement un arc.";
        if (skillId == "steady_guard") return "Passif de chevalier : posture plus stable lorsque tu tiens ta ligne.";
        if (skillId == "living_rampart") return "Passif de colosse : présence défensive plus lourde.";
        if (skillId == "sure_hand") return "Passif d'artisan/forgeron : meilleure maîtrise des réparations et gestes précis.";
        if (skillId == "careful_dosage") return "Passif d'alchimiste : meilleure lecture des potions et catalyseurs.";
        if (skillId == "chain_execution") return "Actif, recharge 3 tours : après plusieurs exécutions à la dague, tu cherches l'ouverture pour enchaîner si une cible tombe.";
        if (skillId == "reflex_counter") return "Actif, recharge 4 tours : contre brutal appris en combattant aux mains nues.";
        if (skillId == "cautious_channeling") return "Actif, recharge 3 tours : canalisation plus sûre après usage répété du bâton.";
        if (skillId == "shadow_step") return "Actif d'assassin, recharge 4 tours : déplacement court pour mieux choisir l'ouverture.";
        if (skillId == "arcane_impulse") return "Actif de mage, recharge 4 tours : poussée magique courte.";
        if (skillId == "prepared_volley") return "Actif d'artificier, recharge 5 tours : salve préparée de projectiles et outils.";
        if (skillId == "tracking_mark") return "Actif de rôdeur, recharge 3 tours : marque une cible après lecture des traces.";
        if (skillId == "blade_discipline") return "Passif : habitude des lignes propres à l'épée, gagnée à force de combattre avec une lame.";
        if (skillId == "splitting_blow") return "Actif, recharge 4 tours : coup ample appris en ouvrant les défenses avec une hache.";
        if (skillId == "armor_crack") return "Actif, recharge 4 tours : frappe lourde qui cherche les points faibles d'une garde.";
        if (skillId == "reach_control") return "Passif : meilleure lecture des distances, gagnée en maniant régulièrement une lance.";
        if (skillId == "battle_instinct") return "Passif de carrière : après plusieurs combats lancés, les premiers gestes deviennent plus propres et gagnent un léger bonus stable.";
        if (skillId == "veteran_rhythm") return "Passif de carrière : les victoires répétées améliorent la finition des coups critiques.";
        if (skillId == "scar_tissue") return "Passif de survie : les défaites et morts apprennent au corps à rester dangereux même quand tout va mal.";
        if (skillId == "escape_reader") return "Passif de prudence : les fuites réussies apprennent à mieux lire les distances et les sorties.";
        if (skillId == "boss_memory") return "Passif de boss : chaque vraie victoire majeure laisse une mémoire de rythme contre les ennemis importants.";
        if (skillId == "armor_habit") return "Passif d'équipement : après plusieurs combats avec une armure portée, ton corps connaît mieux son poids et ses angles morts.";
        if (skillId == "loadout_memory") return "Passif d'équipement : l'habitude d'un ensemble arme/armure rend les transitions plus naturelles.";
        if (skillId == "field_maintenance") return "Passif de durabilité : tu repères plus vite les signes d'usure inquiétants sur l'équipement utilisé.";
        if (skillId == "curse_anchor_awareness") return "Passif de survie occulte : les malédictions de boss restent sur le personnage hors combat jusqu'à ce que la source soit rebattue ; ton instinct commence à les reconnaître.";
        if (skillId == "material_sorting_habit") return "Passif d'inventaire : à force de garder des composants variés, tu repères mieux ce qui doit être vendu, gardé ou trié.";
        if (skillId == "equipment_identity") return "Passif de profil : avec trois titres équipés et un équipement connu, ton identité devient plus lisible dans les dialogues sans donner de gros bonus.";
        if (skillId == "guild_route_memory") return "Passif de guilde : les contrats répétés rendent les routes, comptoirs et délais plus faciles à lire.";
        if (skillId == "bestiary_family_reader") return "Passif de bestiaire : après beaucoup de monstres vaincus, tu reconnais mieux les familles sans découvrir gratuitement leurs faiblesses.";
        if (skillId == "weapon_care_habit") return "Passif de durabilité : tu entretiens mieux l'arme que tu utilises vraiment ; très faible aide aux réparations, surtout en préparation de terrain.";
        if (skillId == "armor_fit_memory") return "Passif d'équipement : tu repères mieux les sangles, ouvertures et frottements d'une armure portée longtemps, surtout avec races/sous-races.";
        if (skillId == "cautious_pathing") return "Passif d'exploration : après routes, fuites et retours difficiles, tu lis un peu mieux les détours sans transformer ça en téléportation gratuite.";
        if (skillId == "threat_route_planner") return "Passif de guilde : les chasses et explorations répétées t'apprennent à préparer la route avant le combat, pas seulement après la blessure.";
        if (skillId == "environmental_resilience") return "Passif de synergie : plusieurs habitudes de température, résistance et terrain se complètent. Les biomes pénibles deviennent un peu plus lisibles, surtout avec un équipement adapté.";
        if (skillId == "temperature_drill") return "Passif d'entraînement : transitions chaleur/froid travaillées au stand. Il compte pour les synergies d'environnement sans donner d'immunité gratuite.";
        if (skillId == "terrain_drill") return "Passif d'entraînement : appuis, pente, boue et souffle travaillés au stand. Il complète les passifs de route et de résistance.";
        if (skillId == "footwork_drill") return "Passif d'entraînement : les attaques rapides et les replacemements ont un peu plus de sens, surtout pour survivre au mauvais timing.";
        if (skillId == "combat_observation") return "Actif général, recharge 3 tours : lit l'état de la cible et crée une faille courte au lieu de seulement taper plus fort.";
        if (skillId == "field_observer") return "Passif de terrain : après plusieurs observations hors combat, tu lis mieux les lieux, doublons de quête, sorties groupées et indices avant de partir.";
        if (skillId == "contract_reader") return "Passif de comptoir : les mandats, groupes et profils mercenaires deviennent plus lisibles avant de payer ou publier une demande.";
        if (skillId == "guard_breaker") return "Passif tactique : après plusieurs brise-gardes, tes contrôles courts créent des failles plus stables sans devenir une attaque gratuite.";
        if (skillId == "support_rhythm") return "Passif tactique : après plusieurs tours de couverture, tenir la ligne devient un vrai geste de soutien, avec une garde plus propre.";
        if (skillId == "formation_breaker") return "Passif tactique : après plusieurs ruptures de formation, tu lis mieux les lignes ennemies, surtout quand elles protègent ou coordonnent leurs coups.";
        if (skillId == "signal_cut_awareness" || skillId == "signal_cutter") return "Passif de maîtrise : après trois vrais Coupe-signal, tu repères mieux le souffle, le geste ou le regard qui précède l'appel. Il n'active pas Coupe-signal automatiquement.";
        if (skillId == "body_reader") return "Passif tactique : après trois failles de corps vraiment cherchées, tu lis mieux jointures, noyaux, ventres mous et points d'appui.";
        if (skillId == "status_conductor") return "Passif tactique : après trois chaînes d'états forcées, tu déclenches mieux les venins, brûlures, chocs et failles déjà posés.";
        if (skillId == "prey_marker") return "Passif tactique : après trois proies vraiment marquées, tu choisis plus vite le bon angle pour préparer le prochain impact.";
        if (skillId == "controlled_retreat") return "Passif tactique : après trois retraits contrôlés, ton sens du recul t'aide à reculer sans offrir une chasse gratuite.";
        if (skillId == "threat_reader") return "Passif tactique : après trois menaces vraiment lues, tu repères mieux les soigneurs, chefs, boucliers, rageux et sources instables.";
        if (skillId == "low_sweeper") return "Passif tactique : après trois balayages bas utiles, ta lecture des appuis coupe mieux les trajectoires des petites cibles et profils nerveux.";
        if (skillId == "focus_breaker") return "Passif tactique : après trois concentrations brisées, tu perturbes mieux les soigneurs, lanceurs et porteurs d'effets dangereux.";
        if (skillId == "reach_breaker") return "Passif tactique : après trois allonges rompues, tu fermes mieux la distance contre tireurs, lanciers, duellistes et lanceurs trop bien placés.";
        if (skillId == "shield_piercer") return "Passif tactique : après trois remparts percés, tu lis mieux boucliers, carapaces, plaques et postures verrouillées.";
        if (skillId == "anchor_breaker") return "Passif tactique : après trois ancrages rompus, tu perturbes mieux les marques occultes, morts-vivants, spectres et énergies instables.";
        if (skillId == "rally_breath") return "Passif tactique : après trois souffles de ralliement, ton souffle rallié aide tes tours de reprise donnent une garde et un élan plus stables.";
        if (skillId == "momentum_striker") return "Passif tactique : après trois frappes d'élan, ton élan canalisé aide à transformer une attaque réussie en reprise offensive courte.";
        if (skillId == "emergency_warder") return "Passif tactique : après trois voiles d'urgence, ton réflexe de voile rend les protections courtes plus naturelles quand la ligne se referme.";
        if (skillId == "wild_spark") return "Passif tactique : après trois étincelles instables, ton instabilité apprivoisée évite que les gestes magiques ou alchimiques dérapent trop souvent.";
        if (skillId == "war_cry_caller") return "Passif tactique : après trois cris de guerre, ta voix de guerre transforme mieux le souffle en rythme, garde et élan pour la mêlée.";
        if (skillId == "arc_sweep") return "Actif de combat : attaque large choisie, utile pour occuper plusieurs ennemis sans devenir automatique.";
        if (skillId == "vigor_sign") return "Actif de soutien : geste volontaire de reprise, soin lent et purge légère.";
        if (skillId == "rogue_feint") return "Actif de duel sale : faux angle, petite entaille et ouverture. À choisir, avec rythme et risque.";
        if (skillId == "battle_suture") return "Actif de soutien : stabilise sous pression. Une suture ne se lance jamais seule.";
        if (skillId == "arcane_channel") return "Actif de canalisation : prépare volontairement un geste magique ou alchimique.";
        if (skillId == "rampart_oath") return "Actif de front : attire les regards, durcit la garde et prend le tour.";
        if (skillId == "stopping_shot") return "Actif de tireur : stoppe une approche par un projectile ou un geste précis.";
        if (skillId == "mastered_rage") return "Actif de front : transforme une rage choisie en frappe tenue, sans devenir un réflexe passif.";
        if (skillId == "battle_order") return "Actif de commandement : ordre volontaire qui règle rythme, précision et tenue de ligne.";
        if (skillId == "breath_totem") return "Actif de nature : pose un repère de souffle et de protection courte.";
        if (skillId == "workshop_bomb") return "Actif d'artisan : projectile bricolé, impact réel et instabilité contrôlée.";
        if (skillId == "steel_prayer") return "Actif sacré : prière choisie pour protection, souffle et purification.";
        if (skillId == "inner_mantra") return "Actif martial : souffle volontaire, précision et garde.";
        if (skillId == "trick_image") return "Actif d'illusion : fausse ouverture choisie pour reprendre l'angle.";
        if (skillId == "summoning_link") return "Actif de lien : ancre volontairement une présence ou invocation autour de la ligne.";
        if (skillId == "blood_pact") return "Actif risqué : paie un prix en PV pour obtenir élan, morsure et souffle noir.";
        if (skillId == "field_remedy") return "Actif de terrain : soin improvisé, dosage et stabilisation choisis.";
        if (skillId == "elemental_blade") return "Actif élémentaire : charge l'arme ou la main puis frappe avec feu, givre, choc/faille ou poison.";
        if (skillId == "protective_circle") return "Actif protecteur : trace une limite de garde, voile élémentaire et régénération courte.";
        if (skillId == "binding_shot") return "Actif de contrôle : tir qui gêne appuis, ailes ou fuites.";
        if (skillId == "inspiring_chant") return "Actif de présence : chant choisi pour précision, puissance courte, régénération et purge légère.";
        if (skillId == "beast_instinct") return "Actif sauvage : assaut choisi qui suit plaie, faiblesse ou flair vivant.";
        if (skillId == "blade_dance") return "Actif de duel : enchaînement de 2 coupes avec une arme légère, 3 coupes si deux armes légères sont disponibles. Ce n'est jamais un passif.";
        if (skillId == "shadow_step_mastery" || skillId == "shadow_stepper") return "Passif de maîtrise : après trois vrais Pas de l'ombre, tes déplacements courts donnent un angle plus sûr. Le Pas de l'ombre reste une technique active à choisir.";
        if (skillId == "arc_sweep_mastery" || skillId == "arc_sweeper") return "Passif de maîtrise : après trois vrais Coups en arc, tu occupes mieux plusieurs adversaires avec une frappe large. La technique reste active.";
        if (skillId == "vigor_sign_mastery") return "Passif de maîtrise : après trois vrais Signes de vigueur, tes gestes de reprise stabilisent mieux souffle et mauvais états légers. Le signe reste une action choisie.";
        if (skillId == "rogue_feint_mastery" || skillId == "rogue_feinter") return "Passif de maîtrise agile : après trois vraies Feintes sournoises, les ouvertures sales restent plus lisibles. La feinte reste une technique active.";
        if (skillId == "battle_suture_mastery") return "Passif de maîtrise soutien : après trois vraies Sutures de bataille, tu stabilises mieux sous pression. La suture reste une technique active.";
        if (skillId == "arcane_channel_mastery" || skillId == "arcane_channeler") return "Passif de maîtrise arcanique : après trois vraies Canalisations arcaniques, le flux tremble moins avant le prochain geste. La canalisation reste active.";
        if (skillId == "rampart_oath_mastery") return "Passif de maîtrise front : après trois vrais Serments du rempart, attirer les regards devient plus stable. Le serment reste une technique active.";
        if (skillId == "stopping_shot_mastery") return "Passif de maîtrise pisteur : après trois vrais Tirs d'arrêt, tu coupes mieux une approche. Le tir reste une technique active.";
        if (skillId == "rage_control_mastery") return "Passif de maîtrise de front : après trois vraies Rages maîtrisées, tu transformes mieux blessure et élan en frappe tenue. La rage reste une technique active.";
        if (skillId == "battle_order_mastery") return "Passif de maîtrise commandement : après trois vrais Ordres de bataille, ta voix cale mieux tempo, garde et reprise. L'ordre reste une technique active.";
        if (skillId == "breath_totem_mastery") return "Passif de maîtrise nature : après trois vrais Totems de souffle, tes repères vivants stabilisent mieux récupération et appuis. Le totem reste une technique active.";
        if (skillId == "workshop_bomb_mastery" || skillId == "workshop_bomber") return "Passif de maîtrise atelier : après trois vraies Bombes d'atelier, tes bricolages instables ratent moins leur effet utile. La bombe reste une technique active.";
        if (skillId == "steel_prayer_mastery") return "Passif de maîtrise sacrée : après trois vraies Prières d'acier, tes protections courtes gardent mieux souffle et purification. La prière reste une technique active.";
        if (skillId == "inner_mantra_mastery") return "Passif de maîtrise martiale : après trois vrais Mantras intérieurs, ton souffle tient mieux garde, précision et mauvais rythmes. Le mantra reste actif.";
        if (skillId == "trick_image_mastery") return "Passif de maîtrise illusion/ruse : après trois vraies Images trompeuses, tes fausses ouvertures aident mieux à reprendre l'angle. L'image reste active.";
        if (skillId == "summoning_link_mastery") return "Passif de maîtrise invocation : après trois vrais Liens d'invocation, les présences liées soutiennent mieux garde et pression. Le lien reste actif.";
        if (skillId == "blood_pact_mastery") return "Passif de maîtrise sang/pacte : après trois vrais Pactes sanguins, tu paies moins mal la douleur et transformes mieux le prix en élan. Le pacte reste actif.";
        if (skillId == "field_remedy_mastery") return "Passif de maîtrise soin/atelier : après trois vrais Remèdes de fortune, tes soins improvisés stabilisent mieux plaies et mauvais états. Le remède reste actif.";
        if (skillId == "elemental_blade_mastery") return "Passif de maîtrise élémentaire : après trois vraies Lames élémentaires, les impacts colorés accrochent mieux brûlure, choc, givre ou venin. Lame élémentaire reste une technique active.";
        if (skillId == "protective_circle_mastery") return "Passif de maîtrise protectrice : après trois vrais Cercles protecteurs, tes limites de garde tiennent mieux souffle et éléments. Cercle protecteur reste une technique active.";
        if (skillId == "binding_shot_mastery") return "Passif de maîtrise pisteur : après trois vraies Flèches entravantes, tes tirs gênent mieux appuis, ailes et fuites. Flèche entravante reste une technique active.";
        if (skillId == "inspiring_chant_mastery") return "Passif de maîtrise barde/commandement : après trois vrais Chants revigorants, ta voix remet mieux le rythme et chasse les mauvais appuis. Chant revigorant reste une technique active.";
        if (skillId == "beast_instinct_mastery") return "Passif de maîtrise sauvage : après trois vrais Instincts de bête, tes assauts suivent mieux plaies et faiblesses vivantes. Instinct de bête reste une technique active.";
        if (skillId == "blade_dance_mastery") return "Passif de maîtrise duel : après trois vraies Danses de lame, tes enchaînements de deux ou trois coupes laissent un angle plus propre. Danse de lame reste une technique active avec choix, armes légères et rythme de combat.";
        if (skillId == "learned_arcane_mark") return "Sort appris par étude : marque élémentaire simple, réservée aux vrais canalisateurs.";
        if (skillId == "learned_arcane_binding") return "Sort appris par grimoire : entrave la cible sans exister forcément en parchemin commun.";
        if (skillId == "learned_elemental_ward") return "Sort appris par grimoire : voile défensif utilisable avec un catalyseur correct.";
        if (skillId == "learned_resistance_rift") return "Sort rare appris par grimoire : ouvre une faille de résistance après préparation.";
        if (skillId == "learned_frost_needle") return "Sort de bibliothèque sans parchemin courant : givre précis, fragile et exigeant.";
        if (skillId == "learned_mana_suture") return "Sort appris par grimoire, sans parchemin courant : referme lentement les blessures pendant quelques tours.";
        if (skillId == "learned_occult_bramble") return "Sort appris par grimoire, sans parchemin courant : entrave la cible avec ronces, poison et fatigue magique.";
        if (skillId == "church_oath_shield") return "Serment d'église : promesse de tenir la ligne, protéger les alliés et accepter le prix d'un abandon.";
        if (skillId == "church_oath_blood") return "Serment d'église : promesse de payer un prix réel pour un élan réel, au risque de rendre les soins plus délicats.";
        if (skillId == "church_oath_hunter") return "Serment d'église : promesse de comprendre la proie par la piste et l'observation avant de réclamer l'avantage.";
        if (skillId == "church_oath_king") return "Serment d'église : promesse de responsabilité visible envers les alliés, les ordres donnés et la réputation acquise.";
        if (skillId == "church_oath_guarded_flame") return "Serment d'église : garder une flamme qui protège avant de brûler, liée à la chaleur, au courage et à la maîtrise du feu.";
        if (skillId == "church_oath_shadow") return "Serment d'église : marcher dans l'ombre sans transformer discrétion, esquive et ruse en trahison.";
        if (skillId == "church_oath_pilgrim") return "Serment d'église : respecter routes, relais et villages traversés, même lorsque la fatigue ou la réputation compliquent le voyage.";
        if (skillId == "church_oath_memory") return "Serment d'église : porter les traces, témoins et morts dans le récit afin que leur mémoire ne disparaisse pas.";
        if (skillId == "church_oath_silence") return "Serment d'église : garder assez de calme et de concentration pour lire peur, illusions et provocations.";
        if (skillId == "church_oath_open_sky") return "Serment d'église : apprendre à répondre aux ennemis en Vol par angle, patience, allonge ou tir. Effet actuel : petite chance de trouver une ouverture avec arme courte.";
        if (skillId == "church_oath_roots") return "Serment d'église : garder ses appuis face aux fils, racines et lianes. Effet actuel : chance de briser une entrave au début du tour.";
        if (skillId == "church_oath_broken_mirror") return "Serment d'église : lire les reflets sans prétendre deviner gratuitement. Effet actuel : réduit le risque de frapper une illusion.";
        if (skillId == "church_oath_witness") return "Serment d'église : une information doit venir d'un témoin, d'une trace ou d'une observation réelle. Effet actuel : aide les contre-lectures et nourrit les rumeurs logiques.";
        if (skillId == "church_oath_scars") return "Serment d'église : transformer une blessure vécue en tenue réelle. Effet actuel : petite aide sous pression, sans chercher la douleur gratuitement.";
        if (skillId == "church_oath_legacy") return "Serment d'église : préparer héritage, tombes, objets avec mémoire et traces Mortel/Léthal. Effet actuel : aide rare si une trace existe déjà.";
        if (skillId == "church_oath_bound_forge") return "Serment d'église : lier une arme ou armure à ce qu'elle a vécu. Effet actuel : une arme cohérente peut laisser une trace d'objet avec mémoire.";
        if (skillId == "church_oath_bonds") return "Serment d'église : porter les liens du groupe. Les alliés, ordres ou traces cohérentes peuvent renforcer légèrement la pression collective.";
        if (skillId == "church_oath_rivals") return "Serment d'église : ne pas laisser une fuite ou une humiliation devenir anonyme. Effet actuel : les ennemis qui survivent peuvent laisser une trace de rival plus nette.";
        if (skillId == "church_oath_unstable_fate") return "Serment d'église : accepter un destin qui bouge selon les actes réels. Effet actuel : rare oscillation en combat, surtout quand une vraie trace existe déjà.";
        if (skillId == "church_oath_broken_trace") return "Trace d'église : au moins un serment a été rompu. Ce n'est pas un malus direct, mais le registre en garde la mémoire.";
        return "Compétence instable : son usage reste difficile à canaliser.";
    }
}

void Player::normalizeSkillLoadout()
{
    auto uniqueUnlocked = [](std::vector<std::string>& values)
    {
        std::vector<std::string> clean;
        for (const std::string& value : values)
        {
            if (!value.empty() && !containsSkillId(clean, value))
            {
                clean.push_back(value);
            }
        }
        values = clean;
    };

    uniqueUnlocked(unlockedPassiveSkills);
    uniqueUnlocked(unlockedActiveSkills);
    uniqueUnlocked(enabledPassiveSkills);
    uniqueUnlocked(equippedActiveSkills);

    // Church oaths are persistent contracts/statuses, not loadout passives.
    enabledPassiveSkills.erase(
        std::remove_if(enabledPassiveSkills.begin(), enabledPassiveSkills.end(), [](const std::string& skillId) {
            return isChurchOathSkillId(skillId);
        }),
        enabledPassiveSkills.end()
    );

    enabledPassiveSkills.erase(
        std::remove_if(enabledPassiveSkills.begin(), enabledPassiveSkills.end(), [&](const std::string& skillId) {
            return !containsSkillId(unlockedPassiveSkills, skillId);
        }),
        enabledPassiveSkills.end()
    );

    equippedActiveSkills.erase(
        std::remove_if(equippedActiveSkills.begin(), equippedActiveSkills.end(), [&](const std::string& skillId) {
            return !containsSkillId(unlockedActiveSkills, skillId);
        }),
        equippedActiveSkills.end()
    );

    for (const std::string& skillId : unlockedPassiveSkills)
    {
        if (isChurchOathSkillId(skillId)) continue;
        if (static_cast<int>(enabledPassiveSkills.size()) >= MAX_ENABLED_PASSIVE_SKILLS) break;
        if (!containsSkillId(enabledPassiveSkills, skillId)) enabledPassiveSkills.push_back(skillId);
    }

    for (const std::string& skillId : unlockedActiveSkills)
    {
        if (static_cast<int>(equippedActiveSkills.size()) >= MAX_EQUIPPED_ACTIVE_SKILLS) break;
        if (!containsSkillId(equippedActiveSkills, skillId)) equippedActiveSkills.push_back(skillId);
    }

    if (static_cast<int>(enabledPassiveSkills.size()) > MAX_ENABLED_PASSIVE_SKILLS)
    {
        enabledPassiveSkills.resize(MAX_ENABLED_PASSIVE_SKILLS);
    }
    if (static_cast<int>(equippedActiveSkills.size()) > MAX_EQUIPPED_ACTIVE_SKILLS)
    {
        equippedActiveSkills.resize(MAX_EQUIPPED_ACTIVE_SKILLS);
    }
}

// EN: getUnlockedPassiveSkills declares or implements a focused behavior used by this module.
// FR: getUnlockedPassiveSkills déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getUnlockedPassiveSkills() const
{
    return unlockedPassiveSkills;
}

// EN: getUnlockedActiveSkills declares or implements a focused behavior used by this module.
// FR: getUnlockedActiveSkills déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getUnlockedActiveSkills() const
{
    return unlockedActiveSkills;
}

const std::vector<std::string>& Player::getEnabledPassiveSkills() const
{
    return enabledPassiveSkills;
}

const std::vector<std::string>& Player::getEquippedActiveSkills() const
{
    return equippedActiveSkills;
}

bool Player::isPassiveSkillUnlocked(const std::string& skillId) const
{
    return containsSkillId(unlockedPassiveSkills, skillId);
}

bool Player::isActiveSkillUnlocked(const std::string& skillId) const
{
    return containsSkillId(unlockedActiveSkills, skillId);
}

bool Player::isPassiveSkillEnabled(const std::string& skillId) const
{
    return isPassiveSkillUnlocked(skillId)
        && containsSkillId(enabledPassiveSkills, skillId);
}

bool Player::isActiveSkillEquipped(const std::string& skillId) const
{
    return isActiveSkillUnlocked(skillId)
        && containsSkillId(equippedActiveSkills, skillId);
}

bool Player::enablePassiveSkill(const std::string& skillId)
{
    if (isChurchOathSkillId(skillId))
    {
        return false;
    }
    if (!isPassiveSkillUnlocked(skillId) || containsSkillId(enabledPassiveSkills, skillId))
    {
        return false;
    }
    if (static_cast<int>(enabledPassiveSkills.size()) >= MAX_ENABLED_PASSIVE_SKILLS)
    {
        return false;
    }
    enabledPassiveSkills.push_back(skillId);
    return true;
}

bool Player::disablePassiveSkill(const std::string& skillId)
{
    if (isChurchOathSkillId(skillId))
    {
        return false;
    }
    auto it = std::find(enabledPassiveSkills.begin(), enabledPassiveSkills.end(), skillId);
    if (it == enabledPassiveSkills.end())
    {
        return false;
    }
    enabledPassiveSkills.erase(it);
    return true;
}

bool Player::equipActiveSkill(const std::string& skillId)
{
    if (!isActiveSkillUnlocked(skillId) || containsSkillId(equippedActiveSkills, skillId))
    {
        return false;
    }
    if (static_cast<int>(equippedActiveSkills.size()) >= MAX_EQUIPPED_ACTIVE_SKILLS)
    {
        return false;
    }
    equippedActiveSkills.push_back(skillId);
    return true;
}

bool Player::unequipActiveSkill(const std::string& skillId)
{
    auto it = std::find(equippedActiveSkills.begin(), equippedActiveSkills.end(), skillId);
    if (it == equippedActiveSkills.end())
    {
        return false;
    }
    equippedActiveSkills.erase(it);
    return true;
}

int Player::getSkillUseCount(const std::string& category, const std::string& key) const
{
    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (record.category == category && record.key == key)
        {
            return std::max(0, record.count);
        }
    }
    return 0;
}

int Player::getActiveSkillMasteryLevel(const std::string& actionKey) const
{
    return masteryLevelFromSuccessfulUses(getSkillUseCount("actions_tactiques_combat", actionKey), activeMasteryMaxLevelForAction(actionKey));
}

std::string Player::getActiveSkillMasteryLabel(const std::string& actionKey) const
{
    const int masteryLevel = getActiveSkillMasteryLevel(actionKey);
    const int maxLevel = activeMasteryMaxLevelForAction(actionKey);
    return masteryRankLabel(masteryLevel, maxLevel) + " " + std::to_string(masteryLevel) + "/" + std::to_string(maxLevel);
}

std::string Player::getActiveSkillMasteryProgressHint(const std::string& actionKey) const
{
    return masteryProgressHintFromUses(getSkillUseCount("actions_tactiques_combat", actionKey), activeMasteryMaxLevelForAction(actionKey));
}

std::string Player::getActiveSkillMasteryEffectHint(const std::string& actionKey) const
{
    return masteryEffectHint(getActiveSkillMasteryLevel(actionKey), activeMasteryMaxLevelForAction(actionKey));
}

int Player::getPassiveMasteryLevelFromAction(const std::string& actionKey) const
{
    return passiveMasteryLevelFromSuccessfulUses(getSkillUseCount("actions_tactiques_combat", actionKey), passiveMasteryMaxLevelForAction(actionKey));
}

std::string Player::getPassiveMasteryLabelFromAction(const std::string& actionKey) const
{
    const int passiveLevel = getPassiveMasteryLevelFromAction(actionKey);
    const int maxLevel = passiveMasteryMaxLevelForAction(actionKey);
    return passiveMasteryRankLabel(passiveLevel, maxLevel) + " " + std::to_string(passiveLevel) + "/" + std::to_string(maxLevel);
}

std::string Player::getPassiveMasteryProgressHintFromAction(const std::string& actionKey) const
{
    return passiveMasteryProgressHintFromUses(getSkillUseCount("actions_tactiques_combat", actionKey), passiveMasteryMaxLevelForAction(actionKey));
}

std::string Player::getPassiveMasteryEffectHintFromAction(const std::string& actionKey) const
{
    return passiveMasteryEffectHint(getPassiveMasteryLevelFromAction(actionKey), passiveMasteryMaxLevelForAction(actionKey));
}

// EN: getDaggerKillProgress declares or implements a focused behavior used by this module.
// FR: getDaggerKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getDaggerKillProgress() const
{
    return daggerKillProgress;
}

// EN: getBowKillProgress declares or implements a focused behavior used by this module.
// FR: getBowKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getBowKillProgress() const
{
    return bowKillProgress;
}

// EN: getBareHandKillProgress declares or implements a focused behavior used by this module.
// FR: getBareHandKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getBareHandKillProgress() const
{
    return bareHandKillProgress;
}

// EN: getStaffKillProgress declares or implements a focused behavior used by this module.
// FR: getStaffKillProgress déclare ou implémente un comportement précis utilisé par ce module.
int Player::getStaffKillProgress() const
{
    return staffKillProgress;
}

int Player::getSwordKillProgress() const
{
    return swordKillProgress;
}

int Player::getAxeKillProgress() const
{
    return axeKillProgress;
}

int Player::getHammerKillProgress() const
{
    return hammerKillProgress;
}

int Player::getSpearKillProgress() const
{
    return spearKillProgress;
}

// EN: hasPassiveSkill declares or implements a focused behavior used by this module.
// FR: hasPassiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasPassiveSkill(const std::string& skillId) const
{
    if (skillId == "church_oath_broken_trace")
    {
        return isPassiveSkillUnlocked(skillId);
    }
    if (isChurchOathContractId(skillId))
    {
        return isActiveChurchOathContract(*this, skillId);
    }
    return isPassiveSkillEnabled(skillId);
}

// EN: hasActiveSkill declares or implements a focused behavior used by this module.
// FR: hasActiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasActiveSkill(const std::string& skillId) const
{
    return isActiveSkillEquipped(skillId);
}

// EN: unlockPassiveSkill declares or implements a focused behavior used by this module.
// FR: unlockPassiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::unlockPassiveSkill(const std::string& skillId, const std::string& skillName, bool silent)
{
    if (isPassiveSkillUnlocked(skillId))
    {
        return false;
    }

    unlockedPassiveSkills.push_back(skillId);
    const bool churchStatus = isChurchOathSkillId(skillId);
    const bool autoEnabled = !churchStatus && static_cast<int>(enabledPassiveSkills.size()) < MAX_ENABLED_PASSIVE_SKILLS;
    if (autoEnabled)
    {
        enabledPassiveSkills.push_back(skillId);
    }
    normalizeSkillLoadout();

    std::vector<std::string> lines = {
        churchStatus ? ("Nouveau statut d'église : " + skillName) : ("Nouvelle compétence passive : " + skillName),
        churchStatus ? "Ce statut suit son propre contrat et ne consomme aucun emplacement passif." : "Elle s'est développée à force de vivre, combattre et apprendre."
    };
    if (churchStatus)
    {
        lines.push_back("État : géré par le registre de l'église, pas par le loadout de compétences.");
    }
    else
    {
        lines.push_back(autoEnabled
            ? "État : activée dans les passifs équipés."
            : "État : connue, mais non activée car les 10 emplacements passifs sont déjà occupés.");
    }

    if (!silent)
    {
        MessageScreen::show(
            "NOUVELLE COMPÉTENCE PASSIVE",
            "player.skill.passive_unlocked",
            lines,
            false
        );
    }
    return true;
}

// EN: unlockActiveSkill declares or implements a focused behavior used by this module.
// FR: unlockActiveSkill déclare ou implémente un comportement précis utilisé par ce module.
bool Player::unlockActiveSkill(const std::string& skillId, const std::string& skillName, bool silent)
{
    if (isActiveSkillUnlocked(skillId))
    {
        return false;
    }

    unlockedActiveSkills.push_back(skillId);
    const bool autoEquipped = static_cast<int>(equippedActiveSkills.size()) < MAX_EQUIPPED_ACTIVE_SKILLS;
    if (autoEquipped)
    {
        equippedActiveSkills.push_back(skillId);
    }
    normalizeSkillLoadout();

    std::vector<std::string> lines = {
        "Nouvelle compétence active : " + skillName,
        "Elle reste une action choisie : elle ne devient pas automatique."
    };
    lines.push_back(autoEquipped
        ? "État : équipée dans les actifs utilisables."
        : "État : connue, mais déséquipée car les 10 emplacements actifs sont déjà occupés.");

    if (!silent)
    {
        MessageScreen::show(
            "NOUVELLE COMPÉTENCE ACTIVE",
            "player.skill.active_unlocked",
            lines,
            false
        );
    }
    return true;
}

// EN: recordGameplaySkillProgressForKills declares or implements a focused behavior used by this module.
// FR: recordGameplaySkillProgressForKills déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGameplaySkillProgressForKills(int amount)
{
    if (amount <= 0 || !hasEquippedWeapon())
    {
        return;
    }

    Weapon weapon = getEquippedWeapon();

    if (weapon.getType() == WeaponType::Dagger)
    {
        daggerKillProgress += amount;
        if (daggerKillProgress >= 5)
        {
            unlockActiveSkill("chain_execution", "Enchaînement");
        }
    }
    else if (weapon.getType() == WeaponType::Bow)
    {
        bowKillProgress += amount;
        if (bowKillProgress >= 8)
        {
            unlockPassiveSkill("ranger_eye", "Œil de rôdeur");
        }
    }
    else if (weapon.getType() == WeaponType::BareHands)
    {
        bareHandKillProgress += amount;
        if (bareHandKillProgress >= 10)
        {
            unlockActiveSkill("reflex_counter", "Contre réflexe");
        }
    }
    else if (weapon.getType() == WeaponType::Staff)
    {
        staffKillProgress += amount;
        if (staffKillProgress >= 6)
        {
            unlockActiveSkill("cautious_channeling", "Canalisation prudente");
        }
    }
    else if (weapon.getType() == WeaponType::Sword)
    {
        swordKillProgress += amount;
        if (swordKillProgress >= 7)
        {
            unlockPassiveSkill("blade_discipline", "Discipline de lame");
        }
    }
    else if (weapon.getType() == WeaponType::Axe)
    {
        axeKillProgress += amount;
        if (axeKillProgress >= 7)
        {
            unlockActiveSkill("splitting_blow", "Frappe fendue");
        }
    }
    else if (weapon.getType() == WeaponType::Hammer)
    {
        hammerKillProgress += amount;
        if (hammerKillProgress >= 7)
        {
            unlockActiveSkill("armor_crack", "Fracasse-garde");
        }
    }
    else if (weapon.getType() == WeaponType::Spear)
    {
        spearKillProgress += amount;
        if (spearKillProgress >= 7)
        {
            unlockPassiveSkill("reach_control", "Contrôle d'allonge");
        }
    }
}

// EN: refreshLevelAndIdentitySkills declares or implements a focused behavior used by this module.
// FR: refreshLevelAndIdentitySkills déclare ou implémente un comportement précis utilisé par ce module.
void Player::refreshLevelAndIdentitySkills(bool silent)
{
    CharacterRace currentRace = getRace();

    if (RaceCatalog::hasInnateNightVision(currentRace))
    {
        unlockPassiveSkill("night_vision", "Vision nocturne", silent);
    }

    if (currentRace == CharacterRace::Kitsune || currentRace == CharacterRace::HalfDragon)
    {
        unlockPassiveSkill("temperature_adaptation", "Adaptation aux températures", silent);
        unlockPassiveSkill("minor_fire_resistance", currentRace == CharacterRace::Kitsune ? "Résistance légère au feu kitsune" : "Résistance légère au feu draconique", silent);
    }

    if (currentRace == CharacterRace::Tiefling || currentRace == CharacterRace::Demon)
    {
        unlockPassiveSkill("infernal_fire_resistance", currentRace == CharacterRace::Tiefling ? "Résistance infernale tieffeline" : "Résistance infernale démoniaque", silent);
    }

    if (currentRace == CharacterRace::Dwarf || currentRace == CharacterRace::Vampire || currentRace == CharacterRace::HalfDragon)
    {
        unlockPassiveSkill("minor_cold_resistance", currentRace == CharacterRace::Vampire ? "Résistance froide vampirique" : "Résistance légère au froid", silent);
    }

    if (RaceCatalog::hasFireWeakness(currentRace))
    {
        std::string fireWeaknessName = "Faiblesse raciale au feu";
        if (currentRace == CharacterRace::Fairy) fireWeaknessName = "Faiblesse des ailes aux flammes";
        else if (currentRace == CharacterRace::Vampire) fireWeaknessName = "Faiblesse vampirique au feu";
        else if (currentRace == CharacterRace::SemiBird) fireWeaknessName = "Faiblesse des plumes aux flammes";
        unlockPassiveSkill("fire_vulnerability", fireWeaknessName, silent);
    }

    if (currentRace == CharacterRace::SemiWolf)
    {
        unlockPassiveSkill("semi_wolf_tracking", "Flair de meute", silent);
    }
    else if (currentRace == CharacterRace::SemiFox)
    {
        unlockPassiveSkill("semi_fox_cunning", "Flair rusé de renard", silent);
    }
    else if (currentRace == CharacterRace::SemiDog)
    {
        unlockPassiveSkill("semi_dog_loyal_scent", "Flair loyal", silent);
    }
    else if (currentRace == CharacterRace::SemiCat)
    {
        unlockPassiveSkill("semi_cat_reflexes", "Réflexes félins", silent);
    }
    else if (currentRace == CharacterRace::SemiLizard)
    {
        unlockPassiveSkill("semi_lizard_scales", "Écailles tempérées", silent);
        unlockPassiveSkill("minor_fire_resistance", "Résistance légère au feu lézard", silent);
    }
    else if (currentRace == CharacterRace::SemiBird)
    {
        unlockPassiveSkill("semi_bird_open_sky", "Vue des hauteurs", silent);
    }
    else if (currentRace == CharacterRace::Dwarf || currentRace == CharacterRace::Gnome)
    {
        unlockPassiveSkill("dwarven_mine_sense", currentRace == CharacterRace::Dwarf ? "Sens des galeries" : "Sens des galeries gnome", silent);
    }
    else if (currentRace == CharacterRace::Elf)
    {
        unlockPassiveSkill("elven_fine_perception", "Perception elfique", silent);
    }
    else if (currentRace == CharacterRace::Halfling)
    {
        unlockPassiveSkill("halfling_lucky_step", "Pas chanceux", silent);
    }
    else if (currentRace == CharacterRace::HalfDragon)
    {
        unlockPassiveSkill("dragon_weather_blood", "Sang draconique d'endurance", silent);
    }
    else if (currentRace == CharacterRace::Orc)
    {
        unlockPassiveSkill("orcish_forced_march", "Marche forcée orque", silent);
    }
    else if (currentRace == CharacterRace::Fairy || currentRace == CharacterRace::Aasimar)
    {
        unlockPassiveSkill("fairy_mana_sense", currentRace == CharacterRace::Fairy ? "Sens féerique du mana" : "Sens lumineux du mana", silent);
    }

    if (level >= 3)
    {
        unlockPassiveSkill("survival_breath", "Souffle de survie", silent);
    }

    std::string className = getType();

    if (level >= 5 && className == "Assassin")
    {
        unlockActiveSkill("shadow_step", "Pas de l'ombre", silent);
    }
    else if (level >= 5 && className == "Chevalier")
    {
        unlockPassiveSkill("steady_guard", "Garde stable", silent);
    }
    else if (level >= 5 && className == "Colosse")
    {
        unlockPassiveSkill("living_rampart", "Rempart vivant", silent);
    }
    else if (level >= 5 && className == "Mage")
    {
        unlockActiveSkill("arcane_impulse", "Élan arcanique", silent);
    }
    else if (level >= 5 && className == "Forgeron")
    {
        unlockPassiveSkill("sure_hand", "Main sûre", silent);
    }
    else if (level >= 5 && className == "Alchimiste")
    {
        unlockPassiveSkill("careful_dosage", "Dosage prudent", silent);
    }
    else if (level >= 5 && className == "Rôdeur")
    {
        unlockActiveSkill("tracking_mark", "Marque de pisteur", silent);
    }
    else if (level >= 5 && className == "Artificier")
    {
        unlockActiveSkill("prepared_volley", "Salve préparée", silent);
    }

    refreshCareerSkillProgress();
}

// EN: refreshCareerSkillProgress declares or implements a focused behavior used by this module.
// FR: refreshCareerSkillProgress déclare ou implémente un comportement précis utilisé par ce module.
void Player::refreshCareerSkillProgress()
{
    if (combatsStarted >= 8)
    {
        unlockPassiveSkill("battle_instinct", "Instinct de bataille");
    }

    if (victories >= 5)
    {
        unlockPassiveSkill("veteran_rhythm", "Rythme de vétéran");
    }

    if (deaths >= 2 || defeats >= 3)
    {
        unlockPassiveSkill("scar_tissue", "Peau des survivants");
    }

    if (escapes >= 2)
    {
        unlockPassiveSkill("escape_reader", "Lecture de fuite");
    }

    if (bossesKilled >= 1)
    {
        unlockPassiveSkill("boss_memory", "Mémoire de boss");
    }

    if (countRecentEquipmentUsageContaining(recentCombatEquipmentUsage, "Armure:") >= 5)
    {
        unlockPassiveSkill("armor_habit", "Habitude d'armure");
    }

    if (hasRecentFullLoadoutUsage(recentCombatEquipmentUsage, 5))
    {
        unlockPassiveSkill("loadout_memory", "Mémoire d'équipement");
    }

    if (inventory.getMaterialCount() >= 12)
    {
        unlockPassiveSkill("material_sorting_habit", "Tri des composants");
    }

    if (activeTitles.size() >= 3 && hasEquippedWeapon() && hasEquippedArmor())
    {
        unlockPassiveSkill("equipment_identity", "Identité d'équipement");
    }

    if (victories >= 10 && escapes >= 1)
    {
        unlockPassiveSkill("guild_route_memory", "Mémoire de route de guilde");
    }

    if (enemiesKilled >= 250)
    {
        unlockPassiveSkill("bestiary_family_reader", "Lecture des familles");
    }

    if (worldDaysElapsed >= 12 && escapes >= 1)
    {
        unlockPassiveSkill("cautious_pathing", "Pas prudent");
    }

    if (victories >= 15 && enemiesKilled >= 60)
    {
        unlockPassiveSkill("threat_route_planner", "Plan de route dangereux");
    }

    const bool wornWeapon = hasEquippedWeapon()
        && !getEquippedWeapon().isIndestructible()
        && getEquippedWeapon().getMaxDurability() > 0
        && getEquippedWeapon().getDurability() * 100 / getEquippedWeapon().getMaxDurability() <= 45;
    const bool wornArmor = hasEquippedArmor()
        && !getEquippedArmor().isIndestructible()
        && getEquippedArmor().getMaxDurability() > 0
        && getEquippedArmor().getDurability() * 100 / getEquippedArmor().getMaxDurability() <= 45;
    if (combatsStarted >= 6 && (wornWeapon || wornArmor))
    {
        unlockPassiveSkill("field_maintenance", "Entretien de terrain");
    }

    if (combatsStarted >= 10 && wornWeapon)
    {
        unlockPassiveSkill("weapon_care_habit", "Soin d'arme");
    }

    if (combatsStarted >= 10 && wornArmor && countRecentEquipmentUsageContaining(recentCombatEquipmentUsage, "Armure:") >= 5)
    {
        unlockPassiveSkill("armor_fit_memory", "Mémoire d'ajustement");
    }

    for (const PlayerCurse& curse : activeCurses)
    {
        if (curse.bossIdRequiredToBreak > 0 && combatsStarted >= 3)
        {
            unlockPassiveSkill("curse_anchor_awareness", "Conscience d'ancrage");
            break;
        }
    }

    if (countEnvironmentalResiliencePassives(*this) >= 3)
    {
        unlockPassiveSkill("environmental_resilience", "Résilience environnementale");
    }

    if (hasPassiveSkill("weapon_care_habit"))
    {
        grantTitle("Arme entretenue");
    }
    if (hasPassiveSkill("armor_fit_memory"))
    {
        grantTitle("Armure qui respire");
    }
    if (hasPassiveSkill("cautious_pathing"))
    {
        grantTitle("Retour par le bon chemin");
    }

    if (level >= MAX_LEVEL)
    {
        grantTitle("Niveau maximum, problème minimum");
    }

    refreshCurrencyTitles();
}

void Player::refreshCurrencyTitles()
{
    if (inventory.getCoinCount(CoinType::Platinum) > 0)
    {
        grantTitle("Premier éclat de platine");
    }

    const long long currentEconomy = inventory.getEconomyUnits();
    if (currentEconomy >= 1000000)
    {
        grantTitle("Millionnaire qui recompte");
    }
    if (currentEconomy >= 10000000)
    {
        grantTitle("Banquier de l'impossible");
        grantTitle("Le coffre a peur de toi");
    }
}

void Player::setLoadedSkillState(
    const std::vector<std::string>& passiveSkills,
    const std::vector<std::string>& activeSkills,
    int daggerProgress,
    int bowProgress,
    int bareHandProgress,
    int staffProgress,
    int swordProgress,
    int axeProgress,
    int hammerProgress,
    int spearProgress
)
{
    unlockedPassiveSkills = passiveSkills;
    unlockedActiveSkills = activeSkills;
    daggerKillProgress = daggerProgress < 0 ? 0 : daggerProgress;
    bowKillProgress = bowProgress < 0 ? 0 : bowProgress;
    bareHandKillProgress = bareHandProgress < 0 ? 0 : bareHandProgress;
    staffKillProgress = staffProgress < 0 ? 0 : staffProgress;
    swordKillProgress = swordProgress < 0 ? 0 : swordProgress;
    axeKillProgress = axeProgress < 0 ? 0 : axeProgress;
    hammerKillProgress = hammerProgress < 0 ? 0 : hammerProgress;
    spearKillProgress = spearProgress < 0 ? 0 : spearProgress;
    normalizeSkillLoadout();
    refreshLevelAndIdentitySkills();
    normalizeSkillLoadout();
}

void Player::setLoadedSkillLoadout(
    const std::vector<std::string>& enabledPassives,
    const std::vector<std::string>& equippedActives
)
{
    enabledPassiveSkills = enabledPassives;
    equippedActiveSkills = equippedActives;
    normalizeSkillLoadout();
}

// EN: displaySkillProgress declares or implements a focused behavior used by this module.
// FR: displaySkillProgress déclare ou implémente un comportement précis utilisé par ce module.
void Player::displaySkillProgress() const
{
    std::vector<std::string> lines;
    lines.push_back("Limite d'équipement : "
        + std::to_string(equippedActiveSkills.size()) + "/" + std::to_string(MAX_EQUIPPED_ACTIVE_SKILLS)
        + " actifs équipés, "
        + std::to_string(enabledPassiveSkills.size()) + "/" + std::to_string(MAX_ENABLED_PASSIVE_SKILLS)
        + " passifs activés.");
    lines.push_back("Un actif reste une action choisie. Un passif peut agir seul, mais seulement s'il est activé.");
    lines.push_back("Les paliers passifs de maîtrise soutiennent légèrement le geste lié : ils ne lancent jamais l'actif à ta place.");
    lines.push_back("");
    lines.push_back("Passives débloquées :");

    if (unlockedPassiveSkills.empty())
    {
        lines.push_back("- Aucune pour le moment.");
    }
    else
    {
        for (const std::string& skillId : unlockedPassiveSkills)
        {
            const std::string state = isPassiveSkillEnabled(skillId) ? "activé" : "désactivé";
            lines.push_back("- [" + state + "] " + playerSkillDisplayName(skillId) + " : " + playerSkillDescription(skillId));
        }
    }

    lines.push_back("");
    lines.push_back("Actives débloquées :");

    if (unlockedActiveSkills.empty())
    {
        lines.push_back("- Aucune pour le moment.");
    }
    else
    {
        for (const std::string& skillId : unlockedActiveSkills)
        {
            const std::string state = isActiveSkillEquipped(skillId) ? "équipé" : "déséquipé";
            lines.push_back("- [" + state + "] " + playerSkillDisplayName(skillId) + " : " + playerSkillDescription(skillId));
        }
    }

    lines.push_back("");
    lines.push_back("Maîtrise des actifs tactiques :");
    const std::vector<std::pair<std::string, std::string>> trackedActions = {
        {"lanterne_cible", "Lanterne ciblée"},
        {"lanterne_sol", "Lanterne au sol"},
        {"coup_de_pied", "Coup de pied tactique"},
        {"poussiere", "Poussière tactique"},
        {"observation_active", "Observation active"},
        {"exploitation_ouverture", "Exploiter une ouverture"},
        {"piege_artisan", "Piège improvisé de terrain"},
        {"preparation_arme", "Enduire / fusionner vite l'arme"},
        {"brise_garde", "Brise-garde"},
        {"posture_soutien", "Tenir la ligne"},
        {"rupture_formation", "Casser la formation"},
        {"coupe_signal", "Couper le signal"},
        {"lecture_corps", "Viser une faille de corps"},
        {"chaine_etats", "Forcer une chaîne d'états"},
        {"marque_proie", "Marquer une proie"},
        {"retrait_controle", "Sens du retrait"},
        {"lecture_menace", "Lire la menace"},
        {"balayage_bas", "Lecture du balayage bas"},
        {"rupture_concentration", "Étouffer la concentration"},
        {"rupture_allonge", "Rompre l'allonge"},
        {"perce_rempart", "Percer le rempart"},
        {"rupture_ancrage", "Rompre l'ancrage occulte"},
        {"souffle_ralliement", "Souffle rallié"},
        {"frappe_elan", "Élan canalisé"},
        {"voile_urgence", "Réflexe de voile"},
        {"etincelle_instable", "Instabilité apprivoisée"},
        {"cri_guerre", "Voix de guerre"},
        {"pas_ombre", "Pas de l'ombre"},
        {"coup_arc", "Coup en arc"},
        {"signe_vigueur", "Signe de vigueur"},
        {"feinte_sournoise", "Feinte sournoise"},
        {"suture_bataille", "Suture de bataille"},
        {"canalisation_arcanique", "Canalisation arcanique"},
        {"serment_rempart", "Serment du rempart"},
        {"tir_arret", "Tir d'arrêt"},
        {"rage_maitrisee", "Rage maîtrisée"},
        {"ordre_bataille", "Ordre de bataille"},
        {"totem_souffle", "Totem de souffle"},
        {"bombe_atelier", "Bombe d'atelier"},
        {"priere_acier", "Prière d'acier"},
        {"mantra_interieur", "Mantra intérieur"},
        {"image_trompeuse", "Image trompeuse"},
        {"lien_invocation", "Lien d'invocation"},
        {"pacte_sanguin", "Pacte sanguin"},
        {"remede_fortune", "Remède de fortune"},
        {"lame_elementaire", "Lame élémentaire"},
        {"cercle_protecteur", "Cercle protecteur"},
        {"fleche_entravante", "Flèche entravante"},
        {"chant_revigorant", "Chant revigorant"},
        {"instinct_bete", "Instinct de bête"},
        {"danse_lame", "Danse de lame"}
    };
    bool hasTrackedMastery = false;
    for (const auto& action : trackedActions)
    {
        const int uses = getSkillUseCount("actions_tactiques_combat", action.first);
        if (uses <= 0) continue;
        hasTrackedMastery = true;
        lines.push_back("- " + action.second + " : " + getActiveSkillMasteryLabel(action.first)
            + " | usages réussis " + std::to_string(uses)
            + " | " + getActiveSkillMasteryProgressHint(action.first)
            + " | effet : " + getActiveSkillMasteryEffectHint(action.first));
    }
    if (!hasTrackedMastery)
    {
        lines.push_back("- Aucun actif tactique assez répété pour laisser une trace visible.");
    }

    lines.push_back("");
    lines.push_back("Essais de passifs de maîtrise :");
    const std::vector<std::tuple<std::string, std::string, std::string>> trials = {
        {"observation_active", "terrain_reader", "Lecture du terrain"},
        {"preparation_arme", "field_weapon_crafter", "Préparateur d'arme de terrain"},
        {"brise_garde", "guard_breaker", "Casseur de garde"},
        {"posture_soutien", "support_rhythm", "Rythme de soutien"},
        {"rupture_formation", "formation_breaker", "Briseur de formation"},
        {"coupe_signal", "signal_cut_awareness", "Lecture coupe-signal"},
        {"lecture_corps", "body_reader", "Lecture des corps"},
        {"chaine_etats", "status_conductor", "Conducteur d'états"},
        {"marque_proie", "prey_marker", "Marqueur de proie"},
        {"retrait_controle", "controlled_retreat", "Sens du retrait"},
        {"lecture_menace", "threat_reader", "Lecture des menaces"},
        {"balayage_bas", "low_sweeper", "Lecture du balayage bas"},
        {"rupture_concentration", "focus_breaker", "Briseur de concentration"},
        {"rupture_allonge", "reach_breaker", "Briseur d'allonge"},
        {"perce_rempart", "shield_piercer", "Perce-rempart"},
        {"rupture_ancrage", "anchor_breaker", "Briseur d'ancrage"},
        {"souffle_ralliement", "rally_breath", "Souffle rallié"},
        {"frappe_elan", "momentum_striker", "Élan canalisé"},
        {"voile_urgence", "emergency_warder", "Réflexe de voile"},
        {"etincelle_instable", "wild_spark", "Instabilité apprivoisée"},
        {"cri_guerre", "war_cry_caller", "Voix de guerre"},
        {"pas_ombre", "shadow_step_mastery", "Maîtrise du pas de l'ombre"},
        {"coup_arc", "arc_sweep_mastery", "Amplitude contrôlée"},
        {"signe_vigueur", "vigor_sign_mastery", "Souffle de vigueur"},
        {"feinte_sournoise", "rogue_feint_mastery", "Angle de feinte"},
        {"suture_bataille", "battle_suture_mastery", "Gestes de suture"},
        {"canalisation_arcanique", "arcane_channel_mastery", "Canalisation stabilisée"},
        {"serment_rempart", "rampart_oath_mastery", "Tenue du rempart"},
        {"tir_arret", "stopping_shot_mastery", "Œil d'arrêt"},
        {"rage_maitrisee", "rage_control_mastery", "Rage canalisée"},
        {"ordre_bataille", "battle_order_mastery", "Voix de bataille"},
        {"totem_souffle", "breath_totem_mastery", "Ancrage du souffle"},
        {"bombe_atelier", "workshop_bomb_mastery", "Bricolage explosif"},
        {"priere_acier", "steel_prayer_mastery", "Foi d'acier"},
        {"mantra_interieur", "inner_mantra_mastery", "Souffle intérieur"},
        {"image_trompeuse", "trick_image_mastery", "Angle trompeur"},
        {"lien_invocation", "summoning_link_mastery", "Lien stabilisé"},
        {"pacte_sanguin", "blood_pact_mastery", "Sang discipliné"},
        {"remede_fortune", "field_remedy_mastery", "Gestes de terrain"},
        {"lame_elementaire", "elemental_blade_mastery", "Maîtrise élémentaire"},
        {"cercle_protecteur", "protective_circle_mastery", "Garde circulaire"},
        {"fleche_entravante", "binding_shot_mastery", "Trait entravant"},
        {"chant_revigorant", "inspiring_chant_mastery", "Voix revigorante"},
        {"instinct_bete", "beast_instinct_mastery", "Instinct canalisé"},
        {"danse_lame", "blade_dance_mastery", "Rythme de lame"}
    };
    bool hasTrial = false;
    for (const auto& trial : trials)
    {
        const std::string actionKey = std::get<0>(trial);
        const std::string passiveId = std::get<1>(trial);
        const std::string passiveName = std::get<2>(trial);
        if (isPassiveSkillUnlocked(passiveId)) continue;
        const int progress = std::clamp(getSkillUseCount("actions_tactiques_combat", actionKey), 0, 3);
        if (progress <= 0) continue;
        hasTrial = true;
        lines.push_back("- Tenter : " + passiveName + " [niveau d'essai : " + std::to_string(progress) + "/3]"
            + " | " + getPassiveMasteryProgressHintFromAction(actionKey));
    }
    if (!hasTrial)
    {
        lines.push_back("- Aucun essai de passif en cours pour le moment.");
    }

    lines.push_back("");
    lines.push_back("Niveaux des passifs de maîtrise débloqués :");
    bool hasPassiveMasteryLevel = false;
    for (const auto& trial : trials)
    {
        const std::string actionKey = std::get<0>(trial);
        const std::string passiveId = std::get<1>(trial);
        const std::string passiveName = std::get<2>(trial);
        if (!isPassiveSkillUnlocked(passiveId)) continue;
        hasPassiveMasteryLevel = true;
        lines.push_back("- " + passiveName + " : " + getPassiveMasteryLabelFromAction(actionKey)
            + " | " + getPassiveMasteryProgressHintFromAction(actionKey)
            + " | effet : " + getPassiveMasteryEffectHintFromAction(actionKey));
    }
    if (!hasPassiveMasteryLevel)
    {
        lines.push_back("- Aucun passif de maîtrise débloqué pour le moment.");
    }

    lines.push_back("");
    lines.push_back("Progression passive liée au gameplay :");
    lines.push_back("- Kills à la dague vers Enchaînement : " + std::to_string(daggerKillProgress) + "/5");
    lines.push_back("- Kills à l'arc vers Œil de rôdeur : " + std::to_string(bowKillProgress) + "/8");
    lines.push_back("- Kills aux mains nues vers Contre réflexe : " + std::to_string(bareHandKillProgress) + "/10");
    lines.push_back("- Kills au bâton vers Canalisation prudente : " + std::to_string(staffKillProgress) + "/6");
    lines.push_back("- Kills à l'épée vers Discipline de lame : " + std::to_string(swordKillProgress) + "/7");
    lines.push_back("- Kills à la hache vers Frappe fendue : " + std::to_string(axeKillProgress) + "/7");
    lines.push_back("- Kills au marteau vers Fracasse-garde : " + std::to_string(hammerKillProgress) + "/7");
    lines.push_back("- Kills à la lance vers Contrôle d'allonge : " + std::to_string(spearKillProgress) + "/7");

    lines.push_back("");
    lines.push_back("Progression de carrière :");
    lines.push_back("- Combats lancés vers Instinct de bataille : " + std::to_string(combatsStarted) + "/8");
    lines.push_back("- Victoires vers Rythme de vétéran : " + std::to_string(victories) + "/5");
    lines.push_back("- Défaites/morts vers Peau des survivants : " + std::to_string(defeats) + "/3 ou " + std::to_string(deaths) + "/2");
    lines.push_back("- Fuites vers Lecture de fuite : " + std::to_string(escapes) + "/2");
    lines.push_back("- Boss vaincus vers Mémoire de boss : " + std::to_string(bossesKilled) + "/1");

    lines.push_back("");
    lines.push_back("Habitudes d'équipement :");
    lines.push_back("- Combats récents avec armure vers Habitude d'armure : " + std::to_string(countRecentEquipmentUsageContaining(recentCombatEquipmentUsage, "Armure:")) + "/5");
    lines.push_back("- Combats récents avec arme + armure vers Mémoire d'équipement : " + std::to_string(countRecentFullLoadoutUsage(recentCombatEquipmentUsage)) + "/5");
    lines.push_back("- Entretien de terrain : se débloque avec plusieurs combats et de l'équipement vraiment usé.");
    lines.push_back("- Tri des composants : " + std::to_string(inventory.getMaterialCount()) + "/12 types/piles de matériaux dans l'inventaire.");
    lines.push_back("- Identité d'équipement : trois titres équipés + arme/armure portées.");
    lines.push_back("- Mémoire de route de guilde : victoires répétées + au moins une fuite apprise.");
    lines.push_back("- Lecture des familles : " + std::to_string(enemiesKilled) + "/250 monstres vaincus.");
    lines.push_back("- Pas prudent : " + std::to_string(worldDaysElapsed) + "/12 jours vécus + " + std::to_string(escapes) + "/1 fuite apprise.");
    lines.push_back("- Plan de route dangereux : " + std::to_string(victories) + "/15 victoires + " + std::to_string(enemiesKilled) + "/60 monstres vaincus.");
    lines.push_back("- Résilience environnementale : " + std::to_string(countEnvironmentalResiliencePassives(*this)) + "/3 passifs compatibles température / résistance / terrain.");
    lines.push_back("- Lecture de terrain : " + std::to_string(getCanonicalJournalCategoryTotal("observations_terrain")) + "/3 observations de terrain utiles.");
    lines.push_back("- Lecture de contrat : " + std::to_string(getCanonicalJournalCategoryTotal("consultations_comptoir_mercenaire")) + "/2 passages au comptoir mercenaire.");
    lines.push_back("- Entraînement : le stand peut apprendre Observation tactique, Habitude chaleur/froid, Habitude terrain difficile ou Appuis entraînés.");
    lines.push_back("- Soin d'arme : 10 combats + arme équipée vraiment usée.");
    lines.push_back("- Mémoire d'ajustement : 10 combats + armure usée portée sur plusieurs combats.");
    lines.push_back("- Conscience d'ancrage : se débloque si une malédiction de boss reste sur le personnage hors combat.");

    PlayerUiSupport::showPlayerScreen("COMPÉTENCES", "player.skill_progress", lines, false);
}



// EN: getCombatsStarted declares or implements a focused behavior used by this module.
// FR: getCombatsStarted déclare ou implémente un comportement précis utilisé par ce module.
