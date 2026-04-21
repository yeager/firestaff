#ifndef REDMCSB_MEMORY_MAGIC_PC34_COMPAT_H
#define REDMCSB_MEMORY_MAGIC_PC34_COMPAT_H

/*
 * Magic / spell-casting data layer for ReDMCSB PC 3.4 — Phase 14 of M10.
 *
 * Pure, caller-driven resolver for the rune -> cost -> validate ->
 * effect pipeline, plus the magic-side defender state that Phase 13's
 * combat resolver was forced to stub (fire/magic/psychic). Follows
 * the authoritative PHASE14_PLAN.md.
 *
 * Conventions (inherited from earlier phases):
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - MEDIA016 / PC LSB-first serialisation. Every struct round-trips
 *     bit-identical.
 *   - NO globals, NO UI, NO IO. Randomness flows through an explicit
 *     `struct RngState_Compat*` (Phase 13's primitive, reused —
 *     F0732).
 *   - Function numbering continues after phase 13 (F0730-F0747).
 *     Phase 14 claims F0750-F0769.
 *
 * ADDITIVE ONLY: this module consumes phase 13's
 * `CombatantChampionSnapshot_Compat` / `CombatResult_Compat` and
 * phase 12's `TimelineEvent_Compat`. It does NOT modify those types
 * — magic-side state is carried separately in MagicState_Compat.
 */

#include <stdint.h>

#include "memory_combat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

/* -------- Serialised sizes (MEDIA016, 4-byte int32 fields) -------- */

#define RUNE_SEQUENCE_SERIALIZED_SIZE        20  /*  5 int32 */
#define SPELL_DEFINITION_SERIALIZED_SIZE     28  /*  7 int32 */
#define SPELL_CAST_REQUEST_SERIALIZED_SIZE   64  /* 16 int32 */
#define SPELL_EFFECT_SERIALIZED_SIZE         84  /* 21 int32 */
#define MAGIC_STATE_SERIALIZED_SIZE          72  /* 18 int32 */

/* Spell-table size fixed to DM1 (25 entries); CSB magic-map extras
 * are OUT OF SCOPE for Phase 14 v1 — see §1 of PHASE14_PLAN.md. */
#define SPELL_TABLE_SIZE                     25

/* -------- Outcome codes (DEFS.H:2932..2943 mirror + v1 synthetics) -- */

#define SPELL_CAST_SUCCESS                    1
#define SPELL_CAST_FAILURE                    0
#define SPELL_CAST_FAILURE_NEEDS_FLASK        3

#define SPELL_FAILURE_NEEDS_MORE_PRACTICE     0   /* C00 */
#define SPELL_FAILURE_MEANINGLESS_SPELL       1   /* C01 */
#define SPELL_FAILURE_NEEDS_FLASK_IN_HAND    10   /* C10 */
#define SPELL_FAILURE_NEEDS_MAGIC_MAP        11   /* C11 */
#define SPELL_FAILURE_OUT_OF_MANA            12   /* Phase 14 synthetic */
#define SPELL_FAILURE_INVALID_RUNE           13   /* Phase 14 synthetic */

/* -------- Spell kinds (DEFS.H:1760..1763 mirror) -------- */

#define C1_SPELL_KIND_POTION_COMPAT          1
#define C2_SPELL_KIND_PROJECTILE_COMPAT      2
#define C3_SPELL_KIND_OTHER_COMPAT           3
#define C4_SPELL_KIND_MAGIC_MAP_COMPAT       4

/* -------- Spell types for "OTHER" kind (DEFS.H:1767..1775) -------- */

#define C0_SPELL_TYPE_OTHER_LIGHT_COMPAT          0
#define C1_SPELL_TYPE_OTHER_DARKNESS_COMPAT       1
#define C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT    2
#define C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT   3
#define C4_SPELL_TYPE_OTHER_PARTY_SHIELD_COMPAT   4
#define C5_SPELL_TYPE_OTHER_MAGIC_TORCH_COMPAT    5
#define C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT     6
#define C7_SPELL_TYPE_OTHER_ZOKATHRA_COMPAT       7
#define C8_SPELL_TYPE_OTHER_FIRESHIELD_COMPAT     8

/* -------- PROJECTILE sub-type (one documented in DEFS.H:1766) ----- */

#define C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR_COMPAT 4

/* -------- Timeline aux0 tags used by F0757 / F0763 ------------ */

#define TIMELINE_AUX_THIEVES_EYE     0x00740100
#define TIMELINE_AUX_INVISIBILITY    0x00740200
#define TIMELINE_AUX_PARTY_SHIELD    0x00740400
#define TIMELINE_AUX_FIRESHIELD      0x00740800
#define TIMELINE_AUX_FOOTPRINTS      0x00741000
#define TIMELINE_AUX_SPELL_SHIELD    0x00742000

