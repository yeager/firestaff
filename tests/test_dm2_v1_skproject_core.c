#include "dm2_v1_skproject_core.h"
#include "dm2_v1_asset_loader.h"

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
    CHECK(dm2_v1_skproject_get_tile_value(
              tiles, passage, 4, 4, 2, 1, &tile_value) == 0x86 &&
              tile_value.valid &&
              tile_value.in_bounds &&
              tile_value.returned_tile_value == 0x86u,
          "GET_TILE_VALUE returns the current map byte for in-bounds tiles");
    CHECK(dm2_v1_skproject_get_tile_value(
              tiles, passage, 4, 4, -1, 0, &tile_value) == 4 &&
              tile_value.valid &&
              tile_value.used_left_boundary &&
              tile_value.checked_primary_passage,
          "GET_TILE_VALUE returns left-boundary mask when adjacent tile is passage");
    CHECK(dm2_v1_skproject_get_tile_value(
              tiles, passage, 4, 4, 4, 2, &tile_value) == 1 &&
              tile_value.valid &&
              tile_value.used_right_boundary,
          "GET_TILE_VALUE returns right-boundary mask when adjacent tile is passage");
    CHECK(dm2_v1_skproject_get_tile_value(
              tiles, passage, 4, 4, 1, -1, &tile_value) == 0 &&
              tile_value.valid &&
              tile_value.used_top_boundary &&
              tile_value.checked_side_passage,
          "GET_TILE_VALUE returns zero when top boundary is blocked but side passage exists");
    CHECK(dm2_v1_skproject_get_tile_value(
              tiles, passage, 4, 4, 3, 4, &tile_value) == 8 &&
              tile_value.valid &&
              tile_value.used_bottom_boundary,
          "GET_TILE_VALUE returns bottom-boundary mask when adjacent tile is passage");
    CHECK(dm2_v1_skproject_get_tile_value(
              tiles, passage, 4, 4, -1, -1, &tile_value) == 0 &&
              tile_value.valid &&
              tile_value.used_corner_boundary,
          "GET_TILE_VALUE preserves source corner-boundary zero when corner-adjacent passage exists");
    CHECK(dm2_v1_skproject_get_tile_value(
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

        CHECK(dm2_v1_skproject_get_address_of_tile_record(
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
    DM2_V1_SkprojectFreeCacheIndexReceipt free_cache;
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

    if (failed) {
        printf("%d failure(s)\n", failed);
        return 1;
    }
    puts("all DM2 skproject core helper checks passed");
    return 0;
}
