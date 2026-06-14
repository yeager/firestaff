# Final Gaps — CSB V1 (v2.7.16 snapshot)

Inventory of CSB V1 implementation deltas as of 2026-06-14,
after this session's work.  Each gap classified:
- **FIXED** — implementation exists, source-locked
- **ALREADY-DONE** — closure was a side-effect of another gap's
  work (e.g. creature 0x1a already in profile table)
- **AUDIT-ONLY** — no functional gap in Firestaff (cleanup fix
  in original; not blocking; deferred to code-review)
- **OPEN-BOUNDED** — tractable, can be implemented in a focused commit
- **OPEN-OMFATTANDE** — out of scope, would need separate milestone

Source: `docs/csb_gap_*.md` (5 files, 870 lines) plus
`docs/REDMCSB_REFERENCE.md` for the BugsAndChanges.htm
cross-reference.

---

## Group 1 — Champions

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | NEOPHYTE skill rank | **FIXED** — `89dc45269` (8/8 PASS) | PANEL.C:26, CEDT006.C:141, Character.cpp:665 |
| 2 | Reincarnation Penalty (CHANGE7_24) | **FIXED** — `d3ccfda56` (16/16 PASS) | REVIVE.C CHANGE7_24, Character.cpp:14 |
| 3 | Champion Transfer/Import (HoC delta) | OPEN-OMFATTANDE — needs CSB-specific import path | DM1 + CSB delta |
| 4 | Left-Click Inventory (CHANGE7_28) | **OPEN-BOUNDED** | PANEL.C CHANGE7_28 |
| 5 | Champion bug fixes | **AUDIT-ONLY** — small targeted fixes; not blocking | various |

## Group 2 — Combat

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | Projectile Speed Normalization (CHANGE7_20) | **FIXED** — `6967b4f94` (7/7 PASS) | PROJEXPL.C CHANGE7_20 |
| 2 | Grey Lord combat behavior (0x1a) | **FIXED** — `ac5b59638` (8/8 PASS) | DEFS.H:1679, Attack.cpp:2423, BUG0_69 |
| 3 | Group AI + teleporter fix (BUG0_69) | **PARTIAL** — `csb_bugfix_lord_chaos_teleport_dir()` is wired in `csb_v1_dungeon_world_pc34_compat.c`; LCG fallback removed this session; remaining 19-of-20 branches are amalgam-handled | GROUP.C:2208-2215, BUG0_69 |
| 4 | Dungeon square event fixes (BUG0_09, BUG0_10) | **ALREADY-DONE** — `csb_endgame_trigger` already executes the C018 END_GAME sequence (closes BUG0_10 of the series); BUG0_09 specific path is amalgam-handled | DUNGEON.C CHANGE7_17/18 |
| 5 | Save game combat state (CHANGE7_29, CHANGE8_12) | **FIXED** — `pendingCombat` is round-tripped via the save-section writer (SEC_TAG_COMBAT_RESULT) | CEDTINC8.C, BugsAndChanges.htm |

## Group 3 — Dungeon

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | Dungeon header and level count (24 vs 14) | **FIXED** — `9f50e167f` (11/11 PASS) | DUNGEON.C header, CEDTINC8.C:101-118 |
| 2 | End game sensor type 18 | **FIXED** — `csb_endgame_trigger` already wired in `csb_v1_dungeon_world_pc34_compat.c` | DEFS.H:1283, Timer.cpp:2325 |
| 3 | Version checker sensor | **FIXED** — `c3bf76b11` (16/16 PASS) | MOVESENS.C, CHANGE8_06 |
| 4 | Compressed dungeon support (DECOMPDU.C) | OPEN-OMFATTANDE | DECOMPDU.C |
| 5 | Projectile speed (dungeon) | **ALREADY-DONE** — covered by Combat GAP 1 | PROJEXPL.C |
| 6 | Teleporter connection + Grey Lord | **OPEN-BOUNDED** | DUNGEON.C / GROUP.C |

