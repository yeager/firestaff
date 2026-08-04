
#ifndef NEXUS_V1_REST_H
#define NEXUS_V1_REST_H

/* Nexus V1 rest system — DM.BIN 0x02F318 regen pulse, 0x030914 timer reset. */

#include "nexus_v1_champions.h"

#define NEXUS_REST_REGEN_TICKS     30   /* DM.BIN 0x02F32E: CMP/GE #30 */
/* DM.BIN 0x02F318: fixed-point accumulator regen system.
 * R11 = fixmul at 0x060873B0: (R4*R5)>>16 via DMULS.L+XTRCT (16.16 fixed-point).
 * Case 1 (stamina/mana): accum += 2 * fixmul(rate, rate) = 2*rate²/65536.
 *   Fires regen tick when upper 16 bits of accum >= 30.
 * Case 2 (health): accum += sign(rate) * fixmul(rate, rate) = sign(rate)*rate²/65536.
 *   Fires regen tick when upper 16 bits of accum >= 46 and rate > 0.
 * Struct layout (entity_base+24): rate at +12 (32-bit), accumulator at +16 (32-bit).
 * Quadratic scaling: doubling the stat-derived rate quadruples accumulation. */
#define NEXUS_REST_STAMINA_PER_TICK 2
#define NEXUS_REST_MANA_PER_TICK    1
#define NEXUS_REST_HEALTH_PER_TICK  1
#define NEXUS_REST_HEALTH_INTERVAL  46   /* DM.BIN 0x02F382: CMP/GE #46 */
#define NEXUS_REST_ACCUM_INCREMENT  0x4000  /* DM.BIN 0x02F38C */
#define NEXUS_REST_HEALTH_UNIT      0x0400  /* DM.BIN 0x02F394 */

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
