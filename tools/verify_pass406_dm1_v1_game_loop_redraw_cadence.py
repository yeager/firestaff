#!/usr/bin/env python3
"""Pass406: DM1 V1 game-loop redraw/cadence ordering verifier.

This is a source-locked executable probe: it audits ReDMCSB WIP20210206 first,
then verifies Firestaff's live M11 bridge keeps the same ordering contract for
cooldown ageing, command processing, viewport dirty publication, draw, and
present/vblank evidence. It does not claim original DOS pixel/runtime parity.
"""
from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass406_dm1_v1_game_loop_redraw_cadence"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

SOURCES = {
    "GAMELOOP.C": RED / "GAMELOOP.C",
    "COMMAND.C": RED / "COMMAND.C",
    "DUNVIEW.C": RED / "DUNVIEW.C",
    "DRAWVIEW.C": RED / "DRAWVIEW.C",
    "main_loop_m11.c": ROOT / "main_loop_m11.c",
    "m11_game_view.c": ROOT / "m11_game_view.c",
    "dm1_v1_movement_pipeline_pc34_compat.c": ROOT / "dm1_v1_movement_pipeline_pc34_compat.c",
}


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f"{label}: missing {needle!r}")
    return pos


def require_order(text: str, needles: list[str], label: str) -> None:
    last = -1
    for needle in needles:
        pos = require(text, needle, label)
        if pos <= last:
            raise AssertionError(f"{label}: out of order {needle!r}")
        last = pos


