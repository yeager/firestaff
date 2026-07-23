#ifndef FIRESTAFF_DM2_V1_SPELL_H
#define FIRESTAFF_DM2_V1_SPELL_H
#include <stdint.h>

/* DM2 V1 — Spell System (34 spells vs DM1 ~30)
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM, skproject/SKWIN/SkGlobal.cpp:966-1011, SkGlobal.h:37-55
 * SkWinCore.cpp:17521-18174 (CAST_SPELL_PLAYER, ADD_RUNE_TO_TAIL)
 *
 * DM2 spell casting:
 *   - 34 base spells (index 0-33), 255 in extended GDAT mode
 *   - Per-rune mana cost deduction (not at cast time)
 *   - Hand cooldown after successful cast: bp0e ticks (SkWinCore.cpp:17623)
 *   - Skill decay on failed cast: explicit penalty
 *   - 6 new spells vs DM1: Spell Reflector, Attack/Guard/U-Haul Minion, Push, Pull
 */

/* ── Spell type constants ───────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.h:50-53 */

#define DM2_SPELL_TYPE_POTION   1  /* requires empty flask in hand */
#define DM2_SPELL_TYPE_MISSILE  2  /* fires projectile at target */
#define DM2_SPELL_TYPE_GENERAL  3  /* enchantments, light, auras */
#define DM2_SPELL_TYPE_SUMMON   4  /* summons a creature minion */

/* ── Rune symbol constants (for spell construction) ───────────────────────
 * Source: SKULL.ASM rune data, SkWinCore.cpp:18159-18174
 * Rune order matters: first rune = POWER rune (no mana cost)
 * 17 rune symbols (0-16) in DM2's rune alphabet */

#define DM2_RUNE_OH    0
#define DM2_RUNE_IR    1
#define DM2_RUNE_RA    2
#define DM2_RUNE_DES   3
#define DM2_RUNE_SAR   4
#define DM2_RUNE_YA    5
#define DM2_RUNE_EW    6
#define DM2_RUNE_FUL   7
#define DM2_RUNE_BRO   8
#define DM2_RUNE_NETA  9
#define DM2_RUNE_KATH 10
#define DM2_RUNE_KU   11
#define DM2_RUNE_ROS  12
#define DM2_RUNE_VEN  13
#define DM2_RUNE_ZO   14
#define DM2_RUNE_DAIN 15
#define DM2_RUNE_VI   16
#define DM2_RUNE_COUNT 17

/* ── Spell count constants ──────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.h:37-55 */

#define DM2_MAX_SPELL_ORIGINAL  34   /* 0-33 in fixed mode */
#define DM2_MAX_SPELL_CUSTOM   255   /* extended GDAT mode */
#define DM2_MAX_RUNE_CHAIN      8    /* max runes per spell */

/* ── Spell definition struct ──────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.cpp:966-1011 (dSpellsTable)
 * Fields: runes[6], type, difficulty, requiredSkill, power, name
 *
 * Cast chance formula (SkWinCore.cpp:17521-17670):
 *   bp08 = ref->difficulty + power
 *   bp0c = (WizardAbility + 15) vs bp08
 *   fail → skill decay penalty proportional to bp0c << (bp08 - bp06)
 */

typedef struct {
    uint8_t rune_count;
    uint8_t rune_symbols[DM2_MAX_RUNE_CHAIN];
    uint8_t spell_type;    /* DM2_SPELL_TYPE_* */
    uint8_t difficulty;    /* cast difficulty */
    uint16_t required_skill; /* skill level required */
    uint16_t power;        /* spell power (affects damage/duration) */
    uint8_t mana_per_rune; /* mana cost per rune (applied per-rune, not at cast) */
    char    name[32];
} DM2_SpellDefinition;

/* ── Object effect constants (spell resolution) ─────────────────────────
 * Source: skproject/SKWIN/SkWinCore.cpp:27038-27096
 * Used by creature AI attack resolution and champion spell casting */

