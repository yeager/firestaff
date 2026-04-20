# PHASE 19 PLAN — Runtime Dynamics (v1)

Firestaff M10 milestone, Phase 19. Assumed starting state: 18 phases
PASS (Phases 17–18 merged before Phase 19 executes). This document
is the *single source of truth* the executor follows. Any deviation
= abort and ask.

Style rules (non-negotiable, inherited from Phases 10–18):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation. Every new struct round-trips
  bit-identical.
- Pure functions: NO globals, NO UI, NO IO. Every handler is a pure
  state transform:
  ```
  F0860_RUNTIME_HandleGroupGenerator_Compat(generatorSensor, partyPos,
      activeGroupCount, maxActiveGroupCount, rng, gameTimeTick)
      -> (outSpawnedGroup OR noSpawn flag,
          0..1 followup C65-equivalent re-enable event,
          outSensorNewState)
  ```
  Randomness flows through Phase 13's explicit `RngState_Compat*`.
- Function numbering: **F0860 – F0879** (20 slots, Phase 19 range).
- Probe emits `runtime_dynamics_probe.md` +
  `runtime_dynamics_invariants.md` with a trailing `Status: PASS`
  line.
- Verify gate: `run_firestaff_m10_verify.sh` gets **exactly one** new
  `# Phase 19:` block appended. Pre-grep and post-grep mandatory.
- ADDITIVE ONLY: zero edits to Phase 9/10/11/12/13/14/15/16/17/18
  source. Phase 19 consumes their interfaces via `#include` and
  pure composition.

Fontanel primary references:

- `F0245_TIMELINE_ProcessEvent5_Square_Corridor` (TIMELINE.C:912)
  — generator trigger path for `C006_SENSOR_FLOOR_GROUP_GENERATOR`.
- `F0246_TIMELINE_ProcessEvent65_EnableGroupGenerator` (TIMELINE.C:
  1010) — re-enable disabled generator sensor.
- `F0185_GROUP_GetGenerated` (GROUP.C:481) — creature group spawn.
- `F0257_TIMELINE_ProcessEvent70_Light` (TIMELINE.C:1720) — per-tick
  light decay.
- C24_EVENT_REMOVE_FLUXCAGE handler (TIMELINE.C:1906 inline) —
  fluxcage despawn.
- `G0039_ai_Graphic562_LightPowerToLightAmount[16]` (DEFS.H:5333)
  — loaded from GRAPHICS.DAT entry 562.

---

## §1 — Scope

### In scope (v1)

