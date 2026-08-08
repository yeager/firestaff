#include "nexus_v1_dgn_runtime_materialization.h"
#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void build_ready_chain(
    Nexus_V1_DgnMesh *mesh,
    Nexus_V1_DgnFaceMaterialReceipt *material,
    Nexus_V1_DgnPackageHostConsumerReceipt *host,
    Nexus_V1_DgnMenuPrs3RouteReceipt *prs3,
    Nexus_V1_DgnStructure1FItemMaterialReceipt *structure1f,
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt *packed,
    Nexus_V1_DgnRuntimeMaterializationInput *input)
{
    static const uint8_t retail_dgn[] = {0x44, 0x47, 0x4e, 0x11};
    static const Nexus_V1_DgnMeshFixedVertex vertices[] = {
        {0, 0, 0}, {65536, 0, 0}, {65536, 65536, 0}, {0, 65536, 0}
    };
    Nexus_V1_DgnMeshSourceFace faces[] = {
        {{0, 1, 2, 2}, 0x40U, 0x0012U},
        {{0, 2, 3, 1}, 0x40U, 0x0801U}
    };
    Nexus_V1_DgnMeshInput mesh_input;
    Nexus_V1_DgnFaceMaterialBinding bindings[] = {
        {0, 1, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC},
        {1, 1, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED}
    };
    Nexus_V1_DgnFaceMaterialInput material_input;
    Nexus_V1_DgnPackageHostConsumerInput host_input;
    Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt prs3_upload;
    Nexus_V1_DgnMenuPrs3RouteInput route_input;

    memset(&mesh_input, 0, sizeof(mesh_input));
    mesh_input.vertices = vertices;
    mesh_input.vertex_count = 4;
    mesh_input.faces = faces;
    mesh_input.face_count = 2;
    mesh_input.canonical_source_verified = 1;
    mesh_input.topology_receipt_valid = 1;
    mesh_input.fixed_point_vectors_valid = 1;
    expect(nexus_v1_dgn_mesh_build(&mesh_input, mesh) == 1,
           "source-verified Structure3 mesh builds");

    memset(&material_input, 0, sizeof(material_input));
    material_input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    material_input.dgn_bytes = retail_dgn;
    material_input.dgn_size = (int)sizeof(retail_dgn);
    material_input.canonical_dgn_bytes = retail_dgn;
    material_input.canonical_dgn_size = (int)sizeof(retail_dgn);
    material_input.canonical_source_verified = 1;
    material_input.bindings = bindings;
    material_input.face_count = 2;
    material_input.structure2_descriptor_count = 2;
    material_input.material_selector_count = 2;
    material_input.geometry_source_bound = 1;
    material_input.geometry_material_face_count = 2;
    material_input.geometry_can_submit_geometry = 1;
    material_input.geometry_can_submit_textured_raster = 0;
    material_input.geometry_fallback_visuals_permitted = 0;
    expect(nexus_v1_dgn_face_material_validate(&material_input, material) == 1,
           "retail DGN face/material binding validates no-draw");

    memset(&host_input, 0, sizeof(host_input));
    host_input.material_receipt = material;
    host_input.host_route_requested = 1;
    host_input.package_route_consumed = 1;
    host_input.expected_level_index = 0;
    host_input.observed_level_index = 0;
    host_input.expected_canonical_dgn_size = (int)sizeof(retail_dgn);
    host_input.observed_canonical_dgn_size = (int)sizeof(retail_dgn);
    host_input.expected_face_count = 2;
    host_input.observed_face_count = 2;
    host_input.expected_structure2_descriptor_count = 2;
    host_input.observed_structure2_descriptor_count = 2;
    expect(nexus_v1_dgn_package_host_consumer_gate(&host_input, host) == 0 &&
               host->status == NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL,
           "package host consumer gate now blocks on the material receipt's no-draw flags");

    memset(&prs3_upload, 0, sizeof(prs3_upload));
    prs3_upload.entry_index = 5U;
    prs3_upload.stream_offset = 0x240U;
    prs3_upload.stream_size = 0x44U;
    prs3_upload.expected_output_bytes = 0x90U;
    prs3_upload.output_fnv1a64 = 0x33445566778899aaULL;
    prs3_upload.decoded_output_proof_bound = 1;
    prs3_upload.decoded_output_sidecar_bound = 1;
    prs3_upload.reviewed_upload_path_bound = 1;
    prs3_upload.menu_bpk_upload_reviewed = 1;
    prs3_upload.original_saturn_provenance_verified = 1;
    prs3_upload.independent_authentication_required = 1;
    prs3_upload.source_bound_no_runtime = 1;
    memset(&route_input, 0, sizeof(route_input));
    route_input.dgn_host = host;
    route_input.prs3_output_upload = &prs3_upload;
    route_input.startup_route_requested = 1;
    route_input.dgn_route_requested = 1;
    expect(nexus_v1_dgn_menu_prs3_route_gate(&route_input, prs3) == 0 &&
               !prs3->route_proof_bound,
           "PRS3 route proof stays unbound because the DGN package host route never reaches ready-no-draw");

    memset(structure1f, 0, sizeof(*structure1f));
    structure1f->source_hash_verified = 1;
    structure1f->item_entry_count = 1;
    structure1f->command_candidate_count = 1;
    structure1f->bound_special_floor_palette_count = 1;
    structure1f->bound_special_floor_texture_count = 1;
    structure1f->complete = 1;
    memset(packed, 0, sizeof(*packed));
    packed->source_hash_verified = 1;
    packed->special_floor_binding_count = 1;
    packed->source_cell_match_count = 1;
    packed->command_material_count = 1;
    packed->blocked_missing_vdp1_command_provenance_count = 1;
    packed->complete = 1;

    memset(input, 0, sizeof(*input));
    input->mesh = mesh;
    input->face_material = material;
    input->package_host = host;
    input->prs3_route = prs3;
    input->structure1f_item_material = structure1f;
    input->structure1f_packed4bpp = packed;
    input->bpk_source_verified = 1;
    input->bpk_material_plan_bound = 1;
    input->bpk_palette_plan_bound = 1;
    input->bpk_surface_count = 3;
    input->bpk_prs3_surface_count = 1;
    input->m11_host_route_requested = 1;
    input->m11_host_route_consumed = 1;
    input->m11_host_route_package_consumed = 1;
    input->m11_host_route_blocks_runtime = 1;
}

