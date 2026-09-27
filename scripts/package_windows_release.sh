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

VERSION="$(bash ./scripts/get_version.sh)"
REPO_NAME="$(detect_repo_name)"
PACKAGE_DIR="release_packages"
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}"
GAME_ARCHIVE="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}.7z"
INSTALLER_STAGING_DIR="${PACKAGE_DIR}/Installer-Dinotofu-Windows-v${VERSION}"
INSTALLER_ARCHIVE="${PACKAGE_DIR}/Installer-Dinotofu-Windows-v${VERSION}.7z"

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
    "installDir": r"%USERPROFILE%\ProjetDinotofu",
}
with open(path, "w", encoding="utf-8") as handle:
    json.dump(config, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY_JSON
}

generate_bootstrap_cmd() {
    local target_file="$1"
    python3 - "$target_file" "$REPO_NAME" "$VERSION" <<'PY_CMD'
from pathlib import Path
import sys

path = Path(sys.argv[1])
repo = sys.argv[2]
version = sys.argv[3]
tag_url = f"https://raw.githubusercontent.com/{repo}/v{version}/tools/windows/DinotofuInstaller.ps1"
main_url = f"https://raw.githubusercontent.com/{repo}/main/tools/windows/DinotofuInstaller.ps1"
text = rf'''@echo off
chcp 65001 >nul
setlocal
set "DINOTOFU_REPO={repo}"
set "DINOTOFU_INSTALLER_TMP=%TEMP%\DinotofuInstaller-v{version}-%RANDOM%.ps1"

echo Telechargement de l'installateur Dinotofu...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$urls=@('{tag_url}','{main_url}'); $ok=$false; foreach($u in $urls) {{ try {{ Invoke-WebRequest -UseBasicParsing -Uri $u -OutFile $env:DINOTOFU_INSTALLER_TMP; $ok=$true; break }} catch {{ }} }}; if(-not $ok) {{ exit 1 }}"
if errorlevel 1 (
  echo Impossible de telecharger le moteur d'installation depuis GitHub.
  echo Verifie ta connexion puis relance ce fichier.
  pause
  exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%DINOTOFU_INSTALLER_TMP%" -Repo "%DINOTOFU_REPO%" -AssetPattern "Dinotofu-Windows-v*.7z"
set "DINOTOFU_RESULT=%ERRORLEVEL%"
del /q "%DINOTOFU_INSTALLER_TMP%" >nul 2>nul
if not "%DINOTOFU_RESULT%"=="0" pause
exit /b %DINOTOFU_RESULT%
'''
path.write_text(text, encoding="utf-8", newline="\r\n")
PY_CMD
}

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${GAME_ARCHIVE}" "${INSTALLER_STAGING_DIR}" "${INSTALLER_ARCHIVE}"

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

# -----------------------------------------------------------------------------
# Technical game payload. Historical name remains for old launchers/updaters.
# -----------------------------------------------------------------------------
mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
mkdir -p "${STAGING_DIR}/tools"
cp -r tools/gui "${STAGING_DIR}/tools/gui" 2>/dev/null || true
mkdir -p "${STAGING_DIR}/Documentation"
bash ./scripts/stage_release_documentation.sh "${STAGING_DIR}/Documentation" "Windows"
cp SYSTEMES_PREVUS.txt PERSONNAGES_SPECIAUX_DINOTOFU.txt BOSS_DINOTOFU.txt TITRES_DINOTOFU.txt HISTOIRE_PREPARATION_DINOTOFU.txt "${STAGING_DIR}/Documentation/" 2>/dev/null || true
rm -rf "${STAGING_DIR}/assets/saves"
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
write_installer_config_json "${STAGING_DIR}/dinotofu-installer.config.json"
echo "${VERSION}" > "${STAGING_DIR}/version.txt"

(
    cd "${PACKAGE_DIR}"
    7z a -t7z -m0=lzma2 -mx=9 -ms=on "$(basename "${GAME_ARCHIVE}")" "$(basename "${STAGING_DIR}")" \
        -xr!saves -xr!accounts -xr!characters -xr!exported_accounts -xr!import_accounts \
        -xr!*.o -xr!*.d -xr!*.log -xr!*.tmp
)

# -----------------------------------------------------------------------------
# Clean player-facing installer pack: exactly one installer + Documentation/.
# The CMD bootstraps the PowerShell installer from the release tag/main and then
# downloads the technical payload above.
# -----------------------------------------------------------------------------
mkdir -p "${INSTALLER_STAGING_DIR}/Documentation"
generate_bootstrap_cmd "${INSTALLER_STAGING_DIR}/Installer-Dinotofu.cmd"
bash ./scripts/stage_release_documentation.sh "${INSTALLER_STAGING_DIR}/Documentation" "Windows"
cat > "${INSTALLER_STAGING_DIR}/Documentation/LISEZ-MOI.txt" <<TXT
DINOTOFU Windows V${VERSION}

1. Decompresse ce pack.
2. Double-clique sur Installer-Dinotofu.cmd.
3. Le bootstrap recupere le moteur d'installation depuis ${REPO_NAME}, puis
   telecharge le payload Dinotofu-Windows-v*.7z, preserve les sauvegardes connues
   et cree/met a jour ProjetDinotofu.

La racine de ce pack est volontairement propre : un seul fichier d'installation
et le dossier Documentation/.
TXT

# Guard the player-facing installer layout against future root clutter.
mapfile -t installer_root_entries < <(find "${INSTALLER_STAGING_DIR}" -mindepth 1 -maxdepth 1 -printf '%f\n' | sort)
[[ "${#installer_root_entries[@]}" -eq 2 ]] || { echo "Pack installateur invalide : la racine doit contenir exactement 2 entrees." >&2; printf '%s\n' "${installer_root_entries[@]}" >&2; exit 1; }
printf '%s\n' "${installer_root_entries[@]}" | grep -Fxq 'Installer-Dinotofu.cmd' || { echo "Fichier installateur manquant : Installer-Dinotofu.cmd" >&2; exit 1; }
printf '%s\n' "${installer_root_entries[@]}" | grep -Fxq 'Documentation' || { echo "Dossier Documentation manquant." >&2; exit 1; }
if find "${INSTALLER_STAGING_DIR}/Documentation" -type f ! -name '*.txt' | grep -q .; then
    echo "Documentation du pack installateur : seuls les .txt sont autorises." >&2
    exit 1
fi

(
    cd "${PACKAGE_DIR}"
    7z a -t7z -m0=lzma2 -mx=9 -ms=on "$(basename "${INSTALLER_ARCHIVE}")" "$(basename "${INSTALLER_STAGING_DIR}")"
)

rm -rf "${STAGING_DIR}" "${INSTALLER_STAGING_DIR}"
make clean >/dev/null 2>&1 || true

echo "Payload Windows cree : ${GAME_ARCHIVE}"
echo "Pack installateur Windows cree : ${INSTALLER_ARCHIVE}"
