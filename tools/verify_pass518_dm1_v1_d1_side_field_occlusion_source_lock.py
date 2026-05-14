#!/usr/bin/env python3
from pathlib import Path
import json,subprocess
ROOT=Path(__file__).resolve().parents[1]; RED=Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()
MANIFEST=ROOT/"parity-evidence/verification/pass518_dm1_v1_d1_side_field_occlusion_source_lock/manifest.json"; REPORT=ROOT/"parity-evidence/pass518_dm1_v1_d1_side_field_occlusion_source_lock.md"
checks=[("D1L","7391-7557",["L0213_i_Order = C0x0032_CELL_ORDER_BACKRIGHT_FRONTRIGHT;","F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF","F0113_DUNGEONVIEW_DrawField","C713_ZONE_WALL_D1L"]),("D1R","7559-7725",["L0215_i_Order = C0x0041_CELL_ORDER_BACKLEFT_FRONTLEFT;","F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF","F0113_DUNGEONVIEW_DrawField","C714_ZONE_WALL_D1R"])]
red=RED.read_text(encoding="latin-1",errors="replace"); rows=[]
for sq,span,needles in checks:
 a,b=(int(x) for x in span.split("-")); body="\n".join(red.splitlines()[a-1:b]); rows.append({"square":sq,"source":f"DUNVIEW.C:{span}","status":"PASS" if all(n in body for n in needles) else "FAIL"})
locals=[("dm1_v1_viewport_3d_pc34_compat.c","DM1_VIEW_SQUARE_D1L, 0x0032"),("dm1_v1_viewport_3d_pc34_compat.c","DM1_VIEW_SQUARE_D1R, 0x0041"),("test_dm1_v1_viewport_3d_pc34_compat.c","floor_field_order_spec_count(), 10")]
lrows=[{"file":f,"needle":n,"status":"PASS" if n in (ROOT/f).read_text(errors="replace") else "FAIL"} for f,n in locals]
p=subprocess.run([str(ROOT/"build"/"test_dm1_v1_viewport_3d_pc34_compat")],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
ok=all(r["status"]=="PASS" for r in rows+lrows) and p.returncode==0
m={"schema":"pass518_dm1_v1_d1_side_field_occlusion_source_lock.v1","status":"passed" if ok else "failed","statusToken":"PASS518_DM1_V1_D1_SIDE_FIELD_OCCLUSION_SOURCE_LOCKED" if ok else "FAILED_PASS518_DM1_V1_D1_SIDE_FIELD_OCCLUSION_SOURCE_LOCK","redmcsbChecks":rows,"firestaffChecks":lrows,"verificationRuns":[{"command":[str(ROOT/"build"/"test_dm1_v1_viewport_3d_pc34_compat")],"returncode":p.returncode,"passed":p.returncode==0,"outputTail":"\n".join(p.stdout.splitlines()[-10:])}],"nonClaims":["no input or movement queue edits","no original DOS pixel parity claim","no DANNESBURK use"]}
MANIFEST.parent.mkdir(parents=True,exist_ok=True); MANIFEST.write_text(json.dumps(m,indent=2,sort_keys=True)+"\n")
REPORT.write_text("# Pass518 DM1 V1 D1 side field occlusion source lock\n\nStatus: "+m["status"]+"\n\nPrimary evidence: DUNVIEW.C:7391-7557 and DUNVIEW.C:7559-7725.\n")
print(m["statusToken"]); print("- wrote",MANIFEST.relative_to(ROOT)); print("- wrote",REPORT.relative_to(ROOT)); raise SystemExit(0 if ok else 1)
