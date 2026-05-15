#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, re, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
DUNVIEW = RED / "DUNVIEW.C"
LOCAL = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
MANIFEST = ROOT / "parity-evidence/verification/pass557_dm1_v1_viewport_f0128_draw_order_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass557_dm1_v1_viewport_f0128_draw_order_source_lock.md"
CALL_TO_SQUARE={"F0676_DrawD3L2":"DM1_VIEW_SQUARE_D3L2","F0677_DrawD3R2":"DM1_VIEW_SQUARE_D3R2","F0678_DrawD2L2":"DM1_VIEW_SQUARE_D2L2","F0679_DrawD2R2":"DM1_VIEW_SQUARE_D2R2","F0116_DUNGEONVIEW_DrawSquareD3L":"DM1_VIEW_SQUARE_D3L","F0117_DUNGEONVIEW_DrawSquareD3R":"DM1_VIEW_SQUARE_D3R","F0118_DUNGEONVIEW_DrawSquareD3C_CPSF":"DM1_VIEW_SQUARE_D3C","F0119_DUNGEONVIEW_DrawSquareD2L":"DM1_VIEW_SQUARE_D2L","F0120_DUNGEONVIEW_DrawSquareD2R_CPSF":"DM1_VIEW_SQUARE_D2R","F0121_DUNGEONVIEW_DrawSquareD2C":"DM1_VIEW_SQUARE_D2C","F0122_DUNGEONVIEW_DrawSquareD1L":"DM1_VIEW_SQUARE_D1L","F0123_DUNGEONVIEW_DrawSquareD1R":"DM1_VIEW_SQUARE_D1R","F0124_DUNGEONVIEW_DrawSquareD1C":"DM1_VIEW_SQUARE_D1C","F0125_DUNGEONVIEW_DrawSquareD0L":"DM1_VIEW_SQUARE_D0L","F0126_DUNGEONVIEW_DrawSquareD0R":"DM1_VIEW_SQUARE_D0R","F0127_DUNGEONVIEW_DrawSquareD0C":"DM1_VIEW_SQUARE_D0C"}
MACRO_TO_SQUARE={"M598_VIEW_SQUARE_D4L":"DM1_VIEW_SQUARE_D4L","M599_VIEW_SQUARE_D4R":"DM1_VIEW_SQUARE_D4R","M597_VIEW_SQUARE_D4C":"DM1_VIEW_SQUARE_D4C"}
def sha256(path):
 h=hashlib.sha256()
 with path.open("rb") as f:
  for b in iter(lambda:f.read(1048576), b""): h.update(b)
 return h.hexdigest()
def line_number(text, needle):
 for i,l in enumerate(text.splitlines(),1):
  if needle in l: return i
 raise AssertionError(f"missing source needle: {needle}")
def parse_redmcsb_f0128():
 text=DUNVIEW.read_text(encoding="utf-8",errors="replace"); lines=text.splitlines()
 start=line_number(text,"void F0128_DUNGEONVIEW_Draw_CPSF("); end=line_number(text,"F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);")
 seq=[]; pending=None
 update_re=re.compile(r"F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement\(P0183_i_Direction, ([0-4]), (-?[0-2]),")
 call_re=re.compile(r"\b(F0(?:676|677|678|679)_DrawD[23][LR]2|F01(?:16_DUNGEONVIEW_DrawSquareD3L|17_DUNGEONVIEW_DrawSquareD3R|18_DUNGEONVIEW_DrawSquareD3C_CPSF|19_DUNGEONVIEW_DrawSquareD2L|20_DUNGEONVIEW_DrawSquareD2R_CPSF|21_DUNGEONVIEW_DrawSquareD2C|22_DUNGEONVIEW_DrawSquareD1L|23_DUNGEONVIEW_DrawSquareD1R|24_DUNGEONVIEW_DrawSquareD1C|25_DUNGEONVIEW_DrawSquareD0L|26_DUNGEONVIEW_DrawSquareD0R|27_DUNGEONVIEW_DrawSquareD0C))\(")
 d4_re=re.compile(r"F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF\(.*(M59[789]_VIEW_SQUARE_D4[CLR])")
 for off,line in enumerate(lines[start-1:end],start):
  m=update_re.search(line)
  if m: pending=(int(m.group(1)),int(m.group(2))); continue
  d4=d4_re.search(line)
  if d4:
   if pending is None: raise AssertionError(f"missing F0150 coordinates before {d4.group(1)} at DUNVIEW.C:{off}")
   seq.append({"square":MACRO_TO_SQUARE[d4.group(1)],"depth":pending[0],"lateral":pending[1],"sourceLine":off,"call":"F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"}); pending=None; continue
  c=call_re.search(line)
  if c:
   name=c.group(1)
   if name=="F0127_DUNGEONVIEW_DrawSquareD0C": depth,lateral=0,0
   else:
    if pending is None: raise AssertionError(f"missing F0150 coordinates before {name} at DUNVIEW.C:{off}")
    depth,lateral=pending
   seq.append({"square":CALL_TO_SQUARE[name],"depth":depth,"lateral":lateral,"sourceLine":off,"call":name}); pending=None
 return seq
