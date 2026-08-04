
#ifndef NEXUS_V1_ENCUMBRANCE_H
#define NEXUS_V1_ENCUMBRANCE_H

/* Nexus V1 encumbrance — DM.BIN 0x029ECC (max load), 0x02A93A (stamina cost). */

#include "nexus_v1_champions.h"

#define NEXUS_BASE_MOVE_TICKS    4
#define NEXUS_MAX_MOVE_TICKS    20
#define NEXUS_STAMINA_COST_LIGHT    2   /* DM.BIN 0x02A95A: load*8 <= max*5 */
#define NEXUS_STAMINA_COST_MEDIUM   3   /* DM.BIN 0x02A95C: load*8 > max*5 */
#define NEXUS_ENCUMBRANCE_SEVERE_THRESHOLD 4000 /* DM.BIN 0x029EA4 */

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

/* Recalculate max_load — DM.BIN 0x029ECC. */
void nexus_v1_encumbrance_recalc_max_load(Nexus_V1_Champion *champion);

#endif
