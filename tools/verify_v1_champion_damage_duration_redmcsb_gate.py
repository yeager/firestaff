#!/usr/bin/env python3
"""Source-lock Firestaff V1 champion damage overlay duration to ReDMCSB.

ReDMCSB CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds
schedules C12_EVENT_HIDE_DAMAGE_RECEIVED at G0313_ul_GameTime + 5 and
refreshes an existing hide event to the same +5 deadline. Firestaff keeps
that status HUD overlay timer separate from the red viewport damage flash.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "m11_game_view.h"
IMPL = ROOT / "m11_game_view.c"
REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/CHAMPION.C")

checks = []

def require(label: str, ok: bool, detail: str) -> None:
    checks.append((label, ok, detail))

h = HEADER.read_text(errors="replace")
c = IMPL.read_text(errors="replace")
r = REDMCSB.read_text(errors="replace")

macro = re.search(r"#define\s+M11_CHAMPION_DAMAGE_OVERLAY_DURATION\s+(\d+)", h)
require("firestaff_header_has_champion_damage_duration_macro", macro is not None, str(HEADER))
require("firestaff_champion_damage_duration_is_5_ticks", macro is not None and macro.group(1) == "5", macro.group(1) if macro else "missing")
require("firestaff_notify_champion_damage_uses_champion_duration", "championDamageTimer[championSlot] = M11_CHAMPION_DAMAGE_OVERLAY_DURATION;" in c, str(IMPL))
require("firestaff_viewport_flash_stays_separate_4_ticks", re.search(r"#define\s+M11_DAMAGE_FLASH_DURATION\s+4\b", h) is not None, str(HEADER))
require("redmcsb_schedules_hide_damage_at_game_time_plus_5", "C12_EVENT_HIDE_DAMAGE_RECEIVED" in r and "G0313_ul_GameTime + 5" in r, str(REDMCSB))
require("redmcsb_refreshes_existing_hide_damage_to_plus_5", "G0370_ps_Events[AL0969_i_EventIndex].Map_Time, G0309_i_PartyMapIndex, G0313_ul_GameTime + 5" in r, str(REDMCSB))

print("probe=firestaff_v1_champion_damage_duration_redmcsb_gate")
print("sourceEvidence=CHAMPION.C:1780-1784 creates C12_EVENT_HIDE_DAMAGE_RECEIVED at G0313_ul_GameTime+5; CHAMPION.C:1790-1793 refreshes existing hide event to G0313_ul_GameTime+5; CHAMDRAW.C:680-698 draws C015/C016 damage overlay and immediately redraws champion state.")
failed = 0
for label, ok, detail in checks:
    status = "ok" if ok else "FAIL"
    print(f"{label}={status} detail={detail}")
    failed |= 0 if ok else 1
sys.exit(failed)
