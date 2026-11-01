#!/usr/bin/env python3
"""Verify authentic Nexus SCU-DMA payloads against captured WRAM and VDP2 VRAM."""
from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path


def read_manifest(path: Path) -> dict[str, str]:
    return dict(
        line.split("=", 1)
        for line in path.read_text(encoding="utf-8").splitlines()
        if "=" in line and not line.startswith("format=")
    )


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dumps", type=Path, help="Ymir capture dumps directory")
    args = parser.parse_args()
    root = args.dumps

    payload_manifest = read_manifest(root / "scu-dma-payload-trace-bundle.txt")
    dma_manifest = read_manifest(root / "scu-dma-trace-bundle.txt")
    require(payload_manifest.get("trace_complete") == "1", "SCU payload trace is incomplete")
    require(payload_manifest.get("transfer_trace_complete") == "1", "SCU descriptor trace is incomplete")
    require(payload_manifest.get("dropped_rows") == "0", "SCU payload trace dropped rows")
    require(dma_manifest.get("trace_complete") == "1", "SCU descriptor history is incomplete")
    require(dma_manifest.get("dropped_rows") == "0", "SCU descriptor history dropped rows")

    descriptors = {
        row["transfer"]: row
        for row in csv.DictReader((root / "scu-dma-trace.csv").open(newline=""))
    }
    payload_rows = list(csv.DictReader((root / "scu-dma-payload-trace.csv").open(newline="")))
    target_begin = int(payload_manifest["address_begin"], 16)
    target_end = int(payload_manifest["address_end_exclusive"], 16)
    target_descriptors = [
        row
        for row in descriptors.values()
        if int(row["destination_address"], 16) < target_end
        and int(row["destination_address"], 16) + int(row["count"]) > target_begin
    ]
    target_descriptors.sort(key=lambda row: int(row["transfer"]))
    require(len(target_descriptors) == 4, f"expected 4 DMA descriptors for interval, got {len(target_descriptors)}")

    wram = (root / "wram-hi.bin").read_bytes()
    vram = (root / "vdp2-vram.bin").read_bytes()
    vram_bus_base = 0x05E00000
    wram_bus_base = 0x06000000
    verified = []
    for descriptor in target_descriptors:
        transfer = descriptor["transfer"]
        source = int(descriptor["source_address"], 16)
        destination = int(descriptor["destination_address"], 16)
        count = int(descriptor["count"])
        require(descriptor["channel"] == "0", f"unexpected channel for transfer {transfer}")
        require(descriptor["source_increment"] == "4", f"unexpected source increment for transfer {transfer}")
        require(descriptor["destination_increment"] == "2", f"unexpected destination increment for transfer {transfer}")
        require(descriptor["indirect"] == "0", f"unexpected indirect transfer {transfer}")
        require(target_begin <= destination and destination + count <= target_end,
                f"transfer {transfer} exceeds the captured interval")

        writes = [row for row in payload_rows if row["transfer"] == transfer]
        require(len(writes) == count // 2, f"transfer {transfer}: unexpected payload row count")
        payload = bytearray()
        for row in writes:
            address = int(row["destination_address"], 16)
            width = int(row["width"])
            require(width == 2, f"transfer {transfer}: expected 16-bit bus write")
            require(address == destination + len(payload), f"transfer {transfer}: noncontiguous destination writes")
            payload.extend(int(row["value"], 16).to_bytes(width, "big"))
        require(len(payload) == count, f"transfer {transfer}: payload byte count mismatch")

        wram_offset = source - wram_bus_base
        source_bytes = wram[wram_offset : wram_offset + count]
        require(len(source_bytes) == count, f"transfer {transfer}: source outside captured WRAM")
        require(payload == source_bytes, f"transfer {transfer}: payload does not match captured WRAM bytes")
        verified.append((transfer, destination, payload))
        print(f"transfer={transfer} bytes={count} payload=WRAM exact sha256={hashlib.sha256(payload).hexdigest()}")

    final_transfer, final_destination, final_payload = verified[-1]
    vram_offset = final_destination - vram_bus_base
    final_snapshot = vram[vram_offset : vram_offset + len(final_payload)]
    require(final_snapshot == final_payload, "last SCU payload does not match the final VDP2 VRAM snapshot")
    print(f"last_target_transfer={final_transfer} final_vram_match_bytes={len(final_payload)} "
          f"sha256={hashlib.sha256(final_payload).hexdigest()}")

    cpu_manifest_path = root / "sh2-vdp2-write-trace-bundle.txt"
    if cpu_manifest_path.exists():
        cpu_manifest = read_manifest(cpu_manifest_path)
        dropped = int(cpu_manifest.get("master_dropped", "0")) + int(cpu_manifest.get("slave_dropped", "0"))
        if dropped:
            print(f"LIMIT: SH-2 VDP2-write trace dropped {dropped} rows; exclusive final-writer attribution is unproven.")
    print("PASS: SCU descriptor and payload records match captured WRAM; last recorded payload matches captured VDP2 VRAM.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
