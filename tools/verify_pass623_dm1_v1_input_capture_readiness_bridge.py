#!/usr/bin/env python3
"""Pass623: bind DM1 V1 input script rows to capture-readiness blockers."""
from __future__ import annotations

import json
import re
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass623_dm1_v1_input_capture_readiness_bridge"
STATUS = "PASS623_DM1_V1_INPUT_CAPTURE_READINESS_BRIDGE_LOCKED"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"
PASS610_REPORT = ROOT / "parity-evidence/pass610_dm1_v1_firestaff_viewport_crop_capture_gate.md"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id":"pc34_movement_mouse_boxes_map_to_c001_c006","file":"COMMAND.C","lines":"106-121","needles":["C001_COMMAND_TURN_LEFT","C003_COMMAND_MOVE_FORWARD","C002_COMMAND_TURN_RIGHT","C006_COMMAND_MOVE_LEFT","C005_COMMAND_MOVE_BACKWARD","C004_COMMAND_MOVE_RIGHT"],"claim":"movement-arrow input resolves to the same C001..C006 command ids used by the Firestaff script rows"},
    {"id":"f0359_queues_mouse_command_with_coordinates","file":"COMMAND.C","lines":"1452-1661","needles":["void F0359_COMMAND_ProcessClick_CPSC","F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command","G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X","G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y"],"claim":"accepted mouse input enters G0432 with command and coordinates before dispatch"},
    {"id":"f0361_queues_keyboard_command","file":"COMMAND.C","lines":"1709-1813","needles":["void F0361_COMMAND_ProcessKeyPress","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command","G2153_i_QueuedCommandsCount++","F0360_COMMAND_ProcessPendingClick();"],"claim":"keyboard input has the same queue/count boundary that a promotable original transcript must show"},
    {"id":"f0380_pops_and_dispatches_turn_or_step","file":"COMMAND.C","lines":"2045-2156","needles":["void F0380_COMMAND_ProcessQueue_CPSC","L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","G2153_i_QueuedCommandsCount--;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],"claim":"the queue pop must be paired to F0365/F0366 before any post-command viewport row is promotable"},
    {"id":"game_loop_draws_tuple_then_waits_for_command_dispatch","file":"GAMELOOP.C","lines":"90-219","needles":["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);","G0321_B_StopWaitingForPlayerInput = C0_FALSE;","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"],"claim":"an original transcript must tie the command pop to the later F0128 tuple draw in the same bounded route"},
    {"id":"f0128_consumes_tuple_and_calls_present","file":"DUNVIEW.C","lines":"8318-8610","needles":["void F0128_DUNGEONVIEW_Draw_CPSF","P0183_i_Direction","P0184_i_MapX","P0185_i_MapY","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],"claim":"the viewport crop row must bind to the exact direction/X/Y consumed by F0128"},
    {"id":"f0097_presents_the_composed_viewport","file":"DRAWVIEW.C","lines":"709-858","needles":["void F0097_DUNGEONVIEW_DrawViewport","G0324_B_DrawViewportRequested = C1_TRUE;","M526_WaitVerticalBlank();","F0638_GetZone(C007_ZONE_VIEWPORT, L2413_ai_Box);","(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"],"claim":"promotable original crops must be sampled at or after the viewport-present boundary"},
]

FIRESTAFF_LOCKS: list[dict[str, Any]] = [
    {"id":"m11_input_maps_to_dm1_v1_commands","file":"src/engine/m11_game_view.c","lines":"6162-6225","needles":["case M12_MENU_INPUT_LEFT:","return DM1_V1_COMMAND_TURN_LEFT;","case M12_MENU_INPUT_RIGHT:","return DM1_V1_COMMAND_TURN_RIGHT;","case M12_MENU_INPUT_UP:","return DM1_V1_COMMAND_MOVE_FORWARD;","DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(","DM1_V1_MovementPipeline_ProcessOneTickPc34Compat("],"claim":"Firestaff route tokens enter the DM1 V1 queue as source command ids and process one compat tick"},
    {"id":"m11_records_movement_pipeline_capture_state","file":"src/engine/m11_game_view.c","lines":"6229-6297","needles":["state->world.gameTick++;","state->lastDm1V1MovementPipelineResult.core.turnApplied","state->lastDm1V1MovementPipelineResult.anyMovementOccurred","state->lastDm1V1MovementPipelineResult.core.movementBlocked","return state->lastDm1V1MovementPipelineResult.viewportDirty ||","state->lastDm1V1MovementPipelineResult.core.queue.dequeued;"],"claim":"capture rows can distinguish turn, step, blocked no-op, dirty viewport, and dequeued command"},
    {"id":"wall_collision_probe_emits_input_script_and_viewport_crops","file":"probes/m11/firestaff_m11_wall_collision_capture_probe.c","lines":"20-174,210-236","needles":["VIEWPORT_X = 0","VIEWPORT_Y = 33","VIEWPORT_W = 224","VIEWPORT_H = 136","out->command = game->lastDm1V1MovementPipelineResult.core.queue.command;","out->movementBlocked = game->lastDm1V1MovementPipelineResult.core.movementBlocked;","out->viewportDirty = game->lastDm1V1MovementPipelineResult.viewportDirty;","result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);","M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);","M11_GameView_HandleInput(&game, M12_MENU_INPUT_LEFT);","03_blocked_west_wall_1_3","04_forward_south_1_4"],"claim":"the Firestaff-side probe is an input-script capture manifest, not only a screenshot dumper"},
]

GATES = [
    ("movement_queue_capture_closure", ROOT / "parity-evidence/verification/dm1_v1_movement_queue_capture_closure/manifest.json", "PASS_DM1_V1_MOVEMENT_QUEUE_CAPTURE_CLOSURE_LOCKED"),
    ("pass564_original_transcript_gate", ROOT / "parity-evidence/verification/pass564_dm1_v1_original_movement_viewport_transcript_gate/manifest.json", "BLOCKED_PASS564_RUNTIME_TRANSCRIPT_CHAIN_MISSING"),
    ("pass608_same_viewport_blocker", ROOT / "parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json", "BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE"),
    ("pass622_viewport_wall_capture_closure_gap", ROOT / "parity-evidence/verification/pass622_dm1_v1_viewport_wall_capture_closure_gap/manifest.json", "BLOCKED_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP_LOCKED"),
]

CANONICAL_ROWS = [
    {"label":"01_start_south_1_3","inputTokens":[],"commandIds":[],"postTuple":{"map":0,"x":1,"y":3,"direction":2},"claim":"baseline viewport crop before scripted movement input"},
    {"label":"02_turn_right_west_1_3","inputTokens":["M12_MENU_INPUT_RIGHT"],"commandIds":[2],"postTuple":{"map":0,"x":1,"y":3,"direction":3},"claim":"right input resolves to C002 turn-right and changes facing only"},
    {"label":"03_blocked_west_wall_1_3","inputTokens":["M12_MENU_INPUT_UP"],"commandIds":[3],"postTuple":{"map":0,"x":1,"y":3,"direction":3},"sameViewportHashAs":"02_turn_right_west_1_3","claim":"forward command into the west wall is consumed but leaves the same tuple/crop"},
    {"label":"04_forward_south_1_4","inputTokens":["M12_MENU_INPUT_LEFT","M12_MENU_INPUT_UP"],"commandIds":[1,3],"postTuple":{"map":0,"x":1,"y":4,"direction":2},"claim":"turn-left then forward gives a distinct moved viewport crop"},
]

REQUIRED_ORIGINAL_TRANSCRIPT_COLUMNS = ["routeLabel","inputToken","sourceCommandId","G0432WriteIndex","G2153BeforeAfter","F0380DequeuedCommand","F0365OrF0366Dispatch","postCommandMapXMapYDirection","F0128DrawTuple","F0097PresentFrameId","viewportCropSha256"]


def read(path: Path, encoding: str = "utf-8") -> str:
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def window(path: Path, spec: str, encoding: str) -> str:
    lines = read(path, encoding).splitlines()
    out: list[str] = []
    for part in spec.split(","):
        first_s, last_s = part.split("-", 1) if "-" in part else (part, part)
        first, last = int(first_s), int(last_s)
        out.extend(lines[first - 1:last])
    return "\n".join(out)


def audit_rows(root: Path, locks: list[dict[str, Any]], encoding: str) -> list[dict[str, Any]]:
    rows = []
    for lock in locks:
        path = root / lock["file"]
        if not path.exists():
            rows.append({**lock, "ok": False, "missing": [f"missing file: {path}"]})
            continue
        body = compact(window(path, lock["lines"], encoding))
        missing = [needle for needle in lock["needles"] if compact(needle) not in body]
        rows.append({"id": lock["id"], "file": lock["file"], "lines": lock["lines"], "claim": lock["claim"], "ok": not missing, "missing": missing})
    return rows


def load_status(path: Path) -> tuple[str | None, dict[str, Any] | None]:
    if not path.exists():
        return None, None
    data = json.loads(read(path))
    return data.get("status"), data


def audit_gate_statuses() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for gate_id, path, expected in GATES:
        status, data = load_status(path)
        row: dict[str, Any] = {"id": gate_id, "path": str(path.relative_to(ROOT)), "expected": expected, "observed": status, "ok": status == expected}
        if gate_id == "pass608_same_viewport_blocker" and data:
            row["runtimeTranscriptProvided"] = data.get("runtimeTranscript", {}).get("provided")
            row["runtimeTranscriptOk"] = data.get("runtimeTranscript", {}).get("ok")
            row["firestaffStateCount"] = len(data.get("firestaffEvidence", {}).get("states", []))
            row["firestaffViewportHashCount"] = len(data.get("firestaffEvidence", {}).get("viewportHashes", []))
        if gate_id == "pass622_viewport_wall_capture_closure_gap" and data:
            row["blocker"] = data.get("blocker")
            row["manifestOk"] = data.get("ok")
            row["ok"] = row["ok"] and data.get("ok") is True
        rows.append(row)
    return rows


def parse_pass610_rows() -> list[dict[str, Any]]:
    text = read(PASS610_REPORT)
    if "Status: PASS610_DM1_V1_FIRESTAFF_VIEWPORT_CROP_CAPTURE_LOCKED" not in text:
        raise AssertionError("pass610 report status is not locked")
    pattern = re.compile(r"^- (?P<label>\S+) map=(?P<map>\d+) x=(?P<x>\d+) y=(?P<y>\d+) dir=(?P<direction>\d+) crop=(?P<crop>\S+) sha256=(?P<sha>[0-9a-f]{64})$", re.MULTILINE)
    rows = []
    for match in pattern.finditer(text):
        row = match.groupdict()
        for key in ["map", "x", "y", "direction"]:
            row[key] = int(row[key])
        rows.append(row)
    return rows


def audit_canonical_rows() -> list[dict[str, Any]]:
    observed_rows = {row["label"]: row for row in parse_pass610_rows()}
    rows: list[dict[str, Any]] = []
    for expected in CANONICAL_ROWS:
        label = expected["label"]
        observed = observed_rows.get(label)
        problems: list[str] = []
        if observed is None:
            problems.append("missing pass610 runtime crop row")
        else:
            tup = expected["postTuple"]
            for key, obs_key in [("map", "map"), ("x", "x"), ("y", "y"), ("direction", "direction")]:
                if observed[obs_key] != tup[key]:
                    problems.append(f"{key} expected {tup[key]} observed {observed[obs_key]}")
            same_as = expected.get("sameViewportHashAs")
            if same_as:
                other = observed_rows.get(same_as)
                if not other or other.get("sha") != observed.get("sha"):
                    problems.append(f"expected same viewport hash as {same_as}")
        rows.append({**expected, "observed": observed, "ok": not problems, "problems": problems})
    return rows


def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass623 - DM1 V1 input capture readiness bridge", "", f"Status: {manifest['status']}", "", "This gate binds the Firestaff input script to movement queue dispatch and viewport crop rows, while keeping the original-side blocker explicit.", "", "## ReDMCSB source audit"]
    for row in manifest["redmcsbSourceAudit"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{row['lines']} {row['id']} - {row['claim']}")
    lines += ["", "## Firestaff route audit"]
    for row in manifest["firestaffRouteAudit"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{row['lines']} {row['id']} - {row['claim']}")
    lines += ["", "## Canonical input/crop rows"]
    for row in manifest["canonicalInputCaptureRows"]:
        observed = row.get("observed") or {}
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['label']} inputs={row['inputTokens']} commands={row['commandIds']} tuple={row['postTuple']} crop={observed.get('crop')} sha256={observed.get('sha')}")
    lines += ["", "## Required original transcript columns"]
    lines.extend(f"- {item}" for item in manifest["requiredOriginalTranscriptColumns"])
    lines += ["", "## Consumed gates"]
    for row in manifest["gateStatusChecks"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['id']} observed={row['observed']}")
    lines += ["", "## Decision", "", manifest["decision"], "", "## Non-claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if manifest["problems"]:
        lines += ["", "## Problems"]
        lines.extend(f"- {item}" for item in manifest["problems"])
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    red = audit_rows(RED, SOURCE_LOCKS, "latin-1")
    firestaff = audit_rows(ROOT, FIRESTAFF_LOCKS, "utf-8")
    gates = audit_gate_statuses()
    canonical = audit_canonical_rows()
    problems: list[str] = []
    problems.extend(f"redmcsb source audit failed: {row['id']}" for row in red if not row["ok"])
    problems.extend(f"firestaff route audit failed: {row['id']}" for row in firestaff if not row["ok"])
    problems.extend(f"gate status drifted: {row['id']}" for row in gates if not row["ok"])
    problems.extend(f"canonical row failed: {row['label']}" for row in canonical if not row["ok"])
    status = STATUS if not problems else "FAIL_PASS623_DM1_V1_INPUT_CAPTURE_READINESS_BRIDGE"
    manifest = {"schema": f"firestaff.parity.{PASS}.v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": status, "ok": not problems, "sourceRoot": str(RED), "redmcsbSourceAudit": red, "firestaffRouteAudit": firestaff, "gateStatusChecks": gates, "canonicalInputCaptureRows": canonical, "requiredOriginalTranscriptColumns": REQUIRED_ORIGINAL_TRANSCRIPT_COLUMNS, "decision": "Firestaff now has an audited bridge from M12 input tokens to DM1 V1 command ids, movement/blocked state, and 224x136 viewport crop labels. The remaining blocker is specifically original-side: capture one transcript row with the required queue/dispatch/F0128/F0097 fields for one of these route labels before promoting original-vs-Firestaff parity.", "nonClaims": ["no original runtime capture was produced", "no original-vs-Firestaff pixel parity is promoted", "no movement, renderer, or input behavior is changed", "no non-N2 source path is used", "no push or release action"], "problems": problems}
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "manifest": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if not problems else 1

if __name__ == "__main__":
    raise SystemExit(main())
