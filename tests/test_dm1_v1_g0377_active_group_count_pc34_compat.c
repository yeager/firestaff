/*
 * G0377_ui_CurrentActiveGroupCount regression.
 *
 * ReDMCSB GROUP.C owns this live ACTIVE_GROUP count; the PC 3.4
 * GLOBAL_DATA field is uint16_t. Firestaff keeps the bounded live count in
 * GameWorld_Compat.creatureAICount and must export the same current prefix.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", label); \
        return 1; \
    } \
} while (0)

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat group;
    struct SaveGame_Compat imported;
    struct PartyState_Compat imported_party;
    struct TimelineQueue_Compat imported_timeline;
    DM1OriginalSavePC34HandoffReport report;
    unsigned char square = 0x10;
    unsigned short square_first_thing;
    unsigned char save[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int result;

    _Static_assert(sizeof(world.creatureAICount) == sizeof(int32_t),
                   "Firestaff live active-group count remains int32_t");

    memset(&world, 0xa5, sizeof(world));
    CHECK(F0196_DM1_GROUP_InitializeActiveGroups_Compat(&world),
          "F0196 initializes active-group storage");
    CHECK(world.creatureAICount == 0,
          "F0196 resets G0377 current active-group count");

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&group, 0, sizeof(group));
    memset(&imported, 0, sizeof(imported));
    memset(&imported_party, 0, sizeof(imported_party));
    memset(&imported_timeline, 0, sizeof(imported_timeline));
    memset(&report, 0, sizeof(report));

    map.width = 1;
    map.height = 1;
    tiles.squareData = &square;
    tiles.squareCount = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;

    square_first_thing = thing_ref(THING_TYPE_GROUP, 0);
    group.next = THING_ENDOFLIST;
    group.creatureType = CREATURE_TYPE_SKELETON;
    group.cells = 0xff;
    group.count = 1;
    group.direction = 2;
    things.squareFirstThings = &square_first_thing;
    things.squareFirstThingCount = 1;
    things.groups = &group;
    things.groupCount = 1;
    things.loaded = 1;

    world.dungeon = &dungeon;
    world.things = &things;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    CHECK(F0720_TIMELINE_Init_Compat(&world.timeline, 0),
          "timeline initializes for active-group mutation");
    CHECK(F0195_DM1_GROUP_AddAllActiveGroups_Compat(&world) == 1,
          "F0195 admits one current-map group");
    CHECK(world.creatureAICount == 1,
          "F0195 mutates G0377 exactly once");

    result = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x47303737u, save, (int)sizeof(save), &written);
    CHECK(result == SAVEGAME_PC34_OK && written > 0,
          "PC34 export accepts the bounded G0377 live count");

    imported.party = &imported_party;
    imported.timeline = &imported_timeline;
    result = dm1_v1_original_save_pc34_handoff_bytes(
        save, (size_t)written, &imported, &report);
    CHECK(result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "PC34 handoff reads exported GLOBAL_DATA");
    CHECK(report.original_current_active_group_count == 1 &&
          report.original_maximum_active_group_count == 1 &&
          report.decoded_active_group_count == 1,
          "uint16 GLOBAL_DATA G0377 and ACTIVE_GROUP prefix round-trip");

    puts("PASS: G0377 current active-group count lifecycle");
    return 0;
}
