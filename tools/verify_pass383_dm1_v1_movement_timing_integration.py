#!/usr/bin/env python3
"""Pass383 static/runtime-artifact gate for DM1 V1 movement timing integration."""
from __future__ import annotations

import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUTDIR = ROOT / "parity-evidence" / "verification" / "pass383_dm1_v1_movement_timing_integration"
OUT = OUTDIR / "manifest.json"


def read(path: Path, enc: str = "utf-8") -> str:
    return path.read_text(encoding=enc)


def line_no(text: str, needle: str, start: int = 0) -> int:
    pos = text.find(needle, start)
    if pos < 0:
        raise AssertionError(f"missing marker {needle!r}")
    return text.count("\n", 0, pos) + 1


def require(text: str, needle: str, file_name: str, claim: str, start: int = 0) -> dict:
    return {"claim": claim, "file": file_name, "line": line_no(text, needle, start), "ok": True}


def require_order(text: str, markers: list[str], file_name: str, claim: str, start: int = 0) -> dict:
    lines: list[int] = []
    cursor = start
    last_pos = start - 1
    for marker in markers:
        pos = text.find(marker, cursor)
        if pos < 0:
            raise AssertionError(f"{claim}: missing {marker!r}")
        if pos <= last_pos:
            raise AssertionError(f"{claim}: out-of-order {marker!r}")
        lines.append(text.count("\n", 0, pos) + 1)
        last_pos = pos
        cursor = pos + len(marker)
    return {"claim": claim, "file": file_name, "lines": f"{lines[0]}-{lines[-1]}", "ok": True}


def grep_status(path: Path, expected: str) -> dict:
    text = read(path)
    if expected not in text:
        raise AssertionError(f"{path}: expected status {expected!r}")
    return {"artifact": str(path.relative_to(ROOT)), "expected": expected, "ok": True}


