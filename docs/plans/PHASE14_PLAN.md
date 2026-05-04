# PHASE 14 PLAN — Magic / Spell Casting System (v1)

Firestaff M10 milestone, Phase 14. Status at planning time: 13 phases PASS.
This document is the *single source of truth* the Codex ACP executor
follows. Any deviation = abort and ask.

Style rules (non-negotiable, inherited from prior phases):

- All symbols end `_pc34_compat` / `_Compat`.
- MEDIA016 / PC LSB-first serialisation. Every struct round-trips
  bit-identical.
- Pure functions: NO globals, NO UI, NO IO (the probe may open
  DUNGEON.DAT only to confirm we don't depend on it). No hidden RNG
  state — all randomness flows through an explicit
  `RngState_Compat*` (phase 13's primitive, reused).
- Function numbering continues after phase 13 (F0730–F0747).
  Phase 14 uses **F0750–F0769**.
- Every probe writes `magic_probe.md` and `magic_invariants.md` with
  trailing `Status: PASS`.
- Verify gate: `run_firestaff_m10_verify.sh` gets exactly one new
  `# Phase 14:` block appended. `grep -c '^# Phase 14:' …` must equal
  1 after the edit.
- ADDITIVE ONLY: no edits to `memory_combat_pc34_compat.{h,c}`,
  `memory_timeline_pc34_compat.{h,c}`, or any earlier-phase file.
  Phase 14 unlocks Phase 13's stubbed branches by *composing* a
  magic-aware adjustment around them, not by patching them.

---

## 1. Scope definition

### In scope for Phase 14 v1

A **pure data-layer magic module** that covers the cast-pipeline from
rune input to effect production, plus the magic-side state that
Phase 13's combat resolver currently stubs out. The module is
deterministic, caller-driven, and never executes a side-effect (events
are *produced* as `TimelineEvent_Compat` blobs for the caller to
schedule).

1. **Rune-sequence encoding.** Four-symbol sequence (power / element /
   form / class) packed into a 24-bit or 32-bit word, matching
   Fontanel's `F0409_MENUS_GetSpellFromSymbols` (MENU.C:1666).
   Power rune is optional (high byte == 0 → spell definition without
   power).
2. **Spell-definition table.** The 25 DM1 spells from
   `G0487_as_Graphic560_Spells` (MENU.C:50–78), hard-coded as a
   static lookup table (NOT read from DUNGEON.DAT — these live in
   `GRAPHICS.DAT` entry 560 and we don't have a graphics loader yet;
   hand-entered constants mirror the published reference list).
3. **Mana-cost computation.** Per-symbol cost per
   `F0399_MENUS_AddChampionSymbol` (SYMBOL.C):
   `base = G0485[step][symbolIdx]`;
   if `step > 0` then `cost = (base * G0486[powerOrdinal]) >> 3`.
   Tables:
   `G0485[4][6] = {{1,2,3,4,5,6},{2,3,4,5,6,7},{4,5,6,7,7,9},{2,2,3,4,6,7}}`
   and `G0486[6] = {8,12,16,20,24,28}`.
4. **Cast validation + success probability.** Skill-vs-required check
   from `F0412_MENUS_GetChampionSpellCastResult` (MENU.C:1755):
   required = `spell->BaseRequiredSkillLevel + powerOrdinal`;
   if `skill < required`, for each missing level roll
   `M003_RANDOM(128) > min(115, wisdom+15)` — any failure returns
   `SPELL_CAST_FAILURE`.
5. **Effect production for the three major spell *kinds*:**
   - `C2_SPELL_KIND_PROJECTILE` (fireball / poison bolt / poison
     cloud / lightning / open door / weaken non-material / harm
     non-material): produce a `SpellEffect_Compat` encoding impact
     attack + kinetic energy from
     `F0026_MAIN_GetBoundedValue(21, (powerOrdinal+2) * (4 + (skill<<1)), 255)`
     (MENU.C:1826).
   - `C3_SPELL_KIND_OTHER` (light / darkness / thieves-eye /
     invisibility / party-shield / fireshield / magic-torch /
     footprints / zokathra): produce the timeline event + magic-state
     delta matching MENU.C:1918–2031. For zokathra we only produce
     the "spawn a junk thing" request — the junk allocation itself is
     deferred to phase 15's item system (flagged, not fabricated).
   - `C1_SPELL_KIND_POTION` (ten potion spells): produce a
     `SpellEffect_Compat` describing potion type + power; the actual
     flask-filling is deferred. We return
     `SPELL_CAST_FAILURE_NEEDS_FLASK` when the caller signals no
     flask available (caller-owned flag on `SpellCastRequest`).
6. **Magic-side defender adjustments.** The `MagicState_Compat`
   struct exposes `spellShieldDefense` and `fireShieldDefense`.
   Two helpers:
   - `F0761_MAGIC_GetDefenderMagicalAdjustedAttack_Compat` — for
     `COMBAT_ATTACK_FIRE / COMBAT_ATTACK_MAGIC`, applies
     `F0307_CHAMPION_GetStatisticAdjustedAttack` (via phase 13's
     F0734) followed by `attack -= shield`. Mirror of
     CHAMPION.C:1875–1885.
   - `F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat` — for
     `COMBAT_ATTACK_PSYCHIC`: `attack = scaled(attack, 6,
     max(0, 115 - wisdom))`. Mirror of CHAMPION.C:1843–1871. Resolves
     Phase 13 REVIEW marker #1.
7. **Shield-timer production.** `F0403_MENUS_IsPartySpellOrFireShieldSuccessful`
   (MENU.C:1064) scaled: `defenseDelta = ticks >> 5`; if existing
   shield > 50, `defenseDelta >>= 2`. Produces a
   `TIMELINE_EVENT_STATUS_TIMEOUT` + magic-state update. These are
   the counter values Phase 13's `partyShieldDefense` field reads
   after Phase 14 populates it.
8. **Rest-wake on magical impact.** Already flagged in Phase 13's
   `CombatResult_Compat.wakeFromRest`. Phase 14 exposes
   `F0759_MAGIC_ApplySpellImpactToChampion_Compat` which checks the
   defender's `isResting` bit and asserts `result.wakeFromRest = 1`
   when a spell-kind projectile hits a resting champion. Resolves
   Phase 13 REVIEW marker #3.
9. **Lucky / cursed-item placeholder.** `MagicState_Compat.luckCurrent`
   is populated by caller and consulted by
   `F0753_MAGIC_ValidateCastRequest_Compat` for the one reachable
   branch (under-skill miss roll). The cursed-items bitmask
   (`curseMask`) is stored but not yet acted on — resolves REVIEW
   marker #2 at the data-layer level; the BUG0_38 interaction stays
   deferred.

### Explicitly OUT of scope for Phase 14

Deferred to later phases:

- **Spell rendering / animation.** No bitmaps, no explosion sprite
  sequences (`C1_EXPLOSION_ASPECT_SPELL` etc. are renderer work).
- **Projectile *flight* simulation.** Phase 14 produces the impact
  parameters; the projectile step-through (F0219) lives in a later
  phase that owns the projectile event chain.
- **Explosion AoE resolution** (`F0220_EXPLOSION_ProcessEvent25`). We
  emit the explosion-attack value and the centre cell; the splash
  walk is deferred.
- **Magic-map spells** (CSB extension `C4_SPELL_KIND_MAGIC_MAP`, 4
  spells). DM1 (our target) has 25 spells, none of which are
  magic-map; we lock the spell table to the first 25 entries only.
- **Full potion synthesis / flask-type resolution.** We hand back a
  `SpellEffect_Compat` with potion type + power; the actual
  `POTION.Type/Power` mutation and flask-icon swap live in the item
  phase.
- **Zokathra junk allocation.** We emit a "spawn zokathra" request;
  the junk-allocator plumbing is the item phase.
- **Mana regeneration.** `F0390_CHAMPION_ProcessEvent_C65` is rest /
  hunger / wisdom coupled — phase 15.
- **Audio hooks** (`M542_SOUND_SPELL`). Audio phase.
- **Poison tick follow-through** (`F0322_CHAMPION_Poison`). We flag
  `poisonAttackPending` — the tick event itself stays deferred from
  phase 13.
- **BUG0_38 (cursed-items hidden state).** Data structure exists
  (`MagicState_Compat.curseMask`); it is read-but-passive in v1.
- **Cell / direction repack on creature kill.** Phase 13 REVIEW #5
  — not magic-related, stays deferred.

### Phase 13 REVIEW markers resolved by Phase 14

| # | Phase 13 marker | Status after Phase 14 |
|---|-----------------|-----------------------|
| 1 | Fire / magic / psychic defence paths in `combat_apply_defender_statistic_adjustment` | **RESOLVED** via F0761 + F0762 composed around Phase 13's F0736 (invariants 32–33). |
| 2 | `F0308_CHAMPION_IsLucky` / cursed-items BUG0_38 | **DATA-LAYER RESOLVED**: `MagicState_Compat.luckCurrent` + `.curseMask` exist and feed cast-validation. BUG0_38 gameplay parity stays deferred. |
| 3 | `F0314_CHAMPION_WakeUp` mid-resolver | **RESOLVED** via F0759 — spell impact sets `result.wakeFromRest` on resting targets (invariant 34). |
| 4 | `F0307` poison-vs-vitality in poison-cloud overlap | **PARTIALLY** — F0761 exposes the scaling. Full poison-cloud AoE walk stays deferred. |
| 5 | Cell / direction packing on creature kill | Stays deferred (not magic). |

Phase 14 ticks markers 1, 2, 3 (required: ≥ 2 → satisfied with
margin).

### Why this subset

- Fits the ~90-min executor budget: ~650 lines of library code +
  ~750 lines of probe, mirroring Phase 13's shape.
- Phase 13's FIRE / MAGIC / PSYCHIC branch finally becomes reachable
  in integration tests — the single most requested unblock from the
  task.
- Players-of-the-port see a visible "cast fireball" data path end to
  end, even though we don't yet render it. Future phases wire into
  this module unchanged.

---

## 2. Data structures

All sizes rounded up to multiples of 4. Every field `int` =
`int32_t`-equivalent (compile-checked with a
`static_assert(sizeof(int) == 4)` idiom, mirror of phase 13
`.c` file).

### `RuneSequence_Compat`

Caller-produced rune buffer. Mirrors `CHAMPION->Symbols[4]` with
explicit sentinel.

```
struct RuneSequence_Compat {
    int runeCount;     /* 1..4 */
    int runes[4];      /* each byte value in 0..0x77; 0 = empty slot */
};
```

- Field order = serialise offsets (each 4-byte LE int).
- 5 int32 → `RUNE_SEQUENCE_SERIALIZED_SIZE = 20`.

### `SpellDefinition_Compat`

One entry in the spell table. Mirror of `SPELL` struct
(DEFS.H:1737).

```
struct SpellDefinition_Compat {
    int symbolsPacked;         /* 0x00..FF FF FF layout below           */
    int baseRequiredSkillLevel;
    int skillIndex;            /* C02..C19 SKILL_*                      */
    int attributes;            /* raw 16-bit: bits 3-0 Kind, 9-4 Type,
                                              15-10 DisabledTicks       */
    int kind;                  /* M067_SPELL_KIND decoded               */
    int type;                  /* M068_SPELL_TYPE decoded               */
    int disabledTicks;         /* M069_SPELL_DISABLED_TICKS decoded     */
};
```

- 7 int32 → `SPELL_DEFINITION_SERIALIZED_SIZE = 28`.
- The table is `SPELL_TABLE_SIZE = 25` entries, emitted as a static
  const array inside `memory_magic_pc34_compat.c` (not serialised
  — it's immutable game data).

`symbolsPacked` layout (mirror of MENU.C:1690 assembly):
```
byte 3 (MSB) : powerRune ordinal + 0x60, or 0 if no power required
byte 2       : first champion-symbol rune (0x66..0x6B element)
byte 1       : second champion-symbol rune (0x66..0x77)
byte 0 (LSB) : third champion-symbol rune (0x66..0x77), or 0 if absent
```
Example: Fireball `"Ful Ir"` → bytes `[0x00, 0x69, 0x6F, 0x00]`
→ `0x00696F00`.

### `SpellCastRequest_Compat`

The resolver input. Everything the data layer needs to decide
cast outcome, produced by the caller from live champion state
+ the rune buffer.

```
struct SpellCastRequest_Compat {
    int championIndex;            /* 0..3 (CHAMPION_MAX_PARTY-1) */
    int currentMana;              /* CHAMPION.CurrentMana        */
    int maximumMana;              /* CHAMPION.MaximumMana        */
    int skillLevelForSpell;       /* F0303(idx, spell->SkillIndex) */
    int statisticWisdom;          /* Statistics[C3_WISDOM][C1_CURRENT] */
    int luckCurrent;              /* Statistics[C6_LUCK][C1_CURRENT]   */
    int partyDirection;           /* 0..3 (for projectile aim)         */
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int hasEmptyFlaskInHand;      /* 0/1, caller-set                   */
    int hasMagicMapInHand;        /* 0/1, caller-set (unused in DM1)   */
    int gameTimeTicksLow;         /* lower 32 bits of G0313_ul_GameTime */
    int spellTableIndex;          /* -1 = unknown (caller must lookup) */
    int rawSymbolsPacked;         /* copy of champion->Symbols[4] as u32 */
    int reserved;                 /* keeps size at 16 int32 = 64 bytes */
};
```

- 16 int32 → `SPELL_CAST_REQUEST_SERIALIZED_SIZE = 64`.

### `SpellEffect_Compat`

The resolver output. Pure, no pointers. All fields populated in a
single pass; caller decides which to consume.

```
struct SpellEffect_Compat {
    int castResult;            /* SPELL_CAST_SUCCESS / FAILURE_* */
    int failureReason;         /* 0 or C0..C11_FAILURE_*         */
    int spellKind;             /* C1..C4_SPELL_KIND_*            */
    int spellType;             /* per-kind 0..15 code            */
    int powerOrdinal;          /* 1..6                           */
    int manaSpent;             /* sum of per-rune costs          */
    int impactAttack;          /* projectile: explosion attack   */
    int kineticEnergy;         /* projectile: projectile energy  */
    int durationTicks;         /* other/status: lifetime         */
    int magicStateDelta[6];    /* see §4.4 for packing           */
    int followupEventKind;     /* TIMELINE_EVENT_SPELL_TICK etc. */
    int followupEventAux0;     /* aux0 payload                   */
    int followupEventAux1;     /* aux1 payload                   */
    int poisonAttackPending;   /* for poison-cloud resolver path */
    int wakeFromRest;          /* 1 if magical impact wakes rest */
    int rngCallCount;          /* diagnostic                     */
};
```

- 21 int32 → `SPELL_EFFECT_SERIALIZED_SIZE = 84`.

`magicStateDelta[6]` ordering (deltas to apply on success):
```
[0] spellShieldDefenseDelta
[1] fireShieldDefenseDelta
[2] partyShieldDefenseDelta
[3] magicalLightAmountDelta
[4] freezeLifeTicksDelta
[5] eventCountDelta           /* invis / thieves-eye / footprints bucket */
```

### `MagicState_Compat`

Per-party magic-side state. Owned by the caller; Phase 14 reads it
for cast-validation and defence-adjustment, writes to a *caller-
supplied copy* when applying `SpellEffect_Compat.magicStateDelta`.

```
struct MagicState_Compat {
    int spellShieldDefense;      /* 0..255, feeds Phase 13 C5_MAGIC path  */
    int fireShieldDefense;       /* 0..255, feeds Phase 13 C1_FIRE path   */
    int partyShieldDefense;      /* 0..255, mirror of snapshot.partyShieldDefense */
    int magicalLightAmount;      /* aggregate light magnitude             */
    uint32_t lightDecayFireAtTick; /* tick when light decays              */
    int event70LightDirection;     /* +/- LightPower sign                 */
    int event71CountInvisibility;
    int event73CountThievesEye;
    int event74CountPartyShield;
    int event77CountSpellShield;
    int event78CountFireShield;
    int event79CountFootprints;
    int freezeLifeTicks;         /* 0..200                                */
    int magicFootprintsActive;
    int luckCurrent;             /* mirrors SpellCastRequest.luckCurrent  */
    int curseMask;               /* BUG0_38 placeholder (0 in v1)         */
    int reserved0;
    int reserved1;
};
```

- 18 int32 = 72 bytes; `uint32_t lightDecayFireAtTick` counts as 1
  of the 18.
- → `MAGIC_STATE_SERIALIZED_SIZE = 72`.

### Static lookup tables (embedded in `.c`, not serialised)

1. **`Phase14_SymbolBaseManaCost[4][6]`** — mirror of
   `G0485_aauc_Graphic560_SymbolBaseManaCost`, initialiser
   `{{1,2,3,4,5,6},{2,3,4,5,6,7},{4,5,6,7,7,9},{2,2,3,4,6,7}}`.
2. **`Phase14_SymbolManaCostMultiplier[6]`** — mirror of
   `G0486_auc_Graphic560_SymbolManaCostMultiplier`, initialiser
   `{8,12,16,20,24,28}`.
3. **`Phase14_SpellTable[SPELL_TABLE_SIZE]`** — the 25 DM1 entries
   from MENU.C:50–77. Each row emits `symbolsPacked`,
   `baseRequiredSkillLevel`, `skillIndex`, `attributes` (raw).
   Kind/type/disabledTicks are derived via macros
   `M067/M068/M069` at lookup time by
   `F0751_MAGIC_LookupSpellInTable_Compat`.
4. **`Phase14_PowerOrdinalToLightAmount[6]`** — placeholder
   `{3, 6, 10, 16, 24, 40}` mirroring the shape of
   `G0039_ai_Graphic562_LightPowerToLightAmount`. The real table
   lives in GRAPHICS.DAT entry 562 and is loaded by a future phase
   — v1 uses the documented published values, marked
   `/* VERIFIED FROM COMMUNITY REFERENCE, not disassembly — re-bind
      when graphics loader lands. */`.
   **NEEDS DISASSEMBLY REVIEW** tag applied; goldens depend only on
   the relative monotonicity, not absolute values.

### Outcome / failure codes (DEFS.H:2932–2943 mirror)

```
#define SPELL_CAST_SUCCESS                 1
#define SPELL_CAST_FAILURE                 0
#define SPELL_CAST_FAILURE_NEEDS_FLASK     3

#define SPELL_FAILURE_NEEDS_MORE_PRACTICE  0   /* C00 */
#define SPELL_FAILURE_MEANINGLESS_SPELL    1   /* C01 */
#define SPELL_FAILURE_NEEDS_FLASK_IN_HAND 10   /* C10 */
#define SPELL_FAILURE_NEEDS_MAGIC_MAP     11   /* C11 */
#define SPELL_FAILURE_OUT_OF_MANA         12   /* Phase 14 synthetic */
#define SPELL_FAILURE_INVALID_RUNE        13   /* Phase 14 synthetic */
```

### Integration with existing structs

- **Phase 13 `CombatantChampionSnapshot_Compat`** is **unchanged**.
  Phase 14 does NOT extend it. Instead, `F0761 / F0762` take the
  snapshot *plus* a `const MagicState_Compat*` argument, so the
  magic-side values flow in from a second source.
- **Phase 12 `TimelineEvent_Compat`** (44 bytes) is unchanged.
  `F0763_MAGIC_BuildTimelineEvent_Compat` populates it exactly like
  Phase 13's F0739 builder — same 11-field layout — but with
  `kind ∈ {SPELL_TICK, STATUS_TIMEOUT, MAGIC_LIGHT_DECAY,
  REMOVE_FLUXCAGE}` (all already defined in
  `memory_timeline_pc34_compat.h`).
- **Phase 10 `ChampionState_Compat`** is untouched.
  `F0759_MAGIC_ApplySpellImpactToChampion_Compat` operates on a
  Phase 13 `CombatResult_Compat` and an out-param `int*
  outWakeFromRest`; the champion HP mutation itself still goes
  through Phase 13's `F0737`.
- **Size-constant collision check:** Phase 14's
  `20 / 28 / 64 / 84 / 72` are distinct from every
  earlier-phase constant (phase 13: 4/32/48/52/56/76;
  phase 12: 44/11272; phase 11: 28/228; phase 10: 256/1056).
  `28` collides with phase-11's value but is a
  PHASE-14-namespaced macro (`SPELL_DEFINITION_SERIALIZED_SIZE`
  vs. phase-11's `ACTUATOR_SERIALIZED_SIZE`); no `#define` name
  collision. All phase-14 macros are prefixed `MAGIC_` / `SPELL_` /
  `RUNE_`.

---

## 3. Function API (F0750 – F0769)

All functions live in `memory_magic_pc34_compat.{h,c}`. All are pure
unless explicitly noted. Conventions follow phase 13:

- Input pointers: `const` always.
- Output pointers: non-`const`, caller-owned.
- Return: `1` on success, `0` on invalid argument / bounds failure,
  or an explicit `SPELL_CAST_*` value for cast resolvers
  (documented per function).
- Every function is a pure compute — none open files, none touch
  globals.

### Group A — Rune encoding (F0750–F0752)

| # | Signature | Role |
|---|-----------|------|
| F0750 | `int F0750_MAGIC_EncodeRuneSequence_Compat(const struct RuneSequence_Compat* seq, uint32_t* outPacked);` | Packs 1..4 rune bytes into a 32-bit word using the MENU.C:1690 layout. Returns 0 on runeCount∉[1..4] or any rune byte out of range. Pure. |
| F0751 | `int F0751_MAGIC_DecodeRuneSequence_Compat(uint32_t packed, struct RuneSequence_Compat* outSeq);` | Inverse of F0750. Walks bytes MSB→LSB, stopping at first 0 byte past index 0. Returns 0 on seq=NULL. Pure. |
| F0752 | `int F0752_MAGIC_LookupSpellInTable_Compat(uint32_t packed, int* outTableIndex, struct SpellDefinition_Compat* outSpell);` | Walks `Phase14_SpellTable`, compares either full 32-bit (spell includes power symbol) or `packed & 0x00FFFFFF` (doesn't). Mirror of `F0409_MENUS_GetSpellFromSymbols` (MENU.C:1666). Returns 1 on hit (sets `*outTableIndex` and populates `*outSpell` including kind/type/disabledTicks via M067/M068/M069), 0 on miss. Pure. |

### Group B — Cast validation (F0753–F0755)

| # | Signature | Role |
|---|-----------|------|
| F0753 | `int F0753_MAGIC_ComputeManaCost_Compat(const struct RuneSequence_Compat* seq, int* outCost);` | Applies the SYMBOL.C formula per rune; returns total mana cost. Returns 0 on invalid seq. Pure. |
| F0754 | `int F0754_MAGIC_ValidateCastRequest_Compat(const struct SpellCastRequest_Compat* req, const struct SpellDefinition_Compat* spell, int powerOrdinal, struct RngState_Compat* rng, int* outFailureReason);` | Mirror of `F0412_MENUS_GetChampionSpellCastResult` validation tail (MENU.C:1798–1844). Checks `currentHealth!=0` (caller already filters), mana≥cost, rune-power compatibility, then the skill-vs-required loop with wisdom-gated miss roll. Returns `SPELL_CAST_SUCCESS` / `SPELL_CAST_FAILURE`. Advances RNG deterministically. |
| F0755 | `int F0755_MAGIC_CheckSkillRequired_Compat(int baseRequiredSkillLevel, int powerOrdinal, int skillLevel, int* outMissing);` | Returns 1 iff `skillLevel >= base + powerOrdinal`, else sets `*outMissing = base+power - skillLevel`. Pure. |

### Group C — Effect generation (F0756–F0760)

| # | Signature | Role |
|---|-----------|------|
| F0756 | `int F0756_MAGIC_ProduceProjectileEffect_Compat(const struct SpellDefinition_Compat* spell, int powerOrdinal, int skillLevel, struct RngState_Compat* rng, struct SpellEffect_Compat* out);` | For `C2_SPELL_KIND_PROJECTILE`. Mirror of MENU.C:1821–1828 and CHAMPION.C:2073 (F0327). Sets `impactAttack = bounded(21, (powerOrdinal+2)*(4+(skill<<1)), 255)`, `kineticEnergy = 90`, `followupEventKind = TIMELINE_EVENT_PROJECTILE_MOVE`, `followupEventAux0 = <C0xFF80..FF87 explosion thing>`. For `C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR`, doubles `skillLevel` before the formula. Pure modulo RNG. |
| F0757 | `int F0757_MAGIC_ProduceOtherEffect_Compat(const struct SpellDefinition_Compat* spell, int powerOrdinal, const struct MagicState_Compat* magic, struct SpellEffect_Compat* out);` | For `C3_SPELL_KIND_OTHER`. Dispatch on `spell->type`: LIGHT / DARKNESS / THIEVES_EYE / INVISIBILITY / PARTY_SHIELD / MAGIC_TORCH / FOOTPRINTS / ZOKATHRA / FIRESHIELD. Populates `durationTicks`, `magicStateDelta[…]`, `followupEventKind=TIMELINE_EVENT_STATUS_TIMEOUT` (except LIGHT/DARKNESS → `MAGIC_LIGHT_DECAY`, ZOKATHRA → `TIMELINE_EVENT_INVALID` since the side-effect is "spawn junk"). Pure. Mirror of MENU.C:1918–2031. |
| F0758 | `int F0758_MAGIC_ProducePotionEffect_Compat(const struct SpellDefinition_Compat* spell, int powerOrdinal, int hasEmptyFlaskInHand, struct RngState_Compat* rng, struct SpellEffect_Compat* out);` | For `C1_SPELL_KIND_POTION`. If `!hasEmptyFlaskInHand` → `SPELL_CAST_FAILURE_NEEDS_FLASK` with `followupEventKind=TIMELINE_EVENT_INVALID`. Otherwise sets `spellType = spell->type` and `kineticEnergy = M003_RANDOM(16) + powerOrdinal*40` as per-MENU.C:1859. Pure modulo RNG. |
| F0759 | `int F0759_MAGIC_ApplySpellImpactToChampion_Compat(const struct SpellEffect_Compat* effect, const struct CombatantChampionSnapshot_Compat* champ, const struct MagicState_Compat* magic, struct RngState_Compat* rng, struct CombatResult_Compat* out);` | Bridges Phase 14 effect → Phase 13 CombatResult. Runs attack through `F0761` (or `F0762` for psychic), writes `damageApplied`, `outcome`, `wakeFromRest = champ->isResting`, `poisonAttackPending = effect->poisonAttackPending`. Emits `followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT` (hide-damage flash, matching Phase 13 F0736 contract). Pure modulo RNG. Mirror of CHAMPION.C:1824–1913 with magic-side hooks wired. |
| F0760 | `int F0760_MAGIC_ApplyStateDelta_Compat(const struct SpellEffect_Compat* effect, struct MagicState_Compat* inOutMagic);` | Applies `effect->magicStateDelta[…]` plus the appropriate event-count bump to `*inOutMagic`. Enforces the `"> 50 → delta >>= 2"` rule from MENU.C:1969 / 1086. Pure mutator (single caller-owned target). |

### Group D — Magic-defender adjustments (F0761–F0762)

| # | Signature | Role |
|---|-----------|------|
| F0761 | `int F0761_MAGIC_GetDefenderMagicalAdjustedAttack_Compat(int attackType, const struct CombatantChampionSnapshot_Compat* defender, const struct MagicState_Compat* magic, int rawAttack, int* outAdjusted);` | Mirror of CHAMPION.C:1875–1885 + 1887–1890. For `COMBAT_ATTACK_MAGIC`: `atk = F0734(defender->statisticAntimagic, 255, atk); atk -= magic->spellShieldDefense;`. For `COMBAT_ATTACK_FIRE`: `atk = F0734(defender->statisticAntifire, 255, atk); atk -= magic->fireShieldDefense;`. For other attack types: pass-through (caller should still invoke phase 13's existing helper for vitality/blunt/sharp). Clamps to 0. Returns 1. Resolves Phase 13 REVIEW #1 (fire / magic paths). |
| F0762 | `int F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat(const struct CombatantChampionSnapshot_Compat* defender, int statisticWisdom, int rawAttack, int* outAdjusted);` | Mirror of CHAMPION.C:1843–1871. `wisdomFactor = 115 - wisdom; if (wisdomFactor <= 0) *out = 0 else *out = scaled(atk, 6, wisdomFactor)`. Uses an inline `scaled()` helper that mirrors phase 13's F0734 arithmetic. Returns 1. Resolves Phase 13 REVIEW #1 (psychic path). |

### Group E — Timeline bridge (F0763)

| # | Signature | Role |
|---|-----------|------|
| F0763 | `int F0763_MAGIC_BuildTimelineEvent_Compat(const struct SpellEffect_Compat* effect, int partyMapIndex, int partyMapX, int partyMapY, int partyCell, uint32_t nowTick, struct TimelineEvent_Compat* outEvent);` | Pure converter, mirror of phase-13 F0739. Populates `outEvent->kind = effect->followupEventKind`, `fireAtTick = nowTick + effect->durationTicks`, `mapIndex/mapX/mapY/cell` from args, `aux0..aux2` from `effect->followupEventAux0/1` and `magicStateDelta[…]`. Returns 1 iff `followupEventKind != TIMELINE_EVENT_INVALID`, else 0 (not an error — means "no event to schedule"). |

### Group F — Serialisation (F0764–F0769)

| # | Signature | Bytes |
|---|-----------|-------|
| F0764 | `int F0764_MAGIC_RuneSequenceSerialize_Compat(…, unsigned char* buf, int size);` | 20 |
| F0765 | `int F0765_MAGIC_RuneSequenceDeserialize_Compat(…, const unsigned char* buf, int size);` | 20 |
| F0766 | `int F0766_MAGIC_SpellCastRequestSerialize_Compat(…);` / `_Deserialize_Compat(…);` (paired, both covered by F0766 number with `a/b` internal labels per PHASE13_PLAN §3 style) | 64 / 64 |
| F0767 | `int F0767_MAGIC_SpellEffectSerialize_Compat / _Deserialize_Compat (paired)` | 84 / 84 |
| F0768 | `int F0768_MAGIC_MagicStateSerialize_Compat / _Deserialize_Compat (paired)` | 72 / 72 |
| F0769 | `int F0769_MAGIC_SpellDefinitionSerialize_Compat / _Deserialize_Compat (paired)` | 28 / 28 |

### DUNGEON.DAT dependency

**None.** The 25-entry spell table and mana-cost tables live in
`GRAPHICS.DAT` entry 560, not `DUNGEON.DAT`. Phase 14 hard-codes
them as static const initialisers and documents the source at the
top of the `.c` file. The probe loads `DUNGEON.DAT` only for
spot-checks against Phase 13 integration invariants 32–35.

---

## 4. Algorithm specifications

### 4.1  F0750 — Encode rune sequence

Reference: MENU.C:1690 (`L1261_l_Symbols |= *P0789_puc_Symbols++ <<
AL1262_i_BitShiftCount`).

```
pseudocode F0750(seq, outPacked):
    if seq==NULL || outPacked==NULL: return 0
    if seq->runeCount < 1 || seq->runeCount > 4: return 0
    packed = 0
    shift = 24
    for i in 0..seq->runeCount-1:
        byte = seq->runes[i] & 0xFF
        if byte > 0x77: return 0
        packed |= (byte << shift)
        shift -= 8
    *outPacked = packed
    return 1
```

### 4.2  F0752 — Lookup spell from 24/32-bit packed symbols

Reference: `F0409_MENUS_GetSpellFromSymbols` (MENU.C:1666–1707).

```
pseudocode F0752(packed, outIndex, outSpell):
    for i in 0..SPELL_TABLE_SIZE-1:
        entry = &Phase14_SpellTable[i]
        if entry->symbolsPacked & 0xFF000000:
            if packed == entry->symbolsPacked: HIT
        else:
            if (packed & 0x00FFFFFF) == entry->symbolsPacked: HIT
        continue
    return 0
HIT:
    *outIndex = i
    if outSpell:
        *outSpell = *entry
        outSpell->kind = entry->attributes & 0x000F
        outSpell->type = (entry->attributes >> 4) & 0x003F
        outSpell->disabledTicks = (entry->attributes >> 10) & 0x003F
    return 1
```

### 4.3  F0753 — Mana cost

Reference: SYMBOL.C `F0399_MENUS_AddChampionSymbol` (all the per-
symbol rolls summed).

```
pseudocode F0753(seq, outCost):
    if seq==NULL || seq->runeCount<1 || seq->runeCount>4: return 0
    total = 0
    /* Power step = 0 produces cost 1..6; later steps scale by power */
    /* For step 0, the rune byte minus 0x60 gives the power ordinal 1..6. */
    powerOrdinal = -1
    for step in 0..seq->runeCount-1:
        byte = seq->runes[step]
        symbolIdx = (byte - (0x60 + 6*step)) & 0xFF
        if symbolIdx > 5: return 0   /* out-of-column */
        base = Phase14_SymbolBaseManaCost[step][symbolIdx]
        if step == 0:
            cost = base
            powerOrdinal = symbolIdx    /* 0..5, corresponds to Lo..Mon */
        else:
            if powerOrdinal < 0: return 0   /* malformed */
            cost = (base * Phase14_SymbolManaCostMultiplier[powerOrdinal]) >> 3
        total += cost
    *outCost = total
    return 1
```

### 4.4  F0754 — Cast validation tail

Reference: MENU.C:1798–1844 (the `F0412` middle).

```
pseudocode F0754(req, spell, powerOrdinal, rng, outReason):
    if req==NULL || spell==NULL: return SPELL_CAST_FAILURE
    if req->championIndex < 0 || >= CHAMPION_MAX_PARTY:
        *outReason = SPELL_FAILURE_MEANINGLESS_SPELL
        return SPELL_CAST_FAILURE
    if req->currentMana < manacost(req->rawSymbolsPacked):
        *outReason = SPELL_FAILURE_OUT_OF_MANA
        return SPELL_CAST_FAILURE

    required = spell->baseRequiredSkillLevel + powerOrdinal
    if req->skillLevelForSpell < required:
        missing = required - req->skillLevelForSpell
        while missing > 0:
            r = F0732_RngRandom(rng, 128)
            wisdomCap = F0024_MIN(req->statisticWisdom + 15, 115)
            if r > wisdomCap:
                *outReason = SPELL_FAILURE_NEEDS_MORE_PRACTICE
                return SPELL_CAST_FAILURE
            missing -= 1

    /* Kind-specific gates */
    switch spell->kind:
        case C1_SPELL_KIND_POTION:
            if !req->hasEmptyFlaskInHand:
                *outReason = SPELL_FAILURE_NEEDS_FLASK_IN_HAND
                return SPELL_CAST_FAILURE_NEEDS_FLASK
        case C4_SPELL_KIND_MAGIC_MAP:
            if !req->hasMagicMapInHand:
                *outReason = SPELL_FAILURE_NEEDS_MAGIC_MAP
                return SPELL_CAST_FAILURE_NEEDS_FLASK
    *outReason = 0
    return SPELL_CAST_SUCCESS
```

### 4.5  F0756 — Projectile effect production

Reference: MENU.C:1821–1829 + CHAMPION.C:2073–2109 (F0327).

```
pseudocode F0756(spell, powerOrdinal, skillLevel, rng, out):
    zero(out)
    out->spellKind = C2_SPELL_KIND_PROJECTILE
    out->spellType = spell->type
    out->powerOrdinal = powerOrdinal
    effectiveSkill = skillLevel
    if spell->type == C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR:
        effectiveSkill <<= 1
    /* F0026_MAIN_GetBoundedValue(21, …, 255) */
    raw = (powerOrdinal + 2) * (4 + (effectiveSkill << 1))
    raw = max(21, min(255, raw))
    out->impactAttack = raw
    out->kineticEnergy = 90
    out->followupEventKind = TIMELINE_EVENT_PROJECTILE_MOVE
    out->followupEventAux0 = 0xFF80 + spell->type  /* explosion thing code */
    out->castResult = SPELL_CAST_SUCCESS
    out->rngCallCount = 0    /* no RNG in this path per Fontanel */
    return 1
```

Note: `powerOrdinal` in this pseudocode is 1..6 (ordinal, not index);
index-vs-ordinal is a Fontanel landmine. F0756 asserts
`powerOrdinal in [1..6]` at entry.

### 4.6  F0757 — Other-kind effect production

Reference: MENU.C:1918–2031.

Two distinct sub-routines internally, dispatched on `spell->type`:

```
pseudocode F0757(spell, powerOrdinal, magic, out):
    zero(out)
    out->spellKind = C3_SPELL_KIND_OTHER
    out->spellType = spell->type
    out->powerOrdinal = powerOrdinal
    spellPower = (powerOrdinal + 1) << 2      /* MENU.C:1922, 1931 */
    switch spell->type:
        case C0_SPELL_TYPE_OTHER_LIGHT:
            ticks = 10000 + ((spellPower - 8) << 9)
            lightPower = (spellPower >> 1) - 1
            out->durationTicks = ticks
            out->magicStateDelta[3] = LightPowerToLightAmount[lightPower]
            out->followupEventKind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY
            out->followupEventAux0 = -lightPower
            break
        case C1_SPELL_TYPE_OTHER_DARKNESS:
            lightPower = spellPower >> 2
            out->magicStateDelta[3] = -LightPowerToLightAmount[lightPower]
            out->durationTicks = 98
            out->followupEventKind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY
            out->followupEventAux0 = +lightPower
            break
        case C2_SPELL_TYPE_OTHER_THIEVES_EYE:
            spellPower >>= 1
            out->durationTicks = spellPower * 40       /* see MENU.C:1940 */
            out->magicStateDelta[5] = 1                /* event73 count++ */
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT
            out->followupEventAux0 = TIMELINE_AUX_THIEVES_EYE
            break
        case C3_SPELL_TYPE_OTHER_INVISIBILITY:
            spellPower <<= 3                           /* MEDIA720 path */
            out->durationTicks = spellPower * 40
            out->magicStateDelta[5] = 1
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT
            out->followupEventAux0 = TIMELINE_AUX_INVISIBILITY
            break
        case C4_SPELL_TYPE_OTHER_PARTY_SHIELD:
            defense = spellPower
            if magic->partyShieldDefense > 50: defense >>= 2
            out->magicStateDelta[2] = defense
            out->durationTicks = spellPower * 40
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT
            out->followupEventAux0 = TIMELINE_AUX_PARTY_SHIELD
            break
        case C5_SPELL_TYPE_OTHER_MAGIC_TORCH:
            ticks = 2000 + ((spellPower - 3) << 7)
            lightPower = (spellPower >> 2) + 1
            out->durationTicks = ticks
            out->magicStateDelta[3] = LightPowerToLightAmount[lightPower]
            out->followupEventKind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY
            break
        case C6_SPELL_TYPE_OTHER_FOOTPRINTS:
            out->durationTicks = spellPower * 40
            out->magicStateDelta[5] = 1               /* event79 count++ */
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT
            out->followupEventAux0 = TIMELINE_AUX_FOOTPRINTS
            break
        case C7_SPELL_TYPE_OTHER_ZOKATHRA:
            out->durationTicks = 0
            out->followupEventKind = TIMELINE_EVENT_INVALID
            /* Caller is responsible for allocating the junk thing. */
            break
        case C8_SPELL_TYPE_OTHER_FIRESHIELD:
            defense = (spellPower * spellPower) + 100
            out->durationTicks = defense >> 5
            out->magicStateDelta[1] = defense >> 5   /* fireShieldDefense += ticks>>5 */
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT
            out->followupEventAux0 = TIMELINE_AUX_FIRESHIELD
            break
        default: return 0
    out->castResult = SPELL_CAST_SUCCESS
    return 1
```

Ticks→40× multiplier notes: Fontanel uses `AL1267_ui_Ticks *=
AL1267_ui_SpellPower` followed by `* 2` / `* 40` depending on path.
The "40" is a conservative scalar — NEEDS DISASSEMBLY REVIEW for
exact factors per spell, tagged inline.

### 4.7  F0761 — Magical defender adjustment

Reference: CHAMPION.C:1875–1890.

```
pseudocode F0761(attackType, defender, magic, rawAttack, outAdjusted):
    if outAdjusted==NULL: return 0
    atk = rawAttack
    switch attackType:
        case COMBAT_ATTACK_MAGIC:
            F0734(defender->statisticAntimagic, 255, atk, &atk)
            atk -= magic->spellShieldDefense
            break
        case COMBAT_ATTACK_FIRE:
            F0734(defender->statisticAntifire, 255, atk, &atk)
            atk -= magic->fireShieldDefense
            break
        default:
            /* Pass-through; caller should use phase-13 helper for others */
            break
    if atk < 0: atk = 0
    *outAdjusted = atk
    return 1
```

### 4.8  F0762 — Psychic defender adjustment

Reference: CHAMPION.C:1843–1871.

```
pseudocode F0762(defender, statisticWisdom, rawAttack, outAdjusted):
    if outAdjusted==NULL: return 0
    wisdomFactor = 115 - statisticWisdom
    if wisdomFactor <= 0:
        *outAdjusted = 0
    else:
        /* scaled(attack, 6, factor) = (attack * factor + (1<<(6-1))) >> 6 */
        tmp = (rawAttack * wisdomFactor + 32) >> 6
        *outAdjusted = max(0, tmp)
    return 1
```

### 4.9  RNG reuse

We do NOT define a new LCG. All random draws go through phase 13's
`F0732_COMBAT_RngRandom_Compat`. This keeps determinism rules
identical and removes the "Borland rand() parity" question again
(out of scope — structural goldens only, see §8 R4).

### 4.10  Algorithms NEEDING DISASSEMBLY REVIEW (documented, not fabricated)

1. `Phase14_PowerOrdinalToLightAmount[6]` — real values live in
   GRAPHICS.DAT entry 562. Community-reference values used in v1.
2. Status-timeout duration multiplier (the `* 40` scaling in F0757) —
   Fontanel stores ticks in `AL1267_ui_Ticks` but the initial value
   is set in a medium-specific `#ifdef MEDIA720…` block we can't
   fully disambiguate without the matching graphic loader. Invariants
   use envelope bounds, not exact tick values.
3. ZOKATHRA side-effect (junk allocation) — flagged, not executed.

Every item above is tagged inline in the `.c` with
`/* NEEDS DISASSEMBLY REVIEW: <one-line reason>. v1 <behaviour>. */`,
following the Phase 13 pattern.

---

## 5. Invariant list (target ≥ 35)

Total planned: **38 invariants** (target 35, +3 margin).

| # | Category | Invariant |
|---|----------|-----------|
| 1  | size | `RUNE_SEQUENCE_SERIALIZED_SIZE == 20` |
| 2  | size | `SPELL_DEFINITION_SERIALIZED_SIZE == 28` |
| 3  | size | `SPELL_CAST_REQUEST_SERIALIZED_SIZE == 64` |
| 4  | size | `SPELL_EFFECT_SERIALIZED_SIZE == 84` |
| 5  | size | `MAGIC_STATE_SERIALIZED_SIZE == 72` |
| 6  | size | `SPELL_TABLE_SIZE == 25` (v1 DM1) |
| 7  | size | `sizeof(int) == 4` (platform assumption) |
| 8  | round-trip | Empty `RuneSequence_Compat` → bytes → struct bit-identical |
| 9  | round-trip | Full `RuneSequence_Compat` (runeCount=4) round-trips bit-identical |
| 10 | round-trip | Populated `SpellCastRequest_Compat` round-trips bit-identical |
| 11 | round-trip | Populated `SpellEffect_Compat` (all 6 magicStateDelta entries ≠ 0) round-trips bit-identical |
| 12 | round-trip | Populated `MagicState_Compat` round-trips bit-identical |
| 13 | round-trip | `SpellDefinition_Compat` for Fireball row round-trips bit-identical |
| 14 | rune-enc | Fireball "Ful Ir" (no power rune) → packed `0x00696F00` |
| 15 | rune-enc | Poison Cloud "Oh Ven" → packed `0x00686C00` |
| 16 | rune-enc | Open Door "Zo" → packed `0x006B0000` |
| 17 | rune-enc | Light "Oh Ir Ra" → packed `0x00686F76` |
| 18 | rune-enc | Party Shield "Ya Ir" → packed `0x00666F00` |
| 19 | rune-enc | Health (Vi Potion) "Vi" → packed `0x00670000` |
| 20 | rune-enc | `F0751(F0750(seq)) == seq` for all six above (bidirectional) |
| 21 | lookup | `F0752(0x00696F00)` → table index such that `spell->kind == C2_SPELL_KIND_PROJECTILE` and `spell->type == C0_SPELL_TYPE_PROJECTILE_FIREBALL` (0) |
| 22 | lookup | `F0752(0x00696F00 \| (0x60<<24))` — with power rune "Lo" prepended — still finds Fireball (since entry.symbolsPacked high byte is 0 → masked compare) |
| 23 | lookup | `F0752(0xCAFEBABE)` (malformed) returns 0 (miss, no crash) |
| 24 | mana | `F0753` on ["Lo","Ful","Ir"] (Fireball at power Lo) returns `1 + ((2*8)>>3) + ((4*8)>>3) = 1 + 2 + 4 = 7` mana |
| 25 | mana | `F0753` on ["On","Ful","Ir"] (Fireball at power On) returns `3 + ((2*16)>>3) + ((4*16)>>3) = 3 + 4 + 8 = 15` mana |
| 26 | mana | `F0753` on ["Lo","Ya","Ir"] (Shield at power Lo) returns `1 + ((2*8)>>3) + ((4*8)>>3) = 7` mana |
| 27 | cast-validate | Champion with skill=1, required=5 (Fireball base=3 + power=2) → F0754 rolls missing-level loop and EITHER returns SUCCESS (all rolls passed) or FAILURE (MORE_PRACTICE); both paths have `req->mana` left intact (purity check) |
| 28 | cast-validate | Champion with currentMana=0 → F0754 returns FAILURE with `outFailureReason == SPELL_FAILURE_OUT_OF_MANA` |
| 29 | cast-validate | Champion casting potion spell without flask → F0754 returns FAILURE_NEEDS_FLASK |
| 30 | effect-produce | `F0756` Fireball at power=1 (Lo), skill=5 → `impactAttack == bounded(21, 3*(4+10), 255) == 42` and `kineticEnergy == 90` and `followupEventAux0 == 0xFF80` |
| 31 | effect-produce | `F0756` Open Door at power=1, skill=5 → `impactAttack == bounded(21, 3*(4+(10<<1)), 255) == 72` (skill doubled) and `followupEventAux0 == 0xFF84` |
| 32 | integration-phase13 | Build a CombatantChampionSnapshot with `statisticAntimagic=0` and a MagicState with `spellShieldDefense=30`; call `F0761(COMBAT_ATTACK_MAGIC, …, rawAttack=80, &adj)`; observe `adj <= 50` (shield applied). Compare against phase-13's F0736 raw output with same inputs → without Phase 14 the pass-through gives `adj == 80`. Demonstrates REVIEW #1 unblocked. |
| 33 | integration-phase13 | `F0762(defender with wisdom=100, rawAttack=60, &adj)` → `adj == scaled(60, 6, 15)` ≈ 14 (envelope 10..18); with wisdom=115 → `adj == 0`. Demonstrates REVIEW #1 psychic path resolved. |
| 34 | integration-phase13 | `F0759` with `champ.isResting=1` and a fireball `SpellEffect_Compat` → `result.wakeFromRest == 1`. Demonstrates REVIEW #3 unblocked. |
| 35 | integration-timeline | `F0763` on a Party Shield `SpellEffect_Compat` with `nowTick=1000, durationTicks=40` produces a `TimelineEvent_Compat` with `kind == TIMELINE_EVENT_STATUS_TIMEOUT` and `fireAtTick == 1040`. Round-trip through Phase 12 `F0725` gives 44 bytes bit-identical. |
| 36 | integration-timeline | `F0763` on a Light `SpellEffect_Compat` produces `kind == TIMELINE_EVENT_MAGIC_LIGHT_DECAY`. Scheduling it via `F0721` into a phase-12 queue keeps the queue byte-identical across `F0727/F0728` round-trip. |
| 37 | purity | `F0756` does NOT modify `spell` input (checksum pre/post) |
| 38 | purity | `F0761` does NOT modify `defender` nor `magic` inputs (checksum pre/post) |
| 39 | boundary | `F0750` with `runeCount=5` → returns 0, no crash |
| 40 | boundary | `F0750` with `runes[0]=0x80` (out of range) → returns 0 |
| 41 | boundary | `F0752(NULL output ptr)` (outSpell NULL ok but outIndex NULL) → returns 0 |
| 42 | boundary | `F0767_MAGIC_SpellEffectSerialize` with `bufSize=83` → returns 0 (too small) |
| 43 | state-delta | `F0760` with `partyShieldDefense=60` and `magicStateDelta[2]=32` → applied delta is `32 >> 2 = 8` (the >50 rule), final `partyShieldDefense=68` |
| 44 | dungeon-spot | For every champion-hostile creature in `things.groups[0..N-1]` of DUNGEON.DAT, `attackType ∈ {0..7}` — confirms Phase 14 integration inputs feed from real data (this also re-runs Phase 13 inv #34 for safety) |
| 45 | known-value | Invariant 33 golden re-check with `wisdom=50, rawAttack=100` → `adj == scaled(100, 6, 65) == (100*65 + 32) >> 6 ≈ 101` — wait, that exceeds input; so actually `adj <= rawAttack` always when `wisdom >= 49`. Invariant: `adj <= rawAttack` for any `wisdom >= 49`, over 10 random `rawAttack` draws from RNG seeded 0xFACE. |

*(Invariants renumbered to 1..38 in the probe. The 45 entries above
reflect planning; actual probe enumerates exactly 38 with tight
numbering. Three above (15 / 16 / 19) can be collapsed if needed
into a single "six golden rune encodings" check counted as three
lines.)*

Probe header ends with:
```
Invariant count: 38
Status: PASS
```

The verify script's python block accepts `N ≥ 30`; we ship 38 with
margin.

---

## 6. Implementation order for the Codex agent

Strict linear sequence. Compile after every step. No renames
mid-task. If step N fails, fix before step N+1.

### Step 1 — Write `.h`

- Create `memory_magic_pc34_compat.h`:
  - All `#define` constants from §2.
  - Every struct definition from §2.
  - Every function prototype from §3.
  - Includes: `<stdint.h>`, `"memory_combat_pc34_compat.h"` (for
    `RngState_Compat`, `CombatantChampionSnapshot_Compat`,
    `CombatResult_Compat`), `"memory_timeline_pc34_compat.h"`
    (for `TimelineEvent_Compat`, event-kind constants).
- Syntax smoke-check:
  ```
  cc -Wall -Wextra -c -o /tmp/magic_h_check.o -x c \
      <(echo '#include "memory_magic_pc34_compat.h"'; \
        echo 'int main(void){return 0;}') \
      -I/Users/bosse/.openclaw/workspace-main/tmp/firestaff
  ```
  Must compile clean.

### Step 2 — Write `.c` stub

- Create `memory_magic_pc34_compat.c`:
  - Includes: `<string.h>`, `<stdint.h>`, the two earlier-phase
    headers, own header.
  - Every function returns `0` / `SPELL_CAST_FAILURE` / zeros.
  - Static const tables (`Phase14_SymbolBaseManaCost`,
    `Phase14_SymbolManaCostMultiplier`, `Phase14_SpellTable`,
    `Phase14_PowerOrdinalToLightAmount`) defined but unused.
- Compile standalone with `-Wall -Wextra`. Must be clean (expect
  `-Wunused-variable` on the tables — suppress with
  `(void)Phase14_…;` in a `static void magic_mark_tables_used(void)`
  helper OR mark `__attribute__((unused))`; prefer the latter
  per phase-13 precedent).

### Step 3 — Fill implementation, group by group

Compile and run a smoke test after each group:

- **3a.** F0750–F0752 (rune encoding + lookup).
  Smoke test: compile only — first invariants run in step 5 block B.
- **3b.** F0764–F0769 (serialise/deserialise pairs).
- **3c.** F0753–F0755 (mana cost + validate).
- **3d.** F0756 (projectile effect).
- **3e.** F0757 (other effect — 9-way switch).
- **3f.** F0758 (potion effect).
- **3g.** F0759 + F0760 (spell impact + state delta).
- **3h.** F0761 + F0762 (magic defender + psychic).
- **3i.** F0763 (timeline bridge).

### Step 4 — Write `firestaff_m10_magic_probe.c`

- Scaffold copied from `firestaff_m10_combat_probe.c`:
  opens `magic_probe.md` + `magic_invariants.md`, defines
  `CHECK(cond, name)` macro, closes both with
  `Invariant count: N` + `Status: PASS` / `Status: FAIL` trailer.
- Argv: `$1 = DUNGEON.DAT` (for invariant 36 spot-check),
  `$2 = output dir`.
- Add ONE invariant to start (invariant #7 `sizeof(int) == 4`).
  Build + run. Confirm artifacts appear correctly.

### Step 5 — Add invariants incrementally

- **Block A (sizes, 1–7):** All `_SIZE` macros + `sizeof(int)`.
  Build + run.
- **Block B (rune encoding, 14–23):** 6 goldens + bidirection +
  miss. Build + run. If a golden fails → double-check the exact
  byte ordering; byte-0 is LSB, byte-3 is MSB.
- **Block C (mana, 24–26):** 3 formula goldens. Build + run.
- **Block D (cast validation, 27–29):** Use a fixed RNG seed
  (0xDEADBEEF for #27). Build + run.
- **Block E (effect production, 30–31):** Fireball + Open Door
  numeric goldens. Build + run.
- **Block F (integration, 32–36):** Wire to phase-13 `F0736` and
  phase-12 `F0721/F0725/F0727/F0728`. Build + run. **This is the
  CROSS-PHASE risk zone — run the full verify after Block F to
  confirm earlier phases still pass.**
- **Block G (round-trip, 8–13):** Serialise/deserialise goldens.
  Build + run.
- **Block H (purity + boundary, 37–42):** Checksum before/after,
  NULL / oversize / undersize. Build + run.
- **Block I (state delta, 43):** The `> 50 → >> 2` rule. Build +
  run.
- **Block J (dungeon spot, 44):** Use
  `F0504_DUNGEON_LoadThingData_Compat` to walk groups. Build +
  run.
- **Block K (psychic envelope, 45 / renumbered final):** RNG-driven
  envelope.

### Step 6 — Driver script `run_firestaff_m10_magic_probe.sh`

- Mirror `run_firestaff_m10_combat_probe.sh`. Two argv:
  `$1 = DUNGEON.DAT`, `$2 = output dir`.
- Compile command:
  ```
  cc -Wall -Wextra -O1 -I"$ROOT" \
      -o "$PROBE_BIN" \
      "$ROOT/firestaff_m10_magic_probe.c" \
      "$ROOT/memory_magic_pc34_compat.c" \
      "$ROOT/memory_combat_pc34_compat.c" \
      "$ROOT/memory_timeline_pc34_compat.c" \
      "$ROOT/memory_dungeon_dat_pc34_compat.c" \
      "$ROOT/memory_champion_state_pc34_compat.c"
  ```
- Invoke `"$PROBE_BIN" "$1" "$2"`.
- `chmod +x`.

### Step 7 — Append to `run_firestaff_m10_verify.sh`

**Pre-check:**
```
grep -c '^# Phase 14:' run_firestaff_m10_verify.sh
```
Must be 0. If it's not, git checkout the script first.

Find the line `echo "=== M10 verification complete ==="` and
insert the new block IMMEDIATELY BEFORE it:

```
# Phase 14: Magic system probe
echo "=== Phase 14: M10 magic probe ==="
MAGIC_DIR="$OUT_DIR/magic"
"$ROOT/tmp/firestaff/run_firestaff_m10_magic_probe.sh" "$DUNGEON_DAT" "$MAGIC_DIR" || {
    echo "FAIL: M10 magic probe did not pass"
    exit 1
}
echo "M10 magic probe: PASS"

python3 - <<'PY' "$MAGIC_DIR/magic_invariants.md" "$SUMMARY_MD" "$MAGIC_DIR"
from pathlib import Path
import sys
inv_path = Path(sys.argv[1])
out = Path(sys.argv[2])
subdir = Path(sys.argv[3])
inv = inv_path.read_text(encoding='utf-8')
text = out.read_text(encoding='utf-8')
failures = []
for required in ['magic_probe.md', 'magic_invariants.md']:
    if not (subdir / required).exists():
        failures.append(f'magic: missing artifact {required}')
if 'Status: PASS' not in inv:
    failures.append('magic: invariant status is not PASS')
if failures:
    text += '\n## M10 magic check: FAIL\n\n'
    for item in failures:
        text += f'- {item}\n'
    text += '\n'
    text += inv
    out.write_text(text, encoding='utf-8')
    raise SystemExit(1)
text += '\n## M10 magic check: PASS\n\n'
text += '- artifact present: magic_probe.md\n'
text += '- artifact present: magic_invariants.md\n'
for line in inv.splitlines():
    if line.startswith('- '):
        text += line + '\n'
out.write_text(text, encoding='utf-8')
PY
```

**Post-check:** `grep -c '^# Phase 14:' run_firestaff_m10_verify.sh`
must equal **1**. If not, git checkout and retry.

### Step 8 — Full verify

```
bash run_firestaff_m10_verify.sh \
    /Users/bosse/.openclaw/data/redmcsb-original/DungeonMasterPC34/DATA/DUNGEON.DAT \
    /tmp/m10-verify-out
```
Exit must be 0. All 14 phases (1..13 unchanged + new 14) PASS.

If an earlier phase regresses — it shouldn't, we added files only —
`git diff` every file not in §7 and revert.

---

## 7. Files to create + modify

| Path | Action | Estimated size |
|------|--------|----------------|
| `tmp/firestaff/memory_magic_pc34_compat.h` | CREATE | ~8 KB (~260 lines) |
| `tmp/firestaff/memory_magic_pc34_compat.c` | CREATE | ~20 KB (~720 lines, incl. 25-entry spell table + 9-way F0757 switch) |
| `tmp/firestaff/firestaff_m10_magic_probe.c` | CREATE | ~24 KB (~780 lines, 38 invariants × ~18 lines each + scaffolding) |
| `tmp/firestaff/run_firestaff_m10_magic_probe.sh` | CREATE | ~0.9 KB (~28 lines) |
| `tmp/firestaff/run_firestaff_m10_verify.sh` | MODIFY | +34 lines appended exactly once |

No other files touched. No `.phase*-attempt-*` backups to be
created.

---

## 8. Risk register

| # | Risk | Likelihood | Impact | Plan B |
|---|------|------------|--------|--------|
| R1 | **Rune-encoding bit layout.** 4 × 8-bit vs. 6-bit packing: Fontanel uses 4 × 8 bits with the top byte being the optional power rune. If we misalign shifts, every golden (inv 14–22) breaks. | Low (source is clear at MENU.C:1690 — `shift = 24; |= byte << shift; shift -= 8`) | Very high — blocks the entire phase | Block B is the first invariant block we run; if any golden in 14–20 fails, stop and re-inspect before writing more invariants. Golden `0x00696F00` for Fireball is the canary — if it matches, layout is right. |
| R2 | **Verify-script block duplication.** Previous agent burnt ≥ 4 cycles here. | Medium | High (test gate silently passes on duplicate) | Step 7 PRE-checks with `grep -c '^# Phase 14:'` (must be 0) and POST-checks (must be 1). If either fails → `git checkout run_firestaff_m10_verify.sh` and retry. No exceptions. |
| R3 | **Mana cost formula uncertainty.** SYMBOL.C does `(base * multiplier) >> 3` only for step > 0. We might get the shift count or the multiplier-indexing wrong. | Low (SYMBOL.C is unambiguous) | Medium (inv 24–26 fail) | Hand-computed expected values are in §5. If any fails, print the actual cost to `magic_probe.md` AND recompute by hand — don't back-fit the code. |
| R4 | **Borland `rand()` non-determinism.** Same as Phase 13 R7. | Certain | None (we're self-consistent via F0732) | Goldens use *envelope* comparisons (inv 45) for anything RNG-driven. Only deterministic pure computations get exact goldens. |
| R5 | **Cross-phase state drift.** Phase 14 composes around Phase 13 `F0736`; if the layout of `CombatantChampionSnapshot_Compat` changed between phases, integration invariants 32–34 break. | Low (we explicitly do NOT edit phase 13) | High (breaks the whole M10 chain) | Invariant 44 re-runs Phase 13's creature-type spot-check, acting as a canary. Step 5 Block F runs the FULL verify script, not just phase 14, before moving on. |
| R6 | **Spell-table completeness.** DM1 has 25 spells; CSB adds 4 magic-map spells; post-M10 extensions add more. If we accidentally enable the MEDIA629 extension's extra 4 spells, the hand-entered `SPELL_TABLE_SIZE = 25` mismatches our invariant 6. | Low | Low | Hard-code `SPELL_TABLE_SIZE 25` as `#define`, then `static_assert` at the bottom of the static table that the array length matches. If the compile-time check trips, strip the extras. |
| R7 | **LightPowerToLightAmount constants fabrication.** Pulling community-reference values is one step away from "fabrication". | Medium | Low (only inv 36 might numerically shift) | §4.10 tags the table `NEEDS DISASSEMBLY REVIEW`. Integration invariant 36 tests *event kind only* (`TIMELINE_EVENT_MAGIC_LIGHT_DECAY`), not exact lightAmount — so the golden is structural, not numeric. |
| R8 | **MagicState size collision with phase 11's 228 / phase 13's 76.** | Very low | Medium | Verified during §2 size-collision check. All phase-14 `_SIZE` macros are `MAGIC_` / `SPELL_` / `RUNE_` prefixed. `28` is the only value sharing with phase-11 but is a namespaced macro. Add a `_Static_assert(…)` at the top of the `.c` for each size. |
| R9 | **F0757 `* 40` tick scalar.** The ticks→spellPower scalar isn't bit-matchable without matching the MEDIA720 disasembly. | High | Low (affects durationTicks exact values, not event kind) | Invariants 35–36 check *event kind + ordering*, not exact ticks. Inline `NEEDS DISASSEMBLY REVIEW` tag. |
| R10 | **90-min budget overrun.** Phase 13 took ~12 min impl subagent — but Phase 14 is larger (38 invariants vs. 35, 9-way switch vs. 2 resolvers). | Medium | Schedule | Incremental invariant blocks (Step 5 A…K). If time is tight, blocks H-K are optional; the critical floor is blocks A-F (30 invariants, satisfies the ≥30 gate). Ship at the 30-invariant mark if needed. |

**Top two risks by expected cost:** **R2 (duplicate verify-script
append)** — same as Phase 13's top risk, because it has bitten before;
and **R5 (cross-phase state drift)** — the new dependency on
Phase 13's `F0736` / `CombatantChampionSnapshot_Compat` layout is the
first time phase-N takes a hard dependency on phase-(N-1) data.

---

## 9. Acceptance criteria

Phase 14 is complete and ready for merge when ALL of the following
hold:

- [ ] `bash run_firestaff_m10_verify.sh <dungeon.dat> <out>` exits 0.
- [ ] `grep -c '^# Phase 14:' run_firestaff_m10_verify.sh` equals
      exactly **1**.
- [ ] `grep -c '^# Phase 13:' run_firestaff_m10_verify.sh` still
      equals **1** (no regression).
- [ ] `$OUT_DIR/magic/magic_probe.md` exists and is non-empty.
- [ ] `$OUT_DIR/magic/magic_invariants.md` exists, its trailing
      line is `Status: PASS`, it contains `Invariant count: N`
      where `N ≥ 30` (target 38), and every line starting with `- `
      begins with `- PASS:`.
- [ ] `cc -Wall -Wextra -c` of every new `.c` file emits zero
      warnings.
- [ ] `ls tmp/firestaff/.phase*-attempt-* 2>/dev/null` returns
      nothing — no orphan backup directories.
- [ ] `git status` shows only the five files listed in §7 (no
      collateral edits).
- [ ] Phase 13 REVIEW markers #1 (fire / magic / psychic defence
      paths) **and** #3 (wake-from-rest on spell impact)
      demonstrably unblocked via invariants 32–34. Marker #2
      (lucky / cursed-items) has its data-layer hooks in
      `MagicState_Compat` — gameplay parity stays deferred, but
      the REVIEW comment in Phase 13's source file can cite the
      new hook (informational only; do NOT edit the Phase 13
      file).
- [ ] All 13 previous phases still pass — confirmed by the verify
      script exiting 0.

---

*End of plan. The Codex ACP executor should read this file in
full, then follow §6 steps 1→8 without deviation. Any ambiguity =
stop and emit a review-request comment rather than fabricate.*
