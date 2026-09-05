/* Opt-in M11 regression for the original DM2 Amiga title route.
 *
 * The selected ZIP is consumed through ZIP -> ADF -> LZX into boot-owned
 * RAM.  No original member is materialised on the host filesystem. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_startup_menu.h"
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char *G2159_puc_Bitmap_Source;
unsigned char *G2160_puc_Bitmap_Destination;

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static unsigned int nonzero_pixels(const unsigned char *pixels, size_t size)
{
    unsigned int count = 0u;
    size_t index;
    for (index = 0u; pixels && index < size; ++index) {
        if (pixels[index] != 0u) ++count;
    }
    return count;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_AMIGA_ROOT");
    M11_GameViewState view;
    M11_GameLaunchSpec spec;
    M12_AssetStatus asset_status;
    char selected_runtime[M12_ASSET_DATA_DIR_CAPACITY];
    unsigned char framebuffer[M11_FB_BYTES];
    int step;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_AMIGA_ROOT is not set");
        return 77;
    }
    /* A shared DM2 root intentionally auto-prefers FM Towns.  This test is
     * specifically about the selected Amiga edition, so reproduce M12's
     * version-picker handoff instead of relying on the scanner's default.
     * The resolver retains the original ZIP as the runtime source; it does
     * not unpack GRAPHICS.DAT or DUNGEON.DAT onto disk. */
    memset(&asset_status, 0, sizeof(asset_status));
    memset(selected_runtime, 0, sizeof(selected_runtime));
    M12_AssetStatus_ScanGame(&asset_status, root, "dm2");
    expect(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
               &asset_status, "dm2", "amiga-en", selected_runtime,
               sizeof(selected_runtime)),
           "the selected Amiga edition retains its verified archive handoff");
    if (failures != 0) return 1;
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.title = "DUNGEON MASTER II";
    spec.dataDir = selected_runtime;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = M11_FB_WIDTH;
    spec.presentationHeight = M11_FB_HEIGHT;
    M11_GameView_Init(&view);
    expect(M11_GameView_Start(&view, &spec) == 1,
           "the selected Amiga archive enters the DM2 M11 path");
    expect(view.dm2FmtownsTitleBound && view.dm2FmtownsSwooshActive &&
               view.dm2FmtownsTitleFrameReceipt.valid &&
               view.dm2FmtownsTitleFrameReceipt.requested_frame == 0u &&
               view.dm2FmtownsFrameCount == 19u,
           "M11 begins with Amiga SWSH frame zero from the source stream");
    expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
               M11_GAME_INPUT_IGNORED,
           "SWSH does not leak New Game input before the title completes");
    for (step = 0; step < 10000 && view.dm2FmtownsSwooshActive; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleBound && !view.dm2FmtownsSwooshActive &&
               view.dm2FmtownsTitleFrameReceipt.requested_frame == 0u &&
               view.dm2FmtownsFrameCount == 225u,
           "SWSH binds the original Amiga TITL stream without a substitute");
    for (step = 0; step < 20000 && !view.dm2FmtownsTitleFinished; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    expect(view.dm2FmtownsTitleFinished && !view.dm2FmtownsTitleBound,
           "all original Amiga TITL frames elapse before the startup menu");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) != 0u,
           "Amiga TITL hands off to original Amiga GDAT menu pixels");
    {
        const DM2_V1_AssetLoader *loader = dm2_v1_boot_asset_loader(
            (DM2_V1_BootProfile *)view.dm2BootProfile);
        DM2_V1_QueryGdatSummaryImageReceipt image;
        const uint8_t *palette_raw;
        uint8_t expected_rgb8[256][3];
        uint8_t presented_rgb8[256][3];
        size_t palette_size = 0u;
        int color;
        int lossy_vga_round_trip = 0;
        memset(&image, 0, sizeof(image));
        memset(expected_rgb8, 0, sizeof(expected_rgb8));
        memset(presented_rgb8, 0, sizeof(presented_rgb8));
        palette_raw = dm2_v1_asset_load_typed_sized(
            loader, DM2_GDAT_CATEGORY_TITLE, 0,
            DM2_GDAT_ENTRY_TYPE_PAL_IRGB, 4, &palette_size);
        expect(palette_raw && palette_size == 16u * 4u &&
                   dm2_v1_query_gdat_summary_image_receipt(
                       loader, DM2_GDAT_CATEGORY_TITLE, 0, 4, &image) &&
                   image.accepted && image.colors == 16u,
               "Amiga menu exposes its real field-4 IRGB and Palette16 pair");
        for (color = 0; palette_raw && color < 16; ++color) {
            const uint8_t physical = image.palette16[color];
            expect(palette_raw[(size_t)color * 4u] == (uint8_t)color,
                   "Amiga menu IRGB retains its source row index");
            expected_rgb8[physical][0] = palette_raw[(size_t)color * 4u + 1u];
            expected_rgb8[physical][1] = palette_raw[(size_t)color * 4u + 2u];
            expected_rgb8[physical][2] = palette_raw[(size_t)color * 4u + 3u];
            if (palette_raw[(size_t)color * 4u + 1u] !=
                    (uint8_t)(((palette_raw[(size_t)color * 4u + 1u] >> 2u) << 2u) |
                              (palette_raw[(size_t)color * 4u + 1u] >> 6u)) ||
                palette_raw[(size_t)color * 4u + 2u] !=
                    (uint8_t)(((palette_raw[(size_t)color * 4u + 2u] >> 2u) << 2u) |
                              (palette_raw[(size_t)color * 4u + 2u] >> 6u)) ||
                palette_raw[(size_t)color * 4u + 3u] !=
                    (uint8_t)(((palette_raw[(size_t)color * 4u + 3u] >> 2u) << 2u) |
                              (palette_raw[(size_t)color * 4u + 3u] >> 6u))) {
                lossy_vga_round_trip = 1;
            }
        }
        expect(lossy_vga_round_trip,
               "Amiga menu contains components changed by a VGA six-bit round trip");
        expect(M11_Render_CopyIndexedPaletteRgb8(presented_rgb8) &&
                   memcmp(expected_rgb8, presented_rgb8,
                          sizeof(expected_rgb8)) == 0,
               "Amiga menu presents exact full-byte GDAT IRGB components");
    }
    {
        DM2_V1_StartupMenuPointerLayout layout;
        M11_GameInputResult input_result;
        memset(&layout, 0, sizeof(layout));
        expect(dm2_v1_boot_startup_menu_pointer_layout(
                   (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) &&
                   layout.valid && layout.new_game.w > 0 && layout.new_game.h > 0,
               "the Amiga GDAT retains the source New Game rectangle");
        {
            DM2_V1_StartupExecution execution;
            DM2_V1_StartupHostActionReceipt action_receipt;
            memset(&execution, 0, sizeof(execution));
            memset(&action_receipt, 0, sizeof(action_receipt));
            expect(dm2_v1_boot_startup_execute_original_pointer_from_runtime_state(
                       (const DM2_V1_BootProfile *)view.dm2BootProfile,
                       view.dm2State.startup_menu_active,
                       view.dm2State.startup_save_root,
                       view.dm2State.startup_resume_available,
                       view.dm2State.startup_slot_mask,
                       view.dm2State.startup_menu_selected_row,
                       layout.new_game.x + layout.new_game.w / 2,
                       layout.new_game.y + layout.new_game.h / 2,
                       NULL, NULL, &execution, &action_receipt) &&
                       action_receipt.host_menu_route.valid,
                   "the Amiga New Game rectangle resolves the original 0xD7 event");
        }
        input_result = M11_GameView_HandlePointer(
            &view, layout.new_game.x + layout.new_game.w / 2,
            layout.new_game.y + layout.new_game.h / 2, 1);
        expect(input_result == M11_GAME_INPUT_REDRAW &&
                   !view.dm2State.startup_menu_active &&
                   view.dm2State.level_loaded &&
                   view.world.party.championCount > 0 &&
                   view.dm2State.leader_hand_object == 0xffffu,
                "Amiga title New Game hands off to the source runtime without a synthetic party");
        {
            M11_GameInputResult inventory_result = M11_GameView_HandleInput(
                &view, M12_MENU_INPUT_INVENTORY_TOGGLE);
            expect(inventory_result ==
                   M11_GAME_INPUT_REDRAW && view.inventoryPanelActive == 1,
               "Amiga opens the authenticated CHARSHEET inventory frame");
        }
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
        expect(nonzero_pixels(framebuffer, sizeof(framebuffer)) != 0u,
               "Amiga inventory draw consumes original 16-colour CHARSHEET pixels");
        expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                   M11_GAME_INPUT_REDRAW && !view.inventoryPanelActive,
               "Amiga closes inventory through the source panel boundary");
    }
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    puts("PASS: DM2 Amiga M11 title reaches the source startup menu");
    return 0;
}
