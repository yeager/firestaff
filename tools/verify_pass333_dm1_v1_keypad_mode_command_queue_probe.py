#!/usr/bin/env python3
import json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass333_dm1_v1_keypad_mode_command_queue_probe"
MAN=ROOT/"parity-evidence/verification"/PASS/"manifest.json"
ALLOWED={
 "PASS_KEYCODE_REACHES_COMMAND_QUEUE",
 "PASS_KEYCODE_REACHES_F0128_F0097_SEQUENCE",
 "BLOCKED_PASS333_XDOTOOL_KEYPAD_WRONG_SCANCODE",
 "BLOCKED_PASS333_NUMLOCK_KEYPAD_MODE_BLOCKS_I34E",
 "BLOCKED_PASS333_ROUTE_BEFORE_DUNGEON_KEY_TABLE",
 "BLOCKED_PASS333_COMMAND_QUEUE_ADDRESS_REBIND_REQUIRED",
}
err=[]
if not MAN.exists(): err.append("missing manifest")
else:
 data=json.loads(MAN.read_text())
 if data.get("status") not in ALLOWED: err.append("bad status "+str(data.get("status")))
 if data.get("hypothesis") != "NumLock/keypad mode prevents I34E movement codes from entering the command queue": err.append("wrong hypothesis")
 if data.get("probeKeyMode") != "Num_Lock then KP_5/KP_4/KP_6 (numeric keypad symbols)": err.append("wrong probe key mode")
 addr=data.get("addresses",{})
 for k,v in {"F0361_COMMAND_ProcessKeyPress":"22F4:0407","F0380_COMMAND_ProcessQueue_CPSC":"22F4:0699","F0365_COMMAND_ProcessTypes1To2_TurnParty":"1EA4:010D","F0366_COMMAND_ProcessTypes3To6_MoveParty":"1EA4:01AA","F0267_MOVE_GetMoveResult_CPSCE":"1859:0516","F0128_DUNGEONVIEW_Draw_CPSF":"23AD:40FE","F0097_DUNGEONVIEW_DrawViewport":"2809:1EFF"}.items():
  if addr.get(k)!=v: err.append(f"address {k} not locked")
 if data.get("notPromotedBy") != ["BPLIST","BP command echo","tmux/capture-pane"]: err.append("promotion guard missing")
 audits=data.get("sourceAudit",[])
 if not all(r.get("ok") for r in audits): err.append("source audit failed")
 cmd_audit=next((r for r in audits if r.get("file")=="COMMAND.C"), {})
 anchors=cmd_audit.get("anchors",{})
 for needle in ["MEDIA707_I34E_I34M","C001_COMMAND_TURN_LEFT,     0x004B","C003_COMMAND_MOVE_FORWARD,  0x004C","C002_COMMAND_TURN_RIGHT,    0x004D","G2153_i_QueuedCommandsCount++"]:
  if needle not in anchors: err.append("missing I34E/queue anchor "+needle)
 rt=data.get("runtimeProbe",{})
 if rt.get("boundedSecondsPerProbe",999)>75: err.append("runtime bound exceeded")
 probes=[p for p in rt.get("probes",[]) if p]
 if len(probes)>2: err.append("too many probes")
 if rt.get("ran"):
  if not probes: err.append("ran without probe")
  else:
   p=probes[0]
   if len(p.get("routeLog",[])) < 5: err.append("route not delivered")
   if not any(r.get("route_item")=="numlock" for r in p.get("routeLog",[])): err.append("Num_Lock not delivered")
   if data.get("status") != p.get("status"): err.append("manifest/probe status mismatch")
   labels=[s.get("label") for s in p.get("stops",[])]
   if data.get("status")=="PASS_KEYCODE_REACHES_COMMAND_QUEUE" and not (set(labels)&{"F0380_COMMAND_ProcessQueue_CPSC","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty","F0267_MOVE_GetMoveResult_CPSCE","F0128_DUNGEONVIEW_Draw_CPSF"}): err.append("PASS queue without command/consumer stop")
   if data.get("status")=="PASS_KEYCODE_REACHES_F0128_F0097_SEQUENCE" and "F0097_DUNGEONVIEW_DrawViewport" not in labels: err.append("PASS sequence without F0097")
if err:
 print("FAIL", "; ".join(err)); sys.exit(1)
print("pass333 verifier OK")
