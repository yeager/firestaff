# PHASE 13 PLAN — Combat System (v1)

Firestaff M10 milestone, Phase 13. Status at planning time: 12 phases PASS.
This document is the *single source of truth* the Codex ACP executor
follows. Any deviation = abort and ask.

Style rules (non-negotiable, inherited from prior phases):

- All symbols end `_pc34_compat`.
- MEDIA016 / PC LSB-first serialisation. Every struct round-trips bit
  identical.
- Pure functions: NO globals, NO UI, NO IO except what the caller already
  opened. No hidden RNG state — all randomness comes through an explicit
  `RngState_Compat*` parameter.
- Function numbering continues after timeline (F0720–F0728).
  Phase 13 uses F0730–F0747.
- Every probe writes `<phase>_probe.md` and `<phase>_invariants.md` with
  trailing `Status: PASS`.
- One clean block appended to `run_firestaff_m10_verify.sh`. `grep -c` of
  `# Phase 13:` must be exactly 1 after the edit.

---

## 1. Scope definition

### In scope for Phase 13 v1

Only the **pure data-layer resolver** for the two most common combat
interactions in DM/CSB, plus the shared damage-application primitives:

1. **Champion → creature melee attack.** A champion in the party cell
   swings their action-hand weapon (or bare hand) at an adjacent
   creature. We compute hit/miss and damage. Mirror of
   `F0231_GROUP_GetMeleeActionDamage`
   (PROJEXPL.C:1416) reduced to a pure function of (champion stats,
   creature stats, weapon profile, rng).
2. **Creature → champion melee attack.** A creature strikes a champion
   in the adjacent party cell. Mirror of `F0230_GROUP_GetChampionDamage`
   (PROJEXPL.C:1305).
3. **Damage application to a single champion.** Updates `CurrentHealth`,
   accumulates pending damage, sets wound bitmask. Mirror of
   `F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage`
   (CHAMPION.C:1803) — without the render / stamina / sound / timeline
   side-effects.
4. **Damage application to a creature slot in a group.** Decrements
   `Health[creatureIndex]`, returns outcome (`KILLED_NO` /
   `KILLED_SOME` / `KILLED_ALL`). Mirror of
   `F0190_GROUP_GetDamageCreatureOutcome` (GROUP.C:769), stripped of
   group-squash / event cancellation / fear re-behaviour (those belong to
   the rendering + AI phases).
5. **Deterministic RNG primitive.** A caller-owned `RngState_Compat`
   (LCG, 32-bit seed). Replaces the `M003_RANDOM(n)` family with a
   reproducible sequence so every combat calculation is deterministic.
6. **Post-attack timeline event emission.** Pure function: given a
   resolved combat action, return a fully-populated
   `TimelineEvent_Compat` that *would* be scheduled (attack cooldown /
   hide-damage flash). Caller decides whether to push it into the phase
   12 queue. We never call `F0721_TIMELINE_Schedule_Compat` ourselves.

### Explicitly OUT of scope for Phase 13

Deferred to later phases:

- **Projectile flight, impact, thrown-item resolution** → Phase 14
  (needs PROJECTILE event kind 3 + 48/49 of PROJEXPL.C, non-trivial).
- **Explosion AoE resolution** (`F0191`, `F0213`) → Phase 14 with
  projectiles (same family).
- **Magic attacks / spells / shields / wisdom-scaled attacks**
  (`C5_ATTACK_MAGIC`, `C6_ATTACK_PSYCHIC`, `SpellShieldDefense`,
  `FireShieldDefense`) → Phase 14 magic.
- **Door destruction by attack** (`F0232_GROUP_IsDoorDestroyedByAttack`)
  → Phase 14 (tied to projectiles anyway).
- **Creature-vs-creature damage.** DM/CSB has none — the real game uses
  creature-vs-creature friendly fire only via fireball splash; skip.
- **Skill experience awarding** (`F0304_CHAMPION_AddSkillExperience`),
  **stamina decrement** (`F0325_CHAMPION_DecrementStamina`), **food /
  water tick**, **lucky roll** (`F0308_CHAMPION_IsLucky` uses hidden
  state) → Phase 15 or later.
- **Wake-from-rest side effect** (`F0314_CHAMPION_WakeUp`) → caller
  responsibility; we flag it via a `wakeFromRest` bit in the result.
- **Fear / flee behaviour re-decision**, **aspect update events**
  (`C33..C41_EVENT_UPDATE_*`) → AI phase, post-M10.
- **Sound hooks** (`F0064_SOUND_RequestPlay_CPSD`) → audio phase.
- **Save-file integration** → Phase 15.
- **Poison tick** (`F0322_CHAMPION_Poison`). The *flag* that poison
  should be applied is returned in `CombatResult_Compat.poisonAttack`,
  but the caller handles it. The countdown itself is deferred.

### Why this subset

- It fits in a single 45-min Codex run. The four mirror functions above
  are ~250 lines total in Fontanel; our data-layer port removes the
  render + event + sound tangles and leaves ~400 lines of pure C.
- It provides the primitive every future phase needs (damage
  application + RNG + outcome codes).
- It makes the party *playably fighting*, which is the user-visible
  milestone promise for M10.

---

## 2. Data structures

All structs MUST be bit-identical across serialise/deserialise. Every
serialised size is rounded up to a multiple of 4 bytes (int32 LE
count) as per MEDIA016.

### `RngState_Compat`

```
struct RngState_Compat {
    uint32_t seed;
};
```

- `seed`  offset 0, `uint32_t LE`, 4 bytes.
- Total in-memory: 4 bytes. Serialised: `RNG_STATE_SERIALIZED_SIZE = 4`.

Pure LCG: `state = state * 1103515245 + 12345; return (state >> 16) &
0x7FFF`. This matches the behaviour pattern of Borland C `rand()` used
by the original PC 3.4 toolchain closely enough for determinism;
*bit-for-bit* match to original gameplay is NOT required (the original
was inherently non-deterministic due to uninitialised register values —
documented in `BUG0_81`).

### `CombatantChampionSnapshot_Compat`

