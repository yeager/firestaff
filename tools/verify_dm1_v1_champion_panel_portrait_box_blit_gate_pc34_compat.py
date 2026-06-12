#!/usr/bin/env python3
"""Verify DM1 V1 champion-panel portrait box blit dispatch gate.

Source-locked to:
  - ReDMCSB CHAMDRAW.C F0292:757-1110 (redraw-mask, F0355 pre-route,
    status-box branch, dead branch, F0354 call, non-inventory
    fallback, nine-bit clear at exit)
  - ReDMCSB TIMELINE.C F0254:1614-1637 (HideDamageReceived secondary
    F0354 dispatch)
  - ReDMCSB CHAMDRAW.C F0293:1117-1143 (DrawAllChampionStates
    champion-index loop)
  - ReDMCSB DEFS.H:2188-2195 + 3793 (status-box / portrait zone base)

The verifier confirms the Firestaff helper pin the same source-lock
strings in the local header / source / test files, hits the
ReDMCSB anchors in CHAMDRAW.C / TIMELINE.C / DEFS.H, and runs the
contract-only ctest to record the assertion/failure counts.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

PASS = "dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat"
STATUS = "DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_BLIT_GATE_PC34_COMPAT_LOCKED"
FAILED_STATUS = "FAILED_DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_BLIT_GATE_PC34_COMPAT"

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path(os.environ.get("FIRESTAFF_BUILD_DIR", ROOT / "build-local"))
DATA = Path.home() / ".openclaw/data"
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")
MANIFEST = (
    ROOT
    / f"parity-evidence/verification/{PASS}/manifest.json"
)
REPORT = ROOT / f"parity-evidence/{PASS}.md"
TEST_BINARY = BUILD / "test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat"


def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]


RED = first_existing(
    "FIRESTAFF_REDMCSB_SOURCE",
    [
        DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    ],
)


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def slice_text(path: Path, span: str) -> tuple[int, str]:
    first, last = (int(part) for part in span.split("-", 1))
    lines = read_text(path).splitlines()
    return first, "\n".join(lines[first - 1:last])


def ordered_hits(
    body: str, base: int, needles: list[str]
) -> tuple[list[dict[str, Any]], list[str]]:
    cursor = 0
    hits: list[dict[str, Any]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append(
            {"line": base + body.count("\n", 0, pos), "needle": needle}
        )
        cursor = pos + len(needle)
    return hits, missing


def check_region(
    ident: str,
    path: Path,
    span: str,
    claim: str,
    needles: list[str],
) -> dict[str, Any]:
    base, body = slice_text(path, span)
    hits, missing = ordered_hits(body, base, needles)
    return {
        "id": ident,
        "file": str(path),
        "source": f"{path.name}:{span}",
        "claim": claim,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def check_file(
    ident: str, rel: str, needles: list[str]
) -> dict[str, Any]:
    path = ROOT / rel
    body = read_text(path)
    hits: list[dict[str, Any]] = []
    missing: list[str] = []
    for needle in needles:
        pos = body.find(needle)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append(
                {"line": 1 + body.count("\n", 0, pos), "needle": needle}
            )
    return {
        "id": ident,
        "file": rel,
        "status": "PASS" if not missing else "FAIL",
        "hits": hits,
        "missing": missing,
    }


def run_test_binary() -> dict[str, Any]:
    cmd = [str(TEST_BINARY)]
    if not TEST_BINARY.exists():
        return {
            "command": cmd,
            "returncode": 127,
            "passed": False,
            "assertions": 0,
            "failures": 1,
            "outputTail": "missing test binary; build test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat first",
        }
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=180,
    )
    assertions = 0
    failures = 1
    for line in proc.stdout.splitlines():
        if line.startswith("Assertions:"):
            assertions = int(line.split(":", 1)[1].strip())
        if line.startswith("Failures:"):
            failures = int(line.split(":", 1)[1].strip())
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0 and assertions >= 80 and failures == 0,

        "assertions": assertions,
        "failures": failures,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-18:]),
    }


REDMCSB_CHECKS = [
    (
        "champdraw_f0292_short_circuit",
        RED / "CHAMDRAW.C",
        "757-760",
        "F0292 short-circuits when none of the nine redraw-mask bits is set.",
        [
            "if (!M007_GET(L0862_ui_ChampionAttributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD | MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | MASK0x2000_WOUNDS | MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND))",
        ],
    ),
    (
        "champdraw_f0292_f0355_pre_route",
        RED / "CHAMDRAW.C",
        "767-770",
        "F0292 pre-routes the inventory champion through F0355 when G0297 is set.",
        [
            "if (L0863_B_IsInventoryChampion && G0297_B_DrawFloorAndCeilingRequested) {",
            "F0355_INVENTORY_Toggle_CPSE(C05_CHAMPION_SPECIAL_INVENTORY);",
        ],
    ),
    (
        "champdraw_f0292_status_box_branch",
        RED / "CHAMDRAW.C",
        "771-789",
        "F0292 enters the status-box branch only when MASK0x1000_STATUS_BOX is set.",
        [
            "if (M007_GET(L0862_ui_ChampionAttributes, MASK0x1000_STATUS_BOX)) {",
            "F0732_FillScreenArea(L2260_ai_XYZ, C12_COLOR_DARKEST_GRAY);",
        ],
    ),
    (
        "champdraw_f0292_f0354_call",
        RED / "CHAMDRAW.C",
        "810-812",
        "F0292 calls F0354 only when the champion is the inventory champion.",
        [
            "F0354_INVENTORY_DrawStatusBoxPortrait(P0615_ui_ChampionIndex);",
            "M008_SET(L0862_ui_ChampionAttributes, MASK0x0100_STATISTICS);",
        ],
    ),
    (
        "champdraw_f0292_non_inventory_fallback",
        RED / "CHAMDRAW.C",
        "813-814",
        "F0292 takes the non-inventory fallback mask when the champion is not the inventory champion.",
        [
            "M008_SET(L0862_ui_ChampionAttributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x2000_WOUNDS | MASK0x8000_ACTION_HAND);",
        ],
    ),
    (
        "champdraw_f0292_clear_at_exit",
        RED / "CHAMDRAW.C",
        "1110-1110",
        "F0292 clears all nine redraw-mask bits at the T0292042 label.",
        [
            "M009_CLEAR(L0865_ps_Champion->Attributes, MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD | MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX | MASK0x2000_WOUNDS | MASK0x4000_VIEWPORT | MASK0x8000_ACTION_HAND);",
        ],
    ),
    (
        "champdraw_f0293_dispatch_loop",
        RED / "CHAMDRAW.C",
        "1117-1143",
        "F0293 OR-s the per-call P2062_ui_ argument into every active champion's Attributes and dispatches F0292 in champion-index order.",
        [
            "for (L0873_ui_ChampionIndex = C00_CHAMPION_FIRST; L0873_ui_ChampionIndex < G0305_ui_PartyChampionCount; L0873_ui_ChampionIndex++) {",
            "M516_CHAMPIONS[L0873_ui_ChampionIndex].Attributes |= P2062_ui_;",
        ],
    ),
    (
        "timeline_f0254_secondary_dispatch",
        RED / "TIMELINE.C",
        "1614-1637",
        "F0254 routes the inventory champion through F0354 and the non-inventory champion through F0292 with NAME_TITLE only.",
        [
            "F0354_INVENTORY_DrawStatusBoxPortrait(P0531_ui_ChampionIndex);",
            "M008_SET(L0663_ps_Champion->Attributes, MASK0x0080_NAME_TITLE);",
            "F0292_CHAMPION_DrawState(P0531_ui_ChampionIndex);",
        ],
    ),
    (
        "defs_status_box_zones",
        RED / "DEFS.H",
        "3783-3793",
        "DEFS.H:3783-3793 anchors C151..C154 status-box zones and C175 first champion status-box portrait zone.",
        [
            "C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS",
            "C152_ZONE_CHAMPION_1_STATUS_BOX_NAME_HANDS",
            "C175_ZONE_FIRST_CHAMPION_STATUS_BOX",
        ],
    ),
    (
        "defs_portrait_zone",
        RED / "DEFS.H",
        "3793-3793",
        "DEFS.H:3793 anchors C175_ZONE_FIRST_CHAMPION_STATUS_BOX used by F0354.",
        [
            "#define C175_ZONE_FIRST_CHAMPION_STATUS_BOX",
        ],
    ),
]

LOCAL_CHECKS = [
    (
        "firestaff_header_contract",
        "include/dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.h",
        [
            "DM1 V1 champion-panel portrait box blit dispatch gate",
            "CHAMDRAW.C F0292:757-760",
            "CHAMDRAW.C F0292:767-770",
            "CHAMDRAW.C F0292:771",
            "CHAMDRAW.C F0292:784",
            "CHAMDRAW.C F0292:810-812",
            "CHAMDRAW.C F0292:813-814",
            "CHAMDRAW.C F0292:1110",
            "TIMELINE.C F0254:1614-1637",
            "CHAMDRAW.C F0293:1117-1143",
            "DEFS.H:3783-3793",
            "DEFS.H:3793",
            "DM1_V1_CPBBG_MASK_STATUS_BOX_PC34",
            "DM1_V1_CPBBG_POST_F0354_MASK_PC34",
            "DM1_V1_CPBBG_NON_INVENTORY_MASK_PC34",
            "DM1_V1_CPBBG_F0254_NON_INVENTORY_MASK_PC34",
            "DM1_V1_CPBBG_CLEAR_MASK_PC34",
            "DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34",
            "DM1_V1_CPBBG_PATH_DEAD_STATUS_BOX_PC34",
            "DM1_V1_CPBBG_PATH_NON_INVENTORY_REDRAW_PC34",
            "DM1_V1_CPBBG_DISPATCH_F0292_PC34",
            "DM1_V1_CPBBG_DISPATCH_F0254_PC34",
            "DM1_V1_CPBBG_DISPATCH_F0293_PC34",
        ],
    ),
    (
        "firestaff_module_source_lock",
        "src/dm1/dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.c",
        [
            "CHAMDRAW.C F0292:757-760",
            "CHAMDRAW.C F0292:767-770",
            "CHAMDRAW.C F0292:771",
            "CHAMDRAW.C F0292:784",
            "CHAMDRAW.C F0292:810-812",
            "CHAMDRAW.C F0292:813-814",
            "CHAMDRAW.C F0292:1110",
            "TIMELINE.C F0254:1614-1637",
            "CHAMDRAW.C F0293:1117-1143",
            "DEFS.H:3783-3793",
            "DEFS.H:3793",
            "no real-asset bitmap parity claim",
            "DM1_V1_CPBBG_F0254_NON_INVENTORY_MASK_PC34",
        ],
    ),
    (
        "firestaff_test_dispatch_tree",
        "tests/test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.c",
        [
            "test_inventory_champion_portrait_blit_default",
            "test_f0292_redraw_mask_short_circuit",
            "test_f0292_non_status_box_mask_skips_f0354",
            "test_f0292_dead_champion_skips_f0354",
            "test_f0292_non_inventory_champion_fallback",
            "test_f0292_f0355_inventory_pre_route",
            "test_f0292_full_mask_still_lands_on_f0354",
            "test_f0292_zone_bases_per_champion",
            "test_f0254_inventory_routes_through_f0354",
            "test_f0254_dead_champion_short_circuit",
            "test_f0254_non_inventory_routes_through_f0292",
            "test_f0293_champion_index_dispatch_order",
            "test_invalid_champion_index",
            "test_redraw_mask_table",
            "test_source_evidence",
            "Assertions: %d",
            "failures=0",
        ],
    ),
    (
        "cmake_registration",
        "CMakeLists.txt",
        [
            "test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat",
            "tests/test_dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.c",
            "src/dm1/dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.c",
            "dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat",
        ],
    ),
]


def collect_checks() -> list[dict[str, Any]]:
    rows = [check_region(*item) for item in REDMCSB_CHECKS]
    rows.extend(check_file(*item) for item in LOCAL_CHECKS)
    return rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--write-manifest", action="store_true",
        help="Write the verification manifest to parity-evidence/verification/.",
    )
    args = parser.parse_args()

    checks = collect_checks()
    failed = [row for row in checks if row["status"] != "PASS"]
    run = run_test_binary()

    status_token = STATUS
    if failed or not run["passed"]:
        status_token = FAILED_STATUS

    manifest = {
        "claim": (
            "DM1 V1 champion-panel portrait box blit dispatch gate is "
            "source-locked to ReDMCSB CHAMDRAW.C F0292:757-1110 + "
            "TIMELINE.C F0254:1614-1637 + CHAMDRAW.C F0293:1117-1143."
        ),
        "status": "passed" if not failed and run["passed"] else "failed",
        "statusToken": status_token,
        "schema": f"firestaff.parity.{PASS}.v1",
        "redmcsbRoot": str(RED),
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "anchorNotes": [
            "F0254:1635 sets only MASK0x0080_NAME_TITLE on the Attributes field "
            "before calling F0292 (the inventory champion's F0292 call would "
            "re-enter F0292 and trigger the F0292 dispatch tree; the test pins "
            "the F0254 non-inventory Attributes mask separately from the F0292 "
            "non-inventory fallback mask).",
        ],
        "firestaffChecks": [
            {
                "id": row["id"],
                "file": row.get("file"),
                "source": row.get("source"),
                "claim": row.get("claim"),
                "hits": row.get("hits", []),
                "missing": row.get("missing", []),
                "status": row["status"],
            }
            for row in checks
        ],
        "verificationRuns": [run],
    }

    if args.write_manifest:
        MANIFEST.parent.mkdir(parents=True, exist_ok=True)
        MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    print(f"Status: {status_token}")
    print(f"Source-lock checks: {len(checks) - len(failed)}/{len(checks)} passed")
    print(
        f"Test binary: rc={run['returncode']} assertions={run['assertions']} "
        f"failures={run['failures']} passed={run['passed']}"
    )
    if failed:
        for row in failed:
            print(f"  FAIL {row['id']}: {row.get('missing', [])}")
    return 0 if not failed and run["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
