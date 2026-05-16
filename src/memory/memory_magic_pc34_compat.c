/*
 * Magic / spell-casting data layer for ReDMCSB PC 3.4 — Phase 14 of M10.
 *
 * Pure resolver for the rune->cost->validate->effect pipeline,
 * magic-side defender adjustments (fire / magic / psychic — unblocking
 * Phase 13 DISASSEMBLY REVIEW markers), and magic-state delta
 * application. See PHASE14_PLAN.md for the authoritative scope.
 *
 * Design rules:
 *   - NO globals, NO hidden state; every call takes (inputs, out).
 *   - NO IO, NO UI, NO sound hooks.
 *   - Caller supplies snapshots and an RngState_Compat; nothing is
 *     mutated except out-params and (for F0760) the explicit
 *     caller-owned MagicState_Compat target.
 *   - MEDIA016 / PC LSB-first serialisation for every struct.
 *
 * NEEDS DISASSEMBLY REVIEW markers in this file are documented below
 * at each site, mirroring the Phase 13 convention.
 */

#include <string.h>

#include "memory_magic_pc34_compat.h"

/* ==========================================================
 *  Compile-time invariants mirroring size-table macros.
 * ========================================================== */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(int) == 4,
    "memory_magic_pc34_compat requires 32-bit int");
_Static_assert(RUNE_SEQUENCE_SERIALIZED_SIZE == 20,
    "RUNE_SEQUENCE_SERIALIZED_SIZE drift");
_Static_assert(SPELL_DEFINITION_SERIALIZED_SIZE == 28,
    "SPELL_DEFINITION_SERIALIZED_SIZE drift");
_Static_assert(SPELL_CAST_REQUEST_SERIALIZED_SIZE == 64,
    "SPELL_CAST_REQUEST_SERIALIZED_SIZE drift");
_Static_assert(SPELL_EFFECT_SERIALIZED_SIZE == 84,
    "SPELL_EFFECT_SERIALIZED_SIZE drift");
_Static_assert(MAGIC_STATE_SERIALIZED_SIZE == 72,
    "MAGIC_STATE_SERIALIZED_SIZE drift");
_Static_assert(SPELL_TABLE_SIZE == 25,
    "SPELL_TABLE_SIZE locked to DM1 25 entries");
#endif

/* ==========================================================
 *  Internal helpers: LE int32 serialisation (same pattern as
 *  memory_timeline_pc34_compat.c / memory_combat_pc34_compat.c).
 * ========================================================== */

static void write_i32_le(unsigned char* p, int value) {
    unsigned int u = (unsigned int)value;
    p[0] = (unsigned char)(u & 0xFF);
    p[1] = (unsigned char)((u >> 8) & 0xFF);
    p[2] = (unsigned char)((u >> 16) & 0xFF);
    p[3] = (unsigned char)((u >> 24) & 0xFF);
}

static int read_i32_le(const unsigned char* p) {
    unsigned int u =
        ((unsigned int)p[0]) |
        ((unsigned int)p[1] << 8) |
        ((unsigned int)p[2] << 16) |
        ((unsigned int)p[3] << 24);
    return (int)u;
}

static void write_u32_le(unsigned char* p, uint32_t u) {
    p[0] = (unsigned char)(u & 0xFF);
    p[1] = (unsigned char)((u >> 8) & 0xFF);
    p[2] = (unsigned char)((u >> 16) & 0xFF);
    p[3] = (unsigned char)((u >> 24) & 0xFF);
}

static uint32_t read_u32_le(const unsigned char* p) {
    return
        ((uint32_t)p[0]) |
        ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) |
        ((uint32_t)p[3] << 24);
}

/* ==========================================================
 *  Static lookup tables (mirrors of Fontanel MENU.C globals).
 *
 *  G0485_aauc_Graphic560_SymbolBaseManaCost[4][6]
 *  G0486_auc_Graphic560_SymbolManaCostMultiplier[6]
 *  G0487_as_Graphic560_Spells[25]       (DM1 entries only — CSB
 *                                         magic-map extras excluded
 *                                         from v1 per §1 out-of-scope)
 * ========================================================== */

/* MENU.C:44..47 */
static const unsigned char Phase14_SymbolBaseManaCost[4][6] = {
    { 1, 2, 3, 4, 5, 6 },   /* Power step  (ordinal Lo..Mon) */
    { 2, 3, 4, 5, 6, 7 },   /* Element step (Ya..Zo)         */
    { 4, 5, 6, 7, 7, 9 },   /* Form step    (Ven..Gor)       */
    { 2, 2, 3, 4, 6, 7 }    /* Class step   (Ku..Sar)        */
};

/* MENU.C:49 */
static const unsigned char Phase14_SymbolManaCostMultiplier[6] = {
    8, 12, 16, 20, 24, 28
};

/*
 * NEEDS DISASSEMBLY REVIEW: the canonical table lives in GRAPHICS.DAT
 * entry 562 (G0039). v1 ships community-reference values that preserve
 * monotonic ordering. Goldens for light-driven branches are structural
 * (event kind only), not numeric — see §4.10 R7 in PHASE14_PLAN.md.
 */
static const int Phase14_PowerOrdinalToLightAmount[6] = {
    3, 6, 10, 16, 24, 40
};

/* MENU.C:50..77 — 25-entry DM1 spell table. Kind/type/disabledTicks
 * are derived from `attributes` via the M067/M068/M069 helpers in
 * F0752 at lookup time; the raw 16-bit attribute word is what the
 * original cast-tree sees, so we keep it as the canonical field. */
struct Phase14_RawSpellRow {
    int symbolsPacked;
    int baseRequiredSkillLevel;
    int skillIndex;
    int attributes;
};

