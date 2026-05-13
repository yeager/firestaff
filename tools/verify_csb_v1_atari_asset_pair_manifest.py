#!/usr/bin/env python3
"""Verify the CSB V1 Atari asset-pair manifest boundary.

Evidence-only gate: proves the selected Atari ST CSB V1 lane has a paired
GRAPHICS.DAT + DUNGEON.DAT plus the runtime support payloads required by the
local CSB lineage source. It deliberately keeps CSB launch/render/gameplay
blocked; downstream launch intent must consume this manifest and still reject
CSB until an explicit experimental gate is added.
"""
from __future__ import annotations

import hashlib
import json
import subprocess
from dataclasses import asdict, dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json"
REDMCSB = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"
ORIG = (Path.home() / ".openclaw/data/firestaff-original-games/DM")
ATARI_HD = ORIG / "_extracted/csb-atari/HardDisk/2009-02-22 PP"
CANON = ORIG / "_canonical/csb"

EXPECTED_GIT = {
    "csb_lineage": (CSB_SRC, "dda570585abb4c8113a3298d21c0b599e6cac4f9"),
    "csbwin_lineage": (CSBWIN, "2f63d10d9b8c155e0be17888271d394255ce1bac"),
}

# The selected Atari ST lane is intentionally narrow. GRAPHICS/DUNGEON are
# the pair identity; HCSB/MINI are the runtime support payloads named by the
# CSB lineage README. CONFIG.TXT is not present in the original-data anchor and
# is treated as a future local runtime-config concern, not as launch clearance.
EXPECTED_ASSETS = {
    "selected_atari_graphics": (ATARI_HD / "GRAPHICS.DAT", 319080, "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af", "pair"),
    "selected_atari_dungeon": (ATARI_HD / "DUNGEON.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba", "pair"),
    "selected_atari_hcsb_dat": (ATARI_HD / "HCSB.DAT", 30793, "5268b36a108f582e043a0e698052ce6fe67d33132737a3dc2271caa3031e6fcc", "runtime_support"),
    "selected_atari_hcsb_htc": (ATARI_HD / "HCSB.HTC", 66172, "1b2fbff81a11928afd153f46c117355cce1f9a93f482d14d58e35a115d9cde38", "runtime_support"),
    "selected_atari_mini": (ATARI_HD / "MINI.DAT", 42815, "61d981061bbb7a81b9b7f4795e99c24a592ee329169eb0c195459ba4eb62e3a9", "runtime_support"),
    "canonical_atari_graphics": (CANON / "atari-GRAPHICS.DAT", 319080, "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af", "canonical_pair"),
    "canonical_atari_dungeon": (CANON / "atari-DUNGEON.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba", "canonical_pair"),
}

@dataclass(frozen=True)
class Anchor:
    id: str
    path: str
    lines: str
    needles: tuple[str, ...]
    why: str

SOURCE_ANCHORS = (
    Anchor(
        "csb_required_payloads",
        str(CSB_SRC / "README"),
        "14-21",
        ("dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt"),
        "CSB lineage source names the play-directory payload set; local Atari extraction supplies the data payloads but not config.txt.",
    ),
    Anchor(
        "csb_graphics_open_boundary",
        str(CSB_SRC / "Graphics.cpp"),
        "1814-1915",
        ("openGraphicsFile", "graphics.dat", "Cannot find 'graphics.dat'", "OpenCSBgraphicsFile", "CSBgraphics.dat"),
        "Graphics.cpp separates primary graphics.dat open/signature checks from optional CSBgraphics.dat boundary.",
    ),
    Anchor(
        "csb_runtime_save_slots",
        str(CSB_SRC / "Chaos.cpp"),
        "507-623",
        ("CSBGAME", "csbgame.dat", "csbgame.bak", "SAVING NEW ADVENTURE", "MINI.DAT"),
        "Chaos.cpp ties the utility/new-adventure runtime lane to CSBGAME slots and MINI.DAT support data.",
    ),
    Anchor(
        "redmcsb_csb_dungeon_ids",
        str(REDMCSB / "DEFS.H"),
        "519-523",
        ("C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME", "Value used in CSB MINI.DAT"),
        "ReDMCSB source separates CSB prison/game dungeon IDs and explicitly associates C13 with MINI.DAT.",
    ),
    Anchor(
        "redmcsb_new_adventure_gate",
        str(REDMCSB / "CEDTINCH.C"),
        "5-63",
        ("F7086_IsReadyToMakeNewAdventure", "C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK"),
        "New Adventure readiness is state/header-gated, so asset identity alone must not imply launch/play support.",
    ),
    Anchor(
        "csbwin_play_workflow",
        str(CSBWIN / "Game/readme.txt"),
        "1-30",
        ("Enter the dungeon", "choose prison", "Make New Adventure"),
        "CSBWin confirms the workflow boundary: prison entry precedes Make New Adventure.",
    ),
    Anchor(
        "firestaff_csb_launch_still_gated",
        str(ROOT / "menu_startup_m12.c"),
        "225-1412",
        (".gameId = \"csb\"", "static int m12_game_supported", "Only DM1 is launch-supported", "return gameId && strcmp(gameId, \"dm1\") == 0;"),
        "Firestaff currently catalogs CSB but m12_game_supported remains DM1-only.",
    ),
)

