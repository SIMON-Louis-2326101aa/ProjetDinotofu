#!/usr/bin/env bash
set -euo pipefail

TEST_ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${TEST_ROOT_DIR}"

fail() { echo "[FAIL] $1" >&2; exit 1; }
pass() { echo "[OK] $1"; }

TEST_CXX="${CXX:-g++}"
read -r -a TEST_CXX_CMD <<< "$TEST_CXX"
CXX_STD_FLAG="$(bash ./scripts/detect_cpp23_flag.sh "${TEST_CXX_CMD[@]}")"
pass "toolchain C++23 detectee : ${TEST_CXX} ${CXX_STD_FLAG}"

TEST_LDFLAGS=()
if [[ -n "${LDFLAGS:-}" ]]; then
    read -r -a TEST_LDFLAGS <<< "${LDFLAGS}"
elif command -v mold >/dev/null 2>&1; then
    TEST_LDFLAGS=("-fuse-ld=mold")
elif command -v lld >/dev/null 2>&1; then
    TEST_LDFLAGS=("-fuse-ld=lld")
fi

if [[ -f build/libdinotofu.a ]]; then
    PROJECT_OBJECTS="build/libdinotofu.a"
    PROJECT_OBJECTS_LIVING="build/libdinotofu.a"
else
    PROJECT_OBJECTS="$(find build -name '*.o' ! -path 'build/main.o' -print)"
    PROJECT_OBJECTS_LIVING="$PROJECT_OBJECTS"
fi

PARALLEL_DIR="$(mktemp -d /tmp/dinotofu_tests.XXXXXX)"
cleanup() {
    rm -rf "$PARALLEL_DIR"
}
trap cleanup EXIT

declare -A TEST_PIDS
run_bg_test() {
    local test_id="$1"
    local src="$2"
    shift 2
    local extra_link=("$@")
    local bin="$PARALLEL_DIR/${test_id}.bin"
    local log="$PARALLEL_DIR/${test_id}.log"
    local run_dir="$PARALLEL_DIR/run_${test_id}"
    mkdir -p "$run_dir"

    (
        if ! "${TEST_CXX_CMD[@]}" "$CXX_STD_FLAG" -Wall -Wextra -Iinclude "$src" "${extra_link[@]}" "${TEST_LDFLAGS[@]}" -o "$bin" >"$log" 2>&1; then
            echo "COMPILE_ERROR" > "$PARALLEL_DIR/${test_id}.status"
            exit 1
        fi
        if ! ( cd "$run_dir" && "$bin" >"$log" 2>&1 ); then
            echo "RUN_ERROR" > "$PARALLEL_DIR/${test_id}.status"
            exit 1
        fi
        echo "OK" > "$PARALLEL_DIR/${test_id}.status"
        exit 0
    ) &
    TEST_PIDS["$test_id"]=$!
}

wait_bg_test() {
    local test_id="$1"
    local fail_msg="$2"
    local pid="${TEST_PIDS[$test_id]:-}"
    if [[ -n "$pid" ]]; then
        wait "$pid" || true
    fi
    local status="FAIL"
    if [[ -f "$PARALLEL_DIR/${test_id}.status" ]]; then
        status="$(cat "$PARALLEL_DIR/${test_id}.status")"
    fi
    if [[ "$status" != "OK" ]]; then
        echo "[FAIL] $fail_msg" >&2
        if [[ -f "$PARALLEL_DIR/${test_id}.log" ]]; then
            cat "$PARALLEL_DIR/${test_id}.log" >&2
        fi
        exit 1
    fi
}

# Lancement immédiat des 22 tests unitaires C++ en tâche de fond
run_bg_test "rival" "tests/RivalEmergenceSystemTest.cpp" "src/combat/rival/RivalEmergenceSystem.cpp"
run_bg_test "lang" "tests/LanguageSystemTest.cpp" "src/progression/language/LanguageSystem.cpp" -ffunction-sections -fdata-sections -Wl,--gc-sections
run_bg_test "quest_lang" "tests/QuestLanguageSystemTest.cpp" "src/quest/language/QuestLanguageSystem.cpp" -ffunction-sections -fdata-sections -Wl,--gc-sections
run_bg_test "living" "tests/LivingWorldContentTest.cpp" "$PROJECT_OBJECTS_LIVING"
run_bg_test "biome" "tests/BiomeNonCombatInteractionSystemTest.cpp" "src/adventure/content/BiomeNonCombatInteractionSystem.cpp"
run_bg_test "npc_rel" "tests/NpcRelationshipSystemTest.cpp" "src/world/npc/NpcRelationshipSystem.cpp"
run_bg_test "npc_prop" "tests/NpcInformationPropagationSystemTest.cpp" "$PROJECT_OBJECTS_LIVING"
run_bg_test "npc_intercity" "tests/NpcIntercityInformationSystemTest.cpp" "$PROJECT_OBJECTS_LIVING"
run_bg_test "save" "tests/SaveRoundTripTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "economy" "tests/EconomyScaleTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "econ_mig" "tests/EconomySaveMigrationTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "checkpoint" "tests/ImportantSaveCheckpointTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "legacy" "tests/LegacySaveMigrationTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "story" "tests/StoryPrologueMemoryTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "special_char" "tests/SpecialCharacterExpansionTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "duo" "tests/DuoMasterySystemTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "group" "tests/EnemyGroupBehaviorTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "monster_prep" "tests/MonsterPreparedActionSystemTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "monster_content" "tests/MonsterContentExpansionTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "monster_elite" "tests/MonsterAmbientEliteDensityTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "monster_flavor" "tests/MonsterFlavorLifecycleTest.cpp" "$PROJECT_OBJECTS"
run_bg_test "reputation_repair" "tests/LocalReputationRepairSystemTest.cpp" "$PROJECT_OBJECTS"

