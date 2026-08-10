/* Real PC3.4 Prison handoff regression.
 *
 * A completed ENTRANCE.C receipt is not sufficient evidence that the live
 * M11 page consumed PANEL.C's C013 movement raster.  Exercise the same
 * title -> Prison command used by the executable probe and require a real
 * GRAPHICS.DAT C013 pixel to reach its native screen rectangle.
 */

#include "m11_game_view.h"
#include "asset_loader_m11.h"
#include "csb_v1_boot.h"
#include "csb_v1_csbwin_layout_0232.h"
#include "csb_v1_atari_save_runtime_handoff_pc34_compat.h"
#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"
#include "gamepad_config_m12.h"
#include "main_loop_m11.h"
#include "fs_gesture_navigation_gate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", (message)); \
            ++failures; \
        } \
    } while (0)

/* The disposable path is deliberately published only after the real PC3.4
 * title/Prison boot has completed.  `FIRESTAFF_QUICKSAVE_PATH` is consumed
 * by the F0433/F0435 menu route, not by the original TITLE.C/ENTRANCE.C
 * launch transaction.  Keeping it out of the process environment during
 * boot prevents an old user override from changing this real-data ingress
 * proof. */
static int set_test_quicksave_path(const char *path)
{
    if (!path || !path[0]) return 0;
#ifdef _WIN32
    return _putenv_s("FIRESTAFF_QUICKSAVE_PATH", path) == 0;
#else
    return setenv("FIRESTAFF_QUICKSAVE_PATH", path, 1) == 0;
#endif
}

static const char *csb_data_dir(char *fallback, size_t fallback_size)
{
    const char *path = getenv("FIRESTAFF_CSB_DATA_DIR");
    const char *home;

    if (path && path[0]) return path;
    home = getenv("HOME");
    if (!home || !home[0] || !fallback || fallback_size == 0u) return NULL;
    snprintf(fallback, fallback_size, "%s/.firestaff/data/csb", home);
    return fallback;
}

