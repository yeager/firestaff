#!/usr/bin/env python3
"""Pass509: source-lock DM1 V1 wall-set startup binding."""
from __future__ import annotations
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-original-games/DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.md"
LOCAL = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
OUT_DIR = ROOT / "parity-evidence/verification/pass509_dm1_v1_wallset_startup_binding"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence/pass509_dm1_v1_wallset_startup_binding.md"

EXPECTED_WALL_GRAPHICS = {
    "C00_WALL_D0R": ("M661_GRAPHIC_WALLSET_0_D0R", 93, -17),
    "C01_WALL_D0L": ("M662_GRAPHIC_WALLSET_0_D0L", 94, -16),
    "C02_WALL_D1R": ("C095_GRAPHIC_WALLSET_0_D1R", 95, -15),
    "C03_WALL_D1L": ("C096_GRAPHIC_WALLSET_0_D1L", 96, -14),
    "C04_WALL_D1C": ("C097_GRAPHIC_WALLSET_0_D1C", 97, -13),
    "C05_WALL_D2R2": ("C098_GRAPHIC_WALLSET_0_D2R2", 98, -9),
    "C06_WALL_D2L2": ("C099_GRAPHIC_WALLSET_0_D2L2", 99, -8),
    "C07_WALL_D2R": ("C100_GRAPHIC_WALLSET_0_D2R", 100, -12),
    "C08_WALL_D2L": ("C101_GRAPHIC_WALLSET_0_D2L", 101, -11),
    "C09_WALL_D2C": ("C102_GRAPHIC_WALLSET_0_D2C", 102, -10),
    "C10_WALL_D3R2": ("C103_GRAPHIC_WALLSET_0_D3R2", 103, -4),
    "C11_WALL_D3L2": ("M663_GRAPHIC_WALLSET_0_D3L2", 104, -3),
    "C12_WALL_D3R": ("C105_GRAPHIC_WALLSET_0_D3R", 105, -7),
    "C13_WALL_D3L": ("C106_GRAPHIC_WALLSET_0_D3L", 106, -6),
    "C14_WALL_D3C": ("C107_GRAPHIC_WALLSET_0_D3C", 107, -5),
}
LOCAL_ARRAY_ORDER = [
    "C00_WALL_D0R", "C01_WALL_D0L", "C02_WALL_D1R", "C03_WALL_D1L", "C04_WALL_D1C",
    "C05_WALL_D2R2", "C06_WALL_D2L2", "C07_WALL_D2R", "C08_WALL_D2L", "C09_WALL_D2C",
    "C10_WALL_D3R2", "C11_WALL_D3L2", "C12_WALL_D3R", "C13_WALL_D3L", "C14_WALL_D3C",
]

