#!/usr/bin/env python3
"""
Bump Dinotofu's game version across the project (VersionInfo.cpp, READMEs, changelogs, manifests).

Interactive usage (recommended):
  python3 scripts/bump_version.py           # Augmente le patch (+1, mode par défaut)
  python3 scripts/bump_version.py patch     # Augmente le patch (+1, ex: 3.50.12 -> 3.50.13)
  python3 scripts/bump_version.py minor     # Augmente la version mineure (+1, patch=0, ex: 3.50.12 -> 3.51.00)
  python3 scripts/bump_version.py major     # Augmente la version majeure (+1, minor=0, patch=0, ex: 3.50.12 -> 4.00.00)
  python3 scripts/bump_version.py 3.50.13   # Définit une version explicite (ex: 3.50.13)

The interactive mode also asks whether to:
- change the internal save schema version (`saveVersion`);
- make the new game version a mandatory important-save checkpoint (`importantSaveUpdateVersion`).

Automation flags:
  --save-schema keep|next|N
  --checkpoint keep|current|X.Y.Z
  --commit / --no-commit
  --edit-changelog / --no-edit-changelog
  --custom-message / --no-custom-message
  --non-interactive

Examples:
  python3 scripts/bump_version.py patch --save-schema keep --checkpoint keep --non-interactive
  python3 scripts/bump_version.py minor --save-schema next --checkpoint current --non-interactive
"""
from __future__ import annotations

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = ROOT / "src" / "core" / "VersionInfo.cpp"
SAVE_SCHEMA_FILE = ROOT / "include" / "save" / "SaveSchemaVersion.hpp"
VERSION_RE = re.compile(
    r'(std::string\s+VersionInfo::currentVersion\(\)\s*\{\s*return\s+")([0-9]+\.[0-9]+\.[0-9]+)("\s*;\s*\})',
    re.S,
)
CHECKPOINT_RE = re.compile(
    r'(std::string\s+VersionInfo::importantSaveUpdateVersion\(\)\s*\{\s*return\s+")([0-9]+\.[0-9]+\.[0-9]+)("\s*;\s*\})',
    re.S,
)
SAVE_SCHEMA_RE = re.compile(r'(inline\s+constexpr\s+int\s+Current\s*=\s*)([0-9]+)(\s*;)')


def parse_version(text: str) -> tuple[int, int, int]:
    cleaned = text.strip().lstrip("vV")
    parts = cleaned.split(".")
    if len(parts) != 3 or not all(p.isdigit() for p in parts):
        raise ValueError(f"Version invalide : {text!r} (format attendu : X.Y.Z ou X.YY.ZZ)")
    return int(parts[0]), int(parts[1]), int(parts[2])


def format_version(parts: tuple[int, int, int]) -> str:
    return f"{parts[0]}.{parts[1]:02d}.{parts[2]:02d}"


def ask_yes_no(question: str, default: bool = False) -> bool:
    suffix = " [O/n] " if default else " [o/N] "
    while True:
        try:
            answer = input(question + suffix).strip().lower()
        except EOFError:
            return default
        if not answer:
            return default
        if answer in {"o", "oui", "y", "yes"}:
            return True
        if answer in {"n", "non", "no"}:
            return False
        print("Réponds par o/oui ou n/non.")


def read_current_state() -> tuple[str, int, str, str]:
    version_source = VERSION_FILE.read_text(encoding="utf-8")
    version_match = VERSION_RE.search(version_source)
    checkpoint_match = CHECKPOINT_RE.search(version_source)
    if not version_match:
        raise RuntimeError("Impossible de trouver VersionInfo::currentVersion()")
    if not checkpoint_match:
        raise RuntimeError("Impossible de trouver VersionInfo::importantSaveUpdateVersion()")

    schema_source = SAVE_SCHEMA_FILE.read_text(encoding="utf-8")
    schema_match = SAVE_SCHEMA_RE.search(schema_source)
    if not schema_match:
        raise RuntimeError("Impossible de trouver SaveSchemaVersion::Current")

    return version_match.group(2), int(schema_match.group(2)), checkpoint_match.group(2), version_source


