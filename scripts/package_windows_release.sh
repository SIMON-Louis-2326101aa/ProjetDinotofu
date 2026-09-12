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
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}"
GAME_ARCHIVE="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}.7z"

CROSS_CXX="${CXX:-x86_64-w64-mingw32-g++}"

write_installer_config_json() {
    local target_file="$1"
    python3 - "$target_file" "$REPO_NAME" <<'PY_JSON'
import json
import sys

path = sys.argv[1]
repo = sys.argv[2]
config = {
    "repo": repo,
    "assetPattern": "Dinotofu-Windows-v*.7z",
    "installDir": r"%USERPROFILE%\Downloads\ProjetDinotofu",
}
with open(path, "w", encoding="utf-8") as handle:
    json.dump(config, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY_JSON
}

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${GAME_ARCHIVE}"

if ! command -v "${CROSS_CXX}" >/dev/null 2>&1; then
    echo "Compilateur Windows introuvable : ${CROSS_CXX}" >&2
    echo "Sur Ubuntu/GitHub Actions : sudo apt-get install -y mingw-w64" >&2
    echo "Sur Arch/CachyOS : sudo pacman -S mingw-w64-gcc" >&2
    exit 1
fi

make clean >/dev/null 2>&1 || true
make -j"$(nproc 2>/dev/null || echo 2)" \
    CXX="${CROSS_CXX}" \
    APP_NAME="Dinotofu.exe" \
    TARGET_ARCH="${TARGET_ARCH:-x86-64}" \
    OPT_LEVEL="${OPT_LEVEL:--O3}" \
    CXXFLAGS="-std=c++17 ${OPT_LEVEL:--O3} -march=${TARGET_ARCH:-x86-64} -pipe -Wall -Wextra -Iinclude -MMD -MP -finput-charset=UTF-8 -fexec-charset=UTF-8" \
    LDFLAGS="-s -static -static-libgcc -static-libstdc++"

mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
cp README.md READMEFR.md CHANGELOG.md SYSTEMES_PREVUS.txt PERSONNAGES_SPECIAUX_DINOTOFU.txt CHEATS_DINOTOFU.txt BOSS_DINOTOFU.txt TITRES_DINOTOFU.txt HISTOIRE_PREPARATION_DINOTOFU.txt "${STAGING_DIR}/" 2>/dev/null || true
cp output/Dinotofu.exe "${STAGING_DIR}/Dinotofu.exe"
local_strip="${CROSS_CXX%g++}strip"
if command -v "${local_strip}" >/dev/null 2>&1; then
    "${local_strip}" --strip-all "${STAGING_DIR}/Dinotofu.exe" 2>/dev/null || true
elif command -v strip >/dev/null 2>&1; then
    strip --strip-all "${STAGING_DIR}/Dinotofu.exe" 2>/dev/null || true
fi
cp tools/windows/DinotofuInstaller.ps1 "${STAGING_DIR}/DinotofuInstaller.ps1"
cp tools/windows/Installer-Dinotofu.cmd "${STAGING_DIR}/Installer-Dinotofu.cmd"
cp tools/windows/DinotofuLauncher.ps1 "${STAGING_DIR}/DinotofuLauncher.ps1"
cp tools/windows/Lancer-Dinotofu.cmd "${STAGING_DIR}/Lancer-Dinotofu.cmd"
cp tools/windows/Lancer-Dinotofu-Terminal.cmd "${STAGING_DIR}/Lancer-Dinotofu-Terminal.cmd"
mkdir -p "${STAGING_DIR}/tools"
cp -r tools/gui "${STAGING_DIR}/tools/gui"
write_installer_config_json "${STAGING_DIR}/dinotofu-installer.config.json"
echo "${VERSION}" > "${STAGING_DIR}/version.txt"

cat > "${STAGING_DIR}/LISEZ-MOI.txt" <<TXT
============================================================
 DINOTOFU Windows (Version Portable)
============================================================

Ce pack contient le jeu complet directement pret a l'emploi !

Lancement direct (sans installation) :
Double-clique sur "Lancer-Dinotofu.cmd" (ou directement "Dinotofu.exe").

Creation des raccourcis bureau (optionnel) :
Si tu souhaites ajouter des raccourcis sur ton bureau,
double-clique sur "Installer-Dinotofu.cmd".

Remarques :
- Aucun WSL n'est requis.
- Si Windows bloque un script : Clic droit sur le fichier -> Proprietes -> Cocher 'Debloquer'.
TXT

cat > "${STAGING_DIR}/README.txt" <<TXT
============================================================
 DINOTOFU Windows (Portable Edition)
============================================================

This package contains the complete game, ready to play immediately!

Quick Play (No installation required) :
Double-click "Lancer-Dinotofu.cmd" (or directly "Dinotofu.exe").

Create Desktop Shortcuts (Optional) :
If you want to create shortcuts on your desktop,
double-click "Installer-Dinotofu.cmd".

Notes :
- No WSL required.
- If Windows blocks execution: Right-click the file -> Properties -> Check 'Unblock'.
TXT

(
    cd "${PACKAGE_DIR}"
    7z a -t7z -m0=lzma2 -mx=9 -ms=on "$(basename "${GAME_ARCHIVE}")" "$(basename "${STAGING_DIR}")" \
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
make clean >/dev/null 2>&1 || true

echo "Release Windows creee : ${GAME_ARCHIVE}"
