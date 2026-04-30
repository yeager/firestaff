#!/usr/bin/env python3
"""Verify the static boundary for a future DM2 startup file-open trace.

This deliberately does not claim a DM2 startup caller path.  It normalizes the
canonical SKULL.EXE file/image offsets, keeps ReDMCSB as a DM/CSB comparison
source only, and emits the exact dynamic-trace gate needed before a caller-path
claim is allowed.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
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
ARCHIVE = "Dungeon-Master-II-Skullkeep_DOS_EN.zip"
MEMBER = "skull.exe"
EXPECTED_SHA256 = "0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35"

REDMCSB_CHECKS = [
    {
        "id": "redmcsb-dm-csb-member-names",
        "file": "FILENAME.C",
        "range": "4-10",
        "function": "global filename definitions",
        "needles": ["DATA\\\\DUNGEON.DAT", "DATA\\\\GRAPHICS.DAT"],
    },
    {
        "id": "redmcsb-graphics-open-call",
        "file": "MEMORY.C",
        "range": "1212-1285",
        "function": "F0477_MEMORY_OpenGraphicsDat_CPSDF",
        "needles": ["F0770_FILE_Open(G2130_GraphicsDatFileName)", "C41_ERROR_UNABLE_TO_OPEN_GRAPHICS_DAT"],
    },
    {
        "id": "redmcsb-dungeon-open-call",
        "file": "LOADSAVE.C",
        "range": "2333-2377",
        "function": "dungeon load open block in F0436_LOADSAVE_LoadGameFile",
        "needles": ["F0770_FILE_Open(G1059_pc_DungeonFileName)", "F0770_FILE_Open(\"\\\\DUNGEON.DAT\")"],
    },
]

ASM_RANGES = [
    {"id": "dm2-open-wrapper", "range": "6139-6153", "decoded": "wrapper prepares args and calls generic open inner routine"},
    {"id": "dm2-open-inner", "range": "6154-6200", "decoded": "generic DOS open routine reaches AH=3Dh / int 21h"},
    {"id": "dm2-wrapper-caller-a", "range": "4761-4783", "decoded": "direct caller A passes BP+0Ah path buffer"},
    {"id": "dm2-wrapper-caller-b", "range": "4813-4826", "decoded": "direct caller B passes BP+0Ah path buffer"},
    {"id": "dm2-member-strings", "range": "461403-461472", "decoded": "db-only member strings; no xrefs in this dump"},
]

OFFSETS = {
    "direct_wrapper_call_a": {"image_offset": 0x1439, "bytes": bytes.fromhex("E8 53 05")},
    "direct_wrapper_call_b": {"image_offset": 0x1464, "bytes": bytes.fromhex("E8 28 05")},
    "open_wrapper_entry": {"image_offset": 0x198F, "bytes": bytes.fromhex("53 55 89 E5")},
    "open_inner_entry": {"image_offset": 0x19AF, "bytes": bytes.fromhex("53 51 52 56 57 55 89 E5")},
    "open_int21_ah3d": {"image_offset": 0x19DA, "bytes": bytes.fromhex("B4 3D CD 21")},
    "dungeon_dat_string": {"file_offset": 0x70C65, "bytes": b"DUNGEON.DAT"},
    "graphics_dat_string": {"file_offset": 0x70C98, "bytes": b"GRAPHICS.DAT"},
}


def line_slice(path: Path, spec: str) -> str:
    start, end = [int(part) for part in spec.split("-")]
    return "\n".join(path.read_text(encoding="latin1", errors="replace").splitlines()[start - 1 : end])


def source_audit(root: Path) -> tuple[list[dict[str, Any]], list[str]]:
    passed: list[dict[str, Any]] = []
    failures: list[str] = []
    for check in REDMCSB_CHECKS:
        path = root / check["file"]
        text = line_slice(path, check["range"]) if path.exists() else ""
        missing = [needle for needle in check["needles"] if needle not in text]
        if missing:
            failures.append(f"{check['id']}: {path}:{check['range']} missing {missing!r}")
        else:
            passed.append({k: check[k] for k in ("id", "file", "range", "function")})
            passed[-1]["path"] = str(path)
    return passed, failures


def read_exe(root: Path) -> bytes:
    with zipfile.ZipFile(root / ARCHIVE) as zf:
        return zf.read(MEMBER)


def verify_offsets(exe: bytes, header_bytes: int) -> tuple[dict[str, Any], list[str]]:
    failures: list[str] = []
    anchors: dict[str, Any] = {}
    for name, anchor in OFFSETS.items():
        file_offset = anchor.get("file_offset")
        image_offset = anchor.get("image_offset")
        if file_offset is None:
            file_offset = header_bytes + image_offset
        if image_offset is None:
            image_offset = file_offset - header_bytes
        expected = anchor["bytes"]
        got = exe[file_offset : file_offset + len(expected)]
        if got != expected:
            failures.append(f"{name}: file+{file_offset:#x} expected {expected.hex(' ').upper()} got {got.hex(' ').upper()}")
        anchors[name] = {
            "file_offset": hex(file_offset),
            "image_offset": hex(image_offset),
            "bytes": expected.hex(" ").upper(),
        }
    return anchors, failures


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--redmcsb-source", type=Path, default=REDMCSB_SOURCE)
    parser.add_argument("--dm2-canonical", type=Path, default=DM2_CANONICAL)
    parser.add_argument("--dm2-asm", type=Path, default=DM2_ASM)
    args = parser.parse_args()

    source_passed, failures = source_audit(args.redmcsb_source)
    exe = read_exe(args.dm2_canonical)
    sha256 = hashlib.sha256(exe).hexdigest()
    if sha256 != EXPECTED_SHA256:
        failures.append(f"dm2-skull-exe-sha256: got {sha256}")
    header_paragraphs = struct.unpack_from("<H", exe, 8)[0]
    entry_ip, entry_cs = struct.unpack_from("<HH", exe, 0x14)
    header_bytes = header_paragraphs * 16
    anchors, offset_failures = verify_offsets(exe, header_bytes)
    failures.extend(offset_failures)

    asm_evidence = []
    for item in ASM_RANGES:
        path = args.dm2_asm
        text = line_slice(path, item["range"]) if path.exists() else ""
        if not text:
            failures.append(f"{item['id']}: missing {path}:{item['range']}")
        asm_evidence.append({**item, "path": str(path)})

    result = {
        "schema": "firestaff.dm2_fileopen_dynamic_trace_boundary.v1",
        "policy": {
            "redmcsb": "source audit first, but DM/CSB comparison only; not DM2 C source",
            "dm2": "canonical SKULL.EXE plus SKULL.ASM disassembly only; no caller-path claim without runtime/xref bridge",
        },
        "redmcsb_source_audit": source_passed,
        "dm2_asm_ranges": asm_evidence,
        "dm2_exe": {
            "archive": str(args.dm2_canonical / ARCHIVE),
            "member": MEMBER,
            "sha256": sha256,
            "header_paragraphs": header_paragraphs,
            "header_bytes": header_bytes,
            "entry_cs_ip": f"{entry_cs:04x}:{entry_ip:04x}",
            "anchors": anchors,
        },
        "dynamic_trace_gate": {
            "break_or_watch": "At the canonical open_int21_ah3d anchor, capture AH and DS:DX on every INT 21h/AH=3Dh open attempt.",
            "pass_condition": "A trace frame must show DS:DX resolving to DUNGEON.DAT or GRAPHICS.DAT before we claim the startup member-string caller path.",
            "current_status": "blocked_pre_trace: static evidence proves the open routine and member strings, but not the runtime bridge between them.",
            "candidate_tools_on_n2": {name: shutil.which(name) for name in ["dosbox-x", "dosbox", "ndisasm", "objdump"]},
        },
        "failures": failures,
    }
    print(json.dumps(result, indent=2, sort_keys=True))
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
