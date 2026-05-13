#!/usr/bin/env python3
"""Pass446: guard against filename-only original asset identity claims.

Firestaff may use historical filenames as loader paths, but evidence/parity
claims for DUNGEON.DAT / GRAPHICS.DAT must name exact media variant + SHA-256.
Daniel's registry MD5 values are provenance cross-references only.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CANON = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
EXTRACTED = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34"
REGISTRY = Path.home() / ".openclaw/data/firestaff-graphics-hash-registry.md"
RED_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REPORT = ROOT / "parity-evidence/pass446_filename_only_asset_identity_audit.md"
EVIDENCE_JSON = ROOT / "parity-evidence/verification/pass446_filename_only_asset_identity_audit.json"

ASSETS = [
    {
        "label": "dm1_pc34_english_graphics",
        "variant": "DM PC 3.4 English / I34E",
        "path": CANON / "GRAPHICS.DAT",
        "filename": "GRAPHICS.DAT",
        "bytes": 363417,
        "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "md5_provenance": "fa6b1aa29e191418713bf2cda93d962e",
    },
    {
        "label": "dm1_pc34_english_dungeon",
        "variant": "DM PC 3.4 English / I34E",
        "path": CANON / "DUNGEON.DAT",
        "filename": "DUNGEON.DAT",
        "bytes": 33357,
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    },
    {
        "label": "dm1_pc34_multilanguage_graphics_crossref",
        "variant": "DM PC 3.4 Multilanguage / EUDATA",
        "path": EXTRACTED / "DungeonMasterPC34Multilingual/EUDATA/GRAPHICS.DAT",
        "filename": "GRAPHICS.DAT",
        "bytes": 398925,
        "sha256": "291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc",
        "md5_provenance": "f934d97e43e1ba6e5159839acbcd0611",
    },
    {
        "label": "dm1_pc34_multilanguage_dungeon_crossref",
        "variant": "DM PC 3.4 Multilanguage / EUDATA; same dungeon payload as English",
        "path": EXTRACTED / "DungeonMasterPC34Multilingual/EUDATA/DUNGEON.DAT",
        "filename": "DUNGEON.DAT",
        "bytes": 33357,
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    },
]

REDMCSB_ANCHORS = {
    "FILENAME.C": [
        (6, 'G1059_pc_DungeonFileName = "DATA\\\\DUNGEON.DAT"'),
        (9, 'G2130_GraphicsDatFileName = "DATA\\\\GRAPHICS.DAT"'),
        (45, 'G2175_ac_DungeonFileName[] = "EUDATA\\\\DUNGEON~.DAT"'),
        (51, 'G2130_GraphicsDatFileName = "EUDATA\\\\GRAPHICS.DAT"'),
    ],
    "MEMORY.C": [
        (1212, "void F0477_MEMORY_OpenGraphicsDat_CPSDF"),
        (1266, "G1134_ps_GraphicsDatFileHandle = F0770_FILE_Open(G2130_GraphicsDatFileName)"),
        (1269, "G0630_i_GraphicsDatFileHandle = F0770_FILE_Open(G2130_GraphicsDatFileName)"),
        (1330, "void F0479_MEMORY_ReadGraphicsDatHeader"),
    ],
    "CEDTINCA.C": [
        (5, "BOOLEAN F7063_LoadDungeon"),
        (18, "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.Header"),
        (110, "F7059_ReadDungeonPartWithChecksum((unsigned char*)P3584_ps_->Dungeon.RawMapData"),
    ],
    "DM.C": [
        (462, "BOOLEAN F8076_IsVGAGraphicsDetected"),
        (474, "BOOLEAN F8077_IsEGAGraphicsDetected"),
        (556, "void F8082_ExitBecauseFileNotFound"),
        (564, "int16_t F8083_DOS_OpenFile"),
        (590, "BOOLEAN F8085_FileExists"),
    ],
}

DOC_REQUIREMENTS = {
    "docs/plans/M12_PLAN.md": [
        "Current M12 startup status code is narrower: it tries a bounded list of expected filenames and then validates MD5",
        "Filename alone never identifies a variant.",
        "Runtime/parity evidence must name the exact variant and SHA-256",
        "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    ],
    "docs/design/CSB_V1_BOOTSTRAP_SCOUT.md": [
        "catalogue/status evidence, not CSB runtime identity",
        "filename-only match, should remain insufficient",
        "MD5 registry entries may be cited only as cross-reference/provenance",
    ],
    "parity-evidence/dm1_pc34_source_data_provenance.md": [
        "no Firestaff runtime/parity comparison may identify original dungeon/graphics data by basename alone",
        "291eb38eab683317a2500e13363148425f059a2d35f929257d809174f625a4dc",
        "f934d97e43e1ba6e5159839acbcd0611",
    ],
    "parity-evidence/pass446_filename_only_asset_identity_audit.md": [
        "Filename-only assumptions fixed or gated",
        "ReDMCSB loader/decode anchors",
        "MD5 provenance, not SHA-256 substitute",
    ],
}

FORBIDDEN_DOC_STRINGS = {
    "docs/plans/M12_PLAN.md": [
        "File names don't matter (users can rename files freely)",
        "Identical across all PC 3.4 language variants.",
        "b076a54e8b3c89tried9d6f3a2e1c8b4",
        "d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9",
    ],
}


def file_digest(path: Path, algorithm: str) -> str:
    h = hashlib.new(algorithm)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def line_has(path: Path, line_no: int, needle: str) -> bool:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return len(lines) >= line_no and needle in lines[line_no - 1]


def check_assets(errors: list[str]) -> list[dict[str, object]]:
    rows = []
    registry_text = REGISTRY.read_text(encoding="utf-8", errors="replace") if REGISTRY.is_file() else ""
    for item in ASSETS:
        path = item["path"]
        row = {k: (str(v) if isinstance(v, Path) else v) for k, v in item.items()}
        if not path.is_file():
            errors.append(f"missing asset {item['label']}: {path}")
            row["ok"] = False
            rows.append(row)
            continue
        actual_size = path.stat().st_size
        actual_sha = file_digest(path, "sha256")
        row["actual_bytes"] = actual_size
        row["actual_sha256"] = actual_sha
        ok = actual_size == item["bytes"] and actual_sha == item["sha256"]
        if item.get("md5_provenance"):
            actual_md5 = file_digest(path, "md5")
            row["actual_md5"] = actual_md5
            ok = ok and actual_md5 == item["md5_provenance"]
            row["provenanceRegistryPresent"] = REGISTRY.is_file()
            if REGISTRY.is_file() and str(item["md5_provenance"]).upper() not in registry_text:
                errors.append(f"registry missing MD5 provenance for {item['label']}: {item['md5_provenance']}")
                ok = False
        if not ok:
            errors.append(f"asset identity mismatch {item['label']}: sha256={actual_sha} bytes={actual_size}")
        row["ok"] = ok
        rows.append(row)
    return rows


def check_redmcsb(errors: list[str]) -> list[dict[str, object]]:
    rows = []
    for rel, anchors in REDMCSB_ANCHORS.items():
        path = RED_ROOT / rel
        for line_no, needle in anchors:
            ok = path.is_file() and line_has(path, line_no, needle)
            if not ok:
                errors.append(f"missing ReDMCSB anchor {rel}:{line_no}: {needle}")
            rows.append({"file": str(path), "line": line_no, "needle": needle, "ok": ok})
    return rows


def check_docs(errors: list[str]) -> None:
    for rel, needles in DOC_REQUIREMENTS.items():
        path = ROOT / rel
        if not path.is_file():
            errors.append(f"missing required doc {rel}")
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for needle in needles:
            if needle not in text:
                errors.append(f"{rel} missing required text: {needle}")
    for rel, needles in FORBIDDEN_DOC_STRINGS.items():
        path = ROOT / rel
        text = path.read_text(encoding="utf-8", errors="replace") if path.is_file() else ""
        for needle in needles:
            if needle in text:
                errors.append(f"{rel} still contains unsafe/placeholder text: {needle}")


def check_code_gates(errors: list[str]) -> None:
    asset_status = (ROOT / "asset_status_m12.c").read_text(encoding="utf-8", errors="replace")
    for needle in ["m12_file_md5_hex(path, md5Hex)", "strcmp(md5Hex, spec->md5) == 0"]:
        if needle not in asset_status:
            errors.append(f"asset_status_m12.c missing checksum gate: {needle}")
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8", errors="replace")
    for needle in ["pass446_filename_only_asset_identity_audit", "verify_pass446_filename_only_asset_identity_audit.py"]:
        if needle not in cmake:
            errors.append(f"CMakeLists.txt missing pass446 registration: {needle}")


def main() -> int:
    errors: list[str] = []
    asset_rows = check_assets(errors)
    red_rows = check_redmcsb(errors)
    check_docs(errors)
    check_code_gates(errors)
    EVIDENCE_JSON.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE_JSON.write_text(json.dumps({
        "schema": "pass446_filename_only_asset_identity_audit.v1",
        "status": "PASS" if not errors else "FAIL",
        "assetIdentities": asset_rows,
        "redmcsbAnchors": red_rows,
        "rule": "filenames are loader paths only; parity/runtime evidence needs exact variant + SHA-256",
        "errors": errors,
    }, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for err in errors:
            print(f"FAIL {err}")
        print(f"wrote {EVIDENCE_JSON}")
        return 1
    print("PASS pass446 filename-only asset identity audit")
    print(f"wrote {EVIDENCE_JSON}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
