#!/usr/bin/env python3
"""Pass504: N2 DM1 V1 original capture route preflight.

This is a no-runtime verifier. It checks N2 prerequisites for the next
promotable original movement/viewport/wall capture attempt.
"""
from __future__ import annotations

import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass504_dm1_v1_original_capture_route_preflight"
STATUS = "PASS504_ORIGINAL_CAPTURE_ROUTE_PREFLIGHT_READY"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
GREATSTONE = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"
CAPTURE_SCRIPT = ROOT / "scripts/dosbox_dm1_original_viewport_reference_capture.sh"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "pc34_mouse_movement_boxes",
        "file": "COMMAND.C",
        "lines": "106-114",
        "function": "G0448_as_Graphic561_SecondaryMouseInput_Movement",
        "needles": [
            "C001_COMMAND_TURN_LEFT,             234, 261, 125, 145",
            "C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145",
            "C002_COMMAND_TURN_RIGHT,            291, 318, 125, 145",
            "C006_COMMAND_MOVE_LEFT,             234, 261, 147, 167",
            "C005_COMMAND_MOVE_BACKWARD,         263, 289, 147, 167",
            "C004_COMMAND_MOVE_RIGHT,            291, 318, 147, 167",
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168",
        ],
        "claim": "N2 route clicks must use source-owned PC34 movement/viewport boxes.",
    },
    {
        "id": "pc34_keyboard_movement_tokens",
        "file": "COMMAND.C",
        "lines": "252-260,272-280",
        "function": "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
        "needles": [
            "C001_COMMAND_TURN_LEFT",
            "C003_COMMAND_MOVE_FORWARD",
            "C002_COMMAND_TURN_RIGHT",
            "C006_COMMAND_MOVE_LEFT",
            "C005_COMMAND_MOVE_BACKWARD",
            "C004_COMMAND_MOVE_RIGHT",
            "0xAB35",
            "0x9B41",
        ],
        "claim": "keypad/arrow route tokens are readiness inputs; promotion still needs handler/state proof.",
    },
    {
        "id": "f0365_turn_state_delta",
        "file": "CLIKMENU.C",
        "lines": "142-174",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "needles": [
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
            "F0284_CHAMPION_SetPartyDirection",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        ],
        "claim": "turn labels are promotable only after F0365 mutates party direction.",
    },
    {
        "id": "f0366_move_legality_and_cooldown",
        "file": "CLIKMENU.C",
        "lines": "224-347",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "G0465_ai_Graphic561_MovementArrowToStepForwardCount",
            "G0466_ai_Graphic561_MovementArrowToStepRightCount",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "F0357_COMMAND_DiscardAllInput",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks",
        ],
        "claim": "step labels require the source legality path, not a screenshot taken immediately after input.",
    },
    {
        "id": "move_result_party_state",
        "file": "MOVESENS.C",
        "lines": "738-818",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "needles": [
            "G0397_i_MoveResultMapX",
            "G0398_i_MoveResultMapY",
            "G0399_ui_MoveResultMapIndex",
            "G0362_l_LastPartyMovementTime",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        ],
        "claim": "accepted movement must update move-result/state before viewport evidence is labeled.",
    },
    {
        "id": "viewport_wall_bitmap_frames",
        "file": "DUNVIEW.C",
        "lines": "577-593,8318-8618",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": [
            "G0163_aauc_Graphic558_Frame_Walls",
            "F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "G0296_puc_Bitmap_Viewport",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "claim": "wall/viewport captures must bind to the F0128 tuple and G0296 composition.",
    },
    {
        "id": "pc34_viewport_present",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": [
            "F0097_DUNGEONVIEW_DrawViewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "G0296_puc_Bitmap_Viewport",
            "VIDRV_09_BlitViewPort",
        ],
        "claim": "the capture seam is after the PC34 viewport-present blit, not setup/menu echo.",
    },
]

