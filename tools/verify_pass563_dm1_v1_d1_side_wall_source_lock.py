#!/usr/bin/env python3
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
RED=Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST=ROOT/"parity-evidence/verification/pass563_dm1_v1_d1_side_wall_source_lock/manifest.json"
REPORT=ROOT/"parity-evidence/pass563_dm1_v1_d1_side_wall_source_lock.md"
STATUS="PASS563_DM1_V1_D1_SIDE_WALL_SOURCE_LOCKED"
TEST_BINARY=ROOT/"build"/"test_dm1_v1_viewport_3d_pc34_compat"
SRC=[
("defs-pc34-d1-side-wall-zones","DEFS.H","4052-4054",["#define C713_ZONE_WALL_D1L","#define C714_ZONE_WALL_D1R"]),
("f0128-d1-row-left-right-before-center","DUNVIEW.C","8518-8533",["F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, -1, &L0224_i_MapX, &L0225_i_MapY);","F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 1, &L0224_i_MapX, &L0225_i_MapY);","F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0, &L0224_i_MapX, &L0225_i_MapY);","F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);"]),
("d1l-wall-branch-draws-zone-and-returns","DUNVIEW.C","7436-7460",["case C00_ELEMENT_WALL:","F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);","F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);","F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL], M585_VIEW_WALL_D1L_RIGHT);","return;"]),
("d1r-wall-branch-mirrors-zone-and-returns","DUNVIEW.C","7604-7628",["case C00_ELEMENT_WALL:","F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);","F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);","F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M586_VIEW_WALL_D1R_LEFT);","return;"])]
LOCAL=[
("firestaff-d1-side-wall-metadata",ROOT/"src/dm1/dm1_v1_viewport_3d_pc34_compat.c","286-307",["DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R","DM1_PC34_ZONE_WALL_D1L","DUNVIEW.C:7445-7455","DUNVIEW.C:7459-7460 side ornament then return","DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L","DM1_PC34_ZONE_WALL_D1R","DUNVIEW.C:7613-7623","DUNVIEW.C:7627-7628 side ornament then return"]),
("firestaff-d1-side-wall-runtime-test",ROOT/"tests/test_dm1_v1_viewport_3d_pc34_compat.c","237-258",["DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R","DM1_PC34_ZONE_WALL_D1L","\"7460\"","DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L","DM1_PC34_ZONE_WALL_D1R","\"7628\""]),
("firestaff-source-evidence-string",ROOT/"src/dm1/dm1_v1_viewport_3d_pc34_compat.c","1098-1145",["DUNVIEW.C:7391-7557 D1L stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115","DUNVIEW.C:7559-7725 D1R stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115","DUNVIEW.C:7391 F0122_DUNGEONVIEW_DrawSquareD1L","DUNVIEW.C:7559 F0123_DUNGEONVIEW_DrawSquareD1R"])]
def span(path,lines):
    a,b=[int(x) for x in lines.split("-")]; enc="latin-1" if path.suffix.upper() in {".C",".H"} else "utf-8"
    return a,"\n".join(path.read_text(encoding=enc,errors="replace").splitlines()[a-1:b])
def ordered(base,text,needles):
    cur=0; hits=[]; missing=[]
    for n in needles:
        pos=text.find(n,cur)
        if pos<0: missing.append(n)
        else: hits.append({"line":base+text.count("\n",0,pos),"needle":n}); cur=pos+len(n)
    return hits,missing
def audit(rows):
    out=[]
    for ident,name,lines,needles in rows:
        path=RED/name if isinstance(name,str) else name; base,text=span(path,lines); hits,missing=ordered(base,text,needles)
        out.append({"id":ident,"status":"PASS" if not missing else "FAIL","sourceFile":path.name,"lines":lines,"hits":hits,"missing":missing})
    return out
def run(cmd):
    p=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    return {"command":cmd,"returncode":p.returncode,"passed":p.returncode==0,"outputTail":"\n".join(p.stdout.strip().splitlines()[-14:])}
def main(check=False):
    red=audit(SRC); loc=audit(LOCAL); failed=[r["id"] for r in red+loc if r["status"]!="PASS"]
    if check:
        print("PASS pass563 check-only" if not failed else "FAIL pass563 check-only: "+",".join(failed)); return 0 if not failed else 1
    runtime=run([str(TEST_BINARY)]); check_run=run([sys.executable,str(Path(__file__).resolve()),"--check-only"])
    ok=not failed and runtime["passed"] and check_run["passed"]
    manifest={"schema":"pass563_dm1_v1_d1_side_wall_source_lock.v1","status":"passed" if ok else "failed","statusToken":STATUS if ok else "FAILED_PASS563_DM1_V1_D1_SIDE_WALL_SOURCE_LOCK","redmcsbRoot":str(RED),"redmcsbChecks":red,"firestaffChecks":loc,"verificationRuns":[runtime,check_run],"nonClaims":["No input, capture, or pass514 work.","No renderer pixel parity claim.","No original DOS runtime capture claim.","No CSB/DM2 behavior claim.","No DANNESBURK use."]}
    MANIFEST.parent.mkdir(parents=True,exist_ok=True); REPORT.parent.mkdir(parents=True,exist_ok=True); MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n")
    lines=["# Pass563 DM1 V1 D1 side wall source lock","","Status: "+manifest["status"],"","Claim: D1L and mirrored D1R use the ReDMCSB PC34 side-wall lanes: F0128 draws D1L then D1R before D1C, and each D1 side-wall case draws its side-specific wall zone and returns before open-lane content/field paths.","","## Primary ReDMCSB Evidence"]
    for row in red:
        lines+=["",f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]+[f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]+[f"  - missing: {m}" for m in row["missing"]]
    lines+=["","## Firestaff Evidence"]
    for row in loc:
        lines+=["",f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]+[f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]+[f"  - missing: {m}" for m in row["missing"]]
    lines+=["","## Verification"]
    for row in manifest["verificationRuns"]: lines+=["",f"- {' '.join(row['command'])}: rc={row['returncode']}","~~~",row["outputTail"],"~~~"]
    REPORT.write_text("\n".join(lines)+"\n"); print(manifest["statusToken"]); print("- wrote",MANIFEST.relative_to(ROOT)); print("- wrote",REPORT.relative_to(ROOT)); return 0 if ok else 1
if __name__=="__main__": raise SystemExit(main("--check-only" in sys.argv))