NON_CLAIMS = (
    "No CSB launch, rendering, gameplay, save compatibility, or New Adventure support is enabled.",
    "Atari ST and Amiga graphics payloads remain separate; this manifest is Atari ST only.",
    "The HCSB.HTC filename is the observed local Atari 8.3 payload matching the CSB README hcsb.hct support-file intent; it is not treated as a source-code spelling fix.",
    "config.txt is not present in the selected original-data anchor and remains a future local runtime-config input, not launch clearance.",
)


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


def display_path(path: Path) -> str:
    text = str(path)
    replacements = (
        (str(ORIG), "<firestaff-original-games>"),
        (str(CSB_SRC), "<csb-source>/CSB/src"),
        (str(CSBWIN), "<csbwin-source>/CSBWin"),
        (str(REDMCSB.parent.parent.parent), "<redmcsb-source>/ReDMCSB_WIP20210206"),
        (str(ROOT), "<firestaff-repo>"),
    )
    for prefix, repl in replacements:
        if text == prefix:
            return repl
        if text.startswith(prefix + "/"):
            return repl + text[len(prefix):]
    return text


def line_window(path: Path, window: str) -> str:
    start, end = [int(x) for x in window.split("-")]
    try:
        lines = path.read_text(errors="replace").splitlines()
    except Exception:
        return ""
    return "\n".join(lines[start - 1:end])


def main() -> int:
    failures: list[str] = []

    git_rows = []
    for ident, (path, expected) in EXPECTED_GIT.items():
        actual = git_head(path)
        ok = actual == expected
        if not ok:
            failures.append(f"{ident} HEAD mismatch: expected {expected}, actual {actual}")
        git_rows.append({"id": ident, "path": display_path(path), "expected_head": expected, "actual_head": actual, "ok": ok})

    asset_rows = []
    for ident, (path, size, digest, role) in EXPECTED_ASSETS.items():
        exists = path.exists()
        actual_size = path.stat().st_size if exists else None
        actual_digest = sha256(path) if exists else None
        ok = exists and actual_size == size and actual_digest == digest
        if not ok:
            failures.append(f"{ident} mismatch: exists={exists} size={actual_size} sha256={actual_digest}")
        asset_rows.append({
            "id": ident,
            "role": role,
            "path": display_path(path),
            "expected_size": size,
            "actual_size": actual_size,
            "expected_sha256": digest,
            "actual_sha256": actual_digest,
            "ok": ok,
        })

    source_rows = []
    for anchor in SOURCE_ANCHORS:
        path = Path(anchor.path)
        haystack = line_window(path, anchor.lines)
        missing = [needle for needle in anchor.needles if needle not in haystack]
        ok = path.exists() and not missing
        if not ok:
            failures.append(f"{anchor.id} missing {missing} path_exists={path.exists()}")
        row = asdict(anchor)
        row["path"] = display_path(path)
        row.update({"missing": missing, "ok": ok})
        source_rows.append(row)

    pair_ok = all(row["ok"] for row in asset_rows if row["role"] in {"pair", "canonical_pair"})
    runtime_support_ok = all(row["ok"] for row in asset_rows if row["role"] == "runtime_support")
    source_ok = all(row["ok"] for row in source_rows)
    manifest_ready = pair_ok and runtime_support_ok and source_ok and not failures

    result = {
        "schema": "firestaff.csb_v1_atari_asset_pair_manifest.v1",
        "pass": manifest_ready,
        "scope": "Selected Atari ST CSB V1 asset-pair and runtime-support payload manifest; evidence-only launch boundary.",
        "selected_lane": "csb_atari_st_v2x_harddisk_2009_02_22_pp",
        "asset_pair_ready": pair_ok,
        "runtime_support_payloads_ready": runtime_support_ok,
        "launch_intent_allowed": False,
        "launch_intent_blocker": "CSB remains unsupported in menu_startup_m12.c and still lacks an explicit experimental CSB launch-intent gate plus runtime/capture proof.",
        "asset_refs": asset_rows,
        "source_anchors": source_rows,
        "git_refs": git_rows,
        "non_claims": list(NON_CLAIMS),
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n")
    status = "PASS" if result["pass"] else "FAIL"
    print(f"{status} csb v1 atari asset-pair manifest: pair={pair_ok} runtime_support={runtime_support_ok} anchors={len(source_rows)} launch_intent_allowed=false")
    for failure in failures:
        print(f"- {failure}")
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
