#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34 artifact;
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34 facts;
    DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34 receipt;

    memset(&artifact, 0, sizeof(artifact));
    memset(&facts, 0, sizeof(facts));
    artifact.handled = artifact.ready = 1;
    artifact.consume_full_start_production_receipt_only = 1;
    artifact.capture_manifest_ready = 1;
    artifact.title_surface_forbidden = artifact.closed_door_frame_forbidden = 1;
    artifact.host_fallback_visuals_forbidden = 1;
    artifact.opened_entrance_frame_required = 1;
    artifact.hall_mirror_overlay_required = 1;
    artifact.clear_champion_panel_required = 1;
    artifact.block_enter_until_champion_selected = 1;
    artifact.expected_map_index = DM1_V1_ENTRANCE_MAP_INDEX_PC34;
    artifact.expected_map_width = DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34;
    artifact.expected_map_height = DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34;
    artifact.expected_entrance_door_frame_index = 9;
    artifact.expected_hall_overlay_kind = DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34;
    artifact.expected_hoc_render_command_count = 3;
    facts.captured_after_first_frame_render = facts.captured_from_real_assets = 1;
    facts.captured_from_mac_window = facts.captured_from_release_app = 1;
    facts.observed_c026_portrait_asset = facts.observed_c346_mirror_backing_asset = 1;
    facts.observed_required_graphics_hash_match = facts.observed_required_dungeon_hash_match = 1;
    facts.observed_host_window_present = facts.observed_presented_rgba_capture = 1;
    facts.presented_capture_width = 320; facts.presented_capture_height = 200;
    facts.presented_capture_byte_count = 320 * 200 * 4; facts.presented_capture_hash = 0x1234u;
    facts.presented_capture_consumer_mask = DM1_V1_HOC_CAPTURE_CONSUMER_HOST_RENDER_PC34;
    facts.presented_capture_chain_hash = dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(320, 200, facts.presented_capture_byte_count, facts.presented_capture_hash, facts.presented_capture_consumer_mask);
    facts.captured_map_index = artifact.expected_map_index; facts.captured_map_width = artifact.expected_map_width; facts.captured_map_height = artifact.expected_map_height;
    facts.captured_entrance_door_frame_index = 9; facts.captured_hall_overlay_kind = artifact.expected_hall_overlay_kind; facts.captured_hoc_render_command_count = 3;
    facts.saw_opened_entrance_frame = facts.saw_hall_mirror_overlay = facts.cleared_champion_panel = facts.blocked_enter_until_champion_selected = 1;
    if (!dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(&artifact, &facts, &receipt) || receipt.proof_passed) return 1;
    facts.observed_live_hoc_f0115_material_request = 1;
    if (!dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(&artifact, &facts, &receipt) || !receipt.proof_passed || !receipt.observed_live_hoc_material_request) return 1;
    puts("ok: DM1 HoC live material gates entrance transition");
    return 0;
}