def compute_new_game_version(current: str, mode: str) -> str:
    major, minor, patch = parse_version(current)
    if mode == "patch":
        patch += 1
    elif mode == "minor":
        minor += 1
        patch = 0
    elif mode == "major":
        major += 1
        minor = 0
        patch = 0
    else:
        major, minor, patch = parse_version(mode)
    return format_version((major, minor, patch))


def resolve_save_schema(current: int, option: str | None, interactive: bool) -> int:
    if option is not None:
        normalized = option.strip().lower()
        if normalized == "keep":
            return current
        if normalized == "next":
            return current + 1
        if normalized.isdigit() and int(normalized) >= 1:
            return int(normalized)
        raise ValueError("--save-schema attend keep, next ou un entier >= 1")

    if not interactive:
        return current

    print(f"\nSchéma de sauvegarde interne actuel : {current}")
    print("À changer seulement si la structure persistée exige une migration explicite.")
    if not ask_yes_no("Changer aussi le schéma de sauvegarde ?", default=False):
        return current

    while True:
        raw = input(f"Nouveau saveVersion [Entrée = {current + 1}] : ").strip()
        if not raw:
            return current + 1
        if raw.isdigit() and int(raw) >= 1:
            return int(raw)
        print("Entre un entier >= 1.")


def resolve_checkpoint(current: str, new_game_version: str, option: str | None, interactive: bool) -> str:
    if option is not None:
        normalized = option.strip().lower()
        if normalized == "keep":
            return current
        if normalized == "current":
            return new_game_version
        return format_version(parse_version(normalized))

    if not interactive:
        return current

    print(f"\nCheckpoint de sauvegarde important actuel : V{current}")
    print("Un nouveau checkpoint force les anciennes sauvegardes à faire backup + rituel d'adaptation.")
    if ask_yes_no(f"Faire de V{new_game_version} un NOUVEAU checkpoint obligatoire ?", default=False):
        return new_game_version
    return current


def update_sync_files(new_ver: str) -> list[Path]:
    changed: list[Path] = []
    readme_rules = [
        (ROOT / "READMEFR.md", r'(- Version actuelle : \*\*V)[0-9]+\.[0-9]+\.[0-9]+(\*\*)'),
        (ROOT / "README.md", r'(- Current version: \*\*V)[0-9]+\.[0-9]+\.[0-9]+(\*\*)'),
    ]
    for readme, pattern_text in readme_rules:
        if not readme.exists():
            continue
        content = readme.read_text(encoding="utf-8")
        content, count = re.subn(pattern_text, rf'\g<1>{new_ver}\g<2>', content, count=1)
        if count == 0:
            raise RuntimeError(f"Ligne de version courante introuvable dans {readme.name}")
        readme.write_text(content, encoding="utf-8")
        changed.append(readme)

    changelog_sections = [
        (ROOT / "CHANGELOG.md", f"## V{new_ver} — Update notes   \n\n- Version synchronization placeholder. Replace with detailed release notes before publishing.   \n\n---   \n\n"),
        (ROOT / "CHANGELOG_FR.md", f"## V{new_ver} — Notes de mise à jour   \n\n- Synchronisation de version. Remplacer par les notes détaillées avant publication.   \n\n---   \n\n"),
    ]
    for changelog, new_section in changelog_sections:
        if not changelog.exists():
            continue
        content = changelog.read_text(encoding="utf-8")
        if f"V{new_ver}" not in content:
            pattern = re.compile(r'(## V[0-9]+\.[0-9]+\.[0-9]+)')
            if pattern.search(content):
                content = pattern.sub(lambda match: new_section + match.group(1), content, count=1)
            else:
                content += "\n\n" + new_section
            changelog.write_text(content, encoding="utf-8")
            changed.append(changelog)

    for manifest in [ROOT / "release" / "manifest.example.json", ROOT / "assets" / "branding" / "branding_manifest.json"]:
        if not manifest.exists():
            continue
        content = manifest.read_text(encoding="utf-8")
        content, count = re.subn(
            r'("version"\s*:\s*")[0-9]+\.[0-9]+\.[0-9]+(")',
            rf'\g<1>{new_ver}\g<2>',
            content,
            count=1,
        )
        if count == 0:
            raise RuntimeError(f"Champ version introuvable dans {manifest}")
        if manifest.name == "manifest.example.json":
            content = re.sub(r'("releaseTag"\s*:\s*"v)[0-9]+\.[0-9]+\.[0-9]+(")', rf'\g<1>{new_ver}\g<2>', content, count=1)
            content = re.sub(r'((?:INSTALLER-DINOTOFU-(?:WINDOWS|LINUX)|Installer-Dinotofu-(?:Windows|Linux)|Dinotofu-(?:Windows|Linux))-v)[0-9]+\.[0-9]+\.[0-9]+((?:-TECHNICAL-PAYLOAD)?\.(?:7z|zip))', rf'\g<1>{new_ver}\g<2>', content)
        manifest.write_text(content, encoding="utf-8")
        changed.append(manifest)

    return changed