static const struct Phase14_RawSpellRow Phase14_SpellTable[SPELL_TABLE_SIZE] = {
    { 0x00666F00, 2, 15, 0x7843 }, /* Ya Ir          Shield (Party) */
    { 0x00667073, 1, 18, 0x4863 }, /* Ya Bro Ros     Magic Footprints */
    { 0x00686D77, 3, 17, 0xB433 }, /* Oh Ew Sar      Invisibility */
    { 0x00686C00, 3, 19, 0x6C72 }, /* Oh Ven         Poison Cloud */
    { 0x00686D76, 3, 18, 0x8423 }, /* Oh Ew Ra       Thieves Eye */
    { 0x00686E76, 4, 17, 0x7822 }, /* Oh Kath Ra     Lightning Bolt */
    { 0x00686F76, 4, 17, 0x5803 }, /* Oh Ir Ra       Light */
    { 0x00690000, 1, 16, 0x3C53 }, /* Ful            Torch */
    { 0x00696F00, 3, 16, 0xA802 }, /* Ful Ir         Fireball */
    { 0x00697072, 4, 13, 0x3C71 }, /* Ful Bro Ku     Strength Potion */
    { 0x00697075, 4, 15, 0x7083 }, /* Ful Bro Neta   Fire Shield */
    { 0x006A6D00, 1, 18, 0x5032 }, /* Des Ew         Weaken Non-Material */
    { 0x006A6C00, 1, 19, 0x4062 }, /* Des Ven        Poison Bolt */
    { 0x006A6F77, 1, 15, 0x3013 }, /* Des Ir Sar     Darkness */
    { 0x006B0000, 1, 17, 0x3C42 }, /* Zo             Open Door */
    { 0x00667000, 2, 15, 0x64C1 }, /* Ya Bro         Shield Potion */
    { 0x00660000, 2, 13, 0x3CB1 }, /* Ya             Stamina Potion */
    { 0x00667074, 4, 13, 0x3C81 }, /* Ya Bro Dain    Wisdom Potion */
    { 0x00667075, 4, 13, 0x3C91 }, /* Ya Bro Neta    Vitality Potion */
    { 0x00670000, 1, 13, 0x80E1 }, /* Vi             Health Potion */
    { 0x00677000, 1, 13, 0x68A1 }, /* Vi Bro         Cure Poison */
    { 0x00687073, 4, 13, 0x3C61 }, /* Oh Bro Ros     Dexterity Potion */
    { 0x006B7076, 3,  2, 0xFCD1 }, /* Zo Bro Ra      Mana Potion */
    { 0x006B6C00, 2, 19, 0x7831 }, /* Zo Ven         Poison Potion */
    { 0x006B6E76, 0,  3, 0x3C73 }  /* Zo Kath Ra     Zokathra */
};

/* Silence unused-warning for the light table in v1 configurations
 * that don't exercise the magnitude path in goldens. */
static int phase14_light_amount_for_power(int powerIndex)
{
    if (powerIndex < 0 || powerIndex >= 6) {
        return 0;
    }
    return Phase14_PowerOrdinalToLightAmount[powerIndex];
}

/* Decode M067/M068/M069 bit-packed fields. */
static int phase14_spell_kind_from_attr(int attributes) {
    return attributes & 0x000F;
}
static int phase14_spell_type_from_attr(int attributes) {
    return (attributes >> 4) & 0x003F;
}
static int phase14_spell_disabled_ticks_from_attr(int attributes) {
    return (attributes >> 10) & 0x003F;
}

/* ==========================================================
 *  Group A — Rune encoding / lookup (F0750-F0752)
 *
 *  Reference: MENU.C:1666..1707 (F0409_MENUS_GetSpellFromSymbols)
 *             MENU.C:1690 packing loop
 * ========================================================== */

int F0750_MAGIC_EncodeRuneSequence_Compat(
    const struct RuneSequence_Compat* seq,
    uint32_t* outPacked)
{
    uint32_t packed;
    int shift;
    int i;
    int byteValue;

    if (seq == 0 || outPacked == 0) {
        return 0;
    }
    if (seq->runeCount < 1 || seq->runeCount > 4) {
        return 0;
    }

    packed = 0;
    shift = 24;
    for (i = 0; i < seq->runeCount; i++) {
        byteValue = seq->runes[i] & 0xFF;
        if ((seq->runes[i] & ~0xFF) != 0) {
            return 0; /* any bits above byte 0 -> invalid rune */
        }
        if (byteValue > 0x77) {
            return 0;
        }
        packed |= ((uint32_t)byteValue) << shift;
        shift -= 8;
    }
    *outPacked = packed;
    return 1;
}

int F0751_MAGIC_DecodeRuneSequence_Compat(
    uint32_t packed,
    struct RuneSequence_Compat* outSeq)
{
    int i;
    unsigned int b;
    int lastNonZero;

    if (outSeq == 0) {
        return 0;
    }

    /* Byte 3 (MSB) goes to runes[0], byte 0 (LSB) to runes[3]. */
    for (i = 0; i < 4; i++) {
        b = (packed >> (24 - (i * 8))) & 0xFFu;
        outSeq->runes[i] = (int)b;
    }

    /* runeCount = position of last non-zero byte, plus 1. If ALL four
     * bytes are zero, treat as runeCount=1 with runes[0]=0 (degenerate
     * caller input — F0750 will refuse such input, so this is only a
     * round-trip artefact). */
    lastNonZero = -1;
    for (i = 0; i < 4; i++) {
        if (outSeq->runes[i] != 0) {
            lastNonZero = i;
        }
    }
    if (lastNonZero < 0) {
        outSeq->runeCount = 1;
    } else {
        outSeq->runeCount = lastNonZero + 1;
    }
    return 1;
}

int F0752_MAGIC_LookupSpellInTable_Compat(
    uint32_t packed,
    int* outTableIndex,
    struct SpellDefinition_Compat* outSpell)
{
    int i;

    if (outTableIndex == 0) {
        return 0;
    }

