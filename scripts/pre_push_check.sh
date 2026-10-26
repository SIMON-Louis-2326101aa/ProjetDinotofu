#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

info() { echo "[PRE-PUSH] $1"; }

info "1/5 - Nettoyage des artefacts générés"
make clean >/dev/null 2>&1 || true

info "2/5 - Validation de l'arborescence source"
if [[ -s assets/branding/dinotofu_logo.png && -s assets/branding/branding_manifest.json ]]; then
    bash ./scripts/validate_release_tree.sh
    info "Branding officiel présent : contrôle strict effectué."
else
    bash ./scripts/validate_release_tree.sh --skip-branding
    info "Branding officiel absent de cette copie : le push source est validé, mais une release/installateur complet devra restaurer les assets de branding avant publication."
fi

info "3/5 - Validation de tous les fichiers JSON"
python3 - <<'PY'
from pathlib import Path
import json
import sys

errors = []
for path in sorted(Path('.').rglob('*.json')):
    if any(part in {'.git', 'build', 'output', 'release_packages', '__pycache__'} for part in path.parts):
        continue
    try:
        json.loads(path.read_text(encoding='utf-8'))
    except Exception as exc:
        errors.append(f"{path}: {exc}")

if errors:
    for error in errors:
        print(f"[ERREUR JSON] {error}", file=sys.stderr)
    raise SystemExit(1)
print(f"JSON valides : {sum(1 for p in Path('.').rglob('*.json') if '.git' not in p.parts)} fichier(s) inspecté(s).")
PY

info "4/5 - Build + suite complète Dinotofu"
DINOTOFU_TEST_JOBS="${DINOTOFU_TEST_JOBS:-6}" make test

info "5/5 - Vérifications Git si le projet est déjà dans un dépôt"
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    # Markdown uses three trailing spaces by project convention; exclude it from Git's generic trailing-whitespace check.
    git diff --check -- . ':(exclude)**/*.md' ':(exclude)*.md'
    git diff --cached --check -- . ':(exclude)**/*.md' ':(exclude)*.md'
    if git status --porcelain | grep -E '(^|/)(saves|accounts|characters|exported_accounts|import_accounts)/|\.save$|\.sav$|\.pem$|id_(rsa|ed25519)' >/dev/null 2>&1; then
        echo "[ERREUR] Git voit un fichier local sensible ou une sauvegarde privée." >&2
        git status --short >&2
        exit 1
    fi
    info "Git diff --check (hors Markdown conventionnel) : OK."
else
    info "Aucun dépôt .git dans cette copie : contrôles Git spécifiques ignorés."
fi

info "PRÊT POUR PUSH SOURCE : tous les contrôles disponibles sont passés."
