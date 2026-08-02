
#include "nexus_v1_hunger.h"
#include <string.h>

void nexus_v1_hunger_init(Nexus_HungerState *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

int nexus_v1_hunger_tick(Nexus_HungerState *state,
                          Nexus_V1_ChampionPool *pool) {
    int damage_mask = 0;
    int i, ci;

    if (!state || !pool) return 0;

    state->hunger_timer++;
    state->thirst_timer++;

    if (state->hunger_timer >= NEXUS_HUNGER_TICK_RATE) {
        state->hunger_timer = 0;
        for (i = 0; i < pool->party_count; i++) {
            ci = pool->party[i];
            if (!pool->champions[ci].alive) continue;
            if (pool->champions[ci].food > NEXUS_HUNGER_DRAIN)
                pool->champions[ci].food -= NEXUS_HUNGER_DRAIN;
            else if (pool->champions[ci].food > 0)
                pool->champions[ci].food = 0;
            else {
                pool->champions[ci].health -= NEXUS_STARVATION_DAMAGE;
                if (pool->champions[ci].health < 0)
                    pool->champions[ci].health = 0;
                damage_mask |= (1 << i);
            }
        }
    }

    if (state->thirst_timer >= NEXUS_THIRST_TICK_RATE) {
        state->thirst_timer = 0;
        for (i = 0; i < pool->party_count; i++) {
            ci = pool->party[i];
            if (!pool->champions[ci].alive) continue;
            if (pool->champions[ci].water > NEXUS_THIRST_DRAIN)
                pool->champions[ci].water -= NEXUS_THIRST_DRAIN;
            else if (pool->champions[ci].water > 0)
                pool->champions[ci].water = 0;
            else {
                pool->champions[ci].health -= NEXUS_DEHYDRATION_DAMAGE;
                if (pool->champions[ci].health < 0)
                    pool->champions[ci].health = 0;
                damage_mask |= (1 << (i + 4));
            }
        }
    }

    return damage_mask;
}
