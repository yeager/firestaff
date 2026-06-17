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

typedef struct {
    uint8_t rune_count;
    uint8_t runes[6];
    uint8_t spell_type;     /* DM2_SPELL_TYPE_* */
    uint8_t difficulty;
    uint16_t required_skill;
    uint16_t power;
    uint8_t mana_per_rune;
    const char *name;
} dm2_spell_entry;

/* clang-format off */
static const dm2_spell_entry g_spell_table[DM2_MAX_SPELL_ORIGINAL] = {
    /* idx  runes                     type      diff  skill   power  mana  name */
    { 3, {DM2_RUNE_OH, DM2_RUNE_IR, DM2_RUNE_RA, 0,   0,   0}, DM2_SPELL_TYPE_GENERAL, 0x04, 0x0F, 0, 1, "Long Light" },
    { 3, {DM2_RUNE_DES, DM2_RUNE_IR, DM2_RUNE_SAR, 0, 0, 0},  DM2_SPELL_TYPE_GENERAL, 0x04, 0x0F, 0, 1, "Darkness" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_EW, DM2_RUNE_SAR, 0,  0, 0},  DM2_SPELL_TYPE_GENERAL, 0x04, 0x0F, 0, 2, "Spell Shield (Party)" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_EW, DM2_RUNE_SAR, 0,  0, 0},  DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 3, "Invisibility" },
    { 2, {DM2_RUNE_YA, DM2_RUNE_IR, 0, 0, 0, 0},              DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 1, "Magical Shield" },
    { 1, {DM2_RUNE_FUL, 0, 0, 0, 0, 0},                       DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 0, "Light" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_EW, DM2_RUNE_DAIN, 0, 0, 0},  DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 2, "Aura of Wisdom" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_EW, DM2_RUNE_ROS, 0, 0, 0},   DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 2, "Aura of Dexterity" },
    { 3, {DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_NETA, 0, 0, 0},DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 3, "Fire Shield" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_EW, DM2_RUNE_NETA, 0, 0, 0},  DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 2, "Aura of Vitality" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_EW, DM2_RUNE_KU, 0, 0, 0},    DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 2, "Aura of Strength" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_IR, DM2_RUNE_ROS, 0, 0, 0},   DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 2, "Aura of Speed" },
    { 3, {DM2_RUNE_ZO, DM2_RUNE_BRO, DM2_RUNE_ROS, 0, 0, 0},  DM2_SPELL_TYPE_GENERAL, 0x0F, 0x0F, 0, 4, "Spell Reflector" }, /* DM2 new */
    { 2, {DM2_RUNE_YA, DM2_RUNE_EW, 0, 0, 0, 0},              DM2_SPELL_TYPE_GENERAL, 0x00, 0x0F, 0, 1, "Magical Marker" },
    { 2, {DM2_RUNE_OH, DM2_RUNE_VEN, 0, 0, 0, 0},             DM2_SPELL_TYPE_GENERAL, 0x07, 0x0F, 0, 3, "Poison Cloud" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_KATH, DM2_RUNE_RA, 0, 0, 0},  DM2_SPELL_TYPE_MISSILE, 0x0D, 0x0F, 0, 5, "Lightning" },
    { 2, {DM2_RUNE_FUL, DM2_RUNE_IR, 0, 0, 0, 0},             DM2_SPELL_TYPE_MISSILE, 0x00, 0x0F, 0, 4, "Fireball" },
    { 3, {DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_KU, 0, 0, 0},  DM2_SPELL_TYPE_POTION, 0x13, 0x07, 0, 2, "NP: STR Potion" },
    { 2, {DM2_RUNE_DES, DM2_RUNE_EW, 0, 0, 0, 0},             DM2_SPELL_TYPE_MISSILE, 0x03, 0x0F, 0, 6, "Antimatter" },
    { 2, {DM2_RUNE_DES, DM2_RUNE_VEN, 0, 0, 0, 0},            DM2_SPELL_TYPE_MISSILE, 0x06, 0x0F, 0, 4, "Poison Bolt" },
    { 1, {DM2_RUNE_ZO, 0, 0, 0, 0, 0},                        DM2_SPELL_TYPE_GENERAL, 0x04, 0x0F, 0, 1, "Open/Close Door" },
    { 2, {DM2_RUNE_YA, DM2_RUNE_BRO, 0, 0, 0, 0},             DM2_SPELL_TYPE_POTION, 0x13, 0x0C, 0, 3, "NP: Shield Potion" },
    { 1, {DM2_RUNE_YA, 0, 0, 0, 0, 0},                        DM2_SPELL_TYPE_POTION, 0x13, 0x0B, 0, 2, "NP: Stamina Potion" },
    { 3, {DM2_RUNE_YA, DM2_RUNE_BRO, DM2_RUNE_DAIN, 0, 0, 0}, DM2_SPELL_TYPE_POTION, 0x13, 0x08, 0, 3, "NP: Wisdom Potion" },
    { 3, {DM2_RUNE_YA, DM2_RUNE_BRO, DM2_RUNE_NETA, 0, 0, 0}, DM2_SPELL_TYPE_POTION, 0x13, 0x09, 0, 3, "NP: Vitality Potion" },
    { 1, {DM2_RUNE_VI, 0, 0, 0, 0, 0},                        DM2_SPELL_TYPE_POTION, 0x13, 0x0E, 0, 2, "NP: Health Potion" },
    { 2, {DM2_RUNE_VI, DM2_RUNE_BRO, 0, 0, 0, 0},            DM2_SPELL_TYPE_POTION, 0x13, 0x0A, 0, 2, "NP: Anti Venin" },
    { 3, {DM2_RUNE_OH, DM2_RUNE_BRO, DM2_RUNE_ROS, 0, 0, 0}, DM2_SPELL_TYPE_POTION, 0x13, 0x06, 0, 3, "NP: Dexterity Potion" },
    { 3, {DM2_RUNE_ZO, DM2_RUNE_BRO, DM2_RUNE_RA, 0, 0, 0},  DM2_SPELL_TYPE_POTION, 0x13, 0x0D, 0, 3, "NP: Mana Potion" },
    { 3, {DM2_RUNE_ZO, DM2_RUNE_EW, DM2_RUNE_KU, 0, 0, 0},   DM2_SPELL_TYPE_SUMMON, 0x0F, 0x31, 0, 8, "Attack Minion" },      /* DM2 new */
    { 3, {DM2_RUNE_ZO, DM2_RUNE_EW, DM2_RUNE_NETA, 0, 0, 0}, DM2_SPELL_TYPE_SUMMON, 0x0F, 0x34, 0, 8, "Guard Minion" },     /* DM2 new */
    { 3, {DM2_RUNE_ZO, DM2_RUNE_EW, DM2_RUNE_ROS, 0, 0, 0},  DM2_SPELL_TYPE_SUMMON, 0x0F, 0x35, 0, 8, "U-Haul Minion" },    /* DM2 new */
    { 3, {DM2_RUNE_OH, DM2_RUNE_KATH, DM2_RUNE_KU, 0, 0, 0},  DM2_SPELL_TYPE_GENERAL, 0x0D, 0x09, 0, 6, "Push" },           /* DM2 new */
    { 3, {DM2_RUNE_OH, DM2_RUNE_KATH, DM2_RUNE_ROS, 0, 0, 0}, DM2_SPELL_TYPE_GENERAL, 0x0D, 0x0A, 0, 6, "Pull" },           /* DM2 new */
};
/* clang-format on */

