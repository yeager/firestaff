/*
 * test_csb_v1_boot_title_import_ui_gate_pc34_compat.c
 *
 * CSB V1 title/import startup UI gate.
 *
 * Proves that the CSB boot profile reaches the launch intent
 * without falling back to DM1-only behavior on the
 * title/version surface, and that the CMP Utility Disk
 * champion-import handoff reaches the runtime party slot.
 *
 * Specifically asserts:
 *
 *   1. csb_v1_boot_profile_init() leaves the engine-version
 *      display helper at the DM1 baseline ("v2.0").
 *
 *   2. csb_v1_boot_enter_game() flips the engine-version
 *      display helper to the CSB value ("v2.1") once a
 *      verified profile (graphics + dungeon both verified)
 *      hands off to the runtime.  This closes the gap where
 *      the helper retained its data-free DM1 default ("v2.0")
 *      and the CSB launch fell back to DM1-only behavior on
 *      the version-display surface even though the runtime
 *      variant was CSB_V1_VARIANT_PC34_EN.
 *
 *   3. csb_v1_boot_cleanup() resets the engine-version display
 *      helper back to "v2.0" so a follow-up DM1 launch does
 *      not see a stale "v2.1" string from the prior CSB
 *      handoff.
 *
 *   4. csb_v1_boot_set_imported_party_from_cmp() rejects a valid
 *      CMP-only buffer: it lacks the source-backed champion state
 *      required for a live CSB party.
 *
 *   5. csb_v1_boot_set_imported_party_from_cmp() on a buffer
 *      with a bad CMP magic sets cmp_import_attempted to 1,
 *      leaves cmp_import_succeeded at 0, and leaves
 *      imported_party_ready at 0 so the runtime does not
 *      consume a corrupt import.
 *
 *   6. csb_v1_boot_diagnostic_report() surfaces both the
 *      engine-version string ("v2.0" or "v2.1") and the
 *      CMP-import status so a launcher-side log proves the
 *      CSB handoff reached the title surface.
 *
 * Source-locks:
 *   ReDMCSB DIALOG.C:2014-2023 (engine version displayed in
 *     top-right of dialog boxes).
 *   ReDMCSB CHANGE8_13 (CSB engine version 2.1 hardcoded).
 *   ReDMCSB CEDT002.C + CEDT021.C + CEDTINC7.C
 *     (Utility Disk Champion Editor → CSB runtime handoff).
 *   ReDMCSB DEFS.H CMP typedef (size 496 bytes).
 *   CSBWin/CedtData.cpp (CSB Utility Disk tool flow).
 *
 * The boot checks run against an operator-supplied original CSB package.
 */

#include "csb_v1_boot.h"
#include "csb_v1_cmp_import_pc34_compat.h"
#include "csb_v1_engine_version_display_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"
#include "firestaff_cmp_decode.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else      { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

/* The decoder accepts the 464-byte portrait payload in the CMP
 * record. Keep the fixture size local to the test so it does not
 * depend on implementation-private constants. */
#ifndef CMP_PORTRAIT_BYTES_FOR_TEST
#define CMP_PORTRAIT_BYTES_FOR_TEST 464u
#endif

/* Build a synthetic CMP file with a recognisable Name/Title/portrait
 * pattern.  Mirrors the fixture used by
 * csb_v1_cmp_import_self_test but with a different sentinel
 * pattern so the two paths can be told apart. */
static int build_synthetic_cmp(uint8_t *buf, size_t buf_size,
                                const char *name, const char *title,
                                uint8_t portrait_fill)
{
    size_t name_len;
    size_t title_len;
    if (!buf || buf_size < FIRESTAFF_CMP_FILE_SIZE) return -1;
    if (!name || !title) return -1;
    name_len = strlen(name);
    title_len = strlen(title);
    if (name_len == 0 || name_len > FIRESTAFF_CMP_NAME_SIZE) return -1;
    if (title_len == 0 || title_len > FIRESTAFF_CMP_TITLE_SIZE) return -1;
    memset(buf, 0, buf_size);
    /* cmp_i_C + cmp_i_E are already 0 */
    memcpy(buf + 4, name, name_len);
    memcpy(buf + 4 + FIRESTAFF_CMP_NAME_SIZE, title, title_len);
    memset(buf + 4 + FIRESTAFF_CMP_NAME_SIZE + FIRESTAFF_CMP_TITLE_SIZE,
           portrait_fill, CMP_PORTRAIT_BYTES_FOR_TEST);
    return 0;
}

static const char *s_gate_tmp = "/tmp/firestaff-csb-v1-title-import-gate";

static int write_gate_dungeon(const char *path)
{
    uint8_t buf[80];
    uint16_t bit_a;
    FILE *f;
    size_t n;
    memset(buf, 0, sizeof(buf));
    buf[4] = 1;                          /* level_count = 1 */
    buf[8] = 0x21; buf[9] = 0x00;       /* party at (1,1) dir=0 */
    bit_a = (uint16_t)((2u << 6) | (2u << 11));
    buf[52] = (uint8_t)(bit_a & 0xFF);
    buf[53] = (uint8_t)(bit_a >> 8);
    memset(buf + 66, 1, 9);             /* 3x3 corridor squares */
    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, 75, f);
    fclose(f);
    return (n == 75) ? 0 : -1;
}

static void prime_verified_profile(CSB_V1_BootProfile *p)
{
    char dungeon_path[256];
    mkdir(s_gate_tmp, 0755);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", s_gate_tmp);
    write_gate_dungeon(dungeon_path);

    csb_v1_boot_profile_init(p);
    p->assets_verified = 1;
    p->graphics_verified = 1;
    p->dungeon_verified = 1;
    p->state = CSB_V1_BOOT_STATE_ASSETS_READY;
    p->variant_id = CSB_V1_VARIANT_PC34_EN;
    p->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    snprintf(p->asset_root, sizeof(p->asset_root), "%s", s_gate_tmp);
    snprintf(p->graphics_path, sizeof(p->graphics_path), "%s/GRAPHICS.DAT", s_gate_tmp);
    snprintf(p->dungeon_path, sizeof(p->dungeon_path), "%s", dungeon_path);
    csb_v1_boot_set_save_root(p, s_gate_tmp);
}

