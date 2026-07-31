
#ifndef FIRESTAFF_DM2_V1_COMPANION_H
#define FIRESTAFF_DM2_V1_COMPANION_H
#include <stdint.h>

/* DM2 companion boundary.
 *
 * Do not manufacture companion records here.  A DM2 companion is a live
 * source DB creature plus its AI/CCM, inventory and dialogue ownership.  The
 * earlier host-facing API accepted arbitrary names and combat values, which
 * could create a playable-looking NPC without any original record.  It is
 * retained only as a fail-closed compatibility boundary until that complete
 * source route is imported. */

#define DM2_V1_MAX_COMPANIONS 4

typedef struct {
    char name[16];
    int health, max_health;
    int attack, defense;
    int loyalty; /* 0-100, affects behavior */
    int ai_behavior; /* 0=follow, 1=guard, 2=aggressive */
    int alive;
} DM2_V1_Companion;

typedef struct {
    DM2_V1_Companion companions[DM2_V1_MAX_COMPANIONS];
    int companion_count;
} DM2_V1_CompanionState;

void dm2_v1_companion_init(DM2_V1_CompanionState *state);
/* Always rejects: caller-provided companion values have no original owner. */
int dm2_v1_companion_add(DM2_V1_CompanionState *state, const char *name,
                         int health, int attack, int defense);
void dm2_v1_companion_tick(DM2_V1_CompanionState *state);
const char *dm2_v1_companion_source_evidence(void);
#endif
