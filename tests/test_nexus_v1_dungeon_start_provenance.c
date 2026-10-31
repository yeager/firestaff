#include "nexus_v1_game.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_movement.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_retail_headings(const uint8_t *dm_bin, size_t dm_bin_size)
{
    static const struct {
        int level;
        uint16_t heading;
        int party_dir;
    } cases[] = {
        {1, 0x8000U, NEXUS_DIR_NORTH},
        {3, 0x4000U, NEXUS_DIR_EAST},
        {8, 0x0000U, -1},
        {11, 0xc000U, -1}
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        Nexus_V1_RetailStartEntry entry;
        memset(&entry, 0, sizeof(entry));
        entry.party_dir = -1;
        if (!nexus_v1_game_parse_retail_start_entry(
                dm_bin, dm_bin_size, cases[i].level, &entry) ||
            entry.retail_heading != cases[i].heading ||
            entry.party_dir != cases[i].party_dir) {
            fprintf(stderr,
                    "FAIL: DM.BIN level %d heading 0x%04x resolved as 0x%04x / %d\n",
                    cases[i].level, cases[i].heading,
                    entry.retail_heading, entry.party_dir);
            return 0;
        }
    }
    return 1;
}

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long file_size;
    uint8_t *bytes;

    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)file_size);
    if (!bytes || fread(bytes, 1U, (size_t)file_size, file) !=
                      (size_t)file_size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)file_size;
    return bytes;
}

int main(int argc, char **argv)
{
    Nexus_V1_DungeonStartReceipt receipt;
    Nexus_V1_GameState state;
    Nexus_V1_Engine engine;
    Nexus_V1_Level level;

    if (argc > 2) {
        fprintf(stderr, "usage: %s [authentic-DM.BIN]\n", argv[0]);
        return 2;
    }
    if (argc == 2) {
        size_t retail_size = 0;
        uint8_t *retail_dm_bin = read_file(argv[1], &retail_size);
        if (!retail_dm_bin) {
            fprintf(stderr, "FAIL: unable to read authentic DM.BIN: %s\n",
                    argv[1]);
            return 1;
        }
        if (!verify_retail_headings(retail_dm_bin, retail_size)) {
            free(retail_dm_bin);
            return 1;
        }
        free(retail_dm_bin);
        puts("Nexus authentic DM.BIN start headings: PASS");
        return 0;
    }

    if (nexus_v1_game_resolve_dungeon_start(NULL, 0, -1, -1, -1,
                                            &receipt) != 0 ||
        receipt.requested_dir != -1 || receipt.party_dir != -1 ||
        !receipt.blocks_runtime) {
        fprintf(stderr, "FAIL: unknown Saturn start direction was normalized\n");
        return 1;
    }
    if (nexus_v1_game_resolve_dungeon_start(NULL, 0, -1, -1, 2,
                                            &receipt) != 0 ||
        receipt.requested_dir != 2 || receipt.party_dir != 2) {
        fprintf(stderr, "FAIL: known start direction was not preserved\n");
        return 1;
    }
    memset(&level, 0, sizeof(level));
    level.width = 64;
    level.height = 64;
    level.squares[2][3] = 1;
    level.collision_refs[2][3] = 1;
    if (nexus_v1_game_resolve_dungeon_start(&level, 1, 2, 3, -1,
                                            &receipt) != 0 ||
        receipt.status != NEXUS_V1_DUNGEON_START_BLOCKED_DIRECTION ||
        receipt.blocks_runtime != 1 ||
        strcmp(nexus_v1_dungeon_start_status_name(receipt.status),
               "blocked-direction") != 0) {
        fprintf(stderr, "FAIL: unknown direction became a READY start\n");
        return 1;
    }
    nexus_v1_game_init(&state, "retail");
    receipt.status = NEXUS_V1_DUNGEON_START_READY;
    receipt.party_dir = -1;
    receipt.blocks_runtime = 0;
    receipt.fallback_visuals_permitted = 0;
    if (nexus_v1_game_apply_dungeon_start(&state, &receipt) != 0) {
        fprintf(stderr, "FAIL: READY receipt accepted unknown direction\n");
        return 1;
    }
    memset(&engine, 0, sizeof(engine));
    nexus_v1_sync_dgn_runtime_pose(&engine, NEXUS_V1_TITLE_LEVEL, -1, -1, -1);
    if (engine.game.party_dir != -1) {
        fprintf(stderr, "FAIL: unplaced runtime pose became a valid direction\n");
        return 1;
    }
    puts("Nexus dungeon-start provenance: PASS");
    return 0;
}
