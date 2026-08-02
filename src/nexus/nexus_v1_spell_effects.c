
#include "nexus_v1_spell_effects.h"
#include <string.h>

Nexus_SpellEffectResult nexus_v1_spell_effect_party(
    int spell_type, int power,
    Nexus_V1_Champion *caster,
    Nexus_StatusEffects *status,
    Nexus_LightState *light) {
    Nexus_SpellEffectResult r;
    memset(&r, 0, sizeof(r));

    if (!caster) return r;

    switch (spell_type) {
    case NEXUS_SPELL_EFFECT_HEAL: {
        int amount = (power + 1) * 10;
        caster->health += amount;
        if (caster->health > caster->max_health)
            caster->health = caster->max_health;
        r.heal_amount = amount;
        r.applied = 1;
        break;
    }
    case NEXUS_SPELL_EFFECT_SHIELD:
        if (status) {
            nexus_v1_status_apply(status, NEXUS_STATUS_SHIELD,
                                  (power + 1) * 60, (power + 1) * 5);
            r.status_applied = 1;
        }
        r.applied = 1;
        break;
    case NEXUS_SPELL_EFFECT_LIGHT:
        if (light) {
            nexus_v1_light_add(light, (power + 1) * 3);
            r.light_added = (power + 1) * 3;
        }
        r.applied = 1;
        break;
    case NEXUS_SPELL_EFFECT_STRENGTH:
        if (status) {
            nexus_v1_status_apply(status, NEXUS_STATUS_HASTE,
                                  (power + 1) * 80, (power + 1) * 3);
            r.status_applied = 1;
        }
        r.applied = 1;
        break;
    case NEXUS_SPELL_EFFECT_DISPEL:
        if (status) {
            int i;
            for (i = 0; i < NEXUS_STATUS_COUNT; i++) {
                if (i == NEXUS_STATUS_POISON)
                    nexus_v1_status_remove(status, i);
            }
            r.status_applied = 1;
        }
        r.applied = 1;
        break;
    default:
        break;
    }
    return r;
}

int nexus_v1_spell_effect_attack_projectile(
    int spell_type, int power,
    enum Nexus_ProjectileType *out_type,
    int *out_damage) {
    if (!out_type || !out_damage) return 0;

    *out_damage = (power + 1) * 15;

    switch (spell_type) {
    case NEXUS_SPELL_EFFECT_FIREBALL:
        *out_type = NEXUS_PROJ_FIREBALL;
        return 1;
    case NEXUS_SPELL_EFFECT_LIGHTNING:
        *out_type = NEXUS_PROJ_LIGHTNING;
        *out_damage = (power + 1) * 20;
        return 1;
    case NEXUS_SPELL_EFFECT_POISON:
        *out_type = NEXUS_PROJ_POISON_CLOUD;
        *out_damage = (power + 1) * 8;
        return 1;
    default:
        return 0;
    }
}

Nexus_SpellEffectResult nexus_v1_spell_effect_debuff(
    int spell_type, int power,
    Nexus_StatusEffects *target_status) {
    Nexus_SpellEffectResult r;
    memset(&r, 0, sizeof(r));

    if (!target_status) return r;

    switch (spell_type) {
    case NEXUS_SPELL_EFFECT_CONFUSE:
        nexus_v1_status_apply(target_status, NEXUS_STATUS_CONFUSION,
                              (power + 1) * 40, 0);
        r.status_applied = 1;
        r.applied = 1;
        break;
    case NEXUS_SPELL_EFFECT_SLOW:
        nexus_v1_status_apply(target_status, NEXUS_STATUS_HASTE,
                              (power + 1) * 50, -((power + 1) * 2));
        r.status_applied = 1;
        r.applied = 1;
        break;
    case NEXUS_SPELL_EFFECT_DARKNESS:
    case NEXUS_SPELL_EFFECT_DARKNESS_A:
        r.applied = 1;
        break;
    case NEXUS_SPELL_EFFECT_WEAKEN:
        nexus_v1_status_apply(target_status, NEXUS_STATUS_POISON,
                              (power + 1) * 30, (power + 1) * 2);
        r.status_applied = 1;
        r.applied = 1;
        break;
    default:
        break;
    }
    return r;
}
