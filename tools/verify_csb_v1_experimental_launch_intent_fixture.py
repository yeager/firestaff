#!/usr/bin/env python3
"""Verify the experimental CSB V1 launch-intent fixture boundary.

Evidence-only gate. This consumes the Atari ST media manifest and records the
smallest launch-intent contract Firestaff may eventually wire for CSB. It does
not enable production CSB launch; the current M12 menu handoff must remain
DM1-only until a separate runtime/capture gate exists.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/csb_v1_experimental_launch_intent_fixture.json"
MEDIA_MANIFEST = ROOT / "parity-evidence/verification/csb_v1_atari_asset_pair_manifest.json"
MENU = ROOT / "menu_startup_m12.c"
DOC = ROOT / "docs/parity/PARITY_MATRIX_CSB_V1.md"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"

FIXTURE = {
    "schema": "firestaff.csb_v1.experimental_launch_intent_fixture.v1",
    "fixture_id": "csb_v1_atari_st_v2x_experimental_launch_intent",
    "experiment_only": True,
    "production_launch_enabled": False,
    "selected_lane": "csb_atari_st_v2x_harddisk_2009_02_22_pp",
    "game_id": "csb",
    "version_id": "atari-st-v20",
    "presentation_mode": "M12_PRESENTATION_V1_ORIGINAL",
    "renderer_backend": "M12_RENDERER_BACKEND_SOFTWARE",
    "required_manifest_schema": "firestaff.csb_v1_atari_asset_pair_manifest.v1",
    "required_media_roles": ["pair", "runtime_support", "canonical_pair"],
    "required_runtime_payloads": ["GRAPHICS.DAT", "DUNGEON.DAT", "HCSB.DAT", "HCSB.HTC", "MINI.DAT"],
    "deferred_runtime_inputs": ["config.txt", "CSBGAME.DAT/csbgame.dat save-game slot", "CSB prison -> Make New Adventure workflow state"],
    "valid_intent_allowed": False,
}

SOURCE_ANCHORS = [
    {
        "id": "redmcsb_primary_csb_dungeon_ids",
        "path": REDMCSB / "DEFS.H",
        "lines": "519-523",
        "needles": ["C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME", "Value used in CSB MINI.DAT"],
    },
    {
        "id": "redmcsb_primary_new_adventure_gate",
        "path": REDMCSB / "CEDTINCH.C",
        "lines": "5-63",
        "needles": ["F7086_IsReadyToMakeNewAdventure", "GameLoaded", "G7114_LoadedChampionCount", "C13_DUNGEON_CSB_GAME"],
    },
    {
        "id": "csb_lineage_required_payloads",
        "path": CSB_SRC / "README",
        "lines": "14-21",
        "needles": ["dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt"],
    },
    {
        "id": "csbwin_workflow_boundary",
        "path": CSBWIN / "Game/readme.txt",
        "lines": "1-30",
        "needles": ["Enter the dungeon", "choose prison", "Make New Adventure", "Restore the newly created save game"],
    },
]

NON_CLAIMS = [
    "No CSB runtime, rendering, gameplay, save compatibility, or pixel parity is enabled by this fixture.",
    "The fixture is an experiment-only contract and must not make M12_StartupMenu_GetLaunchIntent return a valid CSB intent.",
    "The selected Atari ST media manifest is necessary but not sufficient launch clearance.",
]


def line_window(path: Path, span: str) -> str:
    start, end = [int(x) for x in span.split("-")]
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    return "\n".join(lines[start - 1:end])


def main() -> int:
    failures: list[str] = []

    try:
        media = json.loads(MEDIA_MANIFEST.read_text(encoding="utf-8"))
    except Exception as exc:
        media = {}
        failures.append(f"cannot read media manifest: {exc}")

    if media.get("schema") != FIXTURE["required_manifest_schema"] or not media.get("pass"):
        failures.append("required CSB Atari media manifest is missing, wrong schema, or failing")
    if not media.get("asset_pair_ready") or not media.get("runtime_support_payloads_ready"):
        failures.append("media manifest does not prove both asset pair and runtime support payloads")
    if media.get("launch_intent_allowed") is not False:
        failures.append("media manifest must keep launch_intent_allowed=false")

    asset_roles = {row.get("role") for row in media.get("asset_refs", []) if row.get("ok")}
    for role in FIXTURE["required_media_roles"]:
        if role not in asset_roles:
            failures.append(f"media manifest missing ok role {role}")
    asset_paths = "\n".join(row.get("path", "") for row in media.get("asset_refs", []) if row.get("ok"))
    for payload in FIXTURE["required_runtime_payloads"]:
        if payload not in asset_paths:
            failures.append(f"media manifest missing payload path containing {payload}")

    menu = MENU.read_text(encoding="utf-8", errors="replace") if MENU.exists() else ""
    if 'return gameId && strcmp(gameId, "dm1") == 0;' not in menu:
        failures.append("M12 production launch support is no longer DM1-only; fixture must be reviewed before CSB launch")
    if "intent.valid = m12_game_supported(intent.gameId)" not in menu:
        failures.append("M12 launch intent validity is no longer guarded by m12_game_supported")

    doc = DOC.read_text(encoding="utf-8", errors="replace") if DOC.exists() else ""
    if "experimental CSB launch-intent fixture" not in doc:
        failures.append("CSB parity doc must mention the experimental launch-intent fixture boundary")

    source_rows = []
    for anchor in SOURCE_ANCHORS:
        haystack = line_window(anchor["path"], anchor["lines"])
        missing = [needle for needle in anchor["needles"] if needle not in haystack]
        ok = anchor["path"].exists() and not missing
        if not ok:
            failures.append(f"source anchor {anchor['id']} missing {missing} exists={anchor['path'].exists()}")
        source_rows.append({
            "id": anchor["id"],
            "path": str(anchor["path"]),
            "lines": anchor["lines"],
            "needles": anchor["needles"],
            "missing": missing,
            "ok": ok,
        })

    result = {
        "schema": FIXTURE["schema"],
        "pass": not failures,
        "fixture": FIXTURE,
        "media_manifest": {
            "path": str(MEDIA_MANIFEST.relative_to(ROOT)),
            "schema": media.get("schema"),
            "pass": media.get("pass"),
            "asset_pair_ready": media.get("asset_pair_ready"),
            "runtime_support_payloads_ready": media.get("runtime_support_payloads_ready"),
            "launch_intent_allowed": media.get("launch_intent_allowed"),
        },
        "production_guard": {
            "path": str(MENU.relative_to(ROOT)),
            "dm1_only_supported": 'return gameId && strcmp(gameId, "dm1") == 0;' in menu,
            "intent_valid_guarded_by_supported_game": "intent.valid = m12_game_supported(intent.gameId)" in menu,
        },
        "source_anchors": source_rows,
        "non_claims": NON_CLAIMS,
        "failures": failures,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    status = "PASS" if result["pass"] else "FAIL"
    print(f"{status} csb v1 experimental launch-intent fixture: experiment_only=true production_launch_enabled=false payloads={len(FIXTURE['required_runtime_payloads'])}")
    for failure in failures:
        print(f"- {failure}")
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
