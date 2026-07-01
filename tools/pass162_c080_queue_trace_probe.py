#!/usr/bin/env python3
"""Pass162 C080 queue trace probe planner for original DM PC lane.

This is intentionally source-first and non-invasive: it audits the ReDMCSB
command path, verifies the N2-only original-game inputs are present, and emits a
small breakpoint/probe plan that narrows the next original-runtime question to:
mouse translation before the original queue, C080 enqueue/dequeue/dispatch, or
front-wall hit-state before F0280.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
ORIGINAL_DM = Path("~/.openclaw/data/firestaff-original-games/DM").expanduser()
EXTRACTED_DM = ORIGINAL_DM / "_extracted"
PASS162_SUMMARY = REPO / "parity-evidence/verification/pass162_original_party_route_unblock/source_gated_portrait_then_resurrect/summary.json"
PASS273_MANIFEST = REPO / "parity-evidence/verification/pass273_dm1_v1_fires_public_symbol_unblock/manifest.json"
OUT_DIR = REPO / "parity-evidence/verification/pass162_c080_queue_trace"
BP_ACCEPTANCE_MANIFEST = OUT_DIR / "dosbox_debug_bp_acceptance/manifest.json"
BREAK_START_ACCEPTANCE_MANIFEST = OUT_DIR / "dosbox_x_break_start_bp_acceptance/manifest.json"
LIVE_HOC_MANIFEST = OUT_DIR / "live_hoc_break_start_probe/manifest.json"
LIVE_MOVEMENT_CONTROL_MANIFEST = OUT_DIR / "live_movement_click_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_DOUBLE_CONTROL_MANIFEST = OUT_DIR / "live_movement_double_click_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_AUTOLOCK_CONTROL_MANIFEST = OUT_DIR / "live_movement_autolock_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_KEYBOARD_CAPTURE_CONTROL_MANIFEST = OUT_DIR / "live_movement_keyboard_capture_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_CLICLICK_CONTROL_MANIFEST = OUT_DIR / "live_movement_cliclick_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_SYSTEMEVENTS_CONTROL_MANIFEST = OUT_DIR / "live_movement_systemevents_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_PID_CONTROL_MANIFEST = OUT_DIR / "live_movement_pid_control_break_start_probe/manifest.json"
LIVE_MOVEMENT_OPENGL_CONTROL_MANIFEST = OUT_DIR / "live_movement_opengl_control_break_start_probe/manifest.json"


@dataclass(frozen=True)
class Citation:
    file: str
    lines: str
    symbol: str
    point: str
    must_contain: tuple[str, ...]


CITATIONS: tuple[Citation, ...] = (
    Citation("DEFS.H", "305", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C080 is command ordinal 80.", ("#define C080_COMMAND_CLICK_IN_DUNGEON_VIEW",)),
    Citation("DEFS.H", "3752", "C007_ZONE_VIEWPORT", "C007 is the viewport zone used by the PC movement secondary mouse table.", ("#define C007_ZONE_VIEWPORT",)),
    Citation("COMMAND.C", "1-16", "G0432_as_CommandQueue", "Original command queue storage, first/last indices, queue lock, and pending-click fields.", ("G0432_as_CommandQueue", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex", "G0435_B_CommandQueueLocked", "G0436_B_PendingClickPresent")),
    Citation("COMMAND.C", "106-114", "C007 -> C080 mouse route", "PC secondary movement mouse input maps viewport left-click box 0..223,33..168 to C080.", ("G0448_as_Graphic561_SecondaryMouseInput_Movement", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "0, 223,  33, 168")),
    Citation("COMMAND.C", "397-403", "C007 -> C080 zone route", "Zone-based movement table maps C007_ZONE_VIEWPORT left-click to C080.", ("C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C007_ZONE_VIEWPORT")),
    Citation("COMMAND.C", "1452-1662", "F0359_COMMAND_ProcessClick_CPSC", "Actual mouse-click queue writer: derives command from primary/secondary mouse tables and writes nonzero command plus X/Y into G0432_as_CommandQueue.", ("void F0359_COMMAND_ProcessClick_CPSC", "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G0432_as_CommandQueue", ".Command = L1109_i_Command", ".X = P0725_i_X", ".Y = P0726_i_Y")),
    Citation("CLIKMENU.C", "142-174", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "Required audit symbol: in this ReDMCSB tree F0365 is turn-party handling, not the C080 mouse queue writer; it is dispatched only for C001/C002 in F0380.", ("void F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection")),
    Citation("COMMAND.C", "2045-2127", "F0380_COMMAND_ProcessQueue_CPSC dequeue", "F0380 locks/dequeues command, X, Y from G0432_as_CommandQueue and unlocks before dispatch.", ("void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue", "L1161_i_CommandX", "L1162_i_CommandY", "G0435_B_CommandQueueLocked = C0_FALSE")),
    Citation("COMMAND.C", "2150-2152", "F0380 -> F0365 dispatch", "F0380 dispatches only C001/C002 turn commands to F0365.", ("F0365_COMMAND_ProcessTypes1To2_TurnParty",)),
    Citation("COMMAND.C", "2322-2324", "F0380 -> F0377 dispatch", "F0380 dispatches C080 to F0377 with dequeued X/Y.", ("if (L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW)", "F0377_COMMAND_ProcessType80_ClickInDungeonView")),
    Citation("CLIKVIEW.C", "311-350", "F0377_COMMAND_ProcessType80_ClickInDungeonView", "C080 handler; PC builds normalize screen coordinates by subtracting viewport origin before hit testing.", ("void F0377_COMMAND_ProcessType80_ClickInDungeonView", "P0752_i_X -= G2067_i_ViewportScreenX", "P0753_i_Y -= G2068_i_ViewportScreenY")),
    Citation("CLIKVIEW.C", "406-439", "F0377 empty-hand front-wall hit", "Empty-hand C05 door-button/wall-ornament hit calls F0372, otherwise object cells call grab/drop paths.", ("G0415_ui_LeaderEmptyHanded", "C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor")),
    Citation("CLIKVIEW.C", "5-27", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor", "F0372 computes the square in front of the party and invokes F0275 on the wall face opposite party direction.", ("STATICFUNCTION void F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor", "F0275_SENSOR_IsTriggeredByClickOnWall", "M018_OPPOSITE(G0308_i_PartyDirection)")),
    Citation("MOVESENS.C", "1501-1503", "C127_SENSOR_WALL_CHAMPION_PORTRAIT -> F0280", "A clicked champion portrait wall sensor calls F0280 with sensorData/portrait index.", ("case C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty")),
    Citation("REVIVE.C", "63-88", "F0280_CHAMPION_AddCandidateChampionToParty", "Candidate champion entry point reached after C127 portrait sensor processing.", ("void F0280_CHAMPION_AddCandidateChampionToParty", "P0596_ui_ChampionPortraitIndex")),
)

BREAKPOINTS = (
    {"gate": "mouse translation / queue write", "source": "COMMAND.C:1452-1662 F0359_COMMAND_ProcessClick_CPSC (not F0365 in this tree)", "probe": "break on F0359 entry and after L1109_i_Command is assigned/written", "expect": "after x=111,y=82 left click, P0725/P0726 are 111/82, L1109_i_Command == 80, G0432_as_CommandQueue[last].Command == 80 with X=111,Y=82", "if_missing": "host/DOSBox mouse translation or active mouse-input table is blocking before the original queue"},
    {"gate": "queue dequeue", "source": "COMMAND.C:2045-2127 F0380_COMMAND_ProcessQueue_CPSC", "probe": "break when L1160/L1161/L1162 are loaded from G0432_as_CommandQueue", "expect": "L1160_i_Command == 80 and L1161/L1162 == 111/82", "if_missing": "queue overwrite/drop/BUG0_73 collision or wrong timing before dispatch"},
    {"gate": "C080 dispatch / viewport normalization", "source": "COMMAND.C:2322-2324 + CLIKVIEW.C:311-350", "probe": "break on F0377 entry and after PC coordinate normalization", "expect": "F0377 is entered; normalized point remains inside C05 wall ornament/portrait hit zone for the source-locked front wall", "if_missing": "C080 is not dispatched or screen-to-viewport translation is different than the visual click assumption"},
    {"gate": "front-wall sensor hit-state", "source": "CLIKVIEW.C:406-439, CLIKVIEW.C:5-27, MOVESENS.C:1501-1503, REVIVE.C:63-88", "probe": "break on F0372 and F0280; log G0306/G0307/G0308, forward square, wall face, sensor type/data", "expect": "pose map0 x=1 y=3 dir=South touches front square x=1 y=4 opposite face and reaches F0280(sensorData=10)", "if_missing": "front-wall hit zone/state/sensor face is blocking after F0377 but before F0280"},
)

ADDRESS_FUNCTION_ORDER = (
    "F0359_COMMAND_ProcessClick_CPSC",
    "F0380_COMMAND_ProcessQueue_CPSC",
    "F0377_COMMAND_ProcessType80_ClickInDungeonView",
    "F0275_SENSOR_IsTriggeredByClickOnWall",
    "F0280_CHAMPION_AddCandidateChampionToParty",
)

ADDRESS_GLOBAL_ORDER = (
    "G0432_as_CommandQueue",
    "G0433_i_CommandQueueFirstIndex",
    "G0434_i_CommandQueueLastIndex",
    "G0435_B_CommandQueueLocked",
    "G0436_B_PendingClickPresent",
    "G0306_i_PartyMapX",
    "G0307_i_PartyMapY",
    "G0308_i_PartyDirection",
    "G0305_ui_PartyChampionCount",
)

DEBUG_NOTES = """# pass162_c080_queue_trace breakpoint notes
# Original PC DM is DOS real-mode code under DOSBox/DOSBox-X, so these are
# source-symbol gates, not direct native gdb commands unless a symbolized/debug
# build or emulator bridge maps ReDMCSB symbols to addresses.
#
# Stop at these gates in order:
# 1. COMMAND.C:F0359_COMMAND_ProcessClick_CPSC entry and post L1109 assignment/write
# 2. COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC after L1160/L1161/L1162 dequeue
# 3. COMMAND.C:2322 C080 branch and CLIKVIEW.C:F0377 entry
# 4. CLIKVIEW.C:F0372 entry and MOVESENS.C:1501/REVIVE.C:F0280
"""

MOUSE_LOG_LINE_RE = re.compile(r"MOUSE|INT 33|button|motion|capture|release|lock", re.I)
MOUSE_ROUTE_EVENT_RE = re.compile(r"button|motion", re.I)


def display_path(path: Path | str) -> str:
    try:
        return str(Path(path).resolve().relative_to(REPO))
    except Exception:
        return str(path)


def read_lines(path: Path, line_range: str) -> str:
    if "-" in line_range:
        start, end = [int(part) for part in line_range.split("-", 1)]
    else:
        start = end = int(line_range)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(f"{idx + 1}: {lines[idx]}" for idx in range(start - 1, min(end, len(lines))))


def audit_citations() -> list[dict[str, object]]:
    audited: list[dict[str, object]] = []
    for citation in CITATIONS:
        path = SOURCE_ROOT / citation.file
        excerpt = read_lines(path, citation.lines)
        missing = [token for token in citation.must_contain if token not in excerpt]
        audited.append({**asdict(citation), "path": str(path), "ok": not missing, "missing": missing, "excerpt": excerpt})
    return audited


def run_cmd(args: Iterable[str]) -> dict[str, object]:
    try:
        proc = subprocess.run(list(args), text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=10, check=False)
        return {"args": list(args), "returncode": proc.returncode, "output": proc.stdout.strip().splitlines()[:8]}
    except Exception as exc:
        return {"args": list(args), "error": repr(exc)}


def dosbox_mouse_log_summary(log_path: object) -> dict[str, object] | None:
    if not log_path:
        return None
    path = Path(str(log_path))
    if not path.is_absolute():
        path = REPO / path
    if not path.exists():
        return {"path": display_path(path), "exists": False}
    text = path.read_text(errors="replace")
    mouse_lines = [line for line in text.splitlines() if MOUSE_LOG_LINE_RE.search(line)]
    route_event_lines = [line for line in mouse_lines if MOUSE_ROUTE_EVENT_RE.search(line)]
    return {
        "path": display_path(path),
        "exists": True,
        "mouse_line_count": len(mouse_lines),
        "route_motion_button_line_count": len(route_event_lines),
        "first_mouse_lines": mouse_lines[:8],
        "first_route_motion_button_lines": route_event_lines[:8],
    }


def local_tool(name: str) -> str | None:
    return shutil.which(name)


def load_pass273_address_bindings() -> dict[str, object]:
    if not PASS273_MANIFEST.exists():
        return {
            "status": "blocked/missing-pass273-public-symbol-manifest",
            "manifest": str(PASS273_MANIFEST),
            "functions": {},
            "globals": {},
            "missing_functions": list(ADDRESS_FUNCTION_ORDER),
            "missing_globals": list(ADDRESS_GLOBAL_ORDER),
        }
    data = json.loads(PASS273_MANIFEST.read_text())
    functions = {
        item.get("source_name"): item
        for item in data.get("functions", [])
        if isinstance(item, dict) and item.get("runtime_with_load_seg_0733")
    }
    globals_ = {
        item.get("source_name"): item
        for item in data.get("globals", [])
        if isinstance(item, dict) and item.get("runtime_with_load_seg_0733")
    }
    missing_functions = [name for name in ADDRESS_FUNCTION_ORDER if name not in functions]
    missing_globals = [name for name in ADDRESS_GLOBAL_ORDER if name not in globals_]
    return {
        "status": "ready/public-symbol-addresses-available" if not missing_functions else "blocked/pass273-missing-c080-addresses",
        "manifest": str(PASS273_MANIFEST),
        "schema": data.get("schema"),
        "load_segment_from_pass246_lineage": data.get("load_segment_from_pass246_lineage"),
        "functions": functions,
        "globals": globals_,
        "missing_functions": missing_functions,
        "missing_globals": missing_globals,
        "notes": [
            "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor is static in this FIRES.MAP build and has no public BP symbol.",
            "F0275_SENSOR_IsTriggeredByClickOnWall is the addressable proxy after F0372 computes the front-wall square/face.",
        ],
    }


def live_probe_summary(path: Path, interpretation: str) -> dict[str, object] | None:
    if not path.exists():
        return None
    probe = json.loads(path.read_text())
    runtime = probe.get("runtime") or {}
    return {
        "manifest": str(path),
        "route_label": probe.get("route_label") or runtime.get("route_label"),
        "status": probe.get("status"),
        "summary": probe.get("summary"),
        "route_driver": runtime.get("route_driver"),
        "mouse_post_mode": runtime.get("mouse_post_mode"),
        "mouse_warp": runtime.get("mouse_warp"),
        "dosbox_mouse_config": runtime.get("dosbox_mouse_config"),
        "dosbox_log": runtime.get("dosbox_log"),
        "dosbox_mouse_log_summary": dosbox_mouse_log_summary(runtime.get("dosbox_log")),
        "memory_stop_count": runtime.get("memory_stop_count"),
        "engine_ready_seen": runtime.get("engine_ready_seen"),
        "runtime_ready_seen": runtime.get("runtime_ready_seen"),
        "route_window_found": runtime.get("route_window_found"),
        "route_control_ok": runtime.get("route_control_ok"),
        "first_missing_expected_symbol": runtime.get("first_missing_expected_symbol"),
        "reached_f0280": runtime.get("reached_f0280"),
        "interpretation": interpretation,
    }


def write_address_gate_outputs() -> dict[str, object]:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    bindings = load_pass273_address_bindings()
    dosbox_debug_script = OUT_DIR / "pass162_c080_dosbox_debug_commands.txt"
    address_manifest_path = OUT_DIR / "c080_address_gate_manifest.json"
    tools = {
        "dosbox-debug": local_tool("dosbox-debug"),
        "dosbox-x": local_tool("dosbox-x"),
        "xdotool": local_tool("xdotool"),
        "python3": local_tool("python3"),
    }
    function_rows = []
    for name in ADDRESS_FUNCTION_ORDER:
        row = bindings.get("functions", {}).get(name, {"source_name": name, "status": "missing_from_pass273"})
        function_rows.append(row)
    global_rows = []
    for name in ADDRESS_GLOBAL_ORDER:
        row = bindings.get("globals", {}).get(name, {"source_name": name, "status": "missing_from_pass273"})
        global_rows.append(row)

    commands = ["BPDEL *"]
    commands.extend(f"BP {row['runtime_with_load_seg_0733']} ; {row['source_name']}" for row in function_rows if row.get("runtime_with_load_seg_0733"))
    commands.extend(f"BPM {row['runtime_with_load_seg_0733']} ; {row['source_name']}" for row in global_rows if row.get("runtime_with_load_seg_0733"))
    commands.extend(["BPLIST", "CPU", "MEMDUMP 2C20:3E7A 96 ; G0432_as_CommandQueue window", "MEMDUMP 2C20:3C92 16 ; party direction/map window"])
    dosbox_debug_script.write_text("\n".join(commands) + "\n")

    if bindings["status"] != "ready/public-symbol-addresses-available":
        classification = bindings["status"]
        first_missing_gate = "pass273 public-symbol map does not contain every C080/HoC address"
    elif not (tools.get("dosbox-debug") or tools.get("dosbox-x")):
        classification = "blocked/missing-dosbox-debug-runner"
        first_missing_gate = "no DOSBox debugger runner found"
    elif BREAK_START_ACCEPTANCE_MANIFEST.exists():
        break_start_acceptance = json.loads(BREAK_START_ACCEPTANCE_MANIFEST.read_text())
        if break_start_acceptance.get("ok") is True:
            classification = "ready/address-candidates-and-break-start-bp-bpm-confirmed-no-runtime-hook"
            first_missing_gate = None
        else:
            classification = "ready/address-candidates-emitted-break-start-control-probe-not-confirmed"
            first_missing_gate = "DOSBox-X -debug -break-start BP/BPM acceptance probe did not confirm command parsing"
    elif BP_ACCEPTANCE_MANIFEST.exists():
        bp_acceptance = json.loads(BP_ACCEPTANCE_MANIFEST.read_text())
        if bp_acceptance.get("accepted_all") is False:
            classification = "ready/address-candidates-emitted-control-probe-not-confirmed"
            first_missing_gate = "tmux/pass247 DOSBox-debug BP acceptance probe did not confirm debugger-prompt command parsing on this DOSBox-X symlink"
        else:
            classification = "ready/address-candidates-and-bp-syntax-confirmed-no-runtime-hook"
            first_missing_gate = None
    else:
        classification = "ready/address-candidates-emitted-no-runtime-hook"
        first_missing_gate = None
    bp_acceptance = json.loads(BP_ACCEPTANCE_MANIFEST.read_text()) if BP_ACCEPTANCE_MANIFEST.exists() else None
    break_start_acceptance = json.loads(BREAK_START_ACCEPTANCE_MANIFEST.read_text()) if BREAK_START_ACCEPTANCE_MANIFEST.exists() else None
    live_hoc_probe = live_probe_summary(
        LIVE_HOC_MANIFEST,
        "non-promotable stock-original diagnostic; proves the BP/BPM packet can stop inside the original runtime and native route input can be delivered on this host, but the HoC route did not reach the C080 code chain",
    )
    live_movement_control_probe = live_probe_summary(
        LIVE_MOVEMENT_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; same FIRES/DUNGEON readiness and input path with a movement-arrow click, used to distinguish HoC coordinate/state problems from pre-F0359 mouse delivery problems",
    )
    live_movement_double_control_probe = live_probe_summary(
        LIVE_MOVEMENT_DOUBLE_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; sends two movement-area clicks after FIRES/DUNGEON readiness to test whether the first click is consumed only by DOSBox focus/capture",
    )
    live_movement_autolock_control_probe = live_probe_summary(
        LIVE_MOVEMENT_AUTOLOCK_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; enables DOSBox-X autolock and always-on mouse emulation to test the SDL/capture path before F0359",
    )
    live_movement_keyboard_capture_control_probe = live_probe_summary(
        LIVE_MOVEMENT_KEYBOARD_CAPTURE_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; sends Ctrl-F10 before movement-area clicks to test DOSBox-X keyboard-driven mouse capture before F0359",
    )
    live_movement_cliclick_control_probe = live_probe_summary(
        LIVE_MOVEMENT_CLICLICK_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; uses the external cliclick tool instead of CGEvent mouse posting to test a separate macOS input path before F0359",
    )
    live_movement_systemevents_control_probe = live_probe_summary(
        LIVE_MOVEMENT_SYSTEMEVENTS_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; uses macOS System Events accessibility clicking instead of CGEvent or cliclick mouse posting before F0359",
    )
    live_movement_pid_control_probe = live_probe_summary(
        LIVE_MOVEMENT_PID_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; posts CGEvent mouse move/down/up directly to the DOSBox process PID instead of the HID event tap before F0359",
    )
    live_movement_opengl_control_probe = live_probe_summary(
        LIVE_MOVEMENT_OPENGL_CONTROL_MANIFEST,
        "non-promotable stock-original diagnostic; switches DOSBox-X output from surface to opengl to test the SDL/Cocoa backend path before F0359",
    )

    manifest = {
        "schema": "pass162_c080_address_gate.v1",
        "classification": classification,
        "first_missing_gate": first_missing_gate,
        "pass273_manifest": str(PASS273_MANIFEST),
        "load_segment": bindings.get("load_segment_from_pass246_lineage"),
        "tools": tools,
        "function_breakpoints": function_rows,
        "global_watchpoints": global_rows,
        "dosbox_debug_commands": str(dosbox_debug_script),
        "bp_acceptance_probe": (
            {
                "manifest": str(BP_ACCEPTANCE_MANIFEST),
                "status": bp_acceptance.get("status"),
                "accepted_all": bp_acceptance.get("accepted_all"),
                "interpretation": "non-promotable debugger-control diagnostic; address candidates remain valid, but this tmux path did not prove DOSBox-X debugger command parsing",
            }
            if isinstance(bp_acceptance, dict)
            else None
        ),
        "break_start_acceptance_probe": (
            {
                "manifest": str(BREAK_START_ACCEPTANCE_MANIFEST),
                "status": break_start_acceptance.get("status"),
                "ok": break_start_acceptance.get("ok"),
                "interpretation": "non-promotable debugger-control receipt; proves DOSBox-X -debug -break-start accepts the pass162 BP/BPM command packet without running original binaries",
            }
            if isinstance(break_start_acceptance, dict)
            else None
        ),
        "live_hoc_break_start_probe": (
            live_hoc_probe if isinstance(live_hoc_probe, dict) else None
        ),
        "live_movement_click_control_probe": (
            live_movement_control_probe if isinstance(live_movement_control_probe, dict) else None
        ),
        "live_movement_double_click_control_probe": (
            live_movement_double_control_probe if isinstance(live_movement_double_control_probe, dict) else None
        ),
        "live_movement_autolock_control_probe": (
            live_movement_autolock_control_probe if isinstance(live_movement_autolock_control_probe, dict) else None
        ),
        "live_movement_keyboard_capture_control_probe": (
            live_movement_keyboard_capture_control_probe if isinstance(live_movement_keyboard_capture_control_probe, dict) else None
        ),
        "live_movement_cliclick_control_probe": (
            live_movement_cliclick_control_probe if isinstance(live_movement_cliclick_control_probe, dict) else None
        ),
        "live_movement_systemevents_control_probe": (
            live_movement_systemevents_control_probe if isinstance(live_movement_systemevents_control_probe, dict) else None
        ),
        "live_movement_pid_control_probe": (
            live_movement_pid_control_probe if isinstance(live_movement_pid_control_probe, dict) else None
        ),
        "live_movement_opengl_control_probe": (
            live_movement_opengl_control_probe if isinstance(live_movement_opengl_control_probe, dict) else None
        ),
        "debugger_control_rule": {
            "continue_key": "F5 as vt100 bytes ESC O t",
            "stop_rule": "Accept a hit only after a run transition; reject BP command echoes and BPLIST setup text.",
        },
        "f0372_proxy": {
            "static_symbol": "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor",
            "addressable_proxy": "F0275_SENSOR_IsTriggeredByClickOnWall",
            "reason": "F0372 is static and absent from FIRES.MAP; F0275 is the next public wall-sensor function reached by F0372.",
        },
        "expected_order": [
            "F0359 receives screen click x=111,y=82 and queues C080 with X/Y",
            "F0380 dequeues C080 and X/Y from G0432",
            "F0377 handles C080 and normalizes viewport coordinates",
            "F0275 is reached from static F0372 for the front-wall sensor face",
            "F0280 is reached for C127 champion portrait sensor data",
        ],
        "non_claims": [
            "does not prove a stock original runtime hit",
            "does not prove F0280 is reached until a DOSBox-debug transcript records it",
            "does not promote pass435 original capture readiness",
        ],
    }
    address_manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    lines = [
        "# Pass162 C080 address gate",
        "",
        f"Classification: `{classification}`",
        "",
        "## Function breakpoints",
        "",
    ]
    for row in function_rows:
        lines.append(f"- `{row.get('source_name')}` -> `{row.get('runtime_with_load_seg_0733', 'missing')}`")
    lines.extend(["", "## Watchpoints", ""])
    for row in global_rows:
        lines.append(f"- `{row.get('source_name')}` -> `{row.get('runtime_with_load_seg_0733', 'missing')}`")
    lines.extend([
        "",
        "## DOSBox-debug commands",
        "",
        f"Commands: `{dosbox_debug_script.relative_to(REPO)}`",
        "",
        "F0372 is static in this public map; use F0275 as the addressable front-wall sensor proxy.",
        "A transcript can promote the gate only if the hit order is observed after debugger run control, not from setup echoes.",
        "",
    ])
    if isinstance(bp_acceptance, dict):
        lines.extend([
            "## BP acceptance diagnostic",
            "",
            f"- manifest: `{BP_ACCEPTANCE_MANIFEST.relative_to(REPO)}`",
            f"- status: `{bp_acceptance.get('status')}`",
            f"- accepted all: `{bp_acceptance.get('accepted_all')}`",
            "- interpretation: this is a debugger-control diagnostic only; it does not invalidate the FIRES.MAP addresses.",
            "",
        ])
    if isinstance(break_start_acceptance, dict):
        lines.extend([
            "## DOSBox-X break-start acceptance",
            "",
            f"- manifest: `{BREAK_START_ACCEPTANCE_MANIFEST.relative_to(REPO)}`",
            f"- status: `{break_start_acceptance.get('status')}`",
            f"- ok: `{break_start_acceptance.get('ok')}`",
            "- interpretation: debugger control and numeric BP/BPM command parsing are proven for an empty-autoexec DOSBox-X break-start session; stock-original runtime hits remain unproven.",
            "",
        ])
    if isinstance(live_hoc_probe, dict):
        lines.extend([
            "## Live HoC break-start diagnostic",
            "",
            f"- manifest: `{LIVE_HOC_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_hoc_probe.get('status')}`",
            f"- route driver: `{live_hoc_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_hoc_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_hoc_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_hoc_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_hoc_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_hoc_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_hoc_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_hoc_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_hoc_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_hoc_probe.get('route_window_found')}`",
            f"- route control ok: `{live_hoc_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_hoc_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_hoc_probe.get('reached_f0280')}`",
            "- interpretation: original runtime stop control and native route input are proven; the current HoC route now blocks before F0359/C080.",
            "",
        ])
    if isinstance(live_movement_control_probe, dict):
        lines.extend([
            "## Live movement-click control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_control_probe.get('status')}`",
            f"- route driver: `{live_movement_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_control_probe.get('reached_f0280')}`",
            "- interpretation: a simple movement-area click follows the same host input path and still blocks before F0359/C080, so the next pass should inspect DOSBox SDL mouse ingestion/capture rather than more HoC coordinate guesses.",
            "",
        ])
    if isinstance(live_movement_double_control_probe, dict):
        lines.extend([
            "## Live movement double-click control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_DOUBLE_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_double_control_probe.get('status')}`",
            f"- route driver: `{live_movement_double_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_double_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_double_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_double_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_double_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_double_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_double_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_double_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_double_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_double_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_double_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_double_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_double_control_probe.get('reached_f0280')}`",
            "- interpretation: if this still blocks before F0359, a first-click-only focus/capture explanation is not enough.",
            "",
        ])
    if isinstance(live_movement_autolock_control_probe, dict):
        lines.extend([
            "## Live movement autolock control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_AUTOLOCK_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_autolock_control_probe.get('status')}`",
            f"- route driver: `{live_movement_autolock_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_autolock_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_autolock_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_autolock_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_autolock_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_autolock_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_autolock_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_autolock_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_autolock_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_autolock_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_autolock_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_autolock_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_autolock_control_probe.get('reached_f0280')}`",
            "- interpretation: if this still blocks before F0359, the default autolock/mouse_emulation setting is not the missing original input boundary.",
            "",
        ])
    if isinstance(live_movement_keyboard_capture_control_probe, dict):
        lines.extend([
            "## Live movement keyboard-capture control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_KEYBOARD_CAPTURE_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_keyboard_capture_control_probe.get('status')}`",
            f"- route driver: `{live_movement_keyboard_capture_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_keyboard_capture_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_keyboard_capture_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_keyboard_capture_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_keyboard_capture_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_keyboard_capture_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_keyboard_capture_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_keyboard_capture_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_keyboard_capture_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_keyboard_capture_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_keyboard_capture_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_keyboard_capture_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_keyboard_capture_control_probe.get('reached_f0280')}`",
            "- interpretation: if this still blocks before F0359 and the log lacks motion/button lines, keyboard capture toggling did not make the injected mouse events enter DOSBox-X.",
            "",
        ])
    if isinstance(live_movement_cliclick_control_probe, dict):
        lines.extend([
            "## Live movement cliclick control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_CLICLICK_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_cliclick_control_probe.get('status')}`",
            f"- route driver: `{live_movement_cliclick_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_cliclick_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_cliclick_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_cliclick_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_cliclick_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_cliclick_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_cliclick_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_cliclick_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_cliclick_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_cliclick_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_cliclick_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_cliclick_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_cliclick_control_probe.get('reached_f0280')}`",
            "- interpretation: if this still blocks before F0359, the missing boundary is broader than the Swift/CGEvent helper and should be investigated in DOSBox-X SDL/Cocoa event ingestion or the debugger event pump.",
            "",
        ])
    if isinstance(live_movement_systemevents_control_probe, dict):
        lines.extend([
            "## Live movement System Events control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_SYSTEMEVENTS_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_systemevents_control_probe.get('status')}`",
            f"- route driver: `{live_movement_systemevents_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_systemevents_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_systemevents_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_systemevents_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_systemevents_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_systemevents_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_systemevents_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_systemevents_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_systemevents_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_systemevents_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_systemevents_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_systemevents_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_systemevents_control_probe.get('reached_f0280')}`",
            "- interpretation: if this still blocks before F0359, macOS Accessibility/System Events delivery is not enough either, so the next boundary stays inside DOSBox-X SDL/Cocoa event ingestion or the debugger event pump.",
            "",
        ])
    if isinstance(live_movement_pid_control_probe, dict):
        lines.extend([
            "## Live movement PID-post control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_PID_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_pid_control_probe.get('status')}`",
            f"- route driver: `{live_movement_pid_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_pid_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_pid_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_pid_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_pid_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_pid_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_pid_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_pid_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_pid_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_pid_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_pid_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_pid_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_pid_control_probe.get('reached_f0280')}`",
            "- interpretation: if this still blocks before F0359, direct postToPid delivery is not enough either, so the next boundary stays inside DOSBox-X SDL/Cocoa event ingestion or the debugger event pump.",
            "",
        ])
    if isinstance(live_movement_opengl_control_probe, dict):
        lines.extend([
            "## Live movement OpenGL-output control diagnostic",
            "",
            f"- manifest: `{LIVE_MOVEMENT_OPENGL_CONTROL_MANIFEST.relative_to(REPO)}`",
            f"- status: `{live_movement_opengl_control_probe.get('status')}`",
            f"- route driver: `{live_movement_opengl_control_probe.get('route_driver')}`",
            f"- mouse post mode: `{live_movement_opengl_control_probe.get('mouse_post_mode')}`",
            f"- mouse warp: `{live_movement_opengl_control_probe.get('mouse_warp')}`",
            f"- DOSBox mouse config: `{live_movement_opengl_control_probe.get('dosbox_mouse_config')}`",
            f"- DOSBox log: `{live_movement_opengl_control_probe.get('dosbox_log')}`",
            f"- DOSBox mouse log summary: `{live_movement_opengl_control_probe.get('dosbox_mouse_log_summary')}`",
            f"- memory stop count: `{live_movement_opengl_control_probe.get('memory_stop_count')}`",
            f"- engine ready seen: `{live_movement_opengl_control_probe.get('engine_ready_seen')}`",
            f"- runtime ready seen: `{live_movement_opengl_control_probe.get('runtime_ready_seen')}`",
            f"- route window found: `{live_movement_opengl_control_probe.get('route_window_found')}`",
            f"- route control ok: `{live_movement_opengl_control_probe.get('route_control_ok')}`",
            f"- first missing expected symbol: `{live_movement_opengl_control_probe.get('first_missing_expected_symbol')}`",
            f"- reached F0280: `{live_movement_opengl_control_probe.get('reached_f0280')}`",
            "- interpretation: if this run does not retain the debugger packet and reach FIRES/DUNGEON readiness, treat it as an OpenGL-backend harness/readiness blocker rather than as F0359 mouse-ingestion evidence.",
            "",
        ])
    (OUT_DIR / "c080_address_gate_README.md").write_text("\n".join(lines))
    return manifest


def write_debugger_gate_outputs() -> dict[str, object]:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    dm_exe = EXTRACTED_DM / "dm-pc34/DungeonMasterPC34/DM.EXE"
    gdb_script = OUT_DIR / "pass162_dm_exe_symbol_gate.gdb"
    dosbox_conf = OUT_DIR / "dosbox-x-pass162-runtime-gate.conf"
    gdb_script.write_text(
        "\n".join([
            "set pagination off",
            "set confirm off",
            f"file {dm_exe}",
            "break F0359_COMMAND_ProcessClick_CPSC",
            "break F0380_COMMAND_ProcessQueue_CPSC",
            "break F0377_COMMAND_ProcessType80_ClickInDungeonView",
            "break F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor",
            "break F0280_CHAMPION_AddCandidateChampionToParty",
            "info files",
            "info breakpoints",
            "",
        ])
    )
    dosbox_conf.write_text(
        f"""[sdl]
