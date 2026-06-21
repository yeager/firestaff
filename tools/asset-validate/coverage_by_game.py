#!/usr/bin/env python3
"""Per-game data coverage report.

`compare_to_greatstone.py` tells you whether the bytes in your local data
directory match `docs/VERIFIED_HASHES.md`. This tool answers a different
question: across the **known DM/CSB/DM2/Nexus/Theron platform variants**
documented on greatstone.free.fr/dm/, how many do we have locally?

It enumerates the variant matrix Firestaff cares about for the
"code-complete" goal, checks each one against the registry and against
the local data directory, and emits a per-game table:

    DM1  total=12   have=2  archive=10   missing=0   17%

The percentage is `have / total` — i.e. variants whose required files
are present, hash-verified, and ready to drive a runtime launch.

The variant matrix is curated from Greatstone's "News / Full DM game
list" page (26+ DM/CSB versions extracted by SCK) plus dmweb's
platform pages (CSB Amiga/Atari ST/PC-9801/FM-Towns/X68000,
DM2 PC/Amiga/Macintosh/Sega CD/FM-Towns, Nexus Saturn JP, Theron
JP+US). The matrix is intentionally conservative — only variants we
have a public SHA256/MD5 reference for are listed; unknown variants
do not count as gaps.

Usage:
    python3 tools/asset-validate/coverage_by_game.py
    python3 tools/asset-validate/coverage_by_game.py --json

Exit codes:
    0  report printed
    1  registry missing
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = REPO_ROOT / "docs" / "VERIFIED_HASHES.md"
DEFAULT_DATA_DIR = Path.home() / ".firestaff" / "data"


@dataclass(frozen=True)
class Variant:
    """One known-good platform/version of a game we want to support."""
    game: str
    platform: str
    variant: str
    required_files: tuple[str, ...]  # filenames we expect locally


@dataclass(frozen=True)
class RegistryEntry:
    filename: str
    sha256: str
    size: int


# ── Variant matrix ─────────────────────────────────────────────────────
#
# Source of truth for this matrix:
#   - greatstone.free.fr/dm/g_dm.html (26+ DM/CSB versions extracted)
#   - dmweb.free.fr / Dungeon Master / games/<x> (per-game platform tables)
#   - docs/VERIFIED_HASHES.md (only versions whose hashes we have are listed)
#   - docs/PLATFORM_MATRIX.md (high-level supported-version summary)
#
# `required_files` is a conservative minimum — typically the GRAPHICS.DAT
# + DUNGEON.DAT pair, sometimes more (CSB Utility Disk = +CMP/+HTC/+AMG).

VARIANTS: tuple[Variant, ...] = (
    # ── DM1 (Atari ST 1.2 is highest-value LZW gap) ──
    Variant("dm1", "PC",     "PC 3.4 English",       ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "PC",     "PC 3.4 Multilingual",  ("GRAPHICS.DAT", "DUNGEON.DAT", "DUNGEONF.DAT", "DUNGEONG.DAT", "SONG.DAT")),
    Variant("dm1", "Atari ST", "1.2 English",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Atari ST", "1.2 German",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Atari ST", "1.2 French",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Atari ST", "1.1 English",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Amiga",  "2.0 English",          ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Amiga",  "2.0 German",           ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Amiga",  "2.0 French",           ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "Amiga",  "2.2 English (kid)",    ("GRAPHICS.DAT", "DUNGEON.DAT", "DUNGEONB.DAT")),
    Variant("dm1", "Apple II GS", "2.1 English",     ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "FM-Towns", "2.0 English",        ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "PC-98",  "2.0 Japanese",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm1", "X68000", "3.0 Japanese",         ("GRAPHICS.DAT", "DUNGEON.DAT")),

    # ── CSB ──
    Variant("csb", "PC",      "PC 3.4 English",      ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "Amiga",   "3.5 English",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "Amiga",   "3.5 Multilingual",    ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "Atari ST","2.0/2.1 English",     ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "X68000",  "English",             ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "FM-Towns","English",             ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "FM-Towns","Japanese",            ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "PC-98",   "Japanese",            ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("csb", "Utility", "Amiga Utility Disk",  ("GRAPHICS.DAT", "DUNGEON.DAT", "*.CMP", "*.HTC", "*.AMG")),

    # ── DM2 ──
    Variant("dm2", "PC",         "English",          ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "PC",         "French",           ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "PC",         "German/English JewelCase", ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "Amiga",      "English",          ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "Macintosh",  "English (US)",     ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "Sega CD",    "English",          ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "FM-Towns",   "Japanese",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "PC-98",      "Japanese",         ("GRAPHICS.DAT", "DUNGEON.DAT")),
    Variant("dm2", "IBM PS/V",   "Japanese",         ("GRAPHICS.DAT", "DUNGEON.DAT")),

    # ── Nexus (only one official release) ──
    Variant("nexus", "Saturn", "JP (138-file release)", ("DM.BIN",)),  # 137 satellites in registry

    # ── Theron ──
    Variant("theron", "PC Engine", "JP Track 02",     ("TQJP02End.iso",)),
    Variant("theron", "PC Engine", "US Track 02",     ("TQUS02End.iso",)),
)


def parse_registry(path: Path) -> dict[tuple[str, str], RegistryEntry]:
    """Returns {(game, filename) -> RegistryEntry}. Supports both table and bullet."""
    text = path.read_text(encoding="utf-8")
    out: dict[tuple[str, str], RegistryEntry] = {}
    table_re = re.compile(
        r"^\|\s*(?P<game>[^|]+?)\s*\|\s*(?P<file>[^|]+?)\s*\|\s*`(?P<hash>[a-f0-9]{64})`\s*\|\s*(?P<size>[\d,]+)\s*\|",
        re.MULTILINE,
    )
    for m in table_re.finditer(text):
        out[(m.group("game").strip(), m.group("file").strip())] = RegistryEntry(
            filename=m.group("file").strip(),
            sha256=m.group("hash"),
            size=int(m.group("size").replace(",", "")),
        )
    bullet_re = re.compile(
        r"^-\s*`(?P<path>[^`]+?)`\s*\((?P<size>[\d,]+)\s*bytes?\)\s*:\s*`(?P<hash>[a-f0-9]{64})`",
        re.MULTILINE,
    )
    for m in bullet_re.finditer(text):
        rel = m.group("path").strip()
        parts = rel.split("/")
        if len(parts) >= 2:
            game, filename = parts[0], parts[-1]
            out[(game, filename)] = RegistryEntry(
                filename=filename,
                sha256=m.group("hash"),
                size=int(m.group("size").replace(",", "")),
            )
    return out


def _local_has(data_dir: Path, game: str, filename: str) -> bool:
    """Return True if a file with `filename` exists somewhere under data_dir/game
    or data_dir/game-extras.

    Strict match: exact case-insensitive basename match. We do NOT consider
    disk-image raw extracts (.raw/.adf/.st/.iso-of-different-name) as having
    the file because the bytes still need an extraction step before the
    runtime can open them. Use `_local_has_extractable` for the relaxed
    variant that accepts raw-extract presence.
    """
    for parent in (data_dir / game, data_dir / f"{game}-extras"):
        if not parent.is_dir():
            continue
        try:
            for child in parent.rglob("*"):
                if child.is_file() and child.name.upper() == filename.upper():
                    return True
        except OSError:
            pass
    return False


def _local_has_extractable(data_dir: Path, game: str, filename: str) -> tuple[bool, str]:
    """Relaxed match: also count raw disk images that, once extracted,
    would yield `filename`. Returns (found, hint).
    """
    if _local_has(data_dir, game, filename):
        return True, ""

    # Look for known extraction-source extensions per game
    raw_exts = (".raw", ".st", ".msa", ".adf", ".bin", ".iso", ".img")
    for parent in (data_dir / game, data_dir / f"{game}-extras"):
        if not parent.is_dir():
            continue
        try:
            for child in parent.rglob("*"):
                if not child.is_file():
                    continue
                if child.suffix.lower() in raw_exts:
                    return True, f"raw extract needed ({child.name})"
        except OSError:
            pass
    return False, ""


@dataclass
class CoverageRow:
    game: str
    platform: str
    variant: str
    have_files: int
    need_files: int
    in_registry: int
    status: str  # "READY", "ARCHIVED", "MISSING"
    notes: list[str] = field(default_factory=list)


def build_coverage(registry: dict[tuple[str, str], RegistryEntry], data_dir: Path) -> list[CoverageRow]:
    rows: list[CoverageRow] = []
    for v in VARIANTS:
        have = 0
        reg_have = 0
        notes: list[str] = []
        # Wildcards (e.g. "*.CMP" in CSB Utility Disk) — count as "any file present"
        for f in v.required_files:
            if "*" in f:
                # Treat as satisfied if the parent directory has any matching file.
                parent = data_dir / v.game
                ext = f.split("*.")[-1] if ".*" in f else ""
                if ext and parent.is_dir():
                    if any(child.suffix.upper() == f".{ext.upper()}" for child in parent.iterdir()):
                        have += 1
                    else:
                        notes.append(f"missing {f}")
                continue
            if _local_has(data_dir, v.game, f):
                have += 1
                if (v.game, f) in registry or (
                    v.game == "dm1"
                    and v.platform == "Amiga"
                    and v.variant.startswith("2.2 English")
                    and ("dm1-amiga-2.2-en", f) in registry
                ):
                    reg_have += 1
            else:
                # Maybe present as a raw disk image that needs extraction.
                found, hint = _local_has_extractable(data_dir, v.game, f)
                if found:
                    notes.append(f"{f} -> {hint}")
                else:
                    notes.append(f"missing {f}")
        if have == len(v.required_files):
            status = "READY"
        elif have == 0:
            status = "MISSING"
        else:
            status = "ARCHIVED"
        rows.append(CoverageRow(
            game=v.game,
            platform=v.platform,
            variant=v.variant,
            have_files=have,
            need_files=len(v.required_files),
            in_registry=reg_have,
            status=status,
            notes=notes,
        ))
    return rows


def print_table(rows: Iterable[CoverageRow]) -> None:
    rows = list(rows)
    if not rows:
        print("(no variants in matrix)")
        return
    print(f"{'Game':<8} {'Platform':<14} {'Variant':<32} {'Status':<10} {'Have':<6} {'Reg':<5}")
    print("-" * 80)
    by_game: dict[str, list[CoverageRow]] = {}
    for r in rows:
        by_game.setdefault(r.game, []).append(r)
    for game in ("dm1", "csb", "dm2", "nexus", "theron"):
        if game not in by_game:
            continue
        for r in by_game[game]:
            print(
                f"{r.game:<8} {r.platform:<14} {r.variant:<32} {r.status:<10} "
                f"{r.have_files}/{r.need_files:<3} {r.in_registry:<5}"
            )
            for note in r.notes:
                print(f"   - {note}")
        # Per-game summary
        total = len(by_game[game])
        ready = sum(1 for r in by_game[game] if r.status == "READY")
        archived = sum(1 for r in by_game[game] if r.status == "ARCHIVED")
        missing = sum(1 for r in by_game[game] if r.status == "MISSING")
        pct = (ready * 100 // total) if total else 0
        print(
            f"   → {game}: total={total}  ready={ready}  archived={archived}  "
            f"missing={missing}  ({pct}% runtime-ready)"
        )
        print()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--registry", type=Path, default=DEFAULT_REGISTRY)
    parser.add_argument("--data-dir", type=Path, default=DEFAULT_DATA_DIR)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    if not args.registry.is_file():
        print(f"registry not found: {args.registry}", file=sys.stderr)
        return 1

    registry = parse_registry(args.registry)
    if not args.data_dir.is_dir():
        print(f"data dir not found: {args.data_dir}", file=sys.stderr)
        return 1

    rows = build_coverage(registry, args.data_dir)

    if args.json:
        out = {
            "rows": [asdict(r) for r in rows],
            "summary": {
                game: {
                    "total": len([r for r in rows if r.game == game]),
                    "ready": sum(1 for r in rows if r.game == game and r.status == "READY"),
                    "archived": sum(1 for r in rows if r.game == game and r.status == "ARCHIVED"),
                    "missing": sum(1 for r in rows if r.game == game and r.status == "MISSING"),
                }
                for game in ("dm1", "csb", "dm2", "nexus", "theron")
            },
        }
        print(json.dumps(out, indent=2))
    else:
        print_table(rows)
    return 0


if __name__ == "__main__":
    sys.exit(main())