def function_range(text: str, name: str, rettype: str | None = None) -> tuple[int, int, str]:
    if rettype:
        pattern = r"\b" + rettype + r"\s+" + re.escape(name) + r"\s*\("
    else:
        pattern = r"\b" + re.escape(name) + r"\s*\("
    m = re.search(pattern, text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return line_no(text, m.start()), line_no(text, pos), text[m.start():pos + 1]
    raise AssertionError(f"unterminated function {name}")


def enclosing_range(text: str, start_needle: str, end_needle: str, label: str) -> tuple[int, int, str]:
    start = require(text, start_needle, label)
    end = require(text[start:], end_needle, label) + start + len(end_needle)
    return line_no(text, start), line_no(text, end), text[start:end]


def run(cmd: list[str], timeout: int = 60) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    if p.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}:\n{p.stdout[-2000:]}")
    return p.stdout.strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    texts = {name: path.read_text(encoding="latin-1" if name.endswith(".C") else "utf-8") for name, path in SOURCES.items()}

    gl_start, gl_end, gl = enclosing_range(
        texts["GAMELOOP.C"],
        "STATICFUNCTION void F0002_MAIN_GameLoop_CPSDF",
        "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        "ReDMCSB GAMELOOP F0002 cadence block",
    )
    require_order(gl, [
        "G0318_i_WaitForInputMaximumVerticalBlankCount = 10;",
        "for (;;) { /*_Infinite loop_*/",
        "F0261_TIMELINE_Process_CPSEF();",
        "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        "G0313_ul_GameTime++;",
        "if (G0310_i_DisabledMovementTicks) {",
        "G0310_i_DisabledMovementTicks--;",
        "if (G0311_i_ProjectileDisabledMovementTicks) {",
        "G0311_i_ProjectileDisabledMovementTicks--;",
        "while (M527_IsCharacterInKeyboardBuffer()) {",
        "F0380_COMMAND_ProcessQueue_CPSC();",
    ], "ReDMCSB GAMELOOP draw/tick/input cadence")

    cmd_start, cmd_end, cmd = function_range(texts["COMMAND.C"], "F0380_COMMAND_ProcessQueue_CPSC")
    require_order(cmd, [
        "G0435_B_CommandQueueLocked = C1_TRUE;",
        "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
        "G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks",
        "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
    ], "ReDMCSB F0380 gate before dequeue")
    dequeue_tail = cmd[require(cmd, "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;", "ReDMCSB F0380 dequeue tail"):]
    require_order(dequeue_tail, [
        "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)",
        "G0435_B_CommandQueueLocked = C0_FALSE;",
        "F0360_COMMAND_ProcessPendingClick();",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ], "ReDMCSB F0380 dequeue/unlock/dispatch order")

    dv_start, dv_end, dv = enclosing_range(
        texts["DUNVIEW.C"],
        "void F0128_DUNGEONVIEW_Draw_CPSF",
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling();\n        }\n#endif\n}",
        "ReDMCSB DUNVIEW F0128 viewport draw block",
    )
    require_order(dv, [
        "if (G0297_B_DrawFloorAndCeilingRequested)",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
    ], "ReDMCSB viewport draw then blit request")

    draw_start, draw_end, draw = enclosing_range(
        texts["DRAWVIEW.C"],
        "void F0097_DUNGEONVIEW_DrawViewport",
        "M526_WaitVerticalBlank();",
        "ReDMCSB DRAWVIEW F0097 viewport blit/vblank block",
    )
    require_order(draw, [
        "G0324_B_DrawViewportRequested = C1_TRUE;",
        "M526_WaitVerticalBlank();",
    ], "ReDMCSB viewport blit/vblank request")

    m11_start, m11_end, m11 = enclosing_range(
        texts["m11_game_view.c"],
        "static int m11_apply_dm1_v1_pipeline_tick(M11_GameViewState* state,\n                                           M12_MenuInput input,\n                                           const char* actionLabel) {\n    int command;",
        "state->lastDm1V1MovementPipelineResult.core.queue.dequeued;",
        "Firestaff m11_apply_dm1_v1_pipeline_tick",
    )
    require_order(m11, [
        "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(",
        "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(",
        "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(",
        "state->world.gameTick++;",
        "return state->lastDm1V1MovementPipelineResult.viewportDirty ||",
    ], "Firestaff M11 live bridge cooldown/process/redraw publication order")

    loop_start, loop_end, loop = enclosing_range(
        texts["main_loop_m11.c"],
        "else if (result == M11_GAME_INPUT_REDRAW) {",
        "if (gameView.world.gameTick != tickBeforeInput) {",
        "Firestaff main loop input redraw block",
    )
    require_order(loop, [
        "redrawWasAfterViewportDirty =",
        "gameView.lastDm1V1MovementPipelineResult.viewportDirty;",
        "M11_GameView_Draw(&gameView,",
        "inputRedrawDrawCount++;",
        "inputRedrawAfterViewportDirtyCount++;",
        "lastInputRedrawAfterViewportDirty = redrawWasAfterViewportDirty;",
    ], "Firestaff input redraw instrumentation order")

    pipe_start, pipe_end, pipe = function_range(
        texts["dm1_v1_movement_pipeline_pc34_compat.c"],
        "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat",
        rettype="int",
    )
    require_order(pipe, [
        "DM1_V1_MovementCommandCore_ProcessOnePc34Compat(",
        "F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(",
        "DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(",
        "outResult->viewportDirty = outResult->core.viewportRedrawRequested;",
        "outResult->provenance.viewportPresentEvidence =",
    ], "Firestaff movement pipeline state-before-redraw provenance order")

    turning_stdout = run([str(ROOT / "build/test_m11_v1_turning_presentation_pc34_compat")])
    pipeline_stdout = run([str(ROOT / "build/test_dm1_v1_movement_pipeline_pc34_compat")])
    diffcheck_stdout = run(["git", "diff", "--check"])

    status = "PASS406_DM1_V1_GAME_LOOP_REDRAW_CADENCE_SOURCE_LOCKED"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceAudit": {
            "GAMELOOP.F0002": f"GAMELOOP.C:{gl_start}-{gl_end}",
            "COMMAND.F0380": f"COMMAND.C:{cmd_start}-{cmd_end}",
            "DUNVIEW.F0128": f"DUNVIEW.C:{dv_start}-{dv_end}",
            "DRAWVIEW.F0097": f"DRAWVIEW.C:{draw_start}-{draw_end}",
        },
        "firestaffGuards": {
            "m11_apply_dm1_v1_pipeline_tick": f"m11_game_view.c:{m11_start}-{m11_end}",
            "main_loop_input_redraw_block": f"main_loop_m11.c:{loop_start}-{loop_end}",
            "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat": f"dm1_v1_movement_pipeline_pc34_compat.c:{pipe_start}-{pipe_end}",
        },
        "checks": [
            "build/test_m11_v1_turning_presentation_pc34_compat",
            "build/test_dm1_v1_movement_pipeline_pc34_compat",
            "git diff --check",
        ],
        "checkOutputFirstLines": {
            "turningPresentation": turning_stdout.splitlines()[0] if turning_stdout else "",
            "movementPipeline": pipeline_stdout.splitlines()[0] if pipeline_stdout else "",
            "diffCheck": diffcheck_stdout.splitlines()[0] if diffcheck_stdout else "",
        },
        "notClaimed": [
            "original DOSBox/FIRES.EXE runtime breakpoint parity",
            "pixel-perfect viewport output parity",
            "global game-loop equivalence outside DM1 V1 M11 bridge cadence",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    REPORT.write_text("\n".join([
        "# Pass406 — DM1 V1 game-loop redraw/cadence gate",
        "",
        f"Status: `{status}`",
        "",
        "## ReDMCSB-first source audit",
        f"- `GAMELOOP.C:{gl_start}-{gl_end}` / `F0002_MAIN_GameLoop_CPSDF` — sets the V1 wait budget, redraws the dungeon view from current party state, advances game time, decrements movement/projectile cooldowns, then processes queued commands in the input wait loop.",
        f"- `COMMAND.C:{cmd_start}-{cmd_end}` / `F0380_COMMAND_ProcessQueue_CPSC` — locks the queue, gates movement on `G0310_i_DisabledMovementTicks` / `G0311_i_ProjectileDisabledMovementTicks`, dequeues, unlocks, and dispatches turn/move commands.",
        f"- `DUNVIEW.C:{dv_start}-{dv_end}` / `F0128_DUNGEONVIEW_Draw_CPSF` — draws the viewport from supplied direction/map coordinates and ends by requesting a viewport draw.",
        f"- `DRAWVIEW.C:{draw_start}-{draw_end}` / `F0097_DUNGEONVIEW_DrawViewport` — sets `G0324_B_DrawViewportRequested` and waits for vblank.",
        "",
        "## Firestaff executable guard",
        f"- `m11_game_view.c:{m11_start}-{m11_end}` / `m11_apply_dm1_v1_pipeline_tick` — enqueues the route command, ages old cooldowns before processing, processes one pipeline tick, publishes game tick/hash, and returns redraw/dequeue state.",
        f"- `dm1_v1_movement_pipeline_pc34_compat.c:{pipe_start}-{pipe_end}` / `DM1_V1_MovementPipeline_ProcessOneTickPc34Compat` — applies command/movement/post-move/timing before publishing `viewportDirty` provenance.",
        f"- `main_loop_m11.c:{loop_start}-{loop_end}` — records whether input redraw followed a viewport-dirty pipeline result before calling `M11_GameView_Draw`.",
        "",
        "## Gates run",
        "- `build/test_m11_v1_turning_presentation_pc34_compat`",
        "- `build/test_dm1_v1_movement_pipeline_pc34_compat`",
        "- `git diff --check` (also run inside this verifier)",
        "",
        "## Scope guard",
        "- This is a source-locked executable verifier/probe. It does not claim original DOSBox/FIRES.EXE breakpoint parity or pixel-perfect viewport parity.",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]) + "\n")

    print(status)
    print(f"- ReDMCSB: GAMELOOP.C:{gl_start}-{gl_end}; COMMAND.C:{cmd_start}-{cmd_end}; DUNVIEW.C:{dv_start}-{dv_end}; DRAWVIEW.C:{draw_start}-{draw_end}")
    print(f"- Firestaff: m11_game_view.c:{m11_start}-{m11_end}; main_loop_m11.c:{loop_start}-{loop_end}; dm1_v1_movement_pipeline_pc34_compat.c:{pipe_start}-{pipe_end}")
    print(f"- report: {REPORT.relative_to(ROOT)}")
    print(f"- manifest: {MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, subprocess.SubprocessError) as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        raise SystemExit(1)
