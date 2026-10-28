#!/usr/bin/env python3
"""Validate the upstream Mednafen hooks needed for a future $23AD trace.

This deliberately makes no source edits and does not claim a runtime capture.
It checks that the selected upstream revision exposes all required hook sites,
including direct Page 1 accesses which bypass RdMem/WrMem.
"""

from pathlib import Path
import hashlib
import sys


REQUIRED = {
    "src/pce_fast/huc6280.cpp": (
        "#define HU_Page1\t\tPage1_local",
        "#define LOAD_LOCALS()",
        "#define SAVE_LOCALS()",
        "#define RdAtPC() RdOp(HU_PC)",
        "static INLINE uint8 RdMem(unsigned int A)",
        "static INLINE void WrMem(unsigned int A, uint8 V)",
        "static INLINE uint8 RdOp(unsigned int A)",
        "#define SET_MPR(arg_i, arg_v)",
        "#define TAM     for(int i = 0; i < 8; i ++) {",
        "\tGetOutBMT:",
        "\tSAVE_LOCALS();",
        "FixPC_PC();",
        "PUSH_PC();",
        "#define POP_PC_AP()",
    ),
    "src/pce_fast/huc6280_ops.inc": (
        "case 0x53: // TAMi",
        "HU_A = HU_Page1[HU_X];",
        "HU_Page1[HU_X] /*PAGE1_W[HU_X]*/ =  HU_A;",
    ),
    "configure.ac": ("MEDNAFEN_VERSION='1.32.1'",),
}


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit(f"usage: {Path(sys.argv[0]).name} MEDNAFEN_SOURCE")
    root = Path(sys.argv[1]).resolve()
    digests: dict[str, str] = {}
    failures: list[str] = []
    for relative, anchors in REQUIRED.items():
        path = root / relative
        if not path.is_file():
            failures.append(f"missing {relative}")
            continue
        raw = path.read_bytes()
        text = raw.decode("utf-8", errors="strict")
        digests[relative] = hashlib.sha256(raw).hexdigest()
        for anchor in anchors:
            if anchor not in text:
                failures.append(f"missing anchor in {relative}: {anchor!r}")
    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        raise SystemExit(1)
    for relative, digest in digests.items():
        print(f"source_sha256 {relative} {digest}")
    print("PASS: Mednafen 1.32.1 contains the required HuC6280 trace hook sites")
    print("LIMIT: source inspection only; no instrumentation compile or runtime trace")


if __name__ == "__main__":
    main()
