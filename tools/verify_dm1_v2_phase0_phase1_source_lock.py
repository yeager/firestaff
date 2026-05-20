#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "parity-evidence/verification/dm1_v2_phase0_phase1_20260520"
OUT_JSON = OUT_DIR / "manifest.json"


def redmcsb_source_root() -> Path:
    candidates = []
    if os.environ.get("FIRESTAFF_REDMCSB_SOURCE"):
        candidates.append(Path(os.environ["FIRESTAFF_REDMCSB_SOURCE"]).expanduser())
    candidates.extend([
        Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser(),
    ])
    for candidate in candidates:
        if (candidate / "DEFS.H").exists() and (candidate / "COMMAND.C").exists():
            return candidate
    raise SystemExit("error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE")


SOURCE = redmcsb_source_root()

SOURCE_ANCHORS = [
    ("DEFS.H", 238, "C001_COMMAND_TURN_LEFT"),
    ("DEFS.H", 243, "C006_COMMAND_MOVE_LEFT"),
    ("COMMAND.C", 2045, "F0380_COMMAND_ProcessQueue_CPSC"),
    ("COMMAND.C", 2096, "G0310_i_DisabledMovementTicks"),
    ("COMMAND.C", 2151, "F0365_COMMAND_ProcessTypes1To2_TurnParty"),
    ("COMMAND.C", 2155, "F0366_COMMAND_ProcessTypes3To6_MoveParty"),
    ("CLIKMENU.C", 142, "F0365_COMMAND_ProcessTypes1To2_TurnParty"),
    ("CLIKMENU.C", 172, "F0284_CHAMPION_SetPartyDirection"),
    ("CLIKMENU.C", 180, "F0366_COMMAND_ProcessTypes3To6_MoveParty"),
    ("CLIKMENU.C", 269, "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement"),
    ("CLIKMENU.C", 278, "L1117_B_MovementBlocked = C0_FALSE"),
    ("CLIKMENU.C", 283, "M036_DOOR_STATE"),
    ("CLIKMENU.C", 287, "MASK0x0004_FAKEWALL_OPEN"),
    ("CLIKMENU.C", 312, "F0175_GROUP_GetThing"),
    ("CLIKMENU.C", 318, "F0357_COMMAND_DiscardAllInput"),
    ("CLIKMENU.C", 345, "G0310_i_DisabledMovementTicks"),
    ("GAMELOOP.C", 90, "F0128_DUNGEONVIEW_Draw_CPSF"),
    ("GAMELOOP.C", 150, "G0310_i_DisabledMovementTicks"),
    ("GAMELOOP.C", 153, "G0311_i_ProjectileDisabledMovementTicks"),
    ("GAMELOOP.C", 215, "F0380_COMMAND_ProcessQueue_CPSC"),
    ("GAMELOOP.C", 219, "G0301_B_GameTimeTicking"),
    ("MOVESENS.C", 316, "F0267_MOVE_GetMoveResult_CPSCE"),
    ("LOADSAVE.C", 1520, "L1348_s_GlobalData.PartyMapX"),
    ("LOADSAVE.C", 1532, "DisabledMovementTicks"),
    ("LOADSAVE.C", 2731, "G0308_i_PartyDirection"),
    ("LOADSAVE.C", 2741, "G0310_i_DisabledMovementTicks"),
    ("COORD.C", 1721, "G2073_C224_ViewportPixelWidth = 224"),
    ("COORD.C", 1722, "G2074_C136_ViewportHeight = 136"),
    ("DUNVIEW.C", 2999, "M100_PIXEL_WIDTH(G0296_puc_Bitmap_Viewport)"),
    ("DUNVIEW.C", 3000, "M101_PIXEL_HEIGHT(G0296_puc_Bitmap_Viewport)"),
]

