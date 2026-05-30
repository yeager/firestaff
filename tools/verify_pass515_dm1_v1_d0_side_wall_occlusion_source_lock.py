#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DM1 = Path("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1").expanduser()
GREATSTONE = Path("~/.openclaw/data/firestaff-greatstone-atlas").expanduser()
REPORT = ROOT / "parity-evidence/pass515_dm1_v1_d0_side_wall_occlusion_source_lock.md"
MANIFEST = ROOT / "parity-evidence/verification/pass515_dm1_v1_d0_side_wall_occlusion_source_lock/manifest.json"
STATUS = "PASS515_DM1_V1_D0_SIDE_WALL_OCCLUSION_SOURCE_LOCKED"

SOURCE_CHECKS = [
    {"id":"redmcsb_f0128_d0_side_lanes_precede_d0c","path":RED/"DUNVIEW.C","lines":"8534-8542","claim":"F0128 renders near side lanes D0L then D0R before D0C.","ordered":[
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, -1, &L0224_i_MapX, &L0225_i_MapY);",
        "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, 1, &L0224_i_MapX, &L0225_i_MapY);",
        "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);"]},
    {"id":"redmcsb_f0125_d0l_wall_return_blocks_open_lane_path","path":RED/"DUNVIEW.C","lines":"7999-8059","claim":"D0L wall draws C716 and returns before shared ceiling/field tail.","ordered":[
        "case C16_ELEMENT_DOOR_SIDE:","case C05_ELEMENT_TELEPORTER:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0220_ai_SquareAspect[M550_FIRST_THING], P0174_i_Direction, P0175_i_MapX, P0176_i_MapY, M610_VIEW_SQUARE_D0L, C0x0002_CELL_ORDER_BACKRIGHT);",
        "case C00_ELEMENT_WALL:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
        "return;","F0112_DUNGEONVIEW_DrawCeilingPit(C067_GRAPHIC_CEILING_PIT_D0L",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M610_VIEW_SQUARE_D0L]], C716_ZONE_WALL_D0L);"]},
    {"id":"redmcsb_f0126_d0r_wall_return_blocks_open_lane_path","path":RED/"DUNVIEW.C","lines":"8103-8159","claim":"D0R wall draws C717 and returns before open object/field path.","ordered":[
        "case C16_ELEMENT_DOOR_SIDE:","case C05_ELEMENT_TELEPORTER:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0221_ai_SquareAspect[M550_FIRST_THING], P0177_i_Direction, P0178_i_MapX, P0179_i_MapY, M611_VIEW_SQUARE_D0R, C0x0001_CELL_ORDER_BACKLEFT);",
        "case C00_ELEMENT_WALL:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
        "return;","F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);"]},
    {"id":"redmcsb_d0_wall_zone_ids_are_side_specific","path":RED/"DEFS.H","lines":"4050-4060","claim":"PC34/I34E D0 side-wall zones are distinct from D0C.","ordered":["#define C716_ZONE_WALL_D0L                                      716","#define C717_ZONE_WALL_D0R                                      717"]},
]
FIRESTAFF_CHECKS = [
    {"id":"firestaff_existing_d0_side_wall_specs_cite_return_lines","path":ROOT/"src/dm1/dm1_v1_viewport_3d_pc34_compat.c","lines":"424-430","claim":"Existing runtime metadata records direct D0 side-wall return evidence.","ordered":[
        '{ DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  true,  false, DM1_PC34_ZONE_WALL_D0L,  true,  false, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8016-8033", "DUNVIEW.C:8036-8038 wall case returns" },',
        '{ DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  true,  false, DM1_PC34_ZONE_WALL_D0R,  true,  false, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8126-8139", "DUNVIEW.C:8142-8144 wall case returns" },']},
    {"id":"firestaff_existing_d0_side_occlusion_orders_are_single_back_cells","path":ROOT/"src/dm1/dm1_v1_viewport_3d_pc34_compat.c","lines":"251-256","claim":"Open D0 side lanes use one back cell each; wall is a separate return path.","ordered":[
        '{ DM1_VIEW_SQUARE_D0L, 0x0002, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8000-8005 door-side/teleporter branch", "DUNVIEW.C:8005 F0115 with C0x0002" },',
        '{ DM1_VIEW_SQUARE_D0R, 0x0001, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8110-8115 door-side/teleporter branch", "DUNVIEW.C:8115 F0115 with C0x0001" },']},
    {"id":"firestaff_viewport_test_covers_d0_side_wall_specs","path":ROOT/"tests/test_dm1_v1_viewport_3d_pc34_compat.c","lines":"294-305","claim":"Focused runtime test asserts D0 side return lines and zone/wall pairing.","ordered":[
        '{ DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  0, DM1_PC34_ZONE_WALL_D0L,  1, 0, "8038" },',
        '{ DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  0, DM1_PC34_ZONE_WALL_D0R,  1, 0, "8144" },']},
]
REFERENCE_PATHS=[("dm1_graphics_dat",DM1/"GRAPHICS.DAT"),("dm1_dungeon_dat",DM1/"DUNGEON.DAT"),("greatstone_root",GREATSTONE)]
ALLOWED=[Path("~/.openclaw/data/firestaff-redmcsb-source").expanduser().resolve(),Path("~/.openclaw/data/firestaff-original-games/DM").expanduser().resolve(),Path("~/.openclaw/data/firestaff-greatstone-atlas").expanduser().resolve(),ROOT.resolve()]