VERSION="$(bash ./scripts/get_version.sh)"
[[ -n "$VERSION" ]] || fail "version introuvable"
[[ "$(cd /tmp && bash "${TEST_ROOT_DIR}/scripts/get_version.sh")" == "$VERSION" ]] || fail "get_version.sh dépend du dossier courant"
[[ -x ./output/Dinotofu ]] || fail "output/Dinotofu absent : make test doit dépendre du build"
[[ "$(./output/Dinotofu --version)" == *"V${VERSION}"* ]] || fail "version binaire incohérente"
pass "version ${VERSION} cohérente"

[[ -f CHANGELOG.md ]] || fail "CHANGELOG.md manquant"
[[ -f CHANGELOG_FR.md ]] || fail "CHANGELOG_FR.md manquant"
grep -q "V${VERSION}" CHANGELOG.md || fail "CHANGELOG sans V${VERSION}"
grep -q "V${VERSION}" CHANGELOG_FR.md || fail "CHANGELOG_FR sans V${VERSION}"
[[ ! -f PATCHNOTE_DINOTOFU.md && ! -f PATCHNOTE_DINOTOFU_FR.md ]] || fail "anciens PATCHNOTE encore présents"
pass "CHANGELOG anglais/français remplacent les PATCHNOTE"

! grep -q "V3.30.00" README.md || fail "historique de versions revenu dans README.md"
! grep -q "V3.30.00" READMEFR.md || fail "historique de versions revenu dans READMEFR.md"
pass "README sans historique détaillé"

if find scripts -type f -name '*.sh' -exec grep -Il $'\r' {} + | grep -q .; then
    fail "scripts shell avec fins de ligne CRLF incompatibles Linux"
fi
pass "scripts shell portables en fins de ligne LF"

if find src include -type f -name '*.inc' | grep -q .; then
    fail "des fichiers .inc existent encore dans src/include"
fi
pass "aucun .inc dans src/include"

[[ -f src/core/GameSetup.cpp ]] || fail "création/configuration de partie non extraite de Game.cpp"
[[ -f src/core/GameStory.cpp ]] || fail "runtime histoire non extrait de Game.cpp"
[[ -f src/interface/menu/quest/QuestLocationNpcMenu.cpp && -f src/interface/menu/quest/QuestClientNavigationSupport.cpp ]] || fail "navigation lieux/PNJ de quête non modularisée"
[[ -f src/interface/menu/quest/QuestStoryMenu.cpp && -f src/interface/menu/quest/QuestStorySupport.cpp && -f include/interface/menu/quest/QuestStorySupport.hpp ]] || fail "gestion histoire de QuestMenu non modularisée"
[[ -f src/interface/menu/quest/QuestExplorationMenu.cpp && -f include/interface/menu/quest/QuestExplorationSupport.hpp && -f include/interface/menu/quest/QuestMenuInternalSupport.hpp ]] || fail "moteur d'exploration de QuestMenu non modularisé"
[[ -f src/interface/menu/quest/QuestPresentationSupport.cpp && -f include/interface/menu/quest/QuestPresentationSupport.hpp ]] || fail "présentation partagée des quêtes non modularisée"
[[ -f src/combat/turn/wave/PlayerWaveTacticalActionMenu.cpp && -f include/combat/turn/wave/PlayerWaveTacticalActionMenu.hpp ]] || fail "menu tactique de vague non extrait"
[[ -f src/combat/turn/wave/PlayerWaveTacticalSupport.cpp && -f include/combat/turn/wave/PlayerWaveTacticalSupport.hpp ]] || fail "helpers tactiques de vague non modularisés"
[[ $(wc -l < src/core/Game.cpp) -lt 4000 ]] || fail "Game.cpp a regrossi au-delà de 4000 lignes"
[[ $(wc -l < src/interface/menu/quest/QuestMenu.cpp) -lt 7000 ]] || fail "QuestMenu.cpp a regrossi au-delà de 7000 lignes"
[[ $(wc -l < src/combat/turn/wave/PlayerWaveCombatTurn.cpp) -lt 700 ]] || fail "PlayerWaveCombatTurn.cpp a regrossi au-delà de 700 lignes"
[[ $(wc -l < src/combat/turn/wave/PlayerWaveTacticalActionMenu.cpp) -lt 3200 ]] || fail "PlayerWaveTacticalActionMenu.cpp a regrossi au-delà de 3200 lignes"
pass "modularisation Game/QuestMenu/PlayerWaveCombatTurn et tactiques protégée contre les régressions"

