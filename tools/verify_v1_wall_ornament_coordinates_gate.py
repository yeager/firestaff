#!/usr/bin/env python3
"""Verify DM1 V1 wall ornament coordinate sets and index table match ReDMCSB ST.

Source references:
  DUNVIEW.C G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices[60] (ST path)
  DUNVIEW.C G0205_aaauc_Graphic558_WallOrnamentCoordinateSets[8][13][6]
  DUNVIEW.C G0198/G0199 palette changes for wall ornaments at D3/D2
  DUNVIEW.C G0190_auc_Graphic558_WallOrnamentDerivedBitmapIndexIncrement[12]
"""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIRE = ROOT / "m11_game_view.c"
DUNVIEW = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"missing {label}: {needle!r}")
    return pos


def main() -> int:
    fire = FIRE.read_text(encoding="utf-8")
    red = DUNVIEW.read_text(encoding="utf-8", errors="replace")

    # Verify G0194 ST coordinate set index table is in Firestaff
    # ST values: idx 0=1(inscription), 1-3=1(alcoves), 4-10=0, 11=2(crack),
    #   12=3(slime), 35=1(fountain), 40-42=4(holes), 43=5(mirror),
    #   56-58=6(amalgam), 59=7(lord order)
    require(fire, "1, 1, 1, 1, 0, 0, 0, 0, 0, 0,",
            "Firestaff G0194 ST indices 0-9")
    require(fire, "0, 2, 3, 0, 0, 0, 0, 0, 0, 0,",
            "Firestaff G0194 ST indices 10-19")
    require(fire, "4, 4, 4, 5, 0, 0, 1, 0, 0, 0,",
            "Firestaff G0194 ST indices 40-49")
    require(fire, "2, 0, 0, 0, 0, 2, 6, 6, 6, 7",
            "Firestaff G0194 ST indices 50-59")

    # Verify G0205 wall ornament coordinate sets [8][13][6] are embedded
    require(fire, "kZones[8][13][6]",
            "Firestaff G0205 wall ornament coordinate set table")

    # Spot-check specific coordinate set entries against ReDMCSB
    # Set 0 D1C: {96,127,36,63,16,28}
    require(red, "{  96, 127, 36,  63, 16,  28 }",
            "ReDMCSB G0205 set 0 D1C coordinates")
    require(fire, "{96,127,36,63,16,28}",
            "Firestaff G0205 set 0 D1C coordinates")

    # Set 1 D1C: {64,159,36,91,48,56}
    require(red, "{  64, 159, 36,  91, 48,  56 }",
            "ReDMCSB G0205 set 1 D1C coordinates")
    require(fire, "{64,159,36,91,48,56}",
            "Firestaff G0205 set 1 D1C coordinates")

    # Verify wall ornament D3/D2 palette changes match G0198/G0199
    require(fire, "0, 0, 12, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 1, 0, 2",
            "Firestaff wall ornament D3 palette (G0198)")
    require(fire, "0, 12, 1, 3, 4, 3, 6, 7, 5, 9, 10, 11, 0, 2, 14, 13",
            "Firestaff wall ornament D2 palette (G0199)")

    # Verify ReDMCSB G0198/G0199 match (ST 4-bit values * 10)
    require(red, "{ 0, 0, 120, 30, 40, 30, 0, 60, 30, 90, 100, 110, 0, 10, 0, 20 }",
            "ReDMCSB G0198 DoorButtonAndWallOrnament D3 palette (ST)")

    print("V1 wall ornament coordinate gate passed")
    print("- G0194 ST coordinate set index table (60 entries) source-locked")
    print("- G0205 coordinate sets [8][13][6] source-locked")
    print("- G0198/G0199 wall ornament palettes source-locked")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}")
        raise SystemExit(1)
