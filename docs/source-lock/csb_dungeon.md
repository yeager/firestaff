# CSB V1 — Dungeon Differences (Source-Lock Audit)

**Audit date:** 2026-05-25
**Sources:** ReDMCSB DEFS.H:1202–1283,3830–3848,9295 · CSBWin Timer.cpp:2325 · DUNGEON.C · DECOMPDU.C · MOVESENS.C · TIMELINE.C · BugsAndChanges.htm · M13_PLAN.md:294–355

---

## Finding: CSB Does NOT Extend Dungeon Grid Size

Both DM1 and CSB use the same 16×16-square-per-level grid structure.
No evidence of larger maps or additional squares per level in CSB.

Source: csb_dungeon.md existing audit — confirmed against DEFS.H dungeon constants.
No contradicting evidence found in ReDMCSB or CSBWin source.

---

## Part I: New Dungeon Features in CSB (Not Present in DM1)

### 1. End Game Sensor — `C018_SENSOR_WALL_END_GAME` (NEW)

| Property | Value |
|----------|-------|
| Type | Wall/floor sensor type 18 |
| Source | DEFS.H:1283 |
| Handler | `F0248_TIMELINE_ProcessEvent6_Square_Wall` |
| Trigger result | `F0666_endgame()` called |
| Timer behavior | `Value` field = delay in seconds (not ticks) |

DEFS.H:1202 comment: *"Value is a delay in seconds for end game sensor."*
CSBWin Timer.cpp:2325: `return true;  // Tell folks this is the end of the game`

**UI zones for end game:** DEFS.H:412–438 defines `C412_ZONE_ENDGAME_CHAMPION_MIRROR_0` through
`C438_ZONE_ENDGAME_QUIT`, including champion mirror zones (412–415), champion portrait zones
(416–419), restart zone (437), and quit zone (438).

**DM1:** No end game sensor. DM1 ends via death of all champions or special in-dungeon events.
CSB adds a dedicated sensor-driven game-end trigger.

Source: DEFS.H:1283 · Timer.cpp:2325 · DEFS.H:3827–3848

### 2. Version Checker Sensor (NEW)

CSB adds a floor sensor type that triggers only if `data value <= game engine version`.
This allows dungeon designers to gate content behind engine version requirements.

CHANGE7_23_IMPROVEMENT in MOVESENS.C: version-check logic.
CHANGE8_06: Engine version 21 hardcoded for CSB version 2.1.

**DM1:** No version checker sensor. CSB adds a dungeon data version gate.

Source: MOVESENS.C (CHANGE7_23) · BugsAndChanges.htm

### 3. Compressed Dungeon Support (NEW file format feature)

CSB introduces on-the-fly dungeon data decompression.

| File | Role |
|------|------|
| `DECOMPDU.C` | Decompression logic |
| `MEMORY.C` | Temporary allocation for decompression buffer |
| `READWRIT.C` | Read path for compressed dungeon data |
| `LOADSAVE.C` | Load path for compressed dungeons |

DM1 did not support compressed dungeons. The decompression buffer is a new
temporary memory allocation type not present in DM1.

Source: DECOMPDU.C, MEMORY.C, READWRIT.C, LOADSAVE.C · existing csb_dungeon.md audit

---

## Part II: Dungeon Level Count

### DM1 PC 3.4: 14 Levels (Standard)
Source: DUNGEON.DAT header, `NumLevel()` field.

### CSB: 24 Levels (vs DM1's 14)
Source: M13_PLAN.md:303 — *"CSB DUNGEON.DAT has a different header: 24 levels (vs 14)"*

No direct source citation found in ReDMCSB for the 24-level CSB claim, but the
dungeon header structure differences are confirmed by save-game format divergence
(CEDTINC8.C:101–118: `CSBGAME.DAT` vs `DMSAVE.DAT` routing).

Source: M13_PLAN.md:303 (informed by ReDMCSB header parsing) · CEDTINC8.C:101–118

---

## Part III: Dungeon Teleporter/Projectile Behavior Change

### Projectile Speed Normalization — `PROJEXPL.C` (CHANGE7_20_IMPROVEMENT)

| | DM1 | CSB |
|--|-----|-----|
| Party-map projectile speed | Full speed | Full speed |
| Non-party-map projectile speed | Slower | Full speed (fixed) |