static void test_profile_init_resets_engine_version_to_dm1(void)
{
    /* Force a stale "v2.1" so the init path has to actively
     * reset to "v2.0". */
    csb_v1_engine_version_display_set_csb(1);
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "precondition: helper forced to CSB");
    {
        CSB_V1_BootProfile p;
        csb_v1_boot_profile_init(&p);
        CHECK(csb_v1_engine_version_display_is_csb() == 0,
              "csb_v1_boot_profile_init resets helper to DM1");
        CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
              "engine version display reports v2.0 after profile_init");
        CHECK(p.engine_version_displayed == 0,
              "boot profile marks engine version not yet displayed");
        csb_v1_boot_cleanup(&p);
    }
}

static void test_enter_game_flips_engine_version_to_csb(void)
{
    CSB_V1_BootProfile p;
    prime_verified_profile(&p);
    /* Precondition: helper still at DM1 baseline */
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "precondition: helper is at DM1 baseline before enter_game");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "precondition: helper reports v2.0 before enter_game");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "csb_v1_boot_enter_game accepts a verified profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile advances to RUNTIME_READY");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime starts at CSB_STATE_TITLE");
    /* The actual flip */
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "enter_game flips helper to CSB (CHANGE8_13)");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.1") == 0,
          "engine version display reports v2.1 after CSB enter_game");
    CHECK(p.engine_version_displayed == 1,
          "boot profile records engine_version_displayed = 1");
    csb_v1_boot_cleanup(&p);
}

static void test_cleanup_resets_engine_version_to_dm1(void)
{
    CSB_V1_BootProfile p;
    prime_verified_profile(&p);
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "verified profile enters the CSB runtime");
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "precondition: helper is at CSB after enter_game");
    csb_v1_boot_cleanup(&p);
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "cleanup resets helper back to DM1");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "cleanup returns helper string to v2.0");
    CHECK(p.engine_version_displayed == 0,
          "boot profile records engine_version_displayed = 0 after cleanup");
    /* csb_v1_boot_reset_engine_version_to_dm1() should be idempotent
     * when called after cleanup. */
    csb_v1_boot_reset_engine_version_to_dm1();
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "csb_v1_boot_reset_engine_version_to_dm1() is idempotent");
}

static void test_enter_game_without_assets_leaves_version_at_dm1(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    /* Precondition: helper at DM1 baseline */
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "precondition: helper at DM1 baseline");
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects an unverified profile");
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "failed enter_game leaves helper at DM1");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "engine version still reports v2.0 after failed enter_game");
    csb_v1_boot_cleanup(&p);
}

static void test_cmp_import_cannot_create_boot_party(void)
{
    CSB_V1_BootProfile p;
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];

    prime_verified_profile(&p);
    CHECK(p.cmp_import_attempted == 0,
          "precondition: cmp_import_attempted starts at 0");
    CHECK(p.cmp_import_succeeded == 0,
          "precondition: cmp_import_succeeded starts at 0");
    CHECK(p.imported_party_ready == 0,
          "precondition: imported_party_ready starts at 0");
    CHECK(build_synthetic_cmp(cmp_buf, sizeof(cmp_buf),
                                "HECTOR", "WARRIOR", 0xA5) == 0,
          "synthetic CMP buffer is built");
    CHECK(csb_v1_boot_set_imported_party_from_cmp(&p, cmp_buf,
                                                   sizeof(cmp_buf)) != 0,
          "CMP-only import cannot create a live party");
    CHECK(p.cmp_import_attempted == 1,
          "cmp_import_attempted is 1 after import");
    CHECK(p.cmp_import_succeeded == 0,
          "CMP-only import does not report a completed party import");
    CHECK(p.cmp_imported_slot == -1,
          "CMP-only import publishes no champion slot");
    CHECK(p.imported_party.ChampionCount == 0,
          "CMP-only import leaves the party untouched");
    CHECK(p.imported_party_ready == 0,
          "CMP-only import never marks the party ready");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game accepts the profile without a CMP-created party");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile reaches RUNTIME_READY without a CMP-created party");
    csb_v1_boot_cleanup(&p);
}

static void test_cmp_import_bad_magic_rejected(void)
{
    CSB_V1_BootProfile p;
    int map_index;
    int map_x;
    int map_y;
    int map_dir;
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];

    prime_verified_profile(&p);
    CHECK(build_synthetic_cmp(cmp_buf, sizeof(cmp_buf),
                                "HECTOR", "WARRIOR", 0xA5) == 0,
          "synthetic CMP buffer is built");
    /* Corrupt the CMP magic. */
    cmp_buf[0] = 0x42; cmp_buf[1] = 0x42;
    int rc = csb_v1_boot_set_imported_party_from_cmp(&p, cmp_buf,
                                                       sizeof(cmp_buf));
    CHECK(rc != 0,
          "CMP import helper rejects bad magic");
    CHECK(p.cmp_import_attempted == 1,
          "cmp_import_attempted records the attempt even on bad magic");
    CHECK(p.cmp_import_succeeded == 0,
          "cmp_import_succeeded stays 0 on bad magic");
    CHECK(p.imported_party_ready == 0,
          "imported_party_ready stays 0 so runtime does not consume bad CMP");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game still succeeds without a CMP import");
    /* The runtime party must retain DUNGEON_HEADER.InitialPartyLocation,
     * not a compile-time pose or data from the rejected CMP. ReDMCSB
     * LOADSAVE.C F0435 reads this packed location after the real load. */
    CHECK(p.runtime.dungeon_handle != NULL &&
          csb_v1_dungeon_initial_party_pose_pc34(
              (const CSB_V1_DungeonData *)p.runtime.dungeon_handle,
              &map_index, &map_x, &map_y, &map_dir) &&
          p.runtime.current_level == map_index &&
          p.runtime.party_x == map_x && p.runtime.party_y == map_y &&
          p.runtime.party_dir == map_dir,
          "runtime keeps the original dungeon start pose after rejected CMP");
    csb_v1_boot_cleanup(&p);
}

