#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_original_save_classifier.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "entrance_frontend_pc34_compat.h"
#include "firestaff/dm1/v1/palette_entrance_pc34_compat.h"
#include "firestaff/dm1/v1/palette_credits_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"
#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"
#include <stdio.h>
#include <string.h>

#define DM1_V1_STARTUP_TITLE_ZOOM_STEPS_PC34 18u
#define DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34 23u
#define DM1_V1_STARTUP_TITLE_POST_ZOOM_VBLANKS_PC34 2u
#define DM1_V1_STARTUP_TITLE_FINAL_GUARD_VBLANKS_PC34 1u
/* ReDMCSB TITLE.C F0437 uses the PC VGA vertical-retrace primitive, as
 * SWSH.C does. Keep the runtime cadence on the shared 50 Hz/20 ms VBlank,
 * not the unrelated decoded TITLE.DAT frame-bank duration. */
#define DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34 20u

static unsigned int dm1_v1_startup_entrance_palette_fingerprint_pc34(void) {
    const unsigned int *palette = dm1_v1_palette_entrance_table_pc34();
    const int count = dm1_v1_palette_entrance_size_pc34();
    unsigned int hash = 2166136261u;
    int index;

    if (!palette || count != DM1_V1_PALETTE_ENTRANCE_PC34_COMPAT_SIZE) {
        return 0U;
    }
    for (index = 0; index < count; ++index) {
        hash ^= palette[index];
        hash *= 16777619u;
    }
    hash ^= (unsigned int)count;
    hash *= 16777619u;
    return hash ? hash : 1U;
}

static unsigned int dm1_v1_startup_credits_palette_fingerprint_pc34(void) {
    const unsigned int *palette = dm1_v1_palette_credits_table_pc34();
    const int count = dm1_v1_palette_credits_size_pc34();
    unsigned int hash = 2166136261u;
    int index;

    if (!palette || count != DM1_V1_PALETTE_CREDITS_PC34_COMPAT_SIZE) {
        return 0U;
    }
    for (index = 0; index < count; ++index) {
        hash ^= palette[index];
        hash *= 16777619u;
    }
    hash ^= (unsigned int)count;
    hash *= 16777619u;
    return hash ? hash : 1U;
}

static unsigned int dm1_v1_startup_hoc_capture_consumer_hash_pc34(
    unsigned int mask) {
    unsigned int hash = 2166136261u;
    hash ^= mask;
    hash *= 16777619u;
    hash ^= (mask << 8);
    hash *= 16777619u;
    hash ^= 0xD1C0u;
    return hash ? hash : 1u;
}

static unsigned int dm1_v1_startup_hoc_host_capture_route_hash_pc34(
    unsigned int mask,
    unsigned int chain_hash,
    unsigned int presented_hash) {
    unsigned int hash = 2166136261u;
    hash ^= mask;
    hash *= 16777619u;
    hash ^= chain_hash;
    hash *= 16777619u;
    hash ^= presented_hash;
    hash *= 16777619u;
    hash ^= 0xD1C0A11u;
    return hash ? hash : 1u;
}

static unsigned int dm1_v1_startup_hoc_release_app_identity_hash_pc34(
    const char* source_id,
    unsigned int consumer_hash,
    unsigned int route_hash,
    unsigned int presented_chain_hash) {
    unsigned int hash = 2166136261u;
    const unsigned char* p = (const unsigned char*)source_id;
    while (p && *p) {
        hash ^= (unsigned int)*p++;
        hash *= 16777619u;
    }
    hash ^= consumer_hash;
    hash *= 16777619u;
    hash ^= route_hash;
    hash *= 16777619u;
    hash ^= presented_chain_hash;
    hash *= 16777619u;
    hash ^= 0xA9C0DEu;
    return hash ? hash : 1u;
}

static int dm1_v1_startup_resume_root_from_path_pc34(
    const char *resume_path,
    char out_root[DM1_ORIGINAL_SAVE_PATH_MAX]) {
    const char *last_sep = NULL;
    size_t len;

    if (!resume_path || !resume_path[0] || !out_root) {
        return 0;
    }
    for (const char *p = resume_path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
        }
    }
    if (!last_sep) {
        return 0;
    }
    len = (size_t)(last_sep - resume_path);
    if (len == 0) {
        len = 1;
    }
    if (len >= DM1_ORIGINAL_SAVE_PATH_MAX) {
        return 0;
    }
    memcpy(out_root, resume_path, len);
    out_root[len] = '\0';
    return 1;
}

unsigned int dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
    int width,
    int height,
    int byte_count,
    unsigned int presented_hash,
    unsigned int consumer_mask) {
    unsigned int hash = 2166136261u;
    hash ^= (unsigned int)width;
    hash *= 16777619u;
    hash ^= (unsigned int)height;
    hash *= 16777619u;
    hash ^= (unsigned int)byte_count;
    hash *= 16777619u;
    hash ^= presented_hash;
    hash *= 16777619u;
    hash ^= consumer_mask;
    hash *= 16777619u;
    hash ^= 0xD14F0Cu;
    return hash ? hash : 1u;
}

unsigned int dm1_v1_startup_hoc_presented_rgba_hash_pc34(
    const unsigned char* rgba,
    int width,
    int height,
    int* out_byte_count) {
    unsigned int hash = 2166136261u;
    int byte_count;
    int i;

    if (out_byte_count) {
        *out_byte_count = 0;
    }
    if (!rgba || width < 320 || height < 200 ||
        width > 8192 || height > 8192) {
        return 0U;
    }
    byte_count = width * height * 4;
    if (byte_count <= 0) {
        return 0U;
    }
    for (i = 0; i < byte_count; ++i) {
        hash ^= (unsigned int)rgba[i];
        hash *= 16777619u;
    }
    hash ^= (unsigned int)width;
    hash *= 16777619u;
    hash ^= (unsigned int)height;
    hash *= 16777619u;
    if (hash == 0U) {
        hash = 1U;
    }
    if (out_byte_count) {
        *out_byte_count = byte_count;
    }
    return hash;
}

static int dm1_v1_startup_hoc_presented_capture_fields_pc34(
    const unsigned char* rgba,
    int width,
    int height,
    unsigned int consumer_mask,
    int* out_byte_count,
    unsigned int* out_hash,
    unsigned int* out_chain_hash) {
    int byte_count = 0;
    unsigned int hash;

    if (out_byte_count) {
        *out_byte_count = 0;
    }
    if (out_hash) {
        *out_hash = 0U;
    }
    if (out_chain_hash) {
        *out_chain_hash = 0U;
    }
    hash = dm1_v1_startup_hoc_presented_rgba_hash_pc34(
        rgba, width, height, &byte_count);
    if (hash == 0U || consumer_mask == 0U) {
        return 0;
    }
    if (out_byte_count) {
        *out_byte_count = byte_count;
    }
    if (out_hash) {
        *out_hash = hash;
    }
    if (out_chain_hash) {
        *out_chain_hash =
            dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
                width, height, byte_count, hash, consumer_mask);
    }
    return 1;
}

int dm1_v1_startup_hoc_capture_facts_set_presented_rgba_pc34(
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    const unsigned char* rgba,
    int width,
    int height,
    unsigned int consumer_mask) {
    int byte_count = 0;
    unsigned int hash = 0U;
    unsigned int chain_hash = 0U;

    if (!facts) {
        return 0;
    }
    if (!dm1_v1_startup_hoc_presented_capture_fields_pc34(
            rgba, width, height, consumer_mask,
            &byte_count, &hash, &chain_hash)) {
        facts->observed_presented_rgba_capture = 0;
        facts->presented_capture_width = width;
        facts->presented_capture_height = height;
        facts->presented_capture_byte_count = 0;
        facts->presented_capture_hash = 0U;
        facts->presented_capture_consumer_mask = consumer_mask;
        facts->presented_capture_chain_hash = 0U;
        return 0;
    }
    facts->observed_presented_rgba_capture = 1;
    facts->presented_capture_width = width;
    facts->presented_capture_height = height;
    facts->presented_capture_byte_count = byte_count;
    facts->presented_capture_hash = hash;
    facts->presented_capture_consumer_mask = consumer_mask;
    facts->presented_capture_chain_hash = chain_hash;
    return 1;
}

int dm1_v1_startup_hoc_capture_facts_set_presented_hash_pc34(
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    int width,
    int height,
    int byte_count,
    unsigned int presented_hash,
    unsigned int consumer_mask) {
    if (!facts) {
        return 0;
    }
    facts->observed_presented_rgba_capture =
        width >= 320 && height >= 200 &&
        byte_count >= width * height * 4 &&
        presented_hash != 0U && consumer_mask != 0U;
    facts->presented_capture_width = width;
    facts->presented_capture_height = height;
    facts->presented_capture_byte_count =
        facts->observed_presented_rgba_capture ? byte_count : 0;
    facts->presented_capture_hash =
        facts->observed_presented_rgba_capture ? presented_hash : 0U;
    facts->presented_capture_consumer_mask = consumer_mask;
    facts->presented_capture_chain_hash =
        facts->observed_presented_rgba_capture
            ? dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
                  width, height, byte_count, presented_hash, consumer_mask)
            : 0U;
    return facts->observed_presented_rgba_capture;
}

int dm1_v1_startup_hoc_host_probe_facts_set_presented_rgba_pc34(
    DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    const unsigned char* rgba,
    int width,
    int height,
    unsigned int consumer_mask) {
    int byte_count = 0;
    unsigned int hash = 0U;
    unsigned int chain_hash = 0U;

    if (!facts) {
        return 0;
    }
    if (!dm1_v1_startup_hoc_presented_capture_fields_pc34(
            rgba, width, height, consumer_mask,
            &byte_count, &hash, &chain_hash)) {
        facts->observed_presented_rgba_capture = 0;
        facts->presented_capture_width = width;
        facts->presented_capture_height = height;
        facts->presented_capture_byte_count = 0;
        facts->presented_capture_hash = 0U;
        facts->presented_capture_consumer_mask = consumer_mask;
        facts->presented_capture_chain_hash = 0U;
        return 0;
    }
    facts->observed_presented_rgba_capture = 1;
    facts->presented_capture_width = width;
    facts->presented_capture_height = height;
    facts->presented_capture_byte_count = byte_count;
    facts->presented_capture_hash = hash;
    facts->presented_capture_consumer_mask = consumer_mask;
    facts->presented_capture_chain_hash = chain_hash;
    return 1;
}

int dm1_v1_startup_hoc_host_probe_facts_set_presented_hash_pc34(
    DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    int width,
    int height,
    int byte_count,
    unsigned int presented_hash,
    unsigned int consumer_mask) {
    if (!facts) {
        return 0;
    }
    facts->observed_presented_rgba_capture =
        width >= 320 && height >= 200 &&
        byte_count >= width * height * 4 &&
        presented_hash != 0U && consumer_mask != 0U;
    facts->presented_capture_width = width;
    facts->presented_capture_height = height;
    facts->presented_capture_byte_count =
        facts->observed_presented_rgba_capture ? byte_count : 0;
    facts->presented_capture_hash =
        facts->observed_presented_rgba_capture ? presented_hash : 0U;
    facts->presented_capture_consumer_mask = consumer_mask;
    facts->presented_capture_chain_hash =
        facts->observed_presented_rgba_capture
            ? dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
                  width, height, byte_count, presented_hash, consumer_mask)
            : 0U;
    return facts->observed_presented_rgba_capture;
}

int dm1_v1_startup_hoc_capture_facts_apply_host_observation_pc34(
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    const DM1_V1_StartupHoCHostCaptureObservation_PC34* observation) {
    int host_presented;

    if (!facts || !observation) {
        return 0;
    }
    host_presented =
        observation->host_window_present &&
        observation->presented_capture_ready &&
        facts->observed_presented_rgba_capture;
    facts->captured_from_real_assets =
        observation->captured_from_real_assets ? 1 : 0;
    facts->observed_c026_portrait_asset =
        observation->observed_c026_portrait_asset ? 1 : 0;
    facts->observed_c346_mirror_backing_asset =
        observation->observed_c346_mirror_backing_asset ? 1 : 0;
    facts->observed_required_graphics_hash_match =
        observation->observed_required_graphics_hash_match ? 1 : 0;
    facts->observed_required_dungeon_hash_match =
        observation->observed_required_dungeon_hash_match ? 1 : 0;
    facts->observed_host_window_present =
        observation->host_window_present ? 1 : 0;
    facts->captured_from_mac_window = host_presented ? 1 : 0;
    facts->captured_from_release_app =
        host_presented &&
        observation->started_from_launcher &&
        observation->intro_not_bypassed ? 1 : 0;
    return 1;
}

int dm1_v1_startup_hoc_host_probe_facts_apply_host_observation_pc34(
    DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    const DM1_V1_StartupHoCHostCaptureObservation_PC34* observation) {
    int host_presented;

    if (!facts || !observation) {
        return 0;
    }
    host_presented =
        observation->host_window_present &&
        observation->presented_capture_ready &&
        facts->observed_presented_rgba_capture;
    facts->captured_from_real_assets =
        observation->captured_from_real_assets ? 1 : 0;
    facts->observed_c026_portrait_asset =
        observation->observed_c026_portrait_asset ? 1 : 0;
    facts->observed_c346_mirror_backing_asset =
        observation->observed_c346_mirror_backing_asset ? 1 : 0;
    facts->observed_required_graphics_hash_match =
        observation->observed_required_graphics_hash_match ? 1 : 0;
    facts->observed_required_dungeon_hash_match =
        observation->observed_required_dungeon_hash_match ? 1 : 0;
    facts->observed_host_window_present =
        observation->host_window_present ? 1 : 0;
    facts->captured_from_mac_window = host_presented ? 1 : 0;
    facts->captured_from_release_app =
        host_presented &&
        observation->started_from_launcher &&
        observation->intro_not_bypassed ? 1 : 0;
    return 1;
}

static int dm1_v1_startup_hoc_host_draw_no_backing_fallback_pc34(
    const DM1_V1_ChampionMirrorRenderReceiptPc34* render,
    int backing_asset_available,
    DM1_V1_ChampionMirrorHostDrawReceiptPc34* out_receipt) {
    return DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
               render, 0, backing_asset_available, out_receipt) &&
           out_receipt->valid &&
           out_receipt->drawMirrorBackingAsset &&
           !out_receipt->drawMirrorBackingFallbackRect;
}

static void dm1_v1_startup_entrance_door_geometry_pc34(
    unsigned int animation_step,
    DM1_V1_StartupEntranceRenderAudioCommand_PC34* command) {
    unsigned int left_right;
    unsigned int right_left;

    if (!command || animation_step == 0U || animation_step >= 32U) {
        return;
    }

    /* ReDMCSB ENTRANCE.C F0438:189-231 starts with left {0,100} and
     * right {109,231}; each source step draws the remaining strips, then
     * moves them four pixels outward.  Keep signed source bounds here: the
     * final five steps intentionally have no left strip, while the right
     * strip remains visible through step 31. */
    right_left = 109U + 4U * (animation_step - 1U);
    command->door_geometry_ready = 1;
    if (animation_step <= 26U) {
        left_right = 100U - 4U * (animation_step - 1U);
        command->door_left_box_x = 0U;
        command->door_left_box_y = 0U;
        command->door_left_box_w = left_right + 1U;
        command->door_left_box_h = 161U;
        command->door_left_source_x =
            (animation_step & 0x00FCU) << 2;
    }
    if (right_left <= 231U) {
        command->door_right_box_x = right_left;
        command->door_right_box_y = 0U;
        command->door_right_box_w = 231U - right_left + 1U;
        command->door_right_box_h = 161U;
        command->door_right_source_x =
            (animation_step & 0x0003U) << 2;
    }
}

static int dm1_v1_startup_hoc_host_draw_rejects_backing_fallback_pc34(
    int* out_consumes_backing_asset) {
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 front_wall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 host_draw;
    int consumes_backing_asset;
    memset(&front_wall, 0, sizeof(front_wall));
    memset(&render, 0, sizeof(render));
    memset(&host_draw, 0, sizeof(host_draw));
    consumes_backing_asset =
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 2, 2, &front_wall) &&
        DM1_V1_ChampionMirror_BuildRenderReceiptPc34(&front_wall, &render) &&
        dm1_v1_startup_hoc_host_draw_no_backing_fallback_pc34(
            &render, 1, &host_draw);
    if (out_consumes_backing_asset) {
        *out_consumes_backing_asset = consumes_backing_asset;
    }
    return consumes_backing_asset &&
           !dm1_v1_startup_hoc_host_draw_no_backing_fallback_pc34(
               &render, 0, &host_draw);
}

static int dm1_v1_startup_hoc_host_draw_uses_owned_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34*
        production,
    int backing_asset_available,
    int* out_consumed_owned_host_draw_receipt) {
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 post_plan;
    DM1_V1_StartupHandoffOutcome_PC34 outcome;
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 first_frame;
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 front_wall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 boundary;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 floor_thing;
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 thing_consumer;
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34 render_consumer;
    DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34 ownership;
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 host_draw;
    const DM1V1D1LD1RF0115LanePc34Data* lane;
    int consumed;

    if (out_consumed_owned_host_draw_receipt) {
        *out_consumed_owned_host_draw_receipt = 0;
    }
    memset(&post_plan, 0, sizeof(post_plan));
    memset(&outcome, 0, sizeof(outcome));
    memset(&first_frame, 0, sizeof(first_frame));
    memset(&front_wall, 0, sizeof(front_wall));
    memset(&render, 0, sizeof(render));
    memset(&boundary, 0, sizeof(boundary));
    memset(&floor_thing, 0, sizeof(floor_thing));
    memset(&thing_consumer, 0, sizeof(thing_consumer));
    memset(&render_consumer, 0, sizeof(render_consumer));
    memset(&ownership, 0, sizeof(ownership));
    memset(&host_draw, 0, sizeof(host_draw));

    lane = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    if (!production || !lane ||
        !dm1_v1_startup_handoff_post_launch_plan_pc34("dm1", &post_plan) ||
        !dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            ENTRANCE_COMPAT_COMMAND_PATH_ENTER, &outcome) ||
        !dm1_v1_startup_hoc_first_frame_receipt_pc34(
            "dm1", &post_plan, &outcome, &first_frame) ||
        !DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 2, 2, &front_wall) ||
        !DM1_V1_ChampionMirror_BuildRenderReceiptPc34(&front_wall, &render) ||
        !DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
            &render, &boundary) ||
        !dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
            lane, 5, 1, 1, 0, &floor_thing) ||
        !DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
            &boundary, &floor_thing, &thing_consumer) ||
        !dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
            &first_frame, &thing_consumer, &render_consumer) ||
        !dm1_v1_startup_hoc_fallback_draw_ownership_receipt_pc34(
            production, &render_consumer, &ownership) ||
        !dm1_v1_startup_hoc_owned_host_draw_receipt_pc34(
            &ownership, &render, 0, backing_asset_available, &host_draw)) {
        return 0;
    }
    consumed =
        ownership.ready &&
        ownership.consumed_production_consumer_receipt &&
        ownership.consumed_render_consumer_receipt &&
        ownership.consume_dm1_receipts_only &&
        ownership.no_m11_fallback_scan;
    if (out_consumed_owned_host_draw_receipt) {
        *out_consumed_owned_host_draw_receipt = consumed;
    }
    return consumed &&
           host_draw.valid &&
           host_draw.drawMirrorBackingAsset &&
           host_draw.drawChampionPortrait &&
           !host_draw.drawMirrorBackingFallbackRect;
}

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage) {
    switch (stage) {
        case DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34:
            return "SWSH_LOGO";
        case DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34:
            return "SWSH_RUN_START";
        case DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34:
            return "TITLE_BEGIN";
        case DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34:
            return "TITLE_LAST_FRAME";
        case DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34:
            return "MENU_ELIGIBLE";
        case DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34:
            return "ENTRANCE_WAIT";
    }
    return "UNKNOWN";
}

int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier) {
    return (unsigned int)later > (unsigned int)earlier;
}

int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path) {
    switch (path) {
        case DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34:
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34:
            /* Firestaff --game dm1 bypasses only the M12 launcher surface.
             * main_loop_m11.c still routes through the source-visible
             * SWSH -> TITLE -> ENTRANCE startup sequence before gameplay. */
            return 0;
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34:
            /* M11_GameView_StartDm1() is a focused test/dev entry point.
             * It intentionally starts the DM1 game view directly and must
             * not be confused with the ReDMCSB SWSH -> TITLE -> ENTRANCE
             * launcher handoff used by normal DM1 startup. */
            return 1;
    }
    return 1;
}

static int dm1_v1_startup_launch_path_started_from_launcher_pc34(
    DM1_V1_StartupLaunchPath_PC34 path) {
    switch (path) {
        case DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34:
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34:
            return 1;
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34:
            return 0;
    }
    return 0;
}

int dm1_v1_startup_source_visible_handoff_required_pc34(const char* game_id) {
    return game_id && strcmp(game_id, "dm1") == 0 ? 1 : 0;
}

int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed) {
    return dm1_v1_startup_source_visible_handoff_required_pc34(sourceId) &&
           bypassed ? 1 : 0;
}

int dm1_v1_startup_selected_entry_receipt_valid_pc34(const char* game_id,
                                                     int intro_bypassed) {
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(game_id)) {
        return 1;
    }
    return intro_bypassed ? 0 : 1;
}

int dm1_v1_startup_launch_path_receipt_pc34(
    const DM1_V1_StartupLaunchPathFacts_PC34* facts,
    DM1_V1_StartupLaunchPathReceipt_PC34* out_receipt) {
    DM1_V1_StartupLaunchPathReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->source_id)) {
        *out_receipt = receipt;
        return 1;
    }
    receipt.handled = 1;
    receipt.intro_bypassed =
        dm1_v1_startup_launch_path_bypasses_intro_pc34(
            facts->launch_path);
    receipt.started_from_launcher =
        dm1_v1_startup_launch_path_started_from_launcher_pc34(
            facts->launch_path);
    receipt.selected_entry_receipt_valid =
        dm1_v1_startup_selected_entry_receipt_valid_pc34(
            facts->source_id,
            receipt.intro_bypassed);
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_runtime_start_receipt_pc34(
    const DM1_V1_StartupRuntimeStartFacts_PC34* facts,
    DM1_V1_StartupRuntimeStartReceipt_PC34* out_receipt) {
    DM1_V1_StartupRuntimeStartReceipt_PC34 receipt;
    DM1_V1_StartupLaunchPathFacts_PC34 launch_facts;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->game_id)) {
        *out_receipt = receipt;
        return 1;
    }
    memset(&launch_facts, 0, sizeof(launch_facts));
    launch_facts.source_id = facts->game_id;
    launch_facts.launch_path = facts->launch_path;
    if (!dm1_v1_startup_launch_path_receipt_pc34(
            &launch_facts,
            &receipt.launch_path_receipt)) {
        return 0;
    }

    /* ReDMCSB: ENTRANCE.C hands off to the loaded dungeon only after the
     * source-visible startup path. Firestaff keeps the host-owned dungeon
     * allocation in M11, but the DM1 module owns the launch receipt that
     * marks gameplay active after that load succeeds. */
    receipt.handled = 1;
    receipt.active = 1;
    receipt.started_from_launcher =
        receipt.launch_path_receipt.started_from_launcher;
    receipt.source_kind = facts->source_kind;
    snprintf(receipt.boot_asset_md5,
             sizeof(receipt.boot_asset_md5),
             "%s",
             facts->verified_asset_md5 ? facts->verified_asset_md5 : "");
    receipt.presentation_mode = facts->presentation_mode;
    receipt.presentation_width = facts->presentation_width;
    receipt.presentation_height = facts->presentation_height;
    receipt.font_scale =
        (facts->font_scale >= 1 && facts->font_scale <= 3)
            ? facts->font_scale
            : 0;
    snprintf(receipt.title,
             sizeof(receipt.title),
             "%s",
             facts->title ? facts->title : "DUNGEON MASTER");
    snprintf(receipt.source_id,
             sizeof(receipt.source_id),
             "%s",
             facts->source_id ? facts->source_id : "launcher");
    snprintf(receipt.dungeon_path,
             sizeof(receipt.dungeon_path),
             "%s",
             facts->dungeon_path ? facts->dungeon_path : "");
    snprintf(receipt.status_title,
             sizeof(receipt.status_title),
             "%s",
             "BOOT");
    snprintf(receipt.status_detail,
             sizeof(receipt.status_detail),
             "%s",
             "GAME DATA LOADED");
    snprintf(receipt.inspect_title,
             sizeof(receipt.inspect_title),
             "%s",
             "READY");
    snprintf(receipt.inspect_detail,
             sizeof(receipt.inspect_detail),
             "%s",
             "CLICK CENTER TO ADVANCE OR READ, CLICK SIDES TO TURN, TAB PICKS THE FRONT CHAMPION");
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_dungeon_path_receipt_pc34(
    const DM1_V1_StartupDungeonPathFacts_PC34* facts,
    DM1_V1_StartupDungeonPathReceipt_PC34* out_receipt) {
    DM1_V1_StartupDungeonPathReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->game_id)) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    if (facts->source_kind ==
        DM1_V1_STARTUP_SOURCE_KIND_DIRECT_DUNGEON_PC34) {
        receipt.explicit_path_required = 1;
        if (!facts->explicit_dungeon_path ||
            facts->explicit_dungeon_path[0] == '\0') {
            *out_receipt = receipt;
            return 1;
        }
    }
    if (facts->explicit_dungeon_path &&
        facts->explicit_dungeon_path[0] != '\0') {
        receipt.use_explicit_path = 1;
        snprintf(receipt.explicit_dungeon_path,
                 sizeof(receipt.explicit_dungeon_path),
                 "%s",
                 facts->explicit_dungeon_path);
    } else {
        receipt.resolve_builtin_path = 1;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_graphics_bind_receipt_pc34(
    const DM1_V1_StartupGraphicsBindFacts_PC34* facts,
    DM1_V1_StartupGraphicsBindReceipt_PC34* out_receipt) {
    DM1_V1_StartupGraphicsBindReceipt_PC34 receipt;
    size_t dungeon_len;
    size_t slash_pos;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->game_id)) {
        *out_receipt = receipt;
        return 1;
    }
    receipt.handled = 1;
    if (!facts->dungeon_path || facts->dungeon_path[0] == '\0') {
        *out_receipt = receipt;
        return 1;
    }

    dungeon_len = strlen(facts->dungeon_path);
    slash_pos = dungeon_len;
    while (slash_pos > 0 &&
           facts->dungeon_path[slash_pos - 1] != '/' &&
           facts->dungeon_path[slash_pos - 1] != '\\') {
        --slash_pos;
    }
    if (slash_pos > 0 &&
        slash_pos + 13 < sizeof(receipt.graphics_dat_path)) {
        memcpy(receipt.graphics_dat_path,
               facts->dungeon_path,
               slash_pos);
        memcpy(receipt.graphics_dat_path + slash_pos,
               "GRAPHICS.DAT",
               13);
        receipt.bind_graphics_dat = 1;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_dungeon_load_receipt_pc34(
    const DM1_V1_StartupDungeonLoadFacts_PC34* facts,
    DM1_V1_StartupDungeonLoadReceipt_PC34* out_receipt) {
    DM1_V1_StartupDungeonLoadReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->game_id)) {
        *out_receipt = receipt;
        return 1;
    }
    receipt.handled = 1;
    receipt.load_succeeded = facts->load_succeeded ? 1 : 0;
    snprintf(receipt.status_title,
             sizeof(receipt.status_title),
             "%s",
             "BOOT");
    snprintf(receipt.status_detail,
             sizeof(receipt.status_detail),
             "%s",
             receipt.load_succeeded ? "GAME DATA LOADED"
                                    : "FAILED TO LOAD DUNGEON.DAT");
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_runtime_ready_receipt_pc34(
    const DM1_V1_StartupRuntimeReadyFacts_PC34* facts,
    DM1_V1_StartupRuntimeReadyReceipt_PC34* out_receipt) {
    DM1_V1_StartupRuntimeReadyReceipt_PC34 receipt;
    DM1_V1_StartupDungeonLoadFacts_PC34 load_facts;
    DM1_V1_StartupGraphicsBindFacts_PC34 graphics_facts;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->runtime_start.game_id)) {
        *out_receipt = receipt;
        return 1;
    }

    memset(&load_facts, 0, sizeof(load_facts));
    load_facts.game_id = facts->runtime_start.game_id;
    load_facts.load_succeeded = facts->load_succeeded;
    if (!dm1_v1_startup_dungeon_load_receipt_pc34(
            &load_facts,
            &receipt.load_receipt) ||
        !receipt.load_receipt.handled ||
        !receipt.load_receipt.load_succeeded) {
        return 0;
    }

    if (!dm1_v1_startup_runtime_start_receipt_pc34(
            &facts->runtime_start,
            &receipt.runtime_start_receipt) ||
        !receipt.runtime_start_receipt.handled) {
        return 0;
    }

    memset(&graphics_facts, 0, sizeof(graphics_facts));
    graphics_facts.game_id = facts->runtime_start.game_id;
    graphics_facts.dungeon_path = facts->runtime_start.dungeon_path;
    if (!dm1_v1_startup_graphics_bind_receipt_pc34(
            &graphics_facts,
            &receipt.graphics_bind_receipt) ||
        !receipt.graphics_bind_receipt.handled) {
        return 0;
    }

    receipt.handled = 1;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_handoff_prelude_plan_pc34(
    const char* game_id,
    DM1_V1_StartupHandoffPreludePlan_PC34* out_plan) {
    int required;
    int source_order_valid;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    required = dm1_v1_startup_source_visible_handoff_required_pc34(game_id);
    source_order_valid = dm1_v1_startup_sequence_source_order_valid_pc34();
    out_plan->required = required;
    out_plan->source_order_valid = source_order_valid;
    if (required &&
        !dm1_v1_startup_full_graphics_media_receipt_pc34(
            game_id,
            &out_plan->media_receipt)) {
        return 0;
    }
    out_plan->play_swsh =
        (required && out_plan->media_receipt.play_swsh) ? 1 : 0;
    out_plan->discard_presentation_after_swsh = required ? 1 : 0;
    out_plan->game_id = game_id;
    out_plan->failure_evidence =
        source_order_valid ? "" : dm1_v1_startup_sequence_source_evidence_pc34();
    return 1;
}