DM1 bug: Projectiles moved slower on maps other than the party map.
CSB fix: Projectiles now move at the same speed on all maps.

Source: PROJEXPL.C (CHANGE7_20) · csb_combat.md audit

---

## Part IV: Dungeon Structural Elements — Unchanged

| Element | Status |
|---------|--------|
| 16×16 square grid per level | Unchanged |
| Floor/ceiling bitmap size | 7840 bytes (`FLOOR_BITMAP_SIZE`, CSB.h:33) — same |
| Map connection mechanism | Teleporter/pit/stairs — same structure |
| Square aspect flags | Same bit layout |
| Door types | Same 8 door types |
| Sensor types (0–17) | Same 18 types; only sensor 18 (END_GAME) is new |
| Floor decoration descriptors | `floorDecorDesc[3][9]` — same structure |

---

## Detailed Mechanics — Phase 4

The following areas are covered in full detail in:
**`csb_v1_phase4_mechanics_parity_H2239.md`**

- `F0248_TIMELINE_ProcessEvent6_Square_Wall` — actuator pipeline (all sensor types)
- `F0249_TIMELINE_MoveTeleporterOrPitSquareThings` — CHANGE7_22_FIX group processing order
- Door defense point values (wooden=42, iron=230, Ra=255)
- Endgame flow: `F0666_endgame()`, `G0302_B_GameWon`, `F0444_STARTEND_Endgame()`
- Bug fixes: CHANGE7_17_FIX (sensor squares in discard), CHANGE7_18_FIX (bit 15), CHANGE7_19_FIX (Lord Chaos teleport)

## Summary

| Dungeon Feature | DM1 | CSB Delta |
|-----------------|-----|-----------|
| Grid size | 16×16/sq per level | Unchanged |
| Level count | 14 (PC 3.4) | 24 (CSB, different dungeon format) |
| End game sensor (type 18) | None | **NEW** — triggers endgame with delay |
| Version checker sensor | None | **NEW** — engine version gate |
| Compressed dungeon support | None | **NEW** — DECOMPDU.C etc. |
| Projectile speed on non-party maps | Buggy/slow | Fixed to full speed |
| Map connection types | Teleporter/pit/stairs | Unchanged |
| Floor decorations | Same structure | Unchanged |

**Verdict:** CSB extends the dungeon format significantly:
1. More levels (24 vs 14) in a different header format
2. Two new sensor types (END_GAME, VERSION_CHECK)
3. New compressed-dungeon file format support
4. Projectile speed normalization bug fix

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| ReDMCSB DEFS.H | 1202,1265,1283,3827–3848 | Sensor Value = seconds for end game; C009_VERSION_CHECKER; C018_END_GAME; UI zones |
| ReDMCSB DUNGEON.C | 561–565 | Door defense point values (42/230/255 HP) |
| ReDMCSB DUNGEON.C | 1996–2001 | CHANGE7_17_FIX: sensor squares in discard |
| ReDMCSB TIMELINE.C | 1136–1350 | F0248 (actuator pipeline), F0249 (teleporter/pit group fix) |
| ReDMCSB TIMELINE.C | 1319–1340 | C018_END_GAME trigger + CHANGE8_02 delay |
| ReDMCSB MOVESENS.C | 1716–1750 | C009_VERSION_CHECKER + CHANGE8_06 |
| ReDMCSB ENDGAME.C | 101–327,984–1002 | F0444_STARTEND_Endgame(), F0666_endgame() |
| ReDMCSB GROUP.C | CHANGE7_19 | Lord Chaos teleporter direction fix |
| ReDMCSB PROJEXPL.C | CHANGE7_20 | Projectile speed normalization |
| ReDMCSB CEDTINC8.C | 101–118 | CSBGAME.DAT vs DMSAVE.DAT routing |
| CSBWin Timer.cpp | 2325 | Endgame timer check |
| CSBWin DECOMPDU.C | — | Compressed dungeon decompression |
| M13_PLAN.md | 303 | 24 levels in CSB vs 14 in DM1 |
| BugsAndChanges.htm | CHANGE7_17,18,19,20,21,22,23, CHANGE8_02,06 | All dungeon-related changes |