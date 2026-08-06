/* dm2_v1_spell.c — DM2 V1 Spell System
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKWIN/SkGlobal.cpp:966-1011, SkGlobal.h:37-55
 * SkWinCore.cpp:17521-18174 (CAST_SPELL_PLAYER, ADD_RUNE_TO_TAIL)
 *
 * 34 spells in fixed mode, 255 in extended GDAT mode.
 * Per-rune mana cost; cooldown after casting; skill decay on failure.
 * New DM2 spells: Spell Reflector, Attack/Guard/U-Haul Minion, Push, Pull.
 */

#include "dm2_v1_spell.h"
#include <string.h>

/* ── dSpellsTable — 34 spell definitions ─────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.cpp:966-1011 (dSpellsTable)
 * Structure: {runes[6], type, difficulty, requiredSkill, power, name}
 * Runes encoded as symbol index values; rune_count = position of first 0xFF or 6.
 *
 * Spell index 0-33 fixed mode.
 * Extended mode: MAXSPELL_CUSTOM=255 via GDAT.
 *
 * Rune symbol indices (DM2_RUNE_*):
 *   0=OH, 1=IR, 2=RA, 3=DES, 4=SAR, 5=YA, 6=EW, 7=FUL,
 *   8=BRO, 9=NETA, 10=KATH, 11=KU, 12=ROS, 13=VEN, 14=ZO, 15=DAIN
 *
 * DM2-new spells marked with "(DM2 new)" comment. */

/* Exact dSpellsTable bytes, in source order.  `key` has no power byte:
 * FIND_SPELL_BY_RUNES compares its low 24 bits to the live power+tail key.
 * Source: SKWIN/SkGlobal.cpp:968-1007, MkssymVal in SkGlobal.h. */
static const DM2_V1_SpellRecord g_spell_table[DM2_MAX_SPELL_ORIGINAL] = {
    {0x00686f76u,0x04,0x11,0x2c03},{0x006a6f77u,0x01,0x0f,0x1813},
    {0x00666f74u,0x04,0x0f,0x3823},{0x00686d77u,0x03,0x11,0x5833},
    {0x00666f00u,0x02,0x0f,0x3c43},{0x00690000u,0x01,0x10,0x1c53},
    {0x00686d74u,0x02,0x02,0x3863},{0x00686d73u,0x02,0x02,0x3873},
    {0x00697075u,0x04,0x0f,0x3883},{0x00686d75u,0x02,0x02,0x3893},
    {0x00686d72u,0x02,0x02,0x38a3},{0x00686f73u,0x04,0x03,0x78b3},
    {0x006b7073u,0x03,0x02,0x78e3},{0x00666d00u,0x00,0x03,0x04f3},
    {0x00686c00u,0x03,0x13,0x4072},{0x00686e76u,0x04,0x11,0x3c22},
    {0x00696f00u,0x03,0x10,0x5402},{0x00697072u,0x04,0x0d,0x1c71},
    {0x006a6d00u,0x01,0x12,0x2832},{0x006a6c00u,0x01,0x13,0x2062},
    {0x006b0000u,0x01,0x11,0x1c42},{0x00667000u,0x02,0x0f,0x30c1},
    {0x00660000u,0x02,0x0d,0x1cb1},{0x00667074u,0x04,0x0d,0x1c81},
    {0x00667075u,0x04,0x0d,0x1c91},{0x00670000u,0x01,0x0d,0x40e1},
    {0x00677000u,0x01,0x0d,0x34a1},{0x00687073u,0x04,0x0d,0x1c61},
    {0x006b7076u,0x03,0x02,0x80d1},{0x006b6d72u,0x06,0x03,0x7b14},
    {0x006b6d75u,0x04,0x0f,0x3f44},{0x006b6d73u,0x05,0x02,0x3354},
    {0x00686e72u,0x02,0x03,0x5892},{0x00686e73u,0x02,0x03,0x58a2},
};
/* clang-format on */

const DM2_SpellDefinition *dm2_v1_spell_get(int spell_index) {
    static DM2_SpellDefinition out;
    const DM2_V1_SpellRecord *e;
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return NULL;
    e = &g_spell_table[spell_index];
    memset(&out, 0, sizeof(out));
    out.rune_count = (e->key & 0xffu) ? 3u : ((e->key & 0xff00u) ? 2u : 1u);
    out.rune_symbols[0] = (uint8_t)(e->key >> 16);
    out.rune_symbols[1] = (uint8_t)(e->key >> 8);
    out.rune_symbols[2] = (uint8_t)e->key;
    out.spell_type = (uint8_t)(e->w6 & 0x0fu);
    out.difficulty = e->difficulty;
    out.required_skill = e->skill;
    return &out;
}

const DM2_V1_SpellRecord *dm2_v1_spell_source_record(int spell_index)
{
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return NULL;
    return &g_spell_table[spell_index];
}