static void test_mark_imported_party_ready_uses_existing_party(void)
{
    CSB_V1_BootProfile p;
    CSB_V1_PartyState party;

    prime_verified_profile(&p);
    memset(&party, 0, sizeof(party));
    party.ChampionCount = 1;
    memcpy(party.Champions[0].Name, "ALPHA", 6);
    CHECK(csb_v1_boot_set_imported_party(&p, &party) == 0,
          "boot profile carries an externally-prepared imported party");
    CHECK(p.cmp_import_attempted == 0,
          "precondition: cmp_import_attempted still 0");
    CHECK(p.imported_party_ready == 1,
          "set_imported_party flips imported_party_ready to 1");
    CHECK(csb_v1_boot_mark_imported_party_ready(&p) == 0,
          "mark_imported_party_ready is idempotent");
    CHECK(p.cmp_import_attempted == 1,
          "mark_imported_party_ready records the attempt flag");
    csb_v1_boot_cleanup(&p);
}

static void test_utility_action_cursor_drives_select_action(void)
{
    CSB_V1_UtilFlowContext flow;
    CSB_V1_UtilInputResult input_result;

    csb_v1_util_flow_init(&flow);
    csb_v1_util_flow_mark_utility_disk_verified(&flow, 1);
    CHECK(flow.selected_action_index == 0,
          "utility action cursor starts on import");
    CHECK(csb_v1_util_flow_selected_action(&flow) ==
          CSB_V1_UTIL_ACTION_IMPORT,
          "utility selected action starts as import");
    CHECK(strcmp(csb_v1_util_flow_action_label(
              csb_v1_util_flow_selected_action(&flow)),
              "IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE") == 0,
          "utility selected action label is source menu import row");

    CHECK(csb_v1_util_flow_step(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_INSERT_DISK,
          "utility flow reaches INSERT_DISK");
    CHECK(csb_v1_util_flow_step(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_VERIFY_DISK,
          "utility flow reaches VERIFY_DISK");
    CHECK(csb_v1_util_flow_step(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_DISK_OK,
          "utility flow reaches DISK_OK");
    CHECK(csb_v1_util_flow_step(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_SELECT_ACTION,
          "utility flow reaches SELECT_ACTION");

    CHECK(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_DOWN,
              1,
              &input_result) &&
              input_result.kind == CSB_V1_UTIL_INPUT_RESULT_CURSOR_MOVED &&
              input_result.selected_action_index == 1 &&
              input_result.action == CSB_V1_UTIL_ACTION_LOAD &&
              input_result.preview_active == 0,
          "utility keyboard DOWN is CSB-owned cursor movement and closes preview");
    CHECK(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_BACK,
              1,
              &input_result) &&
              input_result.kind == CSB_V1_UTIL_INPUT_RESULT_CLOSE_PREVIEW &&
              input_result.selected_action_index == 1 &&
              input_result.preview_active == 0,
          "utility keyboard Back closes preview without changing row");
    CHECK(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_UP,
              0,
              &input_result) &&
              input_result.kind == CSB_V1_UTIL_INPUT_RESULT_CURSOR_MOVED &&
              input_result.selected_action_index == 0 &&
              input_result.action == CSB_V1_UTIL_ACTION_IMPORT,
          "utility keyboard UP moves through the CSB menu model");
    CHECK(csb_v1_util_flow_handle_input(
              &flow,
              CSB_V1_UTIL_INPUT_ACTION,
              0,
              &input_result) &&
              input_result.kind == CSB_V1_UTIL_INPUT_RESULT_ACTIVATE &&
              input_result.action == CSB_V1_UTIL_ACTION_IMPORT &&
              input_result.selected_action_index == 0,
          "utility keyboard Action activates the CSB-selected row");

    CHECK(csb_v1_util_flow_move_action_cursor(&flow, 1) == 1 &&
          csb_v1_util_flow_selected_action(&flow) ==
              CSB_V1_UTIL_ACTION_LOAD,
          "utility cursor moves to load row");
    CHECK(csb_v1_util_flow_move_action_cursor(&flow, 2) == 3 &&
          csb_v1_util_flow_selected_action(&flow) ==
              CSB_V1_UTIL_ACTION_VIEW,
          "utility cursor moves to view row");
    CHECK(csb_v1_util_flow_move_action_cursor(&flow, 1) == 0 &&
          csb_v1_util_flow_selected_action(&flow) ==
              CSB_V1_UTIL_ACTION_IMPORT,
          "utility cursor wraps from view to import");
    CHECK(csb_v1_util_flow_move_action_cursor(&flow, -1) == 3 &&
          csb_v1_util_flow_selected_action(&flow) ==
              CSB_V1_UTIL_ACTION_VIEW,
          "utility cursor wraps upward to view");
    CHECK(csb_v1_util_flow_move_action_cursor(&flow, -3) == 0 &&
          csb_v1_util_flow_accept_selected_action(&flow) == 0 &&
          flow.action == CSB_V1_UTIL_ACTION_IMPORT,
          "utility accept commits selected import action");
    CHECK(csb_v1_util_flow_step(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS,
          "utility accepted import advances to import state");
    CHECK(csb_v1_util_flow_cancel_to_menu(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_SELECT_ACTION &&
          flow.action == CSB_V1_UTIL_ACTION_EXIT,
          "utility cancel returns a pending import action to SELECT_ACTION");
    CHECK(csb_v1_util_flow_retry_error(&flow) == -1,
          "utility retry only applies to ERROR state");

    CHECK(csb_v1_util_flow_move_action_cursor(&flow, 2) == 2 &&
          csb_v1_util_flow_accept_selected_action(&flow) == 0 &&
          flow.action == CSB_V1_UTIL_ACTION_NEW,
          "utility cursor can select NEW after cancel");
    CHECK(csb_v1_util_flow_step(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_ERROR &&
          flow.last_error == -8,
          "utility NEW without imported champions enters recoverable ERROR");
    CHECK(csb_v1_util_flow_retry_error(&flow) == 0 &&
          flow.state == CSB_V1_UTIL_FLOW_SELECT_ACTION &&
          flow.action == CSB_V1_UTIL_ACTION_EXIT &&
          flow.last_error == 0,
          "utility retry after verified-disk ERROR returns to SELECT_ACTION");
}

static void test_utility_panel_layout_owns_visible_hit_area(void)
{
    CSB_V1_UtilFlowContext flow;
    CSB_V1_UtilPanelLayout panel;
    CSB_V1_UtilMenuLayout menu;
    CSB_V1_UtilInputResult point_result;

    csb_v1_util_flow_init(&flow);
    flow.state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
    flow.selected_action_index = 0;

    CHECK(csb_v1_util_flow_menu_layout(&flow, &menu) == 1 &&
          menu.x == 38 &&
          menu.y == 104 &&
          menu.w == 244 &&
          menu.h == 48,
          "utility menu layout keeps the source startup menu rectangle");
    CHECK(csb_v1_util_flow_panel_layout(&flow, 0, &panel) == 1 &&
          panel.import_status_x == 38 &&
          panel.import_status_y == 80 &&
          panel.prompt_x == 38 &&
          panel.prompt_y == 92 &&
          panel.preview_x == 48 &&
          panel.preview_y == 154,
          "utility panel layout owns prompt and preview anchors");
    CHECK(panel.x == 38 && panel.y == 80 && panel.w == 244 &&
          panel.h == 72,
          "utility panel without preview spans prompt plus menu rows");
    CHECK(csb_v1_util_flow_panel_contains_point(&flow, 0, 40, 92) == 1 &&
          csb_v1_util_flow_panel_contains_point(&flow, 0, 40, 151) == 1 &&
          csb_v1_util_flow_panel_contains_point(&flow, 0, 40, 152) == 0,
          "utility panel consumes prompt/menu whitespace only to menu bottom");
    CHECK(csb_v1_util_flow_panel_layout(&flow, 1, &panel) == 1 &&
          panel.h == 114 &&
          panel.preview_row_h == 10 &&
          panel.preview_max_rows == 4,
          "utility panel with preview expands through four champion rows");
    CHECK(csb_v1_util_flow_panel_contains_point(&flow, 1, 52, 164) == 1 &&
          csb_v1_util_flow_panel_contains_point(&flow, 1, 52, 193) == 1 &&
          csb_v1_util_flow_panel_contains_point(&flow, 1, 52, 194) == 0,
          "utility panel consumes preview row clicks without leaking to entrance");
    CHECK(csb_v1_util_flow_handle_point(&flow, 40, 116, 1, &point_result) &&
          point_result.kind == CSB_V1_UTIL_INPUT_RESULT_ACTIVATE &&
          point_result.action == CSB_V1_UTIL_ACTION_LOAD &&
          point_result.selected_action_index == 1 &&
          point_result.preview_active == 0 &&
          flow.selected_action_index == 1,
          "utility pointer row activates through CSB-owned result");
    CHECK(csb_v1_util_flow_handle_point(&flow, 40, 92, 1, &point_result) &&
          point_result.kind == CSB_V1_UTIL_INPUT_RESULT_PANEL_CONSUMED &&
          point_result.selected_action_index == 1 &&
          point_result.preview_active == 1,
          "utility pointer panel whitespace is consumed through CSB-owned result");
    CHECK(!csb_v1_util_flow_handle_point(&flow, 2, 2, 0, &point_result),
          "utility pointer outside panel is ignored by CSB-owned result");
}

static void test_diagnostic_report_surfaces_title_import_status(void)
{
    CSB_V1_BootProfile p;
    char diag[2048];
    size_t n;

    prime_verified_profile(&p);
    /* Capture DM1-baseline diagnostic */
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "engine_version=v2.0") != NULL,
          "diagnostic report surfaces v2.0 baseline before enter_game");
    CHECK(strstr(diag, "engine_version=v2.0") != NULL &&
          strstr(diag, "flipped=NO") != NULL,
          "diagnostic report shows engine_version + flipped=NO before enter_game");
    CHECK(strstr(diag, "cmp_import attempted=NO succeeded=NO") != NULL,
          "diagnostic report shows cmp_import status before any attempt");

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "verified profile enters the CSB runtime");
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "engine_version=v2.1") != NULL,
          "diagnostic report surfaces v2.1 after CSB enter_game");
    CHECK(strstr(diag, "flipped=YES") != NULL,
          "diagnostic report shows flipped=YES after enter_game");

    csb_v1_boot_cleanup(&p);
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "engine_version=v2.0") != NULL,
          "diagnostic report returns to v2.0 after cleanup");
}

