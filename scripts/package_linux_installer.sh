#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(./scripts/get_version.sh)"
REPO_NAME="${DINOTOFU_REPO:-TON_COMPTE/TON_REPO}"
PACKAGE_DIR="release_packages"
INSTALLER_DIR="${PACKAGE_DIR}/DinotofuInstaller-Linux-v${VERSION}"
INSTALLER_ZIP="${PACKAGE_DIR}/DinotofuInstaller-Linux-v${VERSION}.zip"

mkdir -p "${PACKAGE_DIR}"
rm -rf "${INSTALLER_DIR}" "${INSTALLER_ZIP}"
mkdir -p "${INSTALLER_DIR}"

cp tools/linux/DinotofuInstaller.sh "${INSTALLER_DIR}/DinotofuInstaller.sh"
cp tools/linux/DinotofuLauncher.sh "${INSTALLER_DIR}/DinotofuLauncher.sh"
cp tools/linux/Installer-Dinotofu.sh "${INSTALLER_DIR}/Installer-Dinotofu.sh"
cp tools/linux/Lancer-Dinotofu.sh "${INSTALLER_DIR}/Lancer-Dinotofu.sh"
chmod +x "${INSTALLER_DIR}"/*.sh

cat > "${INSTALLER_DIR}/dinotofu-installer.config.json" <<JSON
{
  "repo": "${REPO_NAME}",
  "assetPattern": "Dinotofu-Linux-v*.zip",
  "installDir": "~/Downloads/ProjetDinotofu"
}
JSON

cat > "${INSTALLER_DIR}/LISEZ-MOI.txt" <<TXT
Dinotofu Installer Linux

1. Ouvre un terminal dans ce dossier.
2. Lance : ./Installer-Dinotofu.sh
3. Le script telecharge la derniere release Linux depuis GitHub.
4. Par defaut, il installe Dinotofu dans ~/Downloads/ProjetDinotofu.
5. Tu peux choisir un autre dossier parent, mais le dossier final s'appellera toujours ProjetDinotofu.
6. Il cree un raccourci Linux si possible.
7. Le launcher verifie les mises a jour au lancement.

Dependances generalement deja presentes : bash, curl, unzip, python3.
Sur Debian/Ubuntu si besoin : sudo apt install curl unzip python3
TXT

(
    cd "${PACKAGE_DIR}"
    zip -r "$(basename "${INSTALLER_ZIP}")" "$(basename "${INSTALLER_DIR}")" \
        -x "*.o" \
        -x "*.d" \
        -x "*.log" \
        -x "*.tmp"
)

echo "Installer Linux créé : ${INSTALLER_ZIP}"