    for (i = 0; i < SPELL_TABLE_SIZE; i++) {
        const struct Phase14_RawSpellRow* row = &Phase14_SpellTable[i];
        uint32_t spellSymbols = (uint32_t)row->symbolsPacked;
        int hit = 0;

        if ((spellSymbols & 0xFF000000u) != 0u) {
            /* Spell includes the power byte — compare full word. */
            if (packed == spellSymbols) {
                hit = 1;
            }
        } else {
            if ((packed & 0x00FFFFFFu) == spellSymbols) {
                hit = 1;
            }
        }

        if (hit) {
            *outTableIndex = i;
            if (outSpell != 0) {
                outSpell->symbolsPacked          = row->symbolsPacked;
                outSpell->baseRequiredSkillLevel = row->baseRequiredSkillLevel;
                outSpell->skillIndex             = row->skillIndex;
                outSpell->attributes             = row->attributes;
                outSpell->kind          = phase14_spell_kind_from_attr(row->attributes);
                outSpell->type          = phase14_spell_type_from_attr(row->attributes);
                outSpell->disabledTicks = phase14_spell_disabled_ticks_from_attr(row->attributes);
            }
            return 1;
        }
    }
    return 0;
}

int F0752b_MAGIC_LookupSpellByTableIndex_Compat(
    int tableIndex,
    struct SpellDefinition_Compat* outSpell)
{
    const struct Phase14_RawSpellRow* row;
    if (tableIndex < 0 || tableIndex >= SPELL_TABLE_SIZE) return 0;
    if (!outSpell) return 0;
    row = &Phase14_SpellTable[tableIndex];
    outSpell->symbolsPacked          = row->symbolsPacked;
    outSpell->baseRequiredSkillLevel = row->baseRequiredSkillLevel;
    outSpell->skillIndex             = row->skillIndex;
    outSpell->attributes             = row->attributes;
    outSpell->kind          = phase14_spell_kind_from_attr(row->attributes);
    outSpell->type          = phase14_spell_type_from_attr(row->attributes);
    outSpell->disabledTicks = phase14_spell_disabled_ticks_from_attr(row->attributes);
    return 1;
}

/* ==========================================================
 *  Group B — Cast validation (F0753-F0755)
 *
 *  Reference: SYMBOL.C F0399_MENUS_AddChampionSymbol (mana cost)
 *             MENU.C:1755..1844 F0412 validation tail
 * ========================================================== */

int F0753_MAGIC_ComputeManaCost_Compat(
    const struct RuneSequence_Compat* seq,
    int* outCost)
{
    int step;
    int total;
    int powerIndex;
    int byteValue;
    int stepBase;
    int symbolIdx;
    int base;
    int cost;

    if (seq == 0 || outCost == 0) {
        return 0;
    }
    if (seq->runeCount < 1 || seq->runeCount > 4) {
        return 0;
    }

    total = 0;
    powerIndex = -1;

    for (step = 0; step < seq->runeCount; step++) {
        byteValue = seq->runes[step] & 0xFF;
        stepBase = 0x60 + (step * 6);
        symbolIdx = byteValue - stepBase;
        if (symbolIdx < 0 || symbolIdx > 5) {
            return 0;   /* malformed rune for this step */
        }
        base = (int)Phase14_SymbolBaseManaCost[step][symbolIdx];
        if (step == 0) {
            cost = base;
            powerIndex = symbolIdx;         /* 0..5 for mult[] lookup */
        } else {
            if (powerIndex < 0) {
                return 0;
            }
            cost = (base * (int)Phase14_SymbolManaCostMultiplier[powerIndex]) >> 3;
        }
        total += cost;
    }

    *outCost = total;
    return 1;
}

int F0755_MAGIC_CheckSkillRequired_Compat(
    int baseRequiredSkillLevel,
    int powerOrdinal,
    int skillLevel,
    int* outMissing)
{
    int required;
    if (outMissing == 0) {
        return 0;
    }
    required = baseRequiredSkillLevel + powerOrdinal;
    if (skillLevel >= required) {
        *outMissing = 0;
        return 1;
    }
    *outMissing = required - skillLevel;
    return 0;
}

int F0754_MAGIC_ValidateCastRequest_Compat(
    const struct SpellCastRequest_Compat* req,
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    struct RngState_Compat* rng,
    int* outFailureReason)
{
    int required;
    int missing;
    int r;
    int wisdomCap;

    if (outFailureReason == 0) {
        return SPELL_CAST_FAILURE;
    }
    *outFailureReason = 0;

    if (req == 0 || spell == 0) {
        *outFailureReason = SPELL_FAILURE_MEANINGLESS_SPELL;
        return SPELL_CAST_FAILURE;
    }
    if (req->championIndex < 0 || req->championIndex >= CHAMPION_MAX_PARTY) {
        *outFailureReason = SPELL_FAILURE_MEANINGLESS_SPELL;
        return SPELL_CAST_FAILURE;
    }

    /* Mana budget. */
    {
        struct RuneSequence_Compat tmpSeq;
        int tmpCost = 0;
        uint32_t packedLocal = (uint32_t)req->rawSymbolsPacked;
        int ok = F0751_MAGIC_DecodeRuneSequence_Compat(packedLocal, &tmpSeq);
        if (ok && F0753_MAGIC_ComputeManaCost_Compat(&tmpSeq, &tmpCost)) {
            if (req->currentMana < tmpCost) {
                *outFailureReason = SPELL_FAILURE_OUT_OF_MANA;
                return SPELL_CAST_FAILURE;
            }
        } else {
            /* Could not decode cost — conservative fail. */
            if (req->currentMana <= 0) {
                *outFailureReason = SPELL_FAILURE_OUT_OF_MANA;
                return SPELL_CAST_FAILURE;
            }
        }
    }

    /* Skill-vs-required check (MENU.C:1801..1812). */
    required = spell->baseRequiredSkillLevel + powerOrdinal;
    if (req->skillLevelForSpell < required) {
        missing = required - req->skillLevelForSpell;
        while (missing > 0) {
            if (rng == 0) {
                *outFailureReason = SPELL_FAILURE_NEEDS_MORE_PRACTICE;
                return SPELL_CAST_FAILURE;
            }
            r = F0732_COMBAT_RngRandom_Compat(rng, 128);
            wisdomCap = req->statisticWisdom + 15;
            if (wisdomCap > 115) {
                wisdomCap = 115;
            }
            if (r > wisdomCap) {
                *outFailureReason = SPELL_FAILURE_NEEDS_MORE_PRACTICE;
                return SPELL_CAST_FAILURE;
            }
            missing--;
        }
    }

    /* Kind-specific gates (MENU.C:1845..1874). */
    switch (spell->kind) {
        case C1_SPELL_KIND_POTION_COMPAT:
            if (!req->hasEmptyFlaskInHand) {
                *outFailureReason = SPELL_FAILURE_NEEDS_FLASK_IN_HAND;
                return SPELL_CAST_FAILURE_NEEDS_FLASK;
            }
            break;
        case C4_SPELL_KIND_MAGIC_MAP_COMPAT:
            if (!req->hasMagicMapInHand) {
                *outFailureReason = SPELL_FAILURE_NEEDS_MAGIC_MAP;
                return SPELL_CAST_FAILURE_NEEDS_FLASK;
            }
            break;
        case C2_SPELL_KIND_PROJECTILE_COMPAT:
        case C3_SPELL_KIND_OTHER_COMPAT:
        default:
            break;
    }

    return SPELL_CAST_SUCCESS;
}

