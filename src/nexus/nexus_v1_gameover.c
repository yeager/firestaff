
#include "nexus_v1_gameover.h"

#define GAMEOVER_DELAY_TICKS 60

void nexus_v1_gameover_init(Nexus_GameOverState *gs) {
    if (!gs) return;
    gs->state = NEXUS_GAMEOVER_NONE;
    gs->delay_ticks = GAMEOVER_DELAY_TICKS;
    gs->ticks_elapsed = 0;
}

int nexus_v1_gameover_check(Nexus_GameOverState *gs,
                            const Nexus_V1_ChampionPool *pool) {
    int i, any_alive = 0;
    if (!gs || !pool) return NEXUS_GAMEOVER_NONE;
    if (gs->state != NEXUS_GAMEOVER_NONE) return gs->state;
    if (pool->party_count <= 0) return NEXUS_GAMEOVER_NONE;

    for (i = 0; i < pool->party_count; i++) {
        int idx = pool->party[i];
        if (idx >= 0 && idx < NEXUS_MAX_CHAMPIONS &&
            pool->champions[idx].alive) {
            any_alive = 1;
            break;
        }
    }

    if (!any_alive) {
        gs->state = NEXUS_GAMEOVER_DEFEAT;
        gs->ticks_elapsed = 0;
    }

    return gs->state;
}

void nexus_v1_gameover_victory(Nexus_GameOverState *gs) {
    if (!gs) return;
    gs->state = NEXUS_GAMEOVER_VICTORY;
    gs->ticks_elapsed = 0;
}

int nexus_v1_gameover_tick(Nexus_GameOverState *gs) {
    if (!gs || gs->state == NEXUS_GAMEOVER_NONE) return 0;
    gs->ticks_elapsed++;
    return (gs->ticks_elapsed >= gs->delay_ticks) ? 1 : 0;
}

void nexus_v1_gameover_reset(Nexus_GameOverState *gs) {
    if (!gs) return;
    gs->state = NEXUS_GAMEOVER_NONE;
    gs->ticks_elapsed = 0;
}

int nexus_v1_gameover_is_active(const Nexus_GameOverState *gs) {
    if (!gs) return 0;
    return gs->state != NEXUS_GAMEOVER_NONE;
}
