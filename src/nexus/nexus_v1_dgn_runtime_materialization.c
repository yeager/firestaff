#include "nexus_v1_dgn_runtime_materialization.h"

#include <string.h>

static void set_blocked(Nexus_V1_DgnRuntimeMaterializationReceipt *receipt,
                        Nexus_V1_DgnRuntimeMaterializationStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->blocks_real_dgn_mesh_render = 1;
    receipt->no_draw_only = 1;
    receipt->original_render_capture_required = 1;
}

static int mesh_ready(const Nexus_V1_DgnMesh *mesh)
{
    return mesh &&
           mesh->status == NEXUS_V1_DGN_MESH_READY_GEOMETRY &&
           mesh->canonical_source_verified &&
           mesh->face_count > 0 &&
           mesh->can_submit_geometry &&
           !mesh->can_submit_textured_raster &&
           mesh->material_provenance_required &&
           mesh->textured_raster_blocked &&
           mesh->vdp1_provenance_required &&
           mesh->vdp1_draw_list_blocked &&
           !mesh->permits_fallback_visuals;
}

static int material_ready(const Nexus_V1_DgnFaceMaterialReceipt *material,
                          const Nexus_V1_DgnPackageHostConsumerReceipt *host)
{
    return material &&
           host &&
           material->status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
           material->canonical_source_verified &&
           material->reopened_bytes_match_canonical &&
           material->structure3_mesh_materials_bound &&
           material->structure2_descriptor_route_bound &&
           material->selector_bindings_complete &&
           !material->material_semantics_proven &&
           material->package_host_route_bound &&
           material->no_draw_only &&
           material->blocks_real_dgn_mesh_render &&
           !material->permits_fallback_visuals &&
           material->original_saturn_capture_required &&
           !material->original_saturn_capture_available &&
           !material->can_submit_raster_input &&
           host->status == NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW &&
           host->material_receipt_ready &&
           host->source_route_consumed_by_host &&
           host->real_dgn_source_consumed_by_host &&
           host->structure2_structure3_admission_bound &&
           host->package_host_route_bound &&
           host->material_pixel_promotion_blocked &&
           !host->can_submit_raster_input &&
           !host->fallback_visuals_permitted &&
           host->blocks_real_dgn_mesh_render &&
           host->no_draw_only;
}

static int prs3_ready(const Nexus_V1_DgnMenuPrs3RouteReceipt *prs3)
{
    return prs3 &&
           prs3->route_proof_bound &&
           prs3->dgn_package_host_bound &&
           prs3->prs3_output_upload_bound &&
           prs3->prs3_decoded_output_proof_bound &&
           prs3->prs3_decoded_output_sidecar_bound &&
           prs3->prs3_reviewed_upload_path_bound &&
           prs3->prs3_menu_bpk_upload_reviewed &&
           prs3->prs3_original_saturn_provenance_verified &&
           prs3->prs3_independent_authentication_required &&
           prs3->prs3_source_bound_no_runtime &&
           prs3->prs3_stream_size > 0U &&
           prs3->prs3_expected_output_bytes > 0U &&
           prs3->prs3_output_fnv1a64 != 0U &&
           !prs3->original_saturn_capture_authenticated &&
           !prs3->runtime_dgn_render_permitted &&
           !prs3->startup_menu_render_permitted &&
           prs3->material_pixel_promotion_blocked &&
           prs3->prs3_runtime_upload_blocked &&
           !prs3->fallback_visuals_permitted &&
           prs3->no_draw_only &&
           prs3->blocks_real_dgn_mesh_render;
}

static int structure1f_ready(
    const Nexus_V1_DgnStructure1FItemMaterialReceipt *item,
    const Nexus_V1_DgnCommandPacked4BppMaterialReceipt *packed)
{
    return item &&
           packed &&
           item->source_hash_verified &&
           item->item_entry_count > 0 &&
           item->command_candidate_count > 0 &&
           item->complete &&
           !item->fallback_visuals_permitted &&
           packed->source_hash_verified &&
           packed->special_floor_binding_count > 0 &&
           packed->source_cell_match_count > 0 &&
           packed->command_material_count > 0 &&
           packed->complete &&
           packed->blocked_missing_vdp1_command_provenance_count > 0 &&
           packed->original_vdp1_capture_verified_count == 0 &&
           !packed->fallback_visuals_permitted;
}