def update_checkpoint_docs(old_checkpoint: str, new_checkpoint: str) -> list[Path]:
    if old_checkpoint == new_checkpoint:
        return []

    changed: list[Path] = []
    version_source = VERSION_FILE.read_text(encoding="utf-8")
    version_source, count = CHECKPOINT_RE.subn(rf'\g<1>{new_checkpoint}\g<3>', version_source, count=1)
    if count != 1:
        raise RuntimeError("Impossible de mettre à jour importantSaveUpdateVersion()")
    VERSION_FILE.write_text(version_source, encoding="utf-8")
    changed.append(VERSION_FILE)

    # Current-status docs may move with the checkpoint. Historical changelog entries must never move.
    readme_checkpoint_rules = {
        ROOT / "README.md": [
            (r'(- Important save checkpoint: \*\*V)[0-9]+\.[0-9]+\.[0-9]+(\*\*)', rf'\g<1>{new_checkpoint}\g<2>'),
            (r'(mandatory V)[0-9]+\.[0-9]+\.[0-9]+( backup \+ transition ritual)', rf'\g<1>{new_checkpoint}\g<2>'),
        ],
        ROOT / "READMEFR.md": [
            (r'(- Point de sauvegarde important : \*\*V)[0-9]+\.[0-9]+\.[0-9]+(\*\*)', rf'\g<1>{new_checkpoint}\g<2>'),
            (r'(du jalon V)[0-9]+\.[0-9]+\.[0-9]+', rf'\g<1>{new_checkpoint}'),
        ],
    }
    for path, rules in readme_checkpoint_rules.items():
        if not path.exists():
            continue
        content = path.read_text(encoding="utf-8")
        for pattern, replacement in rules:
            content, count = re.subn(pattern, replacement, content, count=1)
            if count != 1:
                raise RuntimeError(f"Impossible de synchroniser le checkpoint dans {path.name}")
        path.write_text(content, encoding="utf-8")
        changed.append(path)

    test_cpp = ROOT / "tests" / "ImportantSaveCheckpointTest.cpp"
    if test_cpp.exists():
        content = test_cpp.read_text(encoding="utf-8").replace(old_checkpoint, new_checkpoint)
        test_cpp.write_text(content, encoding="utf-8")
        changed.append(test_cpp)

    test_sh = ROOT / "scripts" / "test_project.sh"
    if test_sh.exists():
        content = test_sh.read_text(encoding="utf-8").replace(old_checkpoint, new_checkpoint)
        test_sh.write_text(content, encoding="utf-8")
        changed.append(test_sh)

    return changed


def update_save_schema(old_schema: int, new_schema: int) -> list[Path]:
    if old_schema == new_schema:
        return []
    source = SAVE_SCHEMA_FILE.read_text(encoding="utf-8")
    source, count = SAVE_SCHEMA_RE.subn(rf'\g<1>{new_schema}\g<3>', source, count=1)
    if count != 1:
        raise RuntimeError("Impossible de mettre à jour SaveSchemaVersion::Current")
    SAVE_SCHEMA_FILE.write_text(source, encoding="utf-8")
    return [SAVE_SCHEMA_FILE]