Caller-produced snapshot of the attacking / defending champion. Pure
input; combat never touches the live `ChampionState_Compat`. This makes
determinism trivial and lets us test in isolation from phase 10.

```
struct CombatantChampionSnapshot_Compat {
    int      championIndex;        /* 0..3 */
    int      currentHealth;        /* 0 means dead → combat returns 0 */
    int      dexterity;            /* from F0311_CHAMPION_GetDexterity */
    int      strengthActionHand;   /* from F0312_CHAMPION_GetStrength */
    int      skillLevelParry;      /* from F0303 for C07_SKILL_PARRY */
    int      skillLevelAction;     /* from F0303 for the action's skill  */
    int      statisticVitality;    /* Statistics[C4_VITALITY][C1_CURRENT] */
    int      statisticAntifire;    /* for C1_ATTACK_FIRE defence        */
    int      statisticAntimagic;   /* for C5_ATTACK_MAGIC defence       */
    int      actionHandIcon;       /* C039_ICON_WEAPON_DIAMOND_EDGE etc. */
    int      wounds;               /* current wound bitmask (input)      */
    int      woundDefense[6];      /* per-slot pre-computed defence      */
    int      isResting;            /* 1 if party is resting              */
};
```

Field order matches serialise offsets (each 4-byte LE int).

- Total fields: 7 scalars + 6-array + 3 scalars + 1 = 13 + 6 = 19 int32.
- `COMBATANT_CHAMPION_SERIALIZED_SIZE = 76` (19 × 4).

### `CombatantCreatureSnapshot_Compat`

Snapshot of the creature attacker / defender. This replaces live access
to `G0243_as_Graphic559_CreatureInfo[]` — we don't load GRAPHICS.DAT
here; the caller fills this from a future graphics-loader phase (or from
a hand-rolled constants file for tests).

```
struct CombatantCreatureSnapshot_Compat {
    int creatureType;         /* 0..26 */
    int attack;                /* CREATURE_INFO.Attack                  */
    int defense;               /* CREATURE_INFO.Defense                 */
    int dexterity;             /* CREATURE_INFO.Dexterity               */
    int baseHealth;            /* CREATURE_INFO.BaseHealth              */
    int poisonAttack;          /* CREATURE_INFO.PoisonAttack            */
    int attackType;            /* C0..C7_ATTACK_*                       */
    int attributes;            /* CREATURE_INFO.Attributes (size, non-material, see-inv, etc.) */
    int woundProbabilities;    /* packed 4 × 4-bit                      */
    int properties;            /* CREATURE_INFO.Properties              */
    int doubledMapDifficulty;  /* G0269_ps_CurrentMap->Difficulty << 1  */
    int creatureIndex;         /* 0..3 slot inside the group            */
    int healthBefore;          /* Health[creatureIndex]                 */
};
```

- 13 int32 fields → `COMBATANT_CREATURE_SERIALIZED_SIZE = 52`.

### `WeaponProfile_Compat`

Caller-supplied weapon stats. Matches `WEAPON_INFO` subset.

```
struct WeaponProfile_Compat {
    int weaponType;        /* 0..45 (DUNGEON_WEAPON_TYPE_MAX) */
    int weaponClass;       /* C000_CLASS_SWING_WEAPON etc. */
    int weaponStrength;    /* WEAPON_INFO.Strength         */
    int kineticEnergy;     /* WEAPON_INFO.KineticEnergy    */
    int hitProbability;    /* per action, 0..255           */
    int damageFactor;      /* per action, 0..255           */
    int skillIndex;        /* CHAMPION_SKILL_FIGHTER etc.  */
    int attributes;        /* WEAPON_INFO.Attributes       */
};
```

- 8 int32 → `WEAPON_PROFILE_SERIALIZED_SIZE = 32`.
- `weaponType = -1` and `weaponClass = -1` → unarmed (fist). In this
  case `weaponStrength` is taken as the champion's action-hand strength
  contribution (0 for unarmed).

### `CombatAction_Compat`

One pending combat invocation — the data handed to
`F0737_COMBAT_ResolveChampionMelee_Compat` or the creature counterpart.

```
struct CombatAction_Compat {
    int kind;            /* COMBAT_ACTION_CHAMPION_MELEE,
                            COMBAT_ACTION_CREATURE_MELEE,
                            COMBAT_ACTION_APPLY_DAMAGE_CHAMPION,
                            COMBAT_ACTION_APPLY_DAMAGE_GROUP */
    int allowedWounds;   /* bitmask MASK0x0001..0x0020      */
    int attackTypeCode;  /* C0..C7_ATTACK_*                 */
    int rawAttackValue;  /* for APPLY_DAMAGE_* kinds        */
    int targetMapIndex;  /* for scheduling follow-up event  */
    int targetMapX;
    int targetMapY;
    int targetCell;
    int attackerSlotOrCreatureIndex;
    int defenderSlotOrCreatureIndex;
    int scheduleDelayTicks;  /* >0 = schedule follow-up; 0 = no event */
    int flags;               /* bit 0: useSharpDefense; bit 1: nonMaterialHit */
};
```

- 12 int32 → `COMBAT_ACTION_SERIALIZED_SIZE = 48`.

### `CombatResult_Compat`

Output of every resolver. Pure result; no pointers, no aliasing.

```
struct CombatResult_Compat {
    int outcome;                /* COMBAT_OUTCOME_*                   */
    int damageApplied;          /* final damage reaching HP           */
    int rawAttackRoll;          /* intermediate attack value          */
    int defenseRoll;            /* intermediate defence value         */
    int hitLanded;              /* 1 = hit, 0 = miss                  */
    int wasCritical;            /* 1 = extra damage branch taken      */
    int woundMaskAdded;         /* bits OR'd into champion.wounds     */
    int poisonAttackPending;    /* >0 → caller should call poison     */
    int targetKilled;           /* 1 = HP dropped to 0 / creature dead */
    int creatureSlotRemoved;    /* -1 unless a creature died in group  */
    int followupEventKind;      /* TIMELINE_EVENT_* or INVALID         */
    int followupEventAux0;      /* payload for scheduled event         */
    int rngCallCount;           /* how many times the RNG was consumed */
    int wakeFromRest;           /* 1 if caller should wake the party   */
};
```

