#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, os, subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass511_dm1_v1_movement_original_route_contract"
RED = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
DM = Path.home() / ".openclaw/data/firestaff-original-games/DM"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
STATUS = "PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED"
SOURCE_LOCKS = [
    {"id":"pc_keyboard_buffer_to_dm_command_chars","file":"IO2.C","lines":"5-61","function":"F0540_INPUT_Crawcin","needles":["F0540_INPUT_Crawcin","0x4C corresponds to Turn Forward","0x50 corresponds to Move Backward","0x4B corresponds to Turn Left","0x4D corresponds to Turn Right"],"claim":"PC/I34E route evidence must start with original keyboard-buffer tokens that map arrow scancodes to DM command chars, not host-only labels."},
    {"id":"game_loop_drains_keyboard_then_processes_queue","file":"GAMELOOP.C","lines":"164-219","function":"F0002_MAIN_GameLoop_CPSDF","needles":["G0321_B_StopWaitingForPlayerInput = C0_FALSE;","while (M527_IsCharacterInKeyboardBuffer())","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"],"claim":"the original route transcript has to cross keyboard-buffer drain, F0361 queue write, F0380 processing, and wait-loop boundary."},
    {"id":"f0361_queue_write_requires_active_table","file":"COMMAND.C","lines":"1734-1812","function":"F0361_COMMAND_ProcessKeyPress","needles":["if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)","G0435_B_CommandQueueLocked = C1_TRUE;","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;","G2153_i_QueuedCommandsCount++;","F0360_COMMAND_ProcessPendingClick();"],"claim":"a semantic route label needs a real original queue write to G0432/G2153."},
    {"id":"f0380_pop_dispatches_turn_or_step","file":"COMMAND.C","lines":"2045-2156","function":"F0380_COMMAND_ProcessQueue_CPSC","needles":["L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","G2153_i_QueuedCommandsCount--;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],"claim":"movement proof must bind the queued token to original F0380 pop and F0365/F0366 dispatch."},
    {"id":"turn_and_step_mutate_party_state","file":"CLIKMENU.C","lines":"142-347","function":"F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty","needles":["F0365_COMMAND_ProcessTypes1To2_TurnParty","F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE","F0366_COMMAND_ProcessTypes3To6_MoveParty","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0267_MOVE_GetMoveResult_CPSCE","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"],"claim":"turn/step evidence must show source-owned state delta, collision/stairs outcome, and movement cooldown boundary."},
    {"id":"f0267_commits_tuple_sensors_and_last_movement_time","file":"MOVESENS.C","lines":"738-818","function":"F0267_MOVE_GetMoveResult_CPSCE","needles":["G0397_i_MoveResultMapX = P0560_i_DestinationMapX;","G0398_i_MoveResultMapY = P0561_i_DestinationMapY;","G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;","F0317_CHAMPION_AddScentStrength","G0362_l_LastPartyMovementTime = G0313_ul_GameTime;"],"claim":"successful step proof must include committed tuple and timing/scent side effects."},
    {"id":"post_command_viewport_boundary","file":"DUNVIEW.C","lines":"8318-8611","function":"F0128_DUNGEONVIEW_Draw_CPSF","needles":["F0128_DUNGEONVIEW_Draw_CPSF","P0183_i_Direction","P0184_i_MapX","P0185_i_MapY","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],"claim":"overlay capture is movement-meaningful only after F0128 draws from the changed direction/X/Y tuple."},
    {"id":"viewport_present_boundary","file":"DRAWVIEW.C","lines":"709-858","function":"F0097_DUNGEONVIEW_DrawViewport","needles":["F0097_DUNGEONVIEW_DrawViewport","G0296_puc_Bitmap_Viewport","VIDRV_09_BlitViewPort"],"claim":"the route transcript must land at viewport present seam before screenshots can become overlay evidence."},
]
REQUIRED_PRIOR_GATES = {
    "pass504_keyboard_buffer_state_delta_blocker": (ROOT/"parity-evidence/verification/pass504_dm1_v1_keyboard_buffer_state_delta_blocker/manifest.json","PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER_LOCKED"),
    "pass505_blocked_collision_timing": (ROOT/"parity-evidence/verification/pass505_dm1_v1_blocked_movement_collision_timing_gap/manifest.json","PASS505_DM1_V1_BLOCKED_MOVEMENT_COLLISION_TIMING_SOURCE_LOCKED"),
    "pass506_stairs_side_effects": (ROOT/"parity-evidence/verification/pass506_dm1_v1_stairs_movement_side_effect_source_lock/manifest.json","PASS506_DM1_V1_STAIRS_MOVEMENT_SIDE_EFFECT_SOURCE_LOCK_PROVEN"),
    "pass508_key_route_state_delta": (ROOT/"parity-evidence/verification/pass508_dm1_v1_key_route_state_delta_gate/manifest.json","PASS508_DM1_V1_KEY_ROUTE_STATE_DELTA_GATE_LOCKED"),
    "pass509_keyboard_buffer_blocker": (ROOT/"parity-evidence/verification/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker/manifest.json","PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER_LOCKED"),
    "pass510_route_label_filename_fixture": (ROOT/"parity-evidence/verification/pass510_dm1_v1_original_capture_route_label_filename_fixture/manifest.json","PASS510_ORIGINAL_CAPTURE_ROUTE_LABEL_FILENAME_FIXTURE"),
}
ASSET_REFS = {"canonicalDm1DungeonDat":DM/"_canonical/dm1/DUNGEON.DAT","canonicalDm1GraphicsDat":DM/"_canonical/dm1/GRAPHICS.DAT","canonicalDm1Title":DM/"_canonical/dm1/TITLE","canonicalDm1Readme":DM/"_canonical/dm1/README.md","greatstoneOverview":GREATSTONE/"raw/greatstone.free.fr__dm__g_dm.html.html","greatstonePc34DiffManifest":DM/"_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.json"}

def norm(text): return " ".join(text.split())
def read_text(path, encoding="utf-8"):
    if not path.exists(): raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")
def source_window(path, spec):
    lines = read_text(path, "latin-1").splitlines(); out = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)]
        out.extend(lines[start-1:end])
    return "\n".join(out)
def sha256(path):
    h=hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024*1024), b""): h.update(chunk)
    return h.hexdigest()
