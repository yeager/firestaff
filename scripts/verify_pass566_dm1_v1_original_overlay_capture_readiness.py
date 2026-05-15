#!/usr/bin/env python3
"""Pass566 DM1 V1 original overlay/capture readiness manifest.

This gate is intentionally evidence-only. It checks the exact N2-local original
stage, emulator helpers, route shape, and ReDMCSB source anchors needed before
running the next DOSBox original overlay diagnostic. It does not launch DOSBox
or write image artifacts.
"""
from __future__ import annotations

import hashlib
import json
import os
import re
import shutil
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
HOME = Path.home()
RED = HOME / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM_ROOT = HOME / ".openclaw/data/firestaff-original-games/DM"
CANONICAL = DM_ROOT / "_canonical/dm1"
STAGE = DM_ROOT / "_extracted/dm-pc34/DungeonMasterPC34"
OUT_DIR = ROOT / "parity-evidence/verification/pass566_dm1_v1_original_overlay_capture_readiness"
OUT_JSON = OUT_DIR / "manifest.json"

EXPECTED = {
    "canonical_GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "canonical_DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "canonical_TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    "stage_DM.EXE": "4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4",
    "stage_GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "stage_DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}

ROUTE = (
    "wait:7000 shot:title enter wait:1200 shot:pre_enter_menu "
    "click:260,50 wait:1200 shot:after_enter_click click:276,140 "
    "wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 "
    "click:246,140 wait:600 shot:left_turn_probe"
)