1. **GROUP_GENERATOR handler** (mirrors Fontanel C05 corridor event
   + C65 re-enable).
   - Receives a generator sensor's parsed data (creature type from
     `M040_DATA`, creature count from `value` with randomisation
     mask `MASK0x0008`, health multiplier from `M045`, respawn ticks
     from `M046`). All data already decoded by Phase 9's
     `DungeonSensor_Compat`.
   - Spawn a creature group into the live group list (consumes Phase
     9's `DungeonGroup_Compat` structure). Uses
     `F0185_GROUP_GetGenerated`-equivalent pure logic: allocate
     group slot, set creature type, health = baseHealth ×
     healthMultiplier + random(baseHealth >> 2 + 1), assign cell
     positions, direction = random(4).
   - **Active-group-count suppression**: if `currentActiveGroupCount
     >= maxActiveGroupCount - 5` AND generator is on the party map,
     skip this spawn (Fontanel GROUP.C:512 — the function returns
     `THING_NONE`). **No** proximity/adjacency check — Fontanel
     does not suppress generators based on party distance; only the
     global active-group cap applies. (Previous task description
     said "party adjacent → skip"; Fontanel source refutes this.
     `NEEDS DISASSEMBLY REVIEW` tag on whether any build variant
     adds adjacency checks.)
   - If `onceOnly == true`: sensor type set to DISABLED (0),
     no re-enable scheduled.
   - If `onceOnly == false` and `ticks > 0`: sensor disabled,
     schedule `TIMELINE_EVENT_GROUP_GENERATOR` (re-enable variant)
     at `nowTick + decodedTicks`. Tick decoding: if raw ticks > 127
     then `decodedTicks = (raw - 126) << 6`, else `decodedTicks =
     raw` (Fontanel TIMELINE.C:992).
   - If `onceOnly == false` and `ticks == 0`: sensor stays armed
     (no disable/re-enable cycle needed; next corridor event
     re-triggers it).
   - Audible flag → set `outResult.soundRequested = sensor.audible`.

2. **MAGIC_LIGHT_DECAY handler** (mirror of F0257).
   - Receives `lightPower` (int, can be negative for darkness).
   - If `lightPower == 0`: no-op, return.
   - Compute `weakerPower = abs(lightPower) - 1`.
   - Compute `delta = PowerOrdinalToLightAmount[abs(lightPower)] -
     PowerOrdinalToLightAmount[weakerPower]`.
   - If original was negative: `delta = -delta`,
     `weakerPower = -weakerPower`.
   - Output: `magicalLightAmountDelta = delta`.
   - If `weakerPower != 0`: emit follow-up
     `TIMELINE_EVENT_MAGIC_LIGHT_DECAY` at `nowTick + 4` with
     `aux0 = weakerPower` (Fontanel TIMELINE.C:1764 schedules
     `G0313_ul_GameTime + 4`).
   - **PowerOrdinalToLightAmount table**: Phase 14 defined a
     placeholder `{3, 6, 10, 16, 24, 40}` marked
     `NEEDS DISASSEMBLY REVIEW`. Phase 19 consumes that same
     placeholder (via a shared constant array or via Phase 14's
     header). The real table is `G0039_ai_Graphic562_LightPowerToLightAmount[16]`
     loaded from GRAPHICS.DAT entry 562 — indices 0..15 where
     index 0 = 0 (no light). Phase 19 uses indices 0..6 only
     (index 0 = 0, indices 1..6 = placeholder values). Full 16-
     entry table is post-M10 when GRAPHICS.DAT loader lands.
     `NEEDS DISASSEMBLY REVIEW`.

3. **REMOVE_FLUXCAGE handler** (mirror of C24 inline handler).
   - Receives a fluxcage explosion slot reference (slotIndex into
     Phase 17's `ExplosionList_Compat`, map position from the
     event).
   - Despawn: call `F0824_EXPLOSION_Despawn_Compat` (Phase 17's
     existing explosion-list removal).
   - Emit `squareContentChanged` marker (mapIndex, mapX, mapY) so
     caller knows to update thing-list linkage.
   - If the fluxcage was already consumed by a projectile impact
     (Phase 17 sets `ExplosionInstance_Compat.slotIndex = -1` upon
     despawn), the handler detects the empty slot and returns
     success with no further action (idempotent).
   - Multiple fluxcages on the same cell: each has a separate
     explosion slot and a separate REMOVE_FLUXCAGE event. Phase 19
     removes each individually.

4. **Generator re-enable handler** (mirror of C65).
   - Receives the map position of a disabled generator sensor.
   - Iterates sensors on the square (via Phase 9's thing-list
     traversal). Finds first sensor with `sensorType == 0`
     (DISABLED) and restores its type to
     `C006_SENSOR_FLOOR_GROUP_GENERATOR` (Fontanel TIMELINE.C:
     1024-1025).
   - Pure: takes sensor list, returns modified sensor index + new
     type.
   - After re-enable, the sensor is live again; next corridor event
     on that square will trigger a fresh spawn cycle.

### Explicitly OUT of scope (v1)

- Generator *creation* / dungeon-editor placement.
- Light-spell *casting* (Phase 14 owns cast; Phase 19 only decays
  already-active lights).
- Fluxcage *placement* (Phase 14 cast + Phase 17 explosion create).
- Sensor *initial arming* (Phase 8/9).
- Projectile-vs-fluxcage damage transfer (Phase 17).
- Monster spawning from non-generator sources (doors, teleporters).
- Day/night cycles (DM1 is dungeon-only).
- Dynamic difficulty scaling (not a Fontanel feature).
- WATCHDOG event (C53; separate meta-concern, post-M10).
- Full 16-entry `LightPowerToLightAmount` table (requires
  GRAPHICS.DAT loader).
- Fluxcage adjacency logic (`F0221_GROUP_IsFluxcageOnSquare` /
  `F0224_GROUP_FluxCageAction`) — that's Phase 17's domain; Phase
  19 only removes them by timer expiry.
- Sound playback (handlers set `soundRequested` flag; caller plays).

---

## §2 — Data structures

### Plan-reader rule (mandatory)

Before writing ANY code, the implementer MUST open the following
headers and record the exact struct names:

1. `memory_magic_pc34_compat.h` — `MagicState_Compat` (confirmed:
   has `magicalLightAmount`, `lightDecayFireAtTick`,
   `event70LightDirection`).
2. `memory_projectile_pc34_compat.h` — `ExplosionList_Compat`,
   `ExplosionInstance_Compat`, `F0824_EXPLOSION_Despawn_Compat`.
3. `memory_dungeon_dat_pc34_compat.h` — `DungeonSensor_Compat`
   (has `sensorType`, `sensorData`, `onceOnly`, `value`,
   `localMultiple`, `audible`), `DungeonGroup_Compat`.
4. `memory_timeline_pc34_compat.h` — `TimelineEvent_Compat`,
   `TimelineQueue_Compat`, event kind constants.
5. `memory_combat_pc34_compat.h` — `RngState_Compat` (F0732).
6. `memory_champion_state_pc34_compat.h` — `PartyState_Compat`
   (party position for suppression check).
7. Phase 18 header (if exists) — `memory_champion_lifecycle_pc34_compat.h`
   or similar.

If **any** of the above types were renamed from what this plan says,
use the actual name. Do NOT redeclare types that already exist.

### New structures (Phase 19)

All sizes are multiples of 4. Each field is `int32_t`-equivalent.
`_Static_assert(sizeof(int) == 4, ...)` sits at the top of the `.c`.

#### 2.1  `GeneratorContext_Compat`

Caller-built snapshot of generator sensor state + dungeon context.
Avoids Phase 19 needing to traverse thing-lists directly.

```
struct GeneratorContext_Compat {
    int sensorIndex;              /* index into things.sensors[]          */
    int mapIndex;
    int mapX;
    int mapY;
    int creatureType;             /* M040_DATA(sensor): 0..26             */
    int creatureCountRaw;         /* sensor.value (4 bits)                */
    int randomizeCount;           /* bit 3 of creatureCountRaw            */
    int healthMultiplier;         /* M045: localMultiple & 0x000F         */
    int ticksRaw;                 /* M046: localMultiple >> 4             */
    int onceOnly;                 /* sensor.onceOnly                      */
    int audible;                  /* sensor.audible                       */
    int mapDifficulty;            /* current map's difficulty rating      */
    int isOnPartyMap;             /* 1 if generator map == party map      */
    int currentActiveGroupCount;  /* global live group count              */
    int maxActiveGroupCount;      /* dungeon header max                   */
    int reserved0;
};
```

- 16 int32 → `GENERATOR_CONTEXT_SERIALIZED_SIZE = 64`.

#### 2.2  `GeneratorResult_Compat`

Output of the generator handler.

```
struct GeneratorResult_Compat {
    int spawned;                  /* 1 = group was generated              */
    int spawnedCreatureType;      /* creature type of new group           */
    int spawnedCreatureCount;     /* count (0-based like Fontanel)        */
    int spawnedDirection;         /* random direction 0..3                */
    int spawnedHealthMultiplier;  /* effective multiplier used            */
    int sensorDisabled;           /* 1 if sensor was set to DISABLED      */
    int reEnableScheduled;        /* 1 if a re-enable event was emitted   */
    uint32_t reEnableAtTick;      /* fireAtTick of re-enable event        */
    int soundRequested;           /* 1 if audible flag was set            */
    int suppressionReason;        /* 0=none, 1=activeGroupCap, 2=noSlot  */
    int rngCallCount;             /* how many RNG calls consumed          */
    int reserved0;
    int spawnedGroupHealth[4];    /* HP for each creature (up to 4)       */
    struct TimelineEvent_Compat reEnableEvent; /* follow-up, if any      */
};
```

- 12 int32 + 4 int32 (health) + 44 B (event) + 4 B padding =
  `GENERATOR_RESULT_SERIALIZED_SIZE = 112`.

#### 2.3  `LightDecayInput_Compat`

```
struct LightDecayInput_Compat {
    int lightPower;               /* from event.aux0; negative = darkness */
    uint32_t nowTick;             /* current game time                    */
    int partyMapIndex;            /* needed for follow-up event           */
};
```

- 3 int32 → 12 bytes. Trivial; may be inlined as parameters.

#### 2.4  `LightDecayResult_Compat`

```
struct LightDecayResult_Compat {
    int magicalLightAmountDelta;  /* add to Party.MagicalLightAmount      */
    int expired;                  /* 1 if this was the last decay step    */
    int followupScheduled;        /* 1 if a new C70 event was emitted    */
    struct TimelineEvent_Compat followupEvent; /* the new C70, if any    */
};
```

- 3 int32 + 44 B (event) = `LIGHT_DECAY_RESULT_SERIALIZED_SIZE = 56`.

#### 2.5  `FluxcageRemoveInput_Compat`

```
struct FluxcageRemoveInput_Compat {
    int explosionSlotIndex;       /* into ExplosionList_Compat            */
    int mapIndex;
    int mapX;
    int mapY;
};
```

- 4 int32 → 16 bytes.

#### 2.6  `FluxcageRemoveResult_Compat`

```
struct FluxcageRemoveResult_Compat {
    int removed;                  /* 1 = actually removed; 0 = already gone */
    int mapIndex;
    int mapX;
    int mapY;
    int squareContentChanged;     /* always 1 if removed                  */
};
```

- 5 int32 → 20 bytes.

#### 2.7  `GeneratorReEnableInput_Compat`

```
struct GeneratorReEnableInput_Compat {
    int mapIndex;
    int mapX;
    int mapY;
};
```

- 3 int32 → 12 bytes. (Matches C65 event content.)

#### 2.8  `GeneratorReEnableResult_Compat`

```
struct GeneratorReEnableResult_Compat {
    int reEnabled;                /* 1 = found and re-enabled             */
    int sensorIndex;              /* which sensor was re-enabled          */
};
```

- 2 int32 → 8 bytes.

#### 2.9  `PowerOrdinalToLightAmount` constant array

```
static const int Phase19_PowerOrdinalToLightAmount[7] = {
    0,   /* index 0: no light     */
    3,   /* index 1: power 1      */
    6,   /* index 2: power 2      */
    10,  /* index 3: power 3      */
    16,  /* index 4: power 4      */
    24,  /* index 5: power 5      */
    40   /* index 6: power 6      */
};
/* NEEDS DISASSEMBLY REVIEW — placeholder values mirroring Phase 14.
   Real table is G0039_ai_Graphic562_LightPowerToLightAmount[16]
   from GRAPHICS.DAT entry 562. Only indices 0-6 used in v1.
   Phase 14 defined {3,6,10,16,24,40} for indices 1-6; Phase 19
   adds index 0 = 0 and consumes identically. */
```

### Structures NOT declared by Phase 19

- `ActiveLightInstance_Compat` — **not needed**. Fontanel tracks
  light as timeline events (C70_EVENT_LIGHT), not as a persistent
  list. Each cast schedules a C70 event chain; there's no "active
  light list" data structure. Phase 14's `MagicState_Compat` holds
  the aggregate `magicalLightAmount`; the per-light state IS the
  chain of C70 events in the timeline queue.
- `FluxcageInstance_Compat` — **not needed**. Fluxcages are tracked
  as `ExplosionInstance_Compat` entries with `explosionType ==
  C050_EXPLOSION_FLUXCAGE` in Phase 17's `ExplosionList_Compat`.
  Phase 19 despawns them via `F0824`. No separate tracking required.

---

## §3 — Function API (F0860–F0879)

### Group A — Generator lifecycle (F0860–F0863)

| ID | Signature | Purpose |
|----|-----------|---------|
| F0860 | `int F0860_RUNTIME_HandleGroupGenerator_Compat(const struct GeneratorContext_Compat* ctx, struct RngState_Compat* rng, uint32_t nowTick, struct GeneratorResult_Compat* outResult);` | Main generator handler. Mirrors F0245 corridor-event generator path. Checks suppression (active group cap on party map), computes creature count (with randomisation), health, direction. Populates `outResult` with spawn data + re-enable event if applicable. |
| F0861 | `int F0861_RUNTIME_CheckGeneratorSuppression_Compat(const struct GeneratorContext_Compat* ctx, int* outSuppressed, int* outReason);` | Pure check: `isOnPartyMap && currentActiveGroupCount >= maxActiveGroupCount - 5` → suppressed (reason=1). Also validates creature type range. |
| F0862 | `int F0862_RUNTIME_ComputeSpawnedGroupHealth_Compat(int creatureType, int healthMultiplier, int creatureCount, struct RngState_Compat* rng, int outHealth[4], int* outRngCallCount);` | Computes per-creature health: `baseHealth * healthMultiplier + random(baseHealth >> 2 + 1)`. Mirrors GROUP.C:539-546. Requires `CreatureInfo_Compat` base-health lookup (Phase 16 or Phase 9 creature table). |
| F0863 | `int F0863_RUNTIME_BuildGeneratorReEnableEvent_Compat(const struct GeneratorContext_Compat* ctx, uint32_t nowTick, struct TimelineEvent_Compat* outEvent);` | Builds the C65-equivalent re-enable event. Tick decoding: if `ticksRaw > 127` then `delay = (ticksRaw - 126) << 6` else `delay = ticksRaw`. Sets `outEvent->kind = TIMELINE_EVENT_GROUP_GENERATOR`, `fireAtTick = nowTick + delay`, position from ctx. `outEvent->aux0` = a sentinel distinguishing "re-enable" from "spawn" (e.g. aux0=1 for re-enable, 0 for initial trigger). |

### Group B — Light decay (F0864–F0867)

| ID | Signature | Purpose |
|----|-----------|---------|
| F0864 | `int F0864_RUNTIME_HandleLightDecay_Compat(int lightPower, uint32_t nowTick, int partyMapIndex, struct LightDecayResult_Compat* outResult);` | Main light decay handler. Mirror of F0257. No RNG needed. Pure arithmetic on the PowerOrdinalToLightAmount table. |
| F0865 | `int F0865_RUNTIME_ComputeLightDecayDelta_Compat(int lightPower, int* outDelta);` | Pure lookup: `delta = table[abs(power)] - table[abs(power)-1]`; negate if power was negative. |
| F0866 | `int F0866_RUNTIME_BuildLightDecayFollowupEvent_Compat(int weakerPower, uint32_t nowTick, int partyMapIndex, struct TimelineEvent_Compat* outEvent);` | Builds a `TIMELINE_EVENT_MAGIC_LIGHT_DECAY` at `nowTick + 4` with `aux0 = weakerPower`. |
| F0867 | `int F0867_RUNTIME_ComputeTotalLightAmount_Compat(int currentAmount, int delta, int* outNewAmount);` | Apply delta and clamp. Helper for caller integration. |

### Group C — Fluxcage removal (F0868–F0871)

| ID | Signature | Purpose |
|----|-----------|---------|
| F0868 | `int F0868_RUNTIME_HandleRemoveFluxcage_Compat(const struct FluxcageRemoveInput_Compat* in, struct ExplosionList_Compat* explosions, struct FluxcageRemoveResult_Compat* outResult);` | Main handler. Checks if the slot still holds a live fluxcage (type == C050). If yes: calls F0824 to despawn, sets `removed=1`, `squareContentChanged=1`. If slot already empty or different type: `removed=0` (idempotent). |
| F0869 | `int F0869_RUNTIME_IsFluxcageSlotLive_Compat(const struct ExplosionList_Compat* explosions, int slotIndex, int* outIsLive);` | Checks `explosions->entries[slotIndex].slotIndex != -1 && explosionType == C050_EXPLOSION_FLUXCAGE`. |
| F0870 | `int F0870_RUNTIME_FluxcageRemoveByAbsorption_Compat(struct ExplosionList_Compat* explosions, int slotIndex, struct FluxcageRemoveResult_Compat* outResult);` | Called when Phase 17 signals absorption. Same cleanup as F0868 but with different entry point for clarity. In practice delegates to F0868 logic. |
| F0871 | `int F0871_RUNTIME_CountFluxcagesOnSquare_Compat(const struct ExplosionList_Compat* explosions, int mapIndex, int mapX, int mapY, int* outCount);` | Counts live fluxcage explosions matching the given position. Utility for invariants and caller integration. |

### Group D — Generator re-enable (F0872–F0874)

| ID | Signature | Purpose |
|----|-----------|---------|
| F0872 | `int F0872_RUNTIME_HandleGeneratorReEnable_Compat(const struct GeneratorReEnableInput_Compat* in, struct DungeonSensor_Compat* sensors, int sensorCount, struct GeneratorReEnableResult_Compat* outResult);` | Mirror of F0246. Iterates sensors at (mapX, mapY), finds first with `sensorType == 0` (DISABLED), sets type to 6 (`C006_SENSOR_FLOOR_GROUP_GENERATOR`). If none found: `reEnabled=0`. |
| F0873 | `int F0873_RUNTIME_FindDisabledSensorOnSquare_Compat(const struct DungeonSensor_Compat* sensors, int sensorCount, int mapIndex, int mapX, int mapY, int* outSensorIndex);` | Linear scan for sensorType == 0 at the given position. Returns index or -1. Helper for F0872. |
| F0874 | `int F0874_RUNTIME_ReEnableSensor_Compat(struct DungeonSensor_Compat* sensor, int newSensorType);` | Sets `sensor->sensorType = newSensorType`. Trivial but isolated for testability. |

### Group E — Serialisation (F0875–F0879)

| ID | Signature | Purpose |
|----|-----------|---------|
| F0875a | `int F0875_RUNTIME_GeneratorContextSerialize_Compat(...)` | Serialize `GeneratorContext_Compat` (64 B). |
| F0875b | `int F0875_RUNTIME_GeneratorContextDeserialize_Compat(...)` | Deserialize. |
| F0876a | `int F0876_RUNTIME_GeneratorResultSerialize_Compat(...)` | Serialize `GeneratorResult_Compat` (112 B). |
| F0876b | `int F0876_RUNTIME_GeneratorResultDeserialize_Compat(...)` | Deserialize. |
| F0877a | `int F0877_RUNTIME_LightDecayResultSerialize_Compat(...)` | Serialize `LightDecayResult_Compat` (56 B). |
| F0877b | `int F0877_RUNTIME_LightDecayResultDeserialize_Compat(...)` | Deserialize. |
| F0878a | `int F0878_RUNTIME_FluxcageRemoveResultSerialize_Compat(...)` | Serialize `FluxcageRemoveResult_Compat` (20 B). |
| F0878b | `int F0878_RUNTIME_FluxcageRemoveResultDeserialize_Compat(...)` | Deserialize. |
| F0879a | `int F0879_RUNTIME_GeneratorReEnableResultSerialize_Compat(...)` | Serialize `GeneratorReEnableResult_Compat` (8 B). |
| F0879b | `int F0879_RUNTIME_GeneratorReEnableResultDeserialize_Compat(...)` | Deserialize. |

All serialisation follows the MEDIA016 pattern: `int32_t` LE fields,
`memcpy` from struct to buffer and back. Round-trip identity
guaranteed by invariants.

---

## §4 — Algorithm specifications

### §4.1 — Generator spawn (F0860)

Mirror of Fontanel TIMELINE.C:962-1006.

```
FUNCTION F0860_HandleGroupGenerator(ctx, rng, nowTick):
    // §4.1.1 Suppression check
    (suppressed, reason) = F0861_CheckSuppression(ctx)
    IF suppressed:
        outResult.spawned = 0
        outResult.suppressionReason = reason
        // NOTE: no re-enable event on suppression.
        // The corridor event will simply fire again next tick.
        RETURN 1

    // §4.1.2 Creature count resolution
    rawCount = ctx.creatureCountRaw & 0x07  // MASK0x0007
    IF ctx.randomizeCount:  // MASK0x0008
        creatureCount = RANDOM(rng, rawCount)  // 0..rawCount-1
    ELSE:
        creatureCount = rawCount - 1  // Fontanel: L0612_ui_CreatureCount--
    // creatureCount is 0-based (0 = 1 creature, like Fontanel)

    // §4.1.3 Health multiplier
    healthMult = ctx.healthMultiplier
    IF healthMult == 0:
        healthMult = ctx.mapDifficulty  // Fontanel: G0269_ps_CurrentMap->C.Difficulty

    // §4.1.4 Spawn
    direction = RANDOM(rng, 4)
    F0862_ComputeHealth(ctx.creatureType, healthMult, creatureCount,
                        rng, outResult.spawnedGroupHealth,
                        &outResult.rngCallCount)
    outResult.spawned = 1
    outResult.spawnedCreatureType = ctx.creatureType
    outResult.spawnedCreatureCount = creatureCount
    outResult.spawnedDirection = direction
    outResult.spawnedHealthMultiplier = healthMult
    outResult.soundRequested = ctx.audible

    // §4.1.5 Sensor state management
    IF ctx.onceOnly:
        outResult.sensorDisabled = 1
        outResult.reEnableScheduled = 0
    ELSE:
        ticks = ctx.ticksRaw
        IF ticks > 0:
            outResult.sensorDisabled = 1
            outResult.reEnableScheduled = 1
            F0863_BuildReEnableEvent(ctx, nowTick, &outResult.reEnableEvent)
        ELSE:
            // ticks==0: sensor stays armed, no disable/re-enable
            outResult.sensorDisabled = 0
            outResult.reEnableScheduled = 0

    RETURN 1
```

### §4.1a — Suppression check (F0861)

```
FUNCTION F0861_CheckSuppression(ctx):
    IF ctx.isOnPartyMap AND
       ctx.currentActiveGroupCount >= (ctx.maxActiveGroupCount - 5):
        outSuppressed = 1
        outReason = 1  // SUPPRESSION_ACTIVE_GROUP_CAP
    ELSE:
        outSuppressed = 0
        outReason = 0
    RETURN 1
```

Fontanel cite: GROUP.C:512 —
`if (((G0377_ui_CurrentActiveGroupCount >= (G0376_ui_MaximumActiveGroupCount - 5)) && (G0272_i_CurrentMapIndex == G0309_i_PartyMapIndex)) || (... THING_NONE ...))`.
The `THING_NONE` path is a "no free group slot" condition; we represent
that as reason=2 if the caller detects group-list overflow.

### §4.1b — Re-enable event build (F0863)

```
FUNCTION F0863_BuildReEnableEvent(ctx, nowTick, outEvent):
    ticks = ctx.ticksRaw
    IF ticks > 127:
        delay = (ticks - 126) << 6  // Fontanel: TIMELINE.C:990-991
    ELSE:
        delay = ticks
    outEvent.kind = TIMELINE_EVENT_GROUP_GENERATOR
    outEvent.fireAtTick = nowTick + delay
    outEvent.mapIndex = ctx.mapIndex
    outEvent.mapX = ctx.mapX
    outEvent.mapY = ctx.mapY
    outEvent.aux0 = 1  // sentinel: "re-enable mode" vs "trigger mode"
    RETURN 1
```

### §4.2 — Light decay (F0864)

Mirror of Fontanel TIMELINE.C:1720-1772.

```
FUNCTION F0864_HandleLightDecay(lightPower, nowTick, partyMapIndex, out):
    IF lightPower == 0:
        out.expired = 1
        out.magicalLightAmountDelta = 0
        out.followupScheduled = 0
        RETURN 1

    isNegative = (lightPower < 0)
    absPower = ABS(lightPower)
    weakerAbsPower = absPower - 1

    // §4.2.1 Delta computation
    delta = PowerOrdinalToLightAmount[absPower]
          - PowerOrdinalToLightAmount[weakerAbsPower]
    IF isNegative:
        delta = -delta
        weakerPower = -weakerAbsPower
    ELSE:
        weakerPower = weakerAbsPower

    out.magicalLightAmountDelta = delta

    // §4.2.2 Follow-up scheduling
    IF weakerPower != 0:
        out.expired = 0
        out.followupScheduled = 1
        F0866_BuildFollowup(weakerPower, nowTick, partyMapIndex,
                            &out.followupEvent)
    ELSE:
        out.expired = 1
        out.followupScheduled = 0

    RETURN 1
```

**PowerOrdinalToLightAmount table (NEEDS DISASSEMBLY REVIEW)**:

Index 0 = 0 (sentinel). Indices 1–6: `{3, 6, 10, 16, 24, 40}`.
These match Phase 14's placeholder. The real values are from
GRAPHICS.DAT entry 562, `G0039_ai_Graphic562_LightPowerToLightAmount`.

The delta-per-step for a light power 6 spell would be:
- Step 6→5: 40 - 24 = 16
- Step 5→4: 24 - 16 = 8
- Step 4→3: 16 - 10 = 6
- Step 3→2: 10 - 6 = 4
- Step 2→1: 6 - 3 = 3
- Step 1→0: 3 - 0 = 3
Total: 40 (= table[6]), which is correct — all light removed.

Each step fires at `nowTick + 4`, so a power-6 light fully decays
in 6 × 4 = 24 ticks.

### §4.3 — Fluxcage removal (F0868)

Mirror of Fontanel TIMELINE.C:1906 inline.

```
FUNCTION F0868_HandleRemoveFluxcage(in, explosions, outResult):
    slot = in.explosionSlotIndex
    isLive = 0
    F0869_IsFluxcageSlotLive(explosions, slot, &isLive)

    IF NOT isLive:
        // Already consumed by Phase 17 projectile impact or
        // previously expired — idempotent.
        outResult.removed = 0
        outResult.squareContentChanged = 0
        RETURN 1

    // Despawn via Phase 17's existing API
    F0824_EXPLOSION_Despawn_Compat(explosions, slot)

    outResult.removed = 1
    outResult.mapIndex = in.mapIndex
    outResult.mapX = in.mapX
    outResult.mapY = in.mapY
    outResult.squareContentChanged = 1
    RETURN 1
```

### §4.4 — Generator re-enable (F0872)

Mirror of Fontanel TIMELINE.C:1010-1031.

```
FUNCTION F0872_HandleGeneratorReEnable(in, sensors, sensorCount, out):
    foundIndex = -1
    F0873_FindDisabledSensor(sensors, sensorCount,
                             in.mapIndex, in.mapX, in.mapY,
                             &foundIndex)
    IF foundIndex < 0:
        out.reEnabled = 0
        out.sensorIndex = -1
        RETURN 1

    F0874_ReEnableSensor(&sensors[foundIndex], 6)
    // 6 = C006_SENSOR_FLOOR_GROUP_GENERATOR

    out.reEnabled = 1
    out.sensorIndex = foundIndex
    RETURN 1
```

### §4.5 — Unclear points

1. **PowerOrdinalToLightAmount exact values** — `NEEDS DISASSEMBLY
   REVIEW`. Placeholder `{0, 3, 6, 10, 16, 24, 40}` used for
   indices 0–6. Real 16-entry table from GRAPHICS.DAT entry 562
   not yet loaded.

2. **Generator adjacency suppression** — The task description
   mentioned "party adjacent → skip". Fontanel source shows only
   a global active-group-count check on the party map (GROUP.C:512).
   No per-cell adjacency check found. `NEEDS DISASSEMBLY REVIEW` —
   some DM ports may add adjacency, but Fontanel WIP does not.

3. **Sensor-respawn for non-generator types** — The task description
   mentioned "pressure plates with onceOnly=false, counter sensors".
   Fontanel's C65 event only re-enables group generators. No
   evidence of timed respawn for other floor sensor types in
   TIMELINE.C. Phase 19 v1 implements only the generator re-enable
   path. If a broader sensor respawn mechanism is needed, it's v2.
   `NEEDS DISASSEMBLY REVIEW`.

4. **Fluxcage duration** — Phase 14 schedules the REMOVE_FLUXCAGE
   event at cast time. The duration is baked into the event's
   `fireAtTick`. Phase 19 does NOT compute duration; it just fires
   when the timeline says to fire. The duration formula (from
   PROJEXPL.C fluxcage creation path) is Phase 14/17's
   responsibility.

5. **CreatureInfo base health lookup** — F0862 needs the base health
   for a creature type. Phase 9 or Phase 16 may provide a
   `CreatureInfo_Compat` table or equivalent. The implementer MUST
   check which header exports this and consume it. If not yet
   available, Phase 19 adds a minimal `CreatureBaseHealth_Compat`
   lookup with `NEEDS DISASSEMBLY REVIEW` for creature types not
   yet validated.

---

## §5 — Invariants (40 planned, gate ≥ 30)

### Sizes + round-trip (4)

| # | Description |
|---|-------------|
| 01 | `sizeof(GeneratorContext_Compat) == 64` |
| 02 | `sizeof(GeneratorResult_Compat) == 112` (verify against manual field count) |
| 03 | `sizeof(LightDecayResult_Compat) == 56` |
| 04 | `GeneratorContext_Compat` round-trip: serialize → deserialize → memcmp == 0 |

### Generator spawn (4)

| # | Description |
|---|-------------|
| 05 | Generator with creatureType=7, count=3 (non-random), healthMult=2 → spawned=1, spawnedCreatureCount=2 (0-based), all 3 health values > 0 |
| 06 | Generator with randomizeCount=1, rawCount=4 → spawnedCreatureCount ∈ [0..3] (deterministic with seeded RNG) |
| 07 | Generator result healthMultiplier == 0 resolved to mapDifficulty → spawned health uses mapDifficulty |
| 08 | Generator result contains consistent direction ∈ [0..3] |

### Generator suppression (3)

| # | Description |
|---|-------------|
| 09 | `isOnPartyMap=1, currentActive=50, maxActive=55` → suppressed=1, reason=1 |
| 10 | `isOnPartyMap=1, currentActive=49, maxActive=55` → suppressed=0 |
| 11 | `isOnPartyMap=0, currentActive=99, maxActive=55` → suppressed=0 (not on party map — no suppression) |

### Generator cooldown (3)

| # | Description |
|---|-------------|
| 12 | `onceOnly=0, ticksRaw=100` → reEnableScheduled=1, `reEnableEvent.fireAtTick == nowTick + 100` |
| 13 | `onceOnly=0, ticksRaw=200` → `reEnableEvent.fireAtTick == nowTick + ((200-126)<<6)` = `nowTick + 4736` |
| 14 | `onceOnly=0, ticksRaw=0` → sensorDisabled=0, reEnableScheduled=0 (stays armed) |

### Generator max-count (1)

| # | Description |
|---|-------------|
| 15 | Generator with `isOnPartyMap=1, currentActive=maxActive-5` → suppressed=1 (exactly at threshold) |

### Light decay (4)

| # | Description |
|---|-------------|
| 16 | lightPower=6 → delta = table[6]-table[5] = 40-24 = 16. followupScheduled=1, followup lightPower=5 |
| 17 | lightPower=1 → delta = table[1]-table[0] = 3-0 = 3. followupScheduled=0, expired=1 |
| 18 | Simulate full decay chain power=6→5→4→3→2→1→0: cumulative delta sums to table[6]=40. All 6 steps produce followup except last. |
| 19 | lightPower=0 → no-op. delta=0, expired=1 |

### Light stacking (1)

| # | Description |
|---|-------------|
| 20 | currentAmount=0, apply delta for power 3 (table[3]=10), then apply delta for power 2 (table[2]=6). Result = 10+6 = 16. |

### Light with negative power / darkness (2)

| # | Description |
|---|-------------|
| 21 | lightPower=-3 → delta = -(table[3]-table[2]) = -(10-6) = -4. followup lightPower=-2 |
| 22 | Full darkness decay chain power=-3→-2→-1→0: cumulative delta = -10. |

### Fluxcage timer expiry (2)

| # | Description |
|---|-------------|
| 23 | Live fluxcage at slot 5 (type=C050, slotIndex=5) → F0868 removes it, removed=1, squareContentChanged=1 |
| 24 | Call F0868 twice on same slot → first removes, second returns removed=0 (idempotent) |

### Fluxcage absorption expiry (2)

| # | Description |
|---|-------------|
| 25 | Pre-despawn slot via F0824 (simulating Phase 17 absorption), then F0868 → removed=0, already gone |
| 26 | F0870 (absorption entry) on live fluxcage → removed=1 |

### Multiple fluxcages same cell (1)

| # | Description |
|---|-------------|
| 27 | Two fluxcages at same (mapIndex, mapX, mapY) in slots 3 and 7. Remove slot 3 → F0871 count at that position = 1 (slot 7 survives). Remove slot 7 → count = 0. |

### Generator re-enable (2)

| # | Description |
|---|-------------|
| 28 | Sensor array with sensorType=0 (DISABLED) at (3,5) → F0872 re-enables to type 6. outResult.reEnabled=1. |
| 29 | Sensor array with no disabled sensors at target position → outResult.reEnabled=0. |

### Integration Phase 12: timeline scheduling (2)

| # | Description |
|---|-------------|
| 30 | F0863 builds event with `kind == TIMELINE_EVENT_GROUP_GENERATOR`. Can schedule into a Phase 12 `TimelineQueue_Compat` via F0721. |
| 31 | F0866 builds event with `kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY`. F0721-schedulable. |

### Integration Phase 9: generator → group list (2)

| # | Description |
|---|-------------|
| 32 | F0860 result for creatureType=7 produces `spawnedGroupHealth[0..creatureCount]` all > 0. Values are plausible against Phase 9/16 creature base health (if available). |
| 33 | Generator with count=0 (decoded to creatureCount=0, meaning 1 creature since 0-based) → exactly 1 health entry populated. |

### Integration Phase 14: light decay (2)

| # | Description |
|---|-------------|
| 34 | F0864 produces `magicalLightAmountDelta` that, when applied to a `MagicState_Compat.magicalLightAmount`, produces a valid new value. |
| 35 | A C70-equivalent `TimelineEvent_Compat` built by Phase 14's `F0763_MAGIC_BuildTimelineEvent_Compat` can be decoded by F0864 (aux0 = lightPower). |

### Integration Phase 15: ser/deser round-trip (3)

| # | Description |
|---|-------------|
| 36 | `GeneratorContext_Compat`: fill with non-trivial values, serialize, deserialize → memcmp == 0 |
| 37 | `GeneratorResult_Compat` (including embedded TimelineEvent): round-trip bit-identical |
| 38 | `FluxcageRemoveResult_Compat`: round-trip bit-identical |

### Integration Phase 17: fluxcage absorption (1)

| # | Description |
|---|-------------|
| 39 | Create ExplosionList with a fluxcage via F0821. Pre-despawn it (simulating Phase 17 absorption). F0868 returns removed=0. |

### Boundary (3)

| # | Description |
|---|-------------|
| 40 | F0860 with NULL ctx → returns 0 (error) |
| 41 | F0868 with slotIndex out of range (>= EXPLOSION_LIST_CAPACITY) → returns 0 |
| 42 | F0864 with lightPower=0 → delta=0, expired=1, no followup |

### Purity (2)

| # | Description |
|---|-------------|
| 43 | F0864 called twice with same inputs → identical outputs (no hidden state) |
| 44 | F0860 called with identical seeded RNG → identical spawn results |

### Real DUNGEON.DAT (1)

| # | Description |
|---|-------------|
| 45 | Parse a known generator sensor from Level 1 or 2 of DUNGEON.DAT (identified by position and sensorType==6). Confirm creatureType and count fields are non-zero and within valid ranges. Feed into F0860 with dummy context → spawns successfully. |

### Loop guard (1)

| # | Description |
|---|-------------|
| 46 | Simulate 10,000 ticks of generator activity: one generator fires every 100 ticks, check that cumulative group count stays within bounds (maxActiveGroupCount). No memory corruption, no infinite loops. Suppression kicks in correctly at threshold. |

**Total: 46 invariants** (gate ≥ 30 → ✅ exceeds gate by 16).

---

## §6 — Implementation order

Compile after each step. `-Wall -Wextra -Werror` throughout.

### Step 1: Header scaffold

Create `memory_runtime_dynamics_pc34_compat.h` with:
- Include guard, `#include` for Phase 9/12/13/14/17 headers
- All struct definitions (§2)
- All function prototypes (§3)
- All `#define` constants (serialised sizes, table)
- `_Static_assert` for struct sizes
- **Compile**: header-only, no `.c` yet → zero warnings.

### Step 2: Serialisation stubs

Create `memory_runtime_dynamics_pc34_compat.c` with:
- `_Static_assert(sizeof(int) == 4, ...)`
- Implement F0875–F0879 (all serialize/deserialize pairs)
- **Compile** the `.c` file.

### Step 3: Light decay functions

Implement F0864–F0867 (simplest handlers — pure arithmetic, no RNG,
no cross-phase data dependencies beyond the constant table):
- F0865 (delta computation)
- F0866 (followup event build)
- F0867 (total light amount compute)
- F0864 (main handler, calls the above)
- **Compile**.

### Step 4: Fluxcage removal functions

Implement F0868–F0871:
- F0869 (live-check)
- F0871 (count on square)
- F0868 (main handler, delegates to F0824)
- F0870 (absorption variant)
- **Compile**.

### Step 5: Generator re-enable functions

Implement F0872–F0874:
- F0873 (find disabled sensor)
- F0874 (re-enable sensor)
- F0872 (main handler)
- **Compile**.

### Step 6: Generator spawn functions

Implement F0860–F0863 (most complex — needs RNG, creature health
computation, sensor state management):
- F0861 (suppression check)
- F0862 (health computation — may need creature base health lookup
  from Phase 9/16)
- F0863 (re-enable event build)
- F0860 (main handler, calls the above)
- **Compile**.

### Step 7: Probe file

Create `firestaff_m10_runtime_dynamics_probe.c`:
- `#include` the header
- 46 invariant test functions
- `main()` that runs all, writes `runtime_dynamics_invariants.md`
  and `runtime_dynamics_probe.md`
- Must link against Phase 12/14/17 `.o` files for cross-phase calls
- **Compile and run**: all 46 invariants PASS.

### Step 8: Verify-script integration

- Create `run_firestaff_m10_runtime_dynamics_probe.sh` (~0.9 KB):
  compile + run probe + check `Status: PASS`.
- Modify `run_firestaff_m10_verify.sh`:
  - **Pre-grep**: `grep -c '# Phase 19:' < verify.sh` must = 0.
  - Append one `# Phase 19:` block (~34 lines).
  - **Post-grep**: must = 1.
- **Run full verify**: all 19 phases PASS.

### Step 9: Final validation

- `grep -c '# Phase 19:' run_firestaff_m10_verify.sh` = exactly 1
- `grep -c '# Phase [0-9]' run_firestaff_m10_verify.sh` = 19
- All prior phases still PASS (no regressions)
- Zero `-Wall -Wextra` warnings
- `runtime_dynamics_invariants.md` has `Status: PASS`
- Real DUNGEON.DAT generator spot-check (invariant 45) passes
- Loop-guard (invariant 46) passes

---

## §7 — Files

| File | Action | Est. size |
|------|--------|-----------|
| `memory_runtime_dynamics_pc34_compat.h` | CREATE | ~8 KB |
| `memory_runtime_dynamics_pc34_compat.c` | CREATE | ~18 KB |
| `firestaff_m10_runtime_dynamics_probe.c` | CREATE | ~20 KB (46 invariants) |
| `run_firestaff_m10_runtime_dynamics_probe.sh` | CREATE | ~0.9 KB |
| `run_firestaff_m10_verify.sh` | MODIFY | +34 lines |

Estimated total new code: ~47 KB across 4 new files + 1 modified.

---

## §8 — Risks (top 2)

### R1 — Struct redeclaration collision with Phase 14/17

**Threat**: Phase 14 might define `ActiveLight` or `Fluxcage` types
that Phase 19 accidentally re-declares, or Phase 17 might have
renamed `ExplosionList_Compat` since Phase 17 plan was written.

**Mitigation**: Plan-reader rule (§2) is mandatory. Before writing
ANY code, open every consumed header and record exact type names.
Phase 19 does NOT declare `ActiveLightInstance_Compat` or
`FluxcageInstance_Compat` — both are unnecessary (see §2 rationale).
The only cross-phase function call is `F0824_EXPLOSION_Despawn_Compat`
from Phase 17.

**Residual risk**: Low. Phase 19's new types are all unique-named
(`GeneratorContext_Compat`, `LightDecayResult_Compat`, etc.) with no
overlap with prior phases.

### R5 — Fluxcage cell tracking vs Phase 17 projectile absorption ordering

**Threat**: A fluxcage's REMOVE_FLUXCAGE event fires on the same tick
that a projectile hits and despawns the fluxcage via Phase 17's
absorption path. If both execute on the same tick, the order matters:
Phase 17 despawns first → Phase 19's event finds an empty slot →
idempotent. But if Phase 19 fires first → removes the fluxcage →
Phase 17 finds an empty slot and the projectile flies through.

**Mitigation**: Phase 19's F0868 is idempotent by design. The
tick-ordering convention follows Fontanel: timeline events are
processed in FIFO order within a tick. Fluxcage removal events
are scheduled with a specific `fireAtTick`; projectile-move events
are per-tick. Both converge on the same cleanup code
(`F0824_EXPLOSION_Despawn_Compat`). The invariant (#25/#39) tests
the idempotent path explicitly. The caller (Phase 12 timeline
dispatcher) determines execution order; Phase 19 just handles
whichever state it finds.

**Residual risk**: Low-medium. The ordering concern exists but both
paths produce correct results regardless of execution order because
of the idempotent design.

### Honourable mentions (not top 2)

- **R2 — Verify-script duplicate**: Mitigated by pre/post grep, same
  as every phase. Mechanical.
- **R3 — Generator spawning into occupied cell**: Fontanel's
  `F0185_GROUP_GetGenerated` calls `F0267_MOVE_GetMoveResult_CPSCE`
  which can fail if the party is on the destination square. Phase 19
  delegates this to the caller (pure function returns spawn data;
  caller checks occupancy). `NEEDS DISASSEMBLY REVIEW` for exact
  failure semantics.
- **R4 — Light-amount table unknown**: Mitigated by `NEEDS
  DISASSEMBLY REVIEW` tag + placeholder values matching Phase 14.
  Invariants test structural correctness (monotonicity, sum), not
  absolute values.
- **R6 — Sensor-quiet markers from Phase 11**: Phase 19 v1 does NOT
  consume Phase 11 markers for generic sensor respawn. Only the
  generator C65 re-enable path is implemented. Broader sensor
  respawn is deferred to v2. No dependency on Phase 11 markers.

---

## §9 — Acceptance criteria

1. **All 19 phases PASS** via `run_firestaff_m10_verify.sh`.
2. `grep -c '# Phase 19:' run_firestaff_m10_verify.sh` = exactly 1.
3. All prior phases (`# Phase 1:` through `# Phase 18:`) = 1 each.
4. ≥ 30 invariants, all `Status: PASS`.
5. Zero `-Wall -Wextra` warnings on all Phase 19 source files.
6. Real DUNGEON.DAT generator spot-check (invariant 45) passes.
7. Loop-guard 10k-tick simulation (invariant 46) passes.
8. No modifications to any Phase 1–18 source file.
9. `runtime_dynamics_invariants.md` artifact exists and contains
   `Status: PASS` on its last line.