#define DM2_OBJECT_EFFECT_NONE         0
#define DM2_OBJECT_EFFECT_FIREBALL      1   /* high AoE damage */
#define DM2_OBJECT_EFFECT_LIGHTNING     2   /* single-target electric */
#define DM2_OBJECT_EFFECT_DISPELL       3   /* remove enchantments */
#define DM2_OBJECT_EFFECT_PUSH_SPELL    4   /* telekinetic push */
#define DM2_OBJECT_EFFECT_PULL_SPELL    5   /* telekinetic pull */
#define DM2_OBJECT_EFFECT_POISON_CLOUD  6   /* AoE poison */
#define DM2_OBJECT_EFFECT_POISON_BOLT   7   /* single-target poison */
#define DM2_OBJECT_EFFECT_POISON_BLOB   8   /* contact poison */
#define DM2_OBJECT_EFFECT_STEAL         9   /* item theft */
#define DM2_OBJECT_EFFECT_SHOOT        10   /* ranged projectile */
#define DM2_OBJECT_EFFECT_PUSHBACK     11   /* knockback */
#define DM2_OBJECT_EFFECT_SUMMON_ATTACK_MINION 12 /* ATTACK MINION ally */
#define DM2_OBJECT_EFFECT_SUMMON_GUARD_MINION  13 /* GUARD MINION ally */
#define DM2_OBJECT_EFFECT_SUMMON_UHAUL_MINION  14 /* U-HAUL MINION ally */

/* ── Full 34-spell table ─────────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.cpp:966-1011 (dSpellsTable)
 *
 * Runes shown as symbol names; actual GDAT stores indices.
 * Difficulty and skill values from dSpellsTable entry order.
 *
 * New DM2 spells (not in DM1):
 *   12  Spell Reflector (ZO BRO ROS)  — reflects incoming spells
 *   29  Attack Minion (ZO EW KU)    — summons Attack Minion (AI 14)
 *   30  Guard Minion (ZO EW NETA)   — summons Guard Minion (AI 17)
 *   31  U-Haul Minion (ZO EW ROS)   — summons U-Haul Minion (AI 18)
 *   32  Push (OH KATH KU)           — telekinetic push
 *   33  Pull (OH KATH ROS)          — telekinetic pull
 */

/* ── Spell casting result ────────────────────────────────────────────────
 * Source: SkWinCore.cpp:17521-17670 */

/* ── Spell cast attempt result ─────────────────────────────────────
 * Source: SkWinCore.cpp:17521-17670 */
typedef struct {
    int success;
    int mana_used;
    int cooldown_ticks;
    int skill_decay;
    int effective_difficulty; /* bp08 = difficulty + power */
    int effective_chance;     /* bp0c = (wizard_ability + 15) - bp08 */
} DM2_SpellCastResult;

/* ── Public API ──────────────────────────────────────────────────────── */

const DM2_SpellDefinition *dm2_v1_spell_get(int spell_index);
int  dm2_v1_spell_count(void);
int  dm2_v1_spell_type(int spell_index);
int  dm2_v1_spell_resolves_object_effect(int spell_index, int effect_id);
const char *dm2_v1_spell_name(int spell_index);
const char *dm2_v1_spell_source_evidence(void);

