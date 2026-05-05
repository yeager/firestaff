#!/usr/bin/env python3
"""Pass224 DM1 V1 runtime/state probe scaffold.

JSON-only follow-up to pass223. It converts the source-locked post-redraw
instrumentation points into a concrete runtime trace contract, then audits the
current N2 original-runner APIs for a way to observe those states. If the hook
is absent, it emits an exact blocker instead of another screenshot attempt.
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DEFAULT_OUT = ROOT / "parity-evidence/verification/pass224_dm1_v1_runtime_state_probe_scaffold.json"
DEFAULT_REPORT = ROOT / "parity-evidence/pass224_dm1_v1_runtime_state_probe_scaffold.md"
CAPTURE_SCRIPT = ROOT / "scripts/dosbox_dm1_original_viewport_reference_capture.sh"
PASS118_DRIVER = ROOT / "tools/pass118_state_aware_original_route_driver.py"
PASS223_JSON = ROOT / "parity-evidence/verification/pass223_dm1_v1_post_redraw_instrumentation_lock.json"
PASS223_TOOL = ROOT / "tools/pass223_dm1_v1_post_redraw_instrumentation_lock.py"
ATTEMPT = ROOT / "verification-screens/pass212-n2-state-aware-movement-probe"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "command_accepted_queue_dispatch", "file": "COMMAND.C", "line_range": [2045, 2156], "function": "F0380_COMMAND_ProcessQueue_CPSC", "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "runtime_event": {"event": "command_accepted", "required_fields": ["seq", "tick_or_vblank", "command_id", "queue_first_index"], "state_edge": "input delivered -> command accepted"}},
    {"id": "turn_handler_applies_direction_and_releases_wait", "file": "CLIKMENU.C", "line_range": [142, 173], "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0284_CHAMPION_SetPartyDirection", "F0276_SENSOR_ProcessThingAdditionOrRemoval"], "runtime_event": {"event": "turn_state_applied", "required_fields": ["seq", "accepted_seq", "party_direction", "stop_waiting_for_input"], "state_edge": "command accepted -> movement applied"}},
    {"id": "step_handler_resolves_destination_and_releases_wait", "file": "CLIKMENU.C", "line_range": [180, 328], "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"], "runtime_event": {"event": "step_state_applied", "required_fields": ["seq", "accepted_seq", "from_x", "from_y", "to_x", "to_y", "disabled_movement_ticks", "stop_waiting_for_input"], "state_edge": "command accepted -> movement applied"}},
    {"id": "move_result_mutates_party_coordinates", "file": "MOVESENS.C", "line_range": [442, 443], "function": "F0267_MOVE_GetMoveResult_CPSCE", "needles": ["G0306_i_PartyMapX = P0560_i_DestinationMapX;", "G0307_i_PartyMapY = P0561_i_DestinationMapY;"], "runtime_event": {"event": "party_coordinates_mutated", "required_fields": ["seq", "accepted_seq", "party_map_x", "party_map_y"], "state_edge": "movement applied -> party coordinate state committed"}},
    {"id": "game_loop_draws_mutated_party_state", "file": "GAMELOOP.C", "line_range": [88, 91], "function": "F0002_MAIN_GameLoop_CPSDF", "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"], "runtime_event": {"event": "game_loop_draw_tuple", "required_fields": ["seq", "accepted_seq", "party_direction", "party_map_x", "party_map_y"], "state_edge": "movement applied -> redraw requested from mutated state"}},
    {"id": "dungeon_view_consumes_state_before_viewport_request", "file": "DUNVIEW.C", "line_range": [8318, 8610], "function": "F0128_DUNGEONVIEW_Draw_CPSF", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "runtime_event": {"event": "dungeon_view_consumed_tuple", "required_fields": ["seq", "accepted_seq", "direction_arg", "map_x_arg", "map_y_arg"], "state_edge": "redraw requested from mutated state -> viewport draw requested"}},
    {"id": "viewport_requested_then_vertical_blank_returned", "file": "DRAWVIEW.C", "line_range": [709, 722], "function": "F0097_DUNGEONVIEW_DrawViewport", "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE;", "M526_WaitVerticalBlank();"], "runtime_event": {"event": "viewport_request_vblank_return", "required_fields": ["seq", "accepted_seq", "draw_viewport_requested", "vblank_counter_before", "vblank_counter_after"], "state_edge": "viewport draw requested -> vblank returned"}},
    {"id": "viewport_bitmap_blitted_to_screen", "file": "DRAWVIEW.C", "line_range": [840, 842], "function": "E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF", "needles": ["F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT, CM1_COLOR_NO_TRANSPARENCY);"], "runtime_event": {"event": "viewport_present", "required_fields": ["seq", "accepted_seq", "zone", "viewport_bitmap_address_or_digest", "vblank_counter"], "state_edge": "vblank returned -> viewport present"}},
]

PROBE_TRACE_SCHEMA = {
    "schema": "pass224_dm1_v1_runtime_state_trace.v1",
    "capture_policy": {"json_only": True, "forbidden_extensions": [".png", ".ppm"], "no_screenshot_claim": True},
    "promotion_predicate": [
        "one command_accepted event exists for the route label",
        "a later turn_state_applied or step_state_applied event references the accepted command sequence",
        "for step commands, a later party_coordinates_mutated event records the committed destination",
        "a later game_loop_draw_tuple/dungeon_view_consumed_tuple uses the mutated direction/x/y tuple",
        "a later viewport_request_vblank_return records G0324_B_DrawViewportRequested before and after M526_WaitVerticalBlank",
        "a later viewport_present event records the C007_ZONE_VIEWPORT blit",
    ],
}

MISSING_HOOK_BLOCKER = {
    "id": "pass224_missing_original_runtime_state_hook_api",
    "exact_missing_hook_api": "A debugger/emulator bridge that binds ReDMCSB source seams to the loaded stock DM.EXE and can emit JSON events for G0432/G0433 command dequeue, G0308/G0306/G0307/G0310/G0321 state mutations, F0128 draw arguments, G0324 viewport-request/vblank, and C007_ZONE_VIEWPORT blit without using PNG/PPM screenshots.",
    "why_required": "pass223 identified the source seams, but the current route drivers only send input and trigger screenshots. Duplicate dungeon_gameplay SHA values prove visual capture alone cannot establish command accepted -> movement applied -> viewport present.",
    "acceptable_implementations": ["DOSBox-X/debugger or dosbox-debug breakpoint script with a symbol/address map from ReDMCSB functions/globals to the loaded DM.EXE image", "a custom DOSBox/Staging frame/runtime hook that logs the listed globals and call-site hits as JSON", "a real-mode debugger/gdbstub bridge that can stop/log at the listed PC addresses and continue without screenshot artifacts"],
}

def compact(text: str) -> str:
    return " ".join(text.split())

def source_slice(file: str, start: int, end: int) -> str:
    path = REDMCSB / file
    if not path.is_file():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])

def verify_lock(lock: dict[str, Any]) -> dict[str, Any]:
    start, end = lock["line_range"]
    text = compact(source_slice(lock["file"], start, end))
    missing = [needle for needle in lock["needles"] if compact(needle) not in text]
    return {"id": lock["id"], "file": lock["file"], "function": lock["function"], "citation": f"{lock['file']}:{start}-{end}", "verified": not missing, "missing": missing, "runtime_event": lock["runtime_event"]}

def cmd_output(argv: list[str]) -> dict[str, Any]:
    try:
        proc = subprocess.run(argv, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=8)
        return {"argv": argv, "returncode": proc.returncode, "output_head": proc.stdout.splitlines()[:12]}
    except Exception as exc:
        return {"argv": argv, "error": repr(exc)}

def file_tokens(path: Path, tokens: list[str]) -> dict[str, Any]:
    if not path.is_file():
        return {"path": str(path), "exists": False, "tokens": {token: False for token in tokens}}
    text = path.read_text(encoding="utf-8", errors="replace")
    return {"path": str(path.relative_to(ROOT)), "exists": True, "tokens": {token: token in text for token in tokens}}

def audit_runtime_api() -> dict[str, Any]:
    bins = {name: shutil.which(name) for name in ["dosbox", "dosbox-debug", "dosbox-x", "gdb", "xvfb-run", "xdotool"]}
    files = [
        file_tokens(CAPTURE_SCRIPT, ["shot:<label>", "ctrl+F5", "DM1_ORIGINAL_ROUTE_EVENTS", "breakpoint", "G0306", "G0324", "C007_ZONE_VIEWPORT"]),
        file_tokens(PASS118_DRIVER, ["xdotool", "capture_new", "pass118_driver.log", "G0306", "breakpoint"]),
        file_tokens(PASS223_TOOL, ["SOURCE_LOCKS", "F0380_COMMAND_ProcessQueue_CPSC", "F0097_DUNGEONVIEW_DrawViewport"]),
    ]
    hook_capable = any(f.get("tokens", {}).get("breakpoint") and f.get("tokens", {}).get("G0306") and f.get("tokens", {}).get("G0324") for f in files)
    return {"host_tools": bins, "dosbox_version": cmd_output([bins["dosbox"] or "dosbox", "-version"]) if bins.get("dosbox") else {"missing": "dosbox"}, "dosbox_x_version": cmd_output([bins["dosbox-x"] or "dosbox-x", "-version"]) if bins.get("dosbox-x") else {"missing": "dosbox-x"}, "gdb_version": cmd_output([bins["gdb"] or "gdb", "--version"]) if bins.get("gdb") else {"missing": "gdb"}, "existing_runner_api_files": files, "hook_capable": hook_capable, "classification": "ready/runtime-hook-api-found-review-manually" if hook_capable else "blocked/missing-original-runtime-state-hook-api"}

def audit_existing_attempt() -> dict[str, Any]:
    classifier = ATTEMPT / "pass80_movement_six_class_gate.json"
    labels = ATTEMPT / "original_viewport_shot_labels.tsv"
    out: dict[str, Any] = {"attempt_dir": str(ATTEMPT.relative_to(ROOT)), "classifier_exists": classifier.is_file(), "labels_exists": labels.is_file()}
    if classifier.is_file():
        doc = json.loads(classifier.read_text(encoding="utf-8"))
        dups = doc.get("duplicate_sha256_counts") or {}
        out.update({"capture_count": doc.get("capture_count"), "class_counts": doc.get("class_counts"), "duplicate_sha256_counts": dups, "first_duplicate_sha12": next(iter(dups or {"": 0}))[:12]})
    return out

def write_report(manifest: dict[str, Any], report: Path) -> None:
    lines = ["# Pass224 — DM1 V1 runtime/state probe scaffold", "", f"Status: `{manifest['status']}`", "", "Scope: JSON-only runtime/state probe contract for the pass223 post-redraw seams. No screenshots, PNGs, or PPMs are produced or committed.", "", "## ReDMCSB audit citations", ""]
    for item in manifest["source_locks"]:
        mark = "PASS" if item["verified"] else "FAIL"
        ev = item["runtime_event"]
        lines.append(f"- {mark} `{item['id']}` — `{item['citation']}` / `{item['function']}` -> event `{ev['event']}`")
    lines += ["", "## Runtime trace chain", ""]
    for rule in manifest["runtime_trace_schema"]["promotion_predicate"]:
        lines.append(f"- {rule}")
    api = manifest["runtime_api_audit"]
    lines += ["", "## Runtime API audit", "", f"- classification: `{api['classification']}`", f"- hook capable in committed runner files: `{api['hook_capable']}`", f"- tools: `{api['host_tools']}`", "", "## Exact blocker", "", f"`{manifest['blocker']['id']}`", "", manifest["blocker"]["exact_missing_hook_api"], "", "Acceptable implementations:"]
    for item in manifest["blocker"]["acceptable_implementations"]:
        lines.append(f"- {item}")
    attempt = manifest["existing_attempt_audit"]
    lines += ["", "## Existing attempt signal", "", f"- attempt: `{attempt['attempt_dir']}`", f"- class counts: `{attempt.get('class_counts')}`", f"- duplicate SHA counts: `{attempt.get('duplicate_sha256_counts')}`", f"- first duplicate sha12: `{attempt.get('first_duplicate_sha12')}`", "", "Non-claims: no source patching, no DOSBox screenshots, no PNG/PPM output, no pixel parity claim.", ""]
    report.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()
    locks = [verify_lock(lock) for lock in SOURCE_LOCKS]
    api = audit_runtime_api()
    source_ok = all(lock["verified"] for lock in locks)
    status = "BLOCKED_MISSING_ORIGINAL_RUNTIME_STATE_HOOK_API" if source_ok and not api["hook_capable"] else ("PASS_RUNTIME_STATE_PROBE_SCAFFOLD_READY" if source_ok else "FAIL_REDMCSB_SOURCE_AUDIT")
    manifest = {"schema": "pass224_dm1_v1_runtime_state_probe_scaffold.v1", "status": status, "repo": str(ROOT), "redmcsb_source_root": str(REDMCSB), "pass223_input": str(PASS223_JSON.relative_to(ROOT)), "artifact_policy": {"json_only": True, "forbidden_extensions": [".png", ".ppm"]}, "source_locks": locks, "runtime_trace_schema": PROBE_TRACE_SCHEMA, "runtime_api_audit": api, "existing_attempt_audit": audit_existing_attempt(), "blocker": MISSING_HOOK_BLOCKER}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({"status": status, "out": str(args.out), "report": str(args.report), "source_locks": len(locks)}, indent=2, sort_keys=True))
    return 0 if source_ok else 1

if __name__ == "__main__":
    raise SystemExit(main())
