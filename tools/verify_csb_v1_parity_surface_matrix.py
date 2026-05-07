#!/usr/bin/env python3
"""Verify the first CSB V1 parity surface matrix.

Evidence-only gate: defines countable CSB V1 parity surfaces and source-locks
only their acceptance boundary to local ReDMCSB/CSB lineage references. It does
not enable CSB launch, rendering, gameplay, save compatibility, or touch menu
runtime code.
"""
from __future__ import annotations

import json
import subprocess
from dataclasses import asdict, dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/csb_v1_parity_surface_matrix.json"
REDMCSB = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"
ORIG = (Path.home() / ".openclaw/data/firestaff-original-games/DM")

EXPECTED_GIT = {
    "csb_lineage": (CSB_SRC, "dda570585abb4c8113a3298d21c0b599e6cac4f9"),
    "csbwin_lineage": (CSBWIN, "2f63d10d9b8c155e0be17888271d394255ce1bac"),
}

EXPECTED_ASSETS = {
    "atari_v20_graphics": (ORIG / "_extracted/csb-atari/HardDisk/2009-02-22 PP/GRAPHICS.DAT", 319080, "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af"),
    "atari_v20_dungeon": (ORIG / "_extracted/csb-atari/HardDisk/2009-02-22 PP/DUNGEON.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
    "canonical_atari_graphics": (ORIG / "_canonical/csb/atari-GRAPHICS.DAT", 319080, "33f672bf644763411cc465e3553e0605de77e6128070dbd27868813e2a21d9af"),
    "canonical_atari_dungeon": (ORIG / "_canonical/csb/atari-DUNGEON.DAT", 2098, "3cafd2fb9f255df93e99ae27d4bf60ff22cc8e43cfa90de7d29c04172b2542ba"),
}

@dataclass(frozen=True)
class Anchor:
    id: str
    path: str
    lines: str
    needles: tuple[str, ...]

@dataclass(frozen=True)
class Surface:
    id: str
    status: str
    acceptance_boundary: str
    required_evidence: tuple[str, ...]
    anchors: tuple[Anchor, ...]

SURFACES = [
    Surface(
        id="startup_load_assets",
        status="SOURCE_LOCKED_BLOCKED_RUNTIME",
        acceptance_boundary="CSB startup/load cannot be counted until Atari ST v2.x GRAPHICS.DAT and DUNGEON.DAT identity plus CSB support files are verified by a CSB-specific gate.",
        required_evidence=("atari_v20_graphics", "atari_v20_dungeon", "canonical_atari_graphics", "canonical_atari_dungeon"),
        anchors=(
            Anchor("csb_required_payloads", str(CSB_SRC / "README"), "1-30", ("dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt")),
            Anchor("csb_graphics_open", str(CSB_SRC / "Graphics.cpp"), "1740-1830", ("graphics.dat", "Cannot find 'graphics.dat'")),
        ),
    ),
    Surface(
        id="dungeon_load_and_state",
        status="SOURCE_LOCKED_BLOCKED_RUNTIME",
        acceptance_boundary="Dungeon-load parity must prove CSB dungeon identity/state from the curated Atari dungeon payload before gameplay/rendering counts.",
        required_evidence=("atari_v20_dungeon", "canonical_atari_dungeon"),
        anchors=(
            Anchor("redmcsb_dungeon_ids", str(REDMCSB / "DEFS.H"), "519-523", ("C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME")),
            Anchor("csb_dungeon_index_usage", str(CSBWIN / "SaveGame.cpp"), "160-190", ("dungeonDatIndex", "NumWordsInTextArray")),
        ),
    ),
    Surface(
        id="prison_champion_route",
        status="SOURCE_LOCKED_BLOCKED_RUNTIME",
        acceptance_boundary="Prison/champion flow counts only after the Utility/Make-New-Adventure route is reproduced from a CSB prison/game state, not by DM1 champion routing reuse.",
        required_evidence=(),
        anchors=(
            Anchor("csbwin_play_workflow", str(CSBWIN / "Game/readme.txt"), "1-30", ("Enter the dungeon", "choose prison", "Make New Adventure")),
            Anchor("csb_utility_buttons", str(CSB_SRC / "Chaos.cpp"), "4545-4555", ("LOAD CHAMPIONS", "SAVE CHAMPIONS", "MAKE NEW ADVENTURE")),
        ),
    ),
    Surface(
        id="viewport_hud_rendering",
        status="SOURCE_LOCKED_BLOCKED_CAPTURE",
        acceptance_boundary="Viewport/HUD parity remains blocked until a CSB original capture path yields stable frame/state anchors tied to the Atari v2.x renderer lane.",
        required_evidence=("atari_v20_graphics", "canonical_atari_graphics"),
        anchors=(
            Anchor("redmcsb_viewport_boxes", str(REDMCSB / "DUNVIEW.C"), "380-390", ("full dungeon view", "ThievesEye_ViewportVisibleArea")),
            Anchor("csbwin_viewport_trace", str(CSBWIN / "CSBwin.cpp"), "1368-1385", ("traceViewportDrawing", "MF_CHECKED")),
        ),
    ),
    Surface(
        id="input_and_mode_routing",
        status="SOURCE_LOCKED_BLOCKED_RUNTIME",
        acceptance_boundary="Input parity must prove CSB mode-specific mouse/keyboard routing from reference state, including Utility/reincarnate modes, before controls count.",
        required_evidence=(),
        anchors=(
            Anchor("csb_keyboard_modes", str(CSBWIN / "data.cpp"), "1740-1755", ("keyboardMode=1", "adventuring")),
            Anchor("csb_reincarnate_mode", str(CSB_SRC / "Mouse.cpp"), "1410-1425", ("keyboardMode == 2", "Reincarnate mode")),
        ),
    ),
    Surface(
        id="save_new_adventure",
        status="BLOCKED_MISSING_SAMPLE",
        acceptance_boundary="Save/new-adventure parity cannot be counted without a curated extracted CSBGAME*.DAT/BAK sample or generated-save harness with recorded provenance.",
        required_evidence=(),
        anchors=(
            Anchor("redmcsb_csb_save_header", str(REDMCSB / "DEFS.H"), "468-498", ("CSB_SAVE_HEADER", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK")),
            Anchor("redmcsb_new_adventure_gate", str(REDMCSB / "CEDTINCH.C"), "5-64", ("F7086_IsReadyToMakeNewAdventure", "C13_DUNGEON_CSB_GAME")),
            Anchor("firestaff_sample_blocker", str(ROOT / "parity-evidence/blocker-n2-csb-sample-save-search-20260430.md"), "1-20", ("BLOCKED", "CSBGAME*.DAT")),
        ),
    ),
]

NON_CLAIMS = (
    "No CSB launch, rendering, gameplay, or save compatibility is enabled or claimed.",
    "No menu_startup_m12.c or other launch/runtime code is modified by this gate.",
    "Atari ST and Amiga CSB payloads remain separate; this matrix anchors the current Atari ST v2.x lane only.",
)


def sha256(path: Path) -> str:
    import hashlib
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
        git_rows.append({"id": ident, "path": str(path), "expected_head": expected, "actual_head": actual, "ok": ok})

    asset_rows = []
    for ident, (path, size, digest) in EXPECTED_ASSETS.items():
        exists = path.exists()
        actual_size = path.stat().st_size if exists else None
        actual_digest = sha256(path) if exists else None
        ok = exists and actual_size == size and actual_digest == digest
        if not ok:
            failures.append(f"{ident} asset mismatch: exists={exists} size={actual_size} sha256={actual_digest}")
        asset_rows.append({"id": ident, "path": str(path), "expected_size": size, "actual_size": actual_size, "expected_sha256": digest, "actual_sha256": actual_digest, "ok": ok})

    source_rows = []
    for surface in SURFACES:
        for anchor in surface.anchors:
            path = Path(anchor.path)
            haystack = line_window(path, anchor.lines)
            missing = [needle for needle in anchor.needles if needle not in haystack]
            ok = path.exists() and not missing
            if not ok:
                failures.append(f"{surface.id}/{anchor.id} missing {missing} path_exists={path.exists()}")
            source_rows.append({
                "surface": surface.id,
                "id": anchor.id,
                "path": str(path),
                "lines": anchor.lines,
                "needles": list(anchor.needles),
                "missing": missing,
                "ok": ok,
            })

    surface_rows = []
    for surface in SURFACES:
        unknown_required = [req for req in surface.required_evidence if req not in EXPECTED_ASSETS]
        if unknown_required:
            failures.append(f"{surface.id} has unknown required_evidence ids: {unknown_required}")
        surface_rows.append({
            "id": surface.id,
            "status": surface.status,
            "acceptance_boundary": surface.acceptance_boundary,
            "required_evidence": list(surface.required_evidence),
            "anchors": [asdict(anchor) for anchor in surface.anchors],
        })

    result = {
        "schema": "firestaff.csb_v1_parity_surface_matrix.v1",
        "pass": not failures,
        "scope": "First countable CSB V1 parity surface matrix; source/evidence boundary only.",
        "surface_count": len(surface_rows),
        "surfaces": surface_rows,
        "source_anchors": source_rows,
        "asset_refs": asset_rows,
        "git_refs": git_rows,
        "non_claims": list(NON_CLAIMS),
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n")
    status = "PASS" if result["pass"] else "FAIL"
    print(f"{status} csb v1 parity surface matrix: {len(surface_rows)} surfaces, {len(source_rows)} anchors, {len(asset_rows)} asset refs")
    for failure in failures:
        print(f"- {failure}")
    return 0 if result["pass"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