/* ==========================================================
 *  Group C — Effect generation (F0756-F0760)
 *
 *  Reference: MENU.C:1821..2031, CHAMPION.C:1824..1913
 * ========================================================== */

static int phase14_bounded_value(int lo, int value, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

int F0756_MAGIC_ProduceProjectileEffect_Compat(
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    int skillLevel,
    struct RngState_Compat* rng,
    struct SpellEffect_Compat* out)
{
    int effectiveSkill;
    int raw;

    (void)rng;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    if (spell == 0) {
        return 0;
    }
    if (powerOrdinal < 1 || powerOrdinal > 6) {
        return 0;
    }

    out->spellKind = C2_SPELL_KIND_PROJECTILE_COMPAT;
    out->spellType = spell->type;
    out->powerOrdinal = powerOrdinal;

    effectiveSkill = skillLevel;
    if (spell->type == C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR_COMPAT) {
        effectiveSkill <<= 1;   /* MENU.C:1826 */
    }

    /* F0026_MAIN_GetBoundedValue(21, (powerOrdinal+2)*(4+(skill<<1)), 255) */
    raw = (powerOrdinal + 2) * (4 + (effectiveSkill << 1));
    out->impactAttack = phase14_bounded_value(21, raw, 255);
    out->kineticEnergy = 90;
    out->followupEventKind = TIMELINE_EVENT_PROJECTILE_MOVE;
    /* 0xFF80 + type is the THING_FIRST_EXPLOSION + type code used by
     * F0327_CHAMPION_IsProjectileSpellCast (MENU.C:1828). */
    out->followupEventAux0 = 0xFF80 + spell->type;
    out->followupEventAux1 = spell->type;
    out->castResult = SPELL_CAST_SUCCESS;
    out->rngCallCount = 0;
    return 1;
}

int F0757_MAGIC_ProduceOtherEffect_Compat(
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    const struct MagicState_Compat* magic,
    struct SpellEffect_Compat* out)
{
    int spellPower;
    int lightPower;
    int ticks;
    int defense;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    if (spell == 0 || magic == 0) {
        return 0;
    }
    if (powerOrdinal < 1 || powerOrdinal > 6) {
        return 0;
    }

    out->spellKind = C3_SPELL_KIND_OTHER_COMPAT;
    out->spellType = spell->type;
    out->powerOrdinal = powerOrdinal;

    /* MENU.C:1923 AL1267_ui_SpellPower = (PowerSymbolOrdinal + 1) << 2.
     * Note: Fontanel uses ordinal 1..6; plan §4.6 uses the same. */
    spellPower = (powerOrdinal + 1) << 2;

    switch (spell->type) {
        case C0_SPELL_TYPE_OTHER_LIGHT_COMPAT:
            /* MENU.C:1927..1930 */
            ticks = 10000 + ((spellPower - 8) << 9);
            lightPower = (spellPower >> 1) - 1;
            if (lightPower < 0) lightPower = 0;
            if (lightPower > 5) lightPower = 5;
            out->durationTicks = ticks;
            out->magicStateDelta[3] = phase14_light_amount_for_power(lightPower);
            out->followupEventKind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
            out->followupEventAux0 = -lightPower;
            break;

        case C1_SPELL_TYPE_OTHER_DARKNESS_COMPAT:
            /* MENU.C:1954..1957 */
            lightPower = spellPower >> 2;
            if (lightPower < 0) lightPower = 0;
            if (lightPower > 5) lightPower = 5;
            out->magicStateDelta[3] = -phase14_light_amount_for_power(lightPower);
            out->durationTicks = 98;
            out->followupEventKind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
            out->followupEventAux0 = lightPower;
            break;

        case C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT:
            /* MENU.C:1960..1963 (T0412032 tail) */
            spellPower >>= 1;
            /* NEEDS DISASSEMBLY REVIEW: the ticks scalar is derived
             * from AL1269_ui_Ticks being multiplied by SpellPower;
             * the pre-multiplication value comes from a media-variant
             * block we cannot fully disambiguate without MEDIA720
             * context. v1 uses `spellPower * 40` as the conservative
             * envelope; invariants check event KIND only, not exact
             * ticks. */
            out->durationTicks = spellPower * 40;
            out->magicStateDelta[5] = 1;
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
            out->followupEventAux0 = TIMELINE_AUX_THIEVES_EYE;
            break;

        case C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT:
            /* MENU.C:1970..1982 (MEDIA720 path: spellPower <<= 3) */
            spellPower <<= 3;
            out->durationTicks = spellPower * 40;
            out->magicStateDelta[5] = 1;
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
            out->followupEventAux0 = TIMELINE_AUX_INVISIBILITY;
            break;

        case C4_SPELL_TYPE_OTHER_PARTY_SHIELD_COMPAT:
            /* MENU.C:1988..1996 */
            defense = spellPower;
            if (magic->partyShieldDefense > 50) {
                defense >>= 2;
            }
            out->magicStateDelta[2] = defense;
            out->durationTicks = spellPower * 40;
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
            out->followupEventAux0 = TIMELINE_AUX_PARTY_SHIELD;
            break;

        case C5_SPELL_TYPE_OTHER_MAGIC_TORCH_COMPAT:
            /* MENU.C:1934..1937 */
            ticks = 2000 + ((spellPower - 3) << 7);
            lightPower = (spellPower >> 2) + 1;
            if (lightPower < 0) lightPower = 0;
            if (lightPower > 5) lightPower = 5;
            out->durationTicks = ticks;
            out->magicStateDelta[3] = phase14_light_amount_for_power(lightPower);
            out->followupEventKind = TIMELINE_EVENT_MAGIC_LIGHT_DECAY;
            out->followupEventAux0 = -lightPower;
            break;

        case C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT:
            /* MENU.C:2001..2009 */
            out->durationTicks = spellPower * 40;
            out->magicStateDelta[5] = 1;
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
            out->followupEventAux0 = TIMELINE_AUX_FOOTPRINTS;
            break;

        case C7_SPELL_TYPE_OTHER_ZOKATHRA_COMPAT:
            /* MENU.C:2014..2023 — caller is responsible for the JUNK
             * allocation. v1 emits an INVALID event kind meaning
             * "no timeline entry to schedule" and a 0 duration. */
            out->durationTicks = 0;
            out->followupEventKind = TIMELINE_EVENT_INVALID;
            out->followupEventAux0 = 0;
            break;

        case C8_SPELL_TYPE_OTHER_FIRESHIELD_COMPAT:
            /* MENU.C:2026..2030 -> F0403: defense = spellPower^2 + 100.
             * Phase 14 v1 consumes the result directly:
             * magicStateDelta[1] = defense >> 5 (ticks-like scale). */
            defense = (spellPower * spellPower) + 100;
            out->durationTicks = defense >> 5;
            out->magicStateDelta[1] = defense >> 5;
            out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
            out->followupEventAux0 = TIMELINE_AUX_FIRESHIELD;
            break;

        default:
            return 0;
    }

    out->castResult = SPELL_CAST_SUCCESS;
    return 1;
}

int F0758_MAGIC_ProducePotionEffect_Compat(
    const struct SpellDefinition_Compat* spell,
    int powerOrdinal,
    int hasEmptyFlaskInHand,
    struct RngState_Compat* rng,
    struct SpellEffect_Compat* out)
{
    int r16;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    if (spell == 0) {
        return 0;
    }
    if (powerOrdinal < 1 || powerOrdinal > 6) {
        return 0;
    }

    out->spellKind = C1_SPELL_KIND_POTION_COMPAT;
    out->spellType = spell->type;
    out->powerOrdinal = powerOrdinal;

    if (!hasEmptyFlaskInHand) {
        out->castResult = SPELL_CAST_FAILURE_NEEDS_FLASK;
        out->failureReason = SPELL_FAILURE_NEEDS_FLASK_IN_HAND;
        out->followupEventKind = TIMELINE_EVENT_INVALID;
        return 1;
    }

    /* MENU.C:1859: L1275_ps_Potion->Power = M003_RANDOM(16) + (ordinal * 40). */
    r16 = 0;
    if (rng != 0) {
        r16 = F0732_COMBAT_RngRandom_Compat(rng, 16);
        out->rngCallCount = 1;
    }
    out->kineticEnergy = r16 + (powerOrdinal * 40);
    out->followupEventKind = TIMELINE_EVENT_INVALID;  /* potion fill is caller-owned */
    out->castResult = SPELL_CAST_SUCCESS;
    return 1;
}

int F0759_MAGIC_ApplySpellImpactToChampion_Compat(
    const struct SpellEffect_Compat* effect,
    const struct CombatantChampionSnapshot_Compat* champ,
    const struct MagicState_Compat* magic,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out)
{
    int attackType;
    int rawAttack;
    int adjusted;

    (void)rng;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->outcome = COMBAT_OUTCOME_MISS;
    out->creatureSlotRemoved = -1;
    out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;

    if (effect == 0 || champ == 0 || magic == 0) {
        return 0;
    }

    /* Resolve attack type from spell type. For projectile fire spells
     * (Fireball, Fire Shield side-effect, etc.) we treat it as
     * COMBAT_ATTACK_FIRE; for lightning / open-door / weaken /
     * non-material we use COMBAT_ATTACK_MAGIC. Poison cloud routes
     * via COMBAT_ATTACK_NORMAL here; its vitality scaling is applied
     * by the caller to poisonAttackPending. */
    switch (effect->spellType) {
        case 0:  /* FIREBALL */
            attackType = COMBAT_ATTACK_FIRE;
            break;
        case 2:  /* LIGHTNING BOLT */
            attackType = COMBAT_ATTACK_LIGHTNING;
            break;
        case 1:  /* POISON BOLT */
        case 7:  /* POISON CLOUD */
            attackType = COMBAT_ATTACK_NORMAL;
            break;
        case 3:  /* WEAKEN NON-MATERIAL */
        case 4:  /* OPEN DOOR */
        case 6:  /* HARM NON-MATERIAL */
            attackType = COMBAT_ATTACK_MAGIC;
            break;
        default:
            attackType = COMBAT_ATTACK_MAGIC;
            break;
    }

    rawAttack = effect->impactAttack;
    adjusted = rawAttack;

    if (attackType == COMBAT_ATTACK_FIRE ||
        attackType == COMBAT_ATTACK_MAGIC) {
        int tmp = rawAttack;
        if (F0761_MAGIC_GetDefenderMagicalAdjustedAttack_Compat(
                attackType, champ, magic, rawAttack, &tmp)) {
            adjusted = tmp;
        }
    } else if (attackType == COMBAT_ATTACK_PSYCHIC) {
        int tmp = rawAttack;
        /* NEEDS DISASSEMBLY REVIEW: psychic impact from spells is not
         * documented in DM1 spell table (no psychic-damage spells in
         * v1's 25-entry list). Path present for completeness; no
         * golden exercises this branch. */
        if (F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat(
                champ, magic->luckCurrent, rawAttack, &tmp)) {
            adjusted = tmp;
        }
    }

    if (adjusted < 0) adjusted = 0;
    out->damageApplied = adjusted;
    out->rawAttackRoll = rawAttack;
    out->defenseRoll = rawAttack - adjusted;
    out->hitLanded = (adjusted > 0) ? 1 : 0;
    out->outcome = (adjusted > 0) ? COMBAT_OUTCOME_HIT_DAMAGE
                                  : COMBAT_OUTCOME_HIT_NO_DAMAGE;
    out->wakeFromRest = champ->isResting ? 1 : 0;
    out->poisonAttackPending = effect->poisonAttackPending;
    out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
    out->followupEventAux0 = effect->followupEventAux0;
    return 1;
}

int F0760_MAGIC_ApplyStateDelta_Compat(
    const struct SpellEffect_Compat* effect,
    struct MagicState_Compat* inOutMagic)
{
    int shieldDelta;
    int fireDelta;
    int partyDelta;

    if (effect == 0 || inOutMagic == 0) {
        return 0;
    }

    shieldDelta = effect->magicStateDelta[0];
    fireDelta   = effect->magicStateDelta[1];
    partyDelta  = effect->magicStateDelta[2];

    /* "> 50 -> delta >>= 2" rule (MENU.C:1969 + 1086). */
    if (inOutMagic->spellShieldDefense > 50) {
        shieldDelta >>= 2;
    }
    if (inOutMagic->fireShieldDefense > 50) {
        fireDelta >>= 2;
    }
    if (inOutMagic->partyShieldDefense > 50) {
        partyDelta >>= 2;
    }

    inOutMagic->spellShieldDefense += shieldDelta;
    inOutMagic->fireShieldDefense  += fireDelta;
    inOutMagic->partyShieldDefense += partyDelta;
    inOutMagic->magicalLightAmount += effect->magicStateDelta[3];
    inOutMagic->freezeLifeTicks    += effect->magicStateDelta[4];

    /* Per-event-kind count bump (magicStateDelta[5] == +1 when set). */
    switch (effect->spellType) {
        case C2_SPELL_TYPE_OTHER_THIEVES_EYE_COMPAT:
            inOutMagic->event73CountThievesEye += effect->magicStateDelta[5];
            break;
        case C3_SPELL_TYPE_OTHER_INVISIBILITY_COMPAT:
            inOutMagic->event71CountInvisibility += effect->magicStateDelta[5];
            break;
        case C4_SPELL_TYPE_OTHER_PARTY_SHIELD_COMPAT:
            inOutMagic->event74CountPartyShield += effect->magicStateDelta[5];
            break;
        case C6_SPELL_TYPE_OTHER_FOOTPRINTS_COMPAT:
            inOutMagic->event79CountFootprints += effect->magicStateDelta[5];
            if (effect->magicStateDelta[5] > 0) {
                inOutMagic->magicFootprintsActive = 1;
            }
            break;
        case C8_SPELL_TYPE_OTHER_FIRESHIELD_COMPAT:
            inOutMagic->event78CountFireShield += effect->magicStateDelta[5];
            break;
        default:
            break;
    }
    return 1;
}

/* ==========================================================
 *  Group D — Magic-defender adjustments (F0761-F0762)
 *  Resolves Phase 13 REVIEW #1 (fire/magic/psychic defence paths).
 * ========================================================== */

int F0761_MAGIC_GetDefenderMagicalAdjustedAttack_Compat(
    int attackType,
    const struct CombatantChampionSnapshot_Compat* defender,
    const struct MagicState_Compat* magic,
    int rawAttack,
    int* outAdjusted)
{
    int atk;
    int tmp;

    if (outAdjusted == 0 || defender == 0 || magic == 0) {
        return 0;
    }
    atk = rawAttack;
    tmp = atk;

    switch (attackType) {
        case COMBAT_ATTACK_MAGIC:
            if (F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
                    defender->statisticAntimagic, 255, atk, &tmp)) {
                atk = tmp;
            }
            atk -= magic->spellShieldDefense;
            break;

        case COMBAT_ATTACK_FIRE:
            if (F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
                    defender->statisticAntifire, 255, atk, &tmp)) {
                atk = tmp;
            }
            atk -= magic->fireShieldDefense;
            break;

        default:
            /* Pass-through for non-magical attack types — caller
             * should use the phase-13 helpers for vitality / blunt /
             * sharp / lightning. */
            break;
    }

    if (atk < 0) atk = 0;
    *outAdjusted = atk;
    return 1;
}