[[ -f include/entity/player/PlayerHistoryTypes.hpp && -f src/entity/player/PlayerHistory.cpp ]] || fail "module mémoire/rivaux absent"
grep -q 'struct PlayerRivalRecord' include/entity/player/PlayerHistoryTypes.hpp || fail "PlayerRivalRecord absent"
grep -q 'recordRivalReturn' src/entity/player/PlayerHistory.cpp || fail "retours de rivaux absents"
[[ -f include/combat/rival/RivalEmergenceSystem.hpp && -f src/combat/rival/RivalEmergenceSystem.cpp ]] || fail "sélection des rivaux absente"
grep -q "Aucun rival n'est créé" src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "fuite ordinaire non distinguée d'un rival"
wait_bg_test "rival" "logique de sélection des rivaux invalide"
pass "mémoire persistante, fuites ordinaires et rivaux testés"

[[ -f include/progression/language/LanguageSystem.hpp && -f src/progression/language/LanguageSystem.cpp ]] || fail "système de langues absent"
[[ -f include/combat/dialogue/EncounterDialogueSystem.hpp && -f src/combat/dialogue/EncounterDialogueSystem.cpp ]] || fail "dialogues de rencontre multilingues absents"
grep -q 'languageKnowledge' src/save/SaveManager.cpp || fail "sauvegarde des langues absente"
SAVE_SCHEMA_VERSION="$(sed -n 's/.*inline constexpr int Current = \([0-9][0-9]*\);.*/\1/p' include/save/SaveSchemaVersion.hpp | head -n 1)"
[[ -n "$SAVE_SCHEMA_VERSION" ]] || fail "schéma de sauvegarde courant introuvable"
grep -q 'SaveSchemaVersion::Current' src/save/SaveManager.cpp || fail "SaveManager n'utilise pas le schéma de sauvegarde centralisé"
grep -q 'language_goblin_primer' src/economy/shop/LibraryInformationCatalog.cpp || fail "cours de bibliothèque absents"
grep -q 'QuestLanguageSystem::canRead' src/interface/menu/quest/QuestMenu.cpp || fail "contrats étrangers non reliés au journal"
wait_bg_test "lang" "catalogue de langues invalide"
wait_bg_test "quest_lang" "affectation des langues de quête invalide"
pass "langues, bibliothèque, dialogues et contrats étrangers testés"

[[ -f include/adventure/flavor/ExplorationLanguageTrace.hpp && -f src/adventure/flavor/ExplorationLanguageTrace.cpp ]] || fail "traces linguistiques d'exploration absentes"
[[ -f include/adventure/content/BiomeLivingContentCatalog.hpp && -f src/adventure/content/BiomeLivingContentCatalog.cpp ]] || fail "contenu vivant structuré des biomes absent"
[[ -f include/world/npc/LivingNpcProfile.hpp && -f src/world/npc/LivingNpcProfile.cpp ]] || fail "profil PNJ vivant absent"
[[ -f include/world/npc/NpcKnownFact.hpp && -f src/entity/player/PlayerNpcMemory.cpp ]] || fail "mémoire factuelle persistante des PNJ absente"
grep -q 'BiomeLivingContentCatalog::buildCurrentObservationLines' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "contenu vivant des biomes non relié à l'exploration"
grep -q 'BiomeAmbientEventSystem::buildCurrentEvent' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "micro-événements de biome non reliés à l'exploration"
grep -q 'questLocationNamesConcreteBiome' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "lieux précis de quête non prioritaires pendant l'exploration"
grep -q 'AUTONOMIE INSUFFISANTE' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "pré-contrôle des rations d'exploration absent"
grep -q 'Catégorie : ' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "catégories de micro-épreuves d'exploration absentes"
grep -q 'Catégorie de service : ' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "catégories des épreuves de service absentes"
grep -q 'serviceChallengeCategory' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "catégorisation contextuelle des services absente"
grep -q 'recentExplorationChallengeKeys.size() > 10' src/entity/Player.cpp || fail "historique anti-répétition des micro-épreuves trop court"
grep -q 'minimumLevelForGuildTemplateLocation' src/quest/QuestCatalog.cpp || fail "filtrage des quêtes de guilde par niveau réel du lieu absent"
grep -q 'prioritizeGuildBoardForCity' src/interface/menu/quest/QuestMenu.cpp || fail "priorisation locale du panneau de guilde non reliée à la ville courante"
grep -q 'guildOfferLocalityScore' src/quest/QuestLog.cpp || fail "score de proximité des offres de guilde absent"
grep -q 'enforceQuestObjectiveTypeDiversity' src/quest/QuestCatalog.cpp || fail "diversité des familles du panneau de guilde absente"
grep -q 'weeping_garden_shift_confirmed' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "événement signature du Jardin des statues qui pleurent absent"
grep -q 'bouquet_devant_ange' src/adventure/content/BiomeNonCombatInteractionSystem.cpp || fail "interaction non-combat propre au Jardin absente"
grep -q 'notableForLongTermHistory' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "filtrage des interactions réellement mémorables absent"
grep -q 'ExplorationLanguageTraceCatalog::renderForPlayer' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "traces linguistiques non reliées à l'exploration"
wait_bg_test "living" "contenu vivant/langues/PNJ invalide"
pass "traces multilingues d'exploration et profils PNJ vivants testés"