def parse_firestaff_table():
 text=LOCAL.read_text(encoding="utf-8",errors="replace")
 table=re.search(r"static const DM1_ViewportDrawStep s_draw_order\[\] = \{(?P<body>.*?)\n\};",text,flags=re.S)
 if not table: raise AssertionError("missing s_draw_order table")
 row_re=re.compile(r"\{\s*(DM1_VIEW_SQUARE_[A-Z0-9]+),\s*(-?\d+),\s*(-?\d+),\s*\"([^\"]+)\"")
 return [{"index":i,"square":m.group(1),"depth":int(m.group(2)),"lateral":int(m.group(3)),"call":m.group(4)} for i,m in enumerate(row_re.finditer(table.group("body")))]
def main():
 red=parse_redmcsb_f0128(); local=parse_firestaff_table(); problems=[]; pairs=[]
 if len(red)!=19: problems.append(f"redmcsb-sequence-count-{len(red)}")
 if len(local)!=19: problems.append(f"firestaff-sequence-count-{len(local)}")
 for i,(src,loc) in enumerate(zip(red,local)):
  ok=src["square"]==loc["square"] and src["depth"]==loc["depth"] and src["lateral"]==loc["lateral"]
  if not ok: problems.append(f"draw-order-mismatch-{i}")
  pairs.append({"index":i,"ok":ok,"redmcsb":src,"firestaff":loc})
 status="PASS_PASS557_DM1_V1_VIEWPORT_F0128_DRAW_ORDER_SOURCE_LOCK" if not problems else "FAIL_PASS557_DM1_V1_VIEWPORT_F0128_DRAW_ORDER_SOURCE_LOCK"
 manifest={"schema":"firestaff.parity.pass557_dm1_v1_viewport_f0128_draw_order_source_lock.v1","status":status,"ok":not problems,"redmcsbSourceRoot":str(RED),"redmcsbDunviewSha256":sha256(DUNVIEW),"firestaffViewport3dSha256":sha256(LOCAL),"sourceLock":"DUNVIEW.C F0128 F0150 relative-coordinate calls through F0127 draw sequence","pairs":pairs,"nonClaims":["no pixel parity promotion","no runtime capture promotion","no renderer behavior change"],"problems":problems}
 MANIFEST.parent.mkdir(parents=True,exist_ok=True); MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n")
 report=["# Pass557 - DM1 V1 viewport F0128 draw-order source lock","",f"Status: {status}","","This gate extracts the ReDMCSB F0128 relative-coordinate draw sequence and compares it with Firestaff's s_draw_order table.","","## Locked Pairs"]
 for pair in pairs:
  src=pair["redmcsb"]; loc=pair["firestaff"]; report.append(f"- {pair['index']:02d} ok={pair['ok']} {src['square']} depth={src['depth']} lateral={src['lateral']} ReDMCSB DUNVIEW.C:{src['sourceLine']} Firestaff call={loc['call']}")
 report+=["","## Non-claims","- This is a source-lock gate only.","- Same-viewport original-vs-Firestaff pixel capture remains outside this pass."]
 if problems: report+=["","## Problems"]+[f"- {p}" for p in problems]
 REPORT.write_text("\n".join(report)+"\n")
 print(status); print(f"manifest={MANIFEST.relative_to(ROOT)}"); print(f"report={REPORT.relative_to(ROOT)}")
 return 0 if not problems else 1
if __name__=="__main__":
 try: raise SystemExit(main())
 except AssertionError as exc: print(f"FAIL: {exc}",file=sys.stderr); raise SystemExit(1)
