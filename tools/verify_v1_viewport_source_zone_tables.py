#!/usr/bin/env python3
"""Verify V1 viewport content anchor tables against source layout-696 zones.

This source/evidence gate is intentionally independent of an original party
route.  It locks the Firestaff viewport item/projectile/creature anchor tables
(C2500/C2900/C3200 families) to the reconstructed DM1 PC 3.4 GRAPHICS.DAT
layout record dump so static-no-party original captures cannot hide a drift in
world visual placement.
"""
from __future__ import annotations

from collections import Counter
from pathlib import Path
import hashlib
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src/engine/m11_game_view.c"
ZONES = ROOT / "data/zones_h_reconstruction.json"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CANON_GRAPHICS = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT"
EXPECTED_DM1_PC34_GRAPHICS_SHA256 = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def extract_array_pairs(text: str, array_name: str) -> tuple[int, list[tuple[int, int]]]:
    marker = f"static const short {array_name}"
    start = text.find(marker)
    if start < 0:
        raise AssertionError(f"missing array {array_name}")
    eq = text.find("=", start)
    end = text.find(";", eq)
    if eq < 0 or end < 0:
        raise AssertionError(f"unterminated array {array_name}")
    body = text[eq:end]
    pairs = [(int(a), int(b)) for a, b in re.findall(r"\{\s*(-?\d+)\s*,\s*(-?\d+)\s*\}", body)]
    if not pairs:
        raise AssertionError(f"array {array_name} yielded no coordinate pairs")
    return line_no(text, start), pairs


def zone_pairs(records: dict[str, dict[str, int]], first: int, count: int) -> list[tuple[int, int]]:
    pairs: list[tuple[int, int]] = []
    for zone_id in range(first, first + count):
        rec = records.get(str(zone_id))
        if rec is None:
            raise AssertionError(f"missing C{zone_id:04d} in zones_h_reconstruction.json")
        if rec.get("type") != 7 or rec.get("parent") != 4:
            raise AssertionError(
                f"C{zone_id:04d} has unexpected type/parent {rec.get('type')}/{rec.get('parent')}"
            )
        pairs.append((int(rec["d1"]), int(rec["d2"])))
    return pairs


def require_equal(label: str, got: list[tuple[int, int]], expected: list[tuple[int, int]]) -> None:
    if got != expected:
        for idx, (g, e) in enumerate(zip(got, expected)):
            if g != e:
                raise AssertionError(f"{label}: first mismatch at offset {idx}: code {g} != source {e}")
        raise AssertionError(f"{label}: length mismatch code={len(got)} source={len(expected)}")


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def require_ordered_needles(label: str, path: Path, needles: list[str]) -> list[tuple[int, str]]:
    text = path.read_text(encoding="latin-1", errors="replace")
    cursor = 0
    hits: list[tuple[int, str]] = []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            raise AssertionError(f"{label}: missing ordered source marker {needle!r}")
        hits.append((line_no(text, pos), needle))
        cursor = pos + len(needle)
    return hits


def require_multiset_subset(label: str, got: list[tuple[int, int]], source: list[tuple[int, int]]) -> None:
    missing = Counter(got) - Counter(source)
    if missing:
        sample = ", ".join(f"{pair}×{count}" for pair, count in missing.most_common(5))
        raise AssertionError(f"{label}: code coordinates not present in C3200 source range: {sample}")