wait_bg_test "biome" "interactions non-combat de biome invalides"
grep -q 'BiomeNonCombatInteractionSystem::buildCurrentInteraction' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "interactions non-combat non reliées à l'exploration"
grep -q 'interactions_non_combat_biome' src/interface/menu/quest/QuestExplorationMenu.cpp || fail "anti-farm des interactions de biome absent"
pass "interactions non-combat contextuelles de biome testées"

wait_bg_test "npc_rel" "relations nommées entre PNJ invalides"
grep -q 'NpcRelationshipSystem::relayModifier' src/world/npc/NpcInformationPropagationSystem.cpp || fail "relations PNJ non reliées à la propagation"
pass "relations PNJ influençant la circulation des informations testées"

wait_bg_test "npc_prop" "propagation locale des informations PNJ invalide"
grep -q 'rumeur_locale' src/world/npc/NpcInformationPropagationSystem.cpp || fail "chaîne de source locale absente"
grep -q 'spontaneousIntroLines' src/interface/menu/quest/QuestMenu.cpp || fail "prises de parole spontanées PNJ non reliées"
pass "circulation locale sourcée des informations PNJ testée"

wait_bg_test "npc_intercity" "circulation inter-ville des informations PNJ invalide"
grep -q 'intercityTravelDelayDays' src/world/npc/NpcInformationPropagationSystem.cpp || fail "délai inter-ville des rumeurs absent"
grep -q 'messager_de_garde' src/world/npc/NpcInformationPropagationSystem.cpp || fail "canaux physiques inter-ville absents"
pass "circulation inter-ville temporisée et sourcée des informations PNJ testée"

wait_bg_test "save" "round-trip sauvegarde (niveau/argent/temps/langues) ou migration invalide"
pass "round-trip sauvegarde niveau/argent/temps + migration sans languageState testés"

wait_bg_test "economy" "échelle monétaire PF/PC invalide"
grep -q 'getStarterCoinStacks(difficulty)' src/entity/player/PlayerEquipmentLifecycle.cpp || fail "argent de départ hors bourse physique"
grep -q 'currencyIronCoins' src/save/SaveManager.cpp || fail "piles physiques de monnaie non sauvegardées"
grep -q 'getCoinStacks' src/item/Inventory.cpp || fail "portefeuille physique non exposé"
grep -q 'const CoinBreakdown payout = Money::breakdownFromCopper' src/item/Inventory.cpp || fail "les récompenses économiques ne sont plus distribuées en dénominations physiques"
grep -q 'convertCoinLots' src/item/Inventory.cpp || fail "conversion personnalisée entre dénominations absente"
grep -q 'compactCoinsToHighest' src/item/Inventory.cpp || fail "normalisation volontaire vers la bourse compacte absente"
grep -q 'flattenCoinsToCopper' src/item/Inventory.cpp || fail "conversion volontaire intégrale vers PC absente"
grep -q 'Tout vers les pièces les plus élevées' src/interface/menu/quest/QuestMenu.cpp || fail "mode bourse compacte absent du bureau de change"
grep -q 'Maximum possible' src/interface/menu/quest/QuestMenu.cpp || fail "maximum convertible non affiché au bureau de change"
grep -q 'rewardCoinPlatinum' src/save/SaveManager.cpp || fail "récompenses de quête en pièces exactes non sauvegardées"
grep -q 'earnCoinStacks(quest.rewardCoins)' src/interface/menu/quest/QuestMenu.cpp || fail "récompense exacte de quête remplacée par une conversion générique"
grep -q 'refreshCurrencyTitles' src/combat/reward/CombatRewardSystem.cpp || fail "jalons monétaires non rafraîchis après les gains de combat"
if grep -R -n --include='*.cpp' -E '\.earnGold\(|\.spendGold\(' src --exclude='Inventory.cpp' | grep -q .; then
    fail "un flux gameplay utilise encore earnGold/spendGold au lieu des unités économiques"