int F0762_MAGIC_GetDefenderPsychicAdjustedAttack_Compat(
    const struct CombatantChampionSnapshot_Compat* defender,
    int statisticWisdom,
    int rawAttack,
    int* outAdjusted)
{
    int wisdomFactor;
    int tmp;

    (void)defender;

    if (outAdjusted == 0) {
        return 0;
    }

    /* CHAMPION.C:1853..1875: wisdomFactor = 115 - wisdom. */
    wisdomFactor = 115 - statisticWisdom;
    if (wisdomFactor <= 0) {
        *outAdjusted = 0;
        return 1;
    }

    /* Scaled product with shift 6, mirroring F0030_MAIN_GetScaledProduct.
     * Plan §4.8 requests `(attack * factor + 32) >> 6` (half-up round). */
    tmp = ((rawAttack * wisdomFactor) + 32) >> 6;
    if (tmp < 0) tmp = 0;
    *outAdjusted = tmp;
    return 1;
}

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
    struct TimelineEvent_Compat* outEvent)
{
    if (effect == 0 || outEvent == 0) {
        return 0;
    }
    memset(outEvent, 0, sizeof(*outEvent));

    if (effect->followupEventKind == TIMELINE_EVENT_INVALID) {
        outEvent->kind = TIMELINE_EVENT_INVALID;
        return 0;
    }

