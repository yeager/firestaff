#!/usr/bin/env python3
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[1]
RED=Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST=ROOT/"parity-evidence/verification/pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion/manifest.json"
REPORT=ROOT/"parity-evidence/pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion.md"
STATUS="PASS565_DM1_V1_D0C_THIEVES_EYE_DOOR_FRAME_OCCLUSION_SOURCE_LOCKED"
TEST_BINARY=ROOT/"build"/"test_dm1_v1_viewport_3d_pc34_compat"
SRC=[("d0c-door-side-thieves-eye-frame-branch",RED/"DUNVIEW.C","8185-8216",["case C16_ELEMENT_DOOR_SIDE:","if (G0407_s_Party.Event73Count_ThievesEye)","F0616_CopyBitmap(F0631_GetBitmapPointer(G2116_DoorFrameFrontD0C), G0074_puc_Bitmap_Temporary);","F0630_InitBitmapStruct2(M711_NEGGRAPHIC_HOLE_IN_WALL, &L2495_s_Struct2);","F0635_(NULL, L2496_ai_XYZ, C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME, &L2495_s_Struct2.Width, &L2495_s_Struct2.Height);","F0654_Call_F0132_VIDEO_Blit(M772_CAST_PC(L2495_s_Struct2.s2m1), M772_CAST_PC(G0074_puc_Bitmap_Temporary), L2496_ai_XYZ","F0656_BlitBitmapToViewportZoneIndexWithTransparency(G0074_puc_Bitmap_Temporary, C728_ZONE_DOOR_FRAME_D0C, C10_COLOR_FLESH);"]),("d0c-common-f0115-after-frame",RED/"DUNVIEW.C","8215-8295",["F0656_BlitBitmapToViewportZoneIndexWithTransparency(G0074_puc_Bitmap_Temporary, C728_ZONE_DOOR_FRAME_D0C, C10_COLOR_FLESH);","break;","F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);"]),("pc34-i34e-zone-ids",RED/"DEFS.H","4084-4095",["#define C728_ZONE_DOOR_FRAME_D0C","#define C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME"])]
LOCAL=[("firestaff-d0c-thieves-eye-metadata",ROOT/"src/dm1/dm1_v1_viewport_3d_pc34_compat.c","254-265",["DM1_VIEW_SQUARE_D0C, 0x0021, 728, 736","DUNVIEW.C:8185-8188","DUNVIEW.C:8199-8201","DUNVIEW.C:8206-8210","DUNVIEW.C:8215-8216","DUNVIEW.C:8240,8294"]),("firestaff-d0c-thieves-eye-test",ROOT/"tests/test_dm1_v1_viewport_3d_pc34_compat.c","825-850",["test_d0c_thieves_eye_door_frame_occlusion_order","dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C)","spec->door_frame_zone, 728","spec->hole_zone, 736","8215-8216","8294"]),("firestaff-source-evidence-string",ROOT/"src/dm1/dm1_v1_viewport_3d_pc34_compat.c","2120-2128",["DUNVIEW.C:8185-8216 D0C Thieves Eye door-side frame occlusion","copy front frame, composite hole, blit temporary frame before common F0115"])]
def span(path,lines):
    a,b=[int(v) for v in lines.split("-")]; enc="latin-1" if path.suffix.upper() in {".C",".H"} else "utf-8"; return a,"\n".join(path.read_text(encoding=enc,errors="replace").splitlines()[a-1:b])
def ordered(base,text,needles):
    cur=0; hits=[]; missing=[]
    for n in needles:
        pos=text.find(n,cur)
        if pos<0: missing.append(n)
        else: hits.append({"line":base+text.count("\n",0,pos),"needle":n}); cur=pos+len(n)
    return hits,missing
def audit(rows):
    out=[]
    for ident,path,lines,needles in rows:
        base,text=span(path,lines); hits,missing=ordered(base,text,needles); out.append({"id":ident,"status":"PASS" if not missing else "FAIL","sourceFile":path.name,"lines":lines,"hits":hits,"missing":missing})
    return out
def run(cmd):
    p=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT); return {"command":cmd,"returncode":p.returncode,"passed":p.returncode==0,"outputTail":"\n".join(p.stdout.strip().splitlines()[-16:])}
def main(check_only=False):
    red=audit(SRC); loc=audit(LOCAL); failed=[r["id"] for r in red+loc if r["status"]!="PASS"]
    if check_only:
        print("PASS pass565 check-only" if not failed else "FAIL pass565 check-only: "+", ".join(failed)); return 0 if not failed else 1
    runtime=run([str(TEST_BINARY)]); selfcheck=run([sys.executable,str(Path(__file__).resolve()),"--check-only"]); ok=not failed and runtime["passed"] and selfcheck["passed"]
    manifest={"schema":"pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion.v1","status":"passed" if ok else "failed","statusToken":STATUS if ok else "FAILED_PASS565_DM1_V1_D0C_THIEVES_EYE_DOOR_FRAME_OCCLUSION","redmcsbRoot":str(RED),"redmcsbChecks":red,"firestaffChecks":loc,"verificationRuns":[runtime,selfcheck],"nonClaims":["No original DOS runtime capture claim.","No pixel parity or renderer-completeness claim.","No D1C thieves-eye door-mask change; pass447 remains separate.","No DANNESBURK, Sonnet, q3.6/Qwen, push, or external action."]}
    MANIFEST.parent.mkdir(parents=True,exist_ok=True); MANIFEST.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n")
    lines=["# Pass565 DM1 V1 D0C Thieves Eye door-frame occlusion","","Status: "+manifest["status"],"","Claim: ReDMCSB PC34/I34E D0C door-side with Thieves Eye copies the front door frame into a temporary bitmap, composites the hole-in-wall graphic into that temporary frame, blits the temporary frame to C728, then reaches the common D0C F0115 pass with C0x0021. This is a source-lock only.","","## Primary ReDMCSB Evidence"]
    for row in red: lines+=["",f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]+[f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]+[f"  - missing: {m}" for m in row["missing"]]
    lines+=["","## Firestaff Evidence"]
    for row in loc: lines+=["",f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]+[f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]+[f"  - missing: {m}" for m in row["missing"]]
    lines+=["","## Verification"]
    for row in manifest["verificationRuns"]: lines+=["",f"- {' '.join(row['command'])}: rc={row['returncode']}","~~~",row["outputTail"],"~~~"]
    REPORT.write_text("\n".join(lines)+"\n"); print(manifest["statusToken"]); print("- wrote",MANIFEST.relative_to(ROOT)); print("- wrote",REPORT.relative_to(ROOT)); return 0 if ok else 1
if __name__=="__main__": raise SystemExit(main("--check-only" in sys.argv))
