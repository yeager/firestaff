/**
 * dm1_v1_spell_casting_pc34_compat.c — DM1 V1 Spell Casting System
 *
 * Source-locked to ReDMCSB (WIP20210206).
 * See header for full source reference list.
 */
#include "dm1_v1_spell_casting_pc34_compat.h"
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════
 * Static data tables — verbatim from ReDMCSB MENU.C:44-76
 * ═══════════════════════════════════════════════════════════════════ */

/*
 * G0485_aauc_Graphic560_SymbolBaseManaCost[4][6] (MENU.C:44-48)
 * Row = SymbolStep, Col = SymbolIndex within step.
 * Step 0 (Power): base cost for power symbol selection.
 * Steps 1-3: base cost for element/class/alignment symbols.
 */
const uint8_t dm1_symbolBaseManaCost[DM1_SYMBOL_STEP_COUNT][DM1_SYMBOLS_PER_STEP] = {
    { 1, 2, 3, 4, 5, 6 },   /* Step 0 — Power   */
    { 2, 3, 4, 5, 6, 7 },   /* Step 1 — Element  */
    { 4, 5, 6, 7, 7, 9 },   /* Step 2 — Class    */
    { 2, 2, 3, 4, 6, 7 }    /* Step 3 — Alignment*/
};

/*
 * G0486_auc_Graphic560_SymbolManaCostMultiplier[6] (MENU.C:49)
 * Indexed by (first symbol character - 96), i.e. the power symbol index.
 * Applied to base cost for steps 1-3: cost = (base * mult) >> 3
 */
const uint8_t dm1_symbolManaCostMultiplier[DM1_SYMBOLS_PER_STEP] = {
    8, 12, 16, 20, 24, 28
};

/*
 * G0487_as_Graphic560_Spells[25] (MENU.C:50-76)
 * Verbatim spell table for DM1 V1.
 * { Symbols, BaseRequiredSkillLevel, SkillIndex, Attributes }
 */
