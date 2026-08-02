
#ifndef NEXUS_V1_LIGHT_H
#define NEXUS_V1_LIGHT_H

/* Nexus V1 light system — party ambient light level for viewport rendering.
 * Light decays over time; torches and FUL spells restore it.
 * Source: DM1 CHAMPION.C F0325 light decay, COMMAND.C FUL spell light,
 *         OBJECTMAN.C torch burn time. */

#include <stdint.h>

#define NEXUS_LIGHT_MAX       15
#define NEXUS_LIGHT_MIN       0
#define NEXUS_LIGHT_DECAY_TICKS  120

typedef struct {
    int level;           /* 0..15 current ambient light */
    int decay_timer;     /* ticks until next decay step */
    int torch_active;    /* 1 if a torch is lit */
    int torch_ticks;     /* remaining torch burn ticks */
    int ful_active;      /* 1 if FUL spell is active */
    int ful_ticks;       /* remaining FUL spell ticks */
} Nexus_LightState;

void nexus_v1_light_init(Nexus_LightState *ls);

void nexus_v1_light_tick(Nexus_LightState *ls);

void nexus_v1_light_set(Nexus_LightState *ls, int level);

void nexus_v1_light_add(Nexus_LightState *ls, int amount);

void nexus_v1_light_torch_on(Nexus_LightState *ls, int burn_ticks);

void nexus_v1_light_torch_off(Nexus_LightState *ls);

void nexus_v1_light_ful_spell(Nexus_LightState *ls, int power, int duration_ticks);

int nexus_v1_light_get(const Nexus_LightState *ls);

#endif
