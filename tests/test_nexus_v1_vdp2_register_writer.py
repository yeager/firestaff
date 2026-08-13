#!/usr/bin/env python3
"""Regression test for VDP2 register/source witness parsing."""

from __future__ import annotations

import contextlib
import io
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import analyze_nexus_vdp2_register_writer as analyzer  # noqa: E402


def main() -> int:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        writes = root / "writes.trace"
        registers = root / "registers.trace"
        writes.write_text(
            "\n".join([
                "area=vram addr=0x000000 size=2 value=0x0000 "
                "pc0=0x06011860 pc1=0x00000000",
            ] * 64) + "\n",
            encoding="ascii",
        )
        registers.write_text(
            "frame=65 addr=0x000000 pc=0x06011860 "
            "r4=0x25e00000 r5=0x00000000 "
            "src_r4=0x25e00000 src_words=1234,5678,9abc,def0,0001,0002,0003,0004 "
            "src_r5=0x00000000 src5_words=2000,0200,0600,2000,2000,0200,0600,2000\n",
            encoding="ascii",
        )
        old_argv = sys.argv
        output = io.StringIO()
        try:
            sys.argv = [
                str(ROOT / "scripts/analyze_nexus_vdp2_register_writer.py"),
                str(writes), str(registers), "--writer-pc", "0x06011860",
                "--destination", "0x0", "--minimum-writes", "64",
            ]
            with contextlib.redirect_stdout(output):
                result = analyzer.main()
        finally:
            sys.argv = old_argv
        text = output.getvalue()
        if result != 0 or "source_bytes_capture=verified" not in text:
            raise SystemExit(f"unexpected analyzer result:\n{text}")
    print("nexus vdp2 register writer source suffix: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
