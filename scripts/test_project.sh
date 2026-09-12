#!/usr/bin/env bash
set -euo pipefail

fail() { echo "[FAIL] $1" >&2; exit 1; }
pass() { echo "[OK] $1"; }

VERSION="$(bash ./scripts/get_version.sh)"
[[ -n "$VERSION" ]] || fail "version introuvable"
[[ -x ./output/Dinotofu ]] || fail "output/Dinotofu absent : make test doit dépendre du build"
[[ "$(./output/Dinotofu --version)" == *"V${VERSION}"* ]] || fail "version binaire incohérente"
pass "version ${VERSION} cohérente"

[[ -f CHANGELOG.md ]] || fail "CHANGELOG.md manquant"
grep -q "V${VERSION}" CHANGELOG.md || fail "CHANGELOG sans V${VERSION}"
[[ ! -f PATCHNOTE_DINOTOFU.md && ! -f PATCHNOTE_DINOTOFU_FR.md ]] || fail "anciens PATCHNOTE encore présents"
pass "CHANGELOG remplace les PATCHNOTE"

! grep -q "V3.30.00" README.md || fail "historique de versions revenu dans README.md"
! grep -q "V3.30.00" READMEFR.md || fail "historique de versions revenu dans READMEFR.md"
pass "README sans historique détaillé"

if find src include -type f -name '*.inc' | grep -q .; then
    fail "des fichiers .inc existent encore dans src/include"
fi
pass "aucun .inc dans src/include"

[[ -f include/entity/player/PlayerHistoryTypes.hpp && -f src/entity/player/PlayerHistory.cpp ]] || fail "module mémoire/rivaux absent"
grep -q 'struct PlayerRivalRecord' include/entity/player/PlayerHistoryTypes.hpp || fail "PlayerRivalRecord absent"
grep -q 'recordRivalReturn' src/entity/player/PlayerHistory.cpp || fail "retours de rivaux absents"
pass "mémoire persistante et rivaux présents"

grep -q 'persistentId' include/item/Item.hpp || fail "identité persistante Item absente"
grep -q 'Mémoire de cet exemplaire' src/interface/menu/EquipmentMenu.cpp || fail "inspection mémoire équipement absente"
pass "identité et mémoire d'équipement présentes"

grep -q 'Technique combinée' src/combat/modes/pve/MonsterPveMode.cpp || fail "technique combinée alliée absente"
grep -q 'groupCombinedTechniqueOneTurn' src/combat/modes/pve/MonsterPveMode.cpp || fail "ordre de combo allié absent"
pass "techniques combinées alliées présentes"

grep -q 'SERMENT DÉJÀ ROMPU' src/interface/menu/shop/ShopMenu.cpp || fail "protection de rupture de serment absente"
grep -q 'isChurchOathContractId' src/entity/Player.cpp || fail "contrats d'église non séparés du loadout"
pass "serments gérés comme contrats hors loadout"

[[ -f include/world/LocalReputationSystem.hpp && -f src/world/LocalReputationSystem.cpp ]] || fail "réputation locale centralisée absente"
grep -q 'canonicalCategoryIsLocalScoped' src/entity/Player.cpp || fail "journal localisé non protégé"
pass "journal localisé et réputation centralisée présents"

grep -q 'FIN TEMPORAIRE DU DÉVELOPPEMENT HISTOIRE' src/core/Game.cpp || fail "limite histoire absente"
grep -q 'return false;' src/story/StoryCampaign.cpp || fail "verrou développement histoire absent"
pass "limite histoire après introduction chapitre 3 présente"

grep -q 'Mémoire du monde / Rivaux' src/interface/menu/progression/StatisticsMenu.cpp || fail "inspection mémoire monde absente"
pass "inspection mémoire/rivaux disponible"

echo "Tous les tests Dinotofu sont passés."