fi
grep -q 'spendEconomyUnits(finalPrice)' src/economy/shop/ShopTransactionSystem.cpp || fail "prix boutique non débités depuis les unités économiques"
grep -q 'roundBuyQuote' src/interface/menu/shop/ShopMenu.cpp || fail "les tarifs d'artisan ne passent plus par l'arrondi RP"
grep -q "Tarif d'artisan" src/economy/shop/ShopPriceRules.cpp || fail "le style RP des tarifs artisans a disparu"
grep -q 'earnEconomyUnits(reward.getEconomyUnits())' src/combat/reward/CombatRewardSystem.cpp || fail "récompense de combat hors API unités économiques"
grep -q 'earnEconomyUnits(balancedQuestGold(quest))' src/interface/menu/quest/QuestMenu.cpp || fail "récompense de quête encore versée en PO historiques"
! grep -R -n -E 'getStarterGold|starterGold|fineCostGold' include src tests >/dev/null || fail "noms économiques ambigus starter/fine réintroduits"
grep -q 'int economyUnits;' include/combat/reward/CombatReward.hpp || fail "CombatReward ne stocke plus explicitement les unités économiques"
if grep -n -E 'std::to_string\([^)]*(Cost|cost|price|getValue\(\))[^;]*\+ " (PO|or)' \
    src/interface/menu/training/TrainingGroundMenu.cpp \
    src/interface/menu/equipment/EquipmentDisplay.cpp \
    src/interface/menu/inventory/InventorySelection.cpp \
    src/interface/menu/potions/CombatPotionDisplay.cpp \
    src/economy/shop/ShopTransactionSystem.cpp | grep -q .; then
    fail "un menu affiche encore une valeur économique PF comme PO/or brut"
fi
pass "échelle économique physique PC/PF/PE/PO/PP, départ, boutiques, combats et quêtes protégés"

wait_bg_test "econ_mig" "migration monétaire schema 24 -> 25 invalide"
pass "migration monétaire unique 24 -> 25 et conservation du reliquat PC testées"

wait_bg_test "checkpoint" "point de sauvegarde important V3.50.33 invalide"
grep -q 'return "3.50.33";' src/core/VersionInfo.cpp || fail "jalon de sauvegarde important V3.50.33 absent"
grep -Fq 'Important save checkpoint: **V3.50.33**' README.md || fail "README anglais désynchronisé du checkpoint important"
grep -Fq 'Point de sauvegarde important : **V3.50.33**' READMEFR.md || fail "README français désynchronisé du checkpoint important"
grep -q 'POINT DE SAUVEGARDE IMPORTANT' src/save/menu/CharacterMenu.cpp || fail "rituel de transition obligatoire absent"
grep -q 'createImportantUpdateBackup' src/save/menu/CharacterMenu.cpp || fail "backup pré-mise-à-jour non branché au chargement"
pass "point de sauvegarde important, backup non écrasant et rituel de transition testés"

wait_bg_test "legacy" "migration forcée des anciennes sauvegardes invalide"
grep -q 'if (!saved.known)' src/core/VersionInfo.cpp || fail "anciennes sauvegardes non versionnées non forcées vers le checkpoint"
grep -q 'createdForVersion", summary.gameVersion' src/save/SaveManager.cpp || fail "fallback gameVersion des anciennes sauvegardes absent"
pass "anciennes sauvegardes versionnées ou non forcées vers la migration V3.50.33"

# Updater compatibility: old configs using the historical placeholder must be repaired automatically.
grep -q 'TON_COMPTE/TON_REPO' tools/linux/DinotofuLauncher.sh || fail "compatibilité placeholder repo absente du launcher Linux"
grep -q 'SIMON-Louis-2326101aa/ProjetDinotofu' tools/linux/DinotofuLauncher.sh || fail "repo GitHub par défaut absent du launcher Linux"
grep -q 'TON_COMPTE/TON_REPO' tools/windows/DinotofuLauncher.ps1 || fail "compatibilité placeholder repo absente du launcher Windows"
grep -q 'SIMON-Louis-2326101aa/ProjetDinotofu' tools/windows/DinotofuLauncher.ps1 || fail "repo GitHub par défaut absent du launcher Windows"
grep -q 'REPO_NAME="$(detect_repo_name)"' scripts/package_linux_release.sh || fail "repo release Linux non résolu automatiquement"
grep -q 'REPO_NAME="$(detect_repo_name)"' scripts/package_windows_release.sh || fail "repo release Windows non résolu automatiquement"
grep -q 'RELEASE_HAS_REQUIRED_ASSETS' .github/workflows/release-dinotofu.yml || fail "workflow GitHub ne répare plus les releases incomplètes"
grep -q 'cp -r assets "${STAGING_DIR}/"' scripts/package_linux_release.sh || fail "pack Linux ne conserve plus le layout assets historique"
grep -q 'cp -r assets "${STAGING_DIR}/"' scripts/package_windows_release.sh || fail "pack Windows ne conserve plus le layout assets historique"
pass "mise à jour des anciennes installations et packaging historique protégés"