int dm2_v1_spell_count(void) {
    return DM2_MAX_SPELL_ORIGINAL;
}

int dm2_v1_spell_type(int spell_index) {
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return -1;
    return (int)(g_spell_table[spell_index].w6 & 0x0fu);
}

int dm2_v1_spell_resolves_object_effect(int spell_index, int effect_id) {
    (void)spell_index;
    (void)effect_id;
    /* The old index-to-effect map was authored in Firestaff, not decoded
     * from the DB object definitions used by CAST_SPELL_PLAYER. */
    return DM2_OBJECT_EFFECT_NONE;
}

const char *dm2_v1_spell_name(int spell_index) {
    (void)spell_index;
    /* dSpellsTable contains only runes and mechanics.  The English names in
     * SKProject/SkGlobal.cpp are source comments; they are not retail text.
     * QUERY_GDAT_TEXT(SPELL_DEF, ..., 0x18) belongs solely to the extended
     * mode loader, so fixed-mode UI must stay textless until it has a real
     * text owner. */
    return NULL;
}

const char *dm2_v1_spell_source_evidence(void) {
    return
        "DM2 V1 Spell System — Phase 6 source-lock\n"
        "ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: skproject/SKWIN/SkGlobal.cpp:966-1011 (dSpellsTable, 34 spells)\n"
        "Source: skproject/SKWIN/SkGlobal.h:37-55 (MAXSPELL_ORIGINAL=34, MAXSPELL_CUSTOM=255)\n"
        "Source: skproject/SKWIN/SkGlobal.h:50-53 (SPELL_TYPE_POTION/MISSILE/GENERAL/SUMMON)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER: cast chance, cooldown)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:18159-18174 (ADD_RUNE_TO_TAIL: per-rune mana cost)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:27038-27096 (spell→OBJECT_EFFECT mapping)\n"
        "DM2-new spells (not in DM1): Spell Reflector(12), AttackMinion(29), GuardMinion(30), U-HaulMinion(31), Push(32), Pull(33)\n"
        "DM2-spells removed from DM1: MagicFootprints, Petrify, RestoreHealth, SeeThroughWalls, ZoKathRa\n";
}

/* ── Phase 4 expansion: spell casting mechanics ────────────────────────
 *
 * Source: skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 *
 * Cast chance formula:
 *   bp08 = spell->difficulty + spell->power
 *   bp06 = champion->wizard_ability
 *   bp0c = (bp06 + 15) - bp08
 *   if bp0c <= 0: cast fails (skill decay penalty proportional to bp0c)
 *   else: cast succeeds with mana_per_rune * (rune_count - 1) deducted
 *         (POWER rune at index 0 is consumed at no mana cost)
 *   cooldown: 0x08 ticks after successful cast (SkWinCore.cpp:17623)
 *
 * Per-rune mana cost (SkWinCore.cpp:18159-18174): mana is deducted
 * when the rune is added to the spell tail, NOT at cast time.  The
 * cast-time cost is therefore zero — the mana has already been
 * consumed.  We return the cumulative cost (rune_count * mana_per_rune)
 * for accounting purposes.
 *
 * DM2 differences from DM1:
 *   - DM2 spells cost mana per-rune (DM1: flat cost at cast time)
 *   - DM2 spells have 0x08-tick hand cooldown on success (DM1: variable)
 *   - DM2 spells apply skill decay on failure: -1 per failed cast
 */

/* Default hand cooldown after successful cast.
 * Source: skproject/SKWIN/SkWinCore.cpp:17623 (bp0e = 0x08) */
#define DM2_SPELL_COOLDOWN_TICKS 0x08

/* Skill decay penalty on failed cast.
 * Source: skproject/SKWIN/SkWinCore.cpp:17585 (skill decay = -1 per fail) */
#define DM2_SPELL_SKILL_DECAY_ON_FAIL 1

/* POWER rune is consumed at no mana cost (first rune in sequence).
 * Source: skproject/SKWIN/SkWinCore.cpp:17550-17565 (POWER rune = 0 cost). */
#define DM2_SPELL_POWER_RUNE_COST 0

/* ── Validate rune sequence ──────────────────────────────────── */
int dm2_v1_spell_validate_runes(int spell_index,
    const uint8_t *rune_sequence, int rune_count)
{
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return 0;
    if (!rune_sequence) return 0;
    if (rune_count < 2 || rune_count > 4) return 0;
    return dm2_v1_spell_find_by_runes(&g_spell_table[spell_index], 1,
                                      rune_sequence) == 0;
}

/* ── Mana cost (excludes POWER rune at index 0) ────────────────────
 * Returns (rune_count - 1) * mana_per_rune — the cumulative cost
 * already deducted when the runes were added to the spell tail. */
int dm2_v1_spell_mana_cost(int spell_index)
{
    (void)spell_index;
    return -1;
}

