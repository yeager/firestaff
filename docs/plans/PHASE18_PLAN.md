# PHASE 18 PLAN — Champion Lifecycle System (v1)

Firestaff M10 milestone, Phase 18.  Assumed starting state: 17 phases
PASS (Phase 17 projectile flight merged before Phase 18 executes).
This document is the *single source of truth* the executor follows.
Any deviation = abort and ask.

Style rules (non-negotiable, inherited from Phases 10–17):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation.  Every new struct round-trips
  bit-identical.
- Pure functions: NO globals, NO UI, NO IO.
  ```
  F08xx(state_in, tick_context) -> (state_out, emitted_events[])
  ```
  Randomness flows through Phase 13's explicit `RngState_Compat*`
  where needed (level-up stat bonuses use RNG).
- Function numbering: **F0830–F0859** (30 slots, Phase 18 range).
- Probe emits `champion_lifecycle_probe.md` +
  `champion_lifecycle_invariants.md` with trailing `Status: PASS`.
- Verify gate: `run_firestaff_m10_verify.sh` gets **exactly one** new
  `# Phase 18:` block appended.  Pre-grep and post-grep mandatory.
- ADDITIVE ONLY: zero edits to Phase 9-17 source.  Phase 18 consumes
  their interfaces via `#include` and pure composition.

Fontanel primary references:

- `CHAMPION.C:F0331_CHAMPION_ApplyTimeEffects_CPSF` (lines 2254-2503)
  — THE core per-tick lifecycle: food/water decay, stamina/health/mana
  regen, temporary-XP decay, statistic drift.
- `CHAMPION.C:F0310_CHAMPION_GetMovementTicks` (lines 1180-1215)
  — movement cooldown calculation from load, wounds, boots.
- `CHAMPION.C:F0303_CHAMPION_GetSkillLevel` (lines 715-820)
  — experience → skill level: `while (exp >= 500) { exp >>= 1; level++ }`
- `CHAMPION.C:F0304_CHAMPION_AddSkillExperience` (lines 823-1060)
  — XP award, map difficulty multiplier, level-up stat bonuses.
- `CHAMPION.C:F0322_CHAMPION_Poison` (lines 1926-1964)
  — poison self-reschedule: every 36 ticks, attack >> 6 damage,
  attack--, reschedule while attack > 0.
- `CHAMPION.C:F0325_CHAMPION_DecrementStamina` (lines 2025-2050)
  — stamina decrement with overflow → health damage.
- `TIMELINE.C:C70-C83 event dispatch` (lines 1948-2020)
  — status-effect expiry handlers (light, invisibility, shields,
  poison, footprints, spellshield, fireshield, magic map).
- `DEFS.H:C0_SKILL_FIGHTER..C19_SKILL_WATER` — 20 skill indices.
- `DEFS.H:C0_STATISTIC_LUCK..C6_STATISTIC_ANTIFIRE` — 7 statistics,
  each with `[C0_MAXIMUM][C1_CURRENT][C2_MINIMUM]` as `unsigned char[3]`.

---

## §1  Scope definition

### In scope (v1)

1. **HUNGER_THIRST handler** — port of F0331's food/water decay loop.
   Decrement `Food` and `Water` fields per champion per tick, with
   rate dependent on stamina gain cycles.  When Food < -512 or
   Water < -512, stop stamina gain and increase stamina drain.
   Floor at -1024 for both.  Scheduled at fixed cadence via
   TIMELINE_EVENT_HUNGER_THIRST.

2. **Stamina regeneration** — part of F0331.  Multi-cycle loop:
   base stamina amount = bounded(1, MaxStamina >> 8 - 1, 6).
   4 gain cycles if stamina > MaxStamina/2, up to 4 + 2*(halvings)
   cycles.  Food/water >= 0 → gain stamina; Food/water < -512 →
   lose stamina.  Rest mode doubles the stamina amount.

3. **Health regeneration** — part of F0331.  Requires stamina >=
   MaxStamina/4 AND `timeCriteria < vitality_current + 12`.
   Gain = MaxHealth >> 7 + 1 (doubled if resting).  Ekkhard Cross
   bonus.

4. **Mana regeneration** — part of F0331.  Requires `timeCriteria <
   wisdom_current + wizardLevel + priestLevel`.  Gain =
   MaxMana / 40 (doubled if resting), +1.  Costs stamina:
   gain × max(7, 16 - wizardPriestLevel).

5. **Statistic drift** — part of F0331.  Every 256 ticks (or 64 if
   resting): each stat current drifts toward maximum by 1.  If
   current > maximum: decrease by current/maximum.

6. **Temporary XP decay** — part of F0331.  Each tick, each skill's
   TemporaryExperience decrements by 1 if positive.

7. **STATUS_TIMEOUT handler** — port of TIMELINE.C C70-C83 event
   dispatch.  On expiry event:
   - C71 INVISIBILITY: decrement party `Event71Count`.
   - C72 CHAMPION_SHIELD: subtract event Defense from champion
     ShieldDefense.
   - C73 THIEVES_EYE: decrement party `Event73Count`.
   - C74 PARTY_SHIELD: subtract event Defense from party ShieldDefense.
   - C77 SPELLSHIELD: subtract event Defense from party SpellShieldDefense.
   - C78 FIRESHIELD: subtract event Defense from party FireShieldDefense.
   - C75 POISON: apply damage (attack >> 6), decrement attack,
     reschedule if attack > 0 (every 36 ticks).
   - C79 FOOTPRINTS: decrement party `Event79Count`.
   - C70 LIGHT: handled by existing light system (emit marker only
     in v1).
   - C80-C83 MAGIC_MAP: decrement per-champion magic map counts (CSB
     only — emit marker, v1 stub).

8. **MOVE_TIMER handler** — port of F0310.  Calculate movement tick
   count from: load vs maxLoad, wound status (feet), boots of speed.
   Set per-champion `nextAllowedTick`.  Movement requests before
   that tick are rejected.