def open_in_editor(file_path: Path) -> None:
    editor = os.environ.get("VISUAL") or os.environ.get("EDITOR")
    if not editor:
        for cand in ["nano", "vim", "vi"]:
            if shutil.which(cand):
                editor = cand
                break
    if editor:
        try:
            print(f"\nOuverture de {file_path.name} avec {editor}...")
            cmd = shlex.split(editor) + [str(file_path)]
            subprocess.run(cmd, cwd=ROOT, check=False)
        except Exception as exc:
            print(f"Impossible d'ouvrir l'éditeur '{editor}' : {exc}", file=sys.stderr)
    else:
        print(f"Aucun éditeur ($EDITOR / $VISUAL / nano / vim) trouvé pour ouvrir {file_path.name}.")


def ask_commit_mode(default_choice: str = "1") -> str:
    print("\nChoix du mode de commit Git :")
    print("  1. Message automatique [Défaut]")
    print("  2. Message personnalisé (ouvre $EDITOR via 'git commit')")
    print("  3. Ne pas commiter maintenant")
    while True:
        try:
            choice = input(f"Choix [1, 2 ou 3, Défaut = {default_choice}] : ").strip()
        except EOFError:
            return default_choice
        if not choice:
            return default_choice
        if choice in {"1", "2", "3"}:
            return choice
        print("Choix invalide. Entre 1, 2 ou 3.")


def stage_files(files: set[Path]) -> bool:
    if not shutil.which("git"):
        print("Avertissement : 'git' introuvable dans le PATH. Fichiers non ajoutés.")
        return False

    res = subprocess.run(["git", "rev-parse", "--is-inside-work-tree"], cwd=ROOT, capture_output=True, text=True)
    if res.returncode != 0:
        print("Avertissement : le dossier n'est pas un dépôt Git. Fichiers non ajoutés.")
        return False

    rel_paths = [str(f.relative_to(ROOT)) for f in files if f.exists()]
    if not rel_paths:
        return False

    add_res = subprocess.run(["git", "add", "--"] + rel_paths, cwd=ROOT, capture_output=True, text=True)
    if add_res.returncode != 0:
        print(f"Erreur lors de 'git add' : {add_res.stderr}", file=sys.stderr)
        return False

    return True


def commit_changes(
    files: set[Path],
    current_ver: str,
    new_ver: str,
    current_schema: int,
    new_schema: int,
    current_checkpoint: str,
    new_checkpoint: str,
    custom_message: bool = False,
) -> bool:
    if not stage_files(files):
        return False

    if custom_message:
        print("\nOuverture de l'éditeur pour saisir le message de commit Git...")
        commit_res = subprocess.run(["git", "commit"], cwd=ROOT)
        if commit_res.returncode != 0:
            print("Commit Git annulé ou échoué.")
            return False
        print("\n[OK] Commit Git personnalisé créé avec succès.")
        return True

    commit_title = f"chore(release): bump version to {new_ver}"
    commit_body_lines = [
        f"- Version du jeu : {current_ver} -> {new_ver}",
    ]
    if new_schema != current_schema:
        commit_body_lines.append(f"- Schéma de sauvegarde : {current_schema} -> {new_schema}")
    if new_checkpoint != current_checkpoint:
        commit_body_lines.append(f"- Checkpoint important : V{current_checkpoint} -> V{new_checkpoint}")

    commit_args = ["git", "commit", "-m", commit_title]
    if commit_body_lines:
        commit_args.extend(["-m", "\n".join(commit_body_lines)])

    commit_res = subprocess.run(commit_args, cwd=ROOT, capture_output=True, text=True)
    if commit_res.returncode != 0:
        if "nothing to commit" in commit_res.stdout or "nothing to commit" in commit_res.stderr:
            print("Aucune modification à commiter.")
            return True
        print(f"Erreur lors de 'git commit' : {commit_res.stderr}", file=sys.stderr)
        return False

    print(f"\n[OK] Commit Git créé automatiquement : {commit_title}")
    return True


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Synchronise la version Dinotofu et, si demandé, la version de sauvegarde.")
    parser.add_argument("mode", nargs="?", default="patch", help="patch, minor, major ou version explicite X.Y.Z")
    parser.add_argument("--save-schema", dest="save_schema", help="keep, next ou entier explicite")
    parser.add_argument("--checkpoint", help="keep, current ou version explicite X.Y.Z")
    parser.add_argument("--commit", action=argparse.BooleanOptionalAction, default=None, help="ajoute les fichiers modifiés et crée le commit Git (défaut: demande en interactif, désactivé en non-interactif)")
    parser.add_argument("--edit-changelog", action=argparse.BooleanOptionalAction, default=None, help="ouvre CHANGELOG.md dans l'éditeur (défaut: demande en mode interactif)")
    parser.add_argument("--custom-message", action=argparse.BooleanOptionalAction, default=None, help="ouvre l'éditeur Git pour saisir un message personnalisé (défaut: demande en mode interactif)")
    parser.add_argument("--non-interactive", action="store_true", help="ne pose aucune question; conserve les valeurs non précisées")
    return parser