[[ -f include/save/SaveSchemaVersion.hpp ]] || fail "version de schéma de sauvegarde centralisée absente"
grep -q -- '--save-schema' scripts/bump_version.py || fail "bump_version.py ne sait pas proposer le changement de saveVersion"
grep -q -- '--checkpoint' scripts/bump_version.py || fail "bump_version.py ne sait pas proposer un nouveau checkpoint obligatoire"
grep -q 'Créer maintenant un commit Git ?", default=True' scripts/bump_version.py || fail "bump_version.py doit proposer Oui par défaut pour le commit interactif final"
grep -q 'detect_cpp23_flag.sh' Makefile || fail "Makefile non relié au mode C++23"
grep -q 'Changer aussi le schéma de sauvegarde' scripts/bump_version.py || fail "question interactive saveVersion absente"
grep -q 'NOUVEAU checkpoint obligatoire' scripts/bump_version.py || fail "question interactive checkpoint absent"
grep -q 'Dinotofu-Windows-v${VERSION}-TECHNICAL-PAYLOAD.zip' .github/workflows/release-dinotofu.yml || fail "payload technique Windows absent du workflow"
grep -q 'Dinotofu-Linux-v${VERSION}-TECHNICAL-PAYLOAD.7z' .github/workflows/release-dinotofu.yml || fail "payload technique Linux absent du workflow"
grep -q -- '--target "${GITHUB_SHA}"' .github/workflows/release-dinotofu.yml || fail "création de release après build non ciblée sur le commit courant"
[[ -x scripts/trigger_release.sh ]] || fail "helper de déclenchement manuel GitHub absent/non exécutable"
grep -q 'INSTALLER-DINOTOFU.sh' scripts/package_linux_release.sh || fail "installateur Linux joueur absent du packaging"
grep -q '/gui/switch-terminal' tools/gui/dinotofu_gui_experimental.html || fail "bascule IG -> Terminal absente de l accueil graphique"
grep -q 'switch_to_terminal.request' tools/gui/serve_gui_preview.py || fail "signal local IG -> Terminal absent du serveur"
if grep -q 'launch_hidden_gui_backend' tools/linux/DinotofuLauncher.sh; then fail "le placeholder IG Linux ne doit plus lancer le moteur cache"; fi
if grep -q 'moteur Dinotofu en arriere-plan IG' tools/windows/DinotofuLauncher.ps1; then fail "le placeholder IG Windows ne doit plus lancer le moteur cache"; fi
grep -q 'Aucun moteur de jeu n est lance en arriere-plan' tools/linux/DinotofuLauncher.sh || fail "launcher Linux ne documente pas le placeholder leger"
grep -q 'Aucun moteur de jeu n est lance en arriere-plan' tools/windows/DinotofuLauncher.ps1 || fail "launcher Windows ne documente pas le placeholder leger"
grep -q 'INSTALLER-DINOTOFU.cmd' scripts/package_windows_release.sh || fail "installateur Windows joueur absent du packaging"
[[ -x scripts/stage_release_documentation.sh ]] || fail "helper de documentation des packs installateur absent/non exécutable"
grep -q 'INSTALLATION.txt' scripts/stage_release_documentation.sh || fail "documentation des packs installateur non centralisée"
pass "versioning sauvegarde, release GitHub manuelle/auto et packages de distribution protégés"

grep -q 'BUREAU DE CHANGE DE LA GUILDE' src/interface/menu/quest/QuestMenu.cpp || fail "bureau de change de guilde absent"
grep -q 'breakCoinsToLower' src/item/Inventory.cpp || fail "conversion physique des pièces vers rang inférieur absente"
grep -q 'combineCoinsToHigher' src/item/Inventory.cpp || fail "regroupement physique des pièces vers rang supérieur absent"
[[ "$(grep -R --include='*.cpp' -h 'Money::coinScaleText()' src/interface src/core src/item | wc -l)" -eq 1 ]] || fail "légende complète des pièces affichée à plusieurs endroits joueur"
grep -q 'InventorySelection::openCraft(mainPlayer)' src/core/Game.cpp || fail "accès direct à l'artisanat absent du menu Personnage"
pass "bureau de change, légende monétaire unique et accès direct à l'artisanat protégés"

grep -q 'command == "retour"' src/interface/TerminalInterface.cpp || fail "alias texte retour absent des menus terminal"
[[ "$(grep -c 'command == \"aide\"' src/interface/TerminalInterface.cpp)" -ge 2 ]] || fail "aide de sélection terminal absente d'un des deux lecteurs de menu"
grep -q 'Choix disponibles :' src/interface/TerminalInterface.cpp || fail "liste des choix valides absente après erreur de saisie"
grep -q 'activeCombatStatusSummary' src/interface/menu/CombatMenu.cpp || fail "résumé des états actifs absent du menu de combat"
grep -q 'Posture actuelle :' src/interface/menu/CombatMenu.cpp || fail "posture défensive active non visible dans le menu de combat"
pass "sélection terminal tolérante et lisibilité des états de combat protégées"

