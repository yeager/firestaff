#!/usr/bin/env python3
"""Source-lock a narrow DM1 V1 wall/click/touch seam against ReDMCSB.

Evidence-only gate.  It ties three things together without changing renderer
behavior: Firestaff touch/click movement and viewport hit zones; ReDMCSB party
movement collision against wall/door/fakewall target squares; and ReDMCSB
DUNVIEW/DRAWVIEW viewport replay/presentation.  The point is to prevent touch
or click affordance work from accidentally treating the dungeon viewport as a
movement surface or bypassing source wall collision.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MATRIX_C = ROOT / "touch_click_zone_matrix_pc34_compat.c"

CHECKS: list[dict[str, Any]] = [
    {
        "id": "firestaff-touch-movement-and-viewport-remain-distinct",
        "path": MATRIX_C,
        "source": "touch_click_zone_matrix_pc34_compat.c:kTouchClickZones/kSecondaryMovementSourceOrder",
        "function": "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary",
        "range": None,
        "needles": [
            "{  3u,  70u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    263, 125,  27,  21, \"movement.forward\"",
            "{ 80u,   7u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,      0,  33, 224, 136, \"viewport.dungeon\"",
            "static const TouchClickSourceOrderedRoutePc34Compat kSecondaryMovementSourceOrder[] = {",
            "{ 1u, 68u }, { 3u, 70u }, { 2u, 69u },",
            "{ 80u, 7u }, { 83u, 2u }",
        ],
        "why": "The touch matrix keeps movement arrows and dungeon viewport as separate source-ordered click routes; viewport clicks are command C080, not movement.",
    },
    {
        "id": "clikmenu-move-command-computes-target-square-and-blocks-walls",
        "file": "CLIKMENU.C",
        "source": "CLIKMENU.C:180-347",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "range": "180-347",
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty(",
            "static int16_t G0465_ai_Graphic561_MovementArrowToStepForwardCount[4] = {",
            "static int16_t G0466_ai_Graphic561_MovementArrowToStepRightCount[4] = {",
            "F0362_COMMAND_HighlightBoxEnable(C070_ZONE_MOVE_FORWARD + AL1118_ui_MovementArrowIndex);",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection, G0465_ai_Graphic561_MovementArrowToStepForwardCount[AL1118_ui_MovementArrowIndex], G0466_ai_Graphic561_MovementArrowToStepRightCount[AL1118_ui_MovementArrowIndex], &L1121_i_MapX, &L1122_i_MapY);",
            "L1116_i_SquareType = M034_SQUARE_TYPE(AL1115_ui_Square = F0151_DUNGEON_GetSquare(L1121_i_MapX, L1122_i_MapY));",
            "if (L1116_i_SquareType == C00_ELEMENT_WALL) {",
            "L1117_B_MovementBlocked = C1_TRUE;",
            "if (L1116_i_SquareType == C04_ELEMENT_DOOR) {",
            "if (L1116_i_SquareType == C06_ELEMENT_FAKEWALL) {",
            "if (L1117_B_MovementBlocked) {",
            "F0357_COMMAND_DiscardAllInput();",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        ],
        "why": "A movement click/touch may only request the ReDMCSB move command; wall/door/fakewall passability is source-owned by F0366 target-square collision.",
    },
    {
        "id": "dunview-clears-click-metadata-and-replays-map-backed-viewport",
        "file": "DUNVIEW.C",
        "source": "DUNVIEW.C:8318-8610",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8318-8610",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0008_MAIN_ClearBytes(M772_CAST_PC(G2210_aai_XYZ_DungeonViewClickable), (long)sizeof(G2210_aai_XYZ_DungeonViewClickable));",
            "F0010_MAIN_WriteSpacedWords(M773_CAST_PI(G2210_aai_XYZ_DungeonViewClickable), 6, 0xFFFF, 8);",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1, &L0224_i_MapX, &L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "why": "Viewport click metadata is rebuilt during source viewport replay; Firestaff must not infer movement or walls solely from touch coordinates.",
    },
    {
        "id": "dunview-front-square-uses-square-aspect-before-near-content",
        "file": "DUNVIEW.C",
        "source": "DUNVIEW.C:8164-8311",
        "function": "F0127_DUNGEONVIEW_DrawSquareD0C",
        "range": "8164-8311",
        "needles": [
            "STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C(",
            "F0172_DUNGEON_SetSquareAspect(L0222_ai_SquareAspect, P0180_i_Direction, P0181_i_MapX, P0182_i_MapY);",
            "switch (L0222_ai_SquareAspect[C0_ELEMENT]) {",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);",
        ],
        "why": "The front visible square is rendered from square aspect state, not from a touch/click shortcut; near contents follow that source aspect path.",
    },
    {
        "id": "drawview-presents-only-after-viewport-buffer-is-drawn",
        "file": "DRAWVIEW.C",
        "source": "DRAWVIEW.C:709-900",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "range": "709-900",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT, CM1_COLOR_NO_TRANSPARENCY);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "why": "The viewport command zone is a presentation surface backed by G0296; movement collision has already been handled before any redraw/present seam.",
    },
]


def normalize(text: str) -> str:
    return " ".join(text.split())


def slice_text(path: Path, line_range: str | None) -> str:
    text = path.read_text(encoding="latin-1", errors="replace")
    if line_range is None:
        return text
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def verify(source: Path) -> dict[str, Any]:
    results: list[dict[str, Any]] = []
    ok = True
    for check in CHECKS:
        if "path" in check:
            path = Path(check["path"])
        else:
            path = source / check["file"]
        missing: list[str]
        if not path.exists():
            missing = [f"missing source file {path}"]
        else:
            haystack = normalize(slice_text(path, check["range"]))
            missing = [needle for needle in check["needles"] if normalize(needle) not in haystack]
        passed = not missing
        ok = ok and passed
        results.append({
            "id": check["id"],
            "passed": passed,
            "source": check["source"],
            "function": check["function"],
            "why": check["why"],
            "missing": missing,
        })

    mouse_c = source / "MOUSE.C"
    mouse_note = "MOUSE.C absent in this ReDMCSB source root; mandatory MOUSE audit treated as not-present."
    results.append({
        "id": "mouse-c-presence-audit",
        "passed": True,
        "source": "MOUSE.C",
        "function": "n/a",
        "why": mouse_note if not mouse_c.exists() else "MOUSE.C is present in this source root.",
        "missing": [] if not mouse_c.exists() else ["unexpected MOUSE.C present; extend this gate before using it as MOUSE audit evidence"],
    })
    if mouse_c.exists():
        ok = False

    return {
        "gate": "dm1_v1_wall_click_touch_seam_source_lock",
        "source_root": str(source),
        "passed": ok,
        "checks": results,
        "implication": (
            "Touch/click work may expose movement arrow boxes and the viewport box, "
            "but a wall collision seam remains source-owned by CLIKMENU F0366 and "
            "the viewport remains a DUNVIEW/DRAWVIEW presentation/click-metadata surface."
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    payload = verify(args.source)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for result in payload["checks"]:
            status = "PASS" if result["passed"] else "FAIL"
            print("{} {} {} {}".format(status, result["id"], result["source"], result["function"]))
            print("  {}".format(result["why"]))
            for needle in result["missing"]:
                print(f"  missing: {needle}")
        print("IMPLICATION {}".format(payload["implication"]))
    return 0 if payload["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