def audit_sources():
    rows=[]
    for lock in SOURCE_LOCKS:
        path=RED/lock["file"]; text=source_window(path, lock["lines"]) if path.exists() else ""
        missing=[needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        row=dict(lock); row["path"]=str(path); row["ok"]=path.exists() and not missing; row["missing"]=missing; row.pop("needles",None); rows.append(row)
    return rows
def load_json(path): return json.loads(read_text(path))
def prior_gate_rows():
    rows={}
    for name,(path,expected) in REQUIRED_PRIOR_GATES.items():
        status=load_json(path).get("status") if path.exists() else None
        rows[name]={"path":str(path.relative_to(ROOT)),"expectedStatus":expected,"status":status,"ok":status==expected}
    return rows
def asset_rows():
    return {name:{"path":str(path),"exists":path.exists(),"sha256":sha256(path) if path.exists() else None,"size":path.stat().st_size if path.exists() else None} for name,path in ASSET_REFS.items()}
def completion_row():
    for row in load_json(ROOT/"parity-evidence/verification/firestaff_completion_matrix.json")["rows"]:
        if row["target"]=="DM1 V1": return row
    raise AssertionError("missing DM1 V1 completion row")
def run(cmd, timeout=120):
    proc=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=timeout)
    return {"cmd":cmd,"returncode":proc.returncode,"outputTail":proc.stdout[-3000:]}