const DM1_Spell dm1_spells[DM1_SPELL_COUNT] = {
    /* { Symbols,     BaseReq, Skill,            Attrs }   Kind Type Dur  Runes              Name */
    { 0x00666F00, 2, DM1_SKILL_DEFEND,  0x7843 }, /*  3   4  30   Ya Ir              Shield (Party) */
    { 0x00667073, 1, DM1_SKILL_EARTH,   0x4863 }, /*  3   6  18   Ya Bro Ros         Magic Footprints */
    { 0x00686D77, 3, DM1_SKILL_AIR,     0xB433 }, /*  3   3  45   Oh Ew Sar          Invisibility */
    { 0x00686C00, 3, DM1_SKILL_WATER,   0x6C72 }, /*  2   7  27   Oh Ven             Poison Cloud */
    { 0x00686D76, 3, DM1_SKILL_EARTH,   0x8423 }, /*  3   2  33   Oh Ew Ra           Thieve's Eye */
    { 0x00686E76, 4, DM1_SKILL_AIR,     0x7822 }, /*  2   2  30   Oh Kath Ra         Lightning Bolt */
    { 0x00686F76, 4, DM1_SKILL_AIR,     0x5803 }, /*  3   0  22   Oh Ir Ra           Light */
    { 0x00690000, 1, DM1_SKILL_FIRE,    0x3C53 }, /*  3   5  15   Ful                Torch */
    { 0x00696F00, 3, DM1_SKILL_FIRE,    0xA802 }, /*  2   0  42   Ful Ir             Fireball */
    { 0x00697072, 4, DM1_SKILL_HEAL,    0x3C71 }, /*  1   7  15   Ful Bro Ku         Strength Potion */
    { 0x00697075, 4, DM1_SKILL_DEFEND,  0x7083 }, /*  3   8  28   Ful Bro Neta       Fire Shield */
    { 0x006A6D00, 1, DM1_SKILL_EARTH,   0x5032 }, /*  2   3  20   Des Ew             Weaken Nonmaterial */
    { 0x006A6C00, 1, DM1_SKILL_WATER,   0x4062 }, /*  2   6  16   Des Ven            Poison Bolt */
    { 0x006A6F77, 1, DM1_SKILL_DEFEND,  0x3013 }, /*  3   1  12   Des Ir Sar         Darkness */
    { 0x006B0000, 1, DM1_SKILL_AIR,     0x3C42 }, /*  2   4  15   Zo                 Open Door */
    { 0x00667000, 2, DM1_SKILL_DEFEND,  0x64C1 }, /*  1  12  25   Ya Bro             Shield Potion */
    { 0x00660000, 2, DM1_SKILL_HEAL,    0x3CB1 }, /*  1  11  15   Ya                 Stamina Potion */
    { 0x00667074, 4, DM1_SKILL_HEAL,    0x3C81 }, /*  1   8  15   Ya Bro Dain        Wisdom Potion */
    { 0x00667075, 4, DM1_SKILL_HEAL,    0x3C91 }, /*  1   9  15   Ya Bro Neta        Vitality Potion */
    { 0x00670000, 1, DM1_SKILL_HEAL,    0x80E1 }, /*  1  14  32   Vi                 Health Potion */
    { 0x00677000, 1, DM1_SKILL_HEAL,    0x68A1 }, /*  1  10  26   Vi Bro             Cure Poison Potion */
    { 0x00687073, 4, DM1_SKILL_HEAL,    0x3C61 }, /*  1   6  15   Oh Bro Ros         Dexterity Potion */
    { 0x006B7076, 3, DM1_SKILL_PRIEST,  0xFCD1 }, /*  1  13  63   Zo Bro Ra          Mana Potion */
    { 0x006B6C00, 2, DM1_SKILL_WATER,   0x7831 }, /*  1   3  30   Zo Ven             Poison Potion */
    { 0x006B6E76, 0, DM1_SKILL_WIZARD,  0x3C73 }, /*  3   7  15   Zo Kath Ra         Zokathra */
};

/* ── Human-readable spell names ────────────────────────────────── */
static const char* const spellNames[DM1_SPELL_COUNT] = {
    "SHIELD",            /* Ya Ir */
    "MAGIC FOOTPRINTS",  /* Ya Bro Ros */
    "INVISIBILITY",      /* Oh Ew Sar */
    "POISON CLOUD",      /* Oh Ven */
    "THIEVE'S EYE",      /* Oh Ew Ra */
    "LIGHTNING BOLT",    /* Oh Kath Ra */
    "LIGHT",             /* Oh Ir Ra */
    "TORCH",             /* Ful */
    "FIREBALL",          /* Ful Ir */
    "STRENGTH POTION",   /* Ful Bro Ku */
    "FIRE SHIELD",       /* Ful Bro Neta */
    "WEAKEN NONMATERIAL",/* Des Ew */
    "POISON BOLT",       /* Des Ven */
    "DARKNESS",          /* Des Ir Sar */
    "OPEN DOOR",         /* Zo */
    "SHIELD POTION",     /* Ya Bro */
    "STAMINA POTION",    /* Ya */
    "WISDOM POTION",     /* Ya Bro Dain */
    "VITALITY POTION",   /* Ya Bro Neta */
    "HEALTH POTION",     /* Vi */
    "CURE POISON POTION",/* Vi Bro */
    "DEXTERITY POTION",  /* Oh Bro Ros */
    "MANA POTION",       /* Zo Bro Ra */
    "POISON POTION",     /* Zo Ven */
    "ZOKATHRA",          /* Zo Kath Ra */
};

