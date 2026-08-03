
#ifndef NEXUS_V1_ENCUMBRANCE_H
#define NEXUS_V1_ENCUMBRANCE_H

/* Nexus V1 encumbrance — load affects movement speed and stamina drain.
 * Source: DM1 CHAMPION.C F0309 GetMaximumLoad / F0310 GetMovementTicks,
 *         ReDMCSB CHAMPION.C load-to-speed calculation. */

#include "nexus_v1_champions.h"

#define NEXUS_BASE_MOVE_TICKS    4
#define NEXUS_MAX_MOVE_TICKS    20
#define NEXUS_OVERLOADED_STAMINA_DRAIN  3

/* Calculate movement ticks based on load ratio.
 * Returns ticks per movement step (higher = slower). */
int nexus_v1_encumbrance_move_ticks(const Nexus_V1_Champion *champion);

/* Calculate stamina cost per movement step.
 * Normal: 1, overloaded: higher. */
int nexus_v1_encumbrance_stamina_cost(const Nexus_V1_Champion *champion);

/* Check if champion is overloaded (load > max_load). */
int nexus_v1_encumbrance_overloaded(const Nexus_V1_Champion *champion);

/* Load ratio as percentage (0-100+). Over 100 = overloaded. */
int nexus_v1_encumbrance_ratio(const Nexus_V1_Champion *champion);

/* Recalculate max_load for a champion based on strength and wounds.
 * Source: CHAMPION.C F0309 lines 1167-1173. */
void nexus_v1_encumbrance_recalc_max_load(Nexus_V1_Champion *champion);

#endif