static int is_original_atari_save_file(const char *path)
{
    FILE *file;
    long length;
    unsigned char *bytes = NULL;
    CSB_V1_AtariSaveInfo info;
    int valid;

    if (!path || !(file = fopen(path, "rb"))) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (unsigned char *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    valid = csb_v1_atari_save_decode_pc34_compat(bytes, (size_t)length,
                                                   &info) == CSB_V1_ATARI_SAVE_OK;
    free(bytes);
    return valid;
}

static int read_original_atari_save_info(const char *path,
                                         CSB_V1_AtariSaveInfo *out_info)
{
    FILE *file;
    long length;
    unsigned char *bytes = NULL;
    int valid;

    if (!path || !out_info || !(file = fopen(path, "rb"))) return 0;
    memset(out_info, 0, sizeof(*out_info));
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (unsigned char *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    valid = csb_v1_atari_save_decode_pc34_compat(bytes, (size_t)length,
                                                   out_info) == CSB_V1_ATARI_SAVE_OK;
    free(bytes);
    return valid;
}

/* CSBWin's C232 record owns the Atari ST HUD page.  Compare every source
 * row after M11 has drawn it, rather than accepting a merely non-black
 * lower screen.  This keeps the real-data capture tied to the exact
 * package-selected layout and graphic records. */
static int check_atari_st_c232_hud_frame(const char *graphics_path,
                                         const unsigned char *framebuffer)
{
    CSB_V1_CSBWinLayout0232 layout;
    CSB_V1_CSBWinHudMaterialPlan0232 plan;
    unsigned char expected[320 * 200];
    unsigned char covered[320 * 200];
    size_t index;
    int matched = 0;

    if (!graphics_path || !graphics_path[0] || !framebuffer ||
        !csb_v1_csbwin_layout_0232_read_graphics_dat(graphics_path, &layout) ||
        !csb_v1_csbwin_layout_0232_build_hud_material_plan(&layout, &plan)) {
        return 0;
    }
    memset(expected, 0, sizeof(expected));
    memset(covered, 0, sizeof(covered));
    for (index = 0u; index < plan.count; ++index) {
        const CSB_V1_CSBWinHudMaterial0232 *entry = &plan.entries[index];
        CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;
        unsigned char *source = NULL;
        int source_width = 0;
        int source_height = 0;
        int destination_width = entry->destination.x2 - entry->destination.x1 + 1;
        int destination_height = entry->destination.y2 - entry->destination.y1 + 1;
        int row;

        memset(&receipt, 0, sizeof(receipt));
        if (!csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                graphics_path, entry->graphic_index, &source, &source_width,
                &source_height, &receipt) || !receipt.valid || !source ||
            entry->source_x > (uint16_t)source_width ||
            destination_width > source_width - (int)entry->source_x ||
            destination_height > source_height) {
            free(source);
            return 0;
        }
        for (row = 0; row < destination_height; ++row) {
            const unsigned char *source_row = source +
                (size_t)row * (size_t)source_width + entry->source_x;
            unsigned char *expected_row = expected +
                (size_t)(entry->destination.y1 + row) * 320u +
                entry->destination.x1;
            unsigned char *covered_row = covered +
                (size_t)(entry->destination.y1 + row) * 320u +
                entry->destination.x1;
            memcpy(expected_row, source_row, (size_t)destination_width);
            memset(covered_row, 1, (size_t)destination_width);
        }
        free(source);
        ++matched;
    }
    for (index = 0u; index < sizeof(expected); ++index) {
        if (covered[index] && framebuffer[index] != expected[index]) return 0;
    }
    return plan.valid && matched == CSB_V1_CSBWIN_LAYOUT_0232_HUD_MATERIAL_COUNT;
}

int main(void)
{
    char fallback[1024];
    const char *data_dir = csb_data_dir(fallback, sizeof(fallback));
    const char *csbwin_data_dir = getenv("FIRESTAFF_CSBWIN_DATA_DIR");
    const char *mode_text = getenv("FIRESTAFF_CSB_PRESENTATION_MODE");
    const char *atari_mini = getenv("FIRESTAFF_CSB_ATARI_MINI");
    const char *csbwin_graphics = getenv("FIRESTAFF_CSBWIN_GRAPHICS");
    const char *quicksave_path = getenv("FIRESTAFF_CSB_TEST_QUICKSAVE_PATH");
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    unsigned char framebuffer[320 * 200];
    int tick;
    int x;
    int y;
    int c013_nonblack = 0;
    int viewport_nonblack = 0;
    int relaunch_save_available = 0;
    uint32_t relaunch_game_time = 0u;
    int relaunch_level = -1;
    int relaunch_x = -1;
    int relaunch_y = -1;
    int relaunch_dir = -1;
    const M11_AssetSlot *c013;
    unsigned char *decoded = NULL;
    int decoded_w = 0;
    int decoded_h = 0;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 decode_receipt;

    if (csbwin_data_dir && csbwin_data_dir[0]) {
        static const struct {
            unsigned int graphic;
            int width;
            int height;
            const char *name;
        } source_hud_graphics[] = {
            { 9u, 96, 33, "C009 CSBWin spell background" },
            { 10u, 96, 45, "C010 CSBWin action panel" },
            { 13u, 96, 45, "C013 CSBWin movement panel" },
            { 17u, 224, 136, "C017 normal HUD panel" },
            { 28u, 80, 14, "C028 CSBWin champion direction strip" }
        };
        const CSB_V1_StartupRuntimeAssetSession_PC34 *session;
        char csbwin_graphics_path[1200];
        int nonblack = 0;
        size_t graphic_index;

        snprintf(csbwin_graphics_path, sizeof(csbwin_graphics_path),
                 "%s/graphics.dat", csbwin_data_dir);
        for (graphic_index = 0u;
             graphic_index < sizeof(source_hud_graphics) /
                 sizeof(source_hud_graphics[0]);
             ++graphic_index) {
            unsigned char *hud_pixels = NULL;
            int hud_width = 0;
            int hud_height = 0;
            int decoded;
            CSB_V1_StartupGraphicDecodeReceipt_PC34 hud_receipt;

            memset(&hud_receipt, 0, sizeof(hud_receipt));
            decoded = csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                csbwin_graphics_path, source_hud_graphics[graphic_index].graphic,
                &hud_pixels, &hud_width, &hud_height, &hud_receipt);
            CHECK(decoded && hud_receipt.valid &&
                      hud_width == source_hud_graphics[graphic_index].width &&
                      hud_height == source_hud_graphics[graphic_index].height,
                  source_hud_graphics[graphic_index].name);
            free(hud_pixels);
        }

        memset(&spec, 0, sizeof(spec));
        spec.title = "CHAOS STRIKES BACK";
        spec.gameId = "csb";
        spec.sourceId = "csb";
        spec.dataDir = csbwin_data_dir;
        spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
        spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
        spec.presentationWidth = 320;
        spec.presentationHeight = 200;
        M11_GameView_Init(&view);
        CHECK(M11_GameView_Start(&view, &spec),
              "CSBWin standard GRAPHICS.DAT opens its original C001-C005 startup route");
        session = (const CSB_V1_StartupRuntimeAssetSession_PC34 *)
            view.csbStartupRuntimeAssetSession;
        CHECK(session && session->valid && session->full_startup_ready &&
                  !session->hud_assets_bound &&
                  !view.csbStartupReleaseAppCaptureReceipt.valid &&
                  view.csbState.startup_title_active,
              "CSBWin standard route stays startup-only without inventing PC3.4 C040 HUD ownership");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        for (y = 0; y < 200; ++y) {
            for (x = 0; x < 320; ++x) {
                if (framebuffer[y * 320 + x] != 0u) ++nonblack;
            }
        }
        CHECK(nonblack > 0,
              "CSBWin standard route presents decoded original startup pixels");
        M11_GameView_Shutdown(&view);
        if (failures) return 1;
        puts("PASS: CSBWin standard C001-C005 startup-only route");
        return 0;
    }

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: no CSB PC3.4 data directory");
        return 0;
    }
    if (csbwin_graphics && csbwin_graphics[0]) {
        memset(&decode_receipt, 0, sizeof(decode_receipt));
        CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                  csbwin_graphics, 1u, &decoded, &decoded_w, &decoded_h,
                  &decode_receipt) && decode_receipt.valid &&
                  decoded_w == 320 && decoded_h == 200,
              "CSBWin standard GRAPHICS.DAT decodes C001 title source");
        free(decoded);
        decoded = NULL;
        decoded_w = 0;
        decoded_h = 0;
        memset(&decode_receipt, 0, sizeof(decode_receipt));
        CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                  csbwin_graphics, 2u, &decoded, &decoded_w, &decoded_h,
                  &decode_receipt) && decode_receipt.valid &&
                  decoded_w == 128 && decoded_h == 161,
              "CSBWin standard GRAPHICS.DAT decodes C002 left-door source");
        free(decoded);
        decoded = NULL;
        decoded_w = 0;
        decoded_h = 0;
        memset(&decode_receipt, 0, sizeof(decode_receipt));
        CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                  csbwin_graphics, 3u, &decoded, &decoded_w, &decoded_h,
                  &decode_receipt) && decode_receipt.valid &&
                  decoded_w == 128 && decoded_h == 161,
              "CSBWin standard GRAPHICS.DAT decodes C003 right-door source");
        free(decoded);
        decoded = NULL;
        decoded_w = 0;
        decoded_h = 0;
        memset(&decode_receipt, 0, sizeof(decode_receipt));
        CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                  csbwin_graphics, 4u, &decoded, &decoded_w, &decoded_h,
                  &decode_receipt) && decode_receipt.valid &&
                  decoded_w == 320 && decoded_h == 200,
              "CSBWin standard GRAPHICS.DAT decodes C004 entrance source");
        free(decoded);
        decoded = NULL;
        decoded_w = 0;
        decoded_h = 0;
        memset(&decode_receipt, 0, sizeof(decode_receipt));
        CHECK(csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
                  csbwin_graphics, 5u, &decoded, &decoded_w, &decoded_h,
                  &decode_receipt) && decode_receipt.valid &&
                  decoded_w == 320 && decoded_h == 200,
              "CSBWin standard GRAPHICS.DAT decodes C005 credits source");
        free(decoded);
        decoded = NULL;
        decoded_w = 0;
        decoded_h = 0;
        if (failures) return 1;
    }
    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = data_dir;
    spec.savePath = atari_mini && atari_mini[0] ? atari_mini : NULL;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = mode_text && mode_text[0]
        ? atoi(mode_text) : M12_PRESENTATION_V1_ORIGINAL;
    if (spec.presentationMode < M12_PRESENTATION_V1_ORIGINAL ||
        spec.presentationMode > M12_PRESENTATION_V22_MODERN) {
        fputs("SKIP: invalid CSB presentation mode\n", stderr);
        return 0;
    }
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;

    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        puts("SKIP: verified CSB PC3.4 data is unavailable");
        return 0;
    }
    if (spec.savePath) {
        const CSB_V1_BootProfile *profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        CHECK(profile && profile->runtime.party_state.ChampionCount == 1 &&
                  strcmp(profile->runtime.party_state.Champions[0].Name,
                         "HALK") == 0 &&
                  profile->runtime.party_x == 22 &&
                  profile->runtime.party_y == 18 &&
                  profile->runtime.party_dir == 2 &&
                  profile->runtime.current_level == 4,
              "real Atari MINI.DAT restores its party pose and champion into M11 CSB runtime");
            CHECK(view.world.party.championCount == 1 &&
                  memcmp(view.world.party.champions[0].name, "HALK", 4u) == 0 &&
                  view.world.party.mapX == 22 && view.world.party.mapY == 18 &&
                  view.world.party.direction == 2 && view.world.party.mapIndex == 4,
              "M11 party mirror consumes the real Atari MINI.DAT champion state");
        /* Atari ST executes ANIM.C before it transfers to FTLCODE. MINI.DAT
         * supplies the restored GAMEBLOCK but does not replace that original
         * program boundary with the PC3.4 C013/C017 HUD route. ReDMCSB
         * ANIM.C:67-94 opens ANIMATE.DAT/SCR and names FTLCODE; STARTUP1.C:
         * 162-168 likewise enters through the source startup loop before
         * F0435 LOADSAVE succeeds. */
        if (profile && (profile->variant_id == CSB_V1_VARIANT_ST20_EN ||
                        profile->variant_id == CSB_V1_VARIANT_ST21_EN)) {
            for (tick = 0; tick < 800 && view.csbState.startup_title_active;
                 ++tick) {
                (void)M11_GameView_AdvanceIdleTick(&view);
            }
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            for (y = 33; y < 169; ++y) {
                for (x = 48; x < 272; ++x) {
                    if (framebuffer[y * 320 + x] != 0u) {
                        ++viewport_nonblack;
                    }
                }
            }
            CHECK(!view.csbState.startup_title_active &&
                      view.csbAtariStRuntimeHandoffComplete &&
                      view.csbState.level_loaded,
                  "real Atari MINI.DAT crosses the original ANIM.C FTLCODE handoff");
            CHECK(viewport_nonblack > 0,
                  "real Atari MINI.DAT consumes source-owned Atari ST viewport material");
            CHECK(check_atari_st_c232_hud_frame(profile->graphics_path,
                                                 framebuffer),
                  "real Atari MINI.DAT framebuffer contains all C232-owned HUD material");
            CHECK(M11_GameView_HandleInput(
                      &view, M12_MENU_INPUT_CHAMPION_1_INVENTORY) ==
                      M11_GAME_INPUT_REDRAW &&
                      view.world.party.championCount == 1 &&
                      view.inventoryPanelActive,
                  "real Atari MINI.DAT F1 opens the GAMEBLOCK champion inventory");
            CHECK(M11_GameView_HandleInput(
                      &view, M12_MENU_INPUT_CHAMPION_1_INVENTORY) ==
                      M11_GAME_INPUT_REDRAW && !view.inventoryPanelActive,
                  "real Atari MINI.DAT F1 closes inventory through the same GAMEBLOCK route");
            /* CSB 2.x Atari's G0447 shifts the left edge of each visible
             * C007..C010 bar-graph toggle one pixel right (44, 113, 182,
             * 251). Pixel 43 is the inert seam after C012's adjacent status
             * surface, not part of C007. ReDMCSB COMMAND.C:92-100. */
            CHECK(M11_GameView_HandlePointerButton(
                      &view, 43, 14, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_IGNORED && !view.inventoryPanelActive,
                  "real Atari MINI.DAT keeps the C007 left-edge gap out of inventory input");
            CHECK(M11_GameView_HandlePointerButton(
                      &view, 44, 14, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW && view.inventoryPanelActive &&
                      M11_GameView_HandlePointerButton(
                          &view, 44, 14, M11_DM1_MOUSE_MASK_LEFT) ==
                      M11_GAME_INPUT_REDRAW && !view.inventoryPanelActive,
                  "real Atari MINI.DAT uses the native C007 bar-graph input box");
            /* Atari ST's G0447 additionally owns the complete C007 tile
             * with the right button.  That route is distinct from the
             * narrow left C007 bar and must not fall into M11's PC HUD
             * compatibility geometry. ReDMCSB COMMAND.C:97-100. */
            CHECK(M11_GameView_HandlePointerButton(
                      &view, 0, 14, M11_DM1_MOUSE_MASK_RIGHT) ==
                      M11_GAME_INPUT_REDRAW && view.inventoryPanelActive &&
                      M11_GameView_HandlePointerButton(
                          &view, 0, 14, M11_DM1_MOUSE_MASK_RIGHT) ==
                      M11_GAME_INPUT_REDRAW && !view.inventoryPanelActive,
                  "real Atari MINI.DAT uses the native right-button C007 tile");
            CHECK(M11_GameView_HandlePointerButton(
                      &view, 67, 14, M11_DM1_MOUSE_MASK_RIGHT) ==
                      M11_GAME_INPUT_IGNORED && !view.inventoryPanelActive,
                  "real Atari MINI.DAT keeps the C007 right-button gap inert");
            CHECK(M11_GameView_HandlePointerButton(&view, 234, 43, 0x0002) ==
                      M11_GAME_INPUT_REDRAW && view.spellPanelOpen &&
                      M11_GameView_HandleInput(
                          &view, M12_MENU_INPUT_SPELL_RUNE_1) ==
                          M11_GAME_INPUT_REDRAW &&
                      profile->runtime.party_state.Champions[0]
                          .Incantation[0] == 0x60 &&
                      profile->runtime.party_state.Champions[0]
                          .SymbolStep == 1u,
                  "real Atari MINI.DAT C101 writes F0399 incantation fields into GAMEBLOCK");
            CHECK(M11_GameView_HandlePointerButton(&view, 306, 64, 0x0002) ==
                      M11_GAME_INPUT_REDRAW &&
                      profile->runtime.party_state.Champions[0]
                          .Incantation[0] == 0 &&
                      profile->runtime.party_state.Champions[0]
                          .SymbolStep == 0u,
            "real Atari MINI.DAT C107 keeps F0400's deletion in GAMEBLOCK");
            /* ReDMCSB COMMAND.C F0380 tests G0311 together with the
             * projectile's absolute launch direction before it dequeues a
             * C003..C006 movement command.  The live M11 route must pass
             * the throw/shoot producer's retained G0310 direction through
             * to that queue; an old literal -1 bypassed this gate.  Keep the
             * test on the authentic MINI.DAT handoff rather than a
             * caller-built dungeon profile. */
            view.world.projectileDisabledMovementTicks = 2;
            view.world.lastProjectileDisabledMovementDirection =
                profile->runtime.party_dir;
            CHECK(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                      M11_GAME_INPUT_REDRAW &&
                      profile->runtime.last_input_dispatch.movementDisabledGate &&
                      profile->runtime.input_command_queue.count == 1u,
                  "real Atari MINI.DAT preserves the projectile-direction movement gate");
            /* Drain neither a test-only blocked command nor a stale throw
             * lock into the later save/resume assertions.  F0380 leaves the
             * command queued while its movement gate is active. */
            ((CSB_V1_BootProfile *)view.csbBootProfile)
                ->runtime.input_command_queue.count = 0u;
            view.world.projectileDisabledMovementTicks = 0;
            view.world.lastProjectileDisabledMovementDirection = -1;
            if (quicksave_path && quicksave_path[0]) {
                uint32_t saved_game_time = profile->runtime.game_time;
                int save_path_bound = set_test_quicksave_path(quicksave_path);
                char backup_path[1024];
                int backup_path_bound = 0;

                /* The archive runner deliberately supplies CSBGAME.DAT, the
                 * source slot family whose replacement rotates to .BAK in
                 * LOADSAVE.C F0433. Arbitrary Firestaff paths must not gain
                 * that original-media convention. */
                if (strlen(quicksave_path) >= strlen("CSBGAME.DAT") &&
                    strcmp(quicksave_path + strlen(quicksave_path) -
                           strlen("CSBGAME.DAT"), "CSBGAME.DAT") == 0) {
                    int length = snprintf(
                        backup_path, sizeof(backup_path), "%.*s.BAK",
                        (int)(strlen(quicksave_path) - 4u), quicksave_path);
                    backup_path_bound = length >= 0 &&
                        (size_t)length < sizeof(backup_path);
                }

                CHECK(save_path_bound &&
                          csb_v1_runtime_original_atari_save_source_current(
                              &profile->runtime),
                      "real Atari save binds its disposable path to an authenticated MINI.DAT template");
                if (save_path_bound) {
                    uint32_t relaunch_game_time;
                    int relaunch_level;
                    int relaunch_x;
                    int relaunch_y;
                    int relaunch_dir;
                    M11_GameLaunchSpec resumed_spec;
                    M11_GameViewState resumed_view;
                    const CSB_V1_BootProfile *resumed_profile;
                    CSB_V1_AtariSaveInfo first_save_info;
                    CSB_V1_AtariSaveInfo current_save_info;
                    CSB_V1_AtariSaveInfo backup_save_info;

                    remove(quicksave_path);
                    if (backup_path_bound) remove(backup_path);
                    CHECK(M11_GameView_HandleInput(&view,
                                                   M12_MENU_INPUT_DISK_MENU) ==
                              M11_GAME_INPUT_REDRAW &&
                              view.csbDiskMenuActive &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_DOWN) ==
                                  M11_GAME_INPUT_REDRAW &&
                              view.csbDiskMenuSelectedChoice == 2 &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_ACCEPT) ==
                                  M11_GAME_INPUT_REDRAW,
                          "real Atari Ctrl-S Save and Play reaches F0433");
                    CHECK(is_original_atari_save_file(quicksave_path),
                          "real Atari Ctrl-S writes an authenticated original save, not FSSB");
                    CHECK(read_original_atari_save_info(quicksave_path,
                                                        &first_save_info) &&
                              first_save_info.game_time == saved_game_time,
                          "first Atari F0433 write preserves its source-owned game clock");
                    for (tick = 0; tick < 5; ++tick) {
                        (void)M11_GameView_AdvanceIdleTick(&view);
                    }
                    saved_game_time = profile->runtime.game_time;
                    CHECK(M11_GameView_HandleInput(&view,
                                                   M12_MENU_INPUT_DISK_MENU) ==
                              M11_GAME_INPUT_REDRAW &&
                              view.csbDiskMenuActive &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_DOWN) ==
                                  M11_GAME_INPUT_REDRAW &&
                              view.csbDiskMenuSelectedChoice == 2 &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_ACCEPT) ==
                                  M11_GAME_INPUT_REDRAW,
                          "second real Atari Ctrl-S Save and Play replaces the original slot");
                    CHECK(read_original_atari_save_info(quicksave_path,
                                                        &current_save_info) &&
                              current_save_info.game_time == saved_game_time,
                          "second Atari F0433 write advances the source-format slot");
                    if (backup_path_bound) {
                        CHECK(read_original_atari_save_info(backup_path,
                                                            &backup_save_info) &&
                                  backup_save_info.game_time ==
                                      first_save_info.game_time,
                              "second Atari F0433 write rotates the prior original slot to .BAK");
                    }
                    CHECK(M11_GameView_HandleInput(&view,
                                                   M12_MENU_INPUT_DISK_MENU) ==
                              M11_GAME_INPUT_REDRAW &&
                              view.csbDiskMenuSelectedChoice == 1 &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_ACCEPT) ==
                                  M11_GAME_INPUT_REDRAW,
                          "real Atari Ctrl-S Load Saved Game reaches F0435");
                    profile = (const CSB_V1_BootProfile *)view.csbBootProfile;
                    CHECK(profile && profile->runtime.game_time == saved_game_time &&
                              view.loadGameTick == saved_game_time,
                          "F0435 restores the original Atari quicksave clock into M11");
                    /* ReDMCSB LOADSAVE.C F0433 writes the source-format game
                     * state which STARTUP1.C lines 162-165 later gives to
                     * F0435.  Exercise that independent process boundary,
                     * rather than treating the in-process Load Saved Game
                     * action as sufficient resume proof.  The source MINI.DAT
                     * remains read-only; only the explicit disposable path is
                     * written. */
                    relaunch_game_time = profile ? profile->runtime.game_time : 0u;
                    relaunch_level = profile ? profile->runtime.current_level : -1;
                    relaunch_x = profile ? profile->runtime.party_x : -1;
                    relaunch_y = profile ? profile->runtime.party_y : -1;
                    relaunch_dir = profile ? profile->runtime.party_dir : -1;
                    CHECK(M11_GameView_HandleInput(&view,
                                                   M12_MENU_INPUT_DISK_MENU) ==
                              M11_GAME_INPUT_REDRAW &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_DOWN) ==
                                  M11_GAME_INPUT_REDRAW &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_DOWN) ==
                                  M11_GAME_INPUT_REDRAW &&
                              view.csbDiskMenuSelectedChoice == 3 &&
                              M11_GameView_HandleInput(&view,
                                                       M12_MENU_INPUT_ACCEPT) ==
                                  M11_GAME_INPUT_RETURN_TO_MENU &&
                              is_original_atari_save_file(quicksave_path),
                          "real Atari Save and Quit writes the F0433 source-format artifact");
                    M11_GameView_Shutdown(&view);
                    resumed_spec = spec;
                    resumed_spec.savePath = quicksave_path;
                    M11_GameView_Init(&resumed_view);
                    CHECK(M11_GameView_Start(&resumed_view, &resumed_spec),
                          "cold M11 start accepts the real Atari Save and Quit artifact");
                    for (tick = 0; tick < 800 &&
                           resumed_view.csbState.startup_title_active; ++tick) {
                        (void)M11_GameView_AdvanceIdleTick(&resumed_view);
                    }
                    resumed_profile = (const CSB_V1_BootProfile *)
                        resumed_view.csbBootProfile;
                    CHECK(resumed_profile &&
                              resumed_view.csbAtariStRuntimeHandoffComplete &&
                              resumed_view.csbState.level_loaded &&
                              resumed_profile->runtime.game_time ==
                                  relaunch_game_time &&
                              resumed_profile->runtime.current_level ==
                                  relaunch_level &&
                              resumed_profile->runtime.party_x == relaunch_x &&
                              resumed_profile->runtime.party_y == relaunch_y &&
                              resumed_profile->runtime.party_dir == relaunch_dir,
                          "cold F0435 resume restores the saved Atari pose and clock");
                    memset(framebuffer, 0, sizeof(framebuffer));
                    M11_GameView_Draw(&resumed_view, framebuffer, 320, 200);
                    CHECK(resumed_profile &&
                              check_atari_st_c232_hud_frame(
                                  resumed_profile->graphics_path, framebuffer),
                          "cold Atari resume redraws the complete C232-owned HUD");
                    if (resumed_profile) {
                        int resumed_direction = resumed_profile->runtime.party_dir;

                        /* ReDMCSB COMMAND.C F0361 lines 677-684 accepts the
                         * C004 side-step keyboard row alongside forward and
                         * turns.  MINI.DAT supplies the live GAMEBLOCK; only
                         * its authentic dungeon decides whether this attempt
                         * advances or is blocked. */
                        CHECK(M11_GameView_HandleInput(
                                  &resumed_view,
                                  M12_MENU_INPUT_STRAFE_RIGHT) ==
                                  M11_GAME_INPUT_REDRAW &&
                                  resumed_profile->runtime.last_input_dispatch.command ==
                                      DM1_V1_COMMAND_MOVE_RIGHT,
                              "cold Atari resume admits first C004 side-step through the live command queue");
                        CHECK(M11_GameView_HandleInput(
                                  &resumed_view,
                                  M12_MENU_INPUT_TURN_RIGHT) ==
                                  M11_GAME_INPUT_REDRAW &&
                                  resumed_profile->runtime.party_dir ==
                                      ((resumed_direction + 1) & 3) &&
                                  resumed_view.csbState.party_dir ==
                                      resumed_profile->runtime.party_dir,
                              "cold Atari resume routes C002 through the live command queue");
                        resumed_direction = resumed_profile->runtime.party_dir;
                        CHECK(M11_GamepadActionToMenuInput(
                                  M12_ACTION_TURN_RIGHT, 1) ==
                                  M12_MENU_INPUT_TURN_RIGHT &&
                                  M11_GameView_HandleInput(
                                      &resumed_view,
                                      M11_GamepadActionToMenuInput(
                                          M12_ACTION_TURN_RIGHT, 1)) ==
                                      M11_GAME_INPUT_REDRAW &&
                                  resumed_profile->runtime.party_dir ==
                                      ((resumed_direction + 1) & 3) &&
                                  resumed_view.csbState.party_dir ==
                                      resumed_profile->runtime.party_dir,
                              "controller TURN_RIGHT reaches the real Atari GAMEBLOCK command queue");
                        resumed_direction = resumed_profile->runtime.party_dir;
                        CHECK(fs_gesture_gate_init() &&
                                  fs_gesture_gate_set_active_game(
                                      FS_GG_GAME_CSB) == FS_GG_GAME_DM1 &&
                                  fs_gesture_gate_set_enabled(
                                      FS_GG_GAME_CSB, 1) >= 0 &&
                                  M11_GameView_HandleTouchEvent(
                                      &resumed_view, M11_TOUCH_EVENT_DOWN,
                                      120, 100, 1000u) ==
                                      M11_GAME_INPUT_IGNORED &&
                                  M11_GameView_HandleTouchEvent(
                                      &resumed_view, M11_TOUCH_EVENT_UP,
                                      190, 100, 1100u) ==
                                      M11_GAME_INPUT_REDRAW &&
                                  resumed_profile->runtime.party_dir ==
                                      ((resumed_direction + 1) & 3) &&
                                  resumed_view.csbState.party_dir ==
                                      resumed_profile->runtime.party_dir,
                              "touch swipe reaches C002 through the real Atari GAMEBLOCK command queue");
                        CHECK(M11_GameView_HandleTouchEvent(
                                  &resumed_view, M11_TOUCH_EVENT_DOWN,
                                  234, 43, 2000u) ==
                                  M11_GAME_INPUT_IGNORED &&
                                  M11_GameView_HandleTouchEvent(
                                      &resumed_view, M11_TOUCH_EVENT_CANCEL,
                                      -1, 999, 2010u) ==
                                      M11_GAME_INPUT_IGNORED &&
                                  M11_GameView_HandleTouchEvent(
                                      &resumed_view, M11_TOUCH_EVENT_DOWN,
                                      234, 43, 3000u) ==
                                      M11_GAME_INPUT_IGNORED &&
                                  M11_GameView_HandleTouchEvent(
                                      &resumed_view, M11_TOUCH_EVENT_UP,
                                      234, 43, 3100u) ==
                                      M11_GAME_INPUT_REDRAW &&
                                  resumed_view.spellPanelOpen,
                              "touch cancel cannot leak into the real Atari C100 HUD tap route");
                    }
                    M11_GameView_Shutdown(&resumed_view);
                    remove(quicksave_path);
                    if (backup_path_bound) remove(backup_path);
                    if (failures) return 1;
                    puts("PASS: real Atari MINI.DAT save-and-quit cold resume");
                    return 0;
                }
            }
            M11_GameView_Shutdown(&view);
            if (failures) return 1;
            puts("PASS: real Atari MINI.DAT reaches live Atari ST M11 runtime");
            return 0;
        }
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        for (y = 33; y < 169; ++y) {
            for (x = 48; x < 272; ++x) {
                if (framebuffer[y * 320 + x] != 0u) {
                    ++viewport_nonblack;
                }
            }
        }
        c013 = M11_AssetLoader_Load(&view.assetLoader, 13u);
        CHECK(!view.csbState.startup_title_active &&
                  !view.csbState.startup_entrance_active &&
                  c013 && c013->loaded && c013->pixels,
              "real MINI.DAT Resume enters live CSB with source-owned HUD material");
        CHECK(c013->width == 87u && c013->height == 45u,
              "real MINI.DAT Resume retains the native source C013 HUD dimensions");
        CHECK(view.csbState.runtime_viewport_source_session_ready &&
                  viewport_nonblack > 0,
              "real MINI.DAT Resume consumes a source-owned F0128 viewport");
        /* C009/C011 consume the real GAMEBLOCK party. Draw the unmodified
         * live frame, then open the spell panel through C100 rather than
         * seeding an M11-only party or spell state. */
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(M11_GameView_HandlePointerButton(&view, 234, 43, 0x0002) ==
                  M11_GAME_INPUT_REDRAW &&
                  view.world.party.championCount == 1 &&
                  view.spellPanelOpen,
              "CSB C100 opens the spell panel from the real GAMEBLOCK party");
        {
            CSB_V1_BootProfile *mutable_profile =
                (CSB_V1_BootProfile *)view.csbBootProfile;
            unsigned short mana_before = view.world.party.champions[0].mana.current;

            CHECK(mana_before > 1u &&
                      M11_GameView_HandleInput(
                          &view, M12_MENU_INPUT_SPELL_RUNE_1) ==
                          M11_GAME_INPUT_REDRAW &&
                      view.spellBuffer.runeCount == 1 &&
                      view.spellBuffer.runes[0] == 0x60u &&
                      view.world.party.champions[0].mana.current < mana_before &&
                      mutable_profile &&
                      mutable_profile->runtime.party_state.Champions[0]
                          .CurrentMana ==
                          (int16_t)view.world.party.champions[0].mana.current &&
                      mutable_profile->runtime.party_state.Champions[0]
                          .Incantation[0] == 0x60 &&
                      mutable_profile->runtime.party_state.Champions[0]
                          .Incantation[1] == 0 &&
                      mutable_profile->runtime.party_state.Champions[0]
                          .SymbolStep == 1u,
                  "real Atari GAMEBLOCK persists C101 mana, incantation, and F0399 SymbolStep");
            CHECK(M11_GameView_HandlePointerButton(&view, 306, 64, 0x0002) ==
                      M11_GAME_INPUT_REDRAW &&
                      view.spellBuffer.runeCount == 0 &&
                      mutable_profile->runtime.party_state.Champions[0]
                          .Incantation[0] == 0 &&
                      mutable_profile->runtime.party_state.Champions[0]
                          .SymbolStep == 0u,
                  "real Atari C107 recant clears the persisted F0400 incantation without refunding mana");
            (void)M11_GameView_ClearSpell(&view);
        }
        /* The actual saved party contains one champion. F2 must stay inert
         * against that real GAMEBLOCK instead of relying on a caller-seeded
         * second M11 champion. */
        CHECK(M11_GameView_HandleInput(&view,
                                       M12_MENU_INPUT_CHAMPION_2_INVENTORY) ==
                  M11_GAME_INPUT_REDRAW &&
                  view.world.party.championCount == 1 &&
                  !view.inventoryPanelActive,
              "CSB F2 rejects a champion absent from the real PC3.4 GAMEBLOCK");
        CHECK(M11_GameView_HandleInput(&view,
                                       M12_MENU_INPUT_CHAMPION_1_INVENTORY) ==
                  M11_GAME_INPUT_REDRAW &&
                  view.world.party.championCount == 1 &&
                  view.inventoryPanelActive,
              "CSB F1 refreshes GAMEBLOCK party before opening champion inventory");
        {
            int hand_x = 0;
            int hand_y = 0;
            int hand_w = 0;
            int hand_h = 0;

            /* C020..C027 take the same source M516 owner as the inventory
             * grid. The real PC3.4 session has only champion 0, so C022
             * must not touch a nonexistent second champion. */
            CHECK(M11_GameView_GetV1StatusHandSlotBoxZone(
                      1, 0, &hand_x, &hand_y, &hand_w, &hand_h) &&
                      M11_GameView_HandlePointerButton(
                          &view, hand_x + hand_w / 2, hand_y + hand_h / 2,
                          M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_IGNORED &&
                      view.world.party.championCount == 1,
                  "CSB C022 status-hand click refreshes GAMEBLOCK before selecting its champion");
        }
        M11_GameView_Shutdown(&view);
        if (failures) return 1;
        puts("PASS: real Atari MINI.DAT reaches live M11 CSB HUD");
        return 0;
    }
    memset(&decode_receipt, 0, sizeof(decode_receipt));
    CHECK(csb_v1_boot_decode_graphics_dat_asset_pc34(
              view.csbBootProfile ?
                  ((CSB_V1_BootProfile *)view.csbBootProfile)->graphics_path : "",
              13u, &decoded, &decoded_w, &decoded_h, &decode_receipt) &&
              decoded_w == 87 && decoded_h == 45,
          "PC3.4 decoder supplies native C013 source");
    free(decoded);
    decoded = NULL;
    decoded_w = 0;
    decoded_h = 0;
    memset(&decode_receipt, 0, sizeof(decode_receipt));
    CHECK(csb_v1_boot_decode_graphics_dat_asset_pc34(
              ((CSB_V1_BootProfile *)view.csbBootProfile)->graphics_path,
              28u, &decoded, &decoded_w, &decoded_h, &decode_receipt) &&
              decoded_w == 76 && decoded_h == 14,
          "PC3.4 decoder supplies native C028 source");
    free(decoded);
    decoded = NULL;
    for (tick = 0; tick < 120; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    /* M11 has already converted SDL mouse buttons to PC34 masks.  The
     * generic Entrance compatibility boundary also accepts raw SDL-left
     * mask 0x0001, which is M11's right button.  A real Prison frame must
     * therefore leave C200 untouched on a right click in its source box.
     * ReDMCSB COMMAND.C G0445/F0358; ENTRANCE.C F0441/F0806. */
    CHECK(view.csbState.startup_entrance_active &&
              !view.csbState.startup_entrance_opening_active &&
              M11_GameView_HandlePointerButton(
                  &view, 245, 46, M11_DM1_MOUSE_MASK_RIGHT) ==
                  M11_GAME_INPUT_IGNORED &&
              view.csbState.startup_entrance_active &&
              !view.csbState.startup_entrance_opening_active,
          "real Prison entrance right click cannot masquerade as C200 left input");
    CHECK(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
              M11_GAME_INPUT_REDRAW,
          "original Enter route accepts Prison handoff");
    for (tick = 0; tick < 200; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
        M11_GameView_Draw(&view, framebuffer, 320, 200);
    }
    CHECK(!view.csbState.startup_title_active &&
              !view.csbState.startup_entrance_active,
          "Prison handoff reaches live runtime");
    CHECK(!view.inventoryPanelActive,
          "Prison handoff reaches the live HUD rather than an inventory surface");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    c013 = M11_AssetLoader_Load(&view.assetLoader, 13u);
    CHECK(c013 && c013->loaded && c013->pixels &&
              c013->width == 87u && c013->height == 45u,
          "runtime cache contains decoded C013 movement source");
    /* DATA.C G0002 / MENUDRAW.C C013: 233,124, 87x45. */
    for (y = 124; y < 169; ++y) {
        for (x = 233; x < 320; ++x) {
            if (framebuffer[y * 320 + x] != 0u) {
                ++c013_nonblack;
            }
        }
    }
    CHECK(c013_nonblack > 0,
          "live Prison framebuffer consumes real C013 movement pixels");
    {
        CSB_V1_BootProfile *profile =
            (CSB_V1_BootProfile *)view.csbBootProfile;
        DM1_V1_MovementArrowRectPc34 turn_right_rect;
        DM1_V1_MovementArrowRectPc34 forward_rect;
        int old_direction = profile ? profile->runtime.party_dir : -1;

        CHECK(profile && M11_GameView_HandleInput(
                  &view, M12_MENU_INPUT_TURN_RIGHT) == M11_GAME_INPUT_REDRAW,
              "live Prison keyboard routes C002 through the CSB command queue");
        CHECK(profile && profile->runtime.party_dir ==
                  ((old_direction + 1) & 3) &&
                  view.csbState.party_dir == profile->runtime.party_dir,
              "live Prison C002 turn updates both CSB runtime and M11 mirror");
        CHECK(profile && M11_GameView_HandleInput(
                  &view, M12_MENU_INPUT_UP) == M11_GAME_INPUT_REDRAW,
              "live Prison keyboard routes C003 through the CSB command queue");
        CHECK(profile && profile->runtime.last_input_dispatch.dequeued &&
                  profile->runtime.last_input_dispatch.command ==
                      DM1_V1_COMMAND_MOVE_FORWARD &&
                  profile->runtime.last_input_dispatch.dispatchedMove,
              "live Prison C003 reaches the real dungeon movement dispatcher");
        CHECK(dm1_v1_movement_arrow_rect_pc34(
                  DM1_V1_MOVEMENT_ARROW_INDEX_TURN_RIGHT_PC34,
                  &turn_right_rect) &&
                  M11_GameView_HandlePointerButton(
                      &view,
                      turn_right_rect.x + turn_right_rect.w / 2,
                      turn_right_rect.y + turn_right_rect.h / 2,
                      M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "live Prison C002 pointer route reaches the CSB command queue");
        CHECK(profile && profile->runtime.party_dir ==
                  ((old_direction + 2) & 3) &&
                  view.csbState.party_dir == profile->runtime.party_dir,
              "live Prison C002 pointer updates the CSB runtime and M11 mirror");
        CHECK(dm1_v1_movement_arrow_rect_pc34(
                  DM1_V1_MOVEMENT_ARROW_INDEX_FORWARD_PC34,
                  &forward_rect) &&
                  M11_GameView_HandlePointerButton(
                      &view,
                      forward_rect.x + forward_rect.w / 2,
                      forward_rect.y + forward_rect.h / 2,
                      M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "live Prison C003 pointer route reaches the CSB command queue");
        CHECK(profile && profile->runtime.last_input_dispatch.dequeued &&
                  profile->runtime.last_input_dispatch.command ==
                      DM1_V1_COMMAND_MOVE_FORWARD &&
                  profile->runtime.last_input_dispatch.dispatchedMove,
              "live Prison C003 pointer reaches the real dungeon movement dispatcher");
        /* Presentation V2 only changes the CSB raster.  G0448 still owns
         * every C007 viewport click, so a left-edge click must enter C080
         * rather than the old host-only left-third turn shortcut.  Prison's
         * real front square has no matching click sensor here; irrespective
         * of whether the source click redraws, it must not rotate or move
         * the GAMEBLOCK party.  ReDMCSB COMMAND.C:403 and :2322. */
        if (spec.presentationMode != M12_PRESENTATION_V1_ORIGINAL) {
            int viewport_direction = profile ? profile->runtime.party_dir : -1;
            int viewport_x = profile ? profile->runtime.party_x : -1;
            int viewport_y = profile ? profile->runtime.party_y : -1;
            (void)M11_GameView_HandlePointerButton(
                &view, 52, 40, M11_DM1_MOUSE_MASK_LEFT);
            CHECK(profile && profile->runtime.party_dir == viewport_direction &&
                      profile->runtime.party_x == viewport_x &&
                      profile->runtime.party_y == viewport_y,
                  "CSB V2 viewport click retains source C080 instead of host steering");
        }
    }
    if (quicksave_path && quicksave_path[0]) {
        const CSB_V1_BootProfile *profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        uint32_t saved_game_time;
        int save_path_bound = 0;
        FILE *save_file;

        /* This is intentionally opt-in: a real-media probe must never
         * overwrite the player's normal quicksave.  The caller supplies a
         * disposable path through M11's normal F5/F9 environment override.
         * ReDMCSB COMMAND.C F0361 opens C140 from Ctrl-S, whose source menu
         * invokes LOADSAVE.C F0433/F0435.  Exercise that complete route
         * after the real Prison handoff rather than calling the save helpers
         * directly. */
        save_path_bound = set_test_quicksave_path(quicksave_path);
        CHECK(save_path_bound,
              "real-data save probe binds its explicit disposable save path after boot");
        if (save_path_bound) {
            saved_game_time = profile ? profile->runtime.game_time : 0u;
            CHECK(profile &&
                      M11_GameView_HandleInput(&view,
                                                M12_MENU_INPUT_DISK_MENU) ==
                          M11_GAME_INPUT_REDRAW &&
                      view.csbDiskMenuActive && view.dialogOverlayActive &&
                      view.dialogChoiceCount == 4 &&
                      M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                          M11_GAME_INPUT_REDRAW &&
                      view.csbDiskMenuSelectedChoice == 2 &&
                      M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                          M11_GAME_INPUT_REDRAW &&
                      !view.csbDiskMenuActive && !view.dialogOverlayActive,
                  "real Ctrl-S C140 Save and Play writes a native CSB quicksave");
            save_file = fopen(quicksave_path, "rb");
            CHECK(save_file != NULL,
                  "live Prison quicksave materializes at its explicit path");
            if (save_file) {
                fclose(save_file);
            }
            for (tick = 0; tick < 5; ++tick) {
                (void)M11_GameView_AdvanceIdleTick(&view);
            }
            CHECK(profile && profile->runtime.game_time >= saved_game_time,
                  "live Prison runtime remains clocked after saving");
            CHECK(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DISK_MENU) ==
                      M11_GAME_INPUT_REDRAW &&
                      view.csbDiskMenuSelectedChoice == 1 &&
                      M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                          M11_GAME_INPUT_REDRAW &&
                      !view.csbDiskMenuActive && !view.dialogOverlayActive,
                  "real Ctrl-S C140 Load Saved Game resumes its native CSB quicksave");
            profile = (const CSB_V1_BootProfile *)view.csbBootProfile;
            CHECK(profile && profile->runtime.game_time == saved_game_time &&
                      view.loadGameTick == saved_game_time &&
                      view.lastSaveTick == saved_game_time,
                  "F0435 resume restores the Prison quicksave clock into M11");
            for (tick = 0; tick < 101; ++tick) {
                CHECK(csb_v1_boot_runtime_tick_pc34(
                          (CSB_V1_BootProfile *)view.csbBootProfile, NULL),
                      "live Prison session advances its source-owned game clock");
            }
            CHECK(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DISK_MENU) ==
                      M11_GAME_INPUT_REDRAW &&
                      M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                          M11_GAME_INPUT_REDRAW &&
                      M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                          M11_GAME_INPUT_REDRAW &&
                      view.csbDiskMenuSelectedChoice == 3 &&
                      M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                          M11_GAME_INPUT_RETURN_TO_MENU,
                  "real Ctrl-S C140 Save and Quit accepts the native runtime save route");
            profile = (const CSB_V1_BootProfile *)view.csbBootProfile;
            CHECK(profile && profile->runtime.game_time == view.lastSaveTick,
                  "CSB save-and-quit records the F0433 runtime clock, not a DM1 world tick");
            if (profile) {
                /* ReDMCSB LOADSAVE.C F0433 persists the state consumed by a
                 * later F0435 process, not only an in-memory QuickLoad. Keep
                 * this explicit disposable path until the cold-start check
                 * below has rebuilt an independent M11/boot profile. */
                relaunch_save_available = 1;
                relaunch_game_time = profile->runtime.game_time;
                relaunch_level = profile->runtime.current_level;
                relaunch_x = profile->runtime.party_x;
                relaunch_y = profile->runtime.party_y;
                relaunch_dir = profile->runtime.party_dir;
            }
        }
    }
    {
        /* Real PC I34 C161 evidence: use an authenticated mirror record
         * decoded from this GRAPHICS/DUNGEON pair, append it through F0280's
         * GAMEBLOCK bridge, then require F0282 to consume the profile's live
         * G0349 stream. This is deliberately after the save transaction: it
         * must not alter the real Prison save/resume assertion above. */
        CSB_V1_BootProfile *profile =
            (CSB_V1_BootProfile *)view.csbBootProfile;
        CSB_V1_PartyState party;
        uint32_t seed = UINT32_C(0x13579bdf);
        uint32_t expected_seed = seed;
        unsigned int before[CSB_V1_STAT_COUNT];
        unsigned int increments[CSB_V1_STAT_COUNT] = {0};
        int candidate_index;
        int stat;
        int appended = 0;

        CHECK(profile && view.mirrorCatalog.count > 0,
              "real PC3.4 Prison exposes a source mirror for C161 runtime proof");
        if (profile && view.mirrorCatalog.count > 0) {
            profile->runtime.csbwin_random_seed_valid = 1;
            profile->runtime.csbwin_random_seed = seed;
            candidate_index = profile->runtime.party_state_valid
                ? profile->runtime.party_state.ChampionCount : 0;
            appended = csb_v1_runtime_append_mirror_candidate_source_compat(
                      &profile->runtime,
                      &view.mirrorCatalog.records[0].champion) == 0;
            CHECK(appended,
                  "F0280 bridge appends the real PC3.4 mirror candidate");
            if (appended) {
            CHECK(csb_v1_runtime_get_party_state(&profile->runtime, &party) >= 0 &&
                      party.ChampionCount == candidate_index + 1,
                  "real C161 candidate is the final contiguous GAMEBLOCK entry");
            for (stat = 0; stat < CSB_V1_STAT_COUNT; ++stat) {
                before[stat] = party.Champions[candidate_index]
                    .Statistics[stat][CSB_V1_STAT_CUR];
            }
            for (tick = 0; tick < 12; ++tick) {
                uint16_t raw;
                expected_seed = expected_seed * UINT32_C(0xbb40e62d) +
                    UINT32_C(11);
                raw = (uint16_t)(expected_seed >> 8);
                ++increments[raw % CSB_V1_STAT_COUNT];
            }
            CHECK(csb_v1_runtime_reincarnate_pending_mirror_candidate_source_compat(
                      &profile->runtime, candidate_index),
                  "C161 finalizes against the live PC3.4 G0349 stream");
            CHECK(csb_v1_runtime_get_party_state(&profile->runtime, &party) >= 0 &&
                      profile->runtime.csbwin_random_seed == expected_seed,
                  "C161 advances G0349 exactly twelve F0027 calls");
            for (stat = 0; stat < CSB_V1_STAT_COUNT; ++stat) {
                CHECK(party.Champions[candidate_index].Statistics[stat]
                          [CSB_V1_STAT_CUR] == before[stat] + increments[stat] &&
                          party.Champions[candidate_index].Statistics[stat]
                          [CSB_V1_STAT_MAX] == before[stat] + increments[stat],
                      "PC3.4 C161 applies source F0027 boosts including Luck");
            }
            CHECK(party.Champions[candidate_index].Skills[0] == 0 &&
                      party.Champions[candidate_index].SkillExperience[0] == 0 &&
                      party.Champions[candidate_index]
                          .SkillTemporaryExperience[0] == 0,
                  "C161 clears source-owned skill state before its boosts");
            }
        }
    }
    M11_GameView_Shutdown(&view);
    if (relaunch_save_available) {
        M11_GameLaunchSpec resumed_spec = spec;
        M11_GameViewState resumed_view;
        const CSB_V1_BootProfile *resumed_profile;

        resumed_spec.savePath = quicksave_path;
        M11_GameView_Init(&resumed_view);
        CHECK(M11_GameView_Start(&resumed_view, &resumed_spec),
              "cold M11 launch accepts the real PC3.4 Save and Quit artifact");
        resumed_profile = (const CSB_V1_BootProfile *)resumed_view.csbBootProfile;
        CHECK(resumed_profile && !resumed_view.csbState.startup_title_active &&
                  !resumed_view.csbState.startup_entrance_active &&
                  resumed_profile->runtime.game_time == relaunch_game_time &&
                  resumed_profile->runtime.current_level == relaunch_level &&
                  resumed_profile->runtime.party_x == relaunch_x &&
                  resumed_profile->runtime.party_y == relaunch_y &&
                  resumed_profile->runtime.party_dir == relaunch_dir,
              "cold F0435 resume restores the saved PC3.4 runtime pose and clock");
        M11_GameView_Shutdown(&resumed_view);
        remove(quicksave_path);
    }
    if (failures) return 1;
    puts("PASS: csb_v1_m11_prison_runtime_hud_pc34");
    return 0;
}