def main() -> int:
    text = SRC.read_text(encoding="utf-8")
    data = json.loads(ZONES.read_text(encoding="utf-8"))
    records = data.get("records", {})
    prov = data.get("$provenance", {})
    sha = prov.get("source_sha256", "unknown")
    if sha != EXPECTED_DM1_PC34_GRAPHICS_SHA256:
        raise AssertionError(f"unexpected GRAPHICS.DAT source sha256 {sha}")
    if not CANON_GRAPHICS.exists():
        raise AssertionError(f"missing canonical DM1 PC34 GRAPHICS.DAT at {CANON_GRAPHICS}")
    actual_sha = sha256(CANON_GRAPHICS)
    if actual_sha != EXPECTED_DM1_PC34_GRAPHICS_SHA256:
        raise AssertionError(f"canonical GRAPHICS.DAT sha256 {actual_sha} != {EXPECTED_DM1_PC34_GRAPHICS_SHA256}")

    redmcsb_hits: list[str] = []
    coord_hits = require_ordered_needles(
        "ReDMCSB PC34 layout load path",
        RED / "COORD.C",
        [
            "LAYOUT_RANGE* G2174_ps_LayoutData = NULL;",
            "if ((L2300_i_ = M007_GET(P2131_i_ZoneIndex, MASK0x8000_SHIFT_OBJECTS_AND_CREATURES)) != 0)",
            "F0639_LoadLayoutRanges(",
            "void F0640_LoadLayoutData(",
            "F0639_LoadLayoutRanges(M776_CAST_PUI(L2322_puc_Buffer), L2323_ul_ByteCount, &G2174_ps_LayoutData, F0789_AllocateLayoutRange);",
            "F0640_LoadLayoutData(C696_GRAPHIC_LAYOUT);",
        ],
    )
    redmcsb_hits.append(f"COORD.C:{coord_hits[0][0]}-{coord_hits[-1][0]} loads PC34 layout-696 and applies object/creature shifts")
    dunview_hits = require_ordered_needles(
        "ReDMCSB F0115 C2500 routing",
        RED / "DUNVIEW.C",
        [
            "char G2028_ac_ViewSquareIndexTo[23]",
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "L2476_i_ = G2028_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex];",
            "L2474_i_ZoneIndex = (C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES) + (L2476_i_ * 4) + AL0126_i_ViewCell;",
        ],
    )
    redmcsb_hits.append(f"DUNVIEW.C:{dunview_hits[0][0]}-{dunview_hits[-1][0]} maps visible squares to C2500 object rows")
    defs_hits = require_ordered_needles(
        "ReDMCSB C2500 zone constants",
        RED / "DEFS.H",
        [
            "#define MASK0x8000_SHIFT_OBJECTS_AND_CREATURES",
            "#define C2500_ZONE_",
            "#define C2900_ZONE_",
            "#define C3200_ZONE_",
        ],
    )
    redmcsb_hits.append(f"DEFS.H:{defs_hits[0][0]}-{defs_hits[-1][0]} defines shifted object/creature and content zone families")

    ok: list[str] = []

    c2500_line, c2500_raw = extract_array_pairs(text, "kC2500Raw")
    require_equal("C2500 object/item anchor table", c2500_raw, zone_pairs(records, 2500, len(c2500_raw)))
    ok.append(f"C2500 object/item anchors: {len(c2500_raw)} records match layout-696 C2500..C{2500 + len(c2500_raw) - 1} at m11_game_view.c:{c2500_line}")

    c2500_legacy_line, c2500 = extract_array_pairs(text, "kC2500")
    require_equal("C2500 legacy renderer subset", c2500, c2500_raw[: len(c2500)])
    ok.append(f"C2500 renderer subset: {len(c2500)} records match the first source rows at m11_game_view.c:{c2500_legacy_line}")

    c2900_line, c2900_raw = extract_array_pairs(text, "kC2900Raw")
    require_equal("C2900 projectile anchor table", c2900_raw, zone_pairs(records, 2900, len(c2900_raw)))
    ok.append(f"C2900 projectile anchors: {len(c2900_raw)} records match layout-696 C2900..C{2900 + len(c2900_raw) - 1} at m11_game_view.c:{c2900_line}")

    c2900_legacy_line, c2900 = extract_array_pairs(text, "kC2900")
    require_equal("C2900 legacy renderer subset", c2900, c2900_raw[: len(c2900)])
    ok.append(f"C2900 renderer subset: {len(c2900)} records match the first source rows at m11_game_view.c:{c2900_legacy_line}")

    c3200_center_line, c3200_center = extract_array_pairs(text, "kC3200Center")
    c3200_side_line, c3200_side = extract_array_pairs(text, "kC3200Side")
    c3200_source = zone_pairs(records, 3200, 195)
    require_multiset_subset("C3200 center creature anchors", c3200_center, c3200_source)
    require_multiset_subset("C3200 side creature anchors", c3200_side, c3200_source)
    ok.append(f"C3200 center creature anchors: {len(c3200_center)} code points are source C3200-family records at m11_game_view.c:{c3200_center_line}")
    ok.append(f"C3200 side creature anchors: {len(c3200_side)} code points are source C3200-family records at m11_game_view.c:{c3200_side_line}")

    print("V1 viewport content zone table verification passed")
    print(f"- canonical GRAPHICS.DAT: {CANON_GRAPHICS} sha256={actual_sha}")
    print(f"- source: zones_h_reconstruction.json GRAPHICS.DAT sha256={sha}")
    for line in redmcsb_hits:
        print(f"- ReDMCSB anchor: {line}")
    for line in ok:
        print(f"- {line}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