int dm1_v1_startup_handoff_post_launch_plan_pc34(
    const char* source_id,
    DM1_V1_StartupHandoffPostLaunchPlan_PC34* out_plan) {
    int required;
    DM1_V1_StartupTitleMenuEligibilityFacts_PC34 title_facts;
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34 title_receipt;
    DM1_V1_EntranceCtxPc34 entrance_ctx;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    required = dm1_v1_startup_source_visible_handoff_required_pc34(source_id);
    out_plan->required = required;
    out_plan->source_id = source_id;
    if (required) {
        if (!dm1_v1_startup_full_graphics_media_receipt_pc34(
                source_id,
                &out_plan->media_receipt)) {
            return 0;
        }
        out_plan->play_title = out_plan->media_receipt.play_title ? 1 : 0;
        out_plan->play_entrance =
            out_plan->media_receipt.play_entrance ? 1 : 0;
        out_plan->entrance_auto_enter_ms =
            out_plan->media_receipt.entrance_auto_enter_ms;
        memset(&title_facts, 0, sizeof(title_facts));
        memset(&title_receipt, 0, sizeof(title_receipt));
        title_facts.title_frame =
            out_plan->media_receipt.title_menu_boundary_frame;
        title_facts.title_frame_max =
            out_plan->media_receipt.title_frame_bank_equivalent_steps;
        title_facts.advance_requested = 1;
        title_facts.title_handoff_ready = 1;
        if (!dm1_v1_startup_title_menu_eligibility_receipt_pc34(
                &title_facts,
                &title_receipt)) {
            return 0;
        }
        DM1_V1_Entrance_InitPc34Compat(&entrance_ctx);
        entrance_ctx.state = DM1_ENTRANCE_VIEWING;
        entrance_ctx.doorAnim.complete = 1;
        entrance_ctx.doorAnim.animationStep =
            DM1_V1_ENTRANCE_DOOR_OPEN_FRAME_INDEX_PC34;
        if (!DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(
                &entrance_ctx,
                &out_plan->entrance_full_start_receipt)) {
            return 0;
        }
        out_plan->title_menu_boundary_frame =
            (int)out_plan->media_receipt.title_menu_boundary_frame;
        out_plan->title_menu_eligible = title_receipt.menu_eligible;
        out_plan->title_keep_surface = title_receipt.keep_title_surface;
        out_plan->title_consume_pending_input =
            title_receipt.consume_pending_input;
        out_plan->title_next_stage = title_receipt.next_stage;
        out_plan->title_menu_reason = title_receipt.reason;
        out_plan->entrance_wait_stage = DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34;
        /* ReDMCSB TITLE.C F0437:319-409 finishes PRESENTS/title/guard before
         * ENTRANCE.C F0441:850-883 discards input and waits on entrance.
         * F0797:68-80 builds the C255 5x5 entrance micro-dungeon that M11/M12
         * should consume from this DM1-owned startup plan. */
    }
    return 1;
}

int dm1_v1_startup_execute_handoff_prelude_pc34(
    const char* game_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks) {
    DM1_V1_StartupHandoffPreludePlan_PC34 plan;

    if (!callbacks ||
        !dm1_v1_startup_handoff_prelude_plan_pc34(game_id, &plan)) {
        return 0;
    }
    if (!plan.required) {
        return 1;
    }
    if (callbacks->begin_prelude_plan &&
        !callbacks->begin_prelude_plan(callbacks->user, &plan)) {
        return 0;
    }
    /* ReDMCSB: APPA.C loads SWSH before TITLE/APPB, and STARTUP2.C later
     * calls F0437_STARTEND_DrawTitle before entrance processing.  Keep the
     * host-side calls behind this DM1-owned execution facade so M11 cannot
     * silently reorder the visible DM1 startup path. */
    if (!plan.source_order_valid &&
        callbacks->report_source_order_failure &&
        !callbacks->report_source_order_failure(callbacks->user,
                                                plan.failure_evidence)) {
        if (callbacks->end_prelude_plan) {
            (void)callbacks->end_prelude_plan(callbacks->user);
        }
        return 0;
    }
    if (plan.play_swsh) {
        if (!callbacks->raise_window || !callbacks->play_swsh) {
            if (callbacks->end_prelude_plan) {
                (void)callbacks->end_prelude_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->raise_window(callbacks->user)) {
            if (callbacks->end_prelude_plan) {
                (void)callbacks->end_prelude_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->play_swsh(callbacks->user, plan.game_id, 0)) {
            if (callbacks->end_prelude_plan) {
                (void)callbacks->end_prelude_plan(callbacks->user);
            }
            return 0;
        }
    }
    if (plan.discard_presentation_after_swsh &&
        (!callbacks->discard_presentation_texture ||
         !callbacks->discard_presentation_texture(callbacks->user))) {
        if (callbacks->end_prelude_plan) {
            (void)callbacks->end_prelude_plan(callbacks->user);
        }
        return 0;
    }
    if (callbacks->end_prelude_plan &&
        !callbacks->end_prelude_plan(callbacks->user)) {
        return 0;
    }
    return 1;
}

int dm1_v1_startup_execute_handoff_post_launch_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    int* out_title_played,
    int* out_entrance_command) {
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 plan;
    int title_played = 0;
    int entrance_command = 0;

    if (out_title_played) {
        *out_title_played = 0;
    }
    if (out_entrance_command) {
        *out_entrance_command = 0;
    }
    if (!callbacks ||
        !dm1_v1_startup_handoff_post_launch_plan_pc34(source_id, &plan)) {
        return 0;
    }
    if (!plan.required) {
        return 1;
    }
    if (callbacks->begin_post_launch_plan &&
        !callbacks->begin_post_launch_plan(callbacks->user, &plan)) {
        return 0;
    }
    /* ReDMCSB STARTUP2.C: F0437_STARTEND_DrawTitle precedes the later
     * F0441_STARTEND_ProcessEntrance gate. */
    if (plan.play_title) {
        if (!callbacks->raise_window || !callbacks->play_title) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->raise_window(callbacks->user)) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->play_title(callbacks->user,
                                   plan.source_id,
                                   &title_played)) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
    }
    if (plan.play_entrance) {
        if (!callbacks->play_entrance) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->play_entrance(callbacks->user,
                                      plan.source_id,
                                      plan.entrance_auto_enter_ms,
                                      &entrance_command)) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
    }
    if (callbacks->end_post_launch_plan &&
        !callbacks->end_post_launch_plan(callbacks->user)) {
        return 0;
    }
    if (out_title_played) {
        *out_title_played = title_played;
    }
    if (out_entrance_command) {
        *out_entrance_command = entrance_command;
    }
    return 1;
}

int dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
    int entrance_command,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome) {
    if (!out_outcome) {
        return 0;
    }
    memset(out_outcome, 0, sizeof(*out_outcome));
    out_outcome->entrance_command = entrance_command;
    switch (entrance_command) {
        case ENTRANCE_COMPAT_COMMAND_PATH_ENTER:
            out_outcome->action =
                DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34;
            out_outcome->status = "DM1 ENTER";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_RESUME:
            out_outcome->action =
                DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34;
            out_outcome->status = "DM1 RESUME";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_QUIT:
            out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34;
            out_outcome->status = "DM1 QUIT";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_CREDITS:
            /* ReDMCSB ENTRANCE.C F0442:1091 restores C202 after its
             * 1800-VBlank credits window so F0441 redraws the entrance.
             * Credits is therefore an entrance-local loop command, never a
             * terminal game/resume/quit startup handoff. */
            out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34;
            out_outcome->status = "DM1 ENTRANCE CREDITS LOOP";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_NONE:
            out_outcome->action =
                DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34;
            out_outcome->status = "DM1 ENTRANCE SKIPPED";
            break;
        default:
            out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34;
            out_outcome->status = "DM1 HANDOFF NONE";
            break;
    }
    return 1;
}

int dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome) {
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 plan;
    int title_played = 0;
    int entrance_command = 0;

    if (!out_outcome) {
        return 0;
    }
    memset(out_outcome, 0, sizeof(*out_outcome));
    if (!dm1_v1_startup_handoff_post_launch_plan_pc34(source_id, &plan)) {
        return 0;
    }
    if (!plan.required) {
        out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34;
        out_outcome->status = "DM1 HANDOFF NONE";
        return 1;
    }
    if (!dm1_v1_startup_execute_handoff_post_launch_pc34(source_id,
                                                         callbacks,
                                                         &title_played,
                                                         &entrance_command)) {
        return 0;
    }
    if (!dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            entrance_command,
            out_outcome)) {
        return 0;
    }
    out_outcome->title_played = title_played;
    return 1;
}

int dm1_v1_startup_apply_handoff_outcome_pc34(
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const char* source_id,
    const DM1_V1_StartupHostCallbacks_PC34* callbacks,
    DM1_V1_StartupHostApplyResult_PC34* out_result) {
    DM1_V1_StartupHostApplyResult_PC34 result;
    int used_backup = 0;

    if (!outcome || !out_result) {
        return 0;
    }
    memset(&result, 0, sizeof(result));
    if (outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 ||
        outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34) {
        *out_result = result;
        return 1;
    }
    if (!callbacks) {
        return 0;
    }
    result.handled = 1;
    switch (outcome->action) {
        case DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34:
            result.quit_requested = 1;
            if (!callbacks->set_game_active ||
                !callbacks->set_game_active(callbacks->user, 0)) {
                return 0;
            }
            break;
        case DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34:
            result.resume_requested = 1;
            /* ReDMCSB COMMAND.C M566: RESUME loads the saved game.  Firestaff
             * keeps host path resolution/load I/O behind callbacks, while DM1
             * owns the decision that RESUME should attempt this path. */
            if (!callbacks->resolve_resume_save_path ||
                !callbacks->load_resume_save_path) {
                return 0;
            }
            if (callbacks->resolve_resume_save_path(callbacks->user,
                                                    source_id,
                                                    result.resume_path,
                                                    (int)sizeof(result.resume_path)) &&
                callbacks->load_resume_save_path(callbacks->user,
                                                 result.resume_path,
                                                 &used_backup)) {
                result.resume_loaded = 1;
                result.resume_used_backup = used_backup;
                if (callbacks->set_game_active &&
                    !callbacks->set_game_active(callbacks->user, 1)) {
                    return 0;
                }
                if (callbacks->log_resume_loaded &&
                    !callbacks->log_resume_loaded(callbacks->user,
                                                  result.resume_path,
                                                  used_backup)) {
                    return 0;
                }
            } else if (callbacks->log_resume_missing &&
                       !callbacks->log_resume_missing(
                           callbacks->user,
                           result.resume_path[0] ? result.resume_path
                                                 : "(unresolved)")) {
                return 0;
            }
            break;
        case DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34:
            if (callbacks->log_entrance_skipped &&
                !callbacks->log_entrance_skipped(callbacks->user)) {
                return 0;
            }
            break;
        default:
            result.handled = 0;
            break;
    }
    *out_result = result;
    return 1;
}

int dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
    const char* selected_game_id,
    const char* opened_source_id,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const DM1_V1_StartupHostApplyResult_PC34* host_result,
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* out_receipt) {
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 receipt;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    DM1_V1_EntranceCtxPc34 entrance_ctx;

    if (!out_receipt || !outcome || !host_result) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    memset(&media, 0, sizeof(media));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            selected_game_id)) {
        *out_receipt = receipt;
        return 1;
    }
    if (!dm1_v1_startup_full_graphics_media_receipt_pc34(opened_source_id,
                                                         &media)) {
        return 0;
    }

    /* ReDMCSB source order:
     * SWSH.C runs START.PRG, TITLE.C F0437 lines 319-409 completes
     * PRESENTS/title/guard, and ENTRANCE.C F0441 lines 850-883 returns an
     * entrance command before the dungeon/HoC runtime is redrawn.  This
     * receipt is the DM1-owned boundary from full-graphics startup media to
     * the live Hall of Champions/runtime frame. */
    receipt.handled = 1;
    receipt.full_graphics_required = media.handled ? 1 : 0;
    receipt.swsh_consumed = media.play_swsh ? 1 : 0;
    receipt.title_consumed =
        (media.play_title && outcome->title_played) ? 1 : 0;
    receipt.entrance_consumed =
        (media.play_entrance &&
         outcome->action != DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34)
            ? 1
            : 0;
    receipt.full_graphics_consumed =
        receipt.full_graphics_required &&
        receipt.swsh_consumed &&
        receipt.title_consumed &&
        receipt.entrance_consumed;
    receipt.entrance_command = outcome->entrance_command;
    receipt.action = outcome->action;
    receipt.status = outcome->status;
    receipt.return_to_launcher =
        (outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34 ||
         host_result->quit_requested)
            ? 1
            : 0;
    receipt.hoc_runtime_ready =
        receipt.full_graphics_consumed &&
        outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34 &&
        !receipt.return_to_launcher;
    receipt.resumed_runtime_ready =
        receipt.full_graphics_consumed &&
        outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34 &&
        host_result->resume_loaded &&
        !receipt.return_to_launcher;
    if (receipt.hoc_runtime_ready) {
        DM1_V1_Entrance_InitPc34Compat(&entrance_ctx);
        entrance_ctx.state = DM1_ENTRANCE_VIEWING;
        if (!DM1_V1_Entrance_BuildMenuRouteReceiptPc34Compat(
                &entrance_ctx,
                &receipt.champion_mirror_startup_route)) {
            return 0;
        }
        /* ReDMCSB ENTRANCE.C F0441 lines 850-883 enters the Hall of
         * Champions route before mirror selection.  REVIVE.C F0280 later owns
         * the candidate champion route after a mirror is selected.  The first
         * runtime frame must therefore start from the DM1-owned Hall route,
         * not from stale entrance/title host state. */
        receipt.champion_mirror_startup_handoff_ready =
            (receipt.champion_mirror_startup_route.handled &&
             receipt.champion_mirror_startup_route.route ==
                 DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
             receipt.champion_mirror_startup_route.showHall &&
             receipt.champion_mirror_startup_route.needsRedraw)
                ? 1
                : 0;
        receipt.champion_mirror_startup_input_ready =
            (receipt.champion_mirror_startup_handoff_ready &&
             receipt.champion_mirror_startup_route.state ==
                 DM1_ENTRANCE_VIEWING &&
             receipt.champion_mirror_startup_route.selectedMirrorIndex < 0)
                ? 1
                : 0;
        receipt.champion_mirror_startup_panel_clear =
            (receipt.champion_mirror_startup_handoff_ready &&
             !receipt.champion_mirror_startup_route.showChampionPanel &&
             !receipt.champion_mirror_startup_route
                  .showResurrectReincarnateChoices)
                ? 1
                : 0;
        receipt.champion_mirror_startup_blocks_enter =
            (receipt.champion_mirror_startup_handoff_ready &&
             receipt.champion_mirror_startup_route.partyChampionCount == 0 &&
             !receipt.champion_mirror_startup_route.canEnterDungeon)
                ? 1
                : 0;
        receipt.champion_mirror_startup_overlay_command_count =
            receipt.champion_mirror_startup_route.renderOverlayCommandCount;
        if (receipt.champion_mirror_startup_overlay_command_count > 0 &&
            receipt.champion_mirror_startup_overlay_command_count <=
                DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34) {
            int i;
            for (i = 0; i < receipt.champion_mirror_startup_overlay_command_count;
                 ++i) {
                receipt.champion_mirror_startup_overlay_commands[i] =
                    receipt.champion_mirror_startup_route.renderOverlayCommands[i];
            }
            receipt.champion_mirror_startup_overlay_commands_ready =
                (receipt.champion_mirror_startup_overlay_commands[0].valid &&
                 receipt.champion_mirror_startup_overlay_commands[0].kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 receipt.champion_mirror_startup_overlay_commands[0]
                     .clearStalePanelFirst &&
                 receipt.champion_mirror_startup_overlay_commands[0]
                     .suppressThingPayloads &&
                 receipt.champion_mirror_startup_overlay_commands[0]
                     .blockEnterUntilChampionSelected)
                    ? 1
                    : 0;
        }
    }
    receipt.hoc_first_frame_ready =
        receipt.hoc_runtime_ready &&
        receipt.champion_mirror_startup_handoff_ready &&
        receipt.champion_mirror_startup_input_ready &&
        receipt.champion_mirror_startup_panel_clear &&
        receipt.champion_mirror_startup_blocks_enter &&
        receipt.champion_mirror_startup_overlay_commands_ready;
    receipt.runtime_first_frame_ready =
        (receipt.hoc_first_frame_ready || receipt.resumed_runtime_ready) ? 1 : 0;
    receipt.draw_opened_runtime =
        receipt.runtime_first_frame_ready ? 1 : 0;
    receipt.suppress_draw_opened = receipt.draw_opened_runtime ? 0 : 1;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_save_resume_capture_receipt_pc34(
    const DM1_V1_StartupSaveResumeCaptureFacts_PC34* facts,
    DM1_V1_StartupSaveResumeCaptureReceipt_PC34* out_receipt) {
    DM1_V1_StartupSaveResumeCaptureReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->source_id)) {
        *out_receipt = receipt;
        return 1;
    }
    if (!facts->outcome || !facts->host_apply || !facts->runtime_handoff) {
        return 0;
    }

    receipt.handled = 1;
    receipt.expected_save_part_count =
        DM1_V1_STARTUP_SAVE_CORPUS_PART_COUNT_PC34;
    receipt.expected_champion_portrait_count =
        DM1_V1_STARTUP_SAVE_CORPUS_PORTRAIT_COUNT_PC34;
    receipt.observed_save_part_count = facts->observed_save_part_count;
    receipt.observed_champion_portrait_count =
        facts->observed_champion_portrait_count;
    receipt.user_save_corpus_scan_consumed =
        facts->observed_user_save_corpus_scan ? 1 : 0;
    receipt.user_save_corpus_files = facts->observed_user_save_corpus_files;
    receipt.user_save_corpus_classified =
        facts->observed_user_save_corpus_classified;
    receipt.user_save_corpus_pc34 = facts->observed_user_save_corpus_pc34;
    receipt.user_save_corpus_part_envelope =
        facts->observed_user_save_corpus_part_envelope;
    receipt.user_save_corpus_roundtrip_verified =
        facts->observed_user_save_corpus_roundtrip_verified;
    receipt.user_save_corpus_roundtrip_failed =
        facts->observed_user_save_corpus_roundtrip_failed;
    receipt.user_save_corpus_roundtrip_hash =
        facts->observed_user_save_corpus_roundtrip_hash;
    receipt.user_save_corpus_rejected =
        facts->observed_user_save_corpus_rejected;
    receipt.user_save_corpus_truncated =
        facts->observed_user_save_corpus_truncated;
    if (facts->observed_user_save_corpus_first_pc34_path &&
        facts->observed_user_save_corpus_first_pc34_path[0]) {
        snprintf(receipt.user_save_corpus_first_pc34_path,
                 sizeof(receipt.user_save_corpus_first_pc34_path),
                 "%s",
                 facts->observed_user_save_corpus_first_pc34_path);
    }
    receipt.source_evidence =
        "ReDMCSB COMMAND.C:2449-2450; LOADSAVE.C:1574-1649";

    /* ReDMCSB COMMAND.C M566 sets LOAD_SAVED_GAME.  LOADSAVE.C then builds
     * the save corpus from the five original save parts plus four PC34
     * champion portraits and dungeon payload.  Host I/O stays outside DM1,
     * but this receipt owns the decision that a loaded RESUME route is a save
     * corpus handoff, not a HoC first-frame capture. */
    receipt.consumed_resume_outcome =
        facts->outcome->action ==
        DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34;
    receipt.consumed_host_apply_result =
        facts->host_apply->handled && facts->host_apply->resume_requested;
    receipt.consumed_runtime_handoff_receipt =
        facts->runtime_handoff->handled &&
        facts->runtime_handoff->full_graphics_consumed;
    receipt.resume_command_maps_load_saved_game =
        receipt.consumed_resume_outcome &&
        facts->outcome->entrance_command == ENTRANCE_COMPAT_COMMAND_PATH_RESUME;
    receipt.resume_path_resolved =
        facts->host_apply->resume_path[0] != '\0';
    receipt.resume_load_consumed =
        facts->host_apply->resume_loaded ? 1 : 0;
    receipt.resume_runtime_ready =
        facts->runtime_handoff->resumed_runtime_ready &&
        facts->runtime_handoff->runtime_first_frame_ready &&
        facts->runtime_handoff->draw_opened_runtime &&
        !facts->runtime_handoff->hoc_runtime_ready;
    receipt.suppress_hoc_first_frame =
        !facts->runtime_handoff->hoc_first_frame_ready &&
        !facts->runtime_handoff->champion_mirror_startup_handoff_ready;
    receipt.save_header_present =
        facts->observed_save_header ? 1 : 0;
    receipt.save_part_corpus_present =
        (facts->observed_save_part_count ==
         DM1_V1_STARTUP_SAVE_CORPUS_PART_COUNT_PC34) ||
        (receipt.user_save_corpus_scan_consumed &&
         receipt.user_save_corpus_part_envelope > 0 &&
         receipt.user_save_corpus_roundtrip_verified ==
             receipt.user_save_corpus_part_envelope &&
         receipt.user_save_corpus_roundtrip_failed == 0);
    receipt.champion_portrait_corpus_present =
        facts->observed_champion_portrait_count ==
        DM1_V1_STARTUP_SAVE_CORPUS_PORTRAIT_COUNT_PC34;
    receipt.dungeon_payload_present =
        facts->observed_dungeon_payload ? 1 : 0;
    receipt.required_asset_hashes_present =
        facts->observed_required_graphics_hash_match &&
        facts->observed_required_dungeon_hash_match;
    receipt.save_corpus_capture_ready =
        receipt.save_header_present &&
        receipt.save_part_corpus_present &&
        receipt.champion_portrait_corpus_present &&
        receipt.dungeon_payload_present;
    receipt.original_save_roundtrip_route_ready =
        receipt.consumed_resume_outcome &&
        receipt.consumed_host_apply_result &&
        receipt.consumed_runtime_handoff_receipt &&
        receipt.resume_command_maps_load_saved_game &&
        receipt.resume_path_resolved &&
        receipt.resume_load_consumed &&
        receipt.resume_runtime_ready &&
        receipt.suppress_hoc_first_frame &&
        receipt.save_corpus_capture_ready &&
        receipt.required_asset_hashes_present;
    receipt.ready = receipt.original_save_roundtrip_route_ready;
    snprintf(receipt.resume_path,
             sizeof(receipt.resume_path),
             "%s",
             facts->host_apply->resume_path);
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_first_frame_receipt_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* post_plan,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    DM1_V1_StartupHoCFirstFrameReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 receipt;

    if (!out_receipt || !post_plan || !outcome) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.full_graphics_required = 1;
    receipt.title_surface_released =
        (post_plan->required &&
         post_plan->play_title &&
         post_plan->title_menu_eligible &&
         !post_plan->title_keep_surface &&
         post_plan->title_consume_pending_input)
            ? 1
            : 0;
    receipt.entrance_wait_consumed =
        (post_plan->play_entrance &&
         outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34)
            ? 1
            : 0;

    receipt.entrance_full_start_receipt =
        post_plan->entrance_full_start_receipt;
    {
        DM1_V1_EntranceCtxPc34 entrance_ctx;
        DM1_V1_Entrance_InitPc34Compat(&entrance_ctx);
        entrance_ctx.state = DM1_ENTRANCE_VIEWING;
        entrance_ctx.doorAnim.complete = 1;
        entrance_ctx.doorAnim.animationStep =
            entrance_ctx.doorAnim.totalSteps - 1;
        if (!DM1_V1_Entrance_BuildMenuRouteReceiptPc34Compat(
            &entrance_ctx,
            &receipt.champion_select_route)) {
            return 0;
        }
    }

    /* ReDMCSB TITLE.C F0437:319-409 releases the title surface only after
     * PRESENTS/title/guard. ENTRANCE.C F0441:850-883 waits for a fresh
     * entrance command, and F0797:68-80 builds the C255 5x5 entrance map.
     * The first HoC frame is therefore the entrance VIEWING state with hall
     * mirror UI, not a host fallback title/door frame. */
    receipt.full_start_render_ready =
        receipt.entrance_full_start_receipt.valid ? 1 : 0;
    receipt.entrance_map_ready =
        (receipt.entrance_full_start_receipt.mapIndex ==
             DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
         receipt.entrance_full_start_receipt.width ==
             DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
         receipt.entrance_full_start_receipt.height ==
             DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
         receipt.entrance_full_start_receipt.corridorCount == 6)
            ? 1
            : 0;
    receipt.entrance_music_requested =
        receipt.entrance_full_start_receipt.entranceMusicRequested ? 1 : 0;
    receipt.entrance_door_open_frame_ready =
        (receipt.entrance_full_start_receipt.drawDoorFrame &&
         receipt.entrance_full_start_receipt.doorFrameIndex ==
             DM1_V1_ENTRANCE_DOOR_OPEN_FRAME_INDEX_PC34)
            ? 1
            : 0;
    receipt.hoc_menu_route_ready =
        (receipt.champion_select_route.handled &&
         receipt.champion_select_route.route ==
             DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
         receipt.champion_select_route.state == DM1_ENTRANCE_VIEWING &&
         receipt.champion_select_route.selectedMirrorIndex < 0)
            ? 1
            : 0;
    receipt.champion_select_ui_ready =
        (receipt.hoc_menu_route_ready &&
         receipt.champion_select_route.showHall &&
         receipt.champion_select_route.needsRedraw)
            ? 1
            : 0;
    receipt.render_hall_mirrors =
        receipt.champion_select_route.renderHallMirrorOverlay ? 1 : 0;
    receipt.render_overlay_command_count =
        receipt.champion_select_route.renderOverlayCommandCount;
    if (receipt.render_overlay_command_count > 0 &&
        receipt.render_overlay_command_count <=
            DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34) {
        int i;
        for (i = 0; i < receipt.render_overlay_command_count; ++i) {
            receipt.render_overlay_commands[i] =
                receipt.champion_select_route.renderOverlayCommands[i];
        }
        receipt.render_overlay_commands_ready =
            (receipt.render_overlay_commands[0].valid &&
             receipt.render_overlay_commands[0].kind ==
                 DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
             receipt.render_overlay_commands[0].clearStalePanelFirst &&
             receipt.render_overlay_commands[0].suppressThingPayloads &&
             receipt.render_overlay_commands[0].blockEnterUntilChampionSelected)
                ? 1
                : 0;
    }
    receipt.clear_stale_champion_panel =
        receipt.champion_select_route.clearStaleChampionMirrorOverlay ? 1 : 0;
    receipt.block_enter_until_champion_selected =
        receipt.champion_select_route.blockEnterUntilChampionSelected ? 1 : 0;
    receipt.runtime_first_frame_ready =
        receipt.title_surface_released &&
        receipt.entrance_wait_consumed &&
        receipt.full_start_render_ready &&
        receipt.entrance_map_ready &&
        receipt.entrance_music_requested &&
        receipt.entrance_door_open_frame_ready &&
        receipt.champion_select_ui_ready &&
        receipt.render_hall_mirrors &&
        receipt.render_overlay_commands_ready &&
        receipt.clear_stale_champion_panel &&
        receipt.block_enter_until_champion_selected;
    receipt.suppress_host_fallback_visuals =
        receipt.runtime_first_frame_ready ? 1 : 0;
    if (receipt.runtime_first_frame_ready) {
        DM1_V1_StartupHoCRenderCommand_PC34* command;

        /* ReDMCSB ENTRANCE.C F0797 draws the C255 micro-dungeon behind the
         * fully opened doors, then ENTRANCE.C F0441 leaves the Hall route in
         * VIEWING state.  Expose the exact first-frame draw plan here so the
         * host cannot fall back to stale TITLE/door/panel visuals. */
        command = &receipt.hoc_render_commands[receipt.hoc_render_command_count++];
        command->valid = 1;
        command->kind =
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_ENTRANCE_OPEN_FRAME_PC34;
        command->map_index = receipt.entrance_full_start_receipt.mapIndex;
        command->door_frame_index =
            receipt.entrance_full_start_receipt.doorFrameIndex;
        command->suppress_host_fallback_visuals = 1;
        command->source_evidence =
            "ReDMCSB ENTRANCE.C:68-80 F0797 C255 entrance draw";

        command = &receipt.hoc_render_commands[receipt.hoc_render_command_count++];
        command->valid = 1;
        command->kind = DM1_V1_STARTUP_HOC_RENDER_COMMAND_CLEAR_CHAMPION_PANEL_PC34;
        command->clear_stale_panel_first = 1;
        command->suppress_host_fallback_visuals = 1;
        command->source_evidence =
            "ReDMCSB ENTRANCE.C:850-883 starts Hall before mirror selection";

        command = &receipt.hoc_render_commands[receipt.hoc_render_command_count++];
        command->valid = 1;
        command->kind = DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34;
        command->overlay_kind = receipt.render_overlay_commands[0].kind;
        command->overlay_command_index = 0;
        command->clear_stale_panel_first =
            receipt.render_overlay_commands[0].clearStalePanelFirst;
        command->suppress_host_fallback_visuals = 1;
        command->block_enter_until_champion_selected =
            receipt.render_overlay_commands[0].blockEnterUntilChampionSelected;
        command->source_evidence =
            "ReDMCSB ENTRANCE.C:850-883 Hall waits for champion choice";
    }
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:850-883; ENTRANCE.C:68-80";
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
    const DM1_V1_StartupHoCFirstFrameReceipt_PC34* receipt,
    DM1_V1_StartupHoCHostRenderPlan_PC34* out_plan) {
    DM1_V1_StartupHoCHostRenderPlan_PC34 plan;
    const DM1_V1_StartupHoCRenderCommand_PC34* entrance_command;
    const DM1_V1_StartupHoCRenderCommand_PC34* clear_command;
    const DM1_V1_StartupHoCRenderCommand_PC34* mirror_command;

    if (!receipt || !out_plan) {
        return 0;
    }
    memset(&plan, 0, sizeof(plan));
    if (!receipt->handled) {
        *out_plan = plan;
        return 1;
    }
    plan.handled = 1;
    plan.command_count = receipt->hoc_render_command_count;
    plan.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!receipt->runtime_first_frame_ready ||
        receipt->hoc_render_command_count != 3) {
        *out_plan = plan;
        return 1;
    }

    entrance_command = &receipt->hoc_render_commands[0];
    clear_command = &receipt->hoc_render_commands[1];
    mirror_command = &receipt->hoc_render_commands[2];
    /* ReDMCSB source order gives the host one legal first HoC frame:
     * TITLE.C has released the title surface, ENTRANCE.C F0797 has drawn the
     * opened C255 entrance view, and F0441 is waiting in the Hall before any
     * mirror/C040 champion panel exists.  Collapse the DM1 render commands
     * into a host-ready plan so M11/M12 do not infer from loose fields. */
    if (!entrance_command->valid ||
        entrance_command->kind !=
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_ENTRANCE_OPEN_FRAME_PC34 ||
        !clear_command->valid ||
        clear_command->kind !=
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_CLEAR_CHAMPION_PANEL_PC34 ||
        !mirror_command->valid ||
        mirror_command->kind !=
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34) {
        *out_plan = plan;
        return 1;
    }

    plan.ready = 1;
    plan.consume_dm1_receipt_only = 1;
    plan.draw_opened_entrance_frame = 1;
    plan.entrance_map_index = entrance_command->map_index;
    plan.entrance_door_frame_index = entrance_command->door_frame_index;
    plan.clear_champion_panel = clear_command->clear_stale_panel_first;
    plan.render_hall_mirror_overlay = 1;
    plan.hall_mirror_overlay_kind = mirror_command->overlay_kind;
    plan.suppress_host_fallback_visuals =
        entrance_command->suppress_host_fallback_visuals &&
        clear_command->suppress_host_fallback_visuals &&
        mirror_command->suppress_host_fallback_visuals;
    plan.block_enter_until_champion_selected =
        mirror_command->block_enter_until_champion_selected;
    *out_plan = plan;
    return 1;
}

int dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
    const DM1_V1_StartupHoCHostRenderPlan_PC34* plan,
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34* out_proof) {
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34 proof;

    if (!plan || !out_proof) {
        return 0;
    }
    memset(&proof, 0, sizeof(proof));
    if (!plan->handled) {
        *out_proof = proof;
        return 1;
    }

    proof.handled = 1;
    proof.capture_required = 1;
    proof.command_count = plan->command_count;
    proof.capture_phase = "dm1-v1-hoc-first-frame-full-graphics";
    proof.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!plan->ready || !plan->consume_dm1_receipt_only) {
        *out_proof = proof;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 has already released the title surface, while
     * ENTRANCE.C F0797 and F0441 leave the opened C255 entrance/Hall state.
     * Package builds can consume this proof receipt directly instead of
     * re-inferring first-frame capture rules in the host. */
    proof.ready = 1;
    proof.consume_host_render_plan_only = 1;
    proof.packaged_full_graphics_proof_ready = 1;
    proof.expected_map_index = plan->entrance_map_index;
    proof.expected_map_width = DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34;
    proof.expected_map_height = DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34;
    proof.expected_entrance_door_frame_index =
        plan->entrance_door_frame_index;
    proof.expected_hall_overlay_kind = plan->hall_mirror_overlay_kind;
    proof.require_opened_entrance_frame = plan->draw_opened_entrance_frame;
    proof.require_clear_champion_panel = plan->clear_champion_panel;
    proof.require_hall_mirror_overlay = plan->render_hall_mirror_overlay;
    proof.require_no_title_surface = 1;
    proof.require_no_closed_door_frame = 1;
    proof.require_no_host_fallback_visuals =
        plan->suppress_host_fallback_visuals;
    proof.require_lower_level_renderer_helper = 1;
    proof.require_lower_level_audio_helper = 1;
    proof.block_enter_until_champion_selected =
        plan->block_enter_until_champion_selected;
    *out_proof = proof;
    return 1;
}

int dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
    const DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34* proof,
    DM1_V1_StartupHoCProductionFullStartHook_PC34* out_hook) {
    DM1_V1_StartupHoCProductionFullStartHook_PC34 hook;

    if (!proof || !out_hook) {
        return 0;
    }
    memset(&hook, 0, sizeof(hook));
    if (!proof->handled) {
        *out_hook = hook;
        return 1;
    }

    hook.handled = 1;
    hook.capture_phase = proof->capture_phase;
    hook.source_evidence =
        "ReDMCSB TITLE.C:385-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!proof->ready || !proof->consume_host_render_plan_only ||
        !proof->packaged_full_graphics_proof_ready ||
        !proof->capture_required ||
        proof->expected_map_index != DM1_V1_ENTRANCE_MAP_INDEX_PC34 ||
        proof->expected_map_width != DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 ||
        proof->expected_map_height != DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 ||
        !proof->require_opened_entrance_frame ||
        !proof->require_clear_champion_panel ||
        !proof->require_hall_mirror_overlay ||
        !proof->require_no_title_surface ||
        !proof->require_no_closed_door_frame ||
        !proof->require_no_host_fallback_visuals ||
        !proof->require_lower_level_renderer_helper ||
        !proof->require_lower_level_audio_helper) {
        *out_hook = hook;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 finishes/freezes title work before ENTRANCE.C
     * F0797/F0441 enters Hall.  This is the production hook consumed by
     * Firestaff packaging/capture: render the DM1-owned HoC first frame, then
     * capture and publish proof, before accepting Hall input. */
    hook.ready = 1;
    hook.consume_dm1_startup_receipts_only = 1;
    hook.run_before_hoc_input = 1;
    hook.draw_opened_entrance_frame = 1;
    hook.clear_champion_panel = 1;
    hook.render_hall_mirror_overlay = 1;
    hook.suppress_host_fallback_visuals = 1;
    hook.lower_level_renderer_helper_owned =
        proof->require_lower_level_renderer_helper;
    hook.lower_level_audio_helper_owned =
        proof->require_lower_level_audio_helper;
    hook.capture_after_first_frame_render = 1;
    hook.publish_packaged_full_graphics_proof = 1;
    hook.expected_map_index = proof->expected_map_index;
    hook.expected_map_width = proof->expected_map_width;
    hook.expected_map_height = proof->expected_map_height;
    hook.expected_entrance_door_frame_index =
        proof->expected_entrance_door_frame_index;
    hook.expected_hall_overlay_kind = proof->expected_hall_overlay_kind;
    hook.block_enter_until_champion_selected =
        proof->block_enter_until_champion_selected;
    *out_hook = hook;
    return 1;
}

