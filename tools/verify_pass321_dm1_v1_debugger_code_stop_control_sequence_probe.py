#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe/manifest.json"
REPORT = ROOT / "parity-evidence/pass321_dm1_v1_debugger_code_stop_control_sequence_probe.md"
ALLOWED = {
    "PASS321_REAL_F0128_CODE_STOP_AND_F0097_AFTERWARD_CAPTURED",
    "BLOCKED_PASS321_F0128_CODE_STOP_CAPTURED_NO_F0097_AFTERWARD",
    "BLOCKED_PASS321_MISSING_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE",
}

def main() -> int:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    errors = []
    if data.get("schema") != "pass321_dm1_v1_debugger_code_stop_control_sequence_probe.v1":
        errors.append("bad schema")
    if data.get("status") not in ALLOWED:
        errors.append("bad status")
    if not all(item.get("ok") for item in data.get("source_audit", [])):
        errors.append("source audit failed")
    if not data.get("parser_selftest", {}).get("ok"):
        errors.append("parser selftest failed")
    prior = data.get("prior_artifact_audit", {})
    if not prior.get("pass318", {}).get("bplist_echo_confusion"):
        errors.append("pass318 BPLIST confusion not recorded")
    if prior.get("pass320", {}).get("strict_hit_count") != 0:
        errors.append("pass320 strict reparse unexpectedly has stops")
    runtime = data.get("runtime_probe", {})
    if runtime.get("ran") and runtime.get("duration_seconds", 999) > 65:
        errors.append("runtime exceeded bounded limit")
    if not REPORT.exists():
        errors.append("missing report")
    if errors:
        print(json.dumps({"status": "FAIL", "errors": errors}, indent=2, sort_keys=True))
        return 1
    print(json.dumps({"status": "PASS", "manifest_status": data.get("status"), "manifest": str(MANIFEST), "report": str(REPORT)}, indent=2, sort_keys=True))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
