# Final Gaps — CSB V1

Honest inventory of CSB V1 gaps as of 2026-06-14, after this
session's DM1 V1 parity work.  97 CSB source files, 94 CSB
tests; 5 gap-doc files documenting ~25 implementation deltas.

Each gap classified as:
- **FIXED** — implementation exists, source-locked
- **OPEN-BOUNDED** — can be implemented in a focused commit
- **OPEN-OMFATTANDE** — out of scope, would need separate milestone

---

## Group 1 — Champions (docs/csb_gap_champions.md, 170 lines, 5 gaps)

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | NEOPHYTE skill rank | PARTIAL — `CSB_V1_RANK_NEOPHYTE=0` constant exists, but `neophyteSkills` global, mode flag, validation, and skill progression UI all deferred | PANEL.C:26,67 · CEDTDATA.C:16 · CEDT006.C:141 |
| 2 | Reincarnation Penalty (CHANGE7_24) | OPEN-BOUNDED | REVIVE.C CHANGE7_24, Character.cpp:14 (3 globals) |
| 3 | Champion Transfer/Import (HoC delta) | OPEN-OMFATTANDE — needs CSB-specific import path | DM1 + CSB delta |
| 4 | Left-Click Inventory (CHANGE7_28) | OPEN-BOUNDED | PANEL.C CHANGE7_28 |
| 5 | Champion bug fixes | OPEN-BOUNDED — small targeted fixes | various |

## Group 2 — Combat (docs/csb_gap_combat.md, 146 lines, 5 gaps)

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | Projectile Speed Normalization (CHANGE7_20) | OPEN-BOUNDED | PROJEXPL.C CHANGE7_20 |
| 2 | Grey Lord combat behavior (new creature) | OPEN-BOUNDED | DEFS.H:1679 · Attack.cpp:2423 |
| 3 | Group AI + teleporter fix (BUG0_69) | OPEN-BOUNDED | GROUP.C CHANGE7_19 |
| 4 | Dungeon square event fixes (BUG0_09, BUG0_10) | OPEN-BOUNDED | DUNGEON.C CHANGE7_17,7_18 |
| 5 | Save game combat state (CHANGE7_29, CHANGE8_12) | OPEN-BOUNDED | various |

## Group 3 — Dungeon (docs/csb_gap_dungeon.md, 159 lines, 6 gaps)

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | Dungeon header and level count (24 vs 14) | OPEN-BOUNDED | DUNGEON.C header |
| 2 | End game sensor type 18 | OPEN-BOUNDED | SENSOR.C |
| 3 | Version checker sensor | OPEN-BOUNDED | SENSOR.C |
| 4 | Compressed dungeon support (DECOMPDU.C) | OPEN-OMFATTANDE | DECOMPDU.C |
| 5 | Projectile speed (dungeon) | OPEN-BOUNDED | PROJEXPL.C |
| 6 | Teleporter connection + Grey Lord | OPEN-BOUNDED | DUNGEON.C / GROUP.C |

## Group 4 — Mechanics (docs/csb_gap_mechanics.md, 201 lines, 5 gaps)

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | Level count (24 vs 14) | OPEN-BOUNDED | DUNGEON.C |
| 2 | Teleporter changes (BUG0_69) | OPEN-BOUNDED | DUNGEON.C / GROUP.C |
| 3 | ZOKATHRA spell power variant | OPEN-BOUNDED | MAGIC.C |
| 4 | Grey Lord creature roster (0x1a) | OPEN-BOUNDED | DEFS.H |
| 5 | Champion reincarnation penalty (CHANGE7_24) | OPEN-BOUNDED | Character.cpp:14 |

## Group 5 — Graphics (docs/csb_gap_graphics.md, 159 lines, 6 gaps)

| # | Title | Status | Source |
|---|-------|--------|--------|
| 1 | VBL handler fix (BUG0_03) | OPEN-BOUNDED | VBL.C |
| 2 | Engine version display (CHANGE7_36, CHANGE8_13) | OPEN-BOUNDED | VBL.C |
| 3 | Wall drawing optimization (CHANGE7_15) | OPEN-BOUNDED | DUNVIEW.C |
| 4 | BUG0_04 Lord Chaos palette NOT fixed | OPEN-BOUNDED | DUNVIEW.C |
| 5 | Mouse pointer handling fix (BUG0_00) | OPEN-BOUNDED | INPUT.C |
| 6 | Code-to-Assembly conversion (CHANGE7_16) | OPEN-OMFATTANDE | various |

---

## Summary

**27 gaps total:** 0 FIXED, ~24 OPEN-BOUNDED, ~3 OPEN-OMFATTANDE.

Highest-impact B-bounded gaps to implement first:
1. **NEOPHYTE rank** (Champions 1) — single enum + validation
2. **Reincarnation penalty** (Champions 2) — bounded math
3. **Projectile speed normalization** (Combat 1) — bounded fix
4. **Level count 24 vs 14** (Dungeon 1, Mechanics 1) — bounded expansion
5. **Grey Lord combat** (Combat 2) — add new creature entry
6. **ZOKATHRA spell** (Mechanics 3) — bounded tweak

Lower-priority OMFATTANDE gaps:
- Champion Transfer/Import (data format)
- DECOMPDU.C compressed dungeons
- Code-to-Assembly conversion (rewrite path)

---

## Cross-references

- `docs/source-lock/csb_champions.md` — NEOPHYTE rank + reincarnation
- `docs/source-lock/csb_combat.md` — projectile + Grey Lord
- `docs/source-lock/csb_dungeon.md` — header + sensors
- `docs/source-lock/csb_mechanics.md` — 24 vs 14 + ZOKATHRA
- `docs/source-lock/csb_graphics.md` — VBL + BUG0_04

The 94 CSB tests cover viewport gates (D0L2/D0R2/D1L/D1R/D1C/D2L2/D2R2/D3L2/D3R2), save/load, monster generator, runtime handoff, projectile.  Zero tests cover the 27 gaps above — they all need new tests.

---

## DM1 V1 carry-over

From `docs/FINAL_GAPS.md` (this session):
- BUG-106 (Flee), BUG-108 (Light table), BUG-109 (Stat gain),
  BUG-111 (Sub-cell hit mask), BUG-116 (Runtime dynamics):
  all OPEN-BOUNDED for DM1 V1.
- 2 panel-render bleed tests in
  test_m11_inventory_full_panel_runtime still fail (C025 chest
  panel red-transparency).

CSB V1 work should be picked up after the DM1 V1 panel-render
bleed is fixed.