static void test_real_startup_asset_selection_rejects_generic_paths(void)
{
    CSB_V1_BootProfile p;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_BootStartupCSBGraphicsPaletteReadiness_PC34 palette_readiness;
    const CSB_V1_StartupAssetBinding_PC34 *binding;

    prime_verified_profile(&p);
    csb_v1_boot_startup_assets_resolve_pc34(&p);
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34);
    CHECK(binding && binding->source == CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
          binding->graphic_index == 1u && binding->verified == 1 &&
          binding->rejects_generic_or_test_asset == 1 &&
          strstr(binding->path, "GRAPHICS.DAT") != NULL,
          "title CHAOS resolves to verified original GRAPHICS.DAT C001");
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34);
    CHECK(binding && binding->graphic_index == 2u && binding->verified == 1,
          "door opening resolves left door from original C002");
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34);
    CHECK(binding && binding->graphic_index == 5u && binding->verified == 1,
          "credits resolves from original C005");
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34);
    CHECK(binding && binding->source == CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
          binding->graphic_index == 17u && binding->verified == 1,
          "inventory HUD resolves to original C017 before any CSBWin override");
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34);
    CHECK(binding && binding->source == CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
          binding->graphic_index == 40u && binding->verified == 1,
          "resurrect HUD resolves to original C040 before any CSBWin override");

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34;
    plan.asset_command_count = 1;
    plan.asset_commands[0].kind = CSB_V1_STARTUP_ASSET_TITLE_SCALED_REGION_PC34;
    plan.asset_commands[0].asset_id = 1;
    plan.asset_commands[0].visible = 1;
    CHECK(csb_v1_boot_startup_render_plan_uses_real_assets_pc34(&p, &plan) == 1,
          "PRESENTS/CHAOS/STRIKES plan admits only the verified title binding");

    p.csbgraphics_cache.loaded = 1;
    snprintf(p.csbgraphics_cache.resolved_path,
             sizeof(p.csbgraphics_cache.resolved_path), "%s", "CSBgraphics.dat");
    snprintf(p.csbgraphics_cache.matched_md5,
             sizeof(p.csbgraphics_cache.matched_md5), "%s",
             "00000000000000000000000000000000");
    p.csbgraphics_runtime_plan.ready = 1;
    p.csbgraphics_runtime_plan.planned_count = 2u;
    p.csbgraphics_runtime_plan.entries[0].entry_index = 17u;
    p.csbgraphics_runtime_plan.entries[1].entry_index = 40u;
    csb_v1_boot_startup_assets_resolve_pc34(&p);
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34);
    CHECK(binding && binding->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34 &&
          binding->graphic_index == 17u && binding->verified == 0 &&
          strcmp(binding->path, "CSBgraphics.dat") == 0,
          "CSBgraphics inventory stays no-draw without a palette receipt");
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34);
    CHECK(binding && binding->source == CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34 &&
          binding->graphic_index == 40u && binding->verified == 0,
          "CSBgraphics resurrection stays no-draw without a palette receipt");
    CHECK(csb_v1_boot_startup_csbgraphics_palette_readiness_pc34(
              &p, &palette_readiness) == 0 &&
          palette_readiness.m11_no_draw_without_palette == 1 &&
          palette_readiness.hud_palette_ready == 0,
          "boot palette readiness exposes M11 no-draw before admission");

    p.csbgraphics_palette_receipt.valid = 1;
    snprintf(p.csbgraphics_palette_receipt.source_path,
             sizeof(p.csbgraphics_palette_receipt.source_path), "%s",
             p.csbgraphics_cache.resolved_path);
    snprintf(p.csbgraphics_palette_receipt.source_md5,
             sizeof(p.csbgraphics_palette_receipt.source_md5), "%s",
             p.csbgraphics_cache.matched_md5);
    p.csbgraphics_palette_receipt.entry_span.entry_index = 41u;
    p.csbgraphics_palette_receipt.entry_span.decompressed_size =
        CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES;
    p.csbgraphics_palette_receipt.decoded_fnv1a = 0x12345678u;
    csb_v1_boot_startup_assets_resolve_pc34(&p);
    CHECK(csb_v1_boot_startup_csbgraphics_palette_readiness_pc34(
              &p, &palette_readiness) == 1 &&
          palette_readiness.palette_receipt_ready &&
          palette_readiness.hud_palette_ready &&
          !palette_readiness.title_palette_ready &&
          !palette_readiness.door_palette_ready &&
          palette_readiness.palette_entry_index == 41u &&
          palette_readiness.palette_decoded_fnv1a == 0x12345678u,
          "matching boot palette receipt reaches the CSBgraphics HUD readiness");
    binding = csb_v1_boot_startup_asset_binding_pc34(
        &p, CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34);
    CHECK(binding && binding->verified == 1,
          "matching palette receipt verifies the selected CSBgraphics HUD binding");
    p.csbgraphics_palette_receipt.source_path[0] = 'x';
    csb_v1_boot_startup_assets_resolve_pc34(&p);
    CHECK(csb_v1_boot_startup_csbgraphics_palette_readiness_pc34(
              &p, &palette_readiness) == 0 &&
          palette_readiness.m11_no_draw_without_palette == 1,
          "mismatched palette path closes the boot-to-runtime receipt");

    p.startup_assets.bindings[CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34].source =
        CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34;
    CHECK(csb_v1_boot_startup_render_plan_uses_real_assets_pc34(&p, &plan) == 0,
          "real-data title plan rejects a generic or test fallback source");
    csb_v1_boot_cleanup(&p);
}