def allow(p):
    r=p.expanduser().resolve()
    if not any(str(r).startswith(str(a)) for a in ALLOWED): raise SystemExit("refusing non-local evidence path: "+str(p))
    return r
def read(p):
    p=allow(p); return p.read_text(encoding="latin-1" if p.suffix.upper() in {".C",".H"} else "utf-8", errors="replace")
def sha(p):
    p=allow(p); h=hashlib.sha256()
    with p.open("rb") as f:
        for b in iter(lambda:f.read(1024*1024), b""): h.update(b)
    return h.hexdigest()
def slice_body(t,spec):
    a,b=[int(x) for x in spec.split("-",1)]; return a,chr(10).join(t.splitlines()[a-1:b])
def lno(t,pos): return t.count(chr(10),0,pos)+1
def audit(checks):
    out=[]
    for c in checks:
        p=allow(c["path"]); base,body=slice_body(read(p),c["lines"]); cur=0; hits=[]; miss=[]
        for n in c["ordered"]:
            pos=body.find(n,cur)
            if pos<0: miss.append(n)
            else: hits.append({"needle":n,"line":base+lno(body,pos)-1}); cur=pos+len(n)
        out.append({"id":c["id"],"status":"PASS" if not miss else "FAIL","path":str(p),"sourceFile":p.name,"lines":c["lines"],"sha256":sha(p),"hits":hits,"missing":miss,"claim":c["claim"]})
    return out
def run_gate(args):
    p=subprocess.run(args,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    return {"command":args,"returncode":p.returncode,"passed":p.returncode==0,"outputTail":chr(10).join(p.stdout.strip().splitlines()[-12:])}
def refs():
    out=[]
    for i,p in REFERENCE_PATHS:
        p=allow(p); row={"id":i,"path":str(p),"exists":p.exists()}
        if p.is_file(): row.update({"bytes":p.stat().st_size,"sha256":sha(p)})
        out.append(row)
    return out
def write(man):
    MANIFEST.parent.mkdir(parents=True,exist_ok=True); REPORT.parent.mkdir(parents=True,exist_ok=True)
    MANIFEST.write_text(json.dumps(man,indent=2,sort_keys=True)+chr(10),encoding="utf-8")
    lines=["# Pass515 DM1 V1 D0 side wall occlusion source lock","","Status: "+man["status"],"","## Claim","","ReDMCSB draws D0L and D0R before D0C. If either side lane is a wall, it draws its side wall zone and returns before the open-lane ceiling/object/teleporter-field path. This is narrower than the D0C foreground work.","","## Primary ReDMCSB Evidence"]
    for r in man["redmcsbPrimaryChecks"]:
        lines += ["",f"- {r['status']} {r['id']} ({Path(r['path']).name}:{r['lines']})",f"  - {r['claim']}"]
        lines += [f"  - line {h['line']}: {h['needle']}" for h in r["hits"]]
        lines += [f"  - missing: {m}" for m in r["missing"]]
    lines += ["","## Firestaff Evidence"]
    for r in man["firestaffChecks"]: lines += ["",f"- {r['status']} {r['id']} ({Path(r['path']).name}:{r['lines']})",f"  - {r['claim']}"]
    lines += ["","## Verification"]
    for v in man["verificationRuns"]: lines += ["",f"- command: {' '.join(v['command'])}",f"  - returncode: {v['returncode']}","  - output tail:","~~~",v["outputTail"],"~~~"]
    lines += ["","## Local References"] + [f"- {r['id']}: exists={r['exists']} path={r['path']}" + (f", sha256={r['sha256']}" if "sha256" in r else "") for r in man["localReferences"]]
    lines += ["","## Non-Claims","","- No runtime metadata was changed.","- No D0C foreground behavior is promoted by this pass.","- DANNESBURK was not used."]
    REPORT.write_text(chr(10).join(lines)+chr(10),encoding="utf-8")
def main(check=False):
    red=audit(SOURCE_CHECKS); fire=audit(FIRESTAFF_CHECKS); failed=[r for r in red+fire if r["status"]!="PASS"]
    if check: print("PASS check-only" if not failed else "FAIL check-only"); return 0 if not failed else 1
    runtime=run_gate([str(ROOT/"build"/"test_dm1_v1_viewport_3d_pc34_compat")]); local_refs=refs()
    ok=not failed and runtime["passed"] and all(r["exists"] for r in local_refs)
    man={"schema":"pass515_dm1_v1_d0_side_wall_occlusion_source_lock.v1","status":"passed" if ok else "failed","statusToken":STATUS if ok else "FAILED_PASS515_DM1_V1_D0_SIDE_WALL_OCCLUSION_SOURCE_LOCK","redmcsbRoot":str(RED),"claim":"D0L/D0R side wall cases draw before D0C and return before side-lane open content/field paths.","redmcsbPrimaryChecks":red,"firestaffChecks":fire,"localReferences":local_refs,"verificationRuns":[runtime,{"command":[sys.executable,str(Path(__file__).resolve()),"--check-only"],"returncode":0 if not failed else 1,"passed":not failed,"outputTail":"PASS check-only" if not failed else "FAIL check-only"}],"nonClaims":["No runtime metadata was changed.","No D0C foreground behavior is promoted.","No DANNESBURK or external references were used."]}
    write(man)
    print(man["statusToken"])
    print(f"- wrote {MANIFEST.relative_to(ROOT)}")
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    if runtime["outputTail"]: print("- runtime tail: "+runtime["outputTail"].splitlines()[-1])
    return 0 if ok else 1
if __name__=="__main__": raise SystemExit(main("--check-only" in sys.argv))
