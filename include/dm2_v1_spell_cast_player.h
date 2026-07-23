#ifndef FIRESTAFF_DM2_V1_SPELL_CAST_PLAYER_H
#define FIRESTAFF_DM2_V1_SPELL_CAST_PLAYER_H

#include "dm2_v1_spell.h"
#include "dm2_v1_extended_spells_definition.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2-007 — DM2_CAST_SPELL_PLAYER bounded runtime slice.
 *
 * Source: skproject/SKULLWIN/c_events.cpp:2211-2786
 *         DM2_FIND_SPELL_BY_RUNES, DM2_TRY_CAST_SPELL, DM2_CAST_SPELL_PLAYER
 *         skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 *
 * This module binds live hero rune strings to validated original spell records
 * (the fixed 34-spell table plus the bounded GDAT SPELL_DEF receipt), computes
 * resource spending, classifies the execution branch, and emits a timer-effect
 * request.  It does NOT mutate champion state, create objects, or queue timers;
 * those remain host-owned until their respective source contracts are proven.
 */

enum {
    DM2_V1_SPELL_CAST_PLAYER_MAX_RECORDS =
        DM2_MAX_SPELL_ORIGINAL + DM2_V1_EXT_SPELLS_CUSTOM_COUNT
};

/* Runtime spell record: a unified, source-ordered entry used for live lookup.
 * Combines the fixed dSpellsTable layout with the bounded GDAT SPELL_DEF
 * receipt so both paths share one lookup and cast routine. */
typedef struct {
    int index;              /* 0-33 fixed, 34+ maps to extended index - 34 */
    int source;             /* 0 = fixed table, 1 = extended GDAT receipt */
    uint32_t key;           /* source rune query key */
    uint8_t runes[4];       /* rune symbols (power + up to 3 tail runes) */
    int rune_count;         /* number of rune symbols (1-4) */
    uint8_t difficulty;     /* record byte 4 */
    uint8_t skill;          /* record byte 5 */
    uint16_t w6;            /* record word 6: type low 4, result upper bits */
    uint8_t spell_value;    /* (w6 >> 4) & 0x3f, or extended result field */
    uint8_t spell_type;     /* w6 & 0x0f: POTION/MISSILE/GENERAL/SUMMON */
} DM2_V1_RuntimeSpellRecord;

/* Unified runtime spell table. */
typedef struct {
    int count;
    int extended_mode;
    DM2_V1_RuntimeSpellRecord records[DM2_V1_SPELL_CAST_PLAYER_MAX_RECORDS];
} DM2_V1_RuntimeSpellTable;

/* Spell execution branch classification.
 * Source: skproject/SKWIN/SkWinCore.cpp:17563 switch (ref->w6 & 15). */
enum {
    DM2_V1_SPELL_EXEC_POTION  = 1,
    DM2_V1_SPELL_EXEC_MISSILE = 2,
    DM2_V1_SPELL_EXEC_GENERAL = 3,
    DM2_V1_SPELL_EXEC_SUMMON  = 4
};

/* Timer-effect request kind.
 * These are the DM2-owned, source-named side effects that a successful cast
 * would schedule.  Actual timer creation stays host-owned. */
enum {
    DM2_V1_SPELL_TIMER_NONE,          /* no bounded timer effect */
    DM2_V1_SPELL_TIMER_LIGHT,         /* Long Light, Light */
    DM2_V1_SPELL_TIMER_AURA,          /* Aura of Wisdom/Dexterity/etc. */
    DM2_V1_SPELL_TIMER_ENCHANTMENT,   /* Spell Shield, Fire Shield, Invisibility */
    DM2_V1_SPELL_TIMER_CLOUD,         /* Poison Cloud, Spell Reflector */
    DM2_V1_SPELL_TIMER_SUMMON,        /* Attack/Guard/U-Haul Minion */
    DM2_V1_SPELL_TIMER_PROJECTILE     /* Lightning, Fireball, Push, Pull, etc. */
};

/* Full DM2_CAST_SPELL_PLAYER result receipt. */
typedef struct {
    int valid;                        /* receipt populated */
    int found;                        /* rune string resolved to a record */
    int spell_index;                  /* resolved record index in runtime table */
    int cast_power;                   /* power rune value used for scaling */

    /* Lookup/cast inputs and outputs */
    int wizard_skill;                 /* champion wizard skill level (bp06) */
    int current_mana;                 /* champion mana before cast */
    int mana_cost;                    /* source record mana cost */
    int mana_sufficient;              /* 1 if current_mana >= mana_cost */

    /* Source cast-chance math (SkWinCore.cpp:17535-17555):
     *   bp08 = difficulty + cast_power
     *   bp0c = (wizard_skill + 15) - bp08
     * Positive bp0c means success in this bounded slice. */
    int bp08;
    int bp0c;
    int cast_success;

    int cooldown_ticks;               /* bp0e = ((w6_a_f * (power + 18)) / 24) */
    int skill_decay;                  /* bounded skill penalty on failure */

    /* Execution branch classification */
    int execution_class;              /* DM2_V1_SPELL_EXEC_* */
    int flask_required;               /* POTION branch needs empty flask */
    int flask_consumed;               /* 1 when flask would be consumed */

    /* Timer-effect request (host-owned timer creation) */
    int timer_kind;                   /* DM2_V1_SPELL_TIMER_* */
    int timer_duration;               /* source-derived duration in ticks */
    int object_effect;                /* DM2_OBJECT_EFFECT_* for missiles/clouds */

    /* Failure classification (DM2_PROCEED_SPELL_FAILURE) */
    int failure_class;                /* 0x10/0x20/0x30 or 0 */
    DM2_V1_SpellFailureReceipt failure;
} DM2_V1_SpellCastPlayerReceipt;

/* Build a unified runtime spell table from the fixed 34-spell table plus the
 * bounded GDAT SPELL_DEF receipt.  Fixed entries are always present; extended
 * entries are admitted only when the receipt was loaded successfully. */
void dm2_v1_spell_cast_player_build_table(
    const DM2_V1_ExtendedSpellsReceipt *extended,
    DM2_V1_RuntimeSpellTable *out_table);

/* Look up a live hero rune string in a runtime table.
 * Source: c_events.cpp:2211-2264 DM2_FIND_SPELL_BY_RUNES.
 * Returns the table index or -1. */
int dm2_v1_spell_cast_player_find_by_runes(
    const DM2_V1_RuntimeSpellTable *table,
    const uint8_t *runes);

/* Execute the bounded DM2_CAST_SPELL_PLAYER slice for a prepared rune string.
 * Source: skproject/SKWIN/SkWinCore.cpp:17521-17670.
 *
 * Inputs:
 *   table        — unified runtime spell table
 *   runes        — zero-terminated hero rune string (power + tail)
 *   wizard_skill — champion wizard skill level (bp06)
 *   current_mana — champion mana before cast
 *   flask_in_hand — 1 if an empty flask is held (POTION branch gate)
 *
 * The receipt reports cast success/failure, resource cost, execution branch,
 * timer-effect request, and failure classification.  No state is mutated. */
DM2_V1_SpellCastPlayerReceipt dm2_v1_spell_cast_player(
    const DM2_V1_RuntimeSpellTable *table,
    const uint8_t *runes,
    int wizard_skill,
    int current_mana,
    int flask_in_hand);

const char *dm2_v1_spell_cast_player_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SPELL_CAST_PLAYER_H */
