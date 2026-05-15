#!/usr/bin/env python3
from pathlib import Path
import json, subprocess, sys
ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass561_dm1_v1_far_door_front_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass561_dm1_v1_far_door_front_source_lock.md"
STATUS = "PASS561_DM1_V1_FAR_DOOR_FRONT_SOURCE_LOCKED"
SRC = [
    ("d3l2-far-door-front-split", "DUNVIEW.C", "6269-6286", ["case C17_ELEMENT_DOOR_FRONT:", "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT", "C3700_ZONE_DOOR_D3L2", "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT", "goto T0676017;", "C14_VIEW_SQUARE_D3L2, L2483_i_Order"]),
    ("d3r2-mirrored-far-door-front-split", "DUNVIEW.C", "6336-6353", ["case C17_ELEMENT_DOOR_FRONT:", "C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT", "C3710_ZONE_DOOR_D3R2", "C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT", "goto T0677018;", "C15_VIEW_SQUARE_D3R2, L2485_i_Order"]),
]
LOCAL = [
    ("firestaff-far-door-front-metadata", ROOT / "dm1_v1_viewport_3d_pc34_compat.c", "135-145", ["DM1_VIEW_SQUARE_D3L2, 0x0218, 0x0349", "DUNVIEW.C:6270 floor ornament under far rear pass", "DM1_VIEW_SQUARE_D3R2, 0x0128, 0x0439", "DUNVIEW.C:6337 floor ornament under mirrored far rear pass"]),
    ("firestaff-far-door-front-runtime-test", ROOT / "test_dm1_v1_viewport_3d_pc34_compat.c", "435-725", ["DM1_VIEW_SQUARE_D3L2, \"6270\"", "DM1_VIEW_SQUARE_D3R2, \"6337\"", "door_front_occlusion_spec_count(), 9", "source_evidence.far_door_front_occlusion"]),
    ("firestaff-source-evidence-string", ROOT / "dm1_v1_viewport_3d_pc34_compat.c", "960-972", ["DUNVIEW.C:6270-6286 D3L2 far door-front occlusion", "DUNVIEW.C:6337-6353 D3R2 mirrored far door-front occlusion"]),
]
def span(path, lines):
    a,b=[int(x) for x in lines.split("-")]
    enc="latin-1" if path.suffix.upper() in {".C",".H"} else "utf-8"
    return a, "\n".join(path.read_text(encoding=enc, errors="replace").splitlines()[a-1:b])
def ordered(base, text, needles):
    cur=0; hits=[]; missing=[]
    for n in needles:
        pos=text.find(n, cur)
        if pos < 0: missing.append(n)
        else: hits.append({"line": base + text.count("\n",0,pos), "needle": n}); cur=pos+len(n)
    return hits, missing
def audit(rows):
    out=[]
    for ident,name,lines,needles in rows:
        path=RED/name if isinstance(name,str) else name
        base,text=span(path, lines); hits,missing=ordered(base,text,needles)
        out.append({"id":ident,"status":"PASS" if not missing else "FAIL","sourceFile":path.name,"lines":lines,"hits":hits,"missing":missing})
    return out
def run(cmd):
    p=subprocess.run(cmd,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    return {"command":cmd,"returncode":p.returncode,"passed":p.returncode==0,"outputTail":"\n".join(p.stdout.strip().splitlines()[-14:])}
def main(check=False):
    red,loc=audit(SRC),audit(LOCAL)
    failed=[r["id"] for r in red+loc if r["status"]!="PASS"]
    if check:
        print("PASS pass561 check-only" if not failed else "FAIL pass561 check-only: "+",".join(failed)); return 0 if not failed else 1
    runtime=run([str(ROOT/"build-pass561"/"test_dm1_v1_viewport_3d_pc34_compat")])
    check_run=run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    ok=not failed and runtime["passed"] and check_run["passed"]
    manifest={"schema":"pass561_dm1_v1_far_door_front_source_lock.v1","status":"passed" if ok else "failed","statusToken":STATUS if ok else "FAILED_PASS561_DM1_V1_FAR_DOOR_FRONT_SOURCE_LOCK","redmcsbRoot":str(RED),"redmcsbChecks":red,"firestaffChecks":loc,"verificationRuns":[runtime,check_run],"nonClaims":["No input or movement queue edits.","No renderer pixel parity claim.","No original DOS runtime capture claim.","No DANNESBURK use."]}
    MANIFEST.parent.mkdir(parents=True, exist_ok=True); REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True)+"\n")
    lines=["# Pass561 DM1 V1 far door-front source lock","","Status: "+manifest["status"],"","Claim: D3L2 and mirrored D3R2 front-door branches use ReDMCSB's two-pass far door-front order: rear F0115 pass, far F0111 door, then front F0115 pass.","","## Primary ReDMCSB Evidence"]
    for row in red:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]
        lines += [f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]
        lines += [f"  - missing: {m}" for m in row["missing"]]
    lines += ["","## Firestaff Evidence"]
    for row in loc:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]
        lines += [f"  - line {h['line']}: {h['needle']}" for h in row["hits"]]
    lines += ["","## Verification"]
    for row in manifest["verificationRuns"]:
        lines += ["", f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    REPORT.write_text("\n".join(lines)+"\n")
    print(manifest["statusToken"]); print("- wrote", MANIFEST.relative_to(ROOT)); print("- wrote", REPORT.relative_to(ROOT)); return 0 if ok else 1
if __name__ == "__main__": raise SystemExit(main("--check-only" in sys.argv))
