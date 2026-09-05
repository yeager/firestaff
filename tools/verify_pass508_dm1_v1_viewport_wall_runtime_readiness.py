#!/usr/bin/env python3
"""Audit current DM1 PC3.4 wall readiness without external work trees."""
from __future__ import annotations
import hashlib, json, re, struct, zlib
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZIP_STORED, ZipFile

ROOT = Path(__file__).resolve().parents[1]
RED = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"
ZIP = Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
VIEW = ROOT / "src/engine/m11_game_view.c"
OUT = ROOT / "parity-evidence/verification/pass508_dm1_v1_viewport_wall_runtime_readiness/manifest.json"
REPORT = ROOT / "parity-evidence/pass508_dm1_v1_viewport_wall_runtime_readiness.md"
STATUS = "PASS_PASS508_DM1_V1_VIEWPORT_WALL_RUNTIME_READINESS"
SOURCES = [
 ("DUNVIEW.C","8466-8542",["F0116_DUNGEONVIEW_DrawSquareD3L","F0118_DUNGEONVIEW_DrawSquareD3C_CPSF","F0121_DUNGEONVIEW_DrawSquareD2C","F0124_DUNGEONVIEW_DrawSquareD1C","F0127_DUNGEONVIEW_DrawSquareD0C"]),
 ("DUNVIEW.C","7784-7844",["case C00_ELEMENT_WALL:","C712_ZONE_WALL_D1C","F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF","C0x0000_CELL_ORDER_ALCOVE"]),
 ("DUNVIEW.C","7873-7938",["case C17_ELEMENT_DOOR_FRONT:","C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT","F0111_DUNGEONVIEW_DrawDoor","C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"]),
 ("DUNVIEW.C","5915-5933",["/* Draw explosions */","P0141_T_Thing = L0146_T_FirstThingToDraw"]),
 ("DRAWVIEW.C","847-858",["F0638_GetZone(C007_ZONE_VIEWPORT","VIDRV_09_BlitViewPort"]),
]
MEMBERS = {
 "DATA/DUNGEON.DAT":(33357,"d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"),
 "DATA/GRAPHICS.DAT":(363417,"2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"),
 "DM.EXE":(11471,"4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4")}
CALLBACKS = ["M11_DM1_F0128_EXECUTE_WALL_MATERIAL","M11_DM1_F0128_EXECUTE_WALL_ORNAMENT","M11_DM1_F0128_EXECUTE_PRE_DOOR_FLOOR_ORNAMENT","M11_DM1_F0128_EXECUTE_DOOR_PASS1","M11_DM1_F0128_EXECUTE_DOOR_FRAME","M11_DM1_F0128_EXECUTE_DOOR_BUTTON","M11_DM1_F0128_EXECUTE_DOOR_MATERIAL","M11_DM1_F0128_EXECUTE_FOREGROUND","m11_draw_dm1_f0115_explosions_for_square"]

def body(text,name):
 m=re.search(r"\b"+re.escape(name)+r"\s*\(",text)
 if not m: raise AssertionError("missing function "+name)
 b=text.find("{",m.end()); depth=0
 for i in range(b,len(text)):
  depth += (text[i]=="{")-(text[i]=="}")
  if depth==0: return text[m.start():i+1]
 raise AssertionError("unterminated function "+name)

def member(name):
 raw=ZIP.read_bytes()
 with ZipFile(ZIP) as archive: info=archive.getinfo(name)
 off=info.header_offset
 if raw[off:off+4] != b"PK\x03\x04": raise AssertionError("bad local header "+name)
 nl,xl=struct.unpack_from("<HH",raw,off+26); start=off+30+nl+xl
 packed=raw[start:start+info.compress_size]
 if info.compress_type==ZIP_STORED: data=packed
 elif info.compress_type==ZIP_DEFLATED: data=zlib.decompress(packed,-15)
 else: raise AssertionError("unsupported compression "+name)
 if len(data)!=info.file_size or zlib.crc32(data)&0xffffffff!=info.CRC: raise AssertionError("integrity failure "+name)
 return data