int nexus_v1_dgn_runtime_materialization_admit(
    const Nexus_V1_DgnRuntimeMaterializationInput *input,
    Nexus_V1_DgnRuntimeMaterializationReceipt *out_receipt)
{
    const Nexus_V1_DgnMesh *mesh;
    const Nexus_V1_DgnFaceMaterialReceipt *material;
    const Nexus_V1_DgnPackageHostConsumerReceipt *host;
    const Nexus_V1_DgnMenuPrs3RouteReceipt *prs3;
    const Nexus_V1_DgnStructure1FItemMaterialReceipt *structure1f;
    const Nexus_V1_DgnCommandPacked4BppMaterialReceipt *packed;

    if (!out_receipt) return 0;
    set_blocked(out_receipt,
                NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_INPUT);
    if (!input) return 0;

    mesh = input->mesh;
    material = input->face_material;
    host = input->package_host;
    prs3 = input->prs3_route;
    structure1f = input->structure1f_item_material;
    packed = input->structure1f_packed4bpp;

    if (!mesh_ready(mesh)) {
        out_receipt->status =
            NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MESH;
        return 0;
    }
    out_receipt->mesh_plan_bound = 1;
    out_receipt->mesh_face_count = mesh->face_count;
    out_receipt->mesh_textured_face_count =
        mesh->static_texture_face_count + mesh->animated_texture_face_count;

    if (!material_ready(material, host)) {
        out_receipt->status =
            NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL;
        return 0;
    }
    out_receipt->face_material_plan_bound = 1;
    out_receipt->package_host_route_bound = 1;
    out_receipt->static_selector_count = material->static_selector_count;
    out_receipt->animated_selector_count = material->animated_selector_count;

    if (!prs3_ready(prs3) ||
        !input->bpk_source_verified ||
        !input->bpk_material_plan_bound ||
        !input->bpk_palette_plan_bound ||
        input->bpk_surface_count <= 0 ||
        input->bpk_prs3_surface_count <= 0) {
        out_receipt->status =
            NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_BPK_PRS3;
        return 0;
    }
    out_receipt->bpk_prs3_plan_bound = 1;
    out_receipt->palette_plan_bound = 1;
    out_receipt->bpk_surface_count = input->bpk_surface_count;
    out_receipt->bpk_prs3_surface_count = input->bpk_prs3_surface_count;

    if (!structure1f_ready(structure1f, packed)) {
        out_receipt->status =
            NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_STRUCTURE1F;
        return 0;
    }
    out_receipt->structure1f_plan_bound = 1;
    out_receipt->structure1f_item_entry_count =
        structure1f->item_entry_count;
    out_receipt->structure1f_command_material_count =
        packed->command_material_count;

    if (!input->m11_host_route_requested ||
        !input->m11_host_route_consumed ||
        !input->m11_host_route_package_consumed ||
        !input->m11_host_route_blocks_runtime ||
        input->m11_capture_ready ||
        input->m11_frame_hash != 0U) {
        out_receipt->status =
            NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_HOST_ROUTE;
        return 0;
    }

    /* No input field here authenticates the original Saturn VDP1/VDP2 frame,
     * palette state or final pixels.  A source-complete no-draw chain must
     * therefore stop at the original-render boundary; it may never turn
     * itself into a READY_NO_DRAW presentation receipt. */
    out_receipt->status =
        NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_ORIGINAL_RENDER;
    out_receipt->source_bound = 1;
    out_receipt->m11_host_route_bound = 1;
    out_receipt->runtime_consumed_by_m11_host = 1;
    out_receipt->can_present_runtime_dgn = 0;
    out_receipt->blocks_real_dgn_mesh_render = 1;
    out_receipt->no_draw_only = 1;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->original_render_capture_required = 1;
    out_receipt->original_render_capture_authenticated = 0;
    out_receipt->material_semantics_proven = 0;
    out_receipt->palette_semantics_proven = 0;
    out_receipt->texel_order_proven = 0;
    out_receipt->vdp1_command_proven = 0;
    out_receipt->m11_frame_hash = 0U;
    return 0;
}

const char *nexus_v1_dgn_runtime_materialization_status_name(
    Nexus_V1_DgnRuntimeMaterializationStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_READY_NO_DRAW:
        return "ready-no-draw";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MESH:
        return "blocked-mesh";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL:
        return "blocked-material";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_BPK_PRS3:
        return "blocked-bpk-prs3";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_STRUCTURE1F:
        return "blocked-structure1f";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_HOST_ROUTE:
        return "blocked-host-route";
    case NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_ORIGINAL_RENDER:
        return "blocked-original-render";
    }
    return "blocked-input";
}
