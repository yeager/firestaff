#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_RuntimeStaticSceneViewportReceipt receipt;
    DM2_V1_RuntimeDoorSceneOwnershipReceipt door_receipt;
    DM2_V1_RuntimeSceneFrameSelectionReceipt selection;
    DM2_V1_RuntimeSceneFrameSelectionReceipt gated_selection;
    DM2_V1_OriginalMaterialGateReceipt gate;
    DM2_V1_OriginalDoorSurfaceRequest surface_request;
    DM2_V1_OriginalDoorSurfaceBinding surface_binding;
    DM2_V1_OriginalDoorOpeningFrameRequest opening_request;
    uint8_t decoded_u4[4] = { 1u, 2u, 3u, 4u };
    uint8_t palette16[16] = { 0 };
    uint8_t framebuffer[320 * 200];

    dm2_v1_boot_profile_init(&profile);
    dm2_v1_runtime_init(&profile);
    memset(framebuffer, 0, sizeof(framebuffer));
    if (dm2_v1_runtime_render_frame(0, 0, 0, framebuffer, 320, 320, 200) !=
        -1) {
        fprintf(stderr, "FAIL: runtime frame without original assets must block\n");
        return 1;
    }
    memset(&receipt, 0xff, sizeof(receipt));
    if (dm2_v1_runtime_static_scene_viewport_receipt(&receipt) != 0 ||
        receipt.valid != 0 || receipt.map_load_token != 0u ||
        receipt.scene_control_hash != 0u) {
        fprintf(stderr, "FAIL: absent scene record must fail closed\n");
        return 1;
    }
    memset(&door_receipt, 0xff, sizeof(door_receipt));
    if (dm2_v1_runtime_last_door_scene_ownership_receipt(&door_receipt) !=
            0 ||
        door_receipt.valid != 0 || door_receipt.map_load_token != 0u ||
        door_receipt.scene_control_hash != 0u) {
        fprintf(stderr, "FAIL: absent scene record must block door ownership\n");
        return 1;
    }
    memset(&selection, 0xff, sizeof(selection));
    if (dm2_v1_runtime_scene_frame_selection_receipt(&selection) != 0 ||
        selection.valid != 0 || selection.map_load_token != 0u ||
        selection.scene_control_hash != 0u || selection.dungeon_frame_selected != 0 ||
        selection.hud_frame_selected != 0) {
        fprintf(stderr, "FAIL: absent scene record must block frame selection\n");
        return 1;
    }
    memset(&selection, 0, sizeof(selection));
    selection.valid = 1;
    selection.map_load_token = 42u;
    selection.scene_control_hash = 0x53434e45u;
    selection.dungeon_frame_selected = 1;
    selection.hud_frame_selected = 1;
    selection.door_selected = 1;
    selection.material_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    selection.graphicsset = 3u;
    selection.door_frame_field = DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT;
    memset(&gate, 0, sizeof(gate));
    gate.valid = 1;
    gate.original_material_required = 1;
    gate.active_graphicsset_frame = 1;
    gate.decoded_format_gate_ready = 1;
    gate.scene_record_ready = 1;
    gate.palette_ready = 1;
    gate.colorkey_ready = 1;
    gate.accepted = 1;
    if (dm2_v1_runtime_scene_frame_selection_apply_original_material_gate(
            &selection, &gate, &gated_selection) != 1 ||
        !gated_selection.valid ||
        !gated_selection.door_material_gate_present ||
        !gated_selection.door_material_gate_accepted) {
        fprintf(stderr, "FAIL: accepted source gate must reach frame plan\n");
        return 1;
    }
    gate.accepted = 0;
    if (dm2_v1_runtime_scene_frame_selection_apply_original_material_gate(
            &selection, &gate, &gated_selection) != 0 ||
        gated_selection.valid != 0 ||
        !gated_selection.door_material_gate_present ||
        gated_selection.door_material_gate_accepted != 0) {
        fprintf(stderr, "FAIL: rejected source gate must block frame plan\n");
        return 1;
    }
    memset(&surface_request, 0, sizeof(surface_request));
    surface_request.valid = 1;
    surface_request.gdat_category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    surface_request.graphicsset = 3u;
    surface_request.field = DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT;
    surface_request.gdat_index =
        dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
            3, DM2_SQ_D0C);
    surface_request.map_load_token = 42u;
    surface_request.scene_control_hash = 0x53434e45u;
    surface_request.palette_hash = 0x50414c31u;
    surface_request.scene_colorkey = 6u;
    surface_request.decoded_pixels = decoded_u4;
    surface_request.palette16 = palette16;
    surface_request.width = 2;
    surface_request.height = 2;
    surface_request.stride = 2;
    surface_request.format = DM2_IMG_FMT_U4;
    memset(&surface_binding, 0, sizeof(surface_binding));
    surface_binding.valid = 1;
    surface_binding.request = surface_request;
    surface_binding.blit.gdat_index = surface_request.gdat_index;
    surface_binding.blit.src_rect.x = 0;
    surface_binding.blit.src_rect.y = 0;
    surface_binding.blit.src_rect.w = 2;
    surface_binding.blit.src_rect.h = 2;
    surface_binding.blit.src_stride = 2;
    surface_binding.blit.transparent_color = 6;
    surface_binding.palette_stride = 16;
    surface_binding.colorkey_palette_index = palette16[6];
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_surface_binding(
            &selection, &surface_binding, &gated_selection) != 1 ||
        !gated_selection.valid || !gated_selection.door_surface_request_ready) {
        fprintf(stderr, "FAIL: matching U4 surface binding must bind frame plan\n");
        return 1;
    }
    memset(&opening_request, 0, sizeof(opening_request));
    opening_request.valid = 1;
    opening_request.view_square = DM2_SQ_D0C;
    opening_request.skproject_cell =
        dm2_v1_viewport_skproject_cell_for_square(DM2_SQ_D0C);
    opening_request.door_state = 2u;
    opening_request.door_open_pct = 50u;
    opening_request.opening_visible_rect =
        (DM2_V1_ViewportRect){ 80, 60, 40, 20 };
    opening_request.frame_rect =
        (DM2_V1_ViewportRect){ 64, 48, 80, 64 };
    opening_request.source_rect = surface_binding.blit.src_rect;
    opening_request.destination_rect = opening_request.frame_rect;
    opening_request.material = surface_binding;
    opening_request.material.blit.dst_rect = opening_request.frame_rect;
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_opening_frame_request(
            &selection, &opening_request, &gated_selection) != 1 ||
        !gated_selection.valid ||
        !gated_selection.door_opening_frame_request_ready) {
        fprintf(stderr, "FAIL: matching opening frame must bind frame plan\n");
        return 1;
    }
    opening_request.destination_rect.x++;
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_opening_frame_request(
            &selection, &opening_request, &gated_selection) != 0 ||
        gated_selection.valid != 0 ||
        gated_selection.door_opening_frame_request_ready != 0) {
        fprintf(stderr, "FAIL: altered opening-frame destination must not bind\n");
        return 1;
    }
    opening_request.destination_rect.x--;
    surface_binding.request.scene_control_hash = 0x5354414cu;
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_surface_binding(
            &selection, &surface_binding, &gated_selection) != 0 ||
        gated_selection.valid != 0 ||
        gated_selection.door_surface_request_ready != 0) {
        fprintf(stderr, "FAIL: stale surface binding must not bind frame plan\n");
        return 1;
    }
    surface_binding.request.scene_control_hash = 0x53434e45u;
    surface_binding.request.map_load_token = 43u;
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_surface_binding(
            &selection, &surface_binding, &gated_selection) != 0 ||
        gated_selection.valid != 0) {
        fprintf(stderr, "FAIL: stale map token must not bind frame plan\n");
        return 1;
    }
    surface_binding.request.map_load_token = 42u;
    surface_binding.palette_stride = 15;
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_surface_binding(
            &selection, &surface_binding, &gated_selection) != 0 ||
        gated_selection.valid != 0) {
        fprintf(stderr, "FAIL: malformed palette stride must not bind frame plan\n");
        return 1;
    }
    surface_binding.palette_stride = 16;
    surface_binding.blit.src_stride = 1;
    if (dm2_v1_runtime_scene_frame_selection_bind_original_door_surface_binding(
            &selection, &surface_binding, &gated_selection) != 0 ||
        gated_selection.valid != 0) {
        fprintf(stderr, "FAIL: malformed U4 geometry must not bind frame plan\n");
        return 1;
    }
    fprintf(stderr, "PASS: runtime scene record fails closed when absent\n");
    return 0;
}
