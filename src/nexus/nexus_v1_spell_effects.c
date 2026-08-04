
/* DM.BIN spell system — flag-based effect dispatcher, NOT a script interpreter.
 * Vtable at 0x0383AC: 8 entries × 12 bytes = {anim_primary, effect_desc, anim_secondary}.
 * "Script tables" are sprite placement tables: {u16 tile_idx, u16 0, u16 x, u16 y}.
 * Param tables at 0x038320/0x038340: spell power bar HUD coordinates, not formulas.
 * Rune mapping at 0x038360: {0x13,0x23,0x33,0x43,0x11,0x21,0x31,0x41}.
 *
 * Core spell handler at 0x01A53C: 32-bit flag word from champion+84.
 * Sub-function analysis:
 *   0x01EC34: combat data marshalling — copies spell params to combat structs,
 *             delegates to 0x017B80 (item-type matching loop, table at 0x0604AAD3).
 *   0x01ECD8: stat modifier — pure data copy (7 stat slots + 10 modifier words),
 *             0xFF sentinel for unused slots. No arithmetic.
 *   0x0204E2: status effect — resistance formula:
 *             attack_power = random(0..31) + caster_stat + (power+1)*2 - 16
 *             hits if attack_power > target_defense || critical_check()
 *             buff effectiveness = 100 - target_anti_magic, 4 buff slots max.
 *   0x012232: projectile creation — BCD decode + Saturn DMA + entity alloc.
 *             Returns 7 on success, 0xFF on failure.
 * Combat damage (0x017B80): item-type matching loop against table at 0x0604AAD3.
 * Hit resolution (0x029498): table-driven lookup at 0x06080C98, indexed by
 * attacker*4 → sub-table, then defender*2 → damage word. Tables are runtime-init. */

#include "nexus_v1_spell_effects.h"
#include <string.h>

/* DM.BIN 0x01ECD8: real system copies stat modifiers directly from spell struct
 * fields (7 stat slots at struct+0x21, 10 modifier words at struct+0x58).
 * Effect magnitudes below use power scaling; real values come from spell data
 * structs (7 stat slots at +0x21, 10 modifier words at +0x58) loaded from
 * game files at runtime — they are DATA, not CODE, and are not in DM.BIN. */
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

/* DM.BIN 0x012232: projectile creation uses BCD-decoded coordinates from spell
 * data struct, with damage from combat tables at 0x06080C98. Damage values below
 * use power scaling; real values from game data tables, not DM.BIN code. */
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

/* DM.BIN 0x0204E2: status effect resistance check.
 * attack_power = random(0..31) + caster_stat + (power+1)*2 - 16
 * Spell hits if attack_power > target_defense. */
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
        nexus_v1_status_apply(target_status, NEXUS_STATUS_SEE_THROUGH,
                              (power + 1) * 60, -((power + 1) * 2));
        r.status_applied = 1;
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