def main() -> int:
    command = read(RED / "COMMAND.C", "latin-1")
    clik = read(RED / "CLIKMENU.C", "latin-1")
    gameloop = read(RED / "GAMELOOP.C", "latin-1")
    vblank = read(RED / "VBLANK.C", "latin-1")
    io = read(RED / "IO.C", "latin-1")
    entrance = read(RED / "ENTRANCE.C", "latin-1")
    moves = read(RED / "MOVESENS.C", "latin-1")
    fire_pipeline = read(ROOT / "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c")
    fire_timing = read(ROOT / "src/dm1/dm1_v1_movement_timing_pc34_compat.c")

    proofs: list[dict] = []
    f0380_start = command.index("void F0380_COMMAND_ProcessQueue_CPSC")
    f0366_start = clik.index("void F0366_COMMAND_ProcessTypes3To6_MoveParty")
    block_start = clik.index("if (L1117_B_MovementBlocked) {", f0366_start)
    proofs.append(require_order(command, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "goto T0380042;",
    ], "COMMAND.C", "movement cooldown gate returns before dequeue, so the queued C003..C006 command is retained", f0380_start))
    proofs.append(require_order(command, [
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
        "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "COMMAND.C", "ungated command dequeues exactly one entry, unlocks/replays pending click, then dispatches movement", f0380_start))
    proofs.append(require_order(clik, [
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        "AL1115_ui_Ticks = 1;",
        "AL1115_ui_Ticks = F0025_MAIN_GetMaximumValue(AL1115_ui_Ticks, F0310_CHAMPION_GetMovementTicks(L1119_ps_Champion));",
        "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        "G0311_i_ProjectileDisabledMovementTicks = 0;",
    ], "CLIKMENU.C", "accepted step stops the wait loop and arms movement cooldown from max living champion movement ticks", f0366_start))
    proofs.append(require_order(clik, [
        "if (L1117_B_MovementBlocked) {",
        "F0357_COMMAND_DiscardAllInput();",
        "F0693_WaitVerticalBlank();",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "return;",
    ], "CLIKMENU.C", "blocked movement discards buffered input, waits one vblank on PC-family ports, and keeps the input wait loop alive", block_start))
    proofs.append(require_order(moves, [
        "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;",
        "G0407_s_Party.ScentCount++;",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(P0558_i_SourceMapX, P0559_i_SourceMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C0_FALSE);",
        "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, L0725_B_PartySquare, C1_TRUE);",
    ], "MOVESENS.C", "real party square changes update last movement time before leave/enter sensor processing"))
    proofs.append(require_order(gameloop, [
        "G0318_i_WaitForInputMaximumVerticalBlankCount = 10;",
        "G0317_i_WaitForInputVerticalBlankCount = 0;",
        "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        "G0313_ul_GameTime++;",
        "G0310_i_DisabledMovementTicks--;",
        "G0311_i_ProjectileDisabledMovementTicks--;",
        "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
        "F0380_COMMAND_ProcessQueue_CPSC();",
        "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
    ], "GAMELOOP.C", "PC34 game loop redraws first, ticks game time/cooldowns once, then drains input/commands until stop-wait or ticking gate"))
    proofs.append(require_order(vblank, [
        "G0317_i_WaitForInputVerticalBlankCount++;",
        "if (G0317_i_WaitForInputVerticalBlankCount >= G0318_i_WaitForInputMaximumVerticalBlankCount) {",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
    ], "VBLANK.C", "vblank cadence flips StopWaitingForPlayerInput when the per-platform threshold is reached"))
    proofs.append(require_order(io, [
        "G0317_i_WaitForInputVerticalBlankCount += 3;",
        "if (G0317_i_WaitForInputVerticalBlankCount >= G0318_i_WaitForInputMaximumVerticalBlankCount) {",
        "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
    ], "IO.C", "PC-family wait primitive advances the same vblank counter/stop-wait gate"))
    proofs.append(require_order(entrance, [
        "L3366_l_ = L3367_l_ = G0317_i_WaitForInputVerticalBlankCount;",
        "if (G0317_i_WaitForInputVerticalBlankCount >= L3366_l_) {",
        "while (G0317_i_WaitForInputVerticalBlankCount < L3367_l_);",
        "L3367_l_ = G0317_i_WaitForInputVerticalBlankCount + 2;",
    ], "ENTRANCE.C", "ENTRANCE uses the same vblank counter for door animation pacing, not for party movement command acceptance"))
    proofs.append(require(fire_pipeline, "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat", "src/dm1/dm1_v1_movement_pipeline_pc34_compat.c", "Firestaff exposes explicit per-loop cooldown decrement seam"))
    proofs.append(require(fire_timing, "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat", "src/dm1/dm1_v1_movement_timing_pc34_compat.c", "Firestaff has a source-locked successful-step timing seam"))

    artifact_checks = [
        grep_status(ROOT / "parity-evidence/pass349_dm1_v1_full_launcher_keypad_runtime_route.md", "FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED"),
        grep_status(ROOT / "parity-evidence/verification/pass339b_dm1_v1_runtime_movement_gate/manifest.json", "MOVEMENT_PROVED"),
        grep_status(ROOT / "parity-evidence/pass211_dm1_v1_original_movement_fresh_blocker.md", "BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE"),
        grep_status(ROOT / "parity-evidence/pass243_dm1_v1_dunview_emulator_variant_runbook.md", "BLOCKED_DUNVIEW_TCPP101_DOSBOX_VARIANTS_EXHAUSTED"),
    ]

    build_dir = Path("~/.openclaw/data/firestaff-builds/pass383-verify").expanduser()
    build_dir.mkdir(parents=True, exist_ok=True)
    if not (build_dir / "CTestTestfile.cmake").exists():
        cfg = subprocess.run(["cmake", "-S", str(ROOT), "-B", str(build_dir)],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
        if cfg.returncode != 0:
            raise AssertionError(cfg.stdout)
    bld = subprocess.run(["cmake", "--build", str(build_dir), "--target",
        "test_dm1_v1_command_movement_sensor_timing_pc34_compat",
        "test_dm1_v1_movement_pipeline_pc34_compat", "-j2"],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    if bld.returncode != 0:
        raise AssertionError(bld.stdout)
    ctest = subprocess.run(
        ["ctest", "--test-dir", str(build_dir), "--output-on-failure", "-R", "dm1_v1_command_movement_sensor_timing_pc34_compat|dm1_v1_movement_pipeline_pc34_compat"],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    if ctest.returncode != 0:
        raise AssertionError(ctest.stdout)

    branch = subprocess.check_output(["git", "branch", "--show-current"], cwd=ROOT, text=True).strip()
    manifest = {
        "schema": "firestaff.parity.pass383.dm1_v1_movement_timing_integration.v1",
        "status": "PASS_DM1_V1_MOVEMENT_TIMING_GATES_SOURCE_LOCKED_RUNTIME_BOUNDARY_CLASSIFIED",
        "branch": branch,
        "sourceRoot": str(RED),
        "proofs": proofs,
        "artifactChecks": artifact_checks,
        "runtimeBoundary": {
            "firestaffRuntime": "pass339b/pass349 prove live-route command delivery through Firestaff movement pipeline",
            "originalDosboxDunview": "pass211/pass243 remain blockers: no semantically aligned original movement sequence and no authentic FIRES.MAP/DUNVIEW build route",
            "pixelParityClaim": False,
        },
        "buildDir": str(build_dir),
        "ctestTail": ctest.stdout[-4000:],
    }
    OUTDIR.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    print(manifest["status"])
    print(f"manifest={OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
