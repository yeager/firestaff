#!/usr/bin/env python3
"""Inventory N2-local CSB saved-game sample coverage.

This verifier is intentionally non-invasive: it records whether an extracted,
curated CSBGAME*.DAT sample is present in the approved N2-local reference roots.
It exits 0 for both presence and absence so it can be used as a blocker/evidence
probe without breaking unrelated lanes.
"""
from __future__ import annotations

import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from typing import Any

HOME = Path.home()
ROOTS = [
    HOME / ".openclaw/data/firestaff-original-games/DM",
    HOME / ".openclaw/data/firestaff-original-games/DM/_extracted",
]
OUTPUT = Path("parity-evidence/verification/csb_sample_save_search_blocker.json")
CSB_SAMPLE_RE = re.compile(r"^CSBGAME(?:[0-9FG]|[0-9]+)?\.(?:DAT|BAK)$", re.IGNORECASE)
GENERAL_SAVE_RE = re.compile(r"(?:CSBGAME|DMGAME|DMSAVE|SKSAVE|SAVE|\.SAV$)", re.IGNORECASE)
IMAGE_SUFFIXES = {".adf", ".msa", ".st"}
ARCHIVE_SUFFIXES = {".zip"}
MSA_MAGIC = 0x0E0F
MSA_RLE_MARKER = 0xE5
SAVE_DISK_RE = re.compile(r"save disk.*\.msa$", re.IGNORECASE)


def rel(path: Path) -> str:
    try:
        return "~/" + str(path.relative_to(HOME))
    except ValueError:
        return str(path)


def iter_files():
    seen: set[Path] = set()
    for root in ROOTS:
        if not root.exists():
            continue
        for p in root.rglob("*"):
            if p.is_file() and p not in seen:
                seen.add(p)
                yield p


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def archive_hits(path: Path) -> list[str]:
    try:
        proc = subprocess.run(["unzip", "-l", str(path)], text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=10, check=False)
    except (OSError, subprocess.TimeoutExpired):
        return []
    hits: list[str] = []
    for line in proc.stdout.splitlines():
        if GENERAL_SAVE_RE.search(line):
            hits.append(line.strip())
    return hits[:20]


def image_string_hits(path: Path) -> list[str]:
    try:
        proc = subprocess.run(["strings", "-a", str(path)], text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=15, check=False)
    except (OSError, subprocess.TimeoutExpired, UnicodeDecodeError):
        return []
    hits: list[str] = []
    for line in proc.stdout.splitlines():
        if GENERAL_SAVE_RE.search(line):
            hits.append(line.strip())
            if len(hits) >= 20:
                break
    return hits


def decompress_msa(path: Path) -> tuple[dict[str, Any], bytes]:
    data = path.read_bytes()
    if len(data) < 10:
        raise ValueError("MSA image too short for header")
    magic, sectors_per_track, sides_minus_one, start_track, end_track = struct.unpack(">5H", data[:10])
    if magic != MSA_MAGIC:
        raise ValueError(f"unexpected MSA magic 0x{magic:04x}")
    track_size = sectors_per_track * 512
    pos = 10
    raw = bytearray()
    compressed_tracks = 0
    uncompressed_tracks = 0
    for track in range(start_track, end_track + 1):
        for side in range(sides_minus_one + 1):
            if pos + 2 > len(data):
                raise ValueError(f"truncated MSA before track {track} side {side}")
            encoded_len = struct.unpack(">H", data[pos : pos + 2])[0]
            pos += 2
            encoded = data[pos : pos + encoded_len]
            pos += encoded_len
            if len(encoded) != encoded_len:
                raise ValueError(f"truncated MSA track {track} side {side}")
            if encoded_len == track_size:
                decoded = encoded
                uncompressed_tracks += 1
            else:
                out = bytearray()
                i = 0
                while i < len(encoded):
                    byte = encoded[i]
                    i += 1
                    if byte == MSA_RLE_MARKER:
                        if i + 3 > len(encoded):
                            raise ValueError(f"truncated MSA RLE at track {track} side {side}")
                        value = encoded[i]
                        count = struct.unpack(">H", encoded[i + 1 : i + 3])[0]
                        i += 3
                        out.extend([value] * count)
                    else:
                        out.append(byte)
                if len(out) != track_size:
                    raise ValueError(f"decoded MSA track {track} side {side} to {len(out)} bytes, expected {track_size}")
                decoded = bytes(out)
                compressed_tracks += 1
            raw.extend(decoded)
    if pos != len(data):
        raise ValueError(f"MSA trailing bytes: decoded through {pos}, file length {len(data)}")
    header = {
        "magic": f"0x{magic:04x}",
        "sectors_per_track": sectors_per_track,
        "sides_minus_one": sides_minus_one,
        "start_track": start_track,
        "end_track": end_track,
        "decoded_size": len(raw),
        "compressed_tracks": compressed_tracks,
        "uncompressed_tracks": uncompressed_tracks,
    }
    return header, bytes(raw)


