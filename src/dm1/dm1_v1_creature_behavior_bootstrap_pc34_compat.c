#include "dm1_v1_creature_behavior_bootstrap_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include <string.h>

static int group_has_living_creature(const struct DungeonGroup_Compat *group)
{
    int i;
    for (i = 0; i < (int)group->count + 1; ++i) {
        if (group->health[i] > 0) return 1;
    }
    return 0;
}

static int has_scheduled_behavior_event(
    const struct TimelineQueue_Compat *tl,
    int group_index)
{
    int i;
    for (i = 0; i < tl->count; ++i) {
        const struct TimelineEvent_Compat *ev = &tl->events[i];
        if (ev->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            ev->aux0 == group_index &&
            ev->aux2 >= DM1_EVENT_UPDATE_BEHAVIOR_GROUP &&
            ev->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            return 1;
        }
    }
    return 0;
}

int dm1_v1_creature_behavior_bootstrap_pc34(
    struct GameWorld_Compat *world,
    DM1_V1_FindGroupPositionFn find_fn,
    void *find_ctx,
    DM1_V1_CreatureBehaviorBootstrapResultPc34 *out_result)
{
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;
    int gi;

    memset(&result, 0, sizeof(result));
    if (out_result) *out_result = result;

    if (!world || !world->things || !world->things->groups || !find_fn) {
        return 0;
    }

    for (gi = 0; gi < world->things->groupCount; ++gi) {
        const struct DungeonGroup_Compat *group = &world->things->groups[gi];
        int mapIdx = -1, gx = -1, gy = -1;

        if (!group_has_living_creature(group)) continue;
        result.groups_found++;

        if (has_scheduled_behavior_event(&world->timeline, gi)) {
            result.already_scheduled++;
            continue;
        }

        if (!find_fn(find_ctx, gi, &mapIdx, &gx, &gy)) continue;

        {
            struct TimelineEvent_Compat ev;
            memset(&ev, 0, sizeof(ev));
            ev.kind = TIMELINE_EVENT_CREATURE_REACTION;
            ev.fireAtTick = world->gameTick + 1;
            ev.mapIndex = mapIdx;
            ev.mapX = gx;
            ev.mapY = gy;
            ev.aux0 = gi;
            ev.aux1 = group->creatureType;
            ev.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;

            if (F0721_TIMELINE_Schedule_Compat(&world->timeline, &ev)) {
                result.events_scheduled++;
            }
        }
    }

    if (out_result) *out_result = result;
    return 1;
}
