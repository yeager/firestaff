# CSB V1 — GAP: Dungeon Implementation Gaps

**File:** 
**Audit:** Firestaff CSB V1 Audit Runner
**Date:** 2026-05-25
**Reference:** 

---

## Executive Summary

CSB dungeon is DM1 superset with 4 significant deltas:
- **10 more levels** (24 vs 14) in a new header format
- **End Game Sensor** (type 18) — new trigger mechanism
- **Version Checker Sensor** — new dungeon data gate
- **Compressed dungeon support** — new file format (DECOMPDU.C)

16x16 grid unchanged. Projectile speed normalization covered in combat gap doc.

---

## GAP 1: Dungeon Header and Level Count

**What source-lock says:**
- DM1 PC 3.4: NumLevel() field = 14
- CSB: Different header format, NumLevel() = 24
- CSBGAME.DAT uses CEDTINC8.C routing (vs DMSAVE.DAT for DM1)

**Implementation gap:**
Firestaff DM1 implementation currently models 14 levels.
CSB requires:
1. Dungeon data loader handling CSB-format header with 24 levels
2. Level count read from header, not hardcoded
3. DUNGEON.DAT loading must distinguish CSB vs DM1 format

**Specific need:**  — variant parameter
determines header layout interpretation. No current Firestaff code does this.

**Source:** M13_PLAN.md:303 · CEDTINC8.C:101–118 · csb_dungeon.md Part II

---

## GAP 2: End Game Sensor Type 18

**What source-lock says:**
- DEFS.H:1283 — C018_SENSOR_WALL_END_GAME = sensor type 18
- DEFS.H:1202 — Value field = delay in seconds (not ticks)
- DEFS.H:3827–3848 — UI zones C412–C438 for champion mirror/restart/quit
- Timer.cpp:2325 — return true; // Tell folks this is the end of the game
- F0666_endgame() called on trigger

**Implementation gap:**
No SENSOR_TYPE_18 handling in current Firestaff dungeon code.
Need:
1. SensorType::EndGame enum value (=18)
2. SquareWallEventHandler for sensor 18 calling game.endGame(delaySeconds)
3. Endgame UI zones (C412–C438): champion mirror, portrait, restart, quit regions
4. F0666_endgame() — behavior: likely all-champions-exit victorious

**Source:** DEFS.H:1283,1202,3827–3848,9295 · Timer.cpp:2325

---

## GAP 3: Version Checker Sensor

**What source-lock says:**
- MOVESENS.C CHANGE7_23: version-check logic
- CHANGE8_06: Engine version 21 hardcoded for CSB version 2.1
- Sensor triggers only if data_value <= game_engine_version

**Implementation gap:**
No version-gated sensor in current Firestaff code.
Need:
1. Sensor type reading minVersion field
2. Handler comparing DungeonData::engineVersion vs sensor minVersion
3. Content gate behind engine_version >= required check

**Source:** MOVESENS.C (CHANGE7_23) · BugsAndChanges.htm

---

## GAP 4: Compressed Dungeon Support (DECOMPDU.C)

**What source-lock says:**
- CSBWin: DECOMPDU.C, MEMORY.C (temp allocation), READWRIT.C, LOADSAVE.C
- New decompression buffer allocation type not in DM1
- Compressed dungeons decompressed on-the-fly during load

**Implementation gap:**
Firestaff reads uncompressed dungeon data. Need:
1. Detect compressed dungeon (magic bytes or header flag)
2. DECOMPDU.C equivalent — decompression algorithm (LZ or similar)
3. Temporary buffer allocation for decompression working memory
4. Read path: compressed data -> decompress -> parse

**Major new file-format feature.** No current implementation path exists.

**Source:** DECOMPDU.C · MEMORY.C · READWRIT.C · LOADSAVE.C

---

## GAP 5: Projectile Speed Normalization (Dungeon Context)

**What source-lock says (cross-ref from csb_combat.md):**
- PROJEXPL.C CHANGE7_20: DM1 bug — projectiles slower on non-party maps
- CSB fix: Full speed on all maps

**Implementation gap:**
Firestaff projectile system needs per-map speed normalization:
- Track currentMapId vs projectile.targetMapId
- If different: apply normalized full speed (not DM1 buggy slower rate)
- Sub-component of combat gap but affects dungeon traversal

**Source:** PROJEXPL.C (CHANGE7_20) · csb_combat.md Part III

---

## GAP 6: Teleporter Connection and Grey Lord

**What source-lock says:**
- 16x16 grid unchanged, teleporter/pit/stairs unchanged
- BUG0_69: GROUP.C CHANGE7_19 fixed teleporter for Lord Chaos maps
- BUG0_09, BUG0_10: DUNGEON.C fixes for square event processing

**Implementation gap:**
CSB teleporter fixes:
1. Group map allowed checks — Lord Chaos allowed on maps where teleporter leads
2. Square event for group spawns on teleporter arrival
3. Verify CanCreatureUseTeleporter covers Grey Lord (0x1a)

**Source:** GROUP.C (CHANGE7_19) · DUNGEON.C (CHANGE7_17/18)

---

## Summary Table

| Gap | Severity | Description |
|-----|----------|-------------|
| Dungeon header/24 levels | HIGH | New format, 24 vs 14; variant-aware loader needed |
| End Game Sensor type 18 | HIGH | New sensor calling F0666_endgame(); needs UI zones |
| Version Checker Sensor | MEDIUM | Engine version gate; new sensor type/handler |
| Compressed dungeon support | HIGH | DECOMPDU.C; new file format, decompression needed |
| Projectile speed normalization | MEDIUM | Sub-system of combat; per-map speed fix |
| Teleporter/Grey Lord access | MEDIUM | BUG0_69 ensures Grey Lord can use teleporters |

---

## Reference Sources

| Source | Content |
|--------|---------|
| docs/source-lock/csb_dungeon.md | Existing source-lock audit (primary) |
| ReDMCSB DEFS.H:1202,1283,3827-3848,9295 | End game sensor, zones, F0666 |
| CSBWin DECOMPDU.C/MEMORY.C/READWRIT.C/LOADSAVE.C | Compressed dungeon |
| CSBWin MOVESENS.C (CHANGE7_23) | Version checker |
| ReDMCSB PROJEXPL.C (CHANGE7_20) | Projectile speed |
| ReDMCSB GROUP.C (CHANGE7_19) | Teleporter fix |
| M13_PLAN.md:303 | 24 levels vs 14 |
| CEDTINC8.C:101-118 | CSBGAME.DAT vs DMSAVE.DAT |
