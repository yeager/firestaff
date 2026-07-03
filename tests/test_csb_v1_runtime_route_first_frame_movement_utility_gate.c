/*
 * CSB V1 Runtime Route Gate
 *
 * Bounded route proof for the current CSB V1 gap:
 * Utility import -> NEW_GAME boundary, verified boot/runtime handoff,
 * first viewport frame, one queued movement command, and a second viewport
 * render from the moved runtime state, followed by a bounded save-prefix
 * roundtrip of that moved route state.
 *
 * Non-claim: this is not full CSB playability, full save compatibility,
 * original capture, or pixel parity. It joins already source-locked slices
 * into one executable route so the next gap is narrower.
 */

#include "csb_v1_boot.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_movement_command_step_runtime_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "csb_v1_utility_import_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define ROUTE_MKDIR(path) _mkdir(path)
#else
#define ROUTE_MKDIR(path) mkdir((path), 0700)
#endif

#define ROUTE_FB_WIDTH 320
#define ROUTE_FB_HEIGHT 200
#define ROUTE_VIEWPORT_H 136
#define ROUTE_PATH_MAX 1024

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, msg) do { \
    int got_value = (int)(got); \
    int want_value = (int)(want); \
    if (got_value == want_value) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s got=%d want=%d\n", msg, got_value, want_value); } \
} while (0)

static void put_le16(uint8_t *p, int value)
{
    p[0] = (uint8_t)(value & 0xff);
    p[1] = (uint8_t)((value >> 8) & 0xff);
}

static void put_le32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value & 0xffu);
    p[1] = (uint8_t)((value >> 8) & 0xffu);
    p[2] = (uint8_t)((value >> 16) & 0xffu);
    p[3] = (uint8_t)((value >> 24) & 0xffu);
}

static int write_route_dungeon(const char *path)
{
    enum { width = 8, height = 8, header = 10 };
    uint8_t buf[header + width * height * 2];
    FILE *f;
    size_t n;
    int x;
    int y;

    memset(buf, 0, sizeof(buf));
    put_le16(buf, 1);          /* level_count */
    put_le16(buf + 2, 16);     /* legacy padding */
    buf[4] = width;
    buf[5] = height;
    put_le32(buf + 6, header);

    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            size_t off = header + (size_t)(x * height + y) * 2u;
            uint8_t square = 2; /* floor */
            if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                square = 1; /* wall */
            }
            if (x == 5 && y == 3) {
                square = 1; /* visible wall after the first north step */
            }
            buf[off] = square;
            buf[off + 1] = 0;
        }
    }

    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1u, sizeof(buf), f);
    fclose(f);
    return n == sizeof(buf) ? 0 : -1;
}

static int write_route_graphics_stub(const char *path)
{
    static const uint8_t bytes[] = { 'C', 'S', 'B', 'R', 'T', 0 };
    FILE *f = fopen(path, "wb");
    size_t n;
    if (!f) return -1;
    n = fwrite(bytes, 1u, sizeof(bytes), f);
    fclose(f);
    return n == sizeof(bytes) ? 0 : -1;
}

static void seed_dm1_record(uint8_t *record, const char *name, int hp)
{
    int slot;
    memset(record, 0, CSB_V1_DM1_CHAMP_SIZE);
    memcpy(record + CSB_V1_DM1_CHAMP_OFF_NAME, name, 8);
    put_le16(record + CSB_V1_DM1_CHAMP_OFF_HEALTH, hp);
    put_le16(record + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH, hp + 20);
    put_le16(record + CSB_V1_DM1_CHAMP_OFF_STAMINA, 70);
    put_le16(record + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA, 100);
    put_le16(record + CSB_V1_DM1_CHAMP_OFF_MANA, 35);
    put_le16(record + CSB_V1_DM1_CHAMP_OFF_MAX_MANA, 50);
    record[CSB_V1_DM1_CHAMP_OFF_STR] = 55;
    record[CSB_V1_DM1_CHAMP_OFF_DEX] = 66;
    record[CSB_V1_DM1_CHAMP_OFF_WIS] = 77;
    record[CSB_V1_DM1_CHAMP_OFF_VIT] = 88;
    for (slot = 0; slot < CSB_V1_SLOT_COUNT; ++slot) {
        put_le16(record + CSB_V1_DM1_CHAMP_OFF_EQUIP + slot * 2, 0xffff);
    }
}