/* ── Phase 4 expansion: spell casting mechanics ────────────────────────
 *
 * Source: skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 *
 * Cast chance formula:
 *   bp08 = spell->difficulty + spell->power
 *   bp06 = champion->wizard_ability
 *   bp0c = (bp06 + 15) - bp08
 *   if bp0c <= 0: cast fails (skill decay penalty proportional to bp0c)
 *   else: cast succeeds with mana_per_rune * rune_count deducted
 *   cooldown: bp0e ticks after successful cast (default 0x08 = 8 ticks)
 *
 * DM2 differences from DM1:
 *   - DM2 has 34 spells in fixed mode (DM1 has ~30)
 *   - DM2 spells cost mana PER RUNE, not at cast time
 *   - DM2 spells have a hand cooldown after successful cast
 *   - DM2 spells apply skill decay on failed cast (DM1: skill decay on fail)
 *
 * ── Per-rune mana cost deduction (not at cast time)
 * Source: skproject/SKWIN/SkWinCore.cpp:18159-18174 (ADD_RUNE_TO_TAIL)
 *
 * When a rune is added to the spell tail, the champion's mana is reduced
 * by `mana_per_rune`. If mana < mana_per_rune, the rune cannot be added.
 * On cast, mana has already been deducted — only the cooldown + skill
 * decay happen at cast time. */

/* ── Phase 4 expansion: spell casting mechanics ────────────────────────
 *
 * Source: skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 *
 * Cast chance formula:
 *   bp08 = spell->difficulty + spell->power
 *   bp06 = champion->wizard_ability
 *   bp0c = (bp06 + 15) - bp08
 *   if bp0c <= 0: cast fails (skill decay penalty proportional to bp0c)
 *   else: cast succeeds with mana_per_rune * rune_count deducted
 *   cooldown: bp0e ticks after successful cast (default 0x08 = 8 ticks)
 *
 * DM2 differences from DM1:
 *   - DM2 has 34 spells in fixed mode (DM1 has ~30)
 *   - DM2 spells cost mana PER RUNE, not at cast time
 *   - DM2 spells have a hand cooldown after successful cast
 *   - DM2 spells apply skill decay on failed cast (DM1: skill decay on fail)
 *
 * ── Per-rune mana cost deduction (not at cast time)
 * Source: skproject/SKWIN/SkWinCore.cpp:18159-18174 (ADD_RUNE_TO_TAIL)
 *
 * When a rune is added to the spell tail, the champion's mana is reduced
 * by `mana_per_rune`. If mana < mana_per_rune, the rune cannot be added.
 * On cast, mana has already been deducted — only the cooldown + skill
 * decay happen at cast time. */

/* ── Validate a rune sequence matches a known spell ───────────────
 * Returns 1 if the rune sequence matches the spell at spell_index.
 * The first rune (POWER rune) is consumed at no mana cost; the rest
 * cost mana_per_rune each.  Source: SkWinCore.cpp:17521-17670. */
int dm2_v1_spell_validate_runes(int spell_index,
    const uint8_t *rune_sequence, int rune_count);

/* ── Compute total mana cost (excludes the POWER rune at index 0) ──
 * Source: SkWinCore.cpp:18159-18174 (per-rune deduction) */
int dm2_v1_spell_mana_cost(int spell_index);

/* ── Compute cast chance for one champion ────────────────────────
 * Returns bp0c = (wizard_ability + 15) - (difficulty + power).
 * Positive value = cast succeeds; negative or zero = cast fails.
 * Source: SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER). */
int dm2_v1_spell_compute_chance(int spell_index, int wizard_ability);

/* ── Check if champion can cast a spell at all ─────────────────
 * Returns 1 if the champion meets the required_skill threshold
 * AND has enough mana for the spell (excluding POWER rune). */
int dm2_v1_spell_can_cast(int spell_index,
    int wizard_ability, int current_mana);

/* ── Attempt to cast a spell ────────────────────────────────────
 * Computes cast chance, deducts mana, applies cooldown on success,
 * applies skill decay on failure.  Returns a DM2_SpellCastResult with
 * all the per-cast metrics for the wire-up probe. */
DM2_SpellCastResult dm2_v1_spell_cast_attempt(int spell_index,
    int wizard_ability, int current_mana);