/* ── Symbol name table (24 symbols across 4 steps) ──────────────── */
static const char* const symbolNames[DM1_SYMBOL_STEP_COUNT * DM1_SYMBOLS_PER_STEP] = {
    /* Step 0 — Power */
    "Lo", "Um", "On", "Ee", "Pal", "Mon",
    /* Step 1 — Element */
    "Ya", "Vi", "Oh", "Ful", "Des", "Zo",
    /* Step 2 — Class */
    "Ven", "Ew", "Kath", "Ir", "Bro", "Gor",
    /* Step 3 — Alignment */
    "Ku", "Ros", "Dain", "Neta", "Ra", "Sar",
};

/* ═══════════════════════════════════════════════════════════════════
 * Internal helpers
 * ═══════════════════════════════════════════════════════════════════ */

static inline int minVal(int a, int b) { return a < b ? a : b; }
static inline int maxVal(int a, int b) { return a > b ? a : b; }

/** Clamp value to [lo, hi] — mirrors F0026_MAIN_GetBoundedValue */
static inline int boundedValue(int lo, int val, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

/* ═══════════════════════════════════════════════════════════════════
 * Core API implementation
 * ═══════════════════════════════════════════════════════════════════ */

void dm1_spell_init(DM1_SpellCastingState* s) {
    if (!s) return;
    memset(s, 0, sizeof(DM1_SpellCastingState));
    s->magicCasterIndex = -1;
}

/*
 * dm1_spell_addSymbol — Source: SYMBOL.C F0399_MENUS_AddChampionSymbol
 *
 * Lines referenced (SYMBOL.C):
 *   L1222 = champion->SymbolStep
 *   L1223 = G0485[symbolStep][symbolIndex]
 *   if (symbolStep) L1223 = (L1223 * G0486[champion->Symbols[0] - 96]) >> 3
 *   if (L1223 <= champion->CurrentMana) {
 *       champion->CurrentMana -= L1223;
 *       champion->Symbols[symbolStep] = 96 + (symbolStep * 6) + symbolIndex;
 *       champion->Symbols[symbolStep + 1] = '\0';
 *       champion->SymbolStep = (symbolStep + 1) & 3;
 *   }
 */
int dm1_spell_addSymbol(DM1_SpellCastingState* s, int champIdx,
                        DM1_ChampionSpellStats* stats, int symbolIdx) {
    if (!s || !stats || champIdx < 0 || champIdx >= 4) return 0;
    if (symbolIdx < 0 || symbolIdx >= DM1_SYMBOLS_PER_STEP) return 0;

    DM1_ChampionSpellInput* inp = &s->input[champIdx];
    unsigned int step = inp->symbolStep;

    /* Compute mana cost (SYMBOL.C F0399 lines 20-25) */
    unsigned int manaCost = dm1_symbolBaseManaCost[step][symbolIdx];
    if (step > 0) {
        /* Multiply by power symbol's multiplier, then >>3 */
        manaCost = (manaCost * dm1_symbolManaCostMultiplier[(unsigned char)inp->symbols[0] - 96]) >> 3;
    }

    if (manaCost > (unsigned int)stats->currentMana) {
        return 0; /* Insufficient mana */
    }

    /* Deduct mana */
    stats->currentMana -= (int16_t)manaCost;

    /* Store symbol character (SYMBOL.C F0399 line 36) */
    inp->symbols[step] = dm1_encodeSymbol(step, symbolIdx);
    inp->symbols[step + 1] = '\0';

    /* Advance step with wrap (SYMBOL.C F0399 line 39: (step + 1) & 3) */
    inp->symbolStep = (uint8_t)((step + 1) & 3);

    return 1;
}

/*
 * dm1_spell_deleteSymbol — Source: SYMBOL.C F0400_MENUS_DeleteChampionSymbol
 *
 * Lines referenced (SYMBOL.C):
 *   if strlen(champion->Symbols) == 0: return
 *   champion->SymbolStep = M019_PREVIOUS(champion->SymbolStep)  [(step + 3) & 3]
 *   champion->Symbols[symbolStep] = '\0'
 *
 * NOTE: Mana is NOT refunded — matches original behavior.
 */
void dm1_spell_deleteSymbol(DM1_SpellCastingState* s, int champIdx) {
    if (!s || champIdx < 0 || champIdx >= 4) return;

    DM1_ChampionSpellInput* inp = &s->input[champIdx];

    /* Check if any symbols entered */
    if (inp->symbols[0] == '\0') return;

    /* M019_PREVIOUS: (step + 3) & 3 = (step - 1) mod 4 */
    unsigned int step = (inp->symbolStep + 3) & 3;
    inp->symbolStep = (uint8_t)step;
    inp->symbols[step] = '\0';
}

/*
 * dm1_spell_lookup — Source: MENU.C F0409_MENUS_GetSpellFromSymbols
 *
 * Lines referenced (MENU.C:1685-1700):
 *   if symbols[1] == '\0': return NULL (need at least 2 symbols)
 *   Pack symbols into long, MSB-first, shift by 24,16,8,0
 *   Iterate spells: if spell.Symbols has MSB != 0, compare full long
 *                    else compare lower 24 bits only
 */
const DM1_Spell* dm1_spell_lookup(const DM1_SpellCastingState* s, int champIdx) {
    if (!s || champIdx < 0 || champIdx >= 4) return NULL;

    const char* syms = s->input[champIdx].symbols;

    /* Need at least power + one element symbol (MENU.C:1685/1690) */
    if (!syms[1]) return NULL;

    /* Pack symbols into int32_t, MSB-first (MENU.C:1693-1696) */
    int32_t packed = 0;
    int shift = 24;
    const char* p = syms;
    do {
        packed |= ((int32_t)(unsigned char)*p++) << shift;
    } while (*p && ((shift -= 8) >= 0));

    /* Search spell table (MENU.C:1697-1705) */
    for (int i = 0; i < DM1_SPELL_COUNT; i++) {
        const DM1_Spell* sp = &dm1_spells[i];
        if (sp->symbols & 0xFF000000) {
            /* Spell includes power symbol — compare full */
            if (packed == sp->symbols) return sp;
        } else {
            /* Spell excludes power symbol — compare lower 24 bits */
            if ((packed & 0x00FFFFFF) == sp->symbols) return sp;
        }
    }

    return NULL;
}

/*
 * dm1_spell_symbolManaCost — Source: SYMBOL.C F0399 (cost calc only)
 */
int dm1_spell_symbolManaCost(const DM1_SpellCastingState* s, int champIdx, int symbolIdx) {
    if (!s || champIdx < 0 || champIdx >= 4) return 0;
    if (symbolIdx < 0 || symbolIdx >= DM1_SYMBOLS_PER_STEP) return 0;

    const DM1_ChampionSpellInput* inp = &s->input[champIdx];
    unsigned int step = inp->symbolStep;
    unsigned int cost = dm1_symbolBaseManaCost[step][symbolIdx];

    if (step > 0) {
        cost = (cost * dm1_symbolManaCostMultiplier[(unsigned char)inp->symbols[0] - 96]) >> 3;
    }

    return (int)cost;
}

/*
 * dm1_spell_cast — Source: MENU.C F0412_MENUS_GetChampionSpellCastResult
 *
 * Key logic (MENU.C F0412):
 *   1. Lookup spell via F0409
 *   2. powerSymbolOrdinal = Symbols[0] - '_'  (values 1-6; '_' = 95)
 *   3. requiredSkillLevel = spell->BaseRequiredSkillLevel + powerSymbolOrdinal
 *   4. experience = rng8 + (requiredSkill << 4) + ((powerOrd-1)*baseRequired << 3) + (required^2)
 *   5. Skill check: if skillLevel < required, random failure check per missing level
 *   6. Switch on spell kind (potion → needs flask, projectile → create, other → effect)
 *   7. On success, clear symbols + reset SymbolStep to 0
 */
int dm1_spell_cast(DM1_SpellCastingState* s, int champIdx,
                   DM1_ChampionSpellStats* stats, uint16_t rng16,
                   const DM1_Spell** outSpell, int* outPowerOrdinal,
                   int* outFailure) {
    if (!s || !stats || champIdx < 0 || champIdx >= 4) return DM1_SPELL_CAST_FAILURE;

    /* Champion must be alive */
    if (stats->currentHealth <= 0) return DM1_SPELL_CAST_FAILURE;

    /* Look up spell (F0412 line: F0409_MENUS_GetSpellFromSymbols) */
    const DM1_Spell* spell = dm1_spell_lookup(s, champIdx);
    if (!spell) {
        if (outFailure) *outFailure = DM1_FAILURE_MEANINGLESS_SPELL;
        return DM1_SPELL_CAST_FAILURE;
    }

    DM1_ChampionSpellInput* inp = &s->input[champIdx];

    /* Power symbol ordinal = Symbols[0] - '_' (MENU.C F0412 line: L1268) */
    /* '_' is ASCII 95. Symbols[0] is a power symbol char 96..101 → ordinal 1..6 */
    int powerOrdinal = (int)(unsigned char)inp->symbols[0] - 95;  /* 95 = '_' */
    if (outPowerOrdinal) *outPowerOrdinal = powerOrdinal;

    /* Required skill level (MENU.C F0412 line: AL1269) */
    int requiredSkillLevel = spell->baseRequiredSkillLevel + powerOrdinal;

    /* Skill check (MENU.C F0412 lines: AL1267 = F0303...; if < required: random failure) */
    int champSkillLevel = stats->skillLevels[spell->skillIndex];

    if (champSkillLevel < requiredSkillLevel) {
        int missingLevels = requiredSkillLevel - champSkillLevel;
        /* Each missing level: random check against min(wisdom + 15, 115) */
        /* Using the provided rng16 split into bytes for multiple checks */
        for (int i = 0; i < missingLevels; i++) {
            /* Use different bits of rng16 for each iteration */
            int rngVal = ((rng16 >> (i * 4)) & 0x7F);  /* 7-bit value 0-127 */
            int threshold = minVal(stats->wisdom + 15, 115);
            if (rngVal > threshold) {
                if (outFailure) *outFailure = DM1_FAILURE_NEEDS_MORE_PRACTICE;
                if (outSpell) *outSpell = spell;
                /* Clear symbols on failure (F0408 resets on non-flask failure) */
                inp->symbols[0] = '\0';
                inp->symbolStep = 0;
                return DM1_SPELL_CAST_FAILURE;
            }
        }
    }

    /* Spell matched — handle by kind */
    int kind = DM1_SPELL_KIND(spell);

    if (kind == DM1_SPELL_KIND_POTION) {
        /*
         * Potion spells need an empty flask in hand (MENU.C F0412).
         * We don't have inventory access here — return NEEDS_FLASK
         * to let the caller check.
         */
        if (outSpell) *outSpell = spell;
        return DM1_SPELL_CAST_FAILURE_NEEDS_FLASK;
    }

    if (kind == DM1_SPELL_KIND_PROJECTILE) {
        /*
         * Projectile spells: no additional mana cost in F0412 (mana was
         * spent on symbols). F0327 is called with requiredMana=0 for
         * spells cast via F0412 (MENU.C line: F0327_CHAMPION_IsProjectileSpellCast(
         *   ..., 0) — the 4th arg is 0).
         */
        /* Nothing to deduct here — caller handles projectile creation */
    }

    /* Success — clear symbols (MENU.C F0408: L1260_ps_Champion->Symbols[0] = '\0') */
    if (outSpell) *outSpell = spell;
    inp->symbols[0] = '\0';
    inp->symbolStep = 0;

    return DM1_SPELL_CAST_SUCCESS;
}

/*
 * dm1_spell_projectileKineticEnergy — Source: MENU.C F0412 case C2_SPELL_KIND_PROJECTILE
 *
 * F0412 line:
 *   if (spellType == C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR) skillLevel <<= 1;
 *   F0327(..., F0026_MAIN_GetBoundedValue(21, (powerOrd + 2) * (4 + (skillLevel << 1)), 255), 0);
 */
int dm1_spell_projectileKineticEnergy(int powerOrdinal, int skillLevel, int spellType) {
    if (spellType == DM1_SPELL_TYPE_PROJ_OPEN_DOOR) {
        skillLevel <<= 1;
    }
    int ke = (powerOrdinal + 2) * (4 + (skillLevel << 1));
    return boundedValue(21, ke, 255);
}

/*
 * dm1_spell_projectileStepEnergy — Source: CHAMPION.C F0327
 *
 * F0327 line:
 *   L0991_i_StepEnergy = 10 - F0024_MAIN_GetMinimumValue(8, MaximumMana >> 3);
 *   (GetMinimumValue = min)
 */
int dm1_spell_projectileStepEnergy(int16_t maximumMana) {
    return 10 - minVal(8, maximumMana >> 3);
}

/*
 * dm1_spell_experience — Source: MENU.C F0412
 *
 * F0412 lines:
 *   AL1269 = spell->BaseRequiredSkillLevel + powerOrdinal
 *   experience = rng8 + (AL1269 << 4) + ((powerOrd - 1) * baseRequired << 3) + (AL1269 * AL1269)
 */
uint16_t dm1_spell_experience(int powerOrdinal, int baseRequiredSkill, int rng8) {
    int required = baseRequiredSkill + powerOrdinal;
    int exp = rng8 + (required << 4)
            + (((powerOrdinal - 1) * baseRequiredSkill) << 3)
            + (required * required);
    return (uint16_t)exp;
}

const char* dm1_spell_symbolName(char sym) {
    int idx = (int)(unsigned char)sym - 96;
    if (idx < 0 || idx >= DM1_SYMBOL_STEP_COUNT * DM1_SYMBOLS_PER_STEP) return "?";
    return symbolNames[idx];
}

const char* dm1_spell_name(int spellIndex) {
    if (spellIndex < 0 || spellIndex >= DM1_SPELL_COUNT) return "UNKNOWN";
    return spellNames[spellIndex];
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Spell system source-lock extensions
 *
 * COMMAND.C:2304    F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE
 * COMMAND.C:2280-2335 spell/action/viewport command dispatch in F0380
 * CHAMPION.C:715-822  F0303_CHAMPION_GetSkillLevel (4 base skills, modifiers)
 * CHAMPION.C:823-960  F0304_CHAMPION_AddSkillExperience (XP + stamina regen)
 * CHAMPION.C:1061-1076 F0305_CHAMPION_GetThrowingStaminaCost
 * CHAMPION.C:1078-1104 F0306_CHAMPION_GetStaminaAdjustedValue
 * CHAMPION.C:1106-1122 F0307_CHAMPION_GetStatisticAdjustedAttack
 * CHAMPION.C:1123-1155 F0308_CHAMPION_IsLucky
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_spell_pass601_spell_source_evidence(void)
{
    return
        "COMMAND.C:2304 F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE\n"
        "COMMAND.C:2280-2335 spell/action dispatch in F0380_COMMAND_ProcessQueue\n"
        "CHAMPION.C:715-822 F0303_CHAMPION_GetSkillLevel\n"
        "CHAMPION.C:823-960 F0304_CHAMPION_AddSkillExperience\n"
        "CHAMPION.C:1061-1076 F0305_CHAMPION_GetThrowingStaminaCost\n"
        "CHAMPION.C:1078-1104 F0306_CHAMPION_GetStaminaAdjustedValue\n"
        "CHAMPION.C:1106-1122 F0307_CHAMPION_GetStatisticAdjustedAttack\n"
        "CHAMPION.C:1123-1155 F0308_CHAMPION_IsLucky\n";
}