static int write_route_dm1_save(const char *path)
{
    uint8_t buf[1024];
    FILE *f;
    size_t n;

    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 2;
    seed_dm1_record(buf + CSB_V1_DM1_HDR_CHAMPION_START,
                    "ALPHA   ", 90);
    seed_dm1_record(buf + CSB_V1_DM1_HDR_CHAMPION_START +
                    CSB_V1_DM1_CHAMP_SIZE,
                    "BETA    ", 85);

    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1u, sizeof(buf), f);
    fclose(f);
    return n == sizeof(buf) ? 0 : -1;
}

static void run_utility_import_to_new_game(const char *save_path,
                                           CSB_V1_PartyState *party)
{
    CSB_V1_UtilFlowContext ctx;

    csb_v1_util_flow_init(&ctx);
    csb_v1_util_flow_set_dm1_path(&ctx, save_path);
    ctx.state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;

    CHECK_EQ(csb_v1_util_flow_step(&ctx), 0,
             "Utility import step accepts the DM1 save");
    CHECK_EQ(ctx.state, CSB_V1_UTIL_FLOW_CONFIRM_IMPORT,
             "Utility flow reaches CONFIRM_IMPORT");
    csb_v1_util_flow_confirm_import(&ctx, 1);
    CHECK_EQ(csb_v1_util_flow_step(&ctx), 0,
             "Utility confirmed import reaches NEW_GAME");
    CHECK_EQ(ctx.state, CSB_V1_UTIL_FLOW_NEW_GAME,
             "Utility flow is at NEW_GAME boundary");
    CHECK_EQ(csb_v1_util_flow_step(&ctx), 1,
             "Utility NEW_GAME boundary completes");
    CHECK_EQ(ctx.state, CSB_V1_UTIL_FLOW_DONE,
             "Utility flow reports DONE after NEW_GAME");
    CHECK_EQ(csb_v1_util_flow_get_party(&ctx, party), 2,
             "Utility handoff exposes imported champion count");
}

static void seed_runtime_party_from_utility(CSB_V1_PartyState *party)
{
    int i;

    for (i = 0; i < party->ChampionCount && i < CSB_V1_MAX_CHAMPIONS; ++i) {
        csb_v1_champion_init(&party->Champions[i]);
        snprintf(party->Champions[i].Name, sizeof(party->Champions[i].Name),
                 "ROUTE%d", i + 1);
        party->Champions[i].CurrentHealth = (int16_t)(90 + i);
        party->Champions[i].MaximumHealth = (int16_t)(120 + i);
        party->Champions[i].Cell = (uint8_t)i;
        party->Champions[i].Direction = CSB_V1_START_PARTY_DIR;
    }
    party->LeaderIndex = 0;
    party->MagicCasterIndex = -1;
    party->ImportedFromDM1 = 1;
    party->PartyMapX = CSB_V1_START_PARTY_X;
    party->PartyMapY = CSB_V1_START_PARTY_Y;
    party->PartyDirection = CSB_V1_START_PARTY_DIR;
}

static void snapshot_grid(uint8_t out_grid[32 * 32])
{
    const CSB_V1_DungeonData *dun = csb_v1_dungeon_get_current();
    int level = csb_v1_dungeon_get_current_level();
    int x;
    int y;

    if (level < 0) level = 0;
    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {
            int square = csb_v1_dungeon_get_square_type(dun, level, x, y);
            out_grid[y * 32 + x] = (uint8_t)(square < 0 ? 1 : square);
        }
    }
}

static int count_non_baseline(const unsigned char *fb, unsigned char baseline)
{
    int count = 0;
    int i;
    for (i = 0; i < ROUTE_FB_WIDTH * ROUTE_VIEWPORT_H; ++i) {
        if (fb[i] != baseline) ++count;
    }
    return count;
}

static int panel_rows_are_baseline(const unsigned char *fb,
                                   unsigned char baseline)
{
    int i;
    for (i = ROUTE_VIEWPORT_H * ROUTE_FB_WIDTH;
         i < ROUTE_FB_WIDTH * ROUTE_FB_HEIGHT; ++i) {
        if (fb[i] != baseline) return 0;
    }
    return 1;
}

static int render_route_frame(const CSB_V1_RuntimeProfile *runtime,
                              unsigned char *fb,
                              int *out_touched)
{
    CSB_V1_ViewportConfig cv;
    uint8_t grid[32 * 32];

    memset(fb, 0x07, ROUTE_FB_WIDTH * ROUTE_FB_HEIGHT);
    snapshot_grid(grid);
    csb_v1_viewport_init(&cv);
    cv.viewport_pixels = fb;
    cv.viewport_stride = ROUTE_FB_WIDTH;
    cv.dungeon_grid = grid;
    cv.dungeon_width = 32;
    cv.dungeon_height = 32;
    cv.wall_set_index = 0;
    cv.custom_background = 0;
    cv.prison_door_open = 100;
    csb_v1_viewport_render_frame(&cv,
                                 runtime->party_dir,
                                 runtime->party_x,
                                 runtime->party_y);
    if (out_touched) {
        *out_touched = count_non_baseline(fb, 0x07);
    }
    return cv.viewport_pixels == fb &&
           panel_rows_are_baseline(fb, 0x07);
}

