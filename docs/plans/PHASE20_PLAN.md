# PHASE 20 PLAN — Tick Orchestrator & Deterministic Harness (v1)

Firestaff M10 milestone, Phase 20.  THE integration milestone.
Assumed starting state: **19 phases PASS** (Phases 17-19 merged
before Phase 20 executes).  This document is the *single source
of truth* the executor follows.  Any deviation = abort and ask.

Style rules (non-negotiable, inherited from Phases 10–19):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation.  Every new struct
  round-trips bit-identical.
- Pure functions (with one documented exception: the headless
  driver's file IO):
  ```
  F0884(world, input) -> (new world, TickResult)
  ```
  Randomness flows through the master `RngState_Compat` embedded
  in `GameWorld_Compat`; no hidden state, no `rand()`, no FP.
- Function numbering: **F0880–F0899** (20 slots, Phase 20 range).
- Probe emits `tick_orchestrator_probe.md` +
  `tick_orchestrator_invariants.md` with trailing `Status: PASS`.
- Verify gate: `run_firestaff_m10_verify.sh` gets **exactly one** new
  `# Phase 20:` block appended.  Pre-grep and post-grep mandatory.
- ADDITIVE ONLY: zero edits to Phase 9-19 source.  Phase 20
  composes all interfaces via `#include`.

Fontanel primary references:

- `GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF` (lines 39–245) — THE
  game loop.  Infinite loop body:
  1. Check new-party-map transitions → `F0003_MAIN_ProcessNewPartyMap`
  2. **`F0261_TIMELINE_Process_CPSEF()`** — pop+dispatch ALL expired events
  3. Render dungeon view (UI only; SKIP in orchestrator)
  4. Sound play (emit marker only)
  5. `F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds` → apply damage
  6. Check `G0303_B_PartyDead` → break if dead
  7. **`G0313_ul_GameTime++`** (THE tick advance)
  8. Every 512 ticks: `F0338_INVENTORY_DecreaseTorchesLightPower`
  9. Freeze-life countdown: `--G0407_s_Party.FreezeLifeTicks`
  10. `F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived`
  11. Every 64 ticks (16 if resting): `F0331_CHAMPION_ApplyTimeEffects`
  12. Decrement `G0310_i_DisabledMovementTicks`, `G0311_i_ProjectileDisabledMovementTicks`
  13. Clear expired text rows
  14. **Input processing loop**: keyboard → `F0361_COMMAND_ProcessKeyPress`, then `F0380_COMMAND_ProcessQueue_CPSC`
  15. Loop until `G0321_B_StopWaitingForPlayerInput && G0301_B_GameTimeTicking`

- `TIMELINE.C:F0261_TIMELINE_Process_CPSEF` (lines 1833–2074) —
  event dispatch.  While first event is expired, extract it and
  switch on type:
  - C29–C41 (group/creature events) → `F0209_GROUP_ProcessEvents29to41`
  - C48/C49 (projectile move) → `F0219_PROJECTILE_ProcessEvents48To49`
  - C01 (door animation) → `F0241`
  - C25 (explosion) → `F0220_EXPLOSION_ProcessEvent25`
  - C07 (fakewall) → `F0242`
  - C02 (door destruction) → `F0243`
  - C10 (door) → `F0244`
  - C09 (pit) → `F0251`
  - C08 (teleporter) → `F0250`
  - C06 (wall) → `F0248`
  - C05 (corridor) → `F0245`
  - C60/C61 (group move) → `F0252`
  - C65 (enable group generator) → `F0246`
  - C20 (play sound) → emit sound marker
  - C24 (remove fluxcage) → unlink thing
  - C11 (enable champion action) → `F0253` + optional `F0259`
  - C12 (hide damage received) → `F0254`
  - C13 (Vi altar rebirth) → `F0255`
  - C70 (light) → `F0257_TIMELINE_ProcessEvent70_Light`
  - C71 (invisibility) → decrement party count
  - C72 (champion shield) → subtract defense
  - C73 (thieves eye) → decrement party count
  - C74 (party shield) → subtract defense
  - C77 (spellshield) → subtract defense
  - C78 (fireshield) → subtract defense
  - C75 (poison) → apply poison damage
  - C79 (footprints) → decrement party count
  - C80–C83 (magic map) → decrement counts

- `DEFS.H` — event type constants C00–C83, RNG macros (M002–M006).

---

## §1  Scope definition

### In scope (v1)

Phase 20 is the **M10 culmination**.  Every prior phase is a pure
data-layer module with its own probe.  Nothing runs a game.  Phase
20 creates the tick orchestrator that ties all 19 into one engine:

1. **GameWorld_Compat** — single aggregate struct composing every
   sub-system's state.  Init from DUNGEON.DAT, deep-clone, free.

2. **Tick orchestrator** — pure function:
   `(GameWorld, TickInput) → (GameWorld', TickResult)`.
   Mirror of Fontanel's `F0002_MAIN_GameLoop_CPSDF` with UI/audio/
   input-wait stripped.  Dispatch order per tick:
   ```
   PHASE_A: Apply player input (if any) — translate command to
            combat action / movement / spell cast / item use
   PHASE_B: Pop+dispatch ALL expired timeline events at nowTick
            (priority-ordered via Phase 12 queue)
   PHASE_C: Apply pending damage and wounds
   PHASE_D: Advance game time (nowTick++)
   PHASE_E: Periodic champion time effects (every 64 / 16 ticks)
   PHASE_F: Decrement cooldown timers
   PHASE_G: Collect emissions into TickResult
   ```

3. **Deterministic RNG** — single master `RngState_Compat` in
   `GameWorld_Compat`.  All consumers advance the same seed.
   Borland LCG: `seed = seed*214013+2531011; return (seed>>16)&0x7FFF`.

4. **TickStreamRecord_Compat** — per-tick snapshot for replay:
   input, emissions, post-tick world-hash (CRC32).

5. **Determinism harness** — run N ticks twice → bit-identical.
   Save-resume equivalence.  Fuzz test (random inputs).

6. **Headless CLI driver** — binary: load DUNGEON.DAT + tick-stream
   file → run all ticks → output final world-hash + summary stats.

### Out of scope (v1)

- UI rendering, audio playback (sound events → emit marker only)
- Real-time timing (tick-based only; framerate is separate)
- Network play
- Save-file migration / version negotiation
- Debugger / time-travel UI
- Input recording from keyboard/mouse (driver reads text file)
- Save-slot UI

### Explicit deferrals

- Map transitions (`G0327_i_NewPartyMapIndex`) — Fontanel's
  GAMELOOP.C re-enters the loop on map change.  Phase 20 v1
  handles this inline: if map change is triggered, the orchestrator
  updates the party map index and re-runs timeline dispatch before
  continuing.  Full multi-level transition chains deferred to v2.

- BUG0_02 (time overflow at tick 2^24) — documented but NOT
  fixed in v1; we faithfully port the 24-bit time wrap bug.

- Copy protection events (C22, C53 watchdog) — stripped from v1
  orchestrator.  `#ifndef NOCOPYPROTECTION` in Fontanel; we define
  `NOCOPYPROTECTION`.

---

## §2  Data structures

All serialisation is MEDIA016 / PC LSB-first (little-endian).
Every byte layout specifies offset and endian.  `int32` = 4 bytes LE.

### §2.1  GameWorld_Compat — THE aggregate

```
struct GameWorld_Compat {
    /* --- Core state (Phase 1–9 dungeon static layer) --- */
    DungeonDatState_Compat      dungeon;           /* Phase 1–4: ~33 KB */
    DungeonThings_Compat        things;            /* Phase 5–8: ~12 KB */
    MonsterGroupList_Compat     monsterGroups;     /* Phase 9:   ~6 KB  */

    /* --- Champion + party (Phase 10) --- */
    PartyState_Compat           party;             /* ~640 B */
    ChampionState_Compat        champions[CHAMPION_MAX_PARTY]; /* 4×~400 B */

    /* --- Sensor state (Phase 11) --- */
    SensorEffectList_Compat     pendingSensorEffects; /* ~228 B */

    /* --- Timeline (Phase 12) --- */
    TimelineQueue_Compat        timeline;          /* ~11 KB */

    /* --- Combat scratch (Phase 13) --- */
    RngState_Compat             masterRng;         /* 4 B — THE RNG seed */
    CombatResult_Compat         pendingCombat;     /* 56 B */

    /* --- Magic state (Phase 14) --- */
    MagicState_Compat           magic;             /* 72 B */

    /* --- Save metadata (Phase 15) --- */
    SaveGameHeader_Compat       saveHeader;        /* 64 B */
    DungeonMutationList_Compat  dungeonMutations;  /* ~49 KB max */

    /* --- Creature AI (Phase 16) --- */
    CreatureAIState_Compat      creatureAI[CREATURE_AI_MAX_GROUPS]; /* ~5 KB */
    int                         creatureAICount;   /* 4 B */

    /* --- Projectile & explosion flight (Phase 17) --- */
    ProjectileInstance_Compat   projectiles[PROJECTILE_LIST_CAPACITY]; /* 60×96 B */
    int                         projectileCount;   /* 4 B */
    ExplosionInstance_Compat    explosions[EXPLOSION_LIST_CAPACITY];   /* 32×64 B */
    int                         explosionCount;    /* 4 B */

    /* --- Champion lifecycle (Phase 18) --- */
    /* State is embedded in champions[] and party; no separate struct.
       Phase 18 operates on ChampionState_Compat fields directly.
       Additional tracking: */
    int                         partyIsResting;    /* 4 B */
    int                         freezeLifeTicks;   /* 4 B */
    int                         disabledMovementTicks;    /* 4 B */
    int                         projectileDisabledMovementTicks; /* 4 B */

    /* --- Runtime dynamics (Phase 19) --- */
    ActiveLightInstance_Compat  activeLights[ACTIVE_LIGHT_LIST_CAPACITY];
    int                         activeLightCount;
    FluxcageInstance_Compat     fluxcages[FLUXCAGE_LIST_CAPACITY];
    int                         fluxcageCount;
    GeneratorInstance_Compat    generators[GENERATOR_LIST_CAPACITY];
    int                         generatorCount;

    /* --- Orchestrator bookkeeping (Phase 20) --- */
    uint32_t                    gameTick;          /* = G0313_ul_GameTime */
    int                         partyDead;         /* = G0303_B_PartyDead */
    int                         gameWon;           /* = G0302_B_GameWon */
    int                         partyMapIndex;     /* = G0309_i_PartyMapIndex */
    int                         newPartyMapIndex;  /* = G0327_i_NewPartyMapIndex, -1 if none */
};
```

**Size estimate:** ~120 KB typical (dominated by dungeon static layer
+ dungeon mutations list).  Variable due to dungeon size; DM1
DUNGEON.DAT → ~80 KB; CSB → ~120 KB.

**Serialisation order of GameWorld_Compat** (critical for world-hash):

```
Offset   Section                          Serialiser
──────   ───────                          ──────────
  0      gameTick                         raw LE uint32 (4 B)
  4      partyDead                        raw LE int32  (4 B)
  8      gameWon                          raw LE int32  (4 B)
 12      partyMapIndex                    raw LE int32  (4 B)
 16      newPartyMapIndex                 raw LE int32  (4 B)
 20      masterRng.seed                   raw LE uint32 (4 B)
 24      partyIsResting                   raw LE int32  (4 B)
 28      freezeLifeTicks                  raw LE int32  (4 B)
 32      disabledMovementTicks            raw LE int32  (4 B)
 36      projectileDisabledMovementTicks  raw LE int32  (4 B)
 40      party                            F0705_PARTY_Serialize  (Phase 10)
  +      champions[0..3]                  F0706 × 4             (Phase 10)
  +      timeline                         F0727                 (Phase 12)
  +      pendingCombat                    Phase 13 CombatResult serialiser
  +      magic                            Phase 14 MagicState serialiser
  +      dungeonMutations                 Phase 15 mutation-list serialiser
  +      creatureAI[0..N]                 Phase 16 × count
  +      projectiles[0..N]               Phase 17 × count
  +      explosions[0..N]                Phase 17 × count
  +      activeLights[0..N]              Phase 19 × count
  +      fluxcages[0..N]                 Phase 19 × count
  +      generators[0..N]                Phase 19 × count
  +      dungeon                         Phase 1–4 (raw bytes, immutable in v1)
  +      things                          Phase 5–8 (raw bytes)
  +      monsterGroups                   Phase 9
  +      pendingSensorEffects            Phase 11
  +      saveHeader                      Phase 15 header
```

**Excluded from world-hash:** Nothing.  Every field participates.
Transient counters (like `pendingSensorEffects`) are small enough
that including them catches any state divergence early.

### §2.2  TickInput_Compat

```
struct TickInput_Compat {           /* 16 bytes */
    uint32_t  tick;                 /* [0..3]  LE — expected tick index */
    uint8_t   command;              /* [4]     — CMD_NONE / CMD_MOVE_N / etc. */
    uint8_t   commandArg1;          /* [5]     — arg (e.g. slot index, spell rune) */
    uint8_t   commandArg2;          /* [6]     — arg (e.g. champion index) */
    uint8_t   reserved;             /* [7]     — 0x00 */
    uint32_t  forcedRngAdvance;     /* [8..11] LE — 0 = no forced advance; >0 = call RNG N times before dispatch (for fuzzing) */
    uint32_t  reserved2;            /* [12..15] — padding to 16 B alignment */
};
```

**Serialised size:** 16 bytes.  Stream format: contiguous array
of `TickInput_Compat` records.

**Command constants:**
```
CMD_NONE              0x00
CMD_MOVE_NORTH        0x01
CMD_MOVE_EAST         0x02
CMD_MOVE_SOUTH        0x03
CMD_MOVE_WEST         0x04
CMD_TURN_LEFT         0x05
CMD_TURN_RIGHT        0x06
CMD_ATTACK            0x10    /* arg1=champion, arg2=hand(0=L,1=R) */
CMD_CAST_SPELL        0x11    /* arg1=champion, arg2=spell rune seq idx */
CMD_USE_ITEM          0x12    /* arg1=champion, arg2=slot */
CMD_EAT               0x13    /* arg1=champion, arg2=slot */
CMD_DRINK             0x14    /* arg1=champion, arg2=slot */
CMD_REST_TOGGLE       0x20
CMD_THROW_ITEM        0x21    /* arg1=champion, arg2=slot */
```

### §2.3  TickEmission_Compat

```
struct TickEmission_Compat {        /* 20 bytes */
    uint8_t   kind;                 /* [0]     — EMIT_DAMAGE / EMIT_SOUND / etc. */
    uint8_t   reserved;             /* [1]     — 0x00 */
    uint16_t  payloadSize;          /* [2..3]  LE — bytes of payload following */
    int32_t   payload[4];           /* [4..19] LE — kind-dependent data */
};
```

**Emission kinds:**
```
EMIT_DAMAGE_DEALT      0x01   /* payload: target-type, target-id, amount, wound-mask */
EMIT_SOUND_REQUEST     0x02   /* payload: sound-index, mapX, mapY, 0 */
EMIT_XP_AWARD          0x03   /* payload: champion, skill, amount, 0 */
EMIT_KILL_NOTIFY       0x04   /* payload: target-type, target-id, killer-champion, 0 */
EMIT_DOOR_STATE        0x05   /* payload: mapX, mapY, newState, 0 */
EMIT_PARTY_MOVED       0x06   /* payload: newMapX, newMapY, newDir, newMapIndex */
EMIT_CHAMPION_DOWN     0x07   /* payload: champion-index, cause, 0, 0 */
EMIT_GAME_WON          0x08   /* payload: 0, 0, 0, 0 */
EMIT_PARTY_DEAD        0x09   /* payload: 0, 0, 0, 0 */
```

**Serialised size:** 20 bytes each, fixed.

### §2.4  TickResult_Compat

```
#define TICK_EMISSION_CAPACITY 64

struct TickResult_Compat {              /* variable; ~1300 B max */
    uint32_t  preTick;                  /* tick index before advance */
    uint32_t  postTick;                 /* tick index after advance */
    uint32_t  worldHashPost;            /* CRC32 of full serialised GameWorld after tick */
    int       emissionCount;
    TickEmission_Compat emissions[TICK_EMISSION_CAPACITY];
};
```

**Serialised size:** `16 + emissionCount × 20` bytes.
Maximum: `16 + 64 × 20 = 1296` bytes.

### §2.5  TickStreamRecord_Compat

```
struct TickStreamRecord_Compat {        /* one per tick in replay file */
    TickInput_Compat  input;            /* 16 B */
    uint32_t          worldHashPost;    /* 4 B */
    uint16_t          emissionCount;    /* 2 B */
    uint16_t          reserved;         /* 2 B — padding */
    /* Emissions are NOT stored in the stream record to keep it
       compact (~24 B/tick avg).  They can be reconstructed by
       replaying.  The world-hash is the determinism witness. */
};
```

**Serialised size:** 24 bytes per tick.  1000 ticks = 24 KB.

### §2.6  GameConfig_Compat

```
struct GameConfig_Compat {              /* 64 bytes */
    char     dungeonPath[48];           /* [0..47]  NUL-terminated */
    uint32_t startingSeed;              /* [48..51] LE */
    uint32_t flags;                     /* [52..55] LE — bit 0: isDM1, bit 1: isCSB */
    uint32_t reserved[2];               /* [56..63] */
};
```

**Serialised size:** 64 bytes, fixed.

---

## §3  Function API (F0880–F0899)

### Group A — Construct / Destruct / Clone (F0880–F0883)

```
F0880_WORLD_AllocDefault_Compat(void) → GameWorld_Compat*
```
Allocate a zeroed `GameWorld_Compat`.  Returns NULL on OOM.

```
F0881_WORLD_InitDefault_Compat(GameWorld_Compat* world, uint32_t seed)
    → int (1=ok, 0=fail)
```
Zero all fields, set `gameTick=0`, init `masterRng.seed = seed`,
init timeline via `F0720_TIMELINE_Init_Compat`, set
`newPartyMapIndex = -1`.  Does NOT load dungeon.

```
F0882_WORLD_InitFromDungeonDat_Compat(
    const char* dungeonPath,
    uint32_t seed,
    GameWorld_Compat* outWorld)
    → int (1=ok, 0=fail)
```
Read-only access to DUNGEON.DAT.  Call Phase 1–4 loader to populate
`dungeon`, Phase 5–8 for `things`, Phase 9 for `monsterGroups`.
Set default champion state (empty party), empty timeline, seed.
Schedule initial timeline events as per Fontanel's
`F0463_START_InitializeGame` (group generators at tick 1, watchdog
at tick 300 — **minus** copy protection events).

```
F0883_WORLD_Free_Compat(GameWorld_Compat* world)
```
Free all dynamic allocations.  Safe to call on NULL.

```
F0880b_WORLD_Clone_Compat(
    const GameWorld_Compat* src,
    GameWorld_Compat* dst)
    → int (1=ok, 0=fail)
```
Deep copy.  `memcmp(src_serialised, dst_serialised) == 0` after clone.

### Group B — Tick Orchestrator (F0884–F0886)

```
F0884_ORCH_AdvanceOneTick_Compat(
    GameWorld_Compat* world,
    const TickInput_Compat* input,
    TickResult_Compat* outResult)
    → int (1=ok, 0=fail, -1=party dead, -2=game won)
```
THE core function.  Pseudocode in §4.1.  Mirrors
`F0002_MAIN_GameLoop_CPSDF` with UI stripped.

```
F0885_ORCH_RunNTicks_Compat(
    GameWorld_Compat* world,
    const TickInput_Compat* inputs,   /* array of N */
    int tickCount,
    TickStreamRecord_Compat* outRecords,  /* array of N, may be NULL */
    uint32_t* outFinalHash)
    → int (ticks actually executed; may be < N if party dies)
```
Loop calling `F0884` for each tick.  If `outRecords` is non-NULL,
fill one record per tick.

```
F0886_ORCH_RunUntilCondition_Compat(
    GameWorld_Compat* world,
    const TickInput_Compat* inputs,
    int maxTicks,
    int (*condition)(const GameWorld_Compat*),
    TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash)
    → int (ticks executed)
```
Run ticks until `condition(world)` returns non-zero or maxTicks
reached.

### Group C — Dispatch Internals (F0887–F0890)

```
F0887_ORCH_DispatchTimelineEvents_Compat(
    GameWorld_Compat* world,
    TickResult_Compat* result)
    → int (events dispatched count)
```
Pop all expired events from `world->timeline` at `world->gameTick`.
For each event, call the appropriate Phase handler based on
`event.kind`:

| Timeline event kind (Phase 12)     | Handler invoked                      |
|------------------------------------|--------------------------------------|
| TIMELINE_EVENT_CREATURE_TICK       | Phase 16 `F0804_CREATURE_Tick`       |
| TIMELINE_EVENT_DOOR_ANIMATE        | emit EMIT_DOOR_STATE marker          |
| TIMELINE_EVENT_PROJECTILE_MOVE     | Phase 17 `F0811_PROJECTILE_Advance`  |
| TIMELINE_EVENT_EXPLOSION_ADVANCE   | Phase 17 `F0822_EXPLOSION_Advance`   |
| TIMELINE_EVENT_SPELL_TICK          | Phase 14 spell-effect step           |
| TIMELINE_EVENT_HUNGER_THIRST       | Phase 18 `F0830_LIFECYCLE_HungerThirst` |
| TIMELINE_EVENT_MAGIC_LIGHT_DECAY   | Phase 19 `F0860_DYNAMICS_LightDecay` |
| TIMELINE_EVENT_MOVE_TIMER          | Phase 18 `F0840_LIFECYCLE_MoveTimer` |
| TIMELINE_EVENT_SENSOR_DELAYED      | Phase 11 `F0710_SENSOR_Execute`      |
| TIMELINE_EVENT_DOOR_DESTRUCTION    | emit EMIT_DOOR_STATE marker          |
| TIMELINE_EVENT_SQUARE_STATE        | square update (corridor/wall/fakewall/teleporter/pit/door) |
| TIMELINE_EVENT_GROUP_GENERATOR     | Phase 19 `F0862_DYNAMICS_GroupGenerator` |
| TIMELINE_EVENT_STATUS_TIMEOUT      | Phase 18 `F0842_LIFECYCLE_StatusTimeout` |
| TIMELINE_EVENT_REMOVE_FLUXCAGE     | Phase 19 `F0864_DYNAMICS_RemoveFluxcage` |
| TIMELINE_EVENT_PLAY_SOUND          | emit EMIT_SOUND_REQUEST              |
| TIMELINE_EVENT_WATCHDOG            | no-op in v1 (NOCOPYPROTECTION)       |

Each handler receives the world state + the extracted event, returns
zero or more follow-up events (rescheduled into `world->timeline`)
and zero or more emissions (appended to `result->emissions[]`).

```
F0888_ORCH_ApplyPlayerInput_Compat(
    GameWorld_Compat* world,
    const TickInput_Compat* input,
    TickResult_Compat* result)
    → int (1=input consumed, 0=no-op/invalid)
```
Translate player command to engine action:
- CMD_MOVE_* → call Phase 10 `F0702_MOVEMENT_AttemptStep` → update
  `world->party`; emit EMIT_PARTY_MOVED; trigger sensor check
- CMD_TURN_* → call Phase 10 turn
- CMD_ATTACK → build `CombatAction_Compat` → Phase 13 resolver →
  schedule enable-action event
- CMD_CAST_SPELL → Phase 14 resolver → schedule spell effect
- CMD_USE_ITEM / EAT / DRINK → item effect logic (Phase 14 potions,
  Phase 18 food/water)
- CMD_REST_TOGGLE → flip `world->partyIsResting`
- CMD_THROW_ITEM → create projectile via Phase 17

```
F0889_ORCH_ApplyPendingDamage_Compat(
    GameWorld_Compat* world,
    TickResult_Compat* result)
    → void
```
Mirror of `F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds`.
Apply accumulated damage to champions; emit EMIT_CHAMPION_DOWN if
HP <= 0.  Set `world->partyDead` if all champions dead.

```
F0890_ORCH_ApplyPeriodicEffects_Compat(
    GameWorld_Compat* world,
    TickResult_Compat* result)
    → void
```
- Every 512 ticks: torch light decay (Phase 19 light list)
- Every 64 ticks (16 if resting): Phase 18 `F0830` champion time effects
- Decrement `disabledMovementTicks`, `projectileDisabledMovementTicks`
- Decrement `freezeLifeTicks`

### Group D — Determinism + Hash (F0891–F0893)

```
F0891_ORCH_WorldHash_Compat(
    const GameWorld_Compat* world,
    uint32_t* outHash)
    → int (1=ok, 0=fail)
```
Serialise full `GameWorld_Compat` into a temp buffer using F0897,
compute CRC32 (IEEE 802.3, same polynomial as Phase 15:
`0xEDB88320`, init `0xFFFFFFFF`, final XOR `0xFFFFFFFF`).

```
F0892_ORCH_VerifyDeterminism_Compat(
    const GameWorld_Compat* initialWorld,
    const TickInput_Compat* inputs,
    int tickCount)
    → int (1=deterministic PASS, 0=FAIL)
```
Clone world twice.  Run N ticks on clone A.  Run N ticks on clone B.
Compare every per-tick `worldHashPost`.  Return 1 iff all match.

```
F0893_ORCH_VerifyResumeEquivalence_Compat(
    const GameWorld_Compat* initialWorld,
    const TickInput_Compat* inputs,
    int tickCount,
    int resumeAtTick)
    → int (1=PASS, 0=FAIL)
```
Run all N ticks straight → hash A.  Run first K ticks →
serialise → deserialise → run remaining N−K ticks → hash B.
Return 1 iff hash A == hash B.

### Group E — Headless Driver Primitives (F0894–F0896)

```
F0894_DRIVER_LoadTickStream_Compat(
    const char* path,
    TickInput_Compat** outInputs,
    int* outCount)
    → int (1=ok, 0=fail)
```
Read text-format tick stream: one line per tick.
Format: `<tick> <cmd_hex> <arg1_hex> <arg2_hex>`
Parse into `TickInput_Compat` array.  Caller frees.

```
F0895_DRIVER_RunStream_Compat(
    GameWorld_Compat* world,
    const TickInput_Compat* inputs,
    int inputCount,
    TickStreamRecord_Compat* outRecords,
    uint32_t* outFinalHash)
    → int (ticks executed)
```
Wrapper around F0885.

```
F0896_DRIVER_WriteSummary_Compat(
    const GameWorld_Compat* world,
    uint32_t finalHash,
    int ticksRun,
    FILE* outFile)
    → void
```
Print: tick count, final hash (hex), champions alive, total monsters
killed (from emissions), total XP earned, party position.

### Group F — Serialise / Deserialise GameWorld (F0897–F0899)

```
F0897_WORLD_Serialize_Compat(
    const GameWorld_Compat* world,
    unsigned char* outBuf,
    int outBufSize,
    int* outBytesWritten)
    → int (1=ok, 0=buf too small)
```
Write fields in the **exact serialisation order** from §2.1.
Compose every prior phase's serialiser in sequence.  Each sub-section
is preceded by a `[uint32_t sectionTag][uint32_t sectionSize]` pair
(8 bytes) to enable forward-compatible skipping.

**Serialisation order (code MUST follow this exactly):**
1. Orchestrator scalars (40 B): gameTick, partyDead, gameWon, partyMapIndex,
   newPartyMapIndex, masterRng.seed, partyIsResting, freezeLifeTicks,
   disabledMovementTicks, projectileDisabledMovementTicks
2. Party state (Phase 10 serialiser)
3. Champions[0..3] (Phase 10 serialiser × 4)
4. Timeline queue (Phase 12 serialiser)
5. Pending combat (Phase 13 serialiser)
6. Magic state (Phase 14 serialiser)
7. Dungeon mutations (Phase 15 serialiser)
8. Creature AI list: `[int32 count] + [CreatureAI × count]` (Phase 16)
9. Projectile list: `[int32 count] + [Projectile × count]` (Phase 17)
10. Explosion list: `[int32 count] + [Explosion × count]` (Phase 17)
11. Active lights: `[int32 count] + [Light × count]` (Phase 19)
12. Fluxcages: `[int32 count] + [Fluxcage × count]` (Phase 19)
13. Generators: `[int32 count] + [Generator × count]` (Phase 19)
14. Dungeon static data (Phase 1–4, raw)
15. Things data (Phase 5–8, raw)
16. Monster groups (Phase 9)
17. Pending sensor effects (Phase 11)
18. Save header (Phase 15)

```
F0898_WORLD_Deserialize_Compat(
    GameWorld_Compat* world,
    const unsigned char* buf,
    int bufSize,
    int* outBytesRead)
    → int (1=ok, 0=corrupt/size mismatch)
```
Inverse of F0897.  Validate section tags.

```
F0899_WORLD_SerializedSize_Compat(
    const GameWorld_Compat* world)
    → int (byte count needed for F0897)
```
Walk all sub-sections and sum sizes.

---

## §4  Algorithm specs

### §4.1  Tick dispatch order (F0884 pseudocode)

```pseudocode
function F0884_ORCH_AdvanceOneTick(world, input, outResult):
    outResult.preTick = world.gameTick
    outResult.emissionCount = 0

    // Step 0: Optional forced RNG advances (for fuzzing)
    for i in 0..input.forcedRngAdvance-1:
        F0730_COMBAT_RngInit(&world.masterRng, world.masterRng.seed)
        // Actually just advance: seed = seed*214013+2531011

    // Step 1: PLAYER INPUT (mirror: keyboard processing in GAMELOOP.C)
    if input.command != CMD_NONE:
        F0888_ORCH_ApplyPlayerInput(world, input, outResult)

    // Step 2: MAP TRANSITION (if triggered by movement/teleporter)
    MAP_TRANSITION_LOOP:
    if world.newPartyMapIndex != -1:
        // Update party map (simplified version of F0003)
        world.partyMapIndex = world.newPartyMapIndex
        world.newPartyMapIndex = -1

    // Step 3: TIMELINE EVENT DISPATCH (mirror: F0261_TIMELINE_Process)
    eventsDispatched = F0887_ORCH_DispatchTimelineEvents(world, outResult)

    // Step 3b: Re-check map transition (Fontanel does goto T0002002)
    if world.newPartyMapIndex != -1:
        goto MAP_TRANSITION_LOOP

    // Step 4: APPLY PENDING DAMAGE AND WOUNDS
    F0889_ORCH_ApplyPendingDamage(world, outResult)

    // Step 5: CHECK PARTY DEAD
    if world.partyDead:
        emit EMIT_PARTY_DEAD
        compute world hash, return -1

    // Step 6: ADVANCE GAME TIME
    world.gameTick++

    // Step 7: PERIODIC EFFECTS
    F0890_ORCH_ApplyPeriodicEffects(world, outResult)

    // Step 8: COMPUTE WORLD HASH
    F0891_ORCH_WorldHash(world, &outResult.worldHashPost)

    outResult.postTick = world.gameTick
    return 1
```

**Fontanel correspondence:**

| Phase 20 step | GAMELOOP.C line   | Fontanel function |
|---------------|-------------------|-------------------|
| Step 1        | ~190 (input loop) | F0361+F0380       |
| Step 2        | ~71 (new map)     | F0003             |
| Step 3        | ~87               | F0261             |
| Step 4        | ~114              | F0320             |
| Step 5        | ~116              | partyDead check   |
| Step 6        | ~122              | G0313++           |
| Step 7        | ~127-150          | F0338+F0331+decr  |

### §4.2  Timeline event dispatch order (F0887 pseudocode)

```pseudocode
function F0887_ORCH_DispatchTimelineEvents(world, result):
    count = 0
    while timeline_peek(world.timeline).fireAtTick <= world.gameTick:
        event = timeline_pop(world.timeline)
        count++
        set_current_map(world, event.mapIndex)
        switch event.kind:
            CREATURE_TICK:
                // Find creature AI state for this group
                tickInput = build_creature_tick_input(world, event)
                tickResult = F0804_CREATURE_Tick(aiState, tickInput, &world.masterRng)
                if tickResult.hasCombatAction:
                    combatResult = F0735_COMBAT_ResolveMelee(...)
                    accumulate_damage(world, combatResult)
                if tickResult.hasMovement:
                    apply_group_move(world, tickResult)
                if tickResult.rescheduleEvent.kind != INVALID:
                    timeline_schedule(world.timeline, tickResult.rescheduleEvent)

            PROJECTILE_MOVE:
                cellDigest = build_cell_digest(world, event)
                projResult = F0811_PROJECTILE_Advance(proj, cellDigest, &world.masterRng)
                apply_projectile_result(world, projResult, result)
                if !projResult.despawned:
                    timeline_schedule(world.timeline, projResult.followUpEvent)

            EXPLOSION_ADVANCE:
                explResult = F0822_EXPLOSION_Advance(expl, world, &world.masterRng)
                apply_explosion_result(world, explResult, result)

            HUNGER_THIRST:
                F0830_LIFECYCLE_HungerThirst(world, &world.masterRng)
                reschedule(HUNGER_THIRST, world.gameTick + cadence)

            STATUS_TIMEOUT:
                F0842_LIFECYCLE_StatusTimeout(world, event)
                // No reschedule (one-shot)

            MOVE_TIMER:
                F0840_LIFECYCLE_MoveTimer(world, event)

            MAGIC_LIGHT_DECAY:
                F0860_DYNAMICS_LightDecay(world, event)

            GROUP_GENERATOR:
                F0862_DYNAMICS_GroupGenerator(world, event, &world.masterRng)

            REMOVE_FLUXCAGE:
                F0864_DYNAMICS_RemoveFluxcage(world, event)

            SPELL_TICK:
                apply_spell_tick(world, event, &world.masterRng)

            DOOR_ANIMATE / DOOR_DESTRUCTION / SQUARE_STATE:
                apply_square_event(world, event, result)

            SENSOR_DELAYED:
                apply_delayed_sensor(world, event, result)

            PLAY_SOUND:
                emit EMIT_SOUND_REQUEST(event.aux0, event.mapX, event.mapY)

            WATCHDOG:
                // no-op (NOCOPYPROTECTION)

        restore_current_map(world, world.partyMapIndex)
    return count
```

**Priority ordering:** Events at the same tick are dispatched in
priority order per Phase 12's `F0234_TIMELINE_IsEventABeforeEventB`:
1. Earlier time first
2. Same time: higher `Type_Priority` value first (higher event type
   number = higher priority in Fontanel's convention)
3. Same time and priority: lower memory address first (→ lower event
   index in our array)

This mirrors the Fontanel assembly (TIMELINE.C lines 135-180) where
`Type_Priority` is a packed word with type in high bits and priority
in low bits. Phase 12's queue already implements this ordering.

### §4.3  World-hash algorithm

```pseudocode
function F0891_ORCH_WorldHash(world, outHash):
    bufSize = F0899_WORLD_SerializedSize(world)
    buf = alloca(bufSize)   // or malloc+free for large worlds
    F0897_WORLD_Serialize(world, buf, bufSize, &bytesWritten)
    *outHash = crc32_ieee(buf, bytesWritten)
    // crc32_ieee: poly 0xEDB88320, init 0xFFFFFFFF, final XOR 0xFFFFFFFF
    // IDENTICAL to Phase 15's CRC32 implementation
```

**Fields excluded:** None.  All fields serialised and hashed.

### §4.4  Determinism proof

Phase 20 is deterministic because:
1. **No floating-point.** All math is `int32_t` / `uint32_t`.
   No `float`, no `double`, no `long double`.  Integer overflow is
   well-defined for unsigned types; signed overflow is avoided by
   design (all values bounded to 16-bit ranges internally).
2. **Single RNG.** The Borland LCG in `RngState_Compat` is the only
   source of non-determinism.  It is advanced in a strict order
   determined by the tick dispatch sequence.  No threading.
3. **No external state.** No system clock, no file reads during tick
   (DUNGEON.DAT loaded at init only), no network, no user input
   other than `TickInput_Compat`.
4. **Defined iteration order.** Timeline events are popped in
   priority order.  All array iterations use index loops with
   fixed bounds.

### §4.5  RNG advance

```pseudocode
function rng_next(rng):
    rng.seed = (rng.seed * 214013) + 2531011   // uint32 wraparound
    return (rng.seed >> 16) & 0x7FFF            // 15-bit result
```

This is the Borland C `rand()` LCG, already used by Phase 13
(`F0730_COMBAT_RngInit_Compat`, `F0732_COMBAT_RngRandom_Compat`).
Phase 20 does NOT add a new RNG; it passes the same
`world->masterRng` to every consumer.

### §4.6  Input encoding (tick-stream text format)

```
# ReDMCSB tick stream — one line per tick
# Format: TICK CMD ARG1 ARG2
# All values in hex, 0-padded
# Lines starting with # are comments
0000 00 00 00    ← tick 0, no input
0001 01 00 00    ← tick 1, move north
0002 00 00 00    ← tick 2, no input
0003 10 00 01    ← tick 3, attack: champion 0, right hand
...
```

Binary stream format (for programmatic use): contiguous array of
16-byte `TickInput_Compat` records.  The text format is for the
headless driver CLI; internally everything uses the binary struct.

### §4.7  State-mutation commit model

Phase 20 v1 uses **immediate mutation** (not deferred delta).
Rationale: Fontanel's original code mutates state in-place during
event dispatch.  Each handler sees the result of the previous
handler.  A deferred-delta model would change semantics.

However, **combat damage** is accumulated into a pending buffer
(mirror of Fontanel's `G_PendingDamage` / `MaxDamageReceived` fields)
and applied in Step 4 (after all timeline events).  This matches
`F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds` being called
after `F0261_TIMELINE_Process`.

### §4.8  Resume-from-save

```pseudocode
function resume_and_verify(world, inputs, totalTicks, resumeAtK):
    // Run straight through
    worldA = clone(world)
    run_N_ticks(worldA, inputs, totalTicks)
    hashA = world_hash(worldA)

    // Run with save/resume at tick K
    worldB = clone(world)
    run_N_ticks(worldB, inputs[0..K-1], K)
    buf = serialize(worldB)
    worldC = deserialize(buf)
    run_N_ticks(worldC, inputs[K..totalTicks-1], totalTicks - K)
    hashC = world_hash(worldC)

    assert hashA == hashC
```

### §4.9  Unclear points → NEEDS DISASSEMBLY REVIEW

1. **Exact timing of HUNGER_THIRST events.**  Phase 18 plans schedule
   them via timeline, but Fontanel's GAMELOOP.C calls
   `F0331_CHAMPION_ApplyTimeEffects` directly (every 64 ticks, NOT
   via timeline).  Phase 20 must reconcile: use the **GAMELOOP.C
   approach** (call F0330 in Step 7 periodic effects) rather than
   scheduling timeline events for this.  Document divergence from
   Phase 18's plan.

2. **`F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived`**
   — this resets the "max damage received" display flag.  Phase 20
   needs the reset logic (clear champion `MaxDamageReceived` field)
   but not the menu refresh.  Verify this doesn't affect game state
   beyond the display flag.

3. **Map transition re-entry.**  GAMELOOP.C does `goto T0002002`
   which re-runs the movement sensor check and timeline dispatch.
   Phase 20 loops until `newPartyMapIndex == -1` but must ensure
   infinite-loop protection (max 4 map transitions per tick).

---

## §5  Invariants (target: 52, gate: ≥30)

### Category A — Struct sizes + round-trip (7)

1. `sizeof(TickInput_Compat) == 16`
2. `sizeof(TickEmission_Compat) == 20`
3. `sizeof(TickStreamRecord_Compat) == 24`
4. `sizeof(GameConfig_Compat) == 64`
5. `TickInput_Compat` round-trip: serialise → deserialise → `memcmp == 0`
6. `TickEmission_Compat` round-trip: serialise → deserialise → `memcmp == 0`
7. `GameConfig_Compat` round-trip: serialise → deserialise → `memcmp == 0`

### Category B — GameWorld serialisation round-trip (3)

8.  `GameWorld_Compat` serialise → deserialise → re-serialise:
    `memcmp(buf1, buf2) == 0` (bit-identical round-trip)
9.  `GameWorld_Compat` serialised size matches `F0899` prediction
10. `GameWorld_Compat` hash is stable: two calls to `F0891` on same
    world yield identical CRC32

### Category C — Construct from DUNGEON.DAT (4)

11. `F0882` with real DUNGEON.DAT returns 1 (success)
12. `world.gameTick == 0` after init
13. `world.partyDead == 0` after init
14. `world.timeline.count > 0` after init (initial events scheduled)

### Category D — Single-tick dispatch order (6)

15. Advance 1 tick with CMD_NONE: `world.gameTick` increments by 1
16. Advance 1 tick: worldHashPost is non-zero
17. Advance 1 tick with CMD_MOVE_NORTH: if tile is walkable,
    emit EMIT_PARTY_MOVED
18. Advance 1 tick with CMD_ATTACK: emit EMIT_DAMAGE_DEALT (if
    creature present in front)
19. Advance 1 tick with timeline events queued: events dispatch in
    priority order (peek pre-dispatch, verify order of emissions)
20. Advance 1 tick: `outResult.preTick + 1 == outResult.postTick`

### Category E — Null-input tick (1)

21. Advance 1 tick with CMD_NONE and no scheduled events at tick 0:
    world state unchanged except `gameTick` incremented

### Category F — Determinism: run twice (3)

22. Run 10 ticks twice with same seed+inputs: every
    `worldHashPost` is bit-identical
23. Run 100 ticks twice: final world hash bit-identical
24. Run 10 ticks, clone, run 10 more on each clone: final hashes
    identical (clone preserves full state)

### Category G — Determinism fuzz (2)

25. 100 random input streams × 10 ticks each: all produce stable
    hashes (run twice, compare)
26. 50 random input streams × 20 ticks: no crash, no assertion
    failure, all return codes valid

### Category H — Save-resume equivalence (3)

27. Run 50 ticks, serialise at tick 25, deserialise, continue to 50:
    final hash == straight-through run hash
28. Run 100 ticks, serialise at tick 1, resume at tick 1: final hash
    matches
29. Run 100 ticks, serialise at tick 99, resume: final hash matches

### Category I — RNG round-trip (1)

30. After N ticks, serialise world → deserialise → advance 1 more
    tick → hash.  Compare with N+1 straight-through.  Identical.
    (Proves RNG state survives serialisation.)

### Category J — Integration per-phase (14)

One invariant per phase (6–19) asserting that phase's state
survives a 10-tick orchestrator run without corruption:

31. Phase 6 (things): thing data accessible after 10 ticks
32. Phase 7 (things cont.): item counts consistent
33. Phase 8 (things cont.): container contents reachable
34. Phase 9 (monsters): monster group list length unchanged if no
    kills
35. Phase 10 (movement): party position valid coordinates
36. Phase 11 (sensors): sensor effect list count in bounds
37. Phase 12 (timeline): `timeline.count >= 0 && <= CAPACITY`
38. Phase 13 (combat): RNG seed != 0 after ticks (LCG never reaches 0
    from non-zero seed — actually it can; test seed is non-zero at
    init and after 10 ticks RNG has advanced predictably)
39. Phase 14 (magic): magic state round-trips after 10 ticks
40. Phase 15 (save): full serialise/deserialise after 10 ticks → bit-identical
41. Phase 16 (creature AI): AI state count unchanged if no group spawns/kills
42. Phase 17 (projectile): projectile list count in bounds [0..60]
43. Phase 18 (champion lifecycle): champion HP <= MaxHP for all alive champions
44. Phase 19 (dynamics): light/fluxcage/generator counts in bounds

### Category K — Stream-driven (1)

45. Deterministic 100-tick run with known seed+inputs produces
    expected final hash (hard-coded in probe; generated once by
    running the orchestrator)

### Category L — Loop guard (1)

46. 10 000-tick headless run with all CMD_NONE inputs completes
    without hang (< 5 seconds wall time)

### Category M — Real DUNGEON.DAT end-to-end (2)

47. Load real DM1 DUNGEON.DAT → run 50 ticks → serialise → load →
    continue → run 50 more → final hash reproducible (determinism
    end-to-end)
48. Load real DM1 DUNGEON.DAT → run 100 ticks → world serialised size
    is positive and < 1 MiB

### Category N — Boundary / null args (3)

49. `F0884` with NULL world pointer returns 0
50. `F0884` with NULL input pointer returns 0
51. `F0885` with tickCount = 0 returns 0 and does nothing

### Category O — Purity (1)

52. Run F0884 on a cloned world.  Original world's hash unchanged.
    (Proves F0884 doesn't reach through pointers to modify the
    original.)

**Total: 52 invariants.  Gate: ≥30.  Target: 52.**

---

## §6  Implementation order (14 steps)

Every step ends with `gcc -Wall -Wextra -c` (compile check).

### Step 1 — Plan reader (mandatory)

Read ALL 14 prior phase headers to confirm struct names, serialised
sizes, and function signatures match what this plan assumes:

| Phase | Header file                               | Key types to verify |
|-------|-------------------------------------------|---------------------|
| 1–4   | `memory_dungeon_dat_pc34_compat.h`        | `DungeonDatState_Compat`, `DungeonHeader_Compat` |
| 5–8   | `memory_dungeon_dat_pc34_compat.h` (things)| `DungeonThings_Compat` |
| 9     | `memory_dungeon_dat_pc34_compat.h`        | `MonsterGroup_Compat` list type |
| 10    | `memory_champion_state_pc34_compat.h`, `memory_movement_pc34_compat.h` | `PartyState_Compat`, `ChampionState_Compat`, `MovementResult_Compat` |
| 11    | `memory_sensor_execution_pc34_compat.h`   | `SensorEffectList_Compat` |
| 12    | `memory_timeline_pc34_compat.h`           | `TimelineQueue_Compat`, `TimelineEvent_Compat`, `F0720-F0728` |
| 13    | `memory_combat_pc34_compat.h`             | `RngState_Compat`, `CombatAction_Compat`, `CombatResult_Compat`, `F0730-F0747` |
| 14    | `memory_magic_pc34_compat.h`              | `MagicState_Compat`, `SpellEffect_Compat`, `F0750-F0769` |
| 15    | `memory_savegame_pc34_compat.h`           | `SaveGameHeader_Compat`, `DungeonMutationList_Compat`, `F0770-F0789` |
| 16    | `memory_creature_ai_pc34_compat.h`        | `CreatureAIState_Compat`, `CreatureTickInput_Compat`, `F0790-F0809` |
| 17    | `memory_projectile_pc34_compat.h`         | `ProjectileInstance_Compat`, `ExplosionInstance_Compat`, `F0810-F0829` |
| 18    | `memory_champion_lifecycle_pc34_compat.h` | Phase 18 types, `F0830-F0859` |
| 19    | `memory_runtime_dynamics_pc34_compat.h`   | `ActiveLightInstance_Compat`, `FluxcageInstance_Compat`, `GeneratorInstance_Compat`, `F0860-F0879` |

**If any mismatch:** adjust §2/§3 of THIS plan before proceeding.
Do NOT modify prior phase headers.

### Step 2 — Latent-bug triage

Run all 19 prior probes via `run_firestaff_m10_verify.sh`.  Confirm
ALL 19 pass.  If any fail, STOP and investigate — Phase 20 cannot
build on a broken foundation.

### Step 3 — Header skeleton

Write `memory_tick_orchestrator_pc34_compat.h`:
- All `#include` for 14 prior headers
- All struct definitions from §2
- All function declarations from §3
- All `#define` constants (commands, emissions, capacities)
- `_Static_assert` for struct sizes

**Compile check:** `gcc -Wall -Wextra -c` a dummy `.c` that includes
the header.

### Step 4 — GameWorld construct/destruct/clone (F0880–F0883)

Implement `F0880`, `F0881`, `F0882`, `F0883`, `F0880b` in
`memory_tick_orchestrator_pc34_compat.c`.

- `F0882` calls Phase 1–4 dungeon loader, Phase 5–8 thing loader,
  Phase 9 monster loader.  Schedule initial timeline events.
- `F0880b` deep-clones by serialise-then-deserialise (ensures
  bit-identical — slow but correct; optimise later if needed).

**Compile check.**

### Step 5 — GameWorld serialise/deserialise (F0897–F0899)

Implement in the order from §2.1 serialisation table.  This is
the largest single step: composes 14 prior serialisers.

- Write each section with `[tag u32][size u32][payload]`.
- Deserialise validates tags + sizes.
- `F0899` computes total without writing.

**Compile check.**

### Step 6 — World-hash + CRC32 (F0891)

Reuse Phase 15's CRC32 implementation (call it, don't copy).
`F0891` calls `F0897` into a temp buffer then CRC32s it.

**Compile check.**

### Step 7 — Tick orchestrator core (F0884, F0887–F0890)

The heart of Phase 20.  Implement §4.1 pseudocode:
- `F0888` (player input) — command dispatch
- `F0887` (timeline dispatch) — event loop with handler switch
- `F0889` (pending damage)
- `F0890` (periodic effects)
- `F0884` (single-tick wrapper)

Each handler call is a thin wrapper around the relevant phase's
exported function.  The orchestrator is PURE GLUE — no game logic.

**Compile check.**

### Step 8 — N-tick runners (F0885, F0886)

Thin loops around F0884.  F0886 adds the condition predicate.

**Compile check.**

### Step 9 — Determinism verification (F0892, F0893)

Implement using `F0880b` (clone) + `F0885` (run N) + `F0891` (hash).

**Compile check.**

### Step 10 — Headless driver primitives (F0894–F0896)

`F0894`: parse text-format tick stream.
`F0895`: wrapper around `F0885`.
`F0896`: print summary to FILE*.

**Compile check.**

### Step 11 — Headless driver binary

Write `firestaff_headless_driver.c`:
```
main(argc, argv):
    parse args: --dungeon PATH --stream PATH [--seed N]
    F0882_WORLD_InitFromDungeonDat(dungeonPath, seed, &world)
    F0894_DRIVER_LoadTickStream(streamPath, &inputs, &count)
    F0895_DRIVER_RunStream(&world, inputs, count, NULL, &hash)
    F0896_DRIVER_WriteSummary(&world, hash, count, stdout)
    return 0
```

Compile as a standalone binary:
`gcc -Wall -Wextra -o firestaff_headless *.c`

### Step 12 — Probe (52 invariants)

Write `firestaff_m10_tick_orchestrator_probe.c` with all 52
invariants from §5.  Output `tick_orchestrator_probe.md` and
`tick_orchestrator_invariants.md` with trailing `Status: PASS`.

Write `run_firestaff_m10_tick_orchestrator_probe.sh`.

Run probe.  Fix any failures.

### Step 13 — Verify script update

Append **exactly one** `# Phase 20:` block to
`run_firestaff_m10_verify.sh`.

Pre-grep: `grep -c '# Phase 20:' run_firestaff_m10_verify.sh` → 0.
Post-append: → 1.

### Step 14 — Full verification

Run `run_firestaff_m10_verify.sh` end-to-end.  All 20 phases PASS.

Check:
- `grep -c '# Phase 20:' run_firestaff_m10_verify.sh` → 1
- All prior phases still have exactly 1 block each
- Zero `-Wall -Wextra` warnings
- No orphan backup directories
- Determinism invariants PASS
- Real DUNGEON.DAT end-to-end PASS
- Headless driver binary runs and produces output

---

## §7  Files

| File | Est. size | Notes |
|------|-----------|-------|
| `memory_tick_orchestrator_pc34_compat.h` | ~18 KB | Aggregate header: all includes, all structs, all function decls |
| `memory_tick_orchestrator_pc34_compat.c` | ~42 KB | Dispatch logic, world serialise, world-hash, resume, all handlers |
| `firestaff_m10_tick_orchestrator_probe.c` | ~30 KB | 52 invariants, DUNGEON.DAT loader, stream builder |
| `run_firestaff_m10_tick_orchestrator_probe.sh` | ~0.9 KB | Compile + run probe |
| `firestaff_headless_driver.c` | ~8 KB | CLI entry: parse args, load, run, print summary |
| `run_firestaff_headless_driver.sh` | ~0.5 KB | Build + run headless driver |
| `run_firestaff_m10_verify.sh` | +34 lines | Phase 20 block appended |

**Total new code:** ~100 KB (excluding verify-script delta).

---

## §8  Risks (top 2)

### R5 — 14 integration invariants surface latent bugs in prior phases

**Likelihood:** High.  Phase 20 is the first module that exercises
all 19 prior phases together in a running loop.  Interactions
between phases that were never tested (e.g., creature AI scheduling
a projectile that triggers a sensor that reschedules a group
generator) are likely to expose edge cases.

**Mitigation:**
1. Step 2 (latent-bug triage) runs all 19 probes before Phase 20
   code is written.
2. Integration invariants (§5 Category J) test each phase's state
   survival independently.
3. If a latent bug surfaces: document it, add a KNOWN_BUG comment
   in the probe, and gate on whether the bug affects determinism.
   Determinism-breaking bugs block Phase 20; display-only or
   edge-case bugs are documented and deferred.

### R3 — Dispatch-order ambiguity in Fontanel MAIN.C / GAMELOOP.C

**Likelihood:** Medium.  The exact interleaving of champion time
effects (F0331) vs timeline events is partially ambiguous: Fontanel
calls `F0331` **after** `G0313_ul_GameTime++` and **conditionally**
(every 64 ticks / 16 if resting).  Phase 18 may have modeled this
as a timeline event instead of a periodic call.  If so, Phase 20
must reconcile.

**Mitigation:**
1. Step 1 (plan reader) verifies how Phase 18 exposes champion
   time effects.
2. §4.9 documents the two unclear points with NEEDS DISASSEMBLY
   REVIEW markers.
3. The reconciliation rule: **GAMELOOP.C wins** over Phase 18's
   plan if they conflict.  F0331 is called from F0890 (Step 7
   periodic effects), NOT as a timeline event.  If Phase 18
   exported it as a timeline handler, Phase 20 wraps it as a
   periodic call instead.

---

## §9  Acceptance criteria

- [ ] All 20 phases PASS in `run_firestaff_m10_verify.sh`
- [ ] `grep -c '# Phase 20:' run_firestaff_m10_verify.sh` → exactly 1
- [ ] All prior phases: `grep -c '# Phase N:'` → exactly 1 each
- [ ] ≥30 invariants (target 52), `Status: PASS` in
      `tick_orchestrator_invariants.md`
- [ ] Zero `-Wall -Wextra` warnings across all Phase 20 files
- [ ] No orphan backup directories
- [ ] Determinism invariants PASS:
  - F0892 (run-twice) with 100 ticks: bit-identical
  - Fuzz test: 100 random streams × 10 ticks: stable hashes
- [ ] Save-resume equivalence PASS (F0893)
- [ ] Real DUNGEON.DAT end-to-end invariant PASS
- [ ] Headless driver binary compiles, runs, and produces
      deterministic output
- [ ] RNG state round-trips through serialisation
- [ ] `GameWorld_Compat` serialise/deserialise bit-identical round-trip