    outEvent->kind = effect->followupEventKind;
    outEvent->fireAtTick = nowTick + (uint32_t)effect->durationTicks;
    outEvent->mapIndex = partyMapIndex;
    outEvent->mapX = partyMapX;
    outEvent->mapY = partyMapY;
    outEvent->cell = partyCell;
    outEvent->aux0 = effect->followupEventAux0;
    outEvent->aux1 = effect->followupEventAux1;
    outEvent->aux2 = effect->magicStateDelta[0];
    outEvent->aux3 = effect->magicStateDelta[1];
    outEvent->aux4 = effect->magicStateDelta[2];
    return 1;
}

/* ==========================================================
 *  Group F — Serialisation (F0764-F0769)
 * ========================================================== */

int F0764_MAGIC_RuneSequenceSerialize_Compat(
    const struct RuneSequence_Compat* seq,
    unsigned char* outBuf,
    int outBufSize)
{
    if (seq == 0 || outBuf == 0) return 0;
    if (outBufSize < RUNE_SEQUENCE_SERIALIZED_SIZE) return 0;
    write_i32_le(outBuf +  0, seq->runeCount);
    write_i32_le(outBuf +  4, seq->runes[0]);
    write_i32_le(outBuf +  8, seq->runes[1]);
    write_i32_le(outBuf + 12, seq->runes[2]);
    write_i32_le(outBuf + 16, seq->runes[3]);
    return 1;
}

