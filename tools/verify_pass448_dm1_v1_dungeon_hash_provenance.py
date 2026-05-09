#!/usr/bin/env python3
"""Pass448 DM1 V1 DUNGEON.DAT hash/provenance gate.

Map/start/dimension comparisons for the DM1 V1 lane are only meaningful when
bound to the exact PC 3.4 English dungeon payload.  This verifier hashes the
current local Firestaff anchors, records nearby same-basename variants, and
then parses the header/map0 facts used by movement/viewport evidence.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DM_ROOT = Path.home() / ".openclaw/data/firestaff-original-games/DM"
CANONICAL_DM1 = DM_ROOT / "_canonical/dm1/DUNGEON.DAT"
EXTRACTED_PC34 = DM_ROOT / "_extracted/dm-pc34/DungeonMasterPC34/DATA/DUNGEON.DAT"
EUDATA_PC34 = DM_ROOT / "_extracted/dm-pc34/DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT"
RED_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/pass448_dm1_v1_dungeon_hash_provenance.json"

EXPECTED_SHA256 = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
EXPECTED_MD5 = "766450c940651fc021c92fe5d0d0b3a6"
EXPECTED_SIZE = 33357


@dataclass(frozen=True)
class DungeonAnchor:
    label: str
    path: Path
    variant: str
    must_match: bool


ANCHORS = [
    DungeonAnchor("canonical_dm1", CANONICAL_DM1, "DM PC 3.4 English / I34E canonical symlink", True),
    DungeonAnchor("extracted_dm1_pc34_english", EXTRACTED_PC34, "DM PC 3.4 English / I34E DATA", True),
    DungeonAnchor("extracted_dm1_pc34_eudata", EUDATA_PC34, "DM PC 3.4 Multilanguage / EUDATA same dungeon payload", True),
]

REDMCSB_ANCHORS = {
    "FILENAME.C": [
        "G1059_pc_DungeonFileName = \"DATA\\\\DUNGEON.DAT\"",
    ],
    "LOADSAVE.C": [
        "STATICFUNCTION BOOLEAN F0434_STARTEND_IsLoadDungeonSuccessful_CPSC",
        "G0521_i_GameFileHandle = Fopen(\"\\\\DUNGEON.DAT\", C0_READ_ONLY)",
        "G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;",
        "G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F;",
        "G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003;",
    ],
    "CEDTINCA.C": [
        "BOOLEAN F7063_LoadDungeon",
        "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.Header",
        "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.RawMapData",
    ],
    "CEDTINC6.C": [
        "F7059_ReadDungeonPartWithChecksum",
        "*P3720_pi_Checksum += L3962_ui_;",
    ],
}


def digest(path: Path, algorithm: str) -> str:
    h = hashlib.new(algorithm)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def le16(data: bytes, off: int) -> int:
    return data[off] | (data[off + 1] << 8)


def parse_facts(data: bytes) -> dict[str, object]:
    initial = le16(data, 8)
    map0_off = 44
    map0_raw_offset = le16(data, map0_off)
    map0_packed_a = le16(data, map0_off + 8)
    map0_width = ((map0_packed_a >> 6) & 31) + 1
    map0_height = ((map0_packed_a >> 11) & 31) + 1
    raw_count = le16(data, 2)
    raw_file_offset = len(data) - 2 - raw_count

    def raw_square(x: int, y: int) -> int:
        return data[raw_file_offset + map0_raw_offset + x * map0_height + y]

    return {
        "rawMapDataByteCount": raw_count,
        "mapCount": data[4],
        "textDataWordCount": le16(data, 6),
        "initialPartyLocationRaw": f"0x{initial:04X}",
        "initialState": {"mapIndex": 0, "mapX": initial & 31, "mapY": (initial >> 5) & 31, "direction": (initial >> 10) & 3},
        "squareFirstThingCount": le16(data, 10),
        "map0": {
            "level": map0_packed_a & 63,
            "width": map0_width,
            "height": map0_height,
            "rawMapDataByteOffset": map0_raw_offset,
            "rawMapDataFileOffset": raw_file_offset,
            "entrySquareRaw_x1_y3": raw_square(1, 3),
            "frontSquareRaw_x1_y4": raw_square(1, 4),
        },
    }


def line_no(text: str, needle: str) -> int | None:
    idx = text.find(needle)
    if idx < 0:
        return None
    return text.count("\n", 0, idx) + 1


def check_sources(errors: list[str]) -> dict[str, list[dict[str, object]]]:
    out: dict[str, list[dict[str, object]]] = {}
    for rel, needles in REDMCSB_ANCHORS.items():
        path = RED_ROOT / rel
        if not path.is_file():
            errors.append(f"missing ReDMCSB source {rel}")
            continue
        text = path.read_text(encoding="latin-1", errors="replace")
        out[rel] = []
        for needle in needles:
            line = line_no(text, needle)
            out[rel].append({"needle": needle, "line": line})
            if line is None:
                errors.append(f"missing ReDMCSB anchor {rel}: {needle}")
    return out


def all_local_variants() -> list[dict[str, object]]:
    variants = []
    for path in sorted(DM_ROOT.rglob("*")):
        if path.is_file() and path.name.upper().startswith("DUNGEON") and path.suffix.upper() == ".DAT":
            variants.append({
                "relativePath": str(path.relative_to(DM_ROOT)),
                "bytes": path.stat().st_size,
                "sha256": digest(path, "sha256"),
                "md5": digest(path, "md5"),
                "acceptedForDm1V1EnglishComparisons": digest(path, "sha256") == EXPECTED_SHA256 and path.stat().st_size == EXPECTED_SIZE,
            })
    return variants


def main() -> int:
    errors: list[str] = []
    source_anchors = check_sources(errors)
    anchor_results = []
    canonical_bytes = b""
    for anchor in ANCHORS:
        if not anchor.path.is_file():
            errors.append(f"missing {anchor.label}: {anchor.path}")
            continue
        sha = digest(anchor.path, "sha256")
        md5 = digest(anchor.path, "md5")
        size = anchor.path.stat().st_size
        ok = size == EXPECTED_SIZE and sha == EXPECTED_SHA256 and md5 == EXPECTED_MD5
        if anchor.must_match and not ok:
            errors.append(f"{anchor.label} hash mismatch sha256={sha} md5={md5} bytes={size}")
        if anchor.label == "canonical_dm1":
            canonical_bytes = anchor.path.read_bytes()
        anchor_results.append({
            "label": anchor.label,
            "variant": anchor.variant,
            "relativePath": str(anchor.path.relative_to(DM_ROOT)),
            "bytes": size,
            "sha256": sha,
            "md5": md5,
            "matchesExpected": ok,
        })

    facts: dict[str, object] = {}
    if canonical_bytes:
        facts = parse_facts(canonical_bytes)
        expected_facts: dict[str, object] = {
            "rawMapDataByteCount": 12283,
            "mapCount": 14,
            "initialPartyLocationRaw": "0x0861",
            "initialState": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2},
            "map0": {
                "level": 0,
                "width": 18,
                "height": 19,
                "rawMapDataByteOffset": 0,
                "rawMapDataFileOffset": 21072,
                "entrySquareRaw_x1_y3": 0xB0,
                "frontSquareRaw_x1_y4": 0x30,
            },
        }
        for key, expected in expected_facts.items():
            if key == "map0":
                for subkey, subexpected in expected.items():
                    if facts["map0"].get(subkey) != subexpected:  # type: ignore[index]
                        errors.append(f"map0 {subkey} mismatch: {facts['map0'].get(subkey)!r} != {subexpected!r}")  # type: ignore[index]
            elif facts.get(key) != expected:
                errors.append(f"dungeon {key} mismatch: {facts.get(key)!r} != {expected!r}")

    evidence = {
        "pass": "pass448_dm1_v1_dungeon_hash_provenance",
        "status": "PASS" if not errors else "FAIL",
        "contract": "DM1 V1 map/start/dimension comparisons require this exact DUNGEON.DAT byte identity before using parsed facts.",
        "expected": {"variant": "DM PC 3.4 English / I34E DUNGEON.DAT", "bytes": EXPECTED_SIZE, "sha256": EXPECTED_SHA256, "md5": EXPECTED_MD5},
        "anchors": anchor_results,
        "localDungeonVariants": all_local_variants(),
        "parsedCanonicalFacts": facts,
        "redmcsbSourceAnchors": source_anchors,
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        print("FAIL pass448 DM1 V1 dungeon hash provenance")
        for error in errors:
            print(f"  {error}")
        return 1
    print(f"PASS pass448 DM1 V1 dungeon hash provenance: sha256={EXPECTED_SHA256} md5={EXPECTED_MD5} evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
