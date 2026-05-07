#!/usr/bin/env python3
"""Pass345 verifier: live M11 route input -> DM1 V1 compat pipeline -> redraw."""
from __future__ import annotations
import json, os, shutil, subprocess
from datetime import datetime, timezone
from pathlib import Path

PASS = "pass345_dm1_v1_route_to_live_viewport_bridge"
ROOT = Path(__file__).resolve().parents[1]
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
DEFAULT_REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DEFAULT_DM1_DATA = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
BUILD_DIR = Path(os.environ.get("FIRESTAFF_PASS345_BUILD_DIR", str(Path.home() / ".openclaw/data/firestaff-builds/pass345-verify")))
SOURCE_LOCKS = [
    ("COMMAND.C", 252, 260, ["G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT"]),
    ("COMMAND.C", 272, 305, ["0xAB34", "0xAB35", "0xAB36", "0x9B41"]),
    ("GAMELOOP.C", 150, 155, ["G0310_i_DisabledMovementTicks", "G0311_i_ProjectileDisabledMovementTicks"]),
    ("GAMELOOP.C", 164, 219, ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC", "G0321_B_StopWaitingForPlayerInput"]),
    ("CLIKMENU.C", 142, 174, ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection", "F0276_SENSOR_ProcessThingAdditionOrRemoval"]),
    ("CLIKMENU.C", 180, 347, ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"]),
    ("MOVESENS.C", 316, 326, ["F0267_MOVE_GetMoveResult_CPSCE"]),
]
PRODUCT_MARKERS = [
    ("m11_game_view.h", "struct Dm1V1MovementPipelinePc34Compat dm1V1MovementPipeline"),
    ("m11_game_view.h", "struct Dm1V1MovementPipelineResultPc34Compat lastDm1V1MovementPipelineResult"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_InitPc34Compat(&state->dm1V1MovementPipeline)"),
    ("m11_game_view.c", "m11_dm1_v1_pipeline_command_for_input"),
    ("m11_game_view.c", "DM1_V1_COMMAND_MOVE_FORWARD"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat"),
    ("m11_game_view.c", "lastDm1V1MovementPipelineResult.viewportDirty"),
    ("m11_game_view.c", "return M11_GAME_INPUT_REDRAW"),
]
EXPECTED_HALL = {
    "start_hall_initial_south": {"mapX": 1, "mapY": 3, "direction": 2},
    "turn_left_east_view_changes": {"mapX": 1, "mapY": 3, "direction": 1, "result": 1},
    "step_east_blocked_by_hall_side": {"mapX": 1, "mapY": 3, "direction": 1, "result": 1},
    "turn_left_north_front_mirror": {"direction": 0, "front.mirrorOrdinal": 1, "result": 1},
    "step_north_mirror_wall_blocked": {"mapX": 1, "mapY": 3, "direction": 0, "result": 1},
    "turn_left_west_open_lane": {"direction": 3, "result": 1},
    "step_west_moves_in_hall": {"mapX": 0, "mapY": 3, "direction": 3, "result": 1},
    "turn_right_north_from_west_cell": {"direction": 0, "result": 1},
    "turn_right_east_front_second_mirror": {"direction": 1, "front.mirrorOrdinal": 2, "result": 1},
}

def run(cmd):
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-4000:]}

def line_window(path, start, end):
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start-1:end])

def nested(row, key):
    cur = row
    for part in key.split('.'):
        cur = cur[part]
    return cur

def main():
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    redmcsb = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(DEFAULT_REDMCSB)))
    dm1_data = Path(os.environ.get("FIRESTAFF_DM1_CANONICAL_DATA", str(DEFAULT_DM1_DATA)))
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    checks = []
    for rel, marker in PRODUCT_MARKERS:
        text = (ROOT / rel).read_text(errors="replace")
        checks.append({"kind":"product_marker","file":rel,"marker":marker,"ok":marker in text})
    for filename, start, end, markers in SOURCE_LOCKS:
        path = redmcsb / filename
        text = line_window(path, start, end) if path.exists() else ""
        checks.append({"kind":"redmcsb_source_lock","file":filename,"lines":f"{start}-{end}","ok":path.exists() and all(m in text for m in markers),"markers":markers})
    test_bin = BUILD_DIR / "test_dm1_v1_movement_pipeline_pc34_compat"
    probe_bin = BUILD_DIR / "firestaff_m11_hall_walkaround_runtime_probe"
    build_config = run(["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)])
    checks.append({"kind":"build_configure","name":"cmake_configure_build_pass345","ok":build_config["returncode"] == 0,"result":build_config})
    build_res = run(["cmake", "--build", str(BUILD_DIR), "--target", "firestaff_m11_hall_walkaround_runtime_probe", "test_dm1_v1_movement_pipeline_pc34_compat", "-j1"])
    checks.append({"kind":"build_targets","name":"pass345_narrow_targets","ok":build_res["returncode"] == 0 and test_bin.exists() and probe_bin.exists(),"result":build_res})
    if test_bin.exists():
        res = run([str(test_bin)])
        checks.append({"kind":"narrow_test","name":test_bin.name,"ok":res["returncode"] == 0 and "138 passed, 0 failed" in res["outputTail"],"result":res})
    else:
        checks.append({"kind":"narrow_test","name":test_bin.name,"ok":False,"missing":str(test_bin)})
    hall_dir = VERIFY_DIR / "hall_probe"
    if probe_bin.exists() and dm1_data.exists():
        hall_dir.mkdir(parents=True, exist_ok=True)
        res = run([str(probe_bin), str(dm1_data), str(hall_dir)])
        checks.append({"kind":"runtime_probe","name":probe_bin.name,"ok":res["returncode"] == 0 and "PASS dm1 v1 hall walkaround runtime probe" in res["outputTail"],"result":res})
    else:
        checks.append({"kind":"runtime_probe","name":probe_bin.name,"ok":False,"missing":[str(p) for p in (probe_bin, dm1_data) if not p.exists()]})
    hall_json = hall_dir / "dm1_v1_hall_walkaround_runtime_probe.json"
    if hall_json.exists():
        data = json.loads(hall_json.read_text())
        rows = {r["name"]: r for r in data.get("steps", [])}
        route_checks = []
        for name, expected in EXPECTED_HALL.items():
            row = rows.get(name)
            route_checks.append({"name":name,"ok":row is not None and all(nested(row,k)==v for k,v in expected.items()),"expected":expected})
        stable = all(r.get("championCount") == 0 and r.get("candidateMirrorPanelActive") == 0 and r.get("result") == 1 for r in data.get("steps", []))
        checks.append({"kind":"runtime_probe_manifest","path":str(hall_json.relative_to(ROOT)),"ok":all(c["ok"] for c in route_checks) and stable,"routeChecks":route_checks,"championPanelStable":stable})
    else:
        checks.append({"kind":"runtime_probe_manifest","path":str(hall_json.relative_to(ROOT)),"ok":False,"missing":True})
    ok = all(c.get("ok") for c in checks)
    manifest = {"schema":f"{PASS}.v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":"BRIDGE_CLOSED" if ok else "BLOCKED_PASS345_ROUTE_TO_LIVE_VIEWPORT_BRIDGE_VERIFICATION_FAILED","repo":str(ROOT),"sourceRoot":str(redmcsb),"dm1Data":str(dm1_data),"bridge":{"routeInput":"main_loop_m11.c M12 route tokens/up-left-right/down/strafe","productEntry":"M11_GameView_HandleInput","compatPath":"DM1_V1_MovementPipeline_EnqueueCommandPc34Compat -> DM1_V1_MovementPipeline_ProcessOneTickPc34Compat","viewportPath":"lastDm1V1MovementPipelineResult.viewportDirty/dequeued -> M11_GAME_INPUT_REDRAW","keyboardHack":False},"sourceLocks":[{"file":f,"lines":f"{a}-{b}","markers":m} for f,a,b,m in SOURCE_LOCKS],"checks":checks,"notClaimed":["DOSBox/original FIRES debugger hit","pixel parity","full launcher handoff closure"]}
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True)+"\n")
    print(f"status={manifest['status']}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    return 0 if ok else 1
if __name__ == "__main__":
    raise SystemExit(main())