int F0765_MAGIC_RuneSequenceDeserialize_Compat(
    struct RuneSequence_Compat* seq,
    const unsigned char* buf,
    int bufSize)
{
    if (seq == 0 || buf == 0) return 0;
    if (bufSize < RUNE_SEQUENCE_SERIALIZED_SIZE) return 0;
    seq->runeCount = read_i32_le(buf +  0);
    seq->runes[0]  = read_i32_le(buf +  4);
    seq->runes[1]  = read_i32_le(buf +  8);
    seq->runes[2]  = read_i32_le(buf + 12);
    seq->runes[3]  = read_i32_le(buf + 16);
    return 1;
}

int F0766a_MAGIC_SpellCastRequestSerialize_Compat(
    const struct SpellCastRequest_Compat* req,
    unsigned char* outBuf,
    int outBufSize)
{
    if (req == 0 || outBuf == 0) return 0;
    if (outBufSize < SPELL_CAST_REQUEST_SERIALIZED_SIZE) return 0;
    write_i32_le(outBuf +  0, req->championIndex);
    write_i32_le(outBuf +  4, req->currentMana);
    write_i32_le(outBuf +  8, req->maximumMana);
    write_i32_le(outBuf + 12, req->skillLevelForSpell);
    write_i32_le(outBuf + 16, req->statisticWisdom);
    write_i32_le(outBuf + 20, req->luckCurrent);
    write_i32_le(outBuf + 24, req->partyDirection);
    write_i32_le(outBuf + 28, req->partyMapIndex);
    write_i32_le(outBuf + 32, req->partyMapX);
    write_i32_le(outBuf + 36, req->partyMapY);
    write_i32_le(outBuf + 40, req->hasEmptyFlaskInHand);
    write_i32_le(outBuf + 44, req->hasMagicMapInHand);
    write_i32_le(outBuf + 48, req->gameTimeTicksLow);
    write_i32_le(outBuf + 52, req->spellTableIndex);
    write_i32_le(outBuf + 56, req->rawSymbolsPacked);
    write_i32_le(outBuf + 60, req->reserved);
    return 1;
}

int F0766b_MAGIC_SpellCastRequestDeserialize_Compat(
    struct SpellCastRequest_Compat* req,
    const unsigned char* buf,
    int bufSize)
{
    if (req == 0 || buf == 0) return 0;
    if (bufSize < SPELL_CAST_REQUEST_SERIALIZED_SIZE) return 0;
    req->championIndex       = read_i32_le(buf +  0);
    req->currentMana         = read_i32_le(buf +  4);
    req->maximumMana         = read_i32_le(buf +  8);
    req->skillLevelForSpell  = read_i32_le(buf + 12);
    req->statisticWisdom     = read_i32_le(buf + 16);
    req->luckCurrent         = read_i32_le(buf + 20);
    req->partyDirection      = read_i32_le(buf + 24);
    req->partyMapIndex       = read_i32_le(buf + 28);
    req->partyMapX           = read_i32_le(buf + 32);
    req->partyMapY           = read_i32_le(buf + 36);
    req->hasEmptyFlaskInHand = read_i32_le(buf + 40);
    req->hasMagicMapInHand   = read_i32_le(buf + 44);
    req->gameTimeTicksLow    = read_i32_le(buf + 48);
    req->spellTableIndex     = read_i32_le(buf + 52);
    req->rawSymbolsPacked    = read_i32_le(buf + 56);
    req->reserved            = read_i32_le(buf + 60);
    return 1;
}

int F0767a_MAGIC_SpellEffectSerialize_Compat(
    const struct SpellEffect_Compat* effect,
    unsigned char* outBuf,
    int outBufSize)
{
    int i;
    if (effect == 0 || outBuf == 0) return 0;
    if (outBufSize < SPELL_EFFECT_SERIALIZED_SIZE) return 0;
    write_i32_le(outBuf +  0, effect->castResult);
    write_i32_le(outBuf +  4, effect->failureReason);
    write_i32_le(outBuf +  8, effect->spellKind);
    write_i32_le(outBuf + 12, effect->spellType);
    write_i32_le(outBuf + 16, effect->powerOrdinal);
    write_i32_le(outBuf + 20, effect->manaSpent);
    write_i32_le(outBuf + 24, effect->impactAttack);
    write_i32_le(outBuf + 28, effect->kineticEnergy);
    write_i32_le(outBuf + 32, effect->durationTicks);
    for (i = 0; i < 6; i++) {
        write_i32_le(outBuf + 36 + (i * 4), effect->magicStateDelta[i]);
    }
    write_i32_le(outBuf + 60, effect->followupEventKind);
    write_i32_le(outBuf + 64, effect->followupEventAux0);
    write_i32_le(outBuf + 68, effect->followupEventAux1);
    write_i32_le(outBuf + 72, effect->poisonAttackPending);
    write_i32_le(outBuf + 76, effect->wakeFromRest);
    write_i32_le(outBuf + 80, effect->rngCallCount);
    return 1;
}

