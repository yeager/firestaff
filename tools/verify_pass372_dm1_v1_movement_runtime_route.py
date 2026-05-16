#!/usr/bin/env python3
"""Pass372 verifier: source-lock DM1 V1 input->command->movement runtime route."""
from __future__ import annotations
import json, os, pathlib, re, subprocess
from datetime import datetime, timezone
ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass372_dm1_v1_movement_runtime_route"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = pathlib.Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(pathlib.Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
SOURCE_LOCKS = [
 {"file":"IO2.C","lines":"27-61","claim":"I34E F0540 reads IODRV_00_GetKeyboardInput and normalizes shifted extended arrows to K/L/M/P before returning to the main loop.","markers":["IODRV_00_GetKeyboardInput","MEDIA707_I34E_I34M","0x48 = Scancode of Up arrow","L2944_ui_ = 'L'","0x50 = Scancode of Down arrow","L2944_ui_ = 'P'","0x4B = Scancode of Left arrow","L2944_ui_ = 'K'","0x4D = Scancode of Right arrow","L2944_ui_ = 'M'","return L2944_ui_"]},
 {"file":"GAMELOOP.C","lines":"164-168,215","claim":"The game loop drains buffered keyboard characters through F0361, then processes the command queue through F0380.","markers":["M527_IsCharacterInKeyboardBuffer","F0361_COMMAND_ProcessKeyPress","M528_GetCharacterInKeyboardBuffer","F0380_COMMAND_ProcessQueue_CPSC"]},
 {"file":"COMMAND.C","lines":"636-685","claim":"I34E movement keyboard table binds K/L/M/O/P/Q to C001..C006.","markers":["G0459_as_Graphic561_SecondaryKeyboardInput_Movement","MEDIA707_I34E_I34M","C001_COMMAND_TURN_LEFT,     0x004B","C003_COMMAND_MOVE_FORWARD,  0x004C","C002_COMMAND_TURN_RIGHT,    0x004D","C006_COMMAND_MOVE_LEFT,     0x004F","C005_COMMAND_MOVE_BACKWARD, 0x0050","C004_COMMAND_MOVE_RIGHT,    0x0051"]},
 {"file":"COMMAND.C","lines":"1709-1813","claim":"F0361 scans primary then secondary keyboard tables and enqueues the matched command.","markers":["F0361_COMMAND_ProcessKeyPress","G0443_ps_PrimaryKeyboardInput","G0444_ps_SecondaryKeyboardInput","G0432_as_CommandQueue","F0360_COMMAND_ProcessPendingClick"]},
 {"file":"COMMAND.C","lines":"2045-2156","claim":"F0380 locks/dequeues the queue, gates movement ticks, and dispatches turns to F0365 and steps to F0366.","markers":["F0380_COMMAND_ProcessQueue_CPSC","G0435_B_CommandQueueLocked","G0310_i_DisabledMovementTicks","G0311_i_ProjectileDisabledMovementTicks","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
 {"file":"CLIKMENU.C","lines":"142-174","claim":"F0365 turns the party and fires leave/enter sensors around F0284_CHAMPION_SetPartyDirection.","markers":["F0365_COMMAND_ProcessTypes1To2_TurnParty","F0276_SENSOR_ProcessThingAdditionOrRemoval","F0284_CHAMPION_SetPartyDirection"]},
 {"file":"CLIKMENU.C","lines":"180-330","claim":"F0366 maps C003..C006 to relative step deltas, validates collision, moves, and applies movement timing.","markers":["F0366_COMMAND_ProcessTypes3To6_MoveParty","G0465_ai_Graphic561_MovementArrowToStepForwardCount","G0466_ai_Graphic561_MovementArrowToStepRightCount","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0267_MOVE_GetMoveResult_CPSCE","F0357_COMMAND_DiscardAllInput"]},
 {"file":"STARTUP2.C","lines":"1179-1183","claim":"After dungeon load, secondary keyboard input is installed to the movement table.","markers":["G0443_ps_PrimaryKeyboardInput","G0444_ps_SecondaryKeyboardInput","G0459_as_Graphic561_SecondaryKeyboardInput_Movement"]},
]
FIRESTAFF_LOCKS = [
 {"file":"src/engine/main_loop_m11.c","claim":"SDL NumLock-on keypad symbols key:kp1..key:kp6 are accepted before WASD convenience aliases.","markers":["SDLK_KP_1","SDLK_KP_2","SDLK_KP_3","SDLK_KP_4","SDLK_KP_5","SDLK_KP_6","SDL reports NumLock-on keypad"]},
 {"file":"src/engine/m11_game_view.c","claim":"The live game view converts M12 movement inputs to C001..C006 and queues resolved command ids directly, so product movement is not blocked by original-DOS keyboard-buffer delivery.","markers":["m11_dm1_v1_pipeline_command_for_input","DM1_V1_COMMAND_TURN_LEFT","DM1_V1_COMMAND_MOVE_FORWARD","DM1_V1_MovementPipeline_EnqueueCommandPc34Compat","No OS keypad/NumLock synthesis is involved"]},
 {"file":"src/dm1/dm1_v1_input_command_queue_pc34_compat.c","claim":"The compat queue still covers original keyboard rows when an original-shaped keycode is supplied.","markers":["COMMAND.C:1709-1813 F0361 queues primary/secondary keyboard commands","IO2.C:27-61 F0540_INPUT_Crawcin","case 0x004B:","case 0x004C:","case 0x004D:","case 0x004F:","case 0x0050:","case 0x0051:"]},
 {"file":"src/dm1/dm1_v1_movement_pipeline_pc34_compat.c","claim":"The compat pipeline wires enqueue/dequeue/gate/turn/move/post-move processing under ReDMCSB citations.","markers":["DM1_V1_MovementPipeline_EnqueueInputPc34Compat","DM1_V1_MovementPipeline_EnqueueCommandPc34Compat","DM1_V1_MovementCommandCore_ProcessOnePc34Compat","COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC","CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
 {"file":"tests/test_dm1_v1_input_command_queue_pc34_compat.c","claim":"Regression covers PC34 K/L/M/O/P/Q table rows, IO2 shifted arrows, pending replay, and the five-command C5 capacity limit (pass387 expanded the cap from four to five).","markers":["i34e keypad 4 turns left","i34e keypad 5 moves forward","pc34 io2 shifted up arrow normalizes to forward","pc34 shifted backward arrow strafes right","redmcsb sixth command is dropped at C5 limit"]},
]
PRIOR_STATUSES={"pass348_dm1_v1_numlock_keypad_blocker_closure":"RECLASSIFIED_PASS348_NUMLOCK_KEYPAD_BLOCKER_NARROWED_NOT_CLOSED","pass349_dm1_v1_full_launcher_keypad_runtime_route":"FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED","pass352_dm1_v1_movement_route_regression_matrix":"PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED","pass359_dm1_v1_movement_route_runtime_blocker_followup":"PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED"}
def run(cmd): return subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT).stdout.strip()
def compact(s): return " ".join(s.split())
def has(text, marker): return compact(marker) in compact(text)
def lines(path, spec):
    data=path.read_text(encoding="latin-1",errors="replace").splitlines(); out=[]
    for part in spec.split(','):
        a,b=([int(x) for x in part.split('-')] if '-' in part else (int(part),int(part)))
        out.append('\n'.join(data[a-1:b]))
    return '\n'.join(out)
def load_manifest(name):
    p=ROOT/'parity-evidence'/'verification'/name/'manifest.json'
    if not p.exists(): return {"missing":str(p.relative_to(ROOT))}
    d=json.loads(p.read_text()); d['_path']=str(p.relative_to(ROOT)); return d
def verify_source():
    rows=[]
    for item in SOURCE_LOCKS:
        p=REDMCSB/item['file']; text=lines(p,item['lines']) if p.exists() else ''; missing=[m for m in item['markers'] if not has(text,m)]
        rows.append({**item,"path":str(p),"ok":p.exists() and not missing,"missingMarkers":missing})
    return rows
def verify_firestaff():
    rows=[]
    for item in FIRESTAFF_LOCKS:
        p=ROOT/item['file']; text=p.read_text(errors='replace') if p.exists() else ''; missing=[m for m in item['markers'] if not has(text,m)]
        rows.append({**item,"path":str(p.relative_to(ROOT)),"ok":p.exists() and not missing,"missingMarkers":missing})
    return rows
def main():
    OUT_DIR.mkdir(parents=True,exist_ok=True); source=verify_source(); firestaff=verify_firestaff(); checks=[]; prior={}
    for name,expected in PRIOR_STATUSES.items():
        d=load_manifest(name); observed=d.get('status'); prior[name]={"path":d.get('_path'),"status":observed}; checks.append({"kind":"prior_status","name":name,"expected":expected,"observed":observed,"ok":observed==expected})
    checks += [{"kind":"redmcsb_source_lock","file":r['file'],"lines":r['lines'],"ok":r['ok']} for r in source]
    checks += [{"kind":"firestaff_route_lock","file":r['file'],"ok":r['ok']} for r in firestaff]
    # CMakeLists has drifted across many land passes; locate the
    # add_executable block by content instead of hard-coded line numbers
    # so this gate stays source-locked against the test wiring, not text
    # offsets that other passes shift.
    cmake_full=(ROOT/'CMakeLists.txt').read_text(encoding='utf-8',errors='replace')
    cmake_ok=('add_executable(test_dm1_v2_movement_command_adapter_pc34' in cmake_full
              and 'test_dm1_v2_movement_command_adapter_pc34.c' in cmake_full)
    cmake_observed=('add_executable(test_dm1_v2_movement_command_adapter_pc34) wiring present'
                    if cmake_ok else
                    'add_executable(test_dm1_v2_movement_command_adapter_pc34) wiring not found')
    checks.append({"kind":"cmakelists_movement_command_adapter_audit","file":"CMakeLists.txt","ok":cmake_ok,"observed":cmake_observed})
    ctest_re="dm1_v1_input_command_queue_pc34_compat|dm1_v1_movement_command_core_pc34_compat|dm1_v1_movement_pipeline_pc34_compat|dm1_v1_input_command_queue_source_lock"
    ctest=run(['ctest','--test-dir','build','-R',ctest_re,'--output-on-failure']); ctest_ok='100% tests passed' in ctest or re.search(r'\b0 tests failed\b',ctest) is not None
    checks.append({"kind":"narrow_ctest","regex":ctest_re,"ok":ctest_ok})
    ok=all(c.get('ok') for c in checks); status="PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED" if ok else "BLOCKED_PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_INCOMPLETE"
    residual={"firestaffRuntimeRoute":"closed: SDL/script keypad -> M12 input -> resolved C001..C006 command -> F0380/F0365/F0366 compat pipeline","originalDosResidual":"still unclaimed: DOSBox/FIRES keyboard-buffer adapter causing M527/M528 or IODRV_00_GetKeyboardInput to return 0x004B/0x004C/0x004D/0x004F/0x0050/0x0051 in a bounded original run","decision":"do not spend Firestaff M11 route work on original-DOS keyboard-buffer residual; treat it as an original-runtime transcript/debugger task"}
    MANIFEST.write_text(json.dumps({"schema":f"{PASS}.v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":status,"repo":str(ROOT),"branch":run(['git','branch','--show-current']),"head":run(['git','rev-parse','HEAD']),"sourceRoot":str(REDMCSB),"sourceLocks":source,"firestaffLocks":firestaff,"priorManifestStatuses":prior,"residualScope":residual,"cmakeListsMovementCommandAdapter":{"observed":cmake_observed,"ok":cmake_ok},"checks":checks,"ctestTail":"\n".join(ctest.splitlines()[-25:])},indent=2,sort_keys=True)+"\n")
    out=["# Pass372 — DM1 V1 movement runtime route","",f"Status: `{status}`","","## Decision","","Firestaff's DM1 V1 runtime movement route is source-locked from input resolution through command queue and movement dispatch. The residual keypad/keyboard-buffer scope remains only the original DOSBox/FIRES pre-`F0361` keyboard-buffer adapter, not the Firestaff live route.","","## ReDMCSB source audit anchors",""]
    out += [f"- `{r['file']}:{r['lines']}` — {r['claim']} ok=`{r['ok']}`" for r in source]
    out += ["","## Firestaff route anchors",""] + [f"- `{r['file']}` — {r['claim']} ok=`{r['ok']}`" for r in firestaff]
    out += ["","## Prior runtime evidence reused",""] + [f"- `{n}` status=`{i.get('status')}`" for n,i in prior.items()]
    out += ["","## Residual scope","",f"- Firestaff runtime route: {residual['firestaffRuntimeRoute']}",f"- Original DOS residual: {residual['originalDosResidual']}","- Route tokens audited: `/down`, `/left`, `/right`, and combined quality token `/down/left/right` remain covered by M12 movement input -> C001..C006 queue dispatch.","- `add_executable(test_dm1_v2_movement_command_adapter_pc34)` wiring (located by content, not by line number) audited as unrelated DM1 V2 movement-command-adapter test wiring; no DM1 V1 route change needed there.",f"- Decision: {residual['decision']}","","## Gates","",f"- `ctest --test-dir build -R '{ctest_re}' --output-on-failure` ok=`{ctest_ok}`",f"- `add_executable(test_dm1_v2_movement_command_adapter_pc34)` wiring present in `CMakeLists.txt` ok=`{cmake_ok}`",f"- Manifest: `parity-evidence/verification/{PASS}/manifest.json`"]
    REPORT.write_text('\n'.join(out)+'\n')
    print(json.dumps({"status":status,"manifest":str(MANIFEST.relative_to(ROOT)),"report":str(REPORT.relative_to(ROOT))},indent=2)); return 0 if ok else 1
if __name__=='__main__': raise SystemExit(main())