- 14 int32 → `COMBAT_RESULT_SERIALIZED_SIZE = 56`.

### Outcome codes (DEFS.H:2844 mirror)

```
#define COMBAT_OUTCOME_MISS                     0
#define COMBAT_OUTCOME_HIT_NO_DAMAGE            1
#define COMBAT_OUTCOME_HIT_DAMAGE               2
#define COMBAT_OUTCOME_KILLED_NO_CREATURES      3  /* mirror C0_OUTCOME */
#define COMBAT_OUTCOME_KILLED_SOME_CREATURES    4  /* mirror C1_OUTCOME */
#define COMBAT_OUTCOME_KILLED_ALL_CREATURES     5  /* mirror C2_OUTCOME */
#define COMBAT_OUTCOME_CHAMPION_DOWN            6
#define COMBAT_OUTCOME_INVALID                 -1
```

### Static lookup tables (embedded in `.c`, not serialised)

1. **`WoundDefenseFactor[6]`** — mirror of
   `G0050_auc_Graphic562_WoundDefenseFactor`. Known values:
   `{0x15, 0x10, 0x1A, 0x1A, 0x12, 0x12}`. Used inside
   `F0735_COMBAT_ApplyChampionDefense_Compat`. Defined with that exact
   initialiser and a `static_assert(sizeof … == 6)` comment for the
   reader.
2. **`WoundProbabilityIndexToWoundMask[4]`** — mirror of
   `G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask`. Values
   `{MASK0x0002_WOUND_HEAD, MASK0x0010_WOUND_LEGS,
     MASK0x0004_WOUND_TORSO, MASK0x0020_WOUND_FEET}` = `{0x02, 0x10,
   0x04, 0x20}`. Used in both creature and champion attack paths.
3. **`AttackSize_ToExplosionAttack[3]`** — `{110, 190, 255}`. Mirrors
   the `SIZE_QUARTER_SQUARE / HALF / FULL` branch in
   `F0190_GROUP_GetDamageCreatureOutcome`. Used only to set
   `followupEventAux0` for the future explosion/smoke event (we don't
   create the explosion itself).

### Integration with existing structs

- `CombatResult_Compat.followupEventKind` takes values from the
  `TIMELINE_EVENT_*` set already defined in
  `memory_timeline_pc34_compat.h`. Typical kinds emitted by phase 13:
  `TIMELINE_EVENT_STATUS_TIMEOUT` (hide-damage flash, mirrors
  `C12_EVENT_HIDE_DAMAGE_RECEIVED`),
  `TIMELINE_EVENT_CREATURE_TICK` (attack cooldown).
- A helper `F0746_COMBAT_BuildTimelineEvent_Compat` converts
  `(CombatAction, CombatResult, nowTick)` into a fully populated
  `TimelineEvent_Compat` using the existing 11-field layout. No changes
  to the timeline header.
- No modifications to `ChampionState_Compat`, `PartyState_Compat`,
  `DungeonThings_Compat`, `DungeonGroup_Compat`, `TimelineQueue_Compat`,
  or any earlier phase symbol. Phase 13 is *additive only*.
- Size-constant collision check: Phase 12 uses
  `TIMELINE_EVENT_SERIALIZED_SIZE=44`,
  `TIMELINE_QUEUE_SERIALIZED_SIZE=11272`. Phase 10 uses
  `CHAMPION_SERIALIZED_SIZE=256`, `PARTY_SERIALIZED_SIZE=1056`. Phase 11
  uses 28 / 228. Phase 13's 4 / 32 / 48 / 52 / 56 / 76 are all distinct.
  No `#define` name collides (all prefixed `COMBAT_` / `RNG_` /
  `WEAPON_PROFILE_` / `COMBATANT_`).

---

## 3. Function API (F0730 – F0747)

All functions live in `memory_combat_pc34_compat.{h,c}`. All are pure
unless explicitly noted. Conventions:

- Input pointers: `const` always.
- Output pointers: non-`const`, caller-owned.
- Return: `1` on success, `0` on invalid argument / bounds failure, or
  an explicit `COMBAT_OUTCOME_*` value for resolvers (documented per
  function).

### Group A — RNG primitives (pure, side-effect through out-param only)

| # | Signature | Role |
|---|-----------|------|
| F0730 | `int F0730_COMBAT_RngInit_Compat(struct RngState_Compat* rng, uint32_t seed);` | Set seed. Returns 1. |
| F0731 | `uint32_t F0731_COMBAT_RngNextRaw_Compat(struct RngState_Compat* rng);` | Advance LCG, return full 32-bit state (for serialise tests). |
| F0732 | `int F0732_COMBAT_RngRandom_Compat(struct RngState_Compat* rng, int modulus);` | Mirror of `M003_RANDOM(n)` — returns `0..modulus-1` via `(raw >> 16) & 0x7FFF) % modulus`. `modulus<=0` → returns 0. |

### Group B — Defence helpers (pure, no RNG)

| # | Signature | Role |
|---|-----------|------|
| F0733 | `int F0733_COMBAT_GetChampionWoundDefense_Compat(const struct CombatantChampionSnapshot_Compat* champ, int woundSlotIndex, int useSharpDefense, int* outDefense);` | Mirror of `F0313_CHAMPION_GetWoundDefense` (CHAMPION.C:1305) reduced to a lookup over `champ->woundDefense[]` + shield attribute. |
| F0734 | `int F0734_COMBAT_GetStatisticAdjustedAttack_Compat(int statisticCurrent, int statisticMaximum, int attack, int* outAdjusted);` | Mirror of `F0307_CHAMPION_GetStatisticAdjustedAttack` (CHAMPION.C:1106). Uses `F0030_MAIN_GetScaledProduct` inline. No RNG. |

### Group C — Resolvers (pure + RNG-via-out-param)

