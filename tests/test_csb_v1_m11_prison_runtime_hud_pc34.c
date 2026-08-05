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
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    unsigned char framebuffer[320 * 200];
    int tick;
    int x;
    int y;
    int c013_nonblack = 0;
    int viewport_nonblack = 0;
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
        /* The CSB HUD always rebuilds its party surface from GAMEBLOCK data.
         * Make the retained M11 mirror deliberately stale, then require the
         * C009/C011 spell panel to remain drawable through the source-owned
         * party copy.  The old call passed `view` here and cleared the panel
         * because its stale party count was zero. */
        view.world.party.championCount = 0;
        view.spellPanelOpen = 1;
        view.dm1SpellCasting.magicCasterIndex = 0;
        view.dm1SpellCasting.input[0].symbolStep = 0;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        /* C009 itself may be all palette index zero in an original save
         * state. The following C100/F1 input receipts verify the fresh
         * GAMEBLOCK party mirror rather than treating a transparent source
         * panel background as a synthetic visual failure. */
        view.spellPanelOpen = 0;
        view.dm1SpellCasting.magicCasterIndex = -1;
        CHECK(M11_GameView_HandlePointerButton(&view, 234, 43, 0x0002) ==
                  M11_GAME_INPUT_REDRAW &&
                  view.world.party.championCount == 1 &&
                  view.spellPanelOpen,
              "CSB C100 refreshes GAMEBLOCK party before opening the spell panel");
        view.world.party.championCount = 0;
        view.inventoryPanelActive = 0;
        CHECK(M11_GameView_HandleInput(&view,
                                       M12_MENU_INPUT_CHAMPION_1_INVENTORY) ==
                  M11_GAME_INPUT_REDRAW &&
                  view.world.party.championCount == 1 &&
                  view.inventoryPanelActive,
              "CSB F1 refreshes GAMEBLOCK party before opening champion inventory");
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
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    puts("PASS: csb_v1_m11_prison_runtime_hud_pc34");
    return 0;
}
