#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MANIFEST=ROOT/"parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive/manifest.json"
REPORT=ROOT/"parity-evidence/pass324_dm1_v1_debugger_code_stop_control_primitive.md"
STATUS="PASS_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE_FOUND"

def main() -> int:
    if not MANIFEST.exists():
        raise SystemExit(f"missing manifest: {MANIFEST}")
    data=json.loads(MANIFEST.read_text(encoding="utf-8"))
    assert data.get("status")==STATUS, data.get("status")
    assert REPORT.exists(), REPORT
    assert all(a.get("ok") for a in data.get("source_audit",[])), data.get("source_audit")
    trans=data.get("runtime_probe",{}).get("transition",{})
    assert trans.get("running_marker_seen"), trans
    assert trans.get("prompt_reappeared_after_running"), trans
    assert trans.get("separable_from_bplist"), trans
    assert not trans.get("bplist_text_after_running"), trans
    cp=data.get("control_primitive",{})
    assert cp.get("prompt")=="->"
    assert "ESC O t" in cp.get("run_control","")
    print(STATUS)
    return 0
if __name__=="__main__":
    raise SystemExit(main())