static int route_wall_probe(const CSB_V1_RuntimeProfile *profile,
                            int map_x,
                            int map_y,
                            void *context)
{
    const CSB_V1_DungeonData *dun = (const CSB_V1_DungeonData *)context;
    int square;
    (void)profile;
    square = csb_v1_dungeon_get_square_type(
        dun, csb_v1_dungeon_get_current_level(), map_x, map_y);
    return square < 0 || square == 1;
}

static void enqueue_and_process_route_command(
    CSB_V1_RuntimeProfile *runtime,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int command,
    void *wall_context,
    int expected_applied,
    int expected_blocked,
    int expected_x,
    int expected_y,
    const char *label)
{
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat result;

    CHECK_EQ(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
                 queue, command, 0, 0),
             1,
             label);
    CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                 runtime,
                 queue,
                 0,
                 0,
                 0,
                 route_wall_probe,
                 wall_context,
                 &result),
             1,
             label);
    CHECK_EQ(result.step_applied, expected_applied,
             label);
    CHECK_EQ(result.blocked_by_wall, expected_blocked,
             label);
    CHECK_EQ(runtime->party_x, expected_x,
             label);
    CHECK_EQ(runtime->party_y, expected_y,
             label);
    CHECK_EQ(runtime->party_state.PartyMapX, expected_x,
             label);
    CHECK_EQ(runtime->party_state.PartyMapY, expected_y,
             label);
    CHECK_EQ(queue->count, 0,
             label);
}

