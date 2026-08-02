
#include "nexus_v1_light.h"

void nexus_v1_light_init(Nexus_LightState *ls) {
    if (!ls) return;
    ls->level = NEXUS_LIGHT_MAX;
    ls->decay_timer = NEXUS_LIGHT_DECAY_TICKS;
    ls->torch_active = 0;
    ls->torch_ticks = 0;
    ls->ful_active = 0;
    ls->ful_ticks = 0;
}

void nexus_v1_light_tick(Nexus_LightState *ls) {
    if (!ls) return;

    if (ls->torch_active) {
        ls->torch_ticks--;
        if (ls->torch_ticks <= 0) {
            ls->torch_active = 0;
            ls->torch_ticks = 0;
        }
    }

    if (ls->ful_active) {
        ls->ful_ticks--;
        if (ls->ful_ticks <= 0) {
            ls->ful_active = 0;
            ls->ful_ticks = 0;
        }
    }

    ls->decay_timer--;
    if (ls->decay_timer <= 0) {
        ls->decay_timer = NEXUS_LIGHT_DECAY_TICKS;
        if (!ls->torch_active && !ls->ful_active) {
            if (ls->level > NEXUS_LIGHT_MIN)
                ls->level--;
        }
    }
}

void nexus_v1_light_set(Nexus_LightState *ls, int level) {
    if (!ls) return;
    if (level < NEXUS_LIGHT_MIN) level = NEXUS_LIGHT_MIN;
    if (level > NEXUS_LIGHT_MAX) level = NEXUS_LIGHT_MAX;
    ls->level = level;
}

void nexus_v1_light_add(Nexus_LightState *ls, int amount) {
    if (!ls) return;
    ls->level += amount;
    if (ls->level > NEXUS_LIGHT_MAX) ls->level = NEXUS_LIGHT_MAX;
    if (ls->level < NEXUS_LIGHT_MIN) ls->level = NEXUS_LIGHT_MIN;
}

void nexus_v1_light_torch_on(Nexus_LightState *ls, int burn_ticks) {
    if (!ls || burn_ticks <= 0) return;
    ls->torch_active = 1;
    ls->torch_ticks = burn_ticks;
    if (ls->level < 12) ls->level = 12;
}

void nexus_v1_light_torch_off(Nexus_LightState *ls) {
    if (!ls) return;
    ls->torch_active = 0;
    ls->torch_ticks = 0;
}

void nexus_v1_light_ful_spell(Nexus_LightState *ls, int power, int duration_ticks) {
    if (!ls || duration_ticks <= 0) return;
    ls->ful_active = 1;
    ls->ful_ticks = duration_ticks;
    ls->level = NEXUS_LIGHT_MAX;
    (void)power;
}

int nexus_v1_light_get(const Nexus_LightState *ls) {
    if (!ls) return 0;
    return ls->level;
}
