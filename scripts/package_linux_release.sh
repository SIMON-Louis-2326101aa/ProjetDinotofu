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
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}"
PACKAGE_PATH="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}-TECHNICAL-PAYLOAD.7z"
INSTALLER_STAGING_DIR="${PACKAGE_DIR}/INSTALLER-DINOTOFU-LINUX-v${VERSION}"
INSTALLER_PACKAGE_PATH="${PACKAGE_DIR}/INSTALLER-DINOTOFU-LINUX-v${VERSION}.7z"

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
rm -rf "${STAGING_DIR}" "${PACKAGE_PATH}" "${INSTALLER_STAGING_DIR}" "${INSTALLER_PACKAGE_PATH}"

make clean >/dev/null 2>&1 || true
make -j"$(nproc 2>/dev/null || echo 2)" TARGET_ARCH="${TARGET_ARCH:-x86-64}" OPT_LEVEL="${OPT_LEVEL:--O3}" LDFLAGS="-static-libstdc++ -static-libgcc -s"

# -----------------------------------------------------------------------------
# Technical game payload. The historical Dinotofu-<OS>-v* prefix is preserved for old launcher/updater wildcard compatibility; the visible suffix marks it as technical.
# -----------------------------------------------------------------------------
mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
mkdir -p "${STAGING_DIR}/tools"
cp -r tools/gui "${STAGING_DIR}/tools/gui" 2>/dev/null || true
mkdir -p "${STAGING_DIR}/Documentation"
bash ./scripts/stage_release_documentation.sh "${STAGING_DIR}/Documentation" "Linux"
cp SYSTEMES_PREVUS.txt PERSONNAGES_SPECIAUX_DINOTOFU.txt BOSS_DINOTOFU.txt TITRES_DINOTOFU.txt HISTOIRE_PREPARATION_DINOTOFU.txt "${STAGING_DIR}/Documentation/" 2>/dev/null || true
rm -rf "${STAGING_DIR}/assets/saves"
cp tools/linux/DinotofuInstaller.sh "${STAGING_DIR}/Installer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/DinotofuLauncher.sh "${STAGING_DIR}/DinotofuLauncher.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu.sh "${STAGING_DIR}/Lancer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu-Terminal.sh "${STAGING_DIR}/Lancer-Dinotofu-Terminal.sh" 2>/dev/null || true
write_installer_config_json "${STAGING_DIR}/dinotofu-installer.config.json"
cp output/Dinotofu "${STAGING_DIR}/Dinotofu"
if command -v strip >/dev/null 2>&1; then
    strip --strip-all "${STAGING_DIR}/Dinotofu" 2>/dev/null || true
fi
echo "${VERSION}" > "${STAGING_DIR}/version.txt"
chmod +x "${STAGING_DIR}/Dinotofu" "${STAGING_DIR}/Installer-Dinotofu.sh" "${STAGING_DIR}/DinotofuLauncher.sh" "${STAGING_DIR}/Lancer-Dinotofu.sh" "${STAGING_DIR}/Lancer-Dinotofu-Terminal.sh" || true

(
    cd "${PACKAGE_DIR}"
    7z a -t7z -m0=lzma2 -mx=9 -ms=on "$(basename "${PACKAGE_PATH}")" "$(basename "${STAGING_DIR}")" \
        -xr!saves -xr!accounts -xr!characters -xr!exported_accounts -xr!import_accounts \
        -xr!*.o -xr!*.d -xr!*.log -xr!*.tmp
)

rm -rf "${STAGING_DIR}"

# -----------------------------------------------------------------------------
# Player-facing installer pack: installer only + text documentation.
# -----------------------------------------------------------------------------
mkdir -p "${INSTALLER_STAGING_DIR}/Documentation"
bash ./scripts/stage_release_documentation.sh "${INSTALLER_STAGING_DIR}/Documentation" "Linux" "installer"
cp tools/linux/DinotofuInstaller.sh "${INSTALLER_STAGING_DIR}/INSTALLER-DINOTOFU.sh"
chmod +x "${INSTALLER_STAGING_DIR}/INSTALLER-DINOTOFU.sh"

if find "${INSTALLER_STAGING_DIR}/Documentation" -type f ! -name '*.txt' | grep -q .; then
    echo "Erreur : le dossier Documentation du pack installateur Linux contient autre chose que des .txt" >&2
    exit 1
fi
if [[ "$(find "${INSTALLER_STAGING_DIR}" -mindepth 1 -maxdepth 1 | wc -l)" -ne 2 ]]; then
    echo "Erreur : le pack installateur Linux doit contenir exactement INSTALLER-DINOTOFU.sh + Documentation/" >&2
    exit 1
fi
(
    cd "${INSTALLER_STAGING_DIR}"
    7z a -t7z -m0=lzma2 -mx=9 -ms=on "../$(basename "${INSTALLER_PACKAGE_PATH}")" INSTALLER-DINOTOFU.sh Documentation >/dev/null
)
rm -rf "${INSTALLER_STAGING_DIR}"

make clean >/dev/null 2>&1 || true

echo "Payload Linux créé : ${PACKAGE_PATH}"
echo "Pack installateur Linux créé : ${INSTALLER_PACKAGE_PATH}"