9. **Rest mechanic** — party-level boolean `isResting`.  While
   resting: regen rates doubled (stamina amount <<= 1; health gain
   <<= 1; mana gain <<= 1; stat drift every 64 ticks instead of
   256).  Rest interrupt conditions: monster within adjacent cells
   (Phase 16 danger detection), projectile inbound (Phase 17 state),
   champion killed (health ≤ 0).

10. **XP award** — port of F0304.  On combat/spell events:
    - Physical skill XP: awarded to attacker champion for that skill.
    - Map difficulty multiplier: `experience *= map.Difficulty` if > 0.
    - Recent combat bonus: if `lastCreatureAttackTime > gameTime - 25`
      and skill is C04-C11, double XP.
    - Staleness penalty: if `lastCreatureAttackTime < gameTime - 150`
      and skill is C04-C11, halve XP.
    - Both hidden skill AND base skill get XP (hidden skills C04-C19
      contribute XP to their base skill C00-C03).
    - TemporaryExperience += bounded(1, experience >> 3, 100),
      capped at 32000.

11. **Level-up** — part of F0304.  When base skill level increases
    (checked via F0303 before/after), apply stat bonuses per class:
    - **Fighter**: Strength += 1+rand(2), Dexterity += rand(2),
      MaxStamina += MaxStamina>>4, MaxHealth += level + rand(level/2+1)
    - **Ninja**: Strength += rand(2), Dexterity += 1+rand(2),
      MaxStamina += MaxStamina/21, MaxHealth += level + rand(level/2+1)
    - **Wizard**: MaxMana += level + level/2 + min(rand(4), baseLevel-1),
      Wisdom += 1+rand(2), MaxStamina += MaxStamina>>5,
      MaxHealth += level + rand(level/2+1)
    - **Priest**: MaxMana += level + min(rand(4), baseLevel-1),
      Wisdom += rand(2), Vitality += rand(2),
      MaxStamina += MaxStamina/25, MaxHealth += level + rand(level/2+1)
    - All classes: Vitality += rand(2) [Priest always; others only
      if odd level], Antifire += rand(2) [only even levels],
      Antimagic += rand(3 or 4) [Wizard/Priest only].
    - MaxHealth capped at 999, MaxStamina at 9999, MaxMana at 900.
    - Emit LEVEL_UP marker with championIndex + baseSkillIndex.

### Out of scope (v1)

- Food/water item *spawning* — dungeon provides static placements.
- UI for "champion is hungry" warnings — emit markers only.
- Skill bonuses applied *during* combat rolls — Phase 13 handles.
- Starvation death animations — emit marker; Phase 13 handles death.
- Class-specific leveling perks beyond stat bonuses (priest prayers,
  ninja abilities) — Phase 19+.
- Resurrect-from-bones-altar — separate feature.
- Poison cloud interaction — Phase 14/17 concern.
- Object modifiers on skill level (Firestaff, pendants) — F0303 is
  Phase 10/13 concern; Phase 18 uses `IGNORE_OBJECT_MODIFIERS` flag.
- Scent trail management — part of F0331 but is movement concern,
  not champion lifecycle.
- Copy protection watchdog (C53_EVENT_WATCHDOG) — not ported.

### Explicit deferrals with markers

- `NEEDS DISASSEMBLY REVIEW`: XP split for multi-champion kills
  (v1 = killer takes all).
- `NEEDS DISASSEMBLY REVIEW`: Status-stack semantics — Fontanel uses
  additive counters (e.g. two fireshields ADD defense values), NOT
  magnitude-based stacking.  v1 mirrors this.
- `NEEDS DISASSEMBLY REVIEW`: Exact `timeCriteria` formula from F0331
  uses gameTime bit manipulation: `((GT & 0x80) + ((GT & 0x100) >> 2)
  + ((GT & 0x40) << 2)) >> 2`.  Port verbatim but mark for review.

---

## §2  Data structures

### 2.1  `HungerThirstTick_Compat` (per-champion snapshot for one tick)

Not a persistent struct — this is the input/output of the hunger/thirst
handler.  The persistent state lives in `ChampionState_Compat.food` and
`.water` (which must be changed from `unsigned char` to `int16_t` — see
§2.7 CRITICAL DRIFT).

```
// Conceptual — NOT C code
HungerThirstTick_Compat {
    int16_t  food;            // offset 0, 2 bytes — signed, range [-1024..+max]
    int16_t  water;           // offset 2, 2 bytes — signed, range [-1024..+max]
    int16_t  staminaLoss;     // offset 4, 2 bytes — net stamina change (negative = gain)
    uint8_t  padding[2];      // offset 6, 2 bytes
}
// Total: 8 bytes
// Serialised size: 8 bytes (4 × int16_t LE)
```

### 2.2  `StatusEffectState_Compat` (party-level status counters)

Mirrors Fontanel's PARTY struct fields.  Not per-champion per-status
arrays — Fontanel tracks status effects as *additive counters* on the
party plus per-champion `PoisonEventCount` and `ShieldDefense`.

```
StatusEffectState_Compat {
    int16_t  partyShieldDefense;       // offset  0, 2 bytes
    int16_t  partySpellShieldDefense;  // offset  2, 2 bytes
    int16_t  partyFireShieldDefense;   // offset  4, 2 bytes
    uint16_t invisibilityCount;        // offset  6, 2 bytes
    uint16_t thievesEyeCount;          // offset  8, 2 bytes
    uint16_t footprintsCount;          // offset 10, 2 bytes
    uint16_t lightEventCount;          // offset 12, 2 bytes  (stub for v1)
    uint8_t  padding[2];               // offset 14, 2 bytes
}
// Total: 16 bytes
// Serialised size: 16 bytes
```

Per-champion additions to `ChampionState_Compat`:
- `int16_t shieldDefense;` (C72_EVENT_CHAMPION_SHIELD)
- `uint8_t poisonEventCount;` (count of active poison events)

### 2.3  `MoveTimerState_Compat` (per-champion)

