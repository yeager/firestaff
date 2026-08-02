
#ifndef NEXUS_V1_SPELL_EFFECTS_H
#define NEXUS_V1_SPELL_EFFECTS_H

/* Nexus V1 spell effect application — bridges spell casting to
 * game systems (status effects, light, projectiles, healing).
 * Source: DM.BIN 0x0383AC spell handler vtable,
 *         ReDMCSB COMMAND.C F0412 spell dispatch,
 *         DM1 CHAMPION.C spell buff application. */

#include "nexus_v1_champions.h"
#include "nexus_v1_status.h"
#include "nexus_v1_light.h"
#include "nexus_v1_magic.h"
#include "nexus_v1_rasterizer.h"

typedef struct {
    int applied;
    int heal_amount;
    int damage;
    int status_applied;
    int light_added;
    int projectile_spawned;
} Nexus_SpellEffectResult;

/* Apply a party-targeted spell effect (heal, shield, light, strength).
 * Returns what was applied. */
Nexus_SpellEffectResult nexus_v1_spell_effect_party(
    int spell_type, int power,
    Nexus_V1_Champion *caster,
    Nexus_StatusEffects *status,
    Nexus_LightState *light);

/* Compute attack spell projectile parameters.
 * Returns the projectile type to spawn and damage via out params. */
int nexus_v1_spell_effect_attack_projectile(
    int spell_type, int power,
    enum Nexus_ProjectileType *out_type,
    int *out_damage);

/* Apply a debuff spell to a target (confuse, slow, darkness). */
Nexus_SpellEffectResult nexus_v1_spell_effect_debuff(
    int spell_type, int power,
    Nexus_StatusEffects *target_status);

#endif
