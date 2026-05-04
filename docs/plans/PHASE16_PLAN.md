# PHASE 16 PLAN — Creature AI / Monster Behavior System (v1)

Firestaff M10 milestone, Phase 16. Assumed starting state: 15 phases
PASS (Phase 14 magic + Phase 15 save/load already merged). This
document is the *single source of truth* the Codex ACP executor
follows. Any deviation = abort and ask.

Style rules (non-negotiable, inherited from Phases 10–15):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation. Every struct round-trips
  bit-identical.
- Pure functions: NO globals, NO UI, NO IO. The creature tick is a
  pure transformation:
  ```
  F0804_CREATURE_Tick(input, rng) ->
      (new AI state, 0..1 CombatAction, 0..1 movement delta,
       0..1 SpellCastRequest [stub], 1 follow-up TimelineEvent)
  ```
  Randomness flows through Phase 13's explicit `RngState_Compat*`.
- Function numbering: **F0790 – F0809** (20 slots, Phase 16 range).
- Probe emits `creature_ai_probe.md` + `creature_ai_invariants.md`
  with a trailing `Status: PASS` line.
- Verify gate: `run_firestaff_m10_verify.sh` gets **exactly one** new
  `# Phase 16:` block appended. Pre-grep and post-grep mandatory.
- ADDITIVE ONLY: zero edits to Phase 9/10/11/12/13/14/15 source.
  Phase 16 consumes their interfaces via `#include` and pure
  composition.
- Creature-type count for DM1 is **27** (C00..C26,
  `C027_CREATURE_TYPE_COUNT` in DEFS.H:2435). The task brief said
  "29" — that is incorrect for DM1. Plan uses 27 profile entries:
  3 fully implemented, 24 stubbed (stub still schedules next
  CREATURE_TICK, emits NO_ACTION).

---

## 1. Scope definition

### In scope for Phase 16 v1

1. **CREATURE_TICK event handler as a pure state-transform.** Given a
   `CreatureTickInput_Compat` (party pos, group pos, group AI state,
   adjacency digest, current tick, RNG), produce a
   `CreatureTickResult_Compat` (new AI state, optional CombatAction,
   optional movement delta, optional spell-cast-request stub,
   mandatory follow-up CREATURE_TICK `TimelineEvent_Compat`).
2. **Per-group AI state machine** covering the five
   Fontanel-documented behaviours plus our coarse wrapper state:
   - `IDLE` (our wrapper for asleep / out-of-sight)
   - `WANDER` — `C0_BEHAVIOR_WANDER` (DEFS.H:1372)
   - `FLEE` — `C5_BEHAVIOR_FLEE` (DEFS.H:1376, stub)
   - `ATTACK` — `C6_BEHAVIOR_ATTACK` (DEFS.H:1377)
   - `APPROACH` — `C7_BEHAVIOR_APPROACH` (DEFS.H:1378)
   - `STUN` (our wrapper, holds the fear/stun counter)
   - `DEAD` (terminal — emits nothing, no reschedule)
3. **Perception** — port of Fontanel `F0200_GROUP_GetDistanceToVisibleParty`
   (GROUP.C:1315) with the simplified DM1 branch
   (`MEDIA009`/`MEDIA020`): creature sight-range vs
   `F0199_GROUP_GetDistanceBetweenUnblockedSquares`. LoS is approximated
   by a pure "same-row-or-column-AND-no-wall-between" test against
   Phase 6 tile data. Smell range gate per Fontanel
   `F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal` (GROUP.C:1417).
4. **Target selection** — nearest-alive-champion by Manhattan
   distance; ties broken by lowest champion index. Cites
   `F0229_GROUP_SetOrderedCellsToAttack` (GROUP.C:140). No
   "archenemy" logic in v1 (deferred).
5. **One-step pathfinding** — port of
   `F0202_GROUP_IsMovementPossible` (GROUP.C:1457) for a single cell
   step, plus the primary/secondary/opposite/random fallback chain
   from `F0209` ATTACK-branch lines 2264-2267. No multi-step
   planning, no A*.
6. **Attack initiation** — when adjacent + creature facing party,
   emit a `CombatAction_Compat` with `kind=COMBAT_ACTION_CREATURE_MELEE`.
   Phase 13 `F0736_COMBAT_ResolveCreatureMelee_Compat` consumes it
   unmodified. Citation: `F0207_GROUP_IsCreatureAttacking`
   (GROUP.C:1645).
7. **Three fully-implemented creature types** (see §4.11 for
   rationale):
   - **C09 Stone Golem** — slow immobile-ish melee, no special
     sensors, perfect baseline.
   - **C10 Mummy** — standard-speed melee, no poison cloud, no
     side-attack, classic "undead hunt".
   - **C12 Skeleton** — faster melee, similar to Mummy but with
     different sight/attack cadence.
   All three have `AttackType == COMBAT_ATTACK_NORMAL` or
   `COMBAT_ATTACK_SHARP`; none casts spells; none has MASK0x2000
   archenemy or MASK0x0004 side-attack.
8. **24 stubbed creature types** — return `AI_RESULT_NO_ACTION` and
   reschedule next `TIMELINE_EVENT_CREATURE_TICK` at
   `nowTick + profile.movementTicks` (profile table provides
   per-type tick values). No movement, no attack. Safe for
   full-verify.
9. **Profile table** — static `CreatureBehaviorProfile_Compat[27]`
   embedded in `.c`, hand-entered from DEFS.H CREATURE_INFO
   constants + known DM1 community reference. Each stub entry
   tagged `/* v1 STUB — full behavior deferred to post-M10 */`.
10. **Self-damage emission** — when the group tile is marked as
    `onFluxcage` / `onPoisonCloud` / `onPit` (flags in
    `CreatureTickInput_Compat`), emit a
    `COMBAT_ACTION_APPLY_DAMAGE_GROUP` with the caller-provided
    `selfDamage` value. Does NOT decide the damage number — that's
    Phase 14 / projectile layer.
11. **Serialisation** of `CreatureAIState_Compat` so Phase 15
    save/load can roundtrip active-group AI state through the
    composite save-blob. Exposes the same MEDIA016/LSB-first
    primitive helpers used in Phase 13 F0740-F0747.
12. **Integration hooks** — one real CREATURE_TICK from a real
    DUNGEON.DAT group (Level 1 skeleton group, invariant 40) runs
    end-to-end.

### Explicitly OUT of scope for Phase 16 v1

Deferred (tag `NEEDS DISASSEMBLY REVIEW` or `v2`):

- **Spell-casting creatures** — Vexirk (C14), Lord Chaos (C23), etc.
  `SpellCastRequest_Compat` field exists in result struct but is
  *always zero* in v1. Defer to v2.
- **Group formation / splitting / cell reshuffle** — Fontanel
  `F0178_GROUP_GetGroupValueUpdatedWithCreatureValue` (GROUP.C:174)
  is NOT ported. v1 treats groups as atomic.
- **Item drops on death** — `F0186 / F0188 GROUP_DropGroup…`
  (GROUP.C:550, 676). v1 sets `result.dropItemsPending = 1`; actual
  drop logic is post-M10. Save-game round-trip captures the marker.
- **Sound / audio alerting** — no `M542_SOUND_*` hooks.
- **Advanced pathfinding** — no A*. One-step only, direct /
  secondary / opposite / random fallback chain.
- **Fear/flee movement** — `STUN` state holds fear counter and
  decrements per tick. Movement consequence of flee = **none** in
  v1 (stub). Tagged `NEEDS DISASSEMBLY REVIEW`.
- **Water elemental swimming**, **swamp slime movement** — covered
  by stub path.
- **Boss behavior** — Lord Chaos archenemy / double-move /
  teleport-through-walls (BUG0_00 comment) — all deferred. Dragon
  area attack deferred.
- **Reaction events 29/30/31** (`DANGER_ON_SQUARE`,
  `HIT_BY_PROJECTILE`, `PARTY_IS_ADJACENT`, DEFS.H:948-950).
  v1 handles only `C37_EVENT_UPDATE_BEHAVIOR_GROUP`. Reactions stay
  deferred, but the result struct has `reactionPending` field so
  the caller can synthesize one manually.
