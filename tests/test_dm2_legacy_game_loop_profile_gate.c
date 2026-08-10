/*
 * Direct legacy-loop DM2 boot-profile gate.
 *
 * DM2 has already admitted G1/GDAT through dm2_v1_boot_enter_game() before
 * fs_game_load_assets() is reached.  The legacy direct loop must therefore
 * not send those bytes through the generic DM1 PC34 loader.
 */
#include "firestaff_game_loop.h"
#include "dm2_v1_boot.h"
#include "firestaff_asset_pipeline.h"

#include <stdio.h>
#include <string.h>

static int failures;

/* The test exercises only the early DM2 gate.  These legacy DM1-loader
 * symbols must remain unreachable; stubs keep the standalone translation
 * unit linkable without admitting any substitute game data. */
int fs_assets_load_game(FS_AssetBundle *bundle, const char *data_dir,
                        const char *game_subdir)
{
    (void)bundle;
    (void)data_dir;
    (void)game_subdir;
    return -1;
}

void fs_assets_free(FS_AssetBundle *bundle) { (void)bundle; }
int fs_dungeon_load_dat(const uint8_t *data, int size)
{
    (void)data;
    (void)size;
    return -1;
}
void fs_dungeon_set_level(int level) { (void)level; }
int fs_dungeon_get_start_x(void) { return 0; }
int fs_dungeon_get_start_y(void) { return 0; }
int fs_dungeon_get_start_dir(void) { return 0; }
void fs_dm1_get_full_palette(uint32_t *out256)
{
    if (out256) memset(out256, 0, 256U * sizeof(*out256));
}

static void expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    FS_GameState state;
    DM2_V1_BootProfile profile;

    memset(&state, 0, sizeof(state));
    memset(&profile, 0, sizeof(profile));
    state.config.game = FS_GAME_DM2;
    state.config.data_dir = "/this/path/must/not/be/read/by-the-dm1-loader";
    state.running = 1;
    profile.assets_verified = 1;
    profile.dm2_state = &profile;
    profile.dungeon_data = &profile;
    profile.graphics_dat = &profile;
    state.dm2_boot = &profile;

    expect_true(fs_game_load_assets(&state) == 0,
                "verified DM2 boot profile bypasses the generic asset loader");
    expect_true(state.running == 1,
                "verified DM2 profile leaves the direct loop running");
    expect_true(state.current_level == 0 && state.party_x == 0 &&
                    state.party_y == 0 && state.party_direction == 0,
                "DM2 gate does not publish a generic party mirror");

    profile.graphics_dat = NULL;
    state.running = 1;
    expect_true(fs_game_load_assets(&state) < 0,
                "incomplete DM2 boot profile is rejected before generic loading");
    expect_true(state.running == 0,
                "rejected DM2 profile stops the direct loop");
    expect_true(strcmp(state.last_error.message, "DM2 original game load failed") == 0,
                "rejected profile reports a DM2-specific error");

    if (failures) {
        fprintf(stderr, "DM2 legacy game-loop profile gate: %d failure(s)\n", failures);
        return 1;
    }
    printf("DM2 legacy game-loop profile gate: PASS\n");
    return 0;
}
