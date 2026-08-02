
#include "nexus_v1_magic.h"
#include "nexus_v1_combat.h"
#include <stdio.h>

/* Spell type lookup table — 32 BE16 entries from DM.BIN 0x038368.
 * Index: (element * 4 + form) * 2 + class.
 * 0xFFFF = no spell for that combination. */
static const unsigned short g_spell_table[32] = {
    /* YA+GOR  priest/wizard */ 0x0000, 0x0001,
    /* YA+KATH priest/wizard */ 0x0002, 0x0003,
    /* YA+IR   priest/wizard */ 0x0005, 0x0005,
    /* YA+BRO  priest/wizard */ 0xFFFF, 0x0007,
    /* VI+GOR  priest/wizard */ 0x000A, 0x000A,
    /* VI+KATH priest/wizard */ 0x0002, 0x0003,
    /* VI+IR   priest/wizard */ 0x0005, 0x000C,
    /* VI+BRO  priest/wizard */ 0xFFFF, 0x0007,
    /* OH+GOR  priest/wizard */ 0x0001, 0x0001,
    /* OH+KATH priest/wizard */ 0x0002, 0x0003,
    /* OH+IR   priest/wizard */ 0x000A, 0x000C,
    /* OH+BRO  priest/wizard */ 0xFFFF, 0x0007,
    /* FUL+GOR priest/wizard */ 0x000A, 0x000C,
    /* FUL+KATH priest/wizard */ 0x0002, 0x0003,
    /* FUL+IR  priest/wizard */ 0x0005, 0x000C,
    /* FUL+BRO priest/wizard */ 0xFFFF, 0x0007,
};

/* Skill prefix table — DM.BIN 0x038360.
 * Maps element index to required class: 1=priest, 3=wizard.
 * (8 bytes, packed as uint16 pairs.) */
static const int g_skill_prefix[4] = { 1, 3, 1, 3 };

/* Parameter table A (priest costs) — DM.BIN 0x038320.
 * 6 pairs of (cost_base, damage_base) for power levels 0-5. */
static const int g_param_priest[6][2] = {
    {4, 6}, {7, 10}, {12, 18}, {16, 24}, {20, 30}, {24, 36}
};

/* Parameter table B (wizard costs) — DM.BIN 0x038340.
 * 6 pairs of (cost_base, damage_base) for power levels 0-5. */
static const int g_param_wizard[6][2] = {
    {5, 8}, {9, 14}, {15, 22}, {20, 30}, {25, 38}, {30, 46}
};

Nexus_SpellLookup nexus_v1_spell_lookup(int power, int element, int form,
                                        Nexus_SpellClass spell_class)
{
    Nexus_SpellLookup r = {0};
    int idx;
    unsigned short spell_type;
    const int (*params)[2];

    if (element < 0 || element >= NEXUS_ELEMENT_RUNE_COUNT ||
        form < 0 || form >= NEXUS_FORM_RUNE_COUNT ||
        power < 0 || power >= NEXUS_POWER_RUNE_COUNT ||
        (spell_class != NEXUS_SPELL_CLASS_PRIEST &&
         spell_class != NEXUS_SPELL_CLASS_WIZARD))
        return r;

    idx = (element * NEXUS_FORM_RUNE_COUNT + form) * 2 + (int)spell_class;
    spell_type = g_spell_table[idx];
    if (spell_type == NEXUS_SPELL_INVALID)
        return r;

    r.valid = 1;
    r.spell_type = (int)spell_type;
    r.spell_class = spell_class;
    r.power_level = power;
    r.element = element;
    r.form = form;

    params = (spell_class == NEXUS_SPELL_CLASS_PRIEST)
             ? g_param_priest : g_param_wizard;
    r.mana_cost = params[power][0];
    r.required_skill = power;
    return r;
}

int nexus_v1_spell_mana_cost(int power, int element) {
    (void)element;
    if (power < 0 || power >= NEXUS_POWER_RUNE_COUNT) return 999;
    return g_param_priest[power][0];
}

int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int element,
                        int form, int align)
{
    Nexus_SpellLookup sp;
    Nexus_SpellClass cls;
    const int (*params)[2];
    int cost, damage;

    if (!caster || !caster->alive) return -1;

    cls = (caster->wizard_level >= caster->priest_level)
          ? NEXUS_SPELL_CLASS_WIZARD : NEXUS_SPELL_CLASS_PRIEST;

    sp = nexus_v1_spell_lookup(power, element, form, cls);
    if (!sp.valid) {
        cls = (cls == NEXUS_SPELL_CLASS_PRIEST)
              ? NEXUS_SPELL_CLASS_WIZARD : NEXUS_SPELL_CLASS_PRIEST;
        sp = nexus_v1_spell_lookup(power, element, form, cls);
        if (!sp.valid) return -1;
    }

    params = (sp.spell_class == NEXUS_SPELL_CLASS_PRIEST)
             ? g_param_priest : g_param_wizard;
    cost = params[power][0];

    if (caster->mana < cost) return -1;

    if (sp.spell_class == NEXUS_SPELL_CLASS_PRIEST) {
        if (caster->priest_level < sp.required_skill) return -1;
    } else {
        if (caster->wizard_level < sp.required_skill) return -1;
    }

    caster->mana -= cost;

    damage = params[power][1] + nexus_v1_combat_random(params[power][1] / 2 + 1);
    (void)damage;
    (void)align;

    return cost;
}
