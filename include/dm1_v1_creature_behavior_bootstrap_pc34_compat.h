#ifndef DM1_V1_CREATURE_BEHAVIOR_BOOTSTRAP_PC34_COMPAT_H
#define DM1_V1_CREATURE_BEHAVIOR_BOOTSTRAP_PC34_COMPAT_H

/* DM1 creature behavior event bootstrap.
 *
 * ReDMCSB GROUP.C F0180 (StartWandering) schedules a C37
 * UPDATE_BEHAVIOR_GROUP event at gameTick+1 for each newly placed group.
 * In the original game, DUNGEON.DAT's timeline already contains these
 * events for pre-existing groups; a loaded save similarly preserves them.
 *
 * This module bootstraps the event queue for groups that exist in the
 * dungeon but lack scheduled behavior events — typically after a new game
 * starts from raw DUNGEON.DAT without pre-encoded timeline data.
 *
 * The caller provides a callback to locate each group on the map, since
 * the squareFirstThings indexing is internal to the dungeon layer.
 *
 * Source references:
 *   GROUP.C F0180: StartWandering — schedules C37 at gameTick+1
 *   GROUP.C F0209: ProcessCreatureBehavior — dispatches C37..C41
 */

#include <stdint.h>

struct GameWorld_Compat;
struct DungeonGroup_Compat;

/* Callback to locate a group on any map.
 * Returns 1 and fills out_map/out_x/out_y if found, 0 otherwise. */
typedef int (*DM1_V1_FindGroupPositionFn)(
    void *ctx,
    int group_index,
    int *out_map,
    int *out_x,
    int *out_y);

typedef struct {
    int groups_found;
    int events_scheduled;
    int already_scheduled;
} DM1_V1_CreatureBehaviorBootstrapResultPc34;

/* Scan all groups in the dungeon and schedule initial UPDATE_BEHAVIOR_GROUP
 * (C37) timeline events for each living group that does not already have
 * one scheduled.  Must be called after dungeon load and timeline init.
 *
 * find_fn: callback to locate a group by index (see above).
 * find_ctx: opaque context passed to find_fn.
 *
 * Returns 1 on success, 0 on invalid arguments. */
int dm1_v1_creature_behavior_bootstrap_pc34(
    struct GameWorld_Compat *world,
    DM1_V1_FindGroupPositionFn find_fn,
    void *find_ctx,
    DM1_V1_CreatureBehaviorBootstrapResultPc34 *out_result);

#endif
