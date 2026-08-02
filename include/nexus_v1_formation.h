
#ifndef NEXUS_V1_FORMATION_H
#define NEXUS_V1_FORMATION_H

/* Nexus V1 party formation — 2x2 grid positioning for combat targeting.
 * Front row champions are more likely to be hit by melee attacks.
 * Rear row champions get ranged/spell bonuses.
 * Source: DM1 CHAMPION.C F0309 party member sub-positions,
 *         ReDMCSB CHAMPION.C sub-cell 0-3 mapping,
 *         DM.BIN yam\champion.c party grid layout. */

#include "nexus_v1_champions.h"

/* Sub-cell positions within the party's square:
 *   0 = front-left   1 = front-right
 *   2 = rear-left    3 = rear-right
 * Relative to party facing direction. */
#define NEXUS_POS_FRONT_LEFT   0
#define NEXUS_POS_FRONT_RIGHT  1
#define NEXUS_POS_REAR_LEFT    2
#define NEXUS_POS_REAR_RIGHT   3

typedef struct {
    int positions[NEXUS_MAX_PARTY];
} Nexus_Formation;

void nexus_v1_formation_init(Nexus_Formation *f, int party_count);

/* Swap two party members' positions. */
void nexus_v1_formation_swap(Nexus_Formation *f, int slot_a, int slot_b);

/* Check if a party member is in the front row. */
int nexus_v1_formation_is_front(const Nexus_Formation *f, int party_slot);

/* Get the party slot at a given position, or -1 if empty. */
int nexus_v1_formation_slot_at(const Nexus_Formation *f,
                                int position, int party_count);

/* Select a target champion for melee attack.
 * Front row is 75% likely, rear row 25%.
 * rand_val should be 0-99. */
int nexus_v1_formation_melee_target(const Nexus_Formation *f,
                                     int party_count, int rand_val);

/* Select a target for ranged/spell attack (uniform random).
 * rand_val should be 0 to party_count-1. */
int nexus_v1_formation_ranged_target(int party_count, int rand_val);

#endif
