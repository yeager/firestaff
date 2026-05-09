#!/usr/bin/env python3
"""Pass466 DM1 V1 initial Hall C080 source-stop capture path.

This verifier narrows the next pass449 blocker without claiming pixel parity.  It
source-locks the initial Hall candidate click chain in ReDMCSB and emits the
capture-stop contract that the terminal HUD/status crops must use before the
masked pass449 rows can be made comparable again.
"""
from __future__ import annotations

import hashlib
import json
import os
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass466_dm1_v1_initial_hall_c080_source_stop_capture_path"
STATUS = "PASS466_SOURCE_STOP_CAPTURE_PATH_LOCKED_TERMINAL_HUD_ROWS_READY_FOR_RECAPTURE"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"

EXPECTED_DATA = [
    {
        "label": "dm1_pc34_english_graphics",
        "path": CANON_DM1 / "GRAPHICS.DAT",
        "bytes": 363417,
        "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "md5": "fa6b1aa29e191418713bf2cda93d962e",
    },
    {
        "label": "dm1_pc34_english_dungeon",
        "path": CANON_DM1 / "DUNGEON.DAT",
        "bytes": 33357,
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "md5": "766450c940651fc021c92fe5d0d0b3a6",
    },
]

SOURCE_LOCKS = [
    {
        "id": "dungeon_header_initial_party_location_bits",
        "file": "DEFS.H",
        "lines": "989-1013",
        "needles": ["unsigned int16_t InitialPartyLocation;", "Bits 11-10: Direction, Bits 9-5: Y, Bits 4-0: X"],
        "claim": "DUNGEON_HEADER stores the initial party tuple bits consumed at new game start.",
    },
    {
        "id": "initial_hall_party_state",
        "file": "LOADSAVE.C",
        "lines": "1940-1944",
        "needles": ["G0298_B_NewGame", "G0306_i_PartyMapX", "G0307_i_PartyMapY", "G0308_i_PartyDirection", "G0309_i_PartyMapIndex = 0"],
        "claim": "Fresh PC34 new-game state is decoded from DUNGEON_HEADER.InitialPartyLocation into map/x/y/direction globals.",
    },
    {
        "id": "viewport_mouse_input_to_c080",
        "file": "COMMAND.C",
        "lines": "397-403,2322-2323",
        "needles": ["C007_ZONE_VIEWPORT", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "F0377_COMMAND_ProcessType80_ClickInDungeonView"],
        "claim": "A left click in the viewport dispatches C080 to F0377; panel commands cannot substitute for the candidate click.",
    },
    {
        "id": "c080_viewport_normalization_and_front_sensor",
        "file": "CLIKVIEW.C",
        "lines": "311-431",
        "needles": ["void F0377_COMMAND_ProcessType80_ClickInDungeonView", "P0752_i_X -= G2067_i_ViewportScreenX", "P0753_i_Y -= G2068_i_ViewportScreenY", "C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"],
        "claim": "C080 subtracts the PC viewport origin, classifies the front-wall portrait zone, and then invokes F0372 for the front-wall sensor.",
    },
    {
        "id": "initial_portrait_screen_point",
        "file": "DUNVIEW.C",
        "lines": "525,3912-3920",
        "needles": ["G0109_auc_Graphic558_Box_ChampionPortraitOnWall[4] = { 96, 127, 35, 63 }", "P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT", "C026_GRAPHIC_CHAMPION_PORTRAITS"],
        "claim": "The visible D1C front portrait box is viewport x=96..127 y=35..63; center is the source portrait click point.",
    },
    {
        "id": "pc34_viewport_origin",
        "file": "COORD.C",
        "lines": "1693-1698",
        "needles": ["int16_t G2067_i_ViewportScreenX = 0", "int16_t G2068_i_ViewportScreenY = 33"],
        "claim": "For PC34/I34E, viewport origin maps the portrait center to screen x=111 y=82.",
    },
    {
        "id": "c127_sensor_to_candidate_append",
        "file": "MOVESENS.C",
        "lines": "1392,1501-1502",
        "needles": ["C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)"],
        "claim": "The source Hall transition is C127 wall champion portrait sensor dispatch into F0280 candidate append.",
    },
    {
        "id": "candidate_append_hud_state",
        "file": "REVIVE.C",
        "lines": "63-67,272-294",
        "needles": ["F0280_CHAMPION_AddCandidateChampionToParty", "G0299_ui_CandidateChampionOrdinal", "G0305_ui_PartyChampionCount", "F0368_COMMAND_SetLeader", "F0386_MENUS_DrawActionIcon"],
        "claim": "F0280 marks candidate mode, appends the temporary champion, and updates the leader/action HUD state before panel choice.",
    },
    {
        "id": "candidate_panel_command_space",
        "file": "COMMAND.C",
        "lines": "228-240,1985-1991",
        "needles": ["G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel", "C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "C162_COMMAND_CLICK_IN_PANEL_CANCEL", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel"],
        "claim": "Only after C080/C127/F0280 may the decision panel dispatch C160/C161/C162 into F0282.",
    },
    {
        "id": "cancel_terminal_hud_stop",
        "file": "REVIVE.C",
        "lines": "744-783",
        "needles": ["P0598_i_Command == C162_COMMAND_CLICK_IN_PANEL_CANCEL", "G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "G0305_ui_PartyChampionCount--", "F0733_FillZoneByIndex", "F0457_START_DrawEnabledMenus_CPSF", "return"],
        "claim": "Cancel terminal HUD/status capture must stop after candidate is cleared, the status slot is blacked/cleared, and enabled menus are redrawn.",
    },
    {
        "id": "resurrect_terminal_hud_stop",
        "file": "REVIVE.C",
        "lines": "785-899",
        "needles": ["G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "P0598_i_Command == C160_COMMAND_CLICK_IN_PANEL_RESURRECT", " RESURRECTED.", "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)", "F0457_START_DrawEnabledMenus_CPSF"],
        "claim": "Resurrect terminal HUD/status capture must stop after candidate mode is cleared, resurrection message/update is emitted, inventory closes, and menus redraw.",
    },
    {
        "id": "reincarnate_terminal_hud_stop",
        "file": "REVIVE.C",
        "lines": "785-899",
        "needles": ["G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)", "P0598_i_Command == C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "F0281_CHAMPION_Rename(L0826_ps_Champion)", " REINCARNATED.", "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)", "F0457_START_DrawEnabledMenus_CPSF"],
        "claim": "Reincarnate terminal HUD/status capture must stop after rename/reincarnation branch returns, inventory closes, and menus redraw.",
    },
]

CAPTURE_PATH = [
    {"step": 1, "label": "fresh_initial_hall", "sourceStop": "first Hall frame before any movement/turn", "partyTuple": {"map": 0, "x": 1, "y": 3, "dir": "South", "championCount": 0, "candidateOrdinal": 0}},
    {"step": 2, "label": "candidate_click", "command": "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "pcScreenClick": [111, 82], "chain": ["COMMAND.C F0377", "CLIKVIEW.C F0372", "MOVESENS.C C127", "REVIVE.C F0280"]},
    {"step": 3, "label": "candidate_select", "sourceStop": "after F0280 candidate append and before any C160/C161/C162 panel command", "requiredCrops": ["fullframe", "hud_status_crop", "panel_crop"]},
    {"step": 4, "label": "cancel", "command": "C162_COMMAND_CLICK_IN_PANEL_CANCEL", "sourceStop": "after REVIVE.C F0282 cancel return, following F0457_START_DrawEnabledMenus_CPSF", "terminalAction": "cancel", "requiredCrops": ["fullframe", "hud_status_crop"]},
    {"step": 5, "label": "resurrect_confirm", "command": "C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "sourceStop": "after REVIVE.C F0282 confirm path closes inventory and redraws enabled menus", "terminalAction": "resurrect_confirm", "requiredCrops": ["fullframe", "hud_status_crop"]},
    {"step": 6, "label": "reincarnate_confirm", "command": "C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "sourceStop": "after F0281 rename branch returns, F0282 closes inventory, and enabled menus are redrawn", "terminalAction": "reincarnate_confirm", "requiredCrops": ["fullframe", "hud_status_crop"]},
]

PASS449_ROWS = [
    "cancel.hud_status_crop",
    "resurrect_confirm.hud_status_crop",
    "reincarnate_confirm.hud_status_crop",
]


def norm(text: str) -> str:
    return " ".join(text.split())


def line_block(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start_s, end_s = part.split("-", 1)
            start, end = int(start_s), int(end_s)
        else:
            start = end = int(part)
        out.append("\n".join(lines[start - 1:end]))
    return "\n".join(out)


def digest(path: Path, algorithm: str) -> str:
    h = hashlib.new(algorithm)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def le16(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


def decode_initial_state() -> dict[str, Any]:
    dungeon = CANON_DM1 / "DUNGEON.DAT"
    if not dungeon.is_file():
        return {"ok": False, "error": f"missing {dungeon}"}
    data = dungeon.read_bytes()
    initial = le16(data, 8)
    state = {"mapIndex": 0, "mapX": initial & 31, "mapY": (initial >> 5) & 31, "direction": (initial >> 10) & 3}
    expected = {"raw": "0x0861", "state": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2}}
    return {
        "ok": f"0x{initial:04X}" == expected["raw"] and state == expected["state"],
        "raw": f"0x{initial:04X}",
        "state": state,
        "directionName": {0: "North", 1: "East", 2: "South", 3: "West"}.get(state["direction"]),
        "expected": expected,
    }


def audit_data() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for item in EXPECTED_DATA:
        path = item["path"]
        row = {k: (str(v) if isinstance(v, Path) else v) for k, v in item.items()}
        if not path.is_file():
            row.update({"exists": False, "ok": False})
            errors.append(f"missing locked data {item['label']}")
        else:
            actual = {"bytes": path.stat().st_size, "sha256": digest(path, "sha256"), "md5": digest(path, "md5")}
            ok = all(actual[k] == item[k] for k in ("bytes", "sha256", "md5"))
            row.update({"exists": True, "ok": ok, "actual": actual})
            if not ok:
                errors.append(f"locked data mismatch {item['label']}")
        rows.append(row)
    return rows, errors


def audit_sources() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        text = line_block(path, lock["lines"]) if path.exists() else ""
        missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        ok = path.exists() and not missing
        row = {k: v for k, v in lock.items() if k != "needles"}
        row.update({"path": str(path), "ok": ok, "missing": missing})
        rows.append(row)
        if not ok:
            errors.append(f"source lock failed {lock['id']}")
    return rows, errors


def write_outputs(manifest: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n")
    lines = [
        "# pass466_dm1_v1_initial_hall_c080_source_stop_capture_path",
        "",
        f"- status: `{manifest['status']}`",
        f"- generatedUtc: `{manifest['timestampUtc']}`",
        f"- redmcsb: `{REDMCSB}`",
        "- parity claim: **not made**; this is a source-stop capture contract for the masked pass449 terminal HUD/status rows.",
        "",
        "## Capture path",
        "",
    ]
    for step in CAPTURE_PATH:
        bits = [f"`{step['label']}`"]
        if "command" in step:
            bits.append(f"command `{step['command']}`")
        if "pcScreenClick" in step:
            bits.append(f"PC screen click `{step['pcScreenClick'][0]},{step['pcScreenClick'][1]}`")
        if "sourceStop" in step:
            bits.append(step["sourceStop"])
        lines.append(f"- {', '.join(bits)}")
    lines += [
        "",
        "## Terminal HUD/status rows now have source stops",
        "",
    ]
    for row in PASS449_ROWS:
        lines.append(f"- `{row}`")
    lines += [
        "",
        "## ReDMCSB source locks",
        "",
    ]
    for lock in manifest["sourceLocks"]:
        lines.append(f"- `{lock['file']}:{lock['lines']}` — {lock['claim']} ok={lock['ok']}")
    lines += [
        "",
        "## Non-claims",
        "",
        "No new framebuffer parity, no original-vs-Firestaff pixel parity, and no promotion of existing pass173/pass449/pass455/pass464 frames is claimed here.",
    ]
    REPORT.write_text("\n".join(lines) + "\n")


def main() -> int:
    data_rows, data_errors = audit_data()
    initial_state = decode_initial_state()
    source_rows, source_errors = audit_sources()
    errors = data_errors + source_errors
    if not initial_state.get("ok"):
        errors.append("initial DUNGEON.DAT Hall state decode failed")
    manifest = {
        "schema": "pass466_dm1_v1_initial_hall_c080_source_stop_capture_path.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if not errors else "FAIL_PASS466_SOURCE_STOP_CAPTURE_PATH_LOCK_FAILED",
        "redmcsb": str(REDMCSB),
        "lockedData": data_rows,
        "initialDungeonState": initial_state,
        "sourceLocks": source_rows,
        "capturePath": CAPTURE_PATH,
        "pass449MaskedRowsUnblockedForRecapture": PASS449_ROWS,
        "captureContract": {
            "initialState": {"map": 0, "x": 1, "y": 3, "dir": "South"},
            "candidateClickPcScreen": [111, 82],
            "forbiddenBeforeCandidateClick": ["turn_left", "turn_right", "step_forward", "step_back", "strafe", "panel C160/C161/C162"],
            "terminalHudCrop": {"xywh": [0, 0, 320, 33], "requiresTerminalAction": ["cancel", "resurrect_confirm", "reincarnate_confirm"]},
            "mustRecord": ["sourceStop", "terminalAction", "partyTuple", "command", "fullframeSha256", "hudStatusCropSha256"],
        },
        "errors": errors,
        "nonClaims": ["pixel parity", "frame promotion", "runtime capture success"],
    }
    write_outputs(manifest)
    if errors:
        raise SystemExit("; ".join(errors))
    print(f"PASS wrote {MANIFEST}")
    print(f"PASS wrote {REPORT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
