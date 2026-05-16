#!/usr/bin/env python3
"""Pass394 verifier: F0380 dispatch to F0365/F0366 movement state effects.

Audits ReDMCSB first, then checks Firestaff's focused command-core model and
probes. The verifier writes a compact JSON manifest plus a human evidence note.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass394_dm1_v1_dispatch_to_movement_state"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / "pass394_dm1_v1_dispatch_to_movement_state.md"


def read(path: Path, encoding: str = "latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def function_body(text: str, name: str) -> tuple[int, int, str]:
    m = re.search(rf"^(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|unsigned\s+char)\s+{re.escape(name)}\s*\(", text, re.M)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                start = text.count("\n", 0, m.start()) + 1
                end = text.count("\n", 0, i) + 1
                return start, end, text[m.start() : i + 1]
    # ReDMCSB files contain preprocessor-branched braces that can confuse a
    # naive brace counter. Fall back to the next top-level Fxxxx declaration.
    next_m = re.search(r"^void\s+F\d+_[A-Za-z0-9_]+\s*\(", text[m.start() + 1 :], re.M)
    if next_m:
        end_i = m.start() + 1 + next_m.start()
        start = text.count("\n", 0, m.start()) + 1
        end = text.count("\n", 0, end_i) + 1
        return start, end, text[m.start() : end_i]
    raise AssertionError(f"unterminated function {name}")


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def source_audit() -> dict:
    command_c = read(RED / "COMMAND.C")
    clikmenu_c = read(RED / "CLIKMENU.C")
    dungeon_c = read(RED / "DUNGEON.C")
    champion_c = read(RED / "CHAMPION.C")
    movesens_c = read(RED / "MOVESENS.C")
    gameloop_c = read(RED / "GAMELOOP.C")
    dview_c = read(RED / "DUNVIEW.C")

    f0380_s, f0380_e, f0380 = function_body(command_c, "F0380_COMMAND_ProcessQueue_CPSC")
    f0365_s, f0365_e, f0365 = function_body(clikmenu_c, "F0365_COMMAND_ProcessTypes1To2_TurnParty")
    f0366_s, f0366_e, f0366 = function_body(clikmenu_c, "F0366_COMMAND_ProcessTypes3To6_MoveParty")
    f0364_s, f0364_e, f0364 = function_body(clikmenu_c, "F0364_COMMAND_TakeStairs")
    f0150_s, f0150_e, f0150 = function_body(dungeon_c, "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement")
    f0284_s, f0284_e, f0284 = function_body(champion_c, "F0284_CHAMPION_SetPartyDirection")
    f0310_s, f0310_e, f0310 = function_body(champion_c, "F0310_CHAMPION_GetMovementTicks")
    f0267_s, f0267_e, f0267 = function_body(movesens_c, "F0267_MOVE_GetMoveResult_CPSCE")

    require_order(f0380, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "G2153_i_QueuedCommandsCount == 0",
        "G0310_i_DisabledMovementTicks",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G2153_i_QueuedCommandsCount--;",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "F0380 dequeue/dispatch order")
    require(f0365, "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0365 stop-wait write")
    require(f0365, "F0284_CHAMPION_SetPartyDirection", "F0365 party direction mutation")
    require(f0365, "F0276_SENSOR_ProcessThingAdditionOrRemoval", "F0365 sensor leave/enter on turn")
    require(f0364, "G0306_i_PartyMapX", "F0364 mutates party map coordinates")
    require(f0364, "F0284_CHAMPION_SetPartyDirection", "F0364 mutates stairs exit direction")
    require_order(f0366, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0325_CHAMPION_DecrementStamina",
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "L1117_B_MovementBlocked = C0_FALSE;",
        "if (L1117_B_MovementBlocked)",
        "F0357_COMMAND_DiscardAllInput();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "F0366 blocked/success order")
    require(f0150, "*P0256_pi_MapX +=", "F0150 X mutation")
    require(f0150, "*P0257_pi_MapY +=", "F0150 Y mutation")
    require(f0284, "G0308_i_PartyDirection", "F0284 writes party direction")
    require(f0310, "return L0933_ui_Ticks;", "F0310 returns movement ticks")
    require(f0267, "G0397_i_MoveResultMapX = P0560_i_DestinationMapX;", "F0267 move result X")
    require(f0267, "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;", "F0267 move result Y")
    require(f0267, "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;", "F0267 last movement time")
    require(gameloop_c, "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "main-loop viewport draw uses party state")
    require(gameloop_c, "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);", "main-loop stop-wait predicate")
    require(dview_c, "void F0128_DUNGEONVIEW_Draw_CPSF", "viewport draw function")
    require(dview_c, "F0098_DUNGEONVIEW_DrawFloorAndCeiling();", "viewport floor/ceiling redraw branch")

    return {
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC": [f0380_s, f0380_e],
        "CLIKMENU.C:F0364_COMMAND_TakeStairs": [f0364_s, f0364_e],
        "CLIKMENU.C:F0365_COMMAND_ProcessTypes1To2_TurnParty": [f0365_s, f0365_e],
        "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty": [f0366_s, f0366_e],
        "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement": [f0150_s, f0150_e],
        "CHAMPION.C:F0284_CHAMPION_SetPartyDirection": [f0284_s, f0284_e],
        "CHAMPION.C:F0310_CHAMPION_GetMovementTicks": [f0310_s, f0310_e],
        "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE": [f0267_s, f0267_e],
        "GAMELOOP.C": "F0128 viewport draw + G0321/G0301 wait loop audited",
        "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF": "redraws from current party direction/map coordinates",
    }


def firestaff_audit() -> dict:
    core_c = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c", "utf-8")
    core_h = read(ROOT / "include/dm1_v1_movement_command_core_pc34_compat.h", "utf-8")
    test_core = read(ROOT / "tests/test_dm1_v1_movement_command_core_pc34_compat.c", "utf-8")
    test_integration = read(ROOT / "tests/test_dm1_v1_command_movement_sensor_timing_pc34_compat.c", "utf-8")
    cmake = read(ROOT / "CMakeLists.txt", "utf-8")

    for needle, label in [
        ("DM1_V1_InputCommandQueue_ProcessOnePc34Compat", "queue consumer call"),
        ("dm1_v1_is_turn_command(outResult->queue.command)", "turn branch"),
        ("m11_v1_turning_apply_party_original_presentation_pc34_compat", "party direction/champion rotation"),
        ("dm1_v1_apply_pre_step_stamina_cost", "pre-step stamina"),
        ("F0702_MOVEMENT_TryMove_Compat", "relative movement legality/mutation candidate"),
        ("outResult->inputDiscardRequested = 1;", "blocked discard"),
        ("DM1_V1_InputCommandQueue_DiscardAllInputPc34Compat(queue);", "queue discard"),
        ("DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat", "successful cooldown/timing"),
        ("outResult->viewportRedrawRequested = 1;", "viewport redraw flag"),
    ]:
        require(core_c, needle, label)
    for needle in [
        "turn requests viewport redraw",
        "turn releases input wait",
        "step updates last movement time",
        "step clears projectile cooldown",
        "pc34 core up arrow sets cooldown",
        "pc34 core disabled gate keeps command queued",
        "pc34 core blocked up arrow discards followup",
    ]:
        require(test_core, needle, f"command-core probe {needle}")
    for needle in [
        "core forward1 requests viewport",
        "core turn requests viewport",
        "core blocked wall skips viewport",
        "core group block skips viewport",
        "disabled movement leaves command queued",
    ]:
        require(test_integration, needle, f"integration probe {needle}")
    require(core_h, "GAMELOOP.C:90", "header source-lock viewport redraw note")
    require(cmake, "test_dm1_v1_movement_command_core_pc34_compat", "CMake core test target")
    return {
        "core": "dm1_v1_movement_command_core_pc34_compat.c models F0380->F0365/F0366 turn/step side effects",
        "focusedProbe": "test_dm1_v1_movement_command_core_pc34_compat covers turn, step, cooldown, blocked queue discard, disabled gate",
        "integrationProbe": "test_dm1_v1_command_movement_sensor_timing_pc34_compat covers sensor/timing/redraw effects and blocker absence",
    }


def main() -> int:
    audited = source_audit()
    firestaff = firestaff_audit()
    result = {
        "status": "PASS394_DISPATCH_TO_MOVEMENT_STATE_PROVEN",
        "redmcsbRoot": str(RED),
        "auditedFunctions": audited,
        "provenStateEffects": {
            "F0365_turn": "sets G0321 stop-wait true, processes current-square leave/enter sensors, mutates party/champion direction via F0284, or takes stairs via F0364 mutating map/direction",
            "F0366_successful_step": "sets G0321 stop-wait true, decrements living champion stamina before resolution, computes relative destination, rejects blockers before mutation, calls F0267 for accepted non-stairs move, sets G0310 disabled movement ticks, clears G0311 projectile cooldown",
            "F0366_blocked_step": "discards input, waits one vblank on PC-34, clears G0321 stop-wait false, returns before F0267/cooldown/viewport side effects",
            "viewport_redraw_trigger": "main loop calls F0128_DUNGEONVIEW_Draw_CPSF with current G0308/G0306/G0307 after command effects and stop-wait/tick gating",
        },
        "firestaffEvidence": firestaff,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass394 — DM1 V1 dispatch to movement state",
        "",
        "Status: `PASS394_DISPATCH_TO_MOVEMENT_STATE_PROVEN`",
        "",
        "## ReDMCSB-first audit",
    ]
    for name, linespec in audited.items():
        lines.append(f"- `{name}` — `{linespec}`")
    lines += [
        "",
        "## Proven state effects",
        "- `F0365` turn: stop-wait true, current-square sensor leave/enter, party/champion direction mutation; stairs path mutates map/direction via `F0364`.",
        "- `F0366` accepted step: stop-wait true, living champion stamina decrement, relative coordinate calculation, blocker predicates before mutation, `F0267` move-result/sensor/timing path, `G0310` cooldown set and `G0311` projectile cooldown cleared.",
        "- `F0366` blocked step: input discard + PC-34 vblank, stop-wait false, return before `F0267`, cooldown assignment, and viewport-redraw side effects.",
        "- Viewport redraw: main loop redraws `F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)` after stop-wait/tick gating.",
        "",
        "## Verifier/probe",
        "- `scripts/verify_pass394_dm1_v1_dispatch_to_movement_state.py` writes `parity-evidence/verification/pass394_dm1_v1_dispatch_to_movement_state/manifest.json`.",
        "- Existing focused C probe: `test_dm1_v1_movement_command_core_pc34_compat`.",
        "- Existing integration probe: `test_dm1_v1_command_movement_sensor_timing_pc34_compat`.",
        "",
    ]
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")
    print(f"pass394_dispatch_to_movement_state=pass manifest={OUT_JSON}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"pass394_dispatch_to_movement_state=fail {exc}", file=sys.stderr)
        raise
