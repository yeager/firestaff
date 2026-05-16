#!/usr/bin/env python3
"""Pass486: source-lock DM1 V1 door-button viewport occlusion/draw order.

Evidence-only gate. It pins ReDMCSB door-button coordinate/palette/clickable
handling and the door-front branch order to Firestaff's center/D3R button paths,
so button blits cannot drift in front of nearer blocking doors or into squares
where PC-34 never draws a button.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass486_dm1_v1_door_button_occlusion_source_lock"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / "pass486_dm1_v1_door_button_occlusion_source_lock.md"
STATUS = "PASS486_DM1_V1_DOOR_BUTTON_OCCLUSION_SOURCE_LOCKED"


def read(path: Path, enc: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=enc, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing/out-of-order {needle!r}")
        pos = hit


def line_of(text: str, needle: str) -> int:
    return text.count("\n", 0, require(text, needle, "line lookup")) + 1


def slice_digest(path: Path, start: int, end: int, enc: str = "latin-1") -> str:
    blob = "\n".join(read(path, enc).splitlines()[start - 1:end])
    return hashlib.sha256(blob.encode("utf-8", "replace")).hexdigest()


def function_body(text: str, name: str) -> str:
    pat = re.compile(r"\b(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|size_t)\s+" + re.escape(name) + r"\s*\(")
    for m in pat.finditer(text):
        brace = text.find("{", m.end())
        if brace < 0:
            continue
        depth = 0
        for i in range(brace, len(text)):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    return text[m.start():i + 1]
        next_static = text.find("\nSTATICFUNCTION ", brace + 1)
        if next_static > 0:
            return text[m.start():next_static]
    raise AssertionError(f"missing function body {name}")


def redmcsb_audit() -> dict[str, list[int]]:
    d = read(RED / "DUNVIEW.C", "latin-1")
    defs = read(RED / "DEFS.H", "latin-1")

    require_order(d, [
        "unsigned char G0208_aaauc_Graphic558_DoorButtonCoordinateSets[1][4][6] = {",
        "{ { 199, 204, 41, 44, 8, 4}",
        "{ 136, 141, 41, 44, 8, 4}",
        "{ 144, 155, 42, 47, 8, 6}",
        "{ 160, 175, 44, 52, 8, 9}",
    ], "ReDMCSB door-button coordinate table")
    for needle in [
        "#define C0_VIEW_DOOR_BUTTON_D3R 0",
        "#define C1_VIEW_DOOR_BUTTON_D3C 1",
        "#define C2_VIEW_DOOR_BUTTON_D2C 2",
        "#define C3_VIEW_DOOR_BUTTON_D1C 3",
        "#define C1950_ZONE_DOOR_BUTTON",
    ]:
        require(defs, needle, "ReDMCSB door-button constants")
    require_order(d, [
        "STATICFUNCTION void F0110_DUNGEONVIEW_DrawDoorButton(",
        "if (P0122_i_DoorButtonOrdinal) {",
        "L0109_i_NativeBitmapIndex = P0122_i_DoorButtonOrdinal + M634_GRAPHIC_FIRST_DOOR_BUTTON;",
        "G0208_aaauc_Graphic558_DoorButtonCoordinateSets",
        "if (P0123_i_ViewDoorButtonIndex == C3_VIEW_DOOR_BUTTON_D1C)",
        "G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]",
        "F0129_VIDEO_BlitShrinkWithPaletteChanges",
        "F0791_DUNGEONVIEW_DrawBitmapXX",
        "C1950_ZONE_DOOR_BUTTON + G0197_auc_Graphic558_DoorButtonCoordinateSet[P0122_i_DoorButtonOrdinal] + P0123_i_ViewDoorButtonIndex",
        "C10_COLOR_FLESH",
    ], "ReDMCSB F0110 button blit/clickable path")
    for label, needles in {
        "D3R": ["STATICFUNCTION void F0117_DUNGEONVIEW_DrawSquareD3R(", "Button) {", "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C0_VIEW_DOOR_BUTTON_D3R);", "F0111_DUNGEONVIEW_DrawDoor"],
        "D3C": ["STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(", "Button) {", "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C1_VIEW_DOOR_BUTTON_D3C);", "F0111_DUNGEONVIEW_DrawDoor"],
        "D2C": ["STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(", "Button) {", "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C2_VIEW_DOOR_BUTTON_D2C);", "F0111_DUNGEONVIEW_DrawDoor"],
        "D1C": ["STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(", "Button) {", "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C3_VIEW_DOOR_BUTTON_D1C);", "F0111_DUNGEONVIEW_DrawDoor"],
    }.items():
        require_order(d, needles, f"ReDMCSB {label} button-before-door order")
    for name in ["F0116_DUNGEONVIEW_DrawSquareD3L", "F0119_DUNGEONVIEW_DrawSquareD2L", "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "F0122_DUNGEONVIEW_DrawSquareD1L", "F0123_DUNGEONVIEW_DrawSquareD1R"]:
        body = function_body(d, name)
        if "F0110_DUNGEONVIEW_DrawDoorButton" in body:
            raise AssertionError(f"unexpected door button draw in {name}")

    return {
        "DUNVIEW.C:door_button_coordinate_table": [line_of(d, "G0208_aaauc_Graphic558_DoorButtonCoordinateSets"), line_of(d, "OBJECT_ASPECT G0209_as_Graphic558_ObjectAspects") - 1],
        "DUNVIEW.C:F0110_door_button_blit": [line_of(d, "STATICFUNCTION void F0110_DUNGEONVIEW_DrawDoorButton"), line_of(d, "STATICFUNCTION void F0111_DUNGEONVIEW_DrawDoor") - 1],
        "DUNVIEW.C:D3R_D3C_D2C_D1C_button_branches": [line_of(d, "STATICFUNCTION void F0117_DUNGEONVIEW_DrawSquareD3R"), line_of(d, "STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L") - 1],
        "DEFS.H:door_button_constants": [line_of(defs, "#define C0_VIEW_DOOR_BUTTON_D3R"), line_of(defs, "#define C1950_ZONE_DOOR_BUTTON")],
    }


def firestaff_audit() -> dict[str, int]:
    m11 = read(ROOT / "src/engine/m11_game_view.c")
    center = function_body(m11, "m11_draw_dm1_center_door_buttons")
    d3r = function_body(m11, "m11_draw_dm1_d3r_door_button")
    viewport = function_body(m11, "m11_draw_viewport")

    require_order(center, [
        "m11_dm1_nearest_blocking_center_door_depth(cells)",
        "cell = &cells[depth][1]",
        "!cell->hasDoorThing",
        "!state->world.things->doors[doorIdx].button",
        "M11_GFX_DOOR_BUTTON_BASE",
        "M11_VIEWPORT_X + kButtons[depth].dstX",
        "10",
        "depth == 2 ? kButtonD3Palette",
    ], "Firestaff nearest center-door button occlusion")
    for needle in ["160, 44, 16, 9", "144, 42, 12, 6", "136, 41, 6, 4"]:
        require(center, needle, "Firestaff center button coordinate")
    require_order(d3r, [
        "3 > maxVisibleForward",
        "!m11_dm1_side_lane_clear_for_rel(cells, 3, 1)",
        "m11_sample_viewport_cell(state, 3, 1, &cell)",
        "m11_viewport_cell_is_open(&cell)",
        "!state->world.things->doors[doorIdx].button",
        "M11_VIEWPORT_X + 199",
        "M11_VIEWPORT_Y + 41",
        "6, 4",
        "10",
        "kButtonD3Palette",
    ], "Firestaff D3R side-door button guard")
    require_order(viewport, [
        "m11_draw_dm1_side_doors",
        "m11_draw_dm1_side_door_ornaments",
        "m11_draw_dm1_center_doors",
        "m11_draw_dm1_center_door_ornaments",
        "m11_draw_dm1_center_door_buttons",
        "m11_draw_dm1_d3r_door_button",
    ], "Firestaff door button layer placement")
    return {"centerButtonChecks": 8, "centerCoordinateChecks": 3, "d3rButtonChecks": 10, "viewportOrderChecks": 6}


def main() -> int:
    audited = redmcsb_audit()
    local = firestaff_audit()
    digests = {
        "DUNVIEW.C:1210-1216": slice_digest(RED / "DUNVIEW.C", 1210, 1216),
        "DUNVIEW.C:4119-4216": slice_digest(RED / "DUNVIEW.C", 4119, 4216),
        "DUNVIEW.C:6579-7908": slice_digest(RED / "DUNVIEW.C", 6579, 7908),
    }
    payload = {
        "status": STATUS,
        "redmcsbRoot": str(RED),
        "audited": audited,
        "sourceSliceSha256": digests,
        "firestaffNeedleCounts": local,
        "claims": [
            "Door-button viewport coordinates are source-locked to ReDMCSB G0208/DEFS constants.",
            "Buttons exist only on D3R, D3C, D2C, and D1C door-front branches and are drawn before F0111 draws the door panel.",
            "Firestaff keeps center buttons on the nearest blocking center door and keeps D3R side buttons behind same-lane occlusion.",
            "This gate is source/metadata evidence only; it makes no original-runtime or pixel parity claim.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    md = ["# Pass486 — DM1 V1 door-button occlusion source lock", "", f"Status: `{STATUS}`", "", "## Audited ReDMCSB anchors"]
    for k, v in audited.items():
        md.append(f"- `{k}` — `{v}`")
    md += ["", "## Source slice SHA-256"]
    for k, v in digests.items():
        md.append(f"- `{k}` — `{v}`")
    md += ["", "## Firestaff seams", json.dumps(local, indent=2, sort_keys=True), "", "No original-runtime/pixel parity claim is made by this gate."]
    OUT_MD.write_text("\n".join(md) + "\n", encoding="utf-8")
    print(f"pass486_dm1_v1_door_button_occlusion_source_lock=pass manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
