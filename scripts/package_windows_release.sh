#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(./scripts/get_version.sh)"
REPO_NAME="${DINOTOFU_REPO:-TON_COMPTE/TON_REPO}"
PACKAGE_DIR="release_packages"
STAGING_DIR="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}"
INSTALLER_DIR="${PACKAGE_DIR}/DinotofuInstaller-Windows-v${VERSION}"
GAME_ZIP="${PACKAGE_DIR}/Dinotofu-Windows-v${VERSION}.zip"
INSTALLER_ZIP="${PACKAGE_DIR}/DinotofuInstaller-Windows-v${VERSION}.zip"

CROSS_CXX="${CXX:-x86_64-w64-mingw32-g++}"

mkdir -p "${PACKAGE_DIR}"
rm -rf "${STAGING_DIR}" "${INSTALLER_DIR}" "${GAME_ZIP}" "${INSTALLER_ZIP}"

if ! command -v "${CROSS_CXX}" >/dev/null 2>&1; then
    echo "Compilateur Windows introuvable : ${CROSS_CXX}" >&2
    echo "Sur Ubuntu/GitHub Actions : sudo apt-get install -y mingw-w64" >&2
    exit 1
fi

make clean >/dev/null 2>&1 || true
make -j"$(nproc 2>/dev/null || echo 2)" \
    CXX="${CROSS_CXX}" \
    APP_NAME="Dinotofu.exe" \
    LDFLAGS="-static -static-libgcc -static-libstdc++"

mkdir -p "${STAGING_DIR}"
cp -r assets "${STAGING_DIR}/" 2>/dev/null || true
cp README.md READMEFR.md SYSTEMES_PREVUS.txt SPECIAL_CHARACTERS_AND_CHEATS.txt BOSS_VALIDES_DINOTOFU.txt "${STAGING_DIR}/" 2>/dev/null || true
cp output/Dinotofu.exe "${STAGING_DIR}/Dinotofu.exe"
cp tools/windows/DinotofuLauncher.ps1 "${STAGING_DIR}/DinotofuLauncher.ps1"
cp tools/windows/Lancer-Dinotofu.cmd "${STAGING_DIR}/Lancer-Dinotofu.cmd"
cat > "${STAGING_DIR}/dinotofu-installer.config.json" <<JSON
{
  "repo": "${REPO_NAME}",
  "assetPattern": "Dinotofu-Windows-v*.zip",
  "installDir": "%USERPROFILE%\\Downloads\\ProjetDinotofu"
}
JSON
echo "${VERSION}" > "${STAGING_DIR}/version.txt"

(
    cd "${PACKAGE_DIR}"
    zip -r "$(basename "${GAME_ZIP}")" "$(basename "${STAGING_DIR}")" \
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

mkdir -p "${INSTALLER_DIR}"
cp tools/windows/DinotofuInstaller.ps1 "${INSTALLER_DIR}/DinotofuInstaller.ps1"
cp tools/windows/DinotofuLauncher.ps1 "${INSTALLER_DIR}/DinotofuLauncher.ps1"
cp tools/windows/Installer-Dinotofu.cmd "${INSTALLER_DIR}/Installer-Dinotofu.cmd"
cat > "${INSTALLER_DIR}/dinotofu-installer.config.json" <<JSON
{
  "repo": "${REPO_NAME}",
  "assetPattern": "Dinotofu-Windows-v*.zip",
  "installDir": "%USERPROFILE%\\Downloads\\ProjetDinotofu"
}
JSON
cat > "${INSTALLER_DIR}/LISEZ-MOI.txt" <<TXT
Dinotofu Installer Windows

1. Double-clique sur Installer-Dinotofu.cmd.
2. Le script telecharge la derniere release Windows depuis GitHub.
3. Par defaut, il installe Dinotofu dans %%USERPROFILE%%\Downloads\ProjetDinotofu.
4. Tu peux choisir un autre dossier parent, mais le dossier final s'appellera toujours ProjetDinotofu.
5. Il cree un raccourci sur le bureau.
6. Le launcher verifie les mises a jour au lancement.

Aucun WSL n'est requis pour la version Windows. La release Windows doit contenir Dinotofu.exe.
Si Windows bloque l'execution, clic droit sur le fichier, Proprietes, puis Debloquer si l'option existe.
TXT

(
    cd "${PACKAGE_DIR}"
    zip -r "$(basename "${INSTALLER_ZIP}")" "$(basename "${INSTALLER_DIR}")" \
        -x "*.o" \
        -x "*.d" \
        -x "*.log" \
        -x "*.tmp"
)

make clean >/dev/null 2>&1 || true

echo "Release Windows creee : ${GAME_ZIP}"
echo "Installer Windows cree : ${INSTALLER_ZIP}"
