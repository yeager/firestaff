/* Real CSB PC34 GRAPHICS.DAT-only F0134/F0135 consumer regression. */
#include "csb_v1_boot.h"
#include "csb_v1_source_bound_fillbox_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static const char *data_dir(char *buffer, size_t buffer_size)
{
    const char *value = getenv("FIRESTAFF_CSB_STARTUP_DATA");
    const char *home;
    if (value && value[0] != '\0') return value;
    value = getenv("FIRESTAFF_DATA_DIR");
    if (value && value[0] != '\0') return value;
    home = getenv("HOME");
    if (!home || buffer_size == 0u) return NULL;
    snprintf(buffer, buffer_size, "%s/.firestaff/data/csb", home);
    return buffer;
}

static int plan_from_state(int opening, int opening_step,
                           CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupRenderState_PC34 state;
    memset(&state, 0, sizeof(state));
    state.entrance_active = 1;
    state.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    state.opening_active = opening;
    state.opening_step = opening_step;
    return csb_v1_startup_source_render_plan_from_state_pc34(&state, out_plan);
}

static int host_receipt(CSB_V1_StartupRuntimeAssetSession_PC34 *session,
                        const CSB_V1_StartupRenderPlan_PC34 *plan,
                        uint32_t source_tick,
                        CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *out)
{
    return csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
        session, plan, source_tick, out) && out->valid &&
        out->real_asset_matched && out->no_legacy_wrappers &&
        out->no_synthetic_surface;
}

int main(void)
{
    char default_root[1024];
    const char *root = data_dir(default_root, sizeof(default_root));
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 door_receipt;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 hud_receipt;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 forged_receipt;
    CSB_V1_SourceBoundFillTarget_PC34 target;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupAudioAction_PC34 audio;
    const int16_t door_box[4] = { 4, 7, 9, 10 };
    const int16_t hud_box[4] = { 10, 13, 12, 13 };
    uint32_t door_source_hash;
    uint32_t hud_source_hash;
    size_t pixel;

    memset(&door_receipt, 0, sizeof(door_receipt));
    memset(&hud_receipt, 0, sizeof(hud_receipt));
    memset(&forged_receipt, 0, sizeof(forged_receipt));
    csb_v1_source_bound_fill_target_init_pc34(&target);
    csb_v1_boot_profile_init(&profile);
    if (!root || csb_v1_boot_scan_assets(&profile, root) != 0 ||
        !profile.assets_verified || !profile.graphics_verified ||
        !profile.dungeon_verified ||
        profile.variant_id != CSB_V1_VARIANT_PC34_EN) {
        printf("SKIP: no verified PC34 CSB GRAPHICS.DAT corpus was found.\n");
        return 0;
    }
    check(csb_v1_boot_startup_runtime_asset_session_open_pc34(&profile, &session),
          "opens a real GRAPHICS.DAT source session");
    if (failures) return 1;
    check(csb_v1_boot_startup_playback_begin_pc34(&session, &audio) &&
              csb_v1_boot_startup_playback_complete_swoosh_pc34(&session, &audio) &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  &session, 0, &plan, &audio) &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  &session, 60, &plan, &audio) &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  &session, 80, &plan, &audio) &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  &session, 100, &plan, &audio) &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  &session, csb_v1_startup_title_total_ticks_pc34(), &plan, &audio),
          "reaches the original Entrance owner after all C001 title phases");
    check(plan_from_state(1, 31, &plan) &&
              host_receipt(&session, &plan, 7u, &door_receipt) &&
              door_receipt.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34,
          "C002/C003 opening raster is decoder-bound before F0135");
    door_source_hash = door_receipt.raster.pixel_hash;
    check(csb_v1_source_bound_fill_target_from_host_receipt_pc34(
              &session, &door_receipt, &target) &&
              target.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 &&
              target.source_pixel_hash == door_source_hash &&
              target.no_fallback_route,
          "door F0135 target copies only the real C002/C003 raster");
    check(csb_v1_source_bound_f0135_fill_box_pc34(&target, door_box, 0x800eu) &&
              target.operation_count == 1u &&
              target.pixels[9u * 320u + 4u] == 14u &&
              target.pixels[9u * 320u + 5u] ==
                  door_receipt.raster.pixels[9u * 320u + 5u] &&
              target.result_pixel_hash != door_source_hash,
          "F0135 preserves the original every-other-pixel fill mode on the door raster");
    check(!csb_v1_source_bound_f0135_fill_box_pc34(
              &target, (const int16_t[]){ -1, 1, 0, 1 }, 3u),
          "F0135 rejects an out-of-bounds box instead of clipping a fallback raster");
    csb_v1_source_bound_fill_target_release_pc34(&target);

    check(csb_v1_boot_startup_playback_complete_entrance_pc34(&session) &&
              csb_v1_boot_startup_playback_enter_hud_pc34(&session),
          "moves the same real session to C017/C040 HUD");
    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    plan.special_palette = -1;
    plan.title_special_palette = -1;
    check(host_receipt(&session, &plan, 9u, &hud_receipt) &&
              hud_receipt.host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34 &&
              session.hud_inventory_binding.source ==
                  CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
              session.hud_resurrect_binding.source ==
                  CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
          "C017/C040 HUD is bound to original GRAPHICS.DAT, not an override");
    hud_source_hash = hud_receipt.raster.pixel_hash;
    check(csb_v1_source_bound_fill_target_from_host_receipt_pc34(
              &session, &hud_receipt, &target) &&
              target.host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34,
          "HUD F0134 target copies only the current C017/C040 raster");
    check(csb_v1_source_bound_f0134_fill_pc34(&target, 12u) &&
              target.operation_count == 1u && target.result_pixel_hash != hud_source_hash,
          "F0134 fills the complete source-owned HUD target with an original palette index");
    for (pixel = 0u; pixel < target.pixel_count; ++pixel) {
        if (target.pixels[pixel] != 12u) { ++failures; break; }
    }
    check(pixel == target.pixel_count, "F0134 has no generated HUD pixels outside its source-owned target");
    check(csb_v1_source_bound_f0135_fill_box_pc34(&target, hud_box, 2u) &&
              target.operation_count == 2u && target.pixels[12u * 320u + 10u] == 2u &&
              target.pixels[13u * 320u + 13u] == 2u,
          "F0135 applies an inclusive source-bound HUD box after F0134");
    forged_receipt = hud_receipt;
    forged_receipt.real_asset_matched = 0;
    check(!csb_v1_source_bound_fill_target_from_host_receipt_pc34(
              &session, &forged_receipt, &target),
          "a forged HUD receipt cannot create a substitute fill target");

    csb_v1_source_bound_fill_target_release_pc34(&target);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&door_receipt);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&hud_receipt);
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    printf("Summary: %s\n", failures ? "FAILED" : "PASSED");
    return failures ? 1 : 0;
}
