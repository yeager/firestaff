
#ifndef NEXUS_V1_MAGIC_H
#define NEXUS_V1_MAGIC_H

#include "nexus_v1_champions.h"

/* DM1-compatible spell system — 6 power levels × 4 element runes.
 * Nexus uses the same rune symbols but adds new spell combinations. */

typedef enum {
    NEXUS_RUNE_LO = 0, NEXUS_RUNE_UM, NEXUS_RUNE_ON,
    NEXUS_RUNE_EE, NEXUS_RUNE_PAL, NEXUS_RUNE_MON
} Nexus_PowerRune;

typedef enum {
    NEXUS_ELEM_YA = 0, NEXUS_ELEM_VI, NEXUS_ELEM_OH,
    NEXUS_ELEM_FUL, NEXUS_ELEM_DES, NEXUS_ELEM_ZO
} Nexus_ElementRune;

typedef struct {
    int power_level;      /* 0-5 (Lo to Mon) */
    int element;          /* element rune index */
    int form;             /* form rune (optional) */
    int alignment;        /* alignment rune (optional) */
    int mana_cost;
    int damage;
    const char *name;
} Nexus_Spell;

int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int elem, int form, int align);
int nexus_v1_spell_mana_cost(int power, int elem);

#endif

