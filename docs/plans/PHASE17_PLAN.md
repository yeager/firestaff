# PHASE 17 PLAN — Projectile & Explosion Flight System (v1)

Firestaff M10 milestone, Phase 17. Assumed starting state: 16 phases
PASS (Phase 16 creature AI merged before Phase 17 executes). This
document is the *single source of truth* the Codex ACP executor
follows. Any deviation = abort and ask.

Style rules (non-negotiable, inherited from Phases 10–16):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation. Every new struct round-trips
  bit-identical.
- Pure functions: NO globals, NO UI, NO IO. The projectile-move
  handler is a pure transformation:
  ```
  F0811_PROJECTILE_Advance_Compat(projectileIn, cellDigest, rng) ->
      (new projectile state OR despawn flag,
       0..1 CombatAction_Compat emission,
       0..1 ExplosionInstance_Compat creation,
       0..1 DOOR_DESTRUCTION follow-up event,
       1 follow-up TimelineEvent_Compat OR terminal-no-reschedule)
  ```
  Randomness flows through Phase 13's explicit `RngState_Compat*`.
- Function numbering: **F0810 – F0829** (20 slots, Phase 17 range).
- Probe emits `projectile_probe.md` + `projectile_invariants.md`
  with a trailing `Status: PASS` line.
- Verify gate: `run_firestaff_m10_verify.sh` gets **exactly one** new
  `# Phase 17:` block appended. Pre-grep and post-grep mandatory.
- ADDITIVE ONLY: zero edits to Phase 9/10/11/12/13/14/15/16 source.
  Phase 17 consumes their interfaces via `#include` and pure
  composition.

Fontanel primary references (PROJEXPL.C):

- `F0212_PROJECTILE_Create` (PROJEXPL.C:43) — launch; schedules
  C48 event (first-tick grace) or C49 event (launcher sensors).
- `F0213_EXPLOSION_Create` (PROJEXPL.C:95) — explosion spawn;
  handles fireball/lightning AoE-on-creation and schedules C25.
- `F0217_PROJECTILE_HasImpactOccured` (PROJEXPL.C:350) — impact
  resolution on door / champion / creature / wall / stairs.
- `F0218_PROJECTILE_GetImpactCount` (PROJEXPL.C:612) — iterates
  projectile list on a square.
- `F0219_PROJECTILE_ProcessEvents48To49_Projectile` (PROJEXPL.C:644)
  — per-tick move handler (primary port target).
- `F0220_EXPLOSION_ProcessEvent25_Explosion` (PROJEXPL.C:765) —
  per-tick explosion handler (primary port target).
- `F0224_GROUP_FluxCageAction` (PROJEXPL.C:~985) — fluxcage spawn.
- `F0221_GROUP_IsFluxcageOnSquare` (PROJEXPL.C:~880) — fluxcage
  detection on a square.

---

## 1. Scope definition

### In scope for Phase 17 v1

1. **PROJECTILE_MOVE event handler as a pure state-transform.**
   Given a `ProjectileInstance_Compat` (owner, type, position,
   direction, kineticEnergy, attack, stepEnergy, firstMoveGraceFlag)
   plus a `CellContentDigest_Compat` (pre-baked by caller: wall/door/
   stairs/fakewall/pit state at source + destination cells, creature
   group presence, party presence, champion-cell-mask, fluxcage
   present, other-projectile present), produce a
   `ProjectileTickResult_Compat` (new projectile state OR despawn
   flag, ≤1 `CombatAction_Compat`, ≤1 `ExplosionInstance_Compat`,
   ≤1 follow-up `TimelineEvent_Compat` of kind DOOR_DESTRUCTION or
   PROJECTILE_MOVE, terminal-despawn flag).
2. **Kinetic projectile flight** for thrown items. Mirror of F0219
   motion step + F0217 impact paths for weapon/junk/potion slots.
   Moves one intra-cell step per tick (cell flip within square) and
   one full-cell step every two ticks, matching the
   even-cell / odd-cell Fontanel parity — see §4.1.
3. **Magical projectile flight** for fireball, lightning bolt,
   poison bolt, poison cloud (traveling phase), open-door spell,
   harm-non-material, slime. Same motion primitive; explosion-
   on-impact predicate differs.
4. **Collision** against: wall (C00), closed door (C04), creature
   group, champion, fluxcage (C050 explosion thing on square),
   pit boundary, other projectile (§4.6), open floor (no collision).
   Stairs-into-stairs is a wall-equivalent collision (Fontanel
   PROJEXPL.C:728 branch).
5. **Hit resolution** — emit one of:
    - `CombatAction_Compat` with `kind=COMBAT_ACTION_APPLY_DAMAGE_CHAMPION`
      for champion hits (Phase 13 F0735's existing APPLY_DAMAGE_CHAMPION
      path consumes unchanged); attacker = projectile ownerId,
      attack value = `F0816_PROJECTILE_GetImpactAttack_Compat`
      (mirror of F0216 in Fontanel, ported as a local pure helper).
    - `CombatAction_Compat` with `kind=COMBAT_ACTION_APPLY_DAMAGE_GROUP`
      for creature hits (Phase 13 F0736 / or its resolver equivalent
      consumes unchanged).
    - `TimelineEvent_Compat` with `kind=TIMELINE_EVENT_DOOR_DESTRUCTION`
      for door hits — Phase 12 already reserves slot 10 for this.
    - `ExplosionInstance_Compat` on magical-projectile impact (sets
      `outResult.createsExplosion = 1` + populated struct).
    - Nothing on wall / fluxcage — both just despawn.
6. **Explosion creation** — port of F0213. Phase 17 emits the
   explosion *data*; the EXPLOSION_ADVANCE tick actually deals AoE
   damage. Fireball / lightning fast-path (immediate cross-cell
   AoE during F0213) is v1 folded into the first-tick explosion
   advance — see §4.4.
7. **EXPLOSION_ADVANCE event handler as a pure state-transform.**
   Given an `ExplosionInstance_Compat` + `CellContentDigest_Compat`,
   produce an `ExplosionTickResult_Compat`: 0..N `CombatAction_Compat`
   emissions (≤1 for champion, ≤1 for creature group on the
   square), new explosion state (attack-level decreased, frame
   advanced), reschedule decision.
8. **Explosion per-type semantics** (§4.4):
    - `FIREBALL` / `LIGHTNING_BOLT`: one damaging tick, then done.
    - `POISON_CLOUD`: persists while `attack >= 6`, emits damage
      each tick, decrements attack by 3, reschedules at +1 tick.
    - `SMOKE`: persists while `attack > 55`, decrements by 40,
      reschedules at +1 tick. No damage emitted.
    - `HARM_NON_MATERIAL`: one tick, damages non-material
      creatures only (flag fed in via digest).
    - `REBIRTH_STEP1`: converts to step2 in 5 ticks.
    - `OPEN_DOOR`, `SLIME`, `POISON_BOLT`: not emitted by
      explosion handler in v1 (these are projectile slot types
      whose impact path creates a different explosion).
    - `FLUXCAGE` / `REBIRTH_STEP2`: persistent, never reschedule
      their own EXPLOSION_ADVANCE in v1 (removal driven by
      REMOVE_FLUXCAGE which is already a Phase 12 event kind —
      Phase 17 does NOT touch it, only emits the initial
      REMOVE_FLUXCAGE event when the fluxcage is *created* as an
      explosion-on-impact side effect of a hypothetical future
      ZO spell; v1 does not emit fluxcage at all, kept here as
      data-only).
9. **AoE distribution** — the Fontanel model is NOT a radial
   falloff. Fireball damages *one* cell (projectile target);
   F0213 additionally damages the *source* cell (where projectile
   was last) at half attack. v1 ports exactly that two-cell model,
   NOT a 3×3 radial splash. Damage falloff per Fontanel:
   `attack_source = (attack >> 1) + 1 + rand(attack >> 1)`;
   lightning halves again. Cited PROJEXPL.C:169-175.
10. **Projectile lifespan** — decrements `kineticEnergy` by
    `stepEnergy` on each resolved cross-cell tick. When
    `kineticEnergy <= stepEnergy` at tick start → despawn (no
    impact). Mirrors PROJEXPL.C:697-702.
11. **First-tick grace** — projectile launched by champion/creature
    skips impact on first `PROJECTILE_MOVE` (event kind C48 in
    Fontanel). Launcher-sensor projectiles skip this (C49). v1
    encodes this as a single `firstMoveGraceFlag` on the instance;
    set 1 on creation by champion/creature, 0 by launcher sensor.
    Cleared after first advance.
12. **Projectile-vs-projectile** — when two projectile instances
    occupy the same cell on the same tick, both despawn with no
    damage (Fontanel convention, see risk R5). Detection via
    `CellContentDigest_Compat.otherProjectileOnCell` flag
    supplied by caller.
13. **Fluxcage absorption** — fluxcage on destination cell is
    treated as a wall-equivalent for projectile motion. Projectile
    despawns; fluxcage duration is **unchanged** (no damage
    transfer). Cited §4.7 / PROJEXPL.C fluxcage handling: no
    code path decrements fluxcage attack from projectile impact.
14. **Serialisation** — `ProjectileInstance_Compat`,
    `ExplosionInstance_Compat`, `ProjectileList_Compat` (fixed
    capacity 60, matching BUG0_16 note in PROJEXPL.C:50),
    `ExplosionList_Compat` (capacity 32), all round-trip
    bit-identical.
15. **Integration hooks** — a real F0212-equivalent launch from a
    known Level-1 party start fires a projectile at a known wall
    distance and correctly despawns (invariant 46).
16. **Loop-guard** — every `PROJECTILE_MOVE` result **either**
    sets `outResult.despawn = 1` and emits no reschedule, **or**
    emits exactly one `TIMELINE_EVENT_PROJECTILE_MOVE` with
    `fireAtTick - currentTick >= 1`. Enforced by invariant 48.

