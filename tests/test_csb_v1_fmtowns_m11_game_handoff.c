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
#include "csb_v1_fmtowns_game.h"
#include "csb_v1_fmtowns_portrait.h"
#include "csb_v1_fmtowns_switch.h"
#include "csb_v1_fmtowns_utility_render.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

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
    const char *language_name = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
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
    M12_AssetStatus asset_status;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    unsigned char framebuffer[320 * 200];
    unsigned int tick;
    int result;
    int door_frame_seen = 0;
    int live_frame_nonblack = 0;
    int game_music_started = 0;
    uint8_t expected_game_music_track = 0u;
    CSB_V1_FmtownsSwitchInputReceipt switch_input;
    CSB_V1_StartupRuntimeAssetSession_PC34 direct_session;
    CSB_V1_StartupFullRuntimeReceipt_PC34 direct_runtime;
    CSB_V1_FmtownsGameHandoffReceipt direct_handoff;
    CSB_V1_PartyState mini_party;
    CSB_V1_FmtownsStartupPortraitReceipt mini_portraits;
    CSB_V1_FmtownsStartupState mini_state;
    CSB_V1_DungeonData mini_dungeon;
    CSB_V1_FmtownsUtilityHandoffReceipt utility_handoff;
    CSB_V1_FmtownsUtilityMenuReceipt utility_menu;
    CSB_V1_FmtownsUtilityFontReceipt utility_font;
    CSB_V1_FmtownsUtilityRenderReceipt utility_render;
    CSB_V1_FmtownsUtilityMenuHitBox utility_hit;
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal;
    DM1_V1_ChampionStatusRectPc34 champion_name_rect;
    const CSB_V1_FmtownsSwitchButton *story_button;
    uint8_t music_track;
    unsigned int mini_active_index;
    uint8_t utility_palette[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3];
    uint8_t utility_frame[CSB_V1_FMTOWNS_UTILITY_SCREEN_PIXELS];
    uint8_t portrait_pixels[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
    uint8_t portrait_roundtrip[CSB_FMTOWNS_PORTRAIT_DATA_SIZE];

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
    if (archive_data_dir && archive_data_dir[0]) {
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
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR or "
             "FIRESTAFF_CSB_FMTOWNS_ARCHIVE_DATA_DIR not set");
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.title = "CHAOS STRIKES BACK";
    spec.dataDir = data_dir;
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
    }

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        /* SWITCH.C button two exits with status five and AUTOEXEC.BAT opens
         * UTILE.EXP. The recovered C06 F7042 page consumes only UTILE and
         * MINI.DAT bytes; it must not keep SWITCHTW as a synthetic backdrop. */
        result = M11_GameView_HandlePointerButton(
            &view, 57, 59, DM1_V1_MOUSE_MASK_LEFT_PC34);
        CHECK(result == M11_GAME_INPUT_REDRAW && view.csbFmtownsUtilityBound &&
                  !view.csbFmtownsSwitchBound && !view.csbState.startup_entrance_active,
              "F31E Utility opens its verified C06 initial editor owner");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(memcmp(framebuffer, view.csbFmtownsSwitchPixels,
                     sizeof(framebuffer)) != 0 && framebuffer[9u * 320u + 6u] == 9u,
              "F31E Utility presents its C06-owned source raster, not SWITCHTW");
        result = M11_GameView_HandlePointerButton(
            &view, 290, 67, DM1_V1_MOUSE_MASK_LEFT_PC34);
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        CHECK(result == M11_GAME_INPUT_REDRAW &&
                  view.csbFmtownsUtilitySelectedColor == 3u &&
                  framebuffer[68u * 320u + 287u] == 15u,
              "F31E C06 palette selection redraws the original selected swatch");
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
    CHECK(M11_GameView_GetPresentationSpecialPalette(&view) ==
              VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE,
          "F31 C004 uses the source-owned entrance palette");
    CHECK(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
              M11_GAME_INPUT_REDRAW,
          "F31 Game accepts the source-owned Prison command");
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
    for (tick = 0u; tick < sizeof(framebuffer); ++tick) {
        if (framebuffer[tick] != 0u) {
            live_frame_nonblack = 1;
            break;
        }
    }
    CHECK(live_frame_nonblack,
          "F31 C017 HUD and F0128 viewport draw a real live frame after Prison");
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
        CHECK(live_profile &&
                  M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                      M11_GAME_INPUT_REDRAW &&
                  live_profile->runtime.last_input_dispatch.dequeued &&
                  live_profile->runtime.last_input_dispatch.dispatchedMove,
              "F31 live viewport routes C003 through the dungeon command queue");
        /* ReDMCSB COMMAND.C G0447/C007 and CHAMDRAW.C F0292 own this
         * named status strip.  Use its decoded source rectangle rather than
         * an enum shortcut or a host-invented coordinate, then click it a
         * second time to close through the same live pointer route. */
        memset(&champion_name_rect, 0, sizeof(champion_name_rect));
        CHECK(dm1_v1_champion_status_name_rect_pc34(0,
                                                     &champion_name_rect) &&
                  champion_name_rect.w > 0 && champion_name_rect.h > 0 &&
                  M11_GameView_HandlePointerButton(
                      &view,
                      champion_name_rect.x + champion_name_rect.w / 2,
                      champion_name_rect.y + champion_name_rect.h / 2,
                      DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
                  view.inventoryPanelActive && view.pointerPositionKnown &&
                  view.pointerX == champion_name_rect.x +
                                       champion_name_rect.w / 2 &&
                  view.pointerY == champion_name_rect.y +
                                       champion_name_rect.h / 2,
              "F31 live pointer opens the real MINI.DAT champion inventory");
        CHECK(M11_GameView_HandlePointerButton(
                  &view,
                  champion_name_rect.x + champion_name_rect.w / 2,
                  champion_name_rect.y + champion_name_rect.h / 2,
                  DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
                  !view.inventoryPanelActive,
              "F31 live pointer closes that inventory through C007");
        /* A user save must be the original F31 F0433/F0435 container.  No
         * authentic FM Towns corpus is available here, so the live session
         * must reject the generic Firestaff snapshot rather than manufacture
         * a file and present it as a native save. */
        CHECK(!M11_GameView_QuickSave(&view) &&
                  !M11_GameView_QuickLoad(&view),
              "F31 live session refuses a non-native save envelope");
    }
    /* ReDMCSB MUSIC.C F0743 reads G4099[map][y][x] after each game update.
     * The atomic F31 resume owns map 4 and its saved pose, so query the
     * matching retail selector instead of retaining the old map-0 fixture
     * expectation. */
    CHECK(csb_v1_fmtowns_game_music_track_at(
              &direct_handoff,
              (uint32_t)((const CSB_V1_BootProfile *)view.csbBootProfile)
                  ->runtime.current_level,
              (uint32_t)((const CSB_V1_BootProfile *)view.csbBootProfile)
                  ->runtime.party_x,
              (uint32_t)((const CSB_V1_BootProfile *)view.csbBootProfile)
                  ->runtime.party_y,
              &expected_game_music_track) && expected_game_music_track != 0u,
          "F31 resumed party pose selects a nonzero retail F0743 music byte");
    for (tick = 0u; tick < 101u; ++tick) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    game_music_started = view.csbFmtownsGameMusicPlayingTrack ==
                             expected_game_music_track &&
                         !view.csbFmtownsGameMusicPending;
    CHECK(game_music_started,
          "F31 live map music waits 100 F0743 updates then selects its retail CUE track");

    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    printf("PASS: real FM Towns SWITCHTW -> %s entrance handoff\n",
           expected_program);
    return 0;
}
