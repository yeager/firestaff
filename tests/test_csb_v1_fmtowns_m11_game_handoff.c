/* Real FM Towns CSB Game-program handoff.
 *
 * This opt-in test uses a materialized original F31E/F31J media root. It
 * proves that the presentation path reaches SWITCHTW, consumes its Game
 * rectangle, and opens the language-owned C004 entrance session. No generated
 * title, switch, entrance or HUD pixels are accepted.
 */
#include "m11_game_view.h"
#include "asset_status_m12.h"
#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_f0070_champion_formation_pc34_compat.h"
#include "csb_v1_fmtowns_game.h"
#include "csb_v1_fmtowns_graphics_dat.h"
#include "csb_v1_fmtowns_portrait.h"
#include "csb_v1_fmtowns_switch.h"
#include "csb_v1_fmtowns_utility_render.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_creature_render_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "fs_portable_compat.h"
#include "redmcsb_f7062_save_header_pc34_compat.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

static int failures;

static int copy_file(const char *source_path, const char *destination_path)
{
    unsigned char buffer[4096];
    FILE *source = fopen(source_path, "rb");
    FILE *destination = NULL;
    size_t count;
    int ok = 0;

    if (!source) return 0;
    destination = fopen(destination_path, "wb");
    if (!destination) {
        fclose(source);
        return 0;
    }
    while ((count = fread(buffer, 1u, sizeof(buffer), source)) != 0u) {
        if (fwrite(buffer, 1u, count, destination) != count) goto done;
    }
    ok = !ferror(source) && fclose(destination) == 0;
    destination = NULL;
done:
    if (destination) fclose(destination);
    fclose(source);
    return ok;
}

static uint8_t *load_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

/* Packed F31 sources retain authenticated CDATA/CJDATA member bytes in the
 * boot profile.  The archive locator is provenance, not a host pathname, so
 * verification must consume that already-admitted view instead of trying to
 * fopen `archive.zip::CDATA/GRAPHICS.DAT`. */
static uint8_t *load_profile_graphics(const CSB_V1_BootProfile *profile,
                                      size_t *out_size)
{
    uint8_t *bytes;

    if (!profile || !out_size) return NULL;
    if (!profile->fmtowns_graphics_bytes ||
        profile->fmtowns_graphics_size == 0u)
        return load_file(profile->graphics_path, out_size);
    bytes = (uint8_t *)malloc(profile->fmtowns_graphics_size);
    if (!bytes) return NULL;
    memcpy(bytes, profile->fmtowns_graphics_bytes,
           profile->fmtowns_graphics_size);
    *out_size = profile->fmtowns_graphics_size;
    return bytes;
}

