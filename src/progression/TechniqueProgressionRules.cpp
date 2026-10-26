// EN: Central progression thresholds for player combat techniques on the 1-255 level curve.
// FR: Seuils centraux de progression des techniques de combat du joueur sur la courbe 1-255.
#include "progression/TechniqueProgressionRules.hpp"

#include <unordered_map>

namespace
{
    int lookupOr(const std::unordered_map<std::string, int>& levels, const std::string& id, int fallback)
    {
        const auto it = levels.find(id);
        return it == levels.end() ? fallback : it->second;
    }
}

namespace TechniqueProgressionRules
{
    int genericTechniqueUnlockLevel(const std::string& techniqueId)
    {
        // The game caps at level 255. Only very simple survival gestures belong in the first levels;
        // stronger multi-target, execution and unstable techniques are deliberately spread much farther.
        static const std::unordered_map<std::string, int> levels = {
            {"safe_rally", 3},
            {"guard_break", 5},
            {"stop_hit", 12},
            {"focus_signal", 18},
            {"line_sweep", 24},
            {"environment_move", 30},
            {"line_break", 40},
            {"field_suture", 50},
            {"material_sacrifice", 60},
            {"random_ricochet", 65},
            {"opportunist_execute", 80},
            {"unstable_mix", 100}
        };
        return lookupOr(levels, techniqueId, 255);
    }

    int chainUnlockLevel(bool magicProfile)
    {
        return magicProfile ? 15 : 8;
    }

    int tripleChainUnlockLevel()
    {
        return 35;
    }

    int quadrupleChainUnlockLevel()
    {
        // First genuine late-game evolution of the universal multi-hit chain.
        return 120;
    }

    int genericMasteryUnlockLevel(const std::string& masteryId)
    {
        // Late-game mastery should deepen techniques the player already understands instead of
        // flooding the menu with dozens of unrelated buttons. The cap remains level 255.
        static const std::unordered_map<std::string, int> levels = {
            {"chain_mastery", 120},
            {"battlefield_mastery", 150},
            {"veteran_rally", 180},
            {"master_execution", 210},
            {"perfect_guard_break", 240}
        };
        return lookupOr(levels, masteryId, 255);
    }

    int weaponPracticeUnlockLevel(const std::string& skillId)
    {
        static const std::unordered_map<std::string, int> levels = {
            {"combat_observation", 5},
            {"ranger_eye", 6},
            {"blade_discipline", 6},
            {"reach_control", 7},
            {"chain_execution", 10},
            {"cautious_channeling", 10},
            {"reflex_counter", 12},
            {"splitting_blow", 12},
            {"armor_crack", 12},
            {"temperature_drill", 4},
            {"terrain_drill", 4},
            {"footwork_drill", 6}
        };
        return lookupOr(levels, skillId, 255);
    }

    int classTechniqueUnlockLevel(const std::string& techniqueId)
    {
        static const std::unordered_map<std::string, int> levels = {
            {"opportunist_hit", 8},
            {"escape_powder", 20},
            {"pocket_strike", 40},
            {"shadow_step", 10},
            {"low_disappearance", 30},
            {"taunting_wall", 8},
            {"total_anchor", 30},
            {"survival_oath", 8},
            {"purifying_prayer", 25},
            {"arcane_mark", 10},
            {"arcane_binding", 20},
            {"elemental_ward", 35},
            {"controlled_overload", 55},
            {"resistance_rift", 75},
            {"cover_shot", 8},
            {"stopping_shot", 25},
            {"assumed_rage", 8},
            {"rupture_blow", 25},
            {"improvised_mix", 8},
            {"tactical_vial", 20},
            {"fragility_powder", 40},
            {"spectral_order", 10},
            {"pack_shadow_link", 30}
        };
        return lookupOr(levels, techniqueId, 255);
    }

    int magicStudyUnlockLevel(const std::string& learnedSkillId)
    {
        // Grimoires may teach a formula before its natural class progression, but not at level 2-5.
        // The book reduces the learning barrier; it does not erase the physical/magical maturity needed.
        static const std::unordered_map<std::string, int> levels = {
            {"learned_arcane_mark", 8},
            {"learned_arcane_binding", 15},
            {"learned_elemental_ward", 25},
            {"learned_resistance_rift", 55},
            {"learned_frost_needle", 20},
            {"learned_mana_suture", 30},
            {"learned_occult_bramble", 25}
        };
        return lookupOr(levels, learnedSkillId, 255);
    }

    int classIdentityUnlockLevel(const std::string& className)
    {
        // First class signatures remain early, but no longer all arrive at level 5 on a 255-level curve.
        static const std::unordered_map<std::string, int> levels = {
            {"Assassin", 10},
            {"Chevalier", 8},
            {"Colosse", 8},
            {"Mage", 10},
            {"Forgeron", 8},
            {"Alchimiste", 8},
            {"Rôdeur", 8},
            {"Artificier", 8}
        };
        return lookupOr(levels, className, 8);
    }
    int tacticalActionUnlockLevel(const std::string& actionKey)
    {
        // Universal gestures stay available early. More technical control, multi-target and
        // class-flavoured actions are deliberately spread across the 1-255 curve.
        static const std::unordered_map<std::string, int> levels = {
            {"lanterne_cible", 1},
            {"lanterne_sol", 3},
            {"coup_de_pied", 1},
            {"poussiere", 1},
            {"observation_active", 1},
            {"exploitation_ouverture", 3},
            {"piege_artisan", 6},
            {"preparation_arme", 8},
            {"brise_garde", 5},
            {"posture_soutien", 2},
            {"rupture_formation", 40},
            {"coupe_signal", 10},
            {"lecture_corps", 12},
            {"chaine_etats", 55},
            {"marque_proie", 8},
            {"retrait_controle", 2},
            {"lecture_menace", 6},
            {"balayage_bas", 12},
            {"rupture_concentration", 18},
            {"rupture_allonge", 18},
            {"perce_rempart", 35},
            {"rupture_ancrage", 50},
            {"souffle_ralliement", 3},
            {"frappe_elan", 8},
            {"voile_urgence", 12},
            {"etincelle_instable", 15},
            {"cri_guerre", 18},
            {"pas_ombre", 10},
            {"coup_arc", 24},
            {"signe_vigueur", 5},
            {"feinte_sournoise", 8},
            {"suture_bataille", 20},
            {"canalisation_arcanique", 10},
            {"serment_rempart", 8},
            {"tir_arret", 25},
            {"rage_maitrisee", 8},
            {"ordre_bataille", 20},
            {"totem_souffle", 25},
            {"bombe_atelier", 35},
            {"priere_acier", 18},
            {"mantra_interieur", 12},
            {"image_trompeuse", 35},
            {"lien_invocation", 45},
            {"pacte_sanguin", 45},
            {"remede_fortune", 20},
            {"lame_elementaire", 35},
            {"cercle_protecteur", 40},
            {"fleche_entravante", 25},
            {"chant_revigorant", 20},
            {"instinct_bete", 20},
            {"danse_lame", 50}
        };
        return lookupOr(levels, actionKey, 255);
    }

}
