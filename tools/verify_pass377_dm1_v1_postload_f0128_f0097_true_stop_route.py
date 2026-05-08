#!/usr/bin/env python3
import json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass377_dm1_v1_postload_f0128_f0097_true_stop_route"
MAN=ROOT/"parity-evidence/verification"/PASS/"manifest.json"
ALLOWED={"PASS377_POSTLOAD_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN","PASS377_POSTLOAD_F0128_TRUE_STOP_ONLY_F0097_BLOCKED","BLOCKED_PASS377_POSTLOAD_F0128_TRUE_STOP_NOT_RECAPTURED"}
errs=[]
if not MAN.exists():
    errs.append("missing manifest")
else:
    m=json.loads(MAN.read_text())
    if m.get("status") not in ALLOWED: errs.append("bad status "+str(m.get("status")))
    if not all(r.get("ok") for r in m.get("sourceAudit",[])): errs.append("source audit failed")
    a=m.get("addresses",{})
    if a.get("F0128_DUNGEONVIEW_Draw_CPSF")!="23AD:40FE": errs.append("F0128 address drift")
    if a.get("F0097_VIDRV_09_BlitViewPort_indirect_call")!="2809:1EFF": errs.append("VIDRV address drift")
    rt=m.get("runtimeProbe",{})
    if rt.get("boundedSeconds",999)>75: errs.append("runtime bound exceeded")
    if rt.get("strategy")!="post_load_arm_before_route": errs.append("wrong strategy")
    if not (ROOT/rt.get("transcript","")).exists(): errs.append("missing transcript")
    if not (ROOT/rt.get("routeKeylog","")).exists(): errs.append("missing route keylog")
    if m.get("notPromotedBy") != ["BPLIST", "BP command echo", "tmux/capture-pane", "source-only address binding"]: errs.append("promotion guard missing")
    direct=rt.get("directHits",{})
    if m.get("status")=="BLOCKED_PASS377_POSTLOAD_F0128_TRUE_STOP_NOT_RECAPTURED":
        if direct.get("f0128_23AD_40FE"): errs.append("blocked despite F0128 hit")
        if rt.get("routeInputAfterArming") is not True: errs.append("route not delivered after arming")
    if m.get("status","").startswith("PASS377_POSTLOAD_F0128") and not direct.get("f0128_23AD_40FE"):
        errs.append("pass without F0128")
if errs:
    print("FAIL", "; ".join(errs)); sys.exit(1)
print("pass377 verifier OK")
