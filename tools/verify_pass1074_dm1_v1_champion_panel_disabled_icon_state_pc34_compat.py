#!/usr/bin/env python3
"""
pass1074 — DM1 V1 champion-panel disabled/unavailable shield/action
icon state gate verifier.

Source-locked to ReDMCSB:
  - CHAMPION.C F0330_CHAMPION_DisableAction:2208-2255
    (M008_SET MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION,
     then F0292 redraw, then C11_EVENT_ENABLE_CHAMPION_ACTION)
  - ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-296
    (dead-champion early-return L1183 + empty-hand C201 blit
     + line 282 hatch gate on MASK0x0008_DISABLE_ACTION ||
     G0299_ui_CandidateChampionOrdinal || G0300_B_PartyIsResting)
  - MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]:27,157
    (per-action disabled-tick table)
  - m11_collect_v1_status_shield_border_graphics
    (M11 m11_game_view.c append loop returns 0 borders when
     partyShieldDefense / spellShieldDefense / fireShieldDefense
     are all 0 — the disabled side of the shield-border state)

Companion to (and disjoint from):
  - dm1_v1_graphic560_action_disabled_ticks_pc34_compat
    (pass922 G0491 table bytes — separate gate, this gate
     re-uses the values for the per-action predicate).
  - firestaff_dm1_v1_champion_panel_shield_border_pixel_probe
    (asset-backed ENABLED shield pixel probe — separate lane).
  - firestaff_dm1_v1_champion_panel_icon_direction_swap_runtime_probe
    (asset-backed ENABLED icon-direction pixel probe — separate lane).

This verifier is contract-only.  It does NOT load real assets,
run DOSBox, or claim original-vs-Firestaff parity.
"""
from __future__ import annotations

import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PASS = "pass1074_dm1_v1_champion_panel_disabled_icon_state_pc34_compat"
STATUS = "PASS1074_DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_LOCKED"

SRC = ROOT / "src/dm1/dm1_v1_champion_panel_disabled_icon_state_pc34_compat.c"
HDR = ROOT / "include/dm1_v1_champion_panel_disabled_icon_state_pc34_compat.h"
TEST = ROOT / "tests/test_dm1_v1_champion_panel_disabled_icon_state_pc34_compat.c"
CMAKE = ROOT / "CMakeLists.txt"
BINARY_NAME = "test_dm1_v1_champion_panel_disabled_icon_state_pc34_compat"

OUT_DIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

ANCHORS = [
    "CHAMPION.C F0330:2208-2255",
    "CHAMPION.C F0330:2252 M008_SET",
    "CHAMPION.C F0330:2253-2255 C11_EVENT_ENABLE_CHAMPION_ACTION",
    "ACTIDRAW.C F0386:201-296",
    "ACTIDRAW.C F0386_MENUS_DrawActionIcon:282-286",
    "ACTIDRAW.C F0386_MENUS_DrawActionIcon:234-238",
    "ACTIDRAW.C F0386_MENUS_DrawActionIcon:262-264",
    "MENU.C G0491_auc_Graphic560_ActionDisabledTicks[44]",
    "MENU.C:27,157",
    "m11_collect_v1_status_shield_border_graphics",
    "M11_GameView_ShouldHatchV1ActionIconCells",
    "DEFS.H MASK0x8000_ACTION_HAND",
    "DEFS.H MASK0x0008_DISABLE_ACTION",
]

LOCAL_NEEDLES = [
    "DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat",
    "DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat",
    "DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat",
    "DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat",
    "MASK0x8000_ACTION_HAND | MASK0x0008_DISABLE_ACTION",
    "MASK0x0008_DISABLE_ACTION",
    "G0299_ui_CandidateChampionOrdinal",
    "G0300_B_PartyIsResting",
    "C201_ICON_ACTION_ICON_EMPTY_HAND",
    "Disjoint from pass784-790",
    "Disjoint from pass791",
    "Disjoint from pass793",
    "Disjoint from pass794",
    "Disjoint from pass922",
    "Disjoint from shield-border",
    "Disjoint from icon-direction",
    "contract-only",
]

CMAKE_NEEDLES = [
    "test_dm1_v1_champion_panel_disabled_icon_state_pc34_compat",
    "src/dm1/dm1_v1_champion_panel_disabled_icon_state_pc34_compat.c",
    "NAME dm1_v1_champion_panel_disabled_icon_state_pc34_compat",
]


def read(path):
    return path.read_text(encoding="utf-8", errors="replace")


def check_needles(label, path, needles):
    text = read(path)
    missing = [n for n in needles if n not in text]
    return {
        "id": label,
        "file": str(path.relative_to(ROOT)),
        "status": "PASS" if not missing else "FAIL",
        "missing": missing,
    }


def resolve_build_dir():
    candidates = [
        ROOT / "build",
        ROOT / "builds" / "nv1-build",
        ROOT / "builds" / "n2-build",
    ]
    for c in candidates:
        if (c / "CMakeCache.txt").exists() and (c / BINARY_NAME).exists():
            return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]


def run_test(build_dir):
    binary = build_dir / BINARY_NAME
    if not binary.exists():
        return None, f"binary not found: {binary}"
    proc = subprocess.run(
        [str(binary)],
        cwd=ROOT,
        capture_output=True,
        text=True,
        timeout=120,
    )
    return proc, None