static void test_runtime_asset_gate_binds_session_and_owned_artwork(void)
{
    CSB_V1_BootProfile p;
    CSB_V1_BootStartupLaunchReceipts_PC34 launch;
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 gate;

    prime_verified_profile(&p);
    snprintf(p.graphics_md5, sizeof(p.graphics_md5), "%s",
             "61fbfd56887c8bfe85ba4fb306fc2861");
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    memset(&launch, 0, sizeof(launch));
    launch.session_state.import_selected_action_index = 0;
    csb_v1_boot_startup_assets_resolve_pc34(&p);
    CHECK(csb_v1_boot_startup_runtime_asset_gate_from_launch_receipts_pc34(
              &p, &launch, &gate) == 1 && gate.valid &&
              gate.title_assets_owned && gate.entrance_assets_owned &&
              gate.hud_assets_owned && gate.session_state_valid &&
              gate.rejects_fallback_sources,
          "runtime gate binds verified title, entrance, HUD, and session ownership");

    p.startup_assets.bindings[CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34]
        .source = CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34;
    CHECK(csb_v1_boot_startup_runtime_asset_gate_from_launch_receipts_pc34(
              &p, &launch, &gate) == 0,
          "runtime gate rejects a fallback entrance-door binding");
    csb_v1_boot_startup_assets_resolve_pc34(&p);
    launch.session_state.entrance_resume_available = 1;
    CHECK(csb_v1_boot_startup_runtime_asset_gate_from_launch_receipts_pc34(
              &p, &launch, &gate) == 0,
          "runtime gate rejects a resume session without its owned save path");
    csb_v1_boot_cleanup(&p);
}

