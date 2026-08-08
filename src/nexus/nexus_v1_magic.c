
#include "nexus_v1_magic.h"
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
    Nexus_SpellLookup lookup;
    int cost, magnitude;
    (void)align;

    if (!caster || !caster->alive) return -1;

    lookup = nexus_v1_spell_lookup(power, element, form, NEXUS_SPELL_CLASS_WIZARD);
    if (!lookup.valid)
        lookup = nexus_v1_spell_lookup(power, element, form, NEXUS_SPELL_CLASS_PRIEST);
    if (!lookup.valid) return -1;

    cost = lookup.mana_cost;
    if (caster->mana < cost) return -1;
    caster->mana -= cost;

    magnitude = (power >= 0 && power < 6) ? g_spell_magnitude[power] : 0;

    switch (lookup.spell_type) {
    case NEXUS_SPELL_EFFECT_HEAL:
        caster->health += magnitude;
        if (caster->health > caster->max_health)
            caster->health = caster->max_health;
        break;
    case NEXUS_SPELL_EFFECT_SHIELD:
        /* DM.BIN 0x0204E2 buff effectiveness: magnitude/8 defense bonus.
         * Applied directly to the caster's magic-resistance stat since
         * this entry point only receives the caster, not a timed
         * Nexus_StatusEffects store (see nexus_v1_spell_effect_party for
         * the duration-tracked equivalent once one is reachable here). */
        caster->anti_magic += magnitude / 8;
        break;
    case NEXUS_SPELL_EFFECT_FIRE_SHIELD:
        caster->anti_fire += magnitude / 8;
        break;
    case NEXUS_SPELL_EFFECT_STRENGTH:
        /* DM.BIN 0x03B5DC magnitude scaling, /12 to match the duration_mult_b
         * strength-bonus ratio used by nexus_v1_spell_effect_party. */
        caster->strength += magnitude / 12;
        break;
    case NEXUS_SPELL_EFFECT_LIGHT:
        /* Party light level is engine/dungeon scoped (Nexus_LightState),
         * not a Nexus_V1_Champion field; the caller wires the light state
         * via nexus_v1_light_add() using this spell's power level. Cast
         * still succeeds and mana is spent. */
        break;
    case NEXUS_SPELL_EFFECT_DARKNESS:
    case NEXUS_SPELL_EFFECT_DARKNESS_A:
    case NEXUS_SPELL_EFFECT_FIREBALL:
    case NEXUS_SPELL_EFFECT_LIGHTNING:
    case NEXUS_SPELL_EFFECT_POISON:
    case NEXUS_SPELL_EFFECT_WEAKEN:
    case NEXUS_SPELL_EFFECT_CONFUSE:
    case NEXUS_SPELL_EFFECT_SLOW:
    case NEXUS_SPELL_EFFECT_DISPEL:
        /* Target routing (creature or champion status array) is owned by
         * the caller, which has the combat target / party status state
         * this function does not. Mana is already deducted above and the
         * resolved spell_type is returned so the caller can dispatch the
         * effect via nexus_v1_spell_effect_attack_projectile() or
         * nexus_v1_spell_effect_debuff(). */
        break;
    default:
        return -1;
    }

    return lookup.spell_type;
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