static void test_runtime_materialization_gate(void)
{
    Nexus_V1_DgnMesh mesh;
    Nexus_V1_DgnFaceMaterialReceipt material;
    Nexus_V1_DgnPackageHostConsumerReceipt host;
    Nexus_V1_DgnMenuPrs3RouteReceipt prs3;
    Nexus_V1_DgnStructure1FItemMaterialReceipt structure1f;
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt packed;
    Nexus_V1_DgnRuntimeMaterializationInput input;
    Nexus_V1_DgnRuntimeMaterializationReceipt receipt;

    build_ready_chain(&mesh, &material, &host, &prs3, &structure1f, &packed,
                      &input);
    /* The package-host consumer gate and the PRS3 route gate never reach
     * their ready/route-proof-bound states now (see
     * nexus_v1_dgn_face_material_provenance.c), so material_ready() inside
     * this admit() can never see host->status == READY_NO_DRAW. The chain
     * therefore always stops at BLOCKED_MATERIAL even though the mesh stage
     * still validates cleanly. */
    expect(nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) == 0 &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL &&
               receipt.mesh_plan_bound &&
               !receipt.face_material_plan_bound &&
               !receipt.bpk_prs3_plan_bound &&
               !receipt.structure1f_plan_bound &&
               !receipt.palette_plan_bound &&
               !receipt.package_host_route_bound &&
               !receipt.m11_host_route_bound &&
               !receipt.runtime_consumed_by_m11_host &&
               !receipt.can_present_runtime_dgn &&
               receipt.blocks_real_dgn_mesh_render &&
               receipt.no_draw_only &&
               !receipt.fallback_visuals_permitted &&
               receipt.original_render_capture_required &&
               !receipt.original_render_capture_authenticated &&
               !receipt.material_semantics_proven &&
               !receipt.palette_semantics_proven &&
               !receipt.texel_order_proven &&
               !receipt.vdp1_command_proven &&
               receipt.mesh_face_count == 2 &&
               receipt.mesh_textured_face_count == 2 &&
               receipt.m11_frame_hash == 0U,
           "real DGN/BPK/PRS3/Structure1F path stops at the material no-draw boundary");

    mesh.canonical_source_verified = 0;
    expect(!nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MESH &&
               receipt.blocks_real_dgn_mesh_render &&
               receipt.no_draw_only,
           "unverified mesh source blocks runtime materialization");
    mesh.canonical_source_verified = 1;

    prs3.route_proof_bound = 0;
    expect(!nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL &&
               !receipt.can_present_runtime_dgn &&
               !receipt.fallback_visuals_permitted,
           "missing PRS3 proof still leaves the chain blocked at the material stage");
    prs3.route_proof_bound = 1;

    packed.blocked_missing_vdp1_command_provenance_count = 0;
    expect(!nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL,
           "Structure1F provenance changes are unreachable while the material stage blocks");
    packed.blocked_missing_vdp1_command_provenance_count = 1;

    input.m11_host_route_blocks_runtime = 0;
    expect(!nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL &&
               receipt.blocks_real_dgn_mesh_render,
           "M11 host route changes are unreachable while the material stage blocks");
    input.m11_host_route_blocks_runtime = 1;
    input.m11_capture_ready = 1;
    input.m11_frame_hash = 0x1234U;
    expect(!nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL &&
               receipt.m11_frame_hash == 0U,
           "unproved original route still cannot publish a captured frame hash");

    /* Exercise the old false-positive exit directly: even a caller-supplied
     * ready-looking no-draw chain has no Saturn frame witness in this ABI. */
    material.no_draw_only = 1;
    material.blocks_real_dgn_mesh_render = 1;
    material.permits_fallback_visuals = 0;
    material.package_host_route_bound = 1;
    input.bpk_source_verified = 1;
    input.m11_capture_ready = 0;
    input.m11_frame_hash = 0U;
    host.status = NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW;
    host.material_receipt_ready = 1;
    host.source_route_consumed_by_host = 1;
    host.real_dgn_source_consumed_by_host = 1;
    host.structure2_structure3_admission_bound = 1;
    host.package_host_route_bound = 1;
    host.material_pixel_promotion_blocked = 1;
    host.no_draw_only = 1;
    host.blocks_real_dgn_mesh_render = 1;
    host.fallback_visuals_permitted = 0;
    prs3.dgn_package_host_bound = 1;
    prs3.prs3_output_upload_bound = 1;
    prs3.prs3_decoded_output_proof_bound = 1;
    prs3.prs3_decoded_output_sidecar_bound = 1;
    prs3.prs3_reviewed_upload_path_bound = 1;
    prs3.prs3_menu_bpk_upload_reviewed = 1;
    prs3.prs3_original_saturn_provenance_verified = 1;
    prs3.prs3_independent_authentication_required = 1;
    prs3.prs3_source_bound_no_runtime = 1;
    prs3.material_pixel_promotion_blocked = 1;
    prs3.prs3_runtime_upload_blocked = 1;
    prs3.no_draw_only = 1;
    prs3.blocks_real_dgn_mesh_render = 1;
    prs3.fallback_visuals_permitted = 0;
    prs3.runtime_dgn_render_permitted = 0;
    prs3.startup_menu_render_permitted = 0;
    prs3.prs3_stream_size = 1U;
    prs3.prs3_expected_output_bytes = 1U;
    prs3.prs3_output_fnv1a64 = 1U;
    expect(!nexus_v1_dgn_runtime_materialization_admit(&input, &receipt) &&
               receipt.status ==
                   NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_ORIGINAL_RENDER &&
               receipt.source_bound && receipt.blocks_real_dgn_mesh_render &&
               receipt.no_draw_only && !receipt.can_present_runtime_dgn &&
               !receipt.fallback_visuals_permitted &&
               receipt.original_render_capture_required &&
               !receipt.original_render_capture_authenticated &&
               !receipt.material_semantics_proven &&
               !receipt.palette_semantics_proven,
           "source-complete DGN chain remains capture-gated at original render");
}

int main(void)
{
    test_runtime_materialization_gate();
    if (failures) return 1;
    puts("Nexus DGN runtime materialization passed");
    return 0;
}
