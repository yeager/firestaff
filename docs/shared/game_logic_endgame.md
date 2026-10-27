# DM1 V1 — End-Game / Victory Conditions

## Source
ReDMCSB: `ENDGAME.C` (F0444, F0446, F0666), `PROJEXPL.C` (F0222, F0225),
`CHAMPION.C:771–774`

---

## 1. Victory Path — The Firestaff Quest

**DM1 does NOT have a "reach the Torah" ending.** The win condition:

**Assemble Firestaff + Gem of Ages → Fuse Lord Chaos → Grey Lord → Credits**

### Steps to Win:
1. Find the **Firestaff** (C027_ICON_WEAPON_THE_FIRESTAFF) in the dungeon
2. Find the **Gem of Ages** (C120_ICON_JUNK_GEM_OF_AGES) in the dungeon
3. Equip Firestaff in action hand + Gem in second hand → combines to
   **Firestaff Complete** (C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE)
4. Stand adjacent to **Lord Chaos** (C23_CREATURE_LORD_CHAOS)
5. Have exactly **4 Fluxcages** (C050_EXPLOSION_FLUXCAGE) on the 4 squares
   surrounding Lord Chaos (one per cardinal direction)
6. Cast **ZOKATHRA** (Zo Kath Ra) — or use Fuse action with Firestaff Complete
7. `F0446_STARTEND_FuseSequence` plays → `G0302_B_GameWon = C1_TRUE`
8. `F0444_STARTEND_Endgame` plays credits → game won

---

## 2. Firestaff Assembly & Skill Bonus

### Assembly — `CHAMPION.C:771–774`:
```
if (actionHandIcon == C027 && secondItemIcon == C120) → C028
if (actionHandIcon == C120 && secondItemIcon == C027) → C028
```

### Skill Bonus — `CHAMPION.C:780–782` (in F0303_GetSkillLevel):
- **C027 Firestaff**: **+1** to ALL skill levels (action hand)
- **C028 Firestaff Complete**: **+2** to ALL skill levels (action hand)

On S10EA/S11E: Firestaff bonus NOT cumulative with Pendant/Ekkhard/Gem/Moonstone.
On S12E+: Firestaff bonus IS cumulative (CHANGE3_09_FIX).

---

## 3. Lord Chaos — Fuse Action

### Properties
- Type: C23_CREATURE_LORD_CHAOS
- Health: set to 10000 at fuse start (`ENDGAME.C:833`)
- Size: SINGLE centered (C0xFF_SINGLE_CENTERED_CREATURE)
- Direction: opposite of party direction

### Fuse Action Evaluation — `PROJEXPL.C F0225_GROUP_FuseAction`
1. Bounds check (party position vs map boundaries)
2. Create harm-non-material explosion at target square
3. Check if Lord Chaos is on target square
4. Count fluxcages on 4 cardinal neighbors (west/east/north/south)
5. If count < 4: try to move Lord Chaos to adjacent escape square
6. If no escape possible (cornered with < 4 fluxcages):
   → trigger `F0446_STARTEND_FuseSequence`

### Fluxcage Count — `PROJEXPL.C F0222_GROUP_IsLordChaosOnSquare`
- Checks `Group->Type == C23_CREATURE_LORD_CHAOS`
- Fluxcages on neighbors: explosion type C050_EXPLOSION_FLUXCAGE

---

## 4. FuseSequence Animation

**Source: ENDGAME.C:820–965** — `F0446_STARTEND_FuseSequence`

```
GameWon = TRUE
Remove fluxcages from party square (loop)
Remove fluxcages from Lord Chaos square (loop)
Delete all creature groups on map (except Lord Chaos)
Fireball explosions: power 55, 95, 135, 175, 215, 255
  (with FuseSequenceUpdate between each)
Play SOUND_BUZZ
Lord Chaos → Lord Order (C23 → C25)
Harm-Non-Material explosions: power 55, 95, 135, 175, 215, 255
Toggle Lord Order/Lord Chaos: 4 cycles × 5 iterations
Final Fireball(255) + Harm-Non-Material(255)
Lord Order → Grey Lord (C26)
Draw win messages from map[0,0] in alphabetical order
Delay 780 ticks between messages
Play music C2_MUSIC_GAME_WON
F0444_STARTEND_Endgame(C1_TRUE) — credits screen
```

**BUG0_59**: Failure to remove fluxcages from Lord Chaos's own square causes
an infinite loop (explosion thing links to itself in the thing list).

---

## 5. Credits Sequence

**Source: ENDGAME.C F0444_STARTEND_Endgame (lines 94–760)**

- Champion names + skill level titles displayed
- Titles by highest skill (FIGHTER/NINJA/PRIEST/WIZARD)
- "THE END" + Firestaff graphic
- Win messages from dungeon (text strings at map[0,0])
- Win messages printed in alphabetical order by first letter

---

## 6. Loss Conditions (Game Over)

### When the game is lost:
1. **All 4 champions die** — `G0305_ui_PartyChampionCount == 0`
2. **BUG0_02**: `G0313_ul_GameTime` exceeds 2^24-1 (~850–1000 hours)
3. **SYSTEM ERROR 60**: `G0374_l_WatchdogTime_CPSE < G0313_ul_GameTime`

### Death and Resurrection
- Champions resurrect at **Vi Altar** squares (VI spell area effects)
- Event C13_EVENT_VI_ALTAR_REBIRTH schedules revival
- Spells Vi Bro (Cure Poison) and Vi (Health Potion) can also revive

---

## 7. Firestaff Implementation

**File:** `src/dm1/dm1_v1_endgame_system_pc34_compat.c`
- `DM1_Endgame_CanAssembleFirestaff()` — C027+C120 combination check
- `DM1_Endgame_GetAssembledFirestaffIcon()` — returns C028 if assembled
- `DM1_Endgame_GetFirestaffSkillBonus()` — +1 (C027) or +2 (C028)
- `DM1_Endgame_CountFluxcagesAroundSquare()` — 4 cardinal neighbors
- `DM1_Endgame_IsLordChaosOnSquare()` — C23 type check
- `DM1_Endgame_EvaluateFuseAction()` — full PROJEXPL.C F0225 evaluation

**File:** `src/dm1/dm1_v1_game_over_pc34_compat.c`
- `DM1_GameOver_GetLossReason()` — determine loss cause

**File:** `src/frontend/endgame_frontend_pc34_compat.c`
- UI rendering for endgame sequence

**Parity status:** FULL for Firestaff assembly, skill bonus, fluxcage detection,
Lord Chaos identification. FuseSequence evaluation complete; visual animation
sequence not yet fully wired to engine.