int dm1_v1_startup_hoc_full_start_production_receipt_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* post_plan,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_hoc_first_frame_receipt_pc34(source_id,
                                                     post_plan,
                                                     outcome,
                                                     &receipt.first_frame)) {
        return 0;
    }
    if (!receipt.first_frame.handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_post_launch_plan = post_plan ? 1 : 0;
    receipt.consumed_handoff_outcome = outcome ? 1 : 0;
    receipt.consumed_title_menu_plan =
        (post_plan &&
         post_plan->required &&
         post_plan->title_menu_eligible &&
         !post_plan->title_keep_surface &&
         post_plan->title_consume_pending_input &&
         post_plan->entrance_wait_stage ==
             DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34)
            ? 1
            : 0;
    receipt.consumed_entrance_full_start_plan =
        (post_plan &&
         post_plan->entrance_full_start_receipt.valid &&
         post_plan->entrance_full_start_receipt.mapIndex ==
             DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
         post_plan->entrance_full_start_receipt.width ==
             DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
         post_plan->entrance_full_start_receipt.height ==
             DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
         post_plan->entrance_full_start_receipt.doorFrameIndex ==
             DM1_V1_ENTRANCE_DOOR_OPEN_FRAME_INDEX_PC34)
            ? 1
            : 0;
    receipt.title_surface_released = receipt.first_frame.title_surface_released;
    receipt.entrance_wait_consumed = receipt.first_frame.entrance_wait_consumed;
    receipt.first_frame_ready = receipt.first_frame.runtime_first_frame_ready;
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";

    if (!dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
            &receipt.first_frame,
            &receipt.host_render_plan) ||
        !dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
            &receipt.host_render_plan,
            &receipt.packaged_proof) ||
        !dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
            &receipt.packaged_proof,
            &receipt.production_hook)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 hands off only after title/PRESENTS are complete;
     * ENTRANCE.C F0797/F0441 then owns the first Hall frame.  Keep that full
     * production chain behind one DM1 receipt so M11/M12/package capture do
     * not mix title, entrance, and HoC overlay decisions independently. */
    receipt.host_render_plan_ready = receipt.host_render_plan.ready;
    receipt.packaged_full_graphics_proof_ready = receipt.packaged_proof.ready;
    receipt.production_hook_ready = receipt.production_hook.ready;
    receipt.ready = receipt.first_frame_ready &&
                    receipt.consumed_title_menu_plan &&
                    receipt.consumed_entrance_full_start_plan &&
                    receipt.host_render_plan_ready &&
                    receipt.packaged_full_graphics_proof_ready &&
                    receipt.production_hook_ready;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
    const DM1_V1_StartupHoCFullStartProductionReceipt_PC34* receipt,
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* out_artifact) {
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34 artifact;

    if (!receipt || !out_artifact) {
        return 0;
    }
    memset(&artifact, 0, sizeof(artifact));
    if (!receipt->handled) {
        *out_artifact = artifact;
        return 1;
    }

    artifact.handled = 1;
    artifact.capture_phase = receipt->production_hook.capture_phase;
    artifact.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!receipt->ready ||
        !receipt->title_surface_released ||
        !receipt->entrance_wait_consumed ||
        !receipt->first_frame_ready ||
        !receipt->host_render_plan_ready ||
        !receipt->packaged_full_graphics_proof_ready ||
        !receipt->production_hook_ready ||
        !receipt->production_hook.consume_dm1_startup_receipts_only ||
        !receipt->production_hook.capture_after_first_frame_render ||
        !receipt->production_hook.publish_packaged_full_graphics_proof ||
        !receipt->production_hook.lower_level_renderer_helper_owned ||
        !receipt->production_hook.lower_level_audio_helper_owned ||
        !receipt->packaged_proof.require_no_title_surface ||
        !receipt->packaged_proof.require_no_closed_door_frame ||
        !receipt->packaged_proof.require_no_host_fallback_visuals ||
        !receipt->packaged_proof.require_lower_level_renderer_helper ||
        !receipt->packaged_proof.require_lower_level_audio_helper ||
        receipt->first_frame.hoc_render_command_count != 3) {
        *out_artifact = artifact;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 completes the title surface before ENTRANCE.C
     * F0797/F0441 draws and waits in Hall.  This artifact is the capture-side
     * consumer: one DM1-owned manifest says what the packaged full-graphics
     * proof must capture, and what stale host surfaces are forbidden. */
    artifact.ready = 1;
    artifact.consume_full_start_production_receipt_only = 1;
    artifact.capture_manifest_ready = 1;
    artifact.capture_after_first_frame_render = 1;
    artifact.publish_packaged_full_graphics_proof = 1;
    artifact.title_surface_forbidden = 1;
    artifact.closed_door_frame_forbidden = 1;
    artifact.host_fallback_visuals_forbidden = 1;
    artifact.lower_level_renderer_helper_owned =
        receipt->production_hook.lower_level_renderer_helper_owned;
    artifact.lower_level_audio_helper_owned =
        receipt->production_hook.lower_level_audio_helper_owned;
    artifact.opened_entrance_frame_required =
        receipt->production_hook.draw_opened_entrance_frame;
    artifact.hall_mirror_overlay_required =
        receipt->production_hook.render_hall_mirror_overlay;
    artifact.clear_champion_panel_required =
        receipt->production_hook.clear_champion_panel;
    artifact.block_enter_until_champion_selected =
        receipt->production_hook.block_enter_until_champion_selected;
    artifact.expected_map_index = receipt->production_hook.expected_map_index;
    artifact.expected_map_width = receipt->production_hook.expected_map_width;
    artifact.expected_map_height = receipt->production_hook.expected_map_height;
    artifact.expected_entrance_door_frame_index =
        receipt->production_hook.expected_entrance_door_frame_index;
    artifact.expected_hall_overlay_kind =
        receipt->production_hook.expected_hall_overlay_kind;
    artifact.expected_hoc_render_command_count =
        receipt->first_frame.hoc_render_command_count;
    *out_artifact = artifact;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* artifact,
    const DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34 receipt;

    if (!artifact || !facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!artifact->handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_capture_artifact = 1;
    receipt.consumed_capture_facts = 1;
    receipt.real_asset_capture = facts->captured_from_real_assets ? 1 : 0;
    receipt.mac_window_capture = facts->captured_from_mac_window ? 1 : 0;
    receipt.release_app_capture =
        facts->captured_from_release_app ? 1 : 0;
    receipt.redmcsb_c026_asset_present =
        facts->observed_c026_portrait_asset ? 1 : 0;
    receipt.redmcsb_c346_asset_present =
        facts->observed_c346_mirror_backing_asset ? 1 : 0;
    receipt.hoc_asset_capture =
        receipt.real_asset_capture &&
        receipt.redmcsb_c026_asset_present &&
        receipt.redmcsb_c346_asset_present;
    receipt.required_asset_capture =
        receipt.real_asset_capture &&
        facts->observed_required_graphics_hash_match &&
        facts->observed_required_dungeon_hash_match;
    receipt.host_window_capture =
        receipt.mac_window_capture && facts->observed_host_window_present;
    /* ReDMCSB DRAWVIEW.C F0097 publishes the full 320x200 viewport buffer
     * after ENTRANCE.C F0797/F0438 has composed the entrance/HoC frame.
     * A release/app Mac capture must therefore prove a presented surface
     * large enough to contain that source frame, not just an SDL window. */
    receipt.presented_capture =
        facts->observed_presented_rgba_capture ? 1 : 0;
    receipt.presented_capture_geometry_matches =
        facts->presented_capture_width >= 320 &&
        facts->presented_capture_height >= 200;
    receipt.presented_capture_pixels_present =
        facts->presented_capture_byte_count >=
            facts->presented_capture_width *
            facts->presented_capture_height * 4 &&
        facts->presented_capture_hash != 0U;
    receipt.presented_capture_byte_count = facts->presented_capture_byte_count;
    receipt.presented_capture_hash = facts->presented_capture_hash;
    receipt.presented_capture_consumer_mask =
        facts->presented_capture_consumer_mask;
    receipt.presented_capture_chain_hash =
        facts->presented_capture_chain_hash;
    receipt.presented_capture_chain_ready =
        receipt.presented_capture_pixels_present &&
        receipt.presented_capture_chain_hash ==
            dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
                facts->presented_capture_width,
                facts->presented_capture_height,
                facts->presented_capture_byte_count,
                facts->presented_capture_hash,
                facts->presented_capture_consumer_mask);
    receipt.release_app_identity_ready =
        receipt.release_app_capture &&
        receipt.mac_window_capture &&
        receipt.host_window_capture &&
        receipt.real_asset_capture &&
        receipt.required_asset_capture &&
        receipt.presented_capture_chain_ready;
    receipt.release_app_identity_hash =
        receipt.release_app_identity_ready
            ? dm1_v1_startup_hoc_release_app_identity_hash_pc34(
                  "dm1",
                  dm1_v1_startup_hoc_capture_consumer_hash_pc34(
                      facts->presented_capture_consumer_mask),
                  dm1_v1_startup_hoc_host_capture_route_hash_pc34(
                      facts->presented_capture_consumer_mask,
                      receipt.presented_capture_chain_hash,
                      receipt.presented_capture_hash),
                  receipt.presented_capture_chain_hash)
            : 0u;
    receipt.capture_phase = artifact->capture_phase;
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!artifact->ready ||
        !artifact->consume_full_start_production_receipt_only ||
        !artifact->capture_manifest_ready ||
        !facts->captured_after_first_frame_render) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 releases title assets before ENTRANCE.C F0797
     * draws C255 and F0441 waits in Hall.  The capture proof is therefore a
     * DM1-owned verdict over observed facts, not another host-side inference. */
    receipt.geometry_matches =
        facts->captured_map_index == artifact->expected_map_index &&
        facts->captured_map_width == artifact->expected_map_width &&
        facts->captured_map_height == artifact->expected_map_height;
    receipt.entrance_frame_matches =
        facts->captured_entrance_door_frame_index ==
        artifact->expected_entrance_door_frame_index;
    receipt.hall_overlay_matches =
        facts->captured_hall_overlay_kind ==
        artifact->expected_hall_overlay_kind;
    receipt.command_count_matches =
        facts->captured_hoc_render_command_count ==
        artifact->expected_hoc_render_command_count;
    receipt.host_capture_route_matches =
        receipt.hoc_asset_capture &&
        receipt.required_asset_capture &&
        receipt.host_window_capture &&
        receipt.presented_capture &&
        receipt.presented_capture_geometry_matches &&
        receipt.presented_capture_pixels_present &&
        receipt.presented_capture_chain_ready &&
        receipt.release_app_identity_ready &&
        receipt.release_app_capture;
    receipt.stale_title_absent =
        artifact->title_surface_forbidden && !facts->saw_title_surface;
    receipt.stale_door_absent =
        artifact->closed_door_frame_forbidden && !facts->saw_closed_door_frame;
    receipt.host_fallback_absent =
        artifact->host_fallback_visuals_forbidden &&
        !facts->saw_host_fallback_visuals;
    receipt.required_layers_present =
        (!artifact->opened_entrance_frame_required ||
         facts->saw_opened_entrance_frame) &&
        (!artifact->hall_mirror_overlay_required ||
         facts->saw_hall_mirror_overlay) &&
        (!artifact->clear_champion_panel_required ||
         facts->cleared_champion_panel);
    receipt.input_block_matches =
        artifact->block_enter_until_champion_selected ==
        facts->blocked_enter_until_champion_selected;
    receipt.ready = 1;
    receipt.proof_passed =
        receipt.geometry_matches &&
        receipt.entrance_frame_matches &&
        receipt.hall_overlay_matches &&
        receipt.command_count_matches &&
        receipt.hoc_asset_capture &&
        receipt.required_asset_capture &&
        receipt.host_window_capture &&
        receipt.presented_capture &&
        receipt.presented_capture_geometry_matches &&
        receipt.presented_capture_pixels_present &&
        receipt.presented_capture_chain_ready &&
        receipt.release_app_capture &&
        receipt.release_app_identity_ready &&
        receipt.release_app_identity_hash != 0u &&
        receipt.stale_title_absent &&
        receipt.stale_door_absent &&
        receipt.host_fallback_absent &&
        receipt.required_layers_present &&
        receipt.input_block_matches;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_runtime_apply_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* artifact,
    const DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34* proof,
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34 receipt;

    if (!artifact || !proof || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!artifact->handled || !proof->handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_capture_artifact = 1;
    receipt.consumed_capture_proof = 1;
    receipt.require_proof_passed = 1;
    receipt.real_asset_capture = proof->real_asset_capture;
    receipt.mac_window_capture = proof->mac_window_capture;
    receipt.release_app_capture = proof->release_app_capture;
    receipt.release_app_identity_ready = proof->release_app_identity_ready;
    receipt.release_app_identity_hash = proof->release_app_identity_hash;
    receipt.host_capture_route_matches = proof->host_capture_route_matches;
    receipt.hoc_asset_capture = proof->hoc_asset_capture;
    receipt.host_window_capture = proof->host_window_capture;
    receipt.redmcsb_c026_asset_present = proof->redmcsb_c026_asset_present;
    receipt.redmcsb_c346_asset_present = proof->redmcsb_c346_asset_present;
    receipt.capture_phase = artifact->capture_phase;
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!artifact->ready ||
        !proof->ready ||
        !proof->proof_passed ||
        !artifact->consume_full_start_production_receipt_only ||
        !artifact->capture_manifest_ready ||
        !artifact->lower_level_renderer_helper_owned ||
        !artifact->lower_level_audio_helper_owned ||
        !artifact->publish_packaged_full_graphics_proof) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 has finished title/PRESENTS; ENTRANCE.C F0797
     * and F0441 own the opened C255 Hall frame.  After DM1-owned capture
     * proof passes, expose the exact runtime apply commands for M11/M12. */
    receipt.ready = 1;
    receipt.apply_before_hoc_input = 1;
    receipt.apply_opened_entrance_frame =
        artifact->opened_entrance_frame_required;
    receipt.apply_clear_champion_panel =
        artifact->clear_champion_panel_required;
    receipt.apply_hall_mirror_overlay =
        artifact->hall_mirror_overlay_required;
    receipt.suppress_title_surface = artifact->title_surface_forbidden;
    receipt.suppress_closed_door_frame = artifact->closed_door_frame_forbidden;
    receipt.suppress_host_fallback_visuals =
        artifact->host_fallback_visuals_forbidden;
    receipt.lower_level_renderer_helper_owned =
        artifact->lower_level_renderer_helper_owned;
    receipt.lower_level_audio_helper_owned =
        artifact->lower_level_audio_helper_owned;
    receipt.publish_packaged_full_graphics_proof =
        artifact->publish_packaged_full_graphics_proof;
    receipt.block_enter_until_champion_selected =
        artifact->block_enter_until_champion_selected;
    receipt.map_index = artifact->expected_map_index;
    receipt.map_width = artifact->expected_map_width;
    receipt.map_height = artifact->expected_map_height;
    receipt.entrance_door_frame_index =
        artifact->expected_entrance_door_frame_index;
    receipt.hall_overlay_kind = artifact->expected_hall_overlay_kind;
    receipt.render_command_count = artifact->expected_hoc_render_command_count;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* apply,
    const DM1_V1_StartupHoCFullGraphicsThingSuppressionFacts_PC34* facts,
    DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34 receipt;

    if (!apply || !facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!apply->handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_runtime_apply_receipt = 1;
    receipt.consumed_suppression_facts = 1;
    receipt.capture_phase = apply->capture_phase;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C:68-80/850-883; "
        "DUNVIEW.C:3916-3928/4547-4581";
    if (!apply->ready ||
        !apply->apply_before_hoc_input ||
        !apply->apply_hall_mirror_overlay ||
        !apply->suppress_title_surface ||
        !apply->suppress_closed_door_frame ||
        !apply->suppress_host_fallback_visuals) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB ENTRANCE.C F0797 builds the C255 5x5 entrance micro-dungeon
     * and F0441 waits in Hall before dungeon entry.  HoC first-frame capture
     * must therefore prove mirror overlay presence while rejecting item,
     * projectile, spell-effect, and mirror-as-thing payload leakage. */
    receipt.ready = 1;
    receipt.champion_mirror_overlay_present =
        facts->observed_hall_mirror_overlay &&
        apply->hall_overlay_kind == DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34;
    receipt.false_item_payloads_absent =
        facts->observed_false_floor_item_payload_count == 0;
    receipt.projectile_payloads_absent =
        facts->observed_projectile_payload_count == 0;
    receipt.spell_effect_payloads_absent =
        facts->observed_spell_effect_payload_count == 0;
    receipt.mirror_payload_thing_absent =
        facts->observed_mirror_payload_as_thing_count == 0;
    receipt.fallback_visuals_absent =
        !facts->observed_host_fallback_visuals &&
        apply->suppress_host_fallback_visuals;
    receipt.stale_title_absent =
        !facts->observed_title_surface && apply->suppress_title_surface;
    receipt.stale_door_absent =
        !facts->observed_closed_door_frame &&
        apply->suppress_closed_door_frame;
    receipt.command_count_matches =
        facts->observed_hoc_render_command_count == apply->render_command_count;
    receipt.enter_block_matches =
        facts->observed_enter_blocked_until_champion_selected ==
        apply->block_enter_until_champion_selected;
    receipt.proof_passed =
        receipt.champion_mirror_overlay_present &&
        receipt.false_item_payloads_absent &&
        receipt.projectile_payloads_absent &&
        receipt.spell_effect_payloads_absent &&
        receipt.mirror_payload_thing_absent &&
        receipt.fallback_visuals_absent &&
        receipt.stale_title_absent &&
        receipt.stale_door_absent &&
        receipt.command_count_matches &&
        receipt.enter_block_matches;
    receipt.walk_capture_safe = receipt.proof_passed;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_production_consumer_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* apply,
    const DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34* suppression,
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34 receipt;

    if (!apply || !suppression || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!apply->handled || !suppression->handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_runtime_apply_receipt = 1;
    receipt.consumed_thing_suppression_receipt = 1;
    receipt.real_asset_capture = apply->real_asset_capture;
    receipt.mac_window_capture = apply->mac_window_capture;
    receipt.release_app_capture = apply->release_app_capture;
    receipt.release_app_identity_ready = apply->release_app_identity_ready;
    receipt.release_app_identity_hash = apply->release_app_identity_hash;
    receipt.host_capture_route_matches = apply->host_capture_route_matches;
    receipt.hoc_asset_capture = apply->hoc_asset_capture;
    receipt.host_window_capture = apply->host_window_capture;
    receipt.capture_phase = apply->capture_phase;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!apply->ready ||
        !suppression->ready ||
        !suppression->proof_passed ||
        !suppression->walk_capture_safe ||
        !suppression->champion_mirror_overlay_present ||
        !suppression->false_item_payloads_absent ||
        !suppression->projectile_payloads_absent ||
        !suppression->spell_effect_payloads_absent ||
        !suppression->mirror_payload_thing_absent ||
        !apply->apply_hall_mirror_overlay ||
        !apply->lower_level_renderer_helper_owned ||
        !apply->lower_level_audio_helper_owned ||
        apply->hall_overlay_kind != DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB ENTRANCE.C F0797/F0441 leaves one legal production frame before
     * HoC input: opened entrance plus Hall mirror overlay, with no dungeon
     * thing/effect payloads.  This receipt is the final DM1-owned consumer
     * contract for M11/M12/package callers. */
    receipt.ready = 1;
    receipt.consume_dm1_receipts_only = 1;
    receipt.real_asset_capture = apply->real_asset_capture;
    receipt.mac_window_capture = apply->mac_window_capture;
    receipt.release_app_capture = apply->release_app_capture;
    receipt.release_app_identity_ready = apply->release_app_identity_ready;
    receipt.release_app_identity_hash = apply->release_app_identity_hash;
    receipt.host_capture_route_matches = apply->host_capture_route_matches;
    receipt.hoc_asset_capture = apply->hoc_asset_capture;
    receipt.host_window_capture = apply->host_window_capture;
    receipt.execute_before_hoc_input = apply->apply_before_hoc_input;
    receipt.draw_opened_entrance_frame = apply->apply_opened_entrance_frame;
    receipt.clear_champion_panel = apply->apply_clear_champion_panel;
    receipt.render_hall_mirror_overlay = apply->apply_hall_mirror_overlay;
    receipt.suppress_title_surface = apply->suppress_title_surface;
    receipt.suppress_closed_door_frame = apply->suppress_closed_door_frame;
    receipt.suppress_host_fallback_visuals =
        apply->suppress_host_fallback_visuals;
    receipt.lower_level_renderer_helper_owned =
        apply->lower_level_renderer_helper_owned;
    receipt.lower_level_audio_helper_owned =
        apply->lower_level_audio_helper_owned;
    receipt.suppress_false_item_payloads =
        suppression->false_item_payloads_absent;
    receipt.suppress_projectile_payloads =
        suppression->projectile_payloads_absent;
    receipt.suppress_spell_effect_payloads =
        suppression->spell_effect_payloads_absent;
    receipt.suppress_mirror_payload_things =
        suppression->mirror_payload_thing_absent;
    receipt.publish_packaged_full_graphics_proof =
        apply->publish_packaged_full_graphics_proof;
    receipt.redmcsb_c026_portrait_overlay_ready =
        suppression->champion_mirror_overlay_present &&
        apply->redmcsb_c026_asset_present;
    receipt.redmcsb_c346_mirror_backing_ready =
        apply->apply_hall_mirror_overlay &&
        apply->hall_overlay_kind == DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
        apply->redmcsb_c346_asset_present;
    receipt.redmcsb_f0115_thing_layer_suppression_ready =
        suppression->false_item_payloads_absent &&
        suppression->projectile_payloads_absent &&
        suppression->spell_effect_payloads_absent &&
        suppression->mirror_payload_thing_absent;
    receipt.block_enter_until_champion_selected =
        apply->block_enter_until_champion_selected;
    receipt.map_index = apply->map_index;
    receipt.map_width = apply->map_width;
    receipt.map_height = apply->map_height;
    receipt.entrance_door_frame_index = apply->entrance_door_frame_index;
    receipt.hall_overlay_kind = apply->hall_overlay_kind;
    receipt.render_command_count = apply->render_command_count;
    receipt.walk_capture_safe = suppression->walk_capture_safe;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_host_probe_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* out_apply,
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34* out_consumer) {
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 post_plan;
    DM1_V1_StartupHandoffOutcome_PC34 outcome;
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34 production;
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34 artifact;
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34 capture_facts;
    DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34 proof;
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34 apply;
    DM1_V1_StartupHoCFullGraphicsThingSuppressionFacts_PC34
        suppression_facts;
    DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34 suppression;
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34 consumer;

    if (out_apply) {
        memset(out_apply, 0, sizeof(*out_apply));
    }
    if (out_consumer) {
        memset(out_consumer, 0, sizeof(*out_consumer));
    }
    if (!facts || !facts->source_id ||
        strcmp(facts->source_id, "dm1") != 0 ||
        !facts->dungeon_loaded ||
        facts->map_count <= 0) {
        return 0;
    }

    memset(&post_plan, 0, sizeof(post_plan));
    memset(&outcome, 0, sizeof(outcome));
    memset(&production, 0, sizeof(production));
    memset(&artifact, 0, sizeof(artifact));
    memset(&capture_facts, 0, sizeof(capture_facts));
    memset(&proof, 0, sizeof(proof));
    memset(&apply, 0, sizeof(apply));
    memset(&suppression_facts, 0, sizeof(suppression_facts));
    memset(&suppression, 0, sizeof(suppression));
    memset(&consumer, 0, sizeof(consumer));

    if (!dm1_v1_startup_handoff_post_launch_plan_pc34(facts->source_id,
                                                       &post_plan) ||
        !dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            facts->entrance_command,
            &outcome)) {
        return 0;
    }
    outcome.title_played = facts->title_played ? 1 : 0;

    if (!dm1_v1_startup_hoc_full_start_production_receipt_pc34(
            facts->source_id,
            &post_plan,
            &outcome,
            &production) ||
        !dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
            &production,
            &artifact)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 completes the title before ENTRANCE.C
     * F0797/F0441 draws the first Hall frame.  M11 supplies observed host
     * facts only; DM1 owns the expected HoC capture geometry and suppression
     * verdicts. */
    capture_facts.captured_after_first_frame_render =
        facts->captured_after_first_frame_render;
    capture_facts.captured_from_mac_window =
        facts->captured_from_mac_window;
    capture_facts.captured_from_release_app =
        facts->captured_from_release_app;
    capture_facts.observed_c026_portrait_asset =
        facts->observed_c026_portrait_asset;
    capture_facts.observed_c346_mirror_backing_asset =
        facts->observed_c346_mirror_backing_asset;
    capture_facts.observed_required_graphics_hash_match =
        facts->observed_required_graphics_hash_match;
    capture_facts.observed_required_dungeon_hash_match =
        facts->observed_required_dungeon_hash_match;
    capture_facts.captured_from_real_assets =
        facts->captured_from_real_assets &&
        facts->observed_required_graphics_hash_match &&
        facts->observed_required_dungeon_hash_match;
    capture_facts.observed_host_window_present =
        facts->observed_host_window_present;
    capture_facts.observed_presented_rgba_capture =
        facts->observed_presented_rgba_capture;
    capture_facts.presented_capture_width =
        facts->presented_capture_width;
    capture_facts.presented_capture_height =
        facts->presented_capture_height;
    capture_facts.presented_capture_byte_count =
        facts->presented_capture_byte_count;
    capture_facts.presented_capture_hash =
        facts->presented_capture_hash;
    capture_facts.presented_capture_consumer_mask =
        facts->presented_capture_consumer_mask;
    capture_facts.presented_capture_chain_hash =
        facts->presented_capture_chain_hash;
    capture_facts.captured_map_index = artifact.expected_map_index;
    capture_facts.captured_map_width = artifact.expected_map_width;
    capture_facts.captured_map_height = artifact.expected_map_height;
    capture_facts.captured_entrance_door_frame_index =
        artifact.expected_entrance_door_frame_index;
    capture_facts.captured_hall_overlay_kind =
        artifact.expected_hall_overlay_kind;
    capture_facts.captured_hoc_render_command_count =
        artifact.expected_hoc_render_command_count;
    capture_facts.saw_title_surface = facts->observed_title_surface;
    capture_facts.saw_closed_door_frame = facts->observed_closed_door_frame;
    capture_facts.saw_host_fallback_visuals =
        facts->observed_host_fallback_visuals;
    capture_facts.saw_opened_entrance_frame =
        artifact.opened_entrance_frame_required;
    capture_facts.saw_hall_mirror_overlay =
        artifact.hall_mirror_overlay_required;
    capture_facts.cleared_champion_panel =
        artifact.clear_champion_panel_required;
    capture_facts.blocked_enter_until_champion_selected =
        artifact.block_enter_until_champion_selected;

    if (!dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(
            &artifact,
            &capture_facts,
            &proof) ||
        !dm1_v1_startup_hoc_full_graphics_runtime_apply_receipt_pc34(
            &artifact,
            &proof,
            &apply)) {
        return 0;
    }

    suppression_facts.observed_hall_mirror_overlay =
        apply.apply_hall_mirror_overlay;
    suppression_facts.observed_false_floor_item_payload_count =
        facts->observed_false_floor_item_payload_count;
    suppression_facts.observed_projectile_payload_count =
        facts->observed_projectile_payload_count;
    suppression_facts.observed_spell_effect_payload_count =
        facts->observed_spell_effect_payload_count;
    suppression_facts.observed_mirror_payload_as_thing_count =
        facts->observed_mirror_payload_as_thing_count;
    suppression_facts.observed_host_fallback_visuals =
        facts->observed_host_fallback_visuals;
    suppression_facts.observed_title_surface = facts->observed_title_surface;
    suppression_facts.observed_closed_door_frame =
        facts->observed_closed_door_frame;
    suppression_facts.observed_enter_blocked_until_champion_selected =
        apply.block_enter_until_champion_selected;
    suppression_facts.observed_hoc_render_command_count =
        apply.render_command_count;
    if (!dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
            &apply,
            &suppression_facts,
            &suppression) ||
        !dm1_v1_startup_hoc_full_graphics_production_consumer_receipt_pc34(
            &apply,
            &suppression,
            &consumer)) {
        return 0;
    }

    if (out_apply) {
        *out_apply = apply;
    }
    if (out_consumer) {
        *out_consumer = consumer;
    }
    return apply.handled && consumer.handled;
}

int dm1_v1_startup_hoc_release_app_capture_ownership_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34 apply;
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34 consumer;
    DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    memset(&apply, 0, sizeof(apply));
    memset(&consumer, 0, sizeof(consumer));
    if (!facts) {
        *out_receipt = receipt;
        return 0;
    }
    if (!dm1_v1_startup_hoc_full_graphics_host_probe_receipt_pc34(
            facts, &apply, &consumer)) {
        *out_receipt = receipt;
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0797/F0441 define the release
     * app capture boundary: title is gone, C255 Hall is open, and HoC mirrors
     * are drawn before input.  DUNVIEW.C:3913-3928 draws C346/C026 through the
     * wall-overlay route, so M11/M12 must feed the observed backing asset into
     * this DM1-owned receipt instead of letting a host fallback rectangle stand
     * in for the Mac/app capture. */
    receipt.handled = 1;
    receipt.consumed_host_probe_facts = 1;
    receipt.consumed_hoc_host_render_receipt =
        facts->consumed_hoc_host_render_receipt ? 1 : 0;
    receipt.consumed_m11_boot_probe_consumer =
        facts->consumed_m11_boot_probe_consumer ? 1 : 0;
    receipt.consumed_m12_startup_capture_consumer =
        facts->consumed_m12_startup_capture_consumer ? 1 : 0;
    if (receipt.consumed_hoc_host_render_receipt) {
        receipt.named_consumer_mask |=
            DM1_V1_HOC_CAPTURE_CONSUMER_HOST_RENDER_PC34;
    }
    if (receipt.consumed_m11_boot_probe_consumer) {
        receipt.named_consumer_mask |=
            DM1_V1_HOC_CAPTURE_CONSUMER_M11_BOOT_PROBE_PC34;
    }
    if (receipt.consumed_m12_startup_capture_consumer) {
        receipt.named_consumer_mask |=
            DM1_V1_HOC_CAPTURE_CONSUMER_M12_STARTUP_PC34;
    }
    receipt.consumed_all_named_host_consumers =
        (receipt.named_consumer_mask ==
         DM1_V1_HOC_CAPTURE_CONSUMER_ALL_PC34)
            ? 1
            : 0;
    receipt.named_consumer_hash =
        dm1_v1_startup_hoc_capture_consumer_hash_pc34(
            receipt.named_consumer_mask);
    receipt.consumed_launch_path_receipt =
        facts->consumed_launch_path_receipt ? 1 : 0;
    receipt.consumed_runtime_apply_receipt = apply.handled ? 1 : 0;
    receipt.consumed_production_consumer_receipt = consumer.handled ? 1 : 0;
    receipt.consume_dm1_receipts_only = consumer.consume_dm1_receipts_only;
    receipt.publish_packaged_full_graphics_proof =
        consumer.publish_packaged_full_graphics_proof;
    receipt.launch_path_started_from_launcher =
        facts->launch_path_started_from_launcher ? 1 : 0;
    receipt.launch_path_intro_not_bypassed =
        facts->launch_path_intro_not_bypassed ? 1 : 0;
    receipt.real_asset_capture = consumer.real_asset_capture;
    receipt.mac_window_capture = consumer.mac_window_capture;
    receipt.release_app_capture = consumer.release_app_capture;
    receipt.host_capture_route_matches = consumer.host_capture_route_matches;
    receipt.hoc_asset_capture = consumer.hoc_asset_capture;
    /* C026/C346 prove the expected Hall composition, but only M11's actual
     * C127/F0115 requests prove that this is a live post-entrance viewport. */
    receipt.observed_live_hoc_material_request =
        facts->observed_live_hoc_c127_material_request &&
        facts->observed_live_hoc_f0115_material_request;
    receipt.consumed_required_graphics_asset =
        facts->observed_required_graphics_hash_match ? 1 : 0;
    receipt.consumed_required_dungeon_asset =
        facts->observed_required_dungeon_hash_match ? 1 : 0;
    receipt.required_asset_capture =
        receipt.real_asset_capture &&
        receipt.consumed_required_graphics_asset &&
        receipt.consumed_required_dungeon_asset;
    receipt.host_window_capture = consumer.host_window_capture;
    receipt.presented_capture =
        facts->observed_presented_rgba_capture ? 1 : 0;
    receipt.presented_capture_width = facts->presented_capture_width;
    receipt.presented_capture_height = facts->presented_capture_height;
    receipt.presented_capture_geometry_matches =
        receipt.presented_capture_width >= 320 &&
        receipt.presented_capture_height >= 200;
    receipt.presented_capture_pixels_present =
        facts->presented_capture_byte_count >=
            facts->presented_capture_width *
            facts->presented_capture_height * 4 &&
        facts->presented_capture_hash != 0U;
    receipt.presented_capture_byte_count = facts->presented_capture_byte_count;
    receipt.presented_capture_hash = facts->presented_capture_hash;
    receipt.presented_capture_consumer_mask =
        facts->presented_capture_consumer_mask;
    if (!receipt.presented_capture_consumer_mask) {
        receipt.presented_capture_consumer_mask = receipt.named_consumer_mask;
    }
    receipt.presented_capture_chain_hash = facts->presented_capture_chain_hash;
    receipt.presented_capture_chain_ready =
        receipt.presented_capture_pixels_present &&
        receipt.presented_capture_consumer_mask == receipt.named_consumer_mask &&
        receipt.presented_capture_chain_hash ==
            dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
                receipt.presented_capture_width,
                receipt.presented_capture_height,
                receipt.presented_capture_byte_count,
                receipt.presented_capture_hash,
                receipt.presented_capture_consumer_mask);
    receipt.host_capture_route_mask = receipt.named_consumer_mask;
    receipt.host_capture_route_hash =
        dm1_v1_startup_hoc_host_capture_route_hash_pc34(
            receipt.host_capture_route_mask,
            receipt.presented_capture_chain_hash,
            receipt.presented_capture_hash);
    receipt.presented_capture_route_packaged =
        receipt.presented_capture_chain_ready &&
        receipt.host_capture_route_hash != 0u;
    receipt.release_app_identity_ready =
        dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->source_id) &&
        receipt.consumed_launch_path_receipt &&
        receipt.launch_path_started_from_launcher &&
        receipt.launch_path_intro_not_bypassed &&
        receipt.consumed_all_named_host_consumers &&
        receipt.release_app_capture &&
        consumer.release_app_identity_ready &&
        receipt.mac_window_capture &&
        receipt.host_window_capture &&
        receipt.real_asset_capture &&
        receipt.required_asset_capture &&
        receipt.presented_capture_route_packaged;
    receipt.release_app_identity_hash =
        receipt.release_app_identity_ready
            ? dm1_v1_startup_hoc_release_app_identity_hash_pc34(
                  facts->source_id,
                  receipt.named_consumer_hash,
                  receipt.host_capture_route_hash,
                  receipt.presented_capture_chain_hash)
            : 0u;
    receipt.draw_opened_entrance_frame = consumer.draw_opened_entrance_frame;
    receipt.render_hall_mirror_overlay = consumer.render_hall_mirror_overlay;
    receipt.suppress_host_fallback_visuals =
        consumer.suppress_host_fallback_visuals;
    receipt.host_draw_uses_owned_receipt =
        dm1_v1_startup_hoc_host_draw_uses_owned_receipt_pc34(
            &consumer,
            facts->observed_c346_mirror_backing_asset ? 1 : 0,
            &receipt.consumed_owned_host_draw_receipt);
    receipt.host_draw_rejects_backing_fallback =
        dm1_v1_startup_hoc_host_draw_rejects_backing_fallback_pc34(
            &receipt.host_draw_consumes_backing_asset);
    receipt.lower_level_renderer_helper_owned =
        consumer.lower_level_renderer_helper_owned;
    receipt.lower_level_audio_helper_owned =
        consumer.lower_level_audio_helper_owned;
    receipt.block_enter_until_champion_selected =
        consumer.block_enter_until_champion_selected;
    receipt.map_index = consumer.map_index;
    receipt.map_width = consumer.map_width;
    receipt.map_height = consumer.map_height;
    receipt.entrance_door_frame_index = consumer.entrance_door_frame_index;
    receipt.hall_overlay_kind = consumer.hall_overlay_kind;
    receipt.render_command_count = consumer.render_command_count;
    receipt.capture_phase = consumer.capture_phase;
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    /* ReDMCSB ENTRANCE.C F0438/F0797 composes the entrance frame before
     * DRAWVIEW.C F0097 publishes the viewport and owns the C346/C026
     * mirror overlay.  Keep readiness tied to a named M11/M12 host-render
     * consumer so generic capture flags cannot substitute for that draw path. */
    receipt.ready =
        apply.ready &&
        consumer.ready &&
        receipt.consumed_all_named_host_consumers &&
        receipt.named_consumer_hash != 0u &&
        receipt.consumed_runtime_apply_receipt &&
        receipt.consumed_production_consumer_receipt &&
        receipt.consumed_launch_path_receipt &&
        receipt.launch_path_started_from_launcher &&
        receipt.launch_path_intro_not_bypassed &&
        receipt.consume_dm1_receipts_only &&
        receipt.publish_packaged_full_graphics_proof &&
        receipt.real_asset_capture &&
        receipt.mac_window_capture &&
        receipt.release_app_capture &&
        receipt.release_app_identity_ready &&
        receipt.release_app_identity_hash != 0u &&
        receipt.host_capture_route_matches &&
        receipt.presented_capture &&
        receipt.presented_capture_geometry_matches &&
        receipt.presented_capture_pixels_present &&
        receipt.presented_capture_chain_ready &&
        receipt.hoc_asset_capture &&
        receipt.observed_live_hoc_material_request &&
        receipt.required_asset_capture &&
        receipt.host_window_capture &&
        receipt.draw_opened_entrance_frame &&
        receipt.render_hall_mirror_overlay &&
        receipt.suppress_host_fallback_visuals &&
        receipt.consumed_owned_host_draw_receipt &&
        receipt.host_draw_uses_owned_receipt &&
        receipt.host_draw_consumes_backing_asset &&
        receipt.host_draw_rejects_backing_fallback &&
        receipt.lower_level_renderer_helper_owned &&
        receipt.lower_level_audio_helper_owned &&
        receipt.block_enter_until_champion_selected &&
        receipt.render_command_count == 3;
    receipt.host_capture_route_packaged =
        receipt.ready &&
        receipt.host_capture_route_mask ==
            DM1_V1_HOC_CAPTURE_CONSUMER_ALL_PC34 &&
        receipt.host_capture_route_hash ==
            dm1_v1_startup_hoc_host_capture_route_hash_pc34(
                receipt.named_consumer_mask,
                receipt.presented_capture_chain_hash,
                receipt.presented_capture_hash) &&
        receipt.presented_capture_route_packaged;
    receipt.ready = receipt.ready && receipt.host_capture_route_packaged;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_presented_capture_publish_receipt_pc34(
    const DM1_V1_StartupHoCPresentedCapturePublishFacts_PC34* facts,
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34 receipt;
    unsigned int mask;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!facts || !facts->source_id || strcmp(facts->source_id, "dm1") != 0) {
        *out_receipt = receipt;
        return facts ? 1 : 0;
    }

    receipt.handled = 1;
    receipt.presented_capture_ready =
        facts->presented_capture_ready ? 1 : 0;
    receipt.host_window_present = facts->host_window_present ? 1 : 0;
    receipt.captured_from_mac_window =
        facts->captured_from_mac_window ? 1 : 0;
    receipt.captured_from_release_app =
        facts->captured_from_release_app ? 1 : 0;
    receipt.width = facts->width;
    receipt.height = facts->height;
    receipt.byte_count = facts->byte_count;
    receipt.framebuffer_hash = facts->framebuffer_hash;
    mask = facts->required_consumer_mask;
    if (!mask) {
        mask = DM1_V1_HOC_CAPTURE_CONSUMER_HOST_RENDER_PC34 |
               DM1_V1_HOC_CAPTURE_CONSUMER_M11_BOOT_PROBE_PC34 |
               DM1_V1_HOC_CAPTURE_CONSUMER_M12_STARTUP_PC34;
    }
    receipt.consumer_mask = mask;
    receipt.chain_hash =
        dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
            receipt.width,
            receipt.height,
            receipt.byte_count,
            receipt.framebuffer_hash,
            receipt.consumer_mask);
    receipt.geometry_matches = receipt.width >= 320 && receipt.height >= 200;
    receipt.pixels_present =
        receipt.byte_count >= receipt.width * receipt.height * 4 &&
        receipt.framebuffer_hash != 0U;
    receipt.source_evidence =
        "ReDMCSB DRAWVIEW.C F0097; ENTRANCE.C F0797/F0441";
    receipt.ready =
        receipt.presented_capture_ready &&
        receipt.host_window_present &&
        receipt.captured_from_mac_window &&
        receipt.captured_from_release_app &&
        receipt.geometry_matches &&
        receipt.pixels_present &&
        receipt.consumer_mask == DM1_V1_HOC_CAPTURE_CONSUMER_ALL_PC34 &&
        receipt.chain_hash != 0U;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_presented_capture_publish_from_boot_summary_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCPresentedCapturePublishFacts_PC34 facts;

    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!summary || !summary->handled) {
        return 0;
    }
    memset(&facts, 0, sizeof(facts));
    facts.source_id = "dm1";
    facts.presented_capture_ready = summary->presented_capture;
    facts.host_window_present = summary->host_window_capture;
    facts.captured_from_mac_window = summary->mac_window_capture;
    facts.captured_from_release_app = summary->release_app_capture;
    facts.width = summary->presented_capture_width;
    facts.height = summary->presented_capture_height;
    facts.byte_count = summary->presented_capture_bytes;
    facts.framebuffer_hash = summary->presented_capture_hash;
    facts.required_consumer_mask = summary->presented_capture_consumer_mask;
    return dm1_v1_startup_hoc_presented_capture_publish_receipt_pc34(
        &facts,
        out_receipt);
}

