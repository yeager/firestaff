
#include "nexus_v1_rest.h"
#include <string.h>

void nexus_v1_rest_init(Nexus_RestState *rs) {
    if (!rs) return;
    memset(rs, 0, sizeof(*rs));
}

void nexus_v1_rest_start(Nexus_RestState *rs) {
    if (!rs) return;
    rs->resting = 1;
    rs->rest_ticks = 0;
    rs->regen_timer = NEXUS_REST_REGEN_TICKS;
    rs->health_timer = NEXUS_REST_HEALTH_INTERVAL;
    rs->interrupted = 0;
}

void nexus_v1_rest_stop(Nexus_RestState *rs) {
    if (!rs) return;
    rs->resting = 0;
}

int nexus_v1_rest_tick(Nexus_RestState *rs, Nexus_V1_ChampionPool *pool) {
    int i, restored = 0;
    if (!rs || !pool || !rs->resting) return 0;

    rs->rest_ticks++;
    rs->regen_timer--;
    rs->health_timer--;

    if (rs->regen_timer <= 0) {
        rs->regen_timer = NEXUS_REST_REGEN_TICKS;
        for (i = 0; i < pool->party_count; i++) {
            int ci = pool->party[i];
            Nexus_V1_Champion *ch;
            if (ci < 0 || ci >= pool->champion_count) continue;
            ch = &pool->champions[ci];
            if (!ch->alive) continue;

            if (ch->stamina < ch->max_stamina) {
                ch->stamina += NEXUS_REST_STAMINA_PER_TICK;
                if (ch->stamina > ch->max_stamina)
                    ch->stamina = ch->max_stamina;
                restored = 1;
            }
            if (ch->mana < ch->max_mana) {
                ch->mana += NEXUS_REST_MANA_PER_TICK;
                if (ch->mana > ch->max_mana)
                    ch->mana = ch->max_mana;
                restored = 1;
            }
        }
    }

    if (rs->health_timer <= 0) {
        rs->health_timer = NEXUS_REST_HEALTH_INTERVAL;
        for (i = 0; i < pool->party_count; i++) {
            int ci = pool->party[i];
            Nexus_V1_Champion *ch;
            if (ci < 0 || ci >= pool->champion_count) continue;
            ch = &pool->champions[ci];
            if (!ch->alive) continue;

            if (ch->health < ch->max_health) {
                ch->health += NEXUS_REST_HEALTH_PER_TICK;
                if (ch->health > ch->max_health)
                    ch->health = ch->max_health;
                restored = 1;
            }
        }
    }

    if (nexus_v1_rest_all_full(pool)) {
        rs->resting = 0;
    }

    return restored;
}

void nexus_v1_rest_interrupt(Nexus_RestState *rs) {
    if (!rs) return;
    rs->resting = 0;
    rs->interrupted = 1;
}

int nexus_v1_rest_is_resting(const Nexus_RestState *rs) {
    if (!rs) return 0;
    return rs->resting;
}

int nexus_v1_rest_all_full(const Nexus_V1_ChampionPool *pool) {
    int i;
    if (!pool) return 1;
    for (i = 0; i < pool->party_count; i++) {
        int ci = pool->party[i];
        const Nexus_V1_Champion *ch;
        if (ci < 0 || ci >= pool->champion_count) continue;
        ch = &pool->champions[ci];
        if (!ch->alive) continue;
        if (ch->health < ch->max_health) return 0;
        if (ch->stamina < ch->max_stamina) return 0;
        if (ch->mana < ch->max_mana) return 0;
    }
    return 1;
}
