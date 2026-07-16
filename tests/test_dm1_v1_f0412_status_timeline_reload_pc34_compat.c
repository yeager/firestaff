/* F0412 C71/C73 events are party-global: TIMELINE.C dispatches them even
 * after a map change. A restored event may precede reconstruction of M10's
 * compatibility counter, which must remain non-negative. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"

typedef struct {
    const char* label;
    int timelineAux;
    int statusKind;
    int initialCount;
} StatusReloadCase;

static void init_world(struct GameWorld_Compat* world)
{
    memset(world, 0, sizeof(*world));
    world->newPartyMapIndex = -1;
    world->gameTick = 400;
    world->party.mapIndex = 7;
    world->partyMapIndex = 7;
}

int main(void)
{
    static const StatusReloadCase cases[] = {
        { "invisibility_missing_counter", TIMELINE_AUX_INVISIBILITY,
          LIFECYCLE_STATUS_INVISIBILITY, 0 },
        { "invisibility_active_counter", TIMELINE_AUX_INVISIBILITY,
          LIFECYCLE_STATUS_INVISIBILITY, 1 },
        { "thieves_eye_missing_counter", TIMELINE_AUX_THIEVES_EYE,
          LIFECYCLE_STATUS_THIEVES_EYE, 0 },
        { "thieves_eye_active_counter", TIMELINE_AUX_THIEVES_EYE,
          LIFECYCLE_STATUS_THIEVES_EYE, 1 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        struct GameWorld_Compat world;
        struct TickResult_Compat result;
        struct TimelineEvent_Compat event;

        init_world(&world);
        memset(&result, 0, sizeof(result));
        memset(&event, 0, sizeof(event));
        event.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
        event.fireAtTick = world.gameTick;
        event.mapIndex = 2; /* Saved source map; party has moved to map 7. */
        event.aux0 = cases[i].timelineAux;
        assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);

        if (cases[i].statusKind == LIFECYCLE_STATUS_INVISIBILITY) {
            world.magic.event71CountInvisibility = cases[i].initialCount;
            world.lifecycle.status.invisibilityCount = cases[i].initialCount;
        } else {
            world.magic.event73CountThievesEye = cases[i].initialCount;
            world.lifecycle.status.thievesEyeCount = cases[i].initialCount;
        }

        assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
        assert(world.timeline.count == 0);
        if (cases[i].statusKind == LIFECYCLE_STATUS_INVISIBILITY) {
            assert(world.magic.event71CountInvisibility == 0);
            assert(world.lifecycle.status.invisibilityCount == 0);
        } else {
            assert(world.magic.event73CountThievesEye == 0);
            assert(world.lifecycle.status.thievesEyeCount == 0);
        }
        (void)cases[i].label;
    }

    printf("PASS dm1_v1_f0412_status_timeline_reload_pc34_compat\n");
    return 0;
}