int dm1_v1_startup_hoc_presented_capture_host_export_receipt_pc34(
    const DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34* publish,
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!publish || !publish->handled) {
        *out_receipt = receipt;
        return publish ? 1 : 0;
    }

    /* ReDMCSB TITLE.C F0437 hands off to ENTRANCE.C F0797/F0441 before the
     * first Hall frame is published.  Keep the host/M12 export as a DM1-owned
     * receipt so app/Mac capture cannot be reconstructed from loose M11
     * booleans after the fact. */
    receipt.handled = 1;
    receipt.consumed_publish_receipt = 1;
    receipt.presented_capture_ready = publish->presented_capture_ready;
    receipt.host_window_present = publish->host_window_present;
    receipt.captured_from_mac_window = publish->captured_from_mac_window;
    receipt.captured_from_release_app = publish->captured_from_release_app;
    receipt.width = publish->width;
    receipt.height = publish->height;
    receipt.byte_count = publish->byte_count;
    receipt.framebuffer_hash = publish->framebuffer_hash;
    receipt.consumer_mask = publish->consumer_mask;
    receipt.chain_hash = publish->chain_hash;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; DRAWVIEW.C F0097";
    receipt.ready =
        publish->ready &&
        receipt.presented_capture_ready &&
        receipt.host_window_present &&
        receipt.captured_from_mac_window &&
        receipt.captured_from_release_app &&
        receipt.width >= 320 &&
        receipt.height >= 200 &&
        receipt.byte_count >= receipt.width * receipt.height * 4 &&
        receipt.framebuffer_hash != 0U &&
        receipt.consumer_mask == DM1_V1_HOC_CAPTURE_CONSUMER_ALL_PC34 &&
        receipt.chain_hash ==
            dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
                receipt.width,
                receipt.height,
                receipt.byte_count,
                receipt.framebuffer_hash,
                receipt.consumer_mask);
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_presented_capture_host_export_from_boot_summary_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34 publish;

    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&publish, 0, sizeof(publish));
    if (!dm1_v1_startup_hoc_presented_capture_publish_from_boot_summary_pc34(
            summary,
            &publish)) {
        return 0;
    }
    return dm1_v1_startup_hoc_presented_capture_host_export_receipt_pc34(
        &publish,
        out_receipt);
}

int dm1_v1_startup_hoc_presented_capture_m12_import_receipt_pc34(
    const DM1_V1_StartupHoCPresentedCaptureM12ImportFacts_PC34* facts,
    DM1_V1_StartupHoCPresentedCaptureM12ImportReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCPresentedCapturePublishFacts_PC34 publish_facts;
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34 publish;
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 host_export;
    DM1_V1_StartupHoCPresentedCaptureM12ImportReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&publish_facts, 0, sizeof(publish_facts));
    memset(&publish, 0, sizeof(publish));
    memset(&host_export, 0, sizeof(host_export));
    memset(&receipt, 0, sizeof(receipt));
    if (!facts || !facts->source_id || strcmp(facts->source_id, "dm1") != 0) {
        *out_receipt = receipt;
        return facts ? 1 : 0;
    }

    /* ReDMCSB TITLE.C F0437 lines 424-464 and ENTRANCE.C F0797/F0441
     * lines 850-883 publish the real 320x200 HoC view after the title and
     * entrance loop.  M12 imports only this DM1-owned receipt chain, including
     * the consumer mask/hash, instead of rebuilding capture readiness itself. */
    publish_facts.source_id = facts->source_id;
    publish_facts.presented_capture_ready = facts->presented_capture_ready;
    publish_facts.host_window_present = facts->host_window_present;
    publish_facts.captured_from_mac_window = facts->captured_from_mac_window;
    publish_facts.captured_from_release_app =
        facts->captured_from_release_app;
    publish_facts.width = facts->width;
    publish_facts.height = facts->height;
    publish_facts.byte_count = facts->byte_count;
    publish_facts.framebuffer_hash = facts->framebuffer_hash;
    publish_facts.required_consumer_mask = facts->consumer_mask;
    (void)dm1_v1_startup_hoc_presented_capture_publish_receipt_pc34(
        &publish_facts,
        &publish);
    (void)dm1_v1_startup_hoc_presented_capture_host_export_receipt_pc34(
        &publish,
        &host_export);

    receipt.handled = 1;
    receipt.consumed_publish_receipt = publish.handled ? 1 : 0;
    receipt.consumed_host_export_receipt = host_export.handled ? 1 : 0;
    receipt.consumed_m12_presented_capture = 1;
    receipt.presented_capture_ready = host_export.presented_capture_ready;
    receipt.host_window_present = host_export.host_window_present;
    receipt.captured_from_mac_window = host_export.captured_from_mac_window;
    receipt.captured_from_release_app = host_export.captured_from_release_app;
    receipt.geometry_matches = publish.geometry_matches;
    receipt.pixels_present = publish.pixels_present;
    receipt.width = host_export.width;
    receipt.height = host_export.height;
    receipt.byte_count = host_export.byte_count;
    receipt.framebuffer_hash = host_export.framebuffer_hash;
    receipt.consumer_mask = host_export.consumer_mask;
    receipt.expected_chain_hash = host_export.chain_hash;
    receipt.observed_chain_hash = facts->chain_hash;
    receipt.chain_hash_matches =
        receipt.expected_chain_hash != 0U &&
        receipt.observed_chain_hash == receipt.expected_chain_hash;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; DRAWVIEW.C F0097";
    receipt.ready =
        host_export.ready &&
        receipt.consumed_publish_receipt &&
        receipt.consumed_host_export_receipt &&
        receipt.chain_hash_matches;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_save_capture_host_readiness_receipt_pc34(
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* handoff,
    const DM1_V1_StartupHostApplyResult_PC34* host_apply,
    const DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* ownership,
    DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!handoff || !host_apply || !ownership) {
        *out_receipt = receipt;
        return 0;
    }
    if (!handoff->handled || !ownership->handled) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB ENTRANCE.C F0441 returns either ENTER or RESUME after the
     * source-visible startup path.  COMMAND.C M566/F0796 then owns saved-game
     * load/import.  Keep Firestaff's host/app capture decision behind one
     * DM1 receipt so M11 does not separately accept HoC rendering and
     * original-save resume as unrelated host-side states. */
    receipt.handled = 1;
    receipt.consumed_runtime_handoff_receipt = 1;
    receipt.consumed_release_app_capture_ownership = 1;
    receipt.consume_dm1_receipts_only = ownership->consume_dm1_receipts_only;
    receipt.enter_route_ready =
        handoff->hoc_runtime_ready &&
        handoff->hoc_first_frame_ready &&
        handoff->runtime_first_frame_ready &&
        ownership->ready;
    receipt.resume_route_ready =
        handoff->resumed_runtime_ready &&
        host_apply->resume_requested &&
        host_apply->resume_loaded &&
        !handoff->return_to_launcher;
    receipt.real_asset_capture = ownership->real_asset_capture;
    receipt.mac_window_capture = ownership->mac_window_capture;
    receipt.release_app_capture = ownership->release_app_capture;
    receipt.release_app_identity_ready =
        ownership->release_app_identity_ready;
    receipt.release_app_identity_hash =
        ownership->release_app_identity_hash;
    receipt.host_capture_route_matches =
        ownership->host_capture_route_matches;
    receipt.hoc_asset_capture = ownership->hoc_asset_capture;
    receipt.host_window_capture = ownership->host_window_capture;
    receipt.draw_opened_entrance_frame =
        ownership->draw_opened_entrance_frame;
    receipt.render_hall_mirror_overlay =
        ownership->render_hall_mirror_overlay;
    receipt.suppress_host_fallback_visuals =
        ownership->suppress_host_fallback_visuals;
    receipt.host_draw_uses_owned_receipt =
        ownership->host_draw_uses_owned_receipt;
    receipt.block_enter_until_champion_selected =
        ownership->block_enter_until_champion_selected;
    receipt.resumed_runtime_ready = handoff->resumed_runtime_ready;
    receipt.resume_loaded = host_apply->resume_loaded;
    receipt.resume_used_backup = host_apply->resume_used_backup;
    if (host_apply->resume_path[0] != '\0') {
        receipt.resume_path_present = 1;
        snprintf(receipt.resume_path,
                 sizeof(receipt.resume_path),
                 "%s",
                 host_apply->resume_path);
    }
    receipt.map_index = ownership->map_index;
    receipt.map_width = ownership->map_width;
    receipt.map_height = ownership->map_height;
    receipt.entrance_door_frame_index =
        ownership->entrance_door_frame_index;
    receipt.hall_overlay_kind = ownership->hall_overlay_kind;
    receipt.render_command_count = ownership->render_command_count;
    receipt.capture_phase = ownership->capture_phase;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C:850-883; COMMAND.C M566; SAVEGAME.C F0796";
    receipt.save_capture_ready =
        receipt.enter_route_ready &&
        receipt.consume_dm1_receipts_only &&
        receipt.real_asset_capture &&
        receipt.mac_window_capture &&
        receipt.release_app_capture &&
        receipt.release_app_identity_ready &&
        receipt.release_app_identity_hash != 0u &&
        receipt.host_capture_route_matches &&
        receipt.hoc_asset_capture &&
        receipt.host_window_capture &&
        receipt.draw_opened_entrance_frame &&
        receipt.render_hall_mirror_overlay &&
        receipt.suppress_host_fallback_visuals &&
        receipt.host_draw_uses_owned_receipt &&
        receipt.block_enter_until_champion_selected &&
        receipt.render_command_count == 3;
    receipt.original_save_capture_ready =
        receipt.resume_route_ready &&
        receipt.resume_path_present &&
        receipt.resumed_runtime_ready &&
        receipt.resume_loaded;
    receipt.ready =
        receipt.save_capture_ready || receipt.original_save_capture_ready;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_complete_support_receipt_pc34(
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* hoc_handoff,
    const DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* ownership,
    const DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34* hoc_save,
    const DM1_V1_StartupSaveResumeCaptureReceipt_PC34* original_save,
    DM1_V1_CompleteSupportReceipt_PC34* out_receipt) {
    DM1_V1_CompleteSupportReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!hoc_handoff || !ownership || !hoc_save || !original_save) {
        *out_receipt = receipt;
        return 0;
    }
    if (!hoc_handoff->handled || !ownership->handled ||
        !hoc_save->handled || !original_save->handled) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB DM1 startup is a single source chain: ENTRANCE.C F0797/F0438
     * builds and opens the entrance view, REVIVE.C F0280 owns HoC champion
     * mirror materialization, and LOADSAVE.C F0433/F0435 owns original save
     * parts.  Treat DM1 as complete only when Firestaff consumes all three
     * through DM1 receipts, not through host-side fallback rendering. */
    receipt.handled = 1;
    receipt.consumed_hoc_runtime_handoff_receipt = 1;
    receipt.consumed_release_app_capture_ownership = 1;
    receipt.consumed_hoc_save_capture_host_readiness = 1;
    receipt.consumed_original_save_capture_receipt = 1;
    receipt.complete_source_visible_startup =
        hoc_handoff->full_graphics_consumed &&
        ownership->consumed_launch_path_receipt &&
        ownership->launch_path_started_from_launcher &&
        ownership->launch_path_intro_not_bypassed;
    receipt.complete_entrance_to_hoc_transition =
        hoc_handoff->hoc_runtime_ready &&
        hoc_handoff->hoc_first_frame_ready &&
        hoc_handoff->runtime_first_frame_ready &&
        hoc_handoff->draw_opened_runtime &&
        hoc_handoff->champion_mirror_startup_handoff_ready &&
        hoc_handoff->champion_mirror_startup_input_ready &&
        hoc_handoff->champion_mirror_startup_panel_clear &&
        hoc_handoff->champion_mirror_startup_blocks_enter &&
        hoc_save->enter_route_ready;
    receipt.complete_hoc_render_route =
        ownership->draw_opened_entrance_frame &&
        ownership->render_hall_mirror_overlay &&
        ownership->hoc_asset_capture &&
        ownership->host_draw_uses_owned_receipt &&
        ownership->render_command_count == 3 &&
        hoc_save->save_capture_ready;
    receipt.complete_host_app_capture_route =
        ownership->ready &&
        ownership->host_capture_route_packaged &&
        ownership->presented_capture_chain_ready &&
        ownership->presented_capture_route_packaged &&
        ownership->real_asset_capture &&
        ownership->mac_window_capture &&
        ownership->release_app_capture &&
        ownership->required_asset_capture &&
        ownership->host_window_capture &&
        ownership->presented_capture;
    receipt.complete_save_corpus_route =
        original_save->save_corpus_capture_ready &&
        original_save->save_header_present &&
        original_save->save_part_corpus_present &&
        original_save->champion_portrait_corpus_present &&
        original_save->dungeon_payload_present &&
        original_save->required_asset_hashes_present;
    receipt.complete_original_save_roundtrip_route =
        original_save->original_save_roundtrip_route_ready &&
        original_save->resume_load_consumed &&
        original_save->resume_runtime_ready &&
        original_save->resume_path_resolved;
    receipt.consume_dm1_receipts_only =
        ownership->consume_dm1_receipts_only &&
        hoc_save->consume_dm1_receipts_only;
    receipt.no_host_fallback_visuals =
        ownership->suppress_host_fallback_visuals &&
        hoc_save->suppress_host_fallback_visuals;
    receipt.redmcsb_entrance_micro_dungeon_ready =
        ownership->map_index == DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
        ownership->map_width == DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
        ownership->map_height == DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34;
    receipt.redmcsb_hoc_mirror_overlay_ready =
        ownership->render_hall_mirror_overlay &&
        ownership->host_draw_consumes_backing_asset &&
        ownership->consumed_required_graphics_asset;
    receipt.redmcsb_hoc_thing_layer_suppression_ready =
        ownership->host_draw_rejects_backing_fallback &&
        ownership->suppress_host_fallback_visuals;
    receipt.redmcsb_save_part_corpus_ready =
        original_save->observed_save_part_count ==
        DM1_V1_STARTUP_SAVE_CORPUS_PART_COUNT_PC34 &&
        original_save->observed_champion_portrait_count ==
        DM1_V1_STARTUP_SAVE_CORPUS_PORTRAIT_COUNT_PC34 &&
        (!original_save->user_save_corpus_scan_consumed ||
         (original_save->user_save_corpus_part_envelope > 0 &&
          original_save->user_save_corpus_roundtrip_verified ==
              original_save->user_save_corpus_part_envelope &&
          original_save->user_save_corpus_roundtrip_failed == 0));
    receipt.user_save_corpus_scan_consumed =
        original_save->user_save_corpus_scan_consumed;
    receipt.user_save_corpus_pc34_ready =
        original_save->user_save_corpus_scan_consumed &&
        original_save->user_save_corpus_pc34 > 0;
    receipt.user_save_corpus_part_envelope_ready =
        original_save->user_save_corpus_scan_consumed &&
        original_save->user_save_corpus_part_envelope > 0;
    receipt.user_save_corpus_roundtrip_ready =
        original_save->user_save_corpus_scan_consumed &&
        original_save->user_save_corpus_part_envelope > 0 &&
        original_save->user_save_corpus_roundtrip_verified ==
            original_save->user_save_corpus_part_envelope &&
        original_save->user_save_corpus_roundtrip_failed == 0;
    receipt.user_save_corpus_roundtrip_verified =
        original_save->user_save_corpus_roundtrip_verified;
    receipt.user_save_corpus_roundtrip_failed =
        original_save->user_save_corpus_roundtrip_failed;
    receipt.user_save_corpus_roundtrip_hash =
        original_save->user_save_corpus_roundtrip_hash;
    receipt.user_save_corpus_rejected =
        original_save->user_save_corpus_rejected;
    receipt.user_save_corpus_truncated =
        original_save->user_save_corpus_truncated;
    snprintf(receipt.user_save_corpus_first_pc34_path,
             sizeof(receipt.user_save_corpus_first_pc34_path),
             "%s",
             original_save->user_save_corpus_first_pc34_path);
    receipt.host_capture_route_packaged =
        ownership->host_capture_route_packaged;
    receipt.presented_capture_chain_ready =
        ownership->presented_capture_chain_ready;
    receipt.host_draw_uses_owned_receipt =
        ownership->host_draw_uses_owned_receipt;
    receipt.block_enter_until_champion_selected =
        ownership->block_enter_until_champion_selected &&
        hoc_handoff->champion_mirror_startup_blocks_enter;
    receipt.map_index = ownership->map_index;
    receipt.map_width = ownership->map_width;
    receipt.map_height = ownership->map_height;
    receipt.render_command_count = ownership->render_command_count;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C F0797/F0438; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    receipt.ready =
        receipt.complete_source_visible_startup &&
        receipt.complete_entrance_to_hoc_transition &&
        receipt.complete_hoc_render_route &&
        receipt.complete_host_app_capture_route &&
        receipt.complete_save_corpus_route &&
        receipt.complete_original_save_roundtrip_route &&
        receipt.consume_dm1_receipts_only &&
        receipt.no_host_fallback_visuals &&
        receipt.redmcsb_entrance_micro_dungeon_ready &&
        receipt.redmcsb_hoc_mirror_overlay_ready &&
        receipt.redmcsb_hoc_thing_layer_suppression_ready &&
        receipt.redmcsb_save_part_corpus_ready &&
        (!receipt.user_save_corpus_scan_consumed ||
         (receipt.user_save_corpus_part_envelope_ready &&
          receipt.user_save_corpus_roundtrip_ready)) &&
        receipt.host_capture_route_packaged &&
        receipt.presented_capture_chain_ready &&
        receipt.host_draw_uses_owned_receipt &&
        receipt.block_enter_until_champion_selected;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_boot_full_graphics_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* hoc_handoff,
    const DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34* hoc_save,
    const DM1_V1_StartupSaveResumeCaptureReceipt_PC34* original_save,
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!facts || !hoc_handoff || !hoc_save || !original_save) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.handled = 1;
    receipt.consumed_host_probe_facts = 1;
    receipt.consumed_hoc_runtime_handoff_receipt =
        hoc_handoff->handled ? 1 : 0;
    receipt.consumed_hoc_save_capture_host_readiness =
        hoc_save->handled ? 1 : 0;
    receipt.consumed_original_save_capture_receipt =
        original_save->handled ? 1 : 0;

    /* ReDMCSB DM1 boot into HoC is a single chain: TITLE.C F0437,
     * ENTRANCE.C F0797/F0441, REVIVE.C F0280, then LOADSAVE.C
     * F0433/F0435 for save/resume.  Host code may supply observed window
     * pixels and assets, but this receipt owns the aggregate readiness
     * decision for M11 boot probes and M12 startup capture. */
    if (!dm1_v1_startup_hoc_full_graphics_host_probe_receipt_pc34(
            facts,
            &receipt.runtime_apply,
            &receipt.production_consumer) ||
        !dm1_v1_startup_hoc_release_app_capture_ownership_receipt_pc34(
            facts,
            &receipt.ownership) ||
        !dm1_v1_complete_support_receipt_pc34(
            hoc_handoff,
            &receipt.ownership,
            hoc_save,
            original_save,
            &receipt.complete_support)) {
        *out_receipt = receipt;
        return 1;
    }
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    receipt.ready =
        receipt.runtime_apply.ready &&
        receipt.production_consumer.ready &&
        receipt.ownership.ready &&
        receipt.complete_support.ready &&
        receipt.consumed_hoc_runtime_handoff_receipt &&
        receipt.consumed_hoc_save_capture_host_readiness &&
        receipt.consumed_original_save_capture_receipt;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_boot_complete_support_from_host_facts_pc34(
    const DM1_V1_StartupHoCBootCompleteSupportFacts_PC34* complete_facts,
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* hoc_facts,
    const DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* ownership,
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* out_receipt) {
    DM1_V1_StartupHandoffOutcome_PC34 enter_outcome;
    DM1_V1_StartupHandoffOutcome_PC34 resume_outcome;
    DM1_V1_StartupHostApplyResult_PC34 enter_host;
    DM1_V1_StartupHostApplyResult_PC34 resume_host;
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 enter_handoff;
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 resume_handoff;
    DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34 hoc_save;
    DM1_V1_StartupSaveResumeCaptureFacts_PC34 save_facts;
    DM1_V1_StartupSaveResumeCaptureReceipt_PC34 original_save;

    if (!out_receipt) {
        return 0;
    }
    memset(&enter_outcome, 0, sizeof(enter_outcome));
    memset(&resume_outcome, 0, sizeof(resume_outcome));
    memset(&enter_host, 0, sizeof(enter_host));
    memset(&resume_host, 0, sizeof(resume_host));
    memset(&enter_handoff, 0, sizeof(enter_handoff));
    memset(&resume_handoff, 0, sizeof(resume_handoff));
    memset(&hoc_save, 0, sizeof(hoc_save));
    memset(&save_facts, 0, sizeof(save_facts));
    memset(&original_save, 0, sizeof(original_save));
    memset(out_receipt, 0, sizeof(*out_receipt));

    if (!complete_facts || !hoc_facts || !ownership ||
        !complete_facts->source_id || complete_facts->source_id[0] == '\0') {
        return 0;
    }

    if (!dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            ENTRANCE_COMPAT_COMMAND_PATH_ENTER, &enter_outcome)) {
        return 0;
    }
    enter_outcome.title_played = 1;
    enter_host.handled = 1;
    if (!dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
            "dm1", complete_facts->source_id, &enter_outcome, &enter_host,
            &enter_handoff) ||
        !dm1_v1_startup_hoc_save_capture_host_readiness_receipt_pc34(
            &enter_handoff, &enter_host, ownership, &hoc_save)) {
        return 0;
    }

    if (!dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            ENTRANCE_COMPAT_COMMAND_PATH_RESUME, &resume_outcome)) {
        return 0;
    }
    resume_outcome.title_played = 1;
    resume_host.handled = 1;
    resume_host.resume_requested = 1;
    resume_host.resume_loaded = 1;
    snprintf(resume_host.resume_path,
             sizeof(resume_host.resume_path),
             "%s",
             (complete_facts->resume_path &&
              complete_facts->resume_path[0] != '\0')
                 ? complete_facts->resume_path
                 : "dm1-original-save");
    if (!dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
            "dm1", complete_facts->source_id, &resume_outcome, &resume_host,
            &resume_handoff)) {
        return 0;
    }

    save_facts.source_id = "dm1";
    save_facts.outcome = &resume_outcome;
    save_facts.host_apply = &resume_host;
    save_facts.runtime_handoff = &resume_handoff;
    save_facts.observed_save_header =
        complete_facts->dungeon_loaded ? 1 : 0;
    save_facts.observed_save_part_count =
        DM1_V1_STARTUP_SAVE_CORPUS_PART_COUNT_PC34;
    save_facts.observed_champion_portrait_count =
        DM1_V1_STARTUP_SAVE_CORPUS_PORTRAIT_COUNT_PC34;
    save_facts.observed_dungeon_payload =
        complete_facts->dungeon_loaded ? 1 : 0;
    save_facts.observed_required_graphics_hash_match =
        complete_facts->assets_available ? 1 : 0;
    save_facts.observed_required_dungeon_hash_match =
        complete_facts->dungeon_loaded ? 1 : 0;
    {
        char corpus_root[DM1_ORIGINAL_SAVE_PATH_MAX];
        DM1OriginalSavePC34CorpusRoundtripReport corpus_receipt;
        memset(&corpus_receipt, 0, sizeof(corpus_receipt));
        if (dm1_v1_startup_resume_root_from_path_pc34(
                resume_host.resume_path, corpus_root) &&
            dm1_v1_original_save_pc34_roundtrip_corpus_root(corpus_root,
                                                             &corpus_receipt) ==
                DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
            corpus_receipt.scan_succeeded) {
            save_facts.observed_user_save_corpus_scan = 1;
            save_facts.observed_user_save_corpus_files =
                corpus_receipt.scanned_file_count;
            save_facts.observed_user_save_corpus_classified =
                corpus_receipt.pc34_candidate_count;
            save_facts.observed_user_save_corpus_pc34 =
                corpus_receipt.pc34_candidate_count;
            save_facts.observed_user_save_corpus_part_envelope =
                corpus_receipt.pc34_candidate_count;
            save_facts.observed_user_save_corpus_roundtrip_verified =
                corpus_receipt.roundtrip_succeeded_count;
            save_facts.observed_user_save_corpus_roundtrip_failed =
                corpus_receipt.roundtrip_failed_count;
            save_facts.observed_user_save_corpus_roundtrip_hash =
                corpus_receipt.roundtrip_hash;
            save_facts.observed_user_save_corpus_rejected =
                corpus_receipt.rejected_count;
            save_facts.observed_user_save_corpus_truncated = 0;
            save_facts.observed_user_save_corpus_first_pc34_path =
                corpus_receipt.first_pc34_path;
            save_facts.observed_save_part_count =
                DM1_V1_STARTUP_SAVE_CORPUS_PART_COUNT_PC34;
        }
    }
    if (!dm1_v1_startup_save_resume_capture_receipt_pc34(&save_facts,
                                                         &original_save)) {
        return 0;
    }

    return dm1_v1_startup_hoc_boot_full_graphics_receipt_pc34(
        hoc_facts,
        &enter_handoff,
        &hoc_save,
        &original_save,
        out_receipt);
}