SOURCE_ANCHORS: list[dict[str, Any]] = [
    {
        "file": "COMMAND.C",
        "function": "G0459_as_Graphic561_PrimaryKeyboardInput_DungeonView",
        "lines": [677, 683],
        "needles": [
            "{ C001_COMMAND_TURN_LEFT,     0x004B }",
            "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
            "{ C002_COMMAND_TURN_RIGHT,    0x004D }",
            "{ C006_COMMAND_MOVE_LEFT,     0x004F }",
            "{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }",
            "{ C004_COMMAND_MOVE_RIGHT,    0x0051 }",
        ],
    },
    {
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "lines": [2120, 2156],
        "needles": [
            "G2153_i_QueuedCommandsCount--;",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "lines": [142, 174],
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty(",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
            "F0284_CHAMPION_SetPartyDirection",
        ],
    },
    {
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "lines": [180, 346],
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty(",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "if (L1116_i_SquareType == C00_ELEMENT_WALL)",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
    },
    {
        "file": "DUNGEON.C",
        "function": "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "lines": [1371, 1391],
        "needles": [
            "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(",
            "*P0256_pi_MapX +=",
            "*P0257_pi_MapY +=",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "lines": [8318, 8543],
        "ordered": True,
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0117_DUNGEONVIEW_DrawSquareD3R",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
        ],
    },
    {
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "lines": [709, 858],
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport",
            "VIDRV_09_BlitViewPort",
        ],
    },
]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def text(path: Path) -> str:
    return path.read_text(encoding="latin-1", errors="replace")


def compact(value: str) -> str:
    return " ".join(value.split())


def verify_order(slice_text: str, needles: list[str]) -> None:
    flat = compact(slice_text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"missing ordered needle {needle!r}")
        pos = hit


def source_slice(path: Path, lines: list[int]) -> str:
    body = text(path).splitlines()
    return "\n".join(body[lines[0] - 1 : lines[1]])


def audit_source() -> list[dict[str, Any]]:
    rows = []
    for anchor in SOURCE_ANCHORS:
        path = RED / anchor["file"]
        if not path.exists():
            raise AssertionError(f"missing ReDMCSB source file: {path}")
        sliced = source_slice(path, anchor["lines"])
        if anchor.get("ordered"):
            verify_order(sliced, anchor["needles"])
        else:
            flat = compact(sliced)
            for needle in anchor["needles"]:
                if compact(needle) not in flat:
                    raise AssertionError(f"{anchor['file']}:{anchor['lines']} missing {needle!r}")
        rows.append(
            {
                "file": str(path),
                "function": anchor["function"],
                "lines": anchor["lines"],
                "sliceSha256": hashlib.sha256(sliced.encode("utf-8", "replace")).hexdigest(),
            }
        )
    return rows


def audit_files() -> list[dict[str, Any]]:
    files = [
        ("canonical_GRAPHICS.DAT", "DM1 PC 3.4 canonical", CANONICAL / "GRAPHICS.DAT"),
        ("canonical_DUNGEON.DAT", "DM1 PC 3.4 canonical", CANONICAL / "DUNGEON.DAT"),
        ("canonical_TITLE", "DM1 PC 3.4 canonical", CANONICAL / "TITLE"),
        ("stage_DM.EXE", "DM1 PC 3.4 DOS original stage", STAGE / "DM.EXE"),
        ("stage_GRAPHICS.DAT", "DM1 PC 3.4 DOS original stage", STAGE / "DATA/GRAPHICS.DAT"),
        ("stage_DUNGEON.DAT", "DM1 PC 3.4 DOS original stage", STAGE / "DATA/DUNGEON.DAT"),
    ]
    rows = []
    for key, variant, path in files:
        if not path.exists():
            raise AssertionError(f"missing original file: {path}")
        actual = sha256(path)
        if actual != EXPECTED[key]:
            raise AssertionError(f"{key} SHA256 mismatch: expected {EXPECTED[key]}, got {actual}")
        rows.append(
            {
                "key": key,
                "variant": variant,
                "path": str(path),
                "resolvedPath": str(path.resolve()),
                "bytes": path.stat().st_size,
                "sha256": actual,
            }
        )
    return rows


def audit_tools() -> dict[str, str | None]:
    tools = {name: shutil.which(name) for name in ["dosbox", "xvfb-run", "xdotool", "python3"]}
    missing = [name for name, value in tools.items() if not value]
    if missing:
        raise AssertionError("missing capture tool(s): " + ", ".join(missing))
    return tools


def audit_route() -> dict[str, Any]:
    labels = []
    for token in ROUTE.split():
        low = token.lower()
        if low.startswith("shot:"):
            label = low.split(":", 1)[1]
            if not re.fullmatch(r"[a-z0-9][a-z0-9_-]*", label):
                raise AssertionError(f"invalid shot label: {token}")
            labels.append(label)
        elif low.startswith("wait:"):
            if not re.fullmatch(r"wait:[0-9]+", low):
                raise AssertionError(f"invalid wait token: {token}")
        elif low.startswith("click:"):
            m = re.fullmatch(r"click:([0-9]{1,3}),([0-9]{1,3})", low)
            if not m:
                raise AssertionError(f"invalid click token: {token}")
            x, y = map(int, m.groups())
            if not (0 <= x < 320 and 0 <= y < 200):
                raise AssertionError(f"click outside 320x200 frame: {token}")
        elif low not in {"enter"}:
            raise AssertionError(f"unexpected route token: {token}")
    if len(labels) != 6:
        raise AssertionError(f"expected exactly six capture labels, got {len(labels)}")
    return {
        "routeEvents": ROUTE,
        "shotLabels": labels,
        "expectedDiagnosticClasses": [
            "title_or_menu",
            "entrance_menu",
            "dungeon_gameplay",
            "dungeon_gameplay",
            "dungeon_gameplay",
            "dungeon_gameplay",
        ],
    }


def main() -> int:
    source = audit_source()
    original_files = audit_files()
    tools = audit_tools()
    route = audit_route()
    command = (
        "OUT_DIR=$PWD/verification-screens/pass566-original-overlay-diagnostic "
        "DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' "
        "DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=5000 "
        "NEW_FILE_TIMEOUT_MS=6000 "
        f"DM1_ORIGINAL_ROUTE_EVENTS='{ROUTE}' "
        "DOSBOX=/usr/bin/dosbox xvfb-run -a "
        "scripts/dosbox_dm1_original_viewport_reference_capture.sh --run"
    )
    payload = {
        "status": "PASS566_DM1_V1_ORIGINAL_OVERLAY_CAPTURE_READINESS",
        "redmcsbRoot": str(RED),
        "sourceAnchors": source,
        "originalFiles": original_files,
        "tools": tools,
        "route": route,
        "nextCommand": command,
        "claims": [
            "N2-local original DM1 PC 3.4 stage is present and hash-locked.",
            "DOSBox/Xvfb/xdotool route prerequisites are available.",
            "The diagnostic route has exactly six labeled shots and bounded 320x200 clicks.",
            "No screenshot, overlay, original-runtime state delta, or pixel parity claim is made by this verifier.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PASS pass566_dm1_v1_original_overlay_capture_readiness manifest={OUT_JSON.relative_to(ROOT)}")
    print(command)
    return 0


if __name__ == "__main__":
    os.chdir(ROOT)
    raise SystemExit(main())
