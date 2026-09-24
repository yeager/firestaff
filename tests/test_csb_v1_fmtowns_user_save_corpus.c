#define _POSIX_C_SOURCE 200809L

/* FM Towns F0435 user-save corpus boundary.
 *
 * This is deliberately opt-in: it consumes only user-supplied licensed F31
 * media and two external candidate saves. ReDMCSB LOADSAVE.C F0435 first
 * accepts the C5 header and five F7057 parts, then F0434 consumes the F7063
 * dungeon stream from the same file handle. A candidate whose party pose
 * cannot be represented by that authenticated dungeon must not reach M11.
 */
#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_game.h"
#include "asset_status_m12.h"
#include "config_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

static uint32_t file_fnv1a(const char *path, uint32_t *out_size)
{
    FILE *file;
    uint32_t hash = UINT32_C(2166136261);
    uint32_t size = 0u;
    unsigned char bytes[4096];
    size_t count;

    if (out_size) *out_size = 0u;
    if (!path || !(file = fopen(path, "rb"))) return 0u;
    while ((count = fread(bytes, 1u, sizeof(bytes), file)) != 0u) {
        size_t index;
        if (size > UINT32_MAX - (uint32_t)count) {
            fclose(file);
            return 0u;
        }
        for (index = 0u; index < count; ++index) {
            hash ^= bytes[index];
            hash *= UINT32_C(16777619);
        }
        size += (uint32_t)count;
    }
    if (ferror(file) || fclose(file) != 0) return 0u;
    if (out_size) *out_size = size;
    return hash;
}

static void check_candidate(const char *data_dir, const char *corpus_dir,
                            const char *filename,
                            CSB_V1_FmtownsSwitchLanguage language,
                            CSB_V1_VariantId variant, int expected_valid)
{
    CSB_V1_BootStartupLaunch_PC34 launch;
    CSB_V1_FmtownsGameHandoffReceipt game;
    CSB_V1_FmtownsUserSaveReceipt save;
    CSB_V1_FmtownsStartupState state;
    char path[1024];
    uint32_t before_size;
    uint32_t before_hash;
    uint32_t after_size;
    uint32_t after_hash;

    if (snprintf(path, sizeof(path), "%s/%s", corpus_dir, filename) <= 0 ||
        strlen(path) >= sizeof(path)) {
        CHECK(0, "candidate path is bounded");
        return;
    }
    before_hash = file_fnv1a(path, &before_size);
    CHECK(before_hash != 0u && before_size >= 512u,
          "external F31 candidate is readable");
    if (before_hash == 0u || before_size < 512u) return;

    memset(&launch, 0, sizeof(launch));
    CHECK(csb_v1_boot_startup_launch_alloc_with_variant_pc34(
              data_dir, NULL, NULL, NULL, NULL, (int)variant, &launch) &&
              launch.profile && launch.profile->assets_verified &&
              launch.profile->variant_id == variant,
          "selected original F31 media opens before corpus admission");
    if (!launch.profile || !launch.profile->assets_verified ||
        launch.profile->variant_id != variant) {
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return;
    }
    memset(&game, 0, sizeof(game));
    CHECK(csb_v1_fmtowns_game_handoff_open(launch.profile, language, &game) &&
              game.valid && game.language == language,
          "selected original C03 game program owns the candidate language");
    memset(&save, 0, sizeof(save));
    if (expected_valid) {
        CHECK(csb_v1_fmtowns_game_user_save_open(launch.profile, &game, path,
                                                  &save) && save.valid,
              "coherent F31J F0435 candidate is admitted");
        memset(&state, 0, sizeof(state));
        CHECK(save.valid && csb_v1_fmtowns_game_load_user_save_state(
                                  &save, &state) && state.valid &&
                  state.party.PartyMapX == save.party_map_x &&
                  state.party.PartyMapY == save.party_map_y &&
                  state.party_map_index == save.party_map_index,
              "admitted F31J candidate materializes its authenticated state");
        csb_v1_fmtowns_game_startup_state_free(&state);
    } else {
        CHECK(!csb_v1_fmtowns_game_user_save_open(launch.profile, &game, path,
                                                   &save) && !save.valid,
              "incoherent F31E F0435 candidate remains fail-closed");
    }
    after_hash = file_fnv1a(path, &after_size);
    CHECK(after_hash == before_hash && after_size == before_size,
          "rejected F31 candidate remains byte-identical");
    csb_v1_boot_startup_launch_cleanup_pc34(&launch);
}