/* ── Compute cast chance ───────────────────────────────────── */
int dm2_v1_spell_compute_chance(int spell_index, int wizard_ability)
{
    (void)spell_index;
    (void)wizard_ability;
    return -1;
}

/* ── Can-cast pre-check ───────────────────────────────────── */
int dm2_v1_spell_can_cast(int spell_index,
    int wizard_ability, int current_mana)
{
    (void)spell_index;
    (void)wizard_ability;
    (void)current_mana;
    return 0;
}

/* ── Cast attempt ─────────────────────────────────────────── */
DM2_SpellCastResult dm2_v1_spell_cast_attempt(int spell_index,
    int wizard_ability, int current_mana)
{
    DM2_SpellCastResult r;
    memset(&r, 0, sizeof(r));
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) {
        r.effective_chance = -1;
        return r;
    }
    (void)wizard_ability;
    (void)current_mana;
    r.effective_chance = -1;
    return r;
}
/* ── DM2-007: source rune-key lookup ───────────────────────────────────
 * skproject/SKULLWIN/c_events.cpp:2211-2264 (DM2_FIND_SPELL_BY_RUNES) */

uint32_t dm2_v1_spell_pack_query_key(const uint8_t *runes)
{
    uint32_t key = 0u;
    int shift = 0x18;

    if (!runes) return 0u;
    /* c_events.cpp:2220-2234 — the caller guarantees a non-empty string
     * (rune[1] != 0 check happens before the call in the source); pack
     * up to four rune bytes, stopping after the byte whose successor is
     * the zero terminator. */
    if (runes[1] == 0) {
        /* Single-rune tail: the source returns NULL before packing. */
        return 0u;
    }
    for (;;) {
        uint8_t b = *runes++;
        key |= (uint32_t)b << shift;
        if (*runes != 0) {
            shift -= 8;
            if (shift >= 0) continue;
        }
        break;
    }
    return key;
}

int dm2_v1_spell_find_by_runes(const DM2_V1_SpellRecord *records,
    int record_count, const uint8_t *runes)
{
    uint32_t query;
    int index;

    if (!records || record_count <= 0 || !runes) return -1;
    query = dm2_v1_spell_pack_query_key(runes);
    if (query == 0u) return -1;

    /* c_events.cpp:2236-2262 — scan from the last table entry to the
     * first (RG4L = 0x22 decremented to -1). */
    for (index = record_count - 1; index >= 0; --index) {
        uint32_t record_key = records[index].key;
        if ((record_key & 0xff000000u) == 0u) {
            /* Top byte zero: power rune stripped on both sides. */
            if ((query & 0x00ffffffu) == record_key) return index;
        } else {
            /* Non-zero top byte: exact-power lock, full 32-bit match. */
            if (query == record_key) return index;
        }
    }
    return -1;
}

int dm2_v1_spell_record_mana_cost(const DM2_V1_SpellRecord *record,
    int cast_power)
{
    int factor;
    if (!record) return -1;
    /* c_events.cpp:2282-2289 — vw_18 = ((w6 >> 10) & 0x3f) *
     * (cast_power + 0x12) / 0x18 (integer division, truncated). */
    factor = (int)((record->w6 >> 10) & 0x3fu);
    return factor * (cast_power + 0x12) / 0x18;
}

int dm2_v1_spell_proceed_failure(int code,
    DM2_V1_SpellFailureReceipt *out_receipt)
{
    DM2_V1_SpellFailureReceipt local;
    DM2_V1_SpellFailureReceipt *r = out_receipt ? out_receipt : &local;
    int failure_class;

    memset(r, 0, sizeof(*r));
    r->valid = 1;
    r->code = code;
    r->glob_var = -1;

    /* c_events.cpp:2694-2733 — class on the high nibble. */
    failure_class = code & 0xf0;
    r->failure_class = failure_class;
    r->clears_runes = failure_class != 0x30 ? 1 : 0;

    if (failure_class < 0x20) {
        if (failure_class != 0x10) {
            return code; /* returned unchanged, no side effect */
        }
        r->handled = 1;
        r->status = ((code & 0x0f) == 3) ? (1 - 5) : (0 - 5);
        r->status &= 0xffff;
        if (r->status >= 0x8000) r->status -= 0x10000;
        r->status_written = 1;
        r->glob_var = 0x45;
    } else if (failure_class == 0x20) {
        r->handled = 1;
        r->status = -3; /* 0xfffffffd truncated to 16 bits */
        r->status_written = 1;
        r->glob_var = 0x46;
    } else if (failure_class == 0x30) {
        r->handled = 1;
        r->flask_pic_drawn = 1;
        r->glob_var = 0x44;
    } else {
        return code; /* returned unchanged, no side effect */
    }

    /* DM2_UPDATE_GLOB_VAR + v1e0b6c window: not yet bound — receipted
     * as pending instead of simulated. */
    r->glob_update_bound = 0;
    return code;
}
