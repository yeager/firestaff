
#include "nexus_v1_action_timer.h"
#include <string.h>

void nexus_v1_action_timers_init(Nexus_ActionTimers *timers) {
    if (!timers) return;
    memset(timers, 0, sizeof(*timers));
}

void nexus_v1_action_timers_tick(Nexus_ActionTimers *timers) {
    int i;
    if (!timers) return;
    for (i = 0; i < NEXUS_MAX_PARTY; i++) {
        if (timers->cooldown[i] > 0)
            timers->cooldown[i]--;
    }
}

int nexus_v1_action_is_ready(const Nexus_ActionTimers *timers, int party_slot) {
    if (!timers || party_slot < 0 || party_slot >= NEXUS_MAX_PARTY) return 0;
    return timers->cooldown[party_slot] == 0;
}

void nexus_v1_action_start_cooldown(Nexus_ActionTimers *timers,
                                     int party_slot,
                                     int base_ticks,
                                     const Nexus_V1_Champion *champion) {
    int ticks;
    if (!timers || party_slot < 0 || party_slot >= NEXUS_MAX_PARTY) return;

    ticks = base_ticks;

    if (champion) {
        ticks -= champion->dexterity / 8;
        if (champion->load > 0 && champion->max_load > 0) {
            ticks += (champion->load * 4) / (champion->max_load > 0 ? champion->max_load : 1);
        }
    }

    if (ticks < 4) ticks = 4;
    if (ticks > NEXUS_MAX_ACTION_TICKS) ticks = NEXUS_MAX_ACTION_TICKS;

    timers->cooldown[party_slot] = ticks;
    timers->max_cooldown[party_slot] = ticks;
}

int nexus_v1_action_remaining(const Nexus_ActionTimers *timers, int party_slot) {
    if (!timers || party_slot < 0 || party_slot >= NEXUS_MAX_PARTY) return 0;
    return timers->cooldown[party_slot];
}

int nexus_v1_action_fraction(const Nexus_ActionTimers *timers, int party_slot) {
    if (!timers || party_slot < 0 || party_slot >= NEXUS_MAX_PARTY) return 0;
    if (timers->max_cooldown[party_slot] <= 0) return 0;
    return (timers->cooldown[party_slot] * 255) / timers->max_cooldown[party_slot];
}