def parse_gemdos_root(raw: bytes) -> dict[str, Any]:
    if len(raw) < 512:
        raise ValueError("decoded disk image too short for boot sector")
    boot = raw[:512]

    def le(offset: int, size: int) -> int:
        return int.from_bytes(boot[offset : offset + size], "little")

    bytes_per_sector = le(11, 2)
    sectors_per_cluster = boot[13]
    reserved_sectors = le(14, 2)
    fat_count = boot[16]
    root_entry_count = le(17, 2)
    total_sectors = le(19, 2)
    media_descriptor = boot[21]
    sectors_per_fat = le(22, 2)
    sectors_per_track = le(24, 2)
    heads = le(26, 2)
    root_offset = (reserved_sectors + fat_count * sectors_per_fat) * bytes_per_sector
    root_size = root_entry_count * 32
    root = raw[root_offset : root_offset + root_size]
    entries = []
    deleted_entries = 0
    free_entries = 0
    for index in range(0, len(root), 32):
        entry = root[index : index + 32]
        first = entry[0]
        if first == 0x00:
            free_entries += 1
            continue
        if first == 0xE5:
            deleted_entries += 1
            continue
        name = entry[:8].decode("latin-1", "replace").rstrip()
        ext = entry[8:11].decode("latin-1", "replace").rstrip()
        entries.append(
            {
                "slot": index // 32,
                "name": name + (f".{ext}" if ext else ""),
                "attributes": f"0x{entry[11]:02x}",
                "start_cluster": int.from_bytes(entry[26:28], "little"),
                "size": int.from_bytes(entry[28:32], "little"),
            }
        )
    return {
        "boot_sector": {
            "bytes_per_sector": bytes_per_sector,
            "sectors_per_cluster": sectors_per_cluster,
            "reserved_sectors": reserved_sectors,
            "fat_count": fat_count,
            "root_entry_count": root_entry_count,
            "total_sectors": total_sectors,
            "media_descriptor": f"0x{media_descriptor:02x}",
            "sectors_per_fat": sectors_per_fat,
            "sectors_per_track": sectors_per_track,
            "heads": heads,
            "boot_signature_tail": boot[510:512].hex(),
        },
        "root_directory_offset": root_offset,
        "root_directory_size": root_size,
        "active_root_entries": entries,
        "active_root_entry_count": len(entries),
        "deleted_root_entry_count": deleted_entries,
        "free_root_entry_count": free_entries,
        "contains_csbgame_root_entry": any(CSB_SAMPLE_RE.match(e["name"]) for e in entries),
    }


