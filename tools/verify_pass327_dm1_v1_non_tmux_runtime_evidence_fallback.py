#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PASS="pass327_dm1_v1_non_tmux_runtime_evidence_fallback"
MAN=ROOT/"parity-evidence/verification"/f"{PASS}.json"
REPORT=ROOT/"parity-evidence"/f"{PASS}.md"
ALLOWED={"PASS327_NON_TMUX_RUNTIME_EVIDENCE_CAPTURED","BLOCKED_PASS327_NON_TMUX_RAW_STREAM_NO_STRICT_CODE_STOP_PROMOTION","BLOCKED_PASS327_NON_TMUX_RUNTIME_NOT_AVAILABLE"}
def main():
    d=json.loads(MAN.read_text(encoding="utf-8")); errs=[]
    if d.get("schema")!=f"{PASS}.v1": errs.append("bad schema")
    if d.get("status") not in ALLOWED: errs.append("bad status")
    if not all(r.get("ok") for r in d.get("sourceAudit",[])): errs.append("source audit failed")
    if not d.get("parserSelftest",{}).get("ok"): errs.append("parser selftest failed")
    if d.get("fallbackPath",{}).get("tmuxRequired") is not False: errs.append("fallback still requires tmux")
    rt=d.get("runtimeProbe",{})
    if rt.get("usedTmux") not in (False,None): errs.append("runtime used tmux")
    if not d.get("promotionRules",{}).get("sourceLockPreserved"): errs.append("source lock not preserved")
    prior=d.get("priorArtifacts",{})
    if prior.get("pass320Status")!="BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER": errs.append("pass320 blocker not preserved")
    if "MISSING_DEBUGGER_CODE_STOP" not in str(prior.get("pass321Status")): errs.append("pass321 blocker not preserved")
    if prior.get("pass322Status")!="MOVEMENT_STATE_BINDING_SOURCE_SYMBOL_LOCKED_RUNTIME_PROBE_DESIGNED": errs.append("pass322 binding not preserved")
    if rt.get("ran") and float(rt.get("durationSeconds",999))>90: errs.append("runtime exceeded bound")
    if not REPORT.exists(): errs.append("missing report")
    print(json.dumps({"status":"FAIL" if errs else "PASS","manifestStatus":d.get("status"),"errors":errs,"manifest":str(MAN),"report":str(REPORT)},indent=2,sort_keys=True))
    return 1 if errs else 0
if __name__=="__main__": raise SystemExit(main())