def main() -> int:
    args = build_parser().parse_args()
    try:
        current, current_schema, current_checkpoint, version_source = read_current_state()
        new_version = compute_new_game_version(current, args.mode.strip().lower())
        interactive = (not args.non_interactive) and sys.stdin.isatty()
        new_schema = resolve_save_schema(current_schema, args.save_schema, interactive)
        new_checkpoint = resolve_checkpoint(current_checkpoint, new_version, args.checkpoint, interactive)
    except (ValueError, RuntimeError) as exc:
        print(exc, file=sys.stderr)
        return 2

    print("\nRésumé du changement :")
    print(f"  Version du jeu       : {current} -> {new_version}")
    print(f"  Schéma de sauvegarde : {current_schema} -> {new_schema}")
    print(f"  Checkpoint important : V{current_checkpoint} -> V{new_checkpoint}")

    if interactive and not ask_yes_no("Appliquer ces changements ?", default=True):
        print("Annulé. Aucun fichier modifié.")
        return 0

    modified_files: set[Path] = {VERSION_FILE}
    updated, count = VERSION_RE.subn(rf'\g<1>{new_version}\g<3>', version_source, count=1)
    if count != 1:
        print("Impossible de mettre à jour la version du jeu.", file=sys.stderr)
        return 1
    VERSION_FILE.write_text(updated, encoding="utf-8")

    modified_files.update(update_save_schema(current_schema, new_schema))
    modified_files.update(update_checkpoint_docs(current_checkpoint, new_checkpoint))
    modified_files.update(update_sync_files(new_version))

    print("\nDinotofu mis à jour.")
    if new_schema != current_schema:
        print(f"ATTENTION : saveVersion est passé de {current_schema} à {new_schema}. Vérifie/ajoute la migration correspondante.")
    if new_checkpoint != current_checkpoint:
        print(f"ATTENTION : V{new_checkpoint} devient un nouveau checkpoint obligatoire de sauvegarde.")

    if interactive:
        should_edit_changelog = args.edit_changelog
        if should_edit_changelog is None:
            should_edit_changelog = ask_yes_no("Ouvrir CHANGELOG.md pour éditer les notes de version ?", default=True)
        if should_edit_changelog:
            open_in_editor(ROOT / "CHANGELOG.md")
            if (ROOT / "CHANGELOG_FR.md").exists() and ask_yes_no("Ouvrir aussi CHANGELOG_FR.md ?", default=False):
                open_in_editor(ROOT / "CHANGELOG_FR.md")
    elif args.edit_changelog:
        open_in_editor(ROOT / "CHANGELOG.md")

    should_commit = args.commit
    custom_msg = args.custom_message

    if interactive and should_commit is None:
        should_commit = ask_yes_no("Tout est modifié. Créer maintenant un commit Git ?", default=True)
    elif should_commit is None:
        # Safety for automation / other developers: an explicit --commit is required.
        should_commit = False

    if not should_commit:
        print("\nAucun commit créé. Les fichiers restent modifiés localement pour permettre d'autres changements ou une revue avant commit.")
        return 0

    if interactive and custom_msg is None:
        mode_choice = ask_commit_mode(default_choice="1")
        if mode_choice == "1":
            custom_msg = False
        elif mode_choice == "2":
            custom_msg = True
        else:
            print("\nCommit ignoré. Les modifications restent locales.")
            return 0
    elif custom_msg is None:
        custom_msg = False

    commit_changes(
        files=modified_files,
        current_ver=current,
        new_ver=new_version,
        current_schema=current_schema,
        new_schema=new_schema,
        current_checkpoint=current_checkpoint,
        new_checkpoint=new_checkpoint,
        custom_message=custom_msg,
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
