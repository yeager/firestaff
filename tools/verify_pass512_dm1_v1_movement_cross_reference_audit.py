#!/usr/bin/env python3
"""Audit DM1 PC 3.4 movement lineage from repository source and real media."""
from __future__ import annotations
import hashlib, json, os, struct, zlib, zipfile
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass512_dm1_v1_movement_cross_reference_audit"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"
DM1_ZIP = Path(os.environ.get("FIRESTAFF_DM1_PC34_ZIP", str(Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip")))
ZIP_DISPLAY = "~/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
STATUS = "BLOCKED_PASS512_DM1_V1_MOVEMENT_RUNTIME_CAPTURE_MISSING"

SOURCE_LOCKS: list[dict[str, Any]] = [
 {"id":"game-loop-input-to-command","file":"GAMELOOP.C","lines":"164-219","claim":"keyboard input is drained through F0361 before F0380 processes the queue","needles":["G0321_B_StopWaitingForPlayerInput = C0_FALSE;","while (M527_IsCharacterInKeyboardBuffer())","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"]},
 {"id":"command-table-and-dispatch","file":"COMMAND.C","lines":"396-405,636-685,2045-2156","claim":"movement tables map C001..C006 and F0380 dispatches after cooldown filtering","needles":["C001_COMMAND_TURN_LEFT","C003_COMMAND_MOVE_FORWARD","C002_COMMAND_TURN_RIGHT","C006_COMMAND_MOVE_LEFT","C005_COMMAND_MOVE_BACKWARD","C004_COMMAND_MOVE_RIGHT","G0310_i_DisabledMovementTicks","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]},
 {"id":"turn-step-collision-timing","file":"CLIKMENU.C","lines":"142-347","claim":"F0365/F0366 own turning, relative stepping, collision, sensors and cooldown","needles":["F0365_COMMAND_ProcessTypes1To2_TurnParty","F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE","F0366_COMMAND_ProcessTypes3To6_MoveParty","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0357_COMMAND_DiscardAllInput();","F0267_MOVE_GetMoveResult_CPSCE","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"]},
 {"id":"coordinate-sensor-commit","file":"MOVESENS.C","lines":"438-497,760-818,1553-1794","claim":"F0267 commits party coordinates and source-before-destination sensors","needles":["G0306_i_PartyMapX = P0560_i_DestinationMapX;","G0307_i_PartyMapY = P0561_i_DestinationMapY;","G0362_l_LastPartyMovementTime = G0313_ul_GameTime;","F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX","F0175_GROUP_GetThing(G0306_i_PartyMapX, G0307_i_PartyMapY)","F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX","L0768_B_TriggerSensor = (L0779_i_SensorData == M000_INDEX_TO_ORDINAL(G0308_i_PartyDirection));"]},
 {"id":"movement-to-view-tuple","file":"DUNVIEW.C","lines":"8318-8338,8468-8542","claim":"F0128 consumes direction/mapX/mapY and derives visible squares with F0150","needles":["void F0128_DUNGEONVIEW_Draw_CPSF","REGISTER int16_t P0183_i_Direction","REGISTER int16_t P0184_i_MapX","REGISTER int16_t P0185_i_MapY","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1","F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);"]},
]
EXPECTED_HASHES={"DATA/DUNGEON.DAT":"d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85","DATA/GRAPHICS.DAT":"2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e","TITLE":"adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745"}

def norm(text:str)->str: return " ".join(text.split())
def source_block(name:str,ranges:str)->str:
 path=RED/name
 if not path.is_file(): raise AssertionError(f"missing repository ReDMCSB source: {path}")
 lines=path.read_text(encoding="latin-1",errors="replace").splitlines(); out=[]
 for span in ranges.split(","):
  start,end=(int(v) for v in span.split("-",1))
  if start<1 or end>len(lines): raise AssertionError(f"{path}:{span} outside file length {len(lines)}")
  out.extend(lines[start-1:end])
 return "\n".join(out)
def verify_sources()->list[dict[str,Any]]:
 rows=[]
 for item in SOURCE_LOCKS:
  block=norm(source_block(item["file"],item["lines"])); missing=[n for n in item["needles"] if norm(n) not in block]
  if missing: raise AssertionError(f"{item['file']}:{item['lines']} missing {missing}")
  rows.append({"id":item["id"],"path":f"reference/redmcsb-20210206/Toolchains/Common/Source/{item['file']}","lines":item["lines"],"claim":item["claim"]})
 return rows
def zip_member_bytes(raw:bytes,info:zipfile.ZipInfo)->bytes:
 fields=struct.unpack("<IHHHHHIIIHH",raw[info.header_offset:info.header_offset+30])
 if fields[0]!=0x04034B50: raise AssertionError(f"{info.filename}: invalid local ZIP header")
 start=info.header_offset+30+fields[9]+fields[10]; compressed=raw[start:start+info.compress_size]
 if info.compress_type==zipfile.ZIP_STORED: payload=compressed
 elif info.compress_type==zipfile.ZIP_DEFLATED: payload=zlib.decompress(compressed,-15)
 else: raise AssertionError(f"{info.filename}: unsupported ZIP method {info.compress_type}")
 if len(payload)!=info.file_size: raise AssertionError(f"{info.filename}: decoded size mismatch")
 return payload
def verify_originals()->list[dict[str,Any]]:
 if not DM1_ZIP.is_file(): raise AssertionError(f"missing authentic PC 3.4 archive: {DM1_ZIP}")
 raw=DM1_ZIP.read_bytes(); rows=[]
 with zipfile.ZipFile(DM1_ZIP) as archive:
  infos={i.filename.replace("\\","/"):i for i in archive.infolist()}
  for name,expected in EXPECTED_HASHES.items():
   if name not in infos: raise AssertionError(f"authentic archive missing {name}")
   payload=zip_member_bytes(raw,infos[name]); got=hashlib.sha256(payload).hexdigest()
   if got!=expected: raise AssertionError(f"{name} sha256 {got} != {expected}")
   rows.append({"archive":ZIP_DISPLAY,"member":name,"sha256":got,"size":len(payload)})
 return rows
def write_report(m:dict[str,Any])->None:
 lines=["# Pass512 - DM1 V1 movement cross-reference audit","",f"Status: {m['status']}","","Scope: DM1 V1 movement only. This is evidence, not a runtime behavior change.","","## Repository ReDMCSB locks",""]
 lines += [f"- PASS {r['path']}:{r['lines']} - {r['claim']}" for r in m["sourceAudit"]]
 lines += ["","## Authentic PC 3.4 data anchors",""]+[f"- PASS {r['archive']}::{r['member']} sha256 {r['sha256']}" for r in m["originalAnchors"]]
 lines += ["","## Current blocker","",m["remainingBlocker"],"","The old Greatstone, CSBWin, CSB clone, extracted-media, and prior-generated-manifest dependencies are not movement truth owners and are no longer prerequisites for this gate.","","Not claimed: pixel parity, viewport rendering changes, a binary-level F0380 body breakpoint, or route promotion from source/media identity alone.",""]
 REPORT.write_text("\n".join(lines),encoding="utf-8")
def main()->int:
 OUT_DIR.mkdir(parents=True,exist_ok=True)
 m={"schema":f"firestaff.parity.{PASS}.v2","status":STATUS,"scope":"DM1 V1 movement input-to-command-to-movement-to-view tuple evidence","redmcsbRoot":"reference/redmcsb-20210206/Toolchains/Common/Source","sourceAudit":verify_sources(),"originalAnchors":verify_originals(),"remainingBlocker":"Authentic original-runtime evidence is still required: a non-static PC/I34E keyboard-buffer-to-F0380 route transcript and representative movement/HUD/viewport captures tied to before/after party tuples.","removedStaleDependencies":["Greatstone cache","CSBWin clone","CSB clone","extracted original-media tree","prior generated gate manifests"],"notClaimed":["pixel parity","viewport rendering edits","binary-level direct F0380 body breakpoint","route promotion from source/media identity"]}
 MANIFEST.write_text(json.dumps(m,indent=2,sort_keys=True)+"\n",encoding="utf-8"); write_report(m)
 print(json.dumps({"status":m["status"],"manifest":str(MANIFEST.relative_to(ROOT)),"report":str(REPORT.relative_to(ROOT)),"sourceChecks":len(m["sourceAudit"]),"authenticMembers":len(m["originalAnchors"])},indent=2,sort_keys=True)); return 0
if __name__=="__main__": raise SystemExit(main())
