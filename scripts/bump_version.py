#!/usr/bin/env python3
"""
Bump Dinotofu version in src/core/VersionInfo.cpp.

Usage:
  python3 scripts/bump_version.py patch
  python3 scripts/bump_version.py minor
  python3 scripts/bump_version.py major
  python3 scripts/bump_version.py 2.00.10
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = ROOT / "src" / "core" / "VersionInfo.cpp"
VERSION_RE = re.compile(r'(std::string\s+VersionInfo::currentVersion\(\)\s*\{\s*return\s+")([0-9]+\.[0-9]+\.[0-9]+)("\s*;\s*\})', re.S)


def parse_version(text: str) -> tuple[int, int, int]:
    parts = text.strip().split(".")
    if len(parts) != 3 or not all(p.isdigit() for p in parts):
        raise ValueError(f"Version invalide : {text!r}")
    return int(parts[0]), int(parts[1]), int(parts[2])


def format_version(parts: tuple[int, int, int]) -> str:
    return f"{parts[0]}.{parts[1]:02d}.{parts[2]:02d}"


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    mode = sys.argv[1].strip().lower()
    source = VERSION_FILE.read_text(encoding="utf-8")
    match = VERSION_RE.search(source)
    if not match:
        print("Impossible de trouver VersionInfo::currentVersion() dans src/core/VersionInfo.cpp", file=sys.stderr)
        return 1

    current = match.group(2)
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
        try:
            major, minor, patch = parse_version(mode)
        except ValueError as exc:
            print(exc, file=sys.stderr)
            print(__doc__.strip(), file=sys.stderr)
            return 2

    new_version = format_version((major, minor, patch))
    updated = VERSION_RE.sub(rf'\g<1>{new_version}\g<3>', source, count=1)
    VERSION_FILE.write_text(updated, encoding="utf-8")
    print(f"Dinotofu : {current} -> {new_version}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