static void test_runtime_asset_session_frame_keeps_verified_surfaces_alive(void)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRenderPlan_PC34 plan;
    unsigned char title_pixels[4] = {0};
    unsigned char presents_pixels[4] = {0};
    unsigned char chaos_pixels[4] = {0};
    unsigned char strikes_pixels[4] = {0};
    unsigned char left_pixels[4] = {0};
    unsigned char right_pixels[4] = {0};
    unsigned char entrance_pixels[4] = {0};
    unsigned char credits_pixels[4] = {0};
    unsigned char inventory_pixels[4] = {0};
    unsigned char resurrect_pixels[4] = {0};
    uint32_t original_hud_binding_hash;

    /* This is a decoded-session unit test: the loader is covered at the
     * GRAPHICS.DAT boundary elsewhere.  The frame resolver must never
     * replace these verified source pointers with fallback artwork. */
    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    session.valid = 1;
    session.real_asset_matched = 1;
    session.title_presents_ready = 1;
    session.title_chaos_ready = 1;
    session.title_strikes_back_ready = 1;
    session.entrance_assets_ready = 1;
    session.door_assets_ready = 1;
    session.hud_assets_bound = 1;
    session.full_startup_ready = 1;
    session.rejects_legacy_wrappers = 1;
    session.generation = 9u;
    session.surfaces.valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels = title_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].pixels = presents_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].pixels = chaos_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].pixels = strikes_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].pixels = left_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].pixels = right_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].pixels = entrance_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34].pixels = credits_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].pixels = inventory_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].pixels = resurrect_pixels;

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
    plan.title_source_step = 1;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 41u, &frame) == 1 && frame.valid &&
              frame.title_surface->pixels == title_pixels &&
              frame.left_door_surface->pixels == left_pixels &&
              frame.hud_inventory_surface->pixels == inventory_pixels &&
              frame.hud_resurrect_surface->pixels == resurrect_pixels &&
              frame.real_asset_matched &&
              frame.title_sequence_ready &&
              frame.entrance_ready &&
              frame.door_ready &&
              frame.no_legacy_wrappers &&
              frame.title_phase_mask == 0x01 &&
              frame.frame_route_hash != 0u &&
              frame.uses_verified_hud_bindings && frame.source_tick == 41u &&
              frame.session_generation == 9u,
          "asset session keeps PRESENTS, C017/C040 HUD, and door source surfaces stable");
    original_hud_binding_hash = frame.hud_binding_hash;
    session.hud_source_receipt_hash = 0x13579bdfu;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 41u, &frame) == 1 &&
              frame.hud_source_receipt_hash == 0x13579bdfu &&
              frame.hud_binding_hash != original_hud_binding_hash &&
              frame.frame_route_hash != 0u,
          "hash-admitted CSBgraphics HUD receipt reaches the live frame binding");
    session.hud_source_receipt_hash = 0u;
    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 42u, &frame) == 1 &&
              frame.title_surface->pixels == title_pixels &&
              frame.title_phase_mask == 0x02 &&
              frame.frame_route_hash != 0u &&
              frame.left_door_surface->pixels == left_pixels,
          "asset session advances CHAOS timing without reopening door pixels");
    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 43u, &frame) == 1 &&
              frame.title_surface->pixels == title_pixels &&
              frame.title_phase_mask == 0x08 &&
              frame.frame_route_hash != 0u,
          "asset session selects STRIKES BACK from the original title session");
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 44u, &frame) == 1 &&
              frame.entrance_surface->pixels == credits_pixels &&
              frame.title_phase_mask == 0 &&
              frame.frame_route_hash != 0u &&
              frame.right_door_surface->pixels == right_pixels,
          "asset session carries the same verified doors into entrance credits");
}

static void test_runtime_rasterizer_composes_title_and_opening_from_owned_pixels(void)
{
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    unsigned char title[320 * 80];
    unsigned char entrance[320 * 200];
    unsigned char left[128 * 161];
    unsigned char right[128 * 161];

    memset(&frame, 0, sizeof(frame));
    memset(&plan, 0, sizeof(plan));
    memset(title, 3, sizeof(title));
    memset(entrance, 5, sizeof(entrance));
    memset(left, 7, sizeof(left));
    memset(right, 9, sizeof(right));
    frame.valid = 1;
    frame.real_asset_matched = 1;
    frame.no_legacy_wrappers = 1;
    frame.frame_route_hash = 77u;
    frame.title_surface = &(CSB_V1_StartupRuntimeSurface_PC34){
        .pixels = title,
        .width = 320,
        .height = 80,
        .source_asset_id = 1,
        .valid = 1,
        .transparent_color = -1,
        .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34};
    plan.surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
    plan.title_source_x = 0;
    plan.title_source_y = 0;
    plan.title_source_w = 320;
    plan.title_source_h = 80;
    plan.title_dest_x = 136;
    plan.title_dest_y = 74;
    plan.title_dest_w = 48;
    plan.title_dest_h = 12;
    plan.title_transparent_color = -1;
    CHECK(csb_v1_boot_startup_runtime_frame_rasterize_pc34(
              &frame, &plan, &raster) == 1 && raster.valid &&
              raster.title_composited && !raster.entrance_composited &&
              raster.source_surface_count == 1 && raster.pixel_hash != 0u &&
              raster.pixels[74 * 320 + 136] == 3 && raster.pixels[0] == 0,
          "runtime rasterizer centers ReDMCSB CHAOS-scale pixels on a black title frame");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);

    frame.entrance_surface = &(CSB_V1_StartupRuntimeSurface_PC34){
        .pixels = entrance,
        .width = 320,
        .height = 200,
        .source_asset_id = 4,
        .valid = 1,
        .transparent_color = -1,
        .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34};
    frame.left_door_surface = &(CSB_V1_StartupRuntimeSurface_PC34){
        .pixels = left,
        .width = 128,
        .height = 161,
        .source_asset_id = 2,
        .valid = 1,
        .transparent_color = -1,
        .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34};
    frame.right_door_surface = &(CSB_V1_StartupRuntimeSurface_PC34){
        .pixels = right,
        .width = 128,
        .height = 161,
        .source_asset_id = 3,
        .valid = 1,
        .transparent_color = -1,
        .source_kind = CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34};
    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.closed_left_w = 105;
    plan.closed_left_h = 161;
    plan.closed_left_dest_y = 28;
    plan.closed_right_w = 127;
    plan.closed_right_h = 161;
    plan.closed_right_dest_x = 105;
    plan.closed_right_dest_y = 28;
    CHECK(csb_v1_boot_startup_runtime_frame_rasterize_pc34(
              &frame, &plan, &raster) == 1 && raster.valid &&
              raster.entrance_composited && raster.door_composited &&
              raster.source_surface_count == 3 && raster.pixels[0] == 5 &&
              raster.pixels[28 * 320] == 7 && raster.pixels[28 * 320 + 105] == 9,
          "runtime rasterizer composites the closed C002/C003 entrance over C004");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);

    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34;
    CHECK(csb_v1_boot_startup_runtime_frame_rasterize_pc34(
              &frame, &plan, &raster) == 1 && raster.valid &&
              raster.entrance_composited && raster.door_composited &&
              raster.source_surface_count == 3 && raster.pixels[0] == 5 &&
              raster.pixels[28 * 320] == 7 && raster.pixels[28 * 320 + 105] == 9,
          "runtime rasterizer keeps the real closed C002/C003 entrance visible during door delay");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34;
    plan.opening_composite_valid = 1;
    plan.opening_left_w = 8;
    plan.opening_left_h = 161;
    plan.opening_left_dest_y = 28;
    plan.opening_right_w = 8;
    plan.opening_right_h = 161;
    plan.opening_right_dest_x = 224;
    plan.opening_right_dest_y = 28;
    CHECK(csb_v1_boot_startup_runtime_frame_rasterize_pc34(
              &frame, &plan, &raster) == 1 && raster.valid &&
              raster.entrance_composited && raster.door_composited &&
              raster.source_surface_count == 3 && raster.pixels[0] == 5 &&
              raster.pixels[28 * 320] == 7 && raster.pixels[28 * 320 + 224] == 9,
          "runtime rasterizer composites verified C004/C002/C003 opening-door pixels without callbacks");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    plan.opening_right_source_x = 127;
    plan.opening_right_w = 8;
    CHECK(csb_v1_boot_startup_runtime_frame_rasterize_pc34(
              &frame, &plan, &raster) == 0 && !raster.valid &&
              raster.pixels == NULL,
          "runtime rasterizer rejects an opening frame with an unreadable verified door strip");
}

