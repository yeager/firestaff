#!/usr/bin/env python3
import json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass331_dm1_v1_route_to_viewport_redraw_path"
MAN=ROOT/"parity-evidence/verification"/PASS/"manifest.json"
ALLOWED={"PASS_ROUTE_REACHES_F0128_REDRAW","PASS_ROUTE_REACHES_F0128_F0097_SEQUENCE","BLOCKED_PASS331_ROUTE_KEYS_NOT_COMMAND_QUEUE","BLOCKED_PASS331_ROUTE_STUCK_OUTSIDE_DUNGEON","BLOCKED_PASS331_COMMAND_NO_REDRAW_DIRTY_FLAG","BLOCKED_PASS331_REDRAW_DIFFERENT_ADDRESS_THAN_F0128"}
err=[]
if not MAN.exists(): err.append("missing manifest")
else:
 data=json.loads(MAN.read_text())
 if data.get("status") not in ALLOWED: err.append("bad status "+str(data.get("status")))
 if data.get("hypothesis") != "route keys are delivered but not mapped to DM command queue": err.append("wrong hypothesis")
 addr=data.get("addresses",{})
 for k,v in {"F0361_COMMAND_ProcessKeyPress":"22F4:0407","F0380_COMMAND_ProcessQueue_CPSC":"22F4:0699","F0365_COMMAND_ProcessTypes1To2_TurnParty":"1EA4:010D","F0366_COMMAND_ProcessTypes3To6_MoveParty":"1EA4:01AA","F0267_MOVE_GetMoveResult_CPSCE":"1859:0516","F0128_DUNGEONVIEW_Draw_CPSF":"23AD:40FE","F0097_DUNGEONVIEW_DrawViewport":"2809:1EFF"}.items():
  if addr.get(k)!=v: err.append(f"address {k} not locked")
 if data.get("notPromotedBy") != ["BPLIST","BP command echo","tmux/capture-pane"]: err.append("promotion guard missing")
 if not all(r.get("ok") for r in data.get("sourceAudit",[])): err.append("source audit failed")
 rt=data.get("runtimeProbe",{})
 if rt.get("boundedSecondsPerProbe",999)>75: err.append("runtime bound exceeded")
 probes=[p for p in rt.get("probes",[]) if p]
 if len(probes)>2: err.append("too many probes")
 if rt.get("ran"):
  if not probes: err.append("ran without probe")
  else:
   p=probes[0]
   if len(p.get("routeLog",[])) < 5: err.append("route not delivered")
   if data.get("status") != p.get("status"): err.append("manifest/probe status mismatch")
   labels=[s.get("label") for s in p.get("stops",[])]
   if data.get("status")=="PASS_ROUTE_REACHES_F0128_REDRAW" and "F0128_DUNGEONVIEW_Draw_CPSF" not in labels: err.append("PASS without F0128")
   if data.get("status")=="PASS_ROUTE_REACHES_F0128_F0097_SEQUENCE" and "F0097_DUNGEONVIEW_DrawViewport" not in labels: err.append("PASS sequence without F0097")
   if data.get("status")=="BLOCKED_PASS331_COMMAND_NO_REDRAW_DIRTY_FLAG" and "F0267_MOVE_GetMoveResult_CPSCE" not in labels: err.append("dirty-flag blocker without move-result")
if err:
 print("FAIL", "; ".join(err)); sys.exit(1)
print("pass331 verifier OK")
