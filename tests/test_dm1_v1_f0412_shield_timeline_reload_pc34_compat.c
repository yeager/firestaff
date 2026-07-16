/* ReDMCSB MENU.C F0412 creates C74/C78 defense events; TIMELINE.C removes
 * their exact contribution. Restored events remain party-global across maps
 * and must not drive an unrebuilt Firestaff defense mirror negative. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"

typedef struct {
    int timelineAux;
    int statusKind;
    int eventDefense;
    int initialDefense;
} ShieldReloadCase;

int main(void)
{
    static const ShieldReloadCase cases[] = {
        { TIMELINE_AUX_PARTY_SHIELD, LIFECYCLE_STATUS_PARTY_SHIELD, 12, 0 },
        { TIMELINE_AUX_PARTY_SHIELD, LIFECYCLE_STATUS_PARTY_SHIELD, 12, 20 },
        { TIMELINE_AUX_FIRESHIELD, LIFECYCLE_STATUS_FIRE_SHIELD, 9, 0 },
        { TIMELINE_AUX_FIRESHIELD, LIFECYCLE_STATUS_FIRE_SHIELD, 9, 14 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        struct GameWorld_Compat world;
        struct TickResult_Compat result;
        struct TimelineEvent_Compat event;
        int expected = cases[i].initialDefense - cases[i].eventDefense;

        if (expected < 0) expected = 0;
        memset(&world, 0, sizeof(world));
        memset(&result, 0, sizeof(result));
        memset(&event, 0, sizeof(event));
        world.newPartyMapIndex = -1;
        world.gameTick = 800;
        world.party.mapIndex = 7;
        world.partyMapIndex = 7;
        event.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
        event.fireAtTick = world.gameTick;
        event.mapIndex = 1; /* Saved cast map, distinct from current party map. */
        event.aux0 = cases[i].timelineAux;
        if (cases[i].statusKind == LIFECYCLE_STATUS_PARTY_SHIELD) {
            event.aux4 = cases[i].eventDefense;
            world.magic.partyShieldDefense = cases[i].initialDefense;
            world.lifecycle.status.partyShieldDefense = cases[i].initialDefense;
        } else {
            event.aux3 = cases[i].eventDefense;
            world.magic.fireShieldDefense = cases[i].initialDefense;
            world.lifecycle.status.partyFireShieldDefense = cases[i].initialDefense;
        }
        assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1);
        assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
        assert(world.timeline.count == 0);
        if (cases[i].statusKind == LIFECYCLE_STATUS_PARTY_SHIELD) {
            assert(world.magic.partyShieldDefense == expected);
            assert(world.lifecycle.status.partyShieldDefense == expected);
        } else {
            assert(world.magic.fireShieldDefense == expected);
            assert(world.lifecycle.status.partyFireShieldDefense == expected);
        }
    }

    printf("PASS dm1_v1_f0412_shield_timeline_reload_pc34_compat\n");
    return 0;
}
