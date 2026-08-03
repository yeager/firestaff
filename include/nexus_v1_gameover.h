
#ifndef NEXUS_V1_GAMEOVER_H
#define NEXUS_V1_GAMEOVER_H

#include "nexus_v1_champions.h"

enum {
    NEXUS_GAMEOVER_NONE    = 0,
    NEXUS_GAMEOVER_DEFEAT  = 1,
    NEXUS_GAMEOVER_VICTORY = 2
};

typedef struct {
    int state;
    int delay_ticks;
    int ticks_elapsed;
} Nexus_GameOverState;

void nexus_v1_gameover_init(Nexus_GameOverState *gs);

/* Check if all party members are dead. If so, start defeat sequence.
 * Returns current gameover state. */
int nexus_v1_gameover_check(Nexus_GameOverState *gs,
                            const Nexus_V1_ChampionPool *pool);

/* Trigger victory (called when final boss defeated or objective met). */
void nexus_v1_gameover_victory(Nexus_GameOverState *gs);

/* Tick the gameover delay counter. Returns 1 when delay has elapsed
 * and the gameover screen should be shown. */
int nexus_v1_gameover_tick(Nexus_GameOverState *gs);

/* Reset gameover state (e.g. on load). */
void nexus_v1_gameover_reset(Nexus_GameOverState *gs);

int nexus_v1_gameover_is_active(const Nexus_GameOverState *gs);

#endif
