
#include "nexus_v1_magic.h"
#include "nexus_v1_combat.h"
#include <stdio.h>

/* Spell type lookup table — 32 BE16 entries from DM.BIN 0x038368.
 * Index: (element * 4 + form) * 2 + class.
 * 0xFFFF = no spell for that combination. */
static const unsigned short g_spell_table[32] = {
    /* DM.BIN 0x038368: 32 BE16 spell type indices.
     * Index: (element * 4 + form) * 2 + class. 0xFFFF = no spell. */
    0xFFFF, 0xFFFF,  0xFFFF, 0xFFFF,  0xFFFF, 0xFFFF,  0x0000, 0xFFFF,
    0x0000, 0x0001,  0xFFFF, 0x0000,  0x0001, 0x0002,  0x000C, 0xFFFF,
    0xFFFF, 0x0001,  0x0003, 0xFFFF,  0x0000, 0x0003,  0x0001, 0x0005,
    0x000A, 0xFFFF,  0x0007, 0xFFFF,  0x0000, 0x0002,  0x0001, 0x0003,
};


/* DM.BIN 0x0601ABC0: mana cost formula (fixed-point pipeline).
 * Internal 16.16: cost_fp = 0x14000 * skill + fixmul(reserve, 1024) + 0xA000.
 * Unit = 0x4000 (16384). Formula in units: 5*skill + reserve_term + 2.5.
 * Without caster reserve data, cost ≈ (5*power + 2.5) * 10 integer mana.
 * DM.BIN 0x038320/0x038340 are VDP2 scroll register pairs (HUD). */
static int nexus_mana_cost(int power)
{
    if (power < 0) power = 0;
    if (power > 5) power = 5;
    return 50 * power + 25;
}

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

    r.mana_cost = nexus_mana_cost(power);
    r.required_skill = power;
    return r;
}

int nexus_v1_spell_mana_cost(int power, int element) {
    (void)element;
    if (power < 0 || power >= NEXUS_POWER_RUNE_COUNT) return 999;
    return nexus_mana_cost(power);
}

/* DM.BIN 0x03B5DC: magnitude table used for spell effect scaling. */
static const int g_spell_magnitude[6] = {40, 80, 120, 160, 200, 240};

int nexus_v1_cast_spell(Nexus_V1_Champion *caster, int power, int element,
                        int form, int align)
{
    Nexus_SpellLookup sp;
    Nexus_SpellClass cls;
    int cost, skill, base_dmg;

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

    skill = (sp.spell_class == NEXUS_SPELL_CLASS_PRIEST)
            ? caster->priest_level : caster->wizard_level;
    cost = nexus_mana_cost(power);

    if (caster->mana < cost) return -1;

    if (sp.spell_class == NEXUS_SPELL_CLASS_PRIEST) {
        if (caster->priest_level < sp.required_skill) return -1;
    } else {
        if (caster->wizard_level < sp.required_skill) return -1;
    }

    caster->mana -= cost;

    base_dmg = g_spell_magnitude[power < 6 ? power : 5];
    (void)align;

    return base_dmg + nexus_v1_combat_random(base_dmg / 2 + 1);
}

Nexus_SpellCategory nexus_v1_spell_category(int spell_type) {
    switch (spell_type) {
    case NEXUS_SPELL_EFFECT_HEAL:
    case NEXUS_SPELL_EFFECT_SHIELD:
    case NEXUS_SPELL_EFFECT_LIGHT:
    case NEXUS_SPELL_EFFECT_STRENGTH:
    case NEXUS_SPELL_EFFECT_FIRE_SHIELD:
        return NEXUS_SPELL_CAT_PARTY;
    case NEXUS_SPELL_EFFECT_FIREBALL:
    case NEXUS_SPELL_EFFECT_LIGHTNING:
    case NEXUS_SPELL_EFFECT_POISON:
        return NEXUS_SPELL_CAT_ATTACK;
    default:
        return NEXUS_SPELL_CAT_DEBUFF;
    }
}

int nexus_v1_spell_damage(int power, Nexus_SpellClass cls) {
    (void)cls;
    if (power < 0 || power >= NEXUS_POWER_RUNE_COUNT) return 0;
    return g_spell_magnitude[power < 6 ? power : 5];
}