FIRESTAFF_NEEDLES = [
    ("include/dm1_v2_phase_gate_pc34.h", "DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS"),
    ("include/dm1_v2_phase_gate_pc34.h", "DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING"),
    ("include/dm1_v2_phase_gate_pc34.h", "DM1_V2_PHASE_DOMAIN_COLLISION_RULES"),
    ("include/dm1_v2_phase_gate_pc34.h", "DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA"),
    ("include/dm1_v2_phase_gate_pc34.h", "DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION"),
    ("include/dm1_v2_phase_gate_pc34.h", "v2PresentationEnabled"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "DEFS.H:238-243 owns command ids"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "COMMAND.C:2045-2155"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "CLIKMENU.C:142-346"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "GAMELOOP.C:150-155"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "MOVESENS.C:316-345"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "LOADSAVE.C:1520-1534"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "COORD.C:1721-1722"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "dm1_v2_phase_gate_defaults"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "return decision(1, 0"),
    ("src/dm1v2/dm1_v2_phase_gate_pc34.c", "presentationEnabled && configPersistenceEnabled"),
    ("tests/test_dm1_v2_phase_gate_pc34.c", "check_default_v1_locks"),
    ("tests/test_dm1_v2_phase_gate_pc34.c", "check_presentation_toggle_scope"),
    ("tests/test_dm1_v2_phase_gate_pc34.c", "check_existing_scaffolds_obey_gate"),
    ("tests/test_dm1_v2_phase_gate_pc34.c", "v1Route.runtimeCommand == 3"),
    ("tests/test_dm1_v2_phase_gate_pc34.c", "v2Route.runtimeCommand == 1"),
    ("src/dm1v2/dm1_v2_movement_command_adapter_pc34.c", "DM1_V2_MOVEMENT_ROUTE_V1_SOURCE"),
    ("src/dm1v2/dm1_v2_settings_pc34.c", "V1 behavior is not read from these fields"),
    ("src/engine/config_m12.c", "config->graphicsIndex = 0"),
    ("CMakeLists.txt", "NAME dm1_v2_phase_gate_pc34"),
    ("CMakeLists.txt", "NAME dm1_v2_phase0_phase1_source_lock"),
]


def line_anchor(file_name: str, line_no: int, needle: str) -> dict[str, object]:
    path = SOURCE / file_name
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    if not (1 <= line_no <= len(lines)):
        raise AssertionError(f"line out of range {file_name}:{line_no}")
    text = lines[line_no - 1].strip()
    if needle not in text:
        raise AssertionError(f"anchor mismatch {file_name}:{line_no}: expected {needle!r}, got {text!r}")
    return {"file": file_name, "line": line_no, "needle": needle, "text": text}


def require_text(rel: str, needle: str) -> None:
    path = ROOT / rel
    text = path.read_text(encoding="utf-8", errors="replace")
    if needle not in text:
        raise AssertionError(f"missing Firestaff needle {needle!r} in {rel}")


def main() -> int:
    errors: list[str] = []
    anchors: list[dict[str, object]] = []
    for file_name, line_no, needle in SOURCE_ANCHORS:
        try:
            anchors.append(line_anchor(file_name, line_no, needle))
        except Exception as exc:
            errors.append(str(exc))
    for rel, needle in FIRESTAFF_NEEDLES:
        try:
            require_text(rel, needle)
        except Exception as exc:
            errors.append(str(exc))

    manifest = {
        "status": "failed" if errors else "passed",
        "schema": "firestaff.dm1_v2_phase0_phase1_source_lock.v1",
        "redmcsbRoot": str(SOURCE),
        "scope": "DM1 V2 Phase 0 V1 gameplay lock plus Phase 1 explicit presentation scaffold",
        "phase0": {
            "commandSemantics": "locked to DEFS.H C001..C006 and COMMAND.C F0380 dispatch",
            "dungeonTiming": "locked to GAMELOOP disabled-tick decrement and CLIKMENU cooldown writes",
            "collisions": "locked to CLIKMENU wall/door/fakewall/group checks and MOVESENS F0267",
            "saveLoad": "locked to LOADSAVE global data fields",
        },
        "phase1": {
            "defaultBootRuntime": "M12 config defaults graphicsIndex to 0 (V1/original)",
            "presentationToggle": "render/input/config presentation allowed only through explicit V2 gate fields",
            "configPersistence": "V2 config persistence is deterministic and presentation-only",
        },
        "anchors": anchors,
        "firestaffNeedles": [{"path": rel, "needle": needle} for rel, needle in FIRESTAFF_NEEDLES],
        "errors": errors,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if errors:
        for error in errors:
            print("error:", error)
        return 1
    print(f"dm1_v2_phase0_phase1_source_lock: ok evidence={OUT_JSON.relative_to(ROOT)} anchors={len(anchors)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
