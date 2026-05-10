#!/usr/bin/env python3
"""Verify pass402 DM1 V1 movement cooldown tick/F0380 order.

ReDMCSB PC-34 GAMELOOP.C ages G0310/G0311 before the input wait loop calls
F0380. Firestaff live M11 bridge must therefore decrement old cooldowns
before DM1_V1_MovementPipeline_ProcessOneTickPc34Compat and must not decrement
again after that call, otherwise a newly written F0366 step cooldown loses one
tick in the same input tick.  Non-movement real ticks that bypass the
DM1-V1 command pipeline must also age the live cooldown mirror once before
calling the runtime tick orchestrator.
"""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]

def die(msg: str) -> None:
    raise SystemExit(msg)

def read(path: str) -> str:
    return (ROOT / path).read_text()

def index_or_die(haystack: str, needle: str, label: str) -> int:
    i = haystack.find(needle)
    if i < 0:
        die(f"missing {label}: {needle}")
    return i

def line_of(text: str, index: int) -> int:
    return text.count("\n", 0, index) + 1


def function_slice(text: str, start_marker: str, end_marker: str, label: str) -> tuple[int, str]:
    start = index_or_die(text, start_marker, label)
    end = index_or_die(text[start + 1:], end_marker, f"end of {label}") + start + 1
    return start, text[start:end]

def assert_cooldown_before_orch(text: str, start_marker: str, end_marker: str, label: str) -> tuple[int, int]:
    start, body = function_slice(text, start_marker, end_marker, label)
    dec = index_or_die(body, "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat", f"{label} cooldown decrement")
    orch = index_or_die(body, "F0884_ORCH_AdvanceOneTick_Compat", f"{label} orchestrator tick")
    if not dec < orch:
        die(f"{label} must decrement movement cooldown before F0884_ORCH_AdvanceOneTick_Compat")
    return line_of(text, start + dec), line_of(text, start + orch)

m11 = read("m11_game_view.c")
func_start = index_or_die(m11, "static int m11_apply_dm1_v1_pipeline_tick(M11_GameViewState* state,\n                                           M12_MenuInput input,\n                                           const char* actionLabel) {", "m11 pipeline tick definition")
func_end = index_or_die(m11[func_start + 1:], "static int m11_apply_tick", "next m11 function") + func_start + 1
func = m11[func_start:func_end]

enqueue = index_or_die(func, "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat", "enqueue")
pre_dec = index_or_die(func, "DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat", "pre-F0380 cooldown decrement")
process = index_or_die(func, "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat", "F0380 pipeline process")
reset = index_or_die(func, "DM1_V1_VBlankTiming_ResetForNewTick", "vblank reset")
if not (enqueue < pre_dec < process < reset):
    die("m11 bridge order must be enqueue -> cooldown decrement -> ProcessOneTick(F0380) -> vblank reset")
if func.find("DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat", process) != -1:
    die("m11 bridge must not decrement movement cooldown after ProcessOneTick")
comment = "by this successful step must not be decremented in the same tick"
index_or_die(func, comment, "same-tick cooldown preservation comment")

spell_tick_lines = assert_cooldown_before_orch(
    m11,
    "int M11_GameView_CastSpell(M11_GameViewState* state) {",
    "static void m11_apply_survival_drain",
    "M11_GameView_CastSpell",
)

pipeline = read("dm1_v1_movement_pipeline_pc34_compat.c")
index_or_die(pipeline, "COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC", "pipeline F0380 source lock")
index_or_die(pipeline, "GAMELOOP.C:150-155", "pipeline GAMELOOP cooldown source lock")

timing = read("dm1_v1_movement_timing_pc34_compat.c")
index_or_die(timing, "CLIKMENU.C:330-346", "timing F0366 cooldown source lock")
index_or_die(timing, "GAMELOOP.C:150-155", "timing GAMELOOP cooldown source lock")

out = {
    "status": "ok",
    "claim": "M11 DM1 V1 live bridge ages old G0310/G0311 cooldowns before F0380 processing and preserves newly assigned F0366 cooldowns until the next tick.",
    "firestaffAnchors": {
        "m11_game_view.c:m11_apply_dm1_v1_pipeline_tick": [
            line_of(m11, func_start + enqueue),
            line_of(m11, func_start + pre_dec),
            line_of(m11, func_start + process),
            line_of(m11, func_start + reset),
        ],
        "m11_game_view.c:M11_GameView_CastSpell": list(spell_tick_lines),
    },
    "redmcsbAnchors": {
        "CHAMPION.C:F0310_CHAMPION_GetMovementTicks": [1180, 1215],
        "CLIKMENU.C:F0366 cooldown assignment": [330, 346],
        "COMMAND.C:F0380 movement-disabled gates": [2095, 2100],
        "GAMELOOP.C:disabled tick decrement before F0380": [150, 155, 215],
        "MOVESENS.C:F0267 party movement side effects": [752, 775],
    },
}
print(json.dumps(out, indent=2, sort_keys=True))
