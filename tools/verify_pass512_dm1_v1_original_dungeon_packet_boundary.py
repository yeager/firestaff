#!/usr/bin/env python3
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass512_dm1_v1_original_dungeon_packet_boundary"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

DM_ROOT = Path(os.environ.get("FIRESTAFF_ORIGINAL_DM_ROOT", str(Path.home() / ".openclaw/data/firestaff-original-games/DM")))
CANONICAL_DUNGEON = DM_ROOT / "_canonical/dm1/DUNGEON.DAT"
EXTRACTED_PC34_DUNGEON = DM_ROOT / "_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT"
GREATSTONE_ATLAS = Path(os.environ.get("FIRESTAFF_GREATSTONE_ATLAS", str(Path.home() / ".openclaw/data/firestaff-greatstone-atlas")))
REDMCSB = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))

EXPECTED_SHA256 = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
EXPECTED_MD5 = "766450c940651fc021c92fe5d0d0b3a6"
EXPECTED_SIZE = 33357
EXPECTED_PAYLOAD_BYTES = 33355
EXPECTED_CHECKSUM = 0x2A01
EXPECTED_RAW_MAP_OFFSET = 21072
EXPECTED_COLUMN_COUNT = 409
EXPECTED_THING_COUNTS = [170, 179, 125, 684, 182, 107, 121, 35, 56, 12, 280, 0, 0, 0, 0, 0]
THING_DATA_BYTE_COUNTS = [4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4]


@dataclass(frozen=True)
class SourceLock:
    id: str
    file: str
    lines: str
    needles: tuple[str, ...]
    claim: str


SOURCE_LOCKS = (
    SourceLock("dungeon_header_declares_packet_counts", "DEFS.H", "989-998", ("unsigned int16_t RawMapDataByteCount;", "unsigned char MapCount;", "unsigned int16_t TextDataWordCount;", "unsigned int16_t SquareFirstThingCount;", "unsigned int16_t ThingCount[16];"), "The original packet sizes are driven by the DUNGEON_HEADER fields."),
    SourceLock("map_record_is_sixteen_pc34_bytes", "DEFS.H", "1049-1116", ("typedef struct {", "unsigned int16_t RawMapDataByteOffset;", "unsigned char OffsetMapX;", "unsigned char OffsetMapY;", "} MAP;"), "For PC34/I34E, each MAP record is 16 bytes and supplies the width used to count columns."),
    SourceLock("thing_record_byte_counts", "DUNGEON.C", "45-61", ("unsigned char G0235_auc_Graphic559_ThingDataByteCount[16] = {", "4,   /* Door */", "6,   /* Teleporter */", "16,  /* Group */", "8,   /* Projectile */"), "Thing-data packet lengths are ThingCount[type] multiplied by the ReDMCSB byte-count table."),
    SourceLock("loader_reads_packet_order_and_final_checksum", "CEDTINCA.C", "17-113", ("F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.Header", "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.Maps", "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.ColumnsCumulativeSquareThingCount", "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.SquareFirstThings", "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.TextData", "P3584_ps_->Dungeon.ThingData[L3964_i_]", "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.RawMapData", "L3968_i_ == L3969_i_"), "ReDMCSB reads header, maps, cumulative columns, square-first-things, text, 16 thing packets, raw map data, then compares the appended checksum."),
    SourceLock("checksum_is_unsigned_byte_sum", "CEDTINC6.C", "129-155", ("F7059_ReadDungeonPartWithChecksum", "while (P3721_ui_ByteCount--) {", "L3962_ui_ += *P3719_pc_++;", "*P3720_pi_Checksum += L3962_ui_;", "return 1;"), "The packet checksum is the 16-bit running sum of all payload bytes."),
    SourceLock("runtime_loader_preserves_same_boundary_before_new_game_expansion", "LOADSAVE.C", "1928-2088", ("F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful(M774_CAST_PUC(G0278_ps_DungeonHeader)", "F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful(M774_CAST_PUC(G0277_ps_DungeonMaps)", "G0282_ui_DungeonColumnCount = AL1355_ui_ColumnCount;", "G0278_ps_DungeonHeader->SquareFirstThingCount += 300;", "F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful(M774_CAST_PUC(G0283_pT_SquareFirstThings)", "F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful(M774_CAST_PUC(G0260_pui_DungeonTextData)", "F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful(G0284_apuc_ThingData[AL1353_i_ThingType]", "F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful(G0276_puc_DungeonRawMapData", "L1359_i_ExpectedChecksum != L1360_i_RunningChecksum"), "Runtime loading uses the same checksum packet boundary, then expands new-game runtime pools after reading original packet counts."),
)


