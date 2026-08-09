#!/usr/bin/env python3
"""Inspect a raw NXSLSC01 SH-2 write capture without assigning semantics."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

MAGIC = b"NXSLSC01"
HEADER_BYTES = 96
RECORD_BYTES = 16
SH2_LO = 0x06000000
SH2_HI = 0x08000000


def fnv1a64(data: bytes) -> int:
    value = 0xCBF29CE484222325
    for byte in data:
        value = ((value ^ byte) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("capture", type=Path)
    parser.add_argument("--data-dir", type=Path)
    parser.add_argument("--level", type=int, default=0)
    args = parser.parse_args()
    raw = args.capture.read_bytes()
    if len(raw) < HEADER_BYTES or raw[:8] != MAGIC:
        raise SystemExit("NEXUS_SLEV_CAPTURE_INVALID: header")
    version, header_bytes = struct.unpack_from(">II", raw, 8)
    if version != 1 or header_bytes != HEADER_BYTES:
        raise SystemExit("NEXUS_SLEV_CAPTURE_INVALID: version/header-size")
    route_epoch, package, card, task_trace, sal, map_table, driver = struct.unpack_from(
        ">7Q", raw, 16
    )
    payload_offset, payload_length = struct.unpack_from(">II", raw, 72)
    payload_hash, task_source = struct.unpack_from(">2Q", raw, 80)
    if payload_offset != HEADER_BYTES or payload_length != len(raw) - HEADER_BYTES:
        raise SystemExit("NEXUS_SLEV_CAPTURE_INVALID: payload-bounds")
    if payload_length == 0 or payload_length % RECORD_BYTES:
        raise SystemExit("NEXUS_SLEV_CAPTURE_INVALID: record-alignment")
    payload = raw[payload_offset:]
    if fnv1a64(payload) != payload_hash:
        raise SystemExit("NEXUS_SLEV_CAPTURE_INVALID: payload-hash")
    records = [struct.unpack_from(">4I", payload, i)
               for i in range(0, len(payload), RECORD_BYTES)]
    in_sh2 = sum(SH2_LO <= address < SH2_HI for address, *_ in records)
    pcs = sorted({pc for _, _, pc, _ in records if pc})
    print(f"capture_header_valid=1 route_epoch={route_epoch}")
    print(f"payload_bytes={payload_length} records={len(records)}")
    print(f"sh2_write_records={in_sh2} non_sh2_records={len(records) - in_sh2}")
    print("first_record=" + "/".join(f"0x{x:08x}" for x in records[0]))
    print(f"nonzero_pc_count={len(pcs)}")
    matches = {}
    if args.data_dir:
        files = {
            "task_source": args.data_dir / f"SLEV{args.level:02d}.BIN",
            "sal_descriptor": args.data_dir / f"SNDLEV{args.level:02d}.SAL",
            "map_table": args.data_dir / f"SNDLEV{args.level:02d}.MAP",
            "sddrvs": args.data_dir / "SDDRVS.TSK",
        }
        expected = {name: fnv1a64(path.read_bytes()) for name, path in files.items()}
        observed = {"task_source": task_source, "sal_descriptor": sal,
                    "map_table": map_table, "sddrvs": driver}
        matches = {name: int(observed[name] == expected[name]) for name in expected}
        print("retail_fnv_matches=" + ",".join(
            f"{name}:{matches[name]}" for name in matches))
    bound = bool(in_sh2 == len(records) and matches and all(matches.values()))
    print(f"provenance_bound={int(bound)}")
    print("event_selector_semantics=unproven")
    print("sal_codec=unproven")
    print("host_playback=blocked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