int F0767b_MAGIC_SpellEffectDeserialize_Compat(
    struct SpellEffect_Compat* effect,
    const unsigned char* buf,
    int bufSize)
{
    int i;
    if (effect == 0 || buf == 0) return 0;
    if (bufSize < SPELL_EFFECT_SERIALIZED_SIZE) return 0;
    effect->castResult          = read_i32_le(buf +  0);
    effect->failureReason       = read_i32_le(buf +  4);
    effect->spellKind           = read_i32_le(buf +  8);
    effect->spellType           = read_i32_le(buf + 12);
    effect->powerOrdinal        = read_i32_le(buf + 16);
    effect->manaSpent           = read_i32_le(buf + 20);
    effect->impactAttack        = read_i32_le(buf + 24);
    effect->kineticEnergy       = read_i32_le(buf + 28);
    effect->durationTicks       = read_i32_le(buf + 32);
    for (i = 0; i < 6; i++) {
        effect->magicStateDelta[i] = read_i32_le(buf + 36 + (i * 4));
    }
    effect->followupEventKind   = read_i32_le(buf + 60);
    effect->followupEventAux0   = read_i32_le(buf + 64);
    effect->followupEventAux1   = read_i32_le(buf + 68);
    effect->poisonAttackPending = read_i32_le(buf + 72);
    effect->wakeFromRest        = read_i32_le(buf + 76);
    effect->rngCallCount        = read_i32_le(buf + 80);
    return 1;
}

int F0768a_MAGIC_MagicStateSerialize_Compat(
    const struct MagicState_Compat* magic,
    unsigned char* outBuf,
    int outBufSize)
{
    if (magic == 0 || outBuf == 0) return 0;
    if (outBufSize < MAGIC_STATE_SERIALIZED_SIZE) return 0;
    write_i32_le(outBuf +  0, magic->spellShieldDefense);
    write_i32_le(outBuf +  4, magic->fireShieldDefense);
    write_i32_le(outBuf +  8, magic->partyShieldDefense);
    write_i32_le(outBuf + 12, magic->magicalLightAmount);
    write_u32_le(outBuf + 16, magic->lightDecayFireAtTick);
    write_i32_le(outBuf + 20, magic->event70LightDirection);
    write_i32_le(outBuf + 24, magic->event71CountInvisibility);
    write_i32_le(outBuf + 28, magic->event73CountThievesEye);
    write_i32_le(outBuf + 32, magic->event74CountPartyShield);
    write_i32_le(outBuf + 36, magic->event77CountSpellShield);
    write_i32_le(outBuf + 40, magic->event78CountFireShield);
    write_i32_le(outBuf + 44, magic->event79CountFootprints);
    write_i32_le(outBuf + 48, magic->freezeLifeTicks);
    write_i32_le(outBuf + 52, magic->magicFootprintsActive);
    write_i32_le(outBuf + 56, magic->luckCurrent);
    write_i32_le(outBuf + 60, magic->curseMask);
    write_i32_le(outBuf + 64, magic->reserved0);
    write_i32_le(outBuf + 68, magic->reserved1);
    return 1;
}

int F0768b_MAGIC_MagicStateDeserialize_Compat(
    struct MagicState_Compat* magic,
    const unsigned char* buf,
    int bufSize)
{
    if (magic == 0 || buf == 0) return 0;
    if (bufSize < MAGIC_STATE_SERIALIZED_SIZE) return 0;
    magic->spellShieldDefense       = read_i32_le(buf +  0);
    magic->fireShieldDefense        = read_i32_le(buf +  4);
    magic->partyShieldDefense       = read_i32_le(buf +  8);
    magic->magicalLightAmount       = read_i32_le(buf + 12);
    magic->lightDecayFireAtTick     = read_u32_le(buf + 16);
    magic->event70LightDirection    = read_i32_le(buf + 20);
    magic->event71CountInvisibility = read_i32_le(buf + 24);
    magic->event73CountThievesEye   = read_i32_le(buf + 28);
    magic->event74CountPartyShield  = read_i32_le(buf + 32);
    magic->event77CountSpellShield  = read_i32_le(buf + 36);
    magic->event78CountFireShield   = read_i32_le(buf + 40);
    magic->event79CountFootprints   = read_i32_le(buf + 44);
    magic->freezeLifeTicks          = read_i32_le(buf + 48);
    magic->magicFootprintsActive    = read_i32_le(buf + 52);
    magic->luckCurrent              = read_i32_le(buf + 56);
    magic->curseMask                = read_i32_le(buf + 60);
    magic->reserved0                = read_i32_le(buf + 64);
    magic->reserved1                = read_i32_le(buf + 68);
    return 1;
}

int F0769a_MAGIC_SpellDefinitionSerialize_Compat(
    const struct SpellDefinition_Compat* spell,
    unsigned char* outBuf,
    int outBufSize)
{
    if (spell == 0 || outBuf == 0) return 0;
    if (outBufSize < SPELL_DEFINITION_SERIALIZED_SIZE) return 0;
    write_i32_le(outBuf +  0, spell->symbolsPacked);
    write_i32_le(outBuf +  4, spell->baseRequiredSkillLevel);
    write_i32_le(outBuf +  8, spell->skillIndex);
    write_i32_le(outBuf + 12, spell->attributes);
    write_i32_le(outBuf + 16, spell->kind);
    write_i32_le(outBuf + 20, spell->type);
    write_i32_le(outBuf + 24, spell->disabledTicks);
    return 1;
}

int F0769b_MAGIC_SpellDefinitionDeserialize_Compat(
    struct SpellDefinition_Compat* spell,
    const unsigned char* buf,
    int bufSize)
{
    if (spell == 0 || buf == 0) return 0;
    if (bufSize < SPELL_DEFINITION_SERIALIZED_SIZE) return 0;
    spell->symbolsPacked          = read_i32_le(buf +  0);
    spell->baseRequiredSkillLevel = read_i32_le(buf +  4);
    spell->skillIndex             = read_i32_le(buf +  8);
    spell->attributes             = read_i32_le(buf + 12);
    spell->kind                   = read_i32_le(buf + 16);
    spell->type                   = read_i32_le(buf + 20);
    spell->disabledTicks          = read_i32_le(buf + 24);
    return 1;
}
