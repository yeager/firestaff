#include "nexus_v1_game.h"

#include <stdio.h>

int main(void)
{
    Nexus_V1_DungeonStartReceipt receipt;

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
    puts("Nexus dungeon-start provenance: PASS");
    return 0;
}