def write_report(payload):
    lines=["# Pass511 - DM1 V1 movement original route contract","","Status: "+payload["status"],"","## Decision","",payload["decision"],"","## ReDMCSB source audit",""]
    for row in payload["sourceAudit"]:
        state="PASS" if row["ok"] else "FAIL"; lines.append("- {} {}:{} / {} - {}".format(state, row["file"], row["lines"], row["function"], row["claim"]))
    lines += ["","## Required prior gates",""]
    for name,row in payload["priorGates"].items():
        state="PASS" if row["ok"] else "FAIL"; lines.append("- {} {} -> {}".format(state, name, row["status"]))
    lines += ["","## Original/Greatstone anchors",""]
    for name,row in payload["assetRefs"].items():
        state="PASS" if row["exists"] else "FAIL"; lines.append("- {} {} {} sha256={}".format(state, name, row["path"], row["sha256"]))
    lines += ["","## Artifact contract for the next landing step",""]
    lines.extend("- "+item for item in payload["nextLandingStep"]["requiredArtifacts"])
    lines += ["","## Non-claims",""]; lines.extend("- "+item for item in payload["nonClaims"]); lines += ["",f"Manifest: {MANIFEST.relative_to(ROOT)}",""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")
def main():
    source=audit_sources(); priors=prior_gate_rows(); assets=asset_rows(); dm1=completion_row()
    gates={"firestaff_completion_matrix":run(["python3","tools/verify_firestaff_completion_matrix.py"])}
    problems=[]
    problems += ["source lock failed {}:{} missing={}".format(r["file"], r["lines"], r["missing"]) for r in source if not r["ok"]]
    problems += ["prior gate failed {}: {} != {}".format(n, r["status"], r["expectedStatus"]) for n,r in priors.items() if not r["ok"]]
    problems += ["missing asset/ref {}: {}".format(n, r["path"]) for n,r in assets.items() if not r["exists"]]
    problems += [f"gate failed {n}" for n,r in gates.items() if r["returncode"] != 0]
    if dm1["scores"]["core_input_movement"][0] != 13 or dm1["scores"]["original_overlay_regression"][0] != 0: problems.append("DM1 V1 matrix no longer has expected movement/original-overlay boundary")
    if "Original DOSBox/FIRES keyboard-buffer transcript" not in " ".join(dm1["primaryBlockers"]): problems.append("DM1 V1 matrix no longer names the keyboard-buffer transcript blocker")
    payload={"schema":f"firestaff.parity.{PASS}.v1","status":STATUS if not problems else "FAIL_PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT","ok":not problems,"sourceRoot":str(RED),"decision":"The next landable DM1 V1 movement step is a fresh original-runtime route transcript, not another Firestaff movement implementation patch: prove keyboard-buffer token -> F0361 queue write -> F0380 pop -> F0365/F0366 state delta -> F0267 tuple/timing for steps -> F0128/F0097 post-command viewport boundary, then attach route-labeled original captures.","sourceAudit":source,"priorGates":priors,"assetRefs":assets,"completionBoundary":{"completionPercent":dm1["completionPercent"],"coreInputMovement":dm1["scores"]["core_input_movement"],"originalOverlayRegression":dm1["scores"]["original_overlay_regression"],"primaryBlockers":dm1["primaryBlockers"]},"nextLandingStep":{"id":"original_runtime_keyboard_buffer_to_post_command_viewport_transcript","requiredArtifacts":["per-token original PC/I34E keyboard-buffer value from IO2/F0540 or equivalent memory watch","F0361 queue write record: command id, G0432 slot, G0434 last index, G2153 increment","F0380 pop record: same command id, G0433 first index, G2153 decrement","handler record: F0365 direction mutation or F0366 target/collision/stairs outcome","for successful steps: F0267 committed map index, X, Y, direction/cell, and last-movement-time side effect","post-command F0128/F0097 boundary record tied to the same tuple","route-labeled original viewport/HUD captures whose filenames match route labels and whose hashes are not repeated unless source trace proves a no-op/block"],"smallestVerificationGate":"one turn, one blocked step, and one successful step with the above records is enough for a follow-up promotable evidence pass; pixel parity can remain out of scope."},"gates":gates,"nonClaims":["no new DOSBox/FIRES capture was launched","no original-vs-Firestaff pixel parity is claimed","no completion percentage change is claimed","no viewport/wall implementation is modified"],"problems":problems}
    OUT_DIR.mkdir(parents=True, exist_ok=True); MANIFEST.write_text(json.dumps(payload,indent=2,sort_keys=True)+"\n",encoding="utf-8"); write_report(payload)
    print(json.dumps({"status":payload["status"],"ok":payload["ok"],"manifest":str(MANIFEST.relative_to(ROOT)),"report":str(REPORT.relative_to(ROOT)),"problems":problems},indent=2,sort_keys=True)); return 0 if payload["ok"] else 1
if __name__ == "__main__": raise SystemExit(main())