| # | Signature | Role |
|---|-----------|------|
| F0735 | `int F0735_COMBAT_ResolveChampionMelee_Compat(const struct CombatantChampionSnapshot_Compat* attacker, const struct WeaponProfile_Compat* weapon, const struct CombatantCreatureSnapshot_Compat* defender, struct RngState_Compat* rng, struct CombatResult_Compat* out);` | Mirror of `F0231_GROUP_GetMeleeActionDamage` (PROJEXPL.C:1416), stripped. Sets `out->outcome`, `damageApplied`, `rawAttackRoll`, `defenseRoll`, `hitLanded`, `wasCritical`, `followupEventKind=TIMELINE_EVENT_CREATURE_TICK`. Does *not* update defender — caller must pass the result to F0740. Advances `rng` by a deterministic count (recorded in `out->rngCallCount`). Returns 1. |
| F0736 | `int F0736_COMBAT_ResolveCreatureMelee_Compat(const struct CombatantCreatureSnapshot_Compat* attacker, const struct CombatantChampionSnapshot_Compat* defender, struct RngState_Compat* rng, struct CombatResult_Compat* out);` | Mirror of `F0230_GROUP_GetChampionDamage` (PROJEXPL.C:1305). Sets `out->woundMaskAdded`, `damageApplied`, `poisonAttackPending`, `wakeFromRest`, `followupEventKind=TIMELINE_EVENT_STATUS_TIMEOUT`. Returns 1. |

### Group D — Application (pure, mutates out-only struct)

| # | Signature | Role |
|---|-----------|------|
| F0737 | `int F0737_COMBAT_ApplyDamageToChampion_Compat(const struct CombatResult_Compat* result, struct ChampionState_Compat* champ, int* outWasKilled);` | Mirror of `F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage` (CHAMPION.C:1803) tail. Decrements `champ->hp.current`, OR's `woundMaskAdded` into `champ->wounds`, clamps to 0 → `*outWasKilled=1`. Does NOT touch mana/stamina/food/water/poison. Returns 1. |
| F0738 | `int F0738_COMBAT_ApplyDamageToGroup_Compat(const struct CombatResult_Compat* result, struct DungeonGroup_Compat* group, int creatureIndex, int* outOutcome);` | Mirror of `F0190_GROUP_GetDamageCreatureOutcome` (GROUP.C:769) reduced. Only updates `group->health[creatureIndex]` and `group->count` when a slot dies. Does NOT drop possessions, cancel events, or update `cells` / `direction` packing (those are phase-14+ work — flagged as `NEEDS DISASSEMBLY REVIEW` if we want to collapse survivors later). Sets `*outOutcome` to one of the three C0..C2 codes. Returns 1. |

### Group E — Timeline bridge (pure)

| # | Signature | Role |
|---|-----------|------|
| F0739 | `int F0739_COMBAT_BuildTimelineEvent_Compat(const struct CombatAction_Compat* action, const struct CombatResult_Compat* result, uint32_t nowTick, struct TimelineEvent_Compat* outEvent);` | Pure converter. Populates `outEvent->kind = result->followupEventKind`, `outEvent->fireAtTick = nowTick + action->scheduleDelayTicks`, and the location / aux fields from action + result. Returns 1 if `followupEventKind != TIMELINE_EVENT_INVALID`, else 0 (meaning "no event to schedule" — not an error). |

### Group F — Serialisation (pure)

| # | Signature | Role |
|---|-----------|------|
| F0740 | `int F0740_COMBAT_ActionSerialize_Compat(const struct CombatAction_Compat* action, unsigned char* outBuf, int outBufSize);` | 48 bytes LE. |
| F0741 | `int F0741_COMBAT_ActionDeserialize_Compat(struct CombatAction_Compat* action, const unsigned char* buf, int bufSize);` | 48 bytes LE. |
| F0742 | `int F0742_COMBAT_ResultSerialize_Compat(const struct CombatResult_Compat* result, unsigned char* outBuf, int outBufSize);` | 56 bytes LE. |
| F0743 | `int F0743_COMBAT_ResultDeserialize_Compat(struct CombatResult_Compat* result, const unsigned char* buf, int bufSize);` | 56 bytes LE. |
| F0744 | `int F0744_COMBAT_ChampionSnapshotSerialize_Compat(...)` | 76 bytes LE. |
| F0745 | `int F0745_COMBAT_ChampionSnapshotDeserialize_Compat(...)` | 76 bytes LE. |
| F0746 | `int F0746_COMBAT_CreatureSnapshotSerialize_Compat(...)` | 52 bytes LE. |
| F0747 | `int F0747_COMBAT_CreatureSnapshotDeserialize_Compat(...)` | 52 bytes LE. |

(We serialise `WeaponProfile_Compat` too with a helper — numbered
`F0747b_COMBAT_WeaponProfileSerialize_Compat` / `_Deserialize_Compat`.
These share F0747 space because the helper doesn't need a top-level
public number; they're marked `/* internal numbering F0747.1/.2 */`
in the header comment.)

### DUNGEON.DAT dependency

None of Phase 13's functions read from `DUNGEON.DAT` directly. They
operate entirely on caller-supplied snapshots.  The probe DOES load
`DUNGEON.DAT` for spot-checks, but that's test infrastructure, not
library code.

---

## 4. Algorithm specifications

### 4.1  `F0735 — champion melee resolver`

Reference: `F0231_GROUP_GetMeleeActionDamage`, PROJEXPL.C:1416–1553.

