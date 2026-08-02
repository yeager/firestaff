
#ifndef NEXUS_V1_MAGIC_H
#define NEXUS_V1_MAGIC_H

#include "nexus_v1_champions.h"

/* Nexus spell system — real data from DM.BIN (yam\spell.c).
 * 4 elements x 4 forms x 2 classes (priest/wizard) = 32 lookup entries.
 * Source: DM.BIN offset 0x038368, SH-2 address 0x06048368. */

#define NEXUS_SPELL_INVALID 0xFFFF

#define NEXUS_POWER_RUNE_COUNT 6
#define NEXUS_ELEMENT_RUNE_COUNT 4
#define NEXUS_FORM_RUNE_COUNT 4

typedef enum {
    NEXUS_RUNE_LO = 0, NEXUS_RUNE_UM, NEXUS_RUNE_ON,
    NEXUS_RUNE_EE, NEXUS_RUNE_PAL, NEXUS_RUNE_MON
} Nexus_PowerRune;

typedef enum {
    NEXUS_ELEM_YA = 0, NEXUS_ELEM_VI, NEXUS_ELEM_OH, NEXUS_ELEM_FUL
} Nexus_ElementRune;

typedef enum {
    NEXUS_FORM_GOR = 0, NEXUS_FORM_KATH, NEXUS_FORM_IR, NEXUS_FORM_BRO
} Nexus_FormRune;

typedef enum {
    NEXUS_SPELL_CLASS_PRIEST = 0,
    NEXUS_SPELL_CLASS_WIZARD = 1
} Nexus_SpellClass;

typedef struct {
    int valid;
    int spell_type;
    Nexus_SpellClass spell_class;
    int power_level;
    int element;
    int form;
    int mana_cost;
    int required_skill;
} Nexus_SpellLookup;

Nexus_SpellLookup nexus_v1_spell_lookup(int power, int element, int form,
                                        Nexus_SpellClass spell_class);
int nexus_v1_spell_mana_cost(int power, int element);
int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int element,
                        int form, int align);

#endif
