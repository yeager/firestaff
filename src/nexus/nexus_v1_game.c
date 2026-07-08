
#include "nexus_v1_game.h"
#include "asset_find_by_hash.h"
#include <string.h>
#include <stdio.h>

static const char *const g_nexus_level_md5[16] = {
    "603ec9c531a92539babdda84ab09e78e",
    "751e1442bf7dccbd41bf146b5be144ab",
    "e2cb85d9fedc27f894a84e0f465fcde1",
    "19637d6b59849565f64565aed786d7ea",
    "85abc1b822e5c66ec4e99f1f676c140e",
    "ed5d54ab0ac1c927c1346dd966c8a5cc",
    "58c336ff6146e7216f0081e726823ea1",
    "c19e6038a017a320515ecbb66f6da197",
    "9bfc31bea631345a3660c2645be0e95b",
    "32a6450f29eb7babd73fcbe7a0310f22",
    "2928440e9c21457929f1323a28a42f70",
    "d7be5cd0d6e5c10afe99ec9950614fad",
    "db1cf70d6730615f73f191fad5e11e32",
    "f8876d0181d79727013236a6b597b99b",
    "a634dd5e95567ecbbbc332350c8cf12b",
    "5e6e237074f1e6b0decc629868a51f3c"
};

static int nexus_v1_file_exists(const char *path) {
    FILE *fp;
    if (!path || !path[0]) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

void nexus_v1_game_init(Nexus_V1_GameState *state, const char *data_dir) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 11;
    state->party_y = 29;
    state->party_dir = 0;  /* North — same as DM1 */
}

int nexus_v1_game_load_level(Nexus_V1_GameState *state, int level) {
    char path[512];
    if (!state || !state->data_dir || level < 0 || level > 15) return -1;

    if (asset_find_by_md5(state->data_dir,
                          g_nexus_level_md5[level],
                          path,
                          (int)sizeof(path),
                          8)) {
        snprintf(state->level_path, sizeof(state->level_path), "%s", path);
        state->current_level = level;
        printf("Nexus: loading level %d from %s\n", level, state->level_path);
        return 0;
    }

    snprintf(path, sizeof(path), "%s/LEV%02d.DGN", state->data_dir, level);
    if (!nexus_v1_file_exists(path)) {
        return -1;
    }
    snprintf(state->level_path, sizeof(state->level_path), "%s", path);
    state->current_level = level;
    printf("Nexus: loading level %d from %s\n", level, state->level_path);
    return 0;
}

/* Map dungeon levels to CD audio tracks (Track 2-9).
 * 8 audio tracks for 16 levels — each track covers 2 levels. */
int nexus_v1_cd_track_for_level(int level) {
    if (level < 0 || level > 15) return 2; /* default: track 2 */
    return 2 + (level / 2);  /* Track 2-9 */
}
