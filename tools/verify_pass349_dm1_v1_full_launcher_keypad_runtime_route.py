#!/usr/bin/env python3
"""Pass349 verifier: full launcher --script with PC34 keypad key tokens reaches live DM1 V1 movement path."""
from __future__ import annotations

import json
import os
import pathlib
import shutil
import subprocess
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass349_dm1_v1_full_launcher_keypad_runtime_route"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
BUILD_DIR = pathlib.Path(os.environ.get("FIRESTAFF_PASS349_BUILD_DIR", str(pathlib.Path.home() / ".openclaw/data/firestaff-builds/pass349-verify")))
HOME_DIR = pathlib.Path(os.environ.get("FIRESTAFF_PASS349_HOME_DIR", str(pathlib.Path.home() / ".openclaw/data/firestaff-homes/pass349-verify")))
REDMCSB = pathlib.Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(pathlib.Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
DM1_DATA = pathlib.Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1")
SCRIPT = "enter,down,down,down,down,down,down,enter,key:kp4,key:kp4,key:kp4,key:kp5,key:kp6"

SOURCE_LOCKS = [
    ("COMMAND.C", "636-685", ["G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "0x004B", "0x004C", "0x004D", "0x004F", "0x0050", "0x0051"]),
    ("COMMAND.C", "1709-1813", ["F0361_COMMAND_ProcessKeyPress", "G0443_ps_PrimaryKeyboardInput", "G0444_ps_SecondaryKeyboardInput"]),
    ("COMMAND.C", "2045-2156", ["F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
    ("GAMELOOP.C", "150-155", ["G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks"]),
    ("GAMELOOP.C", "164-168,215", ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC"]),
    ("STARTUP2.C", "1179-1183", ["G0441_ps_PrimaryMouseInput", "G0442_ps_SecondaryMouseInput", "G0443_ps_PrimaryKeyboardInput", "G0444_ps_SecondaryKeyboardInput", "G0459_as_Graphic561_SecondaryKeyboardInput_Movement"]),
    ("CLIKMENU.C", "142-174", ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection"]),
    ("CLIKMENU.C", "180-347", ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"]),
    ("MOVESENS.C", "316-326", ["F0267_MOVE_GetMoveResult_CPSCE"]),
    ("IO2.C", "5-61", ["F0540_INPUT_Crawcin", "IODRV_00_GetKeyboardInput"]),
]

PRODUCT_MARKERS = [
    ("main_loop_m11.c", "m11_push_script_event_token"),
    ("main_loop_m11.c", "kp4"),
    ("main_loop_m11.c", "SDLK_KP_4"),
    ("main_loop_m11.c", "SDLK_KP_5"),
    ("main_loop_m11.c", "SDLK_KP_6"),
    ("main_loop_m11.c", "M11_GameView_HandleInput(&gameView, input)"),
    ("main_loop_m11.c", "FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON"),
    ("m11_game_view.c", "m11_dm1_v1_pipeline_command_for_input"),
    ("m11_game_view.c", "DM1_V1_COMMAND_MOVE_FORWARD"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat"),
]


def run(cmd, *, env=None, timeout=None):
    p = subprocess.run(cmd, cwd=ROOT, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": [str(c) for c in cmd], "returncode": p.returncode, "outputTail": p.stdout[-5000:]}


def line_text(path: pathlib.Path, spec: str) -> str:
    lines = path.read_text(errors="replace").splitlines()
    chunks = []
    for part in spec.split(','):
        if '-' in part:
            a, b = [int(x) for x in part.split('-')]
        else:
            a = b = int(part)
        chunks.append("\n".join(lines[a-1:b]))
    return "\n".join(chunks)


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    checks = []

    for rel, marker in PRODUCT_MARKERS:
        text = (ROOT / rel).read_text(errors="replace") if (ROOT / rel).exists() else ""
        checks.append({"kind": "product_marker", "file": rel, "marker": marker, "ok": marker in text})

    for filename, lines, markers in SOURCE_LOCKS:
        path = REDMCSB / filename
        text = line_text(path, lines) if path.exists() else ""
        checks.append({"kind": "redmcsb_source_lock", "file": filename, "lines": lines, "markers": markers, "ok": path.exists() and all(m in text for m in markers)})

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    cmake = run(["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)], timeout=180)
    checks.append({"kind": "cmake_configure", "ok": cmake["returncode"] == 0, "result": cmake})
    build = run(["cmake", "--build", str(BUILD_DIR), "--target", "firestaff", "test_dm1_v1_movement_pipeline_pc34_compat", "-j2"], timeout=900)
    checks.append({"kind": "cmake_build", "ok": build["returncode"] == 0, "result": build})

    test_bin = BUILD_DIR / "test_dm1_v1_movement_pipeline_pc34_compat"
    if test_bin.exists():
        test = run([str(test_bin)], timeout=120)
        checks.append({"kind": "narrow_test", "name": test_bin.name, "ok": test["returncode"] == 0 and "138 passed, 0 failed" in test["outputTail"], "result": test})
    else:
        checks.append({"kind": "narrow_test", "name": test_bin.name, "ok": False, "missing": str(test_bin)})

    if HOME_DIR.exists():
        shutil.rmtree(HOME_DIR)
    HOME_DIR.mkdir(parents=True, exist_ok=True)
    probe_json = OUT_DIR / "full_launcher_keypad_runtime_probe.json"
    env = os.environ.copy()
    env.update({
        "HOME": str(HOME_DIR),
        "SDL_VIDEODRIVER": "dummy",
        "FIRESTAFF_AUTOTEST": "1",
        "FIRESTAFF_FAIL_IF_NO_LAUNCH": "1",
        "FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON": str(probe_json),
    })
    command = [
        "timeout", "45s", str(BUILD_DIR / "firestaff"),
        "--duration", "9000", "--width", "1920", "--height", "1080",
        "--data-dir", str(DM1_DATA), "--script", SCRIPT,
    ]
    probe = run(command, env=env, timeout=60)
    checks.append({"kind": "full_launcher_probe", "ok": probe["returncode"] == 0 and probe_json.exists(), "script": SCRIPT, "result": probe})

    runtime = {}
    route_ok = False
    if probe_json.exists():
        runtime = json.loads(probe_json.read_text())
        party = runtime.get("party", {})
        pipe = runtime.get("pipeline", {})
        route_ok = (
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
    checks.append({"kind": "runtime_state", "ok": route_ok, "expected": {"launchedEver": 1, "active": 1, "party": {"mapIndex": 0, "mapX": 0, "mapY": 3, "direction": 0, "championCount": 0}, "lastPipeline": {"command": 2, "turnApplied": 1, "viewportDirty": 1}}, "observed": runtime})

    ok = all(c.get("ok") for c in checks)
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED" if ok else "BLOCKED_PASS349_FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"])["outputTail"].strip(),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "sourceRoot": str(REDMCSB),
        "dm1Data": str(DM1_DATA),
        "script": SCRIPT,
        "probeJson": str(probe_json.relative_to(ROOT)),
        "sourceLocks": [{"file": f, "lines": lines, "markers": markers} for f, lines, markers in SOURCE_LOCKS],
        "checks": checks,
        "notClaimed": ["DOSBox/original FIRES debugger hit", "pixel parity"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
