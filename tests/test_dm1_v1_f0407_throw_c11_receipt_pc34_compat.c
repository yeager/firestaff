/* ReDMCSB MENU.C F0407:1613-1617 live THROW -> pending C11 receipt. */
#include "m11_game_view.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <assert.h>
#include <string.h>

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static const struct TimelineEvent_Compat* find_pending_c11(
    const struct GameWorld_Compat* world,
    int championIndex)
{
    int i;

    if (!world) return NULL;
    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux4 == championIndex) {
            return event;
        }
    }
    return NULL;
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[3];
    struct TimelineEvent_Compat duplicateC11;
    struct TimelineEvent_Compat staleC11;
    struct TimelineEvent_Compat mixedOwnerC11;
    const struct TimelineEvent_Compat* event;
    unsigned short thrownThing;
    unsigned short firstQuiverWeapon;
    unsigned short secondQuiverWeapon;
    unsigned int c11Tick;
    unsigned int staleTick;

    memset(&state, 0, sizeof(state));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    M11_GameView_Init(&state);
    state.active = 1;
    state.world.gameTick = 100u;
    state.world.party.championCount = 1;
    state.world.party.direction = 1;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].stamina.current = 100;
    state.world.party.champions[0].stamina.maximum = 100;
    state.world.party.champions[0].food = 2048;
    state.world.party.champions[0].water = 2048;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    weapons[0].type = 8;
    weapons[1].type = 8;
    weapons[2].type = 8;
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 3;
    state.world.things = &things;
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    firstQuiverWeapon = make_thing(THING_TYPE_WEAPON, 1);
    secondQuiverWeapon = make_thing(THING_TYPE_WEAPON, 2);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_1] =
        firstQuiverWeapon;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_3] =
        secondQuiverWeapon;

    assert(M11_GameView_TriggerNonMeleeActionByIndex(
               &state, 0, DM1_ACTION_THROW) == 1);
    event = find_pending_c11(&state.world, 0);
    assert(event != NULL);
    assert(event->fireAtTick == 104u);
    assert(event->aux1 == DM1_PC34_C01_ACTION_HAND_SLOT_ORDINAL);
    assert(state.actionEnableSlotOrdinal[0] ==
           DM1_PC34_C01_ACTION_HAND_SLOT_ORDINAL);
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_ACTION_HAND] == THING_NONE);
    assert(!DM1_V1_F0407_MarkPendingThrowActionHandPc34Compat(
        &state.world, 0));

    /* C11 must carry ordinal two through M10, run F0253 in M11, then run
     * F0259 once.  A duplicate in the same source tick is stale after the
     * first receipt consumes the live ordinal owner. */
    c11Tick = event->fireAtTick;
    duplicateC11 = *event;
    assert(F0721_TIMELINE_Schedule_Compat(
        &state.world.timeline, &duplicateC11));
    while (state.world.gameTick <= c11Tick) {
        assert(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    assert(state.actionEnableSlotOrdinal[0] == 0xFFu);
    assert(state.actionDisabledIndex[0] == 0xFFu);
    assert(state.world.party.champions[0].actionIndex == 0xFFu);
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_ACTION_HAND] == firstQuiverWeapon);
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_QUIVER_1] == THING_NONE);
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_QUIVER_3] == secondQuiverWeapon);

    /* A later stale ordinal-two C11 is rejected before F0253/F0259.  Clear
     * the action hand only to make a forbidden second F0259 transfer visible. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    staleC11 = duplicateC11;
    staleC11.fireAtTick = state.world.gameTick;
    staleTick = staleC11.fireAtTick;
    assert(F0721_TIMELINE_Schedule_Compat(&state.world.timeline, &staleC11));
    while (state.world.gameTick <= staleTick) {
        assert(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_ACTION_HAND] == THING_NONE);
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_QUIVER_3] == secondQuiverWeapon);

    /* MENU.C F0407:1613-1617 changes C11's ordinal to two only for a
     * successful C042_ACTION_THROW/F0328 branch.  A malformed receipt
     * coupled to an ordinary SWING owner must remain locked and must not
     * reach TIMELINE.C F0259's action-hand quiver transfer. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_1] =
        firstQuiverWeapon;
    state.world.party.champions[0].actionIndex = DM1_ACTION_SWING;
    state.actionDisabledTicks[0] = 4u;
    state.actionDisabledIndex[0] = DM1_ACTION_SWING;
    state.actionEnableSlotOrdinal[0] =
        DM1_PC34_C01_ACTION_HAND_SLOT_ORDINAL;
    mixedOwnerC11 = duplicateC11;
    mixedOwnerC11.fireAtTick = state.world.gameTick;
    staleTick = mixedOwnerC11.fireAtTick;
    assert(F0721_TIMELINE_Schedule_Compat(
        &state.world.timeline, &mixedOwnerC11));
    while (state.world.gameTick <= staleTick) {
        assert(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_ACTION_HAND] == THING_NONE);
    assert(state.world.party.champions[0]
               .inventory[CHAMPION_SLOT_QUIVER_1] == firstQuiverWeapon);
    assert(state.actionDisabledTicks[0] == 4u);
    assert(state.actionDisabledIndex[0] == DM1_ACTION_SWING);
    assert(state.actionEnableSlotOrdinal[0] ==
           DM1_PC34_C01_ACTION_HAND_SLOT_ORDINAL);
    return 0;
}