static void test_verified_session_owns_swoosh_title_audio_and_hud_handoff(void)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupAudioAction_PC34 audio_action;
    CSB_V1_StartupFullRuntimeReceipt_PC34 full_runtime;
    unsigned char title_pixels[4] = {0};
    unsigned char presents_pixels[4] = {0};
    unsigned char chaos_pixels[4] = {0};
    unsigned char strikes_pixels[4] = {0};
    unsigned char left_pixels[4] = {0};
    unsigned char right_pixels[4] = {0};
    unsigned char entrance_pixels[4] = {0};
    unsigned char credits_pixels[4] = {0};
    unsigned char inventory_pixels[4] = {0};
    unsigned char resurrect_pixels[4] = {0};

    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    session.valid = 1;
    session.real_asset_matched = 1;
    session.title_assets_ready = 1;
    session.title_presents_ready = 1;
    session.title_chaos_ready = 1;
    session.title_strikes_back_ready = 1;
    session.entrance_assets_ready = 1;
    session.door_assets_ready = 1;
    session.hud_assets_bound = 1;
    session.full_startup_ready = 1;
    session.rejects_legacy_wrappers = 1;
    session.generation = 31u;
    session.surfaces.valid = 1;
    session.surfaces.title_regions_ready = 1;
    session.surfaces.opening_frame_ready = 1;
    session.surfaces.hud_surfaces_ready = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels = title_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].pixels = presents_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].pixels = chaos_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].pixels = strikes_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].pixels = left_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].pixels = right_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].pixels = entrance_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34].pixels = credits_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].pixels = inventory_pixels;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid = 1;
    session.surfaces.surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].pixels = resurrect_pixels;

    CHECK(csb_v1_boot_startup_playback_begin_pc34(&session, &audio_action) == 1 &&
              audio_action == CSB_V1_STARTUP_AUDIO_ACTION_PLAY_FTL_SWOOSH_PC34 &&
              session.playback.no_fallback_routes,
          "verified session begins FTL swoosh with fallback routes closed");
    CHECK(csb_v1_boot_startup_playback_complete_swoosh_pc34(&session, &audio_action) == 1 &&
              audio_action == CSB_V1_STARTUP_AUDIO_ACTION_RELEASE_FTL_SWOOSH_PC34,
          "swoosh release enters the original title transaction");
    {
        CSB_V1_StartupRenderState_PC34 title_state;

        memset(&title_state, 0, sizeof(title_state));
        title_state.entrance_active = 1;
        title_state.title_active = 1;
        title_state.title_frame = 0;
        CHECK(csb_v1_startup_source_render_plan_from_state_pc34(
                  &title_state, &plan) == 1 &&
                  csb_v1_boot_startup_playback_accepts_title_plan_pc34(
                      &session, &plan, 0) == 1,
              "F0437 admits the current PRESENTS C001 plan after the swoosh");
        plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34;
        CHECK(csb_v1_boot_startup_playback_accepts_title_plan_pc34(
                  &session, &plan, 0) == 0 &&
                  session.playback.stage ==
                      CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34 &&
                  session.playback.title_phase_mask == 0,
              "a stale transfer plan cannot fast-forward title playback into Entrance");
    }
    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 0, &plan, &audio_action) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              plan.title_source_step == 1 &&
              audio_action == CSB_V1_STARTUP_AUDIO_ACTION_NONE_PC34,
          "PRESENTS keeps its original 60-vblank route");
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 1u, &frame) == 1 &&
              frame.title_surface->pixels == title_pixels,
          "PRESENTS render plan retains the resident verified title surface");
    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 60, &plan, &audio_action) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 2,
          "CHAOS enters through the original first zoom step");
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 2u, &frame) == 1 &&
              frame.title_surface->pixels == title_pixels,
          "CHAOS render plan retains the resident verified title surface");
    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 80, &plan, &audio_action) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 21,
          "CHAOS hold at step 21 remains within the CHAOS zoom stage");
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 3u, &frame) == 1 &&
              frame.title_surface->pixels == title_pixels,
          "completed CHAOS hold retains the resident verified title surface");
    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 100, &plan, &audio_action) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              plan.title_source_step == 22,
          "STRIKES BACK follows the completed CHAOS hold without a synthetic frame");
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 4u, &frame) == 1 &&
              frame.title_surface->pixels == title_pixels,
          "STRIKES BACK render plan retains the resident verified title surface");
    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, csb_v1_startup_title_total_ticks_pc34(), &plan,
              &audio_action) == 1 &&
              audio_action == CSB_V1_STARTUP_AUDIO_ACTION_PLAY_ENTRANCE_MUSIC_PC34 &&
              session.playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34,
          "title completion hands original entrance music to the owned entrance session");
    CHECK(csb_v1_boot_startup_playback_enter_hud_pc34(&session) == 1 &&
              session.playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34,
          "entrance hands the same verified session to the runtime HUD");
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session, &full_runtime) == 1 &&
              full_runtime.playback_route_ready &&
              full_runtime.playback_reaches_title &&
              full_runtime.playback_reaches_entrance &&
              full_runtime.playback_reaches_hud &&
              full_runtime.title_to_hud_same_session &&
              full_runtime.playback_route_hash != 0u,
          "full runtime receipt proves one verified CSB session reaches title entrance and HUD");
    session.hud_assets_bound = 0;
    CHECK(csb_v1_boot_startup_playback_begin_pc34(&session, &audio_action) == 0,
          "fallback or unowned HUD state cannot open a replacement startup route");
}