/* -------- Data structures ------------------------------------- */

/*
 * Caller-produced rune buffer. Mirrors CHAMPION->Symbols[4].
 *
 *   +00 runeCount (1..4)
 *   +04 runes[0]   (power rune ordinal byte; 0 if no power typed)
 *   +08 runes[1]
 *   +12 runes[2]
 *   +16 runes[3]
 *   = 20 bytes
 *
 * Per SYMBOL.C:F0399, each rune byte is `0x60 + 6*step + symbolIdx`,
 * with step = 0..3 and symbolIdx = 0..5. See §4.1 of PHASE14_PLAN.md.
 */
struct RuneSequence_Compat {
    int runeCount;
    int runes[4];
};

/*
 * One entry in the 25-row DM1 spell table. Mirror of the DEFS.H
 * `SPELL` struct.
 *
 *   +00 symbolsPacked         (see below)
 *   +04 baseRequiredSkillLevel
 *   +08 skillIndex
 *   +12 attributes            (raw 16-bit word from G0487)
 *   +16 kind                  (M067 decode)
 *   +20 type                  (M068 decode)
 *   +24 disabledTicks         (M069 decode)
 *   = 28 bytes
 *
 * symbolsPacked layout (mirror of F0409_MENUS_GetSpellFromSymbols):
 *   byte 3 (MSB): power rune byte, or 0 if spell does not require power
 *   byte 2      : step-1 rune (element)
 *   byte 1      : step-2 rune (form)
 *   byte 0 (LSB): step-3 rune (class), or 0 if absent
 *
 * Example: Fireball "Ful Ir" -> 0x00696F00.
 */
struct SpellDefinition_Compat {
    int symbolsPacked;
    int baseRequiredSkillLevel;
    int skillIndex;
    int attributes;
    int kind;
    int type;
    int disabledTicks;
};

/*
 * Resolver input. Everything needed to decide cast outcome without
 * touching live champion/party state.
 */
struct SpellCastRequest_Compat {
    int championIndex;
    int currentMana;
    int maximumMana;
    int skillLevelForSpell;
    int statisticWisdom;
    int luckCurrent;
    int partyDirection;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int hasEmptyFlaskInHand;
    int hasMagicMapInHand;
    int gameTimeTicksLow;
    int spellTableIndex;
    int rawSymbolsPacked;
    int reserved;
};

/*
 * Resolver output. Pure data; no pointers, no aliasing.
 *
 *   magicStateDelta[0] = spellShieldDefenseDelta
 *   magicStateDelta[1] = fireShieldDefenseDelta
 *   magicStateDelta[2] = partyShieldDefenseDelta
 *   magicStateDelta[3] = magicalLightAmountDelta
 *   magicStateDelta[4] = freezeLifeTicksDelta
 *   magicStateDelta[5] = eventCountDelta
 */
struct SpellEffect_Compat {
    int castResult;
    int failureReason;
    int spellKind;
    int spellType;
    int powerOrdinal;          /* 1..6 (ordinal, not index) */
    int manaSpent;
    int impactAttack;
    int kineticEnergy;
    int durationTicks;
    int magicStateDelta[6];
    int followupEventKind;
    int followupEventAux0;
    int followupEventAux1;
    int poisonAttackPending;
    int wakeFromRest;
    int rngCallCount;
};

/*
 * Per-party magic-side state. Owned by the caller. Phase 14 reads it
 * for cast validation and defence adjustment; applies
 * `SpellEffect_Compat.magicStateDelta` to a caller-supplied copy via
 * F0760.
 */
struct MagicState_Compat {
    int spellShieldDefense;
    int fireShieldDefense;
    int partyShieldDefense;
    int magicalLightAmount;
    uint32_t lightDecayFireAtTick;
    int event70LightDirection;
    int event71CountInvisibility;
    int event73CountThievesEye;
    int event74CountPartyShield;
    int event77CountSpellShield;
    int event78CountFireShield;
    int event79CountFootprints;
    int freezeLifeTicks;
    int magicFootprintsActive;
    int luckCurrent;
    int curseMask;
    int reserved0;
    int reserved1;
};

/* ==========================================================
 *  Group A — Rune encoding / lookup (F0750-F0752)
 * ========================================================== */

int F0750_MAGIC_EncodeRuneSequence_Compat(
    const struct RuneSequence_Compat* seq,
    uint32_t* outPacked);

int F0751_MAGIC_DecodeRuneSequence_Compat(
    uint32_t packed,
    struct RuneSequence_Compat* outSeq);

int F0752_MAGIC_LookupSpellInTable_Compat(
    uint32_t packed,
    int* outTableIndex,
    struct SpellDefinition_Compat* outSpell);