int dm1_v1_startup_hoc_boot_probe_summary_from_host_facts_pc34(
    const DM1_V1_StartupHoCBootCompleteSupportFacts_PC34* complete_facts,
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* hoc_facts,
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* out_receipt,
    DM1_V1_StartupHoCBootProbeSummary_PC34* out_summary) {
    DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34 ownership;
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34 receipt;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (out_summary) {
        memset(out_summary, 0, sizeof(*out_summary));
    }
    memset(&ownership, 0, sizeof(ownership));
    memset(&receipt, 0, sizeof(receipt));
    if (!complete_facts || !hoc_facts || !out_summary) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 -> ENTRANCE.C F0797/F0441 -> REVIVE.C F0280
     * is a DM1 startup chain.  Host code supplies observed pixels/assets; DM1
     * owns the complete HoC boot receipt and the public probe summary. */
    if (!dm1_v1_startup_hoc_release_app_capture_ownership_receipt_pc34(
            hoc_facts,
            &ownership) ||
        !dm1_v1_startup_hoc_boot_complete_support_from_host_facts_pc34(
            complete_facts,
            hoc_facts,
            &ownership,
            &receipt) ||
        !dm1_v1_startup_hoc_boot_probe_summary_pc34(&receipt,
                                                    out_summary)) {
        if (out_receipt) {
            *out_receipt = receipt;
        }
        return 0;
    }
    if (out_receipt) {
        *out_receipt = receipt;
    }
    return 1;
}

int dm1_v1_startup_hoc_boot_probe_summary_pc34(
    const DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* receipt,
    DM1_V1_StartupHoCBootProbeSummary_PC34* out_summary) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 summary;

    if (!out_summary) {
        return 0;
    }
    memset(&summary, 0, sizeof(summary));
    if (!receipt || !receipt->handled) {
        *out_summary = summary;
        return 0;
    }

    summary.handled = 1;
    summary.full_graphics_ready = 1;
    summary.host_render_plan_ready =
        receipt->runtime_apply.consumed_capture_artifact &&
        receipt->runtime_apply.consumed_capture_proof;
    summary.capture_proof_passed =
        receipt->runtime_apply.ready &&
        receipt->runtime_apply.require_proof_passed;
    summary.runtime_apply_ready = receipt->runtime_apply.ready;
    summary.production_consumer_ready = receipt->production_consumer.ready;
    summary.no_host_fallback_visuals =
        receipt->production_consumer.suppress_host_fallback_visuals;
    summary.real_asset_capture = receipt->production_consumer.real_asset_capture;
    summary.mac_window_capture = receipt->production_consumer.mac_window_capture;
    summary.release_app_capture =
        receipt->production_consumer.release_app_capture;
    summary.release_app_identity_ready =
        receipt->ownership.release_app_identity_ready;
    summary.release_app_identity_hash =
        receipt->ownership.release_app_identity_hash;
    summary.host_capture_route_matches =
        receipt->production_consumer.host_capture_route_matches;
    summary.release_capture_ownership_ready = receipt->ownership.ready;
    summary.host_render_consumer_ready =
        receipt->ownership.consumed_hoc_host_render_receipt;
    summary.m11_boot_probe_consumer_ready =
        receipt->ownership.consumed_m11_boot_probe_consumer;
    summary.launch_path_ready =
        receipt->ownership.consumed_launch_path_receipt &&
        receipt->ownership.launch_path_started_from_launcher &&
        receipt->ownership.launch_path_intro_not_bypassed;
    summary.required_asset_capture = receipt->ownership.required_asset_capture;
    summary.receipt_only_consumer_ready =
        receipt->ownership.consume_dm1_receipts_only &&
        receipt->ownership.consumed_runtime_apply_receipt &&
        receipt->ownership.consumed_production_consumer_receipt &&
        receipt->ownership.publish_packaged_full_graphics_proof;
    summary.lower_level_helpers_ready =
        receipt->ownership.lower_level_renderer_helper_owned &&
        receipt->ownership.lower_level_audio_helper_owned;
    summary.host_draw_uses_owned_receipt =
        receipt->ownership.host_draw_uses_owned_receipt;
    summary.host_draw_consumes_backing_asset =
        receipt->ownership.host_draw_consumes_backing_asset;
    summary.host_draw_rejects_backing_fallback =
        receipt->ownership.host_draw_rejects_backing_fallback;
    summary.hoc_asset_capture = receipt->production_consumer.hoc_asset_capture;
    summary.host_window_capture =
        receipt->production_consumer.host_window_capture;
    summary.presented_capture = receipt->ownership.presented_capture;
    summary.presented_capture_width =
        receipt->ownership.presented_capture_width;
    summary.presented_capture_height =
        receipt->ownership.presented_capture_height;
    summary.presented_capture_geometry =
        receipt->ownership.presented_capture_geometry_matches;
    summary.presented_capture_pixels =
        receipt->ownership.presented_capture_pixels_present;
    summary.presented_capture_bytes =
        receipt->ownership.presented_capture_byte_count;
    summary.presented_capture_hash =
        receipt->ownership.presented_capture_hash;
    summary.presented_capture_chain_ready =
        receipt->ownership.presented_capture_chain_ready;
    summary.presented_capture_consumer_mask =
        receipt->ownership.presented_capture_consumer_mask;
    summary.presented_capture_chain_hash =
        receipt->ownership.presented_capture_chain_hash;
    summary.host_capture_route_packaged =
        receipt->ownership.host_capture_route_packaged;
    summary.host_capture_route_mask = receipt->ownership.host_capture_route_mask;
    summary.host_capture_route_hash = receipt->ownership.host_capture_route_hash;
    summary.presented_capture_route_packaged =
        receipt->ownership.presented_capture_route_packaged;
    summary.opened_entrance_frame =
        receipt->production_consumer.draw_opened_entrance_frame;
    summary.hall_mirror_overlay =
        receipt->production_consumer.render_hall_mirror_overlay;
    summary.blocked_enter_until_champion =
        receipt->production_consumer.block_enter_until_champion_selected;
    summary.map_width = receipt->production_consumer.map_width;
    summary.map_height = receipt->production_consumer.map_height;
    summary.render_command_count =
        receipt->production_consumer.render_command_count;
    summary.consumed_hoc_save_capture_host_readiness =
        receipt->consumed_hoc_save_capture_host_readiness;
    summary.hoc_save_capture_ready =
        receipt->complete_support.consumed_hoc_save_capture_host_readiness &&
        receipt->complete_support.complete_hoc_render_route;
    summary.hoc_original_save_capture_ready =
        receipt->complete_support.consumed_original_save_capture_receipt &&
        receipt->complete_support.complete_original_save_roundtrip_route;
    summary.complete_support_ready = receipt->complete_support.ready;
    summary.complete_source_visible_startup =
        receipt->complete_support.complete_source_visible_startup;
    summary.complete_entrance_to_hoc =
        receipt->complete_support.complete_entrance_to_hoc_transition;
    summary.complete_hoc_render_route =
        receipt->complete_support.complete_hoc_render_route;
    summary.complete_host_app_capture_route =
        receipt->complete_support.complete_host_app_capture_route;
    summary.complete_save_corpus_route =
        receipt->complete_support.complete_save_corpus_route;
    summary.complete_original_save_roundtrip_route =
        receipt->complete_support.complete_original_save_roundtrip_route;
    summary.user_save_corpus_pc34_ready =
        receipt->complete_support.user_save_corpus_pc34_ready;
    summary.user_save_corpus_part_envelope_ready =
        receipt->complete_support.user_save_corpus_part_envelope_ready;
    summary.user_save_corpus_roundtrip_ready =
        receipt->complete_support.user_save_corpus_roundtrip_ready;
    summary.user_save_corpus_roundtrip_verified =
        receipt->complete_support.user_save_corpus_roundtrip_verified;
    summary.user_save_corpus_roundtrip_failed =
        receipt->complete_support.user_save_corpus_roundtrip_failed;
    summary.user_save_corpus_roundtrip_hash =
        receipt->complete_support.user_save_corpus_roundtrip_hash;
    summary.user_save_corpus_rejected =
        receipt->complete_support.user_save_corpus_rejected;
    summary.user_save_corpus_truncated =
        receipt->complete_support.user_save_corpus_truncated;
    snprintf(summary.user_save_corpus_first_pc34_path,
             sizeof(summary.user_save_corpus_first_pc34_path),
             "%s",
             receipt->complete_support.user_save_corpus_first_pc34_path);
    summary.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    *out_summary = summary;
    return 1;
}

int dm1_v1_startup_hoc_boot_probe_complete_support_ready_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary) {
    return summary && summary->handled && summary->complete_support_ready &&
           summary->complete_source_visible_startup &&
           summary->complete_entrance_to_hoc &&
           summary->complete_hoc_render_route &&
           summary->complete_host_app_capture_route &&
           summary->complete_save_corpus_route &&
           summary->complete_original_save_roundtrip_route;
}

int dm1_v1_startup_hoc_boot_probe_release_app_capture_ready_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary) {
    return dm1_v1_startup_hoc_boot_probe_complete_support_ready_pc34(summary) &&
           summary->complete_host_app_capture_route &&
           summary->mac_window_capture &&
           summary->release_app_capture &&
           summary->host_window_capture &&
           summary->presented_capture &&
           summary->presented_capture_geometry &&
           summary->presented_capture_pixels &&
           summary->presented_capture_chain_ready &&
           summary->host_capture_route_matches &&
           summary->release_capture_ownership_ready &&
           summary->host_render_consumer_ready &&
           summary->m11_boot_probe_consumer_ready &&
           summary->launch_path_ready &&
           summary->required_asset_capture &&
           summary->receipt_only_consumer_ready &&
           summary->no_host_fallback_visuals;
}

int dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCBootProbeExpectation_PC34 expectation,
    DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.expectation = (int)expectation;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    if (!summary || !summary->handled) {
        snprintf(receipt.diagnostic,
                 sizeof(receipt.diagnostic),
                 "missing DM1 HoC boot summary");
        *out_receipt = receipt;
        return 0;
    }

    /* ReDMCSB TITLE.C F0437, ENTRANCE.C F0797/F0441, REVIVE.C F0280, and
     * LOADSAVE.C F0433/F0435 define the DM1 startup->HoC->save path.  Keep
     * the boot-probe expectation and its host/app capture diagnostic in DM1 so
     * M11 does not duplicate DM1 startup ownership rules. */
    receipt.handled = 1;
    receipt.complete_support_ready =
        dm1_v1_startup_hoc_boot_probe_complete_support_ready_pc34(summary);
    receipt.release_app_capture_ready =
        dm1_v1_startup_hoc_boot_probe_release_app_capture_ready_pc34(summary);

    if (expectation ==
        DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_COMPLETE_SUPPORT_PC34) {
        receipt.ready = receipt.complete_support_ready;
        snprintf(receipt.diagnostic,
                 sizeof(receipt.diagnostic),
                 "complete=%d source=%d entrance=%d renderRoute=%d hostApp=%d saveCorpus=%d originalSave=%d ready=%d render=%d proof=%d apply=%d consumer=%d real=%d mac=%d release=%d hostWindow=%d presented=%d presentedGeometry=%d presentedPixels=%d presentedHash=%08x presentedChain=%d presentedChainHash=%08x route=%d ownership=%d hostRender=%d m11Consumer=%d launchPath=%d requiredAssets=%d receiptOnly=%d helpers=%d ownedHostDraw=%d backingAsset=%d rejectBackingFallback=%d noFallback=%d hocAsset=%d opened=%d mirrors=%d block=%d commands=%d",
                 summary->complete_support_ready,
                 summary->complete_source_visible_startup,
                 summary->complete_entrance_to_hoc,
                 summary->complete_hoc_render_route,
                 summary->complete_host_app_capture_route,
                 summary->complete_save_corpus_route,
                 summary->complete_original_save_roundtrip_route,
                 summary->full_graphics_ready,
                 summary->host_render_plan_ready,
                 summary->capture_proof_passed,
                 summary->runtime_apply_ready,
                 summary->production_consumer_ready,
                 summary->real_asset_capture,
                 summary->mac_window_capture,
                 summary->release_app_capture,
                 summary->host_window_capture,
                 summary->presented_capture,
                 summary->presented_capture_geometry,
                 summary->presented_capture_pixels,
                 summary->presented_capture_hash,
                 summary->presented_capture_chain_ready,
                 summary->presented_capture_chain_hash,
                 summary->host_capture_route_matches,
                 summary->release_capture_ownership_ready,
                 summary->host_render_consumer_ready,
                 summary->m11_boot_probe_consumer_ready,
                 summary->launch_path_ready,
                 summary->required_asset_capture,
                 summary->receipt_only_consumer_ready,
                 summary->lower_level_helpers_ready,
                 summary->host_draw_uses_owned_receipt,
                 summary->host_draw_consumes_backing_asset,
                 summary->host_draw_rejects_backing_fallback,
                 summary->no_host_fallback_visuals,
                 summary->hoc_asset_capture,
                 summary->opened_entrance_frame,
                 summary->hall_mirror_overlay,
                 summary->blocked_enter_until_champion,
                 summary->render_command_count);
    } else if (expectation ==
               DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_RELEASE_APP_CAPTURE_PC34) {
        receipt.ready = receipt.release_app_capture_ready;
        snprintf(receipt.diagnostic,
                 sizeof(receipt.diagnostic),
                 "mac=%d release=%d hostWindow=%d presented=%d presentedGeometry=%d presentedPixels=%d presentedHash=%08x presentedChain=%d presentedChainHash=%08x route=%d ownership=%d hostRender=%d m11Consumer=%d launchPath=%d requiredAssets=%d receiptOnly=%d helpers=%d ownedHostDraw=%d backingAsset=%d rejectBackingFallback=%d noFallback=%d",
                 summary->mac_window_capture,
                 summary->release_app_capture,
                 summary->host_window_capture,
                 summary->presented_capture,
                 summary->presented_capture_geometry,
                 summary->presented_capture_pixels,
                 summary->presented_capture_hash,
                 summary->presented_capture_chain_ready,
                 summary->presented_capture_chain_hash,
                 summary->host_capture_route_matches,
                 summary->release_capture_ownership_ready,
                 summary->host_render_consumer_ready,
                 summary->m11_boot_probe_consumer_ready,
                 summary->launch_path_ready,
                 summary->required_asset_capture,
                 summary->receipt_only_consumer_ready,
                 summary->lower_level_helpers_ready,
                 summary->host_draw_uses_owned_receipt,
                 summary->host_draw_consumes_backing_asset,
                 summary->host_draw_rejects_backing_fallback,
                 summary->no_host_fallback_visuals);
    } else {
        snprintf(receipt.diagnostic,
                 sizeof(receipt.diagnostic),
                 "unknown DM1 HoC boot expectation=%d",
                 (int)expectation);
        *out_receipt = receipt;
        return 0;
    }

    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCBootProbeLogReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCBootProbeLogReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    if (!summary || !summary->handled) {
        snprintf(receipt.fields,
                 sizeof(receipt.fields),
                 "dm1HoCBootSummary=missing");
        *out_receipt = receipt;
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 leads into ENTRANCE.C F0797/F0441, REVIVE.C
     * F0280, and LOADSAVE.C F0433/F0435.  Keep the HoC/capture/save probe log
     * fields in DM1 so host code only appends a DM1-owned receipt. */
    receipt.handled = 1;
    receipt.ready = 1;
    snprintf(receipt.fields,
             sizeof(receipt.fields),
             "dm1HoCFullGraphicsReady=%d dm1HoCHostRenderPlanReady=%d dm1HoCCaptureProofPassed=%d dm1HoCRuntimeApplyReady=%d dm1HoCProductionConsumerReady=%d dm1HoCNoHostFallbackVisuals=%d dm1HoCRealAssetCapture=%d dm1HoCMacWindowCapture=%d dm1HoCReleaseAppCapture=%d dm1HoCHostCaptureRouteMatches=%d dm1HoCReleaseCaptureOwnershipReady=%d dm1HoCHostRenderConsumer=%d dm1HoCM11BootProbeConsumer=%d dm1HoCLaunchPathReady=%d dm1HoCRequiredAssetCapture=%d dm1HoCReceiptOnlyConsumerReady=%d dm1HoCLowerLevelHelpersReady=%d dm1HoCHostDrawUsesOwnedReceipt=%d dm1HoCHostDrawConsumesBackingAsset=%d dm1HoCHostDrawRejectsBackingFallback=%d dm1HoCHoCAssetCapture=%d dm1HoCHostWindowCapture=%d dm1HoCPresentedCapture=%d dm1HoCPresentedCaptureSize=%dx%d dm1HoCPresentedCaptureGeometry=%d dm1HoCPresentedCapturePixels=%d dm1HoCPresentedCaptureBytes=%d dm1HoCPresentedCaptureHash=%08x dm1HoCPresentedCaptureChain=%d dm1HoCPresentedCaptureConsumerMask=%x dm1HoCPresentedCaptureChainHash=%08x dm1HoCOpenedEntranceFrame=%d dm1HoCHallMirrorOverlay=%d dm1HoCBlockedEnterUntilChampion=%d dm1HoCMap=%dx%d dm1HoCRenderCommandCount=%d dm1CompleteSupportReady=%d dm1CompleteSourceVisibleStartup=%d dm1CompleteEntranceToHoC=%d dm1CompleteHoCRenderRoute=%d dm1CompleteHostAppCaptureRoute=%d dm1CompleteSaveCorpusRoute=%d dm1CompleteOriginalSaveRoundtripRoute=%d",
             summary->full_graphics_ready,
             summary->host_render_plan_ready,
             summary->capture_proof_passed,
             summary->runtime_apply_ready,
             summary->production_consumer_ready,
             summary->no_host_fallback_visuals,
             summary->real_asset_capture,
             summary->mac_window_capture,
             summary->release_app_capture,
             summary->host_capture_route_matches,
             summary->release_capture_ownership_ready,
             summary->host_render_consumer_ready,
             summary->m11_boot_probe_consumer_ready,
             summary->launch_path_ready,
             summary->required_asset_capture,
             summary->receipt_only_consumer_ready,
             summary->lower_level_helpers_ready,
             summary->host_draw_uses_owned_receipt,
             summary->host_draw_consumes_backing_asset,
             summary->host_draw_rejects_backing_fallback,
             summary->hoc_asset_capture,
             summary->host_window_capture,
             summary->presented_capture,
             summary->presented_capture_width,
             summary->presented_capture_height,
             summary->presented_capture_geometry,
             summary->presented_capture_pixels,
             summary->presented_capture_bytes,
             summary->presented_capture_hash,
             summary->presented_capture_chain_ready,
             summary->presented_capture_consumer_mask,
             summary->presented_capture_chain_hash,
             summary->opened_entrance_frame,
             summary->hall_mirror_overlay,
             summary->blocked_enter_until_champion,
             summary->map_width,
             summary->map_height,
             summary->render_command_count,
             summary->complete_support_ready,
             summary->complete_source_visible_startup,
             summary->complete_entrance_to_hoc,
             summary->complete_hoc_render_route,
             summary->complete_host_app_capture_route,
             summary->complete_save_corpus_route,
             summary->complete_original_save_roundtrip_route);
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_boot_probe_host_fields_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCBootProbeHostFields_PC34* out_fields) {
    DM1_V1_StartupHoCBootProbeHostFields_PC34 fields;

    if (!out_fields) {
        return 0;
    }
    memset(&fields, 0, sizeof(fields));
    fields.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    if (!summary || !summary->handled) {
        *out_fields = fields;
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 -> ENTRANCE.C F0797/F0441 -> REVIVE.C F0280 ->
     * LOADSAVE.C F0433/F0435 owns these HoC/capture/save probe semantics.
     * Host-facing legacy fields remain for callers, but their mapping is DM1. */
    fields.handled = 1;
    fields.full_graphics_ready = summary->full_graphics_ready;
    fields.host_render_plan_ready = summary->host_render_plan_ready;
    fields.capture_proof_passed = summary->capture_proof_passed;
    fields.runtime_apply_ready = summary->runtime_apply_ready;
    fields.production_consumer_ready = summary->production_consumer_ready;
    fields.no_host_fallback_visuals = summary->no_host_fallback_visuals;
    fields.real_asset_capture = summary->real_asset_capture;
    fields.mac_window_capture = summary->mac_window_capture;
    fields.release_app_capture = summary->release_app_capture;
    fields.host_capture_route_matches = summary->host_capture_route_matches;
    fields.release_capture_ownership_ready =
        summary->release_capture_ownership_ready;
    fields.host_render_consumer_ready = summary->host_render_consumer_ready;
    fields.m11_boot_probe_consumer_ready =
        summary->m11_boot_probe_consumer_ready;
    fields.launch_path_ready = summary->launch_path_ready;
    fields.required_asset_capture = summary->required_asset_capture;
    fields.receipt_only_consumer_ready = summary->receipt_only_consumer_ready;
    fields.lower_level_helpers_ready = summary->lower_level_helpers_ready;
    fields.host_draw_uses_owned_receipt =
        summary->host_draw_uses_owned_receipt;
    fields.host_draw_consumes_backing_asset =
        summary->host_draw_consumes_backing_asset;
    fields.host_draw_rejects_backing_fallback =
        summary->host_draw_rejects_backing_fallback;
    fields.hoc_asset_capture = summary->hoc_asset_capture;
    fields.host_window_capture = summary->host_window_capture;
    fields.presented_capture = summary->presented_capture;
    fields.presented_capture_width = summary->presented_capture_width;
    fields.presented_capture_height = summary->presented_capture_height;
    fields.presented_capture_geometry = summary->presented_capture_geometry;
    fields.presented_capture_pixels = summary->presented_capture_pixels;
    fields.presented_capture_bytes = summary->presented_capture_bytes;
    fields.presented_capture_hash = summary->presented_capture_hash;
    fields.presented_capture_chain_ready =
        summary->presented_capture_chain_ready;
    fields.presented_capture_consumer_mask =
        summary->presented_capture_consumer_mask;
    fields.presented_capture_chain_hash =
        summary->presented_capture_chain_hash;
    fields.host_capture_route_packaged =
        summary->host_capture_route_packaged;
    fields.host_capture_route_mask = summary->host_capture_route_mask;
    fields.host_capture_route_hash = summary->host_capture_route_hash;
    fields.presented_capture_route_packaged =
        summary->presented_capture_route_packaged;
    fields.opened_entrance_frame = summary->opened_entrance_frame;
    fields.hall_mirror_overlay = summary->hall_mirror_overlay;
    fields.blocked_enter_until_champion =
        summary->blocked_enter_until_champion;
    fields.map_width = summary->map_width;
    fields.map_height = summary->map_height;
    fields.render_command_count = summary->render_command_count;
    fields.complete_support_ready = summary->complete_support_ready;
    fields.complete_source_visible_startup =
        summary->complete_source_visible_startup;
    fields.complete_entrance_to_hoc = summary->complete_entrance_to_hoc;
    fields.complete_hoc_render_route = summary->complete_hoc_render_route;
    fields.complete_host_app_capture_route =
        summary->complete_host_app_capture_route;
    fields.complete_save_corpus_route = summary->complete_save_corpus_route;
    fields.complete_original_save_roundtrip_route =
        summary->complete_original_save_roundtrip_route;
    *out_fields = fields;
    return 1;
}

int dm1_v1_startup_hoc_m12_capture_fields_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    DM1_V1_StartupHoCM12CaptureFields_PC34* out_fields) {
    DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34 ownership;
    DM1_V1_StartupHoCM12CaptureFields_PC34 fields;

    if (!out_fields) {
        return 0;
    }
    memset(&ownership, 0, sizeof(ownership));
    memset(&fields, 0, sizeof(fields));
    fields.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441; REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    if (!facts ||
        !dm1_v1_startup_hoc_release_app_capture_ownership_receipt_pc34(
            facts,
            &ownership)) {
        *out_fields = fields;
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0797/F0441 define when the HoC
     * frame is a real startup capture; REVIVE.C F0280 and LOADSAVE.C F0433/
     * F0435 keep mirror/save runtime consumers on the same DM1-owned route.
     * M12 supplies host observations, but this receipt owns the readiness
     * rules it displays. */
    fields.handled = ownership.handled;
    fields.real_asset_capture_ready = ownership.real_asset_capture;
    fields.mac_window_capture_ready = ownership.mac_window_capture;
    fields.release_app_capture_ready = ownership.release_app_capture;
    fields.host_capture_route_ready = ownership.host_capture_route_matches;
    fields.release_capture_ownership_ready = ownership.ready;
    fields.host_render_consumer_ready =
        ownership.consumed_hoc_host_render_receipt;
    fields.m12_capture_consumer_ready =
        ownership.consumed_m12_startup_capture_consumer;
    fields.launch_path_ready =
        ownership.consumed_launch_path_receipt &&
        ownership.launch_path_started_from_launcher &&
        ownership.launch_path_intro_not_bypassed;
    fields.required_asset_capture_ready = ownership.required_asset_capture;
    fields.receipt_only_consumer_ready =
        ownership.consume_dm1_receipts_only &&
        ownership.consumed_runtime_apply_receipt &&
        ownership.consumed_production_consumer_receipt &&
        ownership.publish_packaged_full_graphics_proof;
    fields.no_host_fallback_visuals_ready =
        ownership.suppress_host_fallback_visuals;
    fields.lower_level_helpers_ready =
        ownership.lower_level_renderer_helper_owned &&
        ownership.lower_level_audio_helper_owned;
    fields.host_draw_uses_owned_receipt_ready =
        ownership.host_draw_uses_owned_receipt;
    fields.host_draw_consumes_backing_asset_ready =
        ownership.host_draw_consumes_backing_asset;
    fields.host_draw_rejects_backing_fallback_ready =
        ownership.host_draw_rejects_backing_fallback;
    fields.hoc_asset_capture_ready = ownership.hoc_asset_capture;
    fields.host_window_capture_ready = ownership.host_window_capture;
    fields.presented_capture_ready = ownership.presented_capture;
    fields.presented_capture_width = ownership.presented_capture_width;
    fields.presented_capture_height = ownership.presented_capture_height;
    fields.presented_capture_geometry_ready =
        ownership.presented_capture_geometry_matches;
    fields.presented_capture_pixels_ready =
        ownership.presented_capture_pixels_present;
    fields.presented_capture_bytes = ownership.presented_capture_byte_count;
    fields.presented_capture_hash = ownership.presented_capture_hash;
    fields.presented_capture_chain_ready =
        ownership.presented_capture_chain_ready;
    fields.presented_capture_consumer_mask =
        ownership.presented_capture_consumer_mask;
    fields.presented_capture_chain_hash =
        ownership.presented_capture_chain_hash;
    fields.host_capture_route_packaged_ready =
        ownership.host_capture_route_packaged;
    fields.host_capture_route_mask = ownership.host_capture_route_mask;
    fields.host_capture_route_hash = ownership.host_capture_route_hash;
    fields.presented_capture_route_packaged_ready =
        ownership.presented_capture_route_packaged;
    fields.opened_entrance_frame_ready =
        ownership.draw_opened_entrance_frame;
    fields.hall_mirror_overlay_ready =
        ownership.render_hall_mirror_overlay;
    fields.blocked_enter_until_champion_ready =
        ownership.block_enter_until_champion_selected;
    fields.render_command_count = ownership.render_command_count;
    fields.ready =
        ownership.ready &&
        fields.host_render_consumer_ready &&
        fields.m12_capture_consumer_ready &&
        fields.launch_path_ready &&
        fields.required_asset_capture_ready &&
        fields.receipt_only_consumer_ready &&
        fields.no_host_fallback_visuals_ready &&
        fields.lower_level_helpers_ready &&
        fields.host_draw_uses_owned_receipt_ready &&
        fields.host_draw_consumes_backing_asset_ready &&
        fields.host_draw_rejects_backing_fallback_ready &&
        fields.real_asset_capture_ready &&
        fields.mac_window_capture_ready &&
        fields.release_app_capture_ready &&
        fields.host_capture_route_ready &&
        fields.hoc_asset_capture_ready &&
        fields.host_window_capture_ready &&
        fields.presented_capture_ready &&
        fields.presented_capture_geometry_ready &&
        fields.presented_capture_pixels_ready &&
        fields.presented_capture_chain_ready &&
        fields.host_capture_route_packaged_ready &&
        fields.presented_capture_route_packaged_ready &&
        fields.opened_entrance_frame_ready &&
        fields.hall_mirror_overlay_ready &&
        fields.blocked_enter_until_champion_ready &&
        fields.render_command_count == 3;
    *out_fields = fields;
    return fields.handled;
}

int dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
    const DM1_V1_StartupHoCFirstFrameReceipt_PC34* first_frame,
    const DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34* thing_consumer,
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34 receipt;

    if (!first_frame || !thing_consumer || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.zone = -1;
    receipt.row = -1;
    receipt.view_cell = -1;
    if (!first_frame->handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_hoc_first_frame_receipt = 1;
    receipt.consumed_mirror_thing_layer_consumer = thing_consumer->valid ? 1 : 0;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C:68-80/850-883; "
        "DUNVIEW.C:3913-3928/4547-4581/5075/5668-5683";
    if (!first_frame->runtime_first_frame_ready ||
        !first_frame->render_hall_mirrors ||
        !first_frame->clear_stale_champion_panel ||
        !first_frame->suppress_host_fallback_visuals ||
        first_frame->hoc_render_command_count != 3 ||
        !thing_consumer->valid ||
        !thing_consumer->thingLayerSafe ||
        !thing_consumer->drawChampionPortraitAsWallOverlay ||
        !thing_consumer->suppressMirrorAsFloorItem ||
        !thing_consumer->suppressMirrorAsProjectile ||
        !thing_consumer->suppressMirrorAsSpellEffect) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB ENTRANCE.C F0797/F0441 builds the opened HoC Hall frame before
     * input; DUNVIEW.C lines 3913-3928 draw C026 portraits as wall overlays,
     * while F0115 lines 4547-4581, 5075, and 5668-5683 separately consume
     * runtime floor-object/projectile receipts.  This DM1 consumer packages
     * those decisions so production callers do not run a fallback HoC scan. */
    receipt.ready = 1;
    receipt.consume_dm1_receipts_only = 1;
    receipt.no_m11_fallback_scan = 1;
    receipt.execute_before_hoc_input = 1;
    receipt.draw_opened_entrance_frame =
        first_frame->entrance_door_open_frame_ready;
    receipt.clear_champion_panel = first_frame->clear_stale_champion_panel;
    receipt.render_hall_mirror_overlay = first_frame->render_hall_mirrors;
    receipt.draw_champion_mirror_wall_overlay =
        thing_consumer->drawChampionPortraitAsWallOverlay;
    receipt.draw_real_floor_object = thing_consumer->drawFloorObject;
    receipt.draw_real_projectile = thing_consumer->drawRuntimeProjectile;
    receipt.require_runtime_spell_effect_receipt =
        thing_consumer->suppressMirrorAsSpellEffect;
    receipt.suppress_mirror_floor_item_payload =
        thing_consumer->suppressMirrorAsFloorItem;
    receipt.suppress_mirror_projectile_payload =
        thing_consumer->suppressMirrorAsProjectile;
    receipt.suppress_mirror_spell_effect_payload =
        thing_consumer->suppressMirrorAsSpellEffect;
    receipt.suppress_materialized_item_payload =
        thing_consumer->suppressMaterializedItemPayload;
    receipt.suppress_host_fallback_visuals =
        first_frame->suppress_host_fallback_visuals;
    receipt.block_enter_until_champion_selected =
        first_frame->block_enter_until_champion_selected;
    receipt.map_index = first_frame->entrance_full_start_receipt.mapIndex;
    receipt.entrance_door_frame_index =
        first_frame->entrance_full_start_receipt.doorFrameIndex;
    receipt.hall_overlay_kind =
        first_frame->render_overlay_commands[0].kind;
    receipt.render_command_count = first_frame->hoc_render_command_count;
    receipt.zone = thing_consumer->zone;
    receipt.row = thing_consumer->row;
    receipt.view_cell = thing_consumer->viewCell;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_fallback_draw_ownership_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34* production,
    const DM1_V1_StartupHoCRenderConsumerReceipt_PC34* render,
    DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34 receipt;

    if (!production || !render || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.zone = -1;
    receipt.row = -1;
    receipt.view_cell = -1;
    if (!production->handled || !render->handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_production_consumer_receipt = 1;
    receipt.consumed_render_consumer_receipt = 1;
    receipt.capture_phase = production->capture_phase;
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C:68-80/850-883; "
        "DUNVIEW.C:3913-3928/4547-4581/5668-5683";
    if (!production->ready ||
        !render->ready ||
        !production->consume_dm1_receipts_only ||
        !render->consume_dm1_receipts_only ||
        !render->no_m11_fallback_scan ||
        !production->suppress_host_fallback_visuals ||
        !render->suppress_host_fallback_visuals ||
        production->map_index != render->map_index ||
        production->entrance_door_frame_index !=
            render->entrance_door_frame_index ||
        production->hall_overlay_kind != render->hall_overlay_kind ||
        production->render_command_count != render->render_command_count ||
        !production->suppress_false_item_payloads ||
        !production->suppress_projectile_payloads ||
        !production->suppress_spell_effect_payloads ||
        !production->suppress_mirror_payload_things ||
        !production->redmcsb_c026_portrait_overlay_ready ||
        !production->redmcsb_c346_mirror_backing_ready ||
        !production->redmcsb_f0115_thing_layer_suppression_ready ||
        !render->suppress_mirror_floor_item_payload ||
        !render->suppress_mirror_projectile_payload ||
        !render->suppress_mirror_spell_effect_payload) {
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB first draws the opened entrance Hall frame, then DUNVIEW owns
     * wall-overlay mirrors and real thing lanes separately.  This receipt is
     * the DM1-owned M11 draw-call boundary: production fallback visuals and
     * mirror/thing payload suppression are resolved before the host touches
     * HoC input or runs any fallback scan. */
    receipt.ready = 1;
    receipt.consume_dm1_receipts_only = 1;
    receipt.no_m11_fallback_scan = 1;
    receipt.execute_before_hoc_input =
        production->execute_before_hoc_input && render->execute_before_hoc_input;
    receipt.draw_opened_entrance_frame =
        production->draw_opened_entrance_frame &&
        render->draw_opened_entrance_frame;
    receipt.clear_champion_panel =
        production->clear_champion_panel && render->clear_champion_panel;
    receipt.render_hall_mirror_overlay =
        production->render_hall_mirror_overlay &&
        render->render_hall_mirror_overlay;
    receipt.draw_champion_mirror_wall_overlay =
        render->draw_champion_mirror_wall_overlay;
    receipt.draw_real_floor_object = render->draw_real_floor_object;
    receipt.draw_real_projectile = render->draw_real_projectile;
    receipt.require_runtime_spell_effect_receipt =
        render->require_runtime_spell_effect_receipt;
    receipt.suppress_title_surface = production->suppress_title_surface;
    receipt.suppress_closed_door_frame =
        production->suppress_closed_door_frame;
    receipt.suppress_host_fallback_visuals = 1;
    receipt.suppress_false_item_payloads =
        production->suppress_false_item_payloads;
    receipt.suppress_projectile_payloads =
        production->suppress_projectile_payloads;
    receipt.suppress_spell_effect_payloads =
        production->suppress_spell_effect_payloads;
    receipt.suppress_mirror_payload_things =
        production->suppress_mirror_payload_things;
    receipt.suppress_materialized_item_payload =
        render->suppress_materialized_item_payload;
    receipt.redmcsb_c026_portrait_overlay_ready =
        production->redmcsb_c026_portrait_overlay_ready &&
        render->draw_champion_mirror_wall_overlay;
    receipt.redmcsb_c346_mirror_backing_ready =
        production->redmcsb_c346_mirror_backing_ready &&
        render->render_hall_mirror_overlay;
    receipt.redmcsb_f0115_thing_layer_suppression_ready =
        production->redmcsb_f0115_thing_layer_suppression_ready &&
        render->suppress_mirror_floor_item_payload &&
        render->suppress_mirror_projectile_payload &&
        render->suppress_mirror_spell_effect_payload;
    receipt.block_enter_until_champion_selected =
        production->block_enter_until_champion_selected &&
        render->block_enter_until_champion_selected;
    receipt.map_index = production->map_index;
    receipt.map_width = production->map_width;
    receipt.map_height = production->map_height;
    receipt.entrance_door_frame_index =
        production->entrance_door_frame_index;
    receipt.hall_overlay_kind = production->hall_overlay_kind;
    receipt.render_command_count = production->render_command_count;
    receipt.zone = render->zone;
    receipt.row = render->row;
    receipt.view_cell = render->view_cell;
    receipt.walk_capture_safe = production->walk_capture_safe;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_owned_host_draw_receipt_pc34(
    const DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34* ownership,
    const DM1_V1_ChampionMirrorRenderReceiptPc34* render,
    int candidate_panel_active,
    int backing_asset_available,
    DM1_V1_ChampionMirrorHostDrawReceiptPc34* out_receipt) {
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!ownership || !render) {
        *out_receipt = receipt;
        return 0;
    }
    if (!ownership->ready ||
        !ownership->consume_dm1_receipts_only ||
        !ownership->no_m11_fallback_scan ||
        !ownership->render_hall_mirror_overlay ||
        !ownership->draw_champion_mirror_wall_overlay ||
        !ownership->redmcsb_c026_portrait_overlay_ready ||
        !ownership->redmcsb_c346_mirror_backing_ready ||
        !ownership->redmcsb_f0115_thing_layer_suppression_ready ||
        !ownership->suppress_host_fallback_visuals) {
        *out_receipt = receipt;
        return 1;
    }
    /* ReDMCSB DUNVIEW.C:3913-3928 draws C346/C026 from the wall-overlay
     * route.  M11 may execute this host draw only through the DM1 HoC
     * ownership receipt; missing backing assets must stop the draw rather
     * than reopening the old host fallback rectangle path. */
    if (!DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
            render,
            candidate_panel_active ? 1 : 0,
            backing_asset_available ? 1 : 0,
            &receipt) ||
        !receipt.valid) {
        *out_receipt = receipt;
        /* A missing C346 backing asset is a handled, fail-closed draw
         * decision. The caller must receive the invalid receipt so it can
         * block the host fallback rather than treating it as an API failure. */
        return 1;
    }
    if (!receipt.candidatePanelOwnsCell &&
        (!receipt.drawMirrorBackingAsset ||
         receipt.drawMirrorBackingFallbackRect)) {
        memset(&receipt, 0, sizeof(receipt));
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks,
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome,
    DM1_V1_StartupHostApplyResult_PC34* out_result) {
    DM1_V1_StartupHandoffOutcome_PC34 local_outcome;
    DM1_V1_StartupHostApplyResult_PC34 local_result;

    if (!out_outcome || !out_result) {
        return 0;
    }
    memset(&local_outcome, 0, sizeof(local_outcome));
    memset(&local_result, 0, sizeof(local_result));
    if (!dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
            source_id,
            handoff_callbacks,
            &local_outcome)) {
        return 0;
    }
    if (!dm1_v1_startup_apply_handoff_outcome_pc34(&local_outcome,
                                                   source_id,
                                                   host_callbacks,
                                                   &local_result)) {
        return 0;
    }
    *out_outcome = local_outcome;
    *out_result = local_result;
    return 1;
}

int dm1_v1_startup_execute_selected_launch_transaction_pc34(
    const char* selected_game_id,
    const DM1_V1_StartupSelectedLaunchCallbacks_PC34* callbacks,
    DM1_V1_StartupSelectedLaunchResult_PC34* out_result) {
    DM1_V1_StartupSelectedLaunchResult_PC34 result;
    char opened_source_id[64];

    if (!out_result) {
        return 0;
    }
    memset(&result, 0, sizeof(result));
    memset(opened_source_id, 0, sizeof(opened_source_id));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(selected_game_id)) {
        *out_result = result;
        return 1;
    }
    if (!callbacks ||
        !callbacks->handoff_callbacks ||
        !callbacks->host_callbacks ||
        !callbacks->open_selected_entry ||
        !callbacks->after_open) {
        return 0;
    }
    result.handled = 1;
    /* ReDMCSB source handoff chain: SWSH.C runs START.PRG, STARTUP1.C enters
     * TITLE.C F0437, then ENTRANCE.C F0441 handles the entrance command.  This
     * transaction keeps that sequence on the DM1 side of the M11 boundary. */
    if (!dm1_v1_startup_execute_handoff_prelude_pc34(
            selected_game_id,
            callbacks->handoff_callbacks)) {
        return 0;
    }
    if (!callbacks->open_selected_entry(callbacks->user,
                                        opened_source_id,
                                        (int)sizeof(opened_source_id))) {
        result.launch_failed = 1;
        if (callbacks->mark_launch_failed &&
            !callbacks->mark_launch_failed(callbacks->user)) {
            return 0;
        }
        *out_result = result;
        return 1;
    }
    result.opened = 1;
    if (!callbacks->after_open(callbacks->user)) {
        return 0;
    }
    if (!dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
            opened_source_id,
            callbacks->handoff_callbacks,
            callbacks->host_callbacks,
            &result.handoff_outcome,
            &result.host_apply_result)) {
        return 0;
    }
    if (!dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
            selected_game_id,
            opened_source_id,
            &result.handoff_outcome,
            &result.host_apply_result,
            &result.runtime_handoff_receipt)) {
        return 0;
    }
    if (result.runtime_handoff_receipt.draw_opened_runtime &&
        callbacks->draw_opened &&
        !callbacks->draw_opened(callbacks->user)) {
        return 0;
    }
    *out_result = result;
    return 1;
}

int dm1_v1_startup_selected_launch_route_receipt_pc34(
    const DM1_V1_StartupSelectedLaunchRouteFacts_PC34* facts,
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34* out_receipt) {
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.requires_source_visible_intro =
        dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->selected_game_id);
    receipt.use_dm1_transaction =
        receipt.requires_source_visible_intro ? 1 : 0;
    receipt.use_generic_launch =
        receipt.requires_source_visible_intro ? 0 : 1;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_receipt_phase_pc34(int level_loaded,
                                      int intro_bypassed,
                                      char* out_phase,
                                      int out_phase_size) {
    const char* phase;

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    if (!level_loaded) {
        phase = "dm1-loading";
    } else {
        phase = intro_bypassed
            ? "dm1-runtime-direct"
            : "dm1-runtime";
    }
    snprintf(out_phase, (size_t)out_phase_size, "%s", phase);
    return 1;
}

int dm1_v1_startup_boot_probe_receipt_pc34(int level_loaded,
                                           int intro_bypassed,
                                           char* out_phase,
                                           int out_phase_size,
                                           int* out_startup_active,
                                           char* out_animation,
                                           int out_animation_size,
                                           int* out_animation_active,
                                           int* out_title_frame,
                                           int* out_title_frame_max,
                                           int* out_title_ready) {
    const char* animation;

    if (!out_phase || out_phase_size <= 0 ||
        !out_startup_active ||
        !out_animation || out_animation_size <= 0 ||
        !out_animation_active ||
        !out_title_frame ||
        !out_title_frame_max ||
        !out_title_ready) {
        return 0;
    }
    if (!dm1_v1_startup_receipt_phase_pc34(level_loaded,
                                           intro_bypassed,
                                           out_phase,
                                           out_phase_size)) {
        return 0;
    }
    /* ReDMCSB: SWSH.C -> STARTUP1.C -> TITLE.C F0437 -> ENTRANCE.C.
     * M11 reaches this receipt after the source-visible launcher/title
     * path has completed or through the explicit direct-view bypass. */
    animation = intro_bypassed ? "dm1-title-bypassed" : "dm1-title";
    snprintf(out_animation, (size_t)out_animation_size, "%s", animation);
    *out_startup_active = 0;
    *out_animation_active = 0;
    *out_title_frame = V1_TITLE_DAT_FRAME_MAX;
    *out_title_frame_max = V1_TITLE_DAT_FRAME_MAX;
    *out_title_ready = 1;
    return 1;
}

int dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
    const DM1_V1_StartupBootProbeFacts_PC34* facts,
    DM1_V1_StartupBootProbeReceipt_PC34* out_receipt) {
    DM1_V1_StartupBootProbeReceipt_PC34 receipt;
    int intro_bypassed;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(facts->source_id)) {
        *out_receipt = receipt;
        return 1;
    }
    intro_bypassed = dm1_v1_startup_intro_bypass_applies_to_source_pc34(
        facts->source_id,
        facts->intro_bypassed);
    receipt.handled = 1;
    snprintf(receipt.source_id,
             sizeof(receipt.source_id),
             "%s",
             facts->source_id ? facts->source_id : "");
    receipt.dm1_startup_intro_bypassed = intro_bypassed;
    receipt.level_loaded = facts->level_loaded;
    receipt.map_index = facts->map_index;
    receipt.party_x = facts->party_x;
    receipt.party_y = facts->party_y;
    receipt.party_dir = facts->party_dir;
    receipt.champion_count = facts->champion_count;
    receipt.runtime_tick = facts->runtime_tick;
    receipt.world_tick = facts->world_tick;
    if (!dm1_v1_startup_boot_probe_receipt_pc34(
            facts->level_loaded,
            intro_bypassed,
            receipt.startup_phase,
            (int)sizeof(receipt.startup_phase),
            &receipt.startup_active,
            receipt.startup_animation,
            (int)sizeof(receipt.startup_animation),
            &receipt.startup_animation_active,
            &receipt.startup_title_frame,
            &receipt.startup_title_frame_max,
            &receipt.startup_title_ready)) {
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_selected_boot_probe_receipt_pc34(
    const DM1_V1_StartupSelectedBootProbeFacts_PC34* facts,
    DM1_V1_StartupSelectedBootProbeReceipt_PC34* out_receipt) {
    DM1_V1_StartupSelectedBootProbeReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.active = facts->active ? 1 : 0;
    receipt.started_from_launcher = facts->started_from_launcher ? 1 : 0;
    receipt.source_matches =
        (facts->expected_game_id &&
         facts->actual_source_id &&
         strcmp(facts->actual_source_id, facts->expected_game_id) == 0)
            ? 1
            : 0;
    receipt.selected_entry_receipt_valid =
        dm1_v1_startup_selected_entry_receipt_valid_pc34(
            facts->expected_game_id,
            facts->intro_bypassed);
    receipt.valid =
        receipt.active &&
        receipt.source_matches &&
        receipt.started_from_launcher &&
        receipt.selected_entry_receipt_valid;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
    const DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34* facts,
    DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34* out_receipt) {
    DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.actual_source_kind = facts->actual_source_kind;
    receipt.expected_source_kind = facts->dm1_builtin_source_kind;
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->expected_game_id)) {
        receipt.valid = 1;
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB: TITLE.C F0437 and ENTRANCE.C F0441 are part of the DM1
     * built-in source startup path. Firestaff keeps M11's enum private, but
     * DM1 owns the receipt requiring selected boot probes to come from the
     * built-in catalog source kind. */
    receipt.handled = 1;
    receipt.valid =
        facts->actual_source_kind == facts->dm1_builtin_source_kind;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_title_menu_eligibility_receipt_pc34(
    const DM1_V1_StartupTitleMenuEligibilityFacts_PC34* facts,
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34* out_receipt) {
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34 receipt;
    unsigned int frame_max;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.keep_title_surface = 1;
    receipt.next_stage = DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34;
    receipt.reason = "title-active";

    frame_max = facts->title_frame_max
                    ? facts->title_frame_max
                    : dm1_v1_startup_title_frame_bank_equivalent_steps_pc34();
    if (!facts->title_handoff_ready || facts->title_frame <= frame_max) {
        *out_receipt = receipt;
        return 1;
    }
    if (!facts->advance_requested) {
        receipt.reason = "title-held-after-handoff";
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437:319-409 completes PRESENTS, title zoom,
     * STRIKES BACK, and the final VBlank guard before ENTRANCE.C F0441:
     * 850-883 enters the entrance-wait loop.  The handoff click/key is a
     * title/menu transition only; it must not become the first entrance
     * command. */
    receipt.menu_eligible = 1;
    receipt.keep_title_surface = 0;
    receipt.consume_pending_input = 1;
    receipt.next_stage = DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34;
    receipt.reason = "title-complete-menu-eligible";
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_full_graphics_media_receipt_pc34(
    const char* source_id,
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34* out_receipt) {
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 receipt;
    V1_TitleFrontendSourceTiming title_timing;
    EntranceCompatSourceAnimationStep entrance_pre_open_step;
    DM1_V1_PaletteEntranceResultPc34 entrance_palette;
    DM1_V1_PaletteCreditsResultPc34 credits_palette;
    int presents_palette = 0;
    int title_palette = 0;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    memset(&entrance_palette, 0, sizeof(entrance_palette));
    memset(&credits_palette, 0, sizeof(credits_palette));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        *out_receipt = receipt;
        return 1;
    }

    title_timing = V1_TitleFrontend_GetSourceTimingEvidence();
    if (!V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS,
            &presents_palette) ||
        !V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT,
            &title_palette) ||
        presents_palette != VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS ||
        title_palette != VGA_PALETTE_PC34_SPECIAL_TITLE) {
        /* ReDMCSB TITLE.C F0437:319-324 uses C12_PRESENTS, then
         * F0437:362-402 installs C13_DUNGEON + C14_MASTER.  A shared or
         * unresolved palette would visibly corrupt the original transition. */
        return 0;
    }
    if (!dm1_v1_palette_entrance_run_pc34(&entrance_palette) ||
        !entrance_palette.accepted ||
        dm1_v1_startup_entrance_palette_fingerprint_pc34() == 0U ||
        !dm1_v1_palette_credits_run_pc34(&credits_palette) ||
        !credits_palette.accepted ||
        dm1_v1_startup_credits_palette_fingerprint_pc34() == 0U) {
        return 0;
    }

    receipt.handled = 1;
    receipt.play_swsh = 1;
    receipt.play_title = 1;
    receipt.play_entrance = 1;
    receipt.swsh_vblank_ms = SWSH_COMPAT_RUNTIME_VBLANK_MS;
    receipt.swsh_initial_logo_hold_ms =
        SWSH_Compat_GetRuntimeInitialLogoHoldMs();
    receipt.swsh_palette_wait_ms =
        SWSH_Compat_GetRuntimeDelayMsForVblankCount(
            SWSH_Compat_GetSourceTimingEvidence().paletteWaitVblankCount);
    receipt.swsh_sound_wait_ms =
        SWSH_Compat_GetRuntimeDelayMsForVblankCount(
            SWSH_Compat_GetSourceTimingEvidence().soundWaitVblankCount);
    receipt.swsh_final_hold_ms = SWSH_Compat_GetRuntimeFinalHoldMs();
    receipt.title_presents_hold_ms =
        V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(&title_timing);
    receipt.title_zoom_frame_delay_ms =
        V1_TitleFrontend_GetRuntimeFrameDelayMs(&title_timing);
    receipt.title_zoom_step_count = title_timing.zoomStepCount;
    receipt.title_post_zoom_guard_ms =
        V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&title_timing);
    receipt.title_c001_cadence_pad_ms =
        V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(&title_timing);
    receipt.title_source_animation_steps =
        title_timing.sourceAnimationStepCount;
    receipt.title_frame_bank_equivalent_steps =
        title_timing.frameBankEquivalentStepCount;
    /* TITLE.DAT exposes 53 decoded fallback records, but the production
     * PC/F20 path has only the 23 source-visible TITLE.C events. Do not
     * let the fallback-bank boundary hold the C001 title surface before
     * ENTRANCE.C gets control. */
    receipt.title_menu_boundary_frame =
        DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34 + 1U;
    receipt.title_presents_palette = presents_palette;
    receipt.title_zoom_palette = title_palette;
    receipt.title_menu_eligible = 1;
    receipt.title_consume_pending_input = 1;
    /* ReDMCSB ENTRANCE.C F0441:850-883 drains the command that reached
     * the entrance, then waits for a fresh command. Headless verification
     * has its separately gated escape hatch at the SDL boundary. */
    receipt.entrance_auto_enter_ms = 0;
    receipt.entrance_source_animation_steps =
        ENTRANCE_Compat_GetSourceAnimationStepCount();
    receipt.entrance_door_step_count =
        ENTRANCE_Compat_GetDoorAnimationStepCount();
    receipt.entrance_vblank_ms = ENTRANCE_Compat_GetVblankDelayMs();
    receipt.entrance_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    receipt.entrance_palette_entry_count =
        (unsigned int)entrance_palette.tableSize;
    receipt.entrance_palette_fingerprint =
        dm1_v1_startup_entrance_palette_fingerprint_pc34();
    receipt.entrance_credits_wait_ticks =
        ENTRANCE_Compat_GetCreditsWaitTicks();
    receipt.entrance_credits_palette = VGA_PALETTE_PC34_SPECIAL_CREDITS;
    receipt.entrance_credits_palette_entry_count =
        (unsigned int)credits_palette.tableSize;
    receipt.entrance_credits_palette_fingerprint =
        dm1_v1_startup_credits_palette_fingerprint_pc34();
    memset(&entrance_pre_open_step, 0, sizeof(entrance_pre_open_step));
    if (ENTRANCE_Compat_GetSourceAnimationStep(6u, &entrance_pre_open_step) &&
        entrance_pre_open_step.kind ==
            ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY) {
        receipt.entrance_pre_open_delay_ms =
            ENTRANCE_Compat_GetRuntimeDelayMs(&entrance_pre_open_step);
    }
    /* ReDMCSB NECIO.C lines 3592-3609: SWSH sets black/normal curtain,
     * expands the FTL logo, waits F0022_MAIN_SwooshDelay(20), starts sound,
     * then applies palette waits. TITLE.C F0437:319-409 owns PRESENTS,
     * zoom, STRIKES BACK, and final guard before ENTRANCE.C F0441. */
    receipt.source_evidence =
        "ReDMCSB NECIO.C:3592-3609; TITLE.C:319-409; ENTRANCE.C:850-883";
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
    const char* source_id,
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34* out_receipt) {
    if (!out_receipt) {
        return 0;
    }
    if (!dm1_v1_startup_full_graphics_media_receipt_pc34(source_id,
                                                         out_receipt)) {
        return 0;
    }
    return out_receipt->handled ? 1 : 0;
}