REQUIRED_SCRIPT_TOKENS = [
    "DM1_ORIGINAL_ROUTE_EVENTS",
    "DM1_ORIGINAL_PROGRAM",
    "DM1_ROUTE_SKIP_STARTUP_SELECTOR",
    "click:<x>,<y>",
    "rclick:<x>,<y>",
    "shot:<label>",
    "xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
    "exactly 6 shot or shot:<label> tokens",
    "NEW_FILE_TIMEOUT_MS",
]
REQUIRED_COMMANDS = ["dosbox", "xvfb-run", "xdotool", "python3"]
CANONICAL_DM1_VARIANT = "DM1 PC 3.4 I34E original runtime inputs"
EXPECTED_CANONICAL_DM1_SHA256 = {
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    "DungeonMasterPC34/DM.EXE": "4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4",
}


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        start_s, end_s = part.split("-", 1)
        start, end = int(start_s), int(end_s)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def sha256(path: Path) -> str | None:
    if not path.exists() or not path.is_file():
        return None
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def audit_source() -> list[dict[str, Any]]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        rows.append({**lock, "path": str(path), "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def audit_commands() -> list[dict[str, Any]]:
    rows = []
    for name in REQUIRED_COMMANDS:
        resolved = shutil.which(name)
        rows.append({"name": name, "path": resolved, "ok": resolved is not None})
    resolved_debug = shutil.which("dosbox-debug")
    rows.append({"name": "dosbox-debug", "path": resolved_debug, "ok": resolved_debug is not None, "optionalButUseful": True})
    try:
        subprocess.run(["python3", "-c", "from PIL import Image"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        pillow_ok = True
        pillow_error = ""
    except Exception as exc:
        pillow_ok = False
        pillow_error = str(exc)
    rows.append({"name": "python3-pillow", "path": "python3 import PIL.Image", "ok": pillow_ok, "error": pillow_error})
    return rows


def audit_canonical() -> list[dict[str, Any]]:
    rows = []
    for rel, expected_sha in EXPECTED_CANONICAL_DM1_SHA256.items():
        path = CANON_DM1 / rel
        actual_sha = sha256(path)
        rows.append({
            "relative": rel,
            "path": str(path),
            "exists": path.exists(),
            "isSymlink": path.is_symlink(),
            "realpath": str(path.resolve()) if path.exists() else None,
            "sha256": actual_sha,
            "expectedSha256": expected_sha,
            "variant": CANONICAL_DM1_VARIANT,
            "ok": path.exists() and (path.is_file() or path.is_symlink()) and actual_sha == expected_sha,
        })
    return rows


def audit_script() -> dict[str, Any]:
    text = CAPTURE_SCRIPT.read_text(encoding="utf-8") if CAPTURE_SCRIPT.exists() else ""
    missing = [token for token in REQUIRED_SCRIPT_TOKENS if token not in text]
    return {
        "path": str(CAPTURE_SCRIPT.relative_to(ROOT)),
        "exists": CAPTURE_SCRIPT.exists(),
        "executable": os.access(CAPTURE_SCRIPT, os.X_OK),
        "requiredTokens": REQUIRED_SCRIPT_TOKENS,
        "missingTokens": missing,
        "ok": CAPTURE_SCRIPT.exists() and os.access(CAPTURE_SCRIPT, os.X_OK) and not missing,
    }


def audit_secondary_refs() -> dict[str, Any]:
    return {
        "greatstoneAtlas": {"path": str(GREATSTONE), "exists": GREATSTONE.exists(), "role": "secondary DM1 atlas/hash context only"},
        "canonicalOriginalDm": {"path": str(CANON_DM1), "exists": CANON_DM1.exists(), "role": "hash-locked original runtime data"},
        "csbwin": {"path": str(CSBWIN), "exists": CSBWIN.exists(), "role": "not used for this DM1 PC34 route gate"},
    }


def main() -> int:
    source = audit_source()
    commands = audit_commands()
    canon = audit_canonical()
    script = audit_script()
    secondary = audit_secondary_refs()
    required_ok = {
        "redmcsb_source_locks": all(row["ok"] for row in source),
        "capture_script_contract": script["ok"],
        "n2_runtime_commands": all(row["ok"] for row in commands if not row.get("optionalButUseful")),
        "canonical_dm1_data": all(row["ok"] for row in canon),
        "greatstone_secondary_available": secondary["greatstoneAtlas"]["exists"],
    }
    problems = [key for key, ok in required_ok.items() if not ok]
    route_contract = {
        "recommendedProgram": "DM -vv -sn -pk",
        "recommendedWrapper": "DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
        "shotCount": 6,
        "clickCenters": {
            "turn_left": [247, 135],
            "move_forward": [276, 135],
            "turn_right": [304, 135],
            "move_left": [247, 157],
            "move_backward": [276, 157],
            "move_right": [304, 157],
        },
        "promotionRequires": [
            "F0380 dequeues the matching command after the route input",
            "F0365 or F0366 mutates source-visible direction/position/move-result state",
            "F0128 consumes the resulting direction/map-X/map-Y tuple",
            "F0097/VIDRV presents the same viewport before the screenshot/crop is promoted",
            "raw frame or 224x136 crop differs from the known repeated static/no-state-delta gameplay family unless the command is proven blocked/no-op by source state",
        ],
    }
    manifest = {
        "schema": "firestaff.parity.pass504_dm1_v1_original_capture_route_preflight.v1",
        "status": STATUS if not problems else "FAIL_PASS504_ORIGINAL_CAPTURE_ROUTE_PREFLIGHT",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "n2CommandAudit": commands,
        "canonicalDm1Audit": canon,
        "canonicalDm1Variant": CANONICAL_DM1_VARIANT,
        "captureScriptAudit": script,
        "secondaryReferenceAudit": secondary,
        "requiredOk": required_ok,
        "routeContract": route_contract,
        "blocker": None if not problems else "N2 original capture route prerequisites are incomplete; fix requiredOk failures before another runtime capture attempt.",
        "nonClaims": [
            "no DOSBox runtime was launched",
            "no original-vs-Firestaff pixel parity is promoted",
            "no game logic behavior is changed or asserted beyond ReDMCSB source locks",
            "CSBWin is not used as evidence for this DM1 PC34 gate",
        ],
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass504 - DM1 V1 original capture route preflight",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This gate checks the exact N2 prerequisites for the next original DM1 V1 movement/viewport/wall capture attempt. It does not run DOSBox and does not promote stale captures.",
        "",
        "## ReDMCSB Source Locks",
    ]
    for row in source:
        lines.append(f"- `{row['file']}:{row['lines']}` / `{row['function']}` - {row['claim']} ok=`{row['ok']}`")
    lines += ["", "## N2 Preconditions"]
    lines.extend(f"- `{row['name']}`: `{row.get('path')}` ok=`{row['ok']}`" for row in commands)
    lines += ["", "## Canonical DM1 Inputs"]
    lines.extend(f"- `{row['relative']}` sha256=`{row['sha256']}` expected=`{row['expectedSha256']}` ok=`{row['ok']}`" for row in canon)
    lines += [
        "",
        "## Capture Contract",
        f"- Program: `{route_contract['recommendedProgram']}`",
        f"- Wrapper: `{route_contract['recommendedWrapper']}`",
        "- Six `shot`/`shot:<label>` tokens are required by the capture script before normalization.",
        "- Click centers are source-locked from `COMMAND.C`, but labels become promotable only after F0380 -> F0365/F0366 -> F0128 -> F0097/VIDRV proof.",
        "",
        "## Secondary References",
        f"- Greatstone atlas: `{secondary['greatstoneAtlas']['path']}` exists=`{secondary['greatstoneAtlas']['exists']}`",
        f"- Original DM canonical data: `{secondary['canonicalOriginalDm']['path']}` exists=`{secondary['canonicalOriginalDm']['exists']}`",
        f"- CSBWin: `{secondary['csbwin']['path']}` exists=`{secondary['csbwin']['exists']}`; not used for this DM1 PC34 gate.",
        "",
        "## Gate",
        "",
        "- `python3 tools/verify_pass504_dm1_v1_original_capture_route_preflight.py`",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]
    if problems:
        lines += ["", "## Problems", *[f"- {problem}" for problem in problems]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(manifest["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    if problems:
        for problem in problems:
            print(f"problem={problem}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