static void test_pointer_quit_row_uses_entrance_without_utility_overlay(void)
{
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupActionReceipt_PC34 receipt;

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot.utility_overlay_active = 0;
    snapshot.utility_selected_action_index = 0;

    CHECK(csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
              &snapshot, 245, 112, 1u, &receipt) == 1 &&
              receipt.handled == 1 &&
              receipt.input_routed_to_entrance == 1 &&
              receipt.input_routed_to_utility == 0 &&
              receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34 &&
              receipt.input_requests_launcher_return == 1 &&
              receipt.host_input_result ==
                  CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34,
          "normal CSB entrance quit row is not stolen by utility panel routing");
}

static void test_pointer_load_row_stays_utility_when_overlay_active(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupActionReceipt_PC34 receipt;

    prime_verified_profile(&profile);
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot.utility_overlay_active = 1;
    snapshot.utility_selected_action_index = 0;
    snapshot.utility_imported_champion_count = 2;
    snapshot.boot_profile = &profile;

    CHECK(csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
              &snapshot, 40, 118, 1u, &receipt) == 1 &&
              receipt.handled == 1 &&
              receipt.input_routed_to_utility == 1 &&
              receipt.entrance_command_id ==
                  CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 &&
              receipt.input_requests_launcher_return == 0,
          "CSB utility overlay still owns its LOAD row");
    csb_v1_boot_cleanup(&profile);
}

static void test_source_evidence(void)
{
    const char *e = csb_v1_boot_source_evidence();
    CHECK(e && strstr(e, "ENTRANCE.C F0806") != NULL,
          "boot source evidence cites ENTRANCE.C F0806");
    CHECK(e && strstr(e, "LOADSAVE.C F0435") != NULL,
          "boot source evidence cites LOADSAVE.C F0435");
}

static void test_verified_dungeon_ingress_projection(void)
{
    CSB_V1_FirstLiveDungeonFrameReceipt_PC34 receipt;
    CSB_V1_ViewportVerifiedDungeonIngressPc34 ingress;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.real_asset_matched = 1;
    receipt.terminal_session_owned = 1;
    receipt.viewport_frame_consumed = 1;
    receipt.no_synthetic_surface = 1;
    receipt.session_generation = 7u;
    receipt.source_tick = 42u;
    CHECK(csb_v1_boot_project_verified_dungeon_ingress_pc34(&receipt, &ingress) == 1 &&
              ingress.valid && ingress.session_generation == 7u &&
              ingress.source_tick == 42u,
          "verified first-live receipt projects only ingress facts");
    receipt.no_synthetic_surface = 0;
    CHECK(csb_v1_boot_project_verified_dungeon_ingress_pc34(&receipt, &ingress) == 0 &&
              !ingress.valid,
          "incomplete first-live receipt cannot project ingress facts");
}

static void test_declared_live_material_route_rejects_without_declaration(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_FirstLiveDungeonFrameReceipt_PC34 receipt;
    CSB_V1_ViewportLiveFrameProgressionPc34 progression;
    CSB_V1_ViewportLiveDungeonSelectionPc34 selection;
    CSB_V1_ViewportOperatorDeclarationManifestPc34 manifest;
    uint8_t framebuffer[320 * 200];

    memset(&profile, 0, sizeof(profile));
    memset(&receipt, 0, sizeof(receipt));
    memset(&progression, 0, sizeof(progression));
    memset(&manifest, 0, sizeof(manifest));
    memset(framebuffer, 0, sizeof(framebuffer));
    receipt.valid = receipt.real_asset_matched = receipt.terminal_session_owned = 1;
    receipt.viewport_frame_consumed = receipt.no_synthetic_surface = 1;
    receipt.session_generation = 1u;
    selection.valid = 1;
    CHECK(csb_v1_boot_render_manifest_live_material_pc34(
              &profile, &receipt, &manifest, NULL, 0, 0, &progression, NULL,
              framebuffer, 320, 200, &selection) == 0 && !selection.valid,
          "boot manifest live material route clears stale selection without admitted manifest");
}

int main(void)
{
    printf("=== CSB V1 Title/Import Startup UI Gate ===\n\n");
    test_profile_init_resets_engine_version_to_dm1();
    test_enter_game_flips_engine_version_to_csb();
    test_cleanup_resets_engine_version_to_dm1();
    test_enter_game_without_assets_leaves_version_at_dm1();
    test_cmp_import_cannot_create_boot_party();
    test_cmp_import_bad_magic_rejected();
    test_mark_imported_party_ready_uses_existing_party();
    test_utility_action_cursor_drives_select_action();
    test_utility_panel_layout_owns_visible_hit_area();
    test_diagnostic_report_surfaces_title_import_status();
    test_real_startup_asset_selection_rejects_generic_paths();
    test_runtime_asset_gate_binds_session_and_owned_artwork();
    test_runtime_asset_session_frame_keeps_verified_surfaces_alive();
    test_runtime_rasterizer_composes_title_and_opening_from_owned_pixels();
    test_verified_session_owns_swoosh_title_audio_and_hud_handoff();
    test_pointer_quit_row_uses_entrance_without_utility_overlay();
    test_pointer_load_row_stays_utility_when_overlay_active();
    test_verified_dungeon_ingress_projection();
    test_declared_live_material_route_rejects_without_declaration();
    test_source_evidence();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
