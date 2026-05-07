#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/manifest.json"
REPORT = ROOT / "parity-evidence/pass326_dm1_v1_direct_pty_f0128_code_stop.md"
TRANSCRIPT = ROOT / "parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/dosbox_debug_pty.clean.txt"
ALLOWED = {"PASS326_DIRECT_PTY_STRICT_F0128_CODE_STOP_PROVEN", "BLOCKED_PASS326_DIRECT_PTY_F0128_CODE_STOP_NOT_PROVEN"}

def main() -> int:
    errors: list[str] = []
    data = None
    if not MANIFEST.exists():
        errors.append(f"missing manifest {MANIFEST}")
    else:
        data = json.loads(MANIFEST.read_text(encoding="utf-8"))
        if data.get("schema") != "pass326_dm1_v1_direct_pty_f0128_code_stop.v1": errors.append("bad schema")
        if data.get("status") not in ALLOWED: errors.append(f"bad status {data.get('status')}")
        if not all(item.get("ok") for item in data.get("source_audit", [])): errors.append("source audit failed")
        addrs = data.get("addresses", {})
        if addrs.get("F0128_DUNGEONVIEW_Draw_CPSF") != "23AD:40FE": errors.append("F0128 address not locked")
        rt = data.get("runtime_probe", {})
        if not rt.get("ran"): errors.append("runtime probe did not run")
        if rt.get("bounded_seconds", 999) > 105 or rt.get("duration_seconds", 999) > 115: errors.append("runtime unbounded")
        if "pexpect-owned PTY" not in rt.get("method", ""): errors.append("runtime method not direct pexpect PTY")
        if not rt.get("bplist_negative_control_before_running"): errors.append("missing BPLIST negative control")
        if "(Running)" not in rt.get("strict_stop_detection_rule", "") and "observed (Running)" not in rt.get("strict_stop_detection_rule", ""):
            errors.append("strict running->prompt rule not recorded")
        stops = rt.get("f0128_target_stops", [])
        if data.get("status") == "PASS326_DIRECT_PTY_STRICT_F0128_CODE_STOP_PROVEN":
            if not stops: errors.append("missing strict F0128 stop")
            for s in stops:
                if not (s.get("running_marker_seen") and s.get("prompt_reappeared_after_running")): errors.append("stop lacks running->prompt transition")
                if s.get("stop_code_addr_after_running") != "23AD:40FE": errors.append("stop not at 23AD:40FE")
                if s.get("bplist_text_after_running"): errors.append("BPLIST leaked into post-running stop")
        else:
            if stops: errors.append("blocked status cannot carry promoted f0128 stops")
        dec = data.get("decision", {})
        if "2809:1EFF" not in dec.get("next_blocker", "") or "22F4:0699" not in dec.get("next_blocker", ""):
            errors.append("next blocker does not name F0097/F0380")
    if not REPORT.exists(): errors.append("missing report")
    if not TRANSCRIPT.exists() or TRANSCRIPT.stat().st_size <= 0: errors.append("missing transcript")
    if errors:
        print(json.dumps({"status": "FAIL", "errors": errors}, indent=2, sort_keys=True)); return 1
    print(json.dumps({"status": "PASS", "manifest_status": data.get("status"), "manifest": str(MANIFEST), "report": str(REPORT)}, indent=2, sort_keys=True))
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
