
#ifndef NEXUS_V1_ENCUMBRANCE_H
#define NEXUS_V1_ENCUMBRANCE_H

/* Nexus V1 encumbrance — DM.BIN 0x029ECC (max load), 0x02A93A (stamina cost). */

#include "nexus_v1_champions.h"

/* DM.BIN 0x02A7FA: movement tick formula.
 * tick = (RNG & 0xF) + champion[94] + weight_adjustment + skill_bonus
 * weight_adjustment uses three tiers against threshold = max_load/16:
 *   light (weight*40 <= threshold): tick += weight*40 - 12
 *   moderate (weight*40 <= threshold*1.5 - 6): tick += (weight*40 - threshold)/2
 *   severe (weight*40 > threshold*1.5 - 6): tick -= (weight*40 - upper)*2
 * Caller at 0x02C2EA clamps: max 31 (AND #0x1F), min 2 (recalc if <= 1). */
#define NEXUS_WEIGHT_SCALE      40  /* DM.BIN 0x02A83C-0x02A844: weight*40 */
#define NEXUS_LOAD_DIVISOR      16  /* DM.BIN 0x02A854: max_load >> 4 */
#define NEXUS_TICK_OFFSET      -12  /* DM.BIN 0x02A85E: ADD #-12 constant */
#define NEXUS_MIN_MOVE_TICKS     2  /* DM.BIN 0x02C2EE: recalc if <= 1 */
#define NEXUS_MAX_MOVE_TICKS    31  /* DM.BIN 0x02C2EA: AND #0x1F */
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
