#!/usr/bin/env python3
"""Verify the DM2 file-open higher-caller evidence boundary.

This is intentionally a blocker/evidence gate, not an implementation probe:
ReDMCSB is audited first as DM/CSB source boundary comparison only.  DM2 is
checked only from the curated SKULL.ASM disassembly plus canonical SKULL.EXE
bytes; SKULL.ASM is not C source.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import zipfile
from pathlib import Path
from typing import Any

REDMCSB_SOURCE = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
DM2_CANONICAL = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2")
DM2_ASM = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm2-dos-asm/SKULL.ASM")
CANONICAL_ARCHIVE = "Dungeon-Master-II-Skullkeep_DOS_EN.zip"
CANONICAL_EXE_MEMBER = "skull.exe"
EXPECTED_EXE_SHA256 = "0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {
        "id": "redmcsb-filename-dm-pc-data-members",
        "file": "FILENAME.C",
        "range": "4-10",
        "function": "global filename definitions",
        "needles": [
            "G1059_pc_DungeonFileName = \"DATA\\\\DUNGEON.DAT\"",
            "G2130_GraphicsDatFileName = \"DATA\\\\GRAPHICS.DAT\"",
        ],
    },
    {
        "id": "redmcsb-graphics-open-boundary",
        "file": "MEMORY.C",
        "range": "1212-1285",
        "function": "F0477_MEMORY_OpenGraphicsDat_CPSDF",
        "needles": [
            "void F0477_MEMORY_OpenGraphicsDat_CPSDF",
            "F0770_FILE_Open(G2130_GraphicsDatFileName)",
            "C41_ERROR_UNABLE_TO_OPEN_GRAPHICS_DAT",
        ],
    },
    {
        "id": "redmcsb-dungeon-open-boundary",
        "file": "LOADSAVE.C",
        "range": "2333-2377",
        "function": "dungeon load open block in F0436_LOADSAVE_LoadGameFile",
        "needles": [
            "F0770_FILE_Open(G1059_pc_DungeonFileName)",
            "F0770_FILE_Open(\"\\\\DUNGEON.DAT\")",
        ],
    },
]

ASM_CHECKS: list[dict[str, Any]] = [
    {
        "id": "dm2-generic-open-inner",
        "range": "6154-6200",
        "byte_needles": [
            bytes.fromhex("8B 76 0E 80 3C 20 75 03 46 EB F8"),
            bytes.fromhex("8A 46 FC 89 F2 0A 46 12 BF FF FF B4 3D CD 21"),
        ],
        "decoded": "mov si,[bp+0Eh]; trim leading spaces; mov dx,si; mov ah,3Dh; int 21h",
    },
    {
        "id": "dm2-open-wrapper-calls-inner",
        "range": "6139-6153",
        "byte_needles": [bytes.fromhex("FF 76 08 FF 76 06 E8 06 00 83 C4 08 5D 5B C3")],
        "decoded": "wrapper pushes its filename/flags args then calls the generic inner open routine",
    },
    {
        "id": "dm2-direct-caller-first-path-buffer",
        "range": "4761-4783",
        "byte_needles": [bytes.fromhex("31 C0 50 B8 00 02 50 8D 46 0A 50 E8 53 05")],
        "decoded": "push 0; push 0200h; lea ax,[bp+0Ah]; push ax; call open wrapper",
    },
    {
        "id": "dm2-direct-caller-second-path-buffer",
        "range": "4813-4826",
        "byte_needles": [bytes.fromhex("31 C0 50 B8 00 02 50 8D 46 0A 50 E8 28 05")],
        "decoded": "push 0; push 0200h; lea ax,[bp+0Ah]; push ax; call open wrapper",
    },
    {
        "id": "dm2-startup-member-strings-only",
        "range": "461403-461472",
        "byte_needles": [b"DUNGEON.DAT", b"DUNGEON.FTL", b"GRAPHICS.DAT"],
        "decoded": "canonical startup member string bytes are present, but this range has no labels/xrefs in SKULL.ASM",
    },
]

BINARY_ANCHORS = {
    "generic-open-int21": {"offset": 0x1A3A, "bytes": bytes.fromhex("B4 3D CD 21")},
    "dungeon-string": {"offset": 0x70C65, "bytes": b"DUNGEON.DAT"},
    "graphics-string": {"offset": 0x70C98, "bytes": b"GRAPHICS.DAT"},
}


def line_slice(path: Path, line_range: str) -> str:
    start, end = [int(x) for x in line_range.split("-")]
    return "\n".join(path.read_text(encoding="latin1", errors="replace").split("\n")[start - 1 : end])


def parse_db_bytes(text: str) -> bytes:
    out = bytearray()
    for line in text.splitlines():
        match = re.search(r"\bdb\s+([^;\r\n]+)", line)
        if not match:
            continue
        for raw in match.group(1).split(","):
            token = raw.strip()
            try:
                if token.lower().endswith("h"):
                    value = int(token[:-1], 16)
                else:
                    value = int(token, 0)
            except ValueError:
                continue
            if 0 <= value <= 0xFF:
                out.append(value)
    return bytes(out)


def check_source(source_root: Path) -> tuple[list[dict[str, Any]], list[str]]:
    passed: list[dict[str, Any]] = []
    failed: list[str] = []
    for check in SOURCE_CHECKS:
        path = source_root / check["file"]
        if not path.exists():
            failed.append(f"{check['id']}: missing {path}")
            continue
        text = line_slice(path, check["range"])
        missing = [needle for needle in check["needles"] if needle not in text]
        if missing:
            failed.append(f"{check['id']}: {path}:{check['range']} missing {missing!r}")
        else:
            passed.append({"id": check["id"], "file": str(path), "range": check["range"], "function": check["function"]})
    return passed, failed


def check_asm(asm_path: Path) -> tuple[list[dict[str, Any]], list[str]]:
    passed: list[dict[str, Any]] = []
    failed: list[str] = []
    if not asm_path.exists():
        return passed, [f"dm2-skull-asm-present: missing {asm_path}"]
    for check in ASM_CHECKS:
        text = line_slice(asm_path, check["range"])
        data = parse_db_bytes(text)
        missing = [needle.hex(" ").upper() for needle in check["byte_needles"] if needle not in data]
        if missing:
            failed.append(f"{check['id']}: {asm_path}:{check['range']} missing {missing!r}")
        else:
            passed.append({"id": check["id"], "file": str(asm_path), "range": check["range"], "decoded": check["decoded"]})
    return passed, failed


def read_exe(dm2_root: Path) -> bytes:
    with zipfile.ZipFile(dm2_root / CANONICAL_ARCHIVE) as zf:
        return zf.read(CANONICAL_EXE_MEMBER)


def find_near_calls(image: bytes, target: int) -> list[int]:
    calls: list[int] = []
    for i in range(len(image) - 2):
        if image[i] != 0xE8:
            continue
        rel = struct.unpack_from("<h", image, i + 1)[0]
        if ((i + 3 + rel) & 0xFFFF) == target:
            calls.append(i)
    return calls


def check_binary(dm2_root: Path) -> tuple[dict[str, Any], list[str]]:
    failed: list[str] = []
    exe = read_exe(dm2_root)
    actual_sha = hashlib.sha256(exe).hexdigest()
    if actual_sha != EXPECTED_EXE_SHA256:
        failed.append(f"dm2-skull-exe-sha256: got {actual_sha}")
    for anchor_id, anchor in BINARY_ANCHORS.items():
        offset = anchor["offset"]
        expected = anchor["bytes"]
        got = exe[offset : offset + len(expected)]
        if got != expected:
            failed.append(f"{anchor_id}: SKULL.EXE+{offset:#x} got {got.hex(' ').upper()}")
    header_paragraphs = struct.unpack_from("<H", exe, 8)[0]
    image = exe[header_paragraphs * 16 :]
    direct_wrapper_calls = find_near_calls(image, 0x198F)
    direct_inner_calls = find_near_calls(image, 0x19AF)
    if direct_wrapper_calls != [0x1439, 0x1464]:
        failed.append(f"open-wrapper direct callers: got {[hex(c) for c in direct_wrapper_calls]}")
    if direct_inner_calls != [0x19A6]:
        failed.append(f"open-inner direct callers: got {[hex(c) for c in direct_inner_calls]}")
    return {
        "archive": str(dm2_root / CANONICAL_ARCHIVE),
        "member": CANONICAL_EXE_MEMBER,
        "sha256": actual_sha,
        "header_paragraphs": header_paragraphs,
        "image_offsets": {
            "open_wrapper": "0x198f",
            "open_inner": "0x19af",
            "direct_wrapper_calls": [hex(c) for c in direct_wrapper_calls],
            "direct_inner_calls": [hex(c) for c in direct_inner_calls],
        },
        "anchors": {key: {"offset": hex(value["offset"]), "bytes": value["bytes"].hex(" ").upper()} for key, value in BINARY_ANCHORS.items()},
    }, failed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=REDMCSB_SOURCE)
    parser.add_argument("--dm2-canonical", type=Path, default=DM2_CANONICAL)
    parser.add_argument("--dm2-asm", type=Path, default=DM2_ASM)
    args = parser.parse_args()

    source_passed, failures = check_source(args.redmcsb_source)
    asm_passed, asm_failures = check_asm(args.dm2_asm)
    failures.extend(asm_failures)
    binary, binary_failures = check_binary(args.dm2_canonical)
    failures.extend(binary_failures)

    result = {
        "schema": "firestaff.dm2_fileopen_higher_caller_blocker.v1",
        "policy": {
            "redmcsb": "boundary comparison only; not DM2 C source",
            "dm2": "SKULL.ASM disassembly plus canonical SKULL.EXE bytes only; SKULL.ASM is not C source",
        },
        "redmcsb_source_audit": source_passed,
        "dm2_disassembly_evidence": asm_passed,
        "dm2_binary_evidence": binary,
        "finding": {
            "status": "blocked_precisely",
            "summary": "The generic DOS open routine and the DUNGEON.DAT/GRAPHICS.DAT strings are verified, but this SKULL.ASM dump does not expose a statically verifiable higher-level caller/string-xref from either canonical startup member to the generic open routine.",
            "direct_caller_gate": "The only direct near calls to the open wrapper are at image offsets 0x1439 and 0x1464, corresponding to SKULL.ASM:4761-4783 and SKULL.ASM:4813-4826; both prepare a BP+0Ah path buffer, not a DUNGEON.DAT or GRAPHICS.DAT string pointer.",
            "member_string_gate": "The member strings are present at SKULL.ASM:461403-461472 / SKULL.EXE+0x70c65 and +0x70c98, but labels/xrefs are absent from the db-only disassembly, so a caller path would require deeper decompilation or dynamic tracing beyond this narrow evidence gate.",
        },
        "failures": failures,
    }
    print(json.dumps(result, indent=2, sort_keys=True))
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