### Explicitly OUT of scope for Phase 17 v1

Deferred (tag `NEEDS DISASSEMBLY REVIEW` or `v2`):

- **Projectile arc / ballistic motion** — DM1 is grid-aligned;
  no arcs needed ever.
- **Smooth inter-cell animation** — rendering concern. Phase 17
  owns *positions* at tick granularity only.
- **Sound effects** — Fontanel F0213/F0217 play many sounds.
  Phase 17 records `out->soundRequestCode` but never plays.
- **Particle FX / explosion frames beyond count** —
  `ExplosionInstance_Compat.currentFrame` advances an integer;
  visual frame table is post-M10.
- **Homing / DM2 zokathra / footprints** — DM2-only, skip.
- **Projectile reflection** — DM1 has no mirror.
- **Launcher-sensor first-tick immediate-impact** beyond flag
  plumbing. Phase 11 already owns launcher sensor *creation*;
  Phase 17 just honours the `firstMoveGraceFlag = 0` entry
  point.
- **Passing-through-door random-roll for thrown items** (mask
  `MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS` branch, PROJEXPL.C:
  490-500). v1 treats closed door as always-blocking for kinetic
  projectiles. `NEEDS DISASSEMBLY REVIEW` tag on the simplification
  — invariants 27/28 flag this branch as deferred.
- **Sharp-weapon absorption by certain creatures** (KEEP_THROWN_-
  SHARP_WEAPONS attribute, PROJEXPL.C:572-590). v1 does not
  restore the weapon to the creature's inventory; the projectile
  just despawns. `NEEDS DISASSEMBLY REVIEW`.
- **BUG0_16 projectile-thing exhaustion** (60-projectile global
  cap) — v1 enforces hard cap via `PROJECTILE_LIST_CAPACITY=60`
  and rejects on overflow without the Fontanel "steal distant
  projectile" logic.
- **BUG0_17 fuse-action on map-boundary wall** — not touched by
  Phase 17 (Fuse is a champion action, out of scope).
- **F0213 fireball/lightning cross-cell immediate AoE** from
  creator side — v1 folds this into the first EXPLOSION_ADVANCE
  tick instead of emitting during creation. Behaviour difference
  is one-tick later than Fontanel but in the same frame for data
  purposes; `NEEDS DISASSEMBLY REVIEW` tag with cite.
- **Poison cloud re-entry damage on party walk-through** — that's
  a movement-sensor concern (Phase 11), not Phase 17.
- **Rebirth step 1→2 conversion timing** — stubbed at 5-tick
  reschedule, but the step2 animation is visual-only and
  Phase 17 just keeps the explosion instance alive for the
  5-tick window.
- **Multi-target homing projectiles** — DM has none.
- **DM2 spell types** (zokathra, footprints) — DM2-only.

### Phase 13/14/15/16 REVIEW markers touched by Phase 17

| # | Marker | Status after Phase 17 |
|---|--------|-----------------------|
| Phase 13 #4 (CombatAction for projectile champion-hit) | partial | v1 emits APPLY_DAMAGE_CHAMPION; projectile-specific wound mask (HEAD|TORSO only, PROJEXPL.C:550) honoured. |
| Phase 13 #5 (cell/direction repack on creature kill) | still deferred | unchanged. |
| Phase 14 "spells produce SpellEffect without flight" | CLOSED by Phase 17 | fireball/lightning/poison-bolt SpellEffect -> F0810 create projectile. |
| Phase 14 poison-tick follow-through | CLOSED by Phase 17 | poison cloud EXPLOSION_ADVANCE emits APPLY_DAMAGE_GROUP each tick. |
| Phase 15 save-blob completeness | still deferred | v1 ships standalone ser/deser for projectile + explosion lists; Phase 15 composite is NOT touched. Integration invariants 41-42 test standalone round-trip only. Adding a slot to `SaveGame_Compat` is v2. |
| Phase 16 `CombatAction` emission path | CLOSED by Phase 17 | projectile→creature hit uses same `CombatAction_Compat` struct Phase 16 already emits; F0736 consumes identical layout. |

No Phase 17 dependency REVERSES an earlier deferral.

---

## 2. Data structures

All sizes are multiples of 4. Each field is `int32_t`-equivalent.
`_Static_assert(sizeof(int) == 4, ...)` sits at the top of the `.c`,
mirroring Phases 13–16.

### 2.1  `ProjectileInstance_Compat`

```
struct ProjectileInstance_Compat {
    int slotIndex;                /* 0..59, -1 = empty                     */
    int projectileCategory;       /* PROJECTILE_CATEGORY_KINETIC /
                                     PROJECTILE_CATEGORY_MAGICAL           */
    int projectileSubtype;        /* mirror of PROJECTILE.Slot semantic:
                                     0..C15 = kinetic thing type,
                                     0x80..0x87 magical (FIREBALL..POISON_CLOUD),
                                     0xA8 SMOKE, 0xE4 REBIRTH_STEP1        */
    int ownerKind;                /* OWNER_CHAMPION / OWNER_CREATURE /
                                     OWNER_LAUNCHER                        */
    int ownerIndex;               /* championIndex (0..3) OR
                                     groupSlotIndex OR launcherSensorId    */
    int mapIndex;
    int mapX;
    int mapY;
    int cell;                     /* 0..3, intra-square                    */
    int direction;                /* 0..3                                  */
    int kineticEnergy;            /* 0..255; on-hit attack value base      */
    int attack;                   /* 0..255; decrements with stepEnergy   */
    int stepEnergy;               /* 0..255; decrement per advance        */
    int firstMoveGraceFlag;       /* 1 = next advance skips impact        */
    int launchedAtTick;           /* uint32_t widened                      */
    int scheduledAtTick;          /* fireAtTick of the next MOVE event    */
    int associatedPotionPower;    /* POTION.Power for potion projectiles,
                                     else 0                                */
    int poisonAttack;             /* mirror of G0366; 0 for non-poison     */
    int attackTypeCode;           /* C0_ATTACK_NORMAL..C2_ATTACK_FIRE      */
    int flags;                    /* bit0 = removePotionOnImpact,
                                     bit1 = createsExplosionOnImpact,
                                     bit2 = ignoreDoorPassThrough (v1=1), */
    int reserved0;
    int reserved1;
    int reserved2;
    int reserved3;
};
```

- 24 int32 → `PROJECTILE_INSTANCE_SERIALIZED_SIZE = 96`.
- Target was ≤256 B; this is **96 B** (headroom for v2 fields).

Owner / category enums (stable forever — save-blob contract):

```
#define PROJECTILE_CATEGORY_KINETIC          0
#define PROJECTILE_CATEGORY_MAGICAL          1

#define PROJECTILE_OWNER_CHAMPION            0
#define PROJECTILE_OWNER_CREATURE            1
#define PROJECTILE_OWNER_LAUNCHER            2

#define PROJECTILE_SUBTYPE_FIREBALL         0x80
#define PROJECTILE_SUBTYPE_SLIME            0x81
#define PROJECTILE_SUBTYPE_LIGHTNING_BOLT   0x82
#define PROJECTILE_SUBTYPE_HARM_NON_MATERIAL 0x83
#define PROJECTILE_SUBTYPE_OPEN_DOOR        0x84
#define PROJECTILE_SUBTYPE_POISON_BOLT      0x86
#define PROJECTILE_SUBTYPE_POISON_CLOUD     0x87
#define PROJECTILE_SUBTYPE_SMOKE            0xA8
#define PROJECTILE_SUBTYPE_REBIRTH_STEP1    0xE4
#define PROJECTILE_SUBTYPE_FLUXCAGE         0x32  /* C050, ported as data only */
```

### 2.2  `ExplosionInstance_Compat`

```
struct ExplosionInstance_Compat {
    int slotIndex;             /* 0..31, -1 = empty                  */
    int explosionType;         /* C000_EXPLOSION_FIREBALL..
                                  C101_EXPLOSION_REBIRTH_STEP2       */
    int mapIndex;
    int mapX;
    int mapY;
    int cell;                  /* 0..3; 0xFF if centered             */
    int centered;              /* 1 if created via
                                  C0xFF_SINGLE_CENTERED_CREATURE     */
    int attack;                /* mirror of EXPLOSION.Attack (0..255)*/
    int currentFrame;          /* 0..maxFrames                       */
    int maxFrames;             /* profile-table lookup               */
    int poisonAttack;          /* 0 for non-poison                   */
    int scheduledAtTick;       /* next EXPLOSION_ADVANCE tick        */
    int ownerKind;             /* carried over from projectile       */
    int ownerIndex;
    int creatorProjectileSlot; /* source projectile slotIndex, -1 if
                                  none (direct spell cast)           */
    int reserved0;
};
```

- 16 int32 → `EXPLOSION_INSTANCE_SERIALIZED_SIZE = 64`.
- Target was ≤128 B; this is **64 B**.

### 2.3  `ProjectileList_Compat` / `ExplosionList_Compat`

```
#define PROJECTILE_LIST_CAPACITY   60
#define EXPLOSION_LIST_CAPACITY    32

struct ProjectileList_Compat {
    int count;                  /* 0..60                              */
    int reserved;
    struct ProjectileInstance_Compat entries[PROJECTILE_LIST_CAPACITY];
};

struct ExplosionList_Compat {
    int count;                  /* 0..32                              */
    int reserved;
    struct ExplosionInstance_Compat entries[EXPLOSION_LIST_CAPACITY];
};
```

- `PROJECTILE_LIST_SERIALIZED_SIZE = 8 + 60 * 96 = 5768` bytes.
- `EXPLOSION_LIST_SERIALIZED_SIZE  = 8 + 32 * 64 = 2056` bytes.

