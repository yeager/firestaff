#!/usr/bin/env python3
import json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass335_dm1_v1_keyboard_table_route_readiness"
MAN=ROOT/"parity-evidence/verification"/PASS/"manifest.json"
ALLOWED={
 "PASS_DUNGEON_KEY_TABLE_READY_BEFORE_ROUTE",
 "PASS_ROUTE_REACHES_COMMAND_QUEUE_AFTER_TABLE_READY",
 "BLOCKED_PASS335_ROUTE_BEFORE_DUNGEON_KEY_TABLE",
 "BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG",
 "BLOCKED_PASS335_KEYBOARD_TABLE_ADDRESS_REBIND_REQUIRED",
}
err=[]
if not MAN.exists(): err.append("missing manifest")
else:
 data=json.loads(MAN.read_text())
 status=data.get("status")
 if status not in ALLOWED: err.append("bad status "+str(status))
 if data.get("hypothesesDistinguished") != ["ROUTE_BEFORE_DUNGEON_KEY_TABLE","TABLE_READY_BUT_KEYPAD_CODES_WRONG","ADDRESS_BINDING_REQUIRED"]: err.append("hypothesis discriminator missing")
 if data.get("notPromotedBy") != ["BPLIST","BP command echo","tmux/capture-pane"]: err.append("promotion guard missing")
 addr=data.get("addresses",{})
 expected={
  "G0444_ps_SecondaryKeyboardInput":"2C23:3EC0",
  "G0459_as_Graphic561_SecondaryKeyboardInput_Movement":"2C23:26F4",
  "G0458_as_Graphic561_PrimaryKeyboardInput_Interface":"2C23:26D4",
  "G2153_i_QueuedCommandsCount":"2C23:3E78",
  "F0361_COMMAND_ProcessKeyPress":"22F4:0407",
  "F0380_COMMAND_ProcessQueue_CPSC":"22F4:0699",
  "F0128_DUNGEONVIEW_Draw_CPSF":"23AD:40FE",
 }
 for k,v in expected.items():
  if addr.get(k)!=v: err.append(f"address {k} not locked")
 audits=data.get("sourceAudit",[])
 if len(audits) < 5 or not all(r.get("ok") for r in audits): err.append("source audit failed")
 flat={n for r in audits for n in r.get("anchors",{})}
 for needle in [
  "G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
  "C001_COMMAND_TURN_LEFT,     0x004B",
  "C003_COMMAND_MOVE_FORWARD,  0x004C",
  "C002_COMMAND_TURN_RIGHT,    0x004D",
  "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
  "F0380_COMMAND_ProcessQueue_CPSC();",
 ]:
  if needle not in flat: err.append("missing source anchor "+needle)
 rt=data.get("runtimeProbe",{})
 if rt.get("boundedSecondsPerProbe",999)>75: err.append("runtime bound exceeded")
 probes=[p for p in rt.get("probes",[]) if p]
 if len(probes)>2: err.append("too many probes")
 if rt.get("ran"):
  if not probes: err.append("ran without probe")
  else:
   p=probes[0]
   if p.get("status") != status: err.append("manifest/probe status mismatch")
   if p.get("boundedSeconds", 999)>75: err.append("probe bound exceeded")
   if len(p.get("routeLog",[])) < 8: err.append("route not delivered to readiness point")
   ptr=p.get("pointerCheck",{})
   if status in {"PASS_DUNGEON_KEY_TABLE_READY_BEFORE_ROUTE","PASS_ROUTE_REACHES_COMMAND_QUEUE_AFTER_TABLE_READY","BLOCKED_PASS335_TABLE_READY_BUT_KEYPAD_CODES_WRONG"}:
    if not ptr.get("ok"): err.append("table-ready status without pointer read")
    if ptr.get("nearOffsetHex") != "26F4": err.append("table-ready status without movement table offset")
   if status=="BLOCKED_PASS335_ROUTE_BEFORE_DUNGEON_KEY_TABLE" and ptr.get("nearOffsetHex") == "26F4": err.append("route-before status despite ready table")
   labels=[s.get("label") for s in p.get("stops",[])]
   if status=="PASS_ROUTE_REACHES_COMMAND_QUEUE_AFTER_TABLE_READY" and not (set(labels)&{"F0380_COMMAND_ProcessQueue_CPSC","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty","F0267_MOVE_GetMoveResult_CPSCE","F0128_DUNGEONVIEW_Draw_CPSF","F0097_DUNGEONVIEW_DrawViewport"}): err.append("queue pass without queue/consumer stop")
if err:
 print("FAIL", "; ".join(err)); sys.exit(1)
print("pass335 verifier OK")