/* Look up a spell definition by its zero-based table index (0..24). */
int F0752b_MAGIC_LookupSpellByTableIndex_Compat(
    int tableIndex,
    struct SpellDefinition_Compat* outSpell);

/* ==========================================================
 *  Group B — Cast validation (F0753-F0755)
 * ========================================================== */

int F0753_MAGIC_ComputeManaCost_Compat(
    const struct RuneSequence_Compat* seq,
    int* outCost);

int F0754_MAGIC_ValidateCastRequest_Compat(
    const struct SpellCastRequest_Compat* req,
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    struct RngState_Compat* rng,
    int* outFailureReason);

int F0755_MAGIC_CheckSkillRequired_Compat(
    int baseRequiredSkillLevel,
    int powerOrdinal,
    int skillLevel,
    int* outMissing);

/* ==========================================================
 *  Group C — Effect generation (F0756-F0760)
 * ========================================================== */

int F0756_MAGIC_ProduceProjectileEffect_Compat(
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    int skillLevel,
    struct RngState_Compat* rng,
    struct SpellEffect_Compat* out);

int F0757_MAGIC_ProduceOtherEffect_Compat(
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    const struct MagicState_Compat* magic,
    struct SpellEffect_Compat* out);

int F0758_MAGIC_ProducePotionEffect_Compat(
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    int hasEmptyFlaskInHand,
    struct RngState_Compat* rng,
    struct SpellEffect_Compat* out);

int F0759_MAGIC_ApplySpellImpactToChampion_Compat(
    const struct SpellEffect_Compat* effect,
    const struct CombatantChampionSnapshot_Compat* champ,
    const struct MagicState_Compat* magic,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out);

int F0760_MAGIC_ApplyStateDelta_Compat(
    const struct SpellEffect_Compat* effect,
    struct MagicState_Compat* inOutMagic);

/* ==========================================================
 *  Group D — Magic-defender adjustments (F0761-F0762)
 *  Resolves Phase 13's fire/magic/psychic DISASSEMBLY REVIEW markers.
 * ========================================================== */

int F0761_MAGIC_GetDefenderMagicalAdjustedAttack_Compat(
    int attackType,
    const struct CombatantChampionSnapshot_Compat* defender,
    const struct MagicState_Compat* magic,
    int rawAttack,
    int* outAdjusted);

int F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat(
    const struct CombatantChampionSnapshot_Compat* defender,
    int statisticWisdom,
    int rawAttack,
    int* outAdjusted);

/* ==========================================================
 *  Group E — Timeline bridge (F0763)
 * ========================================================== */

int F0763_MAGIC_BuildTimelineEvent_Compat(
    const struct SpellEffect_Compat* effect,
    int partyMapIndex,
    int partyMapX,
    int partyMapY,
    int partyCell,
    uint32_t nowTick,
    struct TimelineEvent_Compat* outEvent);

/* ==========================================================
 *  Group F — Serialisation (F0764-F0769; paired ser/deser)
 * ========================================================== */

int F0764_MAGIC_RuneSequenceSerialize_Compat(
    const struct RuneSequence_Compat* seq,
    unsigned char* outBuf,
    int outBufSize);

int F0765_MAGIC_RuneSequenceDeserialize_Compat(
    struct RuneSequence_Compat* seq,
    const unsigned char* buf,
    int bufSize);

int F0766a_MAGIC_SpellCastRequestSerialize_Compat(
    const struct SpellCastRequest_Compat* req,
    unsigned char* outBuf,
    int outBufSize);

int F0766b_MAGIC_SpellCastRequestDeserialize_Compat(
    struct SpellCastRequest_Compat* req,
    const unsigned char* buf,
    int bufSize);

int F0767a_MAGIC_SpellEffectSerialize_Compat(
    const struct SpellEffect_Compat* effect,
    unsigned char* outBuf,
    int outBufSize);

int F0767b_MAGIC_SpellEffectDeserialize_Compat(
    struct SpellEffect_Compat* effect,
    const unsigned char* buf,
    int bufSize);

int F0768a_MAGIC_MagicStateSerialize_Compat(
    const struct MagicState_Compat* magic,
    unsigned char* outBuf,
    int outBufSize);

int F0768b_MAGIC_MagicStateDeserialize_Compat(
    struct MagicState_Compat* magic,
    const unsigned char* buf,
    int bufSize);

int F0769a_MAGIC_SpellDefinitionSerialize_Compat(
    const struct SpellDefinition_Compat* spell,
    unsigned char* outBuf,
    int outBufSize);

int F0769b_MAGIC_SpellDefinitionDeserialize_Compat(
    struct SpellDefinition_Compat* spell,
    const unsigned char* buf,
    int bufSize);

#endif /* REDMCSB_MEMORY_MAGIC_PC34_COMPAT_H */
