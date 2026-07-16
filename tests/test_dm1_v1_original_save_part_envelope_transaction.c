#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int rejected_without_publish(const unsigned char *bytes, size_t size)
{
    struct GameWorld_Compat world;
    struct DM1_EventQueue_V1 events;
    DM1OriginalSavePC34HandoffReport report;

    memset(&world, 0, sizeof(world));
    memset(&events, 0, sizeof(events));
    memset(&report, 0, sizeof(report));
    world.gameTick = 0x11223344u;
    world.party.championCount = 3;
    world.party.mapIndex = 7;
    events.gameTick = 0x55667788u;
    events.eventCount = 2;
    report.original_game_time = 0x99aabbccu;
    return dm1_v1_original_save_pc34_handoff_load_world_from_bytes(
               bytes, size, &world, &events, &report) !=
               DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
           world.gameTick == 0x11223344u &&
           world.party.championCount == 3 && world.party.mapIndex == 7 &&
           events.gameTick == 0x55667788u && events.eventCount == 2 &&
           report.original_game_time == 0x99aabbccu;
}

int main(void)
{
    DM1OriginalSavePC34FixtureSpec spec;
    unsigned char bytes[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char malformed[SAVEGAME_PC34_MAX_FILE_SIZE];
    size_t size = 0u;

    memset(&spec, 0, sizeof(spec));
    spec.champion_count = 2;
    spec.map_index = 3;
    spec.map_x = 4;
    spec.map_y = 5;
    spec.direction = 1;
    spec.current_active_group_count = 1;
    spec.maximum_active_group_count = 2;
    spec.event_count = 1;
    spec.event_maximum_count = 2;
    spec.game_time = 77u;
    spec.game_id = 4u;
    if (dm1_v1_original_save_pc34_build_handoff_fixture_bytes(
            &spec, bytes, sizeof(bytes), &size) != SAVEGAME_PC34_OK ||
        size <= SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 2u ||
        !rejected_without_publish(bytes, size - 1u)) {
        return 1;
    }
    memcpy(malformed, bytes, size);
    /* ReDMCSB LOADSAVE.C F0435 reads the first F0417 part length before
     * decoding anything. A length beyond the supplied envelope must reject
     * transactionally rather than publishing a partial GLOBAL_DATA state. */
    malformed[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE] = 0xffu;
    malformed[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE + 1u] = 0xffu;
    if (!rejected_without_publish(malformed, size)) {
        return 1;
    }
    puts("ok: DM1 PC34 rejected part envelopes publish no partial runtime state");
    return 0;
}