/* ── Spell name table (aligned with dSpellsTable order) ───────────────────
 * Source: skproject/SKWIN/SkGlobal.cpp:966-1011 */

static const char *const g_spell_names[DM2_MAX_SPELL_ORIGINAL] = {
    "Long Light",
    "Darkness",
    "Spell Shield (Party)",
    "Invisibility",
    "Magical Shield",
    "Light",
    "Aura of Wisdom",
    "Aura of Dexterity",
    "Fire Shield",
    "Aura of Vitality",
    "Aura of Strength",
    "Aura of Speed",
    "Spell Reflector",
    "Magical Marker",
    "Poison Cloud",
    "Lightning",
    "Fireball",
    "NP: STR Potion",
    "Antimatter",
    "Poison Bolt",
    "Open/Close Door",
    "NP: Shield Potion",
    "NP: Stamina Potion",
    "NP: Wisdom Potion",
    "NP: Vitality Potion",
    "NP: Health Potion",
    "NP: Anti Venin",
    "NP: Dexterity Potion",
    "NP: Mana Potion",
    "Attack Minion",
    "Guard Minion",
    "U-Haul Minion",
    "Push",
    "Pull",
};

const DM2_SpellDefinition *dm2_v1_spell_get(int spell_index) {
    static DM2_SpellDefinition out;
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return NULL;
    const dm2_spell_entry *e = &g_spell_table[spell_index];
    memset(&out, 0, sizeof(out));
    out.rune_count = e->rune_count;
    memcpy(out.rune_symbols, e->runes, e->rune_count * sizeof(uint8_t));
    out.spell_type = e->spell_type;
    out.difficulty = e->difficulty;
    out.required_skill = e->required_skill;
    out.power = e->power;
    out.mana_per_rune = e->mana_per_rune;
    strncpy(out.name, e->name ? e->name : g_spell_names[spell_index], sizeof(out.name) - 1);
    return &out;
}

