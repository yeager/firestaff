#!/usr/bin/env python3
"""Exercise trace consistency validation using media-derived synthetic rows."""

from __future__ import annotations

import importlib.util
import os
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "theron_opcode_provenance",
    ROOT / "scripts/verify_theron_stage2_opcode_provenance.py",
)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


FETCH_PCS = (0x4002, 0x4009, 0x4010, 0x4011, 0x4014, 0x4016,
             0x4018, 0x401B, 0x401C, 0x401E, 0x4010, 0x4002)

def make_trace(raw: bytes) -> str:
    rows = [MODULE.TRACE_HEADER]
    for index, pc in enumerate(FETCH_PCS):
        pc_offset = pc - MODULE.STAGE2_PC
        user_offset = MODULE.STAGE2_USER_OFFSET["JP"] + pc_offset
        value = MODULE.raw_source_span(raw, "JP", MODULE.STAGE2_LBA["JP"], user_offset, 1)[0]
        lba = 4_521 + user_offset // 2_048
        raw_offset = 16 + user_offset % 2_048
        physical_pc = MODULE.PHYSICAL_STAGE2_BASE + pc_offset
        rows.append(
            "opcode_origin_fetch "
            f"fetch_sequence={index} generation=9 logical_pc={pc:04x} "
            f"physical_pc={physical_pc:06x} mpr_slot={(pc >> 13) & 7} mpr_bank=80 "
            f"opcode={value:02x} source_lba={lba} "
            f"source_user_offset={user_offset} source_raw_offset={raw_offset} "
            f"source_value={value:02x}"
        )
    return "\n".join(rows) + "\n"


def expect_rejected(trace: str, track02: Path, label: str) -> None:
    path = TEST_ROOT / f"{label}.trace"
    path.write_text(trace, encoding="ascii")
    try:
        MODULE.verify(path, track02, "JP")
    except ValueError:
        return
    raise AssertionError(f"mutation was accepted: {label}")


def main() -> int:
    data_dir = Path(
        os.environ.get("FIRESTAFF_THERON_TEST_DATA_DIR", Path.home() / ".firestaff/data/theron")
    )
    track02 = data_dir / "TQJP02.bin"
    if not track02.is_file():
        print("SKIP: authentic JP Theron Track 02 is unavailable")
        return 77
    raw = track02.read_bytes()
    trace = make_trace(raw)
    TEST_ROOT.mkdir(parents=True, exist_ok=False)
    valid_trace = TEST_ROOT / "valid.trace"
    valid_trace.write_text(trace, encoding="ascii")
    try:
        accepted = MODULE.verify(valid_trace, track02, "JP")
        if len(accepted) != len(FETCH_PCS):
            raise AssertionError("valid branch/repeat provenance contract has wrong length")
        if accepted[0] != accepted[-1] or accepted[2] != accepted[10]:
            raise AssertionError("repeated PCs did not preserve their source bytes")
        rows = trace.splitlines()
        mutations = (
            (rows[:1] + rows[2:], "missing-fetch"),
            (rows + [rows[-1]], "duplicate-fetch"),
        )
        for altered_rows, label in mutations:
            expect_rejected("\n".join(altered_rows) + "\n", track02, label)
        row_mutations = {
            "wrong-generation": ("generation=9", "generation=10"),
            "wrong-lba": ("source_lba=4521", "source_lba=4522"),
            "wrong-user-offset": ("source_user_offset=2", "source_user_offset=3"),
            "wrong-raw-offset": ("source_raw_offset=18", "source_raw_offset=19"),
            "wrong-value": ("source_value=" + rows[1].split("source_value=")[1].split()[0], "source_value=ff"),
            "wrong-logical-pc": ("logical_pc=4002", "logical_pc=4003"),
            "wrong-physical-pc": ("physical_pc=100002", "physical_pc=100003"),
            "wrong-mpr-slot": ("mpr_slot=2", "mpr_slot=3"),
            "wrong-mpr-bank": ("mpr_bank=80", "mpr_bank=68"),
        }
        for label, (before, after) in row_mutations.items():
            if before not in rows[1]:
                raise AssertionError(f"self-test source row lacks mutation anchor {before}")
            changed = rows.copy()
            changed[1] = changed[1].replace(before, after, 1)
            expect_rejected("\n".join(changed) + "\n", track02, label)
    finally:
        for path in TEST_ROOT.iterdir():
            path.unlink()
        TEST_ROOT.rmdir()
    print("PASS: checker accepts branch/repeat rows and rejects missing, duplicate, and altered fields")
    print("LIMIT: rows are synthetic contract-test inputs, not emulator, opcode-origin, or gameplay evidence")
    return 0


TEST_ROOT = Path.home() / ".cache" / f"firestaff-theron-opcode-provenance-test-{os.getpid()}"


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, UnicodeError, ValueError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