[[ -f include/story/StoryPrologueMemory.hpp && -f src/story/StoryPrologueMemory.cpp ]] || fail "module du souvenir pré-brume absent"
grep -q 'StoryPrologueMemory::createTemporaryPlayer' src/core/GameStory.cpp || fail "build temporaire avancé non relié au prologue"
grep -q 'StoryPrologueMemory::runPackHunt' src/core/GameStory.cpp || fail "chasse jouable du nouveau prologue absente"
grep -q 'Glacier des Serments froids' src/story/StoryPrologueMemory.cpp || fail "zone existante du prologue non utilisée"
grep -q "Cette bataille n'accorde ni expérience, ni butin, ni entrée de bestiaire" src/core/GameStory.cpp || fail "séparation progression/souvenir non explicitée"
if grep -n -E 'Scarlett|Lorenzo' src/story/StoryPrologueMemory.cpp src/core/GameStory.cpp | grep -v 'assert' | grep -q .; then
    fail "les vrais noms des compagnons fuitent dans le runtime du souvenir"
fi
wait_bg_test "story" "build temporaire, meute ou effacement des compagnons invalide"
pass "nouveau prologue pré-brume, build temporaire et noms effacés testés"

if grep -R -n --include='*.cpp' --include='*.hpp' -E 'innCommonBedCost\(|innSafeRoomCost\(|innWarmMealCost\(|cityVaultMaterialTransferCost\(|routeRewardBudgetForDistance\(' src include | grep -q .; then
    fail "API économie ambiguë : un coût PC exact a perdu son suffixe Copper"
fi
grep -q 'innCommonBedCostCopper' include/economy/EconomyBalance.hpp || fail "coût auberge PC non explicite"
grep -q 'cityVaultMaterialTransferCostCopper' include/economy/EconomyBalance.hpp || fail "transport de coffre PC non explicite"
pass "noms d API économie explicites au checkpoint V3.50.33"

wait_bg_test "special_char" "Willow/Dwarf/Badr ou leurs identités spéciales sont invalides"
grep -q 'names = {"Willow", "Dwarf", "Badr"}' src/combat/encounter/AdventurerGroupEncounter.cpp || fail "trio Willow/Dwarf/Badr absent des rencontres spéciales"
grep -q 'En avant Second' src/character/SpecialCharacterCatalog.cpp || fail "Second absent du profil de Badr"
grep -q 'combat.special.synergy.willow_dwarf' src/combat/action/SpecialCombatEffects.cpp || fail "synergie Willow/Dwarf absente"
grep -q 'combat.special.badr.group_support' src/combat/action/SpecialCombatEffects.cpp || fail "soutien de groupe de Badr absent"
pass "Willow, Dwarf et Badr intégrés comme personnages spéciaux protégés et groupe cohérent"

grep -q 'persistentId' include/item/Item.hpp || fail "identité persistante Item absente"
grep -q 'Mémoire de cet exemplaire' src/interface/menu/EquipmentMenu.cpp || fail "inspection mémoire équipement absente"
grep -q 'Surnom émergent' src/interface/menu/EquipmentMenu.cpp || fail "renommée émergente d'équipement absente"
grep -q 'item_memory_enemy_signature' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "mémoire d'impact sur exemplaire absente"
grep -q 'item_memory_transferred_sale' src/economy/shop/ShopTransactionSystem.cpp || fail "mémoire de vente d'un exemplaire absente"
grep -q 'item_memory_recovered_after_sale' src/economy/shop/ShopTransactionSystem.cpp || fail "mémoire de rachat du même exemplaire absente"
pass "identité, mémoire et renommée d'équipement présentes"

grep -q 'Technique combinée' src/combat/modes/pve/MonsterPveMode.cpp || fail "technique combinée alliée absente"
grep -q 'groupCombinedTechniqueOneTurn' src/combat/modes/pve/MonsterPveMode.cpp || fail "ordre de combo allié absent"
[[ -f include/combat/ally/DuoMasterySystem.hpp && -f src/combat/ally/DuoMasterySystem.cpp ]] || fail "maîtrise des duos non modularisée"
grep -q 'DuoMasterySystem::experience' src/combat/modes/pve/MonsterPveMode.cpp || fail "expérience persistante des duos non reliée au combat"
grep -q 'Relais vital' src/combat/ally/DuoMasterySystem.cpp || fail "combo soutien+soutien absent"
grep -q 'Mur en mouvement' src/combat/ally/DuoMasterySystem.cpp || fail "combo tank+tank absent"
grep -q 'coordination rompue' src/combat/modes/pve/MonsterPveMode.cpp || fail "échec logique des combos absent"
wait_bg_test "duo" "maîtrise/variantes des duos invalides"
pass "techniques combinées et maîtrise de duo modularisées/testées"

[[ -f include/combat/EnemyCombatQueue.hpp && -f src/combat/EnemyCombatQueue.cpp ]] || fail "file ennemie absente"
grep -q 'getSurrenderedEnemyCount' include/combat/EnemyCombatQueue.hpp || fail "état de reddition absent de la file ennemie"
grep -q 'REDDITION ENNEMIE' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "reddition réelle absente du combat"
grep -q 'chocs_de_groupe_ennemis' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "réaction à la chute du meneur absente"
grep -q 'calculateSurrenderedEnemiesReward' src/combat/reward/CombatRewardSystem.cpp || fail "récompense de reddition non séparée"
wait_bg_test "group" "reddition/file ennemie/profils de groupe invalides"
pass "reddition et réactions de groupe ennemies présentes/testées"

