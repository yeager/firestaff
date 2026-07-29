#include "csb_v1_atari_save_runtime_handoff_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *fp; long n; uint8_t *p = NULL;
    if (!path || !(fp = fopen(path, "rb"))) return 0;
    if (fseek(fp, 0, SEEK_END) || (n = ftell(fp)) < 1 || fseek(fp, 0, SEEK_SET) ||
        !(p = (uint8_t *)malloc((size_t)n)) || fread(p, 1u, (size_t)n, fp) != (size_t)n) {
        free(p); fclose(fp); return 0;
    }
    fclose(fp); *out = p; *out_size = (size_t)n; return 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_ATARI_MINI");
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_PartyState party;
    CSB_V1_AtariSaveInfo info;
    uint8_t *bytes = NULL;
    size_t size = 0u;

    csb_v1_runtime_init(&runtime, NULL);
    if (csb_v1_atari_save_handoff_runtime_pc34_compat(&runtime, NULL, 0u, &info) !=
        CSB_V1_ATARI_RUNTIME_ERR_NULL) return 1;
    if (!path || !path[0]) { puts("SKIP: FIRESTAFF_CSB_ATARI_MINI is not set"); return 0; }
    memset(&party, 0, sizeof(party));
    party.ChampionCount = 1; party.LeaderIndex = 0; party.Champions[0].CurrentHealth = 100;
    party.Champions[0].MaximumHealth = 100;
    if (csb_v1_runtime_set_party_state(&runtime, &party) != 0 || !read_file(path, &bytes, &size) ||
        csb_v1_atari_save_handoff_runtime_pc34_compat(&runtime, bytes, size, &info) != 0 ||
        runtime.dungeon_handle == NULL || runtime.level_count != 11 || runtime.current_level != 4 ||
        runtime.party_x != 22 || runtime.party_y != 18 || runtime.party_dir != 2 ||
        runtime.game_time != 19u || runtime.party_state.PartyMapX != 22 ||
        csb_v1_dungeon_get_current() != runtime.dungeon_handle) {
        free(bytes); csb_v1_runtime_cleanup(&runtime); return 1;
    }
    free(bytes); csb_v1_runtime_cleanup(&runtime);
    puts("PASS: original MINI.DAT hands its authenticated dungeon and pose to runtime");
    return 0;
}
