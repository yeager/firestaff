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

#endif /* FIRESTAFF_DM2_V1_SPELL_H */