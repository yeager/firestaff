#!/usr/bin/env python3
from __future__ import annotations
import json
from datetime import datetime, timezone
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/pass504_dm1_v1_keyboard_buffer_state_delta_blocker"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass504_dm1_v1_keyboard_buffer_state_delta_blocker.md"
PASS386 = ROOT / "parity-evidence/verification/pass386_dm1_v1_keyboard_vs_click_command_dispatch/manifest.json"
PASS498 = ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json"
STATUS = "PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER_LOCKED"
SOURCE_LOCKS = [
 ("IO2.C","F0540_INPUT_Crawcin","27-61",["L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();","switch (L2944_ui_ - 0x1248)","L2944_ui_ = 'L';","L2944_ui_ = 'P';","L2944_ui_ = 'K';","L2944_ui_ = 'M';","return L2944_ui_;"],"PC-34 keyboard evidence starts with an actual Crawcin value after IO2 normalization, not a route-label transcript."),
 ("COMMAND.C","G0459_as_Graphic561_SecondaryKeyboardInput_Movement","677-684",["{ C001_COMMAND_TURN_LEFT,     0x004B }","{ C003_COMMAND_MOVE_FORWARD,  0x004C }","{ C002_COMMAND_TURN_RIGHT,    0x004D }","{ C006_COMMAND_MOVE_LEFT,     0x004F }","{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }","{ C004_COMMAND_MOVE_RIGHT,    0x0051 }"],"F0361 can only queue movement when the drained keyboard code matches the PC-34 secondary keyboard table."),
 ("COMMAND.C","F0361_COMMAND_ProcessKeyPress","1709-1813",["if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)","G0435_B_CommandQueueLocked = C1_TRUE;","if (G2153_i_QueuedCommandsCount < C5_UNKNOWN)","while (L1111_i_Command = L1112_ps_KeyboardInput->Command)","if (P0728_KeyCode == L1112_ps_KeyboardInput->Code)","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;","G2153_i_QueuedCommandsCount++;","if ((L1112_ps_KeyboardInput = G0444_ps_SecondaryKeyboardInput) == NULL)","goto T0361xxx;","G0435_B_CommandQueueLocked = C0_FALSE;","F0360_COMMAND_ProcessPendingClick();"],"A keyboard transcript must prove a matching F0361 queue write and G2153 increment; F0361 entry alone is not enough."),
 ("COMMAND.C","F0380_COMMAND_ProcessQueue_CPSC","2045-2156",["if (G2153_i_QueuedCommandsCount == 0)","L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT) && (G0310_i_DisabledMovementTicks","G2153_i_QueuedCommandsCount--;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],"The state delta starts only after F0380 proves non-empty pop/decrement and dispatches to F0365/F0366."),
 ("GAMELOOP.C","F0002_MAIN_GameLoop_CPSDF","164-219",["G0321_B_StopWaitingForPlayerInput = C0_FALSE;","while (M527_IsCharacterInKeyboardBuffer())","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"],"The runtime chain is keyboard-buffer drain, F0361 queueing, F0380 processing, then wait-loop exit."),
]
LOCAL_LOCKS=[("dm1_v1_input_command_queue_pc34_compat.c","case 0xAB35: case 0x9B41: case 0x9B54: case 0x004C:"),("dm1_v1_input_command_queue_pc34_compat.c","result.movementDisabledGate = 1;"),("test_dm1_v1_input_command_queue_pc34_compat.c","pc34 table up arrow moves forward"),("test_dm1_v1_input_command_queue_pc34_compat.c","redmcsb queue holds five commands"),("test_dm1_v1_input_command_queue_pc34_compat.c","projectile matching direction keeps command queued")]
def read(path,enc="utf-8"):
 if not path.exists(): raise AssertionError(f"missing required file: {path}")
 return path.read_text(encoding=enc,errors="replace")
def compact(s): return " ".join(s.split())
def req_order(text,needles):
 flat=compact(text); pos=-1; missing=[]
 for n in needles:
  hit=flat.find(compact(n),pos+1)
  if hit<0: missing.append(n)
  else: pos=hit
 return missing
def main():
 p386=json.loads(read(PASS386)); p498=json.loads(read(PASS498)); preds=p386.get("proofPredicates",{})
 source=[]
 for f,fn,lines,needles,claim in SOURCE_LOCKS:
  miss=req_order(read(RED/f,"latin-1"),needles)
  source.append({"file":f,"function":fn,"lines":lines,"claim":claim,"ok":not miss,"missing":miss,"path":str(RED/f)})
 local=[{"file":rel,"needle":n,"ok":n in read(ROOT/rel)} for rel,n in LOCAL_LOCKS]
 problems=[f"source audit failed: {r['file']} {r['function']}" for r in source if not r["ok"]]
 problems += [f"local audit failed: {r['file']} missing {r['needle']}" for r in local if not r["ok"]]
 for k,v in {"keyboardRouteRanAfterArm":True,"keyboardF0361Hit":True,"keyboardQueueCountChanged":False,"keyboardDispatchReached":False}.items():
  if preds.get(k) is not v: problems.append(f"pass386 predicate {k}={preds.get(k)!r}, expected {v!r}")
 if p498.get("status")!="PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA": problems.append(f"pass498 not at state-delta blocker: {p498.get('status')!r}")
 promotion=["capture/log the concrete M528_GetCharacterInKeyboardBuffer value after M527 reports non-empty","prove that value matches COMMAND.C G0459 PC-34 table row before F0361 exits","prove F0361 writes G0432 and increments G2153_i_QueuedCommandsCount","prove the next F0380 sees non-zero G2153, loads the same command, decrements G2153, then dispatches F0365/F0366","only then bind the following F0128/F0097 frame to the changed direction/X/Y state"]
 reject=["route keylog labels without M528/F0361 key-code value","F0361 entry stop with no G0432 write and no G2153 increment","F0380 stop where G2153_i_QueuedCommandsCount is zero or movement-disabled gate bypasses dispatch","F0128/F0097 frame evidence without the preceding F0380 pop/decrement/dispatch chain"]
 manifest={"schema":"firestaff.parity.pass504_dm1_v1_keyboard_buffer_state_delta_blocker.v1","timestampUtc":datetime.now(timezone.utc).isoformat(),"status":STATUS if not problems else "FAIL_PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER","ok":not problems,"sourceRoot":str(RED),"inputs":{"pass386":str(PASS386.relative_to(ROOT)),"pass498":str(PASS498.relative_to(ROOT))},"sourceAudit":source,"localAudit":local,"pass386Predicates":preds,"narrowedBlocker":"Keyboard-buffer transcripts are still non-promotable until they prove M528 key extraction, F0361 table match plus G0432/G2153 queue write, and the following F0380 pop/decrement/dispatch before the post-command F0128/F0097 state-delta frame.","promotionPredicate":promotion,"rejectAsNonPromotable":reject,"nonClaims":["no original runtime capture was run","no viewport/pixel parity promotion","no claim that F0365/F0366 source handlers are defective"],"problems":problems}
 OUT_DIR.mkdir(parents=True,exist_ok=True); MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n")
 lines=["# Pass504 - DM1 V1 keyboard-buffer state-delta blocker","","Status: "+manifest["status"],"","## Decision","",manifest["narrowedBlocker"],"","## ReDMCSB source audit",""]
 lines += [f"- {r['file']}:{r['lines']} / {r['function']} - {r['claim']} ok={r['ok']}" for r in source]
 lines += ["","## Promotion predicate",""]+["- "+x for x in promotion]+["","## Reject as non-promotable",""]+["- "+x for x in reject]+["","## Inputs","",f"- pass386 predicates: keyboard F0361 hit={preds.get('keyboardF0361Hit')}, queue count changed={preds.get('keyboardQueueCountChanged')}, dispatch reached={preds.get('keyboardDispatchReached')}",f"- pass498 status: {p498.get('status')}","","## Gate","","- python3 tools/verify_pass504_dm1_v1_keyboard_buffer_state_delta_blocker.py","","Manifest: parity-evidence/verification/pass504_dm1_v1_keyboard_buffer_state_delta_blocker/manifest.json"]
 REPORT.write_text("\n".join(lines)+"\n")
 print(manifest["status"]); print(f"manifest={MANIFEST.relative_to(ROOT)}"); print(f"report={REPORT.relative_to(ROOT)}")
 if problems:
  print("\n".join(problems)); return 1
 return 0
if __name__=="__main__": raise SystemExit(main())