```
MoveTimerState_Compat {
    uint32_t lastMoveTick;       // offset 0, 4 bytes
    uint32_t nextAllowedTick;    // offset 4, 4 bytes
    uint16_t cachedMoveTicks;    // offset 8, 2 bytes — last computed F0310 result
    uint8_t  padding[2];         // offset 10, 2 bytes
}
// Total: 12 bytes
// Serialised size: 12 bytes (3 × uint32 LE)
```

### 2.4  `SkillState_Compat` (per-champion, 20 skills)

Fontanel SKILL struct per-champion has: `TemporaryExperience` (int16)
and `Experience` (long = int32).  20 skills.

```
SkillState_Compat {
    int32_t  experience;            // offset 0, 4 bytes  (long in Fontanel)
    int16_t  temporaryExperience;   // offset 4, 2 bytes
    uint8_t  padding[2];            // offset 6, 2 bytes
}
// Per-skill: 8 bytes.  20 skills × 8 = 160 bytes per champion.
// Serialised size: 160 bytes per champion
```

**NOTE**: Phase 10's `ChampionState_Compat` has `skillLevels[4]` and
`skillExperience[4]` — only 4 base skills.  Fontanel has 20 skills
(4 base + 16 hidden).  Phase 18 must add the full 20-skill array.
This is additive: the existing 4-skill fields remain for backward
compat; Phase 18 adds `SkillState_Compat skills20[20]` to the
lifecycle state.

### 2.5  `RestState_Compat` (party-level)

```
RestState_Compat {
    uint8_t  isResting;          // offset 0, 1 byte  (boolean)
    uint8_t  interruptReason;    // offset 1, 1 byte  (0=none, 1=monster, 2=projectile, 3=death)
    uint8_t  padding[2];         // offset 2, 2 bytes
    uint32_t restStartTick;      // offset 4, 4 bytes
    uint32_t lastMovementTime;   // offset 8, 4 bytes  (G0362_l_LastPartyMovementTime)
}
// Total: 12 bytes
// Serialised size: 12 bytes (3 × uint32 LE)
```

### 2.6  `ChampionLifecycleState_Compat` (aggregate per-champion)

Aggregates all Phase 18 per-champion state that does NOT already
exist in `ChampionState_Compat` from Phase 10:

```
ChampionLifecycleState_Compat {
    SkillState_Compat    skills20[20];        // offset   0, 160 bytes
    MoveTimerState_Compat moveTimer;          // offset 160,  12 bytes
    int16_t              shieldDefense;       // offset 172,   2 bytes
    uint8_t              poisonEventCount;    // offset 174,   1 byte
    uint8_t              padding;             // offset 175,   1 byte
    int16_t              food;                // offset 176,   2 bytes (int16, replaces Phase 10 uint8)
    int16_t              water;               // offset 178,   2 bytes (int16, replaces Phase 10 uint8)
    uint8_t              statistics[7][3];    // offset 180,  21 bytes (Fontanel 7×3 unsigned char)
    uint8_t              statPadding;         // offset 201,   1 byte
    uint16_t             maxHealth;           // offset 202,   2 bytes (for level-up cap check)
    uint16_t             maxStamina;          // offset 204,   2 bytes
    uint16_t             maxMana;             // offset 206,   2 bytes
}
// Total: 208 bytes per champion
// Serialised size: 208 bytes per champion
```

### 2.7  `LifecycleState_Compat` (party-level aggregate — the Phase 18 root)

```
LifecycleState_Compat {
    ChampionLifecycleState_Compat champions[4]; // offset   0, 832 bytes
    StatusEffectState_Compat      status;       // offset 832,  16 bytes
    RestState_Compat              rest;         // offset 848,  12 bytes
    uint32_t                      lastCreatureAttackTime; // offset 860, 4 bytes
    uint32_t                      gameTime;     // offset 864, 4 bytes
    uint8_t                       padding[4];   // offset 868, 4 bytes
}
// Total: 872 bytes
// Serialised size: 872 bytes
```

### 2.8  Byte-layout budget

| Struct | Size (bytes) | Count | Total |
|--------|-------------|-------|-------|
| SkillState_Compat | 8 | 20×4 | 640 |
| MoveTimerState_Compat | 12 | 4 | 48 |
| StatusEffectState_Compat | 16 | 1 | 16 |
| RestState_Compat | 12 | 1 | 12 |
| ChampionLifecycleState_Compat | 208 | 4 | 832 |
| LifecycleState_Compat | 872 | 1 | 872 |
| **Total new data budget** | | | **872 bytes** |

### 2.9  CRITICAL: Phase 10 `ChampionState_Compat` drift

Phase 10 declares `food` and `water` as `unsigned char` (0-255).
Fontanel uses `int16_t Food` and `int16_t Water` with range
[-1024, +max].  The food/water decay algorithm requires negative
values.

**Resolution**: Phase 18 does NOT modify Phase 10's struct.  Instead,
`ChampionLifecycleState_Compat` carries its own `int16_t food` and
`int16_t water` fields.  On init, Phase 10's uint8 values are sign-
extended into Phase 18's int16 fields.  Phase 18's food/water are
the authoritative values; Phase 10's remain frozen.

---

## §3  Function API (F0830–F0859)

### Hunger/thirst lifecycle (F0830-F0834)

