#!/usr/bin/env python3
"""Check placement and idempotence of the PCE Fast status-read hook."""

from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(__file__).resolve().parents[1]
instrumenter = repo / "scripts/instrument_mednafen_theron_syscard_status_trace.py"


def main() -> None:
    build = repo / "build"
    build.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="theron-status-instrument-", dir=build) as temp:
        source = Path(temp) / "src/pce_fast/pcecd.cpp"
        source.parent.mkdir(parents=True)
        source.write_text(
            "MDFN_FASTCALL uint8 PCECD_Read(uint32 timestamp, uint32 A)\n"
            "{\n uint8 ret = 0;\n return(ret);\n}\n"
        )
        subprocess.run([sys.executable, str(instrumenter), temp], check=True)
        text = source.read_text()
        if text.count("FIRESTAFF_THERON_SYSTEM_CARD_STATUS_TRACE") != 1:
            raise AssertionError("status trace hook was not inserted exactly once")
        if text.index("status_read index=%u") > text.index(" return(ret);"):
            raise AssertionError("status trace hook was inserted after the function return")
        result = subprocess.run([sys.executable, str(instrumenter), temp], capture_output=True, text=True)
        if result.returncode == 0 or "already instrumented" not in result.stderr:
            raise AssertionError("instrumenter did not reject duplicate application")
    print("test_instrument_mednafen_theron_syscard_status_trace: PASS")


if __name__ == "__main__":
    main()
