/* Real-PC34 CSB Entrance producer gate.  This opens only operator-supplied
 * GRAPHICS.DAT and proves that M11 receives C004's decoded viewport before
 * the established C002/C003 overlay consumer runs. */

#include "csb_v1_boot.h"
#include "csb_v1_startup_entrance_f0128_m11_handoff_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
    } else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static const char *data_dir(char *buffer, size_t buffer_size)
{
    const char *path = getenv("FIRESTAFF_CSB_STARTUP_DATA");
    const char *home;

    if (path && path[0] != '\0') return path;
    path = getenv("FIRESTAFF_DATA_DIR");
    if (path && path[0] != '\0') return path;
    home = getenv("HOME");
    if (!home || home[0] == '\0' || buffer_size == 0u) return NULL;
    snprintf(buffer, buffer_size, "%s/.firestaff/data/csb", home);
    return buffer;
}

static int closed_entrance_plan(CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupRenderState_PC34 state;

    if (!out_plan) return 0;
    memset(&state, 0, sizeof(state));
    state.entrance_active = 1;
    state.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    return csb_v1_startup_source_render_plan_from_state_pc34(&state, out_plan);
}

int main(void)
{
    char default_data_dir[1024];
    const char *root = data_dir(default_data_dir, sizeof(default_data_dir));
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_ViewportFirstFrameMaterializationReceipt material;
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 raster;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host;
    M11_GameViewState view;
    unsigned char pixels[224 * 136];
    const CSB_V1_StartupRuntimeSurface_PC34 *c004;
    const uint32_t source_tick = 41u;
    int row;

    memset(&session, 0, sizeof(session));
    memset(&material, 0, sizeof(material));
    memset(&raster, 0, sizeof(raster));
    memset(&host, 0, sizeof(host));
    printf("data_dir=%s\n", root ? root : "(none)");

    csb_v1_boot_profile_init(&profile);
    if (!root || csb_v1_boot_scan_assets(&profile, root) != 0 ||
        !profile.assets_verified || !profile.graphics_verified ||
        !profile.dungeon_verified || profile.variant_id != CSB_V1_VARIANT_PC34_EN) {
        printf("SKIP: no verified PC34 CSB GRAPHICS.DAT/DUNGEON.DAT corpus was found.\n");
        return 0;
    }
    check(csb_v1_boot_startup_runtime_asset_session_open_pc34(&profile, &session),
          "opens the verified PC34 GRAPHICS.DAT startup session");
    check(closed_entrance_plan(&plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "derives the real C004 closed-Entrance plan");
    if (failures) goto done;

    check(csb_v1_startup_entrance_f0128_produce_pc34(
              &session, &plan, source_tick, &material, &raster, pixels,
              sizeof(pixels)),
          "producer advances the real session and copies the decoded C004 viewport");
    c004 = &session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    check(session.source_tick == source_tick && material.valid &&
              material.consumed_by_m11_render && material.real_graphics_session &&
              material.no_synthetic_pixels && material.no_fallback_visuals &&
              material.combined_material_hash ==
                  c004->decode_receipt.indexed_pixel_fnv1a &&
              raster.valid && raster.consumed_by_raster &&
              raster.combined_material_hash == material.combined_material_hash &&
              raster.raster_hash != 0u,
          "producer receipt retains real C004 decoder provenance and a nonzero crop hash");
    for (row = 0; row < 136; ++row) {
        if (memcmp(pixels + (size_t)row * 224u,
                   c004->pixels + (size_t)(33 + row) * 320u, 224u) != 0) {
            check(0, "producer pixels exactly match C004's native 224x136 viewport");
            break;
        }
    }
    if (row == 136) check(1, "producer pixels exactly match C004's native 224x136 viewport");

    M11_GameView_Init(&view);
    check(M11_GameView_SetCsbEntranceF0128Raster(
              &view, &material, &raster, pixels, sizeof(pixels), source_tick,
              session.generation) && view.csbStartupF0128EntranceBound &&
              view.csbStartupF0128EntranceSourceTick == source_tick &&
              view.csbStartupF0128EntranceSessionGeneration == session.generation &&
              memcmp(view.csbStartupF0128EntrancePixels, pixels,
                     sizeof(pixels)) == 0,
          "M11 stores the real C004 F0128 material rather than a test-only caller buffer");
    check(csb_v1_boot_startup_runtime_entrance_f0128_receipt_from_session_pc34(
              &session, &plan, view.csbStartupF0128EntranceSourceTick,
              &view.csbStartupF0128EntranceRasterReceipt,
              view.csbStartupF0128EntrancePixels,
              sizeof(view.csbStartupF0128EntrancePixels), &host) && host.valid &&
              host.real_asset_matched && host.no_synthetic_surface &&
              host.raster.source_surface_count == 4,
          "production F0128 consumer composes bound C004 before real C002/C003 doors");

done:
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host);
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    return failures ? 1 : 0;
}
