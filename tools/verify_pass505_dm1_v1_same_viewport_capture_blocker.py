#!/usr/bin/env python3
"""Pass505: lock the same-viewport capture blocker after pass502/pass498."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass505_dm1_v1_same_viewport_capture_blocker"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PASS502_REPORT = ROOT / "parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md"
PASS498 = ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json"
PASS487 = ROOT / "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json"
STATUS = "PASS505_SAME_VIEWPORT_CAPTURE_BLOCKER_LOCKED"

SOURCE_LOCKS = [
    {"id": "game_loop_next_f0128_after_input_wait", "file": "GAMELOOP.C", "lines": "90,164,215-219", "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "A post-command screenshot is promotable only after the command wait boundary permits the next draw."},
    {"id": "f0380_dequeues_to_f0365_f0366", "file": "COMMAND.C", "lines": "2045-2156", "needles": ["L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "G2153_i_QueuedCommandsCount--;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "claim": "Host labels are insufficient; the original run must prove a real dequeued turn/move command."},
    {"id": "f0365_turn_state_delta", "file": "CLIKMENU.C", "lines": "142-174", "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0284_CHAMPION_SetPartyDirection"], "claim": "Accepted turns mutate party direction before the next viewport draw."},
    {"id": "f0366_move_state_delta", "file": "CLIKMENU.C", "lines": "180-347", "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"], "claim": "Accepted steps must be tied to destination/move-result state, not only a captured wall closeup."},
    {"id": "f0128_uses_party_tuple_then_presents", "file": "DUNVIEW.C", "lines": "8318-8610", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "The original frame must bind to the same direction/X/Y tuple used for viewport composition."},
    {"id": "f0097_viewport_present_boundary", "file": "DRAWVIEW.C", "lines": "709-858", "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE;", "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"], "claim": "The compared screenshot must be at or after the G0296 viewport present boundary."},
]

FRESH_ATTEMPT = {
    "host": "N2 firestaff-worker",
    "attemptDir": "/tmp/pass505-dm1v1-original-route-attempt",
    "dateUtc": "2026-05-13",
    "commandShape": "N2 PC34 route with enter/click/kp4/kp6/kp8/kp4/kp6 and six labeled shots",
    "classifier": "python3 tools/pass80_original_frame_classifier.py /tmp/pass505-dm1v1-original-route-attempt",
    "captureCount": 6,
    "dimensionsSeen": {"320x200": 6},
    "classCounts": {"dungeon_gameplay": 4, "wall_closeup": 2},
    "rawSha256Counts": {
        "48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397": 4,
        "fbeb1b82cd096c15c2346f254d9b2b2e8c1a8d0b8d100ba1751c4230c51e3dde": 2,
    },
    "viewportCropSha256Counts": {
        "701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c": 4,
        "1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81": 2,
    },
    "routeLabels": ["readiness_preflight", "turn_left_after_vblank", "turn_right_after_vblank", "forward_after_vblank", "turn_left_2_after_vblank", "post_redraw_after_vblank"],
    "cropFilenames": ["01_ingame_start_original_viewport_224x136.ppm", "02_ingame_turn_right_original_viewport_224x136.ppm", "03_ingame_move_forward_original_viewport_224x136.ppm", "04_ingame_spell_panel_original_viewport_224x136.ppm", "05_ingame_after_cast_original_viewport_224x136.ppm", "06_ingame_inventory_panel_original_viewport_224x136.ppm"],
    "interpretation": "The attempt reaches original gameplay and produces two visible viewport states, but six semantic labels collapse to 4/2 duplicate raw and viewport-crop hashes. The filenames are generic crop slots that drift from the route labels, and the attempt does not include source-bound F0380/F0365/F0366/F0128/F0097 stops.",
}


def compact(text: str) -> str:
    return " ".join(text.split())


def read_json(path: Path) -> dict:
    if not path.exists():
        raise AssertionError(f"missing manifest: {path}")
    return json.loads(path.read_text(encoding="utf-8"))


def source_window(ref: dict) -> str:
    path = RED / ref["file"]
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in ref["lines"].split(","):
        if "-" in part:
            lo, hi = [int(value) for value in part.split("-", 1)]
        else:
            lo = hi = int(part)
        out.extend(lines[lo - 1:hi])
    return "\n".join(out)


def audit_source(ref: dict) -> dict:
    text = compact(source_window(ref))
    missing = [needle for needle in ref["needles"] if compact(needle) not in text]
    return {k: ref[k] for k in ("id", "file", "lines", "claim")} | {"ok": not missing, "missing": missing}


def main() -> int:
    source = [audit_source(ref) for ref in SOURCE_LOCKS]
    pass498 = read_json(PASS498)
    pass487 = read_json(PASS487)
    pass502 = PASS502_REPORT.read_text(encoding="utf-8") if PASS502_REPORT.exists() else ""

    raw_unique = len(FRESH_ATTEMPT["rawSha256Counts"])
    crop_unique = len(FRESH_ATTEMPT["viewportCropSha256Counts"])
    label_count = len(FRESH_ATTEMPT["routeLabels"])
    route_has_some_diversity = raw_unique > 1 and crop_unique > 1
    route_has_full_state_delta = raw_unique == label_count and crop_unique == label_count
    source_bound_stops_present = False
    paired_firestaff_capture_present = False

    required = {
        "redmcsb_source_audit_ok": all(row["ok"] for row in source),
        "pass498_boundary_ok": pass498.get("ok") is True and pass498.get("status") == "PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA",
        "pass487_static_blocker_ok": pass487.get("ok") is True,
        "pass502_same_viewport_requirement_present": "same-viewport runtime capture" in pass502 and "no new pixel-parity promotion" in pass502,
        "fresh_attempt_reaches_gameplay_or_wall": set(FRESH_ATTEMPT["classCounts"]) == {"dungeon_gameplay", "wall_closeup"},
        "fresh_attempt_has_some_route_diversity": route_has_some_diversity,
        "fresh_attempt_still_lacks_full_state_delta": not route_has_full_state_delta,
        "fresh_attempt_lacks_source_bound_runtime_stops": not source_bound_stops_present,
        "fresh_attempt_lacks_paired_firestaff_capture": not paired_firestaff_capture_present,
    }
    problems = [name for name, ok in required.items() if not ok]
    problems.extend(f"source lock failed: {row['id']} missing {row['missing']}" for row in source if not row["ok"])

    decision = "Runtime capture cannot be promoted yet. The fresh N2 original route produced two viewport states, so the capture path is not totally static, but the six post-entry labels still collapse to duplicate 4/2 hashes and no same-run source stops bind those frames to F0380/F0365/F0366 followed by F0128/F0097. No paired Firestaff frame for the same direction/X/Y/wall state is present."
    next_unblocker = "Capture one original frame and one Firestaff frame for the same map/X/Y/direction and wall/door state, with the original run proving F0380 pop/load -> F0365/F0366 state mutation -> later F0128 tuple composition -> F0097/VIDRV present for that shot. Until then, pass502 stays source/probe evidence only."
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if not problems else "FAIL_PASS505_SAME_VIEWPORT_CAPTURE_BLOCKER",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "inputs": {"pass502Report": str(PASS502_REPORT.relative_to(ROOT)), "pass498Manifest": str(PASS498.relative_to(ROOT)), "pass487Manifest": str(PASS487.relative_to(ROOT))},
        "freshOriginalAttempt": FRESH_ATTEMPT,
        "derived": {"rawUniqueHashCount": raw_unique, "viewportCropUniqueHashCount": crop_unique, "routeLabelCount": label_count, "routeHasSomeDiversity": route_has_some_diversity, "routeHasFullPerLabelStateDelta": route_has_full_state_delta, "sourceBoundStopsPresent": source_bound_stops_present, "pairedFirestaffCapturePresent": paired_firestaff_capture_present},
        "required": required,
        "decision": decision,
        "nextUnblocker": next_unblocker,
        "nonClaims": ["no original-vs-Firestaff pixel parity", "no promotion of pass505 images as reference artifacts", "no claim that F0365/F0366 are broken", "no DANNESBURK input or paths used"],
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass505 - DM1 V1 same-viewport capture blocker",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "## Decision",
        "",
        decision,
        "",
        "## Fresh N2 original attempt",
        "",
        f"- capture count: `{FRESH_ATTEMPT['captureCount']}` at `320x200`",
        f"- classes: `{FRESH_ATTEMPT['classCounts']}`",
        f"- raw hash counts: `{FRESH_ATTEMPT['rawSha256Counts']}`",
        f"- viewport crop hash counts: `{FRESH_ATTEMPT['viewportCropSha256Counts']}`",
        f"- route labels: `{', '.join(FRESH_ATTEMPT['routeLabels'])}`",
        f"- interpretation: {FRESH_ATTEMPT['interpretation']}",
        "",
        "## ReDMCSB source audit",
    ]
    for row in source:
        lines.append(f"- `{row['file']}:{row['lines']}` `{row['id']}` ok=`{row['ok']}` - {row['claim']}")
    lines += ["", "## Same-viewport promotion predicate", "", next_unblocker, "", "## Non-claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    lines += ["", "## Gate", "", f"- `python3 tools/verify_{PASS}.py`", "", f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`"]
    if problems:
        lines += ["", "## Problems"] + [f"- {problem}" for problem in problems]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(manifest["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    if problems:
        for problem in problems:
            print(problem)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())