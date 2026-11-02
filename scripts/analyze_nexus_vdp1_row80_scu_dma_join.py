#!/usr/bin/env python3
"""Join authentic Nexus row-80 VDP1 bytes to retail MENU.BPK and SCU DMA."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from analyze_nexus_vdp2_bitmap_source import menu_surfaces, verify_asset  # noqa: E402


MENU_BPK_SHA256 = "740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636"
MENU_ENTRY_144_SHA256 = "e118b99ae895bbc8333a60da25842e20bf4f1d9b7faa0675d005cca625b3a03f"
DMA_VDP1_COMMAND_TRACE_SHA256 = "581f7f5f16aceeeac25b4196e727e64e3f672b63df978c9a0325d349c0427f03"
DMA_VDP1_VRAM_SHA256 = "51189d6084620ee41da0787c1205f3c8d32dc627fa567008786d1ba89292a165"
DMA_DESCRIPTOR_TRACE_SHA256 = "95ceb01ea7d3cee523824f4d567924061e7856bab0ff7122b455c0fc86445acb"
DMA_PAYLOAD_TRACE_SHA256 = "fc8f8fb4fe610f32f008dbfe1b6a1e3c50884b4277169df9666d4cd3cbda5102"
DMA_DESCRIPTOR_BUNDLE_SHA256 = "d22de8806cea3bb2af6d14fc0f6a998ed6eb560fe7ec11c7c0c112c5b3624292"
DMA_PAYLOAD_BUNDLE_SHA256 = "4b51be86cf863b7e5a7be71190889a0796c577f9c3f1883d35189405395b6e40"
DMA_VDP1_BUNDLE_SHA256 = "20fab72d3f34235e8d5a647c82430801efeec2c720189f8eb99d879b9987276f"
FRAME300_COMMAND_TRACE_SHA256 = "5b898067f27b39710e4773ab027de202e9fc14e855e8eebca845f779c8f7875d"
FRAME300_VDP1_VRAM_SHA256 = "223ae7def1993ae4b62d0e180044a831d194fa772429b102ff4ce47ecdb55be0"
FRAME300_VDP1_BUNDLE_SHA256 = "7825022e760eecaec329b1a376106635815f84817afde1212f691a4c227c7965"

DMA_FILE_HASHES = {
    "scu-dma-trace.csv": DMA_DESCRIPTOR_TRACE_SHA256,
    "scu-dma-payload-trace.csv": DMA_PAYLOAD_TRACE_SHA256,
    "scu-dma-trace-bundle.txt": DMA_DESCRIPTOR_BUNDLE_SHA256,
    "scu-dma-payload-trace-bundle.txt": DMA_PAYLOAD_BUNDLE_SHA256,
    "vdp1-command-trace.csv": DMA_VDP1_COMMAND_TRACE_SHA256,
    "vdp1-vram.bin": DMA_VDP1_VRAM_SHA256,
    "vdp1-capture-bundle.txt": DMA_VDP1_BUNDLE_SHA256,
}
FRAME300_FILE_HASHES = {
    "vdp1-command-trace.csv": FRAME300_COMMAND_TRACE_SHA256,
    "vdp1-vram.bin": FRAME300_VDP1_VRAM_SHA256,
    "vdp1-capture-bundle.txt": FRAME300_VDP1_BUNDLE_SHA256,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def verify_files(dumps: Path, expected: dict[str, str]) -> dict[str, str]:
    hashes: dict[str, str] = {}
    for name, wanted in expected.items():
        data = (dumps / name).read_bytes()
        actual = sha256(data)
        require(actual == wanted, f"{name} SHA-256 mismatch: {actual}")
        hashes[name] = actual
    return hashes


def read_properties(path: Path) -> dict[str, str]:
    return dict(
        line.split("=", 1)
        for line in path.read_text(encoding="utf-8").splitlines()
        if "=" in line
    )


def verify_bundle(properties: dict[str, str], label: str) -> None:
    require(properties.get("trace_file_complete") == "1",
            f"{label} trace file is incomplete")
    require(properties.get("trace_complete") == "1",
            f"{label} trace is incomplete")
    require(properties.get("bundle_complete") == "1",
            f"{label} bundle is incomplete")
    require(properties.get("dropped_rows", "0") == "0",
            f"{label} trace dropped rows")


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def run(dma_dumps: Path, frame300_dumps: Path, menu_bpk_path: Path) -> dict[str, object]:
    dma_hashes = verify_files(dma_dumps, DMA_FILE_HASHES)
    frame300_hashes = verify_files(frame300_dumps, FRAME300_FILE_HASHES)

    menu_bpk = verify_asset(menu_bpk_path.read_bytes(), "MENU.BPK")
    require(sha256(menu_bpk) == MENU_BPK_SHA256, "Japanese retail MENU.BPK hash mismatch")
    surfaces, _palette = menu_surfaces(menu_bpk)
    source_surfaces = dict(surfaces)
    sprite = source_surfaces.get("MENU.BPK[144]")
    require(sprite is not None and len(sprite) == 16 * 41,
            "Japanese MENU.BPK entry 144 is not a 16x41 indexed surface")
    sprite_sha = sha256(sprite)
    require(sprite_sha == MENU_ENTRY_144_SHA256,
            "decoded Japanese MENU.BPK entry 144 hash mismatch")

    descriptor_bundle = read_properties(dma_dumps / "scu-dma-trace-bundle.txt")
    payload_bundle = read_properties(dma_dumps / "scu-dma-payload-trace-bundle.txt")
    vdp1_bundle = read_properties(dma_dumps / "vdp1-capture-bundle.txt")
    verify_bundle(descriptor_bundle, "SCU descriptor")
    verify_bundle(payload_bundle, "SCU payload")
    require(descriptor_bundle.get("capture_mode") == "full_history",
            "SCU descriptor history is not complete-history mode")
    require(int(descriptor_bundle.get("retained_rows", "-1")) == 6489,
            "unexpected SCU descriptor row count")
    require(payload_bundle.get("address_begin") == "0x5c64000" and
            payload_bundle.get("address_end_exclusive") == "0x5c65000",
            "SCU payload trace does not cover the row-80 destination")
    require(int(payload_bundle.get("retained_rows", "-1")) == 4096,
            "unexpected SCU payload row count")
    require(vdp1_bundle.get("trace_file_complete") == "1" and
            vdp1_bundle.get("trace_dropped_total") == "0" and
            vdp1_bundle.get("capture_frame_index") == "7663",
            "cold-boot VDP1 capture is incomplete or at a different frame")

    descriptor_rows = read_csv(dma_dumps / "scu-dma-trace.csv")
    descriptor_matches = [row for row in descriptor_rows if row["transfer"] == "3303"]
    require(len(descriptor_matches) == 1, "SCU transfer 3303 is missing or duplicated")
    descriptor = descriptor_matches[0]
    expected_descriptor = {
        "channel": "0",
        "source_address": "0x60c5e08",
        "destination_address": "0x5c643c0",
        "count": "1312",
        "source_increment": "4",
        "destination_increment": "2",
        "indirect": "0",
    }
    require(all(descriptor.get(key) == value
                for key, value in expected_descriptor.items()),
            "SCU transfer 3303 descriptor fields differ from the authentic receipt")

    payload_rows = [
        row for row in read_csv(dma_dumps / "scu-dma-payload-trace.csv")
        if row["transfer"] == "3303"
    ]
    require(len(payload_rows) == 656,
            "SCU transfer 3303 does not have all 656 captured 16-bit writes")
    destination = int(descriptor["destination_address"], 16)
    payload = bytearray()
    for index, row in enumerate(payload_rows):
        require(row["channel"] == "0" and int(row["width"]) == 2,
                f"unexpected transfer-3303 payload shape at write {index}")
        require(int(row["destination_address"], 16) == destination + len(payload),
                f"transfer-3303 payload is not contiguous at write {index}")
        payload.extend(int(row["value"], 16).to_bytes(2, "big"))
    require(len(payload) == int(descriptor["count"]),
            "transfer-3303 payload byte count differs from its descriptor")

    vdp1_vram = (dma_dumps / "vdp1-vram.bin").read_bytes()
    vram_offset = destination - 0x05C00000
    require(len(vdp1_vram) == 0x80000 and
            vdp1_vram[vram_offset:vram_offset + len(payload)] == payload,
            "transfer-3303 payload differs from its same-run final VDP1 VRAM bytes")
    require(payload[:len(sprite)] == sprite,
            "transfer-3303 first 656 bytes differ from decoded MENU.BPK entry 144")

    frame300_bundle = read_properties(frame300_dumps / "vdp1-capture-bundle.txt")
    require(frame300_bundle.get("trace_file_complete") == "1" and
            frame300_bundle.get("trace_dropped_total") == "0" and
            frame300_bundle.get("capture_frame_index") == "300" and
            frame300_bundle.get("trace_rows_for_capture_frame") == "83",
            "frame-300 VDP1 capture bundle is incomplete or at a different capture frame")
    frame300_trace = read_csv(frame300_dumps / "vdp1-command-trace.csv")
    frame300_row80 = [
        row for row in frame300_trace
        if row["frame"] == "300" and row["index"] == "80"
    ]
    require(len(frame300_row80) == 1,
            "frame-300 command row 80 is missing or duplicated")
    command = frame300_row80[0]
    require(int(command["word0"], 16) == 0x1000 and
            int(command["word2"], 16) == 0x00A0 and
            int(command["word4"], 16) == 0xC878 and
            int(command["word5"], 16) == 0x0229,
            "frame-300 command row 80 does not match the authenticated mode-4 sprite")
    command_source = int(command["word4"], 16) * 8
    require(command_source == vram_offset,
            "frame-300 command row 80 source address does not map to DMA destination")
    frame300_vram = (frame300_dumps / "vdp1-vram.bin").read_bytes()
    require(len(frame300_vram) == 0x80000 and
            frame300_vram[vram_offset:vram_offset + len(sprite)] == sprite,
            "frame-300 VDP1 bytes differ from MENU.BPK entry 144")

    coldboot_commands = read_csv(dma_dumps / "vdp1-command-trace.csv")
    coldboot_reads = sum(
        int(row["word4"], 16) == int(command["word4"], 16)
        and (int(row["word0"], 16) & 0x8000) == 0
        for row in coldboot_commands
    )
    require(coldboot_reads == 0,
            "cold-boot command trace now reads row-80 CMDSRCA; re-evaluate capture scope")

    return {
        "result": "PASS",
        "retail_asset": {
            "name": "Japanese MENU.BPK[144]",
            "archive_sha256": MENU_BPK_SHA256,
            "decoded_bytes": len(sprite),
            "decoded_sha256": sprite_sha,
        },
        "scu_dma_capture": {
            "transfer": 3303,
            "source_address": descriptor["source_address"],
            "destination_address": descriptor["destination_address"],
            "descriptor_bytes": int(descriptor["count"]),
            "captured_16bit_writes": len(payload_rows),
            "full_payload_sha256": sha256(payload),
            "same_run_final_vram_match": True,
            "first_656_bytes_match_menu_bpk_144": True,
            "vdp1_snapshot_frame": int(vdp1_bundle["capture_frame_index"]),
            "descriptor_trace_sha256": dma_hashes["scu-dma-trace.csv"],
            "payload_trace_sha256": dma_hashes["scu-dma-payload-trace.csv"],
        },
        "frame300_draw_capture": {
            "command_frame": 300,
            "command_index": 80,
            "pmod": command["word2"],
            "cmdsrca": command["word4"],
            "source_byte_address": f"0x{command_source:x}",
            "vdp1_texture_matches_menu_bpk_144": True,
            "same_run_dma_command_source_reads_in_coldboot_capture": coldboot_reads,
            "dma_and_draw_same_run_join": False,
            "trace_sha256": frame300_hashes["vdp1-command-trace.csv"],
        },
        "scope": "The authentic DMA payload and frame-300 draw are joined across separate runs by exact retail bytes and VDP1 address. This does not prove same-frame DMA timing/consumption, HUD meaning, or production-renderer admission.",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dma-dumps", type=Path, required=True)
    parser.add_argument("--frame300-dumps", type=Path, required=True)
    parser.add_argument("--menu-bpk", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = run(args.dma_dumps, args.frame300_dumps, args.menu_bpk)
    except (OSError, ValueError, KeyError, StopIteration) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
