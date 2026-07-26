/* Focused native CSB F0435 -> M11 F9 provenance gate.
 * ReDMCSB LOADSAVE.C F0435 owns header validation before runtime restore. */

#include "csb_v1_boot.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
static int setenv(const char *name, const char *value, int overwrite) {
    if (!overwrite && getenv(name)) return 0;
    return _putenv_s(name, value);
}
static int unsetenv(const char *name) {
    return _putenv_s(name, "");
}
#endif
#include <unistd.h>

static int failures;

#define CHECK(condition, message) \
    do { if (!(condition)) { fprintf(stderr, "FAIL: %s\n", (message)); ++failures; } } while (0)

static int make_native_save(const char *data_dir, const char *save_path)
{
    CSB_V1_BootProfile profile;
    uint32_t game_time = 0u;
    int ok = 0;

    csb_v1_boot_profile_init(&profile);
    if (csb_v1_boot_scan_assets(&profile, data_dir) != 0 ||
        !profile.assets_verified || csb_v1_boot_enter_game(&profile) != 0) {
        goto done;
    }
    profile.runtime.party_x = CSB_V1_START_PARTY_X + 1;
    profile.runtime.party_y = CSB_V1_START_PARTY_Y + 1;
    profile.runtime.tick_count = 7u;
    profile.runtime.game_time = 7u;
    profile.runtime.party_state.PartyMapX = profile.runtime.party_x;
    profile.runtime.party_state.PartyMapY = profile.runtime.party_y;
    if (csb_v1_boot_runtime_save_game_to_path_pc34(
            &profile, save_path, &game_time) == CSB_V1_SAVE_OK &&
        game_time == 7u) {
        ok = 1;
    }
done:
    csb_v1_boot_cleanup(&profile);
    return ok;
}

static int corrupt_header_byte(const char *path)
{
    FILE *file = fopen(path, "r+b");
    int value;
    if (!file || fseek(file, 511L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    value = fgetc(file);
    if (value == EOF || fseek(file, 511L, SEEK_SET) != 0 ||
        fputc(value ^ 1, file) == EOF) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_DATA_DIR");
    char save_template[] = "/tmp/firestaff-csb-f0435-f9-XXXXXX";
    char *save_path;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    int tick_before;
    int save_fd;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_CSB_DATA_DIR is not set");
        return 0;
    }
    save_fd = mkstemp(save_template);
    save_path = save_fd >= 0 ? save_template : NULL;
    if (!save_path) {
        fputs("FAIL: unable to allocate native save path\n", stderr);
        return 1;
    }
    (void)close(save_fd);
    (void)remove(save_path);
    CHECK(make_native_save(data_dir, save_path),
          "known CSB corpus produces a native F0435 save");
    if (failures) goto done;

    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = data_dir;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    M11_GameView_Init(&view);
    CHECK(M11_GameView_Start(&view, &spec),
          "M11 opens the hash-verified CSB boot session");
    CHECK(setenv("FIRESTAFF_QUICKSAVE_PATH", save_path, 1) == 0,
          "focused F9 path is explicit");
    CHECK(M11_GameView_QuickLoad(&view),
          "M11 F9 accepts the native F0435 save");
    CHECK(view.csbOriginalSaveRuntimeReceiptRequired &&
              view.csbOriginalSaveRuntimeReceipt.valid &&
              csb_v1_boot_original_save_runtime_receipt_current_pc34(
                  (const CSB_V1_BootProfile *)view.csbBootProfile,
                  &view.csbOriginalSaveRuntimeReceipt),
          "M11 F9 binds a current native F0435 receipt");

    tick_before = view.csbState.tick_count;
    CHECK(corrupt_header_byte(save_path), "fixture corrupts native header");
    CHECK(!M11_GameView_QuickLoad(&view),
          "M11 F9 rejects a corrupted native F0435 header");
    CHECK(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_IGNORED &&
              view.csbState.tick_count == tick_before,
          "stale F0435 receipt blocks tick without runtime mutation");
    M11_GameView_Shutdown(&view);
done:
    (void)unsetenv("FIRESTAFF_QUICKSAVE_PATH");
    (void)remove(save_template);
    if (failures) {
        fprintf(stderr, "FAIL: csb_v1_m11_f0435_f9_reload (%d failures)\n", failures);
        return 1;
    }
    puts("PASS: csb_v1_m11_f0435_f9_reload");
    return 0;
}