```
pseudocode F0735(attacker, weapon, defender, rng, out):
    zero(out);
    out->outcome = COMBAT_OUTCOME_MISS;
    out->followupEventKind = TIMELINE_EVENT_CREATURE_TICK;

    if attacker->championIndex < 0 || >= CHAMPION_MAX_PARTY: return 1
    if attacker->currentHealth <= 0: return 1
    if defender->creatureType > DUNGEON_CREATURE_TYPE_MAX: return 1

    doubledMapDifficulty = defender->doubledMapDifficulty
    nonMaterial = (defender->attributes >> 6) & 1        /* MASK0x0040 */
    hitProb = weapon->hitProbability & 0x7FFF
    actionHitsNonMat = (weapon->hitProbability >> 15) & 1  /* MASK0x8000 */

    /* Dexterity duel (PROJEXPL.C:1439–1445) */
    rand1 = F0732_RngRandom(rng, 32)                 ; out->rngCallCount++
    dexThreshold = rand1 + defender->dexterity + doubledMapDifficulty - 16
    lucky = 0                                        /* isLucky deferred */
    dexOk = (attacker->dexterity > dexThreshold)

    rand2 = F0732_RngRandom(rng, 4)                  ; out->rngCallCount++
    rand2IsZero = (rand2 == 0)

    if (!(nonMaterial) || actionHitsNonMat) && (dexOk || rand2IsZero):
        /* Hit landed */
        out->hitLanded = 1
        baseDamage = attacker->strengthActionHand
        if baseDamage == 0: goto weak_branch
        bonus = F0732_RngRandom(rng, (baseDamage>>1)+1) ; out->rngCallCount++
        baseDamage += bonus
        baseDamage = (baseDamage * weapon->damageFactor) >> 5
        defense = F0732_RngRandom(rng,32) + defender->defense + doubledMapDifficulty ; out->rngCallCount++
        /* Weapon-specific armour piercing */
        if weapon->actionHandIcon == C039_DIAMOND_EDGE: defense -= defense>>2
        else if == C043_HARDCLEAVE_EXECUTIONER:        defense -= defense>>3
        damage0 = F0732_RngRandom(rng,32) + baseDamage - defense ; out->rngCallCount++
        if damage0 <= 1:
            weak_branch:
            r = F0732_RngRandom(rng, 4); out->rngCallCount++
            if r == 0: out->damageApplied = 0; goto done
            baseDamage = r + 1
            delta = F0732_RngRandom(rng,16); out->rngCallCount++
            damage0 += delta
            if damage0 > 0 || F0732_RngRandom(rng,2) != 0:
                baseDamage += F0732_RngRandom(rng,4); out->rngCallCount += 1..2
                if F0732_RngRandom(rng,4) == 0:
                    baseDamage += max(0, damage0 + F0732_RngRandom(rng,16))
                    out->wasCritical = 1
        baseDamage >>= 1
        baseDamage += F0732_RngRandom(rng, baseDamage) + F0732_RngRandom(rng,4) ; rngCallCount+=2
        baseDamage += F0732_RngRandom(rng, baseDamage)                         ; rngCallCount++
        baseDamage >>= 2
        baseDamage += F0732_RngRandom(rng,4) + 1                               ; rngCallCount++
        /* Vorpal halves damage on non-non-material creatures */
        if weapon->actionHandIcon == C040_VORPAL_BLADE && !nonMaterial:
            baseDamage >>= 1
            if baseDamage == 0: out->damageApplied = 0; goto done
        /* Skill bonus */
        if F0732_RngRandom(rng,64) < attacker->skillLevelAction: rngCallCount++
            baseDamage = baseDamage + baseDamage + 10
        out->damageApplied = baseDamage
        out->outcome = (baseDamage > 0) ? COMBAT_OUTCOME_HIT_DAMAGE
                                        : COMBAT_OUTCOME_HIT_NO_DAMAGE
done:
    return 1
```

Any branch where the original called `F0308_CHAMPION_IsLucky` is
collapsed to `dexOk || rand2IsZero` — we mark this with
`/* NEEDS DISASSEMBLY REVIEW: luck state. v1 always returns 0. */` in
the .c, per task rules (no fabrication).

### 4.2  `F0736 — creature melee resolver`

Reference: `F0230_GROUP_GetChampionDamage`, PROJEXPL.C:1305–1414.

```
pseudocode F0736(attacker, defender, rng, out):
    zero(out); out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
    if defender->currentHealth <= 0: return 1
    if defender->championIndex >= CHAMPION_MAX_PARTY: return 1
    if defender->isResting: out->wakeFromRest = 1  /* flagged, not acted */

    /* Dexterity duel. MEDIA064 path, matches PROJEXPL.C:1326 */
    r = F0732_RngRandom(rng,32); out->rngCallCount++
    if defender->dexterity < r + attacker->dexterity + attacker->doubledMapDifficulty - 16
       || F0732_RngRandom(rng,4)==0:
        rngCallCount++; /* account for second random */
        out->hitLanded = 1

        /* Wound mask */
        woundTest = F0732_RngRandom(rng, 65536); rngCallCount++
        if woundTest & 0x0070:
            woundTest &= 0x000F
            probs = attacker->woundProbabilities
            idx = 0
            while woundTest > (probs & 0x000F):
                probs >>= 4; idx++
            if idx > 3: idx = 3   /* guard — Fontanel assumes <=3 */
            out->woundMaskAdded = WoundProbabilityIndexToWoundMask[idx]
        else:
            out->woundMaskAdded = (woundTest & 0x0001)   /* READY_HAND */

        /* Attack value */
        rnd16 = F0732_RngRandom(rng,16); rngCallCount++
        atk = (rnd16 + attacker->attack + attacker->doubledMapDifficulty)
            - (defender->skillLevelParry << 1)
        if atk <= 1:
            r2 = F0732_RngRandom(rng,2); rngCallCount++
            if r2 != 0:
                goto miss
            atk = F0732_RngRandom(rng,4) + 2; rngCallCount++
        atk >>= 1
        atk += F0732_RngRandom(rng,atk) + F0732_RngRandom(rng,4); rngCallCount+=2
        atk += F0732_RngRandom(rng,atk); rngCallCount++
        atk >>= 2
        atk += F0732_RngRandom(rng,4) + 1; rngCallCount++

        /* Defence scaled by champion stats based on attackType */
        atk = apply_defenders_statistic_adjustment(attacker->attackType, defender, atk)
        if atk <= 0: goto miss
        out->damageApplied = atk
        out->rawAttackRoll = atk
        out->outcome = COMBAT_OUTCOME_HIT_DAMAGE

        /* Poison follow-up trigger (flag only, caller applies) */
        if attacker->poisonAttack != 0 && F0732_RngRandom(rng,2) != 0: rngCallCount++
            out->poisonAttackPending = attacker->poisonAttack
        return 1
    miss:
    out->outcome = COMBAT_OUTCOME_MISS
    return 1
```