static uint32_t fnv1a(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

/* Retail C696 C007 owns x0,y33 EN / x0,y31 JP. Keep the live
 * frame assertion scoped to those source-owned pixels: C017 HUD updates
 * elsewhere on the page must neither satisfy nor mask a stale viewport. */
static uint32_t f31_viewport_fnv1a(const uint8_t *framebuffer,
                                  CSB_V1_FmtownsSwitchLanguage language)
{
    uint32_t hash = 2166136261u;
    int x;
    int y;

    if (!framebuffer) return 0u;
    const int top = language == CSB_FMTOWNS_SWITCH_JAPANESE ? 31 : 33;
    for (y = top; y < top + 136; ++y) {
        for (x = 0; x < 224; ++x) {
            hash ^= framebuffer[(size_t)y * 320u + (size_t)x];
            hash *= UINT32_C(16777619);
        }
    }
    return hash;
}

static int f31_has_native_runtime_group_sprite(const M11_GameViewState *view)
{
    int object_sprite_count;
    int object_icon_count;
    int object_marker_count;
    int group_sprite_count;
    int group_marker_count;
    int projectile_sprite_count;
    int projectile_material_count;
    int projectile_marker_count;
    int explosion_sprite_count;
    int explosion_marker_count;

    return M11_GameView_ProbeCsbRuntimeOverlayDrawStats(
               view,
               &object_sprite_count, &object_icon_count,
               &object_marker_count, &group_sprite_count,
               &group_marker_count, &projectile_sprite_count,
               &projectile_material_count, &projectile_marker_count,
               &explosion_sprite_count, &explosion_marker_count) &&
           group_sprite_count > 0 && group_marker_count == 0 &&
           object_marker_count == 0 && projectile_marker_count == 0 &&
           explosion_marker_count == 0;
}

static int f31_visible_group_sprite_matches_img2(const M11_GameViewState *view)
{
    const CSB_V1_BootProfile *profile;
    CSB_V1_ViewportRuntimeThingOverlay overlays[80];
    size_t overlay_count;
    size_t overlay_index;

    if (!view || !(profile = (const CSB_V1_BootProfile *)view->csbBootProfile) ||
        !profile->graphics_path[0]) return 0;
    overlay_count = csb_v1_viewport_runtime_collect_thing_overlays(
        &profile->runtime, overlays, sizeof(overlays) / sizeof(overlays[0]));
    if (overlay_count > sizeof(overlays) / sizeof(overlays[0])) return 0;
    for (overlay_index = 0u; overlay_index < overlay_count; ++overlay_index) {
        const CSB_V1_ViewportRuntimeThingOverlay *overlay =
            &overlays[overlay_index];
        const CSB_V1_ViewportRuntimeGroupOverlayPlacement *placement;
        const M11_AssetSlot *asset;
        CSB_V1_FmtownsItemDecodeReceipt receipt;
        uint8_t *graphics;
        uint8_t *expected;
        size_t graphics_size = 0u;
        unsigned int graphic;
        int mirrored = 0;
        int matches;

        /* C004's source collision leaves the party at (22,18), then C002
         * faces the west Prison group at (21,18). Do not let a different
         * visible group satisfy this exact retail-state regression. */
        if (overlay->kind != CSB_V1_VIEWPORT_RUNTIME_OVERLAY_GROUP ||
            overlay->map_x != 21 || overlay->map_y != 18) continue;
        placement = &overlay->group_placement;
        graphic = dm1_creature_sprite_for_view(
            placement->sprite_creature_type, placement->depth_index,
            placement->sprite_direction, profile->runtime.party_dir, 0,
            &mirrored);
        if (graphic == 0u) continue;
        asset = M11_AssetLoader_Load((M11_AssetLoader *)&view->assetLoader,
                                     graphic);
        graphics = load_profile_graphics(profile, &graphics_size);
        expected = (uint8_t *)malloc(640u * 400u);
        memset(&receipt, 0, sizeof(receipt));
        matches = asset && asset->loaded && asset->pixels && graphics &&
            expected && csb_v1_fmtowns_graphics_decode_item(
                graphics, graphics_size, graphic, expected, 640u * 400u,
                &receipt) && receipt.valid && receipt.is_image &&
            asset->width == receipt.width && asset->height == receipt.height &&
            memcmp(asset->pixels, expected, receipt.pixel_count) == 0;
        free(expected);
        free(graphics);
        if (matches) return 1;
    }
    return 0;
}

static uint32_t f31_advance_random_words(uint32_t state, unsigned int count)
{
    while (count-- != 0u)
        state = state * UINT32_C(0xbb40e62d) + UINT32_C(11);
    return state;
}

static int write_damaged_file(const char *path)
{
    static const unsigned char damaged[] = { 'b', 'a', 'd' };
    FILE *file = fopen(path, "wb");
    int ok;
    if (!file) return 0;
    ok = fwrite(damaged, 1u, sizeof(damaged), file) == sizeof(damaged);
    return fclose(file) == 0 && ok;
}

/* This mutates only a private temporary copy of the supplied original save,
 * after F0435 has admitted it. It proves receipt binding; it is not game
 * data manufactured for the test. */
static int flip_file_byte(const char *path, long offset)
{
    FILE *file;
    int value;

    if (!path || offset < 0) return 0;
    file = fopen(path, "r+b");
    if (!file) return 0;
    if (fseek(file, offset, SEEK_SET) != 0 ||
        (value = fgetc(file)) == EOF || fseek(file, offset, SEEK_SET) != 0 ||
        fputc(value ^ 0x01, file) == EOF) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static int test_set_env(const char *name, const char *value)
{
#ifdef _WIN32
    return _putenv_s(name, value ? value : "") == 0;
#else
    if (value) return setenv(name, value, 1) == 0;
    return unsetenv(name) == 0;
#endif
}

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR");
    const char *archive_data_dir =
        getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE_DATA_DIR");
    const char *loose_data_dir =
        getenv("FIRESTAFF_CSB_FMTOWNS_LOOSE_DATA_DIR");
    const char *language_name = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    const char *user_save_path = getenv("FIRESTAFF_CSB_FMTOWNS_USER_SAVE");
    const char *version_id;
    const char *expected_program;
    const char *expected_utility_program;
    uint32_t expected_mini_size;
    uint32_t expected_mini_fnv1a;
    uint16_t expected_mini_header_key;
    uint16_t expected_mini_header_platform;
    uint32_t expected_utility_load_size;
    uint32_t expected_utility_initial_eip;
    CSB_V1_FmtownsSwitchLanguage language;
    char materialized_data_dir[M12_ASSET_DATA_DIR_CAPACITY];
    char materialized_mini_path[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus asset_status;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    M11_BootProbeReceipt boot_probe;
    unsigned char framebuffer[320 * 200];
    uint8_t presented_palette[256][3];
    unsigned int tick;
    int result;
    int door_frame_seen = 0;
    int live_frame_nonblack = 0;
    uint32_t live_viewport_hash = 0u;
    int game_music_started = 0;
    uint8_t expected_game_music_track = 0u;
    CSB_V1_FmtownsSwitchInputReceipt switch_input;
    CSB_V1_StartupRuntimeAssetSession_PC34 direct_session;
    CSB_V1_StartupFullRuntimeReceipt_PC34 direct_runtime;
    CSB_V1_FmtownsGameHandoffReceipt direct_handoff;
    CSB_V1_FmtownsUserSaveReceipt user_save;
#ifndef _WIN32
    CSB_V1_FmtownsUserSaveReceipt stale_user_save;
#endif
    CSB_V1_FmtownsUserSaveReceipt recovered_user_save;
    CSB_V1_FmtownsStartupState user_save_state;
    CSB_V1_FmtownsGameHandoffReceipt external_save_handoff;
    CSB_V1_PartyState mini_party;
    CSB_V1_FmtownsStartupPortraitReceipt mini_portraits;
    CSB_V1_FmtownsStartupState mini_state;
    CSB_V1_FmtownsStartupState external_save_state;
    CSB_V1_DungeonData mini_dungeon;
    CSB_V1_FmtownsUtilityHandoffReceipt utility_handoff;
    CSB_V1_FmtownsUtilitySaveMappingReceipt utility_save_mapping;
    CSB_V1_FmtownsUtilityMenuReceipt utility_menu;
    CSB_V1_FmtownsUtilityGameSourceReceipt utility_game_source;
    CSB_V1_FmtownsUtilityFontReceipt utility_font;
    CSB_V1_FmtownsUtilityPortraitCatalog utility_portrait_catalog;
    CSB_V1_FmtownsUtilityPortraitSelector utility_portrait_selector;
    CSB_V1_FmtownsUtilityFilePicker utility_file_picker;
    CSB_V1_FmtownsUtilityRenderReceipt utility_render;
    CSB_V1_FmtownsUtilityMenuHitBox utility_hit;
    CSB_V1_FmtownsItemDecodeReceipt utility_arrows_decode;
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal;
    DM1_V1_ChampionStatusRectPc34 champion_name_rect;
    DM1_V1_EntranceMenuRouteReceiptPc34 hoc_menu_route;
    const CSB_V1_FmtownsSwitchButton *story_button;
    uint8_t music_track;
    unsigned int mini_active_index;
    uint8_t utility_palette[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3];
    uint8_t utility_frame[CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS];
    const uint8_t *japanese_utility_frame = NULL;
    int japanese_utility_width = 0;
    int japanese_utility_height = 0;
    uint8_t utility_arrows[32u * 75u];
    uint8_t portrait_pixels[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
    uint8_t portrait_roundtrip[CSB_FMTOWNS_PORTRAIT_DATA_SIZE];
    unsigned char fmtowns_actions[3];
    uint8_t utility_portrait_before[CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES];
    uint8_t *encoded_dungeon_tail = NULL;
    uint8_t *original_dungeon_tail = NULL;
    uint8_t *materialized_mini = NULL;
    size_t materialized_mini_size = 0u;
    int utility_fill_x = -1;
    int utility_fill_y = -1;

    if (language_name && strcmp(language_name, "ja") == 0) {
        language = CSB_FMTOWNS_SWITCH_JAPANESE;
        version_id = "fmtowns-ja";
        expected_program = "CHTWJ.EXP";
        expected_utility_program = "UTILJ.EXP";
        expected_mini_size = 43208u;
        expected_mini_fnv1a = 0x284799d1u;
        expected_mini_header_key = 0xf77du;
        expected_mini_header_platform = 8u;
        expected_utility_load_size = 151987u;
        expected_utility_initial_eip = 65200u;
    } else if (!language_name || language_name[0] == '\0' ||
               strcmp(language_name, "en") == 0) {
        language = CSB_FMTOWNS_SWITCH_ENGLISH;
        version_id = "fmtowns-en";
        expected_program = "CHTWE.EXP";
        expected_utility_program = "UTILE.EXP";
        expected_mini_size = 42776u;
        expected_mini_fnv1a = 0x494999c9u;
        expected_mini_header_key = 0x340fu;
        expected_mini_header_platform = 7u;
        expected_utility_load_size = 151875u;
        expected_utility_initial_eip = 65024u;
    } else {
        fprintf(stderr, "SKIP: unsupported FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE\n");
        return 0;
    }
    memset(materialized_data_dir, 0, sizeof(materialized_data_dir));
    memset(&asset_status, 0, sizeof(asset_status));
    if (loose_data_dir && loose_data_dir[0]) {
        M12_AssetStatus_ScanGame(&asset_status, loose_data_dir, "csb");
        if (!M12_AssetStatus_MaterializeCSBRuntimeVersion(
                &asset_status, version_id, materialized_data_dir,
                sizeof(materialized_data_dir))) {
            fprintf(stderr, "SKIP: verified loose FM Towns %s CD root unavailable\n",
                    version_id);
            return 0;
        }
        CHECK(strstr(materialized_data_dir, "asset-cache") == NULL,
              "F31 retains the selected verified source package");
        if (snprintf(materialized_mini_path, sizeof(materialized_mini_path),
                     "%s/%s/MINI.DAT", materialized_data_dir,
                     language == CSB_FMTOWNS_SWITCH_ENGLISH ? "CDATA" : "CJDATA") < 0 ||
            (materialized_mini = load_file(materialized_mini_path,
                                           &materialized_mini_size)) == NULL) {
            fprintf(stderr, "FAIL: loose F31 root retains selected MINI.DAT path\n");
            ++failures;
        } else {
            CHECK(materialized_mini_size == expected_mini_size &&
                  fnv1a(materialized_mini, materialized_mini_size) ==
                      expected_mini_fnv1a,
                  "loose F31 root retains its selected original MINI.DAT");
            free(materialized_mini);
            materialized_mini = NULL;
        }
        data_dir = materialized_data_dir;
    } else if (archive_data_dir && archive_data_dir[0]) {
        M12_AssetStatus_ScanGame(&asset_status, archive_data_dir, "csb");
        if (!M12_AssetStatus_MaterializeCSBRuntimeVersion(
                &asset_status, version_id, materialized_data_dir,
                sizeof(materialized_data_dir))) {
            fprintf(stderr, "SKIP: verified FM Towns %s archive unavailable\n",
                    version_id);
            return 0;
        }
        data_dir = materialized_data_dir;
    }
    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR, "
             "FIRESTAFF_CSB_FMTOWNS_LOOSE_DATA_DIR or "
             "FIRESTAFF_CSB_FMTOWNS_ARCHIVE_DATA_DIR not set");
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.title = "CHAOS STRIKES BACK";
    spec.dataDir = data_dir;
    /* Direct test launches bypass M12's menu handoff, which normally
     * publishes the selected F31 graphics identity.  M11 must receive that
     * same real-media identity before it can distinguish the English F31
     * ZIP from a generic archive; Japanese also keeps its explicit switch.
     * These are the hashes authenticated from the selected CD member above,
     * not generated test media. */
    spec.verifiedAssetMd5 = language == CSB_FMTOWNS_SWITCH_JAPANESE
        ? "761d6fc588b31aeaaa9caf3725e111b9"
        : "405b757038eea3c263e60f240854d6de";
    spec.csbFmtownsJapanese =
        language == CSB_FMTOWNS_SWITCH_JAPANESE;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    M11_GameView_Init(&view);
    result = M11_GameView_Start(&view, &spec);
    CHECK(result,
          "verified F31 media opens its real TITLE.ANM owner");
    /* A broad loose F31 directory can contain both CDATA and CJDATA.  The
     * production launcher materializes the selected language into its own
     * cache before reaching M11; this direct-root test must never continue
     * with the sibling program merely because it was found first.  In
     * particular, continuing after a failed title owner used to dereference
     * an unopened handoff later in this test.  ReDMCSB COMPILE.H 199-243
     * keeps the English and Japanese executable/media families separate. */
    if (!result || !view.csbBootProfile ||
        ((const CSB_V1_BootProfile *)view.csbBootProfile)->variant_id !=
            (language == CSB_FMTOWNS_SWITCH_JAPANESE
                 ? CSB_V1_VARIANT_FMTOWNS_JA
                 : CSB_V1_VARIANT_FMTOWNS_EN)) {
        if (result) {
            fprintf(stderr,
                    "FAIL: requested FM Towns %s needs its selected "
                    "version-private runtime package\n",
                    language == CSB_FMTOWNS_SWITCH_JAPANESE ? "Japanese"
                                                              : "English");
            ++failures;
        }
        M11_GameView_Shutdown(&view);
        return 1;
    }
    CHECK(view.originalFontAvailable &&
              M11_Font_ResolvedGraphicIndex(&view.originalFont) == 695,
          "verified F31 M653 raw interface font is bound before title playback");
    CHECK(view.csbFmtownsTitleBound && !view.csbStartupRuntimeAssetSession,
          "FM Towns title remains separate from the Game entrance session");
    /* F31J C06 renders Shift-JIS through the user-authorised FMT_FNT.ROM.
     * Its absence is not permission to use a host font, so the Japanese
     * utility must remain closed until that real ROM device is supplied.
     * F31E has its own authenticated bitmap/text route and needs no ROM. */
    if (language == CSB_FMTOWNS_SWITCH_JAPANESE &&
        !getenv("FIRESTAFF_FMTOWNS_FONT_ROM")) {
        CHECK(!M11_GameView_EnterCsbFmtownsUtility(&view) &&
                  view.csbFmtownsSwitchBound && !view.csbFmtownsUtilityBound,
              "F31J C06 remains fail-closed without an authorised FMT_FNT.ROM");
    } else {
        CHECK(M11_GameView_EnterCsbFmtownsUtility(&view) &&
                  !view.csbFmtownsSwitchBound && view.csbFmtownsUtilityBound,
              "direct C06 entry reuses verified F31 boot before C06 takes ownership from SWITCHTW");
    }
    M11_GameView_Shutdown(&view);
    M11_GameView_Init(&view);
    result = M11_GameView_Start(&view, &spec);
    CHECK(result,
          "verified F31 media can restart its real TITLE.ANM owner after direct C06 entry");
    if (!result) {
        M11_GameView_Shutdown(&view);
        return 1;
    }
    memset(&boot_probe, 0, sizeof(boot_probe));
    CHECK(M11_GameView_GetBootProbeReceipt(&view, &boot_probe) &&
              strcmp(boot_probe.startupPhase, "csb-fmtowns-title") == 0 &&
              strcmp(boot_probe.startupAnimation, "title-anm") == 0 &&
              boot_probe.startupActive && boot_probe.startupAnimationActive &&
              boot_probe.startupTitleReady && !boot_probe.levelLoaded &&
              boot_probe.mapIndex == -1 && boot_probe.partyX == -1 &&
              boot_probe.partyY == -1 && boot_probe.partyDir == -1 &&
              boot_probe.championCount == -1 && boot_probe.runtimeTick == 0,
          "boot probe retains the standalone F31 TITLE.ANM phase");

    /* TITLE.ANM has 606 Timer-A ticks. Timer A expires at 18*(1024-100) us,
     * not once per 16 ms M11 wake. At 629 wakes only 605 source ticks have
     * elapsed, so the last native title frame must still be displayed. */
    for (tick = 0u; tick < 629u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    CHECK(view.csbFmtownsTitleBound && !view.csbFmtownsSwitchBound,
          "TITLE.ANM cannot advance at the host 16 ms wake cadence");
    (void)M11_GameView_AdvanceIdleTick(&view);
    CHECK(view.csbFmtownsSwitchBound &&
              view.csbFmtownsSwitchVblanksRemaining == 60u,
          "TITLE.ANM reaches SWITCHTW only after its 606th Timer-A tick");
    for (tick = 0u; tick < 80u &&
                       view.csbFmtownsSwitchVblanksRemaining != 0u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    CHECK(view.csbFmtownsSwitchBound &&
              view.csbFmtownsSwitchVblanksRemaining == 0u,
          "TITLE.ANM returns into the original ready SWITCHTW page");
    memset(&switch_input, 0, sizeof(switch_input));
    CHECK(csb_v1_fmtowns_switch_route_click(
              &view.csbFmtownsSwitchReceipt, view.csbFmtownsSwitchLanguage,
              52, 110, 1, &switch_input) &&
              switch_input.action == CSB_FMTOWNS_SWITCH_ACTION_GAME,
          "source SWITCHTW decoder classifies the Game rectangle as C03_GAME");
    story_button = &view.csbFmtownsSwitchReceipt.buttons[0];
    memset(&switch_input, 0, sizeof(switch_input));
    CHECK(story_button->width > 0u && story_button->height > 0u &&
              csb_v1_fmtowns_switch_route_click(
                  &view.csbFmtownsSwitchReceipt, view.csbFmtownsSwitchLanguage,
                  (int16_t)(story_button->x + story_button->width / 2u),
                  (int16_t)(story_button->y + story_button->height / 2u),
                  1, &switch_input) &&
              switch_input.action == CSB_FMTOWNS_SWITCH_ACTION_STORY &&
              M11_GameView_HandlePointerButton(
                  &view,
                  (int)(story_button->x + story_button->width / 2u),
                  (int)(story_button->y + story_button->height / 2u),
                  DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
              view.csbFmtownsTitleBound && !view.csbFmtownsSwitchBound &&
              !view.csbFmtownsEndingActive,
          "source SWITCHTW Story rectangle binds retail STORY.ANM");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    for (tick = 0u; tick < sizeof(framebuffer); ++tick) {
        if (framebuffer[tick] != 0u) {
            live_frame_nonblack = 1;
            break;
        }
    }
    CHECK(live_frame_nonblack,
          "F31 Story presents a decoded original animation frame");
    for (tick = 0u; tick < 12000u && !view.csbFmtownsSwitchBound; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    CHECK(view.csbFmtownsSwitchBound && !view.csbFmtownsTitleBound &&
              view.csbFmtownsSwitchLanguage == language,
          "F31 Story returns to the same original SWITCHTW language page");
    for (tick = 0u; tick < 80u &&
                       view.csbFmtownsSwitchVblanksRemaining != 0u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    CHECK(view.csbFmtownsSwitchVblanksRemaining == 0u,
          "F31 Story return observes the source SWITCHTW VBlank wait");
    memset(&direct_session, 0, sizeof(direct_session));
    memset(&direct_runtime, 0, sizeof(direct_runtime));
    CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              &direct_session),
          "real F31 GRAPHICS.DAT opens its C001--C005/C017/C040 session");
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &direct_session, &direct_runtime),
          "real F31 session satisfies the authenticated runtime surface set");
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&direct_session);
    memset(&direct_handoff, 0, sizeof(direct_handoff));
    CHECK(csb_v1_fmtowns_game_handoff_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              language, &direct_handoff) &&
              strcmp(direct_handoff.executable_name, expected_program) == 0 &&
              direct_handoff.startup_mini_verified &&
              direct_handoff.startup_mini_size == expected_mini_size &&
              direct_handoff.startup_mini_fnv1a == expected_mini_fnv1a &&
              direct_handoff.startup_mini_header_verified &&
              direct_handoff.startup_mini_header_key == expected_mini_header_key &&
              direct_handoff.startup_mini_header_format_id == 5u &&
              direct_handoff.startup_mini_header_platform ==
                  expected_mini_header_platform &&
              direct_handoff.startup_mini_header_dungeon_id == 13u &&
              direct_handoff.startup_mini_save_parts_verified &&
              direct_handoff.startup_mini_party_champion_count == 1u &&
              direct_handoff.startup_mini_game_time ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 82u : 88u) &&
              direct_handoff.startup_mini_party_map_x == 22 &&
              direct_handoff.startup_mini_party_map_y == 18 &&
              direct_handoff.startup_mini_party_direction == 2 &&
              direct_handoff.startup_mini_party_map_index == 4 &&
              direct_handoff.startup_mini_event_count == 23u &&
              direct_handoff.startup_mini_first_unused_event_index == 23u &&
              direct_handoff.startup_mini_current_active_group_count == 8u &&
              direct_handoff.startup_mini_event_maximum_count == 436u &&
              direct_handoff.startup_mini_active_group_capacity == 60u &&
              direct_handoff.startup_mini_verified_save_body_offset == 8236u &&
              direct_handoff.startup_mini_dungeon_tail_verified &&
              direct_handoff.startup_mini_dungeon_map_count == 11u &&
              direct_handoff.startup_mini_dungeon_column_count == 296u &&
              direct_handoff.startup_mini_first_map_offset_x == 17u &&
              direct_handoff.startup_mini_first_map_offset_y == 14u &&
              direct_handoff.music_table_verified &&
              csb_v1_fmtowns_game_music_track_at(&direct_handoff, 0u, 2u, 0u,
                                                  &music_track),
          "verified F31 profile resolves its language-owned Game program and MINI.DAT");
    if (user_save_path && user_save_path[0]) {
        M11_GameLaunchSpec resume_spec;
        M11_GameViewState resumed_view;
        M11_BootProbeReceipt resumed_probe;

        memset(&user_save, 0, sizeof(user_save));
        CHECK(csb_v1_fmtowns_game_user_save_open(
                  (const CSB_V1_BootProfile *)view.csbBootProfile,
                  &direct_handoff, user_save_path, &user_save) &&
                  user_save.valid && user_save.language == language &&
                  user_save.variant_id == direct_handoff.variant_id &&
                  user_save.platform == expected_mini_header_platform &&
                  (user_save.dungeon_id == 12u || user_save.dungeon_id == 13u) &&
                  user_save.source_size > 512u &&
                  user_save.event_maximum_count > 0u &&
                  user_save.active_group_capacity > 0u &&
                  user_save.dungeon_tail_size > 0u &&
                  user_save.dungeon_tail_offset > user_save.portraits_offset &&
                  user_save.dungeon_tail_offset + user_save.dungeon_tail_size + 2u ==
                      user_save.source_size,
              "F31 F0435 admits the authentic user CSBGAME.DAT and its four portraits");
        CHECK(user_save.active_group_capacity <=
                  CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY &&
                  sizeof(user_save_state.active_groups) /
                      sizeof(user_save_state.active_groups[0]) ==
                      CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY,
              "F31 user resume retains the source F0196 110-entry allocation envelope");
        memset(&user_save_state, 0, sizeof(user_save_state));
        CHECK(csb_v1_fmtowns_game_load_user_save_state(
                  &user_save, &user_save_state) && user_save_state.valid &&
                  user_save_state.game_time == user_save.game_time &&
                  user_save_state.party_map_index == user_save.party_map_index &&
                  user_save_state.dungeon.raw_data != NULL,
              "F31 F0435 transfers the authentic user save and appended dungeon atomically");
        csb_v1_fmtowns_game_startup_state_free(&user_save_state);
#ifndef _WIN32
        {
            char stale_dir[] = "/tmp/firestaff-f31-stale-XXXXXX";
            char stale_path[512];
            CSB_V1_FmtownsStartupState stale_state;

            memset(&stale_user_save, 0, sizeof(stale_user_save));
            memset(&stale_state, 0, sizeof(stale_state));
            CHECK(mkdtemp(stale_dir) != NULL &&
                      snprintf(stale_path, sizeof(stale_path),
                               "%s/CSBGAME.DAT", stale_dir) > 0 &&
                      copy_file(user_save_path, stale_path) &&
                      csb_v1_fmtowns_game_user_save_open(
                          (const CSB_V1_BootProfile *)view.csbBootProfile,
                          &direct_handoff, stale_path, &stale_user_save) &&
                      write_damaged_file(stale_path) &&
                      !csb_v1_fmtowns_game_load_user_save_state(
                          &stale_user_save, &stale_state),
                  "F31 resume rejects a user slot changed after F0435 admission");
            csb_v1_fmtowns_game_startup_state_free(&stale_state);
            remove(stale_path);
            rmdir(stale_dir);

            {
                CSB_V1_FmtownsGameHandoffReceipt stale_handoff;
                CSB_V1_FmtownsStartupPortraitReceipt stale_portraits;
                CSB_V1_DungeonData stale_dungeon;
                CSB_V1_PartyState stale_party;

                memset(&stale_handoff, 0, sizeof(stale_handoff));
                memset(&stale_portraits, 0, sizeof(stale_portraits));
                memset(&stale_dungeon, 0, sizeof(stale_dungeon));
                memset(&stale_party, 0, sizeof(stale_party));
                CHECK(mkdtemp(stale_dir) != NULL &&
                          snprintf(stale_path, sizeof(stale_path),
                                   "%s/CSBGAME.DAT", stale_dir) > 0 &&
                          copy_file(user_save_path, stale_path) &&
                          csb_v1_fmtowns_game_user_save_handoff_open(
                              (const CSB_V1_BootProfile *)view.csbBootProfile,
                              language, stale_path, &stale_handoff) &&
                          flip_file_byte(
                              stale_path,
                              (long)stale_handoff.startup_mini_verified_save_body_offset) &&
                          !csb_v1_fmtowns_game_load_startup_party(
                              &stale_handoff, &stale_party) &&
                          !csb_v1_fmtowns_game_load_startup_portraits(
                              &stale_handoff, &stale_portraits) &&
                          !csb_v1_fmtowns_game_load_startup_dungeon(
                              &stale_handoff, &stale_dungeon),
                      "F31 receipt readers reject a real save slot replaced after F0435 admission");
                csb_v1_dungeon_free(&stale_dungeon);
                remove(stale_path);
                rmdir(stale_dir);
            }
        }
#endif
        /* F0433/F7052 owns the canonical M746 CSBGAME.DAT medium, while the
         * external corpus deliberately gives its Japanese witness a distinct
         * filename.  Stage every writeback proof under the native basename:
         * this keeps the licensed corpus immutable and lets both language
         * bodies exercise the actual .BAK transaction rather than inventing
         * a CSBGAME-JP.BAK convention. */
#ifndef _WIN32
        {
            char writeback_dir[] = "/tmp/firestaff-f31-writeback-XXXXXX";
            char writeback_path[512];
            int writeback_staged = 0;

            writeback_staged = mkdtemp(writeback_dir) != NULL &&
                snprintf(writeback_path, sizeof(writeback_path),
                         "%s/CSBGAME.DAT", writeback_dir) > 0 &&
                copy_file(user_save_path, writeback_path);
            CHECK(writeback_staged,
                  "F31 writeback stages the real language-owned save under M746's canonical name");
            if (writeback_staged) {
        /* Direct CLI resume takes the same M11 start boundary as this spec:
         * CHTWE/CHTWJ owns F0435, then GAMELOOP owns the saved F31 state.
         * It must not route the Towns bytes through the Atari/CSBWin reader
         * or replay TITLE.ANM before the resumed live dungeon. */
        resume_spec = spec;
        resume_spec.savePath = writeback_path;
        M11_GameView_Init(&resumed_view);
        result = M11_GameView_Start(&resumed_view, &resume_spec);
        memset(&resumed_probe, 0, sizeof(resumed_probe));
        CHECK(result && M11_GameView_GetBootProbeReceipt(&resumed_view,
                                                           &resumed_probe) &&
                  !resumed_probe.startupActive && resumed_probe.levelLoaded &&
                  resumed_probe.mapIndex == user_save.party_map_index &&
                  resumed_probe.partyX == user_save.party_map_x &&
                  resumed_probe.partyY == user_save.party_map_y &&
                  resumed_probe.championCount == user_save.party_champion_count,
              "direct F31 resume enters CHTWE/CHTWJ GAMELOOP without title replay");
        if (result) {
            CSB_V1_BootProfile *resumed_profile =
                (CSB_V1_BootProfile *)resumed_view.csbBootProfile;
            uint32_t random_before = resumed_profile
                ? resumed_profile->runtime.csbwin_random_seed : 0u;

            CHECK(test_set_env("FIRESTAFF_QUICKSAVE_PATH", writeback_path),
                  "F31 resumed session keeps its canonical save slot selected");
            CHECK(resumed_profile && resumed_profile->runtime.party_state_valid &&
                      resumed_profile->runtime.csbwin_random_seed_valid &&
                      resumed_profile->runtime.dungeon_handle &&
                      resumed_profile->runtime.active_group_state_count <=
                          user_save.active_group_capacity &&
                      resumed_profile->runtime.timeline_queue.maxEvents >= 0 &&
                      resumed_profile->runtime.timeline_queue.maxEvents <=
                          user_save.event_maximum_count,
                  "F31 resumed runtime remains within its authenticated F0433 save envelope");
            CHECK(M11_GameView_QuickSave(&resumed_view),
                  "F31 resumed session writes its authenticated native CSBGAME.DAT slot");
            CHECK(resumed_profile && resumed_profile->runtime.csbwin_random_seed_valid &&
                      resumed_profile->runtime.csbwin_random_seed ==
                          f31_advance_random_words(
                              random_before, 16u + REDMCSB_F7062_RANDOM_WORDS),
                  "F31 resumed F7052/F7062 path consumes source RNG words exactly once");
            memset(&user_save, 0, sizeof(user_save));
            CHECK(csb_v1_fmtowns_game_user_save_open(
                      resumed_profile, &resumed_view.csbFmtownsGameHandoffReceipt,
                      writeback_path, &user_save) && user_save.valid &&
                      user_save.dungeon_tail_size > 0u,
                  "F31 resumed writeback remains readable through the native F0435 reader");
        }
        M11_GameView_Shutdown(&resumed_view);
            (void)test_set_env("FIRESTAFF_QUICKSAVE_PATH", NULL);
            }
            remove(writeback_path);
            {
                char writeback_backup[512];
                if (snprintf(writeback_backup, sizeof(writeback_backup),
                             "%s/CSBGAME.BAK", writeback_dir) > 0) {
                    remove(writeback_backup);
                }
            }
            rmdir(writeback_dir);
        }
#else
        /* The native backup test below is POSIX-only; keep Windows corpus
         * admission read-only until it has an equivalent private staging
         * primitive. */
        CHECK(1, "F31 Windows corpus writeback remains covered by native writer tests");
#endif
        memset(&external_save_handoff, 0, sizeof(external_save_handoff));
        CHECK(csb_v1_fmtowns_game_user_save_handoff_open(
                  (const CSB_V1_BootProfile *)view.csbBootProfile, language,
                  user_save_path, &external_save_handoff) &&
                  external_save_handoff.startup_mini_header_dungeon_id == 12u &&
                  external_save_handoff.startup_mini_dungeon_tail_verified,
              "legacy F31 handoff also admits the authentic Prison save");
#ifndef _WIN32
        {
            char recovery_dir[] = "/tmp/firestaff-f31-recovery-XXXXXX";
            char selected_path[512];
            char backup_path[512];
            CHECK(mkdtemp(recovery_dir) != NULL &&
                      snprintf(selected_path, sizeof(selected_path),
                               "%s/CSBGAME.DAT", recovery_dir) > 0 &&
                      snprintf(backup_path, sizeof(backup_path),
                               "%s/CSBGAME.BAK", recovery_dir) > 0 &&
                      copy_file(user_save_path, backup_path) &&
                      write_damaged_file(selected_path),
                  "real F31 save corpus and damaged primary are staged in isolation");
            /* The original disk's older BAK is itself retained as corpus but
             * cannot be promoted unless it validates.  A byte-for-byte copy
             * of the genuine current save models F0433's prior slot rotation
             * without fabricating a save body. */
            memset(&recovered_user_save, 0, sizeof(recovered_user_save));
            CHECK(csb_v1_fmtowns_game_user_save_open_or_restore_backup(
                      (const CSB_V1_BootProfile *)view.csbBootProfile,
                      &direct_handoff, selected_path, &recovered_user_save) &&
                      recovered_user_save.valid &&
                      recovered_user_save.recovered_from_backup &&
                      strcmp(recovered_user_save.source_path, selected_path) == 0,
                  "F31 F0435 restores validated CSBGAME.BAK to its canonical slot");
            memset(&external_save_handoff, 0, sizeof(external_save_handoff));
            CHECK(csb_v1_fmtowns_game_user_save_handoff_open(
                      (const CSB_V1_BootProfile *)view.csbBootProfile, language,
                      selected_path, &external_save_handoff) &&
                      external_save_handoff.valid &&
                      strcmp(external_save_handoff.startup_mini_path,
                             selected_path) == 0,
                  "F31 legacy handoff keeps backup recovery on its canonical slot");
            remove(selected_path);
            CHECK(mkdir(selected_path, 0700) == 0 &&
                      copy_file(user_save_path, backup_path) &&
                      !csb_v1_fmtowns_game_user_save_open_or_restore_backup(
                          (const CSB_V1_BootProfile *)view.csbBootProfile,
                          &direct_handoff, selected_path, &recovered_user_save) &&
                      access(backup_path, F_OK) == 0,
                  "F31 failed canonical replacement retains the validated backup");
            rmdir(selected_path);
            remove(selected_path);
            remove(backup_path);
            rmdir(recovery_dir);
        }
#endif
    }
    memset(&external_save_handoff, 0, sizeof(external_save_handoff));
    memset(&external_save_state, 0, sizeof(external_save_state));
    CHECK(csb_v1_fmtowns_game_user_save_handoff_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile, language,
              direct_handoff.startup_mini_path, &external_save_handoff) &&
              external_save_handoff.valid &&
              external_save_handoff.startup_mini_header_verified &&
              external_save_handoff.startup_mini_save_parts_verified &&
              external_save_handoff.startup_mini_dungeon_tail_verified &&
              csb_v1_fmtowns_game_load_startup_state(
                  &external_save_handoff, &external_save_state) &&
              external_save_state.game_time == direct_handoff.startup_mini_game_time &&
              external_save_state.party.PartyMapX == 22 &&
              external_save_state.party.PartyMapY == 18,
          "F31 F0435 admits an external native save candidate without a retail hash");
    csb_v1_fmtowns_game_startup_state_free(&external_save_state);
    memset(&mini_party, 0, sizeof(mini_party));
    CHECK(csb_v1_fmtowns_game_load_startup_party(&direct_handoff, &mini_party) &&
              mini_party.ChampionCount == 1 &&
              mini_party.PartyDirection == 2 &&
              mini_party.PartyMapX == 22 && mini_party.PartyMapY == 18 &&
              mini_party.Champions[0].Name[0] != '\0' &&
              mini_party.Champions[0].CurrentHealth > 0,
          "F31 MINI.DAT supplies its checksum-verified champion record without a fixture");
    memset(&mini_portraits, 0, sizeof(mini_portraits));
    CHECK(csb_v1_fmtowns_game_load_startup_portraits(
              &direct_handoff, &mini_portraits) && mini_portraits.valid &&
              mini_portraits.source_size ==
                  CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT *
                      CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES &&
              mini_portraits.source_file_offset ==
                  direct_handoff.startup_mini_dungeon_tail_offset -
                      mini_portraits.source_size &&
              mini_portraits.source_fnv1a ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0x748ce10fu :
                                                           0x4facab0bu),
          "F31 MINI.DAT preserves the four original C06 portrait payloads");
    CHECK(csb_v1_fmtowns_portrait_decode_planar(
              mini_portraits.source_bytes[0],
              CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES, portrait_pixels,
              sizeof(portrait_pixels)) &&
              csb_v1_fmtowns_portrait_encode_planar(
                  portrait_pixels, sizeof(portrait_pixels), portrait_roundtrip,
                  sizeof(portrait_roundtrip)) &&
              memcmp(mini_portraits.source_bytes[0], portrait_roundtrip,
                     sizeof(portrait_roundtrip)) == 0,
          "F31 C06 portrait F7251/F7252 conversion preserves real MINI bytes");
    memset(&mini_state, 0, sizeof(mini_state));
    CHECK(csb_v1_fmtowns_game_load_startup_state(&direct_handoff, &mini_state) &&
              mini_state.valid && mini_state.game_time ==
                  direct_handoff.startup_mini_game_time &&
              mini_state.party_map_index == 4 &&
              mini_state.party.ChampionCount == 1 &&
              mini_state.timeline_queue.gameTick == mini_state.game_time &&
              mini_state.timeline_queue.eventCount == 23 &&
              mini_state.timeline_queue.firstUnusedIndex == 23 &&
              mini_state.timeline_queue.maxEvents == 436 &&
              mini_state.active_group_capacity == 60u &&
              mini_state.active_group_count == 8u &&
              mini_state.active_group_resolved_count == 8u,
          "F31 MINI.DAT preserves its actual event heap and active-group bytes");
    if (mini_state.valid && mini_state.dungeon.raw_data &&
        direct_handoff.startup_mini_dungeon_tail_size > 0u) {
        const size_t tail_size = direct_handoff.startup_mini_dungeon_tail_size;
        const size_t serialized_size = tail_size + 2u;
        encoded_dungeon_tail = (uint8_t *)malloc(serialized_size);
        original_dungeon_tail = (uint8_t *)malloc(tail_size);
        CHECK(encoded_dungeon_tail && original_dungeon_tail &&
                  csb_v1_fmtowns_game_encode_dungeon_tail(
                      &mini_state.dungeon, encoded_dungeon_tail,
                      serialized_size) &&
                  csb_v1_fmtowns_game_copy_verified_dungeon_tail(
                      &direct_handoff, original_dungeon_tail, tail_size) &&
                  memcmp(encoded_dungeon_tail, original_dungeon_tail,
                         tail_size) == 0,
              "F31 source dungeon tail serializes byte-identically before mutation");
        free(encoded_dungeon_tail);
        free(original_dungeon_tail);
        encoded_dungeon_tail = NULL;
        original_dungeon_tail = NULL;
    }
    for (mini_active_index = 0u; mini_active_index < 8u;
         ++mini_active_index) {
        CHECK(mini_state.active_group_owners[mini_active_index].valid &&
                  mini_state.active_group_owners[mini_active_index].map_index == 4,
              "F31 ACTIVE_GROUP owner resolves to one C04 on the saved party map");
    }
    csb_v1_fmtowns_game_startup_state_free(&mini_state);
    memset(&mini_dungeon, 0, sizeof(mini_dungeon));
    CHECK(csb_v1_fmtowns_game_load_startup_dungeon(
                  &direct_handoff, &mini_dungeon) &&
              mini_dungeon.level_count == 11 &&
              mini_dungeon.map_offset_x[0] == 17 &&
              mini_dungeon.map_offset_y[0] == 14,
          "F31 MINI.DAT dungeon tail opens through the real CSB dungeon loader");
    csb_v1_dungeon_free(&mini_dungeon);
    memset(&utility_handoff, 0, sizeof(utility_handoff));
    CHECK(csb_v1_fmtowns_utility_handoff_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              language, &utility_handoff) &&
              strcmp(utility_handoff.executable_name,
                     expected_utility_program) == 0 &&
              utility_handoff.p3_header_verified &&
              utility_handoff.p3_header_size == 384u &&
              utility_handoff.p3_load_image_offset == 512u &&
              utility_handoff.p3_load_image_size == expected_utility_load_size &&
              utility_handoff.p3_initial_eip == expected_utility_initial_eip &&
              utility_handoff.static_art_verified &&
              utility_handoff.mirror_bitmap_fnv1a == 0xf8a19ba4u &&
              utility_handoff.file_picker_arrows_fnv1a == 0xe2226054u &&
              utility_handoff.mirror_bitmap_file_offset ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0x14e78u :
                                                           0x14ee0u) &&
              utility_handoff.file_picker_arrows_file_offset ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0x14f70u :
                                                           0x14fd8u),
          "verified F31 profile resolves its language-owned C06 P3 envelope");
    memset(&utility_save_mapping, 0, sizeof(utility_save_mapping));
    CHECK(csb_v1_fmtowns_utility_save_mapping_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              language, &utility_save_mapping) &&
              utility_save_mapping.valid &&
              strcmp(utility_save_mapping.template_bytes,
                     "2:\\#CHAMP_NAME#.CMP") == 0 &&
              utility_save_mapping.source_file_offset ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0x11ffcu :
                                                           0x12064u),
          "F7000 save mapping is bound to the authentic C06 image");
    memset(&utility_menu, 0, sizeof(utility_menu));
    CHECK(csb_v1_fmtowns_utility_menu_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              language, &utility_menu) && utility_menu.valid &&
              utility_menu.source_size ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 76u : 68u) &&
              utility_menu.source_fnv1a ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0xfd9986bfu :
                                                           0xdceefc60u),
          "verified F31 profile resolves its language-owned C06 menu bytes");
    memset(&utility_game_source, 0, sizeof(utility_game_source));
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        CHECK(csb_v1_fmtowns_utility_game_source_open(
                  (const CSB_V1_BootProfile *)view.csbBootProfile,
                  language, &utility_game_source) && utility_game_source.valid &&
                  utility_game_source.title_file_offset == 0x11aa0u &&
                  utility_game_source.choices_file_offset == 0x11b4cu &&
                  utility_game_source.save_prompt_file_offset == 0x11db8u &&
                  utility_game_source.ok_file_offset == 0x11afeu &&
                  memcmp(utility_game_source.title, "LOAD WHICH SAVED GAME?", 22u) == 0 &&
                  memcmp(utility_game_source.choices,
                         "DUNGEON MASTER\nCHAOS STRIKES BACK\nCANCEL", 39u) == 0 &&
                  memcmp(utility_game_source.save_prompt,
                         "PLEASE PUT THE GAME SAVE\nDISK IN %DEVICE%", 41u) == 0 &&
                  memcmp(utility_game_source.ok, "OK", 3u) == 0,
              "F31E C06 source chooser and A: prompt retain their exact UTILE P3 bytes");
    } else {
        const int game_source_open = csb_v1_fmtowns_utility_game_source_open(
            (const CSB_V1_BootProfile *)view.csbBootProfile,
            language, &utility_game_source);
        if (!game_source_open ||
            utility_game_source.title_file_offset != 0x11aa0u ||
            utility_game_source.choices_file_offset != 0x11b4cu ||
            utility_game_source.save_prompt_file_offset != 0x11db8u ||
            utility_game_source.ok_file_offset != 0x11afeu ||
            utility_game_source.title[0] != 0x82u ||
            utility_game_source.title[1] != 0xb1u ||
            utility_game_source.choices[0] != 0x8eu ||
            utility_game_source.choices[1] != 0x6eu ||
            utility_game_source.save_prompt[0] != 0xb5u ||
            utility_game_source.save_prompt[1] != 0x82u ||
            utility_game_source.ok[0] != 0x82u ||
            utility_game_source.ok[1] != 0xc5u) {
            printf("F31J C06 source diagnostic: open=%d valid=%d offsets=%08x/%08x/%08x/%08x bytes=%02x%02x/%02x%02x/%02x%02x/%02x%02x\n",
                   game_source_open, utility_game_source.valid,
                   utility_game_source.title_file_offset,
                   utility_game_source.choices_file_offset,
                   utility_game_source.save_prompt_file_offset,
                   utility_game_source.ok_file_offset,
                   utility_game_source.title[0], utility_game_source.title[1],
                   utility_game_source.choices[0], utility_game_source.choices[1],
                   utility_game_source.save_prompt[0], utility_game_source.save_prompt[1],
                   utility_game_source.ok[0], utility_game_source.ok[1]);
        }
        CHECK(game_source_open && utility_game_source.valid &&
                  utility_game_source.title_file_offset == 0x11aa0u &&
                  utility_game_source.choices_file_offset == 0x11b4cu &&
                  utility_game_source.save_prompt_file_offset == 0x11db8u &&
                  utility_game_source.ok_file_offset == 0x11afeu &&
                  utility_game_source.title[0] == 0x82u &&
                  utility_game_source.title[1] == 0xb1u &&
                  utility_game_source.choices[0] == 0x8eu &&
                  utility_game_source.choices[1] == 0x6eu &&
                  utility_game_source.save_prompt[0] == 0xb5u &&
                  utility_game_source.save_prompt[1] == 0x82u &&
                  utility_game_source.ok[0] == 0x82u &&
                  utility_game_source.ok[1] == 0xc5u,
              "F31J C06 binds its own authenticated Shift-JIS source and save-medium strings");
    }
    memset(&utility_font, 0, sizeof(utility_font));
    CHECK(csb_v1_fmtowns_utility_font_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile,
              language, &utility_font) && utility_font.valid &&
              utility_font.source_size ==
                  CSB_V1_FMTOWNS_UTILITY_INTERFACE_FONT_BYTES &&
              utility_font.source_fnv1a == 0x8c36f65bu &&
              utility_font.source_file_offset ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0x150d8u :
                                                           0x15140u),
          "C06 interface font is read from the selected retail utility image");
    memset(&utility_portrait_catalog, 0, sizeof(utility_portrait_catalog));
    CHECK(csb_v1_fmtowns_utility_portrait_catalog_open(
              (const CSB_V1_BootProfile *)view.csbBootProfile, language,
              &utility_portrait_catalog) && utility_portrait_catalog.valid &&
              utility_portrait_catalog.entry_count == 24u &&
              utility_portrait_catalog.rejected_entry_count == 0u &&
              strcmp(utility_portrait_catalog.entries[0].filename, "ALEX.CMP") == 0 &&
              utility_portrait_catalog.entries[0].portrait.valid,
          "F31 C06 FILE_PICKER catalogues only real admitted PORTRAIT CMP files");
    memset(&utility_file_picker, 0, sizeof(utility_file_picker));
    CHECK(csb_v1_fmtowns_utility_file_picker_open(
              &utility_portrait_catalog, 0u, &utility_file_picker) &&
              utility_file_picker.valid && utility_file_picker.first_index == 0u &&
              utility_file_picker.selected_index == 0u,
          "F31 C06 F7083 opens the native file-picker state on the real catalog");
    {
        int picker_command = 0;
        int picker_index = -1;
        CHECK(csb_v1_fmtowns_utility_file_picker_input(
                  &utility_file_picker, 80, 63, &picker_command,
                  &picker_index) &&
                  picker_command == CSB_V1_FMTOWNS_FILE_PICKER_FILE_LIST &&
                  picker_index == 0,
              "F31 C06 F7084 resolves the first authentic file-list row");
        CHECK(csb_v1_fmtowns_utility_file_picker_input(
                  &utility_file_picker, 140, 120, &picker_command,
                  &picker_index) &&
                  picker_command == CSB_V1_FMTOWNS_FILE_PICKER_DOWN &&
                  utility_file_picker.first_index == 1u && picker_index == -1,
              "F31 C06 F7084 applies bounded one-row down scrolling");
        CHECK(csb_v1_fmtowns_utility_file_picker_input(
                  &utility_file_picker, 180, 130, &picker_command,
                  &picker_index) &&
                  picker_command == CSB_V1_FMTOWNS_FILE_PICKER_CANCEL &&
                  picker_index == -1,
              "F31 C06 F7084 preserves the source cancel command");
    }
    memset(&utility_portrait_selector, 0, sizeof(utility_portrait_selector));
    CHECK(csb_v1_fmtowns_utility_portrait_selector_open(
              &utility_portrait_catalog, 0u, &utility_portrait_selector) &&
              utility_portrait_selector.valid &&
              utility_portrait_selector.selected_index == 0u &&
              utility_portrait_selector.entry_count ==
                  utility_portrait_catalog.entry_count,
          "F31 C06 selector binds its initial row to the authenticated catalog");
    CHECK(csb_v1_fmtowns_utility_portrait_selector_move(
              &utility_portrait_selector, 1) &&
              utility_portrait_selector.selected_index == 1u &&
              csb_v1_fmtowns_utility_portrait_selector_move(
                  &utility_portrait_selector, -1) &&
              utility_portrait_selector.selected_index == 0u &&
              !csb_v1_fmtowns_utility_portrait_selector_move(
                  &utility_portrait_selector, -1),
          "F31 C06 selector follows bounded source arrow movement");
    if (utility_portrait_catalog.entry_count > 0u &&
        mini_party.ChampionCount > 0) {
        CSB_V1_PartyState selector_party = mini_party;
        CSB_V1_FmtownsStartupPortraitReceipt selector_portraits = mini_portraits;
        CHECK(csb_v1_fmtowns_utility_portrait_selector_load(
                  &utility_portrait_selector, &selector_party, 0u,
                  &selector_portraits) &&
                  strcmp(selector_party.Champions[0].Name,
                         utility_portrait_catalog.entries[0].portrait.name) == 0,
              "F31 C06 selector load revalidates the selected authentic CMP");
    }
