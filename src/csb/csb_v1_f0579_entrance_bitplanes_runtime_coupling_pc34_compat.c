#include "csb_v1_f0579_entrance_bitplanes_runtime_coupling_pc34_compat.h"

#include <string.h>

static int csb_v1_f0579_geometry_matches_pc34(
    const CSB_V1_F0579_EntranceBitplanesFacts_PC34 *facts)
{
    return facts &&
        facts->source_width == CSB_V1_F0579_SOURCE_COMPOSITE_WIDTH_PC34 &&
        facts->source_height == CSB_V1_F0579_SOURCE_COMPOSITE_HEIGHT_PC34 &&
        facts->target_width == CSB_V1_F0579_TARGET_SCREEN_WIDTH_PC34 &&
        facts->target_height == CSB_V1_F0579_TARGET_SCREEN_HEIGHT_PC34 &&
        facts->upper_half_target_y == CSB_V1_F0579_TARGET_UPPER_Y_PC34 &&
        facts->lower_half_target_y == CSB_V1_F0579_TARGET_LOWER_Y_PC34 &&
        facts->source_plane_count == CSB_V1_F0579_DOOR_HALF_PLANE_COUNT_PC34 &&
        facts->target_plane_count == CSB_V1_F0579_DOOR_HALF_PLANE_COUNT_PC34;
}

void csb_v1_f0579_entrance_bitplanes_receipt_init_pc34(
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

int F0579_ENTRANCE_InitializeBitPlanes(
    const CSB_V1_F0579_EntranceBitplanesFacts_PC34 *facts,
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0579_entrance_initialize_bitplanes_source_evidence_pc34();

    csb_v1_f0579_entrance_bitplanes_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !facts->source_composite_real_asset_bound ||
        !facts->target_screen_real_asset_bound ||
        !csb_v1_f0579_geometry_matches_pc34(facts) ||
        !facts->no_legacy_bitplane_wrapper ||
        !facts->no_synthetic_visuals ||
        !facts->runtime_coupling.valid ||
        !facts->runtime_coupling.real_startup_assets_bound ||
        !facts->runtime_coupling.title_phase_route_complete ||
        !facts->runtime_coupling.hud_runtime_coupled ||
        !facts->runtime_coupling.door_runtime_coupled ||
        !facts->runtime_coupling.draw_consumes_receipt_only ||
        !facts->runtime_coupling.input_consumes_receipt_only ||
        !facts->runtime_coupling.no_legacy_wrappers ||
        !facts->runtime_coupling.no_synthetic_visuals) {
        if (out_receipt) {
            out_receipt->no_synthetic_visuals = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->source_composite_bound = 1;
    out_receipt->target_screen_bound = 1;
    out_receipt->geometry_source_locked = 1;
    out_receipt->runtime_coupling_consumed = 1;
    out_receipt->title_hud_door_runtime_ready = 1;
    out_receipt->no_legacy_bitplane_wrapper = 1;
    out_receipt->no_synthetic_visuals = 1;
    out_receipt->source_evidence = evidence;
    return 1;
}

const char *csb_v1_f0579_entrance_initialize_bitplanes_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:1095-1123 F0579_ENTRANCE_InitializeBitPlanes "
           "binds the 256x161 composite door bitmap planes to the 320x200 "
           "screen bitplanes at y=30 and y=110; CSB PC34 accepts this only "
           "after the real title/HUD/door runtime coupling is complete";
}
