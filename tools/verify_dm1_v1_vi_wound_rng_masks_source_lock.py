#!/usr/bin/env python3
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="latin-1")
    except FileNotFoundError:
        print(f"missing: {path}")
        sys.exit(1)


def require(label: str, condition: bool) -> None:
    if not condition:
        print(f"FAIL {label}")
        sys.exit(1)
    print(f"ok {label}")


panel = read(REDMCSB / "PANEL.C")
defs = read(REDMCSB / "DEFS.H")
base = read(REDMCSB / "BASE.C")
data = read(REDMCSB / "DATA.C")
consumables = read(ROOT / "src/dm1/dm1_v1_inventory_consumables_pc34_compat.c")
m11 = read(ROOT / "src/engine/m11_game_view.c")
test = read(ROOT / "tests/test_dm1_v1_inventory_consumables_pc34_compat.c")

require("redmcsb vi potion switch case", "case C14_POTION_VI_POTION:" in panel)
require("redmcsb vi initial heal iteration count",
        "F0025_MAIN_GetMaximumValue(1, (((POTION*)L1082_ps_Junk)->Power / 42))" in panel)
require("redmcsb vi random wound mask and-apply", "L1083_ps_Champion->Wounds &= M006_RANDOM(65536);" in panel)
require("redmcsb vi retries reset to one mask", "AL1088_ui_HealWoundIterationCount = 1;" in panel)
require("redmcsb vi ten try no-change loop",
        "while ((L1087_ui_Wounds == L1083_ps_Champion->Wounds) && --L1086_ui_Counter)" in panel)
require("redmcsb wound redraw/load attributes", "MASK0x0200_LOAD | MASK0x2000_WOUNDS" in panel)
require("redmcsb m006 is raw 16-bit rng", "#define M006_RANDOM(value) F0027_MAIN_Get16bitRandomNumber()" in defs)
require("redmcsb pc 3.4 rng multiplier", "0xBB40E62D + 11" in base)
require("redmcsb random wound probability mask table",
        "G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask[4] = { MASK0x0020_WOUND_FEET, MASK0x0010_WOUND_LEGS, MASK0x0008_WOUND_TORSO, MASK0x0004_WOUND_HEAD }" in data)

require("firestaff evidence cites vi source lines", "PANEL.C:1898-1910" in consumables)
require("firestaff pure vi heal iteration count", "healIterations = max_int_pc34(1, potionPower / 42);" in consumables)
require("firestaff pure vi applies caller random masks", "champion->wounds &= mask;" in consumables)
require("firestaff pure vi retries reset to one mask", "healIterations = 1;" in consumables)
require("firestaff pure vi ten try no-change loop",
        re.search(r"while \(\(originalWounds == champion->wounds\) && --tries\)", consumables) is not None)
require("firestaff runtime pre-rolls maximum vi masks", "M11_DM1_V1_VI_WOUND_MASK_MAX_PC34 = 15" in m11)
require("firestaff runtime uses pc34 16-bit rng formula", "0xBB40E62Du" in m11 and "rng->seed >> 8" in m11)
require("firestaff runtime passes vi masks into consumable resolver", "viWoundMaskCount ? viWoundMasks : NULL" in m11)
require("firestaff test covers retry mask reset", "VI retry resets to one random mask" in test)
require("firestaff test covers ten failed tries", "VI stops after ten wound-mask tries" in test)

print("DM1_V1_VI_WOUND_RNG_MASKS_SOURCE_LOCK_VERIFIED")
