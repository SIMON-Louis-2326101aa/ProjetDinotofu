#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(./scripts/get_version.sh)"
PACKAGE_DIR="release_packages"
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}"
PACKAGE_PATH="${PACKAGE_DIR}/Dinotofu-Linux-v${VERSION}.zip"

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${PACKAGE_PATH}"

make -j"$(nproc 2>/dev/null || echo 2)"

mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
cp README.md READMEFR.md SYSTEMES_PREVUS.txt SPECIAL_CHARACTERS_AND_CHEATS.txt BOSS_VALIDES_DINOTOFU.txt "${STAGING_DIR}/" 2>/dev/null || true
cp tools/linux/DinotofuInstaller.sh "${STAGING_DIR}/Installer-Dinotofu.sh" 2>/dev/null || true
cp tools/linux/DinotofuLauncher.sh "${STAGING_DIR}/DinotofuLauncher.sh" 2>/dev/null || true
cp tools/linux/Lancer-Dinotofu.sh "${STAGING_DIR}/Lancer-Dinotofu.sh" 2>/dev/null || true
cat > "${STAGING_DIR}/dinotofu-installer.config.json" <<JSON
{
  "repo": "${DINOTOFU_REPO:-TON_COMPTE/TON_REPO}",
  "assetPattern": "Dinotofu-Linux-v*.zip",
  "installDir": "~/Downloads/ProjetDinotofu"
}
JSON
mkdir -p "${STAGING_DIR}/output"
cp output/Dinotofu "${STAGING_DIR}/output/Dinotofu"
echo "${VERSION}" > "${STAGING_DIR}/version.txt"
chmod +x "${STAGING_DIR}/output/Dinotofu" "${STAGING_DIR}/Installer-Dinotofu.sh" "${STAGING_DIR}/DinotofuLauncher.sh" "${STAGING_DIR}/Lancer-Dinotofu.sh" || true

(
    cd "${PACKAGE_DIR}"
    zip -r "$(basename "${PACKAGE_PATH}")" "$(basename "${STAGING_DIR}")" \
        -x "*/assets/saves/*" \
        -x "*/saves/*" \
        -x "*/accounts/*" \
        -x "*/characters/*" \
        -x "*/exported_accounts/*" \
        -x "*/import_accounts/*" \
        -x "*.o" \
        -x "*.d" \
        -x "*.log" \
        -x "*.tmp"
)

echo "Release Linux créée : ${PACKAGE_PATH}"