| F-number | Name | Signature (pseudocode) | Description |
|----------|------|----------------------|-------------|
| F0830 | `F0830_LIFECYCLE_ComputeTimeCriteria_Compat` | `(gameTime: uint32) → uint16` | Port of F0331 timeCriteria: `((GT & 0x80) + ((GT & 0x100) >> 2) + ((GT & 0x40) << 2)) >> 2` |
| F0831 | `F0831_LIFECYCLE_ComputeStaminaAmount_Compat` | `(maxStamina: uint16, isResting: bool, lastMoveTime: uint32, gameTime: uint32) → int16` | Base stamina amount: bounded(1, maxStam >> 8 - 1, 6); doubled if resting; +1 if idle > 80 ticks; +1 if idle > 250 ticks |
| F0832 | `F0832_LIFECYCLE_TickHungerThirst_Compat` | `(champ: ChampionLifecycleState*, staminaAmount: int16, isResting: bool) → int16 staminaLoss` | One iteration of F0331's food/water do-while loop.  Returns net stamina loss for one gain cycle.  Modifies food/water in-place (floors at -1024). |
| F0833 | `F0833_LIFECYCLE_ApplyHungerThirstFull_Compat` | `(champ: ChampionLifecycleState*, hp/stamina/mana in, gameTime, rest, lastMoveTime) → (stamina change, food/water updated)` | Full F0331 hunger/thirst + stamina regen for one champion per tick.  Calls F0831, F0832 in loop, applies DecrementStamina logic. |
| F0834 | `F0834_LIFECYCLE_ClampFoodWater_Compat` | `(food: int16*, water: int16*)` | Clamp food to [-1024, INT16_MAX], water to [-1024, INT16_MAX] |

### Status-effect lifecycle (F0835-F0840)

| F-number | Name | Description |
|----------|------|-------------|
| F0835 | `F0835_LIFECYCLE_HandleStatusExpiry_Compat` | Dispatch: given an expired timeline event (C70-C83), apply the expiry effect to LifecycleState.  Returns 0/1 emitted events (poison reschedule). |
| F0836 | `F0836_LIFECYCLE_HandlePoisonTick_Compat` | Port of F0322: apply damage = max(1, attack >> 6), decrement attack, if > 0 return reschedule event (C75, +36 ticks). |
| F0837 | `F0837_LIFECYCLE_HandleShieldExpiry_Compat` | Handle C72 (champion shield), C74 (party shield): subtract defense from state. |
| F0838 | `F0838_LIFECYCLE_HandleMagicExpiry_Compat` | Handle C77 (spellshield), C78 (fireshield): subtract defense from party state. |
| F0839 | `F0839_LIFECYCLE_HandleCounterExpiry_Compat` | Handle C71 (invisibility), C73 (thieves eye), C79 (footprints): decrement counter. |
| F0840 | `F0840_LIFECYCLE_HandleLightExpiry_Compat` | Handle C70 (light): emit marker (v1 stub — light system is separate). |

### Move-timer calculation (F0841-F0843)

| F-number | Name | Description |
|----------|------|-------------|
| F0841 | `F0841_LIFECYCLE_ComputeMoveTicks_Compat` | Port of F0310.  Input: champion load, maxLoad, wounds, footwearIconIndex, isResting.  Output: uint16 moveTicks (2-6+). |
| F0842 | `F0842_LIFECYCLE_UpdateMoveTimer_Compat` | Given current tick + computed moveTicks, set `nextAllowedTick = currentTick + moveTicks × TICKS_PER_STEP`.  TICKS_PER_STEP = 1 for v1 (mirror of Fontanel where move events fire at calculated intervals). |
| F0843 | `F0843_LIFECYCLE_CanChampionMove_Compat` | `(moveTimer, currentTick) → bool`.  True iff `currentTick >= nextAllowedTick`. |

### Rest regeneration (F0844-F0847)

| F-number | Name | Description |
|----------|------|-------------|
| F0844 | `F0844_LIFECYCLE_ApplyHealthRegen_Compat` | Port of F0331 health regen: requires stamina >= maxStamina/4 AND timeCriteria < vitality + 12.  Gain = maxHealth >> 7 + 1; doubled if resting; +1 or +gain/2+1 for Ekkhard Cross.  Capped at maxHealth. |
| F0845 | `F0845_LIFECYCLE_ApplyManaRegen_Compat` | Port of F0331 mana regen: requires timeCriteria < wisdom + wizardLevel + priestLevel.  Gain = maxMana / 40; doubled if resting; +1.  Stamina cost = gain × max(7, 16 - wizPriestLevel).  If current > max, decrement by 1 (or by current/max for later versions).  Capped at maxMana. |
| F0846 | `F0846_LIFECYCLE_ApplyStatDrift_Compat` | Port of F0331 statistic drift: every 256 ticks (64 if resting), each stat current → maximum drift.  If current < max, +1.  If current > max, -1 (or -current/max for PC 3.4). |
| F0847 | `F0847_LIFECYCLE_ApplyTemporaryXPDecay_Compat` | For each of 20 skills: if tempExp > 0, tempExp--.  Pure, per-champion. |

### XP award & leveling (F0848-F0853)

| F-number | Name | Description |
|----------|------|-------------|
| F0848 | `F0848_LIFECYCLE_ComputeSkillLevel_Compat` | Port of F0303 (pure, no object modifiers).  `while (exp >= 500) { exp >>= 1; level++ }`.  Returns skill level (starting at 1). |
| F0849 | `F0849_LIFECYCLE_AddSkillExperience_Compat` | Port of F0304 core: multiply by map difficulty, apply staleness/freshness modifiers, add to skill + base skill, update tempExp.  Returns (levelBefore, levelAfter). |
| F0850 | `F0850_LIFECYCLE_ApplyLevelUp_Compat` | Port of F0304 stat bonuses on level-up.  Takes baseSkillIndex, newLevel, RNG state.  Modifies champion stats (strength, dex, wisdom, vitality, antifire, antimagic, maxHP, maxStamina, maxMana) per class rules.  Caps: HP 999, Stamina 9999, Mana 900.  Returns LevelUpMarker. |
| F0851 | `F0851_LIFECYCLE_AwardCombatXP_Compat` | Wrapper: on physical hit event, determine skill index from action type (swing/thrust/club/parry/steal/fight/throw/shoot), call F0849.  If level-up, call F0850. |
| F0852 | `F0852_LIFECYCLE_AwardMagicXP_Compat` | Wrapper: on spell cast event, award XP to wizard/priest/specific magic skill, call F0849.  If level-up, call F0850. |
| F0853 | `F0853_LIFECYCLE_AwardKillXP_Compat` | Wrapper: on creature kill, award bonus XP to killing champion.  `NEEDS DISASSEMBLY REVIEW`: Fontanel does NOT have a separate kill-XP function — XP is awarded per-hit via F0304.  v1: no separate kill bonus beyond the hit XP already awarded.  Mark for review. |