`apply_defenders_statistic_adjustment` is the truncated equivalent of
CHAMPION.C:1824–1910 limited to `C0_NORMAL`, `C2_SELF`, `C3_BLUNT`,
`C4_SHARP`, `C7_LIGHTNING` (fire/magic/psychic go through the deferred
magic path and fall back to no adjustment in v1, marked with an inline
`/* NEEDS DISASSEMBLY REVIEW */` tag for phase 14).

### 4.3  `F0737 — apply damage to champion`

Reference: CHAMPION.C:1697–1710.

```
pseudocode F0737(result, champ, outKilled):
    if champ->hp.current == 0: *outKilled = 1; return 1
    newHp = champ->hp.current - result->damageApplied
    if newHp <= 0:
        champ->hp.current = 0
        *outKilled = 1
    else:
        champ->hp.current = newHp
        *outKilled = 0
    champ->wounds |= result->woundMaskAdded
    return 1
```

### 4.4  `F0738 — apply damage to group`

Reference: GROUP.C:826–836 (the simple HP branch), plus the group-count
decrement on kill. We do NOT squash surviving creatures' cells/directions
— that's tightly coupled to phase-14 active-group animations.

```
pseudocode F0738(result, group, slot, outOutcome):
    if group->health[slot] > result->damageApplied:
        group->health[slot] -= result->damageApplied
        *outOutcome = COMBAT_OUTCOME_KILLED_NO_CREATURES
    else:
        group->health[slot] = 0
        if group->count == 0:
            *outOutcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES
        else:
            group->count -= 1
            *outOutcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES
    return 1
```

Caveat comment in the source:
`/* NEEDS DISASSEMBLY REVIEW: cell / direction packing reshuffle on kill
    — deferred to phase 14, because it's tangled with ACTIVE_GROUP
    state which we don't model yet. v1 leaves .cells and .direction
    untouched; the visual consequence is acceptable since we're not
    rendering. */`

### 4.5  `F0739 — timeline-event builder`

Mirror of CHAMPION.C:1777 (HIDE_DAMAGE_RECEIVED) and PROJEXPL.C (CREATURE
cool-down). Pure converter, no side effect.

### 4.6  RNG primitives

Linear congruential; see §2 for formula. `F0732` modulus-0 guard returns
0. Fontanel's `M003_RANDOM(n)` is non-deterministic due to Borland's
`rand()` seed history; we don't need bit-match — only determinism
relative to our own seed.

---

## 5. Invariant list (≥30, target 35)

| # | Category | Invariant |
|---|----------|-----------|
| 1  | size    | `RNG_STATE_SERIALIZED_SIZE == 4` |
| 2  | size    | `COMBAT_ACTION_SERIALIZED_SIZE == 48` |
| 3  | size    | `COMBAT_RESULT_SERIALIZED_SIZE == 56` |
| 4  | size    | `COMBATANT_CHAMPION_SERIALIZED_SIZE == 76` |
| 5  | size    | `COMBATANT_CREATURE_SERIALIZED_SIZE == 52` |
| 6  | size    | `WEAPON_PROFILE_SERIALIZED_SIZE == 32` |
| 7  | round-trip | Empty `CombatAction_Compat` → bytes → struct == original |
| 8  | round-trip | Populated `CombatAction_Compat` (all non-zero) round-trips bit-identical |
| 9  | round-trip | Empty `CombatResult_Compat` round-trips bit-identical |
| 10 | round-trip | Populated `CombatResult_Compat` (outcome=HIT_DAMAGE, wounds set) round-trips bit-identical |
| 11 | round-trip | Populated `CombatantChampionSnapshot_Compat` round-trips bit-identical |
| 12 | round-trip | Populated `CombatantCreatureSnapshot_Compat` round-trips bit-identical |
| 13 | round-trip | Populated `WeaponProfile_Compat` round-trips bit-identical |
| 14 | RNG     | `F0730` with seed=0 → first `F0731` returns a specific golden uint32 |
| 15 | RNG     | `F0732(rng,100)` always in `[0,99]` over 1000 draws |
| 16 | RNG     | `F0732(rng,0)` returns 0 and does not advance state |
| 17 | determinism | Two rngs seeded identically → `F0735` with same inputs yields byte-identical `CombatResult_Compat` |
| 18 | determinism | Two rngs seeded identically → `F0736` with same inputs yields byte-identical `CombatResult_Compat` |
| 19 | purity  | `F0735` does not modify its attacker/defender/weapon inputs (checksum pre/post) |
| 20 | purity  | `F0736` does not modify its attacker/defender inputs (checksum pre/post) |
| 21 | purity  | `F0739` does not modify action/result inputs (checksum pre/post) |
| 22 | boundary | `F0735` with NULL attacker → returns 0, no crash |
| 23 | boundary | `F0735` with attacker.currentHealth=0 → outcome MISS, damage 0 |
| 24 | boundary | `F0736` with defender.currentHealth=0 → outcome MISS, damage 0 |
| 25 | boundary | `F0737` with damage≥hp → `champ->hp.current==0` and `*outKilled==1` |
| 26 | boundary | `F0738` group.health[slot]=5, damage=5 on last creature (count=0) → outcome KILLED_ALL |
| 27 | boundary | `F0738` group.health[slot]=5, damage=2 → slot health becomes 3, outcome KILLED_NO |
| 28 | boundary | `F0741` with `bufSize=47` → returns 0 (too small for 48-byte action) |
| 29 | boundary | `F0743` with `bufSize=55` → returns 0 |
| 30 | known-value | A Mummy-like creature (attack=24, dex=30, defense=18, attackType=C3_BLUNT) vs a champion (dex=40, hp=50, parry=0, rest=0), seed=0xC0FFEE, runs `F0736` and matches a precomputed result recorded in the probe (golden JSON in `verification-m10/combat/golden_mummy_vs_hero.txt`) |
| 31 | known-value | A Ninja champion (strength=60, dexterity=55, skillAction=10, vorpal blade icon) vs a Skeleton (defense=22, dex=28, attributes=MASK_SHARP, HP=40), seed=0x12345678, runs `F0735` and matches a precomputed golden result |
| 32 | integration | `F0739` produces a `TimelineEvent_Compat` whose `F0725_TIMELINE_EventSerialize_Compat` output is 44 bytes (matches phase 12 contract) |
| 33 | integration | A combat event scheduled via `F0739`+`F0721` into a phase-12 queue survives round-trip `F0727`/`F0728` bit-identical |
| 34 | integration | Spot-check against DUNGEON.DAT: for every `DungeonGroup_Compat` in `things.groups[0..groupCount-1]`, `creatureType ≤ DUNGEON_CREATURE_TYPE_MAX` (26) — this gates that our combat inputs can be fed from real data |
| 35 | integration | A killed champion (`F0737` leaves hp=0) → subsequent `F0736` against the same champion returns outcome MISS, damage 0 (combat ignores dead targets, mirroring CHAMPION.C:1814) |