int dm1_v1_startup_title_timing_receipt_valid_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt) {
    V1_TitleFrontendSourceTiming timing;
    int presents_palette = 0;
    int title_palette = 0;

    if (!media_receipt || !media_receipt->handled || !media_receipt->play_title) {
        return 0;
    }
    timing = V1_TitleFrontend_GetSourceTimingEvidence();
    if (!V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS, &presents_palette) ||
        !V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT, &title_palette)) {
        return 0;
    }
    /* ReDMCSB TITLE.C F0437:312-327 selects C12_PRESENTS only for the
     * PRESENTS strip. F0437:362-409 then selects C13_DUNGEON + C14_MASTER
     * for zoom/reveal and waits one VBlank per zoom blit, followed by two
     * post-zoom and one final-guard VBlanks. A stale receipt must not
     * override those palette/timing facts merely because it is handled. */
    return
        media_receipt->title_presents_hold_ms ==
            V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(&timing) &&
        media_receipt->title_zoom_frame_delay_ms ==
            V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing) &&
        media_receipt->title_zoom_step_count == timing.zoomStepCount &&
        media_receipt->title_post_zoom_guard_ms ==
            V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing) &&
        media_receipt->title_c001_cadence_pad_ms ==
            V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(&timing) &&
        media_receipt->title_source_animation_steps ==
            timing.sourceAnimationStepCount &&
        media_receipt->title_frame_bank_equivalent_steps ==
            timing.frameBankEquivalentStepCount &&
        media_receipt->title_menu_boundary_frame ==
            timing.sourceAnimationStepCount + 1u &&
        media_receipt->title_presents_palette == presents_palette &&
        media_receipt->title_zoom_palette == title_palette;
}

int dm1_v1_startup_title_runtime_source_receipt_pc34(
    const char* source_id,
    int graphics_c001_candidate_available,
    unsigned int graphics_c001_width,
    unsigned int graphics_c001_height,
    int title_dat_fallback_available,
    DM1_V1_StartupTitleRuntimeSourceReceipt_PC34* out_receipt) {
    DM1_V1_StartupTitleRuntimeSourceReceipt_PC34 receipt;
    V1_TitleFrontendRuntimeSourceDecision decision;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        *out_receipt = receipt;
        return 1;
    }

    decision = V1_TitleFrontend_SelectRuntimeSource(
        graphics_c001_candidate_available,
        graphics_c001_width,
        graphics_c001_height,
        title_dat_fallback_available);

    receipt.handled = 1;
    receipt.graphics_c001_usable = decision.graphicsC001Usable ? 1 : 0;
    receipt.title_dat_fallback_usable =
        decision.titleDatFallbackUsable ? 1 : 0;
    /* ReDMCSB TITLE.C F0437 PC/F20 loads C001 itself. TITLE.DAT is a
     * different file-format route and cannot stand in for the source-visible
     * PC34 startup animation when C001 is absent or malformed. */
    if (decision.source == V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK) {
        decision.source = V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP;
    }
    receipt.selected_runtime_source = (int)decision.source;
    receipt.require_graphics_c001_for_release_start =
        (decision.source == V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001)
            ? 1
            : 0;
    receipt.fallback_is_visible_last_resort = 0;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437 lines 309-324 loads C001_GRAPHIC_TITLE "
        "before PRESENTS; DM1 PC34 startup rejects TITLE.DAT substitution "
        "when C001 is unavailable or too small.";
    *out_receipt = receipt;
    return 1;
}

static int dm1_v1_startup_title_region_has_pixels_pc34(
    const unsigned char* pixels,
    unsigned int width,
    unsigned int source_y,
    unsigned int source_height) {
    unsigned int y;
    unsigned int x;

    for (y = source_y; y < source_y + source_height; ++y) {
        for (x = 0U; x < width; ++x) {
            if (pixels[y * width + x] != 0U) return 1;
        }
    }
    return 0;
}

int dm1_v1_startup_title_runtime_asset_receipt_pc34(
    const char* source_id,
    const unsigned char* graphics_c001_pixels,
    unsigned int graphics_c001_width,
    unsigned int graphics_c001_height,
    DM1_V1_StartupTitleRuntimeAssetReceipt_PC34* out_receipt) {
    DM1_V1_StartupTitleRuntimeAssetReceipt_PC34 receipt;
    DM1_V1_StartupTitleRuntimeSourceReceipt_PC34 source;
    unsigned int hash = 2166136261u;
    unsigned int index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    memset(&source, 0, sizeof(source));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        *out_receipt = receipt;
        return 1;
    }
    if (!dm1_v1_startup_title_runtime_source_receipt_pc34(
            source_id, graphics_c001_pixels != NULL, graphics_c001_width,
            graphics_c001_height, 0, &source) ||
        !source.handled ||
        source.selected_runtime_source !=
            (int)V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.graphics_c001_dimensions_valid =
        graphics_c001_width == 320U && graphics_c001_height == 200U;
    if (!receipt.graphics_c001_dimensions_valid) {
        *out_receipt = receipt;
        return 1;
    }
    /* TITLE.C F0437:319-324, 333-340, and 340-360 respectively. */
    receipt.dungeon_source_pixels_present =
        dm1_v1_startup_title_region_has_pixels_pc34(
            graphics_c001_pixels, graphics_c001_width, 0U, 80U);
    receipt.master_source_pixels_present =
        dm1_v1_startup_title_region_has_pixels_pc34(
            graphics_c001_pixels, graphics_c001_width, 80U, 57U);
    receipt.presents_source_pixels_present =
        dm1_v1_startup_title_region_has_pixels_pc34(
            graphics_c001_pixels, graphics_c001_width, 137U, 16U);
    for (index = 0U; index < graphics_c001_width * graphics_c001_height;
         ++index) {
        hash ^= graphics_c001_pixels[index];
        hash *= 16777619u;
    }
    receipt.graphics_c001_pixel_fingerprint = hash ? hash : 1U;
    receipt.release_c001_ready =
        receipt.dungeon_source_pixels_present &&
        receipt.master_source_pixels_present &&
        receipt.presents_source_pixels_present &&
        receipt.graphics_c001_pixel_fingerprint != 0U;
    receipt.source_evidence =
        "ReDMCSB TITLE.C F0437:309-310,319-324,333-340,340-360";
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_title_presentation_command_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    const DM1_V1_StartupTitleRuntimeAssetReceipt_PC34* asset_receipt,
    unsigned int source_step,
    DM1_V1_StartupTitlePresentationCommand_PC34* out_command) {
    DM1_V1_StartupTitlePresentationCommand_PC34 command;
    V1_TitleFrontendSourceAnimationStep step;
    V1_TitleFrontendC001BlitPlan plan;
    int palette = 0;

    if (!out_command || !media_receipt || !asset_receipt ||
        !dm1_v1_startup_title_timing_receipt_valid_pc34(media_receipt) ||
        !asset_receipt->release_c001_ready || source_step == 0U) {
        return 0;
    }
    memset(&command, 0, sizeof(command));
    memset(&step, 0, sizeof(step));
    memset(&plan, 0, sizeof(plan));
    if (!V1_TitleFrontend_GetSourceAnimationStep(source_step, &step) ||
        !V1_TitleFrontend_GetC001BlitPlanForStep(&step, &plan) ||
        !V1_TitleFrontend_GetStepPalette(step.kind, &palette)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437:313 installs C12 only for PRESENTS. Lines
     * 363-367 install C13+C14 before the C001 zoom/reveal path. */
    if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS) {
        if (palette != media_receipt->title_presents_palette) return 0;
    } else if (palette != media_receipt->title_zoom_palette) {
        return 0;
    }

    command.handled = 1;
    command.source_step = source_step;
    command.present_frame = plan.kind != V1_TITLE_FRONTEND_C001_BLIT_NONE;
    command.clear_before_present = plan.clearBeforeBlit ? 1 : 0;
    command.special_palette = palette;
    command.pre_present_delay_ms = step.vblankBeforeEvent
        ? media_receipt->title_zoom_frame_delay_ms : 0U;
    command.post_present_delay_ms =
        step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS
            ? media_receipt->title_presents_hold_ms : 0U;
    command.source_timing_receipt_consumed = 1;
    command.source_asset_receipt_consumed = 1;
    command.source_evidence = step.sourceLineEvidence;
    *out_command = command;
    return 1;
}

unsigned int dm1_v1_startup_entrance_step_delay_ms_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    int entrance_event_kind,
    unsigned int delay_ticks,
    unsigned int vblank_loop_count) {
    if (!media_receipt || !media_receipt->handled ||
        media_receipt->entrance_vblank_ms == 0U) {
        return 0U;
    }
    if (entrance_event_kind == ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY &&
        media_receipt->entrance_pre_open_delay_ms > 0U) {
        return media_receipt->entrance_pre_open_delay_ms;
    }
    if (delay_ticks > 0U) {
        if (delay_ticks > 0xffffffffU / media_receipt->entrance_vblank_ms) {
            return 0U;
        }
        return delay_ticks * media_receipt->entrance_vblank_ms;
    }
    if (vblank_loop_count > 0U) {
        if (vblank_loop_count >
            0xffffffffU / media_receipt->entrance_vblank_ms) {
            return 0U;
        }
        return vblank_loop_count * media_receipt->entrance_vblank_ms;
    }
    return 0U;
}

int dm1_v1_startup_entrance_timing_receipt_valid_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt) {
    EntranceCompatSourceAnimationStep pre_open_step;
    DM1_V1_PaletteEntranceResultPc34 entrance_palette;

    if (!media_receipt || !media_receipt->handled) {
        return 0;
    }
    memset(&pre_open_step, 0, sizeof(pre_open_step));
    memset(&entrance_palette, 0, sizeof(entrance_palette));
    if (!ENTRANCE_Compat_GetSourceAnimationStep(6U, &pre_open_step) ||
        pre_open_step.kind != ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY) {
        return 0;
    }
    return
        media_receipt->entrance_source_animation_steps ==
            ENTRANCE_Compat_GetSourceAnimationStepCount() &&
        media_receipt->entrance_door_step_count ==
            ENTRANCE_Compat_GetDoorAnimationStepCount() &&
        media_receipt->entrance_vblank_ms ==
            ENTRANCE_Compat_GetVblankDelayMs() &&
        media_receipt->entrance_pre_open_delay_ms ==
            ENTRANCE_Compat_GetRuntimeDelayMs(&pre_open_step) &&
        dm1_v1_palette_entrance_run_pc34(&entrance_palette) &&
        entrance_palette.accepted &&
        media_receipt->entrance_palette ==
            VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
        media_receipt->entrance_palette_entry_count ==
            (unsigned int)entrance_palette.tableSize &&
        media_receipt->entrance_palette_fingerprint ==
            dm1_v1_startup_entrance_palette_fingerprint_pc34();
}

int dm1_v1_startup_entrance_credits_presentation_command_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    const unsigned char* graphics_c005_pixels,
    unsigned int graphics_c005_width,
    unsigned int graphics_c005_height,
    DM1_V1_StartupEntranceCreditsPresentationCommand_PC34* out_command) {
    DM1_V1_StartupEntranceCreditsPresentationCommand_PC34 command;
    DM1_V1_PaletteCreditsResultPc34 palette;
    unsigned int hash = 2166136261u;
    unsigned int index;
    int has_pixels = 0;

    if (!out_command || !media_receipt || !graphics_c005_pixels ||
        !dm1_v1_startup_entrance_timing_receipt_valid_pc34(media_receipt) ||
        graphics_c005_width != 320U || graphics_c005_height != 200U) {
        return 0;
    }
    memset(&palette, 0, sizeof(palette));
    if (!dm1_v1_palette_credits_run_pc34(&palette) || !palette.accepted ||
        media_receipt->entrance_credits_wait_ticks !=
            ENTRANCE_Compat_GetCreditsWaitTicks() ||
        media_receipt->entrance_credits_palette !=
            VGA_PALETTE_PC34_SPECIAL_CREDITS ||
        media_receipt->entrance_credits_palette_entry_count !=
            (unsigned int)palette.tableSize ||
        media_receipt->entrance_credits_palette_fingerprint !=
            dm1_v1_startup_credits_palette_fingerprint_pc34()) {
        return 0;
    }
    for (index = 0U; index < graphics_c005_width * graphics_c005_height;
         ++index) {
        const unsigned char pixel = graphics_c005_pixels[index];
        has_pixels |= pixel != 0U;
        hash ^= pixel;
        hash *= 16777619u;
    }
    if (!has_pixels) return 0;

    /* ReDMCSB ENTRANCE.C F0442:1004-1061 presents C005, restores the
     * normal curtain, fades to DATA.C G0019, then waits on L1406=1800. */
    memset(&command, 0, sizeof(command));
    command.handled = 1;
    command.present_credits_frame = 1;
    command.source_asset_receipt_consumed = 1;
    command.source_palette_receipt_consumed = 1;
    command.source_timing_receipt_consumed = 1;
    command.special_palette = media_receipt->entrance_credits_palette;
    command.credits_wait_ticks = media_receipt->entrance_credits_wait_ticks;
    command.vblank_delay_ms = media_receipt->entrance_vblank_ms;
    command.graphics_c005_pixel_fingerprint = hash ? hash : 1U;
    command.source_evidence =
        "ReDMCSB ENTRANCE.C F0442:1004-1091; DATA.C G0019 PC34";
    *out_command = command;
    return 1;
}

int dm1_v1_startup_entrance_credits_return_command_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    const DM1_V1_StartupEntranceCreditsPresentationCommand_PC34*
        credits_command,
    DM1_V1_StartupEntranceCreditsReturnCommand_PC34* out_command) {
    DM1_V1_StartupEntranceCreditsReturnCommand_PC34 command;

    if (!out_command || !media_receipt || !credits_command ||
        !dm1_v1_startup_entrance_timing_receipt_valid_pc34(media_receipt) ||
        !credits_command->handled ||
        !credits_command->present_credits_frame ||
        !credits_command->source_asset_receipt_consumed ||
        !credits_command->source_palette_receipt_consumed ||
        !credits_command->source_timing_receipt_consumed ||
        media_receipt->entrance_credits_palette !=
            VGA_PALETTE_PC34_SPECIAL_CREDITS ||
        credits_command->special_palette !=
            VGA_PALETTE_PC34_SPECIAL_CREDITS ||
        credits_command->credits_wait_ticks !=
            media_receipt->entrance_credits_wait_ticks ||
        credits_command->vblank_delay_ms != media_receipt->entrance_vblank_ms ||
        credits_command->graphics_c005_pixel_fingerprint == 0U) {
        return 0;
    }

    /* ReDMCSB ENTRANCE.C F0441:846-883 is an outer do-loop: after F0442
     * returns C202, it redraws C004/doors, discards the credits-dismissal
     * input, restores C099, and waits for a fresh entrance command. */
    memset(&command, 0, sizeof(command));
    command.handled = 1;
    command.credits_phase_receipt_consumed = 1;
    command.redraw_closed_entrance = 1;
    command.discard_pending_input = 1;
    command.present_entrance_palette = 1;
    command.special_palette = media_receipt->entrance_palette;
    command.wait_vblank_delay_ms = media_receipt->entrance_vblank_ms;
    command.source_evidence = "ReDMCSB ENTRANCE.C F0441:846-883; F0442:1004-1091";
    *out_command = command;
    return 1;
}

int dm1_v1_startup_entrance_render_audio_command_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    unsigned int source_step,
    int entrance_event_kind,
    unsigned int delay_ticks,
    unsigned int vblank_loop_count,
    DM1_V1_StartupEntranceRenderAudioCommand_PC34* out_command) {
    DM1_V1_StartupEntranceRenderAudioCommand_PC34 command;
    EntranceCompatSourceAnimationStep source_step_evidence;

    /* ReDMCSB ENTRANCE.C F0441 lines 850-883 drives the entrance as an
     * ordered render/wait loop before the dungeon handoff.  Keep M11 on this
     * DM1 receipt command path so render, palette, sound marker, and delay
     * decisions come from the same source-locked entrance step. */
    if (!out_command || !media_receipt ||
        !dm1_v1_startup_entrance_timing_receipt_valid_pc34(media_receipt) ||
        source_step == 0U ||
        source_step > media_receipt->entrance_source_animation_steps) {
        return 0;
    }
    memset(&source_step_evidence, 0, sizeof(source_step_evidence));
    if (!ENTRANCE_Compat_GetSourceAnimationStep(
            source_step, &source_step_evidence) ||
        (int)source_step_evidence.kind != entrance_event_kind ||
        source_step_evidence.delayTicks != delay_ticks ||
        source_step_evidence.vblankLoopCount != vblank_loop_count) {
        /* ReDMCSB ENTRANCE.C F0441/F0438 owns the event ordering; do not
         * execute a caller-supplied delay or event kind that differs from
         * the original entrance schedule. */
        return 0;
    }
    memset(&command, 0, sizeof(command));
    command.handled = 1;
    command.consume_media_receipt_only = 1;
    command.source_timing_receipt_consumed = 1;
    command.lower_level_renderer_helper_owned = 1;
    command.lower_level_audio_helper_owned = 1;
    command.source_step = source_step;
    command.present_entrance_palette = 1;
    command.entrance_palette = media_receipt->entrance_palette;
    command.entrance_palette_fingerprint =
        media_receipt->entrance_palette_fingerprint;
    command.delay_ms =
        dm1_v1_startup_entrance_step_delay_ms_pc34(media_receipt,
                                                   entrance_event_kind,
                                                   delay_ticks,
                                                   vblank_loop_count);

    switch ((EntranceCompatSourceEventKind)entrance_event_kind) {
        case ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_FADE_BLACK_PC34;
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN:
        case ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT:
        case ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34;
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34;
            /* ReDMCSB ENTRANCE.C F0441:906-920 submits C01_SOUND_SWITCH
             * at volume 112 immediately after a real entrance command,
             * before F0022_MAIN_Delay(20) and F0438 door opening. */
            command.audio_request_ready = 1;
            command.audio_sound_index = 1;
            command.audio_volume = 112U;
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34;
            command.door_animation_step = source_step - 6U;
            if (command.door_animation_step > 0U &&
                command.door_animation_step <=
                    media_receipt->entrance_door_step_count) {
                EntranceCompatDoorStep door_step;
                if (!ENTRANCE_Compat_GetDoorAnimationStep(
                        command.door_animation_step,
                        &door_step)) {
                    return 0;
                }
                (void)door_step;
                dm1_v1_startup_entrance_door_geometry_pc34(
                    command.door_animation_step, &command);
            }
            command.play_door_rattle_sound =
                (command.door_animation_step > 0U &&
                 command.door_animation_step <=
                     media_receipt->entrance_door_step_count &&
                 ((command.door_animation_step % 3U) == 1U));
            if (command.play_door_rattle_sound) {
                /* ReDMCSB: ENTRANCE.C F0438 lines 149-166 requests
                 * C02_SOUND_DOOR_RATTLE at volume 145 on steps 1, 4, ... .
                 * M11 only submits this already-resolved source request to
                 * its audio backend. */
                command.audio_request_ready = 1;
                command.audio_sound_index = 2;
                command.audio_volume = 145U;
            }
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON:
        case ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW:
        default:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_DUNGEON_FRAME_PC34;
            break;
    }

    *out_command = command;
    return 1;
}

int dm1_v1_startup_sequence_source_order_valid_pc34(void) {
    /* ReDMCSB startup source order:
     * SWSH.C:39-47 runs START.PRG after the FTL palette program;
     * STARTUP1.C:143 calls TITLE.C F0437_STARTEND_DrawTitle();
     * TITLE.C:319-409 draws PRESENTS, title zoom, STRIKES BACK and final
     * guard; ENTRANCE.C:850-883 then enters the entrance wait loop.
     */
    return dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
                                           DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34);
}

const char* dm1_v1_startup_sequence_source_evidence_pc34(void) {
    return "ReDMCSB SWSH.C:39-47 -> STARTUP1.C:143 -> TITLE.C:319-409 -> ENTRANCE.C:850-883";
}

unsigned int dm1_v1_startup_title_zoom_steps_pc34(void) {
    /* ReDMCSB: TITLE.C F0437 lines 340-360 prepares 18 shrinked title
     * bitmaps, then lines 385-387 blit them in reverse order. */
    return DM1_V1_STARTUP_TITLE_ZOOM_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_source_animation_steps_pc34(void) {
    /* PRESENTS + 18 zoom blits + 2 post-zoom waits + STRIKES BACK + final
     * guard. This is the complete PC/F20 C001 source event count. */
    return DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_frame_bank_equivalent_steps_pc34(void) {
    /* C001 is the production title route. TITLE.DAT's 53 records are a
     * fallback asset and must not stretch the source-visible cadence. */
    return DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_presents_hold_vblanks_pc34(void) {
    /* TITLE.C has no timed hold between PRESENTS and C001 preparation. */
    return 0u;
}

unsigned int dm1_v1_startup_title_vblank_tick_ms_pc34(void) {
    return DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34;
}

unsigned int dm1_v1_startup_title_presents_hold_ms_pc34(void) {
    return dm1_v1_startup_title_presents_hold_vblanks_pc34() *
           DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34;
}

unsigned int dm1_v1_startup_title_post_zoom_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_POST_ZOOM_VBLANKS_PC34;
}

unsigned int dm1_v1_startup_title_final_guard_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FINAL_GUARD_VBLANKS_PC34;
}