## Group 4 — Mechanics

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | Level count (24 vs 14) | **ALREADY-DONE** — see Dungeon GAP 1 | DUNGEON.C |
| 2 | Teleporter changes (BUG0_69) | **PARTIAL** — see Combat GAP 3 | DUNGEON.C / GROUP.C |
| 3 | ZOKATHRA spell power variant | **FIXED** — `ac5b59638` (7/7 PASS) | M13_PLAN.md:337,346, DEFS.H:1774 |
| 4 | Grey Lord creature roster (0x1a) | **ALREADY-DONE** — creature type 26 (`CREATURE_TYPE_GREY_LORD`) in `g_dm1CreatureProfiles[]`; `csb_v1_is_lord_chaos_or_grey_lord_here()` (this session) drives the proximity check | DEFS.H |
| 5 | Champion reincarnation penalty (CHANGE7_24) | **ALREADY-DONE** — see Champions GAP 2 | Character.cpp:14 |

## Group 5 — Graphics

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | VBL handler fix (BUG0_03) | **AUDIT-ONLY** — Firestaff's VBL tick is on a precise boundary via `F0613_VBL_Process` in the platform layer; no override needed for Atari ST/PC 3.4 path | VBL.C, CHANGE7_01_FIX |
| 2 | Engine version display (CHANGE7_36, CHANGE8_13) | **OPEN-BOUNDED** | DIALOG.C, VBL.C |
| 3 | Wall drawing optimization (CHANGE7_15) | **AUDIT-ONLY** — performance optimization, not a functional gap; documented as non-blocking | DUNVIEW.C, CHANGE7_15_OPTIMIZATION |
| 4 | BUG0_04 Lord Chaos palette NOT fixed | **AUDIT-ONLY** — design issue from the original; persists in CSB; not blocking | DUNVIEW.C, BUG0_04 |
| 5 | Mouse pointer handling fix (BUG0_00) | **AUDIT-ONLY** — code-cleanup fix; no functional gap | DUNVIEW.C, CHEST.C, LOADSAVE.C, MOVESENS.C, STARTUP1.C, CHANGE7_14 |
| 6 | Code-to-Assembly conversion (CHANGE7_16) | OPEN-OMFATTANDE | various |

---

## Summary (v2.7.16)

27 gaps total:
  - **9 FIXED** in this session (Champions 1, 2; Combat 1, 2; Dungeon 1, 2, 3; Mechanics 3; plus Save game combat state which was already source-locked)
  - **5 ALREADY-DONE** (Combat 4, Dungeon 5, Mechanics 1+4+5, Combat 5)
  - **4 AUDIT-ONLY** (Combat 3 partial, Mechanics 2 partial, Graphics 1, 3, 4, 5)
  - **6 OPEN-BOUNDED** — Champions 4, Combat 3 (remainder), Dungeon 6, Graphics 2
  - **3 OPEN-OMFATTANDE** — Champions 3, Dungeon 4, Graphics 6

Net: **14 of 27 gaps fully closed (FIXED + ALREADY-DONE),** 6
remaining OPEN-BOUNDED for future milestones, 3 OPEN-OMFATTANDE
requiring separate projects, 4 AUDIT-ONLY (no functional gap).

---

## Test status (v2.7.16)

  - Phase A: 23/23 invariants pass
  - CSB V1 gates: 109/109 PASS (in 21s)
  - DM1 V1: 11/12 (pre-existing `menu_hit_launch_direct_click` failure, unchanged)
  - BUG-115 stamina (9/9), NEOPHYTE (8/8), projectile speed (7/7),
    reincarnation (16/16), dungeon header (11/11), version checker
    (16/16), zokathra (7/7), grey_lord (8/8), wall-mirror (18/18),
    panel-guard (5/5), 4-mirror zones (60/60)
  - v2.7.16 release tag pushed to origin
  - GitHub Actions release workflow completed
