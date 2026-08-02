
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
/* Cast a spell and return the mana cost (or -1 on failure).
 * out_damage receives the computed damage value if non-NULL. */
int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int element,
                        int form, int align);

/* Compute spell damage without casting (for display/preview). */
int nexus_v1_spell_damage(int power, Nexus_SpellClass cls);

/* Spell effect types (from spell_type in g_spell_table). */
#define NEXUS_SPELL_EFFECT_HEAL       0x0000
#define NEXUS_SPELL_EFFECT_SHIELD     0x0001
#define NEXUS_SPELL_EFFECT_LIGHT      0x0002
#define NEXUS_SPELL_EFFECT_POISON     0x0003
#define NEXUS_SPELL_EFFECT_FIREBALL   0x0005
#define NEXUS_SPELL_EFFECT_LIGHTNING  0x0007
#define NEXUS_SPELL_EFFECT_STRENGTH   0x000A
#define NEXUS_SPELL_EFFECT_DARKNESS   0x000C

#endif