fullscreen=false
output=opengl
[dosbox]
machine=svga_paradise
memsize=4
captures={OUT_DIR}
[cpu]
core=normal
cputype=386
cpu_cycles=3000
[mixer]
nosound=true
[speaker]
pcspeaker=false
tandy=off
[capture]
capture_dir={OUT_DIR}
default_image_capture_formats=raw
[autoexec]
mount c "{dm_exe.parent}"
c:
DM -vv -sn
"""
    )

    tools = {
        "dosbox": local_tool("dosbox"),
        "dosbox-debug": local_tool("dosbox-debug"),
        "dosbox-x": local_tool("dosbox-x"),
        "gdb": local_tool("gdb"),
        "lldb": local_tool("lldb"),
        "file": local_tool("file") or "/usr/bin/file",
        "python3": local_tool("python3"),
    }
    commands = {
        "dm_exe_file": run_cmd([tools["file"], str(dm_exe)]) if tools.get("file") else {"error": "file tool missing"},
        "dosbox_x_version": run_cmd([tools["dosbox-x"], "-version"]) if tools.get("dosbox-x") else {"error": "dosbox-x missing"},
        "gdb_version": run_cmd([tools["gdb"], "--version"]) if tools.get("gdb") else {"error": "gdb missing"},
        "gdb_stock_dm_symbol_gate": run_cmd([tools["gdb"], "--batch", "-x", str(gdb_script)]) if tools.get("gdb") else {"error": "gdb missing"},
    }
    gdb_gate = commands["gdb_stock_dm_symbol_gate"]
    if not dm_exe.exists():
        classification = "blocked/missing-original-dm-exe"
        first_missing_gate = "original DM.EXE stage missing"
    elif not tools.get("dosbox-x"):
        classification = "blocked/missing-dosbox-x"
        first_missing_gate = "debugger-capable DOSBox-X runner missing"
    elif not tools.get("gdb"):
        classification = "blocked/gdb-missing-for-stock-symbol-gate"
        first_missing_gate = "native gdb symbol-binding sanity check unavailable on this host"
    elif gdb_gate.get("returncode") != 0:
        classification = "blocked/gdb-cannot-bind-stock-dos-exe-or-redmcsb-symbols"
        first_missing_gate = "debugger/source-symbol binding prerequisite; C080 mouse/queue/front-wall gates were not reached"
    else:
        classification = "ready/gdb-symbol-gate-unexpectedly-bound"
        first_missing_gate = None

    manifest = {
        "schema": "pass162_c080_gdb_gate.v2",
        "classification": classification,
        "repo": str(REPO),
        "source_root": str(SOURCE_ROOT),
        "original_stage": str(EXTRACTED_DM),
        "dm_exe": str(dm_exe),
        "first_missing_gate": first_missing_gate,
        "tools": tools,
        "commands": commands,
        "runners": {
            "gdb_script": str(gdb_script),
            "gdb_command": f"gdb --batch -x {gdb_script}",
            "dosbox_x_conf": str(dosbox_conf),
            "dosbox_x_command": f"dosbox-x -conf {dosbox_conf} -break-start",
        },
        "redmcsb_audit": [
            item for item in audit_citations()
            if item["symbol"] in {
                "F0359_COMMAND_ProcessClick_CPSC",
                "F0380_COMMAND_ProcessQueue_CPSC dequeue",
                "F0380 -> F0377 dispatch",
                "F0377_COMMAND_ProcessType80_ClickInDungeonView",
                "F0377 empty-hand front-wall hit",
                "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor",
                "C127_SENSOR_WALL_CHAMPION_PORTRAIT -> F0280",
                "F0280_CHAMPION_AddCandidateChampionToParty",
            }
        ],
        "non_claims": [
            "does not prove stock original binary reached C080/F0377/F0280",
            "does not classify mouse translation vs queue dequeue vs C080 dispatch vs F0280 until a DOS real-mode/source-symbol bridge or address map exists",
            "does not do coordinate guessing",
        ],
        "next_step": "Bind the loaded DOS image to source/addresses in DOSBox-X debugger, then apply the emitted breakpoint order.",
    }
    (OUT_DIR / "gdb_gate_manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    lines = [
        "# Pass162 C080 debugger/address gate",
        "",
        f"Classification: `{classification}`",
        f"First missing gate: `{first_missing_gate}`",
        "",
        "## Local tools",
        "",
    ]
    for name, path in tools.items():
        lines.append(f"- {name}: `{path or 'missing'}`")
    lines.extend([
        "",
        "## Commands",
        "",
        f"- gdb sanity check: `{manifest['runners']['gdb_command']}`",
        f"- DOSBox-X start point: `{manifest['runners']['dosbox_x_command']}`",
        "",
        "## Breakpoint order",
        "",
    ])
    for idx, bp in enumerate(BREAKPOINTS, 1):
        lines.append(f"{idx}. **{bp['gate']}**: {bp['probe']}; expect {bp['expect']}")
    lines.extend([
        "",
        "## Non-claims",
        "",
    ])
    lines.extend(f"- {claim}" for claim in manifest["non_claims"])
    (OUT_DIR / "gdb_gate_README.md").write_text("\n".join(lines) + "\n")
    return manifest


def build_manifest(dry_run: bool) -> dict[str, object]:
    citations = audit_citations()
    bins = {name: shutil.which(name) for name in ("dosbox", "dosbox-x", "gdb", "python3", "xdotool", "xwd")}
    pass162 = None
    if PASS162_SUMMARY.exists():
        pass162 = json.loads(PASS162_SUMMARY.read_text())
    manifest = {
        "schema": "pass162_c080_queue_trace.v1",
        "classification": "ready/probe-plan-emitted" if bins.get("dosbox-x") else "blocked/missing-debugger-capable-emulator",
        "dry_run": dry_run,
        "source_root": str(SOURCE_ROOT),
        "allowed_original_roots": [str(ORIGINAL_DM), str(EXTRACTED_DM)],
        "forbidden_roots_note": "<private-host> not used by this script.",
        "pass162_context": {"summary_path": str(PASS162_SUMMARY), "loaded": pass162 is not None, "classification": pass162.get("classification") if isinstance(pass162, dict) else None, "reason": pass162.get("reason") if isinstance(pass162, dict) else None},
        "source_audit": citations,
        "symbol_note": "Hard-rule symbol F0365 is audited, but in this ReDMCSB tree F0365 is turn-party; the actual mouse queue writer for C080 is F0359_COMMAND_ProcessClick_CPSC at COMMAND.C:1452-1662.",
        "probe_gates": BREAKPOINTS,
        "tool_probe": {"bins": bins, "dosbox_version": run_cmd([bins["dosbox"], "-version"]) if bins.get("dosbox") else None, "dosbox_x_version": run_cmd([bins["dosbox-x"], "-version"]) if bins.get("dosbox-x") else None},
        "next_step": "Run the emitted gate list in DOSBox-X/debugger against the source-locked pass162 pose; classify first missing gate instead of trying more coordinates.",
        "non_claims": ["does not prove the stock original binary reached C080/F0377/F0280", "does not use <private-host>", "does not claim x=111,y=82 is wrong; it narrows where to instrument before changing coordinates"],
    }
    if any(not item["ok"] for item in citations):
        manifest["classification"] = "blocked/source-audit-token-mismatch"
    return manifest


def write_outputs(manifest: dict[str, object]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    debugger_gate = write_debugger_gate_outputs()
    address_gate = write_address_gate_outputs()
    manifest["debugger_gate"] = {
        "path": str((OUT_DIR / "gdb_gate_manifest.json").relative_to(REPO)),
        "classification": debugger_gate.get("classification"),
        "first_missing_gate": debugger_gate.get("first_missing_gate"),
    }
    manifest["address_gate"] = {
        "path": str((OUT_DIR / "c080_address_gate_manifest.json").relative_to(REPO)),
        "classification": address_gate.get("classification"),
        "first_missing_gate": address_gate.get("first_missing_gate"),
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "breakpoint_notes.txt").write_text(DEBUG_NOTES)
    lines = ["# pass162 C080 queue trace", "", "Purpose: stop portrait coordinate guessing and isolate whether pass162's x=111,y=82 source-locked portrait click reaches the original queue/dispatch path or blocks in mouse translation/hit-state before F0280.", "", f"Classification: `{manifest['classification']}`", "", "## Source-audited path"]
    for item in manifest["source_audit"]:
        status = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {status} `{item['file']}:{item['lines']}` `{item['symbol']}` — {item['point']}")
    lines += ["", "## Narrow probe gates"]
    for idx, bp in enumerate(BREAKPOINTS, 1):
        lines.append(f"{idx}. **{bp['gate']}** — {bp['source']}; expect: {bp['expect']}; if missing: {bp['if_missing']}")
    bins = manifest["tool_probe"]["bins"]
    lines += ["", "## Tool status", f"- dosbox: `{bins.get('dosbox')}`", f"- dosbox-x: `{bins.get('dosbox-x')}`", f"- gdb: `{bins.get('gdb')}`", "", "## Non-claims"]
    lines += [f"- {claim}" for claim in manifest["non_claims"]]
    lines += [
        "",
        "## Address gate",
        f"- manifest: `{manifest['address_gate']['path']}`",
        f"- classification: `{manifest['address_gate']['classification']}`",
        "",
        f"Next step: {manifest['next_step']}",
        "",
    ]
    (OUT_DIR / "README.md").write_text("\n".join(lines))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true", help="audit and print classification without writing artifacts")
    args = parser.parse_args()
    manifest = build_manifest(dry_run=args.dry_run)
    if not args.dry_run:
        write_outputs(manifest)
    print(json.dumps({"classification": manifest["classification"], "out_dir": str(OUT_DIR), "citation_failures": [item for item in manifest["source_audit"] if not item["ok"]]}, indent=2))
    return 0 if manifest["classification"] != "blocked/source-audit-token-mismatch" else 2


if __name__ == "__main__":
    raise SystemExit(main())