def main():
 failures=[]; source=[]
 for filename,span,needles in SOURCES:
  path=RED/filename; a,b=map(int,span.split("-")); text="\n".join(path.read_text(encoding="latin-1").splitlines()[a-1:b]) if path.exists() else ""
  missing=[n for n in needles if n not in text]; source.append({"file":filename,"lines":span,"ok":not missing,"missing":missing}); failures += [f"source:{filename}:{n}" for n in missing]
 view=VIEW.read_text(encoding="utf-8"); callback=body(view,"m11_dm1_f0128_execute_source_step"); viewport=body(view,"m11_draw_viewport")
 missing=[n for n in CALLBACKS if n not in callback]; failures += ["callback:"+n for n in missing]
 per_square=all(n in viewport for n in ["m11_dm1_f0128_dispatch_wall_material_square","m11_dm1_f0128_dispatch_foreground_square"])
 stale="m11_draw_dm1_deferred_explosion_pass(state" in viewport
 if not per_square: failures.append("viewport:missing-per-square-dispatch")
 if stale: failures.append("viewport:stale-global-explosion-replay")
 media=[]
 if not ZIP.exists(): failures.append("media:missing-pc34-zip")
 else:
  for name,(size,want) in MEMBERS.items():
   data=member(name); digest=hashlib.sha256(data).hexdigest(); ok=len(data)==size and digest==want
   media.append({"archive":str(ZIP),"member":name,"readMode":"in-memory/no-extraction","size":len(data),"sha256":digest,"ok":ok})
   if not ok: failures.append("media:"+name)
 blockers=["No same-frame original DOS viewport capture is supplied by this readiness gate.","No Firestaff-versus-original pixel equality or timing parity is claimed.","MEDIA720 D3L2/D3R2 F0107 coordinate rows remain fail-closed pending authentic item-558 or executable evidence."]
 payload={"schema":"pass508_dm1_v1_viewport_wall_runtime_readiness.v2","status":STATUS if not failures else "FAIL_PASS508_DM1_V1_VIEWPORT_WALL_RUNTIME_READINESS","ok":not failures,"redmcsbSourceRoot":str(RED),"sourceAudit":source,"firestaffAudit":{"callback":"m11_dm1_f0128_execute_source_step","missingMarkers":missing,"perSquareDispatch":per_square,"staleBroadExplosionReplay":stale},"originalMedia":media,"nonClaims":blockers,"failures":failures}
 OUT.parent.mkdir(parents=True,exist_ok=True); OUT.write_text(json.dumps(payload,indent=2,sort_keys=True)+"\n",encoding="utf-8")
 lines=["# Pass508 DM1 V1 viewport/wall runtime-readiness evidence","","Status: "+payload["status"],"","## ReDMCSB anchors",""]+[f"- {'PASS' if r['ok'] else 'FAIL'} {r['file']}:{r['lines']}" for r in source]+["","## Current Firestaff callback audit","",f"- {'PASS' if not missing else 'FAIL'} wall, door, and F0115 families are present in `m11_dm1_f0128_execute_source_step`.",f"- {'PASS' if per_square else 'FAIL'} `m11_draw_viewport` uses per-square wall and foreground dispatch.",f"- {'PASS' if not stale else 'FAIL'} no active once-per-frame D3--D1 explosion replay remains.","","## Original PC 3.4 members",""]+[f"- {'PASS' if r['ok'] else 'FAIL'} {r['archive']}::{r['member']} size={r['size']} sha256={r['sha256']} ({r['readMode']})" for r in media]+["","## Non-claims and remaining evidence",""]+["- "+x for x in blockers]+[""]
 REPORT.write_text("\n".join(lines),encoding="utf-8"); print(payload["status"]); print("- wrote "+str(REPORT.relative_to(ROOT))); print("- wrote "+str(OUT.relative_to(ROOT))); return 0 if not failures else 1

if __name__=="__main__": raise SystemExit(main())
