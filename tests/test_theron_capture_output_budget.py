#!/usr/bin/env python3
"""Exercise the aggregate live-capture output budget watchdog."""

from __future__ import annotations

import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path


def main() -> int:
    repo = Path(__file__).resolve().parents[1]
    watcher = repo / "scripts" / "watch_theron_capture_output_budget.py"
    with tempfile.TemporaryDirectory(prefix="theron-output-budget-") as temp:
        root = Path(temp)
        trace = root / "capture.trace"
        marker = root / "capture.output-budget-exceeded"
        trace.write_bytes(b"")
        capture = subprocess.Popen(
            [sys.executable, "-c", "import time; time.sleep(30)"],
            start_new_session=True,
        )
        guard = subprocess.Popen(
            [
                sys.executable,
                str(watcher),
                "--pid",
                str(capture.pid),
                "--limit-bytes",
                "16",
                "--marker",
                str(marker),
                "--poll-ms",
                "20",
                "--directory",
                str(root),
                "--prefix",
                "capture",
                "--path",
                str(trace),
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            time.sleep(0.08)
            trace.write_bytes(b"x" * 17)
            _, stderr = guard.communicate(timeout=5)
            if guard.returncode != 2:
                raise AssertionError(f"watchdog returned {guard.returncode}: {stderr}")
            if "17 > 16 bytes" not in stderr:
                raise AssertionError(f"watchdog diagnostic omitted observed byte count: {stderr}")
            if not marker.is_file():
                raise AssertionError("watchdog did not preserve a budget-exceeded receipt")
            receipt = marker.read_text(encoding="utf-8")
            if "status=exceeded" not in receipt or "observed_bytes=17" not in receipt:
                raise AssertionError(f"unexpected budget receipt: {receipt}")
            capture.wait(timeout=3)
            if capture.returncode == 0:
                raise AssertionError("budget watchdog did not terminate the capture process group")
        finally:
            if guard.poll() is None:
                guard.terminate()
                guard.wait(timeout=2)
            if capture.poll() is None:
                os.killpg(capture.pid, 9)
                capture.wait(timeout=2)
    print("PASS: aggregate Theron capture output budget stops only its process group")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
