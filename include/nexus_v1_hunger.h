
#ifndef NEXUS_V1_HUNGER_H
#define NEXUS_V1_HUNGER_H

/* Nexus V1 hunger/thirst system — food and water deplete over time.
 * When food or water reach 0, the champion takes periodic damage.
 * Source: DM1 CHAMPION.C F0309 hunger/thirst processing,
 *         ReDMCSB CHAMPION.C food/water depletion per tick. */

#include "nexus_v1_champions.h"

#define NEXUS_HUNGER_TICK_RATE    60
#define NEXUS_THIRST_TICK_RATE    40
#define NEXUS_HUNGER_DRAIN         1
#define NEXUS_THIRST_DRAIN         1
#define NEXUS_STARVATION_DAMAGE    2
#define NEXUS_DEHYDRATION_DAMAGE   3
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
