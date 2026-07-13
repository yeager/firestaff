#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *message)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    unsigned char original[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char reexported[SAVEGAME_PC34_MAX_FILE_SIZE];
    struct GameWorld_Compat world;
    DM1OriginalSavePC34RoundtripReport report;
    int written = 0;
    size_t reexported_size = 0u;
    int slot;
    size_t byte_index;
    int result;

    memset(&world, 0, sizeof(world));
    memset(&report, 0, sizeof(report));
    world.party.championCount = 2;
    world.party.mapIndex = 0;
    world.party.mapX = 11;
    world.party.mapY = 12;
    world.party.direction = 1;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        struct ChampionState_Compat *champion = &world.party.champions[slot];
        champion->present = slot < world.party.championCount;
        champion->portraitBitmapValid = 1;
        for (byte_index = 0u;
             byte_index < CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT; ++byte_index) {
            champion->portraitBitmap[byte_index] =
                (unsigned char)((byte_index + (size_t)slot * 37u) & 0xffu);
        }
    }

    /* This is F0433-shaped output from the real native exporter, not a
     * hand-built save fixture or a Firestaff-native save envelope. */
    result = F0802_SAVEGAME_ExportPC34FromWorld_Compat(
        &world, 0x504f5254u, original, (int)sizeof(original), &written);
    if (!check(result == SAVEGAME_PC34_OK && written > 0,
               "native PC34 export writes the portrait section")) {
        return 1;
    }

    result = dm1_v1_original_save_pc34_roundtrip_world_reload_bytes(
        original, (size_t)written, 0x504f5254u, reexported,
        sizeof(reexported), &reexported_size, &report);
    if (!check(result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
                   reexported_size > 0u &&
                   report.external_portrait_byte_receipt_available &&
                   report.source_external_portrait_byte_count ==
                       SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT &&
                   report.exported_external_portrait_byte_count ==
                       SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT &&
                   report.source_external_portrait_fingerprint != 0u &&
                   report.source_external_portrait_fingerprint ==
                       report.exported_external_portrait_fingerprint &&
                   report.external_portrait_byte_preservation_ok,
               "F0435 -> F0433 -> F0435 preserves all external portrait bytes")) {
        return 1;
    }

    puts("ok: PC34 external portrait receipt preserves raw F0433/F0435 bytes");
    return 0;
}
