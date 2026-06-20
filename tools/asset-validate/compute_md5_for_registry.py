#!/usr/bin/env python3
"""Compute MD5 for every file in docs/VERIFIED_HASHES.md and emit a cross-table.

Background: Firestaff has two parallel asset-identity systems:

1. MD5 (32 hex chars) is embedded in runtime tables in
   src/shared/asset_status_m12.c (g_requiredFiles[]) and
   src/shared/asset_find_by_hash.c. These drive game-version detection
   at M12 launcher time (the user picks DM1 PC 3.4 EN because its
   GRAPHICS.DAT MD5 fa6b1aa29e191418713bf2cda93d962e matches the
   embedded constant).

2. SHA256 (64 hex chars) lives in docs/VERIFIED_HASHES.md and is used
   by compare_to_greatstone.py plus several verify_pass*.py gates.
   SHA256 is the modern hash (no known collisions in the DM/CSB/DM2
   game-data universe) and is what every external validation tool
   (sha256sum, Greatstone future, pass445) prefers.

Both systems are correct and intentionally redundant. This tool bridges
them: for each (game, filename) entry in VERIFIED_HASHES.md whose local
file exists under ~/.firestaff/data/, compute MD5 and emit a cross-table
mapping MD5 ↔ SHA256 for that file.

Usage:
    python3 tools/asset-validate/compute_md5_for_registry.py
    python3 tools/asset-validate/compute_md5_for_registry.py --data-dir ~/Games/DM
    python3 tools/asset-validate/compute_md5_for_registry.py --json

Output (default): human-readable table grouped by game.
Output (--json): one JSON object suitable for piping into other tools.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = REPO_ROOT / "docs" / "VERIFIED_HASHES.md"
DEFAULT_DATA_DIR = Path.home() / ".firestaff" / "data"


@dataclass(frozen=True)
class RegistryEntry:
    game: str
    filename: str
    sha256: str
    size: int


@dataclass(frozen=True)
class CrossTableEntry:
    game: str
    filename: str
    rel_path: str
    sha256: str
    md5: str
    size: int
    source: str  # "local" or "missing"


_TABLE_RE = re.compile(
    r"^\|\s*(?P<game>[^|]+?)\s*\|\s*(?P<file>[^|]+?)\s*\|\s*`(?P<hash>[a-f0-9]{64})`\s*\|\s*(?P<size>[\d,]+)\s*\|",
    re.MULTILINE,
)
_BULLET_RE = re.compile(
    r"^-\s*`(?P<path>[^`]+?)`\s*\((?P<size>[\d,]+)\s*bytes?\)\s*:\s*`(?P<hash>[a-f0-9]{64})`",
    re.MULTILINE,
)


def parse_registry(path: Path) -> dict[Path, RegistryEntry]:
    """Parse docs/VERIFIED_HASHES.md into {rel_path -> RegistryEntry}.

    Supports both the markdown table form ("| game | file | hash | size |")
    and the bullet form ("- `path/to/file` (N bytes): `hash`") so the
    script keeps working if the registry format evolves.
    """
    text = path.read_text(encoding="utf-8")
    out: dict[Path, RegistryEntry] = {}

    for m in _TABLE_RE.finditer(text):
        game = m.group("game").strip()
        filename = m.group("file").strip()
        sha = m.group("hash")
        size = int(m.group("size").replace(",", ""))
        rel = Path(game) / filename
        # Prefer first-seen entry; de-dupe later.
        out.setdefault(rel, RegistryEntry(game=game, filename=filename, sha256=sha, size=size))

    for m in _BULLET_RE.finditer(text):
        rel_path = Path(m.group("path").strip())
        sha = m.group("hash")
        size = int(m.group("size").replace(",", ""))
        game = rel_path.parts[0] if len(rel_path.parts) > 1 else "unknown"
        filename = rel_path.name
        out.setdefault(rel_path, RegistryEntry(game=game, filename=filename, sha256=sha, size=size))

    return out


def _hash_file(path: Path) -> tuple[str, int]:
    md5 = hashlib.md5()
    size = 0
    with open(path, "rb") as fp:
        while True:
            chunk = fp.read(1 << 20)
            if not chunk:
                break
            md5.update(chunk)
            size += len(chunk)
    return md5.hexdigest(), size


def _candidate_paths(data_dir: Path, entry: RegistryEntry) -> list[Path]:
    """Return possible on-disk paths for an entry.

    Some games have well-known case variants (Dungeon.dat vs DUNGEON.DAT),
    so we look both for the registry's exact case and for a case-insensitive
    match in the parent directory.
    """
    candidates: list[Path] = []
    direct = data_dir / entry.game / entry.filename
    candidates.append(direct)

    parent = data_dir / entry.game
    if parent.is_dir():
        try:
            for child in parent.iterdir():
                if child.name.lower() == entry.filename.lower():
                    candidates.append(child)
        except OSError:
            pass

    # Nexus mixes .BIN and .MNS suffixes in the same dir; also check top-level
    # under data_dir for files where the registry omits the game tag (rare).
    for top in data_dir.iterdir() if data_dir.is_dir() else []:
        if top.is_file() and top.name.lower() == entry.filename.lower():
            candidates.append(top)

    seen: set[Path] = set()
    out: list[Path] = []
    for c in candidates:
        if c not in seen:
            seen.add(c)
            out.append(c)
    return out


def build_cross_table(
    registry: Path, data_dir: Path
) -> list[CrossTableEntry]:
    entries = parse_registry(registry)
    rows: list[CrossTableEntry] = []
    for rel, e in entries.items():
        for cand in _candidate_paths(data_dir, e):
            if cand.is_file():
                md5, actual_size = _hash_file(cand)
                rows.append(
                    CrossTableEntry(
                        game=e.game,
                        filename=e.filename,
                        rel_path=str(rel),
                        sha256=e.sha256,
                        md5=md5,
                        size=actual_size,
                        source="local",
                    )
                )
                break
        else:
            rows.append(
                CrossTableEntry(
                    game=e.game,
                    filename=e.filename,
                    rel_path=str(rel),
                    sha256=e.sha256,
                    md5="",
                    size=e.size,
                    source="missing",
                )
            )
    rows.sort(key=lambda r: (r.game, r.filename))
    return rows


def print_table(rows: Iterable[CrossTableEntry]) -> None:
    rows = list(rows)
    if not rows:
        print("(no registry entries)")
        return
    print(f"{'Game':<22} {'File':<28} {'MD5':<34} {'SHA256':<66} {'Size':>10} {'Src':<8}")
    print("-" * 172)
    for r in rows:
        print(
            f"{r.game:<22} {r.filename:<28} {r.md5 or '-':<34} {r.sha256:<66} {r.size:>10} {r.source:<8}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--registry", type=Path, default=DEFAULT_REGISTRY)
    parser.add_argument("--data-dir", type=Path, default=DEFAULT_DATA_DIR)
    parser.add_argument("--json", action="store_true", help="emit JSON instead of table")
    args = parser.parse_args()

    if not args.registry.is_file():
        print(f"registry not found: {args.registry}", file=sys.stderr)
        return 4

    if not args.data_dir.is_dir():
        print(f"data dir not found: {args.data_dir}", file=sys.stderr)
        return 4

    rows = build_cross_table(args.registry, args.data_dir)
    if args.json:
        print(json.dumps([asdict(r) for r in rows], indent=2))
    else:
        print_table(rows)
    return 0


if __name__ == "__main__":
    sys.exit(main())