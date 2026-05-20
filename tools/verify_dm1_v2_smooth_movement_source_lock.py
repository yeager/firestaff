#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_smooth_movement_source_lock.json"


def redmcsb_source_root() -> Path:
    candidates: list[Path] = []
    if os.environ.get("FIRESTAFF_REDMCSB_SOURCE"):
        candidates.append(Path(os.environ["FIRESTAFF_REDMCSB_SOURCE"]).expanduser())
    candidates.extend([
        Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser(),
    ])
    for candidate in candidates:
        if (candidate / "COMMAND.C").exists() and (candidate / "GAMELOOP.C").exists():
            return candidate
    raise SystemExit("error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE")


SOURCE = redmcsb_source_root()

SOURCE_ANCHORS = [
    {
        "file": "COMMAND.C",
        "lineRange": "2095-2100",
        "needles": [
            "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
            "G0310_i_DisabledMovementTicks",
            "G0311_i_ProjectileDisabledMovementTicks",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "claim": "movement commands are suppressed while source cooldown/interlock state is active",
    },
    {
        "file": "COMMAND.C",
        "lineRange": "2150-2156",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "claim": "accepted source turn/move commands dispatch to the source handlers only",
    },
    {
        "file": "CLIKMENU.C",
        "lineRange": "278-329",
        "needles": [
            "L1117_B_MovementBlocked = C1_TRUE;",
            "F0175_GROUP_GetThing",
            "F0357_COMMAND_DiscardAllInput();",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ],
        "claim": "collision and source move-result dispatch occur before any presentation interpolation",
    },
    {
        "file": "CLIKMENU.C",
        "lineRange": "330-346",
        "needles": [
            "AL1115_ui_Ticks = 1;",
            "F0310_CHAMPION_GetMovementTicks",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
            "G0311_i_ProjectileDisabledMovementTicks = 0;",
        ],
        "claim": "source cooldown is derived from champion movement ticks after accepted movement",
    },
    {
        "file": "MOVESENS.C",
        "lineRange": "771-819",
        "needles": [
            "F0317_CHAMPION_AddScentStrength",
            "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        ],
        "claim": "creature scent timing and sensor dispatch are source movement side effects",
    },
    {
        "file": "GAMELOOP.C",
        "lineRange": "69-155",
        "needles": [
            "F0261_TIMELINE_Process_CPSEF();",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0313_ul_GameTime++;",
            "G0310_i_DisabledMovementTicks--;",
            "G0311_i_ProjectileDisabledMovementTicks--;",
        ],
        "claim": "source timeline, redraw, game-time, and cooldown decrement cadence are fixed by the main loop",
    },
    {
        "file": "GAMELOOP.C",
        "lineRange": "215-219",
        "needles": [
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "F0363_COMMAND_HighlightBoxDisable();",
            "G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking",
        ],
        "claim": "source command processing waits for the same stop-waiting/game-time cadence",
    },
    {
        "file": "DUNVIEW.C",
        "lineRange": "8318-8615",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
        ],
        "claim": "source viewport redraw renders from source logical coordinates and presents once per source draw pass",
    },
]

FIRESTAFF_REQUIRED = [
    (ROOT / "tests/test_dm1_v2_movement_camera_pc34.c", "test_camera_ticks_are_presentation_only"),
    (ROOT / "tests/test_dm1_v2_movement_camera_pc34.c", "COMMAND.C:2095-2100"),
    (ROOT / "tests/test_dm1_v2_movement_camera_pc34.c", "CLIKMENU.C:278-329"),
    (ROOT / "tests/test_dm1_v2_movement_camera_pc34.c", "MOVESENS.C:799-819"),
    (ROOT / "tests/test_dm1_v2_movement_camera_pc34.c", "GAMELOOP.C:69,90,124,150-155,215-219"),
    (ROOT / "src/dm1v2/dm1_v2_camera_controller_pc34.c", "presentation-only"),
    (ROOT / "src/dm1v2/dm1_v2_camera_controller_pc34.c", "GAMELOOP.C:90"),
    (ROOT / "src/dm1v2/dm1_v2_movement_command_adapter_pc34.c", "dm1_v2_runtime_apply_command"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_smooth_movement_source_lock"),
    (ROOT / "CMakeLists.txt", "NAME dm1_v2_movement_camera_pc34"),
]

FORBIDDEN_CAMERA_MUTATIONS = [
    "dm1_v2_runtime_apply_command",
    "dm1_v2_runtime_tick",
    "dm1_v2_move_step",
    "dm1_v2_turn",
    "dm1_v2_vp_begin_scroll",
    "dm1_v2_vp_mark_dirty",
    "F0267_MOVE_GetMoveResult_CPSCE",
    "F0276_SENSOR_ProcessThingAdditionOrRemoval",
    "F0261_TIMELINE_Process_CPSEF",
    "G0310_i_DisabledMovementTicks =",
    "G0311_i_ProjectileDisabledMovementTicks =",
    "G0362_l_LastPartyMovementTime =",
]


def line_slice(path: Path, line_range: str) -> tuple[str, str]:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    if not (1 <= start <= end <= len(lines)):
        raise ValueError(f"line range out of bounds for {path.name}:{line_range}")
    return "\n".join(lines[start - 1:end]), lines[start - 1].strip()


def main() -> int:
    errors: list[str] = []
    anchors: list[dict[str, object]] = []

    for anchor in SOURCE_ANCHORS:
        path = SOURCE / str(anchor["file"])
        try:
            text, first_line = line_slice(path, str(anchor["lineRange"]))
        except Exception as exc:
            errors.append(str(exc))
            continue
        missing = [needle for needle in anchor["needles"] if needle not in text]
        if missing:
            errors.append(f"missing source anchor(s) in {anchor['file']}:{anchor['lineRange']}: {missing}")
        anchors.append({
            "file": anchor["file"],
            "lineRange": anchor["lineRange"],
            "claim": anchor["claim"],
            "firstLine": first_line,
            "needles": anchor["needles"],
        })

    for path, needle in FIRESTAFF_REQUIRED:
        text = path.read_text(encoding="utf-8", errors="replace")
        if needle not in text:
            errors.append(f"missing Firestaff smooth-movement source-lock anchor {needle!r} in {path.relative_to(ROOT)}")

    adapter = (ROOT / "src/dm1v2/dm1_v2_movement_command_adapter_pc34.c").read_text(encoding="utf-8", errors="replace")
    apply_idx = adapter.find("dm1_v2_runtime_apply_command")
    camera_idx = adapter.find("dm1_v2_camera_begin", apply_idx + 1)
    if apply_idx < 0 or camera_idx < 0 or camera_idx < apply_idx:
        errors.append("movement adapter must apply the runtime command before starting presentation camera interpolation")

    camera_source = (ROOT / "src/dm1v2/dm1_v2_camera_controller_pc34.c").read_text(encoding="utf-8", errors="replace")
    forbidden_present = [needle for needle in FORBIDDEN_CAMERA_MUTATIONS if needle in camera_source]
    if forbidden_present:
        errors.append(f"camera controller contains forbidden source-state mutation hooks: {forbidden_present}")

    result = {
        "schema": "firestaff.dm1_v2_smooth_movement_source_lock.v1",
        "status": "failed" if errors else "passed",
        "redmcsbSourceRoot": str(SOURCE),
        "scope": "DM1 V2 Phase 5 smooth movement presentation source-lock gate",
        "anchors": anchors,
        "firestaffContract": {
            "cameraController": "interpolates copied logical/presentation state only",
            "movementAdapter": "starts camera interpolation after accepted runtime/source-style command mutation",
            "nonClaims": [
                "no runtime polish",
                "no collision rewrite",
                "no sensor/event dispatch rewrite",
                "no creature timing rewrite",
                "no redraw cadence rewrite",
            ],
        },
        "errors": errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if errors:
        for error in errors:
            print("error:", error)
        return 1
    print(f"dm1_v2_smooth_movement_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
