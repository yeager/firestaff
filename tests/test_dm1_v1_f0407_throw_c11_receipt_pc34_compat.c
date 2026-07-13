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
    struct DungeonWeapon_Compat weapon;
    const struct TimelineEvent_Compat* event;
    unsigned short thrownThing;

    memset(&state, 0, sizeof(state));
    memset(&things, 0, sizeof(things));
    memset(&weapon, 0, sizeof(weapon));
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
    weapon.type = 8;
    things.loaded = 1;
    things.weapons = &weapon;
    things.weaponCount = 1;
    state.world.things = &things;
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

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
    return 0;
}
