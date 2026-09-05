#!/usr/bin/env python3
"""Pass489: source-lock Daniel DM1 V1 door/TITLE/end audit seam.

Evidence/source-lock gate only. It ties the current Firestaff seams for the
button door, TITLE cadence, and end animation to the repository ReDMCSB source
and authentic in-place PC 3.4 ZIP members without promoting pixel parity.
"""
from __future__ import annotations

import hashlib
import json
import re
import struct
import zlib
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED, ZIP_STORED
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]
RED = ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"
DM1_ZIP = Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
OUT_DIR = ROOT / "parity-evidence/verification/pass489_dm1_v1_door_title_end_source_audit"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence/pass489_dm1_v1_door_title_end_source_audit.md"
STATUS = "PASS489_DM1_V1_DOOR_TITLE_END_SOURCE_AUDIT_LOCKED"
MEMBER_SHA256 = {
    "TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    "DATA/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "DATA/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}


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


def line_re(text: str, pattern: str) -> int:
    rx = re.compile(pattern)
    for idx, line in enumerate(text.splitlines(), 1):
        if rx.search(line):
            return idx
    raise AssertionError(f"line regex missing {pattern!r}")


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_member_in_memory(path: Path, name: str) -> bytes:
    """Read the original member without extraction, tolerating DOS slash names."""
    raw = path.read_bytes()
    with ZipFile(path) as archive:
        info = archive.getinfo(name)
    off = info.header_offset
    if raw[off:off + 4] != b"PK\x03\x04":
        raise AssertionError(f"bad local header for {name}")
    name_len, extra_len = struct.unpack_from("<HH", raw, off + 26)
    start = off + 30 + name_len + extra_len
    packed = raw[start:start + info.compress_size]
    if info.compress_type == ZIP_STORED:
        data = packed
    elif info.compress_type == ZIP_DEFLATED:
        data = zlib.decompress(packed, -15)
    else:
        raise AssertionError(f"unsupported compression for {name}")
    if len(data) != info.file_size or (zlib.crc32(data) & 0xffffffff) != info.CRC:
        raise AssertionError(f"member integrity failed for {name}")
    return data


def slice_digest(path: Path, start: int, end: int, enc: str = "latin-1") -> str:
    lines = read(path, enc).splitlines()[start - 1:end]
    return hashlib.sha256("\n".join(lines).encode("utf-8", "replace")).hexdigest()


def function_body(text: str, name: str) -> str:
    pat = re.compile(r"\b(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|unsigned\s+int|const\s+char\*)\s+" + re.escape(name) + r"\s*\(")
    m = pat.search(text)
    if not m:
        raise AssertionError(f"missing function body {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing function brace {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[m.start():i + 1]
    raise AssertionError(f"unterminated function body {name}")


def source_audit() -> dict[str, object]:
    dunview = read(RED / "DUNVIEW.C", "latin-1")
    defs = read(RED / "DEFS.H", "latin-1")
    title = read(RED / "TITLE.C", "latin-1")
    endgame = read(RED / "ENDGAME.C", "latin-1")
    anim = read(RED / "ANIM.C", "latin-1")
    data = read(RED / "DATA.C", "latin-1")

    require_order(dunview, [
        "G0208_aaauc_Graphic558_DoorButtonCoordinateSets",
        "{ { 199, 204, 41, 44, 8, 4}",
        "{ 136, 141, 41, 44, 8, 4}",
        "{ 144, 155, 42, 47, 8, 6}",
        "{ 160, 175, 44, 52, 8, 9}",
    ], "ReDMCSB button coordinate table")
    require_order(dunview, [
        "F0110_DUNGEONVIEW_DrawDoorButton",
        "P0122_i_DoorButtonOrdinal + M634_GRAPHIC_FIRST_DOOR_BUTTON",
        "G0208_aaauc_Graphic558_DoorButtonCoordinateSets",
        "G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]",
        "C1950_ZONE_DOOR_BUTTON",
    ], "ReDMCSB button blit/click route")
    for needle in ["C0_VIEW_DOOR_BUTTON_D3R", "C1_VIEW_DOOR_BUTTON_D3C", "C2_VIEW_DOOR_BUTTON_D2C", "C3_VIEW_DOOR_BUTTON_D1C", "C1950_ZONE_DOOR_BUTTON"]:
        require(defs, needle, "ReDMCSB button constants")

    require_order(title, [
        "F0437_STARTEND_DrawTitle",
        "F0660_(CM58_NEGGRAPHIC_TITLE_PRESENTS",
        "F0129_VIDEO_BlitShrinkWithPaletteChanges",
        "M526_WaitVerticalBlank();",
        "F0021_MAIN_BlitToScreen",
        "F0022_MAIN_Delay(20);",
        "F0660_(CM60_NEGGRAPHIC_TITLE_STRIKES_BACK",
        "F0022_MAIN_Delay(2);",
    ], "ReDMCSB title cadence")

    require_order(endgame, [
        "F0444_STARTEND_Endgame",
        "C006_GRAPHIC_THE_END",
        "F0436_STARTEND_FadeToPalette",
        "F0022_MAIN_Delay(300)",
        "AL1409_i_VerticalBlankCount = 900",
    ], "ReDMCSB end animation/restart cadence")
    require_order(anim, [
        "typedef struct {",
        "ANIMDESC",
        "F9008_OpenPRIM",
        "L4760_[0] = (P4236_->Parameter != 0)",
        "if (P4236_->Parameter ==",
        "void F1209_",
        "void F1210_",
        "VDEO_13_WaitVerticalBlank",
    ], "ReDMCSB ANIM/ENDA control-flow audit")
    require(data, "G0012_ai_Graphic562_Box_Endgame_TheEnd", "ReDMCSB THE END box")

    return {
        "DUNVIEW.C:button_table_line": line_of(dunview, "G0208_aaauc_Graphic558_DoorButtonCoordinateSets"),
        "DUNVIEW.C:F0110_line": line_of(dunview, "F0110_DUNGEONVIEW_DrawDoorButton"),
        "TITLE.C:F0437_line": line_of(title, "F0437_STARTEND_DrawTitle"),
        "ENDGAME.C:F0444_line": line_re(endgame, r"^void\s+F0444_STARTEND_Endgame\s*\("),
        "ANIM.C:ANIMDESC_line": line_of(anim, "} ANIMDESC;"),
        "ANIM.C:F1208_line": line_of(anim, "void F1208_"),
        "ANIM.C:F1209_line": line_of(anim, "void F1209_"),
        "ANIM.C:F1210_line": line_of(anim, "void F1210_"),
        "DATA.C:the_end_box_line": line_of(data, "G0012_ai_Graphic562_Box_Endgame_TheEnd"),
        "slice_sha256": {
            "DUNVIEW.C:button_table": slice_digest(RED / "DUNVIEW.C", line_of(dunview, "G0208_aaauc_Graphic558_DoorButtonCoordinateSets"), line_of(dunview, "OBJECT_ASPECT G0209_as_Graphic558_ObjectAspects") - 1),
            "TITLE.C:F0437_window": slice_digest(RED / "TITLE.C", line_of(title, "F0437_STARTEND_DrawTitle"), min(len(title.splitlines()), line_of(title, "F0469_MEMORY_FreeAtHeapTop") + 4)),
            "ENDGAME.C:F0444_window": slice_digest(RED / "ENDGAME.C", line_of(endgame, "F0444_STARTEND_Endgame"), line_of(endgame, "F0445_STARTEND_FuseSequenceUpdate") - 1),
            "ANIM.C:desc_open_end_hooks": slice_digest(RED / "ANIM.C", line_of(anim, "typedef struct { /* Amiga"), line_of(anim, "void F1211_") - 1),
        },
    }


def firestaff_audit() -> dict[str, object]:
    view = read(ROOT / "src/engine/m11_game_view.c")
    main_loop = read(ROOT / "src/engine/main_loop_m11.c")
    title = read(ROOT / "src/frontend/title_frontend_v1.c")
    endgame = read(ROOT / "src/frontend/endgame_frontend_pc34_compat.c")

    callback = function_body(view, "m11_dm1_f0128_execute_source_step")
    require_order(callback, [
        "M11_DM1_F0128_EXECUTE_DOOR_FRAME",
        "M11_DM1_F0128_EXECUTE_DOOR_BUTTON",
        "M11_DM1_F0128_EXECUTE_DOOR_MATERIAL",
    ], "Firestaff callback door phase ownership")
    require(callback, "m11_draw_dm1_center_door_buttons", "center F0110 callback")
    require(callback, "m11_draw_dm1_d3r_door_button", "D3R F0110 callback")
    require_order(function_body(view, "m11_draw_viewport"), [
        "m11_dm1_f0128_dispatch_door_frame_square",
        "m11_dm1_f0128_dispatch_door_button_square",
        "m11_dm1_f0128_dispatch_door_material_square",
    ], "Firestaff per-square door callback order")
    require_order(function_body(view, "m11_draw_dm1_center_door_buttons"), [
        "m11_dm1_nearest_blocking_center_door_depth(cells)",
        "hasDoorThing",
        "doors[doorIdx].button",
        # M11_GFX_DOOR_BUTTON_BASE was refactored into the helper
        # m11_door_button_graphic_index_for_state(state), which returns
        # M11_GFX_DOOR_BUTTON_BASE for DM1 PC34 and the CSB Amiga M634
        # base (453) otherwise. The button-graphic identity contract is
        # unchanged; the constant just lives one function down. Accept
        # either the constant name or the helper call in the button
        # draw.
        "m11_door_button_graphic_index_for_state",
    ], "Firestaff center button gating")
    require_order(function_body(view, "m11_draw_dm1_d3r_door_button"), [
        "m11_dm1_side_lane_clear_for_rel(cells, 3, 1)",
        "hasDoorThing",
        "doors[doorIdx].button",
        "m11_door_button_graphic_index_for_state",
    ], "Firestaff D3R button gating")

    require_order(main_loop, [
        "V1_TitleFrontend_GetSourceTimingEvidence",
        "V1_TitleFrontend_GetRuntimeFrameDelayMs",
        "V1_TitleFrontend_GetRuntimeFinalGuardDelayMs",
    ], "Firestaff title runtime cadence binding")
    require(title, "V1_TitleFrontend_GetSourceAnimationStepCount", "Firestaff title source schedule")

    require_order(endgame, [
        "ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT",
        "ENDGAME_COMPAT_SOURCE_EVENT_THE_END_PALETTE_FADE",
        "ENDGAME_COMPAT_SOURCE_EVENT_RESTART_DELAY",
        "ENDGAME_COMPAT_SOURCE_EVENT_RESTART_BUTTONS_RENDER",
        "ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT",
    ], "Firestaff end source-event schedule")
    for needle in [
        "M11_GameView_GetV1EndgameTheEndGraphicId",
        "M11_GameView_GetV1EndgameTheEndZone",
        "M11_GameView_GetV1EndgameRestartBox",
        "M11_GameView_GetV1EndgameQuitBox",
    ]:
        require(view, needle, "Firestaff end runtime seam")
    require_order(function_body(view, "M11_GameView_GetV1EndgameTheEndZone"), [
        "*outX = 120",
        "*outY = 95",
        "*outW = 80",
        "*outH = 14",
    ], "Firestaff THE END runtime box source coordinates")
    require(endgame, "step.x = 120u; step.y = 95u; step.width = 80u; step.height = 14u",
            "Firestaff THE END source schedule box")

    return {
        "door_layer_order_locked": True,
        "title_runtime_cadence_helpers_locked": True,
        "endgame_source_schedule_locked": True,
    }


def external_refs() -> dict[str, object]:
    if not DM1_ZIP.is_file():
        raise AssertionError(f"missing original archive {DM1_ZIP}")
    title = read_member_in_memory(DM1_ZIP, "TITLE")
    graphics = read_member_in_memory(DM1_ZIP, "DATA/GRAPHICS.DAT")
    dungeon = read_member_in_memory(DM1_ZIP, "DATA/DUNGEON.DAT")
    if len(title) != 12002 or len(graphics) != 363417 or len(dungeon) != 33357:
        raise AssertionError("original archive member size mismatch")
    members = {"TITLE": title, "DATA/GRAPHICS.DAT": graphics,
               "DATA/DUNGEON.DAT": dungeon}
    for name, data in members.items():
        if hashlib.sha256(data).hexdigest() != MEMBER_SHA256[name]:
            raise AssertionError(f"original archive member hash mismatch: {name}")
    return {
        "archive": str(DM1_ZIP),
        "read_mode": "in-memory/no-extraction",
        "dm1_title_sha256": hashlib.sha256(title).hexdigest(),
        "dm1_graphics_dat_sha256": hashlib.sha256(graphics).hexdigest(),
        "dm1_dungeon_dat_sha256": hashlib.sha256(dungeon).hexdigest(),
    }


def main() -> int:
    payload = {
        "status": STATUS,
        "redmcsb_root": str(RED),
        "dm1_original_archive": str(DM1_ZIP),
        "source_audit": source_audit(),
        "firestaff_audit": firestaff_audit(),
        "external_refs": external_refs(),
        "claims": [
            "Door-with-buttons, TITLE cadence, and end animation seams are source-locked to ReDMCSB anchors.",
            "Original PC 3.4 members are authenticated in memory from the supplied ZIP.",
            "No pixel parity, video parity, or stock runtime promotion is claimed by this pass.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass489 — DM1 V1 door/TITLE/end source-audit closure",
        "",
        f"Status: `{STATUS}`",
        "",
        f"Manifest: `{OUT_JSON.relative_to(ROOT)}`",
        "",
        "## Anchors",
    ]
    for key, value in payload["source_audit"].items():
        if key != "slice_sha256":
            lines.append(f"- `{key}`: `{value}`")
    lines += ["", "## Non-claims", "", "No pixel parity, video parity, or stock-runtime route promotion is made by this evidence gate."]
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"pass489_dm1_v1_door_title_end_source_audit=pass manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