- **BUG0_67 / BUG0_68 parity** — we implement the non-buggy path
  and cite the bug for completeness. No bit-match on buggy path.
- **Scent/smell tracking** — smell-range perception is gated by a
  static "smellsAllowed" bit in the profile; scent-history update
  is NOT ported (that's a champion-state concern).
- **Freeze-life** — Phase 14 owns `freezeLifeTicks`; v1 reads the
  flag from `CreatureTickInput_Compat.freezeLifeTicks` and, if
  nonzero and creature is NOT archenemy, reschedules 4 ticks later
  (mirror of GROUP.C:1935 branch) without executing AI. This is a
  5-line pass-through; full parity deferred.

### Phase 13/14/15 REVIEW markers touched by Phase 16

| # | Marker | Status after Phase 16 |
|---|--------|-----------------------|
| Phase 13 #5 (cell/direction repack on creature kill) | deferred | still deferred (needs group-formation work) |
| Phase 14 poison-tick follow-through | partially addressed | v1 emits `poisonAttackPending` on self-damage path |
| Phase 15 save-blob completeness | unblocks | `CreatureAIState_Compat` serialise lets Phase 15 capture live AI state |

No Phase 16 dependency REVERSES an earlier deferral; we only move
the ball forward.

---

## 2. Data structures

All sizes are multiples of 4. Each field is `int32_t`-equivalent.
A `_Static_assert(sizeof(int) == 4, …)` sits at the top of the `.c`,
mirroring Phases 13–15.

### 2.1  `CreatureAIState_Compat` (per-group persistent AI state)

```
struct CreatureAIState_Compat {
    int stateKind;             /* AI_STATE_IDLE / WANDER / FLEE /
                                  ATTACK / APPROACH / STUN / DEAD  */
    int creatureType;           /* 0..26, mirror of DungeonGroup_Compat.creatureType */
    int groupMapIndex;          /* which dungeon map the group is on */
    int groupMapX;
    int groupMapY;
    int groupCells;             /* mirror of DungeonGroup_Compat.cells   */
    int groupDirection;         /* 0..3, currently-facing direction      */
    int targetChampionIndex;    /* -1 = none, else 0..3                  */
    int lastSeenPartyMapX;
    int lastSeenPartyMapY;
    int lastSeenPartyTick;      /* uint32_t widened to int32 for LSB ser  */
    int fearCounter;            /* 0..255, decrements every tick in STUN */
    int turnCounter;            /* ticks since last decision — diag only */
    int attackCooldownTicks;    /* 0 when ready to attack                */
    int movementCooldownTicks;  /* 0 when ready to move                  */
    int aggressionScore;        /* 0..100, profile-driven, cached        */
    int rngCallCount;           /* diagnostic; sums across ticks         */
    int reserved0;              /* keeps struct 18×int32 = 72 bytes     */
};
```

- 18 int32 → `CREATURE_AI_STATE_SERIALIZED_SIZE = 72`.

State enum values (bit-exact; kept stable for save-blob forever):

```
#define AI_STATE_IDLE       0
#define AI_STATE_WANDER     1
#define AI_STATE_FLEE       2
#define AI_STATE_ATTACK     3
#define AI_STATE_APPROACH   4
#define AI_STATE_STUN       5
#define AI_STATE_DEAD       6
```

### 2.2  `CreatureTickInput_Compat` (world snapshot for one tick)

```
struct CreatureTickInput_Compat {
    int groupSlotIndex;         /* 0..groupCount-1                       */
    int creatureType;           /* 0..26                                 */
    int groupMapIndex;
    int groupMapX;
    int groupMapY;
    int groupCells;
    int groupDirection;
    int groupCountMinus1;       /* 0..3 == DungeonGroup_Compat.count     */
    int groupCurrentHealth[4];  /* mirror of DungeonGroup_Compat.health   */
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int partyDirection;
    int partyChampionsAlive;    /* bitmask 0..15                         */
    int partyChampionCurrentHealth[4];
    int adjacencyWallMask;      /* bit i set iff direction i blocked by wall/fakewall */
    int adjacencyDoorMask;      /* bit i set iff direction i has closed door         */
    int adjacencyPitMask;       /* bit i set iff direction i has open pit            */
    int adjacencyCreatureMask;  /* bit i set iff direction i has another live group  */
    int onFluxcageFlag;         /* 1 iff group tile carries fluxcage       */
    int onPoisonCloudFlag;      /* 1 iff group tile carries poison cloud   */
    int onPitFlag;              /* 1 iff group tile is an open pit         */
    int selfDamagePerTick;      /* caller-computed tile hazard damage      */
    int currentTickLow;         /* lower 32 bits of game time              */
    int freezeLifeTicks;        /* mirror of Phase 14 MagicState field    */
    int partyInvisibility;      /* 0/1 from Phase 14 MagicState           */
    int eventType;              /* 37 = UPDATE_BEHAVIOR_GROUP; others stub */
    int reserved0;              /* padding to 32 int32 = 128 bytes        */
};
```

- 32 int32 → `CREATURE_TICK_INPUT_SERIALIZED_SIZE = 128`.

### 2.3  `CreatureTickResult_Compat` (pure output)

```
struct CreatureTickResult_Compat {
    int resultKind;              /* AI_RESULT_NO_ACTION / MOVED / ATTACKED /
                                    WOKE / FLED / STUNNED / DIED           */
    int newAIState;              /* copy of CreatureAIState_Compat.stateKind */
    int emittedCombatAction;     /* 0 or 1 — struct in outAction below     */
    int emittedSpellRequest;     /* 0 in v1 (stub)                         */
    int emittedMovement;         /* 0 or 1                                 */
    int emittedSelfDamage;       /* 0 or 1                                 */
    int reactionPending;         /* 0 or 1 — caller may synthesize event 29/30/31 */
    int dropItemsPending;        /* 0 or 1 — only when resultKind==DIED    */
    int nextEventDelayTicks;     /* must be >= 1                           */
    int newMovementCooldown;
    int newAttackCooldown;
    int newFearCounter;
    int rngCallCount;
    int reserved0;
    int reserved1;
    int reserved2;               /* keeps struct header at 16 int32 = 64 bytes */
    struct CombatAction_Compat outAction;        /* 12 int32 = 48 bytes    */
    int outMovementTargetMapX;
    int outMovementTargetMapY;
    int outMovementDirection;
    int outMovementReserved;                     /* keeps to 16 bytes      */
    struct TimelineEvent_Compat outNextTick;     /* 11 int32 = 44 bytes    */
    int outTickPadding;                          /* ser envelope = 4 bytes */
};
```

Layout:

| Offset | Field | Size |
|-------:|-------|-----:|
| 0x000  | 16 int32 header | 64 |
| 0x040  | outAction (CombatAction) | 48 |
| 0x070  | 4 int32 movement block  | 16 |
| 0x080  | outNextTick (TimelineEvent) | 44 |
| 0x0AC  | outTickPadding | 4 |
| **total** | | **176 bytes** |

- `CREATURE_TICK_RESULT_SERIALIZED_SIZE = 176`.

Result kind enum (stable):

```
#define AI_RESULT_NO_ACTION   0
#define AI_RESULT_MOVED        1
#define AI_RESULT_ATTACKED     2
#define AI_RESULT_WOKE         3
#define AI_RESULT_FLED         4
#define AI_RESULT_STUNNED      5
#define AI_RESULT_DIED         6
#define AI_RESULT_SKIPPED_FROZEN 7
```

### 2.4  `CreatureBehaviorProfile_Compat` (static per-type constants)

```
struct CreatureBehaviorProfile_Compat {
    int creatureType;           /* 0..26                                   */
    int sightRange;              /* tiles, `M054_SIGHT_RANGE` mirror       */
    int smellRange;              /* tiles, `M055_SMELL_RANGE` mirror       */
    int movementTicks;           /* CREATURE_INFO.MovementTicks            */
    int attackTicks;             /* CREATURE_INFO.AttackTicks              */
    int baseAttack;              /* CREATURE_INFO.Attack                   */
    int baseDefense;             /* CREATURE_INFO.Defense                  */
    int baseHealth;              /* CREATURE_INFO.BaseHealth               */
    int dexterity;               /* CREATURE_INFO.Dexterity                */
    int poisonAttack;            /* CREATURE_INFO.PoisonAttack             */
    int attackType;              /* COMBAT_ATTACK_*  constants             */
    int woundProbabilities;      /* raw 16-bit                             */
    int attributes;              /* raw 16-bit (side-attack, night-vis…)  */
    int aggressionBias;          /* 0..100                                 */
    int implementationTier;      /* 0 = stub, 1 = full                     */
    int reserved0;
};
```

- 16 int32 = 64 bytes; **not serialised** (immutable game data,
  hand-entered in `.c`).
- `CREATURE_BEHAVIOR_PROFILE_SIZE = 64` only used in sizeof-asserts.

Profile table: `Phase16_CreatureBehaviorProfile[27]`, one entry per
creature type, filled from DEFS.H CREATURE_INFO comments +
community reference. Each entry carries `implementationTier`:
1 for C09 / C10 / C12, 0 for all others.

### 2.5  Size-collision check

Phase 16's constants: `72 / 128 / 176 / 64`.
- 72 collides with Phase 14 `MAGIC_STATE_SERIALIZED_SIZE` (72) but
  the **macro name** is `CREATURE_AI_STATE_SERIALIZED_SIZE` — no
  `#define` clash. `_Static_assert` in `.c`.
- 128, 176 are new.
- 64 already used by Phase 14 `SPELL_CAST_REQUEST_SERIALIZED_SIZE`;
  our 64 is the unused-in-file `CREATURE_BEHAVIOR_PROFILE_SIZE`
  (internal only, sizeof-assert only).

No `#define` name collision.

### 2.6  Integration with existing structs

- **Phase 9** `struct DungeonGroup_Compat` (16 bytes on disk):
  Phase 16 **does not** modify it. The caller reads it and fills
  `CreatureTickInput_Compat.creatureType / groupCells / groupDirection
  / groupCurrentHealth[4]`. Pure view.
- **Phase 10** `ChampionState_Compat`: Phase 16 NEVER touches. The
  caller packs `partyChampionCurrentHealth[4]` + `partyChampionsAlive`
  bitmask into the input.
- **Phase 12** `TimelineEvent_Compat` (44 bytes): emitted unchanged
  — we fill kind=1 `TIMELINE_EVENT_CREATURE_TICK`, aux0 =
  groupSlotIndex, aux1 = creatureType, aux2 = newAIState, aux3 = 0,
  aux4 = 0.
- **Phase 13** `CombatAction_Compat` (48 bytes), `CombatResult_Compat`,
  `RngState_Compat`: consumed by `F0800_CREATURE_EmitCombatAction_Compat`
  and `F0804_CREATURE_Tick_Compat`. Phase 13 `F0736` takes the
  emitted action with zero changes.
- **Phase 14** `MagicState_Compat`: Phase 16 reads `freezeLifeTicks`
  + `event71CountInvisibility` only, no writes. Those flow through
  `CreatureTickInput_Compat`.
- **Phase 15** save blob: Phase 15's composite serialiser gains a
  per-group `CreatureAIState_Compat` block via the two new helpers
  `F0805 / F0806`. Phase 15 already has an "AI reserved block"
  slot (confirmed via §7 plan-reader rule before coding). If it
  does not, Phase 16 ships without touching the save blob and logs
  a deferral note — this is the pressure point for R3 below.

---

## 3. Function API (F0790 – F0809)

All live in `memory_creature_ai_pc34_compat.{h,c}`. All pure unless
noted. Conventions inherited from Phase 13:

- Input pointers: `const` always.
- Output pointers: non-`const`, caller-owned.
- Return: 1 on success, 0 on invalid argument / bounds failure.
- RNG draws recorded in `result.rngCallCount`.

### Group A — Perception (F0790 – F0792)

| # | Signature | Role |
|---|-----------|------|
| F0790 | `int F0790_CREATURE_GetManhattanDistance_Compat(int ax, int ay, int bx, int by, int* out);` | Pure `|dx|+|dy|`. Mirror of `F0226_GROUP_GetDistanceBetweenSquares` for Manhattan metric used by F0791. |
| F0791 | `int F0791_CREATURE_IsDestinationVisible_Compat(const struct CreatureTickInput_Compat* in, int* outDistance);` | Same-row-or-column test between group and party, then walks tiles and consults `in->adjacencyWallMask` style flags propagated via Phase 6 tile reader (caller pre-bakes LoS into `in->losBlockedFlag`). Mirror of `F0227_GROUP_IsDestinationVisibleFromSource`. Pure. Returns 1 if party is visible; sets `*outDistance` to tile count (0 if LoS blocked). **NOTE: caller pre-computes LoS into a single flag** — Phase 16 does NOT re-walk tile data; it reads the flag. This is the v1 simplification. |
| F0792 | `int F0792_CREATURE_Perceive_Compat(const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, int* outPartyVisible, int* outDistance, int* outCanSmell);` | Top-level perception. Applies sight range + smell range + invisibility gate (SEE_INVISIBLE attribute bit mirrored from `MASK0x0800`). Mirror of `F0200 + F0201`. Pure. |

### Group B — State machine (F0793 – F0795)

| # | Signature | Role |
|---|-----------|------|
| F0793 | `int F0793_CREATURE_ComputeNextState_Compat(const struct CreatureAIState_Compat* s, const struct CreatureTickInput_Compat* in, int partyVisible, int canSmell, int* outNextState, int* outAggressionDelta);` | Table-driven (see §4.2). Transitions for the six live states are decided from 4 inputs: `partyVisible`, `canSmell`, `inputHealth == 0`, `fearCounter > 0`. Pure. |
| F0794 | `int F0794_CREATURE_ApplyFreezeLifeGate_Compat(const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, int* outSkipTick, int* outRescheduleTicks);` | Mirror of GROUP.C:1935-1946 freeze-life branch. Returns `*outSkipTick=1` + `*outRescheduleTicks=4` if life is frozen and creature is not archenemy. Pure. |
| F0795 | `int F0795_CREATURE_DecrementCounters_Compat(struct CreatureAIState_Compat* inOut, int elapsedTicks);` | Pure mutator of counters: clamps `fearCounter`, `attackCooldownTicks`, `movementCooldownTicks` to max(0, current-elapsed). Caller-owned `inOut`. |

### Group C — Target selection (F0796 – F0797)

| # | Signature | Role |
|---|-----------|------|
| F0796 | `int F0796_CREATURE_PickChampion_Compat(const struct CreatureTickInput_Compat* in, int* outChampionIndex);` | Closest-alive by Manhattan, ties → lowest index. Returns 0 iff no alive champion (sets `*outChampionIndex = -1`). Cites `F0229_GROUP_SetOrderedCellsToAttack`. |
| F0797 | `int F0797_CREATURE_ScoreCandidates_Compat(const struct CreatureTickInput_Compat* in, int outScores[4]);` | Diagnostic helper. Populates per-champion score = `10000 - (health/1 + 10*distance)`. **Advisory only** — F0796 does not use this, invariants do. Pure. |

### Group D — Pathfinding (F0798 – F0799)

| # | Signature | Role |
|---|-----------|------|
| F0798 | `int F0798_CREATURE_IsDirectionOpen_Compat(const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, int direction, int allowImaginaryPitsAndFakeWalls, int* outBlocker);` | Mirror of `F0202_GROUP_IsMovementPossible` (GROUP.C:1457). Checks adjacency masks in this order: wall, other-creature, closed-door, pit (unless LEVITATION — `MASK0x0020` from `profile->attributes`). Sets `*outBlocker` to `{0=open,1=wall,2=creature,3=door,4=pit}`. Pure. |
| F0799 | `int F0799_CREATURE_PickMoveDirection_Compat(const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, int primaryDir, int secondaryDir, int allowFakeWalls, struct RngState_Compat* rng, int* outDirection);` | Mirror of GROUP.C:2264-2267 cascade: primary → secondary (1/2 random) → opposite → random-fallback (1/4). Returns 0 iff no direction open (sets `*outDirection = -1`). RNG-driven; advances rng. |

### Group E — Action emission (F0800 – F0803)

| # | Signature | Role |
|---|-----------|------|
| F0800 | `int F0800_CREATURE_EmitCombatAction_Compat(const struct CreatureAIState_Compat* s, const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, int targetChampionIndex, struct CombatAction_Compat* outAction);` | Fills a `CombatAction_Compat` with kind=`COMBAT_ACTION_CREATURE_MELEE`, attackTypeCode from profile, rawAttackValue = profile.baseAttack, target* = party coords + cell, attackerSlotOrCreatureIndex = groupSlotIndex, scheduleDelayTicks = profile.attackTicks, flags = 0. Pure. |
| F0801 | `int F0801_CREATURE_EmitMovement_Compat(const struct CreatureAIState_Compat* s, const struct CreatureTickInput_Compat* in, int direction, struct CreatureTickResult_Compat* outResult);` | Computes new (mapX, mapY) from direction (using the PC `G0233/G0234_DirectionTo…` mirror table hard-coded in .c) and writes to result.outMovement*. Pure. |
| F0802 | `int F0802_CREATURE_EmitNextTickEvent_Compat(const struct CreatureAIState_Compat* s, const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, int forcedDelayTicks, struct TimelineEvent_Compat* outEvent);` | Fills `outEvent` with kind=`TIMELINE_EVENT_CREATURE_TICK`, fireAtTick = `in->currentTickLow + max(1, forcedDelayTicks)`, map* from group position, aux0=groupSlotIndex, aux1=creatureType, aux2=newAIState. **CRITICAL:** `forcedDelayTicks < 1` is clamped to 1. Infinite-loop guard (see risk R7). |
| F0803 | `int F0803_CREATURE_EmitSelfDamage_Compat(const struct CreatureTickInput_Compat* in, const struct CreatureBehaviorProfile_Compat* profile, struct CombatAction_Compat* outAction);` | Fills a `COMBAT_ACTION_APPLY_DAMAGE_GROUP` action with rawAttackValue = `in->selfDamagePerTick`. Only emits when `onFluxcageFlag || onPoisonCloudFlag || onPitFlag`. Pure. |

### Group F — Top-level entry (F0804)

| # | Signature | Role |
|---|-----------|------|
| F0804 | `int F0804_CREATURE_Tick_Compat(const struct CreatureAIState_Compat* stateIn, const struct CreatureTickInput_Compat* in, struct RngState_Compat* rng, struct CreatureAIState_Compat* stateOut, struct CreatureTickResult_Compat* out);` | Pure orchestrator. Sequence: (1) freeze-life gate via F0794; (2) perceive via F0792; (3) tier check on profile — if profile.implementationTier == 0 → fast-path stub (next tick only, no action); (4) decrement counters via F0795; (5) transition state via F0793; (6) per-state-kind dispatch — ATTACK uses F0796→F0800, APPROACH uses F0799→F0801, WANDER uses F0799→F0801, FLEE stubs to STUN (decrement only), STUN decrements fear, IDLE idles; (7) emit self-damage if hazard flags; (8) always emit F0802 next-tick event with delay = profile.movementTicks or profile.attackTicks clamped to ≥ 1. |

### Group G — Serialisation (F0805 – F0809)

| # | Signature | Bytes |
|---|-----------|-------|
| F0805 | `int F0805_CREATURE_AIStateSerialize_Compat(const struct CreatureAIState_Compat* s, unsigned char* buf, int bufSize);` | 72 |
| F0806 | `int F0806_CREATURE_AIStateDeserialize_Compat(struct CreatureAIState_Compat* s, const unsigned char* buf, int bufSize);` | 72 |
| F0807 | `int F0807_CREATURE_TickInputSerialize_Compat(const struct CreatureTickInput_Compat* s, unsigned char* buf, int bufSize);` | 128 |
| F0808 | `int F0808_CREATURE_TickInputDeserialize_Compat(struct CreatureTickInput_Compat* s, const unsigned char* buf, int bufSize);` | 128 |
| F0809 | `int F0809_CREATURE_TickResultSerialize_Compat / _Deserialize_Compat (paired, internal a/b labels per PHASE13 §3 precedent)` | 176 / 176 |

All serialise via the same MEDIA016 / PC LSB-first helper already
in Phase 13 (`write_le_int32` / `read_le_int32` static functions —
duplicate them locally rather than linking cross-module; Phase 13's
are `static` in its `.c`).

### DUNGEON.DAT dependency

Read-only. The probe opens DUNGEON.DAT via Phase 9's
`F0504_DUNGEON_LoadThingData_Compat` exclusively for invariant 40
(integration spot-check on a real Level 1 group). No other
file access.

---

## 4. Algorithm specifications

### 4.1  F0792 — Perception

Reference: `F0200_GROUP_GetDistanceToVisibleParty` (GROUP.C:1315).

```
pseudocode F0792(in, profile, outVisible, outDistance, outSmell):
    if in == NULL or profile == NULL: return 0
    /* Invisibility gate (MEDIA020 branch, GROUP.C:1332) */
    seeInvisible = (profile->attributes & MASK0x0800_SEE_INVISIBLE) != 0
    if in->partyInvisibility && !seeInvisible:
        *outVisible = 0; *outDistance = 0; *outSmell = 0
        /* Fallback to smell only */
        goto smell
    F0790(in->groupMapX, in->groupMapY, in->partyMapX, in->partyMapY, &dist)
    sightRange = profile->sightRange
    /* Night vision correction omitted (v1: caller pre-bakes dungeon
       palette into profile.sightRange). NEEDS DISASSEMBLY REVIEW. */
    if dist > max(1, sightRange): *outVisible = 0
    else if !in->losClearFlag:    *outVisible = 0   /* caller-provided */
    else:                          *outVisible = 1
    *outDistance = *outVisible ? dist : 0
smell:
    if profile->smellRange == 0: *outSmell = 0
    else if dist <= (profile->smellRange + 1) / 2: *outSmell = 1
    else:                                          *outSmell = 0
    return 1
```

`NEEDS DISASSEMBLY REVIEW`: night-vision palette subtraction; we
defer to caller-provided `sightRange` already-adjusted.

### 4.2  F0793 — State machine transitions

Reference: `F0209_GROUP_ProcessEvents29to41` behaviour switch
(GROUP.C:2024 onwards). Table-driven, NOT per-creature-type:

| current | partyVisible | canSmell | inputHealth=0 | fear>0 | → next |
|---------|-------------:|---------:|--------------:|-------:|--------|
| IDLE     | 1 | * | 0 | 0 | WANDER |
| IDLE     | 0 | 1 | 0 | 0 | WANDER |
| IDLE     | * | * | 1 | * | DEAD |
| IDLE     | 0 | 0 | 0 | 0 | IDLE |
| WANDER   | 1 | * | 0 | 0 | APPROACH |
| WANDER   | 0 | 1 | 0 | 0 | WANDER |
| WANDER   | * | * | 1 | * | DEAD |
| APPROACH | 1 | * | 0 | 0 | ATTACK if distance==1 else APPROACH |
| APPROACH | 0 | 1 | 0 | 0 | APPROACH |
| APPROACH | 0 | 0 | 0 | 0 | WANDER |
| ATTACK   | 1 | * | 0 | 0 | ATTACK if distance==1 else APPROACH |
| ATTACK   | 0 | * | 0 | 0 | APPROACH |
| FLEE     | * | * | 0 | >0 | FLEE (stub) |
| FLEE     | * | * | 0 | 0 | APPROACH |
| STUN     | * | * | 0 | >0 | STUN |
| STUN     | * | * | 0 | 0 | IDLE |
| DEAD     | * | * | * | * | DEAD (terminal) |

Table lives as a static `Phase16_StateTransitions[7][2][2][2][2]`
lookup (7 × 2 × 2 × 2 × 2 = 112 entries) in `.c`. Deterministic;
no RNG.

### 4.3  F0796 — Target selection

```
pseudocode F0796(in, outIdx):
    bestIdx = -1; bestDist = INT_MAX
    for i in 0..3:
        if !(in->partyChampionsAlive & (1<<i)): continue
        if in->partyChampionCurrentHealth[i] <= 0: continue
        /* distance same for all champions (they're in the same party
           tile); tie-break on lowest index -> i=0 wins any tie */
        F0790(in->groupMapX, in->groupMapY, in->partyMapX, in->partyMapY, &d)
        if d < bestDist:
            bestDist = d; bestIdx = i
    *outIdx = bestIdx
    return bestIdx >= 0
```

Because champions share a tile in DM1, this effectively picks the
lowest-index alive champion. Real DM1 uses cell ordering via
`F0229_GROUP_SetOrderedCellsToAttack` — v1 simplifies to index-based.
`NEEDS DISASSEMBLY REVIEW` tag on the simplification.

### 4.4  F0798 — Direction openness

Reference: `F0202_GROUP_IsMovementPossible` (GROUP.C:1457).

```
pseudocode F0798(in, profile, dir, allowFakePits, outBlocker):
    bit = 1 << (dir & 3)
    if in->adjacencyWallMask & bit:
        *outBlocker = 1; return 0
    if in->adjacencyCreatureMask & bit:
        *outBlocker = 2; return 0
    if (in->adjacencyDoorMask & bit):
        /* closed door blocks non-ghost creatures */
        if !(profile->attributes & MASK0x0040_NON_MATERIAL):
            *outBlocker = 3; return 0
    if (in->adjacencyPitMask & bit):
        if !(profile->attributes & MASK0x0020_LEVITATION) && !allowFakePits:
            *outBlocker = 4; return 0
    *outBlocker = 0; return 1
```

### 4.5  F0799 — Move direction cascade

Reference: GROUP.C:2264-2267.

```
pseudocode F0799(in, profile, pri, sec, allowFake, rng, outDir):
    if F0798(in, profile, pri, allowFake, _): *outDir = pri; return 1
    if sec >= 0 and F0798(in, profile, sec, allowFake && F0732_random(rng,2), _):
        *outDir = sec; return 1
    opp = (pri + 2) & 3
    if F0798(in, profile, opp, 0, _): *outDir = opp; return 1
    /* 1/4 random fallback to opposite-of-primary, 3/4 bail */
    if F0732_random(rng, 4) == 0:
        opp2 = (pri + 2) & 3
        if F0798(in, profile, opp2, 0, _): *outDir = opp2; return 1
    *outDir = -1; return 0
```

### 4.6  F0800 — Attack emission

Reference: `F0207_GROUP_IsCreatureAttacking` (GROUP.C:1645).

```
pseudocode F0800(s, in, profile, target, outAction):
    zero(outAction)
    outAction->kind = COMBAT_ACTION_CREATURE_MELEE
    outAction->attackTypeCode = profile->attackType
    outAction->rawAttackValue = profile->baseAttack
    outAction->targetMapIndex = in->partyMapIndex
    outAction->targetMapX = in->partyMapX
    outAction->targetMapY = in->partyMapY
    outAction->targetCell = 0  /* caller can refine per champion cell */
    outAction->attackerSlotOrCreatureIndex = in->groupSlotIndex
    outAction->defenderSlotOrCreatureIndex = target
    outAction->scheduleDelayTicks = profile->attackTicks
    outAction->flags = 0
    outAction->allowedWounds =
        COMBAT_WOUND_READY_HAND | COMBAT_WOUND_HEAD |
        COMBAT_WOUND_TORSO | COMBAT_WOUND_ACTION_HAND |
        COMBAT_WOUND_LEGS | COMBAT_WOUND_FEET
    return 1
```

Phase 13 `F0736_COMBAT_ResolveCreatureMelee_Compat` consumes this
struct without transformation (confirmed against the Phase 13 header
in §2.6).

### 4.7  F0802 — Next-tick emission + infinite-loop guard

```
pseudocode F0802(s, in, profile, forcedDelay, outEvent):
    if outEvent == NULL: return 0
    zero(outEvent)
    delay = forcedDelay
    if delay < 1: delay = 1      /* HARD GUARD — see risk R7 */
    outEvent->kind = TIMELINE_EVENT_CREATURE_TICK
    outEvent->fireAtTick = (uint32_t)(in->currentTickLow) + (uint32_t)delay
    outEvent->mapIndex = in->groupMapIndex
    outEvent->mapX = in->groupMapX
    outEvent->mapY = in->groupMapY
    outEvent->cell = 0
    outEvent->aux0 = in->groupSlotIndex
    outEvent->aux1 = in->creatureType
    outEvent->aux2 = s->stateKind
    outEvent->aux3 = 0
    outEvent->aux4 = 0
    return 1
```

### 4.8  F0804 — Orchestrator

```
pseudocode F0804(stateIn, in, rng, stateOut, out):
    *stateOut = *stateIn           /* copy-on-entry, modify stateOut only */
    zero(out)
    profile = Phase16_CreatureBehaviorProfile[in->creatureType]

    /* (1) freeze-life gate */
    F0794(in, &profile, &skip, &freezeDelay)
    if skip:
        out->resultKind = AI_RESULT_SKIPPED_FROZEN
        F0802(stateOut, in, &profile, freezeDelay, &out->outNextTick)
        return 1

    /* (2) perceive */
    F0792(in, &profile, &visible, &dist, &smell)

    /* (3) stub tier */
    if profile.implementationTier == 0:
        out->resultKind = AI_RESULT_NO_ACTION
        F0802(stateOut, in, &profile,
              max(1, profile.movementTicks), &out->outNextTick)
        return 1

    /* (4) decrement counters */
    F0795(stateOut, 1)

    /* (5) state transition */
    F0793(stateOut, in, visible, smell, &nextState, &aggrDelta)
    stateOut->stateKind = nextState
    stateOut->aggressionScore = clamp(0, 100,
                                      stateOut->aggressionScore + aggrDelta)

    /* (6) per-state dispatch */
    switch nextState:
        case AI_STATE_ATTACK:
            F0796(in, &target)
            if target >= 0 and dist == 1 and stateOut->attackCooldownTicks == 0:
                F0800(stateOut, in, &profile, target, &out->outAction)
                out->emittedCombatAction = 1
                out->resultKind = AI_RESULT_ATTACKED
                stateOut->attackCooldownTicks = profile.attackTicks
            else: out->resultKind = AI_RESULT_NO_ACTION
        case AI_STATE_APPROACH:
        case AI_STATE_WANDER:
            /* primary direction = sign of dx,dy; caller precomputes
               into in->primaryDir / in->secondaryDir */
            F0799(in, &profile, in->primaryDir, in->secondaryDir, 0, rng, &dir)
            if dir >= 0 and stateOut->movementCooldownTicks == 0:
                F0801(stateOut, in, dir, out)
                out->emittedMovement = 1
                out->resultKind = AI_RESULT_MOVED
                stateOut->movementCooldownTicks = profile.movementTicks
            else: out->resultKind = AI_RESULT_NO_ACTION
        case AI_STATE_FLEE:
            /* stub: decrement fear only */
            if stateOut->fearCounter > 0: stateOut->fearCounter -= 1
            out->resultKind = AI_RESULT_FLED
        case AI_STATE_STUN:
            if stateOut->fearCounter > 0: stateOut->fearCounter -= 1
            out->resultKind = AI_RESULT_STUNNED
        case AI_STATE_DEAD:
            out->resultKind = AI_RESULT_DIED
            out->dropItemsPending = 1
            /* no next tick for DEAD */
            return 1
        default: /* IDLE */
            out->resultKind = AI_RESULT_NO_ACTION

    /* (7) self-damage */
    if in->onFluxcageFlag or in->onPoisonCloudFlag or in->onPitFlag:
        /* overwrite outAction only if no combat action was emitted */
        if !out->emittedCombatAction:
            F0803(in, &profile, &out->outAction)
            out->emittedSelfDamage = 1

    /* (8) next tick */
    delay = max(1,
                min(profile.movementTicks, profile.attackTicks))
    F0802(stateOut, in, &profile, delay, &out->outNextTick)
    out->rngCallCount = stateOut->rngCallCount
    out->newAIState = stateOut->stateKind
    return 1
```

### 4.9  Spell-casting branch (stub)

Per §1 bullet "spell-casting creatures v2": the orchestrator has a
single no-op line:

```
out->emittedSpellRequest = 0;   /* NEEDS DISASSEMBLY REVIEW: Vexirk/Lord Chaos/
                                   Materializer cast-branch deferred to v2.
                                   See PROJEXPL.C F0219 + GROUP.C line 2147. */
```

### 4.10  Fear/flee (stub)

Per §1 bullet: `AI_STATE_FLEE` decrements `fearCounter` by 1 per tick,
otherwise no movement consequence. `NEEDS DISASSEMBLY REVIEW` tag on
the movement omission — Fontanel flees via
`F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal` but negated;
ported post-M10.

### 4.11  Three creature-type profiles

Hand-entered from DEFS.H CREATURE_INFO comments + community reference.
v1 `implementationTier = 1` set for these three only.

| Type | Name | sightRange | smellRange | movementTicks | attackTicks | baseAttack | baseDefense | baseHealth | attackType | rationale |
|-----:|------|-----------:|-----------:|--------------:|------------:|-----------:|------------:|-----------:|-----------:|-----------|
| C09 | Stone Golem | 3 | 0 | 36 | 16 | 55 | 70 | 145 | SHARP | slow, no smell, no special attrs. Only sight-driven — ideal for testing perception gate. |
| C10 | Mummy | 3 | 4 | 15 | 7 | 40 | 50 | 110 | NORMAL | classic melee undead, has smell — tests smell branch. No poison. |
| C12 | Skeleton | 3 | 4 | 11 | 6 | 40 | 40 | 90 | SHARP | fast melee — tests attackCooldown + movement cadence. |

Values will be confirmed against Fontanel disassembly at implementation
time; if any differ from DEFS.H CREATURE_INFO initialisers, the
disassembly wins and we re-bind. Any re-bind ≥ 2 fields triggers
`NEEDS DISASSEMBLY REVIEW` tag on the entry.

The other 24 entries carry default stub values
(`implementationTier=0, movementTicks=32, attackTicks=12`,
remainder zero). Stub path only uses `movementTicks` for reschedule.

---

## 5. Invariant list (target ≥ 35; shipped count: 40)

Probe header closes with `Invariant count: 40` + `Status: PASS`.

| # | Category | Invariant |
|---|----------|-----------|
| 1 | size | `CREATURE_AI_STATE_SERIALIZED_SIZE == 72` |
| 2 | size | `CREATURE_TICK_INPUT_SERIALIZED_SIZE == 128` |
| 3 | size | `CREATURE_TICK_RESULT_SERIALIZED_SIZE == 176` |
| 4 | size | `CREATURE_BEHAVIOR_PROFILE_SIZE == 64` (internal sizeof-assert) |
| 5 | size | `sizeof(int) == 4` (platform assumption, mirror of prior phases) |
| 6 | round-trip | Zero-populated `CreatureAIState_Compat` round-trips bit-identical |
| 7 | round-trip | Fully-populated `CreatureAIState_Compat` (all 18 fields ≠ 0) round-trips bit-identical |
| 8 | round-trip | Populated `CreatureTickInput_Compat` round-trips bit-identical (all 32 fields, including 4-entry health arrays) |
| 9 | round-trip | Populated `CreatureTickResult_Compat` round-trips bit-identical — including embedded `CombatAction_Compat` + `TimelineEvent_Compat` blocks |
| 10 | state-det | Given identical `(stateIn, input, rngSeed)`, two calls to `F0804` produce bit-identical `stateOut` and `out` — purity check across two runs |
| 11 | state-det | `F0793` IDLE + partyVisible=1 + inputHealth≠0 → WANDER, regardless of other bits (4 cross-checks) |
| 12 | state-det | `F0793` APPROACH + partyVisible=1 + distance=1 path → ATTACK (orchestrator-level, invariant observes resultKind) |
| 13 | perceive | Sight-range limit: distance == profile.sightRange and LoS clear → visible=1 (C09 Stone Golem at dist=3) |
| 14 | perceive | Sight-range limit+1: distance == profile.sightRange+1 → visible=0 |
| 15 | perceive | LoS blocked flag: distance=1 but losClearFlag=0 → visible=0 |
| 16 | perceive | Invisibility: partyInvisibility=1 and profile.attributes without SEE_INVISIBLE → visible=0, falls back to smell |
| 17 | target | Four-champion party all alive, all same tile → F0796 returns index 0 (tie-break) |
| 18 | target | Champion[0] dead (health=0, bit=0 in partyChampionsAlive) → F0796 returns index 1 |
| 19 | target | All four champions dead → F0796 returns 0 (failure), `outIdx = -1` |
| 20 | target | Sleeping champion (marked via Phase 10 `isResting` flag surfaced into alive mask) → still picked (v1 does not weight by sleep — NEEDS DISASSEMBLY REVIEW) |
| 21 | path | Open floor in primary direction → F0799 returns primaryDir |
| 22 | path | Wall in primary, open in secondary → F0799 returns secondaryDir with RNG seeded so the 1/2 roll passes |
| 23 | path | Closed door in primary (non-NON_MATERIAL creature) → F0798 returns blocker=3 |
| 24 | path | Another creature in primary, secondary blocked, opposite open → F0799 returns opposite |
| 25 | path | All four directions blocked → F0799 returns `outDir = -1`, result is `AI_RESULT_NO_ACTION` (no crash) |
| 26 | attack | Adjacent (distance=1) + ATTACK state + attackCooldown=0 → `out.emittedCombatAction = 1`, `out.outAction.kind == COMBAT_ACTION_CREATURE_MELEE`, `out.outAction.rawAttackValue == profile.baseAttack` |
| 27 | attack | Non-adjacent (distance=2) + ATTACK state → `out.emittedCombatAction = 0` (transitions back to APPROACH), `out.outAction.kind == 0` |
| 28 | attack | Adjacent + ATTACK + `attackCooldownTicks > 0` → `out.emittedCombatAction = 0`, state stays ATTACK |
| 29 | integration | `out.outNextTick.kind == TIMELINE_EVENT_CREATURE_TICK`, `fireAtTick > in.currentTickLow`, `fireAtTick - in.currentTickLow >= 1` |
| 30 | integration | Feeding `out.outNextTick` back through Phase 12 `F0721_TIMELINE_Schedule_Compat` into a TimelineQueue_Compat round-trips bit-identical via `F0727/F0728` (queue ser/deser) |
| 31 | integration | `out.outAction` when emittedCombatAction=1 → `F0736_COMBAT_ResolveCreatureMelee_Compat(&creatureSnapshot, &championSnapshot, rng, &result)` returns 1 (consumes without crash, not bit-match tested) |
| 32 | stub-meta | For every creature type in {0..26}\{9,10,12}, calling `F0804` with synthesised input returns `resultKind == AI_RESULT_NO_ACTION`, `emittedCombatAction == 0`, `emittedMovement == 0`, and `outNextTick.kind == TIMELINE_EVENT_CREATURE_TICK` with delay >= 1 (single meta-invariant, 24 sub-checks) |
| 33 | boundary | `F0804(stateIn=NULL, …)` → returns 0, leaves stateOut/out untouched (compared against all-zero buffer) |
| 34 | boundary | `F0804` with `creatureType = 27` (out of range) → returns 0 |
| 35 | boundary | `F0804` with stateIn.stateKind = AI_STATE_DEAD → resultKind == AI_RESULT_DIED, no CREATURE_TICK emitted (outNextTick.kind stays 0) |
| 36 | boundary | `F0804` with `in.partyChampionsAlive == 0` → resultKind == AI_RESULT_NO_ACTION, emittedCombatAction=0 (even if distance=1 and ATTACK state) |
| 37 | purity | Input buffer (stateIn + in) checksummed pre/post F0804 → CRC32 unchanged (all fields immutable from callee POV) |
| 38 | purity | `Phase16_CreatureBehaviorProfile[]` bytewise checksum pre/post F0804 run over 27 types → unchanged (static table is read-only) |
| 39 | loop-guard | Every one of 100 random valid inputs (RNG seed 0xC0FFEE) produces `outNextTick.fireAtTick - in.currentTickLow >= 1` → 100% pass, no zero-delay event emitted (mitigates R7) |
| 40 | dungeon-spot | Using Phase 9 `F0504_DUNGEON_LoadThingData_Compat` on the real DUNGEON.DAT, find one group with `creatureType == 12` (Skeleton) on map 0; construct input with party at Manhattan distance 3 and LoS clear; run F0804 once → state transitions from IDLE to WANDER (partyVisible triggers it via sight range), `outNextTick.aux0 == groupSlotIndex`, `outNextTick.aux1 == 12`, `outNextTick.aux2 == AI_STATE_WANDER` |

Golden coverage for the three implemented creature types:
- C09 Stone Golem × 2 (perceive at dist=3 pass/fail; movement cadence)
  — invariants 13/14 and 21
- C10 Mummy × 2 (smell gate; approach-to-attack transition)
  — invariants 16 and 12
- C12 Skeleton × 2 (adjacent attack; integration spot-check via
  F0736 + dungeon real-group) — invariants 26/31 and 40

Total: 40 invariants (≥ 30 required, ≥ 35 target — 5 over target).

---

## 6. Implementation order for the Codex agent

Strict linear sequence. Compile after every step. No renames
mid-task. If step N fails, fix before step N+1.

### Step 1 — `.h` file

- Create `memory_creature_ai_pc34_compat.h`:
  - All `#define` constants from §2 + §4.11.
  - Every struct from §2.
  - Every function prototype from §3.
  - Includes: `<stdint.h>`, `"memory_combat_pc34_compat.h"` (for
    `RngState_Compat`, `CombatAction_Compat`), `"memory_timeline_pc34_compat.h"`
    (for `TimelineEvent_Compat`), `"memory_dungeon_dat_pc34_compat.h"`
    (for `DungeonGroup_Compat` — referenced in comments only, no live
    dependency).
- Syntax smoke-check:
  ```
  cc -Wall -Wextra -c -x c -o /tmp/creature_ai_h_check.o <<EOF
  #include "memory_creature_ai_pc34_compat.h"
  int main(void){return 0;}
  EOF
  ```
  Must compile clean.

### Step 2 — `.c` skeleton with profile table

- Create `memory_creature_ai_pc34_compat.c`:
  - Includes: `<string.h>`, `<stdint.h>`, own header, Phase 13 combat
    header (for the LE-int32 helpers — duplicated, not linked).
  - `_Static_assert` block for all size constants.
  - Every function body returns `0` / zeros its outputs.
  - Static `Phase16_CreatureBehaviorProfile[27]` filled (3 real
    + 24 stub entries).
  - Static `Phase16_StateTransitions[…]` table filled.
  - Static helper `write_le_int32` / `read_le_int32` (local duplicates).
- Compile standalone with `-Wall -Wextra`. Clean, modulo the
  `__attribute__((unused))` on the unused table constants during
  skeleton phase.

### Step 3 — Fill implementation group-by-group

Compile after each sub-step. Smoke-link against the probe *after*
Step 4.

- **3a.** F0790–F0792 (perception).
- **3b.** F0793–F0795 (state machine).
- **3c.** F0796–F0797 (target selection).
- **3d.** F0798–F0799 (pathfinding).
- **3e.** F0800–F0803 (action emission).
- **3f.** F0804 (orchestrator).
- **3g.** F0805–F0809 (serialisation pairs).

### Step 4 — `firestaff_m10_creature_ai_probe.c`

- Scaffold copied from `firestaff_m10_combat_probe.c`:
  opens `creature_ai_probe.md` + `creature_ai_invariants.md`, defines
  `CHECK(cond, name)` macro, closes with `Invariant count: N` +
  `Status: PASS/FAIL` trailer.
- Argv: `$1 = DUNGEON.DAT` (for invariant 40), `$2 = output dir`.
- Start with invariant 5 (`sizeof(int) == 4`). Build + run.
  Confirm artifacts appear correctly.

### Step 5 — Add invariants incrementally

- **Block A (sizes, 1–5):** All `_SIZE` macros + sizeof-int. Build + run.
- **Block B (round-trip, 6–9):** Four struct round-trips. Build + run.
- **Block C (state machine, 10–12):** Determinism + 2 transition goldens. Build + run.
- **Block D (perception, 13–16):** Sight limits, LoS, invisibility. Build + run.
- **Block E (target, 17–20):** Four-champion scenarios. Build + run.
- **Block F (pathfinding, 21–25):** Five direction scenarios. Build + run.
- **Block G (attack + integration, 26–31):** **CROSS-PHASE risk
  zone.** After this block, run the full verify script to confirm
  Phases 13/14 still pass.
- **Block H (stub meta, 32):** 24 creature-type sweep. Build + run.
- **Block I (boundary + purity, 33–38):** Null, OOR, DEAD, CRC32. Build + run.
- **Block J (loop guard + dungeon, 39–40):** 100-iteration RNG sweep,
  real DUNGEON.DAT group spot-check. Build + run.

### Step 6 — Driver script `run_firestaff_m10_creature_ai_probe.sh`

- Mirror `run_firestaff_m10_combat_probe.sh`. Two argv:
  `$1 = DUNGEON.DAT`, `$2 = output dir`.
- Compile command:
  ```
  cc -Wall -Wextra -O1 -I"$ROOT" \
      -o "$PROBE_BIN" \
      "$ROOT/firestaff_m10_creature_ai_probe.c" \
      "$ROOT/memory_creature_ai_pc34_compat.c" \
      "$ROOT/memory_combat_pc34_compat.c" \
      "$ROOT/memory_timeline_pc34_compat.c" \
      "$ROOT/memory_dungeon_dat_pc34_compat.c" \
      "$ROOT/memory_champion_state_pc34_compat.c"
  ```
- Invoke `"$PROBE_BIN" "$1" "$2"`.
- `chmod +x`.

### Step 7 — Append to `run_firestaff_m10_verify.sh`

**Pre-check** (MUST pass before the edit):
```
grep -c '^# Phase 16:' run_firestaff_m10_verify.sh   # must be 0
```

If non-zero, `git checkout run_firestaff_m10_verify.sh` first.

Find the line `echo "=== M10 verification complete ==="` and
insert the new block **immediately before** it:

```
# Phase 16: Creature AI probe
echo "=== Phase 16: M10 creature AI probe ==="
CREATURE_AI_DIR="$OUT_DIR/creature-ai"
"$ROOT/tmp/firestaff/run_firestaff_m10_creature_ai_probe.sh" "$DUNGEON_DAT" "$CREATURE_AI_DIR" || {
    echo "FAIL: M10 creature AI probe did not pass"
    exit 1
}
echo "M10 creature AI probe: PASS"

python3 - <<'PY' "$CREATURE_AI_DIR/creature_ai_invariants.md" "$SUMMARY_MD" "$CREATURE_AI_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['creature_ai_probe.md', 'creature_ai_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'creature-ai: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('creature-ai: invariant status is not PASS')
if failures:
    text += '\n## M10 creature AI check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 creature AI check: PASS\n\n'
text += '- artifact present: creature_ai_probe.md\n'
text += '- artifact present: creature_ai_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY
```

**Post-check** (MUST pass after the edit):
```
grep -c '^# Phase 16:' run_firestaff_m10_verify.sh   # must be 1
grep -c '^# Phase 15:' run_firestaff_m10_verify.sh   # must be 1 (unchanged)
grep -c '^# Phase 14:' run_firestaff_m10_verify.sh   # must be 1 (unchanged)
grep -c '^# Phase 13:' run_firestaff_m10_verify.sh   # must be 1 (unchanged)
grep -c '^# Phase 12:' run_firestaff_m10_verify.sh   # must be 1 (unchanged)
...
```

If ANY value != 1, `git checkout run_firestaff_m10_verify.sh` and retry.

### Step 8 — Full verify

```
bash run_firestaff_m10_verify.sh \
    /Users/bosse/.openclaw/data/redmcsb-original/DungeonMasterPC34/DATA/DUNGEON.DAT \
    /tmp/m10-verify-out
```
Exit must be 0. All 16 phases PASS.

If any earlier phase regresses — it shouldn't, we added files only —
`git diff` every file outside §7 and revert.

---

## 7. Files to create + modify

| Path | Action | Estimated size |
|------|--------|----------------|
| `tmp/firestaff/memory_creature_ai_pc34_compat.h` | CREATE | ~10 KB (~310 lines) |
| `tmp/firestaff/memory_creature_ai_pc34_compat.c` | CREATE | ~28 KB (~950 lines: 20-fn bodies + 27-entry profile table + 112-entry state transition table + LE ser helpers) |
| `tmp/firestaff/firestaff_m10_creature_ai_probe.c` | CREATE | ~22 KB (~720 lines: 40 invariants, 10 blocks) |
| `tmp/firestaff/run_firestaff_m10_creature_ai_probe.sh` | CREATE | ~0.9 KB (~30 lines) |
| `tmp/firestaff/run_firestaff_m10_verify.sh` | MODIFY | **+34 lines appended exactly once** |

No other files touched. No `.phase*-attempt-*` backups created.

**Plan-reader rule** (risk R3 mitigation): before writing any code,
the implementer MUST `read` these four headers to confirm exact
struct layouts:
- `memory_combat_pc34_compat.h`
- `memory_timeline_pc34_compat.h`
- `memory_dungeon_dat_pc34_compat.h`
- `memory_magic_pc34_compat.h` (Phase 14)

If Phase 14 or Phase 15 introduced a struct rename between the plan
and implementation, the plan is WRONG; stop and surface before
writing code.

---

## 8. Risk register

| # | Risk | Likelihood | Impact | Plan B |
|---|------|------------|--------|--------|
| R1 | **Creature-type-count explosion**: 27 types × N behaviours. Temptation to over-scope into Vexirk magic, swamp slime swimming, dragon AoE. | High (scope creep is the phase-16 trap) | Very high — blows the 90-min budget | **Ruthless 3-type whitelist** (C09 / C10 / C12). `implementationTier == 0` in all other profile entries. If during coding the implementer feels tempted to add a fourth creature: STOP, document as v2, commit the stub path. Meta-invariant 32 enforces stub-path correctness so we know the fallback works. |
| R2 | **Pathfinding correctness** vs Fontanel's exact cascade (`F0202` + GROUP.C:2264-2267). RNG gating on the secondary-direction 1/2 chance + opposite-direction 1/4 random is a landmine. | Medium | Medium (invariants 21-25 fail) | §4.5 codifies the exact order. Invariants 21-25 are goldens on each branch with tight RNG seeding. If 24 fails, re-inspect `F0799` against GROUP.C:2265 — do NOT back-fit. |
| R3 | **Integration drift with Phase 13 / 14 / 15.** Phase 14 and 15 are executing in parallel subagents; their final struct layouts may differ from what this plan assumes. | Medium (Phase 14 plan is merged; Phase 15 plan may still drift) | High (breaks invariants 30-31, 40) | **Plan-reader rule** in §7: implementer reads actual headers before coding. Invariant 30 tests `F0727/F0728` round-trip as a canary — if it fails, the TimelineEvent_Compat layout changed. Invariant 31 catches CombatAction_Compat drift. If Phase 15's save-blob slot for AI state does not exist, Phase 16 ships without touching save code (§1 bullet 11 is softly dependent). |
| R4 | **State-machine explosion** (7 states × 27 types × 10 triggers = ~1890 paths). | Low if kept table-driven | Medium | State machine is **per-creature-TYPE agnostic** — the profile table provides per-type constants but transitions are type-independent. Table is 112 entries (§4.2). If a creature type needs special transitions, introduce a per-type transition-table override in v2. Invariants 11/12 are the canary. |
| R5 | **Verify-script duplicate append.** Bitten every prior phase. | Medium | High (silent duplicate pass) | §7 Step 7 codifies pre-grep (==0) and post-grep (==1) with five previous-phase grep-counts as extra canaries. Explicit `git checkout` + retry on failure. NO exceptions. |
| R6 | **Borland rand() non-determinism.** Same as Phase 13 R7 / Phase 14 R4. | Certain | None (we're self-consistent via F0732) | All RNG-driven invariants (22, 24, 39) use explicit seeds and envelope bounds. Invariants 30-31-40 do NOT depend on RNG at all (deterministic input). |
| R7 | **Infinite-tick loop**: F0804 always emitting CREATURE_TICK — if delay ever rolls to 0, Phase 12's scheduler processes it forever in the same tick and melts. | Medium (easy to drop the clamp) | Catastrophic — full-verify hangs | §4.7 F0802 has a **hard clamp** `if (delay < 1) delay = 1;`. Invariant 39 runs 100 random inputs, asserts `fireAtTick - currentTickLow >= 1` every time. The DEAD state explicitly does NOT emit a CREATURE_TICK (terminal) — tested by invariant 35. |
| R8 | **Self-damage overwrite collision**: if a creature both attacks and stands on a fluxcage, the single `outAction` field must not silently overwrite the attack. | Low | Low (inconsistent result) | §4.8 F0804 step (7): self-damage only writes outAction iff `!out->emittedCombatAction`. Tested implicitly by invariant 26 (attack path doesn't touch self-damage) and invariant 32 (stub path with hazards emits APPLY_DAMAGE_GROUP). |
| R9 | **Creature-type count miscount**: task says 29, actual DM1 is 27. Implementer may stub 29 → 2 phantom types. | Low | Low (wastes ~40 lines) | §2.4 pins the count at 27; the static array is `[27]`. `_Static_assert(sizeof(Phase16_CreatureBehaviorProfile) / sizeof(*Phase16_CreatureBehaviorProfile) == 27)` at the bottom of the `.c`. Invariant 34 tests out-of-range rejection with type=27. |
| R10 | **Profile numeric values drift from Fontanel CREATURE_INFO**: hand-entered movementTicks/attack for C09/C10/C12 may be off. | Medium | Low (goldens shift) | §4.11 rationale: values are documented as "community reference, confirm at implementation time". Invariant 26 asserts `out.outAction.rawAttackValue == profile.baseAttack` rather than a literal number — goldens are **structural**, not numeric. |

**Top two risks by expected cost:** **R1 (scope creep into 4th/5th
creature type)** — the phase's central trap, because 27 types are
ALL there sitting as stubs begging for full implementation; and **R7
(infinite-tick loop)** — catastrophic failure mode, cheap to prevent
with a one-line clamp and a 100-iteration invariant, but cheap to
forget.

---

## 9. Acceptance criteria

Phase 16 is complete and ready for merge when ALL of the following
hold:

- [ ] `bash run_firestaff_m10_verify.sh <dungeon.dat> <out>` exits 0.
- [ ] `grep -c '^# Phase 16:' run_firestaff_m10_verify.sh` equals **1**.
- [ ] `grep -c '^# Phase 15:' run_firestaff_m10_verify.sh` still equals 1 (no regression).
- [ ] `grep -c '^# Phase 14:' run_firestaff_m10_verify.sh` still equals 1.
- [ ] `grep -c '^# Phase 13:' run_firestaff_m10_verify.sh` still equals 1.
- [ ] Every `# Phase N:` for N ∈ 1..12 still equals 1.
- [ ] `$OUT_DIR/creature-ai/creature_ai_probe.md` exists and is non-empty.
- [ ] `$OUT_DIR/creature-ai/creature_ai_invariants.md` exists, trailing
      line is `Status: PASS`, contains `Invariant count: N` with
      N ≥ 30 (target 40, shipped 40), every `- ` bullet begins with
      `- PASS:`.
- [ ] `cc -Wall -Wextra -c` of every new `.c` file emits zero warnings.
- [ ] `ls tmp/firestaff/.phase*-attempt-* 2>/dev/null` returns nothing.
- [ ] `git status` shows only the five files listed in §7 (no
      collateral edits).
- [ ] **Integration invariant**: invariant 40 passes — one real
      CREATURE_TICK from a real DUNGEON.DAT monster group (Level 1
      skeleton, creatureType=12) processes end-to-end (perception →
      state transition → next-tick schedule).
- [ ] **Infinite-loop guard**: invariant 39 passes — 100/100 random
      inputs produce delay ≥ 1.
- [ ] **Stub-meta**: invariant 32 passes — all 24 stub creature types
      produce `AI_RESULT_NO_ACTION` + valid next-tick event.
- [ ] **Phase 13 integration**: invariant 31 passes — emitted
      CombatAction is consumed by `F0736` without crash.
- [ ] All 15 previous phases still pass — confirmed by verify exiting 0.

---

*End of plan. The Codex ACP executor should read this file in full,
re-read the four dependent headers (§7 plan-reader rule), then
follow §6 steps 1→8 without deviation. Any ambiguity = stop and
emit a review-request comment rather than fabricate.*
