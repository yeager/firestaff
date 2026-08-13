#include "nexus_v1_game.h"
#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_DungeonStartReceipt receipt;
    Nexus_V1_GameState state;
    Nexus_V1_Engine engine;
    Nexus_V1_Level level;

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