int main(void)
{
    const char *tmp_dir = "/tmp/firestaff-csb-v1-runtime-route-gate";
    char dungeon_path[ROUTE_PATH_MAX];
    char graphics_path[ROUTE_PATH_MAX];
    char dm1_save_path[ROUTE_PATH_MAX];
    char route_save_path[ROUTE_PATH_MAX];
    CSB_V1_BootProfile boot;
    CSB_V1_BootProfile loaded_boot;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_SaveHeader loaded_header;
    uint16_t route_game_id;
    unsigned char fb_before[ROUTE_FB_WIDTH * ROUTE_FB_HEIGHT];
    unsigned char fb_after[ROUTE_FB_WIDTH * ROUTE_FB_HEIGHT];
    unsigned char fb_final[ROUTE_FB_WIDTH * ROUTE_FB_HEIGHT];
    unsigned char fb_reloaded[ROUTE_FB_WIDTH * ROUTE_FB_HEIGHT];
    int before_touched = 0;
    int after_touched = 0;
    int final_touched = 0;
    int reloaded_touched = 0;

    printf("=== CSB V1 runtime route first-frame/movement/Utility gate ===\n\n");

    (void)ROUTE_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    snprintf(dm1_save_path, sizeof(dm1_save_path), "%s/DMSAVE.DAT", tmp_dir);
    snprintf(route_save_path, sizeof(route_save_path), "%s/CSBGAME_ROUTE.FSAV", tmp_dir);

    CHECK_EQ(write_route_dungeon(dungeon_path), 0,
             "route DUNGEON.DAT fixture written");
    CHECK_EQ(write_route_graphics_stub(graphics_path), 0,
             "route GRAPHICS.DAT stub written");
    CHECK_EQ(write_route_dm1_save(dm1_save_path), 0,
             "route DM1 save fixture written");

    memset(&party, 0, sizeof(party));
    run_utility_import_to_new_game(dm1_save_path, &party);
    seed_runtime_party_from_utility(&party);

    memset(&boot, 0, sizeof(boot));
    csb_v1_boot_profile_init(&boot);
    snprintf(boot.asset_root, sizeof(boot.asset_root), "%s", tmp_dir);
    snprintf(boot.dungeon_path, sizeof(boot.dungeon_path), "%s", dungeon_path);
    snprintf(boot.graphics_path, sizeof(boot.graphics_path), "%s", graphics_path);
    snprintf(boot.dungeon_md5, sizeof(boot.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de");
    snprintf(boot.graphics_md5, sizeof(boot.graphics_md5),
             "61fbfd56887c94adc26888a9491c6611");
    boot.dungeon_verified = 1;
    boot.graphics_verified = 1;
    boot.assets_verified = 1;
    boot.variant_id = CSB_V1_VARIANT_PC34_EN;
    boot.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    boot.entrance_map_index = 255U;
    boot.start_map_index = 0U;

    CHECK_EQ(csb_v1_boot_enter_game(&boot), 0,
             "verified profile enters CSB runtime");
    CHECK(boot.runtime.dungeon_handle != NULL,
          "runtime owns a loaded dungeon handle");
    CHECK_EQ(csb_v1_runtime_set_party_state(&boot.runtime, &party), 0,
             "Utility-imported party metadata enters runtime");
    CHECK_EQ(boot.runtime.leader_index, 0,
             "runtime selects the imported leader");
    CHECK_EQ(boot.runtime.party_x, CSB_V1_START_PARTY_X,
             "runtime starts at CSB start x");
    CHECK_EQ(boot.runtime.party_y, CSB_V1_START_PARTY_Y,
             "runtime starts at CSB start y");

    CHECK(render_route_frame(&boot.runtime, fb_before, &before_touched),
          "first viewport frame renders without panel bleed");
    CHECK(before_touched > 0,
          "first viewport frame touches the viewport band");

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    enqueue_and_process_route_command(&boot.runtime,
                                      &queue,
                                      DM1_V1_COMMAND_MOVE_FORWARD,
                                      boot.runtime.dungeon_handle,
                                      1,
                                      0,
                                      CSB_V1_START_PARTY_X,
                                      CSB_V1_START_PARTY_Y - 1,
                                      "route processes first forward movement");

    CHECK(render_route_frame(&boot.runtime, fb_after, &after_touched),
          "post-movement viewport frame renders without panel bleed");
    CHECK(after_touched > 0,
          "post-movement viewport frame touches the viewport band");
    CHECK(count_non_baseline(fb_before, 0x07) == before_touched,
          "first frame evidence remains stable after movement dispatch");

    enqueue_and_process_route_command(&boot.runtime,
                                      &queue,
                                      DM1_V1_COMMAND_MOVE_FORWARD,
                                      boot.runtime.dungeon_handle,
                                      0,
                                      1,
                                      CSB_V1_START_PARTY_X,
                                      CSB_V1_START_PARTY_Y - 1,
                                      "route preserves position on wall-blocked forward");
    enqueue_and_process_route_command(&boot.runtime,
                                      &queue,
                                      DM1_V1_COMMAND_MOVE_RIGHT,
                                      boot.runtime.dungeon_handle,
                                      1,
                                      0,
                                      CSB_V1_START_PARTY_X + 1,
                                      CSB_V1_START_PARTY_Y - 1,
                                      "route processes right strafe after blocked move");
    enqueue_and_process_route_command(&boot.runtime,
                                      &queue,
                                      DM1_V1_COMMAND_MOVE_BACKWARD,
                                      boot.runtime.dungeon_handle,
                                      1,
                                      0,
                                      CSB_V1_START_PARTY_X + 1,
                                      CSB_V1_START_PARTY_Y,
                                      "route processes backward move");
    enqueue_and_process_route_command(&boot.runtime,
                                      &queue,
                                      DM1_V1_COMMAND_MOVE_LEFT,
                                      boot.runtime.dungeon_handle,
                                      1,
                                      0,
                                      CSB_V1_START_PARTY_X,
                                      CSB_V1_START_PARTY_Y,
                                      "route returns to the starting cell");

    CHECK(render_route_frame(&boot.runtime, fb_final, &final_touched),
          "final viewport frame renders after multi-step route");
    CHECK(final_touched > 0,
          "final viewport frame touches the viewport band");
    boot.runtime.magic_caster_index = 1;
    boot.runtime.party_state.MagicCasterIndex = 1;
    csb_v1_runtime_tick(&boot.runtime, CSB_V1_TICK_MS_NOMINAL * 2u);
    CHECK_EQ(boot.runtime.game_time, 2,
             "route advances runtime game time before save");

    memset(&loaded_boot, 0, sizeof(loaded_boot));
    memset(&loaded_header, 0, sizeof(loaded_header));
    route_game_id = boot.runtime.dungeon_game_id ?
                    boot.runtime.dungeon_game_id : 0x1234u;
    CHECK_EQ(csb_v1_runtime_save_game_to_path(&boot.runtime,
                                              route_save_path),
             CSB_V1_SAVE_OK,
             "runtime save writes multi-step route state through CSB save path");
    CHECK_EQ(csb_v1_save_verify_compatible(route_save_path,
                                           CSB_V1_SAVE_MAGIC_CSB,
                                           route_game_id),
             CSB_V1_LOAD_OK,
             "runtime save verifies CSB magic/game id");
    CHECK_EQ(csb_v1_load_game(route_save_path,
                              NULL,
                              0,
                              &loaded_header),
             CSB_V1_LOAD_OK,
             "runtime save supports header-only CSB load");
    CHECK_EQ(loaded_header.Magic, CSB_V1_SAVE_MAGIC_CSB,
             "runtime save header keeps CSB magic");
    CHECK_EQ(loaded_header.GameID, route_game_id,
             "runtime save header keeps game id");
    CHECK_EQ(loaded_header.PartyMapY, boot.runtime.party_y,
             "runtime save header follows final y");
    CHECK_EQ(loaded_header.ChampionCount, boot.runtime.champion_count,
             "runtime save header follows imported champion count");

    csb_v1_boot_profile_init(&loaded_boot);
    snprintf(loaded_boot.asset_root, sizeof(loaded_boot.asset_root), "%s", tmp_dir);
    snprintf(loaded_boot.dungeon_path, sizeof(loaded_boot.dungeon_path), "%s", dungeon_path);
    snprintf(loaded_boot.graphics_path, sizeof(loaded_boot.graphics_path), "%s", graphics_path);
    snprintf(loaded_boot.dungeon_md5, sizeof(loaded_boot.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de");
    snprintf(loaded_boot.graphics_md5, sizeof(loaded_boot.graphics_md5),
             "61fbfd56887c94adc26888a9491c6611");
    loaded_boot.dungeon_verified = 1;
    loaded_boot.graphics_verified = 1;
    loaded_boot.assets_verified = 1;
    loaded_boot.variant_id = CSB_V1_VARIANT_PC34_EN;
    loaded_boot.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    loaded_boot.entrance_map_index = 255U;
    loaded_boot.start_map_index = 0U;

    CHECK_EQ(csb_v1_boot_enter_game(&loaded_boot), 0,
             "fresh verified profile enters CSB runtime before reload");
    CHECK_EQ(csb_v1_runtime_load_game_from_path(&loaded_boot.runtime,
                                                route_save_path),
             CSB_V1_LOAD_OK,
             "runtime save reloads into a fresh CSB runtime profile");
    CHECK_EQ(loaded_boot.runtime.party_x, boot.runtime.party_x,
             "runtime reload preserves final x");
    CHECK_EQ(loaded_boot.runtime.party_y, boot.runtime.party_y,
             "runtime reload preserves final y");
    CHECK_EQ(loaded_boot.runtime.party_dir, boot.runtime.party_dir,
             "runtime reload preserves facing");
    CHECK_EQ(loaded_boot.runtime.champion_count, boot.runtime.champion_count,
             "runtime reload preserves champion count");
    CHECK_EQ(loaded_boot.runtime.leader_index, boot.runtime.leader_index,
             "runtime reload preserves leader index");
    CHECK_EQ(loaded_boot.runtime.magic_caster_index,
             boot.runtime.magic_caster_index,
             "runtime reload preserves magic caster index");
    CHECK_EQ(loaded_boot.runtime.game_time, boot.runtime.game_time,
             "runtime reload preserves game time");
    CHECK_EQ((uint32_t)loaded_boot.runtime.total_play_ms,
             (uint32_t)boot.runtime.total_play_ms,
             "runtime reload preserves total play time");
    CHECK_EQ(loaded_boot.runtime.party_state.PartyMapX, boot.runtime.party_x,
             "runtime reload reanchors party-state x");
    CHECK_EQ(loaded_boot.runtime.party_state.PartyMapY, boot.runtime.party_y,
             "runtime reload reanchors party-state y");
    CHECK_EQ(loaded_boot.runtime.party_state.PartyDirection,
             boot.runtime.party_dir,
             "runtime reload reanchors party-state direction");
    CHECK_EQ(loaded_boot.runtime.party_state.MagicCasterIndex,
             boot.runtime.magic_caster_index,
             "runtime reload reanchors party-state caster index");
    CHECK(render_route_frame(&loaded_boot.runtime,
                             fb_reloaded,
                             &reloaded_touched),
          "reloaded runtime viewport frame renders");
    CHECK(reloaded_touched > 0,
          "reloaded runtime viewport frame touches the viewport band");

    csb_v1_boot_cleanup(&loaded_boot);
    csb_v1_boot_cleanup(&boot);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "route cleanup clears current dungeon");
    remove(route_save_path);

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 bounded route joins Utility NEW_GAME, runtime boot, first viewport frame, repeated queued movement with a blocked wall step, post-route render, runtime save/reload, and reloaded render");
        puts("nonClaim=not full CSB playability, full save compatibility, original capture, or pixel parity");
    }
    return failed == 0 ? 0 : 1;
}
