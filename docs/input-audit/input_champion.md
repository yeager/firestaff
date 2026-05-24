# DM1 V1 Champion Action Commands — Source Audit

## ReDMCSB Source Lock
ReDMCSB WIP20210206, Toolchains/Common/Source/COMMAND.C, CLIKCHAM.C, DEFS.H

## 1. Champion Action Command Hierarchy

### DEFS.H — Champion Action Command Ordinals
  C114 COMMAND_CLICK_IN_ACTION_AREA_ACTION_1       (zone C083)
  C115 COMMAND_CLICK_IN_ACTION_AREA_ACTION_2       (zone C084)
  C116 COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_0_ACTION (zone C089)
  C117 COMMAND_CLICK_IN_ACTION_AREA_CHAMPION_1_ACTION (zone C090)
  C126 COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT     (zone C114)
  C127 COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_RIGHT  (zone C115)
  C128 COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_LEFT   (zone C116)
  C129 COMMAND_RELEASE_CHAMPION_ICON               (survives flush)
  C254 COMMAND_STOP_PRESSING_EYE_MOUTH_WALL        (survives flush)

### COMMAND.C:178-192 — Action Area Hit-Test Rows (I34E/I34M)
  C114 (234-318, 98-108, LEFT): Action button row 1
  C115 (234-318, 110-120, LEFT): Action button row 2
  C116 (233-252, 86-120, LEFT): Champion 0 action area
  C117 (255-274, 86-120, LEFT): Champion 1 action area

### COMMAND.C:386-395 — Champion Icon Corner Commands (I34E/I34M)
Source-order active icon rows (COMMAND.C:389-391):
  C126 (TOP_RIGHT):    zone C114
  C127 (BOTTOM_RIGHT): zone C115
  C128 (BOTTOM_LEFT):  zone C116
  C125 is NOT defined in DEFS.H — ordinal 125 is a Firestaff extension.

### DEFS.H:1813 — Action Types
C001 ACTION_BLOCK, C002 ACTION_CHOP, C003 ACTION_X (attack),
C004 ACTION_BLOW_HORN, C005 ACTION_FLIP, C006 ACTION_PUNCH

## 2. Two-Phase Champion Action Selection

Phase 1 — Icon/Area click:
Champion action starts by clicking a champion icon corner or the
action/spell areas. This sets the action context (which champion,
which action type: attack/use/cast).

Phase 2 — Target click:
A second click on a valid dungeon view cell, object, or champion
selects the target and executes the action.

## 3. CLIKCHAM.C — Champion Interaction
- Champion icon corners select the action type per champion
- Eye/mouth/hand click during action mode: sets G0415_ui_LeaderEmptyHanded
- C254 clears this state (STOP_PRESSING_EYE_MOUTH_WALL)
- C129 removes the icon highlight (RELEASE_CHAMPION_ICON)

## 4. Firestaff Implementation

### Current Command Enum (include/dm1_v1_input_command_queue_pc34_compat.h)
  DM1_V1_COMMAND_CLICK_IN_SPELL_AREA = 100
  DM1_V1_COMMAND_CLICK_IN_ACTION_AREA = 111
  DM1_V1_COMMAND_CHAMPION_ICON_TOP_LEFT = 125
  DM1_V1_COMMAND_CHAMPION_ICON_TOP_RIGHT = 126  (matches C126)
  DM1_V1_COMMAND_CHAMPION_ICON_BOTTOM_RIGHT = 127 (matches C127)
  DM1_V1_COMMAND_CHAMPION_ICON_BOTTOM_LEFT = 128  (matches C128)
  DM1_V1_COMMAND_RELEASE_CHAMPION_ICON = 129    (matches C129)
  DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL = 254 (matches C254)

### command_for_primary_mouse (dm1_v1_input_command_queue_pc34_compat.c:89-115)
  Champion icon corners: TOP_LEFT(281,0,19,14), TOP_RIGHT(301,0,19,14),
    BOTTOM_RIGHT(301,15,19,14), BOTTOM_LEFT(281,15,19,14)
  Spell area: (233, 42, 87, 33) → DM1_V1_COMMAND_CLICK_IN_SPELL_AREA
  Action area: (233, 77, 87, 45) → DM1_V1_COMMAND_CLICK_IN_ACTION_AREA

All four icon corners correctly mapped to DM1_V1_COMMAND_CHAMPION_ICON_*.
Spell/action areas are single consolidated zones.

## 5. Gaps

### Gap 1: Action area is consolidated (not per-row/per-champion)
ReDMCSB: C114 (action row 1), C115 (action row 2),
  C116 (champion 0 action), C117 (champion 1 action) as separate commands.
Firestaff: one consolidated DM1_V1_COMMAND_CLICK_IN_ACTION_AREA (111)
  covering (233,77, 87,45) — loses per-row/per-champion granularity.
Impact: Low for basic play. Action menu system determines which action
based on current action row and champion context. But C114/C115/C116/C117
command dispatch distinction is lost.

### Gap 2: Spell area is consolidated (not per-champion)
ReDMCSB: per-champion spell area zones (C089-C092 for champions 0-3).
Firestaff: one consolidated DM1_V1_COMMAND_CLICK_IN_SPELL_AREA (100).
Impact: Low — champion is already contextually known from action context.

### Gap 3: C125 TOP_LEFT has no ReDMCSB equivalent
C125 is not defined in ReDMCSB DEFS.H (first icon command is C126 TOP_RIGHT).
Firestaff uses ordinal 125 for TOP_LEFT — this is a Firestaff extension
with no ReDMCSB source lock. No behavioral impact since the icon corner
detection logic is otherwise correct.

## 6. Verdict
PARTIALLY SOURCE-LOCKED. Champion icon corner routing is correct
(C126/C127/C128 match). Action and spell area commands are consolidated
into single generic commands rather than per-row/per-champion granularity
from ReDMCSB (C114-C117). This is a minor gap for action routing through
the action menu context system.