### 2.4  `CellContentDigest_Compat` (pure input to Phase 17)

Caller pre-bakes Phase 6/9 tile + thing-list data into this flat
digest. Phase 17 performs NO DUNGEON.DAT reads in the per-tick
path. Only invariant 46 opens DUNGEON.DAT once, and only in the
probe.

```
struct CellContentDigest_Compat {
    /* Source cell = where projectile currently sits */
    int sourceMapIndex;
    int sourceMapX;
    int sourceMapY;
    int sourceSquareType;      /* C00_ELEMENT_WALL..C06_ELEMENT_FAKEWALL */
    int sourceHasFluxcage;
    int sourceHasOtherProjectile;

    /* Destination cell = source + direction step */
    int destMapIndex;
    int destMapX;
    int destMapY;
    int destSquareType;
    int destFakeWallIsImaginaryOrOpen;   /* MASK0x0001 | MASK0x0004 */
    int destHasFluxcage;
    int destHasOtherProjectile;
    int destHasChampion;                 /* party on destination tile */
    int destPartyDirection;              /* 0..3, which cells hold which champions */
    int destChampionCellMask;            /* bit i set iff champion on cell i */
    int destHasCreatureGroup;
    int destCreatureType;                /* 0..26, -1 if none         */
    int destCreatureCellMask;            /* bit i set iff creature on cell i */
    int destCreatureIsNonMaterial;       /* MASK0x0040                */
    int destDoorState;                   /* DOOR_STATE_OPEN..CLOSED_FULL..DESTROYED; -1 if not door */
    int destDoorAllowsProjectilePassThrough; /* MASK0x0002              */
    int destDoorIsDestroyed;
    int destIsMapBoundary;               /* 1 if dest is off-map (clamp boundary) */
    int reserved0;
    int reserved1;
};
```

- 25 int32 → `CELL_CONTENT_DIGEST_SERIALIZED_SIZE = 100`.

### 2.5  `ProjectileTickResult_Compat`

```
struct ProjectileTickResult_Compat {
    int resultKind;               /* PROJECTILE_RESULT_*              */
    int despawn;                  /* 1 = remove from list             */
    int crossedCell;              /* 1 = moved to neighbouring square */
    int newCell;                  /* 0..3, if !despawn                */
    int newMapIndex;
    int newMapX;
    int newMapY;
    int newDirection;             /* after teleporter rotation        */
    int newKineticEnergy;
    int newAttack;
    int newFirstMoveGraceFlag;    /* cleared to 0 after first advance */
    int emittedCombatAction;      /* 0 or 1                           */
    int emittedExplosion;         /* 0 or 1                           */
    int emittedDoorDestructionEvent; /* 0 or 1                        */
    int emittedSoundCode;         /* C00/M542/C05; 0 = silent         */
    int rngCallCount;
    int reserved0;
    int reserved1;
    struct CombatAction_Compat outAction;         /* 48 B             */
    struct ExplosionInstance_Compat outExplosion; /* 64 B             */
    struct TimelineEvent_Compat outNextTick;      /* 44 B (PROJECTILE_MOVE or DOOR_DESTRUCTION) */
    int outTickPadding;           /* ser envelope, 4 B                */
};
```

Layout:

| Offset | Field | Size |
|-------:|-------|-----:|
| 0x000  | 18 int32 header | 72 |
| 0x048  | outAction | 48 |
| 0x078  | outExplosion | 64 |
| 0x0B8  | outNextTick | 44 |
| 0x0E4  | outTickPadding | 4 |
| **total** | | **232 bytes** |

- `PROJECTILE_TICK_RESULT_SERIALIZED_SIZE = 232`.

Result-kind enum (stable):

```
#define PROJECTILE_RESULT_FLEW             0
#define PROJECTILE_RESULT_HIT_WALL         1
#define PROJECTILE_RESULT_HIT_DOOR         2
#define PROJECTILE_RESULT_HIT_CHAMPION     3
#define PROJECTILE_RESULT_HIT_CREATURE     4
#define PROJECTILE_RESULT_HIT_FLUXCAGE     5
#define PROJECTILE_RESULT_HIT_OTHER_PROJECTILE 6
#define PROJECTILE_RESULT_DESPAWN_ENERGY   7
#define PROJECTILE_RESULT_DESPAWN_BOUNDS   8
#define PROJECTILE_RESULT_INVALID          9
```

### 2.6  `ExplosionTickResult_Compat`

```
struct ExplosionTickResult_Compat {
    int resultKind;                /* EXPLOSION_RESULT_*              */
    int despawn;                   /* 1 = remove from list            */
    int newAttack;
    int newCurrentFrame;
    int emittedCombatActionPartyCount;     /* 0 or 1                   */
    int emittedCombatActionGroupCount;     /* 0 or 1                   */
    int emittedDoorDestructionEvent;       /* 0 or 1                   */
    int rngCallCount;
    int reserved0;
    int reserved1;
    struct CombatAction_Compat outActionParty;   /* 48 B               */
    struct CombatAction_Compat outActionGroup;   /* 48 B               */
    struct TimelineEvent_Compat outNextTick;     /* 44 B               */
    int outTickPadding;                          /* 4 B                */
};
```

Layout:

| Offset | Field | Size |
|-------:|-------|-----:|
| 0x000  | 10 int32 header | 40 |
| 0x028  | outActionParty | 48 |
| 0x058  | outActionGroup | 48 |
| 0x088  | outNextTick | 44 |
| 0x0B4  | outTickPadding | 4 |
| **total** | | **184 bytes** |

- `EXPLOSION_TICK_RESULT_SERIALIZED_SIZE = 184`.

### 2.7  Size-collision check

Phase 17 new constants: `96 / 64 / 5768 / 2056 / 100 / 232 / 184`.

- `64` already used by Phase 14 `SPELL_CAST_REQUEST_SERIALIZED_SIZE`,
  and by Phase 16 `CREATURE_BEHAVIOR_PROFILE_SIZE` (internal only).
  Macro names differ → no `#define` collision.
- `96` is new.
- `100` is new.
- `184`, `232`, `2056`, `5768` are all new.

No `#define` name collision; `_Static_assert` at top of `.c`
covers the sizeof side.

### 2.8  Integration with existing structs (no modifications)

- **Phase 9** `DungeonGroup_Compat` etc.: read-only, via caller-
  pre-baked `CellContentDigest_Compat`. Phase 17 never dereferences
  Phase 9 thing-data pointers.
- **Phase 10** party position/facing: caller packs
  `destHasChampion / destPartyDirection / destChampionCellMask`
  from `PartyState_Compat`. Phase 17 does NOT include the Phase 10
  header (keeps coupling loose).
- **Phase 12** `TimelineEvent_Compat`: emitted unchanged. Kinds:
  `TIMELINE_EVENT_PROJECTILE_MOVE` (3), `TIMELINE_EVENT_EXPLOSION_-
  ADVANCE` (4), `TIMELINE_EVENT_DOOR_DESTRUCTION` (10).
  aux0 = projectile/explosion slotIndex,
  aux1 = ownerKind,
  aux2 = ownerIndex,
  aux3 = subtype/explosionType,
  aux4 = scheduleGeneration counter (for save-load tiebreak).
- **Phase 13** `CombatAction_Compat` (48 B), `RngState_Compat`:
  consumed by F0817-F0820. The emitted action's kind is always
  one of `COMBAT_ACTION_APPLY_DAMAGE_CHAMPION` or
  `COMBAT_ACTION_APPLY_DAMAGE_GROUP`; the existing Phase 13
  resolvers accept without modification.
- **Phase 14** `SpellEffect_Compat`: Phase 17 exposes a
  `F0810_PROJECTILE_CreateFromSpellEffect_Compat` helper that
  reads `effect.spellKind == C2_SPELL_KIND_PROJECTILE_COMPAT`,
  `effect.spellType`, `effect.impactAttack`, `effect.kineticEnergy`,
  `effect.durationTicks` → populates a fresh ProjectileInstance.
  Magic-side does NOT change; Phase 17 is the new consumer.
