#!/usr/bin/env python3
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
RED=Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST=ROOT/"parity-evidence/verification/pass562_dm1_v1_d2_far_side_wall_source_lock/manifest.json"
REPORT=ROOT/"parity-evidence/pass562_dm1_v1_d2_far_side_wall_source_lock.md"
STATUS="PASS562_DM1_V1_D2_FAR_SIDE_WALL_SOURCE_LOCKED"
SRC=[
("defs-pc34-d2-far-side-zones","DEFS.H","4042-4049",["#define C707_ZONE_WALL_D2L2","#define C708_ZONE_WALL_D2R2"]),
("d2l2-wall-and-field-branch","DUNVIEW.C","6836-6865",["STATICFUNCTION void F0678_DrawD2L2(","case C00_ELEMENT_WALL:","F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2);","F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]","C707_ZONE_WALL_D2L2);","return;","case C05_ELEMENT_TELEPORTER:","C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2);"]),
("d2r2-mirrored-wall-and-field-branch","DUNVIEW.C","6868-6895",["STATICFUNCTION void F0679_DrawD2R2(","case C00_ELEMENT_WALL:","F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C06_WALL_D2L2], C708_ZONE_WALL_D2R2);","F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]","C708_ZONE_WALL_D2R2);","return;","case C05_ELEMENT_TELEPORTER:","C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2);"])]
LOCAL=[
("firestaff-d2-far-side-wall-metadata",ROOT/"dm1_v1_viewport_3d_pc34_compat.c","250-260",["DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2","DM1_PC34_ZONE_WALL_D2L2","DUNVIEW.C:6849-6858","DUNVIEW.C:6848-6862 wall case returns","DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2","DM1_PC34_ZONE_WALL_D2R2","DUNVIEW.C:6880-6889","DUNVIEW.C:6882-6893 wall case returns"]),
("firestaff-d2-far-side-runtime-test",ROOT/"test_dm1_v1_viewport_3d_pc34_compat.c","180-190",["DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2","DM1_PC34_ZONE_WALL_D2L2","\"6862\"","DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2","DM1_PC34_ZONE_WALL_D2R2","\"6893\""]),
("firestaff-source-evidence-string",ROOT/"dm1_v1_viewport_3d_pc34_compat.c","972-979",["DUNVIEW.C:6849-6893 F0678/F0679 PC34 D2L2/D2R2 side-wall zones and wall-case returns"])]
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
        print("PASS pass562 check-only" if not failed else "FAIL pass562 check-only: "+",".join(failed)); return 0 if not failed else 1
    runtime=run([str(ROOT/"build-pass562"/"test_dm1_v1_viewport_3d_pc34_compat")]); check_run=run([sys.executable,str(Path(__file__).resolve()),"--check-only"])
    ok=not failed and runtime["passed"] and check_run["passed"]
    manifest={"schema":"pass562_dm1_v1_d2_far_side_wall_source_lock.v1","status":"passed" if ok else "failed","statusToken":STATUS if ok else "FAILED_PASS562_DM1_V1_D2_FAR_SIDE_WALL_SOURCE_LOCK","redmcsbRoot":str(RED),"redmcsbChecks":red,"firestaffChecks":loc,"verificationRuns":[runtime,check_run],"nonClaims":["No input, capture, or pass514 work.","No renderer pixel parity claim.","No original DOS runtime capture claim.","No CSB/DM2 behavior claim.","No DANNESBURK use."]}
    MANIFEST.parent.mkdir(parents=True,exist_ok=True); REPORT.parent.mkdir(parents=True,exist_ok=True); MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n")
    lines=["# Pass562 DM1 V1 D2 far-side wall source lock","","Status: "+manifest["status"],"","Claim: D2L2 and mirrored D2R2 use the ReDMCSB PC34 far-side wall lanes: wall cases draw the paired native/parity wallset into C707/C708 and return, while teleporter cases draw only the matching field zone.","","## Primary ReDMCSB Evidence"]
    for row in red:
        lines+=["",f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]+[f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]+[f"  - missing: {m}" for m in row["missing"]]
    lines+=["","## Firestaff Evidence"]
    for row in loc:
        lines+=["",f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]+[f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]+[f"  - missing: {m}" for m in row["missing"]]
    lines+=["","## Verification"]
    for row in manifest["verificationRuns"]: lines+=["",f"- {' '.join(row['command'])}: rc={row['returncode']}","~~~",row["outputTail"],"~~~"]
    REPORT.write_text("\n".join(lines)+"\n"); print(manifest["statusToken"]); print("- wrote",MANIFEST.relative_to(ROOT)); print("- wrote",REPORT.relative_to(ROOT)); return 0 if ok else 1
if __name__=="__main__": raise SystemExit(main("--check-only" in sys.argv))