def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def line_of(text: str, needle: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {needle!r}")
    return text.count("\n", 0, pos) + 1

def slice_hash(path: Path, start: int, end: int) -> str:
    lines = read(path).splitlines()
    return hashlib.sha256("\n".join(lines[start - 1:end]).encode("utf-8", "replace")).hexdigest()

def parse_defs_values(defs: str) -> dict[str, int]:
    values: dict[str, int] = {}
    names = [v[0] for v in EXPECTED_WALL_GRAPHICS.values()] + ["M646_GRAPHIC_FIRST_WALL_SET", "M647_WALL_SET_GRAPHIC_COUNT"]
    for name in names:
        matches = re.findall(r"^\s*#define\s+" + re.escape(name) + r"\s+(\d+)\s*$", defs, flags=re.MULTILINE)
        if not matches:
            raise AssertionError(f"missing DEFS.H value {name}")
        values[name] = int(matches[-1])
    return values

def parse_local_default_wall_set(local: str) -> list[int]:
    m = re.search(r"static const int16_t default_wall_set\[DM1_WALL_SET_COUNT\]\s*=\s*\{(?P<body>.*?)\};", local, flags=re.S)
    if not m:
        raise AssertionError("missing Firestaff default_wall_set array")
    body = re.sub(r"/\*.*?\*/", "", m.group("body"), flags=re.S)
    return [int(x) for x in re.findall(r"-?\d+", body)]

def verify() -> dict[str, object]:
    startup = read(RED / "STARTUP2.C")
    defs = read(RED / "DEFS.H")
    local = read(LOCAL)
    greatstone = read(GREATSTONE)
    defs_values = parse_defs_values(defs)
    failures: list[str] = []
    rows: list[dict[str, object]] = []
    for wall_name, (graphic_name, graphic_index, negative_slot) in EXPECTED_WALL_GRAPHICS.items():
        startup_needle = f"F0752_AllocateAndSetNegativeBitmapPointer(G2107_WallSet[{wall_name}], {graphic_name});"
        flipped_needle = f"F0752_AllocateAndSetNegativeBitmapPointer(G3048_WallSetFlipped[{wall_name}], {graphic_name});"
        startup_line = line_of(startup, startup_needle)
        flipped_line = line_of(startup, flipped_needle)
        actual_index = defs_values[graphic_name]
        if actual_index != graphic_index:
            failures.append(f"{wall_name}: index={actual_index} want={graphic_index}")
        rows.append({"wall": wall_name, "graphic": graphic_name, "graphicIndex": actual_index, "firestaffNegativeSlot": negative_slot, "startupLine": startup_line, "flippedLine": flipped_line})
    local_values = parse_local_default_wall_set(local)
    expected_local = [EXPECTED_WALL_GRAPHICS[name][2] for name in LOCAL_ARRAY_ORDER]
    if local_values != expected_local:
        failures.append(f"default_wall_set mismatch got={local_values!r} want={expected_local!r}")
    for needle in [
        "Result: **PASS**",
        "PC34 GRAPHICS.DAT",
        "SHA256: " + chr(96) + "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e" + chr(96),
        "| 93 | Dungeon Graphics - Wall (Right Side 0)",
        "| 107 | Dungeon Graphics - Wall (Front 3)",
        "Mismatches: 0",
    ]:
        if needle not in greatstone:
            failures.append(f"Greatstone manifest missing {needle!r}")
    graphics = DM1 / "GRAPHICS.DAT"
    dungeon = DM1 / "DUNGEON.DAT"
    anchors = {
        "GRAPHICS.DAT": {"sha256": sha256(graphics), "bytes": graphics.stat().st_size},
        "DUNGEON.DAT": {"sha256": sha256(dungeon), "bytes": dungeon.stat().st_size},
    }
    if anchors["GRAPHICS.DAT"]["sha256"] != "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e":
        failures.append("canonical GRAPHICS.DAT hash mismatch")
    if anchors["DUNGEON.DAT"]["sha256"] != "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85":
        failures.append("canonical DUNGEON.DAT hash mismatch")
    return {
        "gate": "pass509_dm1_v1_wallset_startup_binding",
        "status": "passed" if not failures else "failed",
        "redmcsbSourceRoot": str(RED),
        "sourceAudit": {
            "STARTUP2.C:625-639": slice_hash(RED / "STARTUP2.C", 625, 639),
            "STARTUP2.C:982-996": slice_hash(RED / "STARTUP2.C", 982, 996),
            "DEFS.H:2351-2373": slice_hash(RED / "DEFS.H", 2351, 2373),
            "DEFS.H:2428": slice_hash(RED / "DEFS.H", 2428, 2428),
        },
        "rows": rows,
        "firestaffDefaultWallSet": local_values,
        "expectedDefaultWallSet": expected_local,
        "dm1Anchors": anchors,
        "greatstoneManifest": str(GREATSTONE),
        "claims": [
            "ReDMCSB STARTUP2.C binds all fifteen PC34 wall slots to GRAPHICS.DAT 93..107 before viewport rendering.",
            "The flipped wall-set table repeats the same source binding, so parity flip may swap geometry without changing asset identity.",
            "Firestaff default_wall_set stores the same negative slots relative to M646/M647 for C00..C14.",
            "Greatstone/local PC34 audit and canonical hashes identify the exact GRAPHICS.DAT/DUNGEON.DAT variant used by this evidence.",
        ],
        "failures": failures,
    }

def write_evidence(manifest: dict[str, object]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    anchors = manifest["dm1Anchors"]
    lines = [
        "# Pass509 - DM1 V1 wall-set startup binding",
        "",
        f"Status: {manifest['status']}",
        "",
        "## ReDMCSB anchors",
        "- STARTUP2.C:625-639 - PC34 G2107_WallSet[C00..C14] startup allocation.",
        "- STARTUP2.C:982-996 - flipped wall-set startup allocation uses the same GRAPHICS.DAT slots.",
        "- DEFS.H:2351-2373,2428 - PC34 wall-set base/count and GRAPHICS.DAT indices 93..107.",
        "",
        "## Variant anchors",
    ]
    for name, item in anchors.items():
        lines.append(f"- {name} - sha256 {item['sha256']}, bytes {item['bytes']}")
    lines += [
        "- Greatstone manifest: DM/_manifests/dm_pc34_greatstone_item_by_item_diff_20260510.md (Result: PASS, PC34 GRAPHICS.DAT mismatches 0).",
        "",
        "No original-runtime or pixel parity claim is made by this gate.",
    ]
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")

def main() -> int:
    manifest = verify()
    write_evidence(manifest)
    print(f"{manifest['status'].upper()} {manifest['gate']} manifest={OUT_JSON.relative_to(ROOT)}")
    for row in manifest["rows"]:
        print(f"- {row['wall']} -> {row['graphic']} index={row['graphicIndex']} neg={row['firestaffNegativeSlot']} STARTUP2.C:{row['startupLine']}/{row['flippedLine']}")
    for failure in manifest["failures"]:
        print(f"FAIL {failure}")
    return 0 if manifest["status"] == "passed" else 1

if __name__ == "__main__":
    raise SystemExit(main())
