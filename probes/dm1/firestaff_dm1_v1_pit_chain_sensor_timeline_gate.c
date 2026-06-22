/*
 * DM1 V1 open-pit transition chain regression gate.
 *
 * Source-lock anchors:
 *   - ReDMCSB MOVESENS.C F0267 lines 538-574: an open, non-imaginary
 *     pit moves the party through F0154_DUNGEON_GetLocationAfterLevelChange
 *     and applies 20 HP fall damage to every living champion when the
 *     rope is not being used.  When the fall damage meets or exceeds
 *     the champion's current HP, that champion is marked
 *     fall-killed (championFallKilled[i] = 1).
 *   - ReDMCSB MOVESENS.C F0267 line 801: after the final destination
 *     is resolved, the party's source square is processed as WALK_OFF
 *     (F0276_SENSOR_ProcessThingAdditionOrRemoval(... C0_FALSE)) so
 *     the cross-map pit transition routes a sensor enter/leave pass
 *     over the source square even though the destination is on a
 *     different map.
 *   - ReDMCSB MOVESENS.C F0267 line 818: the post-resolve final square
 *     is processed as WALK_ON (F0276_SENSOR_ProcessThingAdditionOrRemoval
 *     (... C1_TRUE)) so the lower-map landing sensor is dispatched
 *     exactly once per cross-map pit fall.
 *   - ReDMCSB MOVESENS.C F0324_CHAMPION_DamageAll_GetDamagedChampionCount:
 *     fall damage is applied with C2_ATTACK_SELF and the
 *     MASK_WOUND_LEGS | MASK_WOUND_FEET wound mask when the rope is
 *     not used (line 601).  When every living champion's HP reaches
 *     zero, the party-dead flag flips and the next tick entry
 *     surfaces EMIT_PARTY_DEAD (orchestrator F0889 line 3302 +
 *     F0884 step 5 line 3362).
 *   - ReDMCSB SENSOR.C F0710 line 62-64: only SENSOR_EVENT_WALK_ON
 *     and SENSOR_EVENT_CHAMPION_ACTION fire sensor effects in v1.
 *     SENSOR_EVENT_WALK_OFF returns an empty effect list (the v1
 *     sensor policy); this gate relies on that policy for the
 *     source-map WALK_OFF pass during a cross-map pit fall.
 *
 * This probe exercises the real F0884 tick orchestrator path for
 * one cardinal move: origin corridor -> open pit -> lower-map landing
 * sensor, with a deliberately lethal two-champion party.  The
 * F0884 -> F0888 -> F0704 -> F0702 -> F0889 -> F0884 step 5 chain
 * is the narrow chain the regression exercises.  The assertion
 * surface is intentionally narrow: one transition, one lethal fall,
 * one landing sensor, one downstream EMIT_PARTY_DEAD.
 *
 * The narrow downstream chain proved here is:
 *   open pit -> 20 dmg per living champion -> fall-killed flag
 *               -> HP drops to 0 -> F0889 alive==0 check
 *               -> F0884 step 5 EMIT_PARTY_DEAD emission.
 * A pit fall that kills every living champion therefore surfaces
 * EMIT_PARTY_DEAD without any additional state mutation; this is
 * the timeline-style death consequence the v1 orchestrator must
 * produce deterministically.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#define MAP_W 3
#define MAP_H 3
#define MAP_COUNT 2
#define SFT_COUNT 1

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg)
{
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static unsigned char square_byte(int elementType, int attrs)
{
    return (unsigned char)(((elementType & 7) << 5) | (attrs & 0x1F));
}

static int count_emissions(const struct TickResult_Compat* result, uint8_t kind)
{
    int i;
    int count = 0;
    for (i = 0; i < result->emissionCount; ++i) {
        if (result->emissions[i].kind == kind) ++count;
    }
    return count;
}

static int find_emission(const struct TickResult_Compat* result, uint8_t kind)
{
    int i;
    for (i = 0; i < result->emissionCount; ++i) {
        if (result->emissions[i].kind == kind) return i;
    }
    return -1;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[MAP_COUNT];
    struct DungeonMapTiles_Compat tiles[MAP_COUNT];
    unsigned char map0[MAP_W * MAP_H];
    unsigned char map1[MAP_W * MAP_H];
    unsigned short squareFirstThings[SFT_COUNT];
    struct DungeonSensor_Compat sensors[1];
    int i;
    int fellIndex;
    int sensorIndex;
    int partyDeadIndex;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(sensors, 0, sizeof(sensors));

    /*
     * Two maps: map 0 is the origin level with the open pit at (1,1).
     * Map 1 is the lower level with a TEXT sensor (type 13) on the
     * landing square (1,1).  The source map keeps no sensors so the
     * WALK_OFF pass during the cross-map pit fall provably produces
     * zero sensor emissions under F0710's v1 policy.
     */
    for (i = 0; i < MAP_W * MAP_H; ++i) {
        map0[i] = square_byte(DUNGEON_ELEMENT_CORRIDOR, 0);
        map1[i] = square_byte(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    for (i = 0; i < SFT_COUNT; ++i) {
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    map0[(1 * MAP_H) + 1] = square_byte(DUNGEON_ELEMENT_PIT, 0x08);
    map1[(1 * MAP_H) + 1] = square_byte(
        DUNGEON_ELEMENT_CORRIDOR,
        DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);

    sensors[0].sensorType = 13;
    sensors[0].sensorData = 77;
    sensors[0].localEffect = 1;
    sensors[0].next = THING_ENDOFLIST;

    maps[0].level = 0;
    maps[0].width = MAP_W;
    maps[0].height = MAP_H;
    maps[1].level = 1;
    maps[1].width = MAP_W;
    maps[1].height = MAP_H;
    tiles[0].squareCount = MAP_W * MAP_H;
    tiles[0].squareData = map0;
    tiles[1].squareCount = MAP_W * MAP_H;
    tiles[1].squareData = map1;

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = MAP_COUNT;
    dungeon.maps = maps;
    dungeon.tiles = tiles;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = SFT_COUNT;
    things.sensors = sensors;
    things.sensorCount = 1;
    things.thingCounts[THING_TYPE_SENSOR] = 1;

    if (!F0881_WORLD_InitDefault_Compat(&world, 0x4242u)) {
        record("PIT_WORLD_INIT", 0, "F0881_WORLD_InitDefault_Compat failed");
        printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
        return 1;
    }
    world.dungeon = &dungeon;
    world.things = &things;
    world.ownsDungeon = 0;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 1;
    world.party.direction = DIR_EAST;
    world.party.championCount = 2;
    world.party.activeChampionIndex = 0;
    /*
     * Deliberately lethal fall: both champions have HP=20 so the
     * 20-point pit fall lands every living champion at HP=0, which
     * lets the F0889 alive==0 check flip partyDead and lets F0884
     * step 5 emit EMIT_PARTY_DEAD.  This is the downstream chain the
     * regression must prove deterministically.
     */
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 20;
    world.party.champions[0].hp.maximum = 20;
    world.party.champions[0].direction = DIR_EAST;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 20;
    world.party.champions[1].hp.maximum = 20;
    world.party.champions[1].direction = DIR_EAST;

    input.command = CMD_MOVE_EAST;
    record("PIT_ADVANCE_ONE_TICK",
           F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_PARTY_DEAD,
           "CMD_MOVE_EAST dispatched through F0884 -> F0888 -> F0889 chain (party dead early return path)");

    /*
     * Open-pit transition regressed: party lands on the lower map at
     * the same local coordinate, facing East, regardless of the
     * fall-killed champion state.
     */
    record("PIT_FINAL_PARTY_STATE",
           world.party.mapIndex == 1 &&
               world.party.mapX == 1 &&
               world.party.mapY == 1 &&
               world.party.direction == DIR_EAST,
           "party lands on lower map at the same local coordinate/facing");

    /*
     * Fall damage regressed: both champions take exactly 20 HP fall
     * damage and end up at HP=0.  This is the precondition for the
     * downstream F0889 alive==0 check.
     */
    record("PIT_FALL_DAMAGE_CHAMPION_0",
           world.party.champions[0].hp.current == 0,
           "open-pit transition drops champion 0 to HP=0 (20 dmg applied)");
    record("PIT_FALL_DAMAGE_CHAMPION_1",
           world.party.champions[1].hp.current == 0,
           "open-pit transition drops champion 1 to HP=0 (20 dmg applied)");

    fellIndex = find_emission(&result, EMIT_PARTY_FELL);
    record("PIT_FELL_EMISSION",
           fellIndex >= 0 &&
               result.emissions[fellIndex].payload[0] == 1 &&
               result.emissions[fellIndex].payload[1] == 1 &&
               result.emissions[fellIndex].payload[2] == 1 &&
               result.emissions[fellIndex].payload[3] == 1,
           "EMIT_PARTY_FELL reports map=1 / x=1 / y=1 and pitCount=1");

    /*
     * Downstream sensor consequence: the source map's WALK_OFF pass
     * and the destination map's WALK_ON pass are both executed by the
     * orchestrator, but only the landing WALK_ON sensor surfaces an
     * EMIT_SENSOR_EFFECT (F0710 returns zero effects for WALK_OFF in
     * the v1 sensor policy).
     */
    record("PIT_LANDING_SENSOR_COUNT",
           count_emissions(&result, EMIT_SENSOR_EFFECT) == 1,
           "only the lower-map landing sensor emits a consequence (WALK_OFF produces no effect in v1)");
    sensorIndex = find_emission(&result, EMIT_SENSOR_EFFECT);
    record("PIT_LANDING_SENSOR_PAYLOAD",
           sensorIndex >= 0 &&
               result.emissions[sensorIndex].payload[0] == SENSOR_EFFECT_SHOW_TEXT &&
               result.emissions[sensorIndex].payload[1] == 13 &&
               result.emissions[sensorIndex].payload[2] == SENSOR_EVENT_WALK_ON &&
               result.emissions[sensorIndex].payload[3] == 77,
           "landing sensor emits WALK_ON text effect with its text index");
    record("PIT_NO_TELEPORT_EMISSION",
           count_emissions(&result, EMIT_PARTY_TELEPORTED) == 0,
           "single open pit does not report a teleporter transition");
    record("PIT_PARTY_MOVED_EMISSION",
           count_emissions(&result, EMIT_PARTY_MOVED) == 1,
           "open-pit transition also surfaces the post-resolve EMIT_PARTY_MOVED");

    /*
     * Downstream timeline consequence: every living champion reached
     * HP=0 from the 20-point fall, F0889 saw alive==0, the orchestrator
     * flipped partyDead=1, and F0884 step 5 emitted EMIT_PARTY_DEAD
     * and returned ORCH_PARTY_DEAD.  The same tick therefore carries
     * the open-pit transition regressed row and the death-timeline
     * row together.
     */
    record("PIT_WORLD_PARTY_DEAD_FLAG",
           world.partyDead == 1,
           "world.partyDead flips to 1 after both champions fall to HP=0");
    partyDeadIndex = find_emission(&result, EMIT_PARTY_DEAD);
    record("PIT_PARTY_DEAD_EMISSION",
           partyDeadIndex >= 0,
           "F0884 step 5 emits EMIT_PARTY_DEAD once every living champion is killed by the pit fall");
    record("PIT_TICK_ADVANCED",
           world.gameTick == 1 && result.postTick == 1,
           "F0884 advances the game tick by one even on the party-dead early return path");
    record("PIT_NO_CHAMPION_DOWN_EMISSION_FROM_FALL",
           count_emissions(&result, EMIT_CHAMPION_DOWN) == 0,
           "EMIT_CHAMPION_DOWN is reserved for pendingCombat damage in v1; pit-fall death is signalled via EMIT_PARTY_DEAD instead");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
