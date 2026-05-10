#!/usr/bin/env python3
"""Pass498: narrow the DM1 V1 original capture blocker to a post-command state delta."""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass498_dm1_v1_original_post_command_state_delta_boundary.md"
PASS487 = ROOT / "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json"
PASS495 = ROOT / "parity-evidence/verification/pass495_dm1_v1_f0365_f0366_runtime_static_boundary/manifest.json"
PASS497 = ROOT / "parity-evidence/verification/pass497_dm1_v1_original_capture_next_blocker/manifest.json"
STATUS = "PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA"

SOURCE_LOCKS = [
    {"id": "game_loop_redraw_before_wait_then_command_wait_wrap", "file": "GAMELOOP.C", "lines": "90,164,215-219", "function": "F0002_MAIN_GameLoop_CPSDF", "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "A promotable post-command frame must be after the wait loop exits and the next outer-loop F0128 consumes the updated party tuple."},
    {"id": "f0380_pop_load_dispatches_turn_or_step", "file": "COMMAND.C", "lines": "2045-2156", "function": "F0380_COMMAND_ProcessQueue_CPSC", "needles": ["L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "G2153_i_QueuedCommandsCount--;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "claim": "The source-visible state delta must be tied to a real dequeued command, not just host click/key labels."},
    {"id": "f0365_turn_mutates_direction_and_stops_wait", "file": "CLIKMENU.C", "lines": "142-174", "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0284_CHAMPION_SetPartyDirection"], "claim": "Accepted turn commands must prove the F0365 stop-wait write and party-direction mutation before the redraw boundary."},
    {"id": "f0366_step_computes_destination_and_stops_wait", "file": "CLIKMENU.C", "lines": "180-347", "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"], "claim": "Accepted move commands must prove the F0366 destination/move-result path, not a repeated static gameplay capture."},
    {"id": "f0128_composes_viewport_from_party_tuple", "file": "DUNVIEW.C", "lines": "8318-8610", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "The post-command state delta becomes capture-eligible only once F0128 composes G0296 for the updated direction/X/Y tuple."},
    {"id": "f0097_present_boundary", "file": "DRAWVIEW.C", "lines": "709-858", "function": "F0097_DUNGEONVIEW_DrawViewport", "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE;", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"], "claim": "A screenshot/viewport crop is promotable only at or after the F0097/VIDRV present boundary after the matching F0128."},
]


def read_json(path: Path) -> dict:
    if not path.exists():
        raise SystemExit(f"missing required manifest: {path}")
    return json.loads(path.read_text(encoding="utf-8"))


def source_row(lock: dict) -> dict:
    path = RED / lock["file"]
    text = path.read_text(encoding="latin-1", errors="replace") if path.exists() else ""
    missing = [needle for needle in lock["needles"] if needle not in text]
    return {**lock, "path": str(path), "ok": path.exists() and not missing, "missing": missing}


def main() -> int:
    p487 = read_json(PASS487)
    p495 = read_json(PASS495)
    p497 = read_json(PASS497)
    blockers = p487.get("blockerFindings", {})
    source = [source_row(lock) for lock in SOURCE_LOCKS]
    observed = {
        "pass487Status": p487.get("status"),
        "pass495Status": p495.get("status"),
        "pass497Status": p497.get("status"),
        "classes": p487.get("classes"),
        "postEntryGameplaySha256": blockers.get("postEntryGameplaySha256"),
        "postEntryGameplayHashRepeated": blockers.get("postEntryGameplayHashRepeated") is True,
        "postEntryRegionStatsRepeated": blockers.get("postEntryRegionStatsRepeated") is True,
        "firstFrameStillEntranceMenu": blockers.get("firstFrameStillEntranceMenu") is True,
        "filenameLabelDriftRows": len(blockers.get("filenameLabelDrift", [])),
        "f0365F0366RuntimeStopsPreviouslyProven": p495.get("runtimeStopEvidence", {}).get("ok") is True,
        "f0365F0366StaticCaptureStillBlocked": p495.get("staticDuplicateEvidence", {}).get("postEntryStaticNoStateDelta") is True,
        "pass497NextBlockerMentionsStateDelta": "source-visible post-command state delta" in p497.get("nextBlocker", ""),
    }
    predicate = {
        "requiredForNextPromotableOriginalCapture": [
            "post-gameplay command observed through F0380 pop/load/decrement",
            "matching F0365 turn or F0366 move handler observed after that command",
            "G0321 stop-wait transition and game-time tick allow the wait loop to exit",
            "later F0128 consumes the resulting direction/map-X/map-Y tuple",
            "F0097/VIDRV present boundary is reached for the same shot",
            "raw viewport/fullframe hash or region fingerprint differs from the repeated 48ed static gameplay frame, unless the command is explicitly proven blocked/no-op by source state",
        ],
        "rejectAsNonPromotable": [
            "host click/key route labels without F0380/F0365/F0366 evidence",
            "BPLIST or setup echoes without strict post-Running stops",
            "repeated 48ed static gameplay captures after entry",
            "filenames whose labels drift from the route action",
            "F0128/F0097 address bindings without a command-state predecessor",
        ],
    }
    problems = []
    problems.extend(f"source lock failed: {row['id']} missing {row['missing']}" for row in source if not row["ok"])
    required_observed = {
        "pass487_ok": p487.get("ok") is True,
        "pass495_ok": p495.get("ok") is True,
        "pass497_ok": p497.get("ok") is True,
        "static_no_state_delta_currently_present": observed["postEntryGameplayHashRepeated"] and observed["postEntryRegionStatsRepeated"],
        "f0365_f0366_runtime_stop_chain_already_closed": observed["f0365F0366RuntimeStopsPreviouslyProven"],
        "next_blocker_state_delta_text_present": observed["pass497NextBlockerMentionsStateDelta"],
    }
    problems.extend(name for name, ok in required_observed.items() if not ok)
    manifest = {
        "schema": "firestaff.parity.pass498_dm1_v1_original_post_command_state_delta_boundary.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if not problems else "FAIL_PASS498_ORIGINAL_POST_COMMAND_STATE_DELTA_BOUNDARY",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "inputs": {"pass487": str(PASS487.relative_to(ROOT)), "pass495": str(PASS495.relative_to(ROOT)), "pass497": str(PASS497.relative_to(ROOT))},
        "observedBlockerState": observed,
        "narrowedBlocker": "Original DM1 V1 capture is blocked specifically at the post-gameplay, post-command state-delta boundary: the next evidence must connect a real F0380-dequeued movement/turn command through F0365/F0366 to the subsequent F0128 composition and F0097/VIDRV present, and must not collapse to the repeated static 48ed gameplay frame.",
        "promotionPredicate": predicate,
        "nonClaims": ["no new original runtime debugger stop was captured by this verifier", "no original-vs-Firestaff pixel parity promotion", "no promotion of pass487/pass492 static frames", "no claim that source movement handlers are defective"],
        "requiredObserved": required_observed,
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass498 — DM1 V1 original post-command state-delta boundary", "", f"Status: `{manifest['status']}`", "", "## Decision", "", manifest["narrowedBlocker"], "", "## Source path that the next capture must prove", "",
    ]
    for row in source:
        lines.append(f"- `{row['file']}:{row['lines']}` / `{row['function']}` — {row['claim']} ok=`{row['ok']}`")
    lines += [
        "", "## Current blocker evidence", "", f"- pass487 status: `{observed['pass487Status']}`", f"- pass495 status: `{observed['pass495Status']}`", f"- pass497 status: `{observed['pass497Status']}`", f"- post-entry gameplay hash repeated: `{observed['postEntryGameplayHashRepeated']}` / `{observed['postEntryGameplaySha256']}`", f"- post-entry region stats repeated: `{observed['postEntryRegionStatsRepeated']}`", f"- filename/route-label drift rows: `{observed['filenameLabelDriftRows']}`", f"- F0365/F0366 runtime stop chain previously closed: `{observed['f0365F0366RuntimeStopsPreviouslyProven']}`", "", "## Promotion predicate for the next run", "",
    ]
    lines.extend(f"- {item}" for item in predicate["requiredForNextPromotableOriginalCapture"])
    lines += ["", "## Reject as non-promotable", ""]
    lines.extend(f"- {item}" for item in predicate["rejectAsNonPromotable"])
    lines += ["", "## Gate", "", "- `python3 tools/verify_pass498_dm1_v1_original_post_command_state_delta_boundary.py`", "", "Manifest: `parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json`"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    if problems:
        print(manifest["status"])
        for problem in problems:
            print(problem)
        return 1
    print(STATUS)
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
