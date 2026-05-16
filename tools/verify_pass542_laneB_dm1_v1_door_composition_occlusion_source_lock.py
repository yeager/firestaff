#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass542_laneB_dm1_v1_door_composition_occlusion_source_lock"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / "pass542_laneB_dm1_v1_door_composition_occlusion_source_lock.md"
STATUS = "PASS542_LANEB_DM1_V1_DOOR_COMPOSITION_OCCLUSION_SOURCE_LOCKED"


def read(path: Path, enc: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=enc, errors="replace")


def compact(s: str) -> str:
    return " ".join(s.split())


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


def function_body(text: str, name: str) -> str:
    pat = re.compile(r"\b(?:STATICFUNCTION\s+)?(?:static\s+)?(?:void|int|size_t|const char\s+\*)\s*" + re.escape(name) + r"\s*\(")
    match = pat.search(text)
    if not match:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", match.end())
    if brace < 0:
        raise AssertionError(f"missing function body {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():i + 1]
    next_func = text.find("\nSTATICFUNCTION ", brace + 1)
    if next_func > 0:
        return text[match.start():next_func]
    raise AssertionError(f"unterminated function {name}")


def sha256_slice(path: Path, start: int, end: int, enc: str = "latin-1") -> str:
    body = "\n".join(read(path, enc).splitlines()[start - 1:end])
    return hashlib.sha256(body.encode("utf-8", "replace")).hexdigest()


def redmcsb_audit() -> dict[str, object]:
    dunview = read(RED / "DUNVIEW.C", "latin-1")
    f0111 = function_body(dunview, "F0111_DUNGEONVIEW_DrawDoor")
    f0109 = function_body(dunview, "F0109_DUNGEONVIEW_DrawDoorOrnament")
    require_order(f0111, [
        "F0616_CopyBitmap(F0489_MEMORY_GetNativeBitmapOrGraphic(P0126_pi_DoorNativeBitmapIndices",
        "G0074_puc_Bitmap_Temporary",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(L0116_ps_Door->OrnamentOrdinal, P0128_i_ViewDoorOrnamentIndex)",
        "if ((P2084_i_ZoneIndex == M631_ZONE_DOOR_D1C) && G0407_s_Party.Event73Count_ThievesEye)",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK)",
        "F0791_DUNGEONVIEW_DrawBitmapXX(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport, P2084_i_ZoneIndex",
    ], "F0111 PC34 door temp composition before viewport blit")
    require_order(f0111, [
        "if (P0125_ui_DoorState == C5_DOOR_STATE_DESTROYED)",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C15_DOOR_ORNAMENT_DESTROYED_MASK), P0128_i_ViewDoorOrnamentIndex)",
        "F0791_DUNGEONVIEW_DrawBitmapXX(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport, P2084_i_ZoneIndex",
    ], "F0111 destroyed-door mask before viewport blit")
    require_order(f0109, [
        "STATICFUNCTION void F0109_DUNGEONVIEW_DrawDoorOrnament(",
        "AL0107_puc_Bitmap = F0675_DUNGEONVIEW_GetScaledBitmap",
        "F0791_DUNGEONVIEW_DrawBitmapXX(AL0107_puc_Bitmap, G0074_puc_Bitmap_Temporary",
        "C2000_ZONE_DOOR_ORNAMENT",
        "C09_COLOR_GOLD",
    ], "F0109 draws door ornament into temp door bitmap")
    line_f0109 = line_of(dunview, "STATICFUNCTION void F0109_DUNGEONVIEW_DrawDoorOrnament")
    line_f0111 = line_of(dunview, "STATICFUNCTION void F0111_DUNGEONVIEW_DrawDoor")
    return {
        "f0109_lines": [line_f0109, line_f0111 - 1],
        "f0111_lines": [line_f0111, line_of(dunview, "STATICFUNCTION void F0112_DUNGEONVIEW_DrawCeilingPit") - 1],
        "f0111_pc34_copy_line": line_of(dunview, "F0616_CopyBitmap(F0489_MEMORY_GetNativeBitmapOrGraphic(P0126_pi_DoorNativeBitmapIndices"),
        "f0111_base_ornament_line": line_of(dunview, "F0109_DUNGEONVIEW_DrawDoorOrnament(L0116_ps_Door->OrnamentOrdinal, P0128_i_ViewDoorOrnamentIndex)"),
        "f0111_thieves_eye_line": line_of(dunview, "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK)"),
        "f0111_viewport_blit_line": line_of(dunview, "F0791_DUNGEONVIEW_DrawBitmapXX(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport, P2084_i_ZoneIndex"),
        "slice_sha256": {
            "DUNVIEW.C:4013-4117": sha256_slice(RED / "DUNVIEW.C", 4013, 4117),
            "DUNVIEW.C:4218-4339": sha256_slice(RED / "DUNVIEW.C", 4218, 4339),
        },
    }


