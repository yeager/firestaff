#!/usr/bin/env python3
"""Record source-bound SH-2 facts from the retail SLEV00..15 corpus.

This is deliberately a static receipt.  It verifies the authenticated retail
bytes, counts unambiguous SH-2 instruction forms, and reports literal loads
whose values land in the observed Saturn address corridors.  It does not
assign an event ABI, callback meaning, sound selector, or dispatch permission.
"""

from __future__ import annotations

import argparse
import hashlib
import runpy
from pathlib import Path

from analyze_nexus_title_vdp2_source import cue_track1, iso_members_in_memory


LEVEL_COUNT = 16
HEADER_BYTES = 36
HARDWARE_RANGES = (
    (0x25000000, 0x2501FFFF),
    (0x26000000, 0x2601FFFF),
)


def is_hardware_literal(value: int) -> bool:
    return any(lo <= value <= hi for lo, hi in HARDWARE_RANGES)


def scan(data: bytes) -> dict[str, object]:
    if len(data) < HEADER_BYTES or len(data) & 1:
        raise ValueError("SLEV payload is shorter than the header or not word aligned")
    literal_rows: list[tuple[int, int, int]] = []
    rts = jsr = branches = stores = immediates = 0
    for offset in range(0, len(data) - 1, 2):
        word = int.from_bytes(data[offset : offset + 2], "big")
        if word == 0x000B:
            rts += 1
        if (word & 0xF0FF) == 0x400B:
            jsr += 1
        if (word >> 12) in (0xA, 0xB):
            branches += 1
        if word >> 12 == 0xE:
            immediates += 1
        if (word & 0xF00F) in (0x2000, 0x2001, 0x2002,
                               0x1000, 0x1001, 0x1002):
            stores += 1
        if word >> 12 != 0xD:
            continue
        literal_offset = ((offset + 4) & ~3) + (word & 0xFF) * 4
        if literal_offset + 4 > len(data):
            continue
        value = int.from_bytes(data[literal_offset : literal_offset + 4], "big")
        literal_rows.append((offset, literal_offset, value))
    hardware = sorted({value for _, _, value in literal_rows
                       if is_hardware_literal(value)})
    ram_literals = sorted({value for _, _, value in literal_rows
                           if 0x00200000 <= value <= 0x003FFFFF})
    return {
        "first_opcode": int.from_bytes(data[:2], "big"),
        "rts": rts,
        "jsr": jsr,
        "branches": branches,
        "immediates": immediates,
        "stores": stores,
        "pc_loads": len(literal_rows),
        "ram_literals": ram_literals,
        "hardware_literals": hardware,
        "hardware_rows": [(instruction, literal, value)
                          for instruction, literal, value in literal_rows
                          if is_hardware_literal(value)],
    }


def read_corpus(data_dir: Path | None, cue: Path | None) -> dict[str, bytes]:
    """Read the authenticated SLEV corpus without extracting a retail disc."""
    names = {f"SLEV{level:02d}.BIN" for level in range(LEVEL_COUNT)}
    if cue is not None:
        return iso_members_in_memory(cue_track1(cue), names)
    if data_dir is None:
        raise ValueError("either --data-dir or --cue is required")
    return {name: (data_dir / name).read_bytes() for name in names}


def main() -> int:
    parser = argparse.ArgumentParser()
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--data-dir", type=Path)
    source.add_argument("--cue", type=Path,
                        help="retail CUE; requested ISO members stay in memory")
    args = parser.parse_args()
    fixture = Path(__file__).parent / "fixtures" / "nexus_v1_disc_file_hashes.py"
    expected = runpy.run_path(str(fixture))["DISC_HASH"]
    try:
        corpus = read_corpus(args.data_dir, args.cue)
    except (OSError, ValueError) as exc:
        raise SystemExit(f"SLEV_CORPUS_INVALID: {exc}") from exc
    total_bytes = 0
    total_rts = total_jsr = total_branches = total_stores = total_pc_loads = 0
    total_immediates = 0
    hardware_rows = 0
    for level in range(LEVEL_COUNT):
        name = f"SLEV{level:02d}.BIN"
        data = corpus[name]
        digest = hashlib.sha256(data).hexdigest()
        if digest != expected[name]:
            raise SystemExit(f"{name}: SHA-256 mismatch: {digest}")
        facts = scan(data)
        total_bytes += len(data)
        total_rts += int(facts["rts"])
        total_jsr += int(facts["jsr"])
        total_branches += int(facts["branches"])
        total_immediates += int(facts["immediates"])
        total_stores += int(facts["stores"])
        total_pc_loads += int(facts["pc_loads"])
        hardware_rows += len(facts["hardware_rows"])
        hw = ",".join(f"0x{v:08x}" for v in facts["hardware_literals"]) or "none"
        hw_rows = ",".join(
            f"0x{instruction:04x}->0x{literal:04x}=0x{value:08x}"
            for instruction, literal, value in facts["hardware_rows"]
        ) or "none"
        print(
            f"{name} sha256={digest} size={len(data)} "
            f"first_opcode=0x{int(facts['first_opcode']):04x} "
            f"rts={facts['rts']} jsr={facts['jsr']} branches={facts['branches']} "
            f"immediates={facts['immediates']} "
            f"stores={facts['stores']} pc_relative_loads={facts['pc_loads']} "
            f"hardware_literals={hw} hardware_rows={hw_rows}"
        )
    print(
        f"SLEV_CORPUS total_bytes={total_bytes} levels={LEVEL_COUNT} "
        f"rts={total_rts} jsr={total_jsr} branches={total_branches} "
        f"immediates={total_immediates} "
        f"stores={total_stores} pc_relative_loads={total_pc_loads} "
        f"hardware_literal_rows={hardware_rows}"
    )
    print("semantic_admission=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
