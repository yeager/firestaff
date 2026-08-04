
#ifndef NEXUS_V1_HUNGER_H
#define NEXUS_V1_HUNGER_H

/* Nexus V1 hunger/thirst system — food and water deplete over time.
 * DM.BIN 0x0208C2: initial food capacity = (stat + 40) * 60.
 * DM.BIN 0x030E78-0x030EA2: food drain amount is formula-based, NOT a
 * fixed tick rate. Depends on game state byte at 0x06064570 and a
 * per-tick counter. Drain = ((counter & 7 + carry) >> 1 + adj) *
 * ((state_byte + 1) >> 2 + 1) * 2. Champion food/water fields at
 * struct offsets +109 (food) and +111 (water), single bytes. */

#include "nexus_v1_champions.h"

/* DM.BIN 0x02EEA8: accumulator-based hunger system.
 * Champion+24 accumulates ±rate each tick, oscillating between -8192 and +8192.
 * Rate constant = 1024 (0x0400), threshold = 8192 (0x2000).
 * Food/water at struct offsets +109/+111 (single bytes), capacity check 1000.
 *
 * Food drain (0x020E78): variable per tick, NOT a fixed constant:
 *   drain = 2 * ((counter & 7) >> 1 + 1) * ((encumbrance + 4) >> 2 + 1)
 *   where encumbrance = byte at 0x06064570 (0 = normal). Avg ~10 unencumbered.
 * Water drain (0x020EE0): stat-threshold model, NOT subtract-per-tick.
 *   Checks champ_word[16] >= 8; on depletion zeroes word[16]+word[26],
 *   sets flag bit 1 in word[12]. Structurally different from food.
 *
 * Starvation (0x01F4EA): E702 = MOV #2,R7 → calls 0x06014EC0 with R7=2.
 * Dehydration (0x0C6CA): E703 = MOV #3,R7 → calls 0x06014EC0 with R7=3.
 * Tick rates below are simplified proxies; real system is accumulator-driven. */
#define NEXUS_HUNGER_ACCUM_RATE   1024  /* DM.BIN 0x02EF36: 0x0400 */
#define NEXUS_HUNGER_ACCUM_THRESH 8192  /* DM.BIN 0x02EF38: 0x2000 */
#define NEXUS_HUNGER_ACCUM_CYCLE     8  /* threshold / rate = 8 ticks per drain */
#define NEXUS_FOOD_CAP_CHECK      1000  /* DM.BIN 0x02F2E6 */
#define NEXUS_FOOD_CAP_CLAMP       999  /* DM.BIN 0x02F2E8 */
#define NEXUS_HUNGER_TICK_RATE    60    /* simplified proxy for accumulator cycle */
#define NEXUS_THIRST_TICK_RATE    40    /* simplified proxy for accumulator cycle */
#define NEXUS_HUNGER_DRAIN         1
#define NEXUS_THIRST_DRAIN         1
#define NEXUS_STARVATION_DAMAGE    2    /* DM.BIN 0x01F4EA: E702 MOV #2,R7 */
#define NEXUS_DEHYDRATION_DAMAGE   3    /* DM.BIN 0x0C6CA: E703 MOV #3,R7 */

/* DM.BIN 0x0208C2: food capacity = (stat + 40) * 60 */
#define NEXUS_FOOD_CAPACITY_BASE   40
#define NEXUS_FOOD_CAPACITY_MULT   60
#define NEXUS_FOOD_MAX           255
#define NEXUS_WATER_MAX          255

typedef struct {
    int hunger_timer;
    int thirst_timer;
} Nexus_HungerState;

void nexus_v1_hunger_init(Nexus_HungerState *state);

/* Tick hunger/thirst for the whole party. Returns bitmask of
 * champions who took starvation/dehydration damage this tick. */
int nexus_v1_hunger_tick(Nexus_HungerState *state,
                          Nexus_V1_ChampionPool *pool);

#endif