/* ── DM2-007: source rune-key lookup (DM2_FIND_SPELL_BY_RUNES) ─────────
 * skproject/SKULLWIN/c_events.cpp:2211-2264 packs the hero rune string
 * into a 32-bit query key (first rune << 24, then << 16, << 8, << 0;
 * max 4 runes; the first rune is the POWER rune) and scans the 34-entry
 * (0x22) 8-byte spell-record table from the last entry to the first.
 * A record whose key top byte is zero matches on the low 24 bits only
 * (power rune stripped); a non-zero top byte is an exact-power lock and
 * requires a full 32-bit match.
 *
 * Record layout (c_events.cpp DM2_CAST_SPELL_PLAYER consumers):
 *   dw0 key: rune1<<16 | rune2<<8 | rune3, top byte = power lock or 0
 *   b4: difficulty   b5: skill id
 *   w6: bits 0-3 execution class, bits 4-9 spell value, bits 10-15
 *       power factor (mana = factor * (cast_power + 0x12) / 0x18,
 *       c_events.cpp:2282-2289) */

typedef struct {
    uint32_t key;        /* rune1<<16|rune2<<8|rune3 (+ power lock <<24) */
    uint8_t difficulty;  /* record byte 4 */
    uint8_t skill;       /* record byte 5 */
    uint16_t w6;         /* record word 6 */
} DM2_V1_SpellRecord;

/* Source key packing: rune[0]<<24 | rune[1]<<16 | rune[2]<<8 | rune[3],
 * stopping at the first zero byte (c_events.cpp:2220-2234). */
uint32_t dm2_v1_spell_pack_query_key(const uint8_t *runes);

/* Source scan: entries count-1 .. 0, masked 24-bit compare when the
 * record key top byte is zero, full 32-bit compare otherwise
 * (c_events.cpp:2236-2262).  Returns the matching record index or -1. */
int dm2_v1_spell_find_by_runes(const DM2_V1_SpellRecord *records,
    int record_count, const uint8_t *runes);

/* Source mana cost: ((w6 >> 10) & 0x3f) * (cast_power + 0x12) / 0x18
 * (c_events.cpp:2282-2289).  Returns -1 on NULL record. */
int dm2_v1_spell_record_mana_cost(const DM2_V1_SpellRecord *record,
    int cast_power);

/* ── DM2-007: source failure classification (DM2_PROCEED_SPELL_FAILURE) ──
 * skproject/SKULLWIN/c_events.cpp:2687-2733 classes the cast result by
 * its high nibble:
 *   0x10: insufficient skill variant — status = (low==3 ? 1:0) - 5,
 *         glob var 0x45
 *   0x20: unknown rune combination — status = -3, glob var 0x46
 *   0x30: no empty flask — transparent static pic drawn, glob var 0x44,
 *         minimum display window 3
 *   other: returned unchanged, no side effect
 * DM2_TRY_CAST_SPELL (c_events.cpp:2738-2786) clears the hero rune tail
 * and redraws the squad hands panel for every class except 0x30.
 * The DM2_UPDATE_GLOB_VAR side effect and the v1e0b6c window write are
 * not yet bound; the receipt marks them pending instead of simulating
 * them. */

typedef struct {
    int valid;            /* receipt populated */
    int code;             /* input code */
    int failure_class;    /* code & 0xf0 */
    int handled;          /* 1 when the source takes a class branch */
    int status;           /* v1e0b7c writeback (16-bit truncation) */
    int status_written;   /* 1 for classes 0x10/0x20 */
    int flask_pic_drawn;  /* 1 for class 0x30 (draw call receipted) */
    int glob_var;         /* 0x45/0x46/0x44, or -1 when not written */
    int glob_update_bound;/* 0: DM2_UPDATE_GLOB_VAR not yet bound */
    int clears_runes;     /* DM2_TRY_CAST_SPELL: class != 0x30 */
} DM2_V1_SpellFailureReceipt;

int dm2_v1_spell_proceed_failure(int code,
    DM2_V1_SpellFailureReceipt *out_receipt);

#endif /* FIRESTAFF_DM2_V1_SPELL_H */