#ifndef _WIN32
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH &&
        utility_portrait_catalog.valid && utility_portrait_catalog.entry_count > 0u) {
        char temporary_dir[] = "/tmp/firestaff-csb-cmp-save-XXXXXX";
        CSB_V1_FmtownsUtilityPortraitCatalog copied_catalog =
            utility_portrait_catalog;
        CSB_V1_PartyState copied_party = mini_party;
        CSB_V1_FmtownsStartupPortraitReceipt copied_portraits = mini_portraits;
        unsigned int entry_index;
        int temporary_ok = mkdtemp(temporary_dir) != NULL;
        if (temporary_ok) {
            for (entry_index = 0u;
                 entry_index < copied_catalog.entry_count; ++entry_index) {
                char destination[sizeof(copied_catalog.entries[entry_index].source_path)];
                int written = snprintf(destination, sizeof(destination), "%s/%s",
                                        temporary_dir,
                                        copied_catalog.entries[entry_index].filename);
                if (written < 0 || (size_t)written >= sizeof(destination) ||
                    !copy_file(copied_catalog.entries[entry_index].source_path,
                               destination)) {
                    temporary_ok = 0;
                    break;
                }
                snprintf(copied_catalog.entries[entry_index].source_path,
                         sizeof(copied_catalog.entries[entry_index].source_path),
                         "%s", destination);
            }
        }
        /* The live MINI.DAT party name is the source key for F7001. If this
         * corpus has a one-champion seed, exercise precisely that admitted
         * name and payload; never create a test-only CMP record. */
        if (temporary_ok && copied_party.ChampionCount > 0) {
            int matched = 0;
            for (entry_index = 0u;
                 entry_index < copied_catalog.entry_count; ++entry_index) {
                if (strncmp(copied_party.Champions[0].Name,
                            copied_catalog.entries[entry_index].portrait.name,
                            CSB_V1_MAX_NAME_LEN) == 0) {
                    matched = 1;
                    break;
                }
            }
            CHECK(matched, "F31 MINI.DAT party name matches an admitted CMP record");
            if (matched) {
                /* F7002_ReadCMP is exercised against a different real
                 * catalogue entry.  This is an in-memory import only: the
                 * source CMP remains untouched and no test portrait is
                 * created. */
                if (copied_catalog.entry_count > 1u) {
                    CSB_V1_PartyState loaded_party = mini_party;
                    CSB_V1_FmtownsStartupPortraitReceipt loaded_portraits =
                        mini_portraits;
                    unsigned int load_index =
                        entry_index == 0u ? 1u : 0u;
                    uint8_t load_file[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
                    FILE *load_handle = fopen(
                        copied_catalog.entries[load_index].source_path, "rb");
                    int load_read = load_handle &&
                        fread(load_file, 1u, sizeof(load_file), load_handle) ==
                            sizeof(load_file);
                    if (load_handle) fclose(load_handle);
                    CHECK(load_read &&
                              csb_v1_fmtowns_utility_load_portrait(
                                  &copied_catalog, (uint16_t)load_index,
                                  &loaded_party, 0u, &loaded_portraits) &&
                              strcmp(loaded_party.Champions[0].Name,
                                     copied_catalog.entries[load_index].portrait.name) == 0 &&
                              strcmp(loaded_party.Champions[0].Title,
                                     copied_catalog.entries[load_index].portrait.title) == 0 &&
                              memcmp(loaded_portraits.source_bytes[0],
                                     load_file + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE,
                                     CSB_FMTOWNS_PORTRAIT_DATA_SIZE) == 0,
                          "F7002 imports only the selected authentic CMP record");
                }
                uint8_t before[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
                uint8_t after[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
                unsigned int target = entry_index;
                FILE *file = fopen(copied_catalog.entries[target].source_path, "rb");
                size_t payload_offset = CSB_FMTOWNS_PORTRAIT_HEADER_SIZE;
                int read_ok = file && fread(before, 1u, sizeof(before), file) == sizeof(before);
                if (file) fclose(file);
                if (read_ok) {
                    copied_portraits.source_bytes[0][0] ^= 0x01u;
                    snprintf(copied_party.Champions[0].Title,
                             sizeof(copied_party.Champions[0].Title),
                             "%s", "SOURCE TITLE");
                    CHECK(csb_v1_fmtowns_utility_save_portraits(
                              &copied_catalog, &copied_party, &copied_portraits),
                          "F7001 saves an admitted existing CMP atomically");
                    file = fopen(copied_catalog.entries[target].source_path, "rb");
                    read_ok = file && fread(after, 1u, sizeof(after), file) == sizeof(after);
                    if (file) fclose(file);
                    CHECK(read_ok &&
                              memcmp(before, after, 24u) == 0 &&
                              memcmp(after + 24u,
                                     copied_party.Champions[0].Title,
                                     strlen(copied_party.Champions[0].Title)) == 0 &&
                              after[24u + strlen(copied_party.Champions[0].Title)] == 0 &&
                              memcmp(after + payload_offset,
                                     copied_portraits.source_bytes[0],
                                     CSB_FMTOWNS_PORTRAIT_DATA_SIZE) == 0,
                          "F7001 preserves CMP identity fields and writes live title plus planar payload");
                }
                /* F7000 writes the selected champion to its dynamic drive
                 * rather than overwriting an admitted CD catalogue entry.
                 * Point HOME at this already-cleaned test root so the real
                 * POSIX mapping is exercised without touching user data. */
                {
                    char saved_path[FSP_PATH_MAX];
                    char saved_dir[FSP_PATH_MAX];
                    char firestaff_dir[FSP_PATH_MAX];
                    char previous_home[FSP_PATH_MAX];
                    const char *home = getenv("HOME");
                    uint8_t saved_record[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
                    FILE *saved_file;
                    int saved_read;
                    int restore_home;
                    snprintf(previous_home, sizeof(previous_home), "%s",
                             home ? home : "");
                    FSP_SetEnv("HOME", temporary_dir, 1);
                    CHECK(csb_v1_fmtowns_utility_save_selected_portrait(
                              &utility_save_mapping,
                              &copied_catalog, &copied_party, &copied_portraits,
                              0u, saved_path, sizeof(saved_path)),
                          "F7000 writes the selected CMP to the mapped portrait medium");
                    saved_file = fopen(saved_path, "rb");
                    saved_read = saved_file &&
                        fread(saved_record, 1u, sizeof(saved_record), saved_file) ==
                            sizeof(saved_record);
                    if (saved_file) fclose(saved_file);
                    CHECK(saved_read &&
                              csb_v1_fmtowns_portrait_probe(saved_record,
                                                            sizeof(saved_record)) &&
                              memcmp(saved_record, before, 16u) == 0 &&
                              memcmp(saved_record + 24u,
                                     copied_party.Champions[0].Title,
                                     strlen(copied_party.Champions[0].Title)) == 0 &&
                              memcmp(saved_record + payload_offset,
                                     copied_portraits.source_bytes[0],
                                     CSB_FMTOWNS_PORTRAIT_DATA_SIZE) == 0,
                          "F7000 preserves source header and writes live selected CMP fields");
                    {
                        CSB_V1_FmtownsUtilityPortraitCatalog saved_catalog;
                        memset(&saved_catalog, 0, sizeof(saved_catalog));
                        CHECK(csb_v1_fmtowns_utility_portrait_medium_catalog_open(
                                  &utility_save_mapping, language, &saved_catalog) &&
                                  saved_catalog.entry_count == 1u &&
                                  strcmp(saved_catalog.entries[0].source_path,
                                         saved_path) == 0,
                              "C06 NEW DISK reopens the mapped portrait medium");
                    }
                    restore_home = previous_home[0] != '\0'
                        ? FSP_SetEnv("HOME", previous_home, 1)
                        : FSP_UnsetEnv("HOME");
                    CHECK(restore_home == 0, "F7000 test restores HOME");
                    if (FSP_ParentDir(saved_dir, sizeof(saved_dir), saved_path) &&
                        FSP_ParentDir(firestaff_dir, sizeof(firestaff_dir), saved_dir)) {
                        remove(saved_path);
                        rmdir(saved_dir);
                        rmdir(firestaff_dir);
                    }
                }
            }
        }
        for (entry_index = 0u;
             entry_index < copied_catalog.entry_count; ++entry_index)
            remove(copied_catalog.entries[entry_index].source_path);
        rmdir(temporary_dir);
    }
#endif
    CHECK(csb_v1_fmtowns_utility_menu_action_at(
              &utility_menu,
              language == CSB_FMTOWNS_SWITCH_ENGLISH ? 102 : 98,
              language == CSB_FMTOWNS_SWITCH_ENGLISH ? 194 : 196,
              &utility_hit) &&
              utility_hit.action ==
                  CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS &&
              csb_v1_fmtowns_utility_menu_action_at(
                  &utility_menu,
                  language == CSB_FMTOWNS_SWITCH_ENGLISH ? 288 : 266,
                  language == CSB_FMTOWNS_SWITCH_ENGLISH ? 5 : 6,
                  &utility_hit) &&
              utility_hit.action == CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT,
          "verified F31 profile retains its language-owned C06 input boxes");
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        size_t arrow_row;
        size_t non_padding_pixels = 0u;
        int padding_is_zero = 1;
        int utility_arrows_ok;
        memset(&utility_arrows_decode, 0, sizeof(utility_arrows_decode));
        memset(utility_arrows, 0xff, sizeof(utility_arrows));
        utility_arrows_ok = csb_v1_fmtowns_img2_decode_strided(
                  utility_handoff.file_picker_arrows,
                  CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_STREAM_BYTES,
                  31u, 75u,
                  32u, utility_arrows, sizeof(utility_arrows),
                  &utility_arrows_decode);
        CHECK(utility_arrows_ok && utility_arrows_decode.valid &&
              utility_arrows_decode.stream_bytes_consumed ==
                  CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_STREAM_BYTES &&
              utility_arrows_decode.pixel_count == sizeof(utility_arrows),
              "C06 F0689 decodes the verified 31x75 file-picker stream with its 32-pixel stride");
        for (arrow_row = 0u; arrow_row < 75u; ++arrow_row) {
            size_t arrow_column;
            if (utility_arrows[arrow_row * 32u + 31u] != 0u)
                padding_is_zero = 0;
            for (arrow_column = 0u; arrow_column < 31u; ++arrow_column)
                non_padding_pixels += utility_arrows[arrow_row * 32u + arrow_column] != 0u;
        }
        CHECK(padding_is_zero && non_padding_pixels != 0u,
              "C06's odd-width IMG2 row padding remains static-buffer zero, not image data");
    }
    memset(utility_palette, 0, sizeof(utility_palette));
    CHECK(csb_v1_fmtowns_utility_icon_palette_rgb6(&utility_menu,
                                                    utility_palette) &&
              utility_menu.icon_palette_verified &&
              utility_menu.icon_palette_file_offset ==
                  (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 0x17db0u : 0x17e18u) &&
              utility_palette[0][0] == 0x00u &&
              utility_palette[4][0] == 0x00u &&
              utility_palette[4][1] == 0x36u &&
              utility_palette[4][2] == 0x36u &&
              utility_palette[14][0] == 0x00u &&
              utility_palette[14][1] == 0x00u &&
              utility_palette[14][2] == 0x3fu &&
              utility_palette[15][0] == 0x3fu &&
              utility_palette[15][1] == 0x3fu &&
              utility_palette[15][2] == 0x3fu,
          "C06 retains its source-owned C09_ICON six-bit palette");

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        memset(&utility_render, 0, sizeof(utility_render));
        memset(utility_frame, 0, sizeof(utility_frame));
        CHECK(csb_v1_fmtowns_utility_render_file_picker(
                  &utility_handoff, &utility_font, &utility_file_picker,
                  utility_frame, sizeof(utility_frame), &utility_render) &&
                  utility_render.valid && utility_render.file_picker_first_index == 1u &&
                  utility_render.file_picker_selected_index == 0u &&
                  utility_frame[63u * 320u + 77u] == 2u &&
                  utility_frame[106u * 320u + 165u] == 0u,
              "F31E C06 file-picker raster uses the real font, arrows and list geometry");
        memset(&utility_render, 0, sizeof(utility_render));
        memset(utility_frame, 0, sizeof(utility_frame));
        CHECK(csb_v1_fmtowns_utility_render_initial(
                  &utility_handoff, &utility_menu, &utility_font, &mini_party,
                  &mini_portraits, utility_frame, sizeof(utility_frame),
                  &utility_render) && utility_render.valid &&
                  utility_render.rendered_champion_count == 1u &&
                  utility_render.pixel_fnv1a != 0u &&
                  utility_frame[9u * 320u + 6u] == 9u &&
                  utility_frame[186u * 320u + 2u] == 0u &&
                  utility_frame[188u * 320u + 4u] == 2u &&
                  utility_frame[43u * 320u + 286u] == 0u &&
                  utility_frame[51u * 320u + 286u] == 1u,
              "F31E C06 initial editor frame uses only verified source pixels");
        {
            CSB_V1_PartyState multi_champion_party = mini_party;
            uint8_t selected_portrait[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
            uint8_t following_portrait[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
            size_t sample;

            /* MINI.DAT carries four original external portrait records even
             * when its startup party has one champion.  Exercise CEDT006
             * F7031 with those real records, not an invented portrait: use a
             * pixel at which the selected and following source portraits
             * differ so the enlarged pane cannot accidentally show the
             * last portrait visited by F7033's top-row loop. */
            multi_champion_party.ChampionCount = 4;
            memset(selected_portrait, 0, sizeof(selected_portrait));
            memset(following_portrait, 0, sizeof(following_portrait));
            CHECK(csb_v1_fmtowns_portrait_decode_planar(
                      mini_portraits.source_bytes[2],
                      CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES,
                      selected_portrait, sizeof(selected_portrait)) &&
                      csb_v1_fmtowns_portrait_decode_planar(
                          mini_portraits.source_bytes[3],
                          CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES,
                          following_portrait, sizeof(following_portrait)),
                  "F31 MINI.DAT supplies the selected and following original portraits");
            for (sample = 0u; sample < sizeof(selected_portrait) &&
                              selected_portrait[sample] == following_portrait[sample];
                 ++sample) {
            }
            memset(&utility_render, 0, sizeof(utility_render));
            memset(utility_frame, 0, sizeof(utility_frame));
            CHECK(sample < sizeof(selected_portrait) &&
                      csb_v1_fmtowns_utility_render_editor(
                          &utility_handoff, &utility_menu, &utility_font,
                          &multi_champion_party, &mini_portraits, 2u, 0u,
                          -1, 0u, 0,
                          utility_frame, sizeof(utility_frame),
                          &utility_render) && utility_render.valid &&
                      utility_frame[(60u + (sample / 32u) * 3u) * 320u +
                                    157u + (sample % 32u) * 3u] ==
                          selected_portrait[sample],
                  "F31E C06 zoom pane retains the selected MINI.DAT portrait after top-row drawing");
        }
    }

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        /* SWITCH.C button two exits with status five and AUTOEXEC.BAT opens
         * UTILE.EXP. C06 first owns its source-family chooser; it must not
         * present the MINI.DAT editor until an F0435 game-save medium is read. */
        result = M11_GameView_HandlePointerButton(
            &view, 57, 59, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW && view.csbFmtownsUtilityBound &&
                  !view.csbFmtownsSwitchBound && !view.csbState.startup_entrance_active,
              "F31E Utility opens its verified C06 game-source chooser");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(memcmp(framebuffer, view.csbFmtownsSwitchPixels,
                     sizeof(framebuffer)) != 0,
              "F31E Utility presents its C06 source chooser, not SWITCHTW");
        result = M11_GameView_HandlePointerButton(
            &view, 160, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityGameSaveMediumDialogActive &&
                  !M11_GameView_CsbFmtownsUtilityTextInputActive(&view),
              "F31E C06 CSB choice waits for the separate A: save medium");
        result = M11_GameView_HandlePointerButton(
            &view, 112, 132, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityGameSaveMediumDialogActive &&
                  strcmp(view.lastOutcome, "GAME SAVE DISK REQUIRED") == 0,
              "F31E C06 rejects the CD MINI.DAT when no A: game-save medium exists");
        result = M11_GameView_HandlePointerButton(
            &view, 200, 132, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityGameSourceDialogActive,
              "F31E C06 save-medium cancel returns to the source chooser");
        result = M11_GameView_HandlePointerButton(
            &view, 160, 132, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW && view.csbFmtownsSwitchBound &&
                  !view.csbFmtownsUtilityBound,
              "F31E C06 source-dialog cancel returns through AUTOEXEC to SWITCHTW");
        for (tick = 0u; tick < 80u && view.csbFmtownsSwitchVblanksRemaining != 0u;
             ++tick) {
            (void)M11_GameView_AdvanceIdleTick(&view);
        }
        /* The old direct-editor regression suite needs a separately supplied
         * F0435 save medium.  A retail install with only CD files must stop
         * above; it may not synthesize that missing A: disk. */
        if (user_save_path && user_save_path[0] &&
            test_set_env("FIRESTAFF_QUICKSAVE_PATH", user_save_path)) {
            result = M11_GameView_HandlePointerButton(
                &view, 57, 59, DM1_V1_MOUSE_MASK_LEFT_PC34);
            result = M11_GameView_HandlePointerButton(
                &view, 160, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
            result = M11_GameView_HandlePointerButton(
                &view, 112, 132, DM1_V1_MOUSE_MASK_LEFT_PC34);
            CHECK(result == M11_GAME_INPUT_REDRAW &&
                      !view.csbFmtownsUtilityGameSaveMediumDialogActive,
                  "F31E C06 opens its editor from the supplied native A: medium");
        /* CEDTDATA.C entries 13/14 and CEDT006.C F7027/F7041: the editor
         * owns native name/title text, including F31's 7/19 byte limits and
         * its upper-case punctuation filter. */
        result = M11_GameView_HandlePointerButton(
            &view, 16, 90, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  M11_GameView_CsbFmtownsUtilityTextInputActive(&view),
              "F31E C06 name field takes the native text cursor");
        result = M11_GameView_HandleCsbFmtownsUtilityTextKey(
            &view, M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_CLEAR);
        result = M11_GameView_ConsumeCsbFmtownsUtilityTextInput(
            &view, "ab-");
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  strcmp(view.csbFmtownsUtilityParty.Champions[0].Name, "AB") == 0,
              "F31E C06 name text uppercases letters and rejects non-source punctuation");
        (void)M11_GameView_HandleCsbFmtownsUtilityTextKey(
            &view, M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_LEFT);
        (void)M11_GameView_ConsumeCsbFmtownsUtilityTextInput(&view, "c");
        (void)M11_GameView_HandleCsbFmtownsUtilityTextKey(
            &view, M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_BACKSPACE);
        CHECK(strcmp(view.csbFmtownsUtilityParty.Champions[0].Name, "AB") == 0,
              "F31E C06 name editing inserts and backspaces at its source cursor");
        for (tick = 0u; tick < 29u; ++tick) {
            CHECK(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_IGNORED,
                  "F31E C06 cursor waits the recovered thirty VBlanks");
        }
        result = M11_GameView_AdvanceIdleTick(&view);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  !view.csbFmtownsUtilityTextCursorVisible &&
                  view.csbFmtownsUtilityTextCursorVblanksRemaining == 30u,
              "F31E C06 cursor toggles after exactly thirty VBlanks");
        result = M11_GameView_HandlePointerButton(
            &view, 16, 104, DM1_V1_MOUSE_MASK_LEFT_PC34);
        (void)M11_GameView_HandleCsbFmtownsUtilityTextKey(
            &view, M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_CLEAR);
        result = M11_GameView_ConsumeCsbFmtownsUtilityTextInput(
            &view, "abcdefghijklmnopqrst");
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  strcmp(view.csbFmtownsUtilityParty.Champions[0].Title,
                         "ABCDEFGHIJKLMNOPQRS") == 0,
              "F31E C06 title retains its recovered nineteen-character limit");
        (void)M11_GameView_HandleCsbFmtownsUtilityTextKey(
            &view, M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_PAGE_UP);
        CHECK(view.csbFmtownsUtilityEditField == 0 &&
                  view.csbFmtownsUtilityEditCharacterIndex == 0u,
              "F31E C06 Page Up switches from title to the start of name");
        (void)M11_GameView_HandleCsbFmtownsUtilityTextKey(
            &view, M11_CSB_FMTOWNS_UTILITY_TEXT_KEY_PAGE_DOWN);
        CHECK(view.csbFmtownsUtilityEditField == 1 &&
                  view.csbFmtownsUtilityEditCharacterIndex == 0u,
              "F31E C06 Page Down switches from name to the start of title");
        /* F7001 presents its own GAME / PORTRAIT / CANCEL dialog.  Its
         * selected-portrait branch resolves the dynamic 2:\\#CHAMP_NAME#
         * medium separately from the scanned read-only CD catalogue. The
         * M747 mapping itself is retained from the authentic F31 image. */
        result = M11_GameView_HandlePointerButton(
            &view, 150, 190, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityBound &&
                  view.csbFmtownsUtilitySaveMappingReceipt.valid &&
                  view.csbFmtownsUtilitySaveDialogActive &&
                  !view.csbFmtownsUtilityFilePickerActive,
              "F31E C06 SAVE CHAMPIONS opens its source GAME/PORTRAIT/CANCEL dialog");
        result = M11_GameView_HandlePointerButton(
            &view, 150, 132, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  !view.csbFmtownsUtilitySaveDialogActive,
              "F31E C06 save-dialog cancel restores the editor");
#ifndef _WIN32
        /* C06 F7001 choice GAME must construct its own F7052 transaction
         * from MINI.DAT, rather than borrowing this test's active C03
         * runtime.  Keep the new M746 medium in a private directory and
         * reopen it through F0435 before continuing with the editor tests. */
        {
            char utility_dir[] = "/tmp/firestaff-csb-c06-game-XXXXXX";
            char utility_save_path[1024];
            char utility_backup_path[1024];
            CSB_V1_FmtownsUserSaveReceipt utility_save;
            CSB_V1_FmtownsStartupState utility_state;
            uint8_t *utility_bytes = NULL;
            uint8_t saved_portraits[CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT]
                                   [CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES];
            size_t utility_size = 0u;
            int created = 0;
            if (mkdtemp(utility_dir) != NULL &&
                snprintf(utility_save_path, sizeof(utility_save_path),
                         "%s/CSBGAME.DAT", utility_dir) >= 0) {
                CHECK(test_set_env("FIRESTAFF_QUICKSAVE_PATH", utility_save_path),
                      "F31E C06 GAME selects an isolated M746 save medium");
                result = M11_GameView_HandlePointerButton(
                    &view, 150, 190, DM1_V1_MOUSE_MASK_LEFT_PC34);
                result = M11_GameView_HandlePointerButton(
                    &view, 112, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
                created = result == M11_GAME_INPUT_REDRAW &&
                    strcmp(view.lastOutcome, "GAME SAVED") == 0;
                CHECK(created,
                      "F31E C06 GAME runs F7052 against its private MINI.DAT receipt");
                memset(&utility_save, 0, sizeof(utility_save));
                CHECK(created && csb_v1_fmtowns_game_user_save_open(
                          (const CSB_V1_BootProfile *)view.csbBootProfile,
                          &direct_handoff, utility_save_path, &utility_save) &&
                          utility_save.valid,
                      "F31E C06 GAME output passes the native F0435 reader");
                memset(&utility_state, 0, sizeof(utility_state));
                CHECK(created && csb_v1_fmtowns_game_load_user_save_state(
                          &utility_save, &utility_state) && utility_state.valid &&
                          strcmp(utility_state.party.Champions[0].Name, "AB") == 0 &&
                          strcmp(utility_state.party.Champions[0].Title,
                                 "ABCDEFGHIJKLMNOPQRS") == 0,
                      "F31E C06 GAME preserves editor-owned name and title bytes");
                utility_bytes = load_file(utility_save_path, &utility_size);
                CHECK(created && utility_bytes &&
                          utility_save.portraits_offset <= utility_size &&
                          sizeof(view.csbFmtownsUtilityPortraitReceipt.source_bytes) <=
                              utility_size - utility_save.portraits_offset &&
                          memcmp(utility_bytes + utility_save.portraits_offset,
                                 view.csbFmtownsUtilityPortraitReceipt.source_bytes,
                                 sizeof(view.csbFmtownsUtilityPortraitReceipt.source_bytes)) == 0,
                      "F31E C06 GAME writes the editor's raw F31 portrait receipt");
                free(utility_bytes);
                csb_v1_fmtowns_game_startup_state_free(&utility_state);
                memcpy(saved_portraits,
                       view.csbFmtownsUtilityPortraitReceipt.source_bytes,
                       sizeof(saved_portraits));
                snprintf(view.csbFmtownsUtilityParty.Champions[0].Name,
                         sizeof(view.csbFmtownsUtilityParty.Champions[0].Name),
                         "%s", "SAVED2");
                result = M11_GameView_HandlePointerButton(
                    &view, 150, 190, DM1_V1_MOUSE_MASK_LEFT_PC34);
                result = M11_GameView_HandlePointerButton(
                    &view, 112, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
                CHECK(created && result == M11_GAME_INPUT_REDRAW &&
                          strcmp(view.lastOutcome, "GAME SAVED") == 0,
                      "F31E C06 F7052 updates an existing native M746 slot");
                memset(&utility_save, 0, sizeof(utility_save));
                memset(&utility_state, 0, sizeof(utility_state));
                CHECK(created && csb_v1_fmtowns_game_user_save_open(
                          (const CSB_V1_BootProfile *)view.csbBootProfile,
                          &direct_handoff, utility_save_path, &utility_save) &&
                          csb_v1_fmtowns_game_load_user_save_state(
                              &utility_save, &utility_state) &&
                          strcmp(utility_state.party.Champions[0].Name,
                                 "SAVED2") == 0,
                      "F31E C06 existing-slot save remains F0435-readable");
                csb_v1_fmtowns_game_startup_state_free(&utility_state);
                snprintf(view.csbFmtownsUtilityParty.Champions[0].Name,
                         sizeof(view.csbFmtownsUtilityParty.Champions[0].Name),
                         "%s", "CHANGED");
                result = M11_GameView_HandlePointerButton(
                    &view, 50, 190, DM1_V1_MOUSE_MASK_LEFT_PC34);
                result = M11_GameView_HandlePointerButton(
                    &view, 112, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
                CHECK(created && result == M11_GAME_INPUT_REDRAW &&
                          strcmp(view.lastOutcome, "GAME LOADED") == 0 &&
                          strcmp(view.csbFmtownsUtilityParty.Champions[0].Name,
                                 "SAVED2") == 0,
                      "F31E C06 F7004/F7051 restores the saved party into the editor");
                CHECK(created &&
                          memcmp(view.csbFmtownsUtilityPortraitReceipt.source_bytes,
                                 saved_portraits,
                                 sizeof(view.csbFmtownsUtilityPortraitReceipt.source_bytes)) == 0,
                      "F31E C06 F7051 retains the selected slot's raw portrait receipt");
                CHECK(created && write_damaged_file(utility_save_path),
                      "F31E C06 backup recovery test can damage only its private slot");
                snprintf(view.csbFmtownsUtilityParty.Champions[0].Name,
                         sizeof(view.csbFmtownsUtilityParty.Champions[0].Name),
                         "%s", "DAMAGED");
                result = M11_GameView_HandlePointerButton(
                    &view, 50, 190, DM1_V1_MOUSE_MASK_LEFT_PC34);
                result = M11_GameView_HandlePointerButton(
                    &view, 112, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
                CHECK(created && result == M11_GAME_INPUT_REDRAW &&
                          strcmp(view.lastOutcome, "GAME LOADED") == 0 &&
                          strcmp(view.csbFmtownsUtilityParty.Champions[0].Name,
                                 "AB") == 0,
                      "F31E C06 F7051 restores the native CSBGAME.BAK slot");
                memset(&utility_save, 0, sizeof(utility_save));
                CHECK(created && csb_v1_fmtowns_game_user_save_open(
                          (const CSB_V1_BootProfile *)view.csbBootProfile,
                          &direct_handoff, utility_save_path, &utility_save) &&
                          utility_save.valid && !utility_save.recovered_from_backup,
                      "F31E C06 backup recovery republishes a canonical F0435 slot");
                remove(utility_save_path);
                if (snprintf(utility_backup_path, sizeof(utility_backup_path),
                             "%s/CSBGAME.BAK", utility_dir) >= 0)
                    remove(utility_backup_path);
                rmdir(utility_dir);
            } else {
                CHECK(0, "F31E C06 GAME test can allocate an isolated temporary directory");
            }
            (void)test_set_env("FIRESTAFF_QUICKSAVE_PATH", NULL);
        }
#endif
        result = M11_GameView_HandlePointerButton(
            &view, 50, 190, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityLoadDialogActive &&
                  !view.csbFmtownsUtilityFilePickerActive,
              "F31E C06 LOAD CHAMPIONS opens its source GAME/PORTRAIT/CANCEL dialog");
        result = M11_GameView_HandlePointerButton(
            &view, 200, 112, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  !view.csbFmtownsUtilityLoadDialogActive &&
                  view.csbFmtownsUtilityFilePickerActive,
              "F31E C06 F7004 PORTRAIT enters the source-owned file picker");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(framebuffer[63u * 320u + 77u] == 13u,
              "F31E C06 file picker presents its real list and arrow raster");
        result = M11_GameView_HandlePointerButton(
            &view, 80, 63, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  !view.csbFmtownsUtilityFilePickerActive &&
                  strcmp(view.csbFmtownsUtilityParty.Champions[0].Name,
                         view.csbFmtownsUtilityPortraitCatalog.entries[0].portrait.name) == 0,
              "F31E C06 F7002 imports the selected authenticated CMP into the party");
        result = M11_GameView_HandlePointerButton(
            &view, 290, 67, DM1_V1_MOUSE_MASK_LEFT_PC34);
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilitySelectedColor == 3u &&
                  framebuffer[68u * 320u + 287u] == 15u,
              "F31E C06 palette selection redraws the original selected swatch");
        /* CEDT006.C F7046 is the source C06 right-button path. Choose an
         * actual MINI.DAT pixel that is not colour 3, rather than arranging a
         * fixture pattern, then prove the active native portrait changes. */
        memcpy(utility_portrait_before,
               view.csbFmtownsUtilityPortraitReceipt.source_bytes[0],
               sizeof(utility_portrait_before));
        CHECK(csb_v1_fmtowns_portrait_decode_planar(
                  utility_portrait_before, sizeof(utility_portrait_before),
                  portrait_pixels, sizeof(portrait_pixels)),
              "F31E C06 flood-fill source portrait decodes from actual MINI.DAT");
        for (tick = 0u; tick < sizeof(portrait_pixels); ++tick) {
            if (portrait_pixels[tick] != 3u) {
                utility_fill_x = (int)(tick % 32u);
                utility_fill_y = (int)(tick / 32u);
                break;
            }
        }
        result = M11_GameView_HandlePointerButton(
            &view, 157 + utility_fill_x * 3, 60 + utility_fill_y * 3,
            DM1_V1_MOUSE_MASK_RIGHT_PC34);
        CHECK(utility_fill_x >= 0 && utility_fill_y >= 0 &&
                  result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityPortraitModified[0] != 0u &&
                  memcmp(view.csbFmtownsUtilityPortraitReceipt.source_bytes[0],
                         utility_portrait_before,
                         sizeof(utility_portrait_before)) != 0,
              "F31E C06 right-click flood fill updates a connected real MINI.DAT area");
        result = M11_GameView_HandlePointerButton(
            &view, 230, 160, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityPortraitModified[0] == 0u &&
                  memcmp(view.csbFmtownsUtilityPortraitReceipt.source_bytes[0],
                         utility_portrait_before,
                         sizeof(utility_portrait_before)) == 0,
              "F31E C06 Undo restores the source-format flood-fill backup");
        result = M11_GameView_HandlePointerButton(
            &view, 157, 60, DM1_V1_MOUSE_MASK_LEFT_PC34);
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityPortraitModified[0] != 0u &&
                  framebuffer[60u * 320u + 157u] == 3u,
              "F31E C06 draw maps the selected source colour into MINI portrait bytes");
        result = M11_GameView_HandlePointerButton(
            &view, 230, 160, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityPortraitModified[0] == 0u &&
                  memcmp(view.csbFmtownsUtilityPortraitReceipt.source_bytes[0],
                         view.csbFmtownsUtilityOriginalPortraits[0],
                         CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES) == 0,
              "F31E C06 Undo swaps the original source-format portrait backup");
        result = M11_GameView_HandlePointerButton(
            &view, 230, 160, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityPortraitModified[0] != 0u,
              "F31E C06 Undo keeps its source-style toggle backup");
        result = M11_GameView_HandlePointerButton(
            &view, 160, 160, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityPortraitModified[0] == 0u &&
                  memcmp(view.csbFmtownsUtilityPortraitReceipt.source_bytes[0],
                         view.csbFmtownsUtilityOriginalPortraits[0],
                         CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES) == 0,
              "F31E C06 Revert restores the original MINI portrait payload");
        result = M11_GameView_HandlePointerButton(
            &view, 300, 8, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW && view.csbFmtownsSwitchBound &&
                  !view.csbFmtownsUtilityBound,
              "F31E C06 Quit returns through AUTOEXEC to SWITCHTW");
        for (tick = 0u; tick < 80u && view.csbFmtownsSwitchVblanksRemaining != 0u;
             ++tick) {
            (void)M11_GameView_AdvanceIdleTick(&view);
        }
        CHECK(view.csbFmtownsSwitchVblanksRemaining == 0u,
              "F31E C06 return observes SWITCHTW's source VBlank wait");
        }
        (void)test_set_env("FIRESTAFF_QUICKSAVE_PATH", NULL);
    } else if (getenv("FIRESTAFF_FMTOWNS_FONT_ROM")) {
        /* The Japanese page has no host text fallback.  It is available only
         * when the caller supplies the real FMT_FNT.ROM glyph device. */
        result = M11_GameView_HandlePointerButton(
            &view, 57, 59, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW && view.csbFmtownsUtilityBound &&
                  view.csbFmtownsUtilityJapaneseActive &&
                  M11_GameView_GetCsbFmtownsUtilityJapaneseFrame(
                      &view, &japanese_utility_frame, &japanese_utility_width,
                      &japanese_utility_height) && japanese_utility_frame &&
                  japanese_utility_width == 640 && japanese_utility_height == 400,
              "F31J Utility opens its ROM-bound native 640x400 C06 chooser");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        for (tick = 0u; tick < sizeof(framebuffer); ++tick) {
            const size_t y = tick / 320u;
            const size_t x = tick % 320u;
            if (framebuffer[tick] != japanese_utility_frame[
                    (y * 2u) * (size_t)japanese_utility_width + x * 2u]) {
                fprintf(stderr,
                        "NOTE: F31J C06 presentation does not retain the "
                        "source 640x400 raster\n");
                break;
            }
        }
        CHECK(tick == sizeof(framebuffer),
              "F31J C06 presentation maps the ROM-bound 640x400 chooser to M11");
        result = M11_GameView_HandlePointerButton(
            &view, 320, 225, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilityGameSourceDialogActive &&
                  strcmp(view.lastOutcome, "CSB SAVE MEDIA REQUIRED") == 0,
              "F31J C06 rejects a CSB choice until an authentic saved-game medium is admitted");
        result = M11_GameView_HandlePointerButton(
            &view, 320, 265, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW && view.csbFmtownsSwitchBound &&
                  !view.csbFmtownsUtilityBound &&
                  view.csbFmtownsSwitchLanguage == CSB_FMTOWNS_SWITCH_JAPANESE,
              "F31J C06 cancel returns through the Japanese AUTOEXEC/SWITCHTW route");
        for (tick = 0u; tick < 80u && view.csbFmtownsSwitchVblanksRemaining != 0u;
             ++tick) {
            (void)M11_GameView_AdvanceIdleTick(&view);
        }
        CHECK(view.csbFmtownsSwitchVblanksRemaining == 0u,
              "F31J C06 return observes SWITCHTW's source VBlank wait");
    }

    /* ReDMCSB SWITCH.C F2279 registers G4171 at (47,105), 62x39. */
    result = M11_GameView_HandlePointerButton(&view, 52, 110,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    CHECK(result == M11_GAME_INPUT_REDRAW,
          "SWITCHTW Game rectangle is handled as a modal source action");
    CHECK(view.csbFmtownsGameHandoffReceipt.valid &&
              view.csbFmtownsGameHandoffReceipt.executable_verified &&
              strcmp(view.csbFmtownsGameHandoffReceipt.executable_name,
                     expected_program) == 0 &&
              view.csbStartupRuntimeAssetSession &&
              view.csbState.startup_entrance_active &&
              !view.csbState.startup_title_active,
          "Game click opens only the verified language-owned entrance owner");
    {
        const CSB_V1_BootProfile *text_profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        CHECK(text_profile && text_profile->runtime.object_name_table_valid &&
                  text_profile->runtime.action_name_table_valid &&
                  text_profile->runtime.object_names[0][0] != '\0' &&
                  strcmp(csb_v1_runtime_action_name_c699(
                             &text_profile->runtime, 0u), "N") == 0 &&
                  strcmp(csb_v1_runtime_action_name_c699(
                             &text_profile->runtime, 3u), "X") == 0 &&
                  (language != CSB_FMTOWNS_SWITCH_JAPANESE ||
                   (unsigned char)csb_v1_runtime_action_name_c699(
                       &text_profile->runtime, 1u)[0] >= 0x80u) &&
                  (language == CSB_FMTOWNS_SWITCH_JAPANESE ||
                   strcmp(csb_v1_runtime_action_name_c699(
                              &text_profile->runtime, 1u), "BLOCK") == 0),
              "F31 live runtime owns edition-native M564 and DYNA_BUTTONS text");
    }
    CHECK(view.csbFmtownsGameHandoffReceipt.entrance_palette_verified &&
              view.csbFmtownsGameHandoffReceipt.dungeon_palettes_verified &&
              view.csbFmtownsEntrancePaletteValid &&
              view.csbFmtownsDungeonPalettesValid &&
              view.csbFmtownsEntrancePaletteRgb6[1][0] == 0x1bu &&
              view.csbFmtownsEntrancePaletteRgb6[1][1] == 0x1bu &&
              view.csbFmtownsEntrancePaletteRgb6[1][2] == 0x1bu &&
              view.csbFmtownsEntrancePaletteRgb6[8][2] == 0x13u &&
              view.csbFmtownsEntrancePaletteRgb6[8][2] !=
                  view.csbFmtownsSwitchReceipt.palette[8].blue6,
          "F31 Game replaces SWITCHTW C26 with source C28_ENTRANCE_CSB");
    live_frame_nonblack = 0;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    CHECK(memcmp(framebuffer,
                 ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                  view.csbStartupRuntimeAssetSession)->surfaces.surfaces[
                     CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                     .pixels,
                 sizeof(framebuffer)) == 0,
          "F31 Game handoff draws the authenticated C004 entrance raster");
    CHECK(M11_GameView_GetPresentationSpecialPalette(&view) == -1,
          "F31 C004 rejects the PC3.4 special palette and retains its native DAC state");
    /* CHTWE/CHTWJ reaches the native C004 Entrance page without the PC
     * TITLE.C session.  Its C407 box is nevertheless the original C200
     * primary-mouse command (COMMAND.C:342; layout-696 (244,45) 55x14).
     * Exercise the real F31E/F31J Game handoff through that visible box,
     * rather than using a host-only keyboard shortcut. */
    result = M11_GameView_HandlePointerButton(&view, 250, 50,
                                              DM1_V1_MOUSE_MASK_LEFT_PC34);
    CHECK(result == M11_GAME_INPUT_REDRAW &&
              view.csbState.startup_entrance_opening_active,
          "F31 Game C407 pointer command enters the source-owned Prison transition");
    for (tick = 0u; tick < 240u && view.csbState.startup_entrance_active;
         ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        if (view.csbState.startup_entrance_opening_active &&
            view.csbState.startup_entrance_opening_step > 0 &&
            memcmp(framebuffer,
                   ((const CSB_V1_StartupRuntimeAssetSession_PC34 *)
                    view.csbStartupRuntimeAssetSession)->surfaces.surfaces[
                       CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                       .pixels,
                   sizeof(framebuffer)) != 0) {
            door_frame_seen = 1;
        }
    }
    CHECK(door_frame_seen,
          "F31 Prison transition draws a source-owned C002/C003 door frame");
    CHECK(!view.csbState.startup_entrance_active && view.csbState.level_loaded,
          "F31 Prison door handoff reaches the live CSB runtime");
    {
        const CSB_V1_RuntimeProfile *runtime =
            &((const CSB_V1_BootProfile *)view.csbBootProfile)->runtime;
        CHECK(runtime->current_level == 4 && runtime->party_x == 22 &&
                  runtime->party_y == 18 && runtime->party_dir == 2 &&
                  runtime->game_time ==
                      (language == CSB_FMTOWNS_SWITCH_ENGLISH ? 82u : 88u) &&
                  runtime->timeline_queue.eventCount == 23 &&
                  runtime->timeline_queue.firstUnusedIndex == 23 &&
                  runtime->timeline_queue.maxEvents == 436 &&
                  runtime->active_group_state_count == 8u,
              "F31 Prison handoff retains the original MINI.DAT runtime graph");
    }
    memset(&terminal, 0, sizeof(terminal));
    CHECK(csb_v1_startup_session_terminal_receipt_pc34(
              (CSB_V1_StartupRuntimeAssetSession_PC34 *)
                  view.csbStartupRuntimeAssetSession, &terminal) &&
              terminal.valid && terminal.c017_ready && terminal.c040_ready,
          "F31 title, Switch and Game handoff reaches the real C017/C040 terminal session");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    {
        int level;
        int matched_level = -1;
        CHECK(M11_Render_CopyIndexedPaletteRgb6(presented_palette),
              "F31 live dungeon publishes an indexed source palette");
        for (level = 0; level < 6; ++level) {
            int color;
            for (color = 0; color < 16; ++color) {
                if (memcmp(presented_palette[color],
                           view.csbFmtownsDungeonPaletteRgb6[level][color],
                           3u) != 0) break;
            }
            if (color == 16) {
                matched_level = level;
                break;
            }
        }
        CHECK(matched_level >= 0 &&
                  memcmp(presented_palette[8],
                         view.csbFmtownsEntrancePaletteRgb6[8], 3u) != 0,
              "F31 post-Entrance frame selects LIGHT0--LIGHT5, not C28");
    }
    for (tick = 0u; tick < sizeof(framebuffer); ++tick) {
        if (framebuffer[tick] != 0u) {
            live_frame_nonblack = 1;
            break;
        }
    }
    CHECK(live_frame_nonblack,
          "F31 C017 HUD and F0128 viewport draw a real live frame after Prison");
    live_viewport_hash = f31_viewport_fnv1a(framebuffer, language);
    CHECK(live_viewport_hash == view.csbState.runtime_viewport_pixel_hash,
          "F31 final source-positioned aperture matches the complete runtime frame receipt");
    CHECK(live_viewport_hash != 0u &&
              view.csbState.runtime_viewport_source_session_ready &&
              view.csbState.runtime_viewport_pixel_hash != 0u,
          "F31 first F0128 aperture is source-receipted before live input");
    {
        const CSB_V1_BootProfile *live_profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        const M11_AssetSlot *floor;
        uint8_t *graphics = NULL;
        uint8_t *expected = NULL;
        size_t graphics_size = 0u;
        CSB_V1_FmtownsItemDecodeReceipt expected_receipt;

        /* F31 has a PC-like 0x8001 header but IMAGE2.C F0689's IMG2 command
         * stream. Prove the live F0128 material uses that actual decoder,
         * rather than accepting a PC IMG3 interpretation of the same table. */
        floor = M11_AssetLoader_Load(&view.assetLoader, 78u);
        CHECK(live_profile && view.assetLoader.csbFmtowns && floor &&
                  floor->loaded && floor->pixels,
              "F31 live viewport binds its native IMG2 graphics owner");
        if (live_profile && floor) {
            graphics = load_profile_graphics(live_profile, &graphics_size);
            expected = (uint8_t *)malloc(640u * 400u);
            memset(&expected_receipt, 0, sizeof(expected_receipt));
            CHECK(graphics && expected &&
                      csb_v1_fmtowns_graphics_decode_item(
                          graphics, graphics_size, 78u, expected,
                          640u * 400u, &expected_receipt) &&
                      expected_receipt.valid && expected_receipt.is_image &&
                      floor->width == expected_receipt.width &&
                      floor->height == expected_receipt.height &&
                      memcmp(floor->pixels, expected,
                             expected_receipt.pixel_count) == 0,
                  "F31 floor material is byte-identical to genuine GRAPHICS.DAT IMG2");
        }
        free(expected);
        free(graphics);
    }
    {
        static const unsigned int f0115_native_families[] = {
            454u, /* M613 first projectile */
            486u, /* M614 first explosion */
            498u, /* M612 first object */
            584u  /* M618 first creature */
        };
        const CSB_V1_BootProfile *live_profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        uint8_t *graphics = NULL;
        uint8_t *expected = NULL;
        size_t graphics_size = 0u;
        size_t family;

        if (live_profile) {
            graphics = load_profile_graphics(live_profile, &graphics_size);
            expected = (uint8_t *)malloc(640u * 400u);
        }
        /* ReDMCSB DEFS.H MEDIA720_F31E_F31J binds these F0115 first-native
         * records; DUNVIEW.C uses their relative aspect offsets for object,
         * group, projectile, and explosion passes.  This proves the F31 IMG2
         * path supplies their genuine pixels before the shared F0115 drawers
         * are admitted. */
        for (family = 0u; graphics && expected &&
             family < sizeof(f0115_native_families) /
                 sizeof(f0115_native_families[0]); ++family) {
            const unsigned int graphic = f0115_native_families[family];
            const M11_AssetSlot *asset =
                M11_AssetLoader_Load(&view.assetLoader, graphic);
            CSB_V1_FmtownsItemDecodeReceipt receipt;

            memset(&receipt, 0, sizeof(receipt));
            CHECK(asset && asset->loaded && asset->pixels &&
                      csb_v1_fmtowns_graphics_decode_item(
                          graphics, graphics_size, graphic, expected,
                          640u * 400u, &receipt) && receipt.valid &&
                      receipt.is_image && asset->width == receipt.width &&
                      asset->height == receipt.height &&
                      memcmp(asset->pixels, expected,
                             receipt.pixel_count) == 0,
                  "F31 F0115 family is byte-identical to genuine GRAPHICS.DAT IMG2");
        }
        CHECK(graphics && expected,
              "F31 real GRAPHICS.DAT opens for F0115-family verification");
        free(expected);
        free(graphics);
    }
    {
        const CSB_V1_BootProfile *live_profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        uint8_t *graphics = NULL;
        uint8_t *expected = NULL;
        size_t graphics_size = 0u;
        unsigned int graphic;

        /* F31E/F31J MEDIA720 M649..M634 is the native F0111/F0110 door
         * family: destroyed/thieves-eye masks, twelve ornaments and the
         * button.  It must remain IMG2-owned rather than falling back to a
         * PC graphic table when a live F0128 frame encounters a door. */
        if (live_profile) {
            graphics = load_profile_graphics(live_profile, &graphics_size);
            expected = (uint8_t *)malloc(640u * 400u);
        }
        for (graphic = 439u; graphics && expected && graphic <= 453u;
             ++graphic) {
            const M11_AssetSlot *asset =
                M11_AssetLoader_Load(&view.assetLoader, graphic);
            CSB_V1_FmtownsItemDecodeReceipt receipt;

            memset(&receipt, 0, sizeof(receipt));
            CHECK(asset && asset->loaded && asset->pixels &&
                      csb_v1_fmtowns_graphics_decode_item(
                          graphics, graphics_size, graphic, expected,
                          640u * 400u, &receipt) && receipt.valid &&
                      receipt.is_image && asset->width == receipt.width &&
                      asset->height == receipt.height &&
                      memcmp(asset->pixels, expected,
                             receipt.pixel_count) == 0,
                  "F31 door material is byte-identical to genuine GRAPHICS.DAT IMG2");
        }
        CHECK(graphics && expected,
              "F31 real GRAPHICS.DAT opens for native door-material verification");
        free(expected);
        free(graphics);
    }
    {
        const CSB_V1_BootProfile *live_profile =
            (const CSB_V1_BootProfile *)view.csbBootProfile;
        int prior_direction = live_profile ? live_profile->runtime.party_dir : -1;

        /* CHTWE/CHTWJ has now returned through STARTUP1.C into the ordinary
         * live command loop.  Require that its real MINI.DAT party drives
         * both the native runtime and the rendered M11 mirror; the F31
         * title/switch programs must not leave an inert PC34-style page
         * behind. ReDMCSB COMMAND.C F0358/F0359 dispatches C002..C006 after
         * the F0435/ENTRANCE.C handoff. Exercise C004 first: a prior turn
         * would otherwise hide a stale C040/C017 first-input gate. */
        CHECK(live_profile &&
                  M11_GameView_HandleInput(&view,
                                           M12_MENU_INPUT_STRAFE_RIGHT) ==
                      M11_GAME_INPUT_REDRAW &&
                  live_profile->runtime.last_input_dispatch.command ==
                      DM1_V1_COMMAND_MOVE_RIGHT,
              "F31 first post-Prison C004 side-step reaches the real MINI.DAT command queue");
        CHECK(live_profile &&
                  M11_GameView_HandleInput(&view,
                                           M12_MENU_INPUT_TURN_RIGHT) ==
                      M11_GAME_INPUT_REDRAW &&
                  live_profile->runtime.party_dir ==
                      ((prior_direction + 1) & 3) &&
                  view.csbState.party_dir == live_profile->runtime.party_dir,
              "F31 live HUD routes C002 into the original runtime party state");
        /* The initial Prison record has active GROUP records on both sides
         * of the party. C004 first attempts the source-owned side step into
         * the west group; C002 then faces that same real group. This proves
         * F0115 emits a native creature blit from the admitted
         * MINI.DAT/GRAPHICS.DAT pair, without adding a test object or
         * manufacturing a viewport state. */
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(f31_has_native_runtime_group_sprite(&view),
              "F31 F0115 draws the adjacent retail Prison group with a native sprite");
        CHECK(f31_visible_group_sprite_matches_img2(&view),
              "F31 Prison group sprite is byte-identical to its selected IMG2 record");
        CHECK(live_profile &&
                  M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                      M11_GAME_INPUT_REDRAW &&
                  live_profile->runtime.last_input_dispatch.dequeued &&
                  live_profile->runtime.last_input_dispatch.dispatchedMove,
              "F31 live viewport routes C003 through the dungeon command queue");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(live_viewport_hash != 0u &&
                  f31_viewport_fnv1a(framebuffer, language) != live_viewport_hash &&
                  view.csbState.runtime_viewport_source_session_ready &&
                  view.csbState.runtime_viewport_pixel_hash != 0u,
              "F31 F0128 redraws its source-owned aperture after live movement input");
        /* Retail F31 C696 C082 is x234..318, EN y86..96 / JP y94..113.
         * C098 is a separate Pass zone. The old seven-pixel text-line
         * approximation incorrectly dispatched the name band. This
         * uses the admitted F31 MINI.DAT champion/action list; no action
         * fixture or synthetic menu state is introduced. */
        memset(fmtowns_actions, 0xff, sizeof(fmtowns_actions));
        CHECK(M11_GameView_SetActingChampion(&view, 0),
              "F31 actor opens original action menu");
        {
            static const int modes[] = {M12_PRESENTATION_V1_ORIGINAL,
                M12_PRESENTATION_V20_FILTERED, M12_PRESENTATION_V21_UPSCALED};
            const int savedMode = view.presentationMode;
            const int jp = language == CSB_FMTOWNS_SWITCH_JAPANESE;
            const int width = jp ? 96 : 87, height = jp ? 72 : 45;
            const int menuY = jp ? 85 : 77;
            int cropHeight = height;
            size_t graphics_size = 0;
            uint8_t *graphics = load_profile_graphics(live_profile, &graphics_size);
            uint8_t source[96 * 72];
            CSB_V1_FmtownsItemDecodeReceipt receipt;
            memset(&receipt, 0, sizeof(receipt));
            CHECK(M11_GameView_GetActingActionIndices(&view, fmtowns_actions),
                  "F31 original action rows available for source crop oracle");
            if (fmtowns_actions[2] == 0xffu) cropHeight = jp ? 51 : 33;
            if (fmtowns_actions[1] == 0xffu) cropHeight = jp ? 30 : 21;
            CHECK(graphics && csb_v1_fmtowns_graphics_decode_item(
                      graphics, graphics_size, 10, source, sizeof(source), &receipt) &&
                      receipt.valid && receipt.width == width && receipt.height == height,
                  "F31 oracle decodes original language-specific C010");
            if (receipt.valid && receipt.width == width && receipt.height == height) {
                for (int mode = 0; mode < 3; ++mode) {
                    int mismatch = 0;
                    view.presentationMode = modes[mode];
                    M11_GameView_Draw(&view, framebuffer, 320, 200);
                    CHECK(f31_viewport_fnv1a(framebuffer, language) ==
                              view.csbState.runtime_viewport_pixel_hash,
                          "F31 full viewport remains source-positioned beside the active menu in every mode");
                    /* Border columns exclude name/action glyphs. Include
                     * x233/234, previously overwritten by viewport restore.
                     * Check the final public frame, not an isolated blit. */
                    for (int y = 0; y < height; ++y)
                        for (int x = 0; x < 87; ++x) {
                            if (x >= 2 && x < 81) continue;
                            uint8_t expected = y < cropHeight ?
                                source[y * width + x + (jp ? 9 : 0)] : 0;
                            if (framebuffer[(menuY + y) * 320 + 233 + x] != expected)
                                ++mismatch;
                        }
                    CHECK(mismatch == 0,
                          "F31 final active-menu border matches original C010 in all modes");
                }
            }
            free(graphics);
            view.presentationMode = savedMode;
        }
        (void)M11_GameView_HandlePointerButton(
            &view, 240, language == CSB_FMTOWNS_SWITCH_JAPANESE ? 87 : 79,
            DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(view.actingChampionOrdinal == 1u,
              "F31 name band does not dispatch an action");
        {
            const int passX = language == CSB_FMTOWNS_SWITCH_JAPANESE ? 295 : 285;
            const int passY = language == CSB_FMTOWNS_SWITCH_JAPANESE ? 85 : 77;
            const int leader = view.world.party.activeChampionIndex;
            const int stamina = view.world.party.champions[0].stamina.current;
            for (int corner = 0; corner < 2; ++corner) {
                CHECK(M11_GameView_SetActingChampion(&view, 0),
                      "F31 original actor reopens menu for Pass");
                (void)M11_GameView_HandlePointerButton(
                    &view, corner ? 319 : passX, passY + (corner ? 6 : 0),
                    DM1_V1_MOUSE_MASK_LEFT_PC34);
                CHECK(view.actingChampionOrdinal == 0u &&
                          view.world.party.activeChampionIndex == leader &&
                          view.world.party.champions[0].stamina.current == stamina,
                      "F31 C098 inclusive Pass corners close without an action");
            }
        }
        CHECK(M11_GameView_SetActingChampion(&view, 0) &&
                  M11_GameView_GetActingActionIndices(&view, fmtowns_actions) &&
                  fmtowns_actions[0] != 0xffu &&
                  M11_GameView_HandlePointerButton(
                      &view, 240, language == CSB_FMTOWNS_SWITCH_JAPANESE ? 99 : 91,
                      DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                      M11_GAME_INPUT_REDRAW &&
                  view.actingChampionOrdinal == 0u,
              "F31 CHTW Game action row uses its original C696 geometry");
        {
            static const int modes[] = {M12_PRESENTATION_V1_ORIGINAL,
                M12_PRESENTATION_V20_FILTERED, M12_PRESENTATION_V21_UPSCALED};
            const int savedMode = view.presentationMode;
            const int jp = language == CSB_FMTOWNS_SWITCH_JAPANESE;
            const int width = jp ? 96 : 87, height = jp ? 41 : 45;
            const int panelY = jp ? 159 : 124;
            const int pointerY = jp ? 169 : 135;
            size_t graphics_size = 0;
            uint8_t *graphics = load_profile_graphics(live_profile, &graphics_size);
            uint8_t source[96 * 45];
            CSB_V1_FmtownsItemDecodeReceipt receipt;
            memset(&receipt, 0, sizeof(receipt));
            /* MENUDRAW.C F0395 -> F0660(C013,C009): decode the retail
             * bitmap independently of M11's installed asset cache. F31J's
             * 96-pixel source is right-anchored then clipped by nine pixels.
             * The full final panel, not an isolated blitter, is the oracle. */
            CHECK(graphics && csb_v1_fmtowns_graphics_decode_item(
                      graphics, graphics_size, 13, source, sizeof(source), &receipt) &&
                      receipt.valid && receipt.width == width && receipt.height == height,
                  "F31 oracle decodes original language-specific C013");
            if (receipt.valid && receipt.width == width && receipt.height == height &&
                live_profile) {
                for (int mode = 0; mode < 3; ++mode) {
                    int mismatch = 0;
                    int beforeDirection = live_profile->runtime.party_dir;
                    int beforeX = live_profile->runtime.party_x;
                    int beforeY = live_profile->runtime.party_y;
                    int beforeLevel = live_profile->runtime.current_level;
                    view.presentationMode = modes[mode];
                    M11_GameView_Draw(&view, framebuffer, 320, 200);
                    for (int y = 0; y < height; ++y)
                        for (int x = 0; x < 87; ++x)
                            if (framebuffer[(panelY + y) * 320 + 233 + x] !=
                                source[y * width + x + (jp ? 9 : 0)]) ++mismatch;
                    CHECK(mismatch == 0,
                          "F31 final movement panel matches all original C013 pixels in every mode");
                    /* COMMAND.C C068/C069: exercise genuine existing party
                     * rotation through public mouse input, without relocating
                     * the party or inventing a traversable square. */
                    CHECK(M11_GameView_HandlePointerButton(
                              &view, 248, pointerY, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                              M11_GAME_INPUT_REDRAW &&
                              live_profile->runtime.party_dir == ((beforeDirection + 3) & 3),
                          "F31 original left-arrow mouse region rotates the source party");
                    CHECK(live_profile->runtime.party_x == beforeX &&
                              live_profile->runtime.party_y == beforeY &&
                              live_profile->runtime.current_level == beforeLevel,
                          "F31 left-arrow rotation preserves source party position");
                    CHECK(M11_GameView_HandlePointerButton(
                              &view, 305, pointerY, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                              M11_GAME_INPUT_REDRAW &&
                              live_profile->runtime.party_dir == beforeDirection,
                          "F31 original right-arrow mouse region restores source facing");
                    CHECK(live_profile->runtime.party_x == beforeX &&
                              live_profile->runtime.party_y == beforeY &&
                              live_profile->runtime.current_level == beforeLevel,
                          "F31 right-arrow rotation preserves source party position");
                }
            }
            free(graphics);
            view.presentationMode = savedMode;
        }
        /* F31 G0447 keeps C012 (status selection) separate from C007
         * (inventory): a named status rectangle must never inherit the
         * convenient host inventory behavior.  C187's adjacent source bar
         * is the actual C007 left-click target. */
        memset(&champion_name_rect, 0, sizeof(champion_name_rect));
        CHECK(dm1_v1_champion_status_name_rect_pc34(0,
                                                     &champion_name_rect) &&
                  champion_name_rect.w > 0 && champion_name_rect.h > 0 &&
                  ((void)M11_GameView_HandlePointerButton(
                       &view,
                       champion_name_rect.x + champion_name_rect.w / 2,
                       champion_name_rect.y + champion_name_rect.h / 2,
                       DM1_V1_MOUSE_MASK_LEFT_PC34), 1) &&
                  !view.inventoryPanelActive && view.pointerPositionKnown &&
                  view.pointerX == champion_name_rect.x +
                                       champion_name_rect.w / 2 &&
                  view.pointerY == champion_name_rect.y +
                                       champion_name_rect.h / 2,
              "F31 C012 status rectangle remains a selection-only source route");
        CHECK(M11_GameView_HandlePointerButton(
                  &view, 50, 10, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                  M11_GAME_INPUT_REDRAW && view.inventoryPanelActive &&
                  M11_GameView_HandlePointerButton(
                      &view, 50, 10, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                  M11_GAME_INPUT_REDRAW && !view.inventoryPanelActive,
              "F31 C187 bar opens and closes inventory through C007");
        {
            /* G0447 binds the F31 C113..C116 cells to C125..C128, and
             * F0380 routes those commands directly to IO.C F0070.  Select
             * the occupied source cell from the real MINI.DAT party rather
             * than manufacturing a formation fixture, then move it to an
             * actually empty native icon cell.  F31's 32x32 IODRV cursor
             * is intentionally not asserted here: only F0070's durable
             * GAMEBLOCK Cell/Direction/0x0400 transaction is portable to
             * the M11 source runtime. */
            CSB_V1_BootProfile *formation_profile =
                (CSB_V1_BootProfile *)view.csbBootProfile;
            int source_icon = -1;
            int target_icon = -1;
            int candidate;
            const int icon_x[4] = { 290, 310, 310, 290 };
            const int icon_y[4] = { 7, 7, 21, 21 };

            if (formation_profile &&
                formation_profile->runtime.party_state_valid &&
                formation_profile->runtime.party_state.ChampionCount > 0) {
                source_icon =
                    ((int)formation_profile->runtime.party_state.Champions[0].Cell -
                     formation_profile->runtime.party_state.PartyDirection) & 3;
                for (candidate = 0; candidate < 4; ++candidate) {
                    int champion;
                    int occupied = 0;
                    const int cell = (candidate +
                                      formation_profile->runtime.party_state.PartyDirection) & 3;
                    for (champion = 0;
                         champion < formation_profile->runtime.party_state.ChampionCount;
                         ++champion) {
                        if (formation_profile->runtime.party_state.Champions[champion].Cell ==
                            (uint8_t)cell) {
                            occupied = 1;
                            break;
                        }
                    }
                    if (!occupied) {
                        target_icon = candidate;
                        break;
                    }
                }
            }
            CHECK(source_icon >= 0 && target_icon >= 0 &&
                      M11_GameView_HandlePointerButton(
                          &view, icon_x[source_icon], icon_y[source_icon],
                          DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
                      view.csbFmtownsHeldChampionIconOrdinal ==
                          (unsigned int)source_icon + 1u,
                  "F31 C125-C128 picks up the real MINI.DAT formation icon through F0070");
            CHECK(source_icon >= 0 && target_icon >= 0 &&
                      M11_GameView_HandlePointerButton(
                          &view, icon_x[target_icon], icon_y[target_icon],
                          DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
                      view.csbFmtownsHeldChampionIconOrdinal == 0u &&
                      formation_profile->runtime.party_state.Champions[0].Cell ==
                          (uint8_t)((target_icon +
                                     formation_profile->runtime.party_state.PartyDirection) & 3) &&
                      (formation_profile->runtime.party_state.Champions[0].Attributes &
                       CSB_V1_F0070_ATTRIBUTE_ICON_DIRTY_PC34) != 0u,
                  "F31 F0070 releases the real champion into its selected source icon cell");
        }
        /* CEDTINC8.C F7052 creates M746's first CSBGAME.DAT from the selected
         * verified MINI.DAT.  Exercise that transaction with real source
         * bytes: the temporary directory is removed before this test exits. */
#ifndef _WIN32
        if (!user_save_path || !user_save_path[0]) {
            char bootstrap_dir[] = "/tmp/firestaff-csb-bootstrap-XXXXXX";
            char bootstrap_save_path[1024];
            char bootstrap_stage_path[1024];
            CSB_V1_BootProfile *bootstrap_profile =
                (CSB_V1_BootProfile *)view.csbBootProfile;
            CSB_V1_FmtownsUserSaveReceipt bootstrap_save;
            CSB_V1_FmtownsStartupState bootstrap_state;
            uint32_t random_before = bootstrap_profile
                ? bootstrap_profile->runtime.csbwin_random_seed : 0u;
            int created = 0;
            if (mkdtemp(bootstrap_dir) != NULL &&
                snprintf(bootstrap_save_path, sizeof(bootstrap_save_path),
                         "%s/CSBGAME.DAT", bootstrap_dir) >= 0 &&
                snprintf(bootstrap_stage_path, sizeof(bootstrap_stage_path),
                         "%s.firestaff-bootstrap", bootstrap_save_path) >= 0) {
                if (bootstrap_profile) {
                    snprintf(bootstrap_profile->runtime.party_state.Champions[0].Name,
                             sizeof(bootstrap_profile->runtime.party_state.Champions[0].Name),
                             "%s", "F31NAME");
                    snprintf(bootstrap_profile->runtime.party_state.Champions[0].Title,
                             sizeof(bootstrap_profile->runtime.party_state.Champions[0].Title),
                             "%s", "F31 NINETEEN CHARS!");
                    bootstrap_profile->runtime.party_state.Champions[0].Direction = 2u;
                    bootstrap_profile->runtime.party_state.Champions[0].Cell = 1u;
                }
                CHECK(test_set_env("FIRESTAFF_QUICKSAVE_PATH", bootstrap_save_path),
                      "F31 C140 binds the selected native save-disk slot");
                created = M11_GameView_QuickSave(&view);
                CHECK(created,
                      "F31 C140/F7052 creates a new canonical CSBGAME.DAT from verified MINI.DAT");
                memset(&bootstrap_save, 0, sizeof(bootstrap_save));
                CHECK(created && csb_v1_fmtowns_game_user_save_open(
                          bootstrap_profile, &direct_handoff, bootstrap_save_path,
                          &bootstrap_save) && bootstrap_save.valid,
                      "F31 C140/F7052-created CSBGAME.DAT passes the native F0435 reader");
                memset(&bootstrap_state, 0, sizeof(bootstrap_state));
                CHECK(created && csb_v1_fmtowns_game_load_user_save_state(
                          &bootstrap_save, &bootstrap_state) && bootstrap_state.valid &&
                          strcmp(bootstrap_state.party.Champions[0].Name,
                                 "F31NAME") == 0 &&
                          strcmp(bootstrap_state.party.Champions[0].Title,
                                 "F31 NINETEEN CHARS!") == 0 &&
                          bootstrap_state.party.Champions[0].Direction == 2u &&
                          bootstrap_state.party.Champions[0].Cell == 1u,
                      "F31 F0433/F0435 preserves 8/20 name-title bytes and direction/cell order");
                csb_v1_fmtowns_game_startup_state_free(&bootstrap_state);
                CHECK(created && bootstrap_profile->runtime.csbwin_random_seed ==
                          f31_advance_random_words(
                              random_before, 16u + REDMCSB_F7062_RANDOM_WORDS),
                      "F31 first-save path consumes F7052 and F7062 RNG words exactly once");
                {
                    FILE *stage = fopen(bootstrap_stage_path, "rb");
                    CHECK(stage == NULL,
                          "F31 first-save bootstrap staging file is never retained");
                    if (stage) fclose(stage);
                }
                remove(bootstrap_save_path);
                rmdir(bootstrap_dir);
            } else {
                CHECK(0, "F31 first-save test can allocate an isolated temporary directory");
            }
        }
#endif
        /* F0433 writes only an already-admitted native F31 slot.  It must
         * still reject MINI.DAT and every non-canonical target, while a real
         * CSBGAME.DAT must survive a native write followed by F0435 readback.
         * Both branches use source bytes; no private envelope is accepted. */
        CHECK(test_set_env("FIRESTAFF_QUICKSAVE_PATH",
                           user_save_path && user_save_path[0]
                               ? user_save_path : direct_handoff.startup_mini_path),
              "F31 resume test selects the authentic native save candidate");
        if (user_save_path && user_save_path[0]) {
            CHECK(!M11_GameView_QuickSave(&view) &&
                      strcmp(view.lastAction, "SAVE") == 0 &&
                      strcmp(view.lastOutcome,
                             language == CSB_FMTOWNS_SWITCH_JAPANESE
                                 ? "FM TOWNS NATIVE WRITEBACK REQUIRED"
                                 : "FM TOWNS NATIVE SAVE FAILED") == 0,
                  "F31 MINI.DAT session cannot overwrite a distinct resumed slot");
        } else {
            CHECK(!M11_GameView_QuickSave(&view) &&
                      strcmp(view.lastAction, "SAVE") == 0 &&
                      strcmp(view.lastOutcome,
                             "FM TOWNS NATIVE WRITEBACK REQUIRED") == 0,
                  "F31 live session rejects MINI.DAT as a user save target");
        }
        memset(&external_save_handoff, 0, sizeof(external_save_handoff));
        CHECK(csb_v1_fmtowns_game_user_save_handoff_open(
                  (const CSB_V1_BootProfile *)view.csbBootProfile, language,
                  direct_handoff.startup_mini_path, &external_save_handoff),
              "F31 live profile continues to admit the selected native save");
        memset(&external_save_state, 0, sizeof(external_save_state));
        CHECK(csb_v1_fmtowns_game_load_startup_state(
                  &external_save_handoff, &external_save_state),
              "F31 live profile decodes the selected native save state");
        csb_v1_fmtowns_game_startup_state_free(&external_save_state);
        memset(&hoc_menu_route, 0, sizeof(hoc_menu_route));
        CHECK(!M11_GameView_GetDm1HocMenuRouteReceipt(&view, &hoc_menu_route) ||
                  (!hoc_menu_route.showChampionPanel &&
                   !hoc_menu_route.showResurrectReincarnateChoices),
              "F31 native resume is not hidden behind an active mirror panel");
        result = M11_GameView_QuickLoad(&view);
        CHECK(result,
              "F31 live session restores a native F0435 candidate");
        CHECK(live_profile && live_profile->runtime.game_time ==
                  (user_save_path && user_save_path[0]
                       ? user_save.game_time : direct_handoff.startup_mini_game_time) &&
                  live_profile->runtime.party_x ==
                      (user_save_path && user_save_path[0]
                           ? user_save.party_map_x : 22) &&
                  live_profile->runtime.party_y ==
                      (user_save_path && user_save_path[0]
                           ? user_save.party_map_y : 18),
              "F31 native resume restores the saved source-owned party state");
#ifndef _WIN32
        if (user_save_path && user_save_path[0]) {
            char recovery_dir[] = "/tmp/firestaff-f31-m11-recovery-XXXXXX";
            char selected_path[512];
            char backup_path[512];

            CHECK(mkdtemp(recovery_dir) != NULL &&
                      snprintf(selected_path, sizeof(selected_path),
                               "%s/CSBGAME.DAT", recovery_dir) > 0 &&
                      snprintf(backup_path, sizeof(backup_path),
                               "%s/CSBGAME.BAK", recovery_dir) > 0 &&
                      copy_file(user_save_path, backup_path) &&
                      write_damaged_file(selected_path) &&
                      test_set_env("FIRESTAFF_QUICKSAVE_PATH", selected_path),
                  "F31 M11 backup-recovery slot is staged from real save bytes");
            result = M11_GameView_QuickLoad(&view);
            CHECK(result && live_profile &&
                      live_profile->runtime.game_time == user_save.game_time &&
                      strcmp(view.lastOutcome, "CSB QUICKSAVE RESTORED") == 0 &&
                      access(selected_path, F_OK) == 0 &&
                      access(backup_path, F_OK) != 0,
                  "F31 M11 F9 restores the canonical slot before binding live runtime");
            remove(selected_path);
            remove(backup_path);
            rmdir(recovery_dir);
        }
#endif
        (void)test_set_env("FIRESTAFF_QUICKSAVE_PATH", NULL);
    }
    /* ReDMCSB MUSIC.C F0743 reads G4099[map][y][x] after each game update.
     * The atomic F31 resume owns map 4 and its saved pose, so query the
     * matching retail selector instead of retaining the old map-0 fixture
     * expectation. */
    if (!user_save_path || !user_save_path[0]) CHECK(csb_v1_fmtowns_game_music_track_at(
              &direct_handoff,
              (uint32_t)((const CSB_V1_BootProfile *)view.csbBootProfile)
                  ->runtime.current_level,
              (uint32_t)((const CSB_V1_BootProfile *)view.csbBootProfile)
                  ->runtime.party_x,
              (uint32_t)((const CSB_V1_BootProfile *)view.csbBootProfile)
                  ->runtime.party_y,
              &expected_game_music_track) && expected_game_music_track != 0u,
          "F31 resumed party pose selects a nonzero retail F0743 music byte");
    for (tick = 0u; (!user_save_path || !user_save_path[0]) && tick < 101u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    game_music_started = view.csbFmtownsGameMusicPlayingTrack ==
                             expected_game_music_track &&
                         !view.csbFmtownsGameMusicPending;
    if (!user_save_path || !user_save_path[0]) CHECK(game_music_started,
          "F31 live map music waits 100 F0743 updates then selects its retail CUE track");

    M11_GameView_Shutdown(&view);
    free(materialized_mini);
    if (failures) return 1;
    printf("PASS: real FM Towns SWITCHTW -> %s entrance handoff\n",
           expected_program);
    return 0;
}
