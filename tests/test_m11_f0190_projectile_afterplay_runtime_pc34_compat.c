/*
 * Source-lock regression for the M11 projectile F0190 killed-all afterplay.
 *
 * ReDMCSB GROUP.C F0188/F0189/F0190: the final group's possessions and
 * C29-C41 events are removed before F0190 creates and schedules C040 smoke.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(actual, expected, message) do { \
    int actual_ = (int)(actual); \
    int expected_ = (int)(expected); \
    if (actual_ == expected_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", \
                              (message), actual_, expected_); } \
} while (0)

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((unsigned short)type << 10) |
                            ((unsigned short)index & 0x03FFu));
}

static void test_projectile_killed_all_frees_timeline_slot_before_c040(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int smokeAdvanceCount = 0;
    int i;

    memset(&state, 0, sizeof(state));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 6;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 50;
    things.loaded = 1;
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;

    M11_GameView_Init(&state);
    state.active = 1;
    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.championCount = 1;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.gameTick = 100;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.creatureAI[0].reserved0 = 0;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->poisonAttack = 3;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    /* One F0181-deletable C29 at the killed group's square and 255 unrelated
     * entries fill the bounded timeline. C040 scheduling only succeeds when
     * F0189 runs before F0190's source explosion creation. */
    state.world.timeline.count = TIMELINE_QUEUE_CAPACITY;
    for (i = 0; i < TIMELINE_QUEUE_CAPACITY; ++i) {
        struct TimelineEvent_Compat* event = &state.world.timeline.events[i];
        memset(event, 0, sizeof(*event));
        event->kind = TIMELINE_EVENT_CREATURE_REACTION;
        event->fireAtTick = 200u + (unsigned int)i;
        event->mapIndex = 0;
        event->mapX = 2;
        event->mapY = 1;
        event->aux2 = 28;
    }
    state.world.timeline.events[0].mapX = 0;
    state.world.timeline.events[0].mapY = 0;
    state.world.timeline.events[0].aux2 =
        DM1_EVENT_REACTION_DANGER_ON_SQUARE;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(groups[0].next, THING_NONE,
              "F0189 returns the killed group record before C040");
    ASSERT_EQ(squareFirstThings[0], THING_ENDOFLIST,
              "F0189 unlinks the killed group before C040");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 0, 0), 1,
              "F0190 materializes one real C040 smoke record");
    ASSERT_EQ(state.world.timeline.count, TIMELINE_QUEUE_CAPACITY,
              "F0181 frees a full-queue slot before C040 scheduling");
    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind ==
            TIMELINE_EVENT_EXPLOSION_ADVANCE) {
            ++smokeAdvanceCount;
        }
    }
    ASSERT_EQ(smokeAdvanceCount, 1,
              "F0190 C040 advance is scheduled after group cleanup");
}

int main(void)
{
    printf("M11 F0190 projectile afterplay source-lock gate\n");
    printf("Source: ReDMCSB GROUP.C F0188/F0189/F0190\n\n");
    test_projectile_killed_all_frees_timeline_slot_before_c040();
    printf("\n--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