### Emission helpers (F0854-F0856)

| F-number | Name | Description |
|----------|------|-------------|
| F0854 | `F0854_LIFECYCLE_EmitTimelineEvent_Compat` | Build a `TimelineEvent_Compat` for scheduling: kind, fireAtTick, aux fields.  Returns filled event struct. |
| F0855 | `F0855_LIFECYCLE_ScheduleNextHungerThirst_Compat` | Build HUNGER_THIRST event: fireAtTick = currentTick + 1 (F0331 runs every game tick, not a sparse schedule — see §4.1). |
| F0856 | `F0856_LIFECYCLE_BuildLevelUpMarker_Compat` | Build marker struct: championIndex, baseSkillIndex, newLevel.  For UI/log consumers. |

### Serialise/deserialise (F0857-F0859)

| F-number | Name | Description |
|----------|------|-------------|
| F0857 | `F0857_LIFECYCLE_Serialize_Compat` | Serialise entire `LifecycleState_Compat` (872 bytes) to flat buffer, LSB-first. |
| F0858 | `F0858_LIFECYCLE_Deserialize_Compat` | Deserialise from flat buffer.  Returns bytes consumed or -1. |
| F0859 | `F0859_LIFECYCLE_Init_Compat` | Zero-init a `LifecycleState_Compat`, then populate from existing Phase 10 `PartyState_Compat` (sign-extend food/water, copy skill experience to 20-skill array, init move timers, init status counters to zero). |

---

## §4  Algorithm specifications

### 4.1  Hunger/thirst decay (F0831–F0834)

Port of F0331 lines 2360-2415 (MEDIA240 PC 3.4 variant).

**timeCriteria** (F0830):
```pseudocode
timeCriteria = ((gameTime & 0x0080) + ((gameTime & 0x0100) >> 2) +
                ((gameTime & 0x0040) << 2)) >> 2
// NEEDS DISASSEMBLY REVIEW: verify this yields pseudo-random 0-95 range
// Purpose: gate mana/health regen so it doesn't fire every tick
```

**staminaAmount** (F0831):
```pseudocode
staminaAmount = clamp(1, (maxStamina >> 8) - 1, 6)
if isResting:
    staminaAmount <<= 1    // double for rest
delay = gameTime - lastMovementTime
if delay > 80:
    staminaAmount += 1
    if delay > 250:
        staminaAmount += 1
```

**Hunger/thirst per-cycle** (F0832):
```pseudocode
// 4 gain cycles if stamina > maxStamina/2, more cycles as stamina drops
staminaGainCycleCount = 4
staminaMagnitude = maxStamina
while currentStamina < (staminaMagnitude >>= 1):
    staminaGainCycleCount += 2

staminaLoss = 0
do:
    aboveHalf = (staminaGainCycleCount <= 4)

    // FOOD
    if food < -512:
        if aboveHalf:
            staminaLoss += staminaAmount
            food -= 2
    else:
        if food >= 0:
            staminaLoss -= staminaAmount  // gain stamina
        food -= (aboveHalf ? 2 : staminaGainCycleCount >> 1)

    // WATER (same pattern, different rates)
    if water < -512:
        if aboveHalf:
            staminaLoss += staminaAmount
            water -= 1
    else:
        if water >= 0:
            staminaLoss -= staminaAmount  // gain stamina
        water -= (aboveHalf ? 1 : staminaGainCycleCount >> 2)

while (--staminaGainCycleCount) && (currentStamina - staminaLoss < maxStamina)

// Clamp
if food < -1024: food = -1024
if water < -1024: water = -1024
```

**Stamina application** (F0325 mirror):
```pseudocode
currentStamina -= staminaLoss
if currentStamina <= 0:
    damage = (-currentStamina) >> 1
    currentStamina = 0
    // apply damage to health (emit CombatAction)
else if currentStamina > maxStamina:
    currentStamina = maxStamina
```

**Fontanel refs**: CHAMPION.C:2360-2415, F0325 at 2025-2050.

### 4.2  Health regeneration (F0844)

```pseudocode
if currentHealth < maxHealth
   AND currentStamina >= (maxStamina >> 2)
   AND timeCriteria < (vitality_current + 12):
    healthGain = (maxHealth >> 7) + 1
    if isResting:
        healthGain <<= 1
    if neckSlotIcon == EKKHARD_CROSS:
        healthGain += (healthGain >> 1) + 1  // 1.5x + 1
    currentHealth = min(currentHealth + healthGain, maxHealth)
```

**Fontanel ref**: CHAMPION.C:2430-2460.

### 4.3  Mana regeneration (F0845)

```pseudocode
wizPriestLevel = getSkillLevel(WIZARD) + getSkillLevel(PRIEST)
if currentMana < maxMana
   AND timeCriteria < (wisdom_current + wizPriestLevel):
    manaGain = maxMana / 40
    if isResting:
        manaGain <<= 1
    manaGain += 1
    staminaCost = manaGain * max(7, 16 - wizPriestLevel)
    decrementStamina(staminaCost)
    currentMana = min(currentMana + manaGain, maxMana)
else if currentMana > maxMana:
    currentMana -= currentMana / maxMana  // PC 3.4: faster decay
```

**Fontanel ref**: CHAMPION.C:2335-2356.

### 4.4  Move cooldown (F0841)

```pseudocode
maxLoad = getMaxLoad(champion)  // F0309 mirror
load = champion.load

if maxLoad > load:  // not overloaded
    ticks = 2
    if (load << 3) > (maxLoad * 5):  // load > 62.5% of max
        ticks = 3
    woundTicks = 1
else:  // overloaded
    ticks = 4 + ((load - maxLoad) << 2) / maxLoad
    woundTicks = 2

if wounds & WOUND_FEET:
    ticks += woundTicks

if footwearIcon == BOOT_OF_SPEED:
    ticks -= 1

return ticks
```

