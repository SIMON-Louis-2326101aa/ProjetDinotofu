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
PACKAGE_PATH="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}.zip"

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
    "assetPattern": "Dinotofu-Windows-v*.zip",
    "installDir": r"%USERPROFILE%\ProjetDinotofu",
}
with open(path, "w", encoding="utf-8") as handle:
    json.dump(config, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY_JSON
}

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${PACKAGE_PATH}"

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
    7z a -tzip -mx=9 -mpass=15 -mfb=258 "$(basename "${PACKAGE_PATH}")" "$(basename "${STAGING_DIR}")" \
        -xr!saves -xr!accounts -xr!characters -xr!exported_accounts -xr!import_accounts \
        -xr!*.o -xr!*.d -xr!*.log -xr!*.tmp
)

rm -rf "${STAGING_DIR}"
make clean >/dev/null 2>&1 || true

echo "Release Windows créée (ZIP natif) : ${PACKAGE_PATH}"