def save_disk_msa_inventory(paths: list[Path]) -> list[dict[str, Any]]:
    inventories = []
    for path in sorted(paths, key=lambda p: str(p).lower()):
        if not SAVE_DISK_RE.search(path.name):
            continue
        try:
            msa_header, raw = decompress_msa(path)
            gemdos = parse_gemdos_root(raw)
            inventories.append(
                {
                    "path": rel(path),
                    "size": path.stat().st_size,
                    "sha256": sha256(path),
                    "format": "Atari ST Magic Shadow Archiver (MSA) floppy image",
                    "msa_header": msa_header,
                    "gemdos_fat12_inventory": gemdos,
                    "sample_status": "NO_ROOT_DIRECTORY_FILES" if not gemdos["active_root_entries"] else "ROOT_DIRECTORY_HAS_FILES",
                }
            )
        except Exception as exc:  # noqa: BLE001 - evidence inventory should report decode failures explicitly.
            inventories.append(
                {
                    "path": rel(path),
                    "size": path.stat().st_size,
                    "sha256": sha256(path),
                    "format": "Atari ST Magic Shadow Archiver (MSA) floppy image",
                    "decode_error": str(exc),
                    "sample_status": "MSA_DECODE_FAILED",
                }
            )
    return inventories


def main() -> int:
    files = sorted(iter_files(), key=lambda x: str(x).lower())
    exact_extracted_samples = []
    exact_save_named_files = []
    archives = []
    images = []
    image_paths: list[Path] = []
    for p in files:
        name = p.name
        if CSB_SAMPLE_RE.match(name):
            exact_extracted_samples.append({"path": rel(p), "size": p.stat().st_size})
        if GENERAL_SAVE_RE.search(name):
            exact_save_named_files.append({"path": rel(p), "size": p.stat().st_size})
        suffix = p.suffix.lower()
        if suffix in ARCHIVE_SUFFIXES:
            hits = archive_hits(p)
            if hits:
                archives.append({"path": rel(p), "hits": hits})
        elif suffix in IMAGE_SUFFIXES:
            image_paths.append(p)
            hits = image_string_hits(p)
            if hits:
                images.append({"path": rel(p), "string_hits": hits})

    save_disk_inventory = save_disk_msa_inventory(image_paths)
    result = {
        "schema": "firestaff.csb_sample_save_search_blocker.v2",
        "pass": True,
        "verifier_semantics": "Inventory only; exits 0 whether a curated sample is present or absent.",
        "roots": [rel(r) for r in ROOTS],
        "curated_csbgame_dat_sample_present": bool(exact_extracted_samples),
        "exact_extracted_csbgame_samples": exact_extracted_samples,
        "save_named_filesystem_entries": exact_save_named_files,
        "atari_st_save_disk_msa_inventory": save_disk_inventory,
        "archive_save_name_hits": archives,
        "disk_image_save_string_hits": images,
        "source_lock_compatibility_criteria": [
            "Use a real extracted CSBGAME*.DAT/BAK saved-game sample, not a game/utility executable disk string reference.",
            "Decoded save header must use C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK (DEFS.H:496-498; CEDTINCU.C:27-32).",
            "CSB_SAVE_HEADER.DungeonID must be C13_DUNGEON_CSB_GAME for CSB game saves (DEFS.H:482-494, 519-523; CEDTINCU.C:49-58; CEDTINCH.C:55-58).",
            "Save-disk MSA images must decode to GEMDOS/FAT12 root entries containing CSBGAME*.DAT/BAK before they count as sample provenance.",
            "Platform/FormatID must pass the source gates for the targeted media (CEDTINCU.C:49-58; CEDTINCH.C:55-58).",
            "Save routing must map C13_DUNGEON_CSB_GAME or C12_DUNGEON_CSB_PRISON to M746_FILE_ID_SAVE_CSBGAME_DAT, not DMSAVE.DAT (CEDTINC8.C:101-118).",
        ],
        "blocker": None if exact_extracted_samples else "No extracted curated CSBGAME*.DAT/BAK saved-game sample is present under the approved N2 original-game roots; the N2 Atari ST Save Disk .msa candidate decodes as an empty GEMDOS/FAT12 root directory.",
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(result, indent=2, sort_keys=False) + "\n")
    if exact_extracted_samples:
        print(f"PASS csb sample-save inventory: {len(exact_extracted_samples)} exact extracted sample(s) present")
    else:
        print("PASS csb sample-save inventory: blocker recorded; no exact extracted CSBGAME*.DAT/BAK sample present")
    for inventory in save_disk_inventory:
        print(f"MSA {inventory['sample_status']}: {inventory['path']}")
    print(f"wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