**Note**: BUG0_72 — when load == maxLoad, champion is treated as
overloaded (comparison is `>` not `>=`).  We port this bug faithfully.

**Fontanel ref**: CHAMPION.C:1180-1215.

### 4.5  XP thresholds and level calculation (F0848)

```pseudocode
// Fontanel F0303, ignoring object modifiers
level = 1
exp = skill.experience + skill.temporaryExperience  // or just experience if IGNORE_TEMP
if skillIndex > SKILL_WIZARD (hidden skill):
    baseSkillIndex = (skillIndex - SKILL_SWING) >> 2
    exp += baseSkill.experience + baseSkill.temporaryExperience
    exp >>= 1  // average of hidden + base
while exp >= 500:
    exp >>= 1
    level += 1
return level
```

**Level thresholds (derived)**:
| Level | Min Experience |
|-------|---------------|
| 1 | 0 |
| 2 | 500 |
| 3 | 1000 |
| 4 | 2000 |
| 5 | 4000 |
| 6 | 8000 |
| 7 | 16000 |
| 8 | 32000 |
| 9 | 64000 |
| 10 | 128000 |
| N | 500 × 2^(N-2) |

This is exact (not 2^N × baseXP — it's 500 × 2^(level-2)).

### 4.6  Status-effect mechanics

Fontanel does NOT use magnitude-decay for shields.  Fireshield,
spellshield, champion shield, and party shield all have a fixed
Defense value set when the spell is cast.  The timeline event
carries `Event.B.Defense`.  On expiry, that exact value is subtracted
from the running total.

**Multiple shields STACK ADDITIVELY**: Two fireshields of defense 10
and 15 yield total FireShieldDefense = 25.  When the first expires,
25 - 10 = 15 remains.  This is the Fontanel behavior (counters, not
highest-wins).

**Poison** is the only self-rescheduling status.  Each poison event
carries `B.Attack`.  On tick: damage = max(1, attack >> 6),
attack -= 1, reschedule at gameTime + 36 if attack > 0.
Multiple poisonings create independent event chains tracked by
`PoisonEventCount`.

**Fontanel refs**: TIMELINE.C:1948-2020, CHAMPION.C:1926-1964.

### 4.7  Rest-interrupt conditions

```pseudocode
// Called before applying regen each tick
function checkRestInterrupt(state, dungeonState):
    if not state.rest.isResting: return NONE

    // Monster within adjacent cells
    if Phase16_DangerDetection(partyPos, dungeonState) != SAFE:
        return INTERRUPT_MONSTER

    // Projectile inbound (Phase 17 state)
    if Phase17_ProjectileOnSquare(partyPos, dungeonState):
        return INTERRUPT_PROJECTILE

    // Champion died
    for each champion in party:
        if champion.hp.current <= 0 AND champion.present:
            return INTERRUPT_DEATH

    return NONE
```

Interrupt sets `rest.isResting = false`, `rest.interruptReason`, and
wakes up (Fontanel `F0314_CHAMPION_WakeUp`).

**Fontanel ref**: CHAMPION.C:1382-1416, referenced at 1394 and 1913.

### 4.8  Regen-per-tick formulas summary

| Regeneration | Formula | Gated by | Rest modifier |
|-------------|---------|----------|---------------|
| Stamina | net of food/water cycle (see §4.1) | food/water > 0 | staminaAmount ×2 |
| Health | (maxHP >> 7) + 1 | stamina ≥ maxStam/4, timeCriteria gate | gain ×2, Ekkhard +50% |
| Mana | maxMana/40 + 1 | timeCriteria < wisdom + wizPriest | gain ×2, costs stamina |
| Stats | +1 toward max (or -current/max) | every 256 ticks (64 rest) | 4× frequency |

### 4.9  Unclear points — NEEDS DISASSEMBLY REVIEW

1. **XP split on multi-champion kill**: Fontanel awards XP per-hit
   via F0304, not per-kill.  No separate kill bonus exists.  v1
   mirrors this — XP is awarded on each hit.  No kill-bonus function.

2. **timeCriteria range**: The bit-manipulation formula yields values
   0-95 (bits 6,7,8 of gameTime).  Verify this produces the intended
   pseudo-random gating pattern.  Port verbatim.

3. **Stamina gain cycle overflow**: The do-while loop termination
   condition `currentStamina - staminaLoss < maxStamina` uses signed
   arithmetic.  Verify no underflow in extreme cases (very negative
   staminaLoss + low currentStamina).

---

## §5  Invariant list (target 40, gate ≥30)

### Block A — Struct sizes + round-trip (6 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| A1 | `sizeof(SkillState_Compat) == 8` | Size |
| A2 | `sizeof(MoveTimerState_Compat) == 12` | Size |
| A3 | `sizeof(StatusEffectState_Compat) == 16` | Size |
| A4 | `sizeof(RestState_Compat) == 12` | Size |
| A5 | `sizeof(ChampionLifecycleState_Compat) == 208` | Size |
| A6 | `sizeof(LifecycleState_Compat) == 872` | Size |

### Block B — Serialise/deserialise round-trip (5 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| B1 | `LifecycleState` round-trip: serialise → deserialise → memcmp == 0 (zero-init) | Round-trip |
| B2 | `LifecycleState` round-trip with non-zero fields (food=-500, water=300, skills set) | Round-trip |
| B3 | Boundary: food=-1024, water=-1024 round-trips exact | Round-trip |
| B4 | `SkillState` with experience=128000 (level 10) round-trips | Round-trip |
| B5 | Serialise returns exactly 872 bytes | Round-trip |

### Block C — Hunger/thirst decay (4 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| C1 | Food starts at 1500, after 750 hunger ticks (full stamina), food decreases by exactly 2 per cycle × iterations | Hunger |
| C2 | Water starts at 1500, after 750 hunger ticks, water decreases by 1 per cycle × iterations | Hunger |
| C3 | Food at 0 after N ticks → still zero or negative; food at -512 threshold triggers different behavior | Hunger |
| C4 | Food clamp: after extreme decay, food never below -1024 | Hunger |

### Block D — Starvation / stamina mechanics (3 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| D1 | When food < -512 AND water < -512: staminaLoss is positive (champion LOSES stamina) | Starvation |
| D2 | When food >= 0 AND water >= 0: staminaLoss is negative (champion GAINS stamina) | Starvation |
| D3 | Stamina overflow → health damage: if currentStamina goes below 0 by X, damage = X >> 1 | Starvation |

### Block E — Status expiry (5 invariants, 5 effect types)

| # | Invariant | Category |
|---|-----------|----------|
| E1 | Fireshield: set defense=10, expire → FireShieldDefense returns to 0 | Status |
| E2 | Spellshield: set defense=15, expire → SpellShieldDefense returns to 0 | Status |
| E3 | Invisibility: count=1, expire → count=0 | Status |
| E4 | Champion shield: set defense=20, expire → champion.shieldDefense returns to 0 | Status |
| E5 | Poison: attack=64, one tick → damage=1, attack→63, reschedule event at +36 ticks | Status |

### Block F — Status stacking (2 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| F1 | Two fireshields (defense 10 + 15): total = 25; expire first → total = 15 | Stack |
| F2 | Two poisons (attack 64, 32): PoisonEventCount = 2; independent chains | Stack |

### Block G — Move-timer (3 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| G1 | Light load (load < 62.5% maxLoad): moveTicks = 2 | MoveTimer |
| G2 | Heavy load (load > maxLoad by 50%): moveTicks = 4 + ceil(50%×4) = 6 | MoveTimer |
| G3 | Wounded feet + normal load: moveTicks = 2 + 1 = 3; with boot of speed: 2 | MoveTimer |

### Block H — Rest + health/mana regen (4 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| H1 | Resting with maxHealth=128, vitality=50: healthGain = (128>>7)+1 = 2; doubled to 4 if resting | Regen |
| H2 | Mana regen: maxMana=100, wizPriest=6 → gain = 100/40 = 2, doubled=4+1=5; staminaCost = 5×max(7,10) = 50 | Regen |
| H3 | Rest interrupt: monster adjacent → isResting=false, reason=MONSTER | Rest |
| H4 | Rest interrupt: projectile on square → isResting=false, reason=PROJECTILE | Rest |

### Block I — XP award + level-up (5 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| I1 | Skill exp=0 → level=1; exp=500 → level=2; exp=999 → level=2; exp=1000 → level=3 | XP |
| I2 | Award 500 XP to SKILL_SWING (map difficulty=1): base FIGHTER also gets 500 XP | XP |
| I3 | Level-up from 1→2 as Fighter: maxHealth increases, maxStamina increases, strength increases | Level |
| I4 | Level-up from 1→2 as Wizard: maxMana increases by level + level/2 (= 3) + random bonus | Level |
| I5 | MaxHealth capped at 999, MaxStamina at 9999, MaxMana at 900 after level-up | Level |

### Block J — Integration + boundary (8 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| J1 | Timeline: HUNGER_THIRST event scheduled at correct future tick | Integration |
| J2 | Timeline: STATUS_TIMEOUT event consumed → status counter decremented | Integration |
| J3 | Phase 15 integration: LifecycleState serialise → deserialise → bit-identical | Save/Load |
| J4 | Phase 15 integration: LifecycleState with all fields max → round-trip | Save/Load |
| J5 | Phase 15 integration: LifecycleState with all fields zero → round-trip | Save/Load |
| J6 | Boundary: NULL champ pointer → early return 0 (no crash) | Boundary |
| J7 | Boundary: food=0, water=0 → no negative stamina loss on first tick | Boundary |
| J8 | Purity: calling F0833 twice with same input → same output | Purity |

### Block K — Loop guard + DUNGEON.DAT spot-check (3 invariants)

| # | Invariant | Category |
|---|-----------|----------|
| K1 | 1000 HUNGER_THIRST ticks: food counter doesn't overflow; no infinite-schedule (event count stays bounded) | Loop guard |
| K2 | Purity: F0848 level computation is side-effect-free (call twice → same result) | Purity |
| K3 | Real DUNGEON.DAT: champion 0's initial food/water values (sign-extended) match expected range [0..2048] | DUNGEON.DAT |

### Summary

| Block | Count | Category |
|-------|-------|----------|
| A | 6 | Sizes |
| B | 5 | Round-trip |
| C | 4 | Hunger |
| D | 3 | Starvation |
| E | 5 | Status expiry |
| F | 2 | Status stack |
| G | 3 | Move-timer |
| H | 4 | Rest/regen |
| I | 5 | XP/level-up |
| J | 8 | Integration/boundary |
| K | 3 | Loop guard/purity/DAT |
| **Total** | **48** | **Target: 40, Gate: ≥30** |

---

## §6  Implementation order (10 steps)

Each step must compile clean (`-Wall -Wextra`) before proceeding.

### Step 1: Header scaffold (F0857-F0859 stubs)
Create `memory_champion_lifecycle_pc34_compat.h` with all struct
definitions from §2, all function declarations from §3, all
`#define` constants.  Create `memory_champion_lifecycle_pc34_compat.c`
with stub implementations returning 0/-1.  Verify compiles.

### Step 2: Serialise/deserialise (F0857, F0858, F0859)
Implement F0857 (serialise), F0858 (deserialise), F0859 (init from
PartyState).  LSB-first, 4-byte fields for int32, 2-byte for int16.
Verify compiles.

### Step 3: Hunger/thirst core (F0830-F0834)
Implement timeCriteria, staminaAmount, hunger/thirst per-cycle,
full application, clamping.  Verify compiles.

### Step 4: Status-effect expiry (F0835-F0840)
Implement all 6 status-effect handlers: poison, shields, counters,
light.  Verify compiles.

### Step 5: Move-timer (F0841-F0843)
Implement move-tick computation, timer update, can-move check.
Verify compiles.

### Step 6: Health + mana + stat regen (F0844-F0847)
Implement health regen, mana regen, stat drift, temp-XP decay.
Verify compiles.

### Step 7: XP award + level-up (F0848-F0853)
Implement skill-level calculation, XP add, level-up stat bonuses,
combat/magic/kill XP wrappers.  Verify compiles.

### Step 8: Emission helpers (F0854-F0856)
Implement timeline event builder, hunger-thirst reschedule, level-up
marker.  Verify compiles.

### Step 9: Probe (Blocks A-K)
Create `firestaff_m10_champion_lifecycle_probe.c`.  Implement all 48
invariants across 11 blocks (A-K).  Self-contained; includes headers,
creates test data, asserts.  Compile and run:
```
gcc -Wall -Wextra -o lifecycle_probe \
    firestaff_m10_champion_lifecycle_probe.c \
    memory_champion_lifecycle_pc34_compat.c \
    [prior phase .c files as needed] \
    && ./lifecycle_probe
```
Verify `Status: PASS` in output.

### Step 10: Verify-script integration
Add `# Phase 18: Champion lifecycle probe` block to
`run_firestaff_m10_verify.sh`.  Pre-grep: confirm 0 existing
`Phase 18` entries.  Add block.  Post-grep: confirm exactly 1.
Run full verify.  Confirm exit 0 and all 18 phases PASS.

---

## §7  Files to create + modify

### New files

| File | Est. size | Description |
|------|-----------|-------------|
| `memory_champion_lifecycle_pc34_compat.h` | ~12 KB | Struct defs, constants, all F0830-F0859 declarations |
| `memory_champion_lifecycle_pc34_compat.c` | ~35 KB | All function implementations (30 functions, largest phase) |
| `firestaff_m10_champion_lifecycle_probe.c` | ~25 KB | 48 invariants across 11 blocks (A-K) |
| `run_firestaff_m10_champion_lifecycle_probe.sh` | ~0.9 KB | Compile + run probe, check artifacts |

### Modified files

| File | Change | Est. delta |
|------|--------|-----------|
| `run_firestaff_m10_verify.sh` | Add `# Phase 18:` block (~34 lines) | +34 lines |

### Artifacts generated

| Artifact | Contents |
|----------|----------|
| `champion_lifecycle_probe.md` | Probe execution log |
| `champion_lifecycle_invariants.md` | Per-invariant pass/fail, `Status: PASS` footer |

### Total new code budget: ~73 KB

---

## §8  Risk register

### R3 (HIGHLIGHTED) — Cross-phase drift: Phase 10 food/water type mismatch

**Severity**: HIGH  
**Impact**: Phase 10 declares `food`/`water` as `unsigned char` (0-255).
Fontanel uses `int16_t` with range [-1024, +max].  The food/water decay
algorithm requires negative values.  Mismatch breaks the entire
hunger/thirst subsystem.

**Mitigation**: Phase 18 carries its OWN `int16_t food` and `int16_t water`
in `ChampionLifecycleState_Compat`.  On init (F0859), Phase 10's uint8
values are sign-extended to int16.  Phase 18 fields are authoritative.
Phase 10 fields are NOT modified.  This is documented in §2.9 and
F0859.  Implementer must read Phase 10 header FIRST and verify the
field types before proceeding.

**Detection**: Probe invariant C4 (food clamp at -1024) and B3
(negative food round-trip) will fail if int16_t is not used.

### R5 (HIGHLIGHTED) — XP award double-counting

**Severity**: MEDIUM  
**Impact**: If Phase 13 combat functions call XP-award directly AND
Phase 18 also awards on combat events, champions get double XP.

**Mitigation**: Phase 13 does NOT call F0304 — it only resolves
damage.  In Fontanel, F0304 is called from action-execution code
(F0234/F0231/etc.), not from the damage resolver.  Phase 18
exposes F0849/F0851/F0852 for the action layer to call.  Phase 13's
existing functions remain unchanged.  The action layer (not yet
ported) will call Phase 18's XP functions.

For v1 probe testing: XP is awarded via explicit test calls, not
via Phase 13 event routing.  No double-counting in probe.

**Detection**: Probe invariant I2 explicitly tests that calling
F0849 once yields expected XP (not doubled).

### Full register (for reference)

| Risk | Severity | Status |
|------|----------|--------|
| R1 — Scope creep into class perks | LOW | Mitigated: strict "skill-level only, no perks" |
| R2 — Verify-script duplicate | LOW | Mitigated: pre-grep/post-grep (standard) |
| **R3 — Food/water type mismatch** | **HIGH** | **Mitigated: own int16 fields, documented** |
| R4 — Status-stack semantics unclear | MEDIUM | Mitigated: Fontanel uses additive, documented + REVIEW tag |
| **R5 — XP double-counting** | **MEDIUM** | **Mitigated: Phase 13 doesn't call F0304** |
| R6 — Rest-interrupt race | LOW | Mitigated: tick-ordering = interrupt check BEFORE regen |
| R7 — Level-up stats mid-combat | LOW | Mitigated: level-up applies after damage resolve |
| R8 — Hunger tick rate (queue pressure) | LOW | Analysis: 4 events/tick max = 4 per game tick, fine |

---

## §9  Acceptance criteria

1. All 18 phases PASS: `run_firestaff_m10_verify.sh` exits 0.
2. `grep -c '^# Phase 18:' run_firestaff_m10_verify.sh` = 1.
   All prior phases unchanged.
3. ≥30 invariants pass (target 48).  `Status: PASS` in
   `champion_lifecycle_invariants.md`.
4. Zero `-Wall -Wextra` warnings from probe compilation.
5. No orphan backup directories.
6. Real DUNGEON.DAT starting food/water invariant (K3) passes.
7. Loop-guard: 1000-tick hunger/thirst simulation (K1) passes without
   overflow or infinite-schedule.
8. All new structs serialise/deserialise bit-identical (Block B).
9. Purity checks pass: same input → same output for F0833, F0848 (J8, K2).
