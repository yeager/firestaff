#!/usr/bin/env python3
"""Pass352 verifier: DM1 V1 movement route regression matrix across script/keypad/direct seams."""
from __future__ import annotations

import json
import os
import pathlib
import shutil
import subprocess
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass352_dm1_v1_movement_route_regression_matrix"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
EVIDENCE = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = pathlib.Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
))
BUILD_DIR = pathlib.Path(os.environ.get(
    "FIRESTAFF_PASS352_BUILD_DIR",
    "/home/trv2/.openclaw/data/firestaff-builds/pass352-verify",
))
HOME_ROOT = pathlib.Path(os.environ.get(
    "FIRESTAFF_PASS352_HOME_ROOT",
    "/home/trv2/.openclaw/data/firestaff-homes/pass352-verify",
))
DM1_DATA = pathlib.Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1")
LAUNCH = "enter,down,down,down,down,down,down,enter"
ROUTE_EQUIVALENT = "left,left,left,up,right"

SOURCE_LOCKS = [
    ("IO2.C", "32", ["IODRV_00_GetKeyboardInput"]),
    ("IO2.C", "37", ["IODRV_00_GetKeyboardInput"]),
    ("IO2.C", "47-59", ["switch (L2944_ui_ - 0x1248)", "Turn Forward", "Move Backward", "Turn Left", "Turn Right"]),
    ("COMMAND.C", "636-685", ["G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "0x004B", "0x004C", "0x004D", "0x004F", "0x0050", "0x0051"]),
    ("STARTUP2.C", "1179-1182", ["G0441_ps_PrimaryMouseInput", "G0442_ps_SecondaryMouseInput", "G0443_ps_PrimaryKeyboardInput", "G0444_ps_SecondaryKeyboardInput", "G0459_as_Graphic561_SecondaryKeyboardInput_Movement"]),
    ("COMMAND.C", "1379-1450", ["F0358_COMMAND_GetCommandFromMouseInput_CPSC"]),
    ("COMMAND.C", "1452-1661", ["F0359_COMMAND_ProcessClick_CPSC", "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G0441_ps_PrimaryMouseInput", "G0442_ps_SecondaryMouseInput", "G0432_as_CommandQueue"]),
    ("COMMAND.C", "1709-1813", ["F0361_COMMAND_ProcessKeyPress", "G0443_ps_PrimaryKeyboardInput", "G0444_ps_SecondaryKeyboardInput", "G0432_as_CommandQueue"]),
    ("COMMAND.C", "2045-2125", ["F0380_COMMAND_ProcessQueue_CPSC", "G0435_B_CommandQueueLocked", "G0432_as_CommandQueue", "G0310_i_DisabledMovementTicks"]),
    ("MENUDRAW.C", "5-19", ["F0395_MENUS_DrawMovementArrows", "C013_GRAPHIC_MOVEMENT_ARROWS", "C009_ZONE_MOVEMENT_ARROWS"]),
    ("CLIKMENU.C", "142-174", ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection"]),
    ("CLIKMENU.C", "180-347", ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"]),
]

PRODUCT_MARKERS = [
    ("main_loop_m11.c", "static M12_MenuInput m11_map_script_token"),
    ("main_loop_m11.c", "strncmp(token, \"strafe-left\", len)"),
    ("main_loop_m11.c", "if (strcmp(name, \"up\") == 0) return SDLK_UP"),
    ("main_loop_m11.c", "if (strcmp(name, \"kp-1\") == 0 || strcmp(name, \"kp1\") == 0) return SDLK_KP_1"),
    ("main_loop_m11.c", "case SDLK_KP_5:"),
    ("main_loop_m11.c", "case SDLK_KP_1:"),
    ("main_loop_m11.c", "case SDLK_KP_6:"),
    ("main_loop_m11.c", "M11_GameView_HandleInput(&gameView, input)"),
    ("m11_game_view.c", "m11_dm1_v1_pipeline_command_for_input"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat"),
    ("m11_game_view.c", "No OS keypad/NumLock synthesis is involved"),
]

EVIDENCE_MARKERS = [
    "PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED",
    "IO2.C:32",
    "IO2.C:37",
    "IO2.C:47",
    "COMMAND.C:1379",
    "COMMAND.C:1452",
    "COMMAND.C:1641",
    "COMMAND.C:1709",
    "COMMAND.C:2045-2125",
    "BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF",
    "BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E",
]

RUNTIME_EXPECTED = {
    "launchedEver": 1,
    "active": 1,
    "party": {"mapIndex": 0, "mapX": 0, "mapY": 3, "direction": 0, "championCount": 0},
    "pipeline": {"dequeued": 1, "command": 2, "turnApplied": 1, "viewportDirty": 1},
}

RUNTIME_CASES = [
    ("script_tokens", f"{LAUNCH},{ROUTE_EQUIVALENT}"),
    ("arrow_key_symbols", f"{LAUNCH},key:left,key:left,key:left,key:up,key:right"),
    ("pc34_numpad_aliases", f"{LAUNCH},key:kp4,key:kp4,key:kp4,key:kp5,key:kp6"),
    ("pc34_numpad_hyphen_aliases", f"{LAUNCH},key:kp-4,key:kp-4,key:kp-4,key:kp-5,key:kp-6"),
]


def run(cmd: list[str], *, env: dict[str, str] | None = None, timeout: int | None = None) -> dict:
    p = subprocess.run(cmd, cwd=ROOT, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": [str(c) for c in cmd], "returncode": p.returncode, "outputTail": p.stdout[-5000:]}


def slice_text(path: pathlib.Path, spec: str) -> str:
    lines = path.read_text(errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            a, b = [int(x) for x in part.split("-", 1)]
        else:
            a = b = int(part)
        chunks.append("\n".join(lines[a - 1:b]))
    return "\n".join(chunks)


def runtime_ok(runtime: dict) -> bool:
    party = runtime.get("party", {})
    pipe = runtime.get("pipeline", {})
    return (
        runtime.get("launchedEver") == 1 and
        runtime.get("active") == 1 and
        party.get("mapIndex") == 0 and
        party.get("mapX") == 0 and
        party.get("mapY") == 3 and
        party.get("direction") == 0 and
        party.get("championCount") == 0 and
        pipe.get("dequeued") == 1 and
        pipe.get("command") == 2 and
        pipe.get("turnApplied") == 1 and
        pipe.get("viewportDirty") == 1
    )


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    checks: list[dict] = []

    evidence_text = EVIDENCE.read_text(encoding="utf-8") if EVIDENCE.exists() else ""
    checks.append({"kind": "evidence_file", "ok": EVIDENCE.exists() and all(m in evidence_text for m in EVIDENCE_MARKERS), "markers": EVIDENCE_MARKERS})

    for filename, lines, markers in SOURCE_LOCKS:
        path = REDMCSB / filename
        text = slice_text(path, lines) if path.exists() else ""
        checks.append({"kind": "redmcsb_source_lock", "file": filename, "lines": lines, "markers": markers, "ok": path.exists() and all(m in text for m in markers)})

    for rel, marker in PRODUCT_MARKERS:
        path = ROOT / rel
        text = path.read_text(errors="replace") if path.exists() else ""
        checks.append({"kind": "product_marker", "file": rel, "marker": marker, "ok": marker in text})

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    cmake = run(["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)], timeout=180)
    checks.append({"kind": "cmake_configure", "ok": cmake["returncode"] == 0, "result": cmake})
    build = run(["cmake", "--build", str(BUILD_DIR), "--target", "firestaff", "test_dm1_v1_input_command_queue_pc34_compat", "test_dm1_v1_movement_pipeline_pc34_compat", "-j2"], timeout=900)
    checks.append({"kind": "cmake_build", "ok": build["returncode"] == 0, "result": build})

    input_test = BUILD_DIR / "test_dm1_v1_input_command_queue_pc34_compat"
    if input_test.exists():
        res = run([str(input_test)], timeout=120)
        checks.append({"kind": "direct_input_queue_gate", "ok": res["returncode"] == 0 and "dm1V1InputCommandQueueInvariantOk=1" in res["outputTail"] and "sourceEvidence=" in res["outputTail"], "result": res})
    else:
        checks.append({"kind": "direct_input_queue_gate", "ok": False, "missing": str(input_test)})

    pipeline_test = BUILD_DIR / "test_dm1_v1_movement_pipeline_pc34_compat"
    if pipeline_test.exists():
        res = run([str(pipeline_test)], timeout=120)
        checks.append({"kind": "direct_command_queue_pipeline_gate", "ok": res["returncode"] == 0 and "passed" in res["outputTail"] and "failed" in res["outputTail"], "result": res})
    else:
        checks.append({"kind": "direct_command_queue_pipeline_gate", "ok": False, "missing": str(pipeline_test)})

    runtime_results = []
    if HOME_ROOT.exists():
        shutil.rmtree(HOME_ROOT)
    for name, script in RUNTIME_CASES:
        home = HOME_ROOT / name
        home.mkdir(parents=True, exist_ok=True)
        probe_json = OUT_DIR / f"{name}_runtime_probe.json"
        env = os.environ.copy()
        env.update({
            "HOME": str(home),
            "SDL_VIDEODRIVER": "dummy",
            "FIRESTAFF_AUTOTEST": "1",
            "FIRESTAFF_FAIL_IF_NO_LAUNCH": "1",
            "FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON": str(probe_json),
        })
        cmd = [
            "timeout", "45s", str(BUILD_DIR / "firestaff"),
            "--duration", "9000", "--width", "1920", "--height", "1080",
            "--data-dir", str(DM1_DATA), "--script", script,
        ]
        proc = run(cmd, env=env, timeout=60)
        runtime = json.loads(probe_json.read_text()) if probe_json.exists() else {}
        ok = proc["returncode"] == 0 and probe_json.exists() and runtime_ok(runtime)
        item = {"case": name, "script": script, "probeJson": str(probe_json.relative_to(ROOT)), "ok": ok, "observed": runtime, "result": proc}
        runtime_results.append(item)
        checks.append({"kind": "runtime_route", **item})

    ok = all(c.get("ok") for c in checks)
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED" if ok else "BLOCKED_PASS352_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"])["outputTail"].strip(),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "sourceRoot": str(REDMCSB),
        "dm1Data": str(DM1_DATA),
        "expectedRuntimeState": RUNTIME_EXPECTED,
        "runtimeCases": runtime_results,
        "sourceLocks": [{"file": f, "lines": lines, "markers": markers} for f, lines, markers in SOURCE_LOCKS],
        "retiredStaleBlockersInScope": [
            "BLOCKED_FULL_LAUNCHER_SCRIPT_HANDOFF",
            "BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E for M11 SDL key:kp* route only",
        ],
        "notClaimed": ["DOSBox/original FIRES debugger hit", "pixel parity", "original DOS/I34E keyboard-buffer proof"],
        "checks": checks,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
