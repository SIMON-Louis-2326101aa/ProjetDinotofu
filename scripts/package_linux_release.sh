#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

detect_repo_name() {
    if [[ -n "${DINOTOFU_REPO:-}" ]]; then
        echo "${DINOTOFU_REPO}"
        return
    fi

    local origin_url
    origin_url="$(git config --get remote.origin.url 2>/dev/null || true)"
    if [[ -n "${origin_url}" ]]; then
        local parsed
        parsed="$(echo "${origin_url}" | sed -E 's/.*github\.com[:\/]//; s/\.git$//')"
        if [[ "${parsed}" =~ ^[^/]+/[^/]+$ ]]; then
            echo "${parsed}"
            return
        fi
    fi

    echo "SIMON-Louis-2326101aa/ProjetDinotofu"
}

VERSION="$(./scripts/get_version.sh)"
REPO_NAME="$(detect_repo_name)"
PACKAGE_DIR="release_packages"
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}"
PACKAGE_PATH="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}.7z"

write_installer_config_json() {
    local target_file="$1"
    python3 - "$target_file" "$REPO_NAME" <<'PY_JSON'
import json
import sys

path = sys.argv[1]
repo = sys.argv[2]
config = {
    "repo": repo,
    "assetPattern": "Dinotofu-Linux-v*.7z",
    "installDir": "~/ProjetDinotofu",
}
with open(path, "w", encoding="utf-8") as handle:
    json.dump(config, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY_JSON
}

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${PACKAGE_PATH}"

make clean >/dev/null 2>&1 || true
make -j"$(nproc 2>/dev/null || echo 2)" TARGET_ARCH="${TARGET_ARCH:-x86-64}" OPT_LEVEL="${OPT_LEVEL:--O3}" LDFLAGS="-s"

mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
cp README.md READMEFR.md CHANGELOG.md SYSTEMES_PREVUS.txt PERSONNAGES_SPECIAUX_DINOTOFU.txt CHEATS_DINOTOFU.txt BOSS_DINOTOFU.txt TITRES_DINOTOFU.txt HISTOIRE_PREPARATION_DINOTOFU.txt "${STAGING_DIR}/" 2>/dev/null || true
cp tools/linux/DinotofuInstaller.sh "${STAGING_DIR}/Installer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/DinotofuLauncher.sh "${STAGING_DIR}/DinotofuLauncher.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu.sh "${STAGING_DIR}/Lancer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu-Terminal.sh "${STAGING_DIR}/Lancer-Dinotofu-Terminal.sh" 2>/dev/null || true
mkdir -p "${STAGING_DIR}/tools"
cp -r tools/gui "${STAGING_DIR}/tools/gui"
write_installer_config_json "${STAGING_DIR}/dinotofu-installer.config.json"
cp output/Dinotofu "${STAGING_DIR}/Dinotofu"
if command -v strip >/dev/null 2>&1; then
    strip --strip-all "${STAGING_DIR}/Dinotofu" 2>/dev/null || true
fi
echo "${VERSION}" > "${STAGING_DIR}/version.txt"

cat > "${STAGING_DIR}/LISEZ-MOI.txt" <<TXT
============================================================
 DINOTOFU Linux (Version Portable)
============================================================

Ce pack contient le jeu complet directement pret a l'emploi !

Lancement direct (sans installation) :
1. Ouvre un terminal dans ce dossier.
2. Lance : ./Lancer-Dinotofu.sh
   (Ou ./Lancer-Dinotofu-Terminal.sh pour forcer le mode terminal, ou directement ./Dinotofu).

Creation des raccourcis bureau/applications (optionnel) :
Si tu souhaites ajouter des raccourcis sur ton bureau,
lance : ./Installer-Dinotofu.sh
TXT

cat > "${STAGING_DIR}/README.txt" <<TXT
============================================================
 DINOTOFU Linux (Portable Edition)
============================================================

This package contains the complete game, ready to play immediately!

Quick Play (No installation required) :
1. Open a terminal in this directory.
2. Run: ./Lancer-Dinotofu.sh
   (Or ./Lancer-Dinotofu-Terminal.sh to force terminal mode, or directly ./Dinotofu).

Create Desktop/Application Shortcuts (Optional) :
If you want to create desktop/application shortcuts,
run: ./Installer-Dinotofu.sh
TXT

chmod +x "${STAGING_DIR}/Dinotofu" "${STAGING_DIR}/Installer-Dinotofu.sh" "${STAGING_DIR}/DinotofuLauncher.sh" "${STAGING_DIR}/Lancer-Dinotofu.sh" "${STAGING_DIR}/Lancer-Dinotofu-Terminal.sh" || true

(
    cd "${PACKAGE_DIR}"
    7z a -t7z -m0=lzma2 -mx=9 -ms=on "$(basename "${PACKAGE_PATH}")" "$(basename "${STAGING_DIR}")" \
        -xr!saves \
        -xr!accounts \
        -xr!characters \
        -xr!exported_accounts \
        -xr!import_accounts \
        -xr!*.o \
        -xr!*.d \
        -xr!*.log \
        -xr!*.tmp
)

rm -rf "${STAGING_DIR}"

echo "Release Linux créée : ${PACKAGE_PATH}"
