
#ifndef NEXUS_V1_ACTION_TIMER_H
#define NEXUS_V1_ACTION_TIMER_H

/* Nexus V1 champion action timer — controls attack/spell readiness.
 * Each champion has a cooldown timer that must reach 0 before
 * they can perform another combat action.
 * Source: DM1 CHAMPION.C F0309 action timing,
 *         ReDMCSB CHAMPION.C action readiness clock,
 *         DM.BIN yam\champion.c action tick processing. */

#include "nexus_v1_champions.h"

#define NEXUS_MAX_ACTION_TICKS  255
#define NEXUS_BASE_MELEE_COOLDOWN  16
#define NEXUS_BASE_SPELL_COOLDOWN  24
#define NEXUS_BASE_THROW_COOLDOWN  12

typedef struct {
    int cooldown[NEXUS_MAX_PARTY];
    int max_cooldown[NEXUS_MAX_PARTY];
} Nexus_ActionTimers;

void nexus_v1_action_timers_init(Nexus_ActionTimers *timers);

/* Tick all action timers. Decrements active cooldowns. */
void nexus_v1_action_timers_tick(Nexus_ActionTimers *timers);

/* Check if a champion can act (cooldown == 0). */
int nexus_v1_action_is_ready(const Nexus_ActionTimers *timers, int party_slot);

/* Start a cooldown for a champion after performing an action.
 * base_ticks is modified by champion dexterity and load. */
void nexus_v1_action_start_cooldown(Nexus_ActionTimers *timers,
                                     int party_slot,
                                     int base_ticks,
                                     const Nexus_V1_Champion *champion);

/* Get remaining cooldown ticks. */
int nexus_v1_action_remaining(const Nexus_ActionTimers *timers, int party_slot);

/* Get cooldown fraction (0-255) for HUD display. */
int nexus_v1_action_fraction(const Nexus_ActionTimers *timers, int party_slot);

#endif
