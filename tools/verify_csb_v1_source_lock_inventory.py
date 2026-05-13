#!/usr/bin/env python3
"""Verify the CSB V1 source-lock inventory anchors.

Evidence-only gate: checks local N2 CSB/CSBWin/ReDMCSB references and canonical
CSB asset anchors. It does not run Firestaff, launch CSB, render frames, or
modify runtime/menu code.
"""
from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REPO_OUT = ROOT / "parity-evidence/verification/csb_v1_source_lock_inventory.json"
REDMCSB = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"
ORIG = (Path.home() / ".openclaw/data/firestaff-original-games/DM")

EXPECTED_GIT = {
    "csb_source": (CSB_SRC, "dda570585abb4c8113a3298d21c0b599e6cac4f9"),
    "csbwin_source": (CSBWIN, "2f63d10d9b8c155e0be17888271d394255ce1bac"),
}

EXPECTED_FILES = {
    "atari_archive": (ORIG / "Game,Chaos_Strikes_Back,Atari_ST,Software.7z", 1669479, "ce6e638622a099bbf15e6dacd7750ce811a52373a20b2d0f92ef6332cc47d7f5"),
    "atari_v20_msa": (ORIG / "_extracted/csb-atari/Floppy Disks MSA/Chaos Strikes Back for Atari ST Game Disk v2.0 (English).msa", 404254, "e3d8dc75956ade33658b700e9ae2512dcef7a8dfa538116b8a717f4efaefe0b4"),
    "atari_v21_stx": (ORIG / "_extracted/csb-atari/Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx", 941828, "aea724d663554e84393a77c45c010753c5bfb1a3a5a83d1264b5ef2af9aa5c6f"),
    "canonical_atari_graphics": (ORIG / "_canonical/csb/atari-GRAPHICS.DAT", 319080, "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af"),
    "canonical_atari_dungeon": (ORIG / "_canonical/csb/atari-DUNGEON.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
    "canonical_amiga_graphics": (ORIG / "_canonical/csb/amiga-Graphics.DAT", 435076, "3af5396fa32af08af5e0581a6cdf5b30c8397834efa5b9e0c8c991219d256942"),
    "canonical_amiga_dungeon": (ORIG / "_canonical/csb/amiga-Dungeon.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
}

SOURCE_CHECKS = [
    ("redmcsb_dungeon_ids", REDMCSB / "DEFS.H", 519, 523, ["C10_DUNGEON_DM", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME"]),
    ("redmcsb_csb_save_format", REDMCSB / "DEFS.H", 468, 498, ["DM_SAVE_HEADER", "CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK"]),
    ("redmcsb_new_adventure_gate", REDMCSB / "CEDTINCH.C", 5, 64, ["F7086_IsReadyToMakeNewAdventure", "F7272_IsDungeonValid(&G7111_Games[C0_GAME_SOURCE], 3)", "C13_DUNGEON_CSB_GAME"]),
    ("redmcsb_save_routing", REDMCSB / "CEDTINC8.C", 101, 118, ["M745_FILE_ID_SAVE_DMSAVE_DAT", "M746_FILE_ID_SAVE_CSBGAME_DAT", "C12_DUNGEON_CSB_PRISON"]),
    ("csb_required_payloads", CSB_SRC / "README", 1, 30, ["dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt"]),
    ("csb_graphics_open_boundary", CSB_SRC / "Graphics.cpp", 1740, 1915, ["graphics.dat", "Cannot find 'graphics.dat'", "CSBgraphics.dat"]),
    ("csb_new_adventure_flow", CSB_SRC / "Chaos.cpp", 1, 40, ["Create New Adventure", "prison savegame", "Make New Adventure"]),
    ("csb_csbgame_slots", CSB_SRC / "Chaos.cpp", 500, 625, ["CSBGAME", "CSBGAME2", "csbgame.dat", "csbgame.bak"]),
    ("csbwin_play_workflow", CSBWIN / "Game/readme.txt", 1, 30, ["Enter the dungeon", "choose prison", "Make New Adventure"]),
    ("firestaff_csb_launch_still_gated", ROOT / "menu_startup_m12.c", 1390, 1413, ["strcmp(gameId, \"csb\") == 0", "return gameId && strcmp(gameId, \"dm1\") == 0;"]),
]

NON_CLAIMS = [
    "No CSB Firestaff runtime/render parity claim.",
    "No CSB launch enablement claim; menu_startup_m12 remains launch-gated to DM1.",
    "No platform substitution claim; Atari ST and Amiga graphics anchors differ and stay separate.",
    "No CSB sample-save/new-adventure runtime proof; existing sample-save blocker remains open.",
]

NEXT_GAPS = [
    "Land a CSB asset-pair manifest/verifier that requires the selected Atari ST graphics plus dungeon/runtime data before any launch intent is accepted.",
    "Add an explicit experimental CSB launch-intent probe that proves menu/config routing only, with render/gameplay still disabled.",
    "Resolve the curated CSB sample-save/new-adventure fixture gap, or formalize it as blocked with exact approved search roots and missing filenames.",
    "Define the first CSB V1 parity surface matrix: startup/load, prison/champion route, viewport/HUD, input, save/new-adventure, with each row tied to a source anchor or BLOCKED.",
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def git_head(path: Path) -> str | None:
    try:
        return subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()
    except Exception:
        return None


def line_window(path: Path, start: int, end: int) -> str:
    try:
        text = path.read_text(errors="replace")
    except Exception:
        return ""
    return "\n".join(text.splitlines()[start - 1:end])


def display_path(path: Path) -> str:
    mapping = [
        (CSB_SRC, "<csb-source>/CSB/src"),
        (CSBWIN, "<csbwin-source>/CSBWin"),
        (REDMCSB.parent.parent.parent, "<redmcsb-source>/ReDMCSB_WIP20210206"),
        (ORIG, "<firestaff-original-games>"),
        (ROOT, "<firestaff-repo>"),
    ]
    resolved = path.absolute()
    for base, label in mapping:
        try:
            rel = resolved.relative_to(base.absolute())
        except ValueError:
            continue
        rel_text = str(rel)
        return label if rel_text == "." else f"{label}/{rel_text}"
    return str(path)


def main() -> int:
    failures: list[str] = []
    git_rows = []
    for ident, (path, expected) in EXPECTED_GIT.items():
        actual = git_head(path)
        ok = actual == expected
        if not ok:
            failures.append(f"{ident} HEAD mismatch: expected {expected}, actual {actual}")
        git_rows.append({"id": ident, "path": display_path(path), "expected_head": expected, "actual_head": actual, "ok": ok})

    file_rows = []
    for ident, (path, size, digest) in EXPECTED_FILES.items():
        exists = path.exists()
        actual_size = path.stat().st_size if exists else None
        actual_digest = sha256(path) if exists else None
        ok = exists and actual_size == size and actual_digest == digest
        if not ok:
            failures.append(f"{ident} mismatch: exists={exists} size={actual_size} sha256={actual_digest}")
        file_rows.append({"id": ident, "path": display_path(path), "expected_size": size, "actual_size": actual_size, "expected_sha256": digest, "actual_sha256": actual_digest, "ok": ok})

    source_rows = []
    for ident, path, start, end, needles in SOURCE_CHECKS:
        haystack = line_window(path, start, end)
        missing = [needle for needle in needles if needle not in haystack]
        ok = path.exists() and not missing
        if not ok:
            failures.append(f"{ident} missing: {missing} path_exists={path.exists()}")
        source_rows.append({"id": ident, "path": display_path(path), "lines": f"{start}-{end}", "needles": needles, "missing": missing, "ok": ok})

    result = {
        "schema": "firestaff.csb_v1_source_lock_inventory.v1",
        "pass": not failures,
        "scope": "CSB V1 source-lock inventory and first landable gaps only, using N2-local CSB/CSBWin/ReDMCSB/original-data references.",
        "git_refs": git_rows,
        "asset_refs": file_rows,
        "source_anchors": source_rows,
        "next_landable_gaps": NEXT_GAPS,
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    REPO_OUT.parent.mkdir(parents=True, exist_ok=True)
    REPO_OUT.write_text(json.dumps(result, indent=2) + "\n")
    status = "PASS" if result["pass"] else "FAIL"
    print("{} csb v1 source-lock inventory: {} source anchors, {} asset refs, {} source repos".format(status, len(source_rows), len(file_rows), len(git_rows)))
    for failure in failures:
        print(f"- {failure}")
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
