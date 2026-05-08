#!/usr/bin/env python3
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REPORT = ROOT / "parity-evidence" / "pass328_dm1_v1_direct_pty_postload_route_timing.md"
MANIFEST = ROOT / "parity-evidence" / "verification" / "pass328_dm1_v1_direct_pty_postload_route_timing" / "manifest.json"
EXPECTED = {
    "PASS_DIRECT_PTY_F0128_CODE_STOP_PROVEN",
    "PASS_DIRECT_PTY_F0128_F0097_SEQUENCE_PROVEN",
    "BLOCKED_PASS328_POSTLOAD_ROUTE_INPUT_MISSING",
    "BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING",
}

def fail(msg):
    print(json.dumps({"status": "FAIL", "error": msg, "report": str(REPORT), "manifest": str(MANIFEST)}, indent=2))
    return 1

def main():
    if not REPORT.exists():
        return fail("missing report")
    if not MANIFEST.exists():
        return fail("missing manifest")
    try:
        data = json.loads(MANIFEST.read_text())
    except Exception as exc:
        return fail(f"manifest json invalid: {exc}")
    status = data.get("status") or data.get("manifest_status") or data.get("manifestStatus")
    if status not in EXPECTED:
        return fail(f"unexpected status: {status!r}")
    text = REPORT.read_text(errors="replace")
    required = ["DUNVIEW.C", "DRAWVIEW.C", "COMMAND.C", status]
    missing = [item for item in required if item not in text]
    if missing:
        return fail("report missing required anchors/status: " + ",".join(missing))
    if "tmux" in text.lower() and "non-tmux" not in text.lower():
        return fail("report appears to rely on tmux")
    print(json.dumps({"status": "PASS", "manifest_status": status, "report": str(REPORT), "manifest": str(MANIFEST)}, indent=2))
    return 0

if __name__ == "__main__":
    sys.exit(main())