static void check_launcher_quick_resume(const char *data_dir,
                                        const char *corpus_dir)
{
    char home_template[1024];
    char cwd[512];
    const char* cwd_leaf;
    char config_dir[1024];
    char save_path[1024];
    M12_Config config;
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;

    if (!getcwd(cwd, sizeof(cwd))) {
        CHECK(0, "test working directory is readable");
        return;
    }
    cwd_leaf = strrchr(cwd, '/');
    cwd_leaf = cwd_leaf ? cwd_leaf + 1 : cwd;
    if (snprintf(home_template, sizeof(home_template),
                 "%s%s/firestaff-fmtowns-quick-resume-%ld-%u", cwd,
                 strcmp(cwd_leaf, "build") == 0 ? "" : "/build",
                 (long)getpid(), (unsigned int)rand()) <= 0 ||
        mkdir(home_template, 0700) != 0) {
        CHECK(0, "isolated launcher HOME is created in the build directory");
        return;
    }
    CHECK(setenv("HOME", home_template, 1) == 0,
          "launcher test redirects configuration to its private HOME");
    if (snprintf(config_dir, sizeof(config_dir), "%s/.firestaff",
                 home_template) <= 0 ||
        mkdir(config_dir, 0700) != 0 ||
        snprintf(save_path, sizeof(save_path), "%s/CSBGAME-JP.DAT",
                 corpus_dir) <= 0) {
        CHECK(0, "private launcher config and save paths are prepared");
        return;
    }
    M12_Config_Load(&config, data_dir);
    snprintf(config.dataDir, sizeof(config.dataDir), "%s", data_dir);
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s", save_path);
    config.gameArchitectureIndex[1] = M12_ARCH_FM_TOWNS;
    config.gameVersionIndex[1] = M12_AssetStatus_FindVersionIndex(
        "csb", "fmtowns-ja");
    M12_Config_Save(&config);
    M12_StartupMenu_InitWithDataDir(&menu, data_dir, "csb");
    CHECK(menu.quickResumeAvailable &&
              strcmp(menu.quickResumeGameId, "csb") == 0 &&
              strcmp(menu.quickResumeSavePath, save_path) == 0,
          "M12 offers the authentic language-matched F31J save as Quick Resume");
    menu.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    CHECK(intent.valid && intent.gameId && strcmp(intent.gameId, "csb") == 0 &&
              intent.savePath && strcmp(intent.savePath, save_path) == 0,
          "M12 Quick Resume carries the exact F31J save path to M11");

    M12_Config_Load(&config, data_dir);
    snprintf(config.lastSavePath, sizeof(config.lastSavePath), "%s/CSBGAME.DAT",
             corpus_dir);
    config.gameVersionIndex[1] = M12_AssetStatus_FindVersionIndex(
        "csb", "fmtowns-en");
    M12_Config_Save(&config);
    M12_StartupMenu_InitWithDataDir(&menu, data_dir, "csb");
    CHECK(!menu.quickResumeAvailable,
          "M12 continues to reject the incoherent F31E candidate");
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR");
    const char *loose_data_dir =
        getenv("FIRESTAFF_CSB_FMTOWNS_LOOSE_DATA_DIR");
    const char *corpus_dir = getenv("FIRESTAFF_CSB_FMTOWNS_SAVE_CORPUS_DIR");
    char english_data_dir[M12_ASSET_DATA_DIR_CAPACITY];
    char japanese_data_dir[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus asset_status;

    if ((!data_dir || !data_dir[0]) &&
        (!loose_data_dir || !loose_data_dir[0])) {
        printf("SKIP: set FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR or "
               "FIRESTAFF_CSB_FMTOWNS_LOOSE_DATA_DIR, and "
               "FIRESTAFF_CSB_FMTOWNS_SAVE_CORPUS_DIR\n");
        return 77;
    }
    if (!corpus_dir || !corpus_dir[0]) {
        printf("SKIP: set FIRESTAFF_CSB_FMTOWNS_SAVE_CORPUS_DIR\n");
        return 77;
    }
    if (loose_data_dir && loose_data_dir[0]) {
        memset(&asset_status, 0, sizeof(asset_status));
        memset(english_data_dir, 0, sizeof(english_data_dir));
        memset(japanese_data_dir, 0, sizeof(japanese_data_dir));
        M12_AssetStatus_ScanGame(&asset_status, loose_data_dir, "csb");
        if (!M12_AssetStatus_MaterializeCSBRuntimeVersion(
                &asset_status, "fmtowns-en", english_data_dir,
                sizeof(english_data_dir)) ||
            !M12_AssetStatus_MaterializeCSBRuntimeVersion(
                &asset_status, "fmtowns-ja", japanese_data_dir,
                sizeof(japanese_data_dir))) {
            printf("SKIP: verified English and Japanese F31 packages are "
                   "unavailable under FIRESTAFF_CSB_FMTOWNS_LOOSE_DATA_DIR\n");
            return 77;
        }
        check_candidate(english_data_dir, corpus_dir, "CSBGAME.DAT",
                        CSB_FMTOWNS_SWITCH_ENGLISH,
                        CSB_V1_VARIANT_FMTOWNS_EN, 0);
        check_candidate(japanese_data_dir, corpus_dir, "CSBGAME-JP.DAT",
                        CSB_FMTOWNS_SWITCH_JAPANESE,
                        CSB_V1_VARIANT_FMTOWNS_JA, 1);
        return failures == 0 ? 0 : 1;
    }
    check_candidate(data_dir, corpus_dir, "CSBGAME.DAT",
                    CSB_FMTOWNS_SWITCH_ENGLISH, CSB_V1_VARIANT_FMTOWNS_EN, 0);
    check_candidate(data_dir, corpus_dir, "CSBGAME-JP.DAT",
                    CSB_FMTOWNS_SWITCH_JAPANESE, CSB_V1_VARIANT_FMTOWNS_JA, 1);
    check_launcher_quick_resume(data_dir, corpus_dir);
    return failures == 0 ? 0 : 1;
}