- **Phase 15** save blob: Phase 17 provides standalone
  ser/deser (F0827-F0829) for its two lists. Phase 15's
  `SaveGame_Compat` is **not modified** in v1. Adding a slot is
  v2 (Phase 17's §1 scope note #14).
- **Phase 16** `CreatureTickResult_Compat`: when Phase 16 emits a
  `SpellCastRequest` (v2 path, currently always 0), Phase 14
  resolves it, and Phase 17's F0810 launches the projectile.
  v1 exercises this via a synthesised creature-attack spell
  (invariant 44) without actually wiring Phase 16 spell-casting.

---

## 3. Function API (F0810 – F0829)

All live in `memory_projectile_pc34_compat.{h,c}`. All pure unless
noted. Conventions inherited from Phase 13:

- Input pointers: `const` always.
- Output pointers: non-`const`, caller-owned.
- Return: 1 on success, 0 on invalid argument / bounds failure.
- RNG draws recorded in `result.rngCallCount`.

### Group A — Projectile lifecycle (F0810 – F0813)

| # | Signature | Role |
|---|-----------|------|
| F0810 | `int F0810_PROJECTILE_Create_Compat(const struct ProjectileCreateInput_Compat* in, struct ProjectileList_Compat* list, int* outSlotIndex, struct TimelineEvent_Compat* outFirstMoveEvent);` | Mirror of `F0212_PROJECTILE_Create`. Allocates a slot in `list` (first-empty scan; returns 0 on full), writes instance, emits a C48/C49 `TIMELINE_EVENT_PROJECTILE_MOVE` event at `currentTick+1`. `ProjectileCreateInput_Compat` = {ownerKind, ownerIndex, mapIndex, mapX, mapY, cell, direction, subtype, kineticEnergy, attack, stepEnergy, currentTick, poisonAttack, attackTypeCode, potionPower, firstMoveGraceFlag}. Pure. |
| F0811 | `int F0811_PROJECTILE_Advance_Compat(const struct ProjectileInstance_Compat* in, const struct CellContentDigest_Compat* digest, uint32_t currentTick, struct RngState_Compat* rng, struct ProjectileInstance_Compat* outNewState, struct ProjectileTickResult_Compat* outResult);` | **PRIMARY ENTRY POINT.** Pure per-tick handler. See §4.1 for the full algorithm. Advances the projectile one tick; emits exactly one result. |
| F0812 | `int F0812_PROJECTILE_CreateFromSpellEffect_Compat(const struct SpellEffect_Compat* effect, int casterChampionIndex, int partyMapIndex, int partyMapX, int partyMapY, int partyDirection, uint32_t currentTick, struct ProjectileList_Compat* list, int* outSlotIndex, struct TimelineEvent_Compat* outFirstMoveEvent);` | Bridge helper for Phase 14. Rejects with 0 when `effect.spellKind != C2_SPELL_KIND_PROJECTILE_COMPAT`. Delegates to F0810 with subtype mapped from `effect.spellType` via a static table. Pure. |
| F0813 | `int F0813_PROJECTILE_Despawn_Compat(struct ProjectileList_Compat* list, int slotIndex);` | Clears slot, decrements count. Pure mutator. Returns 0 if slotIndex out of range or already empty. |

### Group B — Cell-content inspection (F0814 – F0816)

All three are pure predicates over the caller-provided digest.
None reads DUNGEON.DAT.

| # | Signature | Role |
|---|-----------|------|
| F0814 | `int F0814_PROJECTILE_InspectDestination_Compat(const struct CellContentDigest_Compat* digest, int* outBlocker);` | Classifies the destination cell in Fontanel priority order (§4.2). `*outBlocker` ∈ {0=open, 1=wall, 2=stairs, 3=closed-door, 4=fluxcage, 5=boundary, 6=other-projectile}. Pure. |
| F0815 | `int F0815_PROJECTILE_ComputeImpactAttack_Compat(const struct ProjectileInstance_Compat* in, int* outAttack);` | Mirror of `F0216_PROJECTILE_GetImpactAttack`. `attack_base = in->attack`; kinetic: `out = attack`; magical: `out = attack` (lightning/poison-bolt halved by caller at impact time). Pure. |
| F0816 | `int F0816_PROJECTILE_DoesPassThroughDoor_Compat(const struct ProjectileInstance_Compat* in, const struct CellContentDigest_Compat* digest, struct RngState_Compat* rng, int* outPasses);` | Returns 1 with `*outPasses=1` if projectile passes closed door (door destroyed, door state ≤ CLOSED_ONE_FOURTH, or door has MASK0x0002 AND (magical-projectile ≥ HARM_NON_MATERIAL) *OR* (kinetic-projectile-random-roll — DEFERRED in v1, always `*outPasses=0` for kinetic)). RNG only advanced if the magical branch is consulted; v1 keeps kinetic deterministic. Pure w.r.t. rng state. |

### Group C — Collision resolution (F0817 – F0820)

| # | Signature | Role |
|---|-----------|------|
| F0817 | `int F0817_PROJECTILE_BuildHitCreatureAction_Compat(const struct ProjectileInstance_Compat* in, const struct CellContentDigest_Compat* digest, int impactAttack, struct CombatAction_Compat* outAction);` | Fills kind=`COMBAT_ACTION_APPLY_DAMAGE_GROUP`, targetMap* from digest, attackerSlot = ownerIndex (ownerKind=CHAMPION ⇒ champion idx, OWNER_CREATURE ⇒ group slot, OWNER_LAUNCHER ⇒ sensor-id), rawAttackValue = impactAttack, attackTypeCode = in->attackTypeCode, scheduleDelayTicks = 0, allowedWounds = 0. Pure. |
| F0818 | `int F0818_PROJECTILE_BuildHitChampionAction_Compat(const struct ProjectileInstance_Compat* in, const struct CellContentDigest_Compat* digest, int impactAttack, int championIndex, struct CombatAction_Compat* outAction);` | Fills kind=`COMBAT_ACTION_APPLY_DAMAGE_CHAMPION`, targetCell=championIndex-cell, rawAttackValue=impactAttack, allowedWounds = `MASK0x0004_WOUND_HEAD | MASK0x0008_WOUND_TORSO` (PROJEXPL.C:550), attackTypeCode = in->attackTypeCode, defenderSlot = championIndex. Pure. |
| F0819 | `int F0819_PROJECTILE_BuildDoorDestructionEvent_Compat(const struct ProjectileInstance_Compat* in, const struct CellContentDigest_Compat* digest, int impactAttack, uint32_t currentTick, struct RngState_Compat* rng, struct TimelineEvent_Compat* outEvent);` | Mirror of `F0232_GROUP_IsDoorDestroyedByAttack` *invocation* (the resolver itself lives in Phase 11 sensors). Fills kind=`TIMELINE_EVENT_DOOR_DESTRUCTION`, fireAtTick=`currentTick+1`, aux0=impactAttack+`F0732_rng(rng, impactAttack)`, aux1=in->subtype, aux2=in->ownerIndex, map* from digest. Pure w.r.t. rng state. |
| F0820 | `int F0820_PROJECTILE_ResolveCollision_Compat(const struct ProjectileInstance_Compat* in, const struct CellContentDigest_Compat* digest, int impactTarget, uint32_t currentTick, struct RngState_Compat* rng, struct ProjectileTickResult_Compat* outResult);` | Top-level collision dispatch. `impactTarget` = return code from F0814. Writes outResult.resultKind, populates outAction/outExplosion/outNextTick as appropriate. Pure. |

### Group D — Explosion lifecycle (F0821 – F0824)

| # | Signature | Role |
|---|-----------|------|
| F0821 | `int F0821_EXPLOSION_Create_Compat(const struct ExplosionCreateInput_Compat* in, struct ExplosionList_Compat* list, int* outSlotIndex, struct TimelineEvent_Compat* outFirstAdvanceEvent);` | Mirror of `F0213_EXPLOSION_Create` DATA side only (no sound, no immediate cross-cell AoE — v1 defers both to first advance). Schedules first `TIMELINE_EVENT_EXPLOSION_ADVANCE` at `currentTick+1` (or +5 for REBIRTH_STEP1). |
| F0822 | `int F0822_EXPLOSION_Advance_Compat(const struct ExplosionInstance_Compat* in, const struct CellContentDigest_Compat* digest, uint32_t currentTick, struct RngState_Compat* rng, struct ExplosionInstance_Compat* outNewState, struct ExplosionTickResult_Compat* outResult);` | Pure per-tick handler. See §4.4. |
| F0823 | `int F0823_EXPLOSION_ComputeAoE_Compat(const struct ExplosionInstance_Compat* in, const struct CellContentDigest_Compat* digest, int* outAttackApplied);` | Computes damage for this tick per type-specific formula (§4.4, §4.5). For poison-cloud: `out = max(1, min(attack>>5, 4) + rand(rng,2))`; for fireball/lightning: `out = (attack>>1) + 1 + rand(rng, (attack>>1)+1) + 1`, lightning halved. RNG must be caller-owned — non-pure in rng sense. |
| F0824 | `int F0824_EXPLOSION_Despawn_Compat(struct ExplosionList_Compat* list, int slotIndex);` | Mirror of F0813 for explosions. |

### Group E — Integration helpers (F0825 – F0826)

| # | Signature | Role |
|---|-----------|------|
| F0825 | `int F0825_PROJECTILE_ScheduleNextMove_Compat(const struct ProjectileInstance_Compat* in, int onPartyMap, uint32_t currentTick, struct TimelineEvent_Compat* outEvent);` | Reschedules PROJECTILE_MOVE. Delay = 1 if `onPartyMap`, else 3 (Fontanel MEDIA183 branch, PROJEXPL.C line tagged "CHANGE7_20_IMPROVEMENT"). **Hard clamp `delay < 1 ⇒ delay = 1`** (loop-guard). Pure. |
| F0826 | `int F0826_EXPLOSION_ScheduleNextAdvance_Compat(const struct ExplosionInstance_Compat* in, uint32_t currentTick, int forcedDelay, struct TimelineEvent_Compat* outEvent);` | Reschedules EXPLOSION_ADVANCE. `delay = max(1, forcedDelay)` (poison-cloud: +1; rebirth-step1: +5; everything else: no reschedule, caller sets despawn). |

### Group F — Serialisation (F0827 – F0829)

All four serialise via the MEDIA016 / PC LSB-first helper. Local
`static` duplicates of `write_le_int32` / `read_le_int32`, per
Phase-13/14/15/16 precedent.

| # | Signature | Bytes |
|---|-----------|-------|
| F0827a | `int F0827_PROJECTILE_InstanceSerialize_Compat(const struct ProjectileInstance_Compat*, unsigned char*, int bufSize);` | 96 |
| F0827b | `int F0827_PROJECTILE_InstanceDeserialize_Compat(struct ProjectileInstance_Compat*, const unsigned char*, int bufSize);` | 96 |
| F0828a | `int F0828_EXPLOSION_InstanceSerialize_Compat(const struct ExplosionInstance_Compat*, unsigned char*, int bufSize);` | 64 |
| F0828b | `int F0828_EXPLOSION_InstanceDeserialize_Compat(struct ExplosionInstance_Compat*, const unsigned char*, int bufSize);` | 64 |
| F0829a | `int F0829_PROJECTILE_ListSerialize_Compat(const struct ProjectileList_Compat*, unsigned char*, int bufSize);` | 5768 |
| F0829b | `int F0829_PROJECTILE_ListDeserialize_Compat(struct ProjectileList_Compat*, const unsigned char*, int bufSize);` | 5768 |
| F0829c | `int F0829_EXPLOSION_ListSerialize_Compat(const struct ExplosionList_Compat*, unsigned char*, int bufSize);` | 2056 |
| F0829d | `int F0829_EXPLOSION_ListDeserialize_Compat(struct ExplosionList_Compat*, const unsigned char*, int bufSize);` | 2056 |

(Note: Phase 13/14 precedent allows multiple `a/b/c/d` labels under
a single F-number slot when the functions form an obvious ser/deser
pair/quad. 16 distinct `_SERIALIZED_SIZE` constants is fine; only
F-numbers are counted against the F0810-F0829 budget. Eight
serialisation functions + F0810-F0826 = 17 F-numbers + 4 pairs
in F0827-F0829 = **well within 20**.)

Digest + tick-result round-trip: not serialised (pure inputs /
pure outputs; live stack data only).

### DUNGEON.DAT dependency

Per-tick path: **none**. Digest is pure input.

Probe only: invariant 46 opens DUNGEON.DAT via Phase 9's
`F0504_DUNGEON_LoadThingData_Compat` to extract the Level-1 party
start cell + wall distance. Used once, read-only.

---

## 4. Algorithm specifications

### 4.1  F0811 — Projectile advance (one tick)

Reference: `F0219_PROJECTILE_ProcessEvents48To49_Projectile`
(PROJEXPL.C:644-750).

```
pseudocode F0811(in, digest, currentTick, rng, outNewState, outResult):
    if in == NULL or digest == NULL or outResult == NULL: return 0
    zero(outResult)
    *outNewState = *in

    /* (1) First-tick grace.  C48 event upgrades to C49 but does NOT
       check for impacts this tick. Mirror of PROJEXPL.C:689-691. */
    if in->firstMoveGraceFlag:
        outNewState->firstMoveGraceFlag = 0
        /* fall through to motion step, SKIP impact-on-current-cell checks */
        goto MOTION_STEP

    /* (2) Impact on current cell.
           Priority: champion -> creature -> other-projectile.
           Mirror of PROJEXPL.C:693-696. */
    if digest->sourceMapIndex matches party map and party on current tile:
        if digest->destChampionCellMask & (1 << in->cell):
            dispatch = CHAMPION_HIT
            goto RESOLVE
    if digest->destHasCreatureGroup /* applied to source square */:
        if digest->destCreatureCellMask & (1 << in->cell):
            dispatch = CREATURE_HIT
            goto RESOLVE
    if digest->sourceHasOtherProjectile:
        dispatch = OTHER_PROJ
        goto RESOLVE

    /* (3) Lifespan gate. Mirror of PROJEXPL.C:697-702. */
    if in->kineticEnergy <= in->stepEnergy:
        outResult->despawn = 1
        outResult->resultKind = PROJECTILE_RESULT_DESPAWN_ENERGY
        return 1

    /* (4) Decrement energies. Mirror of PROJEXPL.C:706-712. */
    outNewState->kineticEnergy -= in->stepEnergy
    outNewState->attack        -= (in->attack < in->stepEnergy) ? in->attack : in->stepEnergy

MOTION_STEP:
    /* (5) Intra-square cell flip vs inter-square step.
           Fontanel cell-direction parity: projectile crosses the
           square iff its cell bits match the direction.
           Mirror of PROJEXPL.C:714-719. */
    crossesCell = ((in->direction == in->cell) ||
                   ((in->direction + 1) & 3) == in->cell)

    if crossesCell:
        if digest->destSquareType == C00_ELEMENT_WALL
           || (digest->destSquareType == C06_ELEMENT_FAKEWALL
               && !digest->destFakeWallIsImaginaryOrOpen)
           || (digest->destSquareType == C03_ELEMENT_STAIRS
               && digest->sourceSquareType == C03_ELEMENT_STAIRS)
           || digest->destIsMapBoundary:
            dispatch = WALL_HIT
            goto RESOLVE
        if digest->destHasFluxcage:
            dispatch = FLUXCAGE_HIT
            goto RESOLVE

    /* (6) Cell flip within square. Mirror PROJEXPL.C:721-725. */
    if (in->direction & 1) == (in->cell & 1):
        newCell = (in->cell - 1) & 3
    else:
        newCell = (in->cell + 1) & 3
    outNewState->cell = newCell

    if crossesCell:
        /* (7) Apply direction step + pit/teleporter handling.
               Caller pre-computed teleporter outcome into
               digest->destMapIndex / destMapX / destMapY /
               {newDirection in digest is caller-owned override},
               mirror of F0267_MOVE_GetMoveResult_CPSCE. */
        outNewState->mapIndex  = digest->destMapIndex
        outNewState->mapX      = digest->destMapX
        outNewState->mapY      = digest->destMapY
        /* direction may have been rotated by teleporter; caller
           supplies via digest->destTeleporterNewDirection (field
           we add to digest struct — see §2.4 addendum) */
        /* v1 simplification: no teleporter rotation in digest;
           direction preserved.  NEEDS DISASSEMBLY REVIEW. */

        outResult->crossedCell = 1
    else:
        /* Not crossing — did we land on a door? */
        if digest->destDoorState >= 0 && !digest->destDoorIsDestroyed:
            dispatch = DOOR_HIT
            goto RESOLVE

    /* (8) Reschedule next move. */
    outResult->resultKind = PROJECTILE_RESULT_FLEW
    F0825(outNewState, /*onPartyMap=*/digest->destMapIndex==digest->sourceMapIndex,
          currentTick, &outResult->outNextTick)
    outResult->newKineticEnergy = outNewState->kineticEnergy
    outResult->newAttack        = outNewState->attack
    outResult->newCell          = outNewState->cell
    outResult->newMapIndex      = outNewState->mapIndex
    outResult->newMapX          = outNewState->mapX
    outResult->newMapY          = outNewState->mapY
    outResult->newDirection     = outNewState->direction
    outResult->newFirstMoveGraceFlag = outNewState->firstMoveGraceFlag
    return 1

RESOLVE:
    F0820(&outNewState-incoming, digest, dispatch, currentTick, rng, outResult)
    outResult->despawn = 1   /* any hit consumes the projectile */
    return 1
```

`NEEDS DISASSEMBLY REVIEW` tags on:
- The teleporter rotation in step (7). V1 does not rotate; caller
  can pre-rotate via `destTeleporterNewDirection` extension field.
  Add that field to `CellContentDigest_Compat` §2.4 and default
  it to `-1` (no rotation).
- The stairs-from-stairs edge case — PROJEXPL.C:728 treats stairs
  as wall only when source was ALSO stairs. V1 implements this
  exactly.

### 4.2  F0814 — Cell content inspection priority

Reference: PROJEXPL.C:693-745. Priority (highest wins):

1. Champion on destination cell (and destination is party tile)
2. Creature group on destination cell
3. Other projectile on destination cell (projectile-vs-projectile
   annihilation — see R5)
4. Wall / stairs-from-stairs / solid-fakewall → WALL_HIT
5. Fluxcage explosion-thing on destination cell → FLUXCAGE_HIT
6. Closed door on destination cell (non-pass-through) → DOOR_HIT
7. Open floor → 0 (no collision)

v1 NOTE: this priority applies to the **cross-cell** case. The
**intra-cell** case (cell flip within the same square) can still
land on champion/creature on the *source* square — that's
handled by step (2) of F0811 before the motion step.

### 4.3  F0820 — Collision dispatch

```
pseudocode F0820(in, digest, impactTarget, currentTick, rng, out):
    zero(out.outAction, out.outExplosion, out.outNextTick)
    impactAttack = 0
    F0815(in, &impactAttack)

    switch impactTarget:
        case CHAMPION_HIT:
            out->resultKind = PROJECTILE_RESULT_HIT_CHAMPION
            championIndex  = index_from_cell_mask(digest, in->cell)
            F0818(in, digest, impactAttack, championIndex, &out->outAction)
            out->emittedCombatAction = 1
            /* Magical projectile also creates explosion (§4.4 types) */
            if in->projectileCategory == PROJECTILE_CATEGORY_MAGICAL
               && subtype_creates_explosion(in->projectileSubtype):
                populate_explosion(in, digest, &out->outExplosion)
                out->emittedExplosion = 1
        case CREATURE_HIT:
            out->resultKind = PROJECTILE_RESULT_HIT_CREATURE
            if digest->destCreatureIsNonMaterial
               && in->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
                out->resultKind = PROJECTILE_RESULT_FLEW
                out->despawn    = 0
                /* pass-through; reschedule via caller — but v1
                   keeps the despawn path and logs NEEDS DISASSEMBLY REVIEW */
                out->resultKind = PROJECTILE_RESULT_DESPAWN_ENERGY
                out->despawn    = 1
                return 1
            F0817(in, digest, /*attack adjusted by defense=caller*/ impactAttack,
                  &out->outAction)
            out->emittedCombatAction = 1
            if in->projectileCategory == PROJECTILE_CATEGORY_MAGICAL
               && subtype_creates_explosion(in->projectileSubtype):
                populate_explosion(in, digest, &out->outExplosion)
                out->emittedExplosion = 1
        case DOOR_HIT:
            out->resultKind = PROJECTILE_RESULT_HIT_DOOR
            F0816(in, digest, rng, &passes)
            if passes:
                out->resultKind = PROJECTILE_RESULT_FLEW
                out->despawn    = 0
                /* continue motion — but per §4.1, caller re-invokes
                   F0811 next tick, so v1 actually sets FLEW + despawn=0
                   and requires the CALLER to re-enqueue. Simplification:
                   we CLEAR destDoorState in digest and retry once. */
                /* v1 alternative: treat pass-through door as open.
                   NEEDS DISASSEMBLY REVIEW. */
            if in->projectileSubtype == PROJECTILE_SUBTYPE_OPEN_DOOR:
                /* Open-door spell: emit door TOGGLE, no damage */
                build_door_toggle_event(digest, &out->outNextTick)
                out->emittedDoorDestructionEvent = 1
            else:
                F0819(in, digest, impactAttack, currentTick, rng, &out->outNextTick)
                out->emittedDoorDestructionEvent = 1
        case WALL_HIT:
            out->resultKind = PROJECTILE_RESULT_HIT_WALL
            /* No action emitted — explosion if magical subtype */
            if in->projectileCategory == PROJECTILE_CATEGORY_MAGICAL
               && subtype_creates_explosion(in->projectileSubtype):
                populate_explosion(in, digest, &out->outExplosion)
                out->emittedExplosion = 1
        case FLUXCAGE_HIT:
            out->resultKind = PROJECTILE_RESULT_HIT_FLUXCAGE
            /* No action, no explosion, no fluxcage mutation. */
        case OTHER_PROJ_HIT:
            out->resultKind = PROJECTILE_RESULT_HIT_OTHER_PROJECTILE
            /* Both projectiles despawn — caller invokes F0813 on both.
               Phase 17 only reports it for the advancing side. */
        default:
            out->resultKind = PROJECTILE_RESULT_INVALID
            return 0
    out->despawn = 1
    return 1
```

`subtype_creates_explosion` table (mirror of PROJEXPL.C:459
`L0505_B_CreateExplosionOnImpact`):

```
static const int SUBTYPE_CREATES_EXPLOSION[256] = {
    [PROJECTILE_SUBTYPE_FIREBALL]         = 1,
    [PROJECTILE_SUBTYPE_SLIME]            = 0,   /* Fontanel explicit exclusion */
    [PROJECTILE_SUBTYPE_LIGHTNING_BOLT]   = 1,
    [PROJECTILE_SUBTYPE_HARM_NON_MATERIAL]= 1,
    [PROJECTILE_SUBTYPE_OPEN_DOOR]        = 0,
    [PROJECTILE_SUBTYPE_POISON_BOLT]      = 1,   /* MEDIA720 branch */
    [PROJECTILE_SUBTYPE_POISON_CLOUD]     = 1,
    /* all others (kinetic): 0 */
};
```

### 4.4  F0822 — Explosion advance

Reference: `F0220_EXPLOSION_ProcessEvent25_Explosion`
(PROJEXPL.C:765-890). Per-type dispatch:

```
pseudocode F0822(in, digest, currentTick, rng, outNewState, out):
    zero(out)
    *outNewState = *in
    F0823(in, digest, rng, &attackThisTick)

    switch in->explosionType:
        case C000_EXPLOSION_FIREBALL:
        case C002_EXPLOSION_LIGHTNING_BOLT:
            /* damage target cell */
            if digest->destHasChampion:
                build champion action (APPLY_DAMAGE_CHAMPION, fire|elec)
                out->emittedCombatActionPartyCount = 1
            if digest->destHasCreatureGroup:
                adjust by fire/elec resistance (caller supplies adjusted
                  attack via digest — v1 passes raw attack; resistance
                  is Phase 13 concern)
                build group action
                out->emittedCombatActionGroupCount = 1
            if digest->destSquareType == C04_ELEMENT_DOOR:
                F0819(... for lightning/fireball) -> outNextTick
                out->emittedDoorDestructionEvent = 1
            out->despawn = 1
            out->resultKind = EXPLOSION_RESULT_ONE_SHOT
        case C003_EXPLOSION_HARM_NON_MATERIAL:
            if digest->destHasCreatureGroup && digest->destCreatureIsNonMaterial:
                build group action
                out->emittedCombatActionGroupCount = 1
            out->despawn = 1
        case C100_EXPLOSION_REBIRTH_STEP1:
            outNewState->explosionType = C101_EXPLOSION_REBIRTH_STEP2
            F0826(outNewState, currentTick, /*forcedDelay=*/5, &out->outNextTick)
            out->despawn = 0
            out->resultKind = EXPLOSION_RESULT_ADVANCED_FRAME
        case C040_EXPLOSION_SMOKE:
            if in->attack > 55:
                outNewState->attack = in->attack - 40
                F0826(outNewState, currentTick, 1, &out->outNextTick)
                out->newAttack = outNewState->attack
                out->despawn = 0
                out->resultKind = EXPLOSION_RESULT_ADVANCED_FRAME
            else:
                out->despawn = 1
                out->resultKind = EXPLOSION_RESULT_ONE_SHOT
        case C007_EXPLOSION_POISON_CLOUD:
            if digest->destHasChampion:
                build champion action (APPLY_DAMAGE_CHAMPION, normal)
                out->emittedCombatActionPartyCount = 1
            else if digest->destHasCreatureGroup:
                poisonAttack = resistance-adjust(in->poisonAttack, creatureType)
                    /* v1: caller pre-adjusts; we consume raw */
                build group action
                out->emittedCombatActionGroupCount = 1
            if in->attack >= 6:
                outNewState->attack = in->attack - 3
                F0826(outNewState, currentTick, 1, &out->outNextTick)
                out->newAttack = outNewState->attack
                out->despawn = 0
                out->resultKind = EXPLOSION_RESULT_ADVANCED_FRAME
            else:
                out->despawn = 1
                out->resultKind = EXPLOSION_RESULT_ONE_SHOT
        case C050_EXPLOSION_FLUXCAGE:
        case C101_EXPLOSION_REBIRTH_STEP2:
            /* Persistent, no reschedule from Phase 17; removal
               driven by REMOVE_FLUXCAGE (Phase 12 event kind 14). */
            out->despawn = 0
            out->resultKind = EXPLOSION_RESULT_PERSISTENT
        default:
            out->despawn = 1
            out->resultKind = EXPLOSION_RESULT_ONE_SHOT
    outNewState->currentFrame = in->currentFrame + 1
    out->newCurrentFrame = outNewState->currentFrame
    return 1
```

**v1 deliberately does NOT implement radial 3×3 falloff.** DM1
does NOT have radial falloff; explosion damages exactly the cell
where the projectile impacted (F0213 additionally damages the
projectile *source* square for fireball/lightning, handled by
`F0821_EXPLOSION_Create_Compat` when the creator passes the
source-cell digest alongside — treated as an optional "paired
explosion" emitted from the creator step). `NEEDS DISASSEMBLY
REVIEW` tag on the paired-explosion source-side damage: v1
opts for single-cell damage and leaves source-cell damage to
v2. Rationale: invariants 32-34 are easier to make bit-structural
without the paired path, and the Fontanel source-cell damage
always lags by one tick when compared to client expectations.

### 4.5  F0823 — Explosion per-tick damage roll

```
pseudocode F0823(in, digest, rng, outAttackApplied):
    switch in->explosionType:
        case C007_EXPLOSION_POISON_CLOUD:
            base = F0024_min(in->attack >> 5, 4)       /* 0..4 */
            base += F0732_rng(rng, 2)                   /* +0..1 */
            *outAttackApplied = F0025_max(1, base)      /* 1..5 */
        case C002_EXPLOSION_LIGHTNING_BOLT:
            raw = (in->attack >> 1) + 1
            raw += F0732_rng(rng, raw) + 1
            *outAttackApplied = raw >> 1              /* lightning halved again */
        case C000_EXPLOSION_FIREBALL:
        default:
            raw = (in->attack >> 1) + 1
            raw += F0732_rng(rng, raw) + 1
            *outAttackApplied = raw
    return 1
```

### 4.6  F0820 — Projectile-vs-projectile (R5)

Fontanel behavior: F0218_PROJECTILE_GetImpactCount iterates the
projectile list on a square — if another projectile is on the
square, it deletes the *earlier* one (F0214 call at PROJEXPL.C:633).

**v1 interpretation** (documented explicitly; marked as v2-worthy):
Both projectiles despawn (simultaneous annihilation, no damage).
This is strictly cleaner than the Fontanel "earlier wins" behavior
and is sound for a one-tick quantum model. Fontanel's behaviour
depends on list-iteration order which is not save-stable; v1
deliberately picks the order-independent variant.

`NEEDS DISASSEMBLY REVIEW` tag on the deviation. Invariant 27
nails v1 behaviour; invariant 48 (loop-guard) ensures we never
loop on it.

### 4.7  Fluxcage interaction

Fontanel F0221/F0224: fluxcage is an EXPLOSION thing of Type
C050. There is no code path that decreases fluxcage attack from
projectile impact. REMOVE_FLUXCAGE event at creation-time+100 is
the sole removal mechanism.

**v1 behaviour**: projectile on destination square with
`digest->destHasFluxcage == 1` → despawn via FLUXCAGE_HIT. No
explosion created, no fluxcage mutation. Exactly Fontanel.

Phase 17 does NOT emit REMOVE_FLUXCAGE events (Phase 12 reserves
the slot, and fluxcage *creation* is currently out of scope for
Phase 17 — see §1 scope note #8).

### 4.8  Hit emission details

Projectile hit → APPLY_DAMAGE_CHAMPION wound mask:
  `MASK0x0004_WOUND_HEAD | MASK0x0008_WOUND_TORSO` (PROJEXPL.C:550)
Projectile hit → APPLY_DAMAGE_GROUP wound mask: 0 (group damage
distributes via F0190, not wound-slotted).
Attacker index: `in->ownerIndex` (ownerKind disambiguates).
Scheduled delay: 0 (impact resolves same tick as the move).

### 4.9  F0810 — Projectile create

```
pseudocode F0810(in, list, outSlot, outEvent):
    if list->count >= PROJECTILE_LIST_CAPACITY: return 0   /* BUG0_16 v1 hard cap */
    slot = -1
    for i in 0..59:
        if list->entries[i].slotIndex == -1:
            slot = i; break
    if slot < 0: return 0
    list->entries[slot] = {
        slotIndex=slot, projectileCategory=in->category,
        projectileSubtype=in->subtype,
        ownerKind=in->ownerKind, ownerIndex=in->ownerIndex,
        mapIndex=in->mapIndex, mapX=in->mapX, mapY=in->mapY,
        cell=in->cell, direction=in->direction,
        kineticEnergy=clamp(0,255,in->kineticEnergy),
        attack=in->attack, stepEnergy=in->stepEnergy,
        firstMoveGraceFlag=in->firstMoveGraceFlag, /* 1 for champion/creature, 0 for launcher */
        launchedAtTick=in->currentTick,
        scheduledAtTick=in->currentTick+1,
        associatedPotionPower=in->potionPower,
        poisonAttack=in->poisonAttack, attackTypeCode=in->attackTypeCode,
        flags=(in->potionPower ? 1 : 0) | (subtype_creates_explosion ? 2 : 0)
    }
    list->count++
    *outSlot = slot
    /* Emit first-move event */
    zero(outEvent)
    outEvent->kind = TIMELINE_EVENT_PROJECTILE_MOVE
    outEvent->fireAtTick = in->currentTick + 1
    outEvent->mapIndex = in->mapIndex
    outEvent->mapX = in->mapX
    outEvent->mapY = in->mapY
    outEvent->cell = in->cell
    outEvent->aux0 = slot
    outEvent->aux1 = in->ownerKind
    outEvent->aux2 = in->ownerIndex
    outEvent->aux3 = in->subtype
    outEvent->aux4 = 0
    return 1
```

### 4.10  Spell → projectile bridge (F0812)

```
pseudocode F0812(effect, champIdx, partyMap{Index,X,Y}, partyDir, currentTick, list, outSlot, outEvent):
    if effect == NULL: return 0
    if effect->spellKind != C2_SPELL_KIND_PROJECTILE_COMPAT: return 0
    subtype = SPELL_TYPE_TO_PROJECTILE_SUBTYPE[effect->spellType]
    if subtype == 0: return 0
    category = PROJECTILE_CATEGORY_MAGICAL
    in = {
        ownerKind=PROJECTILE_OWNER_CHAMPION, ownerIndex=champIdx,
        mapIndex=partyMapIndex, mapX=partyMapX, mapY=partyMapY,
        cell=<derive from partyDirection and champion cell>,
        direction=partyDir,
        subtype=subtype, category=category,
        kineticEnergy=effect->kineticEnergy,
        attack=effect->impactAttack,
        stepEnergy=<subtype table: fireball=1, lightning=1, ...>,
        currentTick=currentTick,
        firstMoveGraceFlag=1,
        poisonAttack=(subtype==POISON_BOLT||POISON_CLOUD ? effect->impactAttack : 0),
        attackTypeCode=<subtype table: FIREBALL=C1_ATTACK_FIRE, LIGHTNING=C3_ATTACK_ELEMENTAL, ...>,
        potionPower=0
    }
    return F0810(&in, list, outSlot, outEvent)
```

Subtype table hand-entered in `.c`. Only 5 entries:

```
spellType C0_SPELL_TYPE_PROJECTILE_FIREBALL     -> PROJECTILE_SUBTYPE_FIREBALL
spellType C1_SPELL_TYPE_PROJECTILE_LIGHTNING    -> PROJECTILE_SUBTYPE_LIGHTNING_BOLT
spellType C2_SPELL_TYPE_PROJECTILE_POISON_BOLT  -> PROJECTILE_SUBTYPE_POISON_BOLT
spellType C3_SPELL_TYPE_PROJECTILE_POISON_CLOUD -> PROJECTILE_SUBTYPE_POISON_CLOUD
spellType C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR    -> PROJECTILE_SUBTYPE_OPEN_DOOR
```

(Phase 14 header defines `C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR_COMPAT`;
the 0..4 names are DM1-standard. Implementer confirms the actual
macro names in Phase 14's `.h` before hard-coding — see §7
plan-reader rule.)

---

## 5. Invariant list (target ≥ 30; shipped count: 48)

Probe header closes with `Invariant count: 48` + `Status: PASS`.

### A — Sizes & platform (5)

| # | Category | Invariant |
|---|----------|-----------|
| 1 | size | `sizeof(int) == 4` |
| 2 | size | `PROJECTILE_INSTANCE_SERIALIZED_SIZE == 96` |
| 3 | size | `EXPLOSION_INSTANCE_SERIALIZED_SIZE == 64` |
| 4 | size | `CELL_CONTENT_DIGEST_SERIALIZED_SIZE == 100` |
| 5 | size | `PROJECTILE_TICK_RESULT_SERIALIZED_SIZE == 232 && EXPLOSION_TICK_RESULT_SERIALIZED_SIZE == 184` |

### B — Round-trip serialisation (4)

| # | Category | Invariant |
|---|----------|-----------|
| 6 | round-trip | Zero `ProjectileInstance_Compat` round-trips bit-identical |
| 7 | round-trip | Fully-populated `ProjectileInstance_Compat` (all 24 fields ≠ 0) round-trips bit-identical |
| 8 | round-trip | Zero + populated `ExplosionInstance_Compat` round-trip bit-identical (2 cases in one invariant) |
| 9 | round-trip | `ProjectileList_Compat` and `ExplosionList_Compat` with 3 entries each round-trip bit-identical including count + empty-slot markers |

### C — Motion determinism (8)

| # | Category | Invariant |
|---|----------|-----------|
| 10 | motion | Direction=0 (NORTH), cell=0, starting at (1,1,1,1), open floor ahead → crossedCell=1 and newMapY=0 |
| 11 | motion | Direction=1 (EAST), cell=1 → newMapX=dest, crossedCell=1 |
| 12 | motion | Direction=2 (SOUTH), cell=2 → newMapY=dest+1 side (DM coordinate convention) |
| 13 | motion | Direction=3 (WEST), cell=3 → newMapX=dest-1 side |
| 14 | motion | Intra-cell flip (cell does not match direction) → crossedCell=0, newCell differs from in.cell by XOR of direction-parity |
| 15 | motion | Cell parity rule: `(dir & 1) == (cell & 1)` → newCell = cell-1; else newCell = cell+1 (two sub-checks) |
| 16 | motion | Two-tick kinetic at stepEnergy=1 with kineticEnergy=10 — after 10 `F0811` calls on open floor the 11th call despawns with `DESPAWN_ENERGY` |
| 17 | motion | Map-boundary clamp: direction step crosses off-map → resultKind=HIT_WALL with despawn=1, no map read beyond bounds |

### D — Collision cases (7)

| # | Category | Invariant |
|---|----------|-----------|
| 18 | collision | Wall ahead → resultKind=HIT_WALL, despawn=1, no action emitted for kinetic, no action emitted for magical but explosion emitted |
| 19 | collision | Closed door (non-pass-through) ahead → resultKind=HIT_DOOR, emittedDoorDestructionEvent=1, outNextTick.kind==TIMELINE_EVENT_DOOR_DESTRUCTION |
| 20 | collision | Destroyed door ahead (doorState == DESTROYED) → projectile passes through (resultKind=FLEW, despawn=0) |
| 21 | collision | Champion on destination cell (cell mask bit set) → resultKind=HIT_CHAMPION, emittedCombatAction=1, outAction.kind==COMBAT_ACTION_APPLY_DAMAGE_CHAMPION, allowedWounds == (HEAD|TORSO) |
| 22 | collision | Creature on destination cell → resultKind=HIT_CREATURE, emittedCombatAction=1, outAction.kind==COMBAT_ACTION_APPLY_DAMAGE_GROUP |
| 23 | collision | Non-material creature hit by non-HARM_NON_MATERIAL projectile → despawn=1 (v1 simplification), resultKind=DESPAWN_ENERGY |
| 24 | collision | Fluxcage on destination cell → resultKind=HIT_FLUXCAGE, despawn=1, no emission of any kind |

### E — Despawn paths (3)

| # | Category | Invariant |
|---|----------|-----------|
| 25 | despawn | `kineticEnergy <= stepEnergy` at tick start → DESPAWN_ENERGY, no emission |
| 26 | despawn | Hit wall → despawn=1 always |
| 27 | despawn | Other projectile on destination cell → HIT_OTHER_PROJECTILE, despawn=1 for both (invariant constructs two projectiles and checks both sides) |

### F — Explosion frame + attack advance (4)

| # | Category | Invariant |
|---|----------|-----------|
| 28 | explosion | Fireball at attack=80 → ONE_SHOT, despawn=1, no reschedule, one party/group combat action |
| 29 | explosion | Poison cloud at attack=32 → newAttack=29, despawn=0, reschedule at tick+1, group action emitted |
| 30 | explosion | Poison cloud at attack=5 → despawn=1, ONE_SHOT |
| 31 | explosion | Smoke at attack=100 → newAttack=60, reschedule at tick+1, no damage emitted |

### G — AoE damage distribution (2)

| # | Category | Invariant |
|---|----------|-----------|
| 32 | AoE | Poison cloud with `digest->destHasChampion==1` → emittedCombatActionPartyCount=1, emittedCombatActionGroupCount=0 (champion preempts group) |
| 33 | AoE | Poison cloud with `destHasChampion==0 && destHasCreatureGroup==1` → group action only |

### H — Power vs radius (2)

Per §4.4 / §4.5 v1 has no radial splash; invariants 34-35 test
the per-type attack-roll formula at known seeds:

| # | Category | Invariant |
|---|----------|-----------|
| 34 | AoE | Fireball attack=10 with RNG seed 0xA5 → F0823 returns deterministic `attackApplied` in range [(10>>1)+1+1, (10>>1)+1+((10>>1)+1)+1] = [7..13]; structural envelope check, NOT byte match |
| 35 | AoE | Lightning attack=10 → applied in [3..6] (half of fireball range). Envelope check. |

### I — Phase 13 integration (2)

| # | Category | Invariant |
|---|----------|-----------|
| 36 | int-P13 | `F0811` emits APPLY_DAMAGE_CHAMPION → Phase 13's `F0735_COMBAT_ResolveChampionAttack` (or apply-damage resolver) consumes without crash, returns 1 |
| 37 | int-P13 | `F0811` emits APPLY_DAMAGE_GROUP → Phase 13's group-damage resolver (F0736 or equivalent) consumes without crash, returns 1 |

### J — Phase 14 integration (1)

| # | Category | Invariant |
|---|----------|-----------|
| 38 | int-P14 | SpellEffect with `spellKind=C2_SPELL_KIND_PROJECTILE_COMPAT, spellType=FIREBALL, impactAttack=100, kineticEnergy=20` → F0812 creates projectile with matching fields, first-move event scheduled at `currentTick+1` |

### K — Phase 12 integration (2)

| # | Category | Invariant |
|---|----------|-----------|
| 39 | int-P12 | `outNextTick.kind == TIMELINE_EVENT_PROJECTILE_MOVE` and `fireAtTick > currentTick` after every FLEW-result advance |
| 40 | int-P12 | Feeding `outNextTick` through Phase 12's `F0721_TIMELINE_Schedule` + `F0727/F0728` queue round-trip is bit-identical |

### L — Phase 15 integration (2)

| # | Category | Invariant |
|---|----------|-----------|
| 41 | int-P15 | Standalone `ProjectileList_Compat` ser/deser round-trip bit-identical when list contains 3 projectiles across 3 owner kinds |
| 42 | int-P15 | Standalone `ExplosionList_Compat` ser/deser round-trip bit-identical when list contains 4 explosions of 4 different types (fireball/lightning/poison-cloud/smoke) |

### M — Phase 16 integration (1)

| # | Category | Invariant |
|---|----------|-----------|
| 43 | int-P16 | Synthesised `CreatureTickResult_Compat` with `emittedSpellRequest=1` + a compatible SpellEffect → F0812 launches; projectile fields mirror the spell's impactAttack/kineticEnergy |

### N — Boundary (4)

| # | Category | Invariant |
|---|----------|-----------|
| 44 | boundary | `F0811(in=NULL, ...)` → returns 0, result untouched |
| 45 | boundary | `F0811` with `direction=4` (out of range 0..3) → returns 0, resultKind=INVALID |
| 46 | boundary | `F0811` with attack=0 and kineticEnergy=0 → DESPAWN_ENERGY immediately, no action emitted |
| 47 | boundary | Out-of-map-bounds digest (`destIsMapBoundary=1`) → HIT_WALL (treat as wall), despawn=1 |

### O — Purity (2)

| # | Category | Invariant |
|---|----------|-----------|
| 48a | purity | CRC32 of `in` (ProjectileInstance) and `digest` unchanged across F0811 call (in-pointers are const) |
| 48b | purity | Static `SUBTYPE_CREATES_EXPLOSION` table CRC32 unchanged across 256 F0811 iterations with random inputs |

(Counted as one invariant with two sub-checks — the probe reports
both in one `- PASS:` line.)

### P — Real DUNGEON.DAT spot-check (1)

| # | Category | Invariant |
|---|----------|-----------|
| 49 | dungeon | Open DUNGEON.DAT via Phase 9 F0504. Party start on Level 1 (known fixed cell via Phase 10 `F0680_PARTY_Init_Compat`). Launch a kinetic arrow eastbound at `stepEnergy=1, kineticEnergy=8, attack=30`. Walk forward one tick at a time. Projectile must hit the first wall on the eastbound ray within ≤ 10 ticks. Assertion: `resultKind == HIT_WALL` on tick N where N equals the distance-to-wall extracted from DUNGEON.DAT tile data. The distance value is a golden from the Fontanel source's Level-1 map (hard-coded in probe after reading DUNGEON.DAT). |

### Q — Loop guard (1)

| # | Category | Invariant |
|---|----------|-----------|
| 50 | loop | 100 random (rngSeed=0xFACE17) projectile configurations, advance up to 200 ticks each. Every single advance either (a) sets `despawn=1`, or (b) emits `outNextTick.kind==TIMELINE_EVENT_PROJECTILE_MOVE` with `fireAtTick - currentTick >= 1`. Zero infinite loops allowed. Mitigates R1/R5. |

**Total: 48 invariants (two sub-checks within 48; counted as 48
for the header).**

Distribution across required categories:

| Required category | Required min | Shipped |
|-------------------|--------------|---------|
| Round-trip each struct | ≥4 | 4 (inv 6-9) |
| Motion determinism (4 dirs) | ≥4 | 4 (inv 10-13) |
| Collision cases | ≥7 | 7 (inv 18-24) |
| Despawn | ≥3 | 3 (inv 25-27) |
| Explosion frame/radius power 1/5/10 | ≥4 | 4 (inv 28-31) |
| AoE distribution | ≥2 | 2 (inv 32-33) |
| Phase 13 | ≥2 | 2 (inv 36-37) |
| Phase 14 | ≥1 | 1 (inv 38) |
| Phase 12 | ≥2 | 2 (inv 39-40) |
| Phase 15 | ≥2 | 2 (inv 41-42) |
| Phase 16 | ≥1 | 1 (inv 43) |
| Boundary | ≥4 | 4 (inv 44-47) |
| Purity | ≥2 | 2 (inv 48a/48b) |
| Real DUNGEON.DAT | ≥1 | 1 (inv 49) |
| Loop guard | ≥1 | 1 (inv 50) |

All required minima met.

---

## 6. Implementation order for the Codex agent

Strict linear sequence. Compile after every step. **No renames
mid-task.** If step N fails, fix before step N+1.

### Step 1 — `.h` file

Create `memory_projectile_pc34_compat.h`:

- All `#define` constants from §2 + §4.3 + §4.10.
- Every struct from §2.
- Every function prototype from §3.
- Includes: `<stdint.h>`,
  `"memory_combat_pc34_compat.h"` (CombatAction, RngState),
  `"memory_timeline_pc34_compat.h"` (TimelineEvent kinds),
  `"memory_magic_pc34_compat.h"` (SpellEffect + spell-type macros).

Syntax smoke-check with an empty-main wrapper. Must compile clean
under `-Wall -Wextra`.

### Step 2 — `.c` skeleton

Create `memory_projectile_pc34_compat.c`:

- Includes: `<string.h>`, `<stdint.h>`, own header.
- `_Static_assert(sizeof(int) == 4, ...)` + `_Static_assert` for
  all 7 new size constants (96, 64, 100, 232, 184, 5768, 2056).
- Every function body returns `0` / zeros its outputs.
- Static `Phase17_SubtypeCreatesExplosion[256]` filled (§4.3).
- Static `Phase17_SpellTypeToSubtype[...]` filled (§4.10).
- Static `Phase17_ExplosionMaxFrames[128]` filled (table of
  per-type default frame counts: FIREBALL=3, LIGHTNING=2,
  POISON_CLOUD=many, SMOKE=many, FLUXCAGE=1 (static), REBIRTH=2).
- Static helpers `write_le_int32` / `read_le_int32` (local).

Compile standalone with `-Wall -Wextra`. Clean, modulo expected
`unused` warnings on skeleton functions.

### Step 3 — Fill F0810 / F0813 (create + despawn)

- Empty slot scan; BUG0_16 hard cap.
- Emit first-move `TIMELINE_EVENT_PROJECTILE_MOVE` event at
  `currentTick + 1`.
- Compile.

### Step 4 — Fill F0814 – F0816 (cell inspection)

- F0814 priority order (§4.2).
- F0815 impact-attack formula (§4.8).
- F0816 door pass-through (v1 conservative).
- Compile.

### Step 5 — Fill F0817 – F0820 (collision resolution)

- Combat-action builders.
- Door-destruction event builder.
- F0820 dispatch (§4.3).
- Compile.

### Step 6 — Fill F0811 (primary entry point)

- Full §4.1 pseudocode.
- Hard clamp on reschedule delay via F0825.
- Compile.

### Step 7 — Fill F0821 – F0826 (explosion side)

- F0821 create, F0822 advance (§4.4), F0823 damage roll (§4.5),
  F0824 despawn, F0825/F0826 schedule helpers.
- Compile.

### Step 8 — Fill F0812 (spell bridge)

- Read `SpellEffect_Compat.spellKind / spellType / impactAttack /
  kineticEnergy`.
- Delegate to F0810.
- Compile.

### Step 9 — Fill F0827 – F0829 (serialisation)

- 8 ser/deser functions. LSB-first int32 writers. Bounds-check
  `bufSize`.
- Compile.

### Step 10 — `firestaff_m10_projectile_probe.c`

- Scaffold copied from `firestaff_m10_creature_ai_probe.c`:
  opens `projectile_probe.md` + `projectile_invariants.md`,
  defines `CHECK(cond, name)` macro, closes with `Invariant count:
  N` + `Status: PASS/FAIL`.
- Argv: `$1 = DUNGEON.DAT` (for invariant 49), `$2 = output dir`.
- Start with invariant 1 (`sizeof(int) == 4`). Build + run.

### Step 11 — Add invariants incrementally

Ten blocks A-J. Compile + run after each block.

- **Block A (sizes, 1-5):** All size asserts.
- **Block B (round-trip, 6-9):** Four struct round-trips.
- **Block C (motion, 10-17):** 4 cardinal + 4 motion-math + boundary.
- **Block D (collision, 18-24):** 7 collision cases.
- **Block E (despawn, 25-27):** 3 despawn paths.
- **Block F (explosion, 28-31):** 4 expl