The probe prints `- PASS: …` / `- FAIL: …` per invariant and the
trailing `Invariant count: 35` + `Status: PASS` line. Counting >30 gives
headroom for the verify script.

---

## 6. Implementation order for the Codex agent

Strict linear sequence. Compile after every step. If step N fails, fix
before touching step N+1. No renames mid-task (previous Phase-11 agent
failed here).

### Step 1 — Write `.h`
- Create `memory_combat_pc34_compat.h` with:
  - All `#define` constants from §2.
  - Every struct definition from §2.
  - Every function prototype from §3.
- Syntax smoke-check:
  ```
  cc -Wall -Wextra -c -o /tmp/combat_h_check.o -x c \
      <(echo '#include "memory_combat_pc34_compat.h"'; \
        echo 'int main(void){return 0;}') \
      -I/Users/bosse/.openclaw/workspace-main/tmp/firestaff
  ```
- Must compile with no warnings.

### Step 2 — Write `.c` stub
- Create `memory_combat_pc34_compat.c` with all function bodies returning
  `0` / `COMBAT_OUTCOME_INVALID` / zeroed out-params.
- Include `<string.h>`, `<stdint.h>`, `"memory_combat_pc34_compat.h"`,
  `"memory_timeline_pc34_compat.h"`, `"memory_champion_state_pc34_compat.h"`.
- Compile standalone:
  ```
  cc -Wall -Wextra -c memory_combat_pc34_compat.c -o /tmp/combat_stub.o \
      -I/Users/bosse/.openclaw/workspace-main/tmp/firestaff
  ```
- Must compile with `-Wall -Wextra` clean.

### Step 3 — Fill implementation, group by group
Order: RNG → serialise → defence helpers → resolvers → application →
timeline builder. Compile after each group:
- 3a. F0730–F0732 (RNG). Compile.
- 3b. F0740–F0747 (all serialise/deserialise pairs). Compile.
- 3c. F0733–F0734 (defence helpers). Compile.
- 3d. F0735 (champion melee resolver). Compile.
- 3e. F0736 (creature melee resolver). Compile.
- 3f. F0737–F0738 (damage application). Compile.
- 3g. F0739 (timeline builder). Compile.

### Step 4 — Write `firestaff_m10_combat_probe.c`
- Start with `main` that opens report file, opens invariants file, writes
  header, defines `CHECK` macro (same pattern as
  `firestaff_m10_timeline_probe.c`), and closes.
- Add a single invariant (the RNG_STATE size check). Build with a new
  driver script (see Step 6). Run once. Confirm `timeline_invariants.md`-style
  output works.

### Step 5 — Add invariants incrementally
- Block A: invariants 1–6 (size constants). Build + run.
- Block B: invariants 7–13 (round-trips). Build + run.
- Block C: invariants 14–16 (RNG). Build + run.
- Block D: invariants 17–21 (determinism + purity). Build + run.
- Block E: invariants 22–29 (boundary / nulls). Build + run.
- Block F: invariants 30–31 (known-value goldens).
  - For each golden, the probe first computes the actual result,
    prints it to `combat_probe.md` as a fenced block, THEN compares
    against a hardcoded expected byte string committed in the probe.
    To avoid a chicken-and-egg failure, follow this discipline:
    1. Commit the probe with a placeholder expected value and
       `CHECK(0, "golden mummy vs hero")` — this will fail.
    2. Run the probe. Copy the actual bytes printed to the report into
       the expected value constant. Recompile. Now the check passes.
  - Same for the ninja-vs-skeleton golden.
- Block G: invariants 32–35 (integration with phase 12 + DUNGEON.DAT).
  The DUNGEON.DAT spot-check requires loading via
  `F0504_DUNGEON_LoadThingData_Compat` — the probe takes a second
  argv `<dungeon.dat path>` just like the sensor-execution probe.

### Step 6 — Driver script `run_firestaff_m10_combat_probe.sh`
- Mirror `run_firestaff_m10_timeline_probe.sh`. Two argv:
  `$1 = DUNGEON.DAT`, `$2 = output dir`.
- Compile with:
  ```
  cc -Wall -Wextra -O1 -I"$ROOT" \
      -o "$PROBE_BIN" \
      "$ROOT/firestaff_m10_combat_probe.c" \
      "$ROOT/memory_combat_pc34_compat.c" \
      "$ROOT/memory_timeline_pc34_compat.c" \
      "$ROOT/memory_dungeon_dat_pc34_compat.c" \
      "$ROOT/memory_champion_state_pc34_compat.c"
  ```
- Invoke `"$PROBE_BIN" "$1" "$2"`.
- `chmod +x`.

### Step 7 — Append to `run_firestaff_m10_verify.sh`
- Find the last `# Phase 12: Timeline` block. Immediately after its
  closing `PY` + blank line, but *before* the final `echo "=== M10
  verification complete ==="`, append exactly ONE block following the
  phase-12 template.
