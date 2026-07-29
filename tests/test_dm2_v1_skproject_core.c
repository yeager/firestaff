#include "dm2_v1_skproject_core.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_world_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failed;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        ++failed; \
        printf("FAIL: %s\n", msg); \
    } else { \
        printf("PASS: %s\n", msg); \
    } \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_graphics(uint8_t **out_data, size_t *out_size,
                         char *path, size_t path_size)
{
    static const char *suffixes[] = {
        "dm2/GRAPHICS.DAT",
        "dm2/graphics.dat",
        "dm2/DM2GRAPHICS.DAT",
        "dm2/DM2GRA.DAT"
    };
    size_t i;

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static void test_between_value(void)
{
    CHECK(dm2_v1_skproject_between_value(10, 9, 20) == 10,
          "BETWEEN_VALUE clamps below min");
    CHECK(dm2_v1_skproject_between_value(10, 10, 20) == 10,
          "BETWEEN_VALUE admits min");
    CHECK(dm2_v1_skproject_between_value(10, 17, 20) == 17,
          "BETWEEN_VALUE admits middle");
    CHECK(dm2_v1_skproject_between_value(10, 20, 20) == 20,
          "BETWEEN_VALUE admits max");
    CHECK(dm2_v1_skproject_between_value(10, 21, 20) == 20,
          "BETWEEN_VALUE clamps above max");
    CHECK(dm2_v1_skproject_dm2_between_value(-1, 127, 200) == 127,
          "DM2_BETWEEN_VALUE wrapper uses v5 argument order");
}

static void test_temp_rect_ring(void)
{
    DM2_V1_SkprojectTempRectRing ring;
    DM2_V1_SkprojectTempRectReceipt receipt[5];

    dm2_v1_skproject_temp_rect_ring_init(&ring);
    memset(receipt, 0, sizeof(receipt));

    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4,
                                           &receipt[0]) == 1,
          "ALLOC_TEMP_RECT accepts first rect");
    CHECK(receipt[0].slot == 0 && receipt[0].next_slot == 1 &&
              receipt[0].rect.x == 1 && receipt[0].rect.y == 2 &&
              receipt[0].rect.w == 3 && receipt[0].rect.h == 4,
          "ALLOC_TEMP_RECT writes slot 0 and advances ring");
    CHECK(dm2_v1_skproject_alloc_temp_origin_rect(&ring, 5, 6,
                                                  &receipt[1]) == 1,
          "ALLOC_TEMP_ORIGIN_RECT delegates to temp rect");
    CHECK(receipt[1].slot == 1 && receipt[1].next_slot == 2 &&
              receipt[1].rect.x == 0 && receipt[1].rect.y == 0 &&
              receipt[1].rect.w == 5 && receipt[1].rect.h == 6,
          "ALLOC_TEMP_ORIGIN_RECT writes origin rectangle");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 7, 8, 9, 10,
                                           &receipt[2]) == 1 &&
              receipt[2].slot == 2 && receipt[2].next_slot == 3,
          "third temp rect uses slot 2");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 11, 12, 13, 14,
                                           &receipt[3]) == 1 &&
              receipt[3].slot == 3 && receipt[3].next_slot == 0,
          "fourth temp rect wraps next index to zero");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 15, 16, 17, 18,
                                           &receipt[4]) == 1 &&
              receipt[4].slot == 0 && receipt[4].next_slot == 1 &&
              ring.rects[0].x == 15 && ring.rects[0].h == 18,
          "fifth temp rect overwrites slot 0 like skproject ringbuffer");
    CHECK(receipt[0].receipt_hash != 0u && receipt[4].receipt_hash != 0u &&
              receipt[0].receipt_hash != receipt[4].receipt_hash,
          "temp rect receipts carry nonzero slot/value hash");
    CHECK(dm2_v1_skproject_alloc_temp_rect(0, 1, 2, 3, 4,
                                           &receipt[0]) == 0,
          "ALLOC_TEMP_RECT rejects missing ring");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4, 0) == 0,
          "ALLOC_TEMP_RECT rejects missing receipt");
}

static void test_random_helpers(void)
{
    DM2_V1_SkprojectRandomData randdat;

    dm2_v1_skproject_random_init(&randdat);
    CHECK(randdat.random == 0u, "c_random init clears random seed");
    CHECK(dm2_v1_skproject_rand(&randdat) == 0u &&
              randdat.random == 11u,
          "DM2_RAND returns seed*magic+11 shifted by 8");
    CHECK(dm2_v1_skproject_rand16(&randdat, 10u) == 9u,
          "DM2_RAND16 modulo uses the 24-bit DM2_RAND value");
    CHECK(dm2_v1_skproject_randbit(&randdat) == 0,
          "DM2_RANDBIT masks one random bit");
    CHECK(dm2_v1_skproject_randdir(&randdat) == 0u,
          "DM2_RANDDIR masks two random direction bits");
    CHECK(dm2_v1_skproject_rand(&randdat) == 13344383u,
          "c_random sequence advances through every helper");
    CHECK(dm2_v1_skproject_rand16(&randdat, 0u) == 0u,
          "DM2_RAND16 zero range returns zero");
}

static void test_ibmio_anim_mouse_runtime_family(void)
{
    DM2_V1_SkprojectIbmioPaletteState palette_state;
    DM2_V1_SkprojectIbmioPaletteReceipt palette_receipt;
    DM2_V1_SkprojectPaletteSetReceipt select_receipt;
    DM2_V1_SkprojectIbmioBlit4To8Receipt blit_receipt;
    DM2_V1_SkprojectAnimCopy4BppReceipt copy_receipt;
    DM2_V1_SkprojectMouseState mouse_state;
    DM2_V1_SkprojectMouseHideReceipt hide_receipt;
    DM2_V1_SkprojectMouseShapeReceipt shape_receipt;
    DM2_V1_SkprojectMouseBoundsReceipt bounds_receipt;
    DM2_V1_SkprojectAnimRuntimeState anim_state;
    DM2_V1_SkprojectAnimVectorReceipt vector_receipt;
    DM2_V1_SkprojectAnimTimerInstallReceipt timer_install;
    DM2_V1_SkprojectAnimTimerTickReceipt timer_tick;
    DM2_V1_SkprojectIbmioPollReceipt poll_receipt;
    DM2_V1_SkprojectIbmioWaitEventReceipt wait_receipt;
    DM2_V1_SkprojectScreenRectFillReceipt rect_fill;
    DM2_V1_SkprojectScreenClearReceipt screen_clear;
    DM2_V1_SkprojectSoundAvailableReceipt sound_receipt;
    uint8_t rgb[16][3];
    uint8_t pal16[16];
    uint8_t src4[8] = { 0x01u, 0x23u, 0x45u, 0x67u,
                        0x89u, 0xabu, 0xcdu, 0xefu };
    uint8_t dst8[16];
    uint8_t copy4[8] = { 0x01u, 0x23u, 0x45u, 0x67u,
                         0x89u, 0xabu, 0xcdu, 0xefu };
    uint16_t bounds[4] = { 10u, 20u, 30u, 40u };

    for (uint8_t i = 0u; i < 16u; ++i) {
        rgb[i][0] = (uint8_t)(i * 4u);
        rgb[i][1] = (uint8_t)(255u - i * 4u);
        rgb[i][2] = (uint8_t)(i * 8u);
        pal16[i] = (uint8_t)(15u - i);
    }
    memset(dst8, 0, sizeof(dst8));
    dm2_v1_skproject_ibmio_palette_state_init(&palette_state);
    palette_state.update_palette = 1u;
    CHECK(dm2_v1_skproject_00eb_04bc_palette_set(
              &palette_state, rgb, 0u, &palette_receipt) &&
              palette_receipt.valid &&
              palette_receipt.base_palette_updated &&
              palette_receipt.driver_update_requested &&
              palette_state.rgb6[0][0] == 0u &&
              palette_state.rgb6[15][1] == (uint8_t)((255u - 60u) >> 2) &&
              palette_state.base_rgb6[15][2] == 30u,
          "_00eb_04bc shifts 16 RGB888 entries into 6-bit palette set zero");
    CHECK(dm2_v1_skproject_0759_0688_palette_set(
              &palette_state, rgb, 2u, &palette_receipt) &&
              palette_receipt.valid && !palette_receipt.base_palette_updated &&
              palette_state.rgb6[32][0] == 0u,
          "_0759_0688 delegates to the same palette-set receipt");
    CHECK(dm2_v1_skproject_0759_06a1_select_palette_set(
              &palette_state, 2u, &select_receipt) &&
              select_receipt.valid && palette_state.active_set == 2u &&
              select_receipt.driver_setcolors_requested,
          "_0759_06a1 selects the active IBMIO palette set");

    CHECK(dm2_v1_skproject_00eb_070c_blit_4to8(
              src4, sizeof(src4), 1u, dst8, sizeof(dst8), 3u, 6u, pal16,
              &blit_receipt) &&
              blit_receipt.valid && blit_receipt.copied_pixels == 6u &&
              dst8[3] == 14u && dst8[4] == 13u && dst8[8] == 9u,
          "_00eb_070c loads the 16-colour table and expands a 4bpp row");
    memset(dst8, 0, sizeof(dst8));
    CHECK(dm2_v1_skproject_0759_0310_blit_4to8_self(
              src4, sizeof(src4), 2u, dst8, sizeof(dst8), 4u, pal16,
              &blit_receipt) &&
              blit_receipt.valid && dst8[2] == 13u && dst8[5] == 10u,
          "_0759_0310 uses the same source and destination pixel offset");
    CHECK(!dm2_v1_skproject_00eb_070c_blit_4to8(
              src4, sizeof(src4), 15u, dst8, sizeof(dst8), 0u, 2u, pal16,
              &blit_receipt) && blit_receipt.blocked_out_of_bounds,
          "_00eb_070c rejects out-of-bounds source rows");

    CHECK(dm2_v1_skproject_0759_02c6_copy_4bpp_sequence(
              copy4, sizeof(copy4), 8u, 0u, 4u, &copy_receipt) &&
              copy_receipt.valid && copy4[4] == 0x01u &&
              copy4[5] == 0x23u,
          "_0759_02c6 copies a packed 4bpp sequence inside the animation buffer");

    dm2_v1_skproject_mouse_state_init(&mouse_state);
    CHECK(dm2_v1_skproject_01b0_0adb_hide_mouse(
              &mouse_state, &hide_receipt) &&
              hide_receipt.valid && hide_receipt.hide_depth_after == 1u &&
              hide_receipt.locked_mouse_event && hide_receipt.redrew_cursor,
          "_01b0_0adb locks and redraws only for the first hide");
    CHECK(dm2_v1_skproject_01b0_0adb_hide_mouse(
              &mouse_state, &hide_receipt) &&
              hide_receipt.valid && hide_receipt.hide_depth_after == 2u &&
              !hide_receipt.locked_mouse_event,
          "_01b0_0adb nested hides only increment the depth");
    mouse_state.hide_depth = 0u;
    CHECK(dm2_v1_skproject_01b0_0c70_set_cursor_shape(
              &mouse_state, 7u, &shape_receipt) &&
              shape_receipt.valid && shape_receipt.shape_after == 7u &&
              shape_receipt.redrew_before_shape_change &&
              shape_receipt.redrew_after_shape_change,
          "_01b0_0c70 redraws around visible cursor shape changes");
    CHECK(dm2_v1_skproject_01b0_0ca4_set_cursor_bounds(
              &mouse_state, bounds, 32u, &bounds_receipt) &&
              bounds_receipt.valid && bounds_receipt.bounds[2] == 30u &&
              bounds_receipt.mode == 32u && mouse_state.cursor_bounds_dirty,
          "_01b0_0ca4 stores four cursor bounds and marks them dirty");

    dm2_v1_skproject_anim_runtime_state_init(&anim_state);
    CHECK(anim_state.screen_rect.w == 320 && anim_state.screen_rect.h == 200,
          "ANIM runtime init preserves source 320x200 screen rect");
    CHECK(dm2_v1_skproject_0759_0126_capture_int_ff(
              &anim_state, 0xff001234u, &vector_receipt) &&
              vector_receipt.valid &&
              anim_state.interrupt_ff_vector == 0xff001234u,
          "_0759_0126 captures interrupt FF vector");
    anim_state.anim_countdown = 1000;
    CHECK(dm2_v1_skproject_0759_06db_install_timer(
              &anim_state, 0xfe00abcdu, 55u, &timer_install) &&
              timer_install.valid &&
              anim_state.interrupt_fe_vector == 0xfe00abcdu &&
              anim_state.timer_reload_ticks == 55u &&
              anim_state.display_callback_installed,
          "_0759_06db captures interrupt FE and installs timer callback");
    CHECK(dm2_v1_skproject_0759_06c2_timer_tick(
              &anim_state, &timer_tick) &&
              timer_tick.valid && timer_tick.countdown_before == 1000 &&
              timer_tick.countdown_after == 945,
          "_0759_06c2 subtracts timer reload from ANIM countdown");
    anim_state.display_mode_active = 1u;
    CHECK(dm2_v1_skproject_0759_072c_poll_ibmio(
              &anim_state, &poll_receipt) &&
              poll_receipt.valid && poll_receipt.display_callback_called &&
              !poll_receipt.event_available,
          "_0759_072c polls IBMIO event availability after display callback");
    CHECK(dm2_v1_skproject_anim_runtime_push_event(&anim_state, 0x1048u) &&
              dm2_v1_skproject_0759_072c_poll_ibmio(
                  &anim_state, &poll_receipt) &&
              poll_receipt.event_available && poll_receipt.event_count == 1u,
          "ANIM runtime event queue feeds _0759_072c");
    CHECK(dm2_v1_skproject_0759_071b_wait_ibmio_event(
              &anim_state, &wait_receipt) &&
              wait_receipt.valid && wait_receipt.event_word == 0x1048u &&
              wait_receipt.event_count_before == 1u &&
              wait_receipt.event_count_after == 0u,
          "_0759_071b consumes one IBMIO event from the source queue");
    CHECK(!dm2_v1_skproject_0759_071b_wait_ibmio_event(
              &anim_state, &wait_receipt) &&
              wait_receipt.blocked_no_event,
          "_0759_071b stays fail-closed instead of blocking on empty queue");
    CHECK(dm2_v1_skproject_0759_065f_fill_screen_rect(
              &anim_state, &rect_fill) &&
              rect_fill.valid && rect_fill.filled_pixels == 64000u &&
              rect_fill.color == 0u,
          "_0759_065f fills the source screen rect with colour zero");
    CHECK(dm2_v1_skproject_0759_06b5_clear_screen(
              &anim_state, &screen_clear) &&
              screen_clear.valid && screen_clear.lines_filled == 64000u &&
              screen_clear.lfsr_lines_visited == 65535u,
          "_0759_06b5 clears screen through source LFSR line order");
    anim_state.sound_card_type = 0u;
    CHECK(dm2_v1_skproject_01b0_1ed2_sound_available(
              &anim_state, &sound_receipt) &&
              sound_receipt.valid && !sound_receipt.available,
          "_01b0_1ed2 reports no sound card when type is zero");
    anim_state.sound_card_type = 2u;
    CHECK(dm2_v1_skproject_01b0_1ed2_sound_available(
              &anim_state, &sound_receipt) &&
              sound_receipt.valid && sound_receipt.available,
              "_01b0_1ed2 reports sound card availability from runtime type");
}

static void test_ui_predicate_1031_runtime_family(void)
{
    DM2_V1_SkprojectUiPredicateState state;
    DM2_V1_SkprojectUiNodeRef ref = { 0x84u, 0u, 19u };
    DM2_V1_SkprojectUiPredicateReceipt receipt;

    dm2_v1_skproject_ui_predicate_state_init(&state);
    CHECK(state.player_at_position[0] == -1 &&
              state.player_at_position[3] == -1,
          "_1031 UI predicate state init marks all party positions empty");

    ref.b1 = 7u;
    CHECK(dm2_v1_skproject_1031_dispatch_predicate(
              DM2_V1_SKPROJECT_UI_PRED_RETURN_1, &state, &ref, &receipt) &&
              receipt.valid && receipt.predicate_index == 0u &&
              receipt.ref_b0 == 0x84u && receipt.ref_w2 == 19u,
          "_4976_0cba[0] RETURN_1 always admits the UI node");

    state.game_has_ended = 5u;
    ref.b1 = 5u;
    CHECK(dm2_v1_skproject_1031_dispatch_predicate(
              DM2_V1_SKPROJECT_UI_PRED_IS_GAME_ENDED, &state, &ref,
              &receipt) && receipt.result,
          "_4976_0cba[1] IS_GAME_ENDED compares ref b1 to glbGameHasEnded");

    state.selected_panel_token = 9u;
    ref.b1 = 9u;
    CHECK(dm2_v1_skproject_1031_0023(&state, &ref, &receipt) &&
              receipt.predicate_index == DM2_V1_SKPROJECT_UI_PRED_1031_0023 &&
              receipt.result,
          "_1031_0023 admits the selected right-panel token");
    ref.b1 = 8u;
    CHECK(!dm2_v1_skproject_1031_0023(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_0023 rejects a different panel token");

    state.champion_inventory = 2u;
    ref.b1 = 2u;
    CHECK(dm2_v1_skproject_1031_003e(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_003e admits the current champion inventory id");
    ref.b1 = 6u;
    CHECK(!dm2_v1_skproject_1031_003e(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_003e rejects the champion inventory mirror slot");
    ref.b1 = 12u;
    CHECK(dm2_v1_skproject_1031_003e(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_003e admits non-champion inventory UI refs");
    ref.b1 = 4u;
    CHECK(!dm2_v1_skproject_1031_003e(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_003e rejects low fixed champion refs");

    state.champion_hp[1] = 42u;
    ref.b1 = 1u;
    CHECK(dm2_v1_skproject_1031_007b(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_007b admits a champion with nonzero HP");
    ref.b1 = 2u;
    CHECK(!dm2_v1_skproject_1031_007b(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_007b rejects a zero-HP champion");
    ref.b1 = 4u;
    CHECK(!dm2_v1_skproject_1031_007b(&state, &ref, &receipt) &&
              receipt.blocked_champion_index,
          "_1031_007b fails closed for refs outside glbChampionSquad[4]");

    state.player_dir = 1u;
    state.player_at_position[3] = 0;
    ref.b1 = 2u;
    CHECK(dm2_v1_skproject_1031_009e(&state, &ref, &receipt) &&
              receipt.player_position_index == 3u &&
              receipt.player_at_position == 0,
          "_1031_009e probes GET_PLAYER_AT_POSITION((b1+dir)&3)");
    ref.b1 = 1u;
    CHECK(!dm2_v1_skproject_1031_009e(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result &&
              receipt.player_position_index == 2u,
          "_1031_009e rejects an empty rotated party position");

    state.toggle_5dbc = 0u;
    ref.b1 = 0u;
    CHECK(dm2_v1_skproject_1031_00c5(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_00c5 admits zero ref only when _4976_5dbc is zero");
    state.toggle_5dbc = 1u;
    ref.b1 = 3u;
    CHECK(dm2_v1_skproject_1031_00c5(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_00c5 admits nonzero ref when _4976_5dbc is nonzero");
    ref.b1 = 0u;
    CHECK(!dm2_v1_skproject_1031_00c5(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_00c5 rejects zero ref when _4976_5dbc is nonzero");

    state.champion_index = 0u;
    ref.b1 = 5u;
    CHECK(dm2_v1_skproject_1031_00f3(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_00f3 admits refs above party position range with no champion index");
    ref.b1 = 2u;
    CHECK(dm2_v1_skproject_1031_00f3(&state, &ref, &receipt) &&
              receipt.result && receipt.player_position_index == 3u,
          "_1031_00f3 admits occupied rotated party position");
    state.champion_index = 1u;
    CHECK(!dm2_v1_skproject_1031_00f3(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_00f3 rejects all refs while champion index is active");

    state.selected_spell_panel = 6u;
    ref.b1 = 6u;
    CHECK(dm2_v1_skproject_1031_012d(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_012d admits selected spell panel when champion index is active");
    state.champion_index = 0u;
    CHECK(!dm2_v1_skproject_1031_012d(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_012d rejects selected spell panel without champion index");

    state.champion_index = 1u;
    state.champion_runes_count[0] = 2u;
    ref.b1 = 0x04u;
    CHECK(dm2_v1_skproject_1031_dispatch_predicate(
              DM2_V1_SKPROJECT_UI_PRED_1031_014F, &state, &ref, &receipt) &&
              receipt.result,
          "_1031_014f admits refs matching the active champion rune-count bit");
    ref.b1 = 0x08u;
    CHECK(!dm2_v1_skproject_1031_014f(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_014f rejects refs without the active rune-count bit");
    state.champion_index = 5u;
    CHECK(!dm2_v1_skproject_1031_014f(&state, &ref, &receipt) &&
              receipt.blocked_champion_index,
          "_1031_014f fails closed for champion indexes outside the party");

    state.champion_index = 1u;
    state.magical_map_flags = 0x8000u;
    state.selected_spell_panel = 6u;
    ref.b1 = 6u;
    CHECK(dm2_v1_skproject_1031_dispatch_predicate(
              DM2_V1_SKPROJECT_UI_PRED_1031_0184, &state, &ref, &receipt) &&
              receipt.result,
          "_1031_0184 maps magic-map flag to selected spell panel predicate");
    state.magical_map_flags = 0u;
    ref.b1 = 5u;
    CHECK(dm2_v1_skproject_1031_0184(&state, &ref, &receipt) &&
              receipt.result,
          "_1031_0184 maps non-magic-map state to fixed ref five");
    state.champion_index = 0u;
    CHECK(!dm2_v1_skproject_1031_0184(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_0184 rejects all refs without an active champion");

    state.right_panel_type = 3u;
    ref.b1 = 3u;
    CHECK(dm2_v1_skproject_1031_dispatch_predicate(
              DM2_V1_SKPROJECT_UI_PRED_1031_01BA, &state, &ref, &receipt) &&
              receipt.result,
          "_1031_01ba admits the current right-panel type");
    ref.b1 = 4u;
    CHECK(!dm2_v1_skproject_1031_01ba(&state, &ref, &receipt) &&
              receipt.valid && !receipt.result,
          "_1031_01ba rejects non-current right-panel type");

    CHECK(!dm2_v1_skproject_1031_dispatch_predicate(
              12u, &state, &ref, &receipt) &&
              receipt.blocked_unknown_predicate,
          "_4976_0cba dispatcher rejects unknown predicate indexes");
    CHECK(!dm2_v1_skproject_1031_dispatch_predicate(
              DM2_V1_SKPROJECT_UI_PRED_1031_0023, NULL, &ref, &receipt) &&
              receipt.blocked_missing_state,
          "_1031 predicates require real UI runtime state");
}

static void test_ui_1031_node_flow_helpers(void)
{
    DM2_V1_SkprojectUiPredicateState state;
    DM2_V1_SkprojectUiNodeRef root = { DM2_V1_SKPROJECT_UI_PRED_1031_014F, 0u, 0u };
    DM2_V1_SkprojectUiNodeRef nodes[4];
    DM2_V1_SkprojectUiLeafMeta leaves[4];
    uint8_t child_bytes[5] = { 1u, 2u, 0x80u, 3u, 0x80u };
    DM2_V1_SkprojectUiChildListReceipt child_receipt;
    DM2_V1_SkprojectUiResolveRectReceipt rect_receipt;
    DM2_V1_SkprojectUiTraverseReceipt traverse;
    DM2_V1_SkprojectRect rects[20];
    DM2_V1_SkprojectRect out_rect;

    dm2_v1_skproject_ui_predicate_state_init(&state);
    state.champion_index = 1u;
    state.champion_runes_count[0] = 2u;
    state.magical_map_flags = 0x8000u;
    state.selected_spell_panel = 6u;
    memset(nodes, 0, sizeof(nodes));
    memset(leaves, 0, sizeof(leaves));
    nodes[1].b1 = 0x04u;
    nodes[1].w2 = 1u;
    nodes[2].b0 = (uint8_t)(0x80u | DM2_V1_SKPROJECT_UI_PRED_1031_0184);
    nodes[2].b1 = 0x04u;
    nodes[2].w2 = 3u;
    nodes[3].b1 = 6u;
    nodes[3].w2 = 3u;

    CHECK(dm2_v1_skproject_1031_023b_child_list(
              child_bytes, sizeof(child_bytes), &root, &child_receipt) &&
              child_receipt.valid &&
              child_receipt.child_offset == 0u &&
              child_receipt.first_child_index == 1u,
          "_1031_023b returns the child-list cursor from sk1891.w2");
    root.w2 = 9u;
    CHECK(!dm2_v1_skproject_1031_023b_child_list(
              child_bytes, sizeof(child_bytes), &root, &child_receipt) &&
              child_receipt.blocked_child_offset,
          "_1031_023b fails closed when the child-list cursor is out of bounds");
    root.w2 = 0u;

    for (uint16_t i = 0u; i < 20u; ++i) {
        rects[i].x = (int16_t)i;
        rects[i].y = (int16_t)(i + 10u);
        rects[i].w = 5;
        rects[i].h = 6;
    }
    CHECK(dm2_v1_skproject_1031_01d5_resolve_rect(
              3u, rects, 20u, rects, 20u, &out_rect, &rect_receipt) &&
              rect_receipt.valid && out_rect.x == 3 && out_rect.y == 13 &&
              rect_receipt.offset_rectno == 0xffffu,
          "_1031_01d5 returns QUERY_EXPANDED_RECT without offset flags");
    CHECK(dm2_v1_skproject_1031_01d5_resolve_rect(
              0x8003u, rects, 20u, rects, 20u, &out_rect, &rect_receipt) &&
              rect_receipt.applied_8000_offset &&
              rect_receipt.offset_rectno == 7u &&
              out_rect.x == 10 && out_rect.y == 30,
          "_1031_01d5 applies QUERY_TOPLEFT_OF_RECT(7) for the 0x8000 flag");
    CHECK(dm2_v1_skproject_1031_01d5_resolve_rect(
              0x4003u, rects, 20u, rects, 20u, &out_rect, &rect_receipt) &&
              rect_receipt.applied_4000_offset &&
              rect_receipt.offset_rectno == 18u &&
              out_rect.x == 21 && out_rect.y == 41,
          "_1031_01d5 applies QUERY_TOPLEFT_OF_RECT(18) for the 0x4000 flag");
    CHECK(!dm2_v1_skproject_1031_01d5_resolve_rect(
              0x4013u, rects, 20u, rects, 10u, &out_rect, &rect_receipt) &&
              rect_receipt.blocked_rect_out_of_bounds,
          "_1031_01d5 fails closed when a required top-left rect is missing");

    CHECK(dm2_v1_skproject_1031_027e_traverse(
              &state, &root, nodes, 4u, child_bytes, sizeof(child_bytes),
              leaves, 4u, &traverse) &&
              traverse.valid &&
              traverse.visited_nodes == 3u &&
              traverse.marked_leaves == 2u &&
              traverse.recursed_nodes == 1u &&
              (leaves[1].b6 & 0x40u) != 0u &&
              (leaves[3].b6 & 0x40u) != 0u,
          "_1031_027e traverses children through the dispatcher and marks leaf metadata");
    nodes[1].w2 = 9u;
    leaves[1].b6 = 0u;
    leaves[3].b6 = 0u;
    CHECK(!dm2_v1_skproject_1031_027e_traverse(
              &state, &root, nodes, 4u, child_bytes, sizeof(child_bytes),
              leaves, 4u, &traverse) &&
              traverse.blocked_leaf_index,
          "_1031_027e fails closed when a leaf metadata index is out of bounds");
}

static void test_ui_1031_action_and_tree_runtime(void)
{
    DM2_V1_SkprojectUiPredicateState state;
    DM2_V1_SkprojectUiRuntimeState runtime;
    DM2_V1_SkprojectUiNodeRef roots[2];
    DM2_V1_SkprojectUiNodeRef nodes[4];
    DM2_V1_SkprojectUiLeafMeta leaves[4];
    DM2_V1_SkprojectUiClickRectNode clickrects[4];
    DM2_V1_SkprojectUiAction actions[5];
    DM2_V1_SkprojectRect rects[20];
    uint8_t child_bytes[7] = { 1u, 2u, 0x80u, 3u, 0x80u, 0u, 0x80u };
    DM2_V1_SkprojectUiActionListReceipt action_list;
    DM2_V1_SkprojectUiActionResolveReceipt resolve;
    DM2_V1_SkprojectUiHitTestReceipt hit;
    DM2_V1_SkprojectUiActionSearchReceipt search;
    DM2_V1_SkprojectUiPendingRedrawReceipt redraw;
    DM2_V1_SkprojectUiMouseCaptureReceipt capture;
    DM2_V1_SkprojectUiEventResetReceipt reset;
    DM2_V1_SkprojectUiSelectTreeReceipt select_receipt;
    DM2_V1_SkprojectUiButtonGroup button_group;
    DM2_V1_SkprojectUiCenteredButtonReceipt centered;
    DM2_V1_SkprojectRect mouse_rect = { 30, 40, 80, 50 };
    DM2_V1_SkprojectRect centered_rect;
    DM2_V1_SkprojectUiAction direct_actions[3];

    dm2_v1_skproject_ui_predicate_state_init(&state);
    dm2_v1_skproject_ui_runtime_state_init(&runtime);
    memset(roots, 0, sizeof(roots));
    memset(nodes, 0, sizeof(nodes));
    memset(leaves, 0, sizeof(leaves));
    memset(clickrects, 0, sizeof(clickrects));
    memset(actions, 0, sizeof(actions));
    state.champion_index = 1u;
    state.champion_runes_count[0] = 2u;
    state.magical_map_flags = 0x8000u;
    state.selected_spell_panel = 6u;

    roots[0].b0 = DM2_V1_SKPROJECT_UI_PRED_1031_014F;
    roots[0].w2 = 0u;
    roots[1].b0 = DM2_V1_SKPROJECT_UI_PRED_1031_014F;
    roots[1].w2 = 5u;
    nodes[1].b1 = 0x04u;
    nodes[1].w2 = 1u;
    nodes[2].b0 = (uint8_t)(0x80u | DM2_V1_SKPROJECT_UI_PRED_1031_0184);
    nodes[2].b1 = 0x04u;
    nodes[2].w2 = 3u;
    nodes[3].b1 = 6u;
    nodes[3].w2 = 3u;
    leaves[1].w0 = 4u;
    leaves[1].w2 = 0u;
    leaves[1].w4 = 2u;
    leaves[1].b6 = 1u;
    leaves[2].w2 = 0xffffu;
    leaves[3].w0 = 0x8005u;
    leaves[3].w2 = 0xffffu;
    leaves[3].w4 = 3u;
    leaves[3].b6 = 2u;
    actions[0].w0 = 0x0123u;
    actions[0].w2 = 4u;
    actions[0].w4 = 0x0002u;
    actions[1].w0 = 0x8124u;
    actions[1].w2 = 5u;
    actions[1].w4 = 0x0004u;
    actions[2].w0 = 0x8456u;
    actions[2].w2 = 0x00aau;
    actions[2].w4 = 0u;
    actions[3].w0 = 0x0567u;
    actions[3].w2 = 0x00bbu;
    actions[3].w4 = 0u;
    actions[4].w0 = 0x8568u;
    actions[4].w2 = 0x00ccu;
    actions[4].w4 = 0u;
    for (uint16_t i = 0u; i < 20u; ++i) {
        rects[i].x = (int16_t)(i * 3u);
        rects[i].y = (int16_t)(i * 2u);
        rects[i].w = 8;
        rects[i].h = 7;
    }

    CHECK(dm2_v1_skproject_1031_024c_action_list(
              &nodes[1], leaves, 4u, &action_list) &&
              action_list.valid && action_list.action_index == 0u,
          "_1031_024c resolves a leaf action-list index through sk16ed.w2");
    nodes[1].w2 = 2u;
    CHECK(!dm2_v1_skproject_1031_024c_action_list(
              &nodes[1], leaves, 4u, &action_list) &&
              action_list.valid && !action_list.found &&
              action_list.action_index == 0xffffu,
          "_1031_024c returns NULL-equivalent for 0xffff action-list links");
    nodes[1].w2 = 1u;

    CHECK(dm2_v1_skproject_1031_030a_hit_test(
              &state, &roots[0], nodes, 4u, child_bytes, sizeof(child_bytes),
              leaves, 4u, actions, 5u, rects, 20u, rects, 20u,
              rects[4].x, rects[4].y, 0x0002u, &hit) &&
              hit.valid && hit.selected_event == 0x0123u &&
              hit.selected_leaf_index == 1u &&
              hit.selected_action_index == 0u,
          "_1031_030a hit-tests leaf rects and returns the first matching action");
    CHECK(!dm2_v1_skproject_1031_030a_hit_test(
              &state, &roots[0], nodes, 4u, child_bytes, sizeof(child_bytes),
              leaves, 4u, actions, 5u, rects, 20u, rects, 20u,
              300, 190, 0x0002u, &hit) &&
              !hit.valid && hit.selected_event == 0u,
          "_1031_030a returns zero when no admitted leaf rect contains the point");

    CHECK(dm2_v1_skproject_1031_03f2_find_action(
              &state, &roots[0], nodes, 4u, child_bytes, sizeof(child_bytes),
              leaves, 4u, actions, 5u, 0x00aau, &search) &&
              search.valid && search.selected_event == 0x0456u &&
              search.selected_leaf_index == 1u,
          "_1031_03f2 finds an action code through the admitted node tree");
    CHECK(dm2_v1_skproject_1031_03f2_find_action(
              &state, &roots[0], nodes, 4u, child_bytes, sizeof(child_bytes),
              leaves, 4u, actions, 5u, 0x00bbu, &search) &&
              search.valid && search.selected_event == 0x0567u &&
              search.recursed_nodes == 1u,
          "_1031_03f2 searches recursively through admitted branch nodes");

    memset(direct_actions, 0, sizeof(direct_actions));
    direct_actions[0].w0 = 0x0020u;
    direct_actions[0].w2 = 0x8004u;
    direct_actions[0].w4 = 0x0002u;
    direct_actions[1].w0 = 0x0021u;
    direct_actions[1].w2 = 0x4005u;
    direct_actions[1].w4 = 0x0802u;
    direct_actions[2].w0 = 0x8022u;
    direct_actions[2].w2 = 6u;
    direct_actions[2].w4 = 0x0004u;
    CHECK(dm2_v1_skproject_1031_0a88_action_hit(
              &runtime, direct_actions, 3u, 0u, rects, 20u, rects, 20u,
              (int16_t)(rects[4].x + rects[7].x),
              (int16_t)(rects[4].y + rects[7].y), 0x0002u, &resolve) &&
              resolve.valid && resolve.selected_event == 0x0020u &&
              resolve.selected_rectno == 4u &&
              resolve.selected_offset_rectno == 7u &&
              runtime.ui_event_code == 0x0020u &&
              runtime.ui_event_delta == 1u &&
              runtime.selected_x == (int16_t)(rects[4].x + rects[7].x),
          "_1031_0a88 resolves a clickable action and writes the UI event latch");
    CHECK(!dm2_v1_skproject_1031_0a88_action_hit(
              &runtime, direct_actions, 3u, 0u, rects, 20u, rects, 20u,
              300, 190, 0x0002u, &resolve) &&
              !resolve.found && resolve.rect.w == 0 && resolve.rect.h == 0,
          "_1031_0a88 returns no event for a real action outside its rect");

    CHECK(dm2_v1_skproject_1031_0c58_select_event(
              &runtime, 0x0022u, direct_actions, 3u, 0u,
              rects, 20u, rects, 20u, &resolve) &&
              resolve.valid && resolve.selected_event == 0x0022u &&
              resolve.selected_action_index == 2u &&
              runtime.selected_rectno == 6u &&
              runtime.selected_x == rects[6].x &&
              runtime.ui_event_delta == 3u,
          "_1031_0c58 selects an action by event code and latches rect origin");
    CHECK(!dm2_v1_skproject_1031_0c58_select_event(
              &runtime, 0x0030u, direct_actions, 3u, 0u,
              rects, 20u, rects, 20u, &resolve) &&
              runtime.selected_rectno == 0xffffu &&
              runtime.ui_event_code == 0x0030u &&
              runtime.ui_event_delta == 0u,
          "_1031_0c58 clears the selected rect when the source list has no event match");

    runtime.event_count = 0u;
    runtime.event_read_index = 0u;
    runtime.event_write_index = 0u;
    runtime.pending_mouse_event = 1u;
    runtime.pending_event.button = 0x0040u;
    runtime.pending_event.x = 23;
    runtime.pending_event.y = 45;
    runtime.ui_event_code = 0x1234u;
    CHECK(dm2_v1_skproject_1031_0b7e_flush_pending_mouse(
              &runtime, &reset) &&
              reset.valid && reset.queued_pending_event &&
              runtime.pending_mouse_event == 0u &&
              runtime.event_count == 1u &&
              runtime.event_queue[0].button == 0x0040u &&
              runtime.ui_event_code == 0x1234u,
          "_1031_0b7e queues the pending mouse event without resetting UI selection");

    memset(&button_group, 0, sizeof(button_group));
    button_group.button_dbidx = 0xffffu;
    CHECK(dm2_v1_skproject_1031_10c8_center_button(
              &button_group, NULL, &mouse_rect, 20u, 10u,
              &centered_rect, &centered) &&
              centered.valid && centered.copied_mouse_rect &&
              centered.requested_alloc_clickrectdata &&
              centered_rect.x == 60 && centered_rect.y == 60 &&
              centered_rect.w == 20 && centered_rect.h == 10 &&
              button_group.copied_mouse_rect &&
              button_group.allocated_clickrectdata,
          "_1031_10c8 copies the source mouse rect and centers the requested button rect");

    runtime.pending_capture_redraw = 1u;
    CHECK(dm2_v1_skproject_1031_04f5_clear_pending_redraw(
              &runtime, &redraw) &&
              redraw.valid && redraw.cleared_pending_capture_redraw &&
              runtime.requested_guidraw_29ee_000f,
          "_1031_04f5 clears the pending redraw gate and requests the source redraw hook");
    runtime.show_item_stats = 1u;
    runtime.capture_item_stats = 1u;
    CHECK(dm2_v1_skproject_1031_050c_release_item_capture(
              &runtime, &capture) &&
              capture.valid && capture.cleared_sources &&
              capture.requested_mouse_release_capture &&
              capture.requested_show_mouse_cursor &&
              runtime.mouse_visibility == 1u,
          "_1031_050c clears item stat captures and restores mouse visibility");

    runtime.event_queue[0].button = 0x0040u;
    runtime.event_queue[1].button = 0x0001u;
    runtime.event_queue[2].button = 0x0060u;
    runtime.event_count = 3u;
    runtime.pending_mouse_event = 1u;
    runtime.pending_event.button = 0x0004u;
    runtime.ui_event_code = 0x0456u;
    runtime.selected_rectno = 8u;
    CHECK(dm2_v1_skproject_1031_098e_reset_events(&runtime, &reset) &&
              reset.valid && reset.kept_events == 2u &&
              reset.dropped_events == 1u &&
              reset.queued_pending_event &&
              runtime.event_count == 3u &&
              runtime.ui_event_code == 0u &&
              runtime.selected_rectno == 0xffffu,
          "_1031_098e filters mouse queue, resets UI selection, and queues pending event");

    runtime.active_tree = 1u;
    CHECK(dm2_v1_skproject_1031_0541_select_tree(
              &runtime, &state, 0u, roots, 2u, nodes, 4u,
              child_bytes, sizeof(child_bytes), leaves, 4u,
              clickrects, 4u, &select_receipt) &&
              select_receipt.valid &&
              select_receipt.requested_event_reset &&
              select_receipt.activated_leaves == 2u &&
              select_receipt.clickrect_refresh_2 == 2u &&
              (leaves[1].b6 & 0x80u) != 0u &&
              clickrects[0].refresh_link_2,
          "_1031_0541 selects a UI tree, traverses leaves, and refreshes activated clickrects");
    runtime.saved_tree = 0u;
    CHECK(dm2_v1_skproject_1031_0667_restore_active_tree(
              &runtime, &state, roots, 2u, nodes, 4u,
              child_bytes, sizeof(child_bytes), leaves, 4u,
              clickrects, 4u, &select_receipt) &&
              select_receipt.valid && runtime.active_tree == 0u,
          "_1031_0667 restores the saved UI tree through _1031_0541");
    runtime.show_item_stats = 1u;
    CHECK(dm2_v1_skproject_1031_0675_reset_and_select_tree(
              &runtime, &state, 0u, roots, 2u, nodes, 4u,
              child_bytes, sizeof(child_bytes), leaves, 4u,
              clickrects, 4u, &select_receipt) &&
              select_receipt.valid &&
              runtime.previous_tree == 0u &&
              runtime.saved_tree == 0u &&
              runtime.show_item_stats == 0u,
          "_1031_0675 saves current tree, releases UI captures, and selects the requested tree");
}

static void test_real_gdat_ibmio_palette_family(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    char path[1024];
    DM2_V1_AssetLoader loader;
    DM2_V1_InterfacePalette interface_palette;
    DM2_V1_SkprojectIbmioPaletteState palette_state;
    DM2_V1_SkprojectIbmioPaletteReceipt palette_receipt;
    DM2_V1_SkprojectIbmioBlit4To8Receipt blit_receipt;
    uint8_t rgb[16][3];
    uint8_t src4[8] = { 0x01u, 0x23u, 0x45u, 0x67u,
                        0x89u, 0xabu, 0xcdu, 0xefu };
    uint8_t dst8[16];

    if (!load_graphics(&graphics, &graphics_size, path, sizeof(path))) {
        printf("SKIP: real DM2 GRAPHICS.DAT not found for IBMIO palette scan\n");
        return;
    }
    memset(&loader, 0, sizeof(loader));
    CHECK(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0,
          "real DM2 GRAPHICS.DAT initializes for IBMIO palette family");
    if (!loader.loaded) {
        free(graphics);
        return;
    }
    CHECK(dm2_v1_asset_load_interface_palette(
              &loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
              DM2_GDAT_INTERFACE_PALETTE_FIELD, &interface_palette) == 1 &&
              interface_palette.hash != 0u,
          "real INTERFACE_GENERAL dtPalIRGB/dtPalette16 feeds IBMIO palette");

    for (uint8_t i = 0u; i < 16u; ++i) {
        uint8_t index = interface_palette.palette16[i];
        rgb[i][0] = (uint8_t)(interface_palette.rgb6[index][0] << 2);
        rgb[i][1] = (uint8_t)(interface_palette.rgb6[index][1] << 2);
        rgb[i][2] = (uint8_t)(interface_palette.rgb6[index][2] << 2);
    }
    memset(dst8, 0, sizeof(dst8));
    dm2_v1_skproject_ibmio_palette_state_init(&palette_state);
    CHECK(dm2_v1_skproject_00eb_04bc_palette_set(
              &palette_state, rgb, 0u, &palette_receipt) &&
              palette_receipt.valid && palette_receipt.palette_hash != 0u,
          "real GDAT palette bytes enter _00eb_04bc without fallback");
    CHECK(dm2_v1_skproject_00eb_070c_blit_4to8(
              src4, sizeof(src4), 0u, dst8, sizeof(dst8), 0u, 8u,
              interface_palette.palette16, &blit_receipt) &&
              blit_receipt.valid && blit_receipt.palette_hash != 0u &&
              blit_receipt.dest_hash != 0u,
          "real GDAT dtPalette16 drives _00eb_070c 4-to-8 row expansion");

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
}

static void test_util_helpers(void)
{
    DM2_V1_SkprojectRandomData randdat;
    DM2_V1_SkprojectVectorDirReceipt dir_receipt;
    DM2_V1_SkprojectFillI16TableReceipt fill_receipt;
    DM2_V1_SkprojectPtInRectReceipt pt_receipt;
    DM2_V1_SkprojectOffsetRectReceipt offset_receipt;
    DM2_V1_SkprojectPtrAdvanceReceipt advance_receipt;
    DM2_V1_SkprojectCursorAccessReceipt cursor_receipt;
    DM2_V1_SkprojectIsNegativeReceipt neg_receipt;
    DM2_V1_SkprojectContainerMapReceipt map_receipt;
    DM2_V1_SkprojectPossessionSlotReceipt pos_receipt;
    uint8_t cursor_bytes[8] = { 0 };
    uint8_t read_byte = 0u;
    int8_t read_sbyte = 0;
    uint16_t read_word = 0u;
    int16_t table[5] = { 1, 2, 3, 4, 5 };
    DM2_V1_SkprojectRect origin = { 10, 20, 100, 80 };
    DM2_V1_SkprojectRect source = { 13, 31, 7, 5 };
    DM2_V1_SkprojectRect out_rect;
    uint32_t offset = 0u;
    uint16_t inventory[30];

    CHECK(dm2_v1_skproject_abs(-12) == 12 &&
              dm2_v1_skproject_abs(7) == 7,
          "DM2_ABS returns source signed absolute value");
    CHECK(dm2_v1_skproject_calc_square_distance(7, -3, 2, 5) == 13,
          "DM2_CALC_SQUARE_DISTANCE sums absolute x/y deltas");

    CHECK(dm2_v1_skproject_calc_vector_dir(
              0, 10, 20, 4, 18, &dir_receipt) == 1 &&
              dir_receipt.valid && dir_receipt.dir == 3u &&
              !dir_receipt.consumed_randbit &&
              dir_receipt.delta_x == 6 && dir_receipt.delta_y == 2,
          "DM2_CALC_VECTOR_DIR chooses west/east axis without random on non-tie");
    CHECK(dm2_v1_skproject_calc_vector_dir(
              0, 4, 1, 4, 9, &dir_receipt) == 1 &&
              dir_receipt.valid && dir_receipt.dir == 2u,
          "DM2_CALC_VECTOR_DIR chooses north/south axis by y delta");

    dm2_v1_skproject_random_init(&randdat);
    CHECK(dm2_v1_skproject_calc_vector_dir(
              &randdat, 5, 5, 1, 1, &dir_receipt) == 1 &&
              dir_receipt.valid && dir_receipt.tied_axes &&
              dir_receipt.consumed_randbit && dir_receipt.randbit == 0u &&
              dir_receipt.abs_delta_x == 4 &&
              dir_receipt.abs_delta_y == 5 &&
              dir_receipt.dir == 0u,
          "DM2_CALC_VECTOR_DIR consumes DM2_RANDBIT to break diagonal ties");
    CHECK(dm2_v1_skproject_calc_vector_dir(
              0, 5, 5, 1, 1, &dir_receipt) == 0 &&
              dir_receipt.blocked_missing_random,
          "DM2_CALC_VECTOR_DIR fails closed on tied axes without random state");

    CHECK(dm2_v1_skproject_compute_power_4_within(0x2au, 1) == 0x02,
          "DM2_COMPUTE_POWER_4_WITHIN returns first set power");
    CHECK(dm2_v1_skproject_compute_power_4_within(0x2au, 3) == 0x20,
          "DM2_COMPUTE_POWER_4_WITHIN returns requested set-bit ordinal");
    CHECK(dm2_v1_skproject_compute_power_4_within(0x2au, 4) == 0,
          "DM2_COMPUTE_POWER_4_WITHIN returns shifted-out zero when ordinal is absent");

    CHECK(dm2_v1_skproject_fill_i16table(
              table, -9, 5u, &fill_receipt) == 1 &&
              fill_receipt.valid && fill_receipt.written_entries == 5u &&
              table[0] == -9 && table[4] == -9,
          "DM2_FILL_I16TABLE writes every source table entry");
    CHECK(dm2_v1_skproject_fill_i16table(
              0, 4, 0u, &fill_receipt) == 1 &&
              fill_receipt.valid && fill_receipt.written_entries == 0u,
          "DM2_FILL_I16TABLE accepts zero entries without a table");
    CHECK(dm2_v1_skproject_fill_i16table(
              0, 4, 1u, &fill_receipt) == 0 &&
              fill_receipt.blocked_missing_table,
          "DM2_FILL_I16TABLE rejects missing table when entries are required");

    CHECK(dm2_v1_skproject_atimesb_rshiftc(300, 3, 7) == 262,
          "DM2_ATIMESB_RSHIFTC multiplies unsigned 16-bit inputs before shift");
    CHECK(dm2_v1_skproject_atimesb_rshiftc(-2, 4, 2) == 8191,
          "DM2_ATIMESB_RSHIFTC preserves unsignedlong conversion of signed words");

    CHECK(dm2_v1_skproject_pt_in_rect(
              &source, 19, 35, &pt_receipt) == 1 &&
              pt_receipt.valid && pt_receipt.result == 1u,
          "PT_IN_RECT accepts the inclusive lower/right source rectangle edge");
    CHECK(dm2_v1_skproject_pt_in_rect(
              &source, 20, 35, &pt_receipt) == 0 &&
              pt_receipt.valid && pt_receipt.result == 0u,
          "PT_IN_RECT rejects x past x+w-1");
    CHECK(dm2_v1_skproject_pt_in_rect(
              0, 20, 35, &pt_receipt) == 0 &&
              pt_receipt.blocked_missing_rect,
          "PT_IN_RECT rejects a missing source rect");

    CHECK(dm2_v1_skproject_offset_rect(
              &origin, &source, &out_rect, &offset_receipt) == 1 &&
              offset_receipt.valid &&
              out_rect.x == 3 && out_rect.y == 11 &&
              out_rect.w == 7 && out_rect.h == 5,
          "OFFSET_RECT subtracts origin x/y and preserves source size");
    CHECK(dm2_v1_skproject_offset_rect(
              &origin, &source, 0, &offset_receipt) == 0 &&
              offset_receipt.blocked_missing_output,
          "OFFSET_RECT rejects missing output rect");

    CHECK(dm2_v1_skproject_ptr_advance(
              12u, 5, 32u, &offset, &advance_receipt) == 1 &&
              advance_receipt.valid && offset == 17u,
          "PTR_ADVANCE adds byte delta to the source cursor");
    CHECK(dm2_v1_skproject_ptr_advance(
              12u, -20, 32u, &offset, &advance_receipt) == 0 &&
              advance_receipt.blocked_out_of_bounds,
          "PTR_ADVANCE fails closed before buffer start");
    CHECK(dm2_v1_skproject_ptr_advance(
              12u, 21, 32u, &offset, &advance_receipt) == 0 &&
              advance_receipt.blocked_out_of_bounds,
          "PTR_ADVANCE fails closed past buffer capacity");

    CHECK(dm2_v1_skproject_write_byte(
              cursor_bytes, sizeof(cursor_bytes), 2u, 0xabu,
              &cursor_receipt) == 1 &&
              cursor_receipt.valid && cursor_receipt.width_bytes == 1u &&
              cursor_bytes[2] == 0xabu,
          "WRITE_BYTE writes one byte at the source cursor");
    CHECK(dm2_v1_skproject_write_word(
              cursor_bytes, sizeof(cursor_bytes), 4u, 0x1234u,
              &cursor_receipt) == 1 &&
              cursor_receipt.valid && cursor_receipt.width_bytes == 2u &&
              cursor_bytes[4] == 0x34u && cursor_bytes[5] == 0x12u,
          "WRITE_WORD writes little-endian 16-bit data at the source cursor");
    CHECK(dm2_v1_skproject_read_byte(
              cursor_bytes, sizeof(cursor_bytes), 2u, &read_byte,
              &cursor_receipt) == 1 &&
              cursor_receipt.valid && read_byte == 0xabu,
          "READ_BYTE reads one unsigned byte from the source cursor");
    cursor_bytes[6] = 0xf0u;
    CHECK(dm2_v1_skproject_read_sbyte(
              cursor_bytes, sizeof(cursor_bytes), 6u, &read_sbyte,
              &cursor_receipt) == 1 &&
              cursor_receipt.valid && read_sbyte == -16,
          "READ_SBYTE preserves signed 8-bit interpretation");
    CHECK(dm2_v1_skproject_read_word(
              cursor_bytes, sizeof(cursor_bytes), 4u, &read_word,
              &cursor_receipt) == 1 &&
              cursor_receipt.valid && read_word == 0x1234u,
          "READ_WORD reads little-endian 16-bit data at the source cursor");
    CHECK(dm2_v1_skproject_write_word(
              cursor_bytes, sizeof(cursor_bytes), 7u, 0x5678u,
              &cursor_receipt) == 0 &&
              cursor_receipt.blocked_out_of_bounds,
          "WRITE_WORD fails closed when the word crosses buffer end");
    CHECK(dm2_v1_skproject_read_byte(
              0, sizeof(cursor_bytes), 0u, &read_byte,
              &cursor_receipt) == 0 &&
              cursor_receipt.blocked_missing_buffer,
          "READ_BYTE rejects missing cursor storage");

    CHECK(dm2_v1_skproject_is_negative(-1, &neg_receipt) == 1 &&
              neg_receipt.valid && neg_receipt.value == -1 &&
              neg_receipt.result == 1u,
          "IS_NEGATIVE returns one for signed negative words");
    CHECK(dm2_v1_skproject_is_negative(0, &neg_receipt) == 0 &&
              neg_receipt.valid && neg_receipt.result == 0u,
          "IS_NEGATIVE returns zero for zero/non-negative words");

    CHECK(dm2_v1_skproject_is_container_map((uint16_t)((9u << 10) | 4u),
                                            1u, &map_receipt) == 1 &&
              map_receipt.valid && map_receipt.db_type == 9u &&
              map_receipt.container_type == 1u,
          "IS_CONTAINER_MAP accepts dbContainer object with ContainerType 1");
    CHECK(dm2_v1_skproject_is_container_map((uint16_t)((9u << 10) | 4u),
                                            2u, &map_receipt) == 0 &&
              map_receipt.valid && map_receipt.db_type == 9u,
          "IS_CONTAINER_MAP rejects other container types");
    CHECK(dm2_v1_skproject_is_container_map((uint16_t)((8u << 10) | 4u),
                                            1u, &map_receipt) == 0 &&
              map_receipt.blocked_non_container_db,
          "IS_CONTAINER_MAP rejects non-container DB type");

    for (int i = 0; i < 30; ++i)
        inventory[i] = DM2_V1_SKPROJECT_OBJECT_NULL;
    inventory[12] = 0x2244u;
    CHECK(dm2_v1_skproject_find_pouch_or_scabbard_possession_pos(
              inventory, 1u, &pos_receipt) == 12 &&
              pos_receipt.valid && pos_receipt.checked_scabbard1 &&
              pos_receipt.selected_object == 0x2244u,
          "FIND_POUCH_OR_SCABBARD_POSSESSION_POS prefers scabbard slot 12");
    inventory[12] = DM2_V1_SKPROJECT_OBJECT_NULL;
    inventory[8] = 0x2345u;
    CHECK(dm2_v1_skproject_find_pouch_or_scabbard_possession_pos(
              inventory, 1u, &pos_receipt) == 8 &&
              pos_receipt.valid && pos_receipt.checked_scabbard_tail,
          "FIND_POUCH_OR_SCABBARD_POSSESSION_POS scans scabbard slots 7..9");
    for (int i = 0; i < 30; ++i)
        inventory[i] = DM2_V1_SKPROJECT_OBJECT_NULL;
    inventory[6] = 0x3456u;
    CHECK(dm2_v1_skproject_find_pouch_or_scabbard_possession_pos(
              inventory, 0u, &pos_receipt) == 6 &&
              pos_receipt.valid && pos_receipt.checked_pouch1 &&
              pos_receipt.checked_pouch2,
          "FIND_POUCH_OR_SCABBARD_POSSESSION_POS checks pouch 11 then pouch 6");
    inventory[11] = 0x4567u;
    CHECK(dm2_v1_skproject_find_pouch_or_scabbard_possession_pos(
              inventory, 0u, &pos_receipt) == 11 &&
              pos_receipt.selected_object == 0x4567u,
          "FIND_POUCH_OR_SCABBARD_POSSESSION_POS prefers pouch slot 11");
    CHECK(dm2_v1_skproject_find_pouch_or_scabbard_possession_pos(
              0, 0u, &pos_receipt) == -1 &&
              pos_receipt.blocked_missing_inventory,
          "FIND_POUCH_OR_SCABBARD_POSSESSION_POS rejects missing inventory");
}

static void test_xrect_codec(void)
{
    DM2_V1_SkprojectRectTable table;
    DM2_V1_SkprojectCompressRectsReceipt compress;
    DM2_V1_SkprojectQueryRectReceipt query;
    DM2_V1_SkprojectRect rect;
    const int16_t common_xy_words[] = {
        (int16_t)0xfc0d, 1, 100, 101,
        20, 30, 12, 10,
        20, 30, 40, 22
    };
    const int16_t byte_xy_words[] = {
        (int16_t)0xfc0d, 1, 200, 201,
        1, 2, 30, 31,
        40, 50, 60, 70
    };
    const int16_t signed_size_words[] = {
        (int16_t)0xfc0d, 1, 300, 300,
        7, 8, -5, 6
    };
    const int16_t word_size_words[] = {
        (int16_t)0xfc0d, 1, 400, 400,
        40, 20, 260, 9
    };
    const int16_t two_groups_words[] = {
        (int16_t)0xfc0d, 2, 10, 10, 20, 21,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    };

    CHECK(dm2_v1_skproject_compressed_rect_row_size(0x1bu) == 2u &&
              dm2_v1_skproject_compressed_rect_row_size(0x04u) == 6u &&
              dm2_v1_skproject_compressed_rect_row_size(0x00u) == 8u,
          "DM2_CALC_SIZE_OF_COMPRESSED_RECT follows skproject mask rules");

    CHECK(dm2_v1_skproject_compress_rects(
              common_xy_words,
              (uint32_t)(sizeof(common_xy_words) / sizeof(common_xy_words[0])),
              &table, &compress) == 1 &&
              compress.valid && compress.node_count == 1u &&
              table.nodes[0].mask == 0x1bu,
          "DM2_COMPRESS_RECTS stores common x/y and signed byte dimensions");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 101u, &rect, &query) == 1 &&
              query.valid && rect.x == 20 && rect.y == 30 &&
              rect.w == 40 && rect.h == 22 && query.row_size == 2u,
          "DM2_QUERY_RECT expands a common x/y compressed row");

    CHECK(dm2_v1_skproject_compress_rects(
              byte_xy_words,
              (uint32_t)(sizeof(byte_xy_words) / sizeof(byte_xy_words[0])),
              &table, &compress) == 1 &&
              compress.valid && table.nodes[0].mask == 0x1cu,
          "DM2_COMPRESS_RECTS uses byte x/y when no common coordinate exists");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 201u, &rect, &query) == 1 &&
              rect.x == 40 && rect.y == 50 && rect.w == 60 && rect.h == 70,
          "DM2_QUERY_RECT expands byte-packed x/y/unsigned size rows");

    CHECK(dm2_v1_skproject_compress_rects(
              signed_size_words,
              (uint32_t)(sizeof(signed_size_words) / sizeof(signed_size_words[0])),
              &table, &compress) == 1 &&
              table.nodes[0].mask == 0x0bu,
          "DM2_COMPRESS_RECTS preserves signed-byte width/height route");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 300u, &rect, &query) == 1 &&
              rect.x == 7 && rect.y == 8 && rect.w == -5 && rect.h == 6,
          "DM2_QUERY_RECT sign-extends signed byte dimensions");

    CHECK(dm2_v1_skproject_compress_rects(
              word_size_words,
              (uint32_t)(sizeof(word_size_words) / sizeof(word_size_words[0])),
              &table, &compress) == 1 &&
              table.nodes[0].mask == 0x03u,
          "DM2_COMPRESS_RECTS falls back to word x and unsigned-byte size");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 400u, &rect, &query) == 1 &&
              rect.x == 40 && rect.y == 20 && rect.w == 260 && rect.h == 9,
          "DM2_QUERY_RECT expands word-sized fields when byte packing is unsafe");

    CHECK(dm2_v1_skproject_compress_rects(
              two_groups_words,
              (uint32_t)(sizeof(two_groups_words) / sizeof(two_groups_words[0])),
              &table, &compress) == 1 &&
              compress.valid && compress.node_count == 2u,
          "DM2_COMPRESS_RECTS appends multiple source ranges as rnodes");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 21u, &rect, &query) == 1 &&
              query.node_index == 1u && query.local_index == 1u &&
              rect.x == 13 && rect.y == 14 && rect.w == 15 && rect.h == 16,
          "DM2_QUERY_RECT walks later rnodes by requested rectangle number");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 0u, &rect, &query) == 0 &&
              query.blocked_zero_rect,
          "DM2_QUERY_RECT rejects source rect zero");
    CHECK(dm2_v1_skproject_query_rect(
              &table, 99u, &rect, &query) == 0 &&
              query.blocked_not_found,
          "DM2_QUERY_RECT fails closed when no rnode owns the rect");
    CHECK(dm2_v1_skproject_compress_rects(
              common_xy_words, 3u, &table, &compress) == 0 &&
              compress.blocked_group_overflow,
          "DM2_COMPRESS_RECTS rejects truncated raw4 data");
}

static void test_palette_helpers(void)
{
    DM2_V1_SkprojectDriverPaletteReceipt driver_receipt;
    DM2_V1_SkprojectPaletteSetReceipt set_receipt;
    DM2_V1_SkprojectXlatPaletteReceipt xlat_receipt;
    DM2_V1_SkprojectBlitSpecialEffectsReceipt blit;
    DM2_V1_SkprojectDrawIconPictBuffReceipt icon_blit;
    DM2_V1_SkprojectDrawDefPictReceipt def_pict;
    DM2_V1_SkprojectDrawGrayOverlayReceipt gray;
    DM2_V1_SkprojectExtendedPictureRef ext_pict;
    DM2_V1_SkprojectRect rect;
    uint8_t alpha_rgb[1024];
    uint8_t palette[4] = { 1u, 2u, 3u, 4u };
    uint8_t conv[256];
    uint8_t overlay[16];
    uint8_t byte = 0u;
    const uint8_t *selected_palette = 0;

    for (uint16_t i = 0; i < 256u; ++i) {
        alpha_rgb[i * 4u + 0u] = 0xaau;
        alpha_rgb[i * 4u + 1u] = (uint8_t)i;
        alpha_rgb[i * 4u + 2u] = (uint8_t)(255u - i);
        alpha_rgb[i * 4u + 3u] = (uint8_t)(i * 3u);
        conv[i] = (uint8_t)(255u - i);
    }

    CHECK(dm2_v1_skproject_palettecolor_from_color(42u, &byte) == 1 &&
              byte == 42u,
          "color_to_palettecolor is the skproject byte wrapper");
    CHECK(dm2_v1_skproject_palettecolor_from_ui8(43u, &byte) == 1 &&
              byte == 43u,
          "ui8_to_palettecolor preserves the palette byte");
    CHECK(dm2_v1_skproject_palettecolor_to_ui8(44u, &byte) == 1 &&
              byte == 44u,
          "palettecolor_to_ui8 preserves the palette byte");
    CHECK(dm2_v1_skproject_palettecolor_to_pixel(45u, &byte) == 1 &&
              byte == 45u,
          "palettecolor_to_pixel preserves the palette byte");

    CHECK(dm2_v1_skproject_convert_driverpalette(
              alpha_rgb, 1, &driver_receipt) == 1 &&
              driver_receipt.valid &&
              driver_receipt.converted_entries == 256u &&
              driver_receipt.driver_setcolors_requested &&
              driver_receipt.dmpal6[1][0] == (1u >> 2) &&
              driver_receipt.dmpal6[1][1] == (254u >> 2) &&
              driver_receipt.dmpal6[255][2] == ((uint8_t)(255u * 3u) >> 2) &&
              driver_receipt.dmpal_hash != 0u,
          "DM2_CONVERT_DRIVERPALETTE skips alpha and converts RGB8 to DMPAL RGB6");
    CHECK(dm2_v1_skproject_convert_driverpalette(
              0, 0, &driver_receipt) == 0 &&
              driver_receipt.blocked_missing_input,
          "DM2_CONVERT_DRIVERPALETTE fails closed without source palette");

    CHECK(dm2_v1_skproject_select_palette_set(0, &set_receipt) == 1 &&
              set_receipt.valid && set_receipt.fade_to_black_requested &&
              !set_receipt.immediate_colors_after &&
              set_receipt.vsync_waits == 2000u,
          "DM2_SELECT_PALETTE_SET(0) records source fade-to-black timing");
    CHECK(dm2_v1_skproject_select_palette_set(1, &set_receipt) == 1 &&
              set_receipt.valid && set_receipt.driver_setcolors_requested &&
              set_receipt.immediate_colors_after,
          "DM2_SELECT_PALETTE_SET(1) requests immediate driver colors");
    CHECK(dm2_v1_skproject_select_palette_set(2, &set_receipt) == 1 &&
              set_receipt.valid && !set_receipt.immediate_colors_after &&
              !set_receipt.driver_setcolors_requested,
          "DM2_SELECT_PALETTE_SET preserves source no-op behavior for other modes");

    CHECK(dm2_v1_skproject_update_blit_palette(
              palette, 4u, &selected_palette) == 1 &&
              selected_palette == palette,
          "DM2_UPDATE_BLIT_PALETTE assigns the caller palette pointer");
    CHECK(dm2_v1_skproject_update_blit_palette(
              palette, 4u, 0) == 0,
          "DM2_UPDATE_BLIT_PALETTE rejects missing output slot");

    CHECK(dm2_v1_skproject_xlat_palette(
              palette, 4u, conv, &xlat_receipt) == 1 &&
              xlat_receipt.valid && xlat_receipt.colors_after == 4u &&
              xlat_receipt.converted_colors == 4u &&
              xlat_receipt.palette[0] == 254u &&
              xlat_receipt.palette[3] == 251u &&
              !xlat_receipt.large_palette_copy,
          "DM2_xlat_palette applies the conversion table to explicit colors");
    CHECK(dm2_v1_skproject_xlat_palette(
              palette, 0u, conv, &xlat_receipt) == 1 &&
              xlat_receipt.valid && xlat_receipt.colors_after == 256u &&
              xlat_receipt.large_palette_copy &&
              xlat_receipt.palette[0] == 255u &&
              xlat_receipt.palette[255] == 0u,
          "DM2_xlat_palette copies the large palette when colors is zero");
    CHECK(dm2_v1_skproject_xlat_palette(
              0, 4u, conv, &xlat_receipt) == 0 &&
              xlat_receipt.blocked_missing_output,
          "DM2_xlat_palette rejects missing source palette for explicit colors");

    rect = (DM2_V1_SkprojectRect){ 2, 3, 5, 2 };
    CHECK(dm2_v1_skproject_sub_blit_specialeffects_receipt(
              &rect, 4u, 1u, 0u, 0u, 16u, -1, 0, &blit) == 1 &&
              blit.valid &&
              blit.palette_update_requested &&
              blit.used_plain_path &&
              !blit.used_alpha_blit &&
              blit.blit_runs == 4u &&
              blit.odd_width_source_advances == 2u &&
              blit.dest_start_offset == 50u &&
              blit.dest_final_offset == 82u,
          "DM2_sub_blit_specialeffects plain path splits source runs and advances odd-width rows");
    CHECK(dm2_v1_skproject_sub_blit_specialeffects_receipt(
              &rect, 4u, 1u, 0u, 0u, 16u, 7, 0, &blit) == 1 &&
              blit.used_plain_path &&
              blit.used_alpha_blit &&
              blit.alpha_mask == 7,
          "DM2_sub_blit_specialeffects records source alpha-mask blit variant");

    memset(overlay, 0, sizeof(overlay));
    overlay[1] = 1u;
    overlay[2] = 1u;
    overlay[3] = 1u;
    rect = (DM2_V1_SkprojectRect){ 0, 0, 5, 1 };
    CHECK(dm2_v1_skproject_sub_blit_specialeffects_receipt(
              &rect, 6u, 2u, 0u, 8u, 16u, -1, overlay, &blit) == 1 &&
              blit.valid &&
              blit.used_overlay_path &&
              blit.blit_runs == 1u &&
              blit.skipped_prefix_pixels == 1u &&
              blit.skipped_suffix_pixels == 1u,
          "DM2_sub_blit_specialeffects overlay path trims transparent prefix and suffix before blit");

    CHECK(dm2_v1_skproject_draw_icon_pict_buff(
              1, 4u, 5u, 6u, 7u, 1, 2, -1, 0, 16u, 12u, 320u,
              palette, &icon_blit) == 1 &&
              icon_blit.valid &&
              icon_blit.requested_offset_rect &&
              icon_blit.requested_fire_blit_picture &&
              icon_blit.requested_dirty_rect &&
              icon_blit.requested_local_palette &&
              icon_blit.bpp == 8u &&
              icon_blit.dest_stride == 320u,
          "DRAW_ICON_PICT_BUFF plans offset rect, 8bpp FIRE_BLIT_PICTURE, and dirty rect update");
    CHECK(dm2_v1_skproject_draw_icon_pict_buff(
              0, 4u, 5u, 6u, 7u, 1, 2, -1, 0, 16u, 12u, 320u,
              palette, &icon_blit) == 0 &&
              icon_blit.blocked_missing_rect,
          "DRAW_ICON_PICT_BUFF returns without drawing when rect is NULL");

    ext_pict = (DM2_V1_SkprojectExtendedPictureRef){ 0x20u, 0x30u, 0x40u };
    CHECK(dm2_v1_skproject_draw_def_pict(
              &ext_pict, 0xffffu, 20u, 30u, 3, 4, 5, 6, -1, 0,
              &def_pict) == 1 &&
              def_pict.valid &&
              def_pict.rect_no_used_direct_xy &&
              def_pict.requested_query_pict_bits &&
              !def_pict.requested_query_blit_rect &&
              def_pict.requested_blit,
          "DRAW_DEF_PICT rect 0xffff uses direct picture coordinates");
    CHECK(dm2_v1_skproject_draw_def_pict(
              &ext_pict, 0x002au, 20u, 30u, 3, 4, 5, 6, -1, 1,
              &def_pict) == 1 &&
              def_pict.rect_no_forced_blit_flag &&
              def_pict.requested_query_blit_rect &&
              def_pict.requested_blit,
          "DRAW_DEF_PICT normal rect forces the blit flag before QUERY_BLIT_RECT");
    CHECK(dm2_v1_skproject_draw_def_pict(
              &ext_pict, 0x002au, 20u, 30u, 3, 4, 5, 6, -1, 0,
              &def_pict) == 0 &&
              def_pict.blocked_missing_blit_rect,
          "DRAW_DEF_PICT fails closed when QUERY_BLIT_RECT has no destination");
    CHECK(dm2_v1_skproject_draw_def_pict(
              0, 0x002au, 20u, 30u, 3, 4, 5, 6, -1, 1,
              &def_pict) == 0 &&
              def_pict.blocked_missing_picture,
          "DRAW_DEF_PICT rejects missing picture metadata");

    CHECK(dm2_v1_skproject_draw_gray_overlay(
              1, 0x22u, 320u, 0xaaaa, &gray) == 1 &&
              gray.valid &&
              gray.requested_cache_buffer &&
              gray.requested_offset_rect &&
              gray.requested_gray_blit &&
              gray.requested_dirty_rect,
          "DRAW_GRAY_OVERLAY plans cache-buffer gray overlay and dirty rect update");
    CHECK(dm2_v1_skproject_draw_gray_overlay(
              0, 0x22u, 320u, 0xaaaa, &gray) == 0 &&
              gray.blocked_missing_rect,
          "DRAW_GRAY_OVERLAY returns without drawing when rect is NULL");
}

static void test_gui_receipt_helpers(void)
{
    DM2_V1_SkprojectDrawIconPictEntryReceipt icon_entry;
    DM2_V1_Skproject2405RectState rect_state;
    DM2_V1_Skproject2405RectReceipt rect_receipt;
    DM2_V1_Skproject2405ItemState item_state;
    DM2_V1_Skproject2405ItemEntryReceipt entry_receipt;
    DM2_V1_SkprojectDialogueProgressReceipt progress;
    DM2_V1_SkprojectDialoguePictReceipt pict;
    DM2_V1_SkprojectWakeUpTextReceipt wake;
    DM2_V1_SkprojectDrawPlayer3StatHealthBarReceipt health_bar;
    DM2_V1_SkprojectDrawPlayerNameAtCmdSlotReceipt name;
    DM2_V1_SkprojectDrawPlayerDamageReceipt damage;
    DM2_V1_SkprojectDrawSpellToBeCastReceipt spell_cast;
    DM2_V1_SkprojectDrawSpellPanelReceipt spell_panel;
    DM2_V1_SkprojectDrawSquadSpellLeaderReceipt squad_spell;
    DM2_V1_SkprojectDrawSquadPosInterfaceReceipt squad_pos;
    DM2_V1_SkprojectDrawItemIconReceipt item_icon;
    DM2_V1_SkprojectDrawItemInHandReceipt item_hand;
    DM2_V1_SkprojectDrawItemSurveyReceipt item_survey;
    DM2_V1_SkprojectDrawHandActionIconsReceipt hand_actions;
    DM2_V1_SkprojectDrawPowerStatBarReceipt power_bar;
    DM2_V1_SkprojectDrawScrollTextReceipt scroll_text;
    DM2_V1_SkprojectDrawSimpleStrReceipt simple_str;
    DM2_V1_SkprojectDrawSkillPanelReceipt skill_panel;
    uint8_t palette[16] = { 0 };
    const uint8_t champion_pos[4] = { 0u, 1u, 2u, 3u };
    const uint8_t champion_alive[4] = { 1u, 1u, 0u, 1u };
    const uint8_t champion_enchanted[4] = { 0u, 1u, 0u, 1u };
    DM2_V1_SkprojectRect blit_rects[4];
    DM2_V1_SkprojectRect out_rect;
    uint16_t selected_hand_items[4] = { 0u, 0u, 0x1402u, 0u };

    CHECK(dm2_v1_skproject_draw_icon_pict_entry(
              1u, 4u, 20u, 1, 0x3cu, -1, &icon_entry) == 1 &&
              icon_entry.valid && icon_entry.requested_image_entry &&
              icon_entry.requested_blit_rect &&
              icon_entry.requested_local_palette &&
              icon_entry.requested_icon_pict_buff,
          "DRAW_ICON_PICT_ENTRY plans image, blit rect, palette, and icon blit");
    CHECK(dm2_v1_skproject_draw_icon_pict_entry(
              1u, 4u, 20u, 0, 0x3cu, -1, &icon_entry) == 0 &&
              icon_entry.blocked_missing_button_group,
          "DRAW_ICON_PICT_ENTRY fails closed without a button group");

    blit_rects[2] = (DM2_V1_SkprojectRect){ 10, 20, 30, 40 };
    rect_state = (DM2_V1_Skproject2405RectState){ 3, -2, 4 };
    CHECK(dm2_v1_skproject_2405_00ec_query_blit_rect(
              &rect_state, 2u, blit_rects, 4u, &out_rect, &rect_receipt) &&
              rect_receipt.valid &&
              rect_receipt.requested_query_blit_rect &&
              out_rect.x == 13 && out_rect.y == 18 &&
              out_rect.w == 30 && out_rect.h == 40,
          "_2405_00ec resolves a real caller blit rect with source draw offsets");
    CHECK(dm2_v1_skproject_2405_011f_query_inflated_rect(
              &rect_state, 2u, blit_rects, 4u, &out_rect, &rect_receipt) &&
              rect_receipt.valid &&
              rect_receipt.requested_inflate_rect &&
              out_rect.x == 9 && out_rect.y == 14 &&
              out_rect.w == 38 && out_rect.h == 48,
          "_2405_011f delegates to _2405_00ec and inflates by the source margin");
    CHECK(!dm2_v1_skproject_2405_00ec_query_blit_rect(
              &rect_state, 6u, blit_rects, 4u, &out_rect, &rect_receipt) &&
              rect_receipt.blocked_rect_out_of_bounds,
          "_2405_00ec fails closed when the caller rect table has no rectno");

    memset(&item_state, 0, sizeof(item_state));
    item_state.object_id = 0x1402u;
    item_state.dbspec_word6 = 0x0505u;
    item_state.game_tick = 17u;
    CHECK(dm2_v1_skproject_2405_014a_item_entry(
              &item_state, 0u, 1u, &entry_receipt) == 0x1cu &&
              entry_receipt.valid &&
              entry_receipt.used_tick_mode &&
              entry_receipt.selected_entry == 0x1cu,
          "_2405_014a tick mode adds DBIndex to game tick before cls4 selection");
    item_state.dbspec_word6 = 0x0204u;
    item_state.player_dir = 3u;
    CHECK(dm2_v1_skproject_2405_014a_item_entry(
              &item_state, 0u, 1u, &entry_receipt) == 0x1bu &&
              entry_receipt.used_direction_mode,
          "_2405_014a direction mode uses the current player direction");
    item_state.dbspec_word6 = 0x4001u;
    item_state.champion_index = 1u;
    item_state.selected_hand_action = 0u;
    item_state.selected_hand_items = selected_hand_items;
    item_state.selected_hand_item_count = 4u;
    CHECK(dm2_v1_skproject_2405_014a_item_entry(
              &item_state, 0u, 1u, &entry_receipt) == 0x19u &&
              entry_receipt.valid &&
              entry_receipt.used_equip_variant &&
              entry_receipt.frame_count == 0u,
          "_2405_014a selected-hand DBSPEC flag advances to the equipped variant frame");
    item_state.selected_hand_items = NULL;
    CHECK(dm2_v1_skproject_2405_014a_item_entry(
              &item_state, 0u, 1u, &entry_receipt) == 0x18u &&
              entry_receipt.blocked_selected_hand,
          "_2405_014a selected-hand DBSPEC flag rejects non-held records");
    item_state.selected_hand_items = selected_hand_items;
    item_state.dbspec_word6 = 0x04afu;
    item_state.item_w2 = (uint16_t)(8u << 10);
    item_state.game_tick = 5u;
    CHECK(dm2_v1_skproject_2405_014a_item_entry(
              &item_state, 0u, 2u, &entry_receipt) == 0x1fu &&
              entry_receipt.valid &&
              entry_receipt.used_charge_tick_mode &&
              entry_receipt.charge == 8u &&
              entry_receipt.max_charge == 15u &&
              entry_receipt.bucket_width == 5u,
          "_2405_014a charge/tick mode combines item charge bucket and tick modulo");

    CHECK(dm2_v1_skproject_draw_dialogue_progress(
              1, 500u, 200u, 20u, &progress) == 1 &&
              progress.valid && progress.expanded_rect_no == 474u &&
              progress.computed_width == 100u &&
              progress.requested_fill_backbuff_rect &&
              progress.requested_dialogue_to_screen,
          "DRAW_DIALOGUE_PROGRESS scales rect 474 width and requests screen update");
    CHECK(dm2_v1_skproject_draw_dialogue_progress(
              0, 500u, 200u, 20u, &progress) == 0 &&
              progress.blocked_inactive,
          "DRAW_DIALOGUE_PROGRESS is gated by active dballoc dialogue state");

    CHECK(dm2_v1_skproject_draw_dialogue_pict(
              1, 1, 1, 64u, 128u, 1, 3, 4, 12, 4u, 8u,
              palette, &pict) == 1 &&
              pict.valid && pict.dest_width == 320u &&
              pict.requested_blit && pict.requested_palette,
          "DRAW_DIALOGUE_PICT uses ORIG_SWIDTH for screen destination and records blit");
    CHECK(dm2_v1_skproject_draw_dialogue_pict(
              1, 1, 0, 64u, 128u, 0, 0, 0, -1, 8u, 8u,
              0, &pict) == 0 &&
              pict.blocked_missing_rect,
          "DRAW_DIALOGUE_PICT fails closed without the caller rect");

    CHECK(dm2_v1_skproject_draw_wake_up_text(&wake) == 1 &&
              wake.valid && wake.gdat_text_category == 1u &&
              wake.gdat_text_entry == 0x11u &&
              wake.text_rect_no == 6u &&
              wake.requested_vp_rc_str,
          "DRAW_WAKE_UP_TEXT binds the real GDAT wake text route");

    CHECK(dm2_v1_skproject_draw_player_3stat_health_bar(
              2u, 1, &health_bar) == 1 &&
              health_bar.valid && health_bar.rect_no == 0xbbu &&
              health_bar.drew_health && health_bar.drew_stamina &&
              health_bar.drew_mana,
          "DRAW_PLAYER_3STAT_HEALTH_BAR plans the three stat bar draws");
    CHECK(dm2_v1_skproject_draw_player_name_at_cmdslot(
              2u, 1u, &name) == 1 &&
              name.valid && name.used_event_hero_color &&
              name.left_name_icon.button_id == 0x3cu &&
              name.right_name_icon.button_id == 0x3bu &&
              name.name_button_id == 0x3du,
          "DRAW_PLAYER_NAME_AT_CMDSLOT emits both frame icons and name text");
    CHECK(dm2_v1_skproject_draw_player_damage(
              1u, 47u, &damage) == 1 &&
              damage.valid && damage.damage_icon.button_id == 0xb2u &&
              strcmp(damage.damage_text, "047") == 0,
          "DRAW_PLAYER_DAMAGE binds damage icon and formatted text");

    CHECK(dm2_v1_skproject_draw_spell_to_be_cast(
              "FUL", 1, &spell_cast) == 1 &&
              spell_cast.valid && spell_cast.draw_frame_icon &&
              spell_cast.rune_count == 3u &&
              spell_cast.first_rune_button_id == 0x105u &&
              spell_cast.last_rune_button_id == 0x107u,
          "DRAW_SPELL_TO_BE_CAST draws current rune letters on source buttons");
    CHECK(dm2_v1_skproject_draw_spell_panel(
              2u, &spell_panel) == 1 &&
              spell_panel.valid &&
              spell_panel.panel_icon.entry == 3u &&
              spell_panel.drew_rune_choice_buttons &&
              spell_panel.requested_player_attack_dir,
          "DRAW_SPELL_PANEL plans rune choices, current spell, and attack dir");
    CHECK(dm2_v1_skproject_draw_squad_spell_and_leader_icon(
              1u, 1u, 2u, 0u, 100, 1u, 0u, 1u, &squad_spell) == 1 &&
              squad_spell.valid &&
              squad_spell.relative_pos == 2u &&
              squad_spell.mirror_flip &&
              squad_spell.leader_icon_entry == 13u &&
              squad_spell.spell_icon_entry == 9u &&
              squad_spell.leader_rect_no == 0x55u &&
              squad_spell.spell_rect_no == 0x59u &&
              squad_spell.requested_gray_overlay,
          "DRAW_SQUAD_SPELL_AND_LEADER_ICON maps skproject relative slots, icons, and overlays");
    CHECK(dm2_v1_skproject_draw_squad_spell_and_leader_icon(
              1u, 0u, 2u, 0u, 0, 0u, 0u, 0u, &squad_spell) == 0 &&
              squad_spell.blocked_dead_champion,
          "DRAW_SQUAD_SPELL_AND_LEADER_ICON skips dead champions");
    CHECK(dm2_v1_skproject_draw_squad_pos_interface(
              3u, champion_pos, champion_alive, champion_enchanted,
              4u, 1u, 2u, &squad_pos) == 1 &&
              squad_pos.valid &&
              squad_pos.base_icon.category == 8u &&
              squad_pos.base_icon.cls2 == 3u &&
              squad_pos.base_icon.entry == 0xf5u &&
              squad_pos.base_icon.button_id == 47u &&
              squad_pos.drawn_champions == 3u &&
              squad_pos.requested_alloc_pict_buff &&
              squad_pos.requested_squad_icon_palette &&
              squad_pos.requested_aura_summary_image &&
              squad_pos.requested_free_pict_buff,
          "DRAW_SQUAD_POS_INTERFACE draws alive non-selected positions from the source squad panel route");

    CHECK(dm2_v1_skproject_draw_item_icon(
              0x1401u, 1u, 2u, 7u, 0x2eu, 3u, 1, &item_icon) == 1 &&
              item_icon.valid && item_icon.requested_background_dialogue &&
              item_icon.requested_highlight_overlay &&
              item_icon.requested_icon_entry &&
              item_icon.item_icon_entry == 7u,
          "DRAW_ITEM_ICON plans background, highlight, and item GDAT icon");
    CHECK(dm2_v1_skproject_draw_item_in_hand(
              0x1401u, 1u, 2u, 7u, 32u, 24u, &item_hand) == 1 &&
              item_hand.valid && item_hand.requested_image_entry &&
              item_hand.requested_local_palette &&
              item_hand.requested_4bpp_blit,
          "DRAW_ITEM_IN_HAND binds cls1/cls2/cls4 to a real GDAT image route");
    CHECK(dm2_v1_skproject_draw_item_survey(
              0x1401u, 1u, &item_survey) == 1 &&
              item_survey.valid && item_survey.used_scroll_text &&
              item_survey.used_item_icon &&
              item_survey.item_icon_rect == 0x2eu,
          "DRAW_ITEM_SURVEY routes detail text and item icon");
    CHECK(dm2_v1_skproject_draw_hand_action_icons(
              2u, 0x1401u, 4u, 5u, 1u, &hand_actions) == 1 &&
              hand_actions.valid && hand_actions.action_button_id == 0x79u &&
              hand_actions.requested_dialogue_pict &&
              hand_actions.requested_icon_entry,
          "DRAW_HAND_ACTION_ICONS binds action icons to player hand buttons");
    CHECK(dm2_v1_skproject_draw_power_stat_bar(
              512, 496u, 6u, -1024, 0u, 1, &power_bar) == 1 &&
              power_bar.valid &&
              power_bar.scaled_value == 5000 &&
              power_bar.selected_color == 6u &&
              power_bar.requested_scale_rect &&
              power_bar.requested_black_background_fill &&
              power_bar.requested_value_fill,
          "DRAW_POWER_STAT_BAR scales the source 2048-floor range into a fill rect");
    CHECK(dm2_v1_skproject_draw_power_stat_bar(
              -100, 496u, 6u, -1024, 9u, 1, &power_bar) == 1 &&
              power_bar.selected_color == 11u &&
              power_bar.requested_tail_fill,
          "DRAW_POWER_STAT_BAR switches to warning colour below zero and emits tail fill");
    CHECK(dm2_v1_skproject_draw_power_stat_bar(
              0, 496u, 6u, -1024, 0u, 0, &power_bar) == 0 &&
              power_bar.blocked_missing_rect,
          "DRAW_POWER_STAT_BAR rejects missing rects");
    CHECK(dm2_v1_skproject_draw_scroll_text(
              0x3000u, 1, "A\nB\nC", &scroll_text) == 1 &&
              scroll_text.valid &&
              scroll_text.inventory_subpanel == 5u &&
              scroll_text.first_text_rect == 0x230u &&
              scroll_text.line_count == 3u &&
              scroll_text.requested_message_text &&
              scroll_text.requested_scroll_background &&
              scroll_text.requested_centered_lines,
          "DRAW_SCROLL_TEXT binds scroll objects to message text and centered scroll lines");
    CHECK(dm2_v1_skproject_draw_scroll_text(
              0x3000u, 0, "A", &scroll_text) == 0 &&
              scroll_text.blocked_not_scroll,
          "DRAW_SCROLL_TEXT rejects non-scroll objects");
    CHECK(dm2_v1_skproject_draw_simple_str(
              0x123u, 15u, 0x4000u, "80", 1, &simple_str) == 1 &&
              simple_str.valid &&
              simple_str.text_len == 2u &&
              simple_str.requested_query_str_metrics &&
              simple_str.requested_query_blit_rect &&
              simple_str.requested_draw_string &&
              simple_str.requested_dirty_rect,
          "DRAW_SIMPLE_STR queries metrics, blits the target rect, and draws text");
    CHECK(dm2_v1_skproject_draw_simple_str(
              0x123u, 15u, 0x4000u, "80", 0, &simple_str) == 0 &&
              simple_str.blocked_missing_rect,
          "DRAW_SIMPLE_STR rejects missing rects");
    CHECK(dm2_v1_skproject_draw_skill_panel(
              2u, 4u, 6u, &skill_panel) == 1 &&
              skill_panel.valid &&
              skill_panel.inventory_subpanel == 2u &&
              skill_panel.skill_text_rect == 557u &&
              skill_panel.attribute_text_rect == 559u &&
              skill_panel.requested_blank_panel &&
              skill_panel.requested_skill_names &&
              skill_panel.requested_attribute_names &&
              skill_panel.requested_ability_values,
          "DRAW_SKILL_PANEL binds the charsheet skill and attribute text routes");
}

static void test_move_admission_helpers(void)
{
    DM2_V1_SkprojectMoveSideOffsetReceipt side;
    DM2_V1_SkprojectMoveAdmissionRequest request;
    DM2_V1_SkprojectMoveAdmissionReceipt receipt;

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;

    CHECK(dm2_v1_skproject_move_side_offset(
              0x0100u, 0, 7u, &side) == 0 &&
              side.valid && side.facing == 1u &&
              side.relative_direction == 1u &&
              side.side_direction &&
              side.creature_x_offset == 0 &&
              !side.side_offset_nonzero,
          "DM2_12b4_0953 admits side direction and returns zero at 5x5 center");
    CHECK(dm2_v1_skproject_move_side_offset(
              0x0100u, 0, 5u, &side) == 1 &&
              side.valid && side.creature_x_offset == -2 &&
              side.side_offset_nonzero,
          "DM2_12b4_0953 returns nonzero for side creature offset");
    CHECK(dm2_v1_skproject_move_side_offset(
              0x0000u, 0, 5u, &side) == 0 &&
              side.valid && side.relative_direction == 0u &&
              !side.side_direction,
          "DM2_12b4_0953 returns zero when relative direction is not side");

    request.requested_move = 2u;
    request.current_tile_value = (uint16_t)(3u << 5);
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 1 &&
              receipt.valid && receipt.result_code == 1u &&
              receipt.current_tile_type == 3u,
          "DM2_12b4_0881 code 1 fires for requested move 2 on tile type 3");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;
    request.destination_tile_value = (uint16_t)(3u << 5);
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 2 &&
              receipt.result_code == 2u &&
              receipt.destination_tile_type == 3u,
          "DM2_12b4_0881 code 2 fires for destination tile type 3");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;
    request.destination_tile_blocked = 1;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 3 &&
              receipt.result_code == 3u,
          "DM2_12b4_0881 code 3 fires for blocked destination tile");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = 0x1234;
    request.creature_ai_flags = 0u;
    request.side_offset_nonzero = 0;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 4 &&
              receipt.result_code == 4u &&
              receipt.stored_creature == 0x1234 &&
              receipt.used_side_offset_test,
          "DM2_12b4_0881 code 4 fires for creature blocked by side-offset test");

    request.side_offset_nonzero = 1;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 5 &&
              receipt.result_code == 5u &&
              receipt.used_side_offset_test,
          "DM2_12b4_0881 code 5 fires when side-offset test admits creature");

    memset(&request, 0, sizeof(request));
    request.creature_at_destination = -1;
    request.secondary_query_creature = -1;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 6 &&
              receipt.result_code == 6u &&
              receipt.used_secondary_query,
          "DM2_12b4_0881 code 6 fires when secondary query has no creature");

    request.secondary_query_creature = 0x22;
    request.secondary_query_ai_flags = 0u;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 5 &&
              receipt.result_code == 5u &&
              receipt.used_secondary_query,
          "DM2_12b4_0881 secondary creature without 0x8000 flag returns 5");
    request.secondary_query_ai_flags = 0x8000u;
    CHECK(dm2_v1_skproject_move_admission(&request, &receipt) == 6 &&
              receipt.result_code == 6u,
          "DM2_12b4_0881 secondary creature with 0x8000 flag returns 6");
}

static void test_map_helpers(void)
{
    DM2_V1_SkprojectMinionDestinationReceipt dest;
    DM2_V1_SkprojectMapRandomReceipt random;
    DM2_V1_SkprojectMapTileVectorReceipt tile;
    DM2_V1_SkprojectGetTileValueReceipt tile_value;
    DM2_V1_SkprojectMap3B001Receipt map3b001;
    DM2_V1_SkprojectFillReceipt fill;
    DM2_V1_SkprojectFillStrReceipt fill_str;
    DM2_V1_SkprojectHalftoneRectReceipt half;
    DM2_V1_SkprojectMouseReleaseCaptureReceipt mouse_release;
    DM2_V1_SkprojectHighlightArrowPanelReceipt highlight;
    DM2_V1_SkprojectMap0CEEWallDecorationState wall_state;
    DM2_V1_SkprojectMap0CEEWallDecorationReceipt wall_receipt;
    DM2_V1_SkprojectRect half_rect;
    uint8_t tiles[16];
    uint8_t fill_buf[12];
    uint8_t pixels[64];
    uint16_t wall_words[4];
    int16_t capture_count = 3;
    uint8_t passage[16] = {
        1u, 0u, 1u, 1u,
        0u, 0u, 1u, 0u,
        1u, 1u, 0u, 1u,
        0u, 1u, 1u, 1u
    };

    for (uint8_t i = 0; i < 16u; ++i)
        tiles[i] = (uint8_t)(0x80u + i);

    CHECK(dm2_v1_skproject_set_destination_of_minion_map(
              0xffffu, 4, 17, 9, 12, 32, 32, &dest) == 1 &&
              dest.valid && dest.in_bounds && dest.previous_map == 4 &&
              dest.new_record_w6 ==
                  (uint16_t)((12u << 10) | (9u << 5) | 17u),
          "DM2_SET_DESTINATION_OF_MINION_MAP packs x/y/map into record word 6");
    CHECK(dm2_v1_skproject_set_destination_of_minion_map(
              0x1234u, 4, 32, 9, 12, 32, 32, &dest) == 0 &&
              dest.valid && !dest.in_bounds &&
              dest.new_record_w6 == 0x1234u,
          "DM2_SET_DESTINATION_OF_MINION_MAP rejects out-of-bounds destination");

    CHECK(dm2_v1_skproject_map_0cee_17e7(
              0x07d3u, 0x0123u, 30u, 0x4567u, &random) ==
              (int)(((((uint32_t)0x07d3u * 0x7ab9u) / 2u) +
                     11u * 0x0123u + 0x4567u) >> 2) % 30 &&
              random.valid && random.result < 30u,
          "DM2_map_0cee_17e7 preserves source mixed modulo formula");
    CHECK(dm2_v1_skproject_map_0cee_17e7(
              1u, 2u, 0u, 3u, &random) == 0 &&
              !random.valid,
          "DM2_map_0cee_17e7 rejects zero modulo range");

    CHECK(dm2_v1_skproject_map_0cee_04e5(
              tiles, 4, 4, 0, 1, 1, 1, 2, &tile) == 0x86 &&
              tile.valid && tile.tile_x == 2 && tile.tile_y == 1 &&
              tile.tile_value == 0x86u,
          "DM2_map_0cee_04e5 applies CALC_VECTOR_W_DIR before tile lookup");
    CHECK(dm2_v1_skproject_map_0cee_04e5(
              tiles, 4, 4, 3, 3, 0, 0, 0, &tile) == 0 &&
              tile.blocked_out_of_bounds,
          "DM2_map_0cee_04e5 fails closed outside the caller tile map");
    CHECK(dm2_v1_skproject_map_0cee_04e5(
              0, 4, 4, 0, 0, 0, 0, 0, &tile) == 0 &&
              tile.blocked_missing_tiles,
          "DM2_map_0cee_04e5 rejects missing tile storage");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, 2, 1, &tile_value) == 0x86 &&
              tile_value.valid &&
              tile_value.in_bounds &&
              tile_value.returned_tile_value == 0x86u,
          "GET_TILE_VALUE returns the current map byte for in-bounds tiles");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, -1, 0, &tile_value) == 4 &&
              tile_value.valid &&
              tile_value.used_left_boundary &&
              tile_value.checked_primary_passage,
          "GET_TILE_VALUE returns left-boundary mask when adjacent tile is passage");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, 4, 2, &tile_value) == 1 &&
              tile_value.valid &&
              tile_value.used_right_boundary,
          "GET_TILE_VALUE returns right-boundary mask when adjacent tile is passage");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, 1, -1, &tile_value) == 0 &&
              tile_value.valid &&
              tile_value.used_top_boundary &&
              tile_value.checked_side_passage,
          "GET_TILE_VALUE returns zero when top boundary is blocked but side passage exists");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, 3, 4, &tile_value) == 8 &&
              tile_value.valid &&
              tile_value.used_bottom_boundary,
          "GET_TILE_VALUE returns bottom-boundary mask when adjacent tile is passage");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, -1, -1, &tile_value) == 0 &&
              tile_value.valid &&
              tile_value.used_corner_boundary,
          "GET_TILE_VALUE preserves source corner-boundary zero when corner-adjacent passage exists");
    CHECK(dm2_v1_skproject_core_get_tile_value(
              tiles, passage, 4, 4, -2, 1, &tile_value) == 0xe0 &&
              tile_value.valid &&
              tile_value.returned_blocked_value == 0xe0u,
          "GET_TILE_VALUE returns 0xE0 for non-adjacent out-of-bounds probes");

    CHECK(dm2_v1_skproject_map_3b001(
              7, 11, 12, &map3b001) == 1 &&
              map3b001.valid && map3b001.new_v1e0270 == 11 &&
              map3b001.new_v1e0272 == 12 &&
              map3b001.requested_change_to_previous_map,
          "DM2_map_3B001 stores v1e0270/v1e0272 and requests map restore");
    CHECK(dm2_v1_skproject_map_3b001(
              -1, 1, 2, &map3b001) == 1 &&
              !map3b001.requested_change_to_previous_map,
          "DM2_map_3B001 preserves no-op restore when previous map is -1");

    {
        uint8_t candidates[30];
        uint8_t alcoves[256] = { 0 };
        uint16_t words[4] = { 0 };
        uint8_t tmpmap[64] = { 0 };
        DM2_V1_SkprojectMap1815Receipt map1815;
        DM2_V1_SkprojectMap185AReceipt map185a;
        DM2_V1_SkprojectTmpmapFlagReceipt tmpflag;

        for (uint8_t i = 0; i < 30u; ++i)
            candidates[i] = (uint8_t)(0x40u + i);
        CHECK(dm2_v1_skproject_map_0cee_1815(
                  1, 12, 8, 3, 4, 5, 30, 0x2222u,
                  candidates, 30u, &map1815) ==
                  candidates[map1815.selected_index] &&
                  map1815.valid && map1815.selected_index < 30u &&
                  map1815.mixed_random != 0u,
              "DM2_map_0cee_1815 selects v1e02cc entry through skproject random mix");
        CHECK(dm2_v1_skproject_map_0cee_1815(
                  0, 12, 8, 3, 4, 5, 30, 0x2222u,
                  candidates, 30u, &map1815) == -1 &&
                  map1815.blocked_zero_gate,
              "DM2_map_0cee_1815 returns -1 when source gate is zero");

        alcoves[0xffu] = 1u;
        candidates[0] = 0xffu;
        CHECK(dm2_v1_skproject_map_0cee_185a(
                  words, 12, 8, 3, 1, 1, 1, 1, 20, -1, 0, 0,
                  0u, candidates, 1u, alcoves, &map185a) == 1 &&
                  map185a.valid && words[0] == 0x00ffu &&
                  words[1] == 0x00ffu && map185a.sanitized[0] &&
                  map185a.sanitized[3],
              "DM2_map_0cee_185a fills four words and sanitizes ornate alcoves outside map bounds");
        CHECK(dm2_v1_skproject_map_0cee_185a(
                  0, 12, 8, 3, 1, 1, 1, 1, 20, -1, 0, 0,
                  0u, candidates, 1u, alcoves, &map185a) == 0 &&
                  map185a.blocked_missing_output,
              "DM2_map_0cee_185a rejects missing output words");

        memset(&wall_state, 0, sizeof(wall_state));
        wall_state.map_width = 12;
        wall_state.map_height = 8;
        wall_state.current_map_index = 3;
        wall_state.x = 4;
        wall_state.y = 5;
        wall_state.dungeon_seed = 0x2222u;
        wall_state.wall_random_decoration_count = 30u;
        wall_state.gates[0] = 1u;
        wall_state.gates[1] = 1u;
        wall_state.gates[2] = 0u;
        wall_state.gates[3] = 1u;
        wall_state.rotation = 2u;
        wall_state.candidate_table = candidates;
        wall_state.candidate_table_count = 30u;
        wall_state.ornate_alcove_flags = alcoves;
        CHECK(dm2_v1_skproject_map_0cee_wall_decoration_chain(
                  &wall_state, wall_words, &wall_receipt) == 1 &&
                  wall_receipt.valid &&
                  wall_receipt.requested_wall_random_decoration_count &&
                  wall_receipt.requested_random_17e7[0] &&
                  wall_receipt.requested_random_17e7[1] &&
                  !wall_receipt.requested_random_17e7[2] &&
                  wall_receipt.requested_random_17e7[3] &&
                  wall_receipt.divisor == (uint16_t)((3u << 6) + 12u + 8u + 3000u) &&
                  wall_receipt.step_plus_one == 6u &&
                  wall_receipt.random_input[0] == (uint16_t)(32u * 4u + 18u + 2000u) &&
                  wall_words[0] == candidates[wall_receipt.selected_index[0]] &&
                  wall_words[1] == candidates[wall_receipt.selected_index[1]] &&
                  wall_words[2] == 0x00ffu &&
                  wall_words[3] == candidates[wall_receipt.selected_index[3]] &&
                  wall_receipt.returned_default[2],
              "SKWIN _0cee wall decoration chain binds map seed, gates, rotation and v1e02cc candidates");

        for (uint8_t i = 0; i < 30u; ++i)
            candidates[i] = 0x42u;
        alcoves[0x42u] = 1u;
        wall_state.x = 20;
        wall_state.y = 2;
        wall_state.gates[2] = 1u;
        CHECK(dm2_v1_skproject_map_0cee_wall_decoration_chain(
                  &wall_state, wall_words, &wall_receipt) == 1 &&
                  wall_receipt.valid &&
                  wall_receipt.out_of_map &&
                  wall_receipt.requested_wall_ornate_alcove_type &&
                  wall_words[0] == 0x00ffu &&
                  wall_words[1] == 0x00ffu &&
                  wall_words[2] == 0x00ffu &&
                  wall_words[3] == 0x00ffu &&
                  wall_receipt.sanitized[0] &&
                  wall_receipt.sanitized[3],
              "SKWIN _0cee wall decoration chain sanitizes ornate alcoves outside the map");

        CHECK(dm2_v1_skproject_tmpmap_or_flag(
                  tmpmap, sizeof(tmpmap), 2, 3, 1, &tmpflag) == 1 &&
                  tmpflag.valid && tmpflag.offset == 21u &&
                  tmpmap[21] == 2u,
              "SKW_2066_1ea3 ORs bit 1 at the tmpmap pointer-derived byte");
        tmpmap[21] = 5u;
        CHECK(dm2_v1_skproject_tmpmap_or_flag(
                  tmpmap, sizeof(tmpmap), 2, 3, 1, &tmpflag) == 1 &&
                  tmpflag.previous_value == 5u && tmpflag.new_value == 7u,
              "SKW_2066_1ea3 preserves existing tmpmap flags");
        CHECK(dm2_v1_skproject_tmpmap_or_flag(
                  tmpmap, sizeof(tmpmap), 9, 9, 9, &tmpflag) == 0 &&
                  tmpflag.blocked_out_of_bounds,
              "SKW_2066_1ea3 rejects out-of-range tmpmap writes");
    }

    {
        DM2_V1_SkprojectMapRecord records[8];
        DM2_V1_SkprojectMap20661F37Receipt f37;
        DM2_V1_SkprojectMap20661EC9Receipt ec9;
        DM2_V1_SkprojectRecordAddressReceipt addr;
        uint16_t record_counts[16];
        uint16_t record_sizes[16];
        int16_t counter = 0;

        for (uint8_t i = 0; i < 16u; ++i) {
            record_counts[i] = (uint16_t)(8u + i);
            record_sizes[i] = (uint16_t)(2u + i);
        }

        CHECK(dm2_v1_skproject_get_address_of_record(
                  (uint16_t)((5u << 10) | 3u), record_counts,
                  record_sizes, &addr) == 1 &&
                  addr.valid &&
                  addr.db_type == 5u &&
                  addr.db_index == 3u &&
                  addr.record_count == 13u &&
                  addr.record_size == 7u &&
                  addr.byte_offset == 21u,
              "GET_ADDRESS_OF_RECORD applies skproject 4-bit table and 10-bit index addressing");
        CHECK(dm2_v1_skproject_get_address_of_record(
                  (uint16_t)((2u << 10) | 30u), record_counts,
                  record_sizes, &addr) == 0 &&
                  addr.blocked_index_out_of_range,
              "GET_ADDRESS_OF_RECORD rejects indexes outside the source record count");
        CHECK(dm2_v1_skproject_get_address_of_record(
                  DM2_V1_SKPROJECT_MAP_RECORD_END, record_counts,
                  record_sizes, &addr) == 0 &&
                  addr.blocked_end_marker,
              "GET_ADDRESS_OF_RECORD rejects OBJECT_END_MARKER");
        CHECK(dm2_v1_skproject_get_typed_address_of_record(
                  (uint16_t)((3u << 10) | 2u), 3u, record_counts,
                  record_sizes, 0, &addr) == 1 &&
                  addr.valid &&
                  addr.typed_accessor &&
                  addr.actuator_accessor &&
                  addr.requested_type == 3u,
              "GET_ADDRESS_OF_ACTU/RECORD3 require actuator DB type 3");
        CHECK(dm2_v1_skproject_get_typed_address_of_record(
                  (uint16_t)((4u << 10) | 2u), 0x04u, record_counts,
                  record_sizes, 1, &addr) == 1 &&
                  addr.valid &&
                  addr.used_detached_record_route,
              "GET_ADDRESS_OF_RECORDX4 uses the detached record route for creatures");
        CHECK(dm2_v1_skproject_get_typed_address_of_record(
                  0xff80u, 0x04u, record_counts, record_sizes, 1,
                  &addr) == 0 &&
                  addr.blocked_effect_record,
              "GET_ADDRESS_OF_DETACHED_RECORD rejects effect records at or above OBJECT_EFFECT_FIREBALL");
        CHECK(dm2_v1_skproject_get_typed_address_of_record(
                  (uint16_t)((9u << 10) | 1u), 0x10u, record_counts,
                  record_sizes, 0, &addr) == 1 &&
                  addr.generic_container_accessor &&
                  addr.db_type == 9u,
              "GET_ADDRESS_OF_GENERIC_CONTAINER_RECORD accepts concrete item container pools");
        CHECK(dm2_v1_skproject_get_typed_address_of_record(
                  (uint16_t)((2u << 10) | 1u), 0x10u, record_counts,
                  record_sizes, 0, &addr) == 0 &&
                  addr.blocked_type_mismatch,
              "GET_ADDRESS_OF_GENERIC_CONTAINER_RECORD rejects non-container DB pools");
        CHECK(dm2_v1_skproject_get_typed_address_of_record(
                  (uint16_t)((0x0bu << 10) | 1u), 0x0bu, record_counts,
                  record_sizes, 0, &addr) == 1 &&
                  addr.null_accessor,
              "GET_ADDRESS_OF_RECORDB/C/D preserve skproject null-accessor slots");

        memset(records, 0, sizeof(records));
        records[0].next = 1u;
        records[1].next = 2u;
        records[1].record_type = 3u;
        records[1].w2 = 0x0027u;
        records[2].next = DM2_V1_SKPROJECT_MAP_RECORD_END;
        records[2].record_type = 3u;
        records[2].w2 = 0x00a7u;
        CHECK(dm2_v1_skproject_map_2066_1f37(
                  records, 8u, 0u, 4u, &counter, &f37) == 1 &&
                  f37.valid && f37.scanned_records == 2u &&
                  f37.matched_records == 2u &&
                  f37.updated_records == 1u && counter == 1 &&
                  records[1].w2 == (uint16_t)(0x0027u | (5u << 7)) &&
                  records[2].w2 == 0x00a7u,
              "DM2_map_2066_1f37 scans linked records and only updates unset type-0x27 records");

        memset(records, 0, sizeof(records));
        records[1].next = 2u;
        records[1].record_type = 2u;
        records[2].next = 6u;
        records[2].record_type = 3u;
        records[6].next = DM2_V1_SKPROJECT_MAP_RECORD_END;
        records[6].record_type = 5u;
        CHECK(dm2_v1_skproject_map_2066_1ec9(
                  records, 8u, 5u, 1u, &ec9) == 2u &&
                  ec9.valid && ec9.rewired_records == 2u &&
                  ec9.appended_tail == 6u &&
                  records[2].next == 6u &&
                  records[1].next == 5u,
              "DM2_map_2066_1ec9 prepends low-type record chain before existing head");
        CHECK(dm2_v1_skproject_map_2066_1ec9(
                  records, 8u, DM2_V1_SKPROJECT_MAP_RECORD_END, 3u,
                  &ec9) == 3u &&
                  ec9.valid && ec9.returned_head == 3u,
              "DM2_map_2066_1ec9 returns append chain when existing head is end marker");

        CHECK(dm2_v1_skproject_core_get_address_of_tile_record(
                  2, 3, (uint16_t)((5u << 10) | 3u), record_counts,
                  record_sizes, &addr) == 1 &&
                  addr.valid &&
                  addr.db_type == 5u &&
                  addr.db_index == 3u &&
                  addr.byte_offset == 21u,
              "GET_ADDRESS_OF_TILE_RECORD dereferences the tile link through GET_ADDRESS_OF_RECORD");
    }

    CHECK(dm2_v1_skproject_fill_entire_pict(
              7u, 5u, 4u, 0x22u, &fill) == 1 &&
              fill.valid &&
              fill.aligned_width == 8u &&
              fill.pixel_count == 40u &&
              fill.requested_fill_rect_any,
          "FILL_ENTIRE_PICT aligns 4bpp picture width before FIRE_FILL_RECT_ANY");
    CHECK(dm2_v1_skproject_fill_rect_summary(
              9u, 3u, 0x44u, 1, 1, &fill) == 1 &&
              fill.valid &&
              fill.pixel_count == 27u &&
              fill.requested_offset_rect &&
              fill.requested_fill_rect_any &&
              fill.requested_dirty_rect,
          "FILL_RECT_SUMMARY offsets the rect, fills it, and marks it dirty");
    CHECK(dm2_v1_skproject_fill_rect_summary(
              9u, 3u, 0x44u, 1, 0, &fill) == 0 &&
              fill.blocked_missing_rect,
          "FILL_RECT_SUMMARY skips missing rects like the skproject null guard");
    memset(fill_buf, 0, sizeof(fill_buf));
    CHECK(dm2_v1_skproject_fill_str(
              fill_buf, sizeof(fill_buf), 4u, 0x7eu, 3u, &fill_str) == 1 &&
              fill_str.valid &&
              fill_str.written_entries == 4u &&
              fill_str.last_offset == 9u &&
              fill_buf[0] == 0x7eu &&
              fill_buf[3] == 0x7eu &&
              fill_buf[6] == 0x7eu &&
              fill_buf[9] == 0x7eu,
          "FILL_STR writes count entries at source delta offsets");
    CHECK(dm2_v1_skproject_fill_str(
              fill_buf, sizeof(fill_buf), 5u, 0x7eu, 3u, &fill_str) == 0 &&
              fill_str.blocked_missing_buffer,
          "FILL_STR fails closed when source delta walk exceeds buffer");
    memset(pixels, 0x55, sizeof(pixels));
    half_rect = (DM2_V1_SkprojectRect){ 1, 1, 4, 3 };
    CHECK(dm2_v1_skproject_fill_halftone_rectv(
              pixels, sizeof(pixels), 8u, &half_rect, &half) == 1 &&
              half.valid &&
              half.visited_pixels == 12u &&
              half.cleared_pixels == 6u &&
              pixels[1u * 8u + 1u] == 0u &&
              pixels[1u * 8u + 2u] == 0x55u &&
              pixels[3u * 8u + 4u] == 0x55u,
          "IBMIO_FILL_HALFTONE_RECT clears checkerboard pixels over full rect");
    memset(pixels, 0x55, sizeof(pixels));
    CHECK(dm2_v1_skproject_fill_halftone_recti(
              pixels, sizeof(pixels), 8u, 42u, &half_rect, &half) == 1 &&
              half.valid &&
              half.used_query_expanded_rect &&
              half.rectno == 42u,
          "FIRE_FILL_HALFTONE_RECTI queries expanded rect then delegates to RECTV");
    CHECK(dm2_v1_skproject_mouse_release_capture(
              &capture_count, &mouse_release) == 1 &&
              mouse_release.valid &&
              mouse_release.previous_capture_count == 3 &&
              mouse_release.new_capture_count == 2 &&
              mouse_release.requested_driver_command == 2u,
          "FIRE_MOUSE_RELEASE_CAPTURE delegates to IBMIO release/capture command");
    CHECK(dm2_v1_skproject_highlight_arrow_panel(
              5u, 77u, 1u, &highlight) == 1 &&
              highlight.valid &&
              highlight.cls4_input == 5u &&
              highlight.cls4_drawn == 6u &&
              highlight.rectno == 77u &&
              highlight.requested_hide_mouse &&
              highlight.requested_fill_entire_pict &&
              highlight.requested_draw_icon_entry &&
              highlight.requested_wait_refresh,
          "HIGHLIGHT_ARROW_PANEL records source bright icon redraw sequence");

    {
        DM2_V1_SkprojectMapDescriptor maps[4];
        uint8_t cursor[4] = { 2u, 1u, 3u, 0xffu };
        DM2_V1_SkprojectLocateOtherLevelReceipt locate;
        DM2_V1_SkprojectMap3BF83Receipt map3bf83;
        uint16_t resume = 0u;
        int16_t x = 4;
        int16_t y = 2;

        memset(maps, 0, sizeof(maps));
        maps[0].map_id = 0u;
        maps[0].world_x = 20;
        maps[0].world_y = 40;
        maps[0].width = 10;
        maps[0].height = 8;
        maps[1].map_id = 1u;
        maps[1].world_x = 23;
        maps[1].world_y = 41;
        maps[1].width = 7;
        maps[1].height = 6;
        maps[1].tile_type_at_local = 5u;
        maps[1].teleporter_record_active = 1u;
        maps[2].map_id = 2u;
        maps[2].world_x = 24;
        maps[2].world_y = 42;
        maps[2].width = 8;
        maps[2].height = 7;
        maps[2].tile_type_at_local = 7u;
        maps[3].map_id = 3u;
        maps[3].world_x = 22;
        maps[3].world_y = 39;
        maps[3].width = 8;
        maps[3].height = 9;
        maps[3].tile_type_at_local = 1u;

        CHECK(dm2_v1_skproject_locate_other_level(
                  maps, 4u, 0, 1, &x, &y, cursor, 4u, 0u, &resume,
                  &locate) == 3 &&
                  locate.valid && locate.found &&
                  locate.scanned_candidates == 3u &&
                  locate.rejected_teleporter &&
                  locate.selected_x == 2 && locate.selected_y == 3 &&
                  x == 2 && y == 3 && resume == 3u,
              "DM_LOCATE_OTHER_LEVEL scans cursor maps, rejects wall/active teleporter, and returns local coordinates");
        CHECK(dm2_v1_skproject_locate_other_level(
                  maps, 4u, 0, 1, &x, &y, cursor, 4u, resume, &resume,
                  &locate) == -1 &&
                  locate.valid && !locate.found &&
                  locate.used_resume_cursor && resume == 0u,
              "DM_LOCATE_OTHER_LEVEL resumes after caller cursor and fails closed at terminator");
        CHECK(dm2_v1_skproject_locate_other_level(
                  maps, 4u, 8, 0, &x, &y, cursor, 4u, 0u, &resume,
                  &locate) == -1 &&
                  locate.blocked_missing_descriptors,
              "DM_LOCATE_OTHER_LEVEL rejects missing source map descriptor");

        CHECK(dm2_v1_skproject_map_3bf83(
                  5, 6, 2, 7, 2, 3, 4, 12, 9, &map3bf83) == 1 &&
                  map3bf83.valid && map3bf83.in_bounds &&
                  !map3bf83.target_differs_from_current &&
                  map3bf83.move_to_x == 5 && map3bf83.move_to_y == 6 &&
                  map3bf83.requested_party_rotate &&
                  map3bf83.rotation == 3,
              "DM2_map_3BF83 same-map route moves record to target square and rotates party");
        CHECK(dm2_v1_skproject_map_3bf83(
                  5, 6, 3, 1, 2, 3, 4, 12, 9, &map3bf83) == 1 &&
                  map3bf83.target_differs_from_current &&
                  map3bf83.requested_change_to_target &&
                  map3bf83.requested_restore_current &&
                  map3bf83.requested_load_newmap &&
                  map3bf83.move_from_x == 3 &&
                  map3bf83.move_from_y == 4 &&
                  map3bf83.move_to_x == -1 &&
                  map3bf83.move_to_y == 6,
              "DM2_map_3BF83 cross-map route plans restore, LOAD_NEWMAP, and source-shaped move target");
        CHECK(dm2_v1_skproject_map_3bf83(
                  13, 6, 3, 1, 2, 3, 4, 12, 9, &map3bf83) == 0 &&
                  map3bf83.valid && !map3bf83.in_bounds &&
                  map3bf83.requested_restore_current,
              "DM2_map_3BF83 out-of-bounds cross-map route only restores current map");
    }

    {
        DM2_V1_SkprojectArrowHighlightReceipt arrow;
        DM2_V1_SkprojectOtherLevelReceipt other_level;
        DM2_V1_SkprojectMove12B4023FReceipt crush;
        DM2_V1_SkprojectLiftRequest lift;
        DM2_V1_SkprojectLiftReceipt lift_receipt;
        DM2_V1_SkprojectWallAlcoveReceipt alcove;
        DM2_V1_SkprojectTeleporterProbe teleporters[5];
        DM2_V1_SkprojectTeleporterSearchReceipt teleporter_search;
        DM2_V1_SkprojectThrownObjectTerminalReceipt thrown_terminal;
        DM2_V1_SkprojectCreaturePushReceipt creature_push;
        const int16_t direction_champions[4] = { 2, -1, 1, 1 };
        const uint8_t hero_types[4] = { 10u, 11u, 12u, 13u };
        const uint8_t wound_results[4] = { 0u, 1u, 1u, 0u };

        CHECK(dm2_v1_skproject_move_12b4_0092(
                  1u, 7u, -3, &arrow) == 1 &&
                  arrow.valid && arrow.requested_highlight &&
                  arrow.arrow_panel == 7u && arrow.highlight_param == -3,
              "DM2_move_12b4_0092 requests arrow-panel highlight only while v1e0534 is active");
        CHECK(dm2_v1_skproject_move_12b4_0092(
                  0u, 7u, -3, &arrow) == 0 &&
                  arrow.valid && !arrow.requested_highlight,
              "DM2_move_12b4_0092 is a no-op when v1e0534 is clear");

        CHECK(dm2_v1_skproject_move_12b4_00af(
                  0, 3, 10, 11, 4, 12, 13, 2, &other_level) == 1 &&
                  other_level.valid && other_level.requested_drop_record &&
                  other_level.locate_delta == 1 &&
                  other_level.located_map == 4 &&
                  other_level.final_party_dir == 2 &&
                  other_level.requested_restore_source_map,
              "DM2_move_12b4_00af plans source drop, locate-other-level, rotation, and source-map restore");
        CHECK(dm2_v1_skproject_move_12b4_00af(
                  1, 3, 10, 11, 4, 12, 13, 7, &other_level) == 1 &&
                  other_level.locate_delta == -1 &&
                  other_level.final_party_dir == 3,
              "DM2_move_12b4_00af reverses locate delta for forward transition and masks rotation");

        CHECK(dm2_v1_skproject_move_12b4_023f(
                  40, 41, 1, 3, direction_champions, hero_types,
                  wound_results, &crush) == 1 &&
                  crush.valid &&
                  crush.first_direction == 2u &&
                  crush.second_direction == 3u &&
                  crush.first_candidate == 1 &&
                  crush.second_candidate == 1 &&
                  crush.candidate_count == 1u &&
                  crush.wound_attempts == 1u &&
                  crush.wound_successes == 1u &&
                  crush.noise_requests == 1u &&
                  crush.wounded_champions[0] == 1 &&
                  crush.noise_hero_types[0] == 11u,
              "DM2_move_12b4_023f wounds the two source side candidates once when both directions resolve to the same champion");
        CHECK(dm2_v1_skproject_move_12b4_023f(
                  40, 41, 0, 0, direction_champions, hero_types,
                  wound_results, &crush) == 1 &&
                  crush.first_direction == 2u &&
                  crush.second_direction == 3u &&
                  crush.candidate_count == 1u &&
                  crush.wounded_champions[0] == 1,
              "DM2_move_12b4_023f uses `(arg1 + arg0 + 2/3) & 3` source directions");
        CHECK(dm2_v1_skproject_move_12b4_023f(
                  40, 41, 2, 3, direction_champions, hero_types,
                  wound_results, &crush) == 1 &&
                  crush.first_direction == 3u &&
                  crush.second_direction == 0u &&
                  crush.candidate_count == 2u &&
                  crush.wound_attempts == 2u &&
                  crush.wound_successes == 2u &&
                  crush.wounded_champions[0] == 1 &&
                  crush.wounded_champions[1] == 2 &&
                  crush.noise_hero_types[1] == 12u,
              "DM2_move_12b4_023f wounds both distinct champions and records hero-type sound requests");

        {
            DM2_V1_SkprojectAttackDoorReceipt door;
            DM2_V1_SkprojectWallAttackRecord wall_records[4];
            DM2_V1_SkprojectAttackWallReceipt wall;

            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0u, 0u, 10u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 0 &&
                      door.valid && door.blocked_door_closed_flag &&
                      door.test_byte_offset == 3u &&
                      door.tested_flag_mask == 1u,
                  "DM2_ATTACK_DOOR rejects byte3 bit0 gate when normal door flag is clear");
            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0u, 1u, 4u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 0 &&
                      door.blocked_attack_power &&
                      door.attack_power == 4u &&
                      door.required_power == 5u,
                  "DM2_ATTACK_DOOR rejects attacks below GET_DOOR_STAT_0X10 threshold");
            CHECK(dm2_v1_skproject_attack_door(
                      3u, 0u, 1u, 10u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 0 &&
                      door.blocked_tile_type &&
                      door.tile_type_before == 3u &&
                      door.tile_type_after == 3u,
                  "DM2_ATTACK_DOOR rejects non-door tile types before mutation");
            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0x80u, 0u, 10u, 5u, 1, 1, 3u, 6, 7,
                      &door) == 1 &&
                      door.admitted && door.queued_timer &&
                      !door.changed_tile_type &&
                      door.rebirth_altar &&
                      door.test_byte_offset == 2u &&
                      door.tested_flag_mask == 0x80u &&
                      door.timer_ticks == 3u,
                  "DM2_ATTACK_DOOR queues timer route for byte2-gated delayed destruction");
            CHECK(dm2_v1_skproject_attack_door(
                      4u, 0u, 1u, 10u, 5u, 0, 0, 0u, 6, 7,
                      &door) == 1 &&
                      door.admitted && door.changed_tile_type &&
                      !door.queued_timer &&
                      door.tile_type_after == 5u &&
                      door.x == 6 && door.y == 7,
                  "DM2_ATTACK_DOOR opens tile type 4 to 5 when no timer delay is requested");

            memset(wall_records, 0, sizeof(wall_records));
            wall_records[0].link_word = (uint16_t)(1u << 14);
            wall_records[0].alcove_data_index = 0x222u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x33u, 3, 0u, 8, 9,
                      &wall) == 1 &&
                      wall.valid && wall.found_effect &&
                      wall.used_alcove_relocation &&
                      wall.projectile_cut &&
                      wall.projectile_side_after == 1u &&
                      wall.matching_side_records == 1u,
                  "DM2_ATTACK_WALL relocates missile through ornate alcove only on matching side and RANDDIR zero");

            memset(wall_records, 0, sizeof(wall_records));
            wall_records[0].link_word = (uint16_t)(1u << 14);
            wall_records[0].record_type = 3u;
            wall_records[0].actuator_class = 0x22u;
            wall_records[0].required_item_type = 0x44u;
            wall_records[0].consume_projectile = 1u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x44u, 3, 2u, 8, 9,
                      &wall) == 1 &&
                      wall.invoked_actuator &&
                      wall.projectile_cut &&
                      wall.projectile_side_after == 1u &&
                      !wall.used_alcove_relocation,
                  "DM2_ATTACK_WALL invokes class-0x22 actuator and consumes matching projectile");

            wall_records[0].target_flag = 1u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x44u, 3, 2u, 8, 9,
                      &wall) == 0 &&
                      !wall.found_effect &&
                      wall.matching_side_records == 1u,
                  "DM2_ATTACK_WALL honours class-0x22 target flag inversion");

            memset(wall_records, 0, sizeof(wall_records));
            wall_records[0].link_word = (uint16_t)(1u << 14);
            wall_records[0].record_type = 3u;
            wall_records[0].actuator_class = 0x23u;
            wall_records[0].required_item_type = 0x01ffu;
            wall_records[0].tile_type_at_destination = 0u;
            wall_records[0].destination_x = 12;
            wall_records[0].destination_y = 14;
            wall_records[0].side_when_tile_zero = 2u;
            CHECK(dm2_v1_skproject_attack_wall(
                      wall_records, 1u, 0x0400u, 0x77u, 3, 2u, 8, 9,
                      &wall) == 1 &&
                      wall.used_teleport_relocation &&
                      wall.projectile_cut &&
                      wall.target_x == 12 && wall.target_y == 14 &&
                      wall.projectile_side_after == 2u,
                  "DM2_ATTACK_WALL teleports wildcard class-0x23 projectile using destination tile side rule");
        }

        memset(&lift, 0, sizeof(lift));
        lift.creature_weight = 40u;
        lift.event_hero_index = 1u;
        lift.hero_count = 3u;
        lift.heroes[0].alive = 1u;
        lift.heroes[0].strength = 20u;
        lift.heroes[0].stamina_adjusted_strength = 10u;
        lift.heroes[0].max_stamina = 160u;
        lift.heroes[0].cur_stamina = 80u;
        lift.rand16_values[0] = 2u;
        lift.heroes[1].alive = 1u;
        lift.heroes[1].strength = 36u;
        lift.heroes[1].max_stamina = 160u;
        lift.heroes[1].cur_stamina = 80u;
        lift.rand16_values[1] = 2u;
        CHECK(dm2_v1_skproject_move_12b4_099e(
                  &lift, &lift_receipt) == 1 &&
                  lift_receipt.valid && lift_receipt.can_lift &&
                  lift_receipt.checked_heroes == 2u &&
                  lift_receipt.stamina_adjustments[0] == 10u &&
                  lift_receipt.stamina_adjustments[1] == 10u,
              "DM2_move_12b4_099e admits lift through adjusted event-hero strength and drains living stamina");
        lift.creature_weight = 300u;
        CHECK(dm2_v1_skproject_move_12b4_099e(
                  &lift, &lift_receipt) == 0 &&
                  lift_receipt.valid &&
                  lift_receipt.blocked_overweight_creature,
              "DM2_move_12b4_099e rejects creature weights above 0xfd");

        CHECK(dm2_v1_skproject_wall_ornate_alcove_data_index(
                  1, 0x12, 0x345u, &alcove) == 1 &&
                  alcove.valid && alcove.ornate_alcove &&
                  alcove.cls2 == 0x12u &&
                  alcove.data_index == 0x345u,
              "DM2_0cee_317f queries GDAT data index for ornate wall alcove cls2");
        CHECK(dm2_v1_skproject_wall_ornate_alcove_data_index(
                  1, -1, 0x345u, &alcove) == 0 &&
                  alcove.cls2_missing,
              "DM2_0cee_317f rejects missing cls2 before GDAT lookup");
        CHECK(dm2_v1_skproject_wall_ornate_alcove_data_index(
                  0, 0x12, 0x345u, &alcove) == 0 &&
                  !alcove.valid && !alcove.ornate_alcove,
              "DM2_0cee_317f returns zero for non-ornate wall records");

        memset(teleporters, 0, sizeof(teleporters));
        teleporters[0].present = 1u;
        teleporters[0].detail_b4 = 0x82u;
        CHECK(dm2_v1_skproject_move_2fcf_0b8b(
                  10, 20, teleporters, &teleporter_search) == 1 &&
                  teleporter_search.valid &&
                  teleporter_search.direct_present &&
                  teleporter_search.selected_x == 10 &&
                  teleporter_search.selected_y == 20 &&
                  teleporter_search.teleporter_b4 == 0x82u,
              "DM2_move_2fcf_0b8b accepts direct teleporter detail before adjacent scan");
        memset(teleporters, 0, sizeof(teleporters));
        teleporters[2].present = 1u;
        teleporters[2].detail_b4 = 0x41u;
        CHECK(dm2_v1_skproject_move_2fcf_0b8b(
                  10, 20, teleporters, &teleporter_search) == 1 &&
                  teleporter_search.adjacent_present &&
                  teleporter_search.checked_adjacent_count == 2u &&
                  teleporter_search.selected_direction == 1u &&
                  teleporter_search.selected_x == 11 &&
                  teleporter_search.selected_y == 20,
              "DM2_move_2fcf_0b8b scans north/east/south/west neighbours in source order");
        CHECK(dm2_v1_skproject_move_2fcf_0b8b(
                  10, 20, 0, &teleporter_search) == 0 &&
                  teleporter_search.blocked_missing_adjacent_probes,
              "DM2_move_2fcf_0b8b fails closed without teleporter probe corpus");

        CHECK(dm2_v1_skproject_move_075f_0af9(
                  0xff89u, 3u, &thrown_terminal) == 1 &&
                  thrown_terminal.valid &&
                  thrown_terminal.kept_direction_for_ff89 &&
                  thrown_terminal.terminal_direction == 3u &&
                  thrown_terminal.requested_creature_push,
              "DM2_move_075f_0af9 keeps terminal direction for 0xff89 object records");
        CHECK(dm2_v1_skproject_move_075f_0af9(
                  0x1234u, 3u, &thrown_terminal) == 1 &&
                  thrown_terminal.rotated_for_other_records &&
                  thrown_terminal.terminal_direction == 1u,
              "DM2_move_075f_0af9 rotates other terminal records by +2 before creature push");

        CHECK(dm2_v1_skproject_move_12b4_0d75(
                  5, 6, 1u, 0, 20u, 30u, 0u, &creature_push) == 0 &&
                  creature_push.valid &&
                  creature_push.blocked_unmovable,
              "DM2_move_12b4_0d75 rejects when IS_CREATURE_MOVABLE_THERE fails");
        CHECK(dm2_v1_skproject_move_12b4_0d75(
                  5, 6, 1u, 1, 20u, 30u, 9u, &creature_push) == 1 &&
                  creature_push.lifted_by_force &&
                  creature_push.requested_lift_handoff,
              "DM2_move_12b4_0d75 admits push when creature weight is within force threshold");
        CHECK(dm2_v1_skproject_move_12b4_0d75(
                  5, 6, 1u, 1, 50u, 30u, 0u, &creature_push) == 1 &&
                  creature_push.random_range == 6u &&
                  creature_push.lifted_by_random_zero,
              "DM2_move_12b4_0d75 admits overweight creature only on source RAND16 zero route");
        CHECK(dm2_v1_skproject_move_12b4_0d75(
                  5, 6, 1u, 1, 50u, 30u, 2u, &creature_push) == 0 &&
                  creature_push.random_range == 6u &&
                  !creature_push.requested_lift_handoff,
              "DM2_move_12b4_0d75 leaves overweight nonzero-random creature in place");
    }
}

static void test_calc_vector_w_dir(void)
{
    DM2_V1_SkprojectVectorWDirReceipt receipt;
    int16_t x;
    int16_t y;

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              0, 3, 2, &x, &y, &receipt) == 1 &&
              x == 12 && y == 17 &&
              receipt.valid && receipt.dir == 0u &&
              receipt.forward_dx == 0 && receipt.forward_dy == -3 &&
              receipt.side_dx == 2 && receipt.side_dy == 0 &&
              receipt.initial_x == 10 && receipt.initial_y == 20,
          "CALC_VECTOR_W_DIR dir north adds forward and right-hand side deltas");

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              1, -1, 4, &x, &y, &receipt) == 1 &&
              x == 9 && y == 24 &&
              receipt.forward_dx == -1 && receipt.forward_dy == 0 &&
              receipt.side_dx == 0 && receipt.side_dy == 4,
          "CALC_VECTOR_W_DIR dir east preserves signed source operands");

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              3, 5, -2, &x, &y, &receipt) == 1 &&
              x == 5 && y == 22 &&
              receipt.forward_dx == -5 && receipt.forward_dy == 0 &&
              receipt.side_dx == 0 && receipt.side_dy == 2,
          "CALC_VECTOR_W_DIR dir west wraps side direction to north");

    x = -7;
    y = 8;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              5, 2, 3, &x, &y, &receipt) == 1 &&
              x == -5 && y == 11 && receipt.dir == 1u,
          "CALC_VECTOR_W_DIR masks direction like the source table index");

    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              0, 1, 1, 0, &y, &receipt) == 0 &&
              receipt.blocked_missing_output,
          "CALC_VECTOR_W_DIR rejects missing output accumulator");
}

static void test_cache_hash_helpers(void)
{
    DM2_V1_SkprojectCacheState state;
    DM2_V1_SkprojectFindFreeMementiReceipt free_receipt;
    DM2_V1_SkprojectDeallocFreeCacheIndexReceipt free_cache;
    DM2_V1_SkprojectFreeIndexedMementReceipt free_indexed;
    DM2_V1_SkprojectFreeTempCacheIndexReceipt free_temp;
    uint16_t ici = 0xffffu;
    uint16_t cache_index = 0xffffu;
    uint16_t current_mementi = 0xffffu;
    uint8_t *buff;

    dm2_v1_skproject_cache_state_init(&state, 4, 3, 4);
    state.raw_to_mement[2] = 3u;

    CHECK(state.cache_capacity == 4 && state.raw_count == 3 &&
              state.mement_count == 4,
          "cache state clamps source table sizes");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x2000u, &ici) == 0 && ici == 0u,
          "FIND_ICI_FROM_CACHE_HASH returns insertion slot for empty table");
    CHECK(dm2_v1_skproject_insert_cache_hash_at(
              &state, 0x2000u, ici) == 0u,
          "INSERT_CACHE_HASH_AT inserts the first cache hash");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x1000u, &ici) == 0 && ici == 0u,
          "FIND_ICI_FROM_CACHE_HASH finds lower insertion point");
    CHECK(dm2_v1_skproject_insert_cache_hash_at(
              &state, 0x1000u, ici) == 1u,
          "INSERT_CACHE_HASH_AT preserves sorted hash order");
    CHECK(state.cache_count == 2 &&
              state.sorted_cache_indices[0] == 1u &&
              state.sorted_cache_indices[1] == 0u,
          "sorted cache index table mirrors skproject indirect order");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x2000u, &ici) == 1 && ici == 1u,
          "FIND_ICI_FROM_CACHE_HASH returns existing sorted index");
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x2000u, &cache_index) == 1 &&
              cache_index == 0u,
          "ADD_CACHE_HASH returns existing cache index");
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x3000u, &cache_index) == 0 &&
              cache_index == 2u,
          "ADD_CACHE_HASH inserts a new cache index");
    CHECK(dm2_v1_skproject_query_mementi_from(&state, 0x8000u) == 0u &&
              dm2_v1_skproject_query_mementi_from(&state, 0x8001u) == 1u &&
              dm2_v1_skproject_query_mementi_from(&state, 2u) == 3u,
          "QUERY_MEMENTI_FROM handles cache-index and raw-data routes");
    CHECK(dm2_v1_skproject_query_mementi_from(&state, 0x8008u) ==
              DM2_V1_SKPROJECT_MEMENT_NONE,
          "QUERY_MEMENTI_FROM rejects out-of-range cache index");
    buff = dm2_v1_skproject_query_mement_buff_from_cache_index(&state, 1u);
    CHECK(buff == state.mement_buffers[1],
          "QUERY_MEMENT_BUFF_FROM_CACHE_INDEX returns mement payload bytes");
    CHECK(dm2_v1_skproject_get_temp_cache_hash(&state) == 0xffff0000u,
          "GET_TEMP_CACHE_HASH starts in the source temp hash range");
    CHECK(dm2_v1_skproject_alloc_temp_cache_index(&state) == 3u &&
              state.hashes[3] == 0xffff0000u &&
              state.temp_hash_counter == 1u,
          "ALLOC_TEMP_CACHE_INDEX allocates a temp hash through ADD_CACHE_HASH");
    CHECK(dm2_v1_skproject_alloc_temp_cache_index(&state) ==
              DM2_V1_SKPROJECT_MEMENT_NONE,
          "ALLOC_TEMP_CACHE_INDEX fails closed when cache table is full");

    CHECK(dm2_v1_skproject_free_cache_index(
              &state, 1u, &free_cache) == 1 &&
              free_cache.valid &&
              free_cache.cache_hash == 0x1000u &&
              free_cache.removed_sorted_entry &&
              state.cache_count == 3u &&
              state.hashes[1] == 0u &&
              state.sorted_cache_indices[0] == 0u &&
              state.lowest_free_cache_index == 1u,
          "FREE_CACHE_INDEX clears hash and removes the sorted cache index entry");

    dm2_v1_skproject_cache_state_init(&state, 4, 4, 4);
    state.raw_to_mement[2] = 2u;
    current_mementi = 2u;
    CHECK(dm2_v1_skproject_free_indexed_mement(
              &state, 2u, 1, &current_mementi, &free_indexed) == 1 &&
              free_indexed.valid &&
              free_indexed.cleared_raw_slot &&
              free_indexed.requested_recycle_mementi &&
              free_indexed.recycle.valid &&
              state.raw_to_mement[2] == DM2_V1_SKPROJECT_MEMENT_NONE &&
              current_mementi == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_INDEXED_MEMENT clears raw-data mement routes and current mement");

    dm2_v1_skproject_cache_state_init(&state, 4, 2, 4);
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x2222u, &cache_index) == 0 &&
              cache_index == 0u,
          "test setup adds cache hash for FREE_INDEXED_MEMENT cache route");
    state.cache_to_mement[0] = 1u;
    CHECK(dm2_v1_skproject_free_indexed_mement(
              &state, 0x8000u, 1, 0, &free_indexed) == 1 &&
              free_indexed.valid &&
              free_indexed.used_cache_route &&
              free_indexed.cleared_cache_slot &&
              free_indexed.requested_free_cache_index &&
              free_indexed.free_cache.valid &&
              state.cache_count == 0u &&
              state.hashes[0] == 0u,
          "FREE_INDEXED_MEMENT cache route clears slot and delegates FREE_CACHE_INDEX");

    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0xffff0000u, &cache_index) == 0,
          "test setup adds temp cache hash");
    state.cache_to_mement[cache_index] = 2u;
    CHECK(dm2_v1_skproject_free_temp_cache_index(
              &state, cache_index, 0, &free_temp) == 1 &&
              free_temp.valid &&
              free_temp.requested_temp_pin_clear &&
              free_temp.requested_free_indexed_mement &&
              free_temp.indexed.free_cache.valid &&
              state.cache_count == 0u,
          "FREE_TEMP_CACHE_INDEX clears temp ownership and frees indexed cache mement");

    dm2_v1_skproject_cache_state_init(&state, 4, 4, 4);
    state.cache_to_mement[1] = 1u;
    state.raw_to_mement[2] = 2u;
    CHECK(dm2_v1_skproject_find_free_mementi(
              &state, 0u, &free_receipt) == 0u &&
              free_receipt.valid &&
              free_receipt.returned_mementi == 0u &&
              free_receipt.next_free_mementi == 3u &&
              free_receipt.allocation_count == 1u,
          "FIND_FREE_MEMENTI returns current free index and skips referenced mements");
    CHECK(dm2_v1_skproject_find_free_mementi(
              &state, 0u, &free_receipt) == 3u &&
              free_receipt.valid &&
              free_receipt.exhausted_after_allocation &&
              free_receipt.next_free_mementi == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FIND_FREE_MEMENTI marks the free list exhausted after the last slot");

    state.cache_to_mement[0] = 0u;
    CHECK(dm2_v1_skproject_find_free_mementi(
              &state, 0u, &free_receipt) == 0u &&
              free_receipt.valid &&
              free_receipt.recycled_fallback &&
              state.cache_to_mement[0] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FIND_FREE_MEMENTI recycles the fallback mement when next-free is none");
}

static void test_picture_mement_helpers(void)
{
    DM2_V1_SkprojectCacheState state;
    DM2_V1_SkprojectNewPictReceipt new_pict;
    DM2_V1_Skproject0B36CachePicture cache_picture;
    DM2_V1_Skproject0B36Picture cached_picture;
    DM2_V1_Skproject0B36ButtonGroup group;
    DM2_V1_Skproject0B36CachePictureReceipt cache_receipt;
    DM2_V1_Skproject0B36DirtyRectReceipt dirty_receipt;
    DM2_V1_Skproject0B36ButtonGroupInitReceipt group_receipt;
    DM2_V1_Skproject0B36DrawCachedPictureReceipt draw_cached;
    DM2_V1_SkprojectExtendedPictureRef ext;
    DM2_V1_SkprojectImageMementRequest image;
    DM2_V1_SkprojectPictureRef pict;
    DM2_V1_SkprojectImageMementReceipt image_receipt;
    DM2_V1_SkprojectPictMementReceipt pict_receipt;
    DM2_V1_SkprojectFreeImageMementReceipt free_receipt;
    DM2_V1_SkprojectRecycleMementReceipt recycle_receipt;
    DM2_V1_SkprojectFreePict6Receipt free_pict6;
    uint16_t pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    DM2_V1_SkprojectRect rects[8];
    DM2_V1_SkprojectRect dirty;

    dm2_v1_skproject_cache_state_init(&state, 4, 8, 4);
    CHECK(dm2_v1_skproject_test_mement(-20, -20) == 1,
          "TEST_MEMENT accepts matching stored length");
    CHECK(dm2_v1_skproject_test_mement(-20, -24) == 0,
          "TEST_MEMENT rejects mismatched stored length");
    CHECK(dm2_v1_skproject_alloc_new_pict(
              7u, 13u, 5u, 4u, &new_pict) == 1 &&
              new_pict.payload_bytes == 35u &&
              new_pict.header_width == 13u &&
              new_pict.header_height == 5u &&
              new_pict.header_bpp == 4u,
          "ALLOC_NEW_PICT stores headers and 4bpp rounded row bytes");
    CHECK(dm2_v1_skproject_alloc_new_pict(
              8u, 13u, 5u, 8u, &new_pict) == 1 &&
              new_pict.payload_bytes == 65u,
          "ALLOC_NEW_PICT keeps 8bpp row bytes unrounded");

    ext.w6 = 0x3fffu;
    ext.w52 = 0x00ffu;
    ext.w54 = 0x003fu;
    CHECK(dm2_v1_skproject_calc_pict_ent_hash(&ext) ==
              (((uint32_t)0x1fffu << 12) | ((uint32_t)0x7fu << 5) | 0x1fu),
          "CALC_PICT_ENT_HASH masks and packs w6/w52/w54");

    memset(&cached_picture, 0, sizeof(cached_picture));
    cache_picture = (DM2_V1_Skproject0B36CachePicture){
        9u, 24u, 12u, 0x3456u, 1u
    };
    CHECK(dm2_v1_skproject_0b36_00c3_cache_picture(
              &cache_picture, &cached_picture, &cache_receipt) == 1 &&
              cache_receipt.valid &&
              cache_receipt.requested_mement_buffer &&
              cache_receipt.assigned_picture &&
              cached_picture.has_bits &&
              cached_picture.w4 == 0x0008u &&
              cached_picture.w12 == 9u &&
              cached_picture.width == 24u &&
              cached_picture.height == 12u &&
              cached_picture.w22 == 0x3456u,
          "_0b36_00c3 binds cache mement bits and picture header fields");
    cache_picture.payload_available = 0u;
    CHECK(dm2_v1_skproject_0b36_00c3_cache_picture(
              &cache_picture, &cached_picture, &cache_receipt) == 0 &&
              cache_receipt.blocked_missing_payload,
          "_0b36_00c3 fails closed when cache payload is absent");
    cache_picture.payload_available = 1u;

    memset(rects, 0, sizeof(rects));
    rects[2] = (DM2_V1_SkprojectRect){ 10, 6, 20, 14 };
    CHECK(dm2_v1_skproject_0b36_0c52_init_button_group(
              &group, 2u, 1u, 77u, rects, 8u, &group_receipt) == 1 &&
              group_receipt.valid &&
              group_receipt.requested_query_expanded_rect &&
              group_receipt.requested_alloc_temp_cache_index &&
              group_receipt.requested_alloc_new_pict &&
              group_receipt.requested_initial_dirty_rect &&
              group.dbidx == 77u &&
              group.rect.x == 10 &&
              group.rect.y == 6 &&
              group.rect.w == 20 &&
              group.rect.h == 14 &&
              group.group_size == 1u &&
              group.dirty_rects[0].x == 10 &&
              group.dirty_rects[0].y == 6,
          "_0b36_0c52 allocates an 8bpp button-group backing pict and seeds dirty rects");

    dirty = (DM2_V1_SkprojectRect){ 8, 7, 30, 20 };
    CHECK(dm2_v1_skproject_0b36_0d67_adjust_dirty_rects(
              &group, &dirty, &dirty_receipt) == 1 &&
              dirty_receipt.valid &&
              dirty_receipt.clipped_to_group &&
              dirty_receipt.stored_rect.x == 10 &&
              dirty_receipt.stored_rect.y == 7 &&
              dirty_receipt.stored_rect.w == 20 &&
              dirty_receipt.stored_rect.h == 13,
          "_0b36_0d67 clips dirty rects to the button-group bounds");
    dirty = group.dirty_rects[1];
    CHECK(dm2_v1_skproject_0b36_0d67_adjust_dirty_rects(
              &group, &dirty, &dirty_receipt) == 1 &&
              dirty_receipt.reused_covering_rect &&
              dirty_receipt.new_group_size == group.group_size,
          "_0b36_0d67 reuses an existing covering dirty rect");
    group.group_size = 5u;
    for (uint16_t i = 0u; i < 5u; ++i)
        group.dirty_rects[i] = (DM2_V1_SkprojectRect){
            (int16_t)(10 + i), 6, 1, 1
        };
    dirty = (DM2_V1_SkprojectRect){ 28, 19, 1, 1 };
    CHECK(dm2_v1_skproject_0b36_0d67_adjust_dirty_rects(
              &group, &dirty, &dirty_receipt) == 1 &&
              dirty_receipt.requested_compaction &&
              group.group_size == 1u &&
              group.dirty_rects[0].x == 28 &&
              group.dirty_rects[0].y == 19,
          "_0b36_0d67 compacts full dirty-rect lists before appending");

    rects[3] = (DM2_V1_SkprojectRect){ 15, 9, 20, 14 };
    cached_picture.width = 12u;
    cached_picture.height = 8u;
    CHECK(dm2_v1_skproject_0b36_11c0_draw_cached_picture(
              &cached_picture, &group, 3u, -17, rects, 8u,
              &draw_cached) == 1 &&
              draw_cached.valid &&
              draw_cached.requested_group_cache_bits &&
              draw_cached.requested_query_pict_bits &&
              draw_cached.requested_query_blit_rect &&
              draw_cached.requested_offset_rect &&
              draw_cached.requested_draw_def_pict &&
              draw_cached.requested_dirty_rect &&
              cached_picture.rect_no == 0xffffu &&
              cached_picture.color_key_passthrough == -17 &&
              cached_picture.width == 32u &&
              cached_picture.height == 22u &&
              draw_cached.picture_rect.x == 5 &&
              draw_cached.picture_rect.y == 3,
          "_0b36_11c0 draws cached picture bits through blit, offset and dirty receipts");

    memset(&image, 0, sizeof(image));
    image.cls1 = 1u;
    image.cls2 = 2u;
    image.cls4 = 3u;
    image.data_index = 5u;
    image.fallback_data_index = 9u;
    image.y_offset = -32;
    image.bits_pixel = 8u;
    image.existing_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY &&
              pinned_entry == 5u,
          "ALLOC_IMAGE_MEMENT pins 8bpp Y=-32 real image entry");
    image.y_offset = -31;
    pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_Y_OFFSET &&
              pinned_entry == DM2_V1_SKPROJECT_MEMENT_NONE,
          "ALLOC_IMAGE_MEMENT rejects non-source Y offset");
    image.y_offset = -32;
    image.bits_pixel = 4u;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_BPP,
          "ALLOC_IMAGE_MEMENT rejects non-8bpp image mement");
    image.bits_pixel = 8u;
    image.data_absent = 1;
    image.fallback_absent = 0;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY &&
              image_receipt.selected_data_index == 9u,
          "ALLOC_IMAGE_MEMENT falls back to default when primary is absent");
    image.data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status == DM2_V1_SKPROJECT_IMAGE_MEMENT_NO_ENTRY,
          "ALLOC_IMAGE_MEMENT does not fabricate fallback for missing primary");
    image.data_index = 5u;
    image.data_absent = 0;
    image.fallback_absent = 1;
    image.existing_mementi = 2u;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_TOUCHED_EXISTING &&
              image_receipt.touched_mementi == 2u,
          "ALLOC_IMAGE_MEMENT touches existing mement instead of pinning");

    CHECK(dm2_v1_skproject_recycle_mementi(
              &state, 2u, DM2_V1_SKPROJECT_MEMENT_NONE, 0u,
              &recycle_receipt) == 1 &&
              recycle_receipt.valid &&
              recycle_receipt.recycled_to_free_list,
          "RECYCLE_MEMENTI records free-list recycle for w4=0xffff");

    image.existing_mementi = 1u;
    state.raw_to_mement[5] = 1u;
    pinned_entry = 5u;
    CHECK(dm2_v1_skproject_free_image_mement(
              &state, &image, &pinned_entry, &free_receipt) == 1 &&
              free_receipt.cleared_pinned_entry &&
              free_receipt.recycled_existing &&
              state.raw_to_mement[5] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_IMAGE_MEMENT clears pinned entry and recycles existing mement");

    memset(&pict, 0, sizeof(pict));
    pict.w4 = 0x0004u;
    image.existing_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_pict_mement(
              &state, &pict, &image, &pinned_entry, &pict_receipt) == 1 &&
              pict_receipt.route == DM2_V1_SKPROJECT_PICT_MEMENT_IMAGE,
          "ALLOC_PICT_MEMENT routes image-backed pictures to image mement");
    pict.w4 = 0x0008u;
    pict.w12 = 2u;
    CHECK(dm2_v1_skproject_alloc_pict_mement(
              &state, &pict, &image, &pinned_entry, &pict_receipt) == 1 &&
              pict_receipt.route == DM2_V1_SKPROJECT_PICT_MEMENT_CACHE &&
              pict_receipt.cache_index == 2u && state.cache_count == 0u,
          "ALLOC_PICT_MEMENT routes cache-backed pictures by w12 index");
    state.cache_to_mement[2] = 2u;
    CHECK(dm2_v1_skproject_free_pict_mement(
              &state, &pict, &image, &pinned_entry, &free_receipt) == 1 &&
              state.cache_to_mement[2] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_PICT_MEMENT frees cache-backed pictures by w12 index");

    CHECK(dm2_v1_skproject_free_pict6(
              0u, 1u, 0x12345678u, &free_pict6) == 1 &&
              free_pict6.valid &&
              free_pict6.requested_dealloc_upper &&
              free_pict6.requested_draw_icon_entry,
          "FREE_PICT6 deallocates upper memory for afDefault and redraws the icon entry");
    CHECK(dm2_v1_skproject_free_pict6(
              0u, 2u, 0x12345678u, &free_pict6) == 1 &&
              free_pict6.requested_dealloc_lower,
          "FREE_PICT6 deallocates lower memory for non-default allocations");
    CHECK(dm2_v1_skproject_free_pict6(
              1u, 1u, 0x12345678u, &free_pict6) == 1 &&
              !free_pict6.requested_dealloc_upper &&
              free_pict6.requested_draw_icon_entry,
          "FREE_PICT6 global gate skips deallocation but still redraws icon entry");
}

static void test_item_charge_helpers(void)
{
    DM2_V1_SkprojectItemChargeReceipt receipt;
    uint16_t w2;

    CHECK(dm2_v1_skproject_get_max_charge(0x1400u) == 15u,
          "GET_MAX_CHARGE returns 15 for DB5 weapon");
    CHECK(dm2_v1_skproject_get_max_charge(0x1800u) == 15u,
          "GET_MAX_CHARGE returns 15 for DB6 cloth");
    CHECK(dm2_v1_skproject_get_max_charge(0x2800u) == 3u,
          "GET_MAX_CHARGE returns 3 for DB10 miscellaneous item");
    CHECK(dm2_v1_skproject_get_max_charge(0xffffu) == 0u,
          "GET_MAX_CHARGE returns zero for OBJECT_NULL");
    CHECK(dm2_v1_skproject_get_max_charge(0x0800u) == 0u,
          "GET_MAX_CHARGE returns zero for unsupported DB type");

    w2 = (uint16_t)(7u << 10);
    CHECK(dm2_v1_skproject_add_item_charge(0x1400u, &w2, 3, &receipt) ==
              10u &&
              receipt.valid && receipt.db_type == 5 &&
              receipt.previous_charge == 7u && receipt.new_charge == 10u &&
              receipt.max_charge == 15u && ((w2 >> 10) & 0x0fu) == 10u,
          "ADD_ITEM_CHARGE updates DB5 weapon charges in bits 10..13");

    w2 = (uint16_t)(14u << 10);
    CHECK(dm2_v1_skproject_add_item_charge(0x1400u, &w2, 9, &receipt) ==
              15u &&
              ((w2 >> 10) & 0x0fu) == 15u,
          "ADD_ITEM_CHARGE clamps DB5 weapon charges to 15");

    w2 = (uint16_t)(2u << 9);
    CHECK(dm2_v1_skproject_add_item_charge(0x1800u, &w2, -5, &receipt) ==
              0u &&
              receipt.valid && receipt.db_type == 6 &&
              ((w2 >> 9) & 0x0fu) == 0u,
          "ADD_ITEM_CHARGE clamps DB6 cloth charges to zero");

    w2 = (uint16_t)(1u << 14);
    CHECK(dm2_v1_skproject_add_item_charge(0x2800u, &w2, 5, &receipt) ==
              3u &&
              receipt.valid && receipt.db_type == 10 &&
              receipt.max_charge == 3u && ((w2 >> 14) & 0x03u) == 3u,
          "ADD_ITEM_CHARGE clamps DB10 miscellaneous charges to 3");

    w2 = 0xaaaau;
    CHECK(dm2_v1_skproject_add_item_charge(0xffffu, &w2, 1, &receipt) ==
              0u &&
              receipt.blocked_null_object && w2 == 0xaaaau,
          "ADD_ITEM_CHARGE rejects OBJECT_NULL without mutation");

    w2 = 0x5555u;
    CHECK(dm2_v1_skproject_add_item_charge(0x0800u, &w2, 1, &receipt) ==
              0u &&
              receipt.blocked_unsupported_db_type && w2 == 0x5555u,
          "ADD_ITEM_CHARGE rejects unsupported DB type without mutation");
}

static void test_item_value_weight_helpers(void)
{
    DM2_V1_SkprojectItemValueRecord records[8];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectItemValueReceipt receipt;
    DM2_V1_SkprojectItemClassifyReceipt classify;
    DM2_V1_SkprojectItemNameReceipt item_name;
    DM2_V1_SkprojectItemOrderReceipt item_order;
    DM2_V1_SkprojectFmtNumReceipt fmt_num;
    DM2_V1_SkprojectStrLenReceipt strlen_receipt;
    DM2_V1_SkprojectStrStrReceipt strstr_receipt;
    DM2_V1_SkprojectStrCopyCatReceipt copycat;
    DM2_V1_SkprojectLtoa10Receipt ltoa;
    DM2_V1_SkprojectScriptChrReceipt script_chr;
    uint16_t money_ids[6] = { 4u, 5u, 6u, 7u, 261u, 262u };
    char text_buf[32];

    memset(records, 0, sizeof(records));
    world.records = records;
    world.record_count = 8u;

    records[0].object_id = 0x1400u;
    records[0].gdat_cls1 = 1u;
    records[0].gdat_cls2 = 2u;
    records[0].w2 = (uint16_t)(3u << 10);
    records[0].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[0].gdat_word_values[1] = 4u;
    records[0].gdat_word_values[2] = 20u;
    records[0].gdat_word_values[0x34] = 2u;
    records[0].gdat_word_values[0x35] = 5u;

    records[1].object_id = 0x2001u;
    records[1].w2 = 128u;
    records[1].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[1].gdat_word_values[2] = 100u;

    records[2].object_id = 0x2402u;
    records[2].gdat_cls1 = 20u;
    records[2].gdat_cls2 = 4u;
    records[2].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[2].contained_object_id = 0x1400u;
    records[2].container_type = 0u;
    records[2].gdat_word_values[1] = 10u;

    records[3].object_id = 0x2403u;
    records[3].gdat_cls1 = 20u;
    records[3].gdat_cls2 = 6u;
    records[3].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[3].contained_object_id = 0x2804u;
    records[3].container_type = 0u;
    records[3].is_moneybox = 1u;
    records[3].gdat_word_values[1] = 1u;

    records[4].object_id = 0x2804u;
    records[4].w2 = (uint16_t)(2u << 14);
    records[4].next_object_id = 0x2805u;
    records[4].gdat_word_values[0] = 0x4000u;
    records[4].gdat_word_values[1] = 5u;
    records[4].gdat_word_values[2] = 7u;

    records[5].object_id = 0x2805u;
    records[5].w2 = 0u;
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[5].gdat_word_values[1] = 6u;
    records[5].gdat_word_values[2] = 11u;

    records[6].object_id = 0x2406u;
    records[6].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[6].container_type = 1u;
    records[6].gdat_word_values[1] = 9u;

    records[7].object_id = 0x1807u;
    records[7].gdat_cls1 = 0x15u;
    records[7].gdat_cls2 = 0u;
    records[7].champion_bones_owner = 2u;
    records[7].w2 = (uint16_t)(1u << 9);
    records[7].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[7].gdat_word_values[1] = 3u;
    records[7].gdat_word_values[0x34] = 4u;

    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1400u, 1u, &receipt) == 10 &&
              receipt.valid && receipt.base_value == 4 &&
              receipt.charge == 3u && receipt.charge_value_added == 6,
          "QUERY_ITEM_VALUE adds cls4=0x34 weight per charge");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1400u, 2u, &receipt) == 35 &&
              receipt.valid && receipt.base_value == 20 &&
              receipt.charge_value_added == 15,
          "QUERY_ITEM_VALUE adds cls4=0x35 money per charge");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2001u, 2u, &receipt) == 75 &&
              receipt.potion_value_before_scale == 100 &&
              receipt.potion_value_after_scale == 75,
          "QUERY_ITEM_VALUE scales potion money by low-byte power");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2402u, 1u, &receipt) == 20 &&
              receipt.contained_recursive_value == 10,
          "QUERY_ITEM_VALUE recurses normal container contents");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2403u, 1u, &receipt) == 6 &&
              receipt.moneybox_contained_value == 21 &&
              receipt.moneybox_rounding_value == 5,
          "QUERY_ITEM_VALUE rounds moneybox weight as (sum+4)/5");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2403u, 2u, &receipt) == 32 &&
              receipt.moneybox_contained_value == 32 &&
              receipt.moneybox_rounding_value == 32,
          "QUERY_ITEM_VALUE adds moneybox non-weight value directly");
    CHECK(dm2_v1_skproject_query_item_weight(
              &world, 0x1807u, &receipt) == 7 &&
              receipt.charge_multiplier_cls4 == 0x34u,
          "QUERY_ITEM_WEIGHT delegates to QUERY_ITEM_VALUE cls4=1");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, DM2_V1_SKPROJECT_MEMENT_NONE, 1u, &receipt) == 0 &&
              receipt.blocked_null_object,
          "QUERY_ITEM_VALUE rejects OBJECT_NULL");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1410u, 1u, &receipt) == 0 &&
              receipt.blocked_missing_record,
          "QUERY_ITEM_VALUE rejects missing source record");

    CHECK(dm2_v1_skproject_is_container_moneybox(
              &world, 0x2403u, 1, &classify) == 1 &&
              classify.valid &&
              classify.is_moneybox &&
              classify.container_type == 0u,
          "IS_CONTAINER_MONEYBOX requires container DB, type 0, and GDAT moneybox item list");
    CHECK(dm2_v1_skproject_is_container_chest(
              &world, 0x2402u, 0, &classify) == 1 &&
              classify.valid &&
              classify.is_chest &&
              !classify.is_moneybox,
          "IS_CONTAINER_CHEST accepts type-0 containers that are not moneyboxes");
    CHECK(dm2_v1_skproject_is_container_chest(
              &world, 0x2403u, 1, &classify) == 0 &&
              classify.valid &&
              classify.is_moneybox &&
              !classify.is_chest,
          "IS_CONTAINER_CHEST rejects moneybox containers");
    CHECK(dm2_v1_skproject_is_miscitem_currency(
              &world, 0x2804u, &classify) == 1 &&
              classify.valid &&
              classify.is_currency &&
              classify.gdat_flags == 0x4000u,
          "IS_MISCITEM_CURRENCY checks DB10 and GDAT currency flag 0x4000");
    CHECK(dm2_v1_skproject_get_item_name(
              &world, 0x1807u, 0u, 4u, &item_name) == 1 &&
              item_name.valid &&
              item_name.gdat_cls1 == 0x15u &&
              item_name.gdat_cls2 == 0u &&
              item_name.champion_bones_index == 2u &&
              item_name.requested_gdat_item_name,
          "GET_ITEM_NAME records champion-bones owner before querying GDAT item text");
    CHECK(dm2_v1_skproject_get_item_order_in_container(
              0x2403u, 6u, "5-7 J5-6", money_ids, 6u, 3u,
              &item_order) == 4 &&
              item_order.valid &&
              item_order.expanded_item_id == 261u &&
              item_order.returned_money_index == 4,
          "GET_ITEM_ORDER_IN_CONTAINER parses ranges and J-offset item ids against the money table");
    CHECK(dm2_v1_skproject_get_item_order_in_container(
              0x2403u, 6u, "", money_ids, 6u, 0u, &item_order) == -1 &&
              item_order.blocked_missing_text,
          "GET_ITEM_ORDER_IN_CONTAINER rejects empty GDAT order text");
    CHECK(dm2_v1_skproject_fmt_num(47u, 1u, 3u, &fmt_num) == 1 &&
              fmt_num.valid &&
              strcmp(fmt_num.buffer, "  47") == 0 &&
              strcmp(fmt_num.returned_text, " 47") == 0 &&
              fmt_num.returned_offset == 1u,
          "FMT_NUM clean path blanks four bytes and returns 4-keta offset");
    CHECK(dm2_v1_skproject_fmt_num(0u, 0u, 0u, &fmt_num) == 1 &&
              fmt_num.valid &&
              strcmp(fmt_num.returned_text, "0") == 0 &&
              fmt_num.returned_offset == 3u,
          "FMT_NUM non-clean path returns the first generated digit");
    CHECK(dm2_v1_skproject_sk_strlen(
              "SKWIN", &strlen_receipt) == 1 &&
              strlen_receipt.valid &&
              strlen_receipt.length == 5u,
          "SK_STRLEN counts bytes up to the null terminator");
    CHECK(dm2_v1_skproject_sk_strstr(
              "CM4SK4BZ2", "SK", &strstr_receipt) == 1 &&
              strstr_receipt.valid &&
              strstr_receipt.found &&
              strstr_receipt.match_offset == 3u,
          "SK_STRSTR returns the first source substring offset");
    CHECK(dm2_v1_skproject_sk_strstr(
              "CM4SK4BZ2", "", &strstr_receipt) == 0 &&
              strstr_receipt.valid &&
              strstr_receipt.needle_empty_returns_null,
          "SK_STRSTR returns null for an empty needle like skproject");
    CHECK(dm2_v1_skproject_sk_strcpy(
              text_buf, sizeof(text_buf), "DM2", &copycat) == 1 &&
              copycat.valid &&
              copycat.copied_length == 3u &&
              strcmp(text_buf, "DM2") == 0,
          "SK_STRCPY copies source text into caller buffer");
    CHECK(dm2_v1_skproject_sk_strcat(
              text_buf, sizeof(text_buf), "-HUD", &copycat) == 1 &&
              copycat.valid &&
              copycat.result_length == 7u &&
              strcmp(text_buf, "DM2-HUD") == 0,
          "SK_STRCAT appends source text into caller buffer");
    CHECK(dm2_v1_skproject_ltoa10(
              -2048, text_buf, sizeof(text_buf), &ltoa) == 1 &&
              ltoa.valid &&
              ltoa.written_length == 5u &&
              strcmp(text_buf, "-2048") == 0,
          "SK_LTOA10 formats signed base-10 text through SK_STRCPY");
    CHECK(dm2_v1_skproject_skchr_to_scriptchr(
              'C', &script_chr) == 1 &&
              script_chr.valid &&
              script_chr.output == 2u,
          "DM2_SKCHR_TO_SCRIPTCHR maps A-Z to zero-based script chars");
    CHECK(dm2_v1_skproject_skchr_to_scriptchr(
              '.', &script_chr) == 1 &&
              script_chr.output == 0x1bu,
          "DM2_SKCHR_TO_SCRIPTCHR maps dot to 0x1b");
    CHECK(dm2_v1_skproject_skchr_to_scriptchr(
              '?', &script_chr) == 1 &&
              script_chr.output == 0x1au,
          "DM2_SKCHR_TO_SCRIPTCHR maps non-letter fallback to 0x1a");
}

static void test_player_weight_helper(void)
{
    DM2_V1_SkprojectItemValueRecord records[4];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectPlayerWeightRequest request;
    DM2_V1_SkprojectPlayerWeightReceipt receipt;
    DM2_V1_SkprojectEquipItemReceipt equip_receipt;

    memset(records, 0, sizeof(records));
    memset(&request, 0xff, sizeof(request));
    world.records = records;
    world.record_count = 4u;

    records[0].object_id = 0x1400u;
    records[0].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[0].gdat_word_values[1] = 4u;
    records[1].object_id = 0x1801u;
    records[1].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[1].gdat_word_values[1] = 5u;
    records[2].object_id = 0x2402u;
    records[2].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[2].container_type = 0u;
    records[3].object_id = 0x2803u;
    records[3].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[3].gdat_word_values[1] = 6u;

    request.inventory[0] = 0x1400u;
    request.inventory[1] = 0x1801u;
    for (uint16_t i = 2u; i < DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS; ++i)
        request.inventory[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.current_container_items[0] = 0x2803u;
    for (uint16_t i = 1u; i < DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS; ++i)
        request.current_container_items[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.selected_hand_items[0] = 0x2402u;
    request.selected_hand_items[1] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.selected_hand_action = 0u;
    request.selected_player_plus_one = 1u;

    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.valid && receipt.inventory_weight == 9u &&
              receipt.open_chest_weight == 6u &&
              receipt.final_weight == 15u &&
              receipt.included_open_chest_overlay &&
              receipt.hero_flag_or == 0x1000u,
          "CALC_PLAYER_WEIGHT sums inventory and selected open chest overlay");

    request.selected_hand_action = 2u;
    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.final_weight == 9u &&
              receipt.blocked_selected_hand_action &&
              !receipt.included_open_chest_overlay,
          "CALC_PLAYER_WEIGHT skips overlay when selected hand action is not 0/1");

    request.selected_hand_action = 0u;
    request.selected_player_plus_one = 2u;
    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.final_weight == 9u &&
              receipt.blocked_player_not_selected,
          "CALC_PLAYER_WEIGHT skips overlay for non-selected player");

    CHECK(dm2_v1_skproject_equip_item_to_inventory(
              &request, 0u, 0x9402u, 3u, &equip_receipt) == 1 &&
              equip_receipt.valid &&
              equip_receipt.raw_object_id == 0x9402u &&
              equip_receipt.cleared_object_id == 0x1402u &&
              equip_receipt.previous_object_id ==
                  DM2_V1_SKPROJECT_MEMENT_NONE &&
              request.inventory[3] == 0x1402u &&
              equip_receipt.process_item_bonus_requested,
          "EQUIP_ITEM_TO_INVENTORY clears direction bits and equips champion slot");
    CHECK(dm2_v1_skproject_equip_item_to_inventory(
              &request, 0u, 0xa803u,
              DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS + 2u,
              &equip_receipt) == 1 &&
              equip_receipt.valid &&
              equip_receipt.equipped_to_container_overlay &&
              equip_receipt.container_slot == 2u &&
              equip_receipt.cleared_object_id == 0x2803u &&
              request.current_container_items[2] == 0x2803u,
          "EQUIP_ITEM_TO_INVENTORY routes slot >=30 to current container items");
    CHECK(dm2_v1_skproject_equip_item_to_inventory(
              &request, 0u, DM2_V1_SKPROJECT_MEMENT_NONE, 0u,
              &equip_receipt) == 0 &&
              equip_receipt.blocked_null_object,
          "EQUIP_ITEM_TO_INVENTORY rejects OBJECT_NULL");
    CHECK(dm2_v1_skproject_equip_item_to_inventory(
              &request, 0u, 0x1400u,
              DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS +
                  DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS,
              &equip_receipt) == 0 &&
              equip_receipt.blocked_inventory_slot_range,
          "EQUIP_ITEM_TO_INVENTORY rejects out-of-range container slot");
}

static void test_count_by_coin_types(void)
{
    DM2_V1_SkprojectItemValueRecord records[6];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectCountByCoinTypesReceipt receipt;
    uint16_t money_ids[DM2_V1_SKPROJECT_MONEY_ITEM_MAX] = {
        0x10u, 0x20u, 0x20u, 0x30u, 0x40u,
        0x50u, 0x60u, 0x70u, 0x80u, 0x90u
    };
    int16_t counts[DM2_V1_SKPROJECT_MONEY_ITEM_MAX];

    memset(records, 0, sizeof(records));
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_MONEY_ITEM_MAX; ++i)
        counts[i] = -7;
    world.records = records;
    world.record_count = 6u;

    records[0].object_id = 0x2400u;
    records[0].contained_object_id = 0x2801u;
    records[0].container_type = 0u;
    records[0].is_moneybox = 1u;

    records[1].object_id = 0x2801u;
    records[1].w2 = (uint16_t)(2u << 14);
    records[1].next_object_id = 0x2802u;
    records[1].distinctive_item_type = 0x20u;
    records[1].is_currency = 1u;

    records[2].object_id = 0x2802u;
    records[2].w2 = 0u;
    records[2].next_object_id = 0x2803u;
    records[2].distinctive_item_type = 0x10u;
    records[2].is_currency = 1u;

    records[3].object_id = 0x2803u;
    records[3].w2 = (uint16_t)(3u << 14);
    records[3].next_object_id = 0x1404u;
    records[3].distinctive_item_type = 0x99u;
    records[3].is_currency = 1u;

    records[4].object_id = 0x1404u;
    records[4].w2 = (uint16_t)(1u << 10);
    records[4].next_object_id = 0x2805u;
    records[4].distinctive_item_type = 0x30u;
    records[4].is_currency = 1u;

    records[5].object_id = 0x2805u;
    records[5].w2 = (uint16_t)(1u << 14);
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[5].distinctive_item_type = 0x30u;
    records[5].is_currency = 0u;

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, counts, &receipt) == 1 &&
              receipt.valid && receipt.visited_records == 5u &&
              receipt.currency_records == 3u &&
              receipt.matched_currency_records == 3u &&
              counts[0] == 1 && counts[1] == 3 && counts[2] == 3 &&
              counts[3] == 0 && counts[9] == 0,
          "COUNT_BY_COIN_TYPES zeroes ten slots and adds charge+1 by money type");

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids, 2u, counts, &receipt) == 1 &&
              receipt.valid && receipt.money_item_count == 2u &&
              counts[0] == 1 && counts[1] == 3 && counts[2] == 0,
          "COUNT_BY_COIN_TYPES honors caller money table count after zeroing");

    records[5].next_object_id = 0x2810u;
    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, counts, &receipt) == 0 &&
              receipt.blocked_missing_record,
          "COUNT_BY_COIN_TYPES rejects missing source-shaped chain record");
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, 0, &receipt) == 0 &&
              receipt.blocked_missing_output,
          "COUNT_BY_COIN_TYPES rejects missing output counter table");
}

static void test_boost_attribute(void)
{
    DM2_V1_SkprojectChampionAttribute attributes[4];
    DM2_V1_SkprojectBoostAttributeReceipt receipt;

    memset(attributes, 0, sizeof(attributes));
    attributes[1].current = 100u;
    attributes[1].maximum = 150u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 1u, 100, &receipt) == 1 &&
              receipt.valid && receipt.previous_current == 100u &&
              receipt.maximum == 150u && receipt.source_si == 50 &&
              receipt.reduced_delta == 57 &&
              attributes[1].current == 157u,
          "BOOST_ATTRIBUTE reduces large positive boosts by source 20-point buckets");

    attributes[1].current = 100u;
    attributes[1].maximum = 150u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 1u, -80, &receipt) == 1 &&
              receipt.valid && receipt.source_si == -130 &&
              receipt.reduced_delta == -15 &&
              attributes[1].current == 85u,
          "BOOST_ATTRIBUTE reduces large negative boosts symmetrically");

    attributes[2].current = 218u;
    attributes[2].maximum = 20u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 2u, 20, &receipt) == 1 &&
              attributes[2].current == 220u,
          "BOOST_ATTRIBUTE clamps boosted attribute to 220");

    attributes[3].current = 12u;
    attributes[3].maximum = 80u;
    CHECK(dm2_v1_skproject_boost_attribute(
              attributes, 3u, -20, &receipt) == 1 &&
              attributes[3].current == 10u,
          "BOOST_ATTRIBUTE clamps reduced attribute to 10");

    CHECK(dm2_v1_skproject_boost_attribute(
              0, 0u, 1, &receipt) == 0 &&
              receipt.blocked_missing_attribute,
          "BOOST_ATTRIBUTE rejects missing caller champion attributes");
}

static void test_adjust_ui_event(void)
{
    DM2_V1_SkprojectUiChampionState champions[4];
    int16_t positions[4] = { 2, -1, 0, 1 };
    DM2_V1_SkprojectUiEvent event;
    DM2_V1_SkprojectAdjustUiEventReceipt receipt;
    DM2_V1_SkprojectCharsheetOptionIconReceipt option_icon;
    DM2_V1_SkprojectCommandSlotItem slot_item;
    DM2_V1_SkprojectDrawCmdSlotReceipt cmd_slot;
    DM2_V1_SkprojectDrawMoneyboxReceipt moneybox;
    DM2_V1_SkprojectDrawItemStatsBarReceipt stats_bar;
    DM2_V1_SkprojectDrawContainerPanelReceipt container_panel;
    DM2_V1_SkprojectDrawContainerSurveyReceipt survey;
    DM2_V1_SkprojectDrawItemOnWoodPanelReceipt wood_panel;
    DM2_V1_SkprojectDrawCurMaxHmsReceipt hms;
    DM2_V1_SkprojectChampion3StatValues stat_values;
    DM2_V1_SkprojectDrawPlayer3StatTextReceipt stat_text;
    DM2_V1_SkprojectDrawPlayer3StatPaneReceipt stat_pane;
    DM2_V1_SkprojectDrawPlayerAttackDirReceipt attack_dir;
    DM2_V1_SkprojectDrawMajicMapReceipt majic_map;
    DM2_V1_SkprojectDrawFoodWaterPoisonPanelReceipt food_panel;
    DM2_V1_SkprojectDrawCryocellLeverReceipt cryocell;
    DM2_V1_SkprojectDrawEyeMouthRectangleReceipt eye_mouth;
    const int16_t coin_order[10] = { 2, -1, 0, 1, -1, -1, -1, -1, -1, -1 };
    const int16_t coin_counts[10] = { 5, 40, 2, 0, 0, 0, 0, 0, 0, 0 };
    const uint16_t money_item_ids[10] = {
        0x0301u, 0x0302u, 0x0303u, 0u, 0u,
        0u, 0u, 0u, 0u, 0u
    };
    const uint16_t container_items[8] = {
        0x1001u, 0xffffu, 0x1002u, 0xffffu,
        0xffffu, 0xffffu, 0xffffu, 0x1003u
    };
    const uint16_t chain[10] = {
        0x2001u, 0x2002u, 0x2003u, 0x2004u, 0x2005u,
        0x2006u, 0x2007u, 0x2008u, 0x2009u, 0xfffeu
    };

    memset(champions, 0, sizeof(champions));
    for (uint16_t i = 0; i < 4u; ++i) {
        champions[i].present = 1u;
        champions[i].hand_activable[0] = 1u;
        champions[i].hand_activable[1] = 1u;
    }

    memset(&event, 0, sizeof(event));
    event.event = 116u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 120u && receipt.mapped_player == 2 &&
              receipt.mapped_hand == 0u,
          "ADJUST_UI_EVENT maps champion hand event through player direction");

    event.event = 117u;
    champions[2].hand_cooldown[1] = 3u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 0 &&
              event.event == 0u && receipt.blocked_hand_cooldown,
          "ADJUST_UI_EVENT cancels hand event while hand cooldown is active");
    champions[2].hand_cooldown[1] = 0u;

    event.event = 95u;
    event.x = 18;
    event.y = 18;
    event.rect.x = 10;
    event.rect.y = 10;
    event.rect.w = 20;
    event.rect.h = 20;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 16u && receipt.selected_spell_triangle &&
              receipt.diagonal_w3 <= receipt.diagonal_w2,
          "ADJUST_UI_EVENT converts spell/leader triangle click to spell event");

    event.event = 95u;
    event.x = 11;
    event.y = 28;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 95u && !receipt.selected_spell_triangle &&
              receipt.diagonal_w3 > receipt.diagonal_w2,
          "ADJUST_UI_EVENT keeps leader event when click falls outside spell triangle");

    event.event = 95u;
    champions[2].hand_cooldown[2] = 1u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 0 &&
              event.event == 0u && receipt.blocked_leader_hand_cooldown,
          "ADJUST_UI_EVENT cancels leader event when leader hand cooldown is active");
    champions[2].hand_cooldown[2] = 0u;

    event.event = 50u;
    CHECK(dm2_v1_skproject_adjust_ui_event(
              &event, 0u, positions, champions, 4u, &receipt) == 1 &&
              event.event == 50u && receipt.untouched_non_adjustable_event,
          "ADJUST_UI_EVENT leaves unrelated UI events untouched");

    CHECK(dm2_v1_skproject_draw_charsheet_option_icon(
              4u, 0x123u, 0x0002u, 0x0002u, &option_icon) == 1 &&
              option_icon.valid &&
              option_icon.incremented_for_active_option &&
              option_icon.cls4_drawn == 5u &&
              option_icon.gdat_category == 7u &&
              option_icon.gdat_cls2 == 0u &&
              option_icon.alpha == -1,
          "DRAW_CHARSHEET_OPTION_ICON increments cls4 for active option mask before static GDAT draw");
    CHECK(dm2_v1_skproject_draw_charsheet_option_icon(
              4u, 0x123u, 0x0002u, 0x0004u, &option_icon) == 1 &&
              !option_icon.incremented_for_active_option &&
              option_icon.cls4_drawn == 4u,
          "DRAW_CHARSHEET_OPTION_ICON preserves cls4 when option mask is inactive");

    slot_item = (DM2_V1_SkprojectCommandSlotItem){ 9u, 3u, 12u };
    CHECK(dm2_v1_skproject_draw_cmd_slot(
              2u, 1u, 0u, 7u, &slot_item, &cmd_slot) == 1 &&
              cmd_slot.valid &&
              cmd_slot.used_interface_icon &&
              cmd_slot.icon_category == 1u &&
              cmd_slot.icon_index == 4u &&
              cmd_slot.icon_entry == 0x16u &&
              cmd_slot.icon_button_id == 0x41u &&
              cmd_slot.name_button_id == 0x44u &&
              cmd_slot.requested_name_string &&
              cmd_slot.foreground_color == 15u &&
              cmd_slot.background_color == 0x4000u,
          "DRAW_CMD_SLOT normal route draws interface icon and command name string");
    CHECK(dm2_v1_skproject_draw_cmd_slot(
              2u, 1u, 1u, 7u, &slot_item, &cmd_slot) == 1 &&
              cmd_slot.used_container_icon &&
              cmd_slot.icon_category == 20u &&
              cmd_slot.icon_index == 7u &&
              cmd_slot.icon_entry == 0x4au &&
              cmd_slot.icon_button_id == 0x70u &&
              !cmd_slot.requested_name_string,
          "DRAW_CMD_SLOT magical-map route draws held-container icon only");
    CHECK(dm2_v1_skproject_draw_cmd_slot(
              2u, 1u, 0u, 7u, 0, &cmd_slot) == 0 &&
              cmd_slot.blocked_missing_item,
          "DRAW_CMD_SLOT fails closed without command slot item data");

    CHECK(dm2_v1_skproject_draw_moneybox(
              0x4400u, 6u, coin_order, coin_counts, money_item_ids,
              &moneybox) == 1 &&
              moneybox.valid &&
              moneybox.box_icon.category == 20u &&
              moneybox.box_icon.cls2 == 6u &&
              moneybox.box_icon.entry == 0x10u &&
              moneybox.box_icon.button_id == 0x5cu &&
              moneybox.inspected_slots == 10u &&
              moneybox.drawn_coin_slots == 3u &&
              moneybox.first_coin_button_id == 0xddu &&
              moneybox.last_coin_button_id == 0xe0u &&
              moneybox.first_coin_item_db == 3u &&
              moneybox.first_coin_item_type == 3u &&
              moneybox.first_coin_stack_count == 2u,
          "DRAW_MONEYBOX draws container icon, coin slots, and caps visible stack count at source limit");
    CHECK(dm2_v1_skproject_draw_moneybox(
              0x4400u, 6u, 0, coin_counts, money_item_ids,
              &moneybox) == 0 &&
              moneybox.blocked_missing_coin_tables,
          "DRAW_MONEYBOX fails closed without coin order/count/id tables");

    CHECK(dm2_v1_skproject_draw_item_stats_bar(
              0x1f0u, 75, 100, 'K', 5u, 1, &stats_bar) == 1 &&
              stats_bar.valid &&
              stats_bar.scaled_value == 1536 &&
              stats_bar.drew_power_bar &&
              stats_bar.drew_rune_label &&
              stats_bar.drew_low_marker &&
              stats_bar.drew_high_marker,
          "DRAW_ITEM_STATS_BAR scales current/max exactly like skproject before drawing rune markers");
    CHECK(dm2_v1_skproject_draw_item_stats_bar(
              0x1f0u, 75, 0, 'K', 5u, 1, &stats_bar) == 0 &&
              stats_bar.blocked_invalid_max,
          "DRAW_ITEM_STATS_BAR fails closed for invalid maximum value");
    CHECK(dm2_v1_skproject_draw_item_stats_bar(
              0x1f0u, 75, 100, 'K', 5u, 0, &stats_bar) == 0 &&
              stats_bar.blocked_missing_rect,
          "DRAW_ITEM_STATS_BAR fails closed when QUERY_EXPANDED_RECT misses");

    CHECK(dm2_v1_skproject_draw_container_panel(
              0x4500u, 4u, 1u, container_items, &container_panel) == 1 &&
              container_panel.valid &&
              container_panel.right_panel &&
              container_panel.background_icon.category == 20u &&
              container_panel.background_icon.entry == 0x10u &&
              container_panel.opened_lid_icon.entry == 0x12u &&
              container_panel.drawn_slots == 3u &&
              container_panel.first_slot_button_id == 0xe5u &&
              container_panel.last_slot_button_id == 0xecu &&
              !container_panel.uses_inventory_relative_blit,
          "DRAW_CONTAINER_PANEL right-panel route draws container body/lid and item buttons");
    CHECK(dm2_v1_skproject_draw_container_panel(
              0x4500u, 4u, 0u, container_items, &container_panel) == 1 &&
              container_panel.uses_inventory_relative_blit,
          "DRAW_CONTAINER_PANEL inventory route marks source-relative blit path");
    CHECK(dm2_v1_skproject_draw_container_panel(
              0x4500u, 4u, 0u, 0, &container_panel) == 0 &&
              container_panel.blocked_missing_items,
          "DRAW_CONTAINER_PANEL fails closed without current-container item table");

    CHECK(dm2_v1_skproject_draw_container_survey(
              chain, 10u, &survey) == 1 &&
              survey.valid &&
              survey.traversed_records == 8u &&
              survey.drawn_items == 8u &&
              survey.first_button_id == 0x2fu &&
              survey.last_button_id == 0x36u &&
              survey.stopped_at_limit,
          "DRAW_CONTAINER_SURVEY follows record links and stops at the eight-item container limit");
    CHECK(dm2_v1_skproject_draw_container_survey(
              0, 0u, &survey) == 0 &&
              survey.blocked_missing_chain,
          "DRAW_CONTAINER_SURVEY fails closed without a record chain");

    CHECK(dm2_v1_skproject_draw_item_on_wood_panel(
              1u, 0u, 0x3333u, 1, 24u, 18u, 5u, 7u, 0x22u,
              &wood_panel) == 1 &&
              wood_panel.valid &&
              wood_panel.requested_hand_activable_probe &&
              wood_panel.requested_alloc_temp_cache_index &&
              wood_panel.requested_alloc_new_pict &&
              wood_panel.picture_width == 29u &&
              wood_panel.picture_height == 25u &&
              wood_panel.bpp == 8u,
          "DRAW_ITEM_ON_WOOD_PANEL admits only hand-activable items and allocates an 8bpp temp picture");
    CHECK(dm2_v1_skproject_draw_item_on_wood_panel(
              1u, 0u, 0x3333u, 0, 24u, 18u, 5u, 7u, 0x22u,
              &wood_panel) == 0 &&
              wood_panel.blocked_not_hand_activable,
          "DRAW_ITEM_ON_WOOD_PANEL returns NULL-equivalent when item is not hand-activable");

    CHECK(dm2_v1_skproject_draw_cur_max_hms(
              0x226u, 7, 123, &hms) == 1 &&
              hms.valid &&
              strcmp(hms.text, "007/123") == 0 &&
              hms.foreground_color == 13u &&
              hms.background_color == 0x4001u,
          "DRAW_CUR_MAX_HMS formats 3-digit current/max text and local-text colors");
    CHECK(dm2_v1_skproject_draw_cur_max_hms(
              0x226u, -1, 123, &hms) == 0 &&
              hms.blocked_invalid_range,
          "DRAW_CUR_MAX_HMS fails closed outside original 0..999 range");

    stat_values = (DM2_V1_SkprojectChampion3StatValues){
        40, 88, 1234, 2345, 6, 78
    };
    CHECK(dm2_v1_skproject_draw_player_3stat_text(
              &stat_values, &stat_text) == 1 &&
              stat_text.valid &&
              stat_text.hp.rect_no == 0x226u &&
              strcmp(stat_text.hp.text, "040/088") == 0 &&
              stat_text.stamina.rect_no == 0x227u &&
              strcmp(stat_text.stamina.text, "123/234") == 0 &&
              stat_text.mana.rect_no == 0x228u &&
              strcmp(stat_text.mana.text, "006/078") == 0,
          "DRAW_PLAYER_3STAT_TEXT routes HP, stamina/10, and mana through DRAW_CUR_MAX_HMS");
    CHECK(dm2_v1_skproject_draw_player_3stat_text(
              0, &stat_text) == 0 &&
              stat_text.blocked_missing_stats,
          "DRAW_PLAYER_3STAT_TEXT fails closed without champion stats");

    CHECK(dm2_v1_skproject_draw_player_3stat_pane(
              2u, 44, 3u, 0u, 1u, &stat_pane) == 1 &&
              stat_pane.valid &&
              stat_pane.panel_variant_cls4 == 9u &&
              stat_pane.panel_button_id == 0xa3u &&
              stat_pane.gdat_category == 1u &&
              stat_pane.gdat_cls2 == 2u &&
              stat_pane.reset_group_size,
          "DRAW_PLAYER_3STAT_PANE draws selected champion panel variant and can reset group size");
    CHECK(dm2_v1_skproject_draw_player_3stat_pane(
              2u, 0, 0u, 0u, 0u, &stat_pane) == 1 &&
              stat_pane.panel_variant_cls4 == 1u,
          "DRAW_PLAYER_3STAT_PANE uses dead champion panel variant when HP is zero");
    CHECK(dm2_v1_skproject_draw_player_3stat_pane(
              2u, 44, 0u, 1u, 0u, &stat_pane) == 0 &&
              stat_pane.blocked_button_group_busy,
          "DRAW_PLAYER_3STAT_PANE returns without drawing while button group is busy");

    CHECK(dm2_v1_skproject_draw_player_attack_dir(
              3u, 0xf6u, 1u, 3u, 1u, 7u, &attack_dir) == 1 &&
              attack_dir.valid &&
              attack_dir.base_icon.category == 8u &&
              attack_dir.base_icon.cls2 == 0xf6u &&
              attack_dir.base_icon.entry == 0xf6u &&
              attack_dir.base_icon.button_id == 0x5du &&
              attack_dir.squad_icon_rect == 0x5eu &&
              attack_dir.left_arrow_button == 0x60u &&
              attack_dir.right_arrow_button == 0x61u &&
              attack_dir.jitter_y == 1 &&
              attack_dir.jitter_x == -1 &&
              attack_dir.requested_alloc_pict_buff &&
              attack_dir.requested_squad_palette &&
              attack_dir.requested_icon_blit &&
              attack_dir.requested_free_pict_buff &&
              attack_dir.drew_enchantment_aura,
          "DRAW_PLAYER_ATTACK_DIR plans squad icon draw, aura jitter, enchantment overlay, and arrow icons");

    CHECK(dm2_v1_skproject_draw_majic_map(
              0x4500u, 4u, 1u, 0u, 4u, 10u, 11u, 2u, 1u,
              &majic_map) == 1 &&
              majic_map.valid &&
              majic_map.container_cls2 == 4u &&
              majic_map.flags_after == 0x0c90u &&
              majic_map.requested_container_panel_init &&
              majic_map.requested_command_slots &&
              majic_map.requested_map_draw &&
              majic_map.requested_gray_overlay &&
              majic_map.target_x == 10u &&
              majic_map.target_y == 11u &&
              majic_map.target_map == 2u,
          "DRAW_MAJIC_MAP initializes held-container map panel, command slots, map draw, and overlay flags");

    CHECK(dm2_v1_skproject_draw_food_water_poison_panel(
              100, 200, 9, &food_panel) == 1 &&
              food_panel.valid &&
              food_panel.inventory_subpanel == 1u &&
              food_panel.panel_icon.category == 7u &&
              food_panel.panel_icon.entry == 1u &&
              food_panel.food_bar_rect == 0x1f0u &&
              food_panel.water_bar_rect == 0x1f1u &&
              food_panel.poison_bar_rect == 0x1f3u &&
              food_panel.drew_poison &&
              food_panel.food_text_icon.button_id == 0x1f4u &&
              food_panel.water_text_icon.button_id == 0x1f5u &&
              food_panel.poison_text_icon.button_id == 0x1f6u,
          "DRAW_FOOD_WATER_POISON_PANEL binds panel/text icons and stat-bar rects");
    CHECK(dm2_v1_skproject_draw_food_water_poison_panel(
              100, 200, 0, &food_panel) == 1 &&
              !food_panel.drew_poison,
          "DRAW_FOOD_WATER_POISON_PANEL skips poison row when poison value is zero");

    CHECK(dm2_v1_skproject_draw_cryocell_lever(
              1u, &cryocell) == 1 &&
              cryocell.valid &&
              cryocell.lever_icon.category == 9u &&
              cryocell.lever_icon.cls2 == 0x5bu &&
              cryocell.lever_icon.entry == 0xfbu &&
              cryocell.lever_icon.button_id == 0x1eeu &&
              cryocell.requested_drawings_completed &&
              cryocell.requested_open_sound,
          "DRAW_CRYOCELL_LEVER down route draws 0xfb and queues open sound");
    CHECK(dm2_v1_skproject_draw_cryocell_lever(
              0u, &cryocell) == 1 &&
              cryocell.lever_icon.entry == 0xfau &&
              cryocell.inventory_subpanel == 7u,
          "DRAW_CRYOCELL_LEVER up route sets resurrection subpanel");

    CHECK(dm2_v1_skproject_draw_eye_mouth_colored_rectangle(
              4u, 0x1feu, &eye_mouth) == 1 &&
              eye_mouth.valid &&
              eye_mouth.gdat_category == 1u &&
              eye_mouth.gdat_cls2 == 2u &&
              eye_mouth.cls4 == 4u &&
              eye_mouth.rect_no == 0x1feu &&
              eye_mouth.blit_mode == 12u &&
              eye_mouth.requested_inflated_rect &&
              eye_mouth.requested_local_palette,
          "DRAW_EYE_MOUTH_COLORED_RECTANGLE inflates rect and draws interface image with local palette");
}

static void test_gfx_str_helpers(void)
{
    uint8_t font_plane[DM2_V1_SKPROJECT_FONT_PLANE_BYTES];
    DM2_V1_SkprojectFontReceipt font_receipt;
    DM2_V1_SkprojectTextMetricsReceipt metrics;
    DM2_V1_SkprojectTextDrawReceipt draw;
    DM2_V1_SkprojectFormatTextReceipt fmt;
    DM2_V1_SkprojectHintLineReceipt hint;
    char text[64];
    char line[32];
    uint8_t encrypted[6];

    memset(font_plane, 0, sizeof(font_plane));
    for (uint8_t row = 0; row < 6u; ++row)
        font_plane[(uint16_t)row * 128u + 'A'] = 0x18u;
    CHECK(dm2_v1_skproject_query_font(
              font_plane, 'A', 0x0eu, 0x01u, &font_receipt) == 1 &&
              font_receipt.valid && font_receipt.written_pixels == 18u &&
              font_receipt.pixels[0] == 0xeeu &&
              font_receipt.pixels[1] == 0x11u,
          "c_gfx_str QUERY_FONT expands six source rows into 18 packed pixels");
    CHECK(dm2_v1_skproject_query_font(
              0, 'A', 0x0eu, 0x01u, &font_receipt) == 0 &&
              font_receipt.blocked_missing_font_plane,
          "c_gfx_str QUERY_FONT rejects missing font plane");

    CHECK(dm2_v1_skproject_query_str_metrics(
              "DM2", &metrics) == 1 &&
              metrics.valid && metrics.text_len == 3u &&
              metrics.width == 17 && metrics.height == 5,
          "c_gfx_str QUERY_STR_METRICS uses -gfxstrw2 plus glyph stride");
    CHECK(dm2_v1_skproject_query_str_metrics(
              "", &metrics) == 0 && !metrics.valid,
          "c_gfx_str QUERY_STR_METRICS rejects zero-width strings");

    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_STRING, 320, 10, 20,
              3u, 0x4004u, "AB", &draw) == 1 &&
              draw.valid && draw.dest_is_screen &&
              draw.uses_alpha_mask && draw.draw_y == 15 &&
              draw.first_char_x == 10 && draw.last_char_x == 16 &&
              draw.char_count == 2u,
          "c_gfx_str DRAW_STRING plan preserves baseline and alpha-mask flag");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_STRONG, 112, 30, 40,
              2u, 4u, "AB", &draw) == 1 &&
              draw.strong_shadow_passes == 2 &&
              draw.fill_background &&
              draw.fill_x == 29 && draw.fill_y == 35 &&
              draw.fill_w == 13 && draw.fill_h == 7,
          "c_gfx_str DRAW_STRONG_TEXT plan includes source background fill rect");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_BUTTON, 80, 4, 12,
              1u, 2u, "OK", &draw) == 1 &&
              draw.adjusts_button_rect,
          "c_gfx_str DRAW_BUTTON_STR plan marks button rect adjustment");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_NAME, 80, 4, 12,
              1u, 2u, "NAME", &draw) == 1 &&
              draw.adjusts_button_rect && draw.strong_shadow_passes == 2,
          "c_gfx_str DRAW_NAME_STR combines button placement with strong text");
    CHECK(dm2_v1_skproject_plan_draw_string(
              DM2_V1_SKPROJECT_TEXT_ROUTE_BACKBUFF, 128, 64, 50,
              1u, 2u, "ABCD", &draw) == 1 &&
              draw.centered_by_metrics && draw.draw_x == 62,
          "c_gfx_str DRAW_TEXT_TO_BACKBUFF plan centers by measured text height");

    CHECK(dm2_v1_skproject_format_skstr_literal(
              "SAVE GAME", text, sizeof(text), &fmt) == 1 &&
              fmt.valid && strcmp(text, "SAVE GAME") == 0 &&
              fmt.consumed_bytes == 9u && fmt.written_bytes == 9u,
          "c_gfx_str FORMAT_SKSTR literal path copies source text");
    CHECK(dm2_v1_skproject_format_skstr_literal(
              "VALUE .Z001", text, sizeof(text), &fmt) == 0 &&
              fmt.blocked_unimplemented_substitution &&
              strcmp(text, "VALUE ") == 0,
          "c_gfx_str FORMAT_SKSTR substitution path fails closed");

    for (uint16_t i = 0; i < 5u; ++i)
        encrypted[i] = (uint8_t)(~((uint8_t)"HELLO"[i] + (uint8_t)i));
    encrypted[5] = 0u;
    CHECK(dm2_v1_skproject_decode_gdat_text_literal(
              encrypted, 5u, 1, text, sizeof(text), &fmt) == 1 &&
              strcmp(text, "HELLO") == 0,
          "c_gfx_str QUERY_GDAT_TEXT decrypts bit-0x08 literal bytes");

    CHECK(dm2_v1_skproject_split_hint_line(
              "ALPHA BETA GAMMA", 0u, 60, line, sizeof(line), &hint) == 1 &&
              hint.valid && hint.split_at_space &&
              strcmp(line, "ALPHA BETA") == 0 && hint.next_offset == 11u,
          "c_gfx_str DM2_gfxstr_3929_04e2 wraps at the last fitting space");
    CHECK(dm2_v1_skproject_split_hint_line(
              "ONE\nTWO", 0u, 60, line, sizeof(line), &hint) == 1 &&
              hint.stopped_at_newline && strcmp(line, "ONE") == 0 &&
              hint.next_offset == 4u,
          "c_gfx_str DISPLAY_HINT_TEXT line splitter consumes explicit newline");
}

static void test_gdat_allocation_helpers(void)
{
    const DM2_V1_SkprojectGdatDescriptor entries[] = {
        { 1u, 0u, 2u, 2u, 4u, 20u },
        { 1u, 0u, 2u, 3u, 4u, 30u },
        { 1u, 0u, 2u, 0x0bu, 5u, 40u },
        { 1u, 0u, 2u, 3u, 0x8006u, 50u },
        { 2u, 0u, 7u, 1u, 8u, 9u }
    };
    const DM2_V1_SkprojectGdatDescriptor scripts[] = {
        { 1u, 0u, 2u, 0u, 0u, 0u },
        { 1u, 0u, 2u, 0u, 0x8000u, 0u }
    };
    uint8_t marks[16] = { 0 };
    DM2_V1_SkprojectGdatSoundAllocationReceipt sound;
    DM2_V1_SkprojectGdatZoneReceipt zone;
    DM2_V1_SkprojectLoadDyn4Receipt dyn4;

    CHECK(dm2_v1_skproject_gdat_sound_allocation_scan(
              entries, 5u, &sound) == 1 &&
              sound.valid &&
              sound.inspected_entries == 4u &&
              sound.unique_raw_indexes == 3u &&
              sound.largest_raw_length == 50u &&
              sound.receipt_hash != 0u,
          "DM2_dballoc_3e74_24b8 counts unique type-2 raw sound indexes");
    CHECK(dm2_v1_skproject_gdat_sound_allocation_scan(
              0, 1u, &sound) == 0 &&
              sound.blocked_missing_entries,
          "DM2_dballoc_3e74_24b8 rejects missing GDAT descriptor corpus");

    CHECK(dm2_v1_skproject_gdat_accepts_current_zone(
              4u, 2u, 0x00u, 0x30u, &zone) == 1 &&
              zone.valid && zone.accepted_zero_gate,
          "DM2_dballoc_3e74_2162 accepts zero upper-nibble zone gate");
    CHECK(dm2_v1_skproject_gdat_accepts_current_zone(
              4u, 2u, 0x30u, 0x30u, &zone) == 1 &&
              zone.valid && zone.accepted_current_zone,
          "DM2_dballoc_3e74_2162 accepts current zone nibble");
    CHECK(dm2_v1_skproject_gdat_accepts_current_zone(
              4u, 2u, 0x40u, 0x30u, &zone) == 0 &&
              zone.rejected_other_zone,
          "DM2_dballoc_3e74_2162 rejects other zone nibble");

    CHECK(dm2_v1_skproject_load_dyn4_receipt(
              scripts, 2u, entries, 5u, marks, 16u, 0, &dyn4) == 1 &&
              dyn4.valid &&
              dyn4.visited_entries == 4u &&
              dyn4.sound_gate_skips == 1u &&
              dyn4.incremented_entries == 1u &&
              dyn4.decremented_entries == 1u &&
              dyn4.skipped_b_or_c_entries == 2u &&
              dyn4.skipped_highbit_entries == 2u &&
              marks[4] == 0u &&
              dyn4.mark_hash != 0u,
          "DM2_LOAD_DYN4 applies skproject mark-table increment/decrement gates");
    CHECK(dm2_v1_skproject_load_dyn4_receipt(
              scripts, 1u, entries, 5u, marks, 4u, 1, &dyn4) == 0 &&
              dyn4.blocked_mark_capacity,
          "DM2_LOAD_DYN4 rejects raw indexes outside mark capacity");
}

static void test_skwin_core_symbol_batch_cycle3(void)
{
    DM2_V1_SkprojectCreatureAISpec ai_spec;
    DM2_V1_SkprojectCreatureAIWord30Receipt ai_receipt;
    DM2_V1_SkprojectMapDescriptor maps[2];
    uint8_t tiles[16];
    DM2_V1_SkprojectLevelTransitionReceipt lt_receipt;
    DM2_V1_SkprojectLevelTransitionPairReceipt lt_pair;
    DM2_V1_Skproject0B36ButtonGroup group;
    DM2_V1_SkprojectRect expanded_rects[2];
    DM2_V1_SkprojectButtonGroupBlackFillReceipt black_receipt;
    DM2_V1_SkprojectCommandSlotLoopReceipt slot_receipt;
    DM2_V1_Skproject0B36BlitDirtyRectsReceipt blit_receipt;
    DM2_V1_Skproject0B36DrawStringReceipt str_receipt;
    DM2_V1_SkprojectSkWin12B40092Receipt arrow_receipt;
    int16_t x, y;

    /* _0cee_2df4 creature AI spec word30 */
    CHECK(dm2_v1_skproject_0cee_2df4_creature_ai_word30(
              0x1234u, NULL, &ai_receipt) == 0,
          "_0cee_2df4 fails closed without AI spec");
    CHECK(ai_receipt.blocked_missing_ai_spec == 1,
          "_0cee_2df4 flags missing AI spec");
    CHECK(dm2_v1_skproject_0cee_2df4_creature_ai_word30(
              0xffffu, &ai_spec, &ai_receipt) == 0,
          "_0cee_2df4 fails closed on OBJECT_NULL");
    CHECK(ai_receipt.blocked_object_null == 1,
          "_0cee_2df4 flags OBJECT_NULL");
    ai_spec.word30 = 0xabcdu;
    CHECK(dm2_v1_skproject_0cee_2df4_creature_ai_word30(
              0x1234u, &ai_spec, &ai_receipt) == 1,
          "_0cee_2df4 returns AI spec word30");
    CHECK(ai_receipt.word30 == 0xabcdu && ai_receipt.valid,
          "_0cee_2df4 receipt carries word30");

    /* _19f0_124b level transition: stairs branch */
    memset(maps, 0, sizeof(maps));
    maps[0].map_id = 0;
    maps[0].world_x = 0;
    maps[0].world_y = 0;
    maps[0].width = 4;
    maps[0].height = 4;
    maps[0].tile_type_at_local = 0;
    memset(tiles, 0, sizeof(tiles));
    tiles[1 * 4 + 1] = (uint8_t)((3u << 5) | 0x04u); /* stairs, bit 2 set */
    x = 1;
    y = 1;
    CHECK(dm2_v1_skproject_19f0_124b_level_transition(
              &x, &y, 0, -1, 0x0100u,
              maps, 1, tiles, 4, 4, NULL, 0, NULL,
              &lt_receipt) == 1,
          "_19f0_124b admits matching stairs down");
    CHECK(lt_receipt.tile_type == 3 && lt_receipt.valid,
          "_19f0_124b reports stairs tile type");
    x = 1;
    y = 1;
    CHECK(dm2_v1_skproject_19f0_124b_level_transition(
              &x, &y, 0, 1, 0x0100u,
              maps, 1, tiles, 4, 4, NULL, 0, NULL,
              &lt_receipt) == 0,
          "_19f0_124b rejects stairs direction mismatch");
    CHECK(lt_receipt.blocked_stairs_direction == 1,
          "_19f0_124b flags stairs direction mismatch");

    /* _19f0_124b pit branch with ladder around */
    tiles[1 * 4 + 1] = (uint8_t)((2u << 5) | 0x08u); /* open pit, no occupant */
    x = 1;
    y = 1;
    CHECK(dm2_v1_skproject_19f0_124b_level_transition(
              &x, &y, 0, -1, 0x0108u,
              maps, 1, tiles, 4, 4, NULL, 1, NULL,
              &lt_receipt) == 1,
          "_19f0_124b admits open pit with ladder");
    CHECK(lt_receipt.requested_locate_other_level == 1,
          "_19f0_124b requests locate_other_level");

    /* _19f0_124b non-pit branch without ladder, ladder-down flag */
    tiles[1 * 4 + 1] = (uint8_t)((1u << 5) | 0x02u); /* floor, bit 1 set */
    x = 1;
    y = 1;
    CHECK(dm2_v1_skproject_19f0_124b_level_transition(
              &x, &y, 0, -1, 0x0010u,
              maps, 1, tiles, 4, 4, NULL, 0, NULL,
              &lt_receipt) == 1,
          "_19f0_124b sets ladder-down flag when no ladder");
    CHECK(lt_receipt.ladder_down_flag == 1 &&
              lt_receipt.requested_target_tile_check == 1,
          "_19f0_124b requests target pit check without target tile");

    /* _19f0_124b impassable target pit when target tile supplied */
    {
        uint8_t target_tile = (uint8_t)((2u << 5) | 0x08u | 0x01u);
        x = 1;
        y = 1;
        CHECK(dm2_v1_skproject_19f0_124b_level_transition(
                  &x, &y, 0, -1, 0x0010u,
                  maps, 1, tiles, 4, 4, NULL, 0, &target_tile,
                  &lt_receipt) == 0,
              "_19f0_124b rejects impassable target pit");
        CHECK(lt_receipt.rejected_target_pit_impassable == 1,
              "_19f0_124b flags impassable target pit");
    }

    /* _29ee_18eb level transition pair */
    tiles[1 * 4 + 1] = (uint8_t)((2u << 5) | 0x08u);
    CHECK(dm2_v1_skproject_29ee_18eb_level_transition_pair(
              1, 1, 0, maps, 1, tiles, 4, 4, NULL, 0, NULL,
              &lt_pair) == 1,
          "_29ee_18eb produces down/up transition pair");
    CHECK(lt_pair.down_transition.direction == -1 &&
              lt_pair.up_transition.direction == 1,
          "_29ee_18eb uses -1 down and +1 up");

    /* _29ee_00a3 init button group + black fill */
    memset(&group, 0xff, sizeof(group));
    expanded_rects[0] = (DM2_V1_SkprojectRect){0, 0, 10, 10};
    expanded_rects[1] = (DM2_V1_SkprojectRect){10, 0, 20, 10};
    CHECK(dm2_v1_skproject_29ee_00a3_init_button_group_black(
              &group, 1, expanded_rects, 2, 42, &black_receipt) == 1,
          "_29ee_00a3 initializes uninitialized group");
    CHECK(black_receipt.fill_black_requested &&
              black_receipt.init_receipt.valid,
          "_29ee_00a3 requests init and black fill");
    CHECK(group.dbidx == 42,
          "_29ee_00a3 sets group cache index");
    CHECK(dm2_v1_skproject_29ee_00a3_init_button_group_black(
              &group, 1, expanded_rects, 2, 42, &black_receipt) == 1,
          "_29ee_00a3 skips already-initialized group");
    CHECK(black_receipt.group_already_initialized == 1,
          "_29ee_00a3 flags already-initialized group");

    /* _29ee_0b2b command slot draw loop */
    CHECK(dm2_v1_skproject_29ee_0b2b_draw_command_slots(
              4, &slot_receipt) == 1,
          "_29ee_0b2b accepts slot count");
    CHECK(slot_receipt.drawn_slots == 4 &&
              slot_receipt.requested_draw_player_attack_dir == 1,
          "_29ee_0b2b requests slot draws and attack dir");
    CHECK(dm2_v1_skproject_29ee_0b2b_draw_command_slots(
              20, &slot_receipt) == 1,
          "_29ee_0b2b caps oversized slot count");
    CHECK(slot_receipt.drawn_slots == 16,
          "_29ee_0b2b caps slot count at 16");

    /* _0b36_0cbe blit dirty rects + cache free */
    memset(&group, 0, sizeof(group));
    group.dbidx = 7;
    group.group_size = 3;
    CHECK(dm2_v1_skproject_0b36_0cbe_blit_dirty_rects(
              &group, 1, &blit_receipt) == 1,
          "_0b36_0cbe blits dirty rects and frees cache");
    CHECK(blit_receipt.requested_blit_picture &&
              blit_receipt.requested_free_temp_cache_index &&
              blit_receipt.cache_index_cleared,
          "_0b36_0cbe requests blit and free");
    CHECK(group.dbidx == 0xffffu,
          "_0b36_0cbe clears group cache index");

    /* _0b36_129a draw string to cache */
    memset(&group, 0, sizeof(group));
    group.dbidx = 5;
    CHECK(dm2_v1_skproject_0b36_129a_draw_string_to_cache(
              &group, 10, 20, 1, 2, "ABC", &str_receipt) == 1,
          "_0b36_129a draws non-empty string");
    CHECK(str_receipt.requested_draw_string &&
              str_receipt.requested_dirty_rect &&
              str_receipt.metrics.valid,
          "_0b36_129a requests string draw and dirty rect");
    CHECK(dm2_v1_skproject_0b36_129a_draw_string_to_cache(
              &group, 10, 20, 1, 2, "", &str_receipt) == 0,
          "_0b36_129a rejects empty string");
    CHECK(str_receipt.blocked_empty_text == 1,
          "_0b36_129a flags empty string");

    /* _12b4_0092 SKWIN arrow panel highlight gate */
    CHECK(dm2_v1_skproject_12b4_0092_skwin_arrow_panel(
              0, 12, &arrow_receipt) == 1,
          "_12b4_0092 accepts inactive arrow panel");
    CHECK(arrow_receipt.requested_highlight == 0,
          "_12b4_0092 does not highlight when inactive");
    CHECK(dm2_v1_skproject_12b4_0092_skwin_arrow_panel(
              1, 12, &arrow_receipt) == 1,
          "_12b4_0092 highlights active arrow panel");
    CHECK(arrow_receipt.requested_highlight == 1 &&
              arrow_receipt.highlight_receipt.valid,
          "_12b4_0092 requests highlight receipt");
}

static void test_skwin_core_symbol_batch_cycle5(void)
{
    DM2_V1_SkprojectMementState state;
    DM2_V1_SkprojectTouchMementReceipt touch;
    DM2_V1_SkprojectRemoveMementReceipt remove;
    DM2_V1_SkprojectUnlinkFreeBlockReceipt unlink;
    DM2_V1_SkprojectInsertFreeBlockReceipt insert;
    DM2_V1_SkprojectCompactHeapReceipt compact;
    DM2_V1_Skproject3e74FreeCacheIndexReceipt free_ci;
    DM2_V1_SkprojectRecycleOrFreeCacheReceipt recycle;
    DM2_V1_SkprojectFindFreeCacheIndexReceipt find_ci;
    DM2_V1_SkprojectResetUsageCountersReceipt reset;

    dm2_v1_skproject_mement_state_init(&state);
    CHECK(state.lru_head < 0 && state.free_head < 0 &&
              state.next_free_ci == 0,
          "mement state init clears lists and starts cache index at zero");

    /* _3e74_4471 allocate cache indices */
    CHECK(dm2_v1_skproject_3e74_4471_find_free_cache_index(
              &state, &find_ci) == 1 && find_ci.valid &&
              find_ci.cache_index == 0,
          "_3e74_4471 returns the first free cache index");
    CHECK(dm2_v1_skproject_3e74_4471_find_free_cache_index(
              &state, &find_ci) == 1 && find_ci.cache_index == 1 &&
              find_ci.ci_count_after == 2u,
          "_3e74_4471 advances the cache-index cursor");

    /* Bind two mements to cache slots like ALLOC_CPXHEAP_MEM would. */
    state.mements[2].size = -100;
    state.mements[2].usage = 0;
    state.mements[2].cache_index = 0;
    state.cache_to_mement[0] = 2;
    state.mements[3].size = -200;
    state.mements[3].usage = 0;
    state.mements[3].cache_index = 1;
    state.cache_to_mement[1] = 3;

    /* _3e74_48c9 touch bumps usage and moves the block to LRU head. */
    CHECK(dm2_v1_skproject_3e74_48c9_touch_mement(
              &state, 2, &touch) == 1 && touch.valid &&
              touch.usage_after == 1u &&
              state.lru_head == 2,
          "_3e74_48c9 warms a cold mement and makes it LRU head");
    CHECK(dm2_v1_skproject_3e74_48c9_touch_mement(
              &state, 3, &touch) == 1 &&
              state.lru_head == 3 &&
              state.mements[3].lru_next == 2,
          "_3e74_48c9 pushes a second touched mement to the LRU head");
    CHECK(dm2_v1_skproject_3e74_48c9_touch_mement(
              &state, 2, &touch) == 1 &&
              touch.usage_after == 2u && state.lru_head == 2,
          "_3e74_48c9 re-promotes an already-warm mement");

    /* _3e74_44ad resets usage counters each tick. */
    CHECK(dm2_v1_skproject_3e74_44ad_reset_usage_counters(
              &state, 7, &reset) == 1 && reset.valid &&
              reset.reset_mements == 2u && state.lru_head < 0,
          "_3e74_44ad resets warm mements to cold and clears the LRU list");

    /* _3e74_4549 removes a mement from the LRU/tracking list. */
    state.mements[2].usage = 5;
    dm2_v1_skproject_mement_lru_push_front(&state, 2);
    CHECK(dm2_v1_skproject_3e74_4549_remove_mement_from_list(
              &state, 2, &remove) == 1 && remove.valid &&
              remove.removed_from_lru && remove.cleared_links &&
              state.mements[2].usage == 0xffffu &&
              state.mements[2].lru_prev < 0,
          "_3e74_4549 unlinks a mement and clears its tracking fields");

    /* _3e74_0d32 / _3e74_0c8c free-block list sorted by size descending. */
    state.mements[4].size = 500;
    state.mements[5].size = 300;
    state.mements[6].size = 400;
    CHECK(dm2_v1_skproject_3e74_0d32_insert_free_block(
              &state, 4, &insert) == 1 && insert.valid &&
              insert.became_head && state.free_head == 4,
          "_3e74_0d32 inserts the first free block at the list head");
    CHECK(dm2_v1_skproject_3e74_0d32_insert_free_block(
              &state, 5, &insert) == 1 && insert.became_tail &&
              state.mements[4].lru_next == 5,
          "_3e74_0d32 appends a smaller block at the tail");
    CHECK(dm2_v1_skproject_3e74_0d32_insert_free_block(
              &state, 6, &insert) == 1 && insert.inserted_after == 4 &&
              state.mements[4].lru_next == 6 &&
              state.mements[6].lru_next == 5,
          "_3e74_0d32 keeps the free list sorted by size descending");
    CHECK(dm2_v1_skproject_3e74_0c8c_unlink_free_block(
              &state, 6, &unlink) == 1 && unlink.valid &&
              unlink.unlinked &&
              state.mements[4].lru_next == 5,
          "_3e74_0c8c unlinks a free block while preserving order");

    /* _3e74_2b30 heap compaction receipt. */
    CHECK(dm2_v1_skproject_3e74_2b30_compact_heap(
              &state, &compact) == 1 && compact.valid &&
              compact.moved_blocks > 0u,
          "_3e74_2b30 reports heap compaction over allocated blocks");

    /* _3e74_583a free by cache index. */
    CHECK(dm2_v1_skproject_3e74_583a_free_cache_index(
              &state, 0, &free_ci) == 1 && free_ci.valid &&
              free_ci.found_mementi && free_ci.removed_from_lru &&
              state.cache_to_mement[0] == 0xffffu &&
              state.mements[2].usage == 0xffffu,
          "_3e74_583a resolves a cache index to a mement and clears it");

    /* _3e74_585a recycles a bound cache index or frees an unbound one. */
    CHECK(dm2_v1_skproject_3e74_585a_recycle_or_free_cache(
              &state, 1, 9, &recycle) == 1 && recycle.valid &&
              recycle.found_mementi && recycle.recycled &&
              state.mements[3].usage == 0u &&
              state.mements[3].raw_index == 9u &&
              state.cache_to_mement[1] == 0xffffu,
          "_3e74_585a recycles a bound mement to raw index yy");
    CHECK(dm2_v1_skproject_3e74_585a_recycle_or_free_cache(
              &state, 7, 0, &recycle) == 1 && recycle.valid &&
              !recycle.found_mementi && recycle.freed_cache_index,
          "_3e74_585a frees an unbound cache index");

    /* Invalid inputs are fail-closed. */
    CHECK(!dm2_v1_skproject_3e74_48c9_touch_mement(
              NULL, 0, &touch) && !touch.valid,
          "_3e74_48c9 rejects a NULL state");
    CHECK(!dm2_v1_skproject_3e74_4549_remove_mement_from_list(
              &state, 99, &remove) && !remove.valid,
          "_3e74_4549 rejects an out-of-range mement index");
    CHECK(!dm2_v1_skproject_3e74_583a_free_cache_index(
              &state, 99, &free_ci) && !free_ci.valid,
          "_3e74_583a rejects an out-of-range cache index");
}

static void test_skwin_core_symbol_batch_cycle6(void)
{
    DM2_V1_Skproject1C9A02C3Receipt ai_receipt;
    DM2_V1_SkprojectAnimFrame frames[8];
    DM2_V1_SkprojectRandomData randdat;
    DM2_V1_Skproject4937_01a9Receipt anim_receipt;
    DM2_V1_Skproject4937_000fReceipt w0_receipt;
    DM2_V1_Skproject2759_0155Receipt cmd155_receipt;
    DM2_V1_Skproject2759_01feReceipt cmd1fe_receipt;
    DM2_V1_Skproject2759_0e93Receipt hand_receipt;
    DM2_V1_Skproject24A5_0732Receipt str_receipt;
    DM2_V1_Skproject2E62SlotState slot_state;
    DM2_V1_Skproject2E62_03B5Receipt icon_receipt;
    uint8_t gdat_loadable[4];
    uint8_t cmdstr_cncm[4];
    uint8_t cmdstr_cnnc[4];
    int16_t selected_hands[4];
    uint16_t yy;

    /* _1c9a_02c3 creature AI pointer resolver */
    CHECK(dm2_v1_skproject_1c9a_02c3_creature_ai_pointer(
              1, 7, &ai_receipt) == 1,
          "_1c9a_02c3 resolves static-object branch");
    CHECK(ai_receipt.valid && ai_receipt.static_branch &&
              ai_receipt.offset == 8u && ai_receipt.creature_index == 7,
          "_1c9a_02c3 static branch carries offset and index");
    CHECK(dm2_v1_skproject_1c9a_02c3_creature_ai_pointer(
              0, 3, &ai_receipt) == 1,
          "_1c9a_02c3 resolves creature-info table branch");
    CHECK(ai_receipt.valid && ai_receipt.table_branch &&
              !ai_receipt.static_branch,
          "_1c9a_02c3 table branch flags non-static");

    /* _4937_000f animation sequence w0 low 10 bits */
    CHECK(dm2_v1_skproject_4937_000f_animation_w0(
              0xabcdu, &w0_receipt) == 1,
          "_4937_000f accepts sequence w0");
    CHECK(w0_receipt.valid && w0_receipt.result == (0xabcdu & 0x03ffu),
          "_4937_000f masks sequence w0 to low 10 bits");

    /* _4937_01a9 animation frame selector */
    memset(frames, 0, sizeof(frames));
    dm2_v1_skproject_random_init(&randdat);
    /* Starting yy = 0xffff resets to 0 and skips the advance read. */
    frames[0].w2 = 0x00f1u; /* stop immediately, probability 0xf */
    frames[0].b4 = 0x02u;
    yy = 0xffffu;
    CHECK(dm2_v1_skproject_4937_01a9_select_frame(
              0, &yy, frames, 8, &randdat, &anim_receipt) == 1,
          "_4937_01a9 selects from 0xffff base");
    CHECK(anim_receipt.valid && anim_receipt.output_si == 0u &&
              anim_receipt.result_di == 1 && anim_receipt.has_content == 1,
          "_4937_01a9 stops at frame 0 and reports content");
    /* Starting yy = 0 reads frames[0] advance and lands on frames[1]. */
    frames[0].w2 = 0x0001u; /* advance 1, probability 0 */
    frames[1].w2 = 0x00f1u; /* stop immediately, probability 0xf */
    frames[1].b4 = 0x02u;
    yy = 0u;
    CHECK(dm2_v1_skproject_4937_01a9_select_frame(
              0, &yy, frames, 8, &randdat, &anim_receipt) == 1,
          "_4937_01a9 advances from explicit base");
    CHECK(anim_receipt.valid && anim_receipt.output_si == 1u &&
              anim_receipt.result_di == 1 && anim_receipt.has_content == 1,
          "_4937_01a9 advances and reports content");
    /* Frame xx+0 with zero advance returns di=0 without consuming frames. */
    frames[0].w2 = 0x0000u;
    yy = 0u;
    CHECK(dm2_v1_skproject_4937_01a9_select_frame(
              0, &yy, frames, 8, &randdat, &anim_receipt) == 1,
          "_4937_01a9 handles zero frame advance");
    CHECK(anim_receipt.output_si == 0u && anim_receipt.result_di == 0,
          "_4937_01a9 returns zero when advance is zero");
    /* Missing random data fails closed when probability is not 0xf. */
    frames[0].w2 = 0x0001u; /* advance 1, probability 0 */
    frames[1].w2 = 0x0011u; /* advance 1, probability 1 (needs random) */
    yy = 0u;
    CHECK(dm2_v1_skproject_4937_01a9_select_frame(
              0, &yy, frames, 8, NULL, &anim_receipt) == 0,
          "_4937_01a9 fails closed without random source");
    CHECK(anim_receipt.blocked_missing_random == 1,
          "_4937_01a9 flags missing random source");
    CHECK(dm2_v1_skproject_4937_01a9_select_frame(
              0, &yy, NULL, 0, &randdat, &anim_receipt) == 0,
          "_4937_01a9 fails closed without frame table");

    /* _2759_0155 query object commands */
    memset(gdat_loadable, 0, sizeof(gdat_loadable));
    memset(cmdstr_cncm, 0, sizeof(cmdstr_cncm));
    memset(cmdstr_cnnc, 0, sizeof(cmdstr_cnnc));
    CHECK(dm2_v1_skproject_2759_0155_query_object_commands(
              1, 0x12u, 0x34u,
              gdat_loadable, cmdstr_cncm, cmdstr_cnnc,
              &cmd155_receipt) == 1,
          "_2759_0155 accepts OBJECT_NULL");
    CHECK(cmd155_receipt.valid && cmd155_receipt.found == 0,
          "_2759_0155 returns zero for OBJECT_NULL");
    gdat_loadable[1] = 1;
    cmdstr_cncm[1] = 1;
    cmdstr_cnnc[1] = 1;
    CHECK(dm2_v1_skproject_2759_0155_query_object_commands(
              0, 0x12u, 0x34u,
              gdat_loadable, cmdstr_cncm, cmdstr_cnnc,
              &cmd155_receipt) == 1,
          "_2759_0155 finds matching command at text 9");
    CHECK(cmd155_receipt.valid && cmd155_receipt.found == 1 &&
              cmd155_receipt.checked_count == 2u,
          "_2759_0155 reports found after checking text 8 and 9");

    /* _2759_01fe command validity for container/minion maps */
    CHECK(dm2_v1_skproject_2759_01fe_command_valid(
              45, 0, 0, 0, 0, 0, 0, &cmd1fe_receipt) == 1,
          "_2759_01fe accepts non-container record");
    CHECK(cmd1fe_receipt.valid && cmd1fe_receipt.result == 1,
          "_2759_01fe returns one for non-container");
    CHECK(dm2_v1_skproject_2759_01fe_command_valid(
              48, 1, 1, 2, 1, 50, 0, &cmd1fe_receipt) == 1,
          "_2759_01fe accepts CmKillMinion with existing minion");
    CHECK(cmd1fe_receipt.result == 1,
          "_2759_01fe returns one for kill-minion");
    CHECK(dm2_v1_skproject_2759_01fe_command_valid(
              45, 1, 1, 2, 1, 50, 0, &cmd1fe_receipt) == 1,
          "_2759_01fe rejects CmCallCarry on carry minion");
    CHECK(cmd1fe_receipt.result == 0,
          "_2759_01fe returns zero for wrong minion command");
    CHECK(dm2_v1_skproject_2759_01fe_command_valid(
              47, 1, 1, 1, 0, 0, 0xffffu, &cmd1fe_receipt) == 1,
          "_2759_01fe accepts CmCallScout on empty scout map");
    CHECK(cmd1fe_receipt.result == 1,
          "_2759_01fe returns one for call-scout");

    /* _2759_0e93 hand activation */
    selected_hands[0] = 0;
    selected_hands[1] = 1;
    selected_hands[2] = -1;
    selected_hands[3] = 0;
    CHECK(dm2_v1_skproject_2759_0e93_hand_activation(
              1, 1, selected_hands, 4, &hand_receipt) == 1,
          "_2759_0e93 matches hand in selected items");
    CHECK(hand_receipt.valid && hand_receipt.result == 1,
          "_2759_0e93 returns one when hand is selected");
    CHECK(dm2_v1_skproject_2759_0e93_hand_activation(
              2, 1, selected_hands, 4, &hand_receipt) == 1,
          "_2759_0e93 rejects unselected hand");
    CHECK(hand_receipt.result == 0,
          "_2759_0e93 returns zero when hand is not selected");

    /* _24a5_0732 centered viewport string */
    CHECK(dm2_v1_skproject_24a5_0732_draw_centered_vp_str(
              100, 50, "ABC", 24, 0, &str_receipt) == 1,
          "_24a5_0732 converts ASCII string");
    CHECK(str_receipt.valid &&
              str_receipt.converted[0] == 0x02u &&
              str_receipt.converted[1] == 0x20u &&
              str_receipt.converted[2] == 1u && /* A */
              str_receipt.converted[3] == 2u && /* B */
              str_receipt.converted[4] == 3u && /* C */
              str_receipt.draw_x == 100 - 12,
          "_24a5_0732 prefixes font bytes and centers string");
    CHECK(dm2_v1_skproject_24a5_0732_draw_centered_vp_str(
              100, 50, "abc", 24, 1, &str_receipt) == 1,
          "_24a5_0732 keeps MBCS string unchanged");
    CHECK(str_receipt.converted[0] == 'a' &&
              str_receipt.converted_len == 3u,
          "_24a5_0732 preserves MBCS bytes");
    CHECK(dm2_v1_skproject_24a5_0732_draw_centered_vp_str(
              100, 50, "", 0, 0, &str_receipt) == 0,
          "_24a5_0732 rejects empty string");

    /* _2e62_03b5 item icon update */
    memset(&slot_state, 0, sizeof(slot_state));
    slot_state.w6 = 0xffffu;
    CHECK(dm2_v1_skproject_2e62_03b5_item_icon_update(
              0, 0, 1, 5, 1, 0, 0x01u, 0x1234u, 0x56u,
              0x80u, 0x07u, &slot_state, &icon_receipt) == 1,
          "_2e62_03b5 updates hand slot state");
    CHECK(icon_receipt.valid && icon_receipt.state_changed == 1 &&
              icon_receipt.requested_draw_item_icon == 1 &&
              icon_receipt.state_after.w6 == 0x1234u &&
              icon_receipt.state_after.b3 == 0x07u &&
              icon_receipt.state_after.b4 == 0x56u,
          "_2e62_03b5 records new object, dbspec frame and cls2");
    CHECK(dm2_v1_skproject_2e62_03b5_item_icon_update(
              2, 3, 1, 5, 1, 0, 0x01u, 0x1234u, 0x56u,
              0x80u, 0x07u, &slot_state, &icon_receipt) == 1,
          "_2e62_03b5 early-exits for out-of-hand item on non-inventory player");
    CHECK(icon_receipt.early_return == 1,
          "_2e62_03b5 flags early return");
}

static void test_skwin_core_symbol_batch_cycle4(void)
{
    DM2_V1_SkprojectUiTrackingState state;
    DM2_V1_SkprojectMouseEventLockReceipt lock_receipt;
    DM2_V1_SkprojectMouseEventUnlockReceipt unlock_receipt;
    DM2_V1_SkprojectMouseTrackingResetReceipt reset_receipt;
    DM2_V1_SkprojectMouseTrackingContextReceipt ctx_receipt;
    DM2_V1_SkprojectUiTrackingInsertReceipt insert_receipt;
    DM2_V1_SkprojectUiTrackingRemoveReceipt remove_receipt;

    /* _443c_087c lock mouse event */
    dm2_v1_skproject_ui_tracking_state_init(&state);
    CHECK(dm2_v1_skproject_443c_087c_lock_mouse_event(
              &state, &lock_receipt) == 1,
          "_443c_087c locks mouse events");
    CHECK(lock_receipt.valid &&
              lock_receipt.lock_depth_before == 0 &&
              lock_receipt.lock_depth_after == 1,
          "_443c_087c increments event lock depth");

    /* _443c_0889 unlock mouse event */
    CHECK(dm2_v1_skproject_443c_0889_unlock_mouse_event(
              &state, &unlock_receipt) == 1,
          "_443c_0889 unlocks mouse events");
    CHECK(unlock_receipt.valid &&
              unlock_receipt.lock_depth_before == 1 &&
              unlock_receipt.lock_depth_after == 0 &&
              !unlock_receipt.underflow,
          "_443c_0889 decrements event lock depth");
    CHECK(dm2_v1_skproject_443c_0889_unlock_mouse_event(
              &state, &unlock_receipt) == 1,
          "_443c_0889 accepts unlock at zero");
    CHECK(unlock_receipt.underflow &&
              unlock_receipt.lock_depth_after == 0,
          "_443c_0889 flags underflow at zero depth");

    /* _443c_040e reset mouse tracking */
    dm2_v1_skproject_ui_tracking_state_init(&state);
    CHECK(dm2_v1_skproject_443c_040e_reset_mouse_tracking(
              &state, &reset_receipt) == 1,
          "_443c_040e resets mouse tracking");
    CHECK(reset_receipt.valid &&
              reset_receipt.hide_requested &&
              reset_receipt.show_requested &&
              reset_receipt.bounds_requested &&
              reset_receipt.reset_rect.w == 1 &&
              reset_receipt.reset_rect.h == 1,
          "_443c_040e requests hide/show/bounds and sets 1x1 rect");

    /* _443c_00a9 set tracking context */
    dm2_v1_skproject_ui_tracking_state_init(&state);
    CHECK(dm2_v1_skproject_443c_00a9_set_tracking_context(
              &state, 0x1234u, 10, 50, 20, 80, &ctx_receipt) == 1,
          "_443c_00a9 sets tracking context");
    CHECK(ctx_receipt.valid &&
              ctx_receipt.context_ref == 0x1234u &&
              ctx_receipt.track_start_x == 10 &&
              ctx_receipt.track_end_x == 50 &&
              ctx_receipt.track_start_y == 20 &&
              ctx_receipt.track_end_y == 80 &&
              ctx_receipt.bounds[2] == 41 &&
              ctx_receipt.bounds[3] == 61 &&
              ctx_receipt.bounds_mode == 0x20u,
          "_443c_00a9 stores ref, extents and cursor bounds");

    /* _443c_06b4 insert tracking object */
    dm2_v1_skproject_ui_tracking_state_init(&state);
    state.objects[0].id = 1;
    state.objects[0].priority = 5;
    state.objects[0].tracked = 0;
    state.objects[0].has_bounds = 0;
    CHECK(dm2_v1_skproject_443c_06b4_insert_tracking_object(
              &state, &state.objects[0], &insert_receipt) == 1,
          "_443c_06b4 inserts first tracking object");
    CHECK(insert_receipt.valid &&
              insert_receipt.inserted &&
              state.head == 0 &&
              state.objects[0].tracked == 1,
          "_443c_06b4 makes object head and marks tracked");

    state.objects[1].id = 2;
    state.objects[1].priority = 3;
    state.objects[1].tracked = 0;
    CHECK(dm2_v1_skproject_443c_06b4_insert_tracking_object(
              &state, &state.objects[1], &insert_receipt) == 1,
          "_443c_06b4 inserts lower-priority object after head");
    CHECK(insert_receipt.inserted &&
              insert_receipt.prev_id == 0 &&
              insert_receipt.next_id == -1 &&
              state.head == 0 &&
              state.objects[0].next == 1,
          "_443c_06b4 appends lower-priority object at tail");

    state.objects[2].id = 3;
    state.objects[2].priority = 7;
    state.objects[2].tracked = 0;
    CHECK(dm2_v1_skproject_443c_06b4_insert_tracking_object(
              &state, &state.objects[2], &insert_receipt) == 1,
          "_443c_06b4 inserts higher-priority object before head");
    CHECK(insert_receipt.inserted &&
              insert_receipt.prev_id == -1 &&
              insert_receipt.next_id == 0 &&
              state.head == 2 &&
              state.objects[2].next == 0 &&
              state.objects[0].prev == 2,
          "_443c_06b4 orders by descending priority");

    CHECK(dm2_v1_skproject_443c_06b4_insert_tracking_object(
              &state, &state.objects[0], &insert_receipt) == 0,
          "_443c_06b4 rejects already-tracked object");
    CHECK(insert_receipt.blocked_already_tracked,
          "_443c_06b4 flags already-tracked object");

    /* _443c_07d5 remove tracking object */
    CHECK(dm2_v1_skproject_443c_07d5_remove_tracking_object(
              &state, &state.objects[1], &remove_receipt) == 1,
          "_443c_07d5 removes tail tracking object");
    CHECK(remove_receipt.valid &&
              remove_receipt.removed &&
              remove_receipt.prev_id == 0 &&
              remove_receipt.next_id == -1 &&
              state.head == 2 &&
              state.objects[0].next == -1 &&
              !state.objects[1].tracked,
          "_443c_07d5 unlinks tail and marks untracked");

    CHECK(dm2_v1_skproject_443c_07d5_remove_tracking_object(
              &state, &state.objects[1], &remove_receipt) == 0,
          "_443c_07d5 rejects untracked object");
    CHECK(remove_receipt.blocked_not_tracked,
          "_443c_07d5 flags untracked object");
}

static void test_skwin_core_symbol_batch_cycle7(void)
{
    DM2_V1_SkprojectUiNodeRef root = { DM2_V1_SKPROJECT_UI_PRED_RETURN_1, 0u, 0u };
    DM2_V1_SkprojectUiLeafMeta leaf_meta[2];
    DM2_V1_SkprojectRect expanded_rects[2];
    DM2_V1_SkprojectRect topleft_rects[2];
    DM2_V1_SkprojectRect out_rect;
    DM2_V1_SkprojectUiResolveRectReceipt rect_receipt;
    DM2_V1_SkprojectUiChildListReceipt child_receipt;
    DM2_V1_SkprojectUiActionListReceipt action_list_receipt;
    DM2_V1_SkprojectUiRuntimeState runtime;
    DM2_V1_SkprojectUiPendingRedrawReceipt redraw_receipt;
    DM2_V1_SkprojectCommandSlotLoopReceipt slot_receipt;
    DM2_V1_Skproject0B36ButtonGroup group;
    DM2_V1_Skproject0B36DrawStringReceipt str_receipt;
    uint8_t child_bytes[2] = { 0u, 0u };

    /* DM2_1031_01d5 resolve rect */
    expanded_rects[0] = (DM2_V1_SkprojectRect){ 10, 20, 30, 40 };
    topleft_rects[0] = (DM2_V1_SkprojectRect){ 0, 0, 0, 0 };
    topleft_rects[1] = (DM2_V1_SkprojectRect){ 0, 0, 0, 0 };
    CHECK(dm2_v1_skproject_1031_01d5_resolve_rect(
              0u, expanded_rects, 1, topleft_rects, 2,
              &out_rect, &rect_receipt) == 1,
          "DM2_1031_01d5 resolves rect with bounded tables");
    CHECK(rect_receipt.valid && out_rect.x == 10 && out_rect.y == 20,
          "DM2_1031_01d5 receipt carries resolved rect");

    /* DM2_1031_023b child list */
    CHECK(dm2_v1_skproject_1031_023b_child_list(
              child_bytes, sizeof(child_bytes), &root,
              &child_receipt) == 1,
          "DM2_1031_023b reads child-list cursor");
    CHECK(child_receipt.valid && child_receipt.child_offset == 0u,
          "DM2_1031_023b returns bounded cursor");

    /* DM2_1031_024c action list */
    memset(leaf_meta, 0, sizeof(leaf_meta));
    leaf_meta[0].w2 = 0x1234u;
    root.w2 = 0u;
    CHECK(dm2_v1_skproject_1031_024c_action_list(
              &root, leaf_meta, 2, &action_list_receipt) == 1,
          "DM2_1031_024c looks up action index from leaf");
    CHECK(action_list_receipt.valid && action_list_receipt.action_index == 0x1234u,
          "DM2_1031_024c returns action index");

    /* DM2_1031_04f5 clear pending redraw */
    dm2_v1_skproject_ui_runtime_state_init(&runtime);
    runtime.pending_capture_redraw = 1u;
    CHECK(dm2_v1_skproject_1031_04f5_clear_pending_redraw(
              &runtime, &redraw_receipt) == 1,
          "DM2_1031_04f5 clears pending redraw gate");
    CHECK(redraw_receipt.valid && redraw_receipt.cleared_pending_capture_redraw &&
              redraw_receipt.requested_guidraw_29ee_000f,
          "DM2_1031_04f5 requests source redraw hook");

    /* DM2_29ee_0b2b command slot draw loop */
    CHECK(dm2_v1_skproject_29ee_0b2b_draw_command_slots(
              4, &slot_receipt) == 1,
          "DM2_29ee_0b2b draws bounded command slots");
    CHECK(slot_receipt.valid && slot_receipt.drawn_slots == 4,
          "DM2_29ee_0b2b reports four drawn slots");

    /* DM2_0b36_129a draw string to cache */
    memset(&group, 0, sizeof(group));
    group.dbidx = 1;
    CHECK(dm2_v1_skproject_0b36_129a_draw_string_to_cache(
              &group, 0, 0, 1, 1, "X", &str_receipt) == 1,
          "DM2_0b36_129a draws string to cache");
    CHECK(str_receipt.valid && str_receipt.requested_draw_string,
          "DM2_0b36_129a requests string draw");
}

static void test_skwin_core_symbol_batch_cycle8(void)
{
    DM2_V1_SkprojectUiPredicateState pred_state;
    DM2_V1_SkprojectUiRuntimeState runtime;
    DM2_V1_SkprojectUiNodeRef nodes[4];
    DM2_V1_SkprojectUiNodeRef roots[1];
    DM2_V1_SkprojectUiLeafMeta leaf_meta[2];
    DM2_V1_SkprojectUiAction actions[4];
    DM2_V1_SkprojectUiClickRectNode clickrects[2];
    DM2_V1_SkprojectRect expanded_rects[2];
    DM2_V1_SkprojectRect topleft_rects[2];
    DM2_V1_SkprojectUiPredicateReceipt pred_receipt;
    DM2_V1_SkprojectUiResetCaptureReceipt reset_receipt;
    DM2_V1_SkprojectUiSelectTreeReceipt tree_receipt;
    DM2_V1_SkprojectUiSearchActionReceipt search_receipt;
    DM2_V1_SkprojectUiQueueEventReceipt queue_receipt;
    DM2_V1_SkprojectUiTableRemapReceipt remap_receipt;
    DM2_V1_SkprojectUiMagicalMapClickReceipt map_receipt;
    uint8_t child_bytes[4] = { 0u, 0x80u, 0u, 0x80u };
    uint8_t table1d3cd0[4] = { 0x80u, 0u, 0x80u, 0u };
    DM2_V1_SkprojectUiNodeRef table1d3ba0[2];
    DM2_V1_SkprojectUiNodeRef table1d3ed5[2];
    DM2_V1_SkprojectUiAction v1d338c[4];
    DM2_V1_SkprojectUiAction v1d39bc[2];
    int16_t capture_count;
    uint8_t item_record[8];
    uint8_t minion_record[4];

    memset(&pred_state, 0, sizeof(pred_state));
    memset(&runtime, 0, sizeof(runtime));
    runtime.active_tree = 0u;
    runtime.saved_tree = 1u;

    /* gate_1031 case 0 (RETURN_1) always true. */
    nodes[0] = (DM2_V1_SkprojectUiNodeRef){ 0u, 0u, 0u };
    CHECK(dm2_v1_skproject_gate_1031(
              0u, &pred_state, &nodes[0], &pred_receipt) == 1,
          "gate_1031 case 0 returns true");
    CHECK(pred_receipt.valid && pred_receipt.result,
          "gate_1031 receipt records true result");

    /* DM2_10777 resets capture state and requests squad recompute. */
    dm2_v1_skproject_ui_runtime_state_init(&runtime);
    runtime.show_item_stats = 1u;
    runtime.capture_item_stats = 1u;
    runtime.pending_capture_redraw = 1u;
    capture_count = 2;
    CHECK(dm2_v1_skproject_10777_reset_capture(
              &runtime, &capture_count, &reset_receipt) == 1,
          "DM2_10777 resets capture state");
    CHECK(reset_receipt.cleared_vcaptures &&
              reset_receipt.cleared_pending_redraw &&
              reset_receipt.requested_squad_recompute &&
              reset_receipt.requested_mouse_release_capture &&
              capture_count == 1,
          "DM2_10777 clears captures and requests release");

    /* DM2_107B0 reselects active_tree; DM2_1031_06a5 reselects saved_tree. */
    roots[0] = (DM2_V1_SkprojectUiNodeRef){ 0u, 0u, 1u };
    nodes[0] = (DM2_V1_SkprojectUiNodeRef){ 0u, 0u, 0u };
    nodes[1] = (DM2_V1_SkprojectUiNodeRef){ 0x80u, 0u, 0u };
    child_bytes[0] = 1u;
    child_bytes[1] = 0x80u;
    memset(leaf_meta, 0, sizeof(leaf_meta));
    memset(clickrects, 0, sizeof(clickrects));
    dm2_v1_skproject_ui_runtime_state_init(&runtime);
    runtime.active_tree = 0u;
    CHECK(dm2_v1_skproject_107b0_select_active_tree(
              &runtime, &pred_state, roots, 1, nodes, 2, child_bytes,
              sizeof(child_bytes), leaf_meta, 2, clickrects, 2,
              &tree_receipt) == 1,
          "DM2_107B0 selects active tree");
    CHECK(tree_receipt.valid && tree_receipt.selected_tree == 0u,
          "DM2_107B0 receipt names active tree");

    dm2_v1_skproject_ui_runtime_state_init(&runtime);
    runtime.saved_tree = 0u;
    CHECK(dm2_v1_skproject_1031_06a5_select_saved_tree(
              &runtime, &pred_state, roots, 1, nodes, 2, child_bytes,
              sizeof(child_bytes), leaf_meta, 2, clickrects, 2,
              &tree_receipt) == 1,
          "DM2_1031_06a5 selects saved tree");
    CHECK(tree_receipt.valid && tree_receipt.selected_tree == 0u,
          "DM2_1031_06a5 receipt names saved tree");

    /* DM2_1031_06b3 searches action lists by low-three-bit code.
       Parent RETURN_1 (predicate 0) gates children with case 0+5 = 5
       (_1031_009e), which reads player_at_position.  Set up a leaf whose
       action list contains the searched code. */
    memset(&pred_state, 0, sizeof(pred_state));
    pred_state.player_at_position[0] = 1;
    pred_state.player_dir = 0u;
    roots[0] = (DM2_V1_SkprojectUiNodeRef){ 0u, 0u, 0u };
    nodes[0] = (DM2_V1_SkprojectUiNodeRef){ 0u, 0u, 0u };
    nodes[1] = (DM2_V1_SkprojectUiNodeRef){ 0u, 0u, 0u };
    child_bytes[0] = 1u;
    child_bytes[1] = 0x80u;
    memset(leaf_meta, 0, sizeof(leaf_meta));
    leaf_meta[0].w4 = 0u;
    memset(actions, 0, sizeof(actions));
    actions[0].w0 = 3u; /* code 3 */
    actions[0].w2 = 0u;
    actions[0].w4 = 0xABu;
    CHECK(dm2_v1_skproject_1031_06b3_search_action(
              &pred_state, &roots[0], nodes, 2, child_bytes,
              sizeof(child_bytes), leaf_meta, 1, actions, 1, 3u,
              &search_receipt) == 1,
          "DM2_1031_06b3 finds action code 3");
    CHECK(search_receipt.found &&
              search_receipt.found_action_index == 0u &&
              search_receipt.found_leaf_index == 0u,
          "DM2_1031_06b3 receipt names found action");

    /* DM2_1031_0781 resolves the rect for a found action. */
    expanded_rects[0] = (DM2_V1_SkprojectRect){ 10, 20, 30, 40 };
    topleft_rects[0] = (DM2_V1_SkprojectRect){ 0, 0, 0, 0 };
    dm2_v1_skproject_ui_runtime_state_init(&runtime);
    runtime.active_tree = 0u;
    CHECK(dm2_v1_skproject_1031_0781_queue_event_by_code(
              &runtime, &pred_state, 3u, roots, 1, nodes, 2, child_bytes,
              sizeof(child_bytes), leaf_meta, 1, actions, 1, expanded_rects, 1,
              topleft_rects, 1, &queue_receipt) == 1,
          "DM2_1031_0781 queues event for found action");
    CHECK(queue_receipt.found_action &&
              queue_receipt.queued_rect.x == 10 &&
              queue_receipt.queued_rect.y == 20 &&
              queue_receipt.queued_action_value == 0xABu,
          "DM2_1031_0781 receipt carries resolved rect and value");

    /* DM2_1031_07d6 remaps UI table indices. */
    memset(leaf_meta, 0, sizeof(leaf_meta));
    leaf_meta[0].w2 = 0u;
    leaf_meta[0].w4 = 0u;
    memset(v1d338c, 0, sizeof(v1d338c));
    v1d338c[0].w0 = 0x8000u;
    memset(v1d39bc, 0, sizeof(v1d39bc));
    v1d39bc[0].w0 = 0x8000u;
    memset(table1d3ba0, 0, sizeof(table1d3ba0));
    table1d3ba0[0].b0 = 0x80u;
    table1d3ba0[0].w2 = 0u;
    memset(table1d3ed5, 0, sizeof(table1d3ed5));
    table1d3ed5[0].b0 = 0x80u;
    table1d3ed5[0].w2 = 2u;
    memset(clickrects, 0, sizeof(clickrects));
    CHECK(dm2_v1_skproject_1031_07d6_remap_ui_tables(
              leaf_meta, 1, clickrects, 2, v1d338c, 4, v1d39bc, 2,
              table1d3cd0, 4, table1d3ba0, 2, table1d3ed5, 2,
              &remap_receipt) == 1,
          "DM2_1031_07d6 remaps UI tables");
    CHECK(remap_receipt.valid && remap_receipt.remapped_v1d338c &&
              remap_receipt.remapped_v1d39bc &&
              remap_receipt.remapped_table1d3cd0 &&
              remap_receipt.remapped_clickrects &&
              leaf_meta[0].w2 == 0u && leaf_meta[0].w4 == 0u &&
              table1d3ba0[0].w2 == 0u && table1d3ed5[0].w2 == 2u,
          "DM2_1031_07d6 receipt records remap");

    /* DM2_CLICK_MAGICAL_MAP_AT validates map-chip click and computes target. */
    memset(item_record, 0, sizeof(item_record));
    item_record[4] = 0x00u;
    item_record[5] = 0x20u; /* bits 13-15 == 0x2 -> map chip */
    memset(minion_record, 0, sizeof(minion_record));
    CHECK(dm2_v1_skproject_click_magical_map_at(
              100, 80, 0x55u, 1u, 0u, 0x1234u, item_record,
              sizeof(item_record), minion_record, sizeof(minion_record),
              50, 40, 10, 2, 0, 0, 0, 5, 6, 0, -1, -1, -1, NULL, 0, 0,
              NULL, &map_receipt) == 1,
          "DM2_CLICK_MAGICAL_MAP_AT accepts valid map-chip click");
    CHECK(map_receipt.valid && map_receipt.requested_set_destination &&
              map_receipt.requested_change_map,
          "DM2_CLICK_MAGICAL_MAP_AT receipt requests destination");

    CHECK(dm2_v1_skproject_click_magical_map_at(
              100, 80, 0x54u, 1u, 0u, 0x1234u, item_record,
              sizeof(item_record), minion_record, sizeof(minion_record),
              50, 40, 10, 2, 0, 0, 0, 5, 6, 0, -1, -1, -1, NULL, 0, 0,
              NULL, &map_receipt) == 0,
          "DM2_CLICK_MAGICAL_MAP_AT rejects wrong UI code");
    CHECK(!map_receipt.valid && map_receipt.blocked_not_magical_map,
          "DM2_CLICK_MAGICAL_MAP_AT receipt records wrong-code block");
}

static void fixture_querydb_wall_ornate_loader(
    DM2_V1_AssetLoader *loader,
    uint8_t data[64],
    uint32_t raw_offsets[4],
    uint32_t raw_sizes[4],
    DM2_V1_GdatEntry entries[4])
{
    memset(loader, 0, sizeof(*loader));
    memset(data, 0, sizeof(data[0]) * 64u);

    /* raw 0: wall ornate cls2=7 field 10 alcove word value 0x0001 */
    data[0] = 0x01u;
    data[1] = 0x00u;
    /* raw 1: wall ornate cls2=7 field 12 spring word value 0x0000 */
    data[2] = 0x00u;
    data[3] = 0x00u;
    /* raw 2: wall ornate cls2=8 field 10 alcove word value 0x0000 */
    data[4] = 0x00u;
    data[5] = 0x00u;
    /* raw 3: wall ornate cls2=8 field 12 spring word value 0x0001 */
    data[6] = 0x01u;
    data[7] = 0x00u;

    raw_offsets[0] = 0u;
    raw_offsets[1] = 2u;
    raw_offsets[2] = 4u;
    raw_offsets[3] = 6u;
    raw_sizes[0] = 2u;
    raw_sizes[1] = 2u;
    raw_sizes[2] = 2u;
    raw_sizes[3] = 2u;

    entries[0].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[0].cls2 = 7u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[0].cls4 = 10u;
    entries[0].data_index = 0u;

    entries[1].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[1].cls2 = 7u;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[1].cls4 = 12u;
    entries[1].data_index = 1u;

    entries[2].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[2].cls2 = 8u;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[2].cls4 = 10u;
    entries[2].data_index = 2u;

    entries[3].cls1 = DM2_GDAT_CATEGORY_WALL_GFX;
    entries[3].cls2 = 8u;
    entries[3].cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entries[3].cls4 = 12u;
    entries[3].data_index = 3u;

    loader->data = data;
    loader->data_size = 64u;
    loader->loaded = 1;
    loader->category_count = DM2_GDAT_CATEGORY_LIMIT + 1;
    loader->raw_data_count = 4u;
    loader->raw_offsets = raw_offsets;
    loader->raw_sizes = raw_sizes;
    loader->entries = entries;
    loader->entry_count = 4u;
}

static void test_skwin_core_symbol_batch_cycle9(void)
{
    DM2_V1_AssetLoader loader;
    uint8_t data[64];
    uint32_t raw_offsets[4];
    uint32_t raw_sizes[4];
    DM2_V1_GdatEntry entries[4];
    DM2_V1_Skproject098d000fReceipt receipt_098d;
    DM2_V1_SkprojectCls1CriticalForLoadReceipt receipt_cls1;
    DM2_V1_SkprojectGdatDynBuffReceipt receipt_dyn;
    DM2_V1_SkprojectWallOrnateAlcoveReceipt receipt_alcove;
    DM2_V1_SkprojectTileBlockedReceipt receipt_tile;
    DM2_V1_SkprojectRebirthAltarReceipt receipt_altar;
    DM2_V1_SkprojectWallOrnateSpringReceipt receipt_spring;
    int16_t w1, w2;
    uint32_t dbidx_out;

    /* DM2_query_098d_000f converts 5x5 position to coarse grid. */
    CHECK(dm2_v1_skproject_098d_000f(1, 2, 7, &w1, &w2,
                                     &receipt_098d) == 1,
          "DM2_query_098d_000f returns success");
    CHECK(w1 == 6 && w2 == 9,
          "DM2_query_098d_000f computes w1=7%5+4*1, w2=7/5+4*2");
    CHECK(receipt_098d.valid && receipt_098d.w1 == 6 &&
              receipt_098d.w2 == 9,
          "DM2_query_098d_000f receipt records outputs");
    CHECK(dm2_v1_skproject_098d_000f(0, 0, 0, &w1, &w2, NULL) == 1 &&
              w1 == 0 && w2 == 0,
          "DM2_query_098d_000f works with NULL receipt");

    /* DM2_IS_CLS1_CRITICAL_FOR_LOAD flags categories 0x1b, 0x06, 0x05. */
    CHECK(dm2_v1_skproject_is_cls1_critical_for_load(0x1b, &receipt_cls1),
          "DM2_IS_CLS1_CRITICAL_FOR_LOAD accepts 0x1b");
    CHECK(receipt_cls1.valid && receipt_cls1.critical,
          "DM2_IS_CLS1_CRITICAL_FOR_LOAD receipt records critical");
    CHECK(dm2_v1_skproject_is_cls1_critical_for_load(0x05, &receipt_cls1),
          "DM2_IS_CLS1_CRITICAL_FOR_LOAD accepts 0x05");
    CHECK(!dm2_v1_skproject_is_cls1_critical_for_load(0x09, &receipt_cls1),
          "DM2_IS_CLS1_CRITICAL_FOR_LOAD rejects 0x09");
    CHECK(receipt_cls1.valid && !receipt_cls1.critical,
          "DM2_IS_CLS1_CRITICAL_FOR_LOAD receipt records non-critical");

    /* DM2_QUERY_GDAT_DYN_BUFF records the source allocation branch. */
    CHECK(dm2_v1_skproject_query_gdat_dyn_buff(
              0x20007u, 0, 0, 0, 100u, &dbidx_out, &receipt_dyn) == 1,
          "DM2_QUERY_GDAT_DYN_BUFF initial path succeeds");
    CHECK(receipt_dyn.valid &&
              receipt_dyn.path_taken ==
                  DM2_V1_SKPROJECT_GDAT_DYN_BUFF_PATH_INITIAL &&
              receipt_dyn.requested_size == 106u &&
              receipt_dyn.loaded_raw_data,
          "DM2_QUERY_GDAT_DYN_BUFF initial path receipt");
    CHECK(dm2_v1_skproject_query_gdat_dyn_buff(
              0x20007u, 1, 1, 1, 100u, &dbidx_out, &receipt_dyn) == 1,
          "DM2_QUERY_GDAT_DYN_BUFF cache path succeeds");
    CHECK(receipt_dyn.valid &&
              receipt_dyn.path_taken ==
                  DM2_V1_SKPROJECT_GDAT_DYN_BUFF_PATH_CACHE &&
              receipt_dyn.dbidx_out == 7u &&
              receipt_dyn.allocated_gfx256,
          "DM2_QUERY_GDAT_DYN_BUFF cache path receipt");
    CHECK(dm2_v1_skproject_query_gdat_dyn_buff(
              0x20007u, 1, 0, 0, 100u, &dbidx_out, &receipt_dyn) == 1,
          "DM2_QUERY_GDAT_DYN_BUFF cpx path succeeds");
    CHECK(receipt_dyn.valid &&
              receipt_dyn.path_taken ==
                  DM2_V1_SKPROJECT_GDAT_DYN_BUFF_PATH_CPX &&
              receipt_dyn.requested_size == 100u &&
              receipt_dyn.loaded_raw_data &&
              receipt_dyn.allocation1_called,
          "DM2_QUERY_GDAT_DYN_BUFF cpx path receipt");

    /* DM2_IS_WALL_ORNATE_ALCOVE reads GDAT category 9, type 11, field 10. */
    fixture_querydb_wall_ornate_loader(&loader, data, raw_offsets,
                                       raw_sizes, entries);
    CHECK(dm2_v1_skproject_is_wall_ornate_alcove(
              7u, 1u, &receipt_alcove),
          "DM2_IS_WALL_ORNATE_ALCOVE detects alcove for cls2=7");
    CHECK(receipt_alcove.valid && receipt_alcove.alcove_flag &&
              receipt_alcove.data_index == 1u,
          "DM2_IS_WALL_ORNATE_ALCOVE receipt records alcove flag");
    CHECK(!dm2_v1_skproject_is_wall_ornate_alcove(
              8u, 0u, &receipt_alcove),
          "DM2_IS_WALL_ORNATE_ALCOVE rejects non-alcove cls2=8");
    CHECK(!dm2_v1_skproject_is_wall_ornate_alcove(
              0xffu, 1u, &receipt_alcove) &&
              receipt_alcove.blocked_invalid_cls2,
          "DM2_IS_WALL_ORNATE_ALCOVE rejects invalid cls2");

    /* DM2_IS_TILE_BLOCKED encodes the source tile-type predicate. */
    CHECK(dm2_v1_skproject_is_tile_blocked(0x00u, &receipt_tile) == 1 &&
              receipt_tile.valid && receipt_tile.branch == 1 &&
              receipt_tile.blocked,
          "DM2_IS_TILE_BLOCKED blocks tile type 0");
    CHECK(dm2_v1_skproject_is_tile_blocked(0x20u, &receipt_tile) == 0 &&
              receipt_tile.branch == 1 && !receipt_tile.blocked,
          "DM2_IS_TILE_BLOCKED admits tile type 0x20");
    CHECK(dm2_v1_skproject_is_tile_blocked(0x80u, &receipt_tile) == 0 &&
              receipt_tile.valid && receipt_tile.branch == 2,
          "DM2_IS_TILE_BLOCKED admits tile type 0x80");
    CHECK(dm2_v1_skproject_is_tile_blocked(0xe0u, &receipt_tile) == 1 &&
              receipt_tile.branch == 4 && receipt_tile.blocked,
          "DM2_IS_TILE_BLOCKED blocks tile type 0xe0");
    CHECK(dm2_v1_skproject_is_tile_blocked(0x41u, &receipt_tile) == 0 &&
              receipt_tile.branch == 1 && !receipt_tile.blocked,
          "DM2_IS_TILE_BLOCKED admits tile type 0x41");
    CHECK(dm2_v1_skproject_is_tile_blocked(0x60u, &receipt_tile) == 0 &&
              receipt_tile.branch == 1 && !receipt_tile.blocked,
          "DM2_IS_TILE_BLOCKED admits tile type 0x60");
    CHECK(dm2_v1_skproject_is_tile_blocked(0xc4u, &receipt_tile) == 0 &&
              receipt_tile.branch == 5,
          "DM2_IS_TILE_BLOCKED admits tile type 0xc4");
    CHECK(dm2_v1_skproject_is_tile_blocked(0xc1u, &receipt_tile) == 0 &&
              receipt_tile.branch == 5,
          "DM2_IS_TILE_BLOCKED admits tile type 0xc1");
    CHECK(dm2_v1_skproject_is_tile_blocked(0xc0u, &receipt_tile) == 1 &&
              receipt_tile.branch == 5 && receipt_tile.blocked,
          "DM2_IS_TILE_BLOCKED blocks tile type 0xc0");

    /* DM2_IS_REBIRTH_ALTAR branches on record byte 2 and map header. */
    CHECK(dm2_v1_skproject_is_rebirth_altar(
              0x01u, 0x00u, 0x01u, 0x3000u,
              &receipt_altar) == 1,
          "DM2_IS_REBIRTH_ALTAR returns rebirth value when bit 0 set");
    CHECK(receipt_altar.valid && receipt_altar.altar_value == 3 &&
              receipt_altar.used_map_header_path,
          "DM2_IS_REBIRTH_ALTAR receipt records upper nibble value");
    CHECK(dm2_v1_skproject_is_rebirth_altar(
              0x00u, 0x80u, 0x00u, 0x0400u,
              &receipt_altar) == 1,
          "DM2_IS_REBIRTH_ALTAR returns rebirth value when bit 7 set");
    CHECK(receipt_altar.valid && receipt_altar.altar_value == 4,
          "DM2_IS_REBIRTH_ALTAR receipt records shifted value");
    CHECK(dm2_v1_skproject_is_rebirth_altar(
              0x00u, 0x00u, 0x00u, 0x0000u,
              &receipt_altar) == 0 &&
              receipt_altar.altar_value == -1,
          "DM2_IS_REBIRTH_ALTAR returns -1 when not rebirth altar");

    /* DM2_IS_WALL_ORNATE_SPRING reads GDAT category 9, type 11, field 12. */
    CHECK(!dm2_v1_skproject_is_wall_ornate_spring(
              7u, 0u, &receipt_spring),
          "DM2_IS_WALL_ORNATE_SPRING rejects non-spring cls2=7");
    CHECK(receipt_spring.valid && !receipt_spring.spring_flag &&
              receipt_spring.data_index == 0u,
          "DM2_IS_WALL_ORNATE_SPRING receipt records zero spring flag");
    CHECK(dm2_v1_skproject_is_wall_ornate_spring(
              8u, 1u, &receipt_spring),
          "DM2_IS_WALL_ORNATE_SPRING detects spring for cls2=8");
    CHECK(receipt_spring.valid && receipt_spring.spring_flag &&
              receipt_spring.data_index == 1u,
          "DM2_IS_WALL_ORNATE_SPRING receipt records spring flag");
}

static void put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static size_t build_word_square_fixture(uint8_t *buf, size_t cap,
                                        uint16_t center,
                                        uint16_t north,
                                        uint16_t east)
{
    const size_t header_size = 44u;
    const size_t desc_count = 28u;
    const size_t desc_size = 16u;
    const size_t tile_base = header_size + desc_count * desc_size;
    uint8_t *desc;

    if (cap < tile_base + 18u) return 0u;
    memset(buf, 0, cap);
    put16le(buf + 2, 0x4731u);
    put16le(buf + 4, (uint16_t)header_size);
    buf[6] = 1u;
    desc = buf + header_size;
    put16le(desc + 0, 0u);
    put16le(desc + 4, (uint16_t)(((3u - 1u) << 5) | (3u - 1u)));
    put16le(desc + 12, 3u);
    put16le(desc + 14, 3u);

    for (int i = 0; i < 9; ++i) {
        put16le(buf + tile_base + (size_t)i * 2u, DM2_SQUARE_FLOOR);
    }
    put16le(buf + tile_base + ((size_t)(1 * 3 + 1) * 2u), center);
    put16le(buf + tile_base + ((size_t)(1 * 3 + 0) * 2u), north);
    put16le(buf + tile_base + ((size_t)(2 * 3 + 1) * 2u), east);
    return tile_base + 18u;
}

static void test_skwin_core_symbol_batch_cycle10(void)
{
    DM2_V1_SkprojectGetCreatureAtReceipt creature_receipt;
    DM2_V1_SkprojectFindLadderAroundReceipt ladder_receipt;
    DM2_V1_SkprojectGetPlayerAtPositionReceipt player_receipt;
    DM2_V1_SkprojectDirFrom5x5PosReceipt dir_receipt;
    DM2_V1_SkprojectGetGlobVarReceipt glob_receipt;
    DM2_V1_SkprojectGetCreatureWeightReceipt weight_receipt;
    DM2_V1_SkprojectConvertPalette256Receipt palette_receipt;
    int8_t player_at_position[4] = { 3, -1, 1, 0 };
    uint16_t global_words[64] = { 0 };
    uint8_t src_rgb8[256 * 3];
    uint8_t dst_rgb6[256][3];
    uint8_t xlat[256];
    uint8_t dir;
    int16_t creature;
    int8_t player;
    uint16_t value;
    uint16_t weight;
    DM2_V1_DungeonData dungeon;
    uint8_t dungeon_buf[1024];
    size_t dungeon_size;

    /* DM2_GET_CREATURE_AT fail-closed when dungeon state is missing. */
    CHECK(!dm2_v1_skproject_get_creature_at(
              NULL, NULL, 0, 1, 2, &creature, &creature_receipt) &&
              creature_receipt.blocked_missing_pool_set &&
              creature == DM2_V1_RECORD_HANDLE_NULL,
          "DM2_GET_CREATURE_AT rejects missing pool set");

    /* DM2_FIND_LADDAR_AROUND wraps the proven ladder search. */
    memset(&dungeon, 0, sizeof(dungeon));
    dungeon_size = build_word_square_fixture(
        dungeon_buf, sizeof(dungeon_buf),
        0x0006u /* ladder up */, 0x0005u /* ladder down */,
        DM2_SQUARE_FLOOR);
    CHECK(dungeon_size != 0u &&
              dm2_v1_dungeon_load(&dungeon, dungeon_buf,
                                  (int)dungeon_size) == 0,
          "word-square fixture loads for ladder search");
    CHECK(dm2_v1_skproject_find_ladder_around(
              &dungeon, 0, 1, 1, &ladder_receipt) &&
              ladder_receipt.valid && ladder_receipt.found &&
              ladder_receipt.kind == 1,
          "DM2_FIND_LADDAR_AROUND finds the upward ladder");
    dm2_v1_dungeon_free(&dungeon);

    /* DM2_GET_PLAYER_AT_POSITION reads caller-owned party positions. */
    CHECK(dm2_v1_skproject_get_player_at_position(
              0u, player_at_position, &player, &player_receipt) &&
              player_receipt.valid && player == 3,
          "DM2_GET_PLAYER_AT_POSITION returns champion at position zero");
    CHECK(dm2_v1_skproject_get_player_at_position(
              1u, player_at_position, &player, &player_receipt) &&
              player == -1,
          "DM2_GET_PLAYER_AT_POSITION returns -1 for empty position");
    CHECK(!dm2_v1_skproject_get_player_at_position(
              2u, NULL, &player, &player_receipt),
          "DM2_GET_PLAYER_AT_POSITION rejects missing position table");

    /* DM2_DIR_FROM_5x5_POS extracts dominant-axis directions. */
    CHECK(dm2_v1_skproject_dir_from_5x5_pos(
              17u, &dir, &dir_receipt) &&
              dir_receipt.valid && dir == 2u,
          "DM2_DIR_FROM_5x5_POS returns south for front-center cell");
    CHECK(dm2_v1_skproject_dir_from_5x5_pos(
              13u, &dir, &dir_receipt) &&
              dir == 1u,
          "DM2_DIR_FROM_5x5_POS returns east for right-center cell");
    CHECK(!dm2_v1_skproject_dir_from_5x5_pos(
              12u, &dir, &dir_receipt) &&
              dir_receipt.blocked_center,
          "DM2_DIR_FROM_5x5_POS rejects the center cell");

    /* DM2_GET_GLOB_VAR bounds-checks the global-words table. */
    global_words[5] = 0x1234u;
    CHECK(dm2_v1_skproject_get_glob_var(
              5u, global_words, 64u, &value, &glob_receipt) &&
              glob_receipt.valid && value == 0x1234u,
          "DM2_GET_GLOB_VAR returns the requested global word");
    CHECK(!dm2_v1_skproject_get_glob_var(
              64u, global_words, 64u, &value, &glob_receipt) &&
              glob_receipt.blocked_out_of_range,
          "DM2_GET_GLOB_VAR rejects out-of-range index");

    /* DM2_UPDATE_GLOB_VAR modifies the global-words table. */
    {
        DM2_V1_SkprojectUpdateGlobVarReceipt ug;
        global_words[10] = 0x0005u;
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 0, 0, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 1u,
              "DM2_UPDATE_GLOB_VAR op 0 sets to 1");
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 1, 0, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 0u,
              "DM2_UPDATE_GLOB_VAR op 1 sets to 0");
        global_words[10] = 0u;
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 2, 0, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 1u,
              "DM2_UPDATE_GLOB_VAR op 2 toggles 0 to 1");
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 2, 0, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 0u,
              "DM2_UPDATE_GLOB_VAR op 2 toggles nonzero to 0");
        global_words[10] = 10u;
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 3, 5, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 15u,
              "DM2_UPDATE_GLOB_VAR op 3 adds");
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 4, 3, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 12u,
              "DM2_UPDATE_GLOB_VAR op 4 subtracts");
        CHECK(dm2_v1_skproject_update_glob_var(
                  10u, 6, 42, global_words, 64u, &ug) &&
                  ug.valid && ug.new_value == 42u,
              "DM2_UPDATE_GLOB_VAR op 6 assigns");
        CHECK(!dm2_v1_skproject_update_glob_var(
                  64u, 0, 0, global_words, 64u, &ug) &&
                  ug.blocked_out_of_range,
              "DM2_UPDATE_GLOB_VAR rejects out-of-range");
        CHECK(!dm2_v1_skproject_update_glob_var(
                  10u, 7, 0, global_words, 64u, &ug) &&
                  ug.blocked_bad_op,
              "DM2_UPDATE_GLOB_VAR rejects bad op");
    }

    /* DM2_GET_CREATURE_WEIGHT records a caller-resolved weight. */
    CHECK(dm2_v1_skproject_get_creature_weight(
              100u, &weight, &weight_receipt) &&
              weight_receipt.valid && weight == 100u &&
              !weight_receipt.overweight,
          "DM2_GET_CREATURE_WEIGHT records normal weight");
    CHECK(dm2_v1_skproject_get_creature_weight(
              0x0100u, &weight, &weight_receipt) &&
              weight_receipt.overweight,
          "DM2_GET_CREATURE_WEIGHT flags weight above source threshold");

    /* DM2_CONVERT_PALETTE256 converts a 256-entry RGB888 palette. */
    for (uint16_t i = 0u; i < 256u; ++i) {
        src_rgb8[i * 3u + 0u] = (uint8_t)(255u - i);
        src_rgb8[i * 3u + 1u] = (uint8_t)i;
        src_rgb8[i * 3u + 2u] = (uint8_t)(i >> 1);
        xlat[i] = (uint8_t)i;
    }
    memset(dst_rgb6, 0, sizeof(dst_rgb6));
    CHECK(dm2_v1_skproject_convert_palette256(
              src_rgb8, NULL, dst_rgb6, &palette_receipt) &&
              palette_receipt.valid && palette_receipt.palette_hash != 0u &&
              dst_rgb6[0][0] == 63u && dst_rgb6[255][1] == 63u,
          "DM2_CONVERT_PALETTE256 shifts RGB888 to RGB666");
    xlat[255] = 0u;
    CHECK(dm2_v1_skproject_convert_palette256(
              src_rgb8, xlat, dst_rgb6, &palette_receipt) &&
              dst_rgb6[255][1] == 0u,
          "DM2_CONVERT_PALETTE256 applies the translation table");
}

static uint16_t cycle11_mk_handle(int type, int index)
{
    return (uint16_t)((type << 10) | (index & 0x3ff));
}

static uint16_t cycle11_distinctive_type_cb(uint16_t object_id, void *user)
{
    (void)user;
    switch (object_id) {
        case 0x1400u: return 0x0123u;
        case 0x2000u: return 0x0014u;
        case 0x2001u: return 0x0014u;
        default: return 0u;
    }
}

static uint8_t cycle11_cls2_cb(uint16_t object_id, void *user)
{
    (void)user;
    if (object_id == 0x2000u || object_id == 0x2001u)
        return 0x14u;
    return 0u;
}

static void cycle11_build_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
{
    const size_t header_size = 44u;
    const size_t map_desc_size = 16u;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 2u * 2u;
    const size_t text_base = sft_base + 2u * 2u;
    const size_t db3_base = text_base;
    const size_t db4_base = db3_base + 1u * 8u;
    const size_t db8_base = db4_base + 1u * 16u;
    const size_t db5_base = db8_base + 2u * 4u;
    const size_t map_base = db5_base + 1u * 4u;

    memset(d, 0, sizeof(*d));
    memset(raw, 0, 256u);
    d->level_count = 1;
    d->level_widths[0] = 2;
    d->level_heights[0] = 2;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = (int)map_base;
    d->column_index_base = (int)column_base;
    d->square_first_thing_base = (int)sft_base;
    d->square_first_thing_count = 2;
    d->raw_data = raw;
    d->raw_size = 256;
    d->record_graph_complete = 1;

    /* Header */
    raw[4] = 1u; /* map count */
    put16le(raw + 6, 0u);  /* text_word_count */
    put16le(raw + 10, 2u); /* square_first_thing_count */
    put16le(raw + 12 + 3u * 2u, 1u); /* thing_type_counts[3] */
    put16le(raw + 12 + 4u * 2u, 1u); /* thing_type_counts[4] */
    put16le(raw + 12 + 5u * 2u, 1u); /* thing_type_counts[5] */
    put16le(raw + 12 + 8u * 2u, 2u); /* thing_type_counts[8] */

    /* Map descriptor */
    put16le(raw + header_size + 0, 0);     /* rel_offset */
    put16le(raw + header_size + 2,
            (uint16_t)(((2u - 1u) << 6) | ((2u - 1u) << 11))); /* dimensions */

    /* Column index table: column 0 has one flagged cell, column 1 none. */
    put16le(raw + column_base + 0, 0);
    put16le(raw + column_base + 2, 1);

    /* Square first thing table: entry 0 is the chain head at (0,0). */
    put16le(raw + sft_base + 0, cycle11_mk_handle(3, 0));
    put16le(raw + sft_base + 2, 0xfffeu);

    /* DB3 actuator record 0: next -> type 5 item, ordinal 0x25. */
    put16le(raw + db3_base + 0, cycle11_mk_handle(5, 0));
    put16le(raw + db3_base + 2, 0x0025u);

    /* DB4 container record 0: next -> END, child -> type 8 index 1. */
    put16le(raw + db4_base + 0, 0xfffeu);
    put16le(raw + db4_base + 2, cycle11_mk_handle(8, 1));

    /* DB8 flask record 0: next -> container, cls2 will resolve to 0x14. */
    put16le(raw + db8_base + 0, cycle11_mk_handle(4, 0));
    put16le(raw + db8_base + 2, 0xfffeu);

    /* DB8 flask record 1: next -> END. */
    put16le(raw + db8_base + 4, 0xfffeu);

    /* DB5 item record 0: next -> flask 0, distinctive type 0x123. */
    put16le(raw + db5_base + 0, cycle11_mk_handle(8, 0));

    /* Raw map: only (0,0) is flagged. */
    raw[map_base + 0] = 0x10; /* (0,0) */
    raw[map_base + 1] = 0x00; /* (0,1) */
    raw[map_base + 2] = 0x00; /* (1,0) */
    raw[map_base + 3] = 0x00; /* (1,1) */

    d->thing_data_bases[3] = (int)db3_base;
    d->thing_data_bases[4] = (int)db4_base;
    d->thing_data_bases[5] = (int)db5_base;
    d->thing_data_bases[8] = (int)db8_base;
    d->thing_type_counts[3] = 1;
    d->thing_type_counts[4] = 1;
    d->thing_type_counts[5] = 1;
    d->thing_type_counts[8] = 2;
}

static void test_skwin_core_symbol_batch_cycle11(void)
{
    DM2_V1_DungeonData dungeon;
    uint8_t raw[256];
    DM2_V1_SkprojectDistinctiveItemOnActuatorReceipt actuator_item_receipt;
    DM2_V1_SkprojectFindHandWithEmptyFlaskReceipt flask_receipt;
    DM2_V1_SkprojectFindDistinctiveItemOnTileReceipt distinctive_receipt;
    DM2_V1_SkprojectFindTileActuatorReceipt find_actuator_receipt;
    DM2_V1_SkprojectCalcPlayerWalkDelayReceipt walk_receipt;
    DM2_V1_SkprojectComputePlayerAttackOrThrowStrengthReceipt strength_receipt;
    uint16_t hands[2];
    uint16_t object_id;
    int16_t hand;
    int32_t delay;
    int16_t strength;

    cycle11_build_dungeon(&dungeon, raw);

    /* DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR */
    CHECK(dm2_v1_skproject_is_distinctive_item_on_actuator(
              &dungeon, 0, 0, 0, 0x0123u, 1,
              cycle11_distinctive_type_cb, NULL,
              &actuator_item_receipt) == 1 &&
              actuator_item_receipt.valid && actuator_item_receipt.found &&
              actuator_item_receipt.matched_object_id == cycle11_mk_handle(5, 0),
          "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR finds matching item on tile");
    CHECK(dm2_v1_skproject_is_distinctive_item_on_actuator(
              &dungeon, 0, 0, 0, 0x0014u, 1,
              cycle11_distinctive_type_cb, NULL,
              &actuator_item_receipt) == 1 &&
              actuator_item_receipt.matched_object_id == cycle11_mk_handle(8, 0),
          "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR finds flask before container descent");
    CHECK(dm2_v1_skproject_is_distinctive_item_on_actuator(
              &dungeon, 0, 0, 0, 0x0014u, 0,
              cycle11_distinctive_type_cb, NULL,
              &actuator_item_receipt) == 0 &&
              actuator_item_receipt.container_count == 1u &&
              !actuator_item_receipt.found,
          "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR skips items when search_items is zero");
    CHECK(dm2_v1_skproject_is_distinctive_item_on_actuator(
              &dungeon, 0, 0, 0, 0x0999u, 1,
              cycle11_distinctive_type_cb, NULL,
              &actuator_item_receipt) == 0 &&
              actuator_item_receipt.valid && !actuator_item_receipt.found,
          "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR returns not found for absent type");
    CHECK(!dm2_v1_skproject_is_distinctive_item_on_actuator(
              NULL, 0, 0, 0, 0x0014u, 1,
              cycle11_distinctive_type_cb, NULL,
              &actuator_item_receipt) &&
              actuator_item_receipt.blocked_missing_dungeon,
          "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR rejects missing dungeon");
    CHECK(!dm2_v1_skproject_is_distinctive_item_on_actuator(
              &dungeon, 0, 0, 0, 0x0014u, 1, NULL, NULL,
              &actuator_item_receipt) &&
              actuator_item_receipt.blocked_missing_callback,
          "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR rejects missing callback");

    /* DM2_FIND_HAND_WITH_EMPTY_FLASK */
    hands[0] = cycle11_mk_handle(8, 0);
    hands[1] = cycle11_mk_handle(5, 0);
    CHECK(dm2_v1_skproject_find_hand_with_empty_flask(
              hands, cycle11_cls2_cb, NULL, &hand,
              &flask_receipt) == 1 &&
              flask_receipt.valid && flask_receipt.found && hand == 0 &&
              flask_receipt.hand_types[0] == 8u &&
              flask_receipt.hand_cls2[0] == 0x14u,
          "DM2_FIND_HAND_WITH_EMPTY_FLASK selects right hand empty flask");
    hands[0] = cycle11_mk_handle(5, 0);
    hands[1] = cycle11_mk_handle(8, 0);
    CHECK(dm2_v1_skproject_find_hand_with_empty_flask(
              hands, cycle11_cls2_cb, NULL, &hand,
              &flask_receipt) == 1 && hand == 1,
          "DM2_FIND_HAND_WITH_EMPTY_FLASK prefers left hand when right has no flask");
    hands[0] = cycle11_mk_handle(5, 0);
    hands[1] = cycle11_mk_handle(5, 0);
    CHECK(dm2_v1_skproject_find_hand_with_empty_flask(
              hands, cycle11_cls2_cb, NULL, &hand,
              &flask_receipt) == 0 &&
              flask_receipt.valid && !flask_receipt.found && hand == -1,
          "DM2_FIND_HAND_WITH_EMPTY_FLASK returns -1 when no flask present");
    CHECK(!dm2_v1_skproject_find_hand_with_empty_flask(
              hands, NULL, NULL, &hand,
              &flask_receipt) &&
              flask_receipt.blocked_missing_callback,
          "DM2_FIND_HAND_WITH_EMPTY_FLASK rejects missing cls2 callback");

    /* DM2_FIND_DISTINCTIVE_ITEM_ON_TILE */
    CHECK(dm2_v1_skproject_find_distinctive_item_on_tile(
              &dungeon, 0, 0, 0, 0x0123u, -1,
              cycle11_distinctive_type_cb, NULL,
              &distinctive_receipt) == 1 &&
              distinctive_receipt.valid && distinctive_receipt.found &&
              distinctive_receipt.found_object_id == cycle11_mk_handle(5, 0),
          "DM2_FIND_DISTINCTIVE_ITEM_ON_TILE finds distinctive item with subtype -1");
    CHECK(dm2_v1_skproject_find_distinctive_item_on_tile(
              &dungeon, 0, 0, 0, 0x0014u, 0,
              cycle11_distinctive_type_cb, NULL,
              &distinctive_receipt) == 1 &&
              distinctive_receipt.found_object_id == cycle11_mk_handle(8, 0),
          "DM2_FIND_DISTINCTIVE_ITEM_ON_TILE filters by subtype 0");
    CHECK(dm2_v1_skproject_find_distinctive_item_on_tile(
              &dungeon, 0, 0, 0, 0x0014u, 1,
              cycle11_distinctive_type_cb, NULL,
              &distinctive_receipt) == 0 &&
              distinctive_receipt.valid && !distinctive_receipt.found,
          "DM2_FIND_DISTINCTIVE_ITEM_ON_TILE rejects non-matching subtype");
    CHECK(!dm2_v1_skproject_find_distinctive_item_on_tile(
              NULL, 0, 0, 0, 0x0123u, -1,
              cycle11_distinctive_type_cb, NULL,
              &distinctive_receipt) &&
              distinctive_receipt.blocked_missing_dungeon,
          "DM2_FIND_DISTINCTIVE_ITEM_ON_TILE rejects missing dungeon");

    /* DM2_FIND_TILE_ACTUATOR */
    CHECK(dm2_v1_skproject_find_tile_actuator(
              &dungeon, 0, 0, 0, 0x25u, -1, &object_id,
              &find_actuator_receipt) == 1 &&
              find_actuator_receipt.valid && find_actuator_receipt.found &&
              object_id == cycle11_mk_handle(3, 0),
          "DM2_FIND_TILE_ACTUATOR finds actuator by ordinal");
    CHECK(dm2_v1_skproject_find_tile_actuator(
              &dungeon, 0, 0, 0, 0x25u, 0, &object_id,
              &find_actuator_receipt) == 1 &&
              object_id == cycle11_mk_handle(3, 0),
          "DM2_FIND_TILE_ACTUATOR matches subtype 0");
    CHECK(dm2_v1_skproject_find_tile_actuator(
              &dungeon, 0, 0, 0, 0x25u, 1, &object_id,
              &find_actuator_receipt) == 0 &&
              find_actuator_receipt.valid && !find_actuator_receipt.found,
          "DM2_FIND_TILE_ACTUATOR rejects non-matching subtype");
    CHECK(dm2_v1_skproject_find_tile_actuator(
              &dungeon, 0, 0, 0, 0x99u, -1, &object_id,
              &find_actuator_receipt) == 0 &&
              find_actuator_receipt.actuator_count == 1u,
          "DM2_FIND_TILE_ACTUATOR returns not found for absent ordinal");
    CHECK(!dm2_v1_skproject_find_tile_actuator(
              NULL, 0, 0, 0, 0x25u, -1, &object_id,
              &find_actuator_receipt) &&
              find_actuator_receipt.blocked_missing_dungeon,
          "DM2_FIND_TILE_ACTUATOR rejects missing dungeon");

    /* DM2_CALC_PLAYER_WALK_DELAY */
    CHECK(dm2_v1_skproject_calc_player_walk_delay(
              100u, 50u, 0u, 0, 1u, &delay,
              &walk_receipt) == 1 && delay == 1 && walk_receipt.valid,
          "DM2_CALC_PLAYER_WALK_DELAY returns 1 when savegames1.b_04 is set");
    CHECK(dm2_v1_skproject_calc_player_walk_delay(
              100u, 50u, 0u, 0, 0u, &delay,
              &walk_receipt) == 1 && delay == 2 &&
              !walk_receipt.heavy_load && !walk_receipt.overburdened,
          "DM2_CALC_PLAYER_WALK_DELAY base delay is 2 for normal load");
    CHECK(dm2_v1_skproject_calc_player_walk_delay(
              100u, 70u, 0u, 0, 0u, &delay,
              &walk_receipt) == 1 && walk_receipt.heavy_load && delay == 4,
          "DM2_CALC_PLAYER_WALK_DELAY bumps to even 4 for heavy load");
    CHECK(dm2_v1_skproject_calc_player_walk_delay(
              100u, 120u, 0u, 0, 0u, &delay,
              &walk_receipt) == 1 && walk_receipt.overburdened && delay == 4,
          "DM2_CALC_PLAYER_WALK_DELAY overburdened branch starts at 4");
    CHECK(dm2_v1_skproject_calc_player_walk_delay(
              100u, 50u, 0x20u, 0, 0u, &delay,
              &walk_receipt) == 1 && walk_receipt.bodyflag_slow && delay == 4,
          "DM2_CALC_PLAYER_WALK_DELAY bodyflag 0x20 adds 1 then rounds to even");
    CHECK(dm2_v1_skproject_calc_player_walk_delay(
              100u, 120u, 0x20u, 2, 0u, &delay,
              &walk_receipt) == 1 && walk_receipt.bodyflag_slow && delay == 4,
          "DM2_CALC_PLAYER_WALK_DELAY subtracts walkspeed and clamps");

    /* DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH */
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 0, 0u, 0u, 0u, 0u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.valid && strength == 15,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH returns stamina_adj/2");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 0, 0u, 0u, 0u, 0x01u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.bodyflag_halved && strength == 7,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH halves for bodyflag bit");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 0, 0u, 0u, 0u, 0x02u, 1, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.bodyflag_halved && strength == 7,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH halves for off-hand bodyflag");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, -1, 0u, 10u, 5u, 0u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.pre_strength == 31 && strength == 15,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH ignores skill_kind -1");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 0, 0x8000u, 10u, 5u, 0u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.pre_strength == 51,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH adds dbspec_word8 for skill 0");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 11, 0x8000u, 10u, 5u, 0u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.pre_strength == 46,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH adds dbspec_word9 for skill 11 with word5 0x8000");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 11, 0u, 10u, 5u, 0u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.pre_strength == 41,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH ignores dbspec_word9 for skill 11 without word5 0x8000");
    CHECK(dm2_v1_skproject_compute_player_attack_or_throw_strength(
              50u, 160u, 20u, 5u, 1, 0x8000u, 10u, 5u, 0u, 0, 30,
              &strength, &strength_receipt) == 1 &&
              strength_receipt.pre_strength == 41,
          "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH ignores dbspec_word9 for skill 1 with word5 0x8000");

    (void)dungeon;
}

static void test_skwin_core_symbol_batch_cycle12(void)
{
    DM2_V1_RecordPoolSet pools;
    uint8_t pool4_bytes[16];
    uint8_t creatures[34 * 4];
    uint16_t ai_word10[8];
    uint8_t object_pos_table[4] = { 0u, 5u, 10u, 15u };
    uint8_t obj_rec[8];
    uint8_t palette[256][3];
    uint16_t timer_word;
    uint16_t result;
    int32_t result48;
    int16_t colors;
    uint8_t res;
    uint8_t pos;
    DM2_V1_SkprojectQuery4e26Receipt q4e26;
    DM2_V1_SkprojectQuery1c9a08bdReceipt q08bd;
    DM2_V1_SkprojectIsCreatureFloatingReceipt icf;
    DM2_V1_SkprojectIsObjectFloatingReceipt iof;
    DM2_V1_SkprojectQueryObject5x5PosReceipt q5x5;
    DM2_V1_SkprojectQuery32cb0804Receipt q32;
    DM2_V1_SkprojectQuery0b36037eReceipt q0b;
    DM2_V1_SkprojectQuery48ae05aeReceipt q48;

    memset(&pools, 0, sizeof(pools));
    memset(pool4_bytes, 0, sizeof(pool4_bytes));
    pools.valid = 1;
    pools.pools[4].bytes = pool4_bytes;
    pools.pools[4].record_count = 1;
    pools.pools[4].record_size = 16;

    memset(creatures, 0, sizeof(creatures));
    creatures[34 * 1 + 0x1a] = 5u;
    creatures[34 * 1 + 0x1f] = 1u;
    creatures[34 * 2 + 0x1a] = 5u;
    creatures[34 * 2 + 0x1f] = 2u;
    creatures[34 * 3 + 0x1a] = 4u;
    creatures[34 * 3 + 0x1f] = 1u;

    memset(ai_word10, 0, sizeof(ai_word10));
    ai_word10[1] = 0x0004u;

    pool4_bytes[4] = 1u; /* creature type */
    pool4_bytes[5] = 1u; /* creature index */

    /* DM2_query_4E26 */
    timer_word = 0x4000u;
    CHECK(dm2_v1_skproject_query_4e26(&timer_word, 123u, &result, &q4e26) == 1 &&
              q4e26.valid && result == 0u && q4e26.bit_4000,
          "DM2_query_4E26 returns 0 when bit 0x4000 is set");
    timer_word = 0x8067u; /* interval = 1, period = 0x27 */
    CHECK(dm2_v1_skproject_query_4e26(&timer_word, 100u, &result, &q4e26) == 1 &&
              q4e26.valid && result == ((1u + 100u) % 0x27u),
          "DM2_query_4E26 adds interval to game tick modulo period");
    timer_word = 0x9000u;
    CHECK(!dm2_v1_skproject_query_4e26(&timer_word, 100u, &result, &q4e26) &&
              q4e26.blocked_zero_divisor,
          "DM2_query_4E26 fails closed on zero period");
    timer_word = 0x0017u;
    CHECK(dm2_v1_skproject_query_4e26(&timer_word, 100u, &result, &q4e26) == 1 &&
              result == 0x17u,
          "DM2_query_4E26 returns lower six bits when no timer flag set");

    /* DM2_query_1c9a_08bd */
    memset(obj_rec, 0, sizeof(obj_rec));
    obj_rec[5] = 1u;
    CHECK(dm2_v1_skproject_query_1c9a_08bd(
              obj_rec, creatures, 4u, &res, &q08bd) == 1 &&
              q08bd.valid && res == 1u,
          "DM2_query_1c9a_08bd returns 1 for levitate creature");
    obj_rec[5] = 3u;
    CHECK(dm2_v1_skproject_query_1c9a_08bd(
              obj_rec, creatures, 4u, &res, &q08bd) == 1 &&
              q08bd.valid && res == 0u,
          "DM2_query_1c9a_08bd returns 0 for non-levitate creature");
    obj_rec[5] = 0xffu;
    CHECK(dm2_v1_skproject_query_1c9a_08bd(
              obj_rec, creatures, 4u, &res, &q08bd) == 1 &&
              q08bd.valid && res == 0u,
          "DM2_query_1c9a_08bd returns 0 for null creature index");
    CHECK(!dm2_v1_skproject_query_1c9a_08bd(
              NULL, creatures, 4u, &res, &q08bd) &&
              q08bd.blocked_missing_record,
          "DM2_query_1c9a_08bd rejects missing record");

    /* DM2_IS_CREATURE_FLOATING */
    CHECK(dm2_v1_skproject_is_creature_floating(
              0x1000u, &pools, creatures, 4u,
              ai_word10, 8u, &res, &icf) == 1 &&
              icf.valid && res == 1u,
          "DM2_IS_CREATURE_FLOATING returns 1 when AI spec bit 2 set");
    ai_word10[1] = 0u;
    CHECK(dm2_v1_skproject_is_creature_floating(
              0x1000u, &pools, creatures, 4u,
              ai_word10, 8u, &res, &icf) == 1 &&
              icf.valid && res == 1u && icf.used_fallback,
          "DM2_IS_CREATURE_FLOATING falls back to query_1c9a_08bd");
    pool4_bytes[5] = 3u;
    CHECK(dm2_v1_skproject_is_creature_floating(
              0x1000u, &pools, creatures, 4u,
              ai_word10, 8u, &res, &icf) == 1 &&
              icf.valid && res == 0u,
          "DM2_IS_CREATURE_FLOATING returns 0 when AI bit clear and fallback false");
    pool4_bytes[5] = 1u;
    ai_word10[1] = 0x0004u;
    CHECK(!dm2_v1_skproject_is_creature_floating(
              0x1000u, NULL, creatures, 4u,
              ai_word10, 8u, &res, &icf) &&
              icf.blocked_missing_record,
          "DM2_IS_CREATURE_FLOATING rejects missing record pool");

    /* DM2_IS_OBJECT_FLOATING */
    CHECK(dm2_v1_skproject_is_object_floating(
              0x1000u, &pools, creatures, 4u,
              ai_word10, 8u, &res, &iof) == 1 &&
              iof.valid && res == 1u && iof.delegated_to_creature,
          "DM2_IS_OBJECT_FLOATING delegates type-4 creatures");
    CHECK(dm2_v1_skproject_is_object_floating(
              0x3800u, NULL, NULL, 0u, NULL, 0u, &res, &iof) == 1 &&
              iof.valid && res == 1u,
          "DM2_IS_OBJECT_FLOATING returns 1 for type 0xe");
    CHECK(dm2_v1_skproject_is_object_floating(
              0x3c00u, NULL, NULL, 0u, NULL, 0u, &res, &iof) == 1 &&
              iof.valid && res == 1u,
          "DM2_IS_OBJECT_FLOATING returns 1 for type 0xf");
    CHECK(dm2_v1_skproject_is_object_floating(
              0x0000u, NULL, NULL, 0u, NULL, 0u, &res, &iof) == 1 &&
              iof.valid && res == 0u,
          "DM2_IS_OBJECT_FLOATING returns 0 for type 0");

    /* DM2_QUERY_OBJECT_5x5_POS */
    CHECK(dm2_v1_skproject_query_object_5x5_pos(
              0x0000u, 0u, &pools, object_pos_table, &pos, &q5x5) == 1 &&
              q5x5.valid && q5x5.used_default_pos && pos == 0x0cu,
          "DM2_QUERY_OBJECT_5x5_POS uses default base 0xc with no rotation");
    CHECK(dm2_v1_skproject_query_object_5x5_pos(
              0x5400u, 1u, &pools, object_pos_table, &pos, &q5x5) == 1 &&
              q5x5.valid && q5x5.used_object_table &&
              q5x5.base_pos == object_pos_table[1] && pos == 21u,
          "DM2_QUERY_OBJECT_5x5_POS rotates subtype-mapped base");
    CHECK(!dm2_v1_skproject_query_object_5x5_pos(
              0x5400u, 0u, &pools, NULL, &pos, &q5x5) &&
              q5x5.blocked_missing_pos_table,
          "DM2_QUERY_OBJECT_5x5_POS fails closed without object position table");
    CHECK(!dm2_v1_skproject_query_object_5x5_pos(
              0x1000u, 0u, &pools, object_pos_table, &pos, &q5x5) &&
              q5x5.blocked_missing_creature_pos,
          "DM2_QUERY_OBJECT_5x5_POS fails closed on creature GDAT path");

    /* DM2_query_32cb_0804 / DM2_query_0b36_037e / DM2_query_48ae_05ae */
    memset(palette, 0, sizeof(palette));
    colors = 0x100;
    CHECK(!dm2_v1_skproject_query_32cb_0804(
              palette, 1, 2, 3, &colors, &q32) &&
              q32.blocked_missing_gdat_path && q32.colors_before == 0x100,
          "DM2_query_32cb_0804 records inputs and fails closed on GDAT path");
    CHECK(!dm2_v1_skproject_query_32cb_0804(
              NULL, 1, 2, 3, &colors, &q32) &&
              q32.blocked_missing_palette,
          "DM2_query_32cb_0804 rejects missing palette");
    CHECK(!dm2_v1_skproject_query_0b36_037e(
              palette, 8u, 0u, 7u, 10u, 1, 2, &colors, &q0b) &&
              q0b.blocked_missing_dballoc_path && q0b.colors_before == 0x100,
          "DM2_query_0b36_037e records inputs and fails closed on dballoc path");
    CHECK(!dm2_v1_skproject_query_48ae_05ae(
              0x1234u, 5u, 0x0abcu, 0, -1, &result48, &q48) &&
              q48.blocked_missing_gdat_path && q48.item_handle == 0x1234u,
          "DM2_query_48ae_05ae records inputs and fails closed on GDAT path");
}

static int cycle13_loadable_fn(
    uint8_t cls1,
    uint8_t cls2,
    uint8_t entry_index,
    uint8_t entry_id,
    void *user)
{
    const uint8_t *loadable = (const uint8_t *)user;
    uint8_t key;
    (void)cls1;
    (void)cls2;
    key = (uint8_t)((entry_index << 4) | (entry_id & 0x0fu));
    for (size_t i = 0; loadable && loadable[i] != 0; ++i) {
        if (loadable[i] == key)
            return 1;
    }
    return 0;
}

static void test_skwin_core_symbol_batch_cycle13(void)
{
    DM2_V1_SkprojectQuery4da3Receipt q4da3;
    DM2_V1_SkprojectQueryCreature5x5PosReceipt qc5x5;
    DM2_V1_SkprojectQuery0cee0897Receipt q0897;
    DM2_V1_SkprojectGetTeleporterDetailReceipt qtele;
    DM2_V1_SkprojectIsCreatureMovableThereReceipt qmov;
    DM2_V1_SkprojectQuery0cee1a46Receipt q1a46;
    DM2_V1_SkprojectQuery48ae011aReceipt q48ae011a;
    DM2_V1_SkprojectQuery0cee2e09Receipt q2e09;
    DM2_V1_SkprojectCreatureAISpec ai_spec;
    DM2_V1_SkprojectTeleporterDetail detail;
    DM2_V1_RecordPoolSet pools;
    uint8_t pool3_bytes[64];
    uint8_t gdat_blob[16];
    uint8_t creature_rec[8];
    uint8_t tile_values[9];
    uint8_t dest_tiles[64];
    uint8_t out_bytes[8];
    uint8_t pos;
    uint16_t word;
    uint16_t word32;
    int32_t frame_class;
    int16_t wall_idx;
    int16_t wall_field;

    memset(&pools, 0, sizeof(pools));
    memset(pool3_bytes, 0, sizeof(pool3_bytes));
    pools.valid = 1;
    pools.pools[3].bytes = pool3_bytes;
    pools.pools[3].record_count = 4;
    pools.pools[3].record_size = 8;

    /* DM2_query_4DA3 */
    memset(gdat_blob, 0, sizeof(gdat_blob));
    memcpy(gdat_blob + 8, "abcdefgh", 8u);
    word = 0x8061u; /* interval = 1, period = 0x21 */
    CHECK(!dm2_v1_skproject_query_4da3(
              0x12u, 0u, &word, NULL, sizeof(gdat_blob), out_bytes, &q4da3) &&
              q4da3.blocked_missing_gdat,
          "DM2_query_4DA3 fails closed without GDAT blob");
    CHECK(dm2_v1_skproject_query_4da3(
              0x12u, 0u, &word, gdat_blob, sizeof(gdat_blob), out_bytes,
              &q4da3) == 1 &&
              q4da3.valid && q4da3.offset == 8u &&
              memcmp(out_bytes, "abcdefgh", 8u) == 0,
          "DM2_query_4DA3 copies eight bytes at computed offset");
    CHECK(!dm2_v1_skproject_query_4da3(
              0x12u, 1u, &word, gdat_blob, sizeof(gdat_blob), out_bytes,
              &q4da3) &&
              q4da3.blocked_out_of_bounds,
          "DM2_query_4DA3 fails closed when offset overruns blob");

    /* DM2_QUERY_CREATURE_5x5_POS */
    memset(creature_rec, 0, sizeof(creature_rec));
    creature_rec[4] = 0x05u; /* creature type */
    memset(gdat_blob, 0, sizeof(gdat_blob));
    gdat_blob[4] = 0x0cu; /* base 5x5 position */
    ai_spec.word30 = 0u;
    ai_spec.word32 = 0u;
    CHECK(!dm2_v1_skproject_query_creature_5x5_pos(
              NULL, 0u, &ai_spec, 0u, 0u, gdat_blob, sizeof(gdat_blob), &pos,
              &qc5x5) &&
              qc5x5.blocked_missing_record && pos == 0x0cu,
          "DM2_QUERY_CREATURE_5x5_POS fails closed without record");
    CHECK(!dm2_v1_skproject_query_creature_5x5_pos(
              creature_rec, 0u, NULL, 0u, 0u, gdat_blob, sizeof(gdat_blob),
              &pos, &qc5x5) &&
              qc5x5.blocked_missing_ai_spec,
          "DM2_QUERY_CREATURE_5x5_POS fails closed without AI spec");
    gdat_blob[4] = 0x08u; /* base 8, direction 1 rotates to 6 */
    CHECK(dm2_v1_skproject_query_creature_5x5_pos(
              creature_rec, 1u, &ai_spec, 0u, 0x8020u, gdat_blob,
              sizeof(gdat_blob), &pos, &qc5x5) == 1 &&
              qc5x5.valid && qc5x5.base_pos == 0x08u &&
              qc5x5.rotated_pos == 0x06u && pos == 0x06u,
          "DM2_QUERY_CREATURE_5x5_POS rotates GDAT base position");

    /* DM2_query_0cee_0897 */
    memset(tile_values, 0, sizeof(tile_values));
    tile_values[4] = (uint8_t)((5u << 5) | 0u); /* type 5, record index 0 */
    /* Record 0 is the tile's first record; record 1 is the type-3 actuator.
       The source stores the first record pointer and scans subsequent links. */
    pool3_bytes[0] = 0x01u;
    pool3_bytes[1] = 0x0cu; /* next link -> record 1, type 3 */
    pool3_bytes[2] = 0x27u; /* first record word2 low byte (detail source) */
    pool3_bytes[3] = 0x00u;
    pool3_bytes[8] = 0xfeu;
    pool3_bytes[9] = 0xffu; /* record 1 next = end marker */
    pool3_bytes[10] = 0x27u; /* record 1 word2 low byte: 0x27 */
    pool3_bytes[11] = 0x00u;
    CHECK(!dm2_v1_skproject_query_0cee_0897(
              1, 1, NULL, 3, 3, &pools, NULL, NULL, &q0897) &&
              q0897.blocked_missing_tiles,
          "DM2_query_0cee_0897 fails closed without tile values");
    tile_values[4] = 0x00u; /* type 0 */
    CHECK(!dm2_v1_skproject_query_0cee_0897(
              1, 1, tile_values, 3, 3, &pools, NULL, NULL, &q0897) &&
              q0897.blocked_not_tile_type_5,
          "DM2_query_0cee_0897 rejects non-teleporter tile type");
    tile_values[4] = (uint8_t)((5u << 5) | 0u);
    CHECK(dm2_v1_skproject_query_0cee_0897(
              1, 1, tile_values, 3, 3, &pools, NULL, NULL, &q0897) == 1 &&
              q0897.valid && q0897.detail == 3u,
          "DM2_query_0cee_0897 derives detail from first record word2 bits");

    /* DM2_GET_TELEPORTER_DETAIL */
    CHECK(!dm2_v1_skproject_get_teleporter_detail(
              1, 1, tile_values, 3, 3, &pools, 0u, NULL, 0, 0, NULL, &qtele) &&
              qtele.blocked_missing_destination,
          "DM2_GET_TELEPORTER_DETAIL fails closed without output");
    memset(dest_tiles, 0, sizeof(dest_tiles));
    /* word2=0x0027 forces dest_x=7 (low 5 bits) and dest_y=1 (bits 5-9),
       so the destination tile plane must be at least 8x2. */
    dest_tiles[1 * 8 + 7] = (uint8_t)((5u << 5) | 0u); /* dest (7,1), record 0 */
    pool3_bytes[8] = 0xfeu;
    pool3_bytes[9] = 0xffu;
    pool3_bytes[10] = 0x27u;
    pool3_bytes[11] = 0x00u;
    pool3_bytes[5] = 0x02u; /* dest map = 2 (high byte of record word4) */
    CHECK(dm2_v1_skproject_get_teleporter_detail(
              1, 1, tile_values, 3, 3, &pools, 0u, dest_tiles, 8, 8, &detail,
              &qtele) == 1 &&
              qtele.valid && detail.b_04 == 2u,
          "DM2_GET_TELEPORTER_DETAIL resolves destination map and square");

    /* DM2_IS_CREATURE_MOVABLE_THERE */
    memset(tile_values, 0, sizeof(tile_values));
    tile_values[4] = 0x20u; /* open floor at (1,1) */
    tile_values[1] = 0x20u; /* open floor north of (1,1) */
    CHECK(!dm2_v1_skproject_is_creature_movable_there(
              1, 1, 0u, 0xffffu, 10u, tile_values, 3, 3, &pools, 0u,
              dest_tiles, 3, 3, NULL, &qmov) &&
              qmov.blocked_missing_creature,
          "DM2_IS_CREATURE_MOVABLE_THERE rejects missing creature");
    CHECK(!dm2_v1_skproject_is_creature_movable_there(
              1, 1, 0u, 0x1234u, 0x00feu, tile_values, 3, 3, &pools, 0u,
              dest_tiles, 3, 3, NULL, &qmov) &&
              qmov.blocked_overweight,
          "DM2_IS_CREATURE_MOVABLE_THERE rejects overweight creature");
    CHECK(dm2_v1_skproject_is_creature_movable_there(
              1, 1, 0u, 0x1234u, 10u, tile_values, 3, 3, &pools, 0u,
              dest_tiles, 3, 3, NULL, &qmov) == 1 &&
              qmov.valid && qmov.movable,
          "DM2_IS_CREATURE_MOVABLE_THERE admits open forward tile");
    tile_values[1] = 0x00u; /* blocked tile north of (1,1) */
    CHECK(!dm2_v1_skproject_is_creature_movable_there(
              1, 1, 0u, 0x1234u, 10u, tile_values, 3, 3, &pools, 0u,
              dest_tiles, 3, 3, NULL, &qmov) &&
              qmov.blocked_target_blocked,
          "DM2_IS_CREATURE_MOVABLE_THERE rejects blocked forward tile");

    /* DM2_query_0cee_1a46 */
    CHECK(!dm2_v1_skproject_query_0cee_1a46(
              NULL, 0u, 0, 0, &wall_idx, &wall_field, &q1a46) &&
              q1a46.blocked_missing_dungeon,
          "DM2_query_0cee_1a46 fails closed without dungeon data");

    /* DM2_query_48ae_011a */
    memset(pool3_bytes, 0, sizeof(pool3_bytes));
    pool3_bytes[0] = 0x0au; /* cls1 */
    pool3_bytes[1] = 0x14u; /* cls2 */
    pools.pools[3].bytes = pool3_bytes;
    pools.pools[3].record_count = 1;
    pools.pools[3].record_size = 8;
    {
        const uint8_t loadable_8_9_c[] = { 0x18u, 0x19u, 0x1cu, 0 };
        const uint8_t loadable_no8[] = { 0x1cu, 0x1au, 0x19u, 0 };
        const uint8_t loadable_8_only[] = { 0x18u, 0 };
        const uint8_t loadable_all[] = { 0x18u, 0x1cu, 0x1au, 0x19u, 0 };
        CHECK(!dm2_v1_skproject_query_48ae_011a(
                  0x0c00u, &pools, NULL, NULL, &frame_class, &q48ae011a) &&
                  q48ae011a.blocked_missing_loadable_fn,
              "DM2_query_48ae_011a fails closed without loadability callback");
        CHECK(!dm2_v1_skproject_query_48ae_011a(
                  0x0c00u, NULL, cycle13_loadable_fn, (void *)loadable_all,
                  &frame_class, &q48ae011a) &&
                  q48ae011a.blocked_missing_record,
              "DM2_query_48ae_011a fails closed without record pool");
        CHECK(!dm2_v1_skproject_query_48ae_011a(
                  0x0c00u, &pools, cycle13_loadable_fn, (void *)loadable_no8,
                  &frame_class, &q48ae011a) &&
                  q48ae011a.blocked_missing_gdat_path && frame_class == -1,
              "DM2_query_48ae_011a returns -1 when entry 8 absent");
        CHECK(dm2_v1_skproject_query_48ae_011a(
                  0x0c00u, &pools, cycle13_loadable_fn,
                  (void *)loadable_8_only, &frame_class, &q48ae011a) == 1 &&
                  frame_class == 3,
              "DM2_query_48ae_011a returns 3 when only entry 8 is loadable");
        CHECK(dm2_v1_skproject_query_48ae_011a(
                  0x0c00u, &pools, cycle13_loadable_fn,
                  (void *)loadable_8_9_c, &frame_class, &q48ae011a) == 1 &&
                  frame_class == 0,
              "DM2_query_48ae_011a returns 0 when entries 8/9/c loadable");
        CHECK(dm2_v1_skproject_query_48ae_011a(
                  0x0c00u, &pools, cycle13_loadable_fn, (void *)loadable_all,
                  &frame_class, &q48ae011a) == 1 &&
                  frame_class == 1,
              "DM2_query_48ae_011a returns 1 when entry 0xa is also loadable");
    }

    /* DM2_query_0cee_2e09 */
    CHECK(!dm2_v1_skproject_query_0cee_2e09(
              0xffffu, &ai_spec, &word32, &q2e09) &&
              q2e09.blocked_object_null,
          "DM2_query_0cee_2e09 fails closed on OBJECT_NULL");
    CHECK(!dm2_v1_skproject_query_0cee_2e09(
              0x1234u, NULL, &word32, &q2e09) &&
              q2e09.blocked_missing_ai_spec,
          "DM2_query_0cee_2e09 fails closed without AI spec");
    ai_spec.word32 = 0xbeefu;
    CHECK(dm2_v1_skproject_query_0cee_2e09(
              0x1234u, &ai_spec, &word32, &q2e09) == 1 &&
              q2e09.valid && word32 == 0xbeefu,
          "DM2_query_0cee_2e09 returns AI spec word32");
}

/* ---- cycle-14 c_querydb query batch fakes ---- */

typedef struct {
    uint8_t container_record[8];
    int32_t cls2;
    uint16_t types[8]; /* distinctive item types, indexed by handle & 0xff */
    int32_t next[8];   /* next record links, indexed by handle & 0xff */
} Cycle14RecordDb;

static const uint8_t *cycle14_container_accessor(
    uint16_t handle, uint16_t *out_size, void *user)
{
    Cycle14RecordDb *db = (Cycle14RecordDb *)user;
    if ((handle & 0x3c00u) != 0x2400u)
        return NULL;
    if (out_size) *out_size = (uint16_t)sizeof(db->container_record);
    return db->container_record;
}

static int32_t cycle14_cls2_fn(uint16_t handle, void *user)
{
    Cycle14RecordDb *db = (Cycle14RecordDb *)user;
    (void)handle;
    return db->cls2;
}

static uint16_t cycle14_type_fn(uint16_t handle, void *user)
{
    Cycle14RecordDb *db = (Cycle14RecordDb *)user;
    uint16_t idx = (uint16_t)(handle & 0xffu);
    return (idx < 8u) ? db->types[idx] : 0u;
}

static int32_t cycle14_next_fn(uint16_t handle, void *user)
{
    Cycle14RecordDb *db = (Cycle14RecordDb *)user;
    uint16_t idx = (uint16_t)(handle & 0xffu);
    return (idx < 8u) ? db->next[idx] : -1;
}

static uint16_t cycle14_gdat_word_fn(
    uint8_t creature_type, uint8_t word_index, void *user)
{
    const uint16_t *value = (const uint16_t *)user;
    (void)creature_type;
    (void)word_index;
    return value ? *value : 0u;
}

typedef struct {
    int16_t creature_x;
    int16_t creature_y;
    int32_t creature; /* handle at (creature_x, creature_y); -1 = none */
    uint8_t record[16];
    DM2_V1_SkprojectCreatureAISpec spec;
    uint16_t pos5x5;
    int q098d_fail;
} Cycle14CreatureDb;

static int32_t cycle14_creature_at(int16_t x, int16_t y, void *user)
{
    Cycle14CreatureDb *db = (Cycle14CreatureDb *)user;
    return (x == db->creature_x && y == db->creature_y) ? db->creature : -1;
}

static const uint8_t *cycle14_record16_accessor(
    uint16_t handle, uint16_t *out_size, void *user)
{
    Cycle14CreatureDb *db = (Cycle14CreatureDb *)user;
    (void)handle;
    if (out_size) *out_size = (uint16_t)sizeof(db->record);
    return db->record;
}

static const DM2_V1_SkprojectCreatureAISpec *cycle14_ai_spec_fn(
    uint8_t creature_type, void *user)
{
    Cycle14CreatureDb *db = (Cycle14CreatureDb *)user;
    (void)creature_type;
    return &db->spec;
}

static uint16_t cycle14_pos5x5_fn(
    const uint8_t *record, uint16_t record_size, uint8_t rotation_param,
    void *user)
{
    Cycle14CreatureDb *db = (Cycle14CreatureDb *)user;
    (void)record;
    (void)record_size;
    (void)rotation_param;
    return db->pos5x5;
}

static int cycle14_q098d_fn(
    int16_t x, int16_t y, int16_t value, int16_t *out_x, int16_t *out_y,
    void *user)
{
    Cycle14CreatureDb *db = (Cycle14CreatureDb *)user;
    if (db->q098d_fail)
        return 0;
    *out_x = x;
    *out_y = (int16_t)(y + value);
    return 1;
}

typedef struct {
    int16_t start_x;
    int16_t start_y;
    uint8_t start_tile;
    uint8_t neighbour_tile;
} Cycle14TileDb;

static uint8_t cycle14_tile_value_fn(int16_t x, int16_t y, void *user)
{
    Cycle14TileDb *db = (Cycle14TileDb *)user;
    return (x == db->start_x && y == db->start_y)
               ? db->start_tile
               : db->neighbour_tile;
}

static void test_skwin_core_symbol_batch_cycle14(void)
{
    DM2_V1_SkprojectQuery1c9a03cfReceipt q03cf;
    DM2_V1_SkprojectQuery48ae01afReceipt q01af;
    DM2_V1_SkprojectQuery0cee2e35Receipt q2e35;
    DM2_V1_SkprojectQueryCreaturePicstReceipt qpicst;
    DM2_V1_SkprojectQuery2fcf164eReceipt q164e;
    DM2_V1_SkprojectQuery2fcf16ffReceipt q16ff;
    DM2_V1_SkprojectQuery48ae0767Receipt q0767;
    DM2_V1_SkprojectQuery0cee06dcReceipt q06dc;
    Cycle14RecordDb rdb;
    Cycle14CreatureDb cdb;
    Cycle14TileDb tdb;
    DM2_V1_SkprojectPartyState party;
    uint16_t gdat_word;
    uint16_t w16;
    uint32_t handle32;
    int32_t total_weight;
    int16_t x;
    int16_t y;
    uint8_t v8;
    uint8_t out_indices[8];
    uint16_t out_written;
    uint8_t picst_record[16];
    uint8_t palette_state[8];

    const int8_t table2660[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    const int16_t table2752[4] = { 8, 9, 10, 11 };
    const int16_t table62b0[8][2] = {
        { 1, 0 }, { 1, 0 }, { 0, 1 }, { 0, 1 },
        { -1, 0 }, { -1, 0 }, { 0, -1 }, { 0, -1 }
    };
    const int16_t table62d0[5][2] = {
        { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }
    };
    const int16_t table62e0[4] = { 1, 4, 9, 16 };
    const int8_t table62e8[4] = { 0, 1, 2, 3 };
    const int16_t table27fc[2] = { 1, 0 };
    const int16_t table2804[2] = { 0, 1 };

    /* DM2_query_1c9a_03cf */
    memset(&cdb, 0, sizeof(cdb));
    cdb.creature_x = 10;
    cdb.creature_y = 20;
    cdb.creature = 0x9004;
    cdb.record[4] = 0x05u; /* creature type */
    cdb.record[14] = 0x40u; /* word@0xe = 0x40 -> >>6 = 1 */
    cdb.record[15] = 0x00u;
    cdb.spec.word34 = 0x0100u; /* byte@0x23 = 1 -> threshold table62e0[1] */
    cdb.pos5x5 = 13u; /* dy = 13 - 12 -> dist2 = 1 < 4 */
    x = 10;
    y = 20;
    CHECK(!dm2_v1_skproject_query_1c9a_03cf(
              &x, &y, 0xffu, NULL, cycle14_record16_accessor,
              cycle14_ai_spec_fn, cycle14_pos5x5_fn, cycle14_q098d_fn, &cdb,
              table2752, 4u, table62b0, 8u, table62d0, 5u,
              table62e0, 4u, table62e8, 4u, &handle32, &q03cf) &&
              q03cf.blocked_missing_callback && handle32 == 0xffffu,
          "DM2_query_1c9a_03cf fails closed without creature callback");
    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_query_1c9a_03cf(
              &x, &y, 0xffu, cycle14_creature_at, cycle14_record16_accessor,
              cycle14_ai_spec_fn, cycle14_pos5x5_fn, cycle14_q098d_fn, &cdb,
              table2752, 4u, table62b0, 8u, table62d0, 5u,
              table62e0, 4u, table62e8, 4u, &handle32, &q03cf) == 1 &&
              q03cf.valid && q03cf.found && handle32 == 0x9004u &&
              x == 10 && y == 20 && q03cf.range == 12 &&
              q03cf.distance2 == 1 && q03cf.threshold == 4 &&
              q03cf.ai_spec_byte23 == 1,
          "DM2_query_1c9a_03cf returns the nearest creature below threshold");
    x = 10;
    y = 20;
    cdb.creature = -1; /* no creature anywhere */
    CHECK(!dm2_v1_skproject_query_1c9a_03cf(
              &x, &y, 0xffu, cycle14_creature_at, cycle14_record16_accessor,
              cycle14_ai_spec_fn, cycle14_pos5x5_fn, cycle14_q098d_fn, &cdb,
              table2752, 4u, table62b0, 8u, table62d0, 5u,
              table62e0, 4u, table62e8, 4u, &handle32, &q03cf) &&
              q03cf.valid && !q03cf.found && handle32 == 0xffffu &&
              q03cf.steps == 5u,
          "DM2_query_1c9a_03cf exhausts the five-cell scan and fails");
    x = 10;
    y = 20;
    CHECK(!dm2_v1_skproject_query_1c9a_03cf(
              &x, &y, 0x05u, cycle14_creature_at, cycle14_record16_accessor,
              cycle14_ai_spec_fn, cycle14_pos5x5_fn, cycle14_q098d_fn, &cdb,
              table2752, 4u, table62b0, 8u, table62d0, 5u,
              table62e0, 4u, table62e8, 4u, &handle32, &q03cf) &&
              q03cf.blocked_table_bounds,
          "DM2_query_1c9a_03cf fails closed past the range table");

    /* DM2_query_48ae_01af */
    CHECK(dm2_v1_skproject_query_48ae_01af(
              0x0402u, 5u, table2660, 16u, &v8, &q01af) == 1 &&
              q01af.valid && q01af.cls2 == 2u && v8 == 9u,
          "DM2_query_48ae_01af reads table1d2660[4*cls2 + offset - 4]");
    CHECK(dm2_v1_skproject_query_48ae_01af(
              0x0602u, 5u, table2660, 16u, &v8, &q01af) == 1 &&
              v8 == 0x0fu,
          "DM2_query_48ae_01af returns 0xf when bit 9 is set");
    CHECK(dm2_v1_skproject_query_48ae_01af(
              0x0400u, 5u, table2660, 16u, &v8, &q01af) == 1 &&
              v8 == 0u,
          "DM2_query_48ae_01af returns 0 when cls2 is zero");
    CHECK(!dm2_v1_skproject_query_48ae_01af(
              0x0402u, 5u, NULL, 16u, &v8, &q01af) &&
              q01af.blocked_missing_table && v8 == 0x0fu,
          "DM2_query_48ae_01af fails closed without the table");
    CHECK(!dm2_v1_skproject_query_48ae_01af(
              0x040fu, 5u, table2660, 16u, &v8, &q01af) &&
              q01af.blocked_table_bounds && v8 == 0x0fu,
          "DM2_query_48ae_01af fails closed past the table");

    /* DM2_query_0cee_2e35 */
    gdat_word = 7u;
    CHECK(dm2_v1_skproject_query_0cee_2e35(
              0x05u, cycle14_gdat_word_fn, &gdat_word, &w16, &q2e35) == 1 &&
              q2e35.valid && q2e35.gdat_value == 7u && w16 == 7u,
          "DM2_query_0cee_2e35 returns the GDAT creature word");
    gdat_word = 0u;
    CHECK(dm2_v1_skproject_query_0cee_2e35(
              0x05u, cycle14_gdat_word_fn, &gdat_word, &w16, &q2e35) == 1 &&
              w16 == 4u,
          "DM2_query_0cee_2e35 substitutes 4 for a zero word");
    CHECK(!dm2_v1_skproject_query_0cee_2e35(
              0x05u, NULL, NULL, &w16, &q2e35) &&
              q2e35.blocked_missing_callback && w16 == 4u,
          "DM2_query_0cee_2e35 fails closed without the GDAT callback");

    /* DM2_QUERY_CREATURE_PICST */
    memset(picst_record, 0, sizeof(picst_record));
    picst_record[4] = 0x12u;
    picst_record[14] = 0x34u;
    picst_record[15] = 0x12u;
    memset(palette_state, 0, sizeof(palette_state));
    palette_state[7] = 0x55u;
    CHECK(!dm2_v1_skproject_query_creature_picst(
              3, 4, NULL, 0u, palette_state, 0u, &qpicst) &&
              qpicst.blocked_missing_record,
          "DM2_QUERY_CREATURE_PICST fails closed without a record");
    CHECK(!dm2_v1_skproject_query_creature_picst(
              3, 4, picst_record, (uint16_t)sizeof(picst_record),
              palette_state, 0x21u, &qpicst) &&
              qpicst.blocked_picture_query && qpicst.creature_type == 0x12u &&
              qpicst.creature_word_e == 0x1234u &&
              qpicst.palette_state_present && qpicst.palette_byte7 == 0x55u &&
              qpicst.argw0 == 0x21u,
          "DM2_QUERY_CREATURE_PICST decodes inputs and fails closed on blit");

    /* DM2_query_2fcf_164e */
    memset(&rdb, 0, sizeof(rdb));
    rdb.cls2 = 2;
    rdb.container_record[2] = 0x03u; /* first child handle 0x0003 */
    rdb.container_record[3] = 0x00u;
    rdb.types[3] = 0x0042u;
    CHECK(dm2_v1_skproject_query_2fcf_164e(
              0x2421u, 0x0042u, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q164e) == 1 &&
              q164e.valid && q164e.found && q164e.matched_handle == 0x0003u &&
              q164e.steps == 1u && q164e.cls2 == 2u,
          "DM2_query_2fcf_164e finds the distinctive type in the chain");
    CHECK(!dm2_v1_skproject_query_2fcf_164e(
              0x0021u, 0x0042u, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q164e) &&
              q164e.valid && q164e.blocked_not_container,
          "DM2_query_2fcf_164e rejects non-container record types");
    rdb.types[3] = 0u;
    rdb.next[3] = 0x0004;
    rdb.types[4] = 0x0042u;
    CHECK(dm2_v1_skproject_query_2fcf_164e(
              0x2421u, 0x0042u, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q164e) == 1 &&
              q164e.found && q164e.matched_handle == 0x0004u &&
              q164e.steps == 2u,
          "DM2_query_2fcf_164e follows next-record links");
    rdb.types[4] = 0u;
    rdb.next[3] = 0xfffe;
    CHECK(!dm2_v1_skproject_query_2fcf_164e(
              0x2421u, 0x0042u, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q164e) &&
              q164e.valid && !q164e.found,
          "DM2_query_2fcf_164e stops at the 0xfffe chain terminator");
    rdb.cls2 = 8;
    CHECK(!dm2_v1_skproject_query_2fcf_164e(
              0x2421u, 0x0042u, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q164e) &&
              q164e.valid && q164e.blocked_cls2_range,
          "DM2_query_2fcf_164e rejects cls2 >= 8");
    rdb.cls2 = 2;

    /* DM2_query_2fcf_16ff */
    memset(&party, 0, sizeof(party));
    party.hero_count = 1u;
    party.heroes[0].cur_hp = 10u;
    for (v8 = 0u; v8 < 30u; ++v8)
        party.heroes[0].inventory[v8] = 0xffffu;
    for (v8 = 0u; v8 < 8u; ++v8)
        party.hand_containers[v8] = 0xffffu;
    party.wielded = 0xffffu;
    party.heroes[0].inventory[0] = 0x0005u;
    rdb.types[5] = 0x0042u;
    CHECK(dm2_v1_skproject_query_2fcf_16ff(
              0x0042u, &party, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q16ff) == 1 &&
              q16ff.valid && q16ff.found && q16ff.hero_index == 0u &&
              q16ff.matched_handle == 0x0005u,
          "DM2_query_2fcf_16ff finds the type in a hero inventory");
    rdb.types[5] = 0u;
    party.heroes[0].inventory[0] = 0xffffu;
    party.hand_container_mode = 5;
    party.hand_containers[2] = 0x0006u;
    rdb.types[6] = 0x0042u;
    CHECK(dm2_v1_skproject_query_2fcf_16ff(
              0x0042u, &party, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q16ff) == 1 &&
              q16ff.found && q16ff.from_hand_container &&
              q16ff.matched_handle == 0x0006u,
          "DM2_query_2fcf_16ff scans hand containers when mode is 5");
    rdb.types[6] = 0u;
    party.hand_containers[2] = 0xffffu;
    party.hand_container_mode = 0;
    party.wielded = 0x0007u;
    rdb.types[7] = 0x0042u;
    CHECK(dm2_v1_skproject_query_2fcf_16ff(
              0x0042u, &party, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q16ff) == 1 &&
              q16ff.found && q16ff.from_wielded &&
              q16ff.matched_handle == 0x0007u,
          "DM2_query_2fcf_16ff checks the wielded object");
    rdb.types[7] = 0u;
    party.wielded = 0xffffu;
    CHECK(!dm2_v1_skproject_query_2fcf_16ff(
              0x0042u, &party, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q16ff) &&
              q16ff.valid && !q16ff.found,
          "DM2_query_2fcf_16ff reports not-found without a match");
    CHECK(!dm2_v1_skproject_query_2fcf_16ff(
              0x0042u, NULL, cycle14_container_accessor, cycle14_cls2_fn,
              cycle14_type_fn, cycle14_next_fn, &rdb, &q16ff) &&
              q16ff.blocked_missing_party,
          "DM2_query_2fcf_16ff fails closed without party state");

    /* DM2_query_48ae_0767 */
    {
        const int16_t weights_a[3] = { 3, 4, 5 };
        const int16_t weights_b[3] = { 2, 5, 3 };
        CHECK(dm2_v1_skproject_query_48ae_0767(
                  10, 4u, out_indices, &out_written, 3u, weights_a,
                  &total_weight, &q0767) == 1 &&
                  q0767.valid && out_written == 2u && total_weight == 10 &&
                  out_indices[0] == 2u && out_indices[1] == 2u,
              "DM2_query_48ae_0767 packs the last item until capacity ends");
        CHECK(dm2_v1_skproject_query_48ae_0767(
                  4, 4u, out_indices, &out_written, 3u, weights_b,
                  &total_weight, &q0767) == 1 &&
                  out_written == 1u && total_weight == 3 &&
                  out_indices[0] == 2u,
              "DM2_query_48ae_0767 skips items too heavy and continues");
        CHECK(!dm2_v1_skproject_query_48ae_0767(
                  4, 4u, NULL, &out_written, 3u, weights_b,
                  &total_weight, &q0767) &&
                  q0767.blocked_missing_output,
              "DM2_query_48ae_0767 fails closed without outputs");
    }

    /* DM2_query_0cee_06dc */
    tdb.start_x = 5;
    tdb.start_y = 6;
    tdb.start_tile = 0x08u; /* bit 3 set -> bit = 0 -> neighbour east */
    tdb.neighbour_tile = 0x60u; /* type 3 */
    CHECK(dm2_v1_skproject_query_0cee_06dc(
              5, 6, cycle14_tile_value_fn, &tdb, table27fc, table2804,
              &v8, &q06dc) == 1 &&
              q06dc.valid && q06dc.bit == 0u && q06dc.neighbour_x == 6 &&
              q06dc.neighbour_y == 6 && q06dc.neighbour_type == 3u &&
              v8 == 2u,
          "DM2_query_0cee_06dc returns 2 + bit for a type-3 neighbour");
    tdb.start_tile = 0x00u; /* bit 3 clear -> bit = 1 -> neighbour south */
    tdb.neighbour_tile = 0x20u; /* type 1 */
    CHECK(dm2_v1_skproject_query_0cee_06dc(
              5, 6, cycle14_tile_value_fn, &tdb, table27fc, table2804,
              &v8, &q06dc) == 1 &&
              q06dc.bit == 1u && q06dc.neighbour_x == 5 &&
              q06dc.neighbour_y == 7 && q06dc.neighbour_type == 1u &&
              v8 == 1u,
          "DM2_query_0cee_06dc returns bit for other neighbour types");
    CHECK(!dm2_v1_skproject_query_0cee_06dc(
              5, 6, NULL, &tdb, table27fc, table2804, &v8, &q06dc) &&
              q06dc.blocked_missing_callback,
          "DM2_query_0cee_06dc fails closed without tile access");
}

/* ---- cycle-15 c_querydb / c_1c9a query batch fakes ---- */

typedef struct {
    int16_t current_map;
    int16_t start_map;
    uint8_t start_tile;
    uint8_t target_tile;
    int16_t locate_map;
    int16_t locate_result;
    int ladder_result;
    int32_t change_map_calls[8];
    int change_map_count;
} Cycle15TransitionDb;

static int32_t cycle15_change_map_fn(int16_t map, void *user)
{
    Cycle15TransitionDb *db = (Cycle15TransitionDb *)user;
    db->current_map = map;
    if (db->change_map_count < 8)
        db->change_map_calls[db->change_map_count++] = map;
    return 0;
}

static uint8_t cycle15_transition_tile_fn(int16_t x, int16_t y, void *user)
{
    Cycle15TransitionDb *db = (Cycle15TransitionDb *)user;
    (void)x;
    (void)y;
    return (db->current_map == db->locate_map) ? db->target_tile
                                               : db->start_tile;
}

static int32_t cycle15_ladder_fn(
    int16_t x, int16_t y, int16_t direction, void *user)
{
    Cycle15TransitionDb *db = (Cycle15TransitionDb *)user;
    (void)x;
    (void)y;
    (void)direction;
    return db->ladder_result;
}

static int32_t cycle15_locate_fn(
    int16_t map, int16_t direction, int16_t *x, int16_t *y, void *user)
{
    Cycle15TransitionDb *db = (Cycle15TransitionDb *)user;
    (void)map;
    (void)direction;
    (void)x;
    (void)y;
    return db->locate_result;
}

typedef struct {
    uint16_t ai_flags;
    int32_t cls2;
    uint8_t list[8];
    uint16_t list_count;
    int null_list;
    uint16_t gdat_value;
    uint8_t gdat_cls1;
    uint8_t gdat_cls2;
    uint8_t gdat_entry;
    uint8_t gdat_data;
} Cycle15AllowDb;

static uint16_t cycle15_ai_flags_fn(uint16_t handle, void *user)
{
    Cycle15AllowDb *db = (Cycle15AllowDb *)user;
    (void)handle;
    return db->ai_flags;
}

static int32_t cycle15_cls2_fn(uint16_t handle, void *user)
{
    Cycle15AllowDb *db = (Cycle15AllowDb *)user;
    (void)handle;
    return db->cls2;
}

static const uint8_t *cycle15_list_fn(
    int16_t level, uint16_t *out_count, void *user)
{
    Cycle15AllowDb *db = (Cycle15AllowDb *)user;
    (void)level;
    *out_count = db->list_count;
    return db->null_list ? NULL : db->list;
}

static uint16_t cycle15_gdat_index_fn(
    uint8_t cls1, uint8_t cls2, uint8_t entry_index, uint8_t data_index,
    void *user)
{
    Cycle15AllowDb *db = (Cycle15AllowDb *)user;
    db->gdat_cls1 = cls1;
    db->gdat_cls2 = cls2;
    db->gdat_entry = entry_index;
    db->gdat_data = data_index;
    return db->gdat_value;
}

typedef struct {
    uint8_t tile;
    int32_t tile_record;
    int32_t rebirth;
    int32_t door_gdat;
    int randbit;
    int32_t chain[4];
    int chain_len;
    uint8_t record[16];
    int32_t creature; /* handle at the cell; -1 = none */
    DM2_V1_SkprojectCreatureAISpec spec;
    uint16_t pos5x5;
    uint16_t creature_flags;
} Cycle15PassDb;

static uint8_t cycle15_pass_tile_fn(int16_t x, int16_t y, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)x;
    (void)y;
    return db->tile;
}

static int32_t cycle15_tile_record_fn(int16_t x, int16_t y, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)x;
    (void)y;
    return db->tile_record;
}

static int32_t cycle15_rebirth_fn(int32_t record, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)record;
    return db->rebirth;
}

static int32_t cycle15_door_gdat_fn(uint8_t value, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)value;
    return db->door_gdat;
}

static int cycle15_randbit_fn(void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    return db->randbit;
}

static int32_t cycle15_wall_record_fn(int16_t x, int16_t y, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)x;
    (void)y;
    return (db->chain_len > 0) ? db->chain[0] : 0xfffe;
}

static const uint8_t *cycle15_pass_record_fn(
    uint16_t handle, uint16_t *out_size, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)handle;
    if (out_size) *out_size = (uint16_t)sizeof(db->record);
    return db->record;
}

static int32_t cycle15_pass_next_fn(uint16_t handle, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    int i;
    for (i = 0; i + 1 < db->chain_len; ++i) {
        if ((uint16_t)db->chain[i] == handle)
            return db->chain[i + 1];
    }
    return 0xfffe;
}

static uint16_t cycle15_pass_ai_flags_fn(uint16_t handle, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)handle;
    return db->creature_flags;
}

static int32_t cycle15_pass_creature_at_fn(int16_t x, int16_t y, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)x;
    (void)y;
    return db->creature;
}

static const DM2_V1_SkprojectCreatureAISpec *cycle15_pass_ai_spec_fn(
    uint8_t creature_type, void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)creature_type;
    return &db->spec;
}

static uint16_t cycle15_pass_pos5x5_fn(
    const uint8_t *record, uint16_t record_size, uint8_t rotation_param,
    void *user)
{
    Cycle15PassDb *db = (Cycle15PassDb *)user;
    (void)record;
    (void)record_size;
    (void)rotation_param;
    return db->pos5x5;
}

static int cycle15_pass_q098d_fn(
    int16_t x, int16_t y, int16_t value, int16_t *out_x, int16_t *out_y,
    void *user)
{
    (void)user;
    *out_x = x;
    *out_y = (int16_t)(y + value);
    return 1;
}

typedef struct {
    int16_t xs[16];
    int16_t ys[16];
    int count;
    int abort_at; /* 1-based call index that returns nonzero; 0 = never */
} Cycle15LineDb;

static int cycle15_line_cell_fn(int16_t x, int16_t y, void *user)
{
    Cycle15LineDb *db = (Cycle15LineDb *)user;
    db->count++;
    if (db->count <= 16) {
        db->xs[db->count - 1] = x;
        db->ys[db->count - 1] = y;
    }
    return (db->abort_at == db->count) ? 1 : 0;
}

static void test_skwin_core_symbol_batch_cycle15(void)
{
    DM2_V1_SkprojectQuery19f0124bReceipt q124b;
    DM2_V1_SkprojectQuery29ee18ebReceipt q18eb;
    DM2_V1_SkprojectIsCreatureAllowedOnLevelReceipt qallow;
    DM2_V1_SkprojectQuery0cee319eReceipt q319e;
    DM2_V1_Skproject1baadReceipt q1baad;
    DM2_V1_Skproject1bc29Receipt q1bc29;
    DM2_V1_Skproject19f00207Receipt q0207;
    DM2_V1_Skproject19f0045aReceipt q045a;
    DM2_V1_Skproject29ee18ebState st18eb;
    DM2_V1_Skproject1baadContext ctx;
    DM2_V1_Skproject1bc29Cache cache;
    DM2_V1_Skproject19f0045aState st045a;
    Cycle15TransitionDb tdb;
    Cycle15AllowDb adb;
    Cycle15PassDb pdb;
    Cycle15LineDb ldb;
    int16_t x;
    int16_t y;
    int32_t result;
    uint16_t w16;

    const int16_t table2752[4] = { 8, 9, 10, 11 };
    const int16_t table62b0[8][2] = {
        { 1, 0 }, { 1, 0 }, { 0, 1 }, { 0, 1 },
        { -1, 0 }, { -1, 0 }, { 0, -1 }, { 0, -1 }
    };
    const int16_t table62d0[5][2] = {
        { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }
    };
    const int16_t table62e0[4] = { 1, 4, 9, 16 };
    const int8_t table62e8[4] = { 0, 1, 2, 3 };

    /* DM2_query_19f0_124b */
    memset(&tdb, 0, sizeof(tdb));
    tdb.start_map = 3;
    tdb.current_map = 3;
    tdb.locate_map = 8;
    tdb.locate_result = 8;
    tdb.ladder_result = -1;
    tdb.start_tile = 0x60u; /* type 3 stairs, bit 2 clear */
    tdb.target_tile = 0x48u; /* open pit on the target map */
    x = 4;
    y = 5;
    CHECK(!dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, 1, 0x010u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) &&
              q124b.valid && result == -1,
          "DM2_query_19f0_124b rejects stairs without flags 0x100");
    x = 4;
    y = 5;
    tdb.change_map_count = 0;
    CHECK(dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, 1, 0x110u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) == 1 &&
              q124b.admitted_stairs && result == 8 && !q124b.fallthrough,
          "DM2_query_19f0_124b admits stairs down with flags 0x100");
    tdb.start_tile = 0x6cu; /* stairs, bit 2 set -> direction -1 */
    x = 4;
    y = 5;
    CHECK(!dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, 1, 0x108u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) &&
              result == -1,
          "DM2_query_19f0_124b rejects wrong stairs direction");
    tdb.start_tile = 0x48u; /* type 2 open pit (bit 3 set, bit 0 clear) */
    x = 4;
    y = 5;
    CHECK(dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, 1, 0x108u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) == 1 &&
              q124b.admitted_pit && result == 8,
          "DM2_query_19f0_124b admits open pit with flags 0x8 direction 1");
    tdb.start_tile = 0x00u; /* type 0 wall */
    x = 4;
    y = 5;
    CHECK(!dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, 1, 0x108u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) &&
              result == -1,
          "DM2_query_19f0_124b rejects type-0 tiles");
    tdb.start_tile = 0xa2u; /* type 5, bit 1 set */
    tdb.ladder_result = 0;
    x = 4;
    y = 5;
    CHECK(dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, 1, 0x108u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) == 1 &&
              q124b.ladder_found && result == 8,
          "DM2_query_19f0_124b admits ladder targets");
    tdb.ladder_result = -1;
    x = 4;
    y = 5;
    tdb.change_map_count = 0;
    CHECK(dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, -1, 0x010u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) == 1 &&
              q124b.fallthrough && q124b.target_admitted && result == 8 &&
              tdb.change_map_calls[tdb.change_map_count - 1] == 3,
          "DM2_query_19f0_124b validates fall target and restores the map");
    tdb.target_tile = 0xa0u; /* type 5, not an open pit */
    x = 4;
    y = 5;
    CHECK(!dm2_v1_skproject_query_19f0_124b(
              &x, &y, 3, -1, 0x010u, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &result, &q124b) &&
              q124b.fallthrough && !q124b.target_admitted && result == -1,
          "DM2_query_19f0_124b rejects falls onto non-pit targets");
    tdb.target_tile = 0x48u;

    /* DM2_query_29ee_18eb */
    memset(&st18eb, 0, sizeof(st18eb));
    tdb.start_tile = 0xa2u; /* type 5, bit 1: ladder path both ways */
    tdb.ladder_result = 0;
    tdb.locate_result = 6;
    CHECK(dm2_v1_skproject_query_29ee_18eb(
              4u, 5u, 3u, &st18eb, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &q18eb) == 1 &&
              q18eb.valid && q18eb.down_result == 6 &&
              q18eb.up_result == 6 && st18eb.v1e0b68 == 4u &&
              st18eb.v1e0b6a == 5u && st18eb.v1e0b60 == 6u &&
              st18eb.v1e0b66 == 6u && st18eb.v1e0b64 == 3u,
          "DM2_query_29ee_18eb runs the down/up transition pair");
    CHECK(!dm2_v1_skproject_query_29ee_18eb(
              4u, 5u, 3u, NULL, cycle15_change_map_fn,
              cycle15_transition_tile_fn, cycle15_ladder_fn,
              cycle15_locate_fn, &tdb, &q18eb) &&
              q18eb.blocked_missing_state,
          "DM2_query_29ee_18eb fails closed without state");

    /* DM2_IS_CREATURE_ALLOWED_ON_LEVEL */
    memset(&adb, 0, sizeof(adb));
    adb.cls2 = 3;
    adb.list[0] = 1u;
    adb.list[1] = 3u;
    adb.list[2] = 5u;
    adb.list_count = 3u;
    CHECK(dm2_v1_skproject_is_creature_allowed_on_level(
              0x1234u, 2, cycle15_ai_flags_fn, cycle15_cls2_fn,
              cycle15_list_fn, &adb, &qallow) == 1 &&
              qallow.allowed && qallow.cls2 == 3u && qallow.checked == 2u,
          "DM2_IS_CREATURE_ALLOWED_ON_LEVEL matches cls2 in the level list");
    adb.list[1] = 7u;
    CHECK(!dm2_v1_skproject_is_creature_allowed_on_level(
              0x1234u, 2, cycle15_ai_flags_fn, cycle15_cls2_fn,
              cycle15_list_fn, &adb, &qallow) &&
              qallow.valid && !qallow.allowed && qallow.checked == 3u,
          "DM2_IS_CREATURE_ALLOWED_ON_LEVEL rejects unlisted cls2");
    adb.ai_flags = 0x4000u;
    CHECK(dm2_v1_skproject_is_creature_allowed_on_level(
              0x1234u, 2, cycle15_ai_flags_fn, cycle15_cls2_fn,
              cycle15_list_fn, &adb, &qallow) == 1 &&
              qallow.ai_flag_override && qallow.allowed,
          "DM2_IS_CREATURE_ALLOWED_ON_LEVEL allows AI flag 0x40 override");
    adb.ai_flags = 0u;
    adb.null_list = 1;
    CHECK(!dm2_v1_skproject_is_creature_allowed_on_level(
              0x1234u, 2, cycle15_ai_flags_fn, cycle15_cls2_fn,
              cycle15_list_fn, &adb, &qallow) &&
              qallow.blocked_missing_list,
          "DM2_IS_CREATURE_ALLOWED_ON_LEVEL fails closed without the list");
    adb.null_list = 0;
    CHECK(!dm2_v1_skproject_is_creature_allowed_on_level(
              0x1234u, 2, NULL, cycle15_cls2_fn, cycle15_list_fn, &adb,
              &qallow) &&
              qallow.blocked_missing_callback,
          "DM2_IS_CREATURE_ALLOWED_ON_LEVEL fails closed without callbacks");

    /* DM2_query_0cee_319e */
    adb.cls2 = 0xff;
    adb.gdat_value = 0x1234u;
    CHECK(dm2_v1_skproject_query_0cee_319e(
              0x1234u, cycle15_cls2_fn, cycle15_gdat_index_fn, &adb, &w16,
              &q319e) == 1 &&
              q319e.valid && w16 == 0u,
          "DM2_query_0cee_319e returns 0 for cls2 0xff");
    adb.cls2 = 5;
    CHECK(dm2_v1_skproject_query_0cee_319e(
              0x1234u, cycle15_cls2_fn, cycle15_gdat_index_fn, &adb, &w16,
              &q319e) == 1 &&
              w16 == 0x1234u && adb.gdat_cls1 == 9u &&
              adb.gdat_cls2 == 5u && adb.gdat_entry == 11u &&
              adb.gdat_data == 11u,
          "DM2_query_0cee_319e queries GDAT entry 9 data 11 by cls2");
    CHECK(!dm2_v1_skproject_query_0cee_319e(
              0x1234u, NULL, cycle15_gdat_index_fn, &adb, &w16, &q319e) &&
              q319e.blocked_missing_callback && w16 == 0u,
          "DM2_query_0cee_319e fails closed without cls2 callback");

    /* DM2_1BAAD */
    memset(&pdb, 0, sizeof(pdb));
    memset(&ctx, 0, sizeof(ctx));
    ctx.user = &pdb;
    ctx.tile_fn = cycle15_pass_tile_fn;
    ctx.tile_record_fn = cycle15_tile_record_fn;
    ctx.rebirth_fn = cycle15_rebirth_fn;
    ctx.door_gdat_fn = cycle15_door_gdat_fn;
    ctx.randbit_fn = cycle15_randbit_fn;
    ctx.wall_record_fn = cycle15_wall_record_fn;
    ctx.record_fn = cycle15_pass_record_fn;
    ctx.next_fn = cycle15_pass_next_fn;
    ctx.ai_flags_fn = cycle15_pass_ai_flags_fn;
    ctx.creature_at_fn = cycle15_pass_creature_at_fn;
    ctx.ai_spec_fn = cycle15_pass_ai_spec_fn;
    ctx.pos5x5_fn = cycle15_pass_pos5x5_fn;
    ctx.q098d_fn = cycle15_pass_q098d_fn;
    ctx.table1d2752 = table2752;
    ctx.table1d2752_size = 4u;
    ctx.table1d62b0 = table62b0;
    ctx.table1d62b0_rows = 8u;
    ctx.table1d62d0 = table62d0;
    ctx.table1d62d0_rows = 5u;
    ctx.table1d62e0 = table62e0;
    ctx.table1d62e0_size = 4u;
    ctx.table1d62e8 = table62e8;
    ctx.table1d62e8_size = 4u;

    pdb.tile = 0x00u; /* type 0 */
    CHECK(dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) == 1 &&
              q1baad.passable && q1baad.tile_type == 0u,
          "DM2_1BAAD passes type-0 tiles");
    pdb.tile = (uint8_t)((4u << 5) | 0x10u | 3u); /* door variant 3 */
    pdb.door_gdat = 0;
    CHECK(dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) == 1 &&
              q1baad.via_door && q1baad.door_variant == 3u,
          "DM2_1BAAD passes open doors without GDAT gate");
    pdb.door_gdat = 5;
    pdb.randbit = 1;
    CHECK(!dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) &&
              q1baad.valid && !q1baad.passable && q1baad.randbit == 1,
          "DM2_1BAAD blocks door when GDAT gate wins the randbit");
    pdb.randbit = 0;
    CHECK(dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) == 1 &&
              q1baad.via_door,
          "DM2_1BAAD passes door when the randbit loses");
    pdb.tile = 0xc0u; /* type 6, bit 2 clear */
    CHECK(dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) == 1 &&
              q1baad.via_type6,
          "DM2_1BAAD passes type-6 tiles with bit 2 clear");
    pdb.tile = 0xa0u; /* type 5, no bit 0x10 */
    CHECK(!dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) &&
              q1baad.valid && !q1baad.passable,
          "DM2_1BAAD blocks tiles without bit 0x10");
    pdb.tile = 0x50u; /* type 2 with bit 0x10 */
    pdb.chain_len = 1;
    pdb.chain[0] = 0x3c01; /* record type 0xf */
    pdb.record[2] = 0x0eu;
    pdb.record[3] = 0x00u;
    CHECK(dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) == 1 &&
              q1baad.via_actuator && q1baad.records_checked == 1u,
          "DM2_1BAAD passes on type-0xf record with word@2 0x0e");
    pdb.record[2] = 0x01u;
    pdb.chain[0] = 0x1001; /* record type 4 (creature) */
    pdb.creature = 0x9004;
    pdb.record[4] = 0x05u;
    pdb.record[14] = 0x00u;
    pdb.record[15] = 0x00u;
    pdb.spec.word30 = 0u;
    pdb.spec.word32 = 0u;
    pdb.spec.word34 = 0x0100u; /* byte@0x23 = 1 -> threshold 4 */
    pdb.pos5x5 = 13u; /* dist2 = 1 < 4 */
    pdb.creature_flags = 0x0000u; /* not material, not non-solid */
    CHECK(dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) == 1 &&
              q1baad.via_creature && q1baad.creature_handle == 0x9004u,
          "DM2_1BAAD passes non-material creatures");
    pdb.creature_flags = 0x0081u; /* material, (flags>>6)&3 == 2 */
    CHECK(!dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) &&
              q1baad.valid && !q1baad.passable,
          "DM2_1BAAD blocks material large creatures");
    pdb.chain[0] = 0xfffe;
    CHECK(!dm2_v1_skproject_1baad(4, 5, &ctx, &q1baad) &&
              q1baad.valid && !q1baad.passable,
          "DM2_1BAAD blocks at the chain terminator");
    CHECK(!dm2_v1_skproject_1baad(4, 5, NULL, &q1baad) &&
              q1baad.blocked_missing_callback,
          "DM2_1BAAD fails closed without context");

    /* DM2_1BC29 */
    memset(&cache, 0, sizeof(cache));
    cache.v1d3248 = 3u;
    cache.v1e08d6 = 3u;
    cache.v1e08d8 = 4u;
    cache.v1e08d4 = 5u;
    CHECK(dm2_v1_skproject_1bc29(4u, 5u, &cache, &ctx, &q1bc29) == 1 &&
              q1bc29.cache_hit && q1bc29.passable,
          "DM2_1BC29 passes on the cached transition");
    pdb.tile = 0x00u;
    CHECK(dm2_v1_skproject_1bc29(6u, 7u, &cache, &ctx, &q1bc29) == 1 &&
              !q1bc29.cache_hit && q1bc29.passable &&
              q1bc29.nested.passable,
          "DM2_1BC29 delegates to DM2_1BAAD on cache miss");
    CHECK(!dm2_v1_skproject_1bc29(4u, 5u, NULL, &ctx, &q1bc29) &&
              q1bc29.blocked_missing_cache,
          "DM2_1BC29 fails closed without the cache");

    /* DM2_19f0_0207 */
    memset(&ldb, 0, sizeof(ldb));
    CHECK(dm2_v1_skproject_19f0_0207(0, 0, 1, 0, cycle15_line_cell_fn,
                                     &ldb, &q0207) == 1 &&
              q0207.valid && ldb.count == 0,
          "DM2_19f0_0207 returns 1 for adjacent endpoints");
    CHECK(dm2_v1_skproject_19f0_0207(0, 0, 4, 0, cycle15_line_cell_fn,
                                     &ldb, &q0207) == 4 &&
              q0207.valid && !q0207.aborted && ldb.count == 3 &&
              ldb.xs[0] == 3 && ldb.xs[1] == 2 && ldb.xs[2] == 1 &&
              ldb.ys[0] == 0 && ldb.ys[2] == 0,
          "DM2_19f0_0207 walks the x line back to the start");
    memset(&ldb, 0, sizeof(ldb));
    CHECK(dm2_v1_skproject_19f0_0207(0, 0, 3, 3, cycle15_line_cell_fn,
                                     &ldb, &q0207) == 6 &&
              ldb.count == 6 && ldb.xs[0] == 2 && ldb.ys[0] == 3 &&
              ldb.xs[1] == 2 && ldb.ys[1] == 2 && ldb.xs[2] == 1 &&
              ldb.ys[2] == 2 && ldb.xs[3] == 1 && ldb.ys[3] == 1 &&
              ldb.xs[5] == 0 && ldb.ys[5] == 0,
          "DM2_19f0_0207 steps both axes on diagonals");
    memset(&ldb, 0, sizeof(ldb));
    ldb.abort_at = 2;
    CHECK(dm2_v1_skproject_19f0_0207(0, 0, 4, 0, cycle15_line_cell_fn,
                                     &ldb, &q0207) == 0 &&
              q0207.aborted && q0207.last_x == 2 && q0207.last_y == 0,
          "DM2_19f0_0207 aborts when the callback blocks a cell");
    CHECK(dm2_v1_skproject_19f0_0207(0, 0, 4, 0, NULL, &ldb, &q0207) == 0 &&
              q0207.blocked_missing_callback,
          "DM2_19f0_0207 fails closed without the cell callback");

    /* DM2_19f0_045a */
    memset(&st045a, 0, sizeof(st045a));
    memset(&tdb, 0, sizeof(tdb));
    tdb.locate_map = 1; /* tile fake returns start_tile */
    st045a.v1d3248 = 2u;
    tdb.start_tile = 0x10u;
    CHECK(dm2_v1_skproject_19f0_045a(
              5u, 6u, &st045a, cycle15_transition_tile_fn, &tdb,
              &q045a) == 0xffff &&
              q045a.valid && !q045a.cache_hit && st045a.v1e08a8 == 5u &&
              st045a.v1e08aa == 6u && st045a.v1e08ac == 2u &&
              st045a.v1e08b0 == 0xffffu && st045a.v1e08b4 == 0xffffu &&
              st045a.v1e08be == -1 && st045a.v1e08c4 == 1u,
          "DM2_19f0_045a seeds the cache on a miss with bit 0x10 set");
    CHECK(dm2_v1_skproject_19f0_045a(
              5u, 6u, &st045a, cycle15_transition_tile_fn, &tdb,
              &q045a) == 5 &&
              q045a.cache_hit,
          "DM2_19f0_045a returns the input x on a cache hit");
    tdb.start_tile = 0x00u;
    st045a.v1d3248 = 4u; /* map word changed -> miss */
    CHECK(dm2_v1_skproject_19f0_045a(
              5u, 6u, &st045a, cycle15_transition_tile_fn, &tdb,
              &q045a) == 0xfffe &&
              !q045a.cache_hit && st045a.v1e08ac == 4u,
          "DM2_19f0_045a seeds 0xfffe when tile bit 0x10 is clear");
    CHECK(dm2_v1_skproject_19f0_045a(
              5u, 6u, NULL, cycle15_transition_tile_fn, &tdb, &q045a) == 0 &&
              q045a.blocked_missing_state,
          "DM2_19f0_045a fails closed without state");
}

/* ---- cycle-16 c_1c9a / c_ai symbol batch fakes ---- */

typedef struct {
    int32_t link;           /* tile record link */
    int32_t next[16];       /* next links by handle & 0xff */
    uint16_t types[16];     /* distinctive types by handle & 0xff */
    uint8_t record[16];
    uint8_t tile;
    int32_t creature;       /* creature handle at probe cell; -1 none */
    int16_t creature_x;
    int16_t creature_y;
    int32_t can_handle;     /* can_handle_fn result */
    int32_t go_there;       /* go_there_fn result */
    int32_t can_handle_in;  /* can_handle_item_in_fn result */
    int16_t add_charge;
    int32_t find_actuator;
    int32_t cmd06bd;
    int16_t hero_at;        /* hero index or -1 */
    uint16_t hero_item0;
    uint16_t hero_item1;
    uint16_t hero_pos;
    int16_t player_at;
    uint16_t timer_dir;
    uint8_t cut_count;
    uint8_t append_count;
    uint8_t cmd2165_count;
    uint16_t cmd2165_mode;
    int16_t cmd2165_arg5;
    int16_t cmd2165_arg6;
    int16_t oversee_seen[5];
    int16_t oversee_result;
} Cycle16Db;

static int32_t cycle16_link_fn(int16_t x, int16_t y, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)x;
    (void)y;
    return db->link;
}

static int32_t cycle16_next_fn(uint16_t handle, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    uint16_t idx = (uint16_t)(handle & 0xffu);
    return (idx < 16u) ? db->next[idx] : -1;
}

static uint16_t cycle16_type_fn(uint16_t handle, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    uint16_t idx = (uint16_t)(handle & 0xffu);
    return (idx < 16u) ? db->types[idx] : 0u;
}

static const uint8_t *cycle16_record_fn(
    uint16_t handle, uint16_t *out_size, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)handle;
    if (out_size) *out_size = (uint16_t)sizeof(db->record);
    return db->record;
}

static uint8_t cycle16_tile_fn(int16_t x, int16_t y, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)x;
    (void)y;
    return db->tile;
}

static int32_t cycle16_creature_at_fn(int16_t x, int16_t y, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    return (x == db->creature_x && y == db->creature_y) ? db->creature : -1;
}

static int32_t cycle16_can_handle_fn(uint16_t item, int16_t handle,
                                     void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)item;
    (void)handle;
    return db->can_handle;
}

static int cycle16_go_there_fn(uint16_t mode, int16_t x, int16_t y,
                               int16_t dir_x, int16_t arg_y,
                               uint16_t direction, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)mode;
    (void)x;
    (void)y;
    (void)dir_x;
    (void)arg_y;
    (void)direction;
    return db->go_there;
}

static int32_t cycle16_can_handle_in_fn(uint16_t item_type,
                                        uint16_t possession, uint16_t slot,
                                        void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)item_type;
    (void)possession;
    (void)slot;
    return db->can_handle_in;
}

static int16_t cycle16_add_charge_fn(uint16_t item, int16_t delta,
                                     void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)item;
    (void)delta;
    return db->add_charge;
}

static void cycle16_cmd2165_fn(uint16_t mode, int16_t x, int16_t y,
                               int16_t tx, int16_t ty, int16_t arg5,
                               int16_t arg6, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)x;
    (void)y;
    (void)tx;
    (void)ty;
    db->cmd2165_count++;
    db->cmd2165_mode = mode;
    db->cmd2165_arg5 = arg5;
    db->cmd2165_arg6 = arg6;
}

static int32_t cycle16_find_actuator_fn(int16_t x, int16_t y, uint8_t cls,
                                        uint8_t type, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)x;
    (void)y;
    (void)cls;
    (void)type;
    return db->find_actuator;
}

static int32_t cycle16_wall_record_fn(int16_t x, int16_t y, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)x;
    (void)y;
    return db->link;
}

static void cycle16_cut_fn(uint16_t record, int16_t x, int16_t y,
                           void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)record;
    (void)x;
    (void)y;
    db->cut_count++;
}

static void cycle16_append_fn(uint16_t record, int16_t x, int16_t y,
                              void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)record;
    (void)x;
    (void)y;
    db->append_count++;
}

static int32_t cycle16_cmd06bd_fn(uint16_t creature, int16_t type,
                                  uint16_t direction, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)creature;
    (void)type;
    (void)direction;
    return db->cmd06bd;
}

static int32_t cycle16_hero_at_fn(int16_t x, int16_t y, uint16_t pos,
                                  void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)x;
    (void)y;
    (void)pos;
    return db->hero_at;
}

static uint16_t cycle16_hero_item_fn(uint16_t hero, uint16_t slot,
                                     void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)hero;
    return (slot == 0u) ? db->hero_item0 : db->hero_item1;
}

static uint16_t cycle16_hero_pos_fn(uint16_t hero, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)hero;
    return db->hero_pos;
}

static int16_t cycle16_player_at_fn(uint16_t pos, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)pos;
    return db->player_at;
}

static uint16_t cycle16_timer_dir_fn(uint16_t timer_index, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    (void)timer_index;
    return db->timer_dir;
}

static void cycle16_oversee_fn(uint16_t record, uint8_t mode,
                               int16_t *state_words, void *user)
{
    Cycle16Db *db = (Cycle16Db *)user;
    int i;
    (void)record;
    (void)mode;
    for (i = 0; i < 5; ++i)
        db->oversee_seen[i] = state_words[i];
    state_words[0] = db->oversee_result;
}

static void cycle16_ctx_fill(DM2_V1_SkprojectXactContext *ctx,
                             Cycle16Db *db,
                             DM2_V1_SkprojectRandomData *randdat)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->user = db;
    ctx->go_there_fn = cycle16_go_there_fn;
    ctx->can_handle_item_in_fn = cycle16_can_handle_in_fn;
    ctx->can_handle_fn = cycle16_can_handle_fn;
    ctx->add_charge_fn = cycle16_add_charge_fn;
    ctx->type_fn = cycle16_type_fn;
    ctx->cmd2165_fn = cycle16_cmd2165_fn;
    ctx->creature_at_fn = cycle16_creature_at_fn;
    ctx->record_fn = cycle16_record_fn;
    ctx->next_fn = cycle16_next_fn;
    ctx->find_actuator_fn = cycle16_find_actuator_fn;
    ctx->wall_record_fn = cycle16_wall_record_fn;
    ctx->cut_record_fn = cycle16_cut_fn;
    ctx->append_record_fn = cycle16_append_fn;
    ctx->cmd06bd_fn = cycle16_cmd06bd_fn;
    ctx->randdat = randdat;
}

static void cycle16_1baad_ctx_fill(DM2_V1_Skproject1baadContext *ctx,
                                   Cycle16Db *db)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->user = db;
    ctx->tile_fn = cycle16_tile_fn;
    ctx->wall_record_fn = cycle16_wall_record_fn;
    ctx->record_fn = cycle16_record_fn;
    ctx->next_fn = cycle16_next_fn;
}

static uint16_t stub_rand16(uint16_t max, void *u)
{
    (void)u;
    return max > 0 ? (uint16_t)(max / 2) : 0;
}

static int32_t cycle16_cls2_stub(uint16_t h, void *u)
{
    (void)h;
    (void)u;
    return -1;
}

static uint16_t cycle16_gdat_stub(uint8_t a, uint8_t b, uint8_t c,
                                  uint8_t d, void *u)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)u;
    return 0u;
}

static void test_skwin_core_symbol_batch_cycle16(void)
{
    DM2_V1_Skproject19f004bfReceipt q04bf;
    DM2_V1_Skproject19f0050fReceipt q050f;
    DM2_V1_Skproject19f00547Receipt q0547;
    DM2_V1_Skproject19f00559Receipt q0559;
    DM2_V1_Skproject1c9a0598Receipt q0598;
    DM2_V1_Skproject19f005e8Receipt q05e8;
    DM2_V1_Skproject19f00891Receipt q0891;
    DM2_V1_Skproject19f00d10Receipt q0d10;
    DM2_V1_Skproject14cd2807Receipt q2807;
    DM2_V1_Skproject14cd2886Receipt q2886;
    DM2_V1_SkprojectXact56Receipt qx56;
    DM2_V1_SkprojectXact57Receipt qx57;
    DM2_V1_SkprojectXact5976Receipt qx5976;
    DM2_V1_SkprojectXact62Receipt qx62;
    DM2_V1_SkprojectXact63Receipt qx63;
    DM2_V1_SkprojectXact64Receipt qx64;
    DM2_V1_Skproject19f004bfState st04bf;
    DM2_V1_Skproject19f00559State st0559;
    DM2_V1_Skproject19f0045aState st045a;
    DM2_V1_Skproject1bc29Cache cache1bc29;
    DM2_V1_Skproject1baadContext ctx1baad;
    DM2_V1_Skproject0891Context ctx0891;
    DM2_V1_Skproject05e8Context ctx05e8;
    DM2_V1_Skproject0d10Context ctx0d10;
    DM2_V1_SkprojectXactContext ctxx;
    DM2_V1_SkprojectCreatureShadow shadow;
    DM2_V1_SkprojectRandomData randdat;
    Cycle16Db db;
    uint16_t v1e08b4;
    uint16_t v1e08b0;
    int16_t v1e056f;
    uint16_t packed;
    int16_t state_words[10];
    uint8_t vis_grid[1024];
    int32_t r32;
    int i;

    dm2_v1_skproject_random_init(&randdat);

    /* DM2_19f0_04bf */
    memset(&db, 0, sizeof(db));
    memset(&st04bf, 0, sizeof(st04bf));
    st04bf.v1e08a8 = 0xffffu;
    st04bf.v1e08aa = 0xffffu;
    st04bf.v1e08b0 = 0xffffu;
    st04bf.v1e08b2 = 0xffffu;
    db.link = 0x0401; /* chain head, record type 1 */
    db.next[1] = 0x1402; /* record type 5 > 3 stops the walk */
    db.next[2] = 0xfffe;
    r32 = dm2_v1_skproject_19f0_04bf(&st04bf, cycle16_link_fn,
                                     cycle16_next_fn, &db, &q04bf);
    CHECK(r32 == 0x1402 && q04bf.valid && st04bf.v1e08b2 == 0x1402u &&
              st04bf.v1e08b0 == 0x0401u && q04bf.records_walked == 1u,
          "DM2_19f0_04bf walks past type<=3 records and caches the result");
    CHECK(dm2_v1_skproject_19f0_04bf(&st04bf, cycle16_link_fn,
                                     cycle16_next_fn, &db, &q04bf) ==
                  0x1402 &&
              q04bf.records_walked == 0u,
          "DM2_19f0_04bf returns the cached v1e08b2 without walking");

    /* DM2_19f0_050f */
    memset(&st04bf, 0, sizeof(st04bf));
    st04bf.v1e08b0 = 0x0401u;
    st04bf.v1e08b2 = 0xffffu;
    v1e08b4 = 0xffffu;
    db.next[1] = 0x1c05; /* type 7 stops 04bf */
    db.next[5] = 0x1006; /* type 4 found by 050f */
    db.next[6] = 0xfffe;
    r32 = dm2_v1_skproject_19f0_050f(&v1e08b4, &st04bf, cycle16_link_fn,
                                     cycle16_next_fn, &db, &q050f);
    CHECK(r32 == 0x1006 && q050f.valid && v1e08b4 == 0x1006u,
          "DM2_19f0_050f finds and caches the first type-4 record");

    /* DM2_19f0_0547 */
    db.can_handle = 1;
    CHECK(dm2_v1_skproject_19f0_0547(0x9004u, 0x42, cycle16_can_handle_fn,
                                     &db, &q0547) == 1 &&
              q0547.valid,
          "DM2_19f0_0547 delegates to CREATURE_CAN_HANDLE_IT");
    db.can_handle = 0;
    CHECK(dm2_v1_skproject_19f0_0547(0x9004u, 0x42, cycle16_can_handle_fn,
                                     &db, &q0547) == 0 &&
              q0547.valid,
          "DM2_19f0_0547 returns 0 when the creature cannot handle it");
    CHECK(dm2_v1_skproject_19f0_0547(0x9004u, 0x42, NULL, &db, &q0547) ==
                  0 &&
              q0547.blocked_missing_callback,
          "DM2_19f0_0547 fails closed without the callback");

    /* DM2_19f0_0559 */
    memset(&st0559, 0, sizeof(st0559));
    CHECK(dm2_v1_skproject_19f0_0559(0, 0x0000u, &randdat, &st0559,
                                     &q0559) == 0 &&
              q0559.already_facing && st0559.b1a == 0u &&
              st0559.v1e056f == -2,
          "DM2_19f0_0559 returns 0 when already facing the direction");
    memset(&st0559, 0, sizeof(st0559));
    CHECK(dm2_v1_skproject_19f0_0559(1, 0x0000u, &randdat, &st0559,
                                     &q0559) == 1 &&
              q0559.turn == 1 && st0559.b1d == 1u && st0559.b1a == 7u &&
              st0559.v1e056f == -4,
          "DM2_19f0_0559 turns right on the shorter arc");
    memset(&st0559, 0, sizeof(st0559));
    CHECK(dm2_v1_skproject_19f0_0559(3, 0x0000u, &randdat, &st0559,
                                     &q0559) == 1 &&
              q0559.turn == -1 && st0559.b1d == 3u && st0559.b1a == 6u,
          "DM2_19f0_0559 turns left on the shorter arc");
    memset(&st0559, 0, sizeof(st0559));
    CHECK(dm2_v1_skproject_19f0_0559(2, 0x0000u, &randdat, &st0559,
                                     &q0559) == 1 &&
              st0559.v1e056f == -4 &&
              (st0559.b1a == 6u || st0559.b1a == 7u),
          "DM2_19f0_0559 picks a random arc on a 180 turn");

    /* DM2_1c9a_0598 */
    CHECK(dm2_v1_skproject_1c9a_0598(0u, &q0598) == 0u &&
              dm2_v1_skproject_1c9a_0598(0xffu, &q0598) == 8u &&
              dm2_v1_skproject_1c9a_0598(0xffffffffu, &q0598) == 32u &&
              dm2_v1_skproject_1c9a_0598(0x80000000u, &q0598) == 1u,
          "DM2_1c9a_0598 counts set bits over the low 32 bits");

    /* shared contexts for 0891/05e8/0d10 */
    cycle16_1baad_ctx_fill(&ctx1baad, &db);
    memset(&cache1bc29, 0, sizeof(cache1bc29));
    memset(&ctx0891, 0, sizeof(ctx0891));
    ctx0891.v1e0578 = 0x0008u;
    ctx0891.sight_range = 10u;
    ctx0891.current_map = 1u;
    ctx0891.transition_map = 1u;
    ctx0891.transition_x = 9u;
    ctx0891.transition_y = 9u;
    ctx0891.map_width = 32;
    ctx0891.map_height = 32;
    ctx0891.creature_x = 2;
    ctx0891.creature_y = 2;
    ctx0891.randdat = &randdat;
    ctx0891.user = &db;
    ctx0891.go_there_fn = cycle16_go_there_fn;
    ctx0891.hero_at_fn = cycle16_hero_at_fn;
    ctx0891.hero_item_fn = cycle16_hero_item_fn;
    ctx0891.hero_pos_fn = cycle16_hero_pos_fn;
    ctx0891.can_handle_fn = cycle16_can_handle_fn;
    ctx0891.player_at_fn = cycle16_player_at_fn;
    ctx0891.ctx1baad = &ctx1baad;
    ctx0891.state045a = &st045a;
    ctx0891.state04bf = &st04bf;
    ctx0891.v1e08b4 = &v1e08b4;
    ctx0891.tile_fn = cycle16_tile_fn;
    ctx0891.tile_link_fn = cycle16_link_fn;
    ctx0891.next_fn = cycle16_next_fn;
    ctx0891.record_fn = cycle16_record_fn;
    ctx0891.state0559 = &st0559;
    ctx0891.creature = &shadow;
    ctx0891.v1e056f = &v1e056f;

    /* DM2_19f0_0891: rejection without movement flags */
    memset(&db, 0, sizeof(db));
    memset(&st045a, 0, sizeof(st045a));
    memset(&st04bf, 0, sizeof(st04bf));
    memset(&st0559, 0, sizeof(st0559));
    memset(&shadow, 0, sizeof(shadow));
    st04bf.v1e08b0 = 0xffffu;
    st04bf.v1e08b2 = 0xffffu;
    v1e08b4 = 0x1004u; /* preset type-4 cache for the vb_18 == 2 path */
    v1e056f = 0;
    db.tile = 0x30u; /* type 1 with bit 0x10: 1BAAD sees through */
    db.link = 0xfffe;
    ctx0891.v1e0578 = 0u;
    CHECK(dm2_v1_skproject_19f0_0891(0x82u, 5, 5, 7, 5, 1, &ctx0891,
                                     &q0891) == 0 &&
              q0891.rejected,
          "DM2_19f0_0891 rejects when v1e0578 is zero");
    /* vb_18 <= 1 requires the transition cache triple */
    ctx0891.v1e0578 = 0x0008u;
    CHECK(dm2_v1_skproject_19f0_0891(0x81u, 5, 5, 7, 5, 1, &ctx0891,
                                     &q0891) == 0 &&
              q0891.rejected,
          "DM2_19f0_0891 rejects vb_18<=1 away from the transition");
    /* full commit on the vb_18 == 2 path */
    ctx0891.creature_word_e = 0x0100u; /* facing 1 == direction */
    CHECK(dm2_v1_skproject_19f0_0891(0x82u, 5, 5, 7, 5, 1, &ctx0891,
                                     &q0891) == 1 &&
              q0891.committed && q0891.result_word == -4 &&
              shadow.w18 == (uint16_t)(7u | (5u << 5) | (1u << 10)) &&
              shadow.b1b == 1u && shadow.b20 == 2u &&
              (shadow.b1a == 0x0eu || shadow.b1a == 0x0fu) &&
              v1e056f == -4,
          "DM2_19f0_0891 commits the move into the creature shadow");
    /* turn-toward early return through DM2_19f0_0559 */
    memset(&st0559, 0, sizeof(st0559));
    memset(&shadow, 0, sizeof(shadow));
    ctx0891.creature_word_e = 0x0000u; /* facing 0, direction 1 -> turn */
    CHECK(dm2_v1_skproject_19f0_0891(0x82u, 5, 5, 7, 5, 1, &ctx0891,
                                     &q0891) == 1 &&
              !q0891.committed && q0891.result_word == -4 &&
              st0559.b1d == 1u,
          "DM2_19f0_0891 returns through the 0559 turn path");
    /* probe-only mode returns 1 without committing */
    CHECK(dm2_v1_skproject_19f0_0891(0x02u, 5, 5, 7, 5, 1, &ctx0891,
                                     &q0891) == 1 &&
              !q0891.committed,
          "DM2_19f0_0891 probe mode returns 1 without committing");

    /* DM2_19f0_05e8 */
    memset(&ctx05e8, 0, sizeof(ctx05e8));
    memset(vis_grid, 0, sizeof(vis_grid));
    memset(&db, 0, sizeof(db));
    ctx05e8.creature_x = 2;
    ctx05e8.creature_y = 2;
    ctx05e8.v1e0578 = 0x0008u;
    ctx05e8.sight_range = 4u;
    ctx05e8.current_map = 1u;
    ctx05e8.map_width = 8;
    ctx05e8.map_height = 8;
    ctx05e8.vis_grid = vis_grid;
    ctx05e8.user = &db;
    ctx05e8.tile_fn = cycle16_tile_fn;
    ctx05e8.creature_at_fn = cycle16_creature_at_fn;
    ctx05e8.can_handle_fn = cycle16_can_handle_fn;
    ctx05e8.tile_link_fn = cycle16_link_fn;
    ctx05e8.cls2_fn = NULL;
    ctx05e8.gdat_fn = NULL;
    ctx05e8.next_fn = cycle16_next_fn;
    ctx05e8.cache1bc29 = &cache1bc29;
    ctx05e8.ctx1baad = &ctx1baad;
    ctx05e8.ctx0891 = &ctx0891;
    ctx05e8.state045a = &st045a;
    db.tile = 0x30u; /* type 1 with bit 0x10 */
    db.creature_x = 3;
    db.creature_y = 2;
    db.creature = 0x9004;
    db.can_handle = 1;
    db.link = 0xfffe;
    /* cls2/gdat are required by the context check */
    ctx05e8.cls2_fn = cycle16_cls2_stub;
    ctx05e8.gdat_fn = cycle16_gdat_stub;
    memset(&st0559, 0, sizeof(st0559));
    memset(&shadow, 0, sizeof(shadow));
    packed = 0u;
    CHECK(dm2_v1_skproject_19f0_05e8(0x42u, &packed, 2, 2, 1, 0, &ctx05e8,
                                     &q05e8) == 1 &&
              q05e8.found && q05e8.found_via == 1u &&
              q05e8.found_x == 3 && q05e8.found_y == 2 &&
              packed == (uint16_t)(3u | (2u << 5) | (1u << 10)) &&
              q05e8.result == 1,
          "DM2_19f0_05e8 finds a handleable creature and delegates the move");
    db.can_handle = 0;
    db.creature = -1;
    CHECK(dm2_v1_skproject_19f0_05e8(0x42u, &packed, 2, 2, 1, 0, &ctx05e8,
                                     &q05e8) == 0 &&
              q05e8.rejected,
          "DM2_19f0_05e8 returns 0 when nothing handleable is found");

    /* DM2_19f0_0d10 */
    memset(&ctx0d10, 0, sizeof(ctx0d10));
    memset(&db, 0, sizeof(db));
    memset(&st045a, 0, sizeof(st045a));
    memset(&st0559, 0, sizeof(st0559));
    memset(&shadow, 0, sizeof(shadow));
    v1e08b0 = 0xffffu;
    ctx0d10.v1e057a = 0x0073u;
    ctx0d10.v1e0578 = 1u;
    ctx0d10.sight_range = 10u;
    ctx0d10.current_map = 1u;
    ctx0d10.v1e08b0 = &v1e08b0;
    ctx0d10.map_width = 8;
    ctx0d10.map_height = 8;
    ctx0d10.randdat = &randdat;
    ctx0d10.user = &db;
    ctx0d10.tile_link_fn = cycle16_link_fn;
    ctx0d10.record_fn = cycle16_record_fn;
    ctx0d10.next_fn = cycle16_next_fn;
    ctx0d10.wall_record_fn = cycle16_wall_record_fn;
    ctx0d10.timer_dir_fn = cycle16_timer_dir_fn;
    ctx0d10.ctx1baad = &ctx1baad;
    ctx0d10.state045a = &st045a;
    ctx0d10.ctx0891 = &ctx0891;
    ctx0d10.state0559 = &st0559;
    ctx0d10.creature = &shadow;
    ctx0d10.v1e056f = &v1e056f;
    db.link = 0x2401; /* door record */
    db.record[2] = 0x00u;
    db.record[3] = 0x00u;
    /* capability zero rejects */
    ctx0d10.v1e057a = 0u;
    CHECK(dm2_v1_skproject_19f0_0d10(0x81u, 4, 4, 5, 4, 1, &ctx0d10,
                                     &q0d10) == 0 &&
              q0d10.rejected,
          "DM2_19f0_0d10 rejects without the capability mask");
    ctx0d10.v1e057a = 0x0073u;
    /* adjacent door, variant 1, vb_1c 1: turn path through 0559 */
    ctx0d10.v1e08ae = (uint16_t)((4u << 5) | 1u);
    ctx0d10.creature_word_e = 0x0000u; /* facing 0, direction 1 -> turn */
    CHECK(dm2_v1_skproject_19f0_0d10(0x81u, 4, 4, 5, 4, 1, &ctx0d10,
                                     &q0d10) == 1 &&
              q0d10.outcome == 4u && q0d10.result_word == -4 &&
              st0559.b1d == 1u,
          "DM2_19f0_0d10 returns through the 0559 turn path");
    /* door variant 5 rejects for vb_1c != 0 */
    ctx0d10.v1e08ae = (uint16_t)((4u << 5) | 5u);
    CHECK(dm2_v1_skproject_19f0_0d10(0x81u, 4, 4, 5, 4, 1, &ctx0d10,
                                     &q0d10) == 0 &&
              q0d10.rejected,
          "DM2_19f0_0d10 rejects door variant 5 for vb_1c != 0");
    /* vb_1c 0 with variant 0: vw_04 1 path */
    ctx0d10.v1e08ae = (uint16_t)((4u << 5) | 0u);
    v1e056f = 0;
    CHECK(dm2_v1_skproject_19f0_0d10(0x80u, 4, 4, 5, 4, 1, &ctx0d10,
                                     &q0d10) == 1 &&
              q0d10.outcome == 1u && q0d10.result_word == -2 &&
              v1e056f == -2,
          "DM2_19f0_0d10 reports the vw_04 1 open-door path");
    /* probe mode returns 1 without the commit gate */
    ctx0d10.v1e08ae = (uint16_t)((4u << 5) | 1u);
    CHECK(dm2_v1_skproject_19f0_0d10(0x01u, 4, 4, 5, 4, 1, &ctx0d10,
                                     &q0d10) == 1 &&
              q0d10.outcome == 0u && q0d10.result_word == 0,
          "DM2_19f0_0d10 probe mode returns 1 without committing");

    /* DM2_14cd_2807 / DM2_14cd_2886 */
    memset(&db, 0, sizeof(db));
    cycle16_ctx_fill(&ctxx, &db, &randdat);
    memset(state_words, 0, sizeof(state_words));
    state_words[0] = -1;
    state_words[2] = 0x42;
    state_words[4] = 3;
    state_words[6] = 4;
    state_words[8] = 1;
    db.can_handle = 0;
    CHECK(dm2_v1_skproject_14cd_2807(0x1001u, state_words, &ctxx,
                                     &q2807) == 0 &&
              q2807.valid && !q2807.admitted,
          "DM2_14cd_2807 skips items the creature cannot handle");
    db.can_handle = 1;
    db.add_charge = 3;
    db.types[1] = 7u;
    CHECK(dm2_v1_skproject_14cd_2807(0x1001u, state_words, &ctxx,
                                     &q2807) == 0 &&
              q2807.admitted && q2807.charge == 3 &&
              q2807.distinctive_type == 7u &&
              q2807.blocked_gdat_path && state_words[0] == 0,
          "DM2_14cd_2807 accumulates the fail-closed GDAT damage value");
    db.oversee_result = 42;
    CHECK(dm2_v1_skproject_14cd_2886(0x1001u, 1u, 2u, 3u, 4u,
                                     cycle16_oversee_fn, &db, &q2886) ==
                  42 &&
              q2886.valid && db.oversee_seen[0] == -1 &&
              db.oversee_seen[1] == 1 && db.oversee_seen[4] == 4,
          "DM2_14cd_2886 builds the state array and returns word 0");

    /* DM2_PROCEED_XACT_56 */
    db.go_there = 1;
    ctxx.creature_word_e = 0x0100u;
    CHECK(dm2_v1_skproject_proceed_xact_56(&ctxx, &qx56) == -4 &&
              qx56.go_there_ok,
          "DM2_PROCEED_XACT_56 returns -4 when the move is accepted");
    db.go_there = 0;
    CHECK(dm2_v1_skproject_proceed_xact_56(&ctxx, &qx56) == -2,
          "DM2_PROCEED_XACT_56 returns -2 when the move fails");

    /* DM2_PROCEED_XACT_57 */
    memset(&st0559, 0, sizeof(st0559));
    ctxx.state0559 = &st0559;
    ctxx.creature_x = 2;
    ctxx.creature_y = 2;
    db.go_there = 1;
    CHECK(dm2_v1_skproject_proceed_xact_57(&ctxx, &qx57) == 1 &&
              qx57.first_ok && !qx57.turned,
          "DM2_PROCEED_XACT_57 takes the first side step");
    db.go_there = 0;
    CHECK(dm2_v1_skproject_proceed_xact_57(&ctxx, &qx57) == 1 &&
              !qx57.first_ok && !qx57.second_ok && qx57.turned,
          "DM2_PROCEED_XACT_57 falls back to the 0559 turn");

    /* DM2_PROCEED_XACT_59_76 */
    memset(&db, 0, sizeof(db));
    cycle16_ctx_fill(&ctxx, &db, &randdat);
    ctxx.v1e0572 = 5;
    ctxx.v1e0574 = 1u;
    ctxx.possession = 0x1001u;
    ctxx.v1e056f = -7;
    db.can_handle_in = -2;
    CHECK(dm2_v1_skproject_proceed_xact_59_76(&ctxx, &qx5976) == -7 &&
              qx5976.command_issued && db.cmd2165_mode == 0x80u &&
              db.cmd2165_arg6 == 5,
          "DM2_PROCEED_XACT_59_76 issues the 2165 command and returns v1e056f");
    db.can_handle_in = 0;
    CHECK(dm2_v1_skproject_proceed_xact_59_76(&ctxx, &qx5976) == -2 &&
              qx5976.rejected,
          "DM2_PROCEED_XACT_59_76 returns -2 when the item is not handled");

    /* DM2_PROCEED_XACT_62 */
    memset(&db, 0, sizeof(db));
    cycle16_ctx_fill(&ctxx, &db, &randdat);
    ctxx.v1e057c = 0u;
    CHECK(dm2_v1_skproject_proceed_xact_62(&ctxx, &qx62) == -3,
          "DM2_PROCEED_XACT_62 returns -3 when disabled");
    ctxx.v1e057c = 0x0077u;
    ctxx.v1e0574 = 1u;
    db.can_handle_in = 0;
    CHECK(dm2_v1_skproject_proceed_xact_62(&ctxx, &qx62) == -2,
          "DM2_PROCEED_XACT_62 returns -2 when the item is handled");
    /* sorting path */
    ctxx.v1e0574 = 0u;
    ctxx.v1e0572 = 1;
    ctxx.creature_x = 3;
    ctxx.creature_y = 3;
    ctxx.v1e056f = -5;
    db.find_actuator = 0x2401;
    db.record[2] = 0x80u; /* word@2 = 0x80 -> wanted type 1 */
    db.record[3] = 0x00u;
    db.link = 0x1401; /* type 5 stops the first walk */
    db.types[1] = 2u;
    db.next[1] = 0x1402;
    db.types[2] = 1u;
    db.next[2] = 0xfffe;
    CHECK(dm2_v1_skproject_proceed_xact_62(&ctxx, &qx62) == -5 &&
              qx62.command_issued && qx62.records_moved == 1 &&
              db.cut_count == 1u && db.append_count == 1u &&
              db.cmd2165_arg6 == 16,
          "DM2_PROCEED_XACT_62 sorts matching records and issues the command");

    /* DM2_PROCEED_XACT_63 */
    memset(&db, 0, sizeof(db));
    cycle16_ctx_fill(&ctxx, &db, &randdat);
    ctxx.v1e0572 = 5;
    ctxx.v1e0574 = 0xffu;
    ctxx.creature_x = 3;
    ctxx.creature_y = 3;
    ctxx.creature_word_e = 0x0000u; /* facing 0: probe (3, 2) */
    db.creature_x = 3;
    db.creature_y = 2;
    db.creature = 0x9004;
    db.record[2] = 0x34u;
    db.record[3] = 0x12u;
    db.can_handle_in = 0;
    CHECK(dm2_v1_skproject_proceed_xact_63(&ctxx, &qx63) == -2 &&
              qx63.creature_handle == 0x9004u,
          "DM2_PROCEED_XACT_63 returns -2 when the creature can handle it");
    db.can_handle_in = 0xfffe;
    CHECK(dm2_v1_skproject_proceed_xact_63(&ctxx, &qx63) == -3,
          "DM2_PROCEED_XACT_63 returns -3 otherwise");

    /* DM2_PROCEED_XACT_64 */
    memset(&db, 0, sizeof(db));
    cycle16_ctx_fill(&ctxx, &db, &randdat);
    ctxx.possession = 0x1001u;
    ctxx.v1e057c = 0x0008u;
    ctxx.v1e0572 = -1;
    ctxx.creature_word_e = 0x0100u;
    ctxx.v1e056f = -6;
    db.can_handle_in = 0;
    CHECK(dm2_v1_skproject_proceed_xact_64(&ctxx, &qx64) == -6 &&
              qx64.command_issued && qx64.item_type == 63u &&
              db.cmd2165_mode == 0x81u && db.cmd2165_arg6 == 63,
          "DM2_PROCEED_XACT_64 throws the possessed item forward");
    db.can_handle_in = -2;
    CHECK(dm2_v1_skproject_proceed_xact_64(&ctxx, &qx64) == -3,
          "DM2_PROCEED_XACT_64 returns -3 when the item cannot be thrown");
    (void)i;
}

/* ---- cycle-16 batch-17 (c_0aaf + c_1c9a) fakes ---- */

typedef struct {
    uint8_t record_a[16];
    uint8_t record_b[16];
    uint16_t record_b_handle;
    int32_t link;
    int32_t next[16];
    uint16_t types[16];
    uint8_t tile;
    int32_t tile_record;
    int32_t rebirth;
    int32_t door_gfx;
    int32_t can_handle;
    int32_t move075f;
    uint16_t timer_dir;
    int32_t gdat_index;
    uint8_t gdat_text_ok;
    uint8_t gdat_image[8];
    uint8_t gdat_data[64];
    int16_t alloc_ret[2];
    uint16_t alloc_seen[2];
    uint16_t freed[2];
    int16_t change_map_seen;
    int chest;
    uint16_t ai_flags;
    DM2_V1_SkprojectCreatureAISpec spec;
    uint8_t detail_ok[4];
    DM2_V1_SkprojectTeleporterDetail detail;
} Cycle16bDb;

static const uint8_t *cycle16b_record_fn(
    uint16_t handle, uint16_t *out_size, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    if (out_size) *out_size = 16u;
    return (handle == db->record_b_handle) ? db->record_b : db->record_a;
}

static int32_t cycle16b_link_fn(int16_t x, int16_t y, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)x;
    (void)y;
    return db->link;
}

static int32_t cycle16b_next_fn(uint16_t handle, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    uint16_t idx = (uint16_t)(handle & 0xffu);
    return (idx < 16u) ? db->next[idx] : -1;
}

static uint8_t cycle16b_tile_fn(int16_t x, int16_t y, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)x;
    (void)y;
    return db->tile;
}

static int32_t cycle16b_tile_record_fn(int16_t x, int16_t y, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)x;
    (void)y;
    return db->tile_record;
}

static int32_t cycle16b_rebirth_fn(int32_t record, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)record;
    return db->rebirth;
}

static int32_t cycle16b_door_gfx_fn(uint8_t value, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)value;
    return db->door_gfx;
}

static int32_t cycle16b_can_handle_fn(uint16_t item, int16_t handle,
                                      void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)item;
    (void)handle;
    return db->can_handle;
}

static int32_t cycle16b_wall_record_fn(int16_t x, int16_t y, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)x;
    (void)y;
    return db->link;
}

static uint16_t cycle16b_timer_dir_fn(uint16_t timer_index, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)timer_index;
    return db->timer_dir;
}

static int32_t cycle16b_move075f_fn(const uint8_t *record, uint16_t word2,
                                    void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)record;
    (void)word2;
    return db->move075f;
}

static int cycle16b_gdat_text_fn(uint8_t cls1, uint8_t cls2, uint8_t index,
                                 char *out_text, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)cls1;
    (void)cls2;
    if (index >= db->gdat_text_ok) {
        out_text[0] = '\0';
        return 0;
    }
    out_text[0] = 'X';
    out_text[1] = '\0';
    return 1;
}

static uint16_t cycle16b_gdat_data_fn(uint8_t cls1, uint8_t cls2,
                                      uint8_t entry_index,
                                      uint8_t data_index, void *user)
{
    static const uint16_t data_words[3] = { 0x0005u, 0x0000u, 0x0307u };
    (void)cls1;
    (void)cls2;
    (void)entry_index;
    (void)data_index;
    (void)user;
    return data_words[(data_index < 3u) ? data_index : 0u];
}

static uint16_t cycle16b_gdat_index_fn(uint8_t cls1, uint8_t cls2,
                                       uint8_t entry_index,
                                       uint8_t data_index, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)cls1;
    (void)cls2;
    (void)entry_index;
    (void)data_index;
    return (uint16_t)db->gdat_index;
}

static const uint8_t *cycle16b_gdat_ptr_fn(uint8_t cls1, uint8_t cls2,
                                           uint8_t entry, uint8_t data,
                                           void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)cls1;
    (void)cls2;
    (void)entry;
    (void)data;
    return db->gdat_data;
}

static int cycle16b_detail_fn(DM2_V1_SkprojectTeleporterDetail *out_detail,
                              int16_t x, int16_t y, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    int side = (x == 6) ? 0 : (x == 4) ? 1 : (y == 6) ? 2 : 3;
    if (!db->detail_ok[side])
        return 0;
    *out_detail = db->detail;
    return 1;
}

static int cycle16b_alloc_fn(uint32_t key, uint16_t *out_index, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    int slot = ((key & 0x30000000u) == 0x30000000u) ? 1 : 0;
    db->alloc_seen[slot] = (uint16_t)(key & 0xffffu);
    *out_index = 7u;
    return db->alloc_ret[slot];
}

static void cycle16b_dballoc_free_fn(uint16_t index, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    if (db->freed[0] == 0u)
        db->freed[0] = index;
    else
        db->freed[1] = index;
}

static int cycle16b_is_chest_fn(uint16_t handle, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)handle;
    return db->chest;
}

static const DM2_V1_SkprojectCreatureAISpec *cycle16b_ai_spec_fn(
    uint8_t creature_type, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)creature_type;
    return &db->spec;
}

static uint16_t cycle16b_ai_flags_fn(uint16_t handle, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    (void)handle;
    return db->ai_flags;
}

static int32_t cycle16b_change_map_fn(int16_t map, void *user)
{
    Cycle16bDb *db = (Cycle16bDb *)user;
    db->change_map_seen = map;
    return 0;
}

static void test_skwin_core_symbol_batch_cycle16b(void)
{
    DM2_V1_Skproject0aaf0067Receipt q0067;
    DM2_V1_Skproject0aaf01dbReceipt q01db;
    DM2_V1_Skproject0aaf02f8Receipt q02f8;
    DM2_V1_Skproject19f013aaReceipt q13aa;
    DM2_V1_Skproject19f01511Receipt q1511;
    DM2_V1_SkprojectD283Receipt qd283;
    DM2_V1_SkprojectCreatureGoThereReceipt qgo;
    DM2_V1_Skproject19f02024Receipt q2024;
    DM2_V1_Skproject19f02165Receipt q2165;
    DM2_V1_Skproject19f0266cReceipt q266c;
    DM2_V1_Skproject19f02723Receipt q2723;
    DM2_V1_Skproject19f02813Receipt q2813;
    DM2_V1_Skproject4deaReceipt q4dea;
    DM2_V1_Skproject1ba1bReceipt q1ba1b;
    DM2_V1_Skproject1c9a0247Receipt q0247;
    DM2_V1_Skproject1c9a0648Receipt q0648;
    DM2_V1_Skproject0aaf0067List list0067;
    DM2_V1_Skproject19f013aaContext ctx13aa;
    DM2_V1_Skproject19f02024Context ctx2024;
    DM2_V1_Skproject19f02165Context ctx2165;
    DM2_V1_Skproject19f02165State st2165;
    DM2_V1_Skproject19f02813Context ctx2813;
    DM2_V1_Skproject1c9a0648State st0648;
    DM2_V1_Skproject19f004bfState st04bf;
    DM2_V1_Skproject19f0045aState st045a;
    DM2_V1_Skproject19f00559State st0559;
    DM2_V1_SkprojectCreatureShadow shadow;
    DM2_V1_Skproject1baadContext ctx1baad;
    DM2_V1_SkprojectRandomData randdat;
    Cycle16bDb db;
    uint16_t v1e08b4;
    uint16_t v1e08b0;
    int16_t v1e056f;
    uint32_t v32;
    uint16_t w16;
    int32_t r32;

    const int8_t table6290[8] = { 1, 1, 1, 1, 0, 0, 0, 0 };
    const int8_t table6299[2] = { 0x2b, 0x2c };
    const int8_t table2660[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };

    dm2_v1_skproject_random_init(&randdat);
    memset(&db, 0, sizeof(db));
    memset(&ctx1baad, 0, sizeof(ctx1baad));
    ctx1baad.user = &db;
    ctx1baad.tile_fn = cycle16b_tile_fn;
    ctx1baad.wall_record_fn = cycle16b_wall_record_fn;
    ctx1baad.record_fn = cycle16b_record_fn;
    ctx1baad.next_fn = cycle16b_next_fn;

    /* DM2_0aaf_0067 */
    db.gdat_text_ok = 3u;
    CHECK(dm2_v1_skproject_0aaf_0067(0x10u, cycle16b_gdat_text_fn,
                                     cycle16b_gdat_data_fn, &db, &list0067,
                                     &q0067) == 0 &&
              q0067.valid && q0067.blocked_ui_loop &&
              list0067.count == 3u && list0067.v1e0204 == 3u &&
              list0067.low[0] == 5u && list0067.low[1] == 1u,
          "DM2_0aaf_0067 builds the GDAT text list with index substitution");
    CHECK(dm2_v1_skproject_0aaf_0067(0x87u, cycle16b_gdat_text_fn,
                                     cycle16b_gdat_data_fn, &db,
                                     &list0067, &q0067) == 0 &&
              list0067.last_index == 3 && list0067.value == 7u &&
              list0067.v1e0204 == 7u,
          "DM2_0aaf_0067 records the terminator entry and 0x87 count");
    CHECK(dm2_v1_skproject_0aaf_0067(0x10u, NULL, cycle16b_gdat_index_fn,
                                     &db, &list0067, &q0067) == 0 &&
              q0067.blocked_missing_callback,
          "DM2_0aaf_0067 fails closed without GDAT text access");

    /* DM2_0aaf_01db */
    memset(&db, 0, sizeof(db));
    db.gdat_data[2] = 10u;
    db.gdat_data[4] = 6u;
    CHECK(dm2_v1_skproject_0aaf_01db(0u, 1, 0u, 0u, 0u, 0u, 0, 0, 0, 0, 0, 0,
                                     cycle16b_gdat_ptr_fn, &db, &q01db) ==
                  0 &&
              q01db.route_fill && q01db.blocked_draw_path,
          "DM2_0aaf_01db records the palette fill route");
    CHECK(dm2_v1_skproject_0aaf_01db(0u, 1, 1u, 0u, 0u, 0u, 2, 1, 0, 0, 20,
                                     10, cycle16b_gdat_ptr_fn, &db,
                                     &q01db) == 0 &&
              q01db.route_draw && q01db.image_width == 10u &&
              q01db.image_height == 6u && q01db.rect_x == 3 &&
              q01db.rect_y == 1,
          "DM2_0aaf_01db centers the GDAT image with event offsets");
    CHECK(dm2_v1_skproject_0aaf_01db(0u, 0, 1u, 0u, 0u, 0u, 0, 0, 0, 0, 0, 0,
                                     cycle16b_gdat_ptr_fn, &db, &q01db) ==
                  0 &&
              q01db.skipped,
          "DM2_0aaf_01db skips the image path when the flag is zero");

    /* DM2_0aaf_02f8 */
    memset(&db, 0, sizeof(db));
    CHECK(dm2_v1_skproject_0aaf_02f8(0x0eu, 0u, 0u, cycle16b_gdat_index_fn,
                                     &db, &q02f8) == 0 &&
              q02f8.skip_fade && q02f8.blocked_dialog_path,
          "DM2_0aaf_02f8 suppresses the fade for mode 0x0e with zero flag");
    db.gdat_index = 1u;
    CHECK(dm2_v1_skproject_0aaf_02f8(0x07u, 1u, 0u, cycle16b_gdat_index_fn,
                                     &db, &q02f8) == 0 &&
              q02f8.remap_59 && q02f8.mode == 0x59u,
          "DM2_0aaf_02f8 remaps mode 7 to the 0x59 text when loadable");
    CHECK(dm2_v1_skproject_0aaf_02f8(0x05u, 1u, 0u, cycle16b_gdat_index_fn,
                                     &db, &q02f8) == 0 &&
              q02f8.recursion_requested && q02f8.recursion_mode == 0x05u,
          "DM2_0aaf_02f8 records the mode-0x0e recursion request");

    /* DM2_19f0_13aa */
    memset(&db, 0, sizeof(db));
    memset(&ctx13aa, 0, sizeof(ctx13aa));
    ctx13aa.v1e0584_flags = 0x04u;
    ctx13aa.creature_x = 9;
    ctx13aa.creature_y = 9;
    ctx13aa.map_width = 8;
    ctx13aa.map_height = 8;
    ctx13aa.randdat = &randdat;
    ctx13aa.user = &db;
    ctx13aa.wall_record_fn = cycle16b_wall_record_fn;
    ctx13aa.timer_dir_fn = cycle16b_timer_dir_fn;
    ctx13aa.record_fn = cycle16b_record_fn;
    ctx13aa.next_fn = cycle16b_next_fn;
    ctx13aa.move075f_fn = cycle16b_move075f_fn;
    ctx13aa.ctx1baad = &ctx1baad;
    db.link = 0x3801; /* type 0xe record */
    db.record_a[6] = 0x00u;
    db.record_a[7] = 0x00u;
    db.record_a[2] = 0x42u;
    db.record_a[3] = 0x00u;
    db.timer_dir = 3u; /* opposes direction 1 */
    db.move075f = 1;
    db.tile = 0x30u; /* 1BAAD sees through */
    CHECK(dm2_v1_skproject_19f0_13aa(2, 2, &ctx13aa, &q13aa) == 1 &&
              q13aa.found && q13aa.direction == 1u &&
              q13aa.found_step == 1u && q13aa.found_word2 == 0x42u,
          "DM2_19f0_13aa finds the opposing teleporter actuator");
    db.move075f = 0;
    CHECK(dm2_v1_skproject_19f0_13aa(2, 2, &ctx13aa, &q13aa) == 0 &&
              !q13aa.found && q13aa.valid,
          "DM2_19f0_13aa returns 0 when move075f rejects the record");

    /* DM2_19f0_1511 */
    db.can_handle = 1;
    CHECK(dm2_v1_skproject_19f0_1511(0x9004u, cycle16b_can_handle_fn, &db,
                                     &q1511) == 1 &&
              q1511.valid,
          "DM2_19f0_1511 delegates to CREATURE_CAN_HANDLE_IT(item, 9)");

    /* DM2_D283 */
    memset(&db, 0, sizeof(db));
    db.tile = (uint8_t)((5u << 5) | 0x8u);
    db.detail_ok[0] = 1u;
    db.detail.b_02 = 3u;
    db.detail.b_03 = 5u;
    db.detail.b_04 = 2u;
    db.tile_record = 0x2401;
    db.record_a[4] = 0x00u;
    db.record_a[5] = 0x02u; /* word@4 high byte = map 2 */
    db.record_a[2] = (uint8_t)(3u | (4u << 5));
    db.record_a[3] = 0x00u;
    r32 = dm2_v1_skproject_d283(5, 5, cycle16b_tile_fn, cycle16b_detail_fn,
                                cycle16b_tile_record_fn, cycle16b_record_fn,
                                &db, &qd283);
    CHECK(r32 == 0x2401 && qd283.found && qd283.distance == 1 &&
              qd283.probe_side == 0u,
          "DM2_D283 returns the tile record for a matching teleporter");
    db.tile = 0x00u;
    CHECK(dm2_v1_skproject_d283(5, 5, cycle16b_tile_fn, cycle16b_detail_fn,
                                cycle16b_tile_record_fn, cycle16b_record_fn,
                                &db, &qd283) == -1 &&
              !qd283.found,
          "DM2_D283 rejects non-teleporter tiles");

    /* DM2_CREATURE_GO_THERE (narrow receipt) */
    CHECK(dm2_v1_skproject_creature_go_there(2u, 3, 4, -1, -1, 1,
                                             table6290, 8u, 1u, &qgo) == 0 &&
              qgo.valid && !qgo.gate_open,
          "DM2_CREATURE_GO_THERE rejects mode 2 in the preamble");
    CHECK(dm2_v1_skproject_creature_go_there(1u, 3, 4, -1, -1, 1,
                                             table6290, 8u, 1u, &qgo) == 0 &&
              qgo.gate_open && qgo.cell_x == 4 && qgo.cell_y == 4 &&
              qgo.blocked_runtime_dispatch,
          "DM2_CREATURE_GO_THERE resolves the step cell and fails closed");
    CHECK(dm2_v1_skproject_creature_go_there(1u, 3, 4, -1, -1, 1,
                                             table6290, 8u, 0u, &qgo) == 0 &&
              qgo.gate_open && qgo.table_entry == 1u,
          "DM2_CREATURE_GO_THERE opens the gate on the v1e0576 flag");

    /* DM2_19f0_2024 */
    memset(&db, 0, sizeof(db));
    memset(&ctx2024, 0, sizeof(ctx2024));
    ctx2024.v1e057c = 0x10u;
    ctx2024.table1d2660 = table2660;
    ctx2024.table1d2660_size = 16u;
    ctx2024.user = &db;
    ctx2024.is_chest_fn = cycle16b_is_chest_fn;
    ctx2024.record_fn = cycle16b_record_fn;
    ctx2024.next_fn = cycle16b_next_fn;
    ctx2024.can_handle_fn = cycle16b_can_handle_fn;
    ctx2024.ai_spec_fn = cycle16b_ai_spec_fn;
    ctx2024.ai_flags_fn = cycle16b_ai_flags_fn;
    db.ai_flags = 0u;
    db.chest = 1;
    db.record_a[2] = 0x02u; /* child handle 0x0002, side 0 */
    db.record_a[3] = 0x00u;
    db.next[2] = 0xfffe;
    db.can_handle = 1;
    r32 = dm2_v1_skproject_19f0_2024(0x4001u, 9, 1, &ctx2024, &q2024);
    CHECK(r32 == 1 && q2024.is_chest_scan && q2024.side_mask == 0x0fu,
          "DM2_19f0_2024 scans the chest contents");
    /* creature possession path */
    ctx2024.v1e057c = 0x28u;
    db.chest = 0;
    db.ai_flags = 1u; /* AI spec word@0 bit set -> GDAT mask */
    db.spec.word30 = 0x0402u; /* cls2 2 -> table2660[9] at offset 5 */
    db.record_a[2] = 0x03u; /* child handle 0xc003, side 3 */
    db.record_a[3] = 0xc0u;
    db.next[3] = 0xfffe;
    r32 = dm2_v1_skproject_19f0_2024(0x1001u, 9, 5, &ctx2024, &q2024);
    CHECK(r32 == 11 && q2024.side_mask == 9u,
          "DM2_19f0_2024 applies the 48ae_01af side mask");
    CHECK(dm2_v1_skproject_19f0_2024(0x0401u, 9, 1, &ctx2024, &q2024) ==
                  -1 &&
              q2024.valid,
          "DM2_19f0_2024 rejects non-creature records");

    /* DM2_19f0_2165 */
    memset(&db, 0, sizeof(db));
    memset(&st2165, 0, sizeof(st2165));
    memset(&st04bf, 0, sizeof(st04bf));
    memset(&st045a, 0, sizeof(st045a));
    memset(&st0559, 0, sizeof(st0559));
    memset(&shadow, 0, sizeof(shadow));
    memset(&ctx2165, 0, sizeof(ctx2165));
    st2165.v1e057c = 0x2au;
    st2165.v1e08ae = 0x30u; /* type 1 with bit 0x10 */
    st2165.v1e08be = 0;
    st2165.v1e08a8 = 5u;
    st2165.v1e08aa = 6u;
    st2165.v1e08ac = 2u;
    st2165.creature_word_e = 0x0300u; /* facing 3 */
    st04bf.v1e08b2 = 0x1003u; /* cached type-4 record */
    v1e08b4 = 0x1004u;
    db.spec.word30 = 0x0000u;
    db.record_a[2] = 0x01u; /* child side 0 */
    db.record_a[3] = 0x00u;
    db.next[1] = 0xfffe;
    db.can_handle = 1;
    db.tile = 0x30u;
    ctx2165.state = &st2165;
    ctx2165.state045a = &st045a;
    ctx2165.state04bf = &st04bf;
    ctx2165.v1e08b4 = &v1e08b4;
    ctx2165.table1d2660 = table2660;
    ctx2165.table1d2660_size = 16u;
    ctx2165.table1d6299 = table6299;
    ctx2165.table1d6299_size = 2u;
    ctx2165.randdat = &randdat;
    ctx2165.user = &db;
    ctx2165.tile_fn = cycle16b_tile_fn;
    ctx2165.tile_link_fn = cycle16b_link_fn;
    ctx2165.next_fn = cycle16b_next_fn;
    ctx2165.record_fn = cycle16b_record_fn;
    ctx2165.can_handle_fn = cycle16b_can_handle_fn;
    ctx2165.is_chest_fn = cycle16b_is_chest_fn;
    ctx2165.ai_spec_fn = cycle16b_ai_spec_fn;
    ctx2165.ai_flags_fn = cycle16b_ai_flags_fn;
    db.ai_flags = 0u;
    ctx2165.state0559 = &st0559;
    ctx2165.creature = &shadow;
    ctx2165.v1e056f = &v1e056f;
    v1e056f = 0;
    CHECK(dm2_v1_skproject_19f0_2165(0x80u, 5, 5, 6, 6, 3, 7, &ctx2165,
                                     &q2165) == 1 &&
              q2165.committed && q2165.action == 24u &&
              q2165.result_word == -4 &&
              shadow.w18 == (uint16_t)(5u | (6u << 5) | (2u << 10)) &&
              shadow.b1a == 24u && shadow.b1e == 7u && shadow.b1d == 3u,
          "DM2_19f0_2165 commits the attack action into the shadow record");
    CHECK(dm2_v1_skproject_19f0_2165(0x00u, 5, 5, 6, 6, 3, 7, &ctx2165,
                                     &q2165) == 1 &&
              !q2165.committed && q2165.action == 24u,
          "DM2_19f0_2165 probe mode returns 1 without committing");
    st2165.v1e057c = 0u;
    CHECK(dm2_v1_skproject_19f0_2165(0x80u, 5, 5, 6, 6, 3, 7, &ctx2165,
                                     &q2165) == 0 &&
              q2165.rejected,
          "DM2_19f0_2165 rejects when v1e057c is zero");

    /* DM2_19f0_266c */
    memset(&db, 0, sizeof(db));
    db.record_a[2] = 0x1au;
    db.record_a[3] = 0x00u;
    db.record_a[4] = 0x00u;
    db.next[1] = 0xfffe;
    db.can_handle = 1;
    r32 = dm2_v1_skproject_19f0_266c(0x0c01u, 0u, 1u, 9, cycle16b_record_fn,
                                     cycle16b_next_fn, cycle16b_can_handle_fn,
                                     &db, &q266c);
    CHECK(r32 == 0x0c01 && q266c.valid,
          "DM2_19f0_266c admits the side-matching 0x1a record");
    db.record_a[2] = 0x26u; /* 0x26 records are skipped */
    r32 = dm2_v1_skproject_19f0_266c(0x0c01u, 0u, 1u, 9, cycle16b_record_fn,
                                     cycle16b_next_fn, cycle16b_can_handle_fn,
                                     &db, &q266c);
    CHECK(r32 == 0xffff && q266c.valid,
          "DM2_19f0_266c returns 0xffff without a matching record");

    /* DM2_19f0_2723 */
    memset(&db, 0, sizeof(db));
    db.record_a[2] = 0x01u;
    db.record_a[3] = 0x00u;
    CHECK(dm2_v1_skproject_19f0_2723(0x1001u, 0, 9, 0, cycle16b_record_fn,
                                     cycle16b_can_handle_fn, &db, &q2723) ==
                  1 &&
              q2723.record_class == 1u,
          "DM2_19f0_2723 admits class 1 with zero arg1");
    CHECK(dm2_v1_skproject_19f0_2723(0x1001u, 2, 9, 0, cycle16b_record_fn,
                                     cycle16b_can_handle_fn, &db, &q2723) ==
                  0,
          "DM2_19f0_2723 rejects class 1 with nonzero arg1");
    db.record_a[2] = 0x17u;
    db.record_a[4] = 0x00u;
    CHECK(dm2_v1_skproject_19f0_2723(0x1001u, 0, 9, 2, cycle16b_record_fn,
                                     cycle16b_can_handle_fn, &db, &q2723) ==
                  1 &&
              q2723.record_class == 0x17u,
          "DM2_19f0_2723 admits class 0x17 against the byte@4 flag");
    db.record_a[2] = 0x03u;
    db.can_handle = 1;
    CHECK(dm2_v1_skproject_19f0_2723(0x1001u, 0, 9, 0, cycle16b_record_fn,
                                     cycle16b_can_handle_fn, &db, &q2723) ==
                  1,
          "DM2_19f0_2723 delegates class 3 to CAN_HANDLE_IT");

    /* DM2_19f0_2813 */
    memset(&db, 0, sizeof(db));
    memset(&st045a, 0, sizeof(st045a));
    memset(&st0559, 0, sizeof(st0559));
    memset(&shadow, 0, sizeof(shadow));
    memset(&ctx2813, 0, sizeof(ctx2813));
    ctx2813.v1e057e = 1u;
    ctx2813.current_map = 1u;
    ctx2813.v1e08ae = 0x10u;
    ctx2813.v1e08a8 = 4u;
    ctx2813.v1e08aa = 5u;
    ctx2813.v1e08b0 = &v1e08b0;
    ctx2813.creature_type = 3u;
    ctx2813.creature_word_e = 0x0200u; /* facing 2 == argw1 */
    ctx2813.map_width = 8;
    ctx2813.map_height = 8;
    ctx2813.randdat = &randdat;
    ctx2813.user = &db;
    ctx2813.tile_link_fn = cycle16b_link_fn;
    ctx2813.record_fn = cycle16b_record_fn;
    ctx2813.next_fn = cycle16b_next_fn;
    ctx2813.can_handle_fn = cycle16b_can_handle_fn;
    ctx2813.state045a = &st045a;
    ctx2813.state0559 = &st0559;
    ctx2813.creature = &shadow;
    ctx2813.v1e056f = &v1e056f;
    v1e08b0 = 0x0c01u; /* type 3, side 0 */
    db.record_a[2] = 0xa6u; /* word@2 0x1a6: class 0x26, type 3 */
    db.record_a[3] = 0x01u;
    db.record_a[4] = 0x04u;
    db.record_b_handle = 0x0c02u;
    db.record_b[2] = 0x03u;
    db.record_b[3] = 0x00u;
    db.next[1] = 0x0c02;
    db.next[2] = 0xfffe;
    db.can_handle = 1;
    v1e056f = 0;
    CHECK(dm2_v1_skproject_19f0_2813(0x80u, 4, 4, 4, 5, 2, 9, &ctx2813,
                                     &q2813) == 1 &&
              q2813.committed && q2813.admitted_handle == 0x0c02u &&
              q2813.result_word == -4 &&
              shadow.w18 == (uint16_t)(4u | (5u << 5) | (1u << 10)) &&
              shadow.b1a == 0x2fu && shadow.b1d == 2u,
          "DM2_19f0_2813 commits the door interaction into the shadow");
    ctx2813.v1e057e = 0u;
    CHECK(dm2_v1_skproject_19f0_2813(0x80u, 4, 4, 4, 5, 2, 9, &ctx2813,
                                     &q2813) == 0 &&
              q2813.rejected,
          "DM2_19f0_2813 rejects without the v1e057e bit");

    /* DM2_4DEA */
    memset(&db, 0, sizeof(db));
    db.gdat_data[32] = 0x44u;
    db.gdat_data[33] = 0x33u;
    db.gdat_data[34] = 0x22u;
    db.gdat_data[35] = 0x11u;
    w16 = 5u;
    CHECK(dm2_v1_skproject_4dea(7u, 3u, &w16, 0u, cycle16b_gdat_ptr_fn, &db,
                                &v32, &q4dea) == 1 &&
              v32 == 0x11223344u && q4dea.index == 8u,
          "DM2_4DEA fetches four GDAT bytes at the computed index");

    /* DM2_1BA1B */
    memset(&db, 0, sizeof(db));
    db.tile = (uint8_t)((4u << 5) | 4u); /* door variant 4 */
    db.door_gfx = 0;
    CHECK(dm2_v1_skproject_1ba1b(4, 5, cycle16b_tile_fn,
                                 cycle16b_tile_record_fn, cycle16b_rebirth_fn,
                                 cycle16b_door_gfx_fn, &db, &q1ba1b) == 1 &&
              q1ba1b.passable && q1ba1b.door_variant == 4u,
          "DM2_1BA1B passes variant-4 doors without door graphics");
    db.door_gfx = 3;
    CHECK(dm2_v1_skproject_1ba1b(4, 5, cycle16b_tile_fn,
                                 cycle16b_tile_record_fn, cycle16b_rebirth_fn,
                                 cycle16b_door_gfx_fn, &db, &q1ba1b) == 0,
          "DM2_1BA1B blocks doors with door graphics");
    db.tile = 0xc0u; /* type 6, bit 2 clear */
    CHECK(dm2_v1_skproject_1ba1b(4, 5, cycle16b_tile_fn,
                                 cycle16b_tile_record_fn, cycle16b_rebirth_fn,
                                 cycle16b_door_gfx_fn, &db, &q1ba1b) == 1,
          "DM2_1BA1B passes type-6 tiles with bit 2 clear");
    db.tile = 0xc4u; /* type 6, bit 2 set */
    CHECK(dm2_v1_skproject_1ba1b(4, 5, cycle16b_tile_fn,
                                 cycle16b_tile_record_fn, cycle16b_rebirth_fn,
                                 cycle16b_door_gfx_fn, &db, &q1ba1b) == 0,
          "DM2_1BA1B blocks type-6 tiles with bit 2 set");

    /* DM2_1c9a_0247 */
    memset(&db, 0, sizeof(db));
    db.alloc_ret[0] = 1;
    db.alloc_ret[1] = 0;
    CHECK(dm2_v1_skproject_1c9a_0247(0x0200u, 0x0100u, cycle16b_alloc_fn,
                                     cycle16b_dballoc_free_fn, &db,
                                     &q0247) == 1 &&
              q0247.key_low == 0x0200u && q0247.freed_2 == 1u &&
              q0247.freed_3 == 0u && db.freed[0] == 7u,
          "DM2_1c9a_0247 frees the 0x20000000 allocation only");

    /* DM2_1c9a_0648 */
    memset(&db, 0, sizeof(db));
    memset(&st0648, 0, sizeof(st0648));
    st0648.v1d3248 = 2u;
    st0648.v1e027c = 9u;
    st0648.v1e0258 = 1u;
    st0648.v1e0270 = 7u;
    st0648.v1e0272 = 8u;
    st0648.v1e0266 = 4u;
    st0648.party_absdir = 3u;
    CHECK(dm2_v1_skproject_1c9a_0648(2u, &st0648, cycle16b_change_map_fn,
                                     &db, &q0648) == 2 &&
              !q0648.map_changed && db.change_map_seen == 0,
          "DM2_1c9a_0648 returns early when the map is unchanged");
    CHECK(dm2_v1_skproject_1c9a_0648(5u, &st0648, cycle16b_change_map_fn,
                                     &db, &q0648) == 5 &&
              q0648.map_changed && !q0648.from_party &&
              st0648.v1e08da == 1u && st0648.v1e08d8 == 7u &&
              st0648.v1e08d4 == 8u && st0648.v1e08d6 == 4u &&
              db.change_map_seen == 5,
          "DM2_1c9a_0648 copies the transition words on a map change");
    CHECK(dm2_v1_skproject_1c9a_0648(9u, &st0648, cycle16b_change_map_fn,
                                     &db, &q0648) == 9 &&
              q0648.from_party && st0648.v1e08da == 3u &&
              st0648.v1e08d6 == 9u,
          "DM2_1c9a_0648 uses the party direction at the transition map");
}

/* ---- cycle-18 batch-18 (c_1c9a.cpp) test fixtures ---- */

static uint8_t g_c18_records[4][24];

static uint16_t cycle18_distinctive_type_fn(uint16_t record, void *user)
{
    (void)user;
    return record;
}

static int32_t cycle18_oversee_search_fn(uint16_t start, uint16_t creature,
                                          int32_t filter, void *user)
{
    (void)creature; (void)user;
    if (filter == (int32_t)0xfffffffe) return (int32_t)start;
    if ((int32_t)start == filter) return (int32_t)start;
    return (int32_t)0xfffffffe;
}

#define CYCLE18_NODE_MONEYBOX 0x2401u /* type nibble 9 */
#define CYCLE18_NODE_ITEM 0x1802u     /* type nibble 6 */

static int32_t cycle18_next_record_fn(uint16_t handle, void *user)
{
    (void)user;
    if (handle == CYCLE18_NODE_MONEYBOX) return CYCLE18_NODE_ITEM;
    if (handle == CYCLE18_NODE_ITEM) return 0xfffeu;
    return 0xfffeu;
}

static int32_t cycle18_can_handle_fn(uint16_t item, int16_t creature, void *user)
{
    (void)item; (void)user;
    return creature != 0;
}

static int32_t cycle18_moneybox_fn(uint16_t record, void *user)
{
    (void)user;
    return ((record >> 10) & 0xfu) == 9u;
}

static int g_c18_cut_calls;
static int g_c18_append_calls;
static int g_c18_dealloc_calls;

static void cycle18_cut_fn(uint16_t record, uint16_t container, int16_t x,
                            int16_t y, void *user)
{
    (void)record; (void)container; (void)x; (void)y; (void)user;
    g_c18_cut_calls++;
}

static void cycle18_append_fn(uint16_t record, uint16_t container, int16_t x,
                               int16_t y, void *user)
{
    (void)record; (void)container; (void)x; (void)y; (void)user;
    g_c18_append_calls++;
}

static void cycle18_dealloc_fn(uint16_t record, void *user)
{
    (void)record; (void)user;
    g_c18_dealloc_calls++;
}

static int32_t cycle18_contents_head_fn(uint16_t container_record, void *user)
{
    (void)container_record; (void)user;
    return (int32_t)0xfffeu;
}

static int32_t cycle18_blend_fn(uint8_t creature_type, uint16_t base,
                                 const int16_t *table, void *user)
{
    (void)creature_type; (void)table; (void)user;
    return (int32_t)base;
}

static int g_c18_anim_calls;
static void cycle18_animation_fn(uint8_t creature_type, uint16_t mode,
                                  uint16_t ai_pointer, uint16_t ai_addend,
                                  uint16_t v1e055e_word0, void *user)
{
    (void)creature_type; (void)mode; (void)ai_pointer; (void)ai_addend;
    (void)v1e055e_word0; (void)user;
    g_c18_anim_calls++;
}

static const uint8_t *cycle18_record_fn(uint16_t handle, uint16_t *out_size,
                                         void *user)
{
    uint16_t index = (uint16_t)(handle & 0x3u); /* low bits address the slot;
                                                    the high nibble carries
                                                    the record's type. */
    (void)user;
    if (out_size) *out_size = sizeof(g_c18_records[index]);
    return (const uint8_t *)g_c18_records[index];
}

static int32_t cycle18_creature_at_fn(uint16_t x, uint16_t y, void *user)
{
    (void)user;
    if (x == 1 && y == 2) return 0;
    return -1;
}

static int g_c18_queue_calls;
static void cycle18_queue_timer_fn(uint16_t creature_slot, uint8_t type,
                                    uint8_t actor, uint8_t x, uint8_t y,
                                    uint16_t tick, int32_t *out_timer,
                                    void *user)
{
    (void)creature_slot; (void)type; (void)actor; (void)x; (void)y;
    (void)tick; (void)user;
    g_c18_queue_calls++;
    if (out_timer) *out_timer = 42;
}

static int g_c18_delete_timer_calls;
static void cycle18_delete_timer_fn(uint16_t timer, void *user)
{
    (void)timer; (void)user;
    g_c18_delete_timer_calls++;
}

static int g_c18_slot_occupied[8];
static int cycle18_slot_occupied_fn(uint16_t slot, void *user)
{
    (void)user;
    return g_c18_slot_occupied[slot];
}

static int cycle18_recycle_calls;
static int32_t cycle18_recycle_fn(uint8_t cls, uint8_t priority, void *user)
{
    (void)cls; (void)priority; (void)user;
    cycle18_recycle_calls++;
    return -1;
}

static int g_c18_deleted_creature_record;
static void cycle18_delete_creature_fn(uint16_t x, uint16_t y, uint16_t arg2,
                                        uint16_t arg3, void *user)
{
    (void)x; (void)y; (void)arg2; (void)arg3; (void)user;
    g_c18_deleted_creature_record = 1;
}

static int32_t cycle18_missile_ref_fn(uint16_t creature, uint16_t default_map,
                                      void *user)
{
    (void)creature; (void)default_map; (void)user;
    return (int32_t)((2u << 10) | (3u << 5) | 4u);
}

static uint16_t g_c18_current_map;
static int32_t cycle18_change_map_fn(int16_t map, void *user)
{
    (void)user;
    g_c18_current_map = (uint16_t)map;
    return 0;
}

static int g_c18_ai_13e4_calls;
static void cycle18_ai_13e4_0360_fn(uint16_t creature, int16_t x, int16_t y,
                                     uint16_t reason, uint16_t arg4,
                                     void *user)
{
    (void)creature; (void)x; (void)y; (void)reason; (void)arg4; (void)user;
    g_c18_ai_13e4_calls++;
}

static int32_t cycle18_calc_vector_dir_fn(uint16_t ref_y, int16_t dy,
                                           int16_t ref_x, int16_t dx,
                                           void *user)
{
    (void)ref_y; (void)dy; (void)ref_x; (void)dx; (void)user;
    return 1;
}

static int g_c18_attack_calls;
static void cycle18_attack_fn(uint16_t creature, int16_t x, int16_t y,
                               uint16_t dir, uint16_t power, uint16_t arg5,
                               void *user)
{
    (void)creature; (void)x; (void)y; (void)dir; (void)power; (void)arg5;
    (void)user;
    g_c18_attack_calls++;
}

static void test_skwin_core_symbol_batch_cycle18(void)
{
    DM2_V1_Skproject06bdReceipt q06bd;
    DM2_V1_Skproject078bReceipt q078b;
    DM2_V1_Skproject0958Receipt q0958;
    DM2_V1_Skproject09dbReceipt q09db;
    DM2_V1_Skproject0a48Receipt q0a48;
    DM2_V1_Skproject0cf7Receipt q0cf7;
    DM2_V1_Skproject0db0Receipt q0db0;
    DM2_V1_Skproject14cd0802Slot slot0802;
    DM2_V1_SkprojectAllocCaiiReceipt qalloc;
    DM2_V1_Skproject0fcbReceipt q0fcb;
    DM2_V1_SkprojectCreateMinionReceipt qminion;
    DM2_V1_SkprojectReleaseMinionReceipt qrelease;
    DM2_V1_Skproject17c7State st17c7;
    DM2_V1_Skproject19d4Receipt q19d4;

    /* DM2_1c9a_0694 */
    CHECK(dm2_v1_skproject_1c9a_0694(5u, -2, cycle18_distinctive_type_fn,
                                     NULL) == 1,
          "DM2_1c9a_0694 wildcard filter always matches");
    CHECK(dm2_v1_skproject_1c9a_0694(5u, 5, cycle18_distinctive_type_fn,
                                     NULL) == 1,
          "DM2_1c9a_0694 matches equal distinctive type");
    CHECK(dm2_v1_skproject_1c9a_0694(5u, 6, cycle18_distinctive_type_fn,
                                     NULL) == 0,
          "DM2_1c9a_0694 rejects mismatched distinctive type");

    /* DM2_1c9a_06bd */
    CHECK(dm2_v1_skproject_1c9a_06bd(-1, 0u, 0, cycle18_oversee_search_fn,
                                     NULL, &q06bd) == 0 && q06bd.valid,
          "DM2_1c9a_06bd short-circuits the -1 sentinel");
    CHECK(dm2_v1_skproject_1c9a_06bd(7, 0u, 7, cycle18_oversee_search_fn,
                                     NULL, &q06bd) == 7 && !q06bd.no_match,
          "DM2_1c9a_06bd returns the matched record");
    CHECK(dm2_v1_skproject_1c9a_06bd(7, 0u, 9, cycle18_oversee_search_fn,
                                     NULL, &q06bd) == 0 && q06bd.no_match,
          "DM2_1c9a_06bd reports no_match on the 0xfffffffe sentinel");
    CHECK(dm2_v1_skproject_1c9a_06bd(7, 0u, 9, NULL, NULL, &q06bd) == 0 &&
              q06bd.blocked_missing_callback,
          "DM2_1c9a_06bd fails closed without the oversee callback");

    /* DM2_1c9a_078b */
    g_c18_cut_calls = g_c18_append_calls = g_c18_dealloc_calls = 0;
    CHECK(dm2_v1_skproject_1c9a_078b(
              0x0001u, 1, 0xffu, CYCLE18_NODE_MONEYBOX,
              cycle18_next_record_fn, cycle18_can_handle_fn,
              cycle18_moneybox_fn, cycle18_cut_fn, cycle18_append_fn,
              cycle18_dealloc_fn, cycle18_contents_head_fn, NULL,
              &q078b) == (int32_t)0xfffeu &&
              q078b.valid && q078b.visited == 2u &&
              g_c18_cut_calls > 0 && g_c18_dealloc_calls > 0,
          "DM2_1c9a_078b walks the chain and redistributes handled items");
    CHECK(dm2_v1_skproject_1c9a_078b(0x0001u, 1, 0xffu,
                                     CYCLE18_NODE_MONEYBOX, NULL, NULL, NULL,
                                     NULL, NULL, NULL, NULL, NULL,
                                     &q078b) ==
              (int32_t)CYCLE18_NODE_MONEYBOX &&
              q078b.blocked_missing_callback,
          "DM2_1c9a_078b fails closed without callbacks");

    /* DM2_1c9a_0958 */
    CHECK(dm2_v1_skproject_1c9a_0958(3u, 0x1234u, NULL, cycle18_blend_fn,
                                     NULL, &q0958) ==
              (int32_t)((0x1234u & 0xffffff80u) >> 7) &&
              q0958.valid,
          "DM2_1c9a_0958 blends and shifts by 7");
    CHECK(dm2_v1_skproject_1c9a_0958(3u, 0x1234u, NULL, NULL, NULL,
                                     &q0958) == 0 &&
              q0958.blocked_missing_callback,
          "DM2_1c9a_0958 fails closed without the blend callback");

    /* DM2_1c9a_09b9 */
    memset(g_c18_records, 0, sizeof(g_c18_records));
    g_c18_records[0][8] = 0x0007u;
    CHECK(dm2_v1_skproject_1c9a_09b9(0u, 7u, cycle18_record_fn, NULL) == 1,
          "DM2_1c9a_09b9 matches the creature index");
    CHECK(dm2_v1_skproject_1c9a_09b9(0u, 8u, cycle18_record_fn, NULL) == 0,
          "DM2_1c9a_09b9 rejects a mismatched creature index");

    /* DM2_1c9a_09db */
    g_c18_anim_calls = 0;
    CHECK(dm2_v1_skproject_1c9a_09db(2u, 0x10u, 0u, cycle18_animation_fn,
                                     NULL, &q09db) == 1 &&
              g_c18_anim_calls == 1 && q09db.valid,
          "DM2_1c9a_09db forwards to the animation-frame callback");
    CHECK(dm2_v1_skproject_1c9a_09db(2u, 0x10u, 0u, NULL, NULL, &q09db) ==
              0 &&
              q09db.blocked_missing_callback,
          "DM2_1c9a_09db fails closed without the animation callback");

    /* DM2_CREATURE_SOMETHING_1c9a_0a48 */
    CHECK(dm2_v1_skproject_creature_something_1c9a_0a48(NULL, &q0a48) == 0 &&
              q0a48.blocked_missing_state,
          "DM2_CREATURE_SOMETHING_1c9a_0a48 stays fail-closed");

    /* DM2_1c9a_0cf7 / DM2_1c9a_0db0 */
    memset(g_c18_records, 0, sizeof(g_c18_records));
    g_c18_records[0][4] = 0x02u; /* rec[4]=actor */
    g_c18_records[0][5] = 0xffu; /* rec[5] unallocated -> 0db0 no-op */
    g_c18_records[0][8] = 0xffu;
    g_c18_records[0][9] = 0xffu;
    g_c18_queue_calls = g_c18_delete_timer_calls = 0;
    CHECK(dm2_v1_skproject_1c9a_0cf7(3u, 1u, 2u, 100u, cycle18_creature_at_fn,
                                     cycle18_record_fn,
                                     cycle18_queue_timer_fn,
                                     cycle18_delete_timer_fn, NULL,
                                     &q0cf7) == 1 &&
              q0cf7.creature_slot == 0 && q0cf7.timer == 42 &&
              g_c18_queue_calls == 1,
          "DM2_1c9a_0cf7 queues a move-away timer for the found creature");
    CHECK(dm2_v1_skproject_1c9a_0cf7(3u, 9u, 9u, 100u, cycle18_creature_at_fn,
                                     cycle18_record_fn,
                                     cycle18_queue_timer_fn,
                                     cycle18_delete_timer_fn, NULL,
                                     &q0cf7) == 0 && q0cf7.creature_slot < 0,
          "DM2_1c9a_0cf7 is a no-op when no creature occupies the cell");

    CHECK(dm2_v1_skproject_1c9a_0db0(0u, cycle18_record_fn,
                                     cycle18_delete_timer_fn, NULL,
                                     &q0db0) == 0 && q0db0.valid &&
              !q0db0.cancelled,
          "DM2_1c9a_0db0 is a no-op outside type nibble 4");
    {
        uint16_t masked = (uint16_t)(0u | (4u << 10));
        memset(g_c18_records, 0, sizeof(g_c18_records));
        g_c18_records[0][5] = 0u; /* allocated */
        g_c18_records[0][4 + 2] = 0x07u; /* timer slot word */
        g_c18_delete_timer_calls = 0;
        CHECK(dm2_v1_skproject_1c9a_0db0(masked, cycle18_record_fn,
                                         cycle18_delete_timer_fn, NULL,
                                         &q0db0) == 1 && q0db0.cancelled &&
                  g_c18_delete_timer_calls == 1,
              "DM2_1c9a_0db0 cancels the pending timer for type nibble 4");
    }

    /* DM2_14cd_0802 */
    slot0802.caii_index = 3u;
    slot0802.caii_flags = 7u;
    dm2_v1_skproject_14cd_0802(&slot0802);
    CHECK(slot0802.caii_index == 0xffu && slot0802.caii_flags == 0u,
          "DM2_14cd_0802 resets the caii index and flags");

    /* DM2_ALLOC_CAII_TO_CREATURE */
    memset(g_c18_slot_occupied, 0, sizeof(g_c18_slot_occupied));
    g_c18_slot_occupied[0] = g_c18_slot_occupied[1] = 1;
    CHECK(dm2_v1_skproject_alloc_caii_to_creature(
              0u, 0xffu, 8u, cycle18_slot_occupied_fn, cycle18_recycle_fn,
              NULL, &qalloc) == 1 &&
              qalloc.slot == 2 && qalloc.valid,
          "DM2_ALLOC_CAII_TO_CREATURE finds the first free slot");
    CHECK(dm2_v1_skproject_alloc_caii_to_creature(
              0u, 0u, 8u, cycle18_slot_occupied_fn, cycle18_recycle_fn, NULL,
              &qalloc) == 0 &&
              qalloc.already_allocated,
          "DM2_ALLOC_CAII_TO_CREATURE skips an already-allocated record");
    {
        int all_occupied[8];
        int i;
        for (i = 0; i < 8; i++) all_occupied[i] = 1;
        memcpy(g_c18_slot_occupied, all_occupied, sizeof(all_occupied));
        cycle18_recycle_calls = 0;
        CHECK(dm2_v1_skproject_alloc_caii_to_creature(
                  0u, 0xffu, 8u, cycle18_slot_occupied_fn, cycle18_recycle_fn,
                  NULL, &qalloc) == 0 &&
                  qalloc.blocked_missing_callback &&
                  cycle18_recycle_calls == 1,
              "DM2_ALLOC_CAII_TO_CREATURE fails closed when recycle fails");
    }

    /* DM2_1c9a_0fcb */
    g_c18_deleted_creature_record = 0;
    CHECK(dm2_v1_skproject_1c9a_0fcb(2u, 8u, 0u, 0x13u, 0u, 42, 1u, 2u,
                                     cycle18_record_fn,
                                     cycle18_delete_timer_fn,
                                     cycle18_delete_creature_fn, NULL,
                                     &q0fcb) == 1 &&
              q0fcb.deleted_creature_record &&
              g_c18_deleted_creature_record,
          "DM2_1c9a_0fcb deletes the creature record for type 0x13 "
          "without the AI-spec flag");
    CHECK(dm2_v1_skproject_1c9a_0fcb(2u, 8u, 0u, 0x13u, 0x1u, 42, 1u, 2u,
                                     cycle18_record_fn,
                                     cycle18_delete_timer_fn,
                                     cycle18_delete_creature_fn, NULL,
                                     &q0fcb) == 1 &&
              !q0fcb.deleted_creature_record,
          "DM2_1c9a_0fcb keeps the record when the AI-spec flag is set");
    CHECK(dm2_v1_skproject_1c9a_0fcb(9u, 8u, 0u, 0x13u, 0u, 42, 1u, 2u,
                                     cycle18_record_fn,
                                     cycle18_delete_timer_fn,
                                     cycle18_delete_creature_fn, NULL,
                                     &q0fcb) == 0 &&
              q0fcb.blocked_out_of_range,
          "DM2_1c9a_0fcb fails closed for an out-of-range slot");

    /* DM2_CREATE_MINION */
    CHECK(dm2_v1_skproject_create_minion(NULL, &qminion) == -1 &&
              qminion.blocked_missing_state,
          "DM2_CREATE_MINION stays fail-closed");

    /* DM2_RELEASE_MINION */
    g_c18_current_map = 0u;
    g_c18_ai_13e4_calls = 0;
    dm2_v1_skproject_release_minion(0u, 5u, cycle18_missile_ref_fn,
                                    cycle18_change_map_fn,
                                    cycle18_ai_13e4_0360_fn, NULL,
                                    &qrelease);
    CHECK(qrelease.had_missile_ref && qrelease.valid &&
              g_c18_ai_13e4_calls == 1 && g_c18_current_map == 5u,
          "DM2_RELEASE_MINION dispatches ai_13e4_0360 and restores the map");

    /* DM2_1c9a_17c7 */
    memset(&st17c7, 0, sizeof(st17c7));
    st17c7.map = 4u;
    st17c7.v1e08d6 = 4u;
    st17c7.v1e08d8 = 10u;
    st17c7.v1e08d4 = 10u;
    st17c7.v1e08da = 1u;
    CHECK(dm2_v1_skproject_1c9a_17c7(8, 9, &st17c7,
                                     cycle18_calc_vector_dir_fn, NULL) == 1,
          "DM2_1c9a_17c7 matches when close and direction agrees");
    CHECK(dm2_v1_skproject_1c9a_17c7(1, 1, &st17c7,
                                     cycle18_calc_vector_dir_fn, NULL) == 0,
          "DM2_1c9a_17c7 rejects targets too far away");
    st17c7.map = 3u;
    CHECK(dm2_v1_skproject_1c9a_17c7(8, 9, &st17c7,
                                     cycle18_calc_vector_dir_fn, NULL) == 0,
          "DM2_1c9a_17c7 rejects when off the current map");

    /* DM2_1c9a_19d4 */
    g_c18_attack_calls = 0;
    dm2_v1_skproject_1c9a_19d4(0u, 1, 8u, 2, cycle18_attack_fn, NULL,
                               &q19d4);
    CHECK(!q19d4.out_of_range && g_c18_attack_calls == 1,
          "DM2_1c9a_19d4 dispatches DM2_ATTACK_CREATURE inside [6,0x15]");
    g_c18_attack_calls = 0;
    dm2_v1_skproject_1c9a_19d4(0u, 1, 3u, 2, cycle18_attack_fn, NULL,
                               &q19d4);
    CHECK(q19d4.out_of_range && g_c18_attack_calls == 0,
          "DM2_1c9a_19d4 rejects command words below 6");
}

int main(void)
{
    test_between_value();
    test_temp_rect_ring();
    test_random_helpers();
    test_ibmio_anim_mouse_runtime_family();
    test_ui_predicate_1031_runtime_family();
    test_ui_1031_node_flow_helpers();
    test_ui_1031_action_and_tree_runtime();
    test_real_gdat_ibmio_palette_family();
    test_util_helpers();
    test_palette_helpers();
    test_gui_receipt_helpers();
    test_move_admission_helpers();
    test_map_helpers();
    test_calc_vector_w_dir();
    test_cache_hash_helpers();
    test_picture_mement_helpers();
    test_item_charge_helpers();
    test_item_value_weight_helpers();
    test_player_weight_helper();
    test_count_by_coin_types();
    test_boost_attribute();
    test_adjust_ui_event();
    test_gfx_str_helpers();
    test_gdat_allocation_helpers();
    test_xrect_codec();
    test_skwin_core_symbol_batch_cycle3();
    test_skwin_core_symbol_batch_cycle4();
    test_skwin_core_symbol_batch_cycle5();
    test_skwin_core_symbol_batch_cycle6();
    test_skwin_core_symbol_batch_cycle7();
    test_skwin_core_symbol_batch_cycle8();
    test_skwin_core_symbol_batch_cycle9();
    test_skwin_core_symbol_batch_cycle10();
    test_skwin_core_symbol_batch_cycle11();
    test_skwin_core_symbol_batch_cycle12();
    test_skwin_core_symbol_batch_cycle13();
    test_skwin_core_symbol_batch_cycle14();
    test_skwin_core_symbol_batch_cycle15();
    test_skwin_core_symbol_batch_cycle16();
    test_skwin_core_symbol_batch_cycle16b();
    test_skwin_core_symbol_batch_cycle18();
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_CREATURE_SOMETHING_1c9a_0a48") != 0,
          "source evidence names the cycle-18 batch-18 symbols");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_TEMP_RECT") != 0,
          "source evidence names ALLOC_TEMP_RECT");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_BETWEEN_VALUE") != 0,
          "source evidence names DM2_BETWEEN_VALUE");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_RAND16") != 0,
          "source evidence names c_random helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_CALC_SQUARE_DISTANCE") != 0,
          "source evidence names utility helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "CALC_VECTOR_W_DIR") != 0,
          "source evidence names vector direction helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADD_CACHE_HASH") != 0,
          "source evidence names cache hash helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_IMAGE_MEMENT") != 0,
          "source evidence names picture mement helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADD_ITEM_CHARGE") != 0,
          "source evidence names item charge helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "GET_MAX_CHARGE") != 0,
          "source evidence names max charge helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "QUERY_ITEM_VALUE") != 0,
          "source evidence names item value helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "CALC_PLAYER_WEIGHT") != 0,
          "source evidence names player weight helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "COUNT_BY_COIN_TYPES") != 0,
          "source evidence names coin count helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "IS_CONTAINER_MONEYBOX") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "GET_ITEM_ORDER_IN_CONTAINER") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FMT_NUM") != 0,
          "source evidence names item/container classifier helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "FILL_STR") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "HIGHLIGHT_ARROW_PANEL") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "FIRE_FILL_HALFTONE_RECTI") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_MOUSE_RELEASE_CAPTURE") != 0,
          "source evidence names text/fill/mouse wrapper helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "SK_STRLEN") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "SK_STRSTR") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_SKCHR_TO_SCRIPTCHR") != 0,
          "source evidence names skproject string helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "BOOST_ATTRIBUTE") != 0,
          "source evidence names attribute boost helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADJUST_UI_EVENT") != 0,
          "source evidence names UI event adjustment helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_QUERY_FONT") != 0,
          "source evidence names c_gfx_str helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_12b4_0881") != 0,
          "source evidence names c_move admission helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_ATTACK_WALL") != 0,
          "source evidence names c_move wall attack helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_ATTACK_DOOR") != 0,
          "source evidence names c_move door attack helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_SET_DESTINATION_OF_MINION_MAP") != 0,
          "source evidence names c_map helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "IS_NEGATIVE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "IS_CONTAINER_MAP") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "FIND_POUCH_OR_SCABBARD_POSSESSION_POS") != 0,
          "source evidence names scalar/container possession helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "PT_IN_RECT") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "OFFSET_RECT") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "PTR_ADVANCE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "WRITE_BYTE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "WRITE_WORD") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "READ_BYTE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "READ_SBYTE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "READ_WORD") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "COMPRESS_RECTS") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "QUERY_RECT") != 0,
          "source evidence names rect and cursor helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "FREE_CACHE_INDEX") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "FREE_INDEXED_MEMENT") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "FREE_TEMP_CACHE_INDEX") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "FREE_PICT6") != 0,
          "source evidence names cache and picture free helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_GET_ADDRESS_OF_RECORD") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "GET_ADDRESS_OF_RECORDX4") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "GET_ADDRESS_OF_GENERIC_CONTAINER_RECORD") != 0,
          "source evidence names c_record address helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "GET_ADDRESS_OF_TILE_RECORD") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "GET_TILE_VALUE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "FILL_RECT_SUMMARY") != 0,
          "source evidence names tile and fill helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM_LOCATE_OTHER_LEVEL") != 0,
          "source evidence names other-level locator");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_map_3BF83") != 0,
          "source evidence names cross-map record mover");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_0cee_17e7") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_0cee_1815") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_0cee_185a") != 0,
          "source evidence names _0cee wall decoration helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_LOAD_DYN4") != 0,
          "source evidence names GDAT DYN4 helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_00eb_04bc") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_00eb_070c") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_01b0_0adb") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_0759_06db") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_01b0_1ed2") != 0,
          "source evidence names IBMIO palette/anim/mouse runtime helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_4976_0cba") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_0023") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_012d") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_014f") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_027e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_030a") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_0a88") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_0b7e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_0c58") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_10c8") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_1031_098e") != 0,
          "source evidence names _1031 UI predicate dispatch family");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_DRAW_SPELL_PANEL") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_DRAW_ITEM_ICON") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2405_00ec") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2405_011f") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2405_014a") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DRAW_SQUAD_POS_INTERFACE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DRAW_SKILL_PANEL") != 0,
          "source evidence names bundled c_gui_draw receipt helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_0b36_00c3") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_0b36_0c52") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_0b36_0d67") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_0b36_11c0") != 0,
          "source evidence names _0b36 cached picture button-group helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_443c_087c/_443c_0889/_443c_040e/") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_443c_00a9/_443c_06b4/_443c_07d5") != 0,
          "source evidence names cycle-4 _443c UI tracking batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_3e74_48c9") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_4549") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_0c8c") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_0d32") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_2b30") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_583a") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_585a") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_4471") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_3e74_44ad") != 0,
          "source evidence names cycle-5 _3e74 mement/cache batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "_1c9a_02c3") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_4937_01a9") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_4937_000f") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2759_0155") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2759_01fe") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2759_0e93") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_24a5_0732") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "_2e62_03b5") != 0,
          "source evidence names cycle-6 creature/animation/UI batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_1031_01d5") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_023b") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_024c") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_027e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_030a") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_04f5") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_0541") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_0675") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_29ee_0b2b") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_03f2") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_0b36_129a") != 0,
          "source evidence names cycle-7 SKULLWIN original audit batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "gate_1031") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_10777") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_107B0") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_06a5") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_06b3") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_0781") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1031_07d6") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_CLICK_MAGICAL_MAP_AT") != 0,
          "source evidence names cycle-8 c_1031 completion batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_query_098d_000f") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_CLS1_CRITICAL_FOR_LOAD") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_QUERY_GDAT_DYN_BUFF") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_WALL_ORNATE_ALCOVE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_TILE_BLOCKED") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_REBIRTH_ALTAR") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_WALL_ORNATE_SPRING") != 0,
          "source evidence names cycle-9 c_querydb predicate batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_GET_CREATURE_AT") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FIND_LADDAR_AROUND") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_GET_PLAYER_AT_POSITION") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_DIR_FROM_5x5_POS") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_GET_GLOB_VAR") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_GET_CREATURE_WEIGHT") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_CONVERT_PALETTE256") != 0,
          "source evidence names cycle-10 c_querydb lookup batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_IS_DISTINCTIVE_ITEM_ON_ACTUATOR") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FIND_HAND_WITH_EMPTY_FLASK") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FIND_DISTINCTIVE_ITEM_ON_TILE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FIND_TILE_ACTUATOR") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_CALC_PLAYER_WALK_DELAY") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH") != 0,
          "source evidence names cycle-11 c_querydb query batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_query_32cb_0804") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0b36_037e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_1c9a_08bd") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_CREATURE_FLOATING") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_OBJECT_FLOATING") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_QUERY_OBJECT_5x5_POS") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_48ae_05ae") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_4E26") != 0,
          "source evidence names cycle-12 c_querydb query batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_query_4DA3") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_QUERY_CREATURE_5x5_POS") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0cee_0897") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_GET_TELEPORTER_DETAIL") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_CREATURE_MOVABLE_THERE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0cee_1a46") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_48ae_011a") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0cee_2e09") != 0,
          "source evidence names cycle-13 c_querydb query batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_query_1c9a_03cf") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_48ae_01af") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0cee_2e35") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_QUERY_CREATURE_PICST") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_2fcf_164e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_2fcf_16ff") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_48ae_0767") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0cee_06dc") != 0,
          "source evidence names cycle-14 c_querydb query batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_query_19f0_124b") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_29ee_18eb") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_IS_CREATURE_ALLOWED_ON_LEVEL") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_query_0cee_319e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1BAAD") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1BC29") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_0207") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_045a") != 0,
          "source evidence names cycle-15 c_querydb/c_1c9a query batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_19f0_04bf") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_050f") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_0547") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_0559") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_05e8") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_0598") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_0891") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_0d10") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_2807") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_2886") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_56") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_57") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_59_76") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_62") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_63") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_64") != 0,
          "source evidence names cycle-16 c_1c9a/c_ai symbol batch");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_0aaf_0067") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_0aaf_01db") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_0aaf_02f8") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_13aa") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_1511") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_D283") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_CREATURE_GO_THERE") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_2024") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_2165") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_266c") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_2723") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_19f0_2813") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_4DEA") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1BA1B") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_0247") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_0648") != 0,
          "source evidence names cycle-16 batch-17");

    /* batch-19a: walk path / CAII symbols */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_1c9a_1a48") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_1b16") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_1bae") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FIND_WALK_PATH") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2___SET_CURRENT_THINKING_CREATURE_WALK_PATH") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_381c") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_1c9a_38a8") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_FILL_CAII_CUR_MAP") != 0,
          "source evidence names batch-19a walk path/CAII");

    /* batch-19a: 1a48 no-match returns -1 */
    {
        DM2_V1_Skproject1a48Receipt r1a48;
        int32_t ret = dm2_v1_skproject_1c9a_1a48(
            0, 0, (int16_t)0xFFFE, NULL, NULL, NULL, &r1a48);
        CHECK(ret == -1 && r1a48.valid, "1a48 end-of-list returns -1");
    }

    /* batch-19a: 1b16 no-match returns -1 */
    {
        DM2_V1_Skproject1b16Receipt r1b16;
        int32_t ret = dm2_v1_skproject_1c9a_1b16(
            0, 0, (int16_t)0xFFFE, NULL, NULL, NULL, &r1b16);
        CHECK(ret == -1 && r1b16.valid, "1b16 end-of-list returns -1");
    }

    /* batch-19a: 1bae matches creature pos */
    {
        DM2_V1_Skproject1baeReceipt r1bae;
        int32_t ret = dm2_v1_skproject_1c9a_1bae(
            5, 10, 5, 10, NULL, NULL, &r1bae);
        CHECK(ret == 0 && r1bae.matched_creature_pos && r1bae.valid,
              "1bae returns 0 at creature position");
    }

    /* batch-19a: find_walk_path receipt init */
    {
        DM2_V1_SkprojectFindWalkPathReceipt rwp;
        rwp.valid = 99;
        dm2_v1_skproject_find_walk_path_receipt_init(&rwp);
        CHECK(rwp.valid == 0, "find_walk_path receipt init clears");
    }

    /* batch-19a: set walk path — null creatures early exit */
    {
        DM2_V1_SkprojectWalkPathState wps;
        memset(&wps, 0, sizeof(wps));
        wps.creatures = NULL;
        DM2_V1_SkprojectSetWalkPathReceipt rswp;
        dm2_v1_skproject_set_current_thinking_creature_walk_path(
            &wps, NULL, NULL, NULL, &rswp);
        CHECK(rswp.valid && rswp.early_exit_no_creatures,
              "set walk path exits on null creatures");
    }

    /* batch-19a: 381c null state returns 0 */
    {
        DM2_V1_Skproject381cReceipt r381c;
        int32_t ret = dm2_v1_skproject_1c9a_381c(
            NULL, NULL, NULL, NULL, NULL, &r381c);
        CHECK(ret == 0 && r381c.valid, "381c null returns 0");
    }

    /* batch-19a: 38a8 null state returns 0 */
    {
        DM2_V1_Skproject38a8Receipt r38a8;
        int32_t ret = dm2_v1_skproject_1c9a_38a8(NULL, &r38a8);
        CHECK(ret == 0 && r38a8.valid, "38a8 null returns 0");
    }

    /* batch-19a: fill_caii_cur_map null returns 0 */
    {
        DM2_V1_SkprojectFillCaiiReceipt rfill;
        int32_t ret = dm2_v1_skproject_fill_caii_cur_map(
            NULL, NULL, NULL, NULL, NULL, NULL, &rfill);
        CHECK(ret == 0 && rfill.valid, "fill_caii null returns 0");
    }

    /* ---- DM2_FILL_ORPHAN_CAII ---- */
    {
        DM2_V1_SkprojectFillOrphanCaiiReceipt r;
        dm2_v1_skproject_fill_orphan_caii(3, 5, NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_callback == 1,
              "fill_orphan_caii blocks without callbacks");
    }

    /* ---- event_loop_T1 ---- */
    {
        DM2_V1_SkprojectEventLoopT1Receipt r;
        int vsync_out = 0, tick_out = 0;
        dm2_v1_skproject_event_loop_t1(4, 1, &vsync_out, &tick_out, &r);
        CHECK(r.valid == 1 && r.tick_count == 4 && r.blit_due == 1 &&
              r.vsync_triggered == 1 && vsync_out == 0,
              "event_loop_t1 4-tick 25Hz blit with vsync");
    }
    {
        DM2_V1_SkprojectEventLoopT1Receipt r;
        int vsync_out = 0, tick_out = 0;
        dm2_v1_skproject_event_loop_t1(3, 0, &vsync_out, &tick_out, &r);
        CHECK(r.valid == 1 && r.tick_count == 3 && r.blit_due == 0,
              "event_loop_t1 3-tick no blit");
    }

    /* ---- wait_for_vsync ---- */
    {
        int vc = 0;
        dm2_v1_skproject_wait_for_vsync(&vc);
        CHECK(vc == 1, "wait_for_vsync increments counter");
        dm2_v1_skproject_wait_for_vsync(&vc);
        CHECK(vc == 2, "wait_for_vsync increments again");
    }

    /* ---- wft ---- */
    {
        DM2_V1_SkprojectWftReceipt r;
        int tick_out = 99;
        dm2_v1_skproject_wft(0, &tick_out, &r);
        CHECK(r.valid == 1 && r.would_block == 1 && tick_out == 0,
              "wft would_block when tick is 0");
    }
    {
        DM2_V1_SkprojectWftReceipt r;
        int tick_out = 99;
        dm2_v1_skproject_wft(5, &tick_out, &r);
        CHECK(r.valid == 1 && r.would_block == 0 && tick_out == 0,
              "wft clears tick when nonzero");
    }

    /* ---- DM2_PROCEED_XACT_65 ---- */
    {
        DM2_V1_SkprojectXact65Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_65(NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_65 blocks without context");
    }

    /* ---- DM2_14cd_2662 ---- */
    {
        DM2_V1_Skproject14cd2662Receipt r;
        int rv = dm2_v1_skproject_14cd_2662(2, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "14cd_2662 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_66 ---- */
    {
        DM2_V1_SkprojectXact66Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_66(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_66 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_67 ---- */
    {
        DM2_V1_SkprojectXact67Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_67(
            NULL, NULL, NULL, NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_67 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_68 ---- */
    {
        DM2_V1_SkprojectXact68Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_68(NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_68 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_69 ---- */
    {
        DM2_V1_SkprojectXact69Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_69(NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_69 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_70 ---- */
    {
        DM2_V1_SkprojectXact70Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_70(NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_70 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_71 ---- */
    {
        DM2_V1_SkprojectXact71Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_71(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_71 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_72_87_88 ---- */
    {
        DM2_V1_SkprojectXact72Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_72_87_88(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_72_87_88 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_72_87_88 value test ---- */
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.v1e0572 = 5;
        DM2_V1_SkprojectXact72Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_72_87_88(&ctx, &r);
        CHECK(r.valid == 1 && rv == 1 && r.out_b1a == 5,
              "xact_72_87_88 assigns v1e0572 to b1a");
    }

    /* ---- DM2_PROCEED_XACT_72_87_88 fallback test ---- */
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.v1e0572 = -1;
        ctx.v1e07d8_w04 = 12;
        DM2_V1_SkprojectXact72Receipt r;
        dm2_v1_skproject_proceed_xact_72_87_88(&ctx, &r);
        CHECK(r.valid == 1 && r.out_b1a == 12,
              "xact_72_87_88 falls back to v1e07d8_w04");
    }

    /* ---- DM2_PROCEED_XACT_73 ---- */
    {
        DM2_V1_SkprojectXact73Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_73(NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_73 blocks without context");
    }

    /* ---- DM2_PROCEED_XACT_73 bit set ---- */
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.v1e0574 = 1;     /* op=set */
        ctx.v1e0572 = 3;     /* bit 3 */
        DM2_V1_SkprojectXact73State st;
        memset(&st, 0, sizeof(st));
        st.creature_word_a = 0;
        DM2_V1_SkprojectXact73Receipt r;
        dm2_v1_skproject_proceed_xact_73(&ctx, &st, &r);
        CHECK(r.valid == 1 && r.out_word_a == 0x8 && r.flag_changed == 1,
              "xact_73 bit set works");
    }

    /* ---- DM2_PROCEED_XACT_74 ---- */
    {
        DM2_V1_SkprojectXact74Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_74(
            NULL, NULL, NULL, NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_74 blocks without context");
    }

    /* ---- DM2_14cd_102e ---- */
    {
        DM2_V1_Skproject102eReceipt r;
        int32_t rv = dm2_v1_skproject_14cd_102e(
            0, 0xfffe, 0xff, 0, 0,
            NULL, NULL, NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_callback == 1 && rv == 0,
              "14cd_102e blocks without callbacks");
    }

    /* ---- source evidence batch 20a ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_PROCEED_XACT_68") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_69") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_70") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_71") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_72_87_88") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_73") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_74") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_102e") != 0,
          "source evidence names cycle-20 batch-20a");

    /* ---- batch 20b: DM2_ai_14cd_10d2 ---- */
    {
        DM2_V1_Skproject14cd10d2Receipt r;
        int rv = dm2_v1_skproject_14cd_10d2(NULL, 0, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "14cd_10d2 blocks without context");
    }
    {
        DM2_V1_Skproject14cd10d2Receipt r;
        uint8_t cache[4][0x20];
        int dirty = 1;
        uint8_t rec[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
        int rv = dm2_v1_skproject_14cd_10d2(rec, 42, cache, &dirty, &r);
        CHECK(rv == 1 && r.claimed_new == 1 && r.slot_index == 0 &&
              dirty == 0, "14cd_10d2 claims first slot after clearing cache");
    }

    /* ---- batch 20b: DM2_PROCEED_XACT_75 ---- */
    {
        DM2_V1_SkprojectXact75Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_75(NULL, NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_75 blocks without context");
    }

    /* ---- batch 20b: DM2_ai_14cd_0f3c ---- */
    {
        DM2_V1_Skproject14cd0f3cReceipt r;
        int rv = dm2_v1_skproject_14cd_0f3c(NULL, NULL, NULL, NULL, 0, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "14cd_0f3c blocks without context");
    }
    {
        DM2_V1_Skproject14cd0f3cInput input;
        DM2_V1_Skproject14cd0f3cState state;
        uint8_t plan[16][22];
        int plan_count = 0;
        DM2_V1_Skproject14cd0f3cReceipt r;
        uint8_t hexe[16] = {0,0,0,0, 0x10,0x20, 0x30,0x40, 50, 7};
        uint8_t rec[4] = {1,2,3,4};

        memset(&input, 0, sizeof(input));
        memset(&state, 0, sizeof(state));
        input.hexe_ptr = hexe;
        input.record_ptr = rec;
        input.eaxl = 0x14;
        input.ecxl = 17;
        input.argb0 = 10;
        input.argl1 = 0xffff;
        input.argb2 = 0;
        input.argb3 = 0;
        state.v1e0571 = 5;
        state.v1e08d6 = 5;
        state.v1e0580 = 0xffff;

        int rv = dm2_v1_skproject_14cd_0f3c(&input, &state, plan,
                                             &plan_count, 16, &r);
        CHECK(rv == 1 && r.entry_added == 1 && r.entry_index == 0 &&
              plan_count == 1, "14cd_0f3c adds plan entry");
    }

    /* ---- batch 20b: DM2_PROCEED_XACT_77 ---- */
    {
        DM2_V1_SkprojectXact77Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_77(NULL, NULL, 0, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_77 blocks without context");
    }

    /* ---- batch 20b: DM2_PROCEED_XACT_78 ---- */
    {
        DM2_V1_SkprojectXact78Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_78(NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_78 blocks without context");
    }

    /* ---- batch 20b: DM2_PROCEED_XACT_79 ---- */
    {
        DM2_V1_SkprojectXact79Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_79(NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_79 blocks without context");
    }

    /* ---- batch 20b: DM2_PROCEED_XACT_80 ---- */
    {
        DM2_V1_SkprojectXact80Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_80(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_80 blocks without context");
    }

    /* ---- batch 20b: DM2_PROCEED_XACT_81 ---- */
    {
        DM2_V1_SkprojectXact81Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_81(NULL, 0, 0, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_81 blocks without context");
    }

    /* ---- source evidence batch 20b ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_ai_14cd_10d2") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_75") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_ai_14cd_0f3c") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_77") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_78") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_79") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_80") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_81") != 0,
          "source evidence names cycle-20 batch-20b");

    /* ---- source evidence batch 19b ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_FILL_ORPHAN_CAII") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "event_loop_T1") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "wait_for_vsync") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "wft") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_65") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_2662") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_66") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_67") != 0,
          "source evidence names cycle-19 batch-19b");

    /* ---- batch 21a: DM2_14cd_3582 ---- */
    {
        uint16_t vals[] = {10, 20, 30};
        DM2_V1_Skproject14cd3582Receipt r;
        int rv = dm2_v1_skproject_14cd_3582(0, 0x100, vals, 3, &r);
        CHECK(rv == 1 && r.valid == 1 && r.total_value == 60,
              "14cd_3582 sums coin values");
    }
    {
        DM2_V1_Skproject14cd3582Receipt r;
        int rv = dm2_v1_skproject_14cd_3582(1, 0x100, NULL, 0, &r);
        CHECK(rv == 1 && r.valid == 1 && r.needs_rebalance == 0,
              "14cd_3582 mode 1 no rebalance");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_82 ---- */
    {
        DM2_V1_SkprojectXact82Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_82(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_82 blocks without context");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        DM2_V1_SkprojectXact82Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_82(&ctx, &r);
        CHECK(rv == 1 && r.valid == 1 && r.out_b1a == 29,
              "xact_82 sets b1a to 29");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_83 ---- */
    {
        DM2_V1_SkprojectXact83Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_83(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_83 blocks without context");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.creature_word_a = 0x80;
        ctx.v1e0572 = 1;
        DM2_V1_SkprojectXact83Receipt r;
        dm2_v1_skproject_proceed_xact_83(&ctx, &r);
        CHECK(r.valid == 1 && r.out_b1a == 0x24 && r.result == -4,
              "xact_83 w0a bit7 + v1e0572=1 gives -4");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        DM2_V1_SkprojectXact83Receipt r;
        dm2_v1_skproject_proceed_xact_83(&ctx, &r);
        CHECK(r.valid == 1 && r.result == -3,
              "xact_83 no flags gives -3");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_84 ---- */
    {
        DM2_V1_SkprojectXact84Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_84(NULL, NULL, NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_84 blocks without context");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.possession = (uint16_t)-2;
        DM2_V1_SkprojectXact84Receipt r;
        dm2_v1_skproject_proceed_xact_84(&ctx, NULL, NULL, &r);
        CHECK(r.valid == 1 && r.has_possession == 0,
              "xact_84 no possession");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.possession = 0x1C00; /* category bits = 7, so 7-5=2 */
        DM2_V1_SkprojectXact84Receipt r;
        dm2_v1_skproject_proceed_xact_84(&ctx, NULL, NULL, &r);
        CHECK(r.valid == 1 && r.has_possession == 1 && r.item_category == 2,
              "xact_84 computes item category");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_85 ---- */
    {
        DM2_V1_SkprojectXact85Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_85(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_85 blocks without context");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        DM2_V1_SkprojectXact85Receipt r;
        dm2_v1_skproject_proceed_xact_85(&ctx, &r);
        CHECK(r.valid == 1 && r.out_b1a == 51 && r.result == -3,
              "xact_85 default path b1a=51");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_86 ---- */
    {
        DM2_V1_SkprojectXact86Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_86(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_86 blocks without context");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.v1e07d8_w04 = 0x12;
        ctx.v1e07d8_w06 = 0x34;
        ctx.v1e0572 = 2;
        DM2_V1_SkprojectXact86Receipt r;
        dm2_v1_skproject_proceed_xact_86(&ctx, &r);
        CHECK(r.valid == 1 && r.out_b20 == 0x12 && r.out_b1e == 0x34 &&
              r.out_b1a == 63 && r.result == -2,
              "xact_86 sets b20/b1e/b1a");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_89 ---- */
    {
        DM2_V1_SkprojectXact89Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_89(NULL, 0x45, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_89 blocks without context");
    }
    {
        DM2_V1_SkprojectXactContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        DM2_V1_SkprojectXact89Receipt r;
        dm2_v1_skproject_proceed_xact_89(&ctx, 0x45, &r);
        CHECK(r.valid == 1 && r.command_byte == 0xC5,
              "xact_89 command_byte = w06|0x80");
    }

    /* ---- batch 21a: DM2_PROCEED_XACT_90 ---- */
    {
        DM2_V1_SkprojectXact90Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_90(50, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1 && r.result == -3,
              "xact_90 no rand_fn gives -3");
    }
    {
        DM2_V1_SkprojectXact90Receipt r;
        dm2_v1_skproject_proceed_xact_90(50, stub_rand16, NULL, &r);
        CHECK(r.valid == 1 && (r.result == -2 || r.result == -3),
              "xact_90 with rand returns -2 or -3");
    }

    /* ---- source evidence batch 21a ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_3582") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_82") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_83") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_84") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_85") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_86") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_89") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_PROCEED_XACT_90") != 0,
          "source evidence names cycle-21 batch-21a");

    /* ---- batch 21b: DM2_PROCEED_XACT_91 ---- */
    {
        DM2_V1_SkprojectXact91Receipt r;
        int rv = dm2_v1_skproject_proceed_xact_91(NULL, &r);
        CHECK(r.blocked_missing_context == 1 && rv == 0,
              "xact_91 blocks without context");
    }

    /* ---- batch 21b: DM2_PROCEED_XACT classify ---- */
    {
        DM2_V1_SkprojectProceedXactReceipt r;
        int rv;
        rv = dm2_v1_skproject_proceed_xact_classify(63, &r);
        CHECK(rv == 1 && r.dispatched == 1 && r.opt == 0 && r.result == -2,
              "proceed_xact opt=0 dispatches");
        rv = dm2_v1_skproject_proceed_xact_classify(98, &r);
        CHECK(rv == 1 && r.dispatched == 1 && r.opt == 35,
              "proceed_xact opt=35 dispatches");
        rv = dm2_v1_skproject_proceed_xact_classify(62, &r);
        CHECK(rv == 0 && r.dispatched == 0,
              "proceed_xact opt=-1 out of range");
        rv = dm2_v1_skproject_proceed_xact_classify(99, &r);
        CHECK(rv == 0 && r.dispatched == 0,
              "proceed_xact opt=36 out of range");
    }

    /* ---- batch 21b: DM2_13e4_01a3 classify ---- */
    {
        DM2_V1_Skproject13e401a3Receipt r;
        int rv;
        rv = dm2_v1_skproject_13e4_01a3_classify(1, NULL, 0xffff, &r);
        CHECK(rv == 0 && r.blocked_already_init == 1,
              "13e4_01a3 blocks when already init");
        {
            uint8_t spec[0x18];
            memset(spec, 0, sizeof(spec));
            spec[0xa] = 0x34; spec[0xb] = 0x12;
            spec[0xe] = 0x78; spec[0xf] = 0x56;
            rv = dm2_v1_skproject_13e4_01a3_classify(0, spec, 0xffff, &r);
            CHECK(rv == 1 && r.valid == 1 && r.v1e0576 == 0x1234 &&
                  r.v1e0578 == 0x5678,
                  "13e4_01a3 extracts v1e0552 fields");
        }
    }

    /* ---- batch 21b: DM2_14cd_062e classify ---- */
    {
        DM2_V1_Skproject14cd062eReceipt r;
        uint8_t creature[0x14];
        int rv;
        memset(creature, 0, sizeof(creature));
        creature[0x12] = 0xff;
        rv = dm2_v1_skproject_14cd_062e_classify(creature, 5, 5, &r);
        CHECK(rv == 1 && r.has_table_entry == 0 && r.result == 0,
              "14cd_062e no table entry when b12=0xff");
    }

    /* ---- batch 21b: DM2_14cd_18cc classify ---- */
    {
        DM2_V1_Skproject14cd18ccReceipt r;
        dm2_v1_skproject_14cd_18cc_classify(0x42, 0x13, &r);
        CHECK(r.valid == 1 && r.parb03 == 0x42 && r.parb02 == 0x13,
              "14cd_18cc byte swap classify");
    }

    /* ---- batch 21b: DM2_2c1d_09d9 compute ---- */
    {
        DM2_V1_Skproject2c1d09d9Receipt r;
        uint16_t skills[2][4] = {
            {100, 100, 100, 100},
            {50, 50, 50, 50}
        };
        int rv = dm2_v1_skproject_2c1d_09d9_compute(2, skills, 2, &r);
        CHECK(rv == 1 && r.skill_sum == 600 && r.result == 2,
              "2c1d_09d9 sum=600 shifts once to get result=2");
        {
            uint16_t z[1][4] = {{0, 0, 0, 0}};
            rv = dm2_v1_skproject_2c1d_09d9_compute(1, z, 1, &r);
            CHECK(rv == 1 && r.skill_sum == 0 && r.result == 1,
                  "2c1d_09d9 sum=0 result=1");
        }
    }

    /* ---- batch 21b: DM2_14cd_1316 classify ---- */
    {
        DM2_V1_Skproject14cd1316Receipt r;
        int rv;
        rv = dm2_v1_skproject_14cd_1316_classify(0x40 | 5, 0, 3, 3, &r);
        CHECK(rv == 1 && r.has_0x40_gate == 1 && r.gate_matched == 1 &&
              r.result == 1,
              "14cd_1316 0x40 gate match returns 1");
        rv = dm2_v1_skproject_14cd_1316_classify(23, 0, 0, 0, &r);
        CHECK(rv == 1 && r.condition == 23 && r.result == 0,
              "14cd_1316 condition 23 out of range");
        rv = dm2_v1_skproject_14cd_1316_classify(0x80 | 5, 0, 0, 0, &r);
        CHECK(rv == 1 && r.inverted == 1 && r.condition == 5,
              "14cd_1316 0x80 inversion flag");
    }

    /* ---- batch 21b: DM2_14cd_18f2 classify ---- */
    {
        DM2_V1_Skproject14cd18f2Receipt r;
        int rv;
        rv = dm2_v1_skproject_14cd_18f2_classify(5, 0, NULL, 0, 0xffff, &r);
        CHECK(rv == 0 && r.blocked_null_ptr == 1,
              "14cd_18f2 blocks on null pointer");
        {
            uint8_t table[14];
            memset(table, 0, sizeof(table));
            table[0xc] = 5;
            rv = dm2_v1_skproject_14cd_18f2_classify(5, 0, table, 0, 0xffff, &r);
            CHECK(rv == 1 && r.entries_visited == 1 && r.entries_matched == 1 &&
                  r.action_byte == 5,
                  "14cd_18f2 single entry match");
        }
    }

    /* ---- source evidence batch 21b ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_PROCEED_XACT_91") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_13e4_01a3") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_062e") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_18cc") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_2c1d_09d9") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_1316") != 0 &&
              strstr(dm2_v1_skproject_core_source_evidence(),
                     "DM2_14cd_18f2") != 0,
          "source evidence names cycle-21 batch-21b");

    /* ---- cycle-22 batch-22a: 14cd_19a4 through 14cd_1bac ---- */

    /* 14cd_19a4: sign-extend wrapper to 18f2 */
    {
        DM2_V1_Skproject14cd19a4Receipt r19a4;
        uint8_t tbl[14];
        memset(tbl, 0, sizeof(tbl));

        CHECK(dm2_v1_skproject_14cd_19a4_classify(3, -2, tbl, &r19a4) == 1,
              "19a4 valid with table");
        CHECK(r19a4.valid == 1, "19a4 receipt valid");
        CHECK(r19a4.eaxb_extended == 3, "19a4 eaxb preserved");
        CHECK(r19a4.edxb_extended == -2, "19a4 edxb preserved");
        CHECK(dm2_v1_skproject_14cd_19a4_classify(0, 0, NULL, &r19a4) == 0,
              "19a4 blocked on NULL");
    }

    /* 14cd_19c2: guarded delegate */
    {
        DM2_V1_Skproject14cd19c2Receipt r19c2;
        uint8_t tbl[14];
        uint8_t lkup[8];
        memset(tbl, 0, sizeof(tbl));
        memset(lkup, 0, sizeof(lkup));
        lkup[5] = 0;

        CHECK(dm2_v1_skproject_14cd_19c2_classify(0, NULL, 0, 2, 1, 1, 1, lkup, &r19c2) == 0,
              "19c2 blocked on NULL ptr");
        CHECK(r19c2.blocked_null_ptr == 1, "19c2 null_ptr flag");

        CHECK(dm2_v1_skproject_14cd_19c2_classify(0, tbl, 0, 2, 1, 0, 1, lkup, &r19c2) == 0,
              "19c2 blocked on v1e058d=0");
        CHECK(r19c2.blocked_no_readiness == 1, "19c2 readiness flag");

        CHECK(dm2_v1_skproject_14cd_19c2_classify(0, tbl, 0, 2, 1, 1, 0, lkup, &r19c2) == 0,
              "19c2 blocked on v1e0578=0");
        CHECK(r19c2.blocked_no_v1e0578 == 1, "19c2 v1e0578 flag");
        CHECK(r19c2.byte5_lte_zero == 1, "19c2 byte5<=0 detected");

        CHECK(dm2_v1_skproject_14cd_19c2_classify(1, tbl, 5, 3, 1, 1, 4, lkup, &r19c2) == 1,
              "19c2 valid with negation");
        CHECK(r19c2.negation_flag == 1, "19c2 negation on");
        CHECK(r19c2.ecxb_delegated == -3, "19c2 ecxb negated");
        CHECK(r19c2.edxb_delegated == 5, "19c2 edxb passed through");

        CHECK(dm2_v1_skproject_14cd_19c2_classify(0, tbl, 5, 3, 1, 1, 4, lkup, &r19c2) == 1,
              "19c2 valid without negation");
        CHECK(r19c2.negation_flag == 0, "19c2 negation off");
        CHECK(r19c2.ecxb_delegated == 3, "19c2 ecxb unchanged");
    }

    /* 14cd_1a3c: wrapper to 19c2(ecxl=2, argb0=1) */
    {
        DM2_V1_Skproject14cd1a3cReceipt r1a3c;
        uint8_t tbl[14];
        memset(tbl, 0, sizeof(tbl));

        CHECK(dm2_v1_skproject_14cd_1a3c_classify(7, -1, tbl, &r1a3c) == 1,
              "1a3c valid");
        CHECK(r1a3c.eaxb_extended == 7, "1a3c eaxb");
        CHECK(r1a3c.edxb_extended == -1, "1a3c edxb");
        CHECK(dm2_v1_skproject_14cd_1a3c_classify(0, 0, NULL, &r1a3c) == 0,
              "1a3c blocked on NULL");
    }

    /* 14cd_1a5a: wrapper to 19c2(ecxl=4, argb0=3) */
    {
        DM2_V1_Skproject14cd1a5aReceipt r1a5a;
        uint8_t tbl[14];
        memset(tbl, 0, sizeof(tbl));

        CHECK(dm2_v1_skproject_14cd_1a5a_classify(2, 3, tbl, &r1a5a) == 1,
              "1a5a valid");
        CHECK(r1a5a.eaxb_extended == 2, "1a5a eaxb");
        CHECK(dm2_v1_skproject_14cd_1a5a_classify(0, 0, NULL, &r1a5a) == 0,
              "1a5a blocked on NULL");
    }

    /* 14cd_1a78: table walker */
    {
        DM2_V1_Skproject14cd1a78Receipt r1a78;
        uint8_t tbl[28]; /* 2 entries */
        uint8_t lkup[16];
        memset(tbl, 0, sizeof(tbl));
        memset(lkup, 0, sizeof(lkup));

        CHECK(dm2_v1_skproject_14cd_1a78_classify(0, 0, NULL, 1, lkup, &r1a78) == 0,
              "1a78 blocked on NULL ptr");
        CHECK(r1a78.blocked_null_ptr == 1, "1a78 null_ptr flag");

        lkup[7] = 0;
        CHECK(dm2_v1_skproject_14cd_1a78_classify(0, 0, tbl, 1, lkup, &r1a78) == 0,
              "1a78 blocked on byte7=0");
        CHECK(r1a78.blocked_byte7_zero == 1, "1a78 byte7 flag");

        lkup[7] = 2;
        /* Set up entry 0: byte@0xc=1, word@4=0x0001, byte@0xd=1 (continue) */
        tbl[0xc] = 1;
        tbl[4] = 0x01; tbl[5] = 0x00;
        tbl[0xd] = 1;
        /* Entry 1: byte@0xc=1, word@4=0xffff, byte@0xd=0 (stop) */
        tbl[14 + 0xc] = 1;
        tbl[14 + 4] = 0xff; tbl[14 + 5] = 0xff;
        tbl[14 + 0xd] = 0;

        CHECK(dm2_v1_skproject_14cd_1a78_classify(0, 0, tbl, 1, lkup, &r1a78) == 1,
              "1a78 valid walk");
        CHECK(r1a78.entries_visited == 2, "1a78 visited 2");
        CHECK(r1a78.entries_matched == 1, "1a78 matched 1 (w4!=ffff)");
        CHECK(r1a78.entries_delegated == 1, "1a78 delegated 1");
    }

    /* 14cd_1b74: wrapper to 1a78(ecxl=1) */
    {
        DM2_V1_Skproject14cd1b74Receipt r1b74;
        uint8_t tbl[14];
        memset(tbl, 0, sizeof(tbl));

        CHECK(dm2_v1_skproject_14cd_1b74_classify(5, -3, tbl, NULL, &r1b74) == 1,
              "1b74 valid");
        CHECK(r1b74.eaxb_extended == 5, "1b74 eaxb");
        CHECK(dm2_v1_skproject_14cd_1b74_classify(0, 0, NULL, NULL, &r1b74) == 0,
              "1b74 blocked on NULL");
    }

    /* 14cd_1b90: wrapper to 1a78(ecxl=3) */
    {
        DM2_V1_Skproject14cd1b90Receipt r1b90;
        uint8_t tbl[14];
        memset(tbl, 0, sizeof(tbl));

        CHECK(dm2_v1_skproject_14cd_1b90_classify(4, 2, tbl, NULL, &r1b90) == 1,
              "1b90 valid");
        CHECK(r1b90.eaxb_extended == 4, "1b90 eaxb");
        CHECK(dm2_v1_skproject_14cd_1b90_classify(0, 0, NULL, NULL, &r1b90) == 0,
              "1b90 blocked on NULL");
    }

    /* 14cd_1bac: like 19c2 but checks v1e0578&8 */
    {
        DM2_V1_Skproject14cd1bacReceipt r1bac;
        uint8_t tbl[14];
        uint8_t lkup[8];
        memset(tbl, 0, sizeof(tbl));
        memset(lkup, 0, sizeof(lkup));
        lkup[5] = 0;

        CHECK(dm2_v1_skproject_14cd_1bac_classify(0, 0, NULL, 2, 1, 1, 1, lkup, &r1bac) == 0,
              "1bac blocked on NULL");
        CHECK(r1bac.blocked_null_ptr == 1, "1bac null_ptr flag");

        CHECK(dm2_v1_skproject_14cd_1bac_classify(0, 0, tbl, 2, 1, 0, 1, lkup, &r1bac) == 0,
              "1bac blocked on readiness");

        /* v1e0578=8 => bit3 set, byte5<=0 detected */
        CHECK(dm2_v1_skproject_14cd_1bac_classify(1, 5, tbl, 3, 1, 1, 0x8, lkup, &r1bac) == 1,
              "1bac valid with bit3");
        CHECK(r1bac.v1e0578_bit3_set == 1, "1bac bit3 set");
        CHECK(r1bac.byte5_lte_zero == 1, "1bac byte5<=0");
        CHECK(r1bac.negation_flag == 1, "1bac negation on");
        CHECK(r1bac.ecxb_delegated == -3, "1bac ecxb negated");

        /* v1e0578=4 => bit3 not set, byte5 not checked */
        CHECK(dm2_v1_skproject_14cd_1bac_classify(0, 5, tbl, 3, 1, 1, 0x4, lkup, &r1bac) == 1,
              "1bac valid without bit3");
        CHECK(r1bac.v1e0578_bit3_set == 0, "1bac bit3 clear");
        CHECK(r1bac.byte5_lte_zero == 0, "1bac byte5 not checked");
        CHECK(r1bac.negation_flag == 0, "1bac negation off");
        CHECK(r1bac.ecxb_delegated == 3, "1bac ecxb unchanged");
    }

    /* source evidence: cycle-22 batch-22a */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_19a4") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_19c2") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_1a3c") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_1a5a") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_1a78") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_1b74") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_1b90") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_14cd_1bac") != 0,
          "source evidence names cycle-22 batch-22a");

    /* ---- batch 22b: DM2_14cd_1c27 ---- */
    {
        DM2_V1_Skproject14cd1c27Receipt r;
        int rv = dm2_v1_skproject_14cd_1c27_classify(0x83, 0x7f, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c27 valid");
        CHECK(r.sign_ext_eax == -125, "14cd_1c27 sign-ext eax 0x83 => -125");
        CHECK(r.sign_ext_edx == 127, "14cd_1c27 sign-ext edx 0x7f => 127");
        CHECK(r.ecxl == 2, "14cd_1c27 ecxl == 2");
        CHECK(r.argb0 == 1, "14cd_1c27 argb0 == 1");
    }

    /* ---- batch 22b: DM2_14cd_1c45 ---- */
    {
        DM2_V1_Skproject14cd1c45Receipt r;
        int rv = dm2_v1_skproject_14cd_1c45_classify(0xfe, 0x02, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c45 valid");
        CHECK(r.sign_ext_eax == -2, "14cd_1c45 sign-ext eax 0xfe => -2");
        CHECK(r.sign_ext_edx == 2, "14cd_1c45 sign-ext edx 0x02 => 2");
        CHECK(r.ecxl == 4, "14cd_1c45 ecxl == 4");
        CHECK(r.argb0 == 3, "14cd_1c45 argb0 == 3");
    }

    /* ---- batch 22b: DM2_14cd_1c63 (b_03 == 0xd) ---- */
    {
        DM2_V1_Skproject14cd1c63Receipt r;
        int rv = dm2_v1_skproject_14cd_1c63_classify(0x05, 0x0d, 0x1234, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c63 valid b03=0xd");
        CHECK(r.b03_is_0d == 1, "14cd_1c63 b03 matches 0xd");
        CHECK(r.argw0 == 0x1234, "14cd_1c63 argw0 from w_08");
        CHECK(r.eaxb == 5, "14cd_1c63 eaxb == 5");
    }

    /* ---- batch 22b: DM2_14cd_1c63 (b_03 != 0xd) ---- */
    {
        DM2_V1_Skproject14cd1c63Receipt r;
        int rv = dm2_v1_skproject_14cd_1c63_classify(0x80, 0x0a, 0x1234, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c63 valid b03!=0xd");
        CHECK(r.b03_is_0d == 0, "14cd_1c63 b03 does not match");
        CHECK(r.argw0 == 0xffff, "14cd_1c63 argw0 fallback 0xffff");
    }

    /* ---- batch 22b: DM2_14cd_1c8d (all match => skipped) ---- */
    {
        DM2_V1_Skproject14cd1c8dReceipt r;
        int rv = dm2_v1_skproject_14cd_1c8d_classify(
            0x01, 0x05, 0x08e3, 3, 7, 2, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c8d valid all-match");
        CHECK(r.x_match == 1, "14cd_1c8d x match");
        CHECK(r.y_match == 1, "14cd_1c8d y match");
        CHECK(r.map_match == 1, "14cd_1c8d map match");
        CHECK(r.skipped == 1, "14cd_1c8d skipped (all match)");
    }

    /* ---- batch 22b: DM2_14cd_1c8d (x mismatch => delegates) ---- */
    {
        DM2_V1_Skproject14cd1c8dReceipt r;
        int rv = dm2_v1_skproject_14cd_1c8d_classify(
            0x01, 0x05, 0x08e3, 4, 7, 2, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c8d valid x-mismatch");
        CHECK(r.x_match == 0, "14cd_1c8d x mismatch");
        CHECK(r.skipped == 0, "14cd_1c8d not skipped");
        CHECK(r.eaxb == 6, "14cd_1c8d eaxb == 6");
    }

    /* ---- batch 22b: DM2_14cd_1c8d (eax zero => delegates) ---- */
    {
        DM2_V1_Skproject14cd1c8dReceipt r;
        int rv = dm2_v1_skproject_14cd_1c8d_classify(
            0x00, 0x05, 0x08e3, 3, 7, 2, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1c8d valid eax-zero");
        CHECK(r.blocked_eax_zero == 1, "14cd_1c8d eax zero");
        CHECK(r.skipped == 0, "14cd_1c8d not skipped when eax zero");
        CHECK(r.eaxb == 6, "14cd_1c8d eaxb == 6 on eax-zero path");
    }

    /* ---- batch 22b: DM2_14cd_1cec (no callbacks) ---- */
    {
        DM2_V1_Skproject14cd1cecReceipt r;
        int rv = dm2_v1_skproject_14cd_1cec_classify(
            0x05, 0x1234, NULL, NULL, NULL, &r);
        CHECK(rv == 0, "14cd_1cec blocked no callbacks");
    }

    /* ---- batch 22b: DM2_14cd_1d42 (b_03 == 5) ---- */
    {
        DM2_V1_Skproject14cd1d42Receipt r;
        int rv = dm2_v1_skproject_14cd_1d42_classify(0x03, 0x05, 0xabcd, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1d42 valid b03=5");
        CHECK(r.b03_is_05 == 1, "14cd_1d42 b03 matches 5");
        CHECK(r.argw0 == 0xabcd, "14cd_1d42 argw0 from w_08");
        CHECK(r.eaxb == 0x12, "14cd_1d42 eaxb == 0x12");
    }

    /* ---- batch 22b: DM2_14cd_1d42 (b_03 != 5) ---- */
    {
        DM2_V1_Skproject14cd1d42Receipt r;
        int rv = dm2_v1_skproject_14cd_1d42_classify(0x03, 0x07, 0xabcd, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1d42 valid b03!=5");
        CHECK(r.b03_is_05 == 0, "14cd_1d42 b03 does not match");
        CHECK(r.argw0 == 0xffff, "14cd_1d42 argw0 fallback");
    }

    /* ---- batch 22b: DM2_14cd_1d6c (null ptr) ---- */
    {
        DM2_V1_Skproject14cd1d6cReceipt r;
        int rv = dm2_v1_skproject_14cd_1d6c_classify(
            0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1d6c null ptr");
        CHECK(r.blocked_null_ptr == 1, "14cd_1d6c blocked null");
    }

    /* ---- batch 22b: DM2_14cd_1d6c (single entry, no match) ---- */
    {
        uint8_t entry[14] = {0};
        entry[0x0c] = 0x03;
        entry[0x0d] = 0x00;

        DM2_V1_Skproject14cd1d6cReceipt r;
        int rv = dm2_v1_skproject_14cd_1d6c_classify(
            0, 0, entry, 0x0f, 0, NULL, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1d6c single no match");
        CHECK(r.entries_visited == 1, "14cd_1d6c visited 1");
        CHECK(r.entries_matched == 0, "14cd_1d6c matched 0");
    }

    /* ---- batch 22b: DM2_14cd_1e36 ---- */
    {
        DM2_V1_Skproject14cd1e36Receipt r;
        int rv = dm2_v1_skproject_14cd_1e36_classify(0x01, 0xff, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1e36 valid");
        CHECK(r.sign_ext_eax == 1, "14cd_1e36 sign-ext eax");
        CHECK(r.sign_ext_edx == -1, "14cd_1e36 sign-ext edx 0xff");
        CHECK(r.ecxl == 0x0f, "14cd_1e36 ecxl == 0x0f");
    }

    /* ---- batch 22b: source evidence ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1c27") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1c45") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1c63") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1c8d") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1cec") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1d42") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1d6c") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1e36") != 0,
          "source evidence names cycle-22 batch-22b");

    /* ==== batch 23a: DM2_14cd_1e52 ==== */
    {
        DM2_V1_Skproject14cd1e52Receipt r;
        int rv = dm2_v1_skproject_14cd_1e52_classify(0x83, 0x7f, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1e52 valid");
        CHECK(r.sign_ext_eax == -125, "14cd_1e52 sign-ext eax 0x83 => -125");
        CHECK(r.sign_ext_edx == 127, "14cd_1e52 sign-ext edx 0x7f => 127");
        CHECK(r.ecxl == 0x10, "14cd_1e52 ecxl == 0x10");
    }

    /* ==== batch 23a: DM2_3DC4C (bit5 not set => returns 1) ==== */
    {
        DM2_V1_Skproject3DC4CReceipt r;
        /* Provide callbacks that return known values */
        int rv = dm2_v1_skproject_3dc4c_classify(
            0, 5, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "3dc4c valid no callbacks");
        /* With no callbacks, table_word=0, gdat_result=0, bit5=0 => return 1 */
        CHECK(r.return_value == 1, "3dc4c returns 1 when bit5 not set");
        CHECK(r.bit5_set == 0, "3dc4c bit5 not set");
    }

    /* ==== batch 23a: DM2_14cd_1e6e (dc4c returns 0 => clear bit7) ==== */
    {
        DM2_V1_Skproject14cd1e6eReceipt r;
        int rv = dm2_v1_skproject_14cd_1e6e_classify(
            0x01, 0x05, 0, 0x0080, NULL, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1e6e valid");
        /* No callbacks => dc4c gets gdat_result=0 => bit5 not set => dc4c returns 1.
           But with no read_table/query_gdat callbacks, gdat_result stays 0,
           so dc4c_result=1. eaxl=1 => nonzero path. No rand_fn => rand=0,
           rand & 0x1f == 0 => clear bit7, delegate. */
        CHECK(r.eaxl_nonzero_path == 1, "14cd_1e6e eaxl nonzero path");
        CHECK(r.delegated == 1, "14cd_1e6e delegated");
        CHECK(r.clear_bit7 == 1, "14cd_1e6e clear bit7");
    }

    /* ==== batch 23a: DM2_14cd_1e6e (eaxl=0, bit7 set => clear) ==== */
    {
        DM2_V1_Skproject14cd1e6eReceipt r;
        int rv = dm2_v1_skproject_14cd_1e6e_classify(
            0x00, 0x05, 0, 0x0080, NULL, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1e6e valid eax0 bit7set");
        CHECK(r.eaxl_nonzero_path == 0, "14cd_1e6e eax zero path");
        CHECK(r.bit7_state == 1, "14cd_1e6e bit7 was set");
        CHECK(r.clear_bit7 == 1, "14cd_1e6e clears bit7 on bit7-set path");
    }

    /* ==== batch 23a: DM2_14cd_1eec (null ptr) ==== */
    {
        DM2_V1_Skproject14cd1eecReceipt r;
        int rv = dm2_v1_skproject_14cd_1eec_classify(
            0, 0, NULL, 0, 0, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1eec null ptr");
        CHECK(r.blocked_null_ptr == 1, "14cd_1eec blocked null");
    }

    /* ==== batch 23a: DM2_14cd_1eec (single entry, no match) ==== */
    {
        uint8_t entry[14];
        memset(entry, 0, sizeof(entry));
        entry[0x0c] = 0x03;
        entry[0x0d] = 0x00; /* sentinel */

        DM2_V1_Skproject14cd1eecReceipt r;
        int rv = dm2_v1_skproject_14cd_1eec_classify(
            0, 0, entry, 0x15, 0, NULL, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1eec single no match");
        CHECK(r.entries_visited == 1, "14cd_1eec visited 1");
        CHECK(r.entries_matched == 0, "14cd_1eec matched 0");
    }

    /* ==== batch 23a: DM2_14cd_1f8b ==== */
    {
        DM2_V1_Skproject14cd1f8bReceipt r;
        int rv = dm2_v1_skproject_14cd_1f8b_classify(0xfe, 0x02, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1f8b valid");
        CHECK(r.sign_ext_eax == -2, "14cd_1f8b sign-ext eax 0xfe => -2");
        CHECK(r.sign_ext_edx == 2, "14cd_1f8b sign-ext edx 0x02 => 2");
        CHECK(r.ecxl == 0x15, "14cd_1f8b ecxl == 0x15");
    }

    /* ==== batch 23a: DM2_14cd_1fa7 ==== */
    {
        DM2_V1_Skproject14cd1fa7Receipt r;
        /* v1e08d8=0x13, v1e08d4=0x0a, v1e08d6=0x2f */
        int rv = dm2_v1_skproject_14cd_1fa7_classify(
            0x05, 0x0013, 0x000a, 0x002f, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_1fa7 valid");
        /* packed = (0x13 & 0x1f) | ((0x0a & 0x1f) << 5) | ((0x2f & 0x3f) << 10)
           = 0x13 | (0x0a << 5) | (0x2f << 10)
           = 0x13 | 0x140 | 0xbc00
           = 0xbd53 */
        CHECK(r.packed_word == 0xbd53, "14cd_1fa7 packed word");
        CHECK(r.sign_ext_edx == 5, "14cd_1fa7 sign-ext edx");
    }

    /* ==== batch 23a: DM2_14cd_0f0a (case 13) ==== */
    {
        DM2_V1_Skproject14cd0f0aReceipt r;
        int rv = dm2_v1_skproject_14cd_0f0a_classify(0x0d, 0x05, 0x03, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_0f0a valid case 13");
        CHECK(r.sub_index == 0x0d, "14cd_0f0a sub_index 13");
        CHECK(r.dispatched == 1, "14cd_0f0a dispatched");
        CHECK(r.case_taken == 13, "14cd_0f0a case 13");
    }

    /* ==== batch 23a: DM2_14cd_0f0a (case 16) ==== */
    {
        DM2_V1_Skproject14cd0f0aReceipt r;
        int rv = dm2_v1_skproject_14cd_0f0a_classify(0x10, 0, 0, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_0f0a valid case 16");
        CHECK(r.case_taken == 16, "14cd_0f0a case 16");
    }

    /* ==== batch 23a: DM2_14cd_0f0a (out of range) ==== */
    {
        DM2_V1_Skproject14cd0f0aReceipt r;
        int rv = dm2_v1_skproject_14cd_0f0a_classify(0x11, 0, 0, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_0f0a valid out-of-range");
        CHECK(r.dispatched == 0, "14cd_0f0a not dispatched");
        CHECK(r.case_taken == -1, "14cd_0f0a case -1 default");
    }

    /* ==== batch 23a: DM2_14cd_0389 (blocked b00) ==== */
    {
        DM2_V1_Skproject14cd0389Receipt r;
        uint8_t creature[0x20];
        memset(creature, 0, sizeof(creature));
        int rv = dm2_v1_skproject_14cd_0389_classify(
            0, 1, 0, creature, NULL, NULL, &r);
        CHECK(rv == 0, "14cd_0389 blocked b00");
        CHECK(r.blocked_b00 == 1, "14cd_0389 b00 flag");
    }

    /* ==== batch 23a: DM2_14cd_0389 (blocked b03 == -1) ==== */
    {
        DM2_V1_Skproject14cd0389Receipt r;
        uint8_t creature[0x20];
        memset(creature, 0, sizeof(creature));
        int rv = dm2_v1_skproject_14cd_0389_classify(
            1, 1, (int32_t)0xffffffff, creature, NULL, NULL, &r);
        CHECK(rv == 0, "14cd_0389 blocked b03");
        CHECK(r.blocked_b03 == 1, "14cd_0389 b03 flag");
    }

    /* ==== batch 23a: DM2_14cd_0389 (blocked b12 == 0xff) ==== */
    {
        DM2_V1_Skproject14cd0389Receipt r;
        uint8_t creature[0x20];
        memset(creature, 0, sizeof(creature));
        creature[0x12] = 0xff;
        int rv = dm2_v1_skproject_14cd_0389_classify(
            1, 1, 0, creature, NULL, NULL, &r);
        CHECK(rv == 0, "14cd_0389 blocked b12 ff");
        CHECK(r.blocked_b12_ff == 1, "14cd_0389 b12 ff flag");
    }

    /* ==== batch 23a: DM2_14cd_0389 (valid, reads b12/b13) ==== */
    {
        DM2_V1_Skproject14cd0389Receipt r;
        uint8_t creature[0x20];
        memset(creature, 0, sizeof(creature));
        creature[0x12] = 0x03;
        creature[0x13] = 0x02;
        int rv = dm2_v1_skproject_14cd_0389_classify(
            1, 1, 0, creature, NULL, NULL, &r);
        CHECK(rv == 1 && r.valid == 1, "14cd_0389 valid");
        CHECK(r.creature_b12 == 0x03, "14cd_0389 b12");
        CHECK(r.creature_b13 == 0x02, "14cd_0389 b13");
    }

    /* ---- batch 23a: source evidence ---- */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1e52") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_3DC4C") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1e6e") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1eec") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1f8b") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_1fa7") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0f0a") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0389") != 0,
          "source evidence names cycle-23 batch-23a");

    /* ---- batch 23b: 0457, 0550, 0276, 0684, 08f5, DECIDE_NEXT_XACT, 0067, SELECT_CREATURE_37FC ---- */

    /* 0457 null receipt */
    {
        int32_t cnt = 3;
        int8_t ents[66];
        memset(ents, 0, sizeof(ents));
        CHECK(dm2_v1_skproject_0457_classify(ents, &cnt, 4, NULL, NULL, NULL) == 0,
              "0457 null receipt blocked");
    }
    /* 0457 null entries */
    {
        DM2_V1_Skproject0457Receipt r;
        int32_t cnt = 3;
        CHECK(dm2_v1_skproject_0457_classify(NULL, &cnt, 4, NULL, NULL, &r) == 0,
              "0457 null entries blocked");
    }
    /* 0457 zero count */
    {
        DM2_V1_Skproject0457Receipt r;
        int32_t cnt = 0;
        int8_t ents[22]; memset(ents, 0, sizeof(ents));
        CHECK(dm2_v1_skproject_0457_classify(ents, &cnt, 4, NULL, NULL, &r) == 1 &&
              r.initial_count == 0 && r.final_count == 0,
              "0457 zero count");
    }

    /* 0550 null receipt */
    {
        int8_t tbl[7] = {0};
        CHECK(dm2_v1_skproject_0550_classify(tbl, 1, 0, 0, 0, NULL, NULL, NULL, NULL) == 0,
              "0550 null receipt blocked");
    }
    /* 0550 null table */
    {
        DM2_V1_Skproject0550Receipt r;
        CHECK(dm2_v1_skproject_0550_classify(NULL, 1, 0, 0, 0, NULL, NULL, NULL, &r) == 0,
              "0550 null table blocked");
    }
    /* 0550 exact match */
    {
        DM2_V1_Skproject0550Receipt r;
        int8_t tbl[7]; memset(tbl, 0, sizeof(tbl));
        tbl[0] = 5;
        CHECK(dm2_v1_skproject_0550_classify(tbl, 5, 0, 0, 0, NULL, NULL, NULL, &r) == 1 &&
              r.exact_match == 1 && r.entries_visited == 1,
              "0550 exact match first entry");
    }

    /* 0276 null receipt */
    {
        int8_t inp[0x16]; memset(inp, 0, sizeof(inp));
        CHECK(dm2_v1_skproject_0276_classify(inp, 0, NULL, NULL) == 0,
              "0276 null receipt blocked");
    }
    /* 0276 null input */
    {
        DM2_V1_Skproject0276Receipt r;
        CHECK(dm2_v1_skproject_0276_classify(NULL, 0, NULL, &r) == 0,
              "0276 null input blocked");
    }

    /* 0684 null receipt */
    {
        int8_t cr[32] = {0}; int8_t tbl[8] = {0};
        CHECK(dm2_v1_skproject_0684_classify(cr, 0, tbl, NULL, NULL, NULL, NULL) == 0,
              "0684 null receipt blocked");
    }

    /* 08f5 null receipt */
    {
        int8_t cr[32] = {0}; int8_t tbl[64] = {0};
        CHECK(dm2_v1_skproject_08f5_classify(0, cr, tbl, NULL) == 0,
              "08f5 null receipt blocked");
    }
    /* 08f5 null creatures */
    {
        DM2_V1_Skproject08f5Receipt r;
        int8_t tbl[64] = {0};
        CHECK(dm2_v1_skproject_08f5_classify(0, NULL, tbl, &r) == 0,
              "08f5 null creatures blocked");
    }
    /* 08f5 basic lookup */
    {
        DM2_V1_Skproject08f5Receipt r;
        int8_t cr[32]; memset(cr, 0, sizeof(cr));
        cr[0x12] = 0; cr[0x13] = 0;
        int8_t tbl[64]; memset(tbl, 0, sizeof(tbl));
        tbl[2] = 5;
        CHECK(dm2_v1_skproject_08f5_classify(0, cr, tbl, &r) == 1 &&
              r.looked_up_byte == 5 && r.reset_to_ff == 0,
              "08f5 basic lookup col2");
    }
    /* 08f5 0xFE reset */
    {
        DM2_V1_Skproject08f5Receipt r;
        int8_t cr[32]; memset(cr, 0, sizeof(cr));
        int8_t tbl[64]; memset(tbl, 0, sizeof(tbl));
        tbl[2] = (int8_t)0xFE;
        CHECK(dm2_v1_skproject_08f5_classify(0, cr, tbl, &r) == 1 &&
              r.reset_to_ff == 1 && r.advance_result == 1,
              "08f5 reset on 0xFE");
    }

    /* decide_next_xact null receipt */
    {
        int8_t cr[32] = {0}; int8_t tbl[64] = {0};
        CHECK(dm2_v1_skproject_decide_next_xact_classify(0, cr, tbl, NULL) == 0,
              "decide_next_xact null receipt blocked");
    }
    /* decide_next_xact basic */
    {
        DM2_V1_SkprojectDecideNextXactReceipt r;
        int8_t cr[32]; memset(cr, 0, sizeof(cr));
        cr[0x12] = 1; cr[0x13] = 2;
        int8_t tbl[64]; memset(tbl, 0, sizeof(tbl));
        CHECK(dm2_v1_skproject_decide_next_xact_classify(0, cr, tbl, &r) == 1 &&
              r.table_index == 1 && r.initial_entry == 2,
              "decide_next_xact basic classification");
    }

    /* 0067 null receipt */
    {
        int8_t bt[6] = {0}; int8_t cr[32] = {0}; int8_t spx[16] = {0};
        CHECK(dm2_v1_skproject_0067_classify(bt, cr, spx, NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL) == 0,
              "0067 null receipt blocked");
    }

    /* select_creature_37fc null receipt */
    {
        int8_t spx[16] = {0};
        CHECK(dm2_v1_skproject_select_creature_37fc_classify(0, spx, NULL, NULL, NULL) == 0,
              "select_creature_37fc null receipt blocked");
    }
    /* select_creature_37fc null spx */
    {
        DM2_V1_SkprojectSelectCreature37FCReceipt r;
        CHECK(dm2_v1_skproject_select_creature_37fc_classify(0, NULL, NULL, NULL, &r) == 0,
              "select_creature_37fc null spx blocked");
    }

    /* source evidence batch 23b */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0457") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0550") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0276") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0684") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_08f5") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_DECIDE_NEXT_XACT") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_0067") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_SELECT_CREATURE_37FC") != 0,
          "source evidence names cycle-23 batch-23b");

    /* ---- batch 24a: 14cd_09e2, 50CB, 13e4_0982, 4EA8, PREPARE/UNPREPARE, 13e4_0360, 13e4_071b ---- */

    /* 14cd_09e2 NULL receipt */
    {
        int32_t rc = dm2_v1_skproject_14cd_09e2_classify(0x00, 0, 0, 0, 0, NULL);
        CHECK(rc == 0, "14cd_09e2: NULL receipt returns 0");
    }
    /* 14cd_09e2 0x40 flag early finalize */
    {
        DM2_V1_Skproject14cd09e2Receipt r;
        int32_t rc = dm2_v1_skproject_14cd_09e2_classify(0x40, 3, 0, 0, 0, &r);
        CHECK(rc == 1 && r.has_0x40_flag == 1 && r.finalized == 1,
              "14cd_09e2: 0x40 flag early finalize");
    }
    /* 14cd_09e2 no 0x40 no 0x20 => direction 5 */
    {
        DM2_V1_Skproject14cd09e2Receipt r;
        dm2_v1_skproject_14cd_09e2_classify(0x00, 0, 0, 0, 0, &r);
        CHECK(r.direction == 5, "14cd_09e2: direction 5 when no 0x20");
    }

    /* 50CB NULL receipt */
    {
        CHECK(dm2_v1_skproject_50cb_classify(0, 0, 0, NULL, 0, NULL) == 0,
              "50cb: NULL receipt returns 0");
    }
    /* 50CB 0xFFFF offset */
    {
        uint8_t data[16]; memset(data, 0, sizeof(data));
        DM2_V1_Skproject50CBReceipt r;
        dm2_v1_skproject_50cb_classify(0x0F, (int16_t)0xFFFF, 0, data, 16, &r);
        CHECK(r.offset_was_ffff == 1 && r.offset_result == 0,
              "50cb: 0xFFFF offset resets to 0");
    }

    /* 13e4_0982 NULL receipt */
    CHECK(dm2_v1_skproject_13e4_0982_classify(0, 0, 0, 0, 0, NULL) == 0,
          "13e4_0982: NULL receipt returns 0");
    /* 13e4_0982 savegame b03 zero skip */
    {
        DM2_V1_Skproject13e40982Receipt r;
        dm2_v1_skproject_13e4_0982_classify(0, 0, 0, 0, 0x22, &r);
        CHECK(r.savegame_b03_zero == 1 && r.skip_to_dispatch == 1,
              "13e4_0982: b03 zero skip to dispatch");
    }

    /* 4EA8 NULL receipt */
    CHECK(dm2_v1_skproject_4ea8_classify(0, 0, NULL, 0, NULL) == 0,
          "4ea8: NULL receipt returns 0");
    /* 4EA8 tick counting */
    {
        uint8_t data[20]; memset(data, 0, sizeof(data));
        data[1] = 0x10; data[5] = 0x20; data[9] = 0x00;
        DM2_V1_Skproject4EA8Receipt r;
        dm2_v1_skproject_4ea8_classify(0x0F, 0, data, 20, &r);
        CHECK(r.tick_count == 3, "4ea8: counted 3 ticks");
    }

    /* PREPARE NULL receipt */
    CHECK(dm2_v1_skproject_prepare_local_creature_var_classify(
        0, 0, 0, 0, 0, 0, 0, 0, NULL) == 0,
          "prepare: NULL receipt returns 0");
    /* PREPARE prior context */
    {
        DM2_V1_SkprojectPrepareLocalCreatureVarReceipt r;
        dm2_v1_skproject_prepare_local_creature_var_classify(
            0x1234, 5, 3, 0x22, 100, 1, 7, (int8_t)0xFF, &r);
        CHECK(r.had_prior_context == 1 && r.timer_type_is_0x22 == 1,
              "prepare: prior context saved, type 0x22");
    }

    /* UNPREPARE NULL receipt */
    CHECK(dm2_v1_skproject_unprepare_local_creature_var_classify(NULL, NULL) == 0,
          "unprepare: NULL receipt returns 0");
    /* UNPREPARE with context */
    {
        int dummy = 1;
        DM2_V1_SkprojectUnprepareLocalCreatureVarReceipt r;
        dm2_v1_skproject_unprepare_local_creature_var_classify(&dummy, &r);
        CHECK(r.had_saved_context == 1 && r.restored == 1,
              "unprepare: with context restores");
    }
    /* UNPREPARE without context */
    {
        DM2_V1_SkprojectUnprepareLocalCreatureVarReceipt r;
        dm2_v1_skproject_unprepare_local_creature_var_classify(NULL, &r);
        CHECK(r.cleared_v1e07ea == 1, "unprepare: clears v1e07ea");
    }

    /* ai_13e4_0360 NULL receipt */
    CHECK(dm2_v1_skproject_ai_13e4_0360_classify(0, 0, 0, 0, 0, 0, 0, 0, 0, NULL) == 0,
          "ai_13e4_0360: NULL receipt returns 0");
    /* ai_13e4_0360 byte5 0xFF early exit */
    {
        DM2_V1_Skproject13e40360Receipt r;
        dm2_v1_skproject_ai_13e4_0360_classify(
            0x1000, 5, 3, 0x0A, 1, (int8_t)0xFF, 0, 0, 0, &r);
        CHECK(r.creature_byte5_ff == 1 && r.wrote_behavior == 0,
              "ai_13e4_0360: byte5 0xFF early exit");
    }

    /* ai_13e4_071b NULL receipt */
    CHECK(dm2_v1_skproject_ai_13e4_071b_classify(0, 0, 0, 0, 0, NULL) == 0,
          "ai_13e4_071b: NULL receipt returns 0");
    /* ai_13e4_071b early exit 0x8001 */
    {
        DM2_V1_Skproject13e4071bReceipt r;
        dm2_v1_skproject_ai_13e4_071b_classify(0, (int16_t)0x8001, 0x0F, 1000, 4, &r);
        CHECK(r.early_exit_8001 == 1, "ai_13e4_071b: early exit 0x8001");
    }
    /* ai_13e4_071b modulo zero */
    {
        DM2_V1_Skproject13e4071bReceipt r;
        dm2_v1_skproject_ai_13e4_071b_classify(100, (int16_t)0x4000, 0x0F, 8, 4, &r);
        CHECK(r.modulo_zero == 1 && r.queued_timer == 0,
              "ai_13e4_071b: modulo zero no timer");
    }

    /* ---- batch 24b: dtor, set_mouse, vsync, stretchblit, start_timer,
       stop_timer, hide_mouse, ai_13e4_0806 ---- */

    /* dtor NULL receipt */
    CHECK(dm2_v1_skproject_dtor_classify(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == 0,
          "dtor: NULL receipt returns 0");

    /* set_mouse NULL receipt */
    CHECK(dm2_v1_skproject_set_mouse_classify(10, 20, NULL, NULL, NULL) == 0,
          "set_mouse: NULL receipt returns 0");
    /* set_mouse scaling */
    {
        DM2_V1_SkprojectSetMouseReceipt r;
        dm2_v1_skproject_set_mouse_classify(50, 100, NULL, NULL, &r);
        CHECK(r.scaled_x == 100 && r.scaled_y == 200,
              "set_mouse: x/y doubled");
    }

    /* vsync NULL receipt */
    CHECK(dm2_v1_skproject_vsync_classify(NULL) == 0,
          "vsync: NULL receipt returns 0");
    /* vsync no-op */
    {
        DM2_V1_SkprojectVsyncReceipt r;
        dm2_v1_skproject_vsync_classify(&r);
        CHECK(r.called == 1, "vsync: called flag set");
    }

    /* stretchblit NULL receipt */
    CHECK(dm2_v1_skproject_stretchblit_classify(NULL, 320, 200, NULL, NULL, NULL, NULL, NULL) == 0,
          "stretchblit: NULL receipt returns 0");

    /* start_timer NULL receipt */
    CHECK(dm2_v1_skproject_start_timer_classify(NULL, NULL, NULL) == 0,
          "start_timer: NULL receipt returns 0");

    /* stop_timer NULL receipt */
    CHECK(dm2_v1_skproject_stop_timer_classify(NULL, NULL, NULL) == 0,
          "stop_timer: NULL receipt returns 0");

    /* hide_mouse NULL receipt */
    CHECK(dm2_v1_skproject_hide_mouse_classify(NULL, NULL, NULL) == 0,
          "hide_mouse: NULL receipt returns 0");

    /* ai_13e4_0806 NULL receipt */
    CHECK(dm2_v1_skproject_ai_13e4_0806_classify(0, 0, 0, 0, NULL, NULL, NULL) == 0,
          "ai_13e4_0806: NULL receipt returns 0");
    /* ai_13e4_0806 early return */
    {
        DM2_V1_SkprojectAi13e40806Receipt r;
        dm2_v1_skproject_ai_13e4_0806_classify(0x100, (int16_t)0x8002, 4, 1000, NULL, NULL, &r);
        CHECK(r.early_return == 1, "ai_13e4_0806: early return on masked 0x8000 low>1");
    }

    /* source evidence batch 24 */
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(), "DM2_14cd_09e2") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_50CB") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_4EA8") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_ai_13e4_0806") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "dtor") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "set_mouse") != 0,
          "source evidence names cycle-24 batch-24a+24b");

    /* --- Batch 25a: c_alloc.cpp memory allocation tests --- */

    CHECK(dm2_v1_skproject_get_from_freepool_classify(100, 20, NULL) == 0,
          "get_from_freepool: NULL receipt returns 0");
    {
        DM2_V1_SkprojectGetFromFreepoolReceipt r;
        CHECK(dm2_v1_skproject_get_from_freepool_classify(100, 20, &r) == 1,
              "get_from_freepool: normal returns 1");
        CHECK(r.amount == 20, "get_from_freepool: amount");
        CHECK(r.available_before == 100, "get_from_freepool: available_before");
        CHECK(r.available_after == 80, "get_from_freepool: available_after");
        CHECK(r.did_subtract == 1, "get_from_freepool: did_subtract");
    }

    CHECK(dm2_v1_skproject_find_free_pool_classify(NULL, 0, 10, 1, NULL, NULL) == 0,
          "find_free_pool: NULL receipt returns 0");
    {
        DM2_V1_FreepoolEntry pools[3] = {
            { .tag = 0, .mode = 1, .available = 50 },
            { .tag = 0, .mode = 1, .available = 30 },
            { .tag = 1, .mode = 1, .available = 25 },
        };
        DM2_V1_SkprojectFindFreePoolReceipt r;
        int32_t best = -1;
        CHECK(dm2_v1_skproject_find_free_pool_classify(pools, 3, 20, 1, &best, &r) == 1,
              "find_free_pool: best-fit returns 1");
        CHECK(r.found == 1, "find_free_pool: found");
        CHECK(best == 1, "find_free_pool: best index");
        CHECK(r.smallest_slack == 10, "find_free_pool: smallest slack");
    }
    {
        DM2_V1_FreepoolEntry pools[1] = {
            { .tag = 0, .mode = 2, .available = 100 },
        };
        DM2_V1_SkprojectFindFreePoolReceipt r;
        int32_t best = -1;
        CHECK(dm2_v1_skproject_find_free_pool_classify(pools, 1, 10, 1, &best, &r) == 1,
              "find_free_pool: no match returns 1");
        CHECK(r.found == 0, "find_free_pool: not found");
        CHECK(best == -1, "find_free_pool: best stays -1");
    }

    CHECK(dm2_v1_skproject_alloc_memory_ram_classify(10, 1, 0, NULL, NULL, 0, NULL) == 0,
          "alloc_memory_ram: NULL receipt returns 0");
    {
        DM2_V1_FreepoolEntry pools[1] = {
            { .tag = 0, .mode = 1, .available = 100 },
        };
        DM2_V1_AllocMemoryRamState state = { .bigpool = 500, .secondpool_mode = 0, .secondpool_available = 0 };
        DM2_V1_SkprojectAllocMemoryRamReceipt r;
        CHECK(dm2_v1_skproject_alloc_memory_ram_classify(20, 1, 0, &state, pools, 1, &r) == 1,
              "alloc_memory_ram: freepool route");
        CHECK(r.route_freepool == 1, "alloc_memory_ram: route_freepool");
    }
    {
        DM2_V1_AllocMemoryRamState state = { .bigpool = 500, .secondpool_mode = 1, .secondpool_available = 50 };
        DM2_V1_SkprojectAllocMemoryRamReceipt r;
        CHECK(dm2_v1_skproject_alloc_memory_ram_classify(20, 1, 0, &state, NULL, 0, &r) == 1,
              "alloc_memory_ram: secondpool route");
        CHECK(r.route_secondpool == 1, "alloc_memory_ram: route_secondpool");
    }
    {
        DM2_V1_AllocMemoryRamState state = { .bigpool = 500, .secondpool_mode = 0, .secondpool_available = 0 };
        DM2_V1_SkprojectAllocMemoryRamReceipt r;
        CHECK(dm2_v1_skproject_alloc_memory_ram_classify(20, 1, 1, &state, NULL, 0, &r) == 1,
              "alloc_memory_ram: bigpool lo route");
        CHECK(r.route_bigpool_lo == 1, "alloc_memory_ram: route_bigpool_lo");
    }
    {
        DM2_V1_AllocMemoryRamState state = { .bigpool = 500, .secondpool_mode = 0, .secondpool_available = 0 };
        DM2_V1_SkprojectAllocMemoryRamReceipt r;
        CHECK(dm2_v1_skproject_alloc_memory_ram_classify(20, 1, 2, &state, NULL, 0, &r) == 1,
              "alloc_memory_ram: bigpool hi route");
        CHECK(r.route_bigpool_hi == 1, "alloc_memory_ram: route_bigpool_hi");
    }
    {
        DM2_V1_AllocMemoryRamState state = { .bigpool = 10, .secondpool_mode = 0, .secondpool_available = 0 };
        DM2_V1_SkprojectAllocMemoryRamReceipt r;
        CHECK(dm2_v1_skproject_alloc_memory_ram_classify(20, 1, 1, &state, NULL, 0, &r) == 1,
              "alloc_memory_ram: syserr route");
        CHECK(r.route_syserr == 1, "alloc_memory_ram: route_syserr");
    }
    {
        DM2_V1_AllocMemoryRamState state = { .bigpool = 500, .secondpool_mode = 0, .secondpool_available = 0 };
        DM2_V1_SkprojectAllocMemoryRamReceipt r;
        int16_t wtype = 1 | (int16_t)0x8000;
        CHECK(dm2_v1_skproject_alloc_memory_ram_classify(21, 1, wtype, &state, NULL, 0, &r) == 1,
              "alloc_memory_ram: clean+odd");
        CHECK(r.clean_flag == 1, "alloc_memory_ram: clean_flag");
        CHECK(r.amount_was_odd == 1, "alloc_memory_ram: amount_was_odd");
        CHECK(r.amount == 22, "alloc_memory_ram: adjusted amount");
    }

    CHECK(dm2_v1_skproject_dealloc_lobigpool_classify(10, NULL) == 0,
          "dealloc_lobigpool: NULL receipt returns 0");
    {
        DM2_V1_SkprojectDeallocLobigpoolReceipt r;
        CHECK(dm2_v1_skproject_dealloc_lobigpool_classify(20, &r) == 1,
              "dealloc_lobigpool: even");
        CHECK(r.adjusted_amount == 20, "dealloc_lobigpool: even adjusted");
    }
    {
        DM2_V1_SkprojectDeallocLobigpoolReceipt r;
        CHECK(dm2_v1_skproject_dealloc_lobigpool_classify(21, &r) == 1,
              "dealloc_lobigpool: odd");
        CHECK(r.amount_was_odd == 1, "dealloc_lobigpool: odd flag");
        CHECK(r.adjusted_amount == 22, "dealloc_lobigpool: odd adjusted");
    }

    CHECK(dm2_v1_skproject_dealloc_hibigpool_classify(10, NULL) == 0,
          "dealloc_hibigpool: NULL receipt returns 0");
    {
        DM2_V1_SkprojectDeallocHibigpoolReceipt r;
        CHECK(dm2_v1_skproject_dealloc_hibigpool_classify(20, &r) == 1,
              "dealloc_hibigpool: even");
        CHECK(r.adjusted_amount == 20, "dealloc_hibigpool: even adjusted");
    }
    {
        DM2_V1_SkprojectDeallocHibigpoolReceipt r;
        CHECK(dm2_v1_skproject_dealloc_hibigpool_classify(21, &r) == 1,
              "dealloc_hibigpool: odd");
        CHECK(r.amount_was_odd == 1, "dealloc_hibigpool: odd flag");
        CHECK(r.adjusted_amount == 22, "dealloc_hibigpool: odd adjusted");
    }

    CHECK(dm2_v1_skproject_alloc_freepool_memory_classify(10, 0, NULL) == 0,
          "alloc_freepool_memory: NULL receipt returns 0");
    {
        DM2_V1_SkprojectAllocFreepoolMemoryReceipt r;
        CHECK(dm2_v1_skproject_alloc_freepool_memory_classify(100, 0, &r) == 1,
              "alloc_freepool_memory: no clean");
        CHECK(r.composed_wtype == 0, "alloc_freepool_memory: wtype 0");
    }
    {
        DM2_V1_SkprojectAllocFreepoolMemoryReceipt r;
        CHECK(dm2_v1_skproject_alloc_freepool_memory_classify(100, 1, &r) == 1,
              "alloc_freepool_memory: clean");
        CHECK(r.composed_wtype == (int16_t)0x8000, "alloc_freepool_memory: wtype clean");
    }

    CHECK(dm2_v1_skproject_alloc_lobigpool_memory_classify(10, 0, NULL) == 0,
          "alloc_lobigpool_memory: NULL receipt returns 0");
    {
        DM2_V1_SkprojectAllocLobigpoolMemoryReceipt r;
        CHECK(dm2_v1_skproject_alloc_lobigpool_memory_classify(100, 0, &r) == 1,
              "alloc_lobigpool_memory: no clean");
        CHECK(r.composed_wtype == 1, "alloc_lobigpool_memory: wtype 1");
    }
    {
        DM2_V1_SkprojectAllocLobigpoolMemoryReceipt r;
        CHECK(dm2_v1_skproject_alloc_lobigpool_memory_classify(100, 1, &r) == 1,
              "alloc_lobigpool_memory: clean");
        CHECK(r.composed_wtype == (1 | (int16_t)0x8000), "alloc_lobigpool_memory: wtype clean");
    }

    CHECK(dm2_v1_skproject_alloc_hibigpool_memory_classify(10, 0, NULL) == 0,
          "alloc_hibigpool_memory: NULL receipt returns 0");
    {
        DM2_V1_SkprojectAllocHibigpoolMemoryReceipt r;
        CHECK(dm2_v1_skproject_alloc_hibigpool_memory_classify(100, 0, &r) == 1,
              "alloc_hibigpool_memory: no clean");
        CHECK(r.composed_wtype == 2, "alloc_hibigpool_memory: wtype 2");
    }
    {
        DM2_V1_SkprojectAllocHibigpoolMemoryReceipt r;
        CHECK(dm2_v1_skproject_alloc_hibigpool_memory_classify(100, 1, &r) == 1,
              "alloc_hibigpool_memory: clean");
        CHECK(r.composed_wtype == (2 | (int16_t)0x8000), "alloc_hibigpool_memory: wtype clean");
    }

    /* --- Batch 25b: c_alloc.cpp memory pool management tests --- */

    {
        int32_t r = dm2_v1_skproject_tag_largest_free_pool_classify(NULL, 0, NULL, NULL);
        CHECK(r == 0, "tag_largest_free_pool: NULL receipt returns 0");
    }
    {
        DM2_V1_SkprojectTagLargestFreePoolReceipt receipt;
        DM2V1_FreepoolNode *result = NULL;
        int32_t r = dm2_v1_skproject_tag_largest_free_pool_classify(NULL, 1, &result, &receipt);
        CHECK(r == 1, "tag_largest_free_pool: empty list");
        CHECK(receipt.list_empty == 1, "tag_largest_free_pool: list_empty");
        CHECK(result == NULL, "tag_largest_free_pool: result NULL");
    }
    {
        DM2V1_FreepoolNode n1, n2;
        memset(&n1, 0, sizeof(n1));
        memset(&n2, 0, sizeof(n2));
        n1.tag = 0; n1.mode = 1; n1.amount = 100; n1.fp_prev = NULL;
        n2.tag = 0; n2.mode = 1; n2.amount = 200; n2.fp_prev = &n1;
        DM2_V1_SkprojectTagLargestFreePoolReceipt receipt;
        DM2V1_FreepoolNode *result = NULL;
        int32_t r = dm2_v1_skproject_tag_largest_free_pool_classify(&n2, 1, &result, &receipt);
        CHECK(r == 1, "tag_largest_free_pool: finds largest");
        CHECK(receipt.found_match == 1, "tag_largest_free_pool: found_match");
        CHECK(receipt.largest_amount == 200, "tag_largest_free_pool: largest_amount");
        CHECK(result == &n2, "tag_largest_free_pool: result ptr");
        CHECK(n2.tag == 1, "tag_largest_free_pool: tagged");
    }

    {
        int32_t r = dm2_v1_skproject_append_free_pool_classify(NULL, 0, 0, NULL, NULL, NULL, NULL);
        CHECK(r == 0, "append_free_pool: NULL receipt returns 0");
    }
    {
        DM2V1_FreepoolNode node;
        memset(&node, 0, sizeof(node));
        DM2_V1_SkprojectAppendFreePoolReceipt receipt;
        int32_t r = dm2_v1_skproject_append_free_pool_classify(
            &node, 3, 256, NULL, NULL, NULL, &receipt);
        CHECK(r == 1, "append_free_pool: sets fields");
        CHECK(receipt.applied == 1, "append_free_pool: applied");
        CHECK(node.mode == 3, "append_free_pool: mode");
        CHECK(node.tag == 0, "append_free_pool: tag");
        CHECK(node.amount == 256 - (int32_t)sizeof(DM2V1_FreepoolNode),
              "append_free_pool: amount");
    }

    {
        int32_t r = dm2_v1_skproject_add_mem_to_free_pool_classify(NULL, 0, 0, NULL, NULL, NULL, NULL);
        CHECK(r == 0, "add_mem_to_free_pool: NULL receipt returns 0");
    }
    {
        DM2V1_FreepoolNode node;
        memset(&node, 0, sizeof(node));
        DM2_V1_SkprojectAddMemToFreePoolReceipt receipt;
        int32_t r = dm2_v1_skproject_add_mem_to_free_pool_classify(
            &node, 1, 50, NULL, NULL, NULL, &receipt);
        CHECK(r == 1, "add_mem_to_free_pool: too small");
        CHECK(receipt.too_small == 1, "add_mem_to_free_pool: too_small flag");
    }
    {
        DM2V1_FreepoolNode node;
        memset(&node, 0, sizeof(node));
        DM2_V1_SkprojectAddMemToFreePoolReceipt receipt;
        int32_t r = dm2_v1_skproject_add_mem_to_free_pool_classify(
            &node, 1, 101, NULL, NULL, NULL, &receipt);
        CHECK(r == 1, "add_mem_to_free_pool: aligns and appends");
        CHECK(receipt.final_amount == 100, "add_mem_to_free_pool: aligned to 100");
        CHECK(receipt.appended == 1, "add_mem_to_free_pool: appended");
    }

    {
        int32_t r = dm2_v1_skproject_bup_freepool_classify(NULL, NULL);
        CHECK(r == 0, "bup_freepool: NULL receipt returns 0");
    }
    {
        DM2V1_FreepoolNode n1;
        memset(&n1, 0, sizeof(n1));
        n1.tag = 0; n1.endoffree = (void *)0x1000; n1.available = 500; n1.fp_prev = NULL;
        DM2_V1_SkprojectBupFreepoolReceipt receipt;
        int32_t r = dm2_v1_skproject_bup_freepool_classify(&n1, &receipt);
        CHECK(r == 1, "bup_freepool: backs up");
        CHECK(receipt.nodes_backed_up == 1, "bup_freepool: nodes_backed_up");
        CHECK(n1.eof_bup == (void *)0x1000, "bup_freepool: eof_bup");
        CHECK(n1.ava_bup == 500, "bup_freepool: ava_bup");
    }
    {
        DM2V1_FreepoolNode n1;
        memset(&n1, 0, sizeof(n1));
        n1.tag = 1; n1.fp_prev = NULL;
        DM2_V1_SkprojectBupFreepoolReceipt receipt;
        int32_t r = dm2_v1_skproject_bup_freepool_classify(&n1, &receipt);
        CHECK(r == 1, "bup_freepool: skips tagged");
        CHECK(receipt.nodes_backed_up == 0, "bup_freepool: none backed up");
    }

    {
        int32_t r = dm2_v1_skproject_restore_freepool_classify(NULL, NULL);
        CHECK(r == 0, "restore_freepool: NULL receipt returns 0");
    }
    {
        DM2V1_FreepoolNode n1;
        memset(&n1, 0, sizeof(n1));
        n1.tag = 0;
        n1.endoffree = (void *)0x2000;
        n1.available = 300;
        n1.eof_bup = (void *)0x1000;
        n1.ava_bup = 500;
        n1.fp_prev = NULL;
        DM2_V1_SkprojectRestoreFreepoolReceipt receipt;
        int32_t r = dm2_v1_skproject_restore_freepool_classify(&n1, &receipt);
        CHECK(r == 1, "restore_freepool: restores from backup");
        CHECK(receipt.nodes_restored == 1, "restore_freepool: nodes_restored");
        CHECK(n1.endoffree == (void *)0x1000, "restore_freepool: endoffree restored");
        CHECK(n1.available == 500, "restore_freepool: available restored");
    }

    {
        int32_t r = dm2_v1_skproject_complete_allocation_classify(NULL, NULL, NULL, NULL);
        CHECK(r == 0, "complete_allocation: NULL receipt returns 0");
    }
    {
        DM2_V1_SkprojectCompleteAllocationReceipt receipt;
        int32_t r = dm2_v1_skproject_complete_allocation_classify(NULL, NULL, NULL, &receipt);
        CHECK(r == 1, "complete_allocation: NULL first pool");
        CHECK(receipt.first_pool_null_error == 1, "complete_allocation: first_pool_null_error");
    }

    {
        int32_t r = dm2_v1_skproject_dtor_memory_allocation_classify(NULL, NULL, NULL);
        CHECK(r == 0, "dtor_memory_allocation: NULL receipt returns 0");
    }
    {
        void *mem = (void *)0x1234;
        DM2_V1_SkprojectDtorMemoryAllocationReceipt receipt;
        int32_t r = dm2_v1_skproject_dtor_memory_allocation_classify(&mem, NULL, &receipt);
        CHECK(r == 1, "dtor_memory_allocation: frees and NULLs");
        CHECK(receipt.was_allocated == 1, "dtor_memory_allocation: was_allocated");
        CHECK(receipt.freed == 1, "dtor_memory_allocation: freed");
        CHECK(mem == NULL, "dtor_memory_allocation: mem nulled");
    }
    {
        void *mem = NULL;
        DM2_V1_SkprojectDtorMemoryAllocationReceipt receipt;
        int32_t r = dm2_v1_skproject_dtor_memory_allocation_classify(&mem, NULL, &receipt);
        CHECK(r == 1, "dtor_memory_allocation: NULL is no-op");
        CHECK(receipt.was_allocated == 0, "dtor_memory_allocation: not allocated");
    }

    {
        int32_t r = dm2_v1_skproject_setup_memory_allocation_classify(
            NULL, 0, 0, 0, NULL, NULL, NULL, NULL, NULL);
        CHECK(r == 0, "setup_memory_allocation: NULL receipt returns 0");
    }

    CHECK(strstr(dm2_v1_skproject_core_source_evidence(), "DM2_GET_FROM_FREEPOOL") != 0 &&
          strstr(dm2_v1_skproject_core_source_evidence(), "DM2_DTOR_MEMORYALLOCATION") != 0,
          "source evidence names cycle-25 batch-25a+25b");

    if (failed) {
        printf("%d failure(s)\n", failed);
        return 1;
    }
    puts("all DM2 skproject core helper checks passed");
    return 0;
}
