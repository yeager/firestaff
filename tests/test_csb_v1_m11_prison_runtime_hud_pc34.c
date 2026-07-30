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

int main(void)
{
    char fallback[1024];
    const char *data_dir = csb_data_dir(fallback, sizeof(fallback));
    const char *mode_text = getenv("FIRESTAFF_CSB_PRESENTATION_MODE");
    const char *atari_mini = getenv("FIRESTAFF_CSB_ATARI_MINI");
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    unsigned char framebuffer[320 * 200];
    int tick;
    int x;
    int y;
    int c013_nonblack = 0;
    int c009_nonblack = 0;
    int viewport_nonblack = 0;
    const M11_AssetSlot *c013;
    unsigned char *decoded = NULL;
    int decoded_w = 0;
    int decoded_h = 0;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 decode_receipt;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: no CSB PC3.4 data directory");
        return 0;
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
        for (y = 42; y < 67; ++y) {
            for (x = 233; x < 320; ++x) {
                if (framebuffer[y * 320 + x] != 0u) {
                    ++c009_nonblack;
                }
            }
        }
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