def repo_relative_command(path):
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def parse_output(text):
    passes = text.count("PASS ")
    fails = text.count("FAIL ")
    assertions = 0
    final_marker = ""
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("Assertions:"):
            try:
                assertions = int(s.split(":", 1)[1].strip())
            except (ValueError, IndexError):
                pass
        elif s.startswith("Failures:"):
            try:
                _ = int(s.split(":", 1)[1].strip())
            except (ValueError, IndexError):
                pass
        if "_PC34_COMPAT_OK" in s:
            final_marker = s
    return passes, fails, assertions, final_marker


def write_outputs(local_checks, runs):
    ok = all(r["status"] == "PASS" for r in local_checks) and all(
        r["passed"] for r in runs
    )
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else f"FAILED_{STATUS}",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": (
            "DM1 V1 champion-panel DISABLED/UNAVAILABLE shield/action "
            "icon state contract — disjoint from existing shield-border "
            "ENABLED pixel coverage and existing icon-direction ENABLED "
            "coverage."
        ),
        "anchors": ANCHORS,
        "sourceChecks": local_checks,
        "verificationRuns": runs,
        "nonOverlap": [
            "Disjoint from pass784-790 (mirror-candidate C040 + wound).",
            "Disjoint from pass791 (champion-panel ammunition-compat).",
            "Disjoint from pass793 (action-hand slot-priority).",
            "Disjoint from pass794 (all-states redraw dispatcher).",
            "Disjoint from pass795-797 (leader/mirror/chest-action-hand).",
            "Disjoint from pass798-806 (graphics.dat init-table gates).",
            "Disjoint from pass922 (G0491 table bytes — separate gate; "
            "this gate re-uses the values for the per-action predicate).",
            "Disjoint from M11_GetV1StatusShieldBorderGraphicForChampion "
            "+ the firestaff_dm1_v1_champion_panel_shield_border_pixel_"
            "probe ENABLED pixel path.",
            "Disjoint from M11_GetV1ChampionIconSourceIndex + the "
            "firestaff_dm1_v1_champion_panel_icon_direction_swap_runtime_"
            "probe ENABLED direction-swap path.",
        ],
        "nonClaims": [
            "No real GRAPHICS.DAT or DUNGEON.DAT load.",
            "No live rendering, no DOSBox / dosbox-debug / dosbox-x capture.",
            "No Firestaff-vs-original pixel parity claim.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    rl = [
        f"# {PASS}",
        "",
        f"- Status: {manifest['status']}",
        "",
        "## Source-locked anchors",
        "",
    ]
    for a in ANCHORS:
        rl.append(f"- {a}")
    rl.append("")
    rl.append("## Verification runs")
    rl.append("")
    for r in runs:
        rl.append(
            f"- `{ ' '.join(r['command']) }`: "
            f"rc={r['returncode']} passes={r['passes']} "
            f"fails={r['fails']} assertions={r['assertions']}"
        )
    REPORT.write_text("\n".join(rl) + "\n")


def main():
    local_checks = [
        check_needles("source_required_anchors", SRC, ANCHORS),
        check_needles("source_runtime_contract", SRC, LOCAL_NEEDLES),
        check_needles("header_api_surface", HDR, [
            "DM1_V1_ChampionPanelDisabledIconState_DisabledTicksPc34Compat",
            "DM1_V1_ChampionPanelDisabledIconState_ResolvePc34Compat",
            "DM1_V1_ChampionPanelDisabledIconState_ApplyDisableActionPc34Compat",
            "DM1_V1_ChampionPanelDisabledIconState_AdvanceTimelinePc34Compat",
            "MASK0x8000_ACTION_HAND",
            "MASK0x0008_DISABLE_ACTION",
            "C201_ICON_ACTION_ICON_EMPTY_HAND",
        ]),
        check_needles("test_entry_and_assertions", TEST, [
            "test_evidence_and_invariants",
            "test_g0491_disabled_ticks_table",
            "test_apply_disable_action_attribute_paths",
            "test_advance_timeline_enable_event",
            "test_global_hatch_gates",
            "test_dead_champion_early_return",
            "test_empty_hand_available",
            "test_shield_border_disabled_state",
            "test_per_champion_priority_over_global",
            "test_boundary_clamps",
            "test_party_hole_rejected_without_row_drift",
            "test_state_name_strings",
            "DM1_V1_CHAMPION_PANEL_DISABLED_ICON_STATE_PC34_COMPAT_OK",
        ]),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    build_dir = resolve_build_dir()
    proc, err = run_test(build_dir)
    if proc is None:
        runs = [{
            "command": [repo_relative_command(build_dir / BINARY_NAME)],
            "returncode": -1,
            "passed": False,
            "passes": 0,
            "fails": 0,
            "assertions": 0,
            "error": err,
        }]
    else:
        passes, fails, assertions, final = parse_output(
            proc.stdout + proc.stderr
        )
        runs = [{
            "command": [repo_relative_command(build_dir / BINARY_NAME)],
            "returncode": proc.returncode,
            "passed": proc.returncode == 0 and fails == 0,
            "passes": passes,
            "fails": fails,
            "assertions": assertions,
            "finalMarker": final,
        }]
    write_outputs(local_checks, runs)
    ok = all(r["status"] == "PASS" for r in local_checks) and all(
        r["passed"] for r in runs
    )
    print(f"{PASS}: {'PASS' if ok else 'FAIL'}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    for r in local_checks:
        if r["status"] != "PASS":
            print(f"missing in {r['id']}: {r['missing']}")
    for r in runs:
        if r.get("error"):
            print(f"run error: {r['error']}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
