
#include "nexus_v1_hunger.h"
#include <string.h>

void nexus_v1_hunger_init(Nexus_HungerState *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

/* DM.BIN 0x020E78: food drain formula.
 * drain = 2 * ((counter & 7) >> 1 + 1) * ((encumbrance + 4) >> 2 + 1)
 * Encumbrance 0 (normal): drain cycles through {4,4,6,6,8,8,10,10}, avg ~7.5. */
static int nexus_food_drain_amount(int tick_counter, int encumbrance)
{
    int base = ((tick_counter & 7) >> 1) + 1;
    int load = ((encumbrance + 4) >> 2) + 1;
    return 2 * base * load;
}

int nexus_v1_hunger_tick(Nexus_HungerState *state,
                          Nexus_V1_ChampionPool *pool) {
    int damage_mask = 0;
    int i, ci, drain;

    if (!state || !pool) return 0;

    state->food_accumulator += NEXUS_HUNGER_ACCUM_RATE;
    state->water_accumulator += NEXUS_HUNGER_ACCUM_RATE;
    state->tick_counter++;

    if (state->food_accumulator >= NEXUS_HUNGER_ACCUM_THRESH) {
        state->food_accumulator -= NEXUS_HUNGER_ACCUM_THRESH;
        drain = nexus_food_drain_amount(state->tick_counter, state->encumbrance);
        for (i = 0; i < pool->party_count; i++) {
            ci = pool->party[i];
            if (!pool->champions[ci].alive) continue;
            if (pool->champions[ci].food > drain)
                pool->champions[ci].food -= drain;
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

    /* DM.BIN 0x020EE0: water uses stat-threshold model.
     * Checks champ_word[16] >= 8; on depletion zeroes water, applies damage. */
    if (state->water_accumulator >= NEXUS_HUNGER_ACCUM_THRESH) {
        state->water_accumulator -= NEXUS_HUNGER_ACCUM_THRESH;
        for (i = 0; i < pool->party_count; i++) {
            ci = pool->party[i];
            if (!pool->champions[ci].alive) continue;
            if (pool->champions[ci].water >= NEXUS_WATER_THRESHOLD)
                pool->champions[ci].water -= NEXUS_WATER_THRESHOLD;
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
