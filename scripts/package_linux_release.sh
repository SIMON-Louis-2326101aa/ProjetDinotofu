#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(./scripts/get_version.sh)"
PACKAGE_DIR="release_packages"
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}"
PACKAGE_PATH="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}.7z"

write_installer_config_json() {
    local target_file="$1"
    python3 - "$target_file" "${DINOTOFU_REPO:-TON_COMPTE/TON_REPO}" <<'PY_JSON'
import json
import sys

path = sys.argv[1]
repo = sys.argv[2]
config = {
    "repo": repo,
    "assetPattern": "Dinotofu-Linux-v*.7z",
    "installDir": "~/Downloads/ProjetDinotofu",
}
with open(path, "w", encoding="utf-8") as handle:
    json.dump(config, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY_JSON
}

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${PACKAGE_PATH}"

make clean >/dev/null 2>&1 || true
make -j"$(nproc 2>/dev/null || echo 2)" TARGET_ARCH="${TARGET_ARCH:-x86-64}" OPT_LEVEL="${OPT_LEVEL:--O3}"

mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
cp README.md READMEFR.md PATCHNOTE_DINOTOFU.md PATCHNOTE_DINOTOFU_FR.md SYSTEMES_PREVUS.txt PERSONNAGES_SPECIAUX_DINOTOFU.txt CHEATS_DINOTOFU.txt BOSS_DINOTOFU.txt TITRES_DINOTOFU.txt HISTOIRE_PREPARATION_DINOTOFU.txt "${STAGING_DIR}/" 2>/dev/null || true
cp tools/linux/DinotofuInstaller.sh "${STAGING_DIR}/DinotofuInstaller.sh" 2>/dev/null || true
cp tools/linux/DinotofuInstaller.sh "${STAGING_DIR}/Installer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/DinotofuLauncher.sh "${STAGING_DIR}/DinotofuLauncher.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu.sh "${STAGING_DIR}/Lancer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu-Terminal.sh "${STAGING_DIR}/Lancer-Dinotofu-Terminal.sh" 2>/dev/null || true
mkdir -p "${STAGING_DIR}/tools"
cp -r tools/gui "${STAGING_DIR}/tools/gui"
write_installer_config_json "${STAGING_DIR}/dinotofu-installer.config.json"
mkdir -p "${STAGING_DIR}/output"
cp output/Dinotofu "${STAGING_DIR}/output/Dinotofu"
cp output/Dinotofu "${STAGING_DIR}/Dinotofu"
echo "${VERSION}" > "${STAGING_DIR}/version.txt"

cat > "${STAGING_DIR}/LISEZ-MOI.txt" <<TXT
Dinotofu Linux (Version Portable)

Ce pack contient le jeu complet directement pret a l'emploi !

Lancement direct (sans installation) :
1. Ouvre un terminal dans ce dossier.
2. Lance : ./Lancer-Dinotofu.sh
   (Ou ./Lancer-Dinotofu-Terminal.sh pour forcer le mode terminal).

Installation optionnelle :
Si tu souhaites installer le jeu dans un autre dossier et creer des raccourcis bureau/applications,
lance : ./Installer-Dinotofu.sh
TXT

chmod +x "${STAGING_DIR}/output/Dinotofu" "${STAGING_DIR}/Dinotofu" "${STAGING_DIR}/Installer-Dinotofu.sh" "${STAGING_DIR}/DinotofuInstaller.sh" "${STAGING_DIR}/DinotofuLauncher.sh" "${STAGING_DIR}/Lancer-Dinotofu.sh" "${STAGING_DIR}/Lancer-Dinotofu-Terminal.sh" || true

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
