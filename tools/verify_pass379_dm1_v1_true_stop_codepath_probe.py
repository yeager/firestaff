#!/usr/bin/env python3
import json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass379_dm1_v1_true_stop_codepath_probe"
MAN=ROOT/"parity-evidence/verification"/PASS/"manifest.json"
ALLOWED={"PASS379_STRICT_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN","PASS379_STRICT_F0128_ONLY_F0097_BLOCKED","BLOCKED_PASS379_07FB_TRUE_STOP_PROVES_CONTROL_F0128_MAP_OR_PATH_UNRESOLVED","BLOCKED_PASS379_NO_DIRECT_PROBE_STOP","BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED"}
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
    if a.get("postRoutePauseProbe")!="07FB:01EB": errs.append("probe address drift")
    rt=m.get("runtimeProbe",{})
    if rt.get("boundedSeconds",999)>75: errs.append("runtime bound exceeded")
    for key in ["transcript","routeKeylog","commandLog"]:
        if not (ROOT/rt.get(key,"")).exists(): errs.append("missing "+key)
    dh=rt.get("directHits",{})
    if rt.get("sawRunning") is not True: errs.append("no running marker")
    if rt.get("routeInputAfterArming") is not True: errs.append("route not delivered after arming")
    if m.get("status") == "BLOCKED_PASS379_07FB_TRUE_STOP_PROVES_CONTROL_F0128_MAP_OR_PATH_UNRESOLVED":
        if dh.get("f0128_23AD_40FE"): errs.append("blocked despite F0128 hit")
        if not dh.get("probe_07FB_01EB"): errs.append("07FB narrowing without 07FB hit")
        stops=rt.get("stops",[])
        if not any(s.get("addr")=="07FB:01EB" and s.get("details") for s in stops): errs.append("07FB hit lacks detail commands")
    if m.get("status") == "BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED":
        if dh.get("f0128_23AD_40FE") or dh.get("f0097_2809_1EFF") or dh.get("probe_07FB_01EB"): errs.append("moved-pause blocker despite direct hit")
        if rt.get("breakpointRetainedPostRoute") is not True: errs.append("moved-pause blocker without retained breakpoints")
        if not rt.get("finalPauseCodeAddr"): errs.append("missing final pause addr")
    if m.get("notPromotedBy") != ["BPLIST","BP command echo","tmux/capture-pane","source-only address binding","07FB stop alone"]:
        errs.append("promotion guard drift")
if errs:
    print("FAIL", "; ".join(errs)); sys.exit(1)
print("pass379 verifier OK")
