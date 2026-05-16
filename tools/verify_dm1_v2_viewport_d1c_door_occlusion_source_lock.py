#!/usr/bin/env python3
"""Source-lock DM1 V2 D1C front-door occlusion ordering to ReDMCSB."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_viewport_d1c_door_occlusion_source_lock.json"

LINE_ANCHORS = [
    ("DUNVIEW.C", 7727, "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C"),
    ("DUNVIEW.C", 7873, "case C17_ELEMENT_DOOR_FRONT"),
    ("DUNVIEW.C", 7874, "F0108_DUNGEONVIEW_DrawFloorOrnament"),
    ("DUNVIEW.C", 7875, "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT"),
    ("DUNVIEW.C", 7877, "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0704_puc_Bitmap_WallSet_DoorFrameTop_D1LCR"),
    ("DUNVIEW.C", 7901, "if (((DOOR*)G0284_apuc_ThingData[C00_THING_TYPE_DOOR])[L0218_ai_SquareAspect[M557_DOOR_THING_INDEX]].Button)"),
    ("DUNVIEW.C", 7908, "F0111_DUNGEONVIEW_DrawDoor"),
    ("DUNVIEW.C", 7910, "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT"),
    ("DUNVIEW.C", 7937, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"),
]

ORDER_SYMBOLS = [
    "C17_ELEMENT_DOOR_FRONT",
    "F0108_DUNGEONVIEW_DrawFloorOrnament",
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
    "G2112_DoorFrameTopD1LCR",
    "G2117_DoorFrameLeftD1C",
    "G2196_DoorFrameRightD1C",
    "F0110_DUNGEONVIEW_DrawDoorButton",
    "F0111_DUNGEONVIEW_DrawDoor",
    "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
    "T0124018:",
    "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
]

FIRESTAFF_REQUIRED = [
    ("CMakeLists.txt", "dm1_v2_viewport_d1c_door_occlusion_source_lock"),
    ("tools/verify_dm1_v2_completion_matrix.py", "dm1_v2_viewport_d1c_door_occlusion_source_lock"),
    ("include/dm1_v2_viewport_renderer_pc34.h", "DM1_V2_VIEW_SQUARE_D1C"),
    ("include/dm1_v2_viewport_renderer_pc34.h", "DM1_V2_ELEMENT_DOOR_FRONT"),
    ("src/dm1v2/dm1_v2_viewport_renderer_pc34.c", "DM1_V2_VIEW_SQUARE_D1C"),
    ("src/dm1v2/dm1_v2_viewport_renderer_pc34.c", "DUNVIEW.C:7873-7874"),
    ("src/dm1v2/dm1_v2_viewport_renderer_pc34.c", "DUNVIEW.C:7875"),
    ("src/dm1v2/dm1_v2_viewport_renderer_pc34.c", "DUNVIEW.C:7877-7910"),
    ("src/dm1v2/dm1_v2_viewport_renderer_pc34.c", "DUNVIEW.C:7910-7937"),
]


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def line_text(text: str, line: int) -> str:
    lines = text.splitlines()
    if line < 1 or line > len(lines):
        raise ValueError(f"line out of range: {line}")
    return lines[line - 1].strip()


def main() -> int:
    errors: list[str] = []
    anchors: list[dict[str, object]] = []
    dunview = read(SOURCE / "DUNVIEW.C")

    for filename, line, needle in LINE_ANCHORS:
        actual = ""
        try:
            actual = line_text(read(SOURCE / filename), line)
        except ValueError as exc:
            errors.append(f"{filename}:{line}: {exc}")
        if needle not in actual:
            errors.append(f"{filename}:{line}: expected {needle!r}, got {actual!r}")
        anchors.append({"file": filename, "line": line, "needle": needle, "text": actual})

    window = "\n".join(dunview.splitlines()[7872:7938])
    order: list[dict[str, int | str]] = []
    last = -1
    for symbol in ORDER_SYMBOLS:
        pos = window.find(symbol, last + 1)
        order.append({"symbol": symbol, "offset": pos})
        if pos < 0:
            errors.append(f"missing D1C door occlusion symbol after offset {last}: {symbol}")
        elif pos <= last:
            errors.append(f"out-of-order D1C door occlusion symbol: {symbol}")
        last = pos

    for rel, needle in FIRESTAFF_REQUIRED:
        text = read(ROOT / rel)
        if needle not in text:
            errors.append(f"missing Firestaff D1C door source-lock anchor {needle!r} in {rel}")

    result = {
        "status": "failed" if errors else "passed",
        "scope": "DM1 V2 viewport D1C front-door two-pass occlusion source lock",
        "redmcsbSourceRoot": str(SOURCE),
        "sourceAnchors": anchors,
        "doorOcclusionOrder": order,
        "firestaffContract": {
            "viewSquare": "DM1_V2_VIEW_SQUARE_D1C",
            "element": "DM1_V2_ELEMENT_DOOR_FRONT",
            "floorOrnament": "DUNVIEW.C:7873-7874",
            "pass1ObjectsBehindFrame": "DUNVIEW.C:7875",
            "doorFrameButtonAndDoor": "DUNVIEW.C:7877-7910",
            "pass2ObjectsInFrontOfFrame": "DUNVIEW.C:7910-7937",
        },
        "completionStatus": "source-locked draw-list evidence only; matched original/Firestaff pixels remain open",
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for error in errors:
            print(f"error: {error}")
        return 1
    print(f"dm1_v2_viewport_d1c_door_occlusion_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
