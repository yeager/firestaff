#!/usr/bin/env python3
"""Verify pass402 DM1 V1 movement cooldown tick/F0380 order.

ReDMCSB PC-34 GAMELOOP.C ages G0310/G0311 before the input wait loop calls
F0380. Firestaff live M11 bridge must therefore decrement old cooldowns
before DM1_V1_MovementPipeline_ProcessOneTickPc34Compat and must not decrement
again after that call, otherwise a newly written F0366 step cooldown loses one
tick in the same input tick.
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
        ]
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
