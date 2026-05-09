#!/usr/bin/env python3
"""Pass445 DM1 V1 PC34 original data hash-lock gate.

This gate keeps DM1 V1 comparisons tied to exact local PC34 data files before
runtime/viewport evidence may cite DUNGEON.DAT or GRAPHICS.DAT.  Greatstone /
Daniel MD5 catalogue entries are treated as provenance cross-references only;
the local files are locked by computed SHA-256 and size.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
CANON = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
EXTRACTED = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34"
REGISTRY = Path.home() / ".openclaw/data/firestaff-graphics-hash-registry.md"
RED_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"


@dataclass(frozen=True)
class LockedFile:
    label: str
    path: Path
    filename: str
    variant: str
    size: int
    sha256: str
    md5: str | None = None


LOCKED_FILES = [
    LockedFile(
        label="dm1_pc34_english_graphics",
        path=CANON / "GRAPHICS.DAT",
        filename="GRAPHICS.DAT",
        variant="DM PC 3.4 English / I34E",
        size=363417,
        sha256="2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        md5="fa6b1aa29e191418713bf2cda93d962e",
    ),
    LockedFile(
        label="dm1_pc34_english_dungeon",
        path=CANON / "DUNGEON.DAT",
        filename="DUNGEON.DAT",
        variant="DM PC 3.4 English / I34E",
        size=33357,
        sha256="d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    ),
    LockedFile(
        label="dm1_pc34_multilanguage_graphics_crossref",
        path=EXTRACTED / "DungeonMasterPC34Multilingual/EUDATA/GRAPHICS.DAT",
        filename="GRAPHICS.DAT",
        variant="DM PC 3.4 Multilanguage / EUDATA",
        size=398925,
        sha256="291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc",
        md5="f934d97e43e1ba6e5159839acbcd0611",
    ),
    LockedFile(
        label="dm1_pc34_multilanguage_dungeon_crossref",
        path=EXTRACTED / "DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT",
        filename="DUNGEON.DAT",
        variant="DM PC 3.4 Multilanguage / EUDATA; same dungeon payload as English",
        size=33357,
        sha256="d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    ),
]

REGISTRY_CROSSREFS = {
    "FA6B1AA29E191418713BF2CDA93D962E": "DM PC 3.4 English GRAPHICS.DAT",
    "F934D97E43E1BA6E5159839ACBCD0611": "DM PC 3.4 Multilanguage GRAPHICS.DAT",
}

REDMCSB_NEEDLES = {
    "FILENAME.C": [
        'G1059_pc_DungeonFileName = "DATA\\\\DUNGEON.DAT"',
        'G2130_GraphicsDatFileName = "DATA\\\\GRAPHICS.DAT"',
        'G2175_ac_DungeonFileName[] = "EUDATA\\\\DUNGEON~.DAT"',
        'G2130_GraphicsDatFileName = "EUDATA\\\\GRAPHICS.DAT"',
    ],
    "MEMORY.C": [
        "F0477_MEMORY_OpenGraphicsDat_CPSDF",
        'F0770_FILE_Open(G2130_GraphicsDatFileName)',
        "F0479_MEMORY_ReadGraphicsDatHeader",
    ],
    "CEDTINCA.C": [
        "BOOLEAN F7063_LoadDungeon",
        "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.Header",
        "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.RawMapData",
    ],
}

CMMAKE_TEST_NAME = "pass445_dm1_v1_pc34_data_hash_lock"
PROVENANCE_DOC = ROOT / "parity-evidence/dm1_pc34_source_data_provenance.md"


def digest(path: Path, algorithm: str) -> str:
    h = hashlib.new(algorithm)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def check_locked_files() -> bool:
    ok = True
    for item in LOCKED_FILES:
        if not item.path.is_file():
            print(f"FAIL {item.label}: missing {item.path}")
            ok = False
            continue
        actual_size = item.path.stat().st_size
        actual_sha = digest(item.path, "sha256")
        actual_md5 = digest(item.path, "md5") if item.md5 else None
        file_ok = actual_size == item.size and actual_sha == item.sha256 and (item.md5 is None or actual_md5 == item.md5)
        status = "PASS" if file_ok else "FAIL"
        print(f"{status} {item.label}: filename={item.filename} variant={item.variant}")
        print(f"  path={item.path}")
        print(f"  sha256={actual_sha} bytes={actual_size}")
        if item.md5:
            print(f"  md5={actual_md5}")
        if not file_ok:
            print(f"  expected sha256={item.sha256} bytes={item.size}")
            if item.md5:
                print(f"  expected md5={item.md5}")
            ok = False
    return ok


def check_registry_crossrefs() -> bool:
    if not REGISTRY.is_file():
        print(f"FAIL registry: missing {REGISTRY}")
        return False
    text = REGISTRY.read_text(encoding="utf-8", errors="replace")
    ok = True
    for md5_upper, label in REGISTRY_CROSSREFS.items():
        if md5_upper in text and label in text:
            print(f"PASS registry crossref: {md5_upper} {label}")
        else:
            print(f"FAIL registry crossref missing: {md5_upper} {label}")
            ok = False
    return ok


def check_redmcsb_audit_anchors() -> bool:
    ok = True
    for rel, needles in REDMCSB_NEEDLES.items():
        path = RED_ROOT / rel
        if not path.is_file():
            print(f"FAIL ReDMCSB anchor {rel}: missing {path}")
            ok = False
            continue
        text = path.read_text(encoding="latin-1", errors="replace")
        for needle in needles:
            if needle in text:
                print(f"PASS ReDMCSB anchor {rel}: {needle}")
            else:
                print(f"FAIL ReDMCSB anchor {rel}: missing {needle}")
                ok = False
    return ok


def check_gate_registration() -> bool:
    cmake = ROOT / "CMakeLists.txt"
    text = cmake.read_text(encoding="utf-8")
    ok = True
    for needle in [CMMAKE_TEST_NAME, "verify_pass445_dm1_v1_pc34_data_hash_lock.py"]:
        if needle not in text:
            print(f"FAIL CMake gate registration: missing {needle}")
            ok = False
    if ok:
        print(f"PASS CMake gate registration: {CMMAKE_TEST_NAME}")
    return ok


def check_provenance_doc() -> bool:
    if not PROVENANCE_DOC.is_file():
        print(f"FAIL provenance doc missing: {PROVENANCE_DOC}")
        return False
    text = PROVENANCE_DOC.read_text(encoding="utf-8", errors="replace")
    required = [
        "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "fa6b1aa29e191418713bf2cda93d962e",
        "f934d97e43e1ba6e5159839acbcd0611",
    ]
    missing = [needle for needle in required if needle not in text]
    if missing:
        print(f"FAIL provenance doc missing locked hashes: {missing}")
        return False
    print("PASS provenance doc carries DM1 PC34 dungeon/graphics hash-lock evidence")
    return True


def main() -> int:
    ok = check_locked_files()
    ok = check_registry_crossrefs() and ok
    ok = check_redmcsb_audit_anchors() and ok
    ok = check_provenance_doc() and ok
    ok = check_gate_registration() and ok
    if ok:
        print("PASS pass445 DM1 V1 PC34 original data hash lock")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