int dm2_v1_spell_count(void) {
    return DM2_MAX_SPELL_ORIGINAL;
}

int dm2_v1_spell_type(int spell_index) {
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return -1;
    return g_spell_table[spell_index].spell_type;
}

int dm2_v1_spell_resolves_object_effect(int spell_index, int effect_id) {
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return 0;
    (void)effect_id;
    /* Stub: spell->OBJECT_EFFECT mapping.
     * Source: SkWinCore.cpp:27038-27096.
     * Fireball→1, Lightning→2, Dispell→3, Push→4, Pull→5,
     * PoisonCloud→6, PoisonBolt→7, PoisonBlob→8, Steal→9, Shoot→10, Pushback→11 */
    switch (spell_index) {
        case 16: return DM2_OBJECT_EFFECT_FIREBALL;      /* Fireball */
        case 15: return DM2_OBJECT_EFFECT_LIGHTNING;      /* Lightning */
        case 12: return DM2_OBJECT_EFFECT_DISPELL;         /* Spell Reflector */
        case 32: return DM2_OBJECT_EFFECT_PUSH_SPELL;      /* Push */
        case 33: return DM2_OBJECT_EFFECT_PULL_SPELL;      /* Pull */
        case 14: return DM2_OBJECT_EFFECT_POISON_CLOUD;   /* Poison Cloud */
        case 19: return DM2_OBJECT_EFFECT_POISON_BOLT;     /* Poison Bolt */
        default: return 0;
    }
}

const char *dm2_v1_spell_name(int spell_index) {
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return "?";
    return g_spell_names[spell_index];
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
    if (rune_count < 1) return 0;
    const dm2_spell_entry *e = &g_spell_table[spell_index];
    if ((int)e->rune_count != rune_count) return 0;
    for (int i = 0; i < rune_count; i++) {
        if (rune_sequence[i] != e->runes[i]) return 0;
    }
    return 1;
}

/* ── Mana cost (excludes POWER rune at index 0) ────────────────────
 * Returns (rune_count - 1) * mana_per_rune — the cumulative cost
 * already deducted when the runes were added to the spell tail. */
int dm2_v1_spell_mana_cost(int spell_index)
{
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return 0;
    const dm2_spell_entry *e = &g_spell_table[spell_index];
    int paid_runes = (int)e->rune_count - 1;
    if (paid_runes < 0) paid_runes = 0;
    return paid_runes * (int)e->mana_per_rune;
}

/* ── Compute cast chance ───────────────────────────────────── */
int dm2_v1_spell_compute_chance(int spell_index, int wizard_ability)
{
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return -1;
    const dm2_spell_entry *e = &g_spell_table[spell_index];
    int bp08 = (int)e->difficulty + (int)e->power;
    int bp06 = wizard_ability;
    int bp0c = (bp06 + 15) - bp08;
    return bp0c;
}

/* ── Can-cast pre-check ───────────────────────────────────── */
int dm2_v1_spell_can_cast(int spell_index,
    int wizard_ability, int current_mana)
{
    if (spell_index < 0 || spell_index >= DM2_MAX_SPELL_ORIGINAL) return 0;
    const dm2_spell_entry *e = &g_spell_table[spell_index];
    if (wizard_ability < (int)e->required_skill) return 0;
    int cost = dm2_v1_spell_mana_cost(spell_index);
    if (current_mana < cost) return 0;
    return 1;
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
    const dm2_spell_entry *e = &g_spell_table[spell_index];
    int bp08 = (int)e->difficulty + (int)e->power;
    int bp0c = (wizard_ability + 15) - bp08;
    r.effective_difficulty = bp08;
    r.effective_chance = bp0c;
    r.mana_used = dm2_v1_spell_mana_cost(spell_index);

    /* Pre-check: skill + mana sufficient to even attempt */
    if (!dm2_v1_spell_can_cast(spell_index, wizard_ability, current_mana)) {
        r.success = 0;
        r.cooldown_ticks = 0;
        r.skill_decay = DM2_SPELL_SKILL_DECAY_ON_FAIL;
        return r;
    }

    if (bp0c > 0) {
        /* Cast succeeds */
        r.success = 1;
        r.cooldown_ticks = DM2_SPELL_COOLDOWN_TICKS;
        r.skill_decay = 0;
    } else {
        /* Cast fails: skill decay penalty */
        r.success = 0;
        r.cooldown_ticks = 0;
        r.skill_decay = DM2_SPELL_SKILL_DECAY_ON_FAIL;
    }
    return r;
}