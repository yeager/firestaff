
#ifndef NEXUS_V1_REST_H
#define NEXUS_V1_REST_H

/* Nexus V1 rest system — party rest/sleep for stamina and mana regen.
 * Source: DM1 CHAMPION.C F0325 stamina regen, F0309 sleep mechanics.
 * Party rests in place. Creatures can interrupt rest. */

#include "nexus_v1_champions.h"

#define NEXUS_REST_REGEN_TICKS     10
#define NEXUS_REST_STAMINA_PER_TICK 2
#define NEXUS_REST_MANA_PER_TICK    1
#define NEXUS_REST_HEALTH_PER_TICK  1
#define NEXUS_REST_HEALTH_INTERVAL  30

typedef struct {
    int resting;         /* 1 if party is currently resting */
    int rest_ticks;      /* ticks spent resting this session */
    int regen_timer;     /* ticks until next regen pulse */
    int health_timer;    /* ticks until next health regen */
    int interrupted;     /* 1 if rest was interrupted by enemy */
} Nexus_RestState;

void nexus_v1_rest_init(Nexus_RestState *rs);

void nexus_v1_rest_start(Nexus_RestState *rs);

void nexus_v1_rest_stop(Nexus_RestState *rs);

/* Tick rest. Applies regen to all living party champions.
 * Returns 1 if any champion was healed/restored. */
int nexus_v1_rest_tick(Nexus_RestState *rs, Nexus_V1_ChampionPool *pool);

void nexus_v1_rest_interrupt(Nexus_RestState *rs);

int nexus_v1_rest_is_resting(const Nexus_RestState *rs);

int nexus_v1_rest_all_full(const Nexus_V1_ChampionPool *pool);

#endif
