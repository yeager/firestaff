/* ReDMCSB CHAMPION.C F0330 C11 producer regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_f0259_quiver_refill_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

#include <assert.h>
#include <string.h>

static int find_c11_enable_action(const struct GameWorld_Compat* world)
{
    int i;

    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux4 == 0) {
            return i;
        }
    }
    return -1;
}

static int count_c11_enable_actions(const struct GameWorld_Compat* world)
{
    int i;
    int count = 0;

    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux0 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux2 == DM1_EVENT_ENABLE_CHAMPION_ACTION &&
            event->aux4 == 0) {
            ++count;
        }
    }
    return count;
}

static void prepare_open_door_spell_world(struct GameWorld_Compat* world,
                                          struct DungeonThings_Compat* things,
                                          struct DungeonWeapon_Compat* weapon)
{
    struct ChampionState_Compat* champion;

    memset(world, 0, sizeof(*world));
    memset(things, 0, sizeof(*things));
    memset(weapon, 0, sizeof(*weapon));
    things->weapons = weapon;
    things->weaponCount = 1;
    world->things = things;
    assert(F0881_WORLD_InitDefault_Compat(world, 0x789Au));
    world->things = things;
    champion = &world->party.champions[0];
    champion->present = 1;
    champion->hp.current = 100;
    champion->mana.current = 100;
    champion->mana.maximum = 24;
    champion->attributes[CHAMPION_ATTR_WISDOM] = 60;
    champion->cell = 1;
    champion->direction = 3;
    champion->inventory[CHAMPION_SLOT_ACTION_HAND] =
        (unsigned short)(THING_TYPE_WEAPON << 10);
    world->party.championCount = 1;
    world->party.activeChampionIndex = 0;
    world->party.direction = 1;
    world->lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 500;
    world->lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_AIR].experience = 500;
}

static int cast_open_door(struct GameWorld_Compat* world,
                          struct TickResult_Compat* result)
{
    struct TickInput_Compat input;

    memset(&input, 0, sizeof(input));
    memset(result, 0, sizeof(*result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 14; /* G0487 Open Door */
    input.reserved = 1;
    return F0884_ORCH_AdvanceOneTick_Compat(world, &input, result);
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct TickResult_Compat result;
    int eventIndex;
    uint32_t firstTick;
    int disabledTicks = 0;
    int i;

    prepare_open_door_spell_world(&world, &things, &weapon);
    assert(cast_open_door(&world, &result) == ORCH_OK);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_ACTION_DISABLED) {
            disabledTicks = result.emissions[i].payload[1];
        }
    }
    assert(disabledTicks > 0);
    eventIndex = find_c11_enable_action(&world);
    assert(eventIndex >= 0);
    assert(count_c11_enable_actions(&world) == 1);
    assert(world.timeline.events[eventIndex].aux1 == 0);
    assert(world.timeline.events[eventIndex].fireAtTick == (uint32_t)disabledTicks);
    firstTick = world.timeline.events[eventIndex].fireAtTick;

    /* A second F0330 action replaces, rather than duplicates, C11 and uses
     * the source half-distance rule against the current pending event. */
    assert(cast_open_door(&world, &result) == ORCH_OK);
    eventIndex = find_c11_enable_action(&world);
    assert(eventIndex >= 0);
    assert(count_c11_enable_actions(&world) == 1);
    assert(world.timeline.events[eventIndex].fireAtTick ==
           1u + (uint32_t)disabledTicks + ((firstTick - 1u) >> 1));
    return 0;
}
