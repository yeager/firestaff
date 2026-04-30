#!/usr/bin/env python3
"""Verify narrow CSB/DM2 source-lock boundary evidence without building Firestaff."""
from __future__ import annotations

import argparse
import hashlib
import re
import struct
from pathlib import Path

DEFAULT_REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DEFAULT_ORIGINAL_DM = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM")
DEFAULT_REPO = Path(__file__).resolve().parents[1]

CHECKS = [
    {
        "id": "redmcsb-save-header-formats",
        "file": "DEFS.H",
        "range": "468-498",
        "needles": [
            "typedef struct {",
            "unsigned int16_t Noise[149];",
            "unsigned int16_t Noise[150];",
            "} DM_SAVE_HEADER;",
            "} CSB_SAVE_HEADER;",
            "#define C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER     0x01",
            "#define C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK 0x02",
        ],
        "why": "DM and CSB save headers are structurally separate; do not infer one from the other.",
    },
    {
        "id": "redmcsb-dungeon-id-namespace",
        "file": "DEFS.H",
        "range": "519-523",
        "needles": [
            "#define C10_DUNGEON_DM         10",
            "#define C12_DUNGEON_CSB_PRISON 12",
            "#define C13_DUNGEON_CSB_GAME   13",
        ],
        "why": "CSB/prison are source IDs 13/12, distinct from DM source ID 10.",
    },
    {
        "id": "redmcsb-csb-validation-criteria",
        "file": "CEDTINCU.C",
        "range": "5-77",
        "needles": [
            "BOOLEAN F7272_IsDungeonValid",
            "case C0x01_SAVE_HEADER_FORMAT_DUNGEON_MASTER:",
            "case C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK:",
            "case 1:",
            "((L4109_i_ == C13_DUNGEON_CSB_GAME) || (L4109_i_ == C12_DUNGEON_CSB_PRISON) || (L4109_i_ == C10_DUNGEON_DM))",
            "case 2:",
            "(L4109_i_ == C13_DUNGEON_CSB_GAME)",
            "case 3:",
            "((L4109_i_ == C12_DUNGEON_CSB_PRISON) || (L4109_i_ == C10_DUNGEON_DM))",
        ],
        "why": "CSB utility validation has explicit criteria; criteria 2 is CSB-game only, criteria 3 excludes CSB-game.",
    },
    {
        "id": "redmcsb-new-adventure-boundary",
        "file": "CEDTINCH.C",
        "range": "5-47",
        "needles": [
            "BOOLEAN F7086_IsReadyToMakeNewAdventure",
            "if (!G7111_Games[C0_GAME_SOURCE].GameLoaded || !G7114_LoadedChampionCount)",
            "F7272_IsDungeonValid(&G7111_Games[C0_GAME_SOURCE], 3)",
            "return F7076_AreAllChampionNamesUnique();",
        ],
        "why": "New Adventure is source-gated through validation criteria 3, not a generic CSB acceptance path.",
    },
    {
        "id": "redmcsb-su1e-csb-game-only-boundary",
        "file": "CEDTINCH.C",
        "range": "49-64",
        "needles": [
            "BOOLEAN F1996_(GAME* P5909_)",
            "P5909_->SaveHeaderFormat == C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK",
            "L5768_->DungeonID == C13_DUNGEON_CSB_GAME",
            "return C1_TRUE;",
        ],
        "why": "The SU1E-specific gate accepts only CSB-format headers for the CSB game dungeon ID.",
    },
    {
        "id": "redmcsb-save-file-routing-boundary",
        "file": "CEDTINC8.C",
        "range": "101-118",
        "needles": [
            "if (L3949_i_DungeonID == C10_DUNGEON_DM)",
            "L3940_i_ = M745_FILE_ID_SAVE_DMSAVE_DAT;",
            "if ((L3949_i_DungeonID == C13_DUNGEON_CSB_GAME) || (L3949_i_DungeonID == C12_DUNGEON_CSB_PRISON))",
            "L3940_i_ = M746_FILE_ID_SAVE_CSBGAME_DAT;",
        ],
        "why": "Save routing source-lock keeps DM saves and CSB saves on separate file IDs.",
    },
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def line_slice(text: str, line_range: str) -> str:
    start, end = [int(x) for x in line_range.split("-")]
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def check_source(source: Path) -> tuple[list[str], list[str]]:
    passed: list[str] = []
    failed: list[str] = []
    for check in CHECKS:
        path = source / check["file"]
        if not path.exists():
            failed.append(f"FAIL {check['id']}: missing {path}")
            continue
        text = path.read_text(errors="replace")
        haystack = line_slice(text, check["range"])
        missing = [needle for needle in check["needles"] if needle not in haystack]
        if missing:
            failed.append(
                f"FAIL {check['id']}: {check['file']}:{check['range']} missing "
                + "; ".join(repr(m) for m in missing)
            )
        else:
            passed.append(f"PASS {check['id']}: {check['file']}:{check['range']} - {check['why']}")
    return passed, failed


def inventory_refs(original_dm: Path) -> list[str]:
    rows: list[str] = []
    candidates = [
        "Game,Chaos_Strikes_Back,Amiga,Software.7z",
        "Game,Chaos_Strikes_Back,Atari_ST,Software.7z",
        "_extracted/csb-atari/Floppy Disks MSA/Chaos Strikes Back for Atari ST Game Disk v2.0 (English).msa",
        "_canonical/csb/amiga-Dungeon.DAT",
        "_canonical/csb/atari-DUNGEON.DAT",
        "_canonical/csb/amiga-Graphics.DAT",
        "_canonical/csb/atari-GRAPHICS.DAT",
        "Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z",
        "_extracted/dm2-dos-asm/SKULL.ASM",
    ]
    for rel in candidates:
        path = original_dm / rel
        if path.exists():
            rows.append(f"FOUND {rel} bytes={path.stat().st_size} sha256={sha256(path)}")
        else:
            rows.append(f"MISSING {rel}")
    return rows



def msa_root_members(msa_path: Path) -> dict[str, tuple[int, str]]:
    """Return root-file size/hash pairs from a simple Atari ST MSA disk image."""
    data = msa_path.read_bytes()
    if len(data) < 10:
        raise ValueError("MSA image is too short")
    magic, sectors_per_track, sides, start_track, end_track = struct.unpack(">5H", data[:10])
    if magic != 0x0E0F:
        raise ValueError(f"not an MSA image: magic=0x{magic:04x}")
    pos = 10
    track_bytes = sectors_per_track * 512
    raw = bytearray()
    for _track in range(start_track, end_track + 1):
        for _side in range(sides):
            if pos + 2 > len(data):
                raise ValueError("truncated MSA track header")
            compressed_len = struct.unpack(">H", data[pos : pos + 2])[0]
            pos += 2
            chunk = data[pos : pos + compressed_len]
            pos += compressed_len
            if compressed_len == track_bytes:
                decoded = chunk
            else:
                decoded_out = bytearray()
                i = 0
                while i < len(chunk):
                    value = chunk[i]
                    i += 1
                    if value == 0xE5:
                        if i + 3 > len(chunk):
                            raise ValueError("truncated MSA RLE run")
                        repeated = chunk[i]
                        count = struct.unpack(">H", chunk[i + 1 : i + 3])[0]
                        i += 3
                        decoded_out.extend([repeated] * count)
                    else:
                        decoded_out.append(value)
                decoded = bytes(decoded_out)
            if len(decoded) != track_bytes:
                raise ValueError(f"decoded MSA track is {len(decoded)} bytes, expected {track_bytes}")
            raw.extend(decoded)

    disk = bytes(raw)
    bytes_per_sector = struct.unpack_from("<H", disk, 11)[0]
    sectors_per_cluster = disk[13]
    reserved_sectors = struct.unpack_from("<H", disk, 14)[0]
    fat_count = disk[16]
    root_entries = struct.unpack_from("<H", disk, 17)[0]
    sectors_per_fat = struct.unpack_from("<H", disk, 22)[0]
    root_start = (reserved_sectors + fat_count * sectors_per_fat) * bytes_per_sector
    root_size = root_entries * 32
    data_start = root_start + root_size
    fat = disk[reserved_sectors * bytes_per_sector : (reserved_sectors + sectors_per_fat) * bytes_per_sector]

    def fat12(cluster: int) -> int:
        offset = cluster + cluster // 2
        value = fat[offset] | (fat[offset + 1] << 8)
        return value >> 4 if cluster & 1 else value & 0x0FFF

    def read_chain(cluster: int) -> bytes:
        out = bytearray()
        seen: set[int] = set()
        while 2 <= cluster < 0xFF8 and cluster not in seen:
            seen.add(cluster)
            offset = data_start + (cluster - 2) * sectors_per_cluster * bytes_per_sector
            out.extend(disk[offset : offset + sectors_per_cluster * bytes_per_sector])
            cluster = fat12(cluster)
        return bytes(out)

    members: dict[str, tuple[int, str]] = {}
    for idx in range(root_entries):
        entry = disk[root_start + idx * 32 : root_start + (idx + 1) * 32]
        if entry[0] in (0, 0xE5) or (entry[11] & 0x18):
            continue
        name = entry[:8].decode("ascii", errors="replace").rstrip()
        ext = entry[8:11].decode("ascii", errors="replace").rstrip()
        filename = name + (f".{ext}" if ext else "")
        cluster = struct.unpack_from("<H", entry, 26)[0]
        size = struct.unpack_from("<I", entry, 28)[0]
        payload = read_chain(cluster)[:size] if cluster else b""
        members[filename.upper()] = (size, hashlib.sha256(payload).hexdigest())
    return members


def csb_target_curation(original_dm: Path) -> list[str]:
    rows: list[str] = []
    atari_archive = original_dm / "Game,Chaos_Strikes_Back,Atari_ST,Software.7z"
    amiga_archive = original_dm / "Game,Chaos_Strikes_Back,Amiga,Software.7z"
    atari_dungeon = original_dm / "_canonical/csb/atari-DUNGEON.DAT"
    amiga_dungeon = original_dm / "_canonical/csb/amiga-Dungeon.DAT"
    atari_graphics = original_dm / "_canonical/csb/atari-GRAPHICS.DAT"
    amiga_graphics = original_dm / "_canonical/csb/amiga-Graphics.DAT"
    atari_msa_v20 = original_dm / "_extracted/csb-atari/Floppy Disks MSA/Chaos Strikes Back for Atari ST Game Disk v2.0 (English).msa"
    atari_stx_v21 = original_dm / "_extracted/csb-atari/Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx"
    amiga_harddisk = original_dm / "_extracted/csb-amiga/HardDisk/Chaos Strikes Back for Amiga v3.3 (French) Hacked by Meynaf/DungeonMaster/Graphics.DAT"

    for label, path in [
        ("atari_archive", atari_archive),
        ("amiga_archive", amiga_archive),
        ("atari_dungeon", atari_dungeon),
        ("amiga_dungeon", amiga_dungeon),
        ("atari_graphics", atari_graphics),
        ("amiga_graphics", amiga_graphics),
        ("atari_official_english_v2_0_game_msa", atari_msa_v20),
        ("atari_official_english_v2_1_game_stx", atari_stx_v21),
        ("amiga_extracted_graphics_anchor", amiga_harddisk),
    ]:
        if path.exists():
            rows.append(f"TARGET_REF {label}: {path} bytes={path.stat().st_size} sha256={sha256(path)}")
        else:
            rows.append(f"TARGET_REF_MISSING {label}: {path}")

    if atari_msa_v20.exists():
        try:
            members = msa_root_members(atari_msa_v20)
        except Exception as exc:  # noqa: BLE001 - verifier should report exact evidence-read failure
            rows.append(f"TARGET_CURATION canonical_atari_st_english_v2_0: BLOCKED; cannot read MSA root members: {exc}")
        else:
            expected = {
                "GRAPHICS.DAT": (272069, "cff31dbdc071af2c6de8a0b9e1110b189e067706868d42fc8b2267e18422f687"),
                "DUNGEON.DAT": (2098, "59a72978879f3a3e9de3a6767ee069266d369244b1091314ddc16c03d8d41530"),
            }
            for member, (size, digest) in expected.items():
                actual = members.get(member)
                if actual == (size, digest):
                    rows.append(f"TARGET_CURATION canonical_atari_st_english_v2_0 {member}: bytes={size} sha256={digest}")
                else:
                    rows.append(f"TARGET_CURATION canonical_atari_st_english_v2_0 {member}: BLOCKED; expected bytes={size} sha256={digest}, got {actual}")
    else:
        rows.append(f"TARGET_CURATION canonical_atari_st_english_v2_0: BLOCKED; missing {atari_msa_v20}")

    if atari_dungeon.exists() and amiga_dungeon.exists():
        if sha256(atari_dungeon) == sha256(amiga_dungeon):
            rows.append("TARGET_CURATION dungeon: Atari and Amiga canonical dungeon payloads match; dungeon identity is not the blocker.")
        else:
            rows.append("TARGET_CURATION dungeon: BLOCKED; Atari and Amiga canonical dungeon payloads differ.")
    if atari_graphics.exists() and amiga_graphics.exists():
        same = sha256(atari_graphics) == sha256(amiga_graphics) and atari_graphics.stat().st_size == amiga_graphics.stat().st_size
        if same:
            rows.append("TARGET_CURATION graphics: Atari and Amiga canonical graphics payloads match.")
        else:
            rows.append("TARGET_CURATION graphics: BLOCKER; Atari GRAPHICS.DAT and Amiga Graphics.DAT differ in size/hash, so graphics/render parity must choose exactly one platform asset lineage.")
    rows.append("TARGET_CURATION choice: use Atari ST English v2.x as the CSB graphics/render parity lane; reject Amiga Graphics.DAT for that lane because it is a separate size/hash lineage, not an interchangeable renderer input.")
    return rows


def repo_boundary_scan(repo: Path) -> list[str]:
    matrix = repo / "PARITY_MATRIX_DM1_V1.md"
    config = repo / "config_m12.h"
    rows: list[str] = []
    if matrix.exists():
        text = matrix.read_text(errors="replace")
        w10 = re.search(r"## 10\. CSB and DM2 readiness.*?(?=\n---\n)", text, re.S)
        if w10:
            for line in w10.group(0).splitlines():
                if "CSB" in line or "DM2" in line or "DM1 assumptions" in line:
                    rows.append(f"MATRIX {line}")
    if config.exists():
        for i, line in enumerate(config.read_text(errors="replace").splitlines(), start=1):
            if "M12_CONFIG_GAME_COUNT" in line:
                rows.append(f"CONFIG config_m12.h:{i}: {line.strip()}")
    return rows


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=DEFAULT_REDMCSB_SOURCE)
    parser.add_argument("--original-dm", type=Path, default=DEFAULT_ORIGINAL_DM)
    parser.add_argument("--repo", type=Path, default=DEFAULT_REPO)
    args = parser.parse_args()

    print("CSB save-header / dungeon-ID / save-routing source-lock boundary guard")
    print(f"redmcsb_source={args.redmcsb_source}")
    print(f"original_dm={args.original_dm}")
    print(f"repo={args.repo}")
    passed, failed = check_source(args.redmcsb_source)
    print("\n[source checks]")
    for row in passed + failed:
        print(row)
    print("\n[reference inventory]")
    for row in inventory_refs(args.original_dm):
        print(row)
    print("\n[csb target curation]")
    curation_rows = csb_target_curation(args.original_dm)
    for row in curation_rows:
        print(row)
    failed.extend(row for row in curation_rows if "canonical_atari_st_english_v2_0" in row and "BLOCKED" in row)
    print("\n[repo boundary scan]")
    for row in repo_boundary_scan(args.repo):
        print(row)
    if failed:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
