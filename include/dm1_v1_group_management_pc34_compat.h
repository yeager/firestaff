#ifndef FIRESTAFF_DM1_V1_GROUP_MANAGEMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_MANAGEMENT_PC34_COMPAT_H

/*
 * DM1 V1 Creature Group Management — source-locked to ReDMCSB GROUP.C
 *
 * Active creature groups, group movement, cell assignment,
 * party distance/direction calculations.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   GROUP.C: F0175 (get group thing at position),
 *            F0176 (get creature ordinal in cell),
 *            F0177-F0191 (group movement, AI direction),
 *            F0192 (group damage), F0193 (add active group),
 *            F0194 (remove active group)
 *
 * An "active group" is a group within tracking range of the party.
 * G0375: active group array, G0376: max count, G0377: current count.
 * Groups have 1-4 creatures in 4 cells (2x2 grid per square).
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M11_MAX_ACTIVE_GROUPS   32
#define M11_MAX_CREATURES_PER_GROUP 4
#define M11_GROUP_CELLS         4
#define M11_FLUX_CAGE_MAX       4

/* Creature group — maps to GROUP struct in DEFS.H */
typedef struct {
    uint16_t thingIndex;        /* thing list index */
    uint16_t creatureType;      /* creature definition index */
    uint8_t  cells[M11_GROUP_CELLS]; /* creature index per cell, 0xFF=empty */
    uint8_t  creatureCount;
    uint8_t  behavior;          /* AI behavior mode */
    int16_t  hitPoints[M11_MAX_CREATURES_PER_GROUP];
    int      mapX;
    int      mapY;
    int      mapIndex;
} M11_Group;

/* Active group tracking — wraps G0375 active group array */
typedef struct {
    M11_Group activeGroups[M11_MAX_ACTIVE_GROUPS];
    int activeGroupCount;       /* G0377 */
    int maxActiveGroups;        /* G0376, typically 32 */

    /* Current group processing state (F0177+) */
    int currentGroupMapX;       /* G0378 */
    int currentGroupMapY;       /* G0379 */
    uint16_t currentGroupThing; /* G0380 */
    int currentGroupDistToParty;    /* G0381 */
    int currentGroupPrimaryDir;     /* G0382 */
    int currentGroupSecondaryDir;   /* G0383 */

    /* Movement testing state */
    uint8_t movementTestedDirs[4];     /* G0384 */
    int movementBlockedByWall;         /* G0387 */
    uint16_t movementBlockedByGroup;   /* G0388 */
    int movementBlockedByDoor;         /* G0389 */
    int movementBlockedByParty;        /* G0390 */

    /* Flux cages */
    int8_t fluxCages[M11_FLUX_CAGE_MAX]; /* G0385 */
    int fluxCageCount;                   /* G0386 */
} M11_GroupState;

/*
 * Initialize group state.
 */
void m11_group_init(M11_GroupState *s);

/*
 * Get the group at a map position (F0175).
 * Returns pointer to active group or NULL.
 */
M11_Group *m11_group_get_at(M11_GroupState *s, int mapX, int mapY);

/*
 * Get creature ordinal in a specific cell of a group (F0176).
 * Returns creature index (0-3) or -1 if cell is empty.
 */
int m11_group_get_creature_in_cell(const M11_Group *group, int cell);

/*
 * Add a group to active tracking (F0193).
 * Returns index or -1 if full.
 */
int m11_group_add_active(M11_GroupState *s, const M11_Group *group);

/*
 * Remove a group from active tracking (F0194).
 * Returns 1 on success, 0 if not found.
 */
int m11_group_remove_active(M11_GroupState *s, int mapX, int mapY);

/*
 * Calculate distance and direction from group to party (used by AI).
 * Sets currentGroupDistToParty, currentGroupPrimaryDir, currentGroupSecondaryDir.
 */
void m11_group_calc_party_relation(M11_GroupState *s,
                                    int groupX, int groupY,
                                    int partyX, int partyY);

/*
 * Test if a group can move in a direction.
 * Checks walls, doors, other groups, party, flux cages.
 * Returns 1 if movement possible.
 */
int m11_group_can_move(M11_GroupState *s,
                       int groupX, int groupY, int direction,
                       int mapWidth, int mapHeight);

/*
 * Move a group one step in a direction.
 * Updates group mapX/mapY. Returns 1 on success.
 */
int m11_group_move(M11_GroupState *s, M11_Group *group, int direction);

/*
 * Apply damage to a creature in a group (F0192).
 * Returns remaining HP. Negative means creature died.
 */
int m11_group_damage_creature(M11_Group *group, int creatureIndex, int damage);

/*
 * Remove a dead creature from a group.
 * Clears cell, decrements count. Returns 1 if group is now empty.
 */
int m11_group_remove_creature(M11_Group *group, int creatureIndex);

/*
 * Check if a group has any living creatures.
 */
int m11_group_is_alive(const M11_Group *group);

/*
 * Get total creature count across all active groups.
 */
int m11_group_total_creature_count(const M11_GroupState *s);

/*
 * Source evidence string.
 */
const char *m11_group_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GROUP_MANAGEMENT_PC34_COMPAT_H */