def norm(text: str) -> str:
    return " ".join(text.split())


def line_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(value) for value in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1 : end]))
    return "\n".join(chunks)


def sha(path: Path, algorithm: str) -> str:
    h = hashlib.new(algorithm)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def le16(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


def parse_header(data: bytes) -> dict[str, Any]:
    return {"ornamentRandomSeed": le16(data, 0), "rawMapDataByteCount": le16(data, 2), "mapCount": data[4], "textDataWordCount": le16(data, 6), "initialPartyLocationRaw": f"0x{le16(data, 8):04X}", "squareFirstThingCount": le16(data, 10), "thingCounts": [le16(data, 12 + index * 2) for index in range(16)]}


def map_width(data: bytes, map_index: int) -> int:
    packed = le16(data, 44 + map_index * 16 + 8)
    return ((packed >> 6) & 31) + 1


def map_height(data: bytes, map_index: int) -> int:
    packed = le16(data, 44 + map_index * 16 + 8)
    return ((packed >> 11) & 31) + 1


def packet_rows(data: bytes) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    header = parse_header(data)
    offset = 0
    rows: list[dict[str, Any]] = []

    def add(name: str, byte_count: int) -> None:
        nonlocal offset
        packet = data[offset : offset + byte_count]
        rows.append({"name": name, "offset": offset, "bytes": byte_count, "endOffset": offset + byte_count, "byteSum16": sum(packet) & 0xFFFF, "sha256": hashlib.sha256(packet).hexdigest()})
        offset += byte_count

    add("header", 44)
    add("maps", header["mapCount"] * 16)
    column_count = sum(map_width(data, index) for index in range(header["mapCount"]))
    add("columns_cumulative_square_thing_count", column_count * 2)
    add("square_first_things", header["squareFirstThingCount"] * 2)
    add("text_data", header["textDataWordCount"] * 2)
    for thing_type, count in enumerate(header["thingCounts"]):
        add(f"thing_data_{thing_type:02d}", count * THING_DATA_BYTE_COUNTS[thing_type])
    add("raw_map_data", header["rawMapDataByteCount"])
    facts = {"header": header, "map0": {"width": map_width(data, 0), "height": map_height(data, 0), "rawMapDataByteOffset": le16(data, 44)}, "columnCount": column_count, "payloadBytes": offset, "expectedChecksumOffset": offset, "expectedChecksumLe16": le16(data, offset), "calculatedChecksum16": sum(data[:offset]) & 0xFFFF, "trailingBytesAfterChecksum": len(data) - offset - 2}
    return rows, facts


def source_audit() -> list[dict[str, Any]]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock.file
        text = line_window(path, lock.lines) if path.exists() else ""
        missing = [needle for needle in lock.needles if norm(needle) not in norm(text)]
        rows.append({"id": lock.id, "file": lock.file, "path": str(path), "lines": lock.lines, "claim": lock.claim, "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def asset_row(label: str, path: Path) -> dict[str, Any]:
    exists = path.is_file()
    return {"label": label, "path": str(path), "exists": exists, "bytes": path.stat().st_size if exists else None, "sha256": sha(path, "sha256") if exists else None, "md5": sha(path, "md5") if exists else None, "matchesExpected": exists and path.stat().st_size == EXPECTED_SIZE and sha(path, "sha256") == EXPECTED_SHA256 and sha(path, "md5") == EXPECTED_MD5}


def build() -> dict[str, Any]:
    errors: list[str] = []
    source = source_audit()
    errors.extend(f"source lock failed: {row['id']} missing={row['missing']}" for row in source if not row["ok"])
    assets = [asset_row("canonical_dm1_dungeon_dat", CANONICAL_DUNGEON), asset_row("extracted_dm1_pc34_dungeon_dat", EXTRACTED_PC34_DUNGEON)]
    for row in assets:
        if not row["matchesExpected"]:
            errors.append(f"asset identity failed: {row['label']} bytes={row['bytes']} sha256={row['sha256']} md5={row['md5']}")
    packet_boundaries: list[dict[str, Any]] = []
    facts: dict[str, Any] = {}
    if CANONICAL_DUNGEON.is_file():
        data = CANONICAL_DUNGEON.read_bytes()
        packet_boundaries, facts = packet_rows(data)
        expected = {"payloadBytes": EXPECTED_PAYLOAD_BYTES, "expectedChecksumLe16": EXPECTED_CHECKSUM, "calculatedChecksum16": EXPECTED_CHECKSUM, "trailingBytesAfterChecksum": 0, "columnCount": EXPECTED_COLUMN_COUNT}
        for key, value in expected.items():
            if facts.get(key) != value:
                errors.append(f"packet fact mismatch {key}: {facts.get(key)!r} != {value!r}")
        header = facts.get("header", {})
        if header.get("thingCounts") != EXPECTED_THING_COUNTS:
            errors.append(f"thing count vector mismatch: {header.get('thingCounts')!r} != {EXPECTED_THING_COUNTS!r}")
        raw_rows = [row for row in packet_boundaries if row["name"] == "raw_map_data"]
        if not raw_rows or raw_rows[0]["offset"] != EXPECTED_RAW_MAP_OFFSET:
            errors.append(f"raw map packet offset mismatch: {raw_rows[0]['offset'] if raw_rows else None!r} != {EXPECTED_RAW_MAP_OFFSET!r}")
    greatstone = {"path": str(GREATSTONE_ATLAS), "exists": GREATSTONE_ATLAS.exists(), "indexPagesExists": (GREATSTONE_ATLAS / "index/pages.json").exists(), "indexFilesExists": (GREATSTONE_ATLAS / "index/files.json").exists(), "claim": "Greatstone is a secondary local atlas/provenance reference; this gate's source of truth is ReDMCSB plus N2 original DUNGEON.DAT bytes."}
    if not (greatstone["exists"] and greatstone["indexPagesExists"] and greatstone["indexFilesExists"]):
        errors.append("Greatstone atlas local reference is missing")
    return {"schema": "firestaff.parity.pass512_dm1_v1_original_dungeon_packet_boundary.v1", "pass": not errors, "status": "PASS512_DM1_V1_ORIGINAL_DUNGEON_PACKET_BOUNDARY_LOCKED" if not errors else "FAIL_PASS512_DM1_V1_ORIGINAL_DUNGEON_PACKET_BOUNDARY", "scope": "DM1 V1 original DUNGEON.DAT packet/order/checksum evidence only; no movement, viewport, runtime, or pixel-parity claim.", "redmcsbSource": str(REDMCSB), "originalDmRoot": str(DM_ROOT), "sourceLocks": source, "assetLocks": assets, "packetFacts": facts, "packetBoundaries": packet_boundaries, "greatstoneReference": greatstone, "nonClaims": ["Does not execute DOSBox or original Dungeon Master.", "Does not modify or verify movement, viewport drawing, or Firestaff runtime state.", "Does not promote any original capture artifact to parity evidence."], "errors": errors}


def write_report(data: dict[str, Any]) -> None:
    facts = data.get("packetFacts", {})
    lines = ["# Pass512 - DM1 V1 original DUNGEON.DAT packet boundary", "", f"Status: {data['status']}", "", str(data["scope"]), "", "## ReDMCSB Source Locks", ""]
    for row in data["sourceLocks"]:
        lines.append(f"- {row['file']}:{row['lines']} ok={row['ok']} - {row['claim']}")
    lines += ["", "## Asset Locks", ""]
    for row in data["assetLocks"]:
        lines.append(f"- {row['label']} ok={row['matchesExpected']} bytes={row['bytes']} sha256={row['sha256']} md5={row['md5']}")
    lines += ["", "## Packet Facts", "", f"- payloadBytes={facts.get('payloadBytes')} checksumOffset={facts.get('expectedChecksumOffset')}", f"- expectedChecksumLe16=0x{int(facts.get('expectedChecksumLe16', 0)):04x} calculatedChecksum16=0x{int(facts.get('calculatedChecksum16', 0)):04x}", f"- columnCount={facts.get('columnCount')} rawMapDataOffset={next((row['offset'] for row in data['packetBoundaries'] if row['name'] == 'raw_map_data'), None)}", f"- header={facts.get('header')}", "", "## Packet Boundaries", ""]
    for row in data["packetBoundaries"]:
        lines.append(f"- {row['name']} offset={row['offset']} bytes={row['bytes']} end={row['endOffset']} byteSum16=0x{row['byteSum16']:04x}")
    lines += ["", "## Non-claims", ""]
    lines.extend(f"- {claim}" for claim in data["nonClaims"])
    if data["errors"]:
        lines += ["", "## Errors", ""]
        lines.extend(f"- {error}" for error in data["errors"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    data = build()
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    print(data["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    for error in data["errors"]:
        print(f"- {error}")
    return 0 if data["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