wait_bg_test "monster_prep" "compétences préparées/interrompables ennemies invalides"
grep -q 'COMPÉTENCE INTERROMPUE' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "feedback d'interruption ennemi absent"
grep -q 'startPreparedSignature' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "préparation ennemie non reliée au tour"
grep -q 'COUVERTURE DE PRÉPARATION' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "protection de compétence préparée absente"
grep -q 'DefensePostureSystem::reduceIncomingDamage(player' src/combat/system/MonsterPreparedActionSystem.cpp || fail "posture défensive ignorée par les attaques préparées"
pass "compétences ennemies préparées, variantes, interruption, défense et protection de groupe testées"

wait_bg_test "monster_content" "nouveaux monstres de biomes absents du catalogue"
pass "nouvelles familles de monstres de biomes présentes"


wait_bg_test "monster_elite" "densité des élites ambiantes trop élevée en début de jeu"
grep -q '!monster.isElite() && shouldCreateEvolvedMonster' src/combat/wave/WaveGenerator.cpp || fail "une élite ambiante peut de nouveau être sur-évoluée dans une vague normale"
pass "densité des élites ambiantes et empilement élite/évolution protégés"


wait_bg_test "monster_flavor" "descriptions de mort/fuite/reddition par créature invalides"
grep -q 'buildSurrenderLine' src/combat/turn/wave/MonsterWaveCombatTurn.cpp || fail "description de reddition non reliée"
grep -q 'buildDeathLine' src/combat/EnemyCombatQueue.cpp || fail "description de mort non reliée"
pass "mort, fuite et reddition décrites selon la créature"

grep -q 'SERMENT DÉJÀ ROMPU' src/interface/menu/shop/ChurchServiceMenu.cpp || fail "protection de rupture de serment absente"
grep -q 'isChurchOathContractId' src/entity/player/PlayerSkills.cpp || fail "contrats d'église non séparés du loadout"
pass "serments gérés comme contrats hors loadout"

[[ -f include/world/LocalReputationSystem.hpp && -f src/world/LocalReputationSystem.cpp ]] || fail "réputation locale centralisée absente"
grep -q 'canonicalCategoryIsLocalScoped' src/entity/Player.cpp || fail "journal localisé non protégé"
grep -q 'surchargeForScore' src/world/LocalReputationSystem.cpp || fail "conséquences de réputation négative absentes"
grep -q 'vente importante refusée' src/interface/menu/shop/ShopMenu.cpp || fail "réaction commerciale négative absente"
[[ -f include/world/LocalReputationRepairSystem.hpp && -f src/world/LocalReputationRepairSystem.cpp ]] || fail "réhabilitation de réputation locale absente"
grep -q 'LocalReputationRepairMenu::open' src/interface/menu/quest/QuestLocationNpcMenu.cpp || fail "bureau de médiation locale non relié aux lieux"
wait_bg_test "reputation_repair" "réhabilitation de réputation locale invalide"
pass "journal localisé, réputation positive/négative et réhabilitation présents/testés"

grep -q "SUITE DE L'HISTOIRE INDISPONIBLE" src/core/GameStory.cpp || fail "limite histoire absente"
grep -q 'return false;' src/story/StoryCampaign.cpp || fail "verrou développement histoire absent"
pass "limite histoire après introduction chapitre 3 présente"

grep -q 'Mémoire du monde / Rivaux' src/interface/menu/progression/StatisticsMenu.cpp || fail "inspection mémoire monde absente"
pass "inspection mémoire/rivaux disponible"

grep -q 'Coup opportuniste", "Niv. 5' src/combat/CombatActions.cpp || fail "première technique voleur redevenue trop précoce"
grep -q 'Surcharge contrôlée", "Niv. 14' src/combat/CombatActions.cpp || fail "progression arcanique de nouveau compressée"
grep -q 'level >= 5 && className == "Assassin"' src/entity/player/PlayerSkills.cpp || fail "déblocages de classe revenus au niveau 4"
pass "déblocages de classe étalés dans la progression"

! grep -q 'Future IG :' src/world/CityTravelRules.cpp src/interface/menu/quest/QuestWorldMenuSupport.cpp || fail "texte de développement Future IG visible en jeu"
! grep -q 'Effets futurs :' src/entity/player/PlayerSkills.cpp || fail "texte de patch/dev visible dans les serments"
! grep -q 'méta-lore' src/lore/LegendTriggerSystem.cpp || fail "texte méta-lore visible dans les légendes"
! grep -q 'Priorité actuelle du développement' src/story/StoryCampaign.cpp || fail "priorité de développement visible dans l'histoire"
pass "textes gameplay nettoyés des notes de développement évidentes"

echo "Tous les tests Dinotofu sont passés."