def firestaff_audit() -> dict[str, object]:
    source = read(ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c")
    evidence = function_body(source, "dm1_viewport_3d_source_evidence")
    require_order(evidence, [
        "DUNVIEW.C:4013-4117 F0109 composes door ornaments into G0074 temporary door bitmap",
        "DUNVIEW.C:4218-4335 F0111 copies door panel to G0074, applies ornaments/masks, then blits G0074 into viewport",
    ], "Firestaff source evidence carries F0109/F0111 composition lock")
    for needle in [
        "DUNVIEW.C:6579-6601 D3R mirrored door-front occlusion",
        "DUNVIEW.C:6722-6746 D3C door-front occlusion",
        "DUNVIEW.C:7874-7937 D1C door-front occlusion",
    ]:
        require(evidence, needle, "Firestaff door-front occlusion evidence")
    return {"file": "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "door_front_occlusion_anchors_checked": 3}


def main() -> int:
    red = redmcsb_audit()
    local = firestaff_audit()
    payload = {
        "schema": "firestaff.parity.pass542_laneB_dm1_v1_door_composition_occlusion_source_lock.v1",
        "status": STATUS,
        "redmcsbSourceRoot": str(RED),
        "redmcsb": red,
        "firestaff": local,
        "claims": [
            "F0109 draws door ornaments into G0074 temporary door bitmap, not straight into the viewport.",
            "F0111 copies the native door panel into G0074, applies base/destroyed/Thieves Eye ornaments or masks, then blits G0074 to G0296 viewport.",
            "Firestaff viewport metadata records this door-surface composition seam alongside door-front occlusion ranges.",
        ],
        "nonClaims": ["No movement core change.", "No capture pipeline change.", "No original-runtime or pixel-parity promotion."],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    md = [
        "# Pass542 Lane B - DM1 V1 door composition occlusion source lock",
        "",
        f"Status: {STATUS}",
        "",
        "## ReDMCSB Anchors",
        f"- DUNVIEW.C:{red['f0109_lines'][0]}-{red['f0109_lines'][1]}: F0109 draws door ornaments into G0074_puc_Bitmap_Temporary.",
        f"- DUNVIEW.C:{red['f0111_lines'][0]}-{red['f0111_lines'][1]}: F0111 copies the door panel to temp, applies ornaments/masks, then blits temp to viewport.",
        f"- Copy/base ornament/Thieves Eye/blit lines: {red['f0111_pc34_copy_line']}, {red['f0111_base_ornament_line']}, {red['f0111_thieves_eye_line']}, {red['f0111_viewport_blit_line']}.",
        "",
        "## Firestaff Check",
        f"- {local['file']} source evidence carries the F0109/F0111 temporary-door composition lock.",
        "",
        "No movement, capture, original-runtime, or pixel parity claim is made by this gate.",
    ]
    OUT_MD.write_text("\n".join(md) + "\n", encoding="utf-8")
    print(f"pass542_laneB_dm1_v1_door_composition_occlusion_source_lock=pass manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