- The block content:
  ```
  # Phase 13: Combat system probe
  echo "=== Phase 13: M10 combat probe ==="
  COMBAT_DIR="$OUT_DIR/combat"
  "$ROOT/tmp/firestaff/run_firestaff_m10_combat_probe.sh" "$DUNGEON_DAT" "$COMBAT_DIR" || {
      echo "FAIL: M10 combat probe did not pass"
      exit 1
  }
  echo "M10 combat probe: PASS"

  python3 - <<'PY' "$COMBAT_DIR/combat_invariants.md" "$SUMMARY_MD" "$COMBAT_DIR"
  (verbatim copy of the timeline block, with "timeline" → "combat")
  PY
  ```
- After the edit, grep check: `grep -c '^# Phase 13:'
  run_firestaff_m10_verify.sh` must equal **1**.
- Do NOT repeat the append; if in doubt, `grep -n` before writing.
  (Previous agent ran into this exact footgun 4× over.)

### Step 8 — Full verify
- Run `bash run_firestaff_m10_verify.sh /Users/bosse/.openclaw/data/redmcsb-original/DungeonMasterPC34/DATA/DUNGEON.DAT /tmp/m10-verify-out`
  from the repo root. Exit must be 0. If any earlier phase regresses
  (unlikely — we added files only), `git diff` the verify script and
  revert any accidental non-phase-13 change.

---

## 7. Files to create + modify

| Path | Action | Estimated size |
|------|--------|----------------|
| `tmp/firestaff/memory_combat_pc34_compat.h` | CREATE | ~7 KB (~230 lines) |
| `tmp/firestaff/memory_combat_pc34_compat.c` | CREATE | ~18 KB (~650 lines, inc. F0735 resolver ≈220 lines) |
| `tmp/firestaff/firestaff_m10_combat_probe.c` | CREATE | ~22 KB (~720 lines; 35 invariants × ~15 lines each + scaffolding) |
| `tmp/firestaff/run_firestaff_m10_combat_probe.sh` | CREATE | ~0.8 KB (~25 lines) |
| `tmp/firestaff/run_firestaff_m10_verify.sh` | MODIFY | +34 lines appended exactly once |

No other files touched. No `.phase*-attempt-*` backups to be created.
If the Codex agent feels an urge to stash a side copy, DON'T — revert in
place.

---

## 8. Risk register

| # | Risk | Likelihood | Impact | Plan B |
|---|------|------------|--------|--------|
| R1 | Serialisation size collision with earlier phases | Very low — verified distinct | Medium (breaks round-trip) | Rename `COMBAT_*_SERIALIZED_SIZE` with a `_COMBAT_` prefix if any future phase claims it. |
| R2 | Agent re-appends phase 13 block multiple times to verify script (Phase 11 precedent) | Medium | High (test gate silently duplicates / passes) | Use `grep -c '^# Phase 13:' run_firestaff_m10_verify.sh` **after each append attempt**; if != 1, `git checkout` the script and retry. Step 7 explicitly pre-checks with grep before writing. |
| R3 | Fontanel algorithm branches on `F0308_CHAMPION_IsLucky` which needs hidden champion state we don't have | High — this is assembly-era code | Medium (combat determinism drifts from reference) | Collapse to `luck=0` with a `/* NEEDS DISASSEMBLY REVIEW: luck state. */` comment. Document in the probe header. The determinism invariants still pass because we're self-consistent. |
| R4 | Fire/magic/psychic attack-type defence paths depend on SpellShieldDefense / FireShieldDefense that don't exist yet | High | Low in v1 (we restrict creature.attackType to C0/C2/C3/C4/C7 in the probe goldens) | `F0736` handles C0/C2/C3/C4/C7 in full; C1/C5/C6 fall through to "no adjustment + `/* REVIEW */`" and the v1 probe does not exercise those paths. |
| R5 | `F0190`'s group-cell/direction reshuffle is tangled with ACTIVE_GROUP (not yet ported) | High | Low — our simplified `F0738` leaves those packed fields alone. Visual ordering wrong until phase 14, but HP math is correct | Document the deviation in `.c` header. Add invariant 34 confirming creature types stay in range. |
| R6 | `CREATURE_INFO` / `WEAPON_INFO` tables live in `GRAPHICS.DAT` entry 559, not yet loaded | Certain | Would block known-value spot checks if we required a loader | By design: phase 13 takes caller-supplied snapshots. Goldens (invariants 30–31) use hand-entered constants matching DM's published Mummy / Skeleton stats, *not* a runtime lookup. Loader is a future phase. |
| R7 | RNG output disagrees with Borland `rand()` bit-for-bit | Certain | None — we never claimed reference-accurate RNG | Ship the LCG as documented. Determinism relative to our seed is the contract. |
| R8 | 45-min budget overrun (probe writing is the long pole) | Low-Medium | Delays phase | Invariant blocks are incremental (Step 5 blocks A–G); if time is tight, Block F (goldens) may be reduced to one golden instead of two and still exceed the 30-invariant bar. |

Top two risks (by expected cost): **R2 (duplicate verify-script append)** and **R3 (luck-state fabrication)**.

---

## 9. Acceptance criteria

Phase 13 is complete and ready for merge when ALL of the following hold:

- [ ] `bash run_firestaff_m10_verify.sh <dungeon.dat> <out>` exits with
      status 0.
- [ ] `grep -c '^# Phase 13:' run_firestaff_m10_verify.sh` equals exactly **1**.
- [ ] `$OUT_DIR/combat/combat_probe.md` exists and is non-empty.
- [ ] `$OUT_DIR/combat/combat_invariants.md` exists, its trailing line
      is `Status: PASS`, and it contains `Invariant count: N` where `N
      ≥ 30` and every line starting with `- ` begins with `- PASS:`.
- [ ] `cc -Wall -Wextra -c` of every new `.c` file emits zero warnings.
- [ ] `ls tmp/firestaff/.phase*-attempt-* 2>/dev/null` returns
      nothing — no orphan backup directories.
- [ ] `git status` shows only the five files listed in §7 (no
      collateral edits).
- [ ] All 12 previous phases still pass — confirmed by the verify script
      exiting 0.

---

*End of plan. The Codex ACP executor should read this file in full,
then follow §6 steps 1→8 without deviation. Any ambiguity = stop and
emit a review-request comment rather than fabricate.*
