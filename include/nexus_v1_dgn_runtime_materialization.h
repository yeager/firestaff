#ifndef NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_H
#define NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_H

#include <stdint.h>

#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_dgn_mesh.h"
#include "nexus_v1_dungeon.h"

typedef enum {
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_READY_NO_DRAW = 0,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_INPUT,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MESH,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_MATERIAL,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_BPK_PRS3,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_STRUCTURE1F,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_HOST_ROUTE,
    NEXUS_V1_DGN_RUNTIME_MATERIALIZATION_BLOCKED_ORIGINAL_RENDER
} Nexus_V1_DgnRuntimeMaterializationStatus;

typedef struct {
    const Nexus_V1_DgnMesh *mesh;
    const Nexus_V1_DgnFaceMaterialReceipt *face_material;
    const Nexus_V1_DgnPackageHostConsumerReceipt *package_host;
    const Nexus_V1_DgnMenuPrs3RouteReceipt *prs3_route;
    const Nexus_V1_DgnStructure1FItemMaterialReceipt *structure1f_item_material;
    const Nexus_V1_DgnCommandPacked4BppMaterialReceipt *structure1f_packed4bpp;
    int bpk_source_verified;
    int bpk_material_plan_bound;
    int bpk_palette_plan_bound;
    int bpk_surface_count;
    int bpk_prs3_surface_count;
    int m11_host_route_requested;
    int m11_host_route_consumed;
    int m11_host_route_package_consumed;
    int m11_host_route_blocks_runtime;
    int m11_capture_ready;
    uint32_t m11_frame_hash;
} Nexus_V1_DgnRuntimeMaterializationInput;

typedef struct {
    Nexus_V1_DgnRuntimeMaterializationStatus status;
    int source_bound;
    int mesh_plan_bound;
    int face_material_plan_bound;
    int bpk_prs3_plan_bound;
    int structure1f_plan_bound;
    int palette_plan_bound;
    int package_host_route_bound;
    int m11_host_route_bound;
    int runtime_consumed_by_m11_host;
    int can_present_runtime_dgn;
    int blocks_real_dgn_mesh_render;
    int no_draw_only;
    int fallback_visuals_permitted;
    int original_render_capture_required;
    int original_render_capture_authenticated;
    int material_semantics_proven;
    int palette_semantics_proven;
    int texel_order_proven;
    int vdp1_command_proven;
    int mesh_face_count;
    int mesh_textured_face_count;
    int static_selector_count;
    int animated_selector_count;
    int structure1f_item_entry_count;
    int structure1f_command_material_count;
    int bpk_surface_count;
    int bpk_prs3_surface_count;
    uint32_t m11_frame_hash;
} Nexus_V1_DgnRuntimeMaterializationReceipt;

int nexus_v1_dgn_runtime_materialization_admit(
    const Nexus_V1_DgnRuntimeMaterializationInput *input,
    Nexus_V1_DgnRuntimeMaterializationReceipt *out_receipt);

const char *nexus_v1_dgn_runtime_materialization_status_name(
    Nexus_V1_DgnRuntimeMaterializationStatus status);

#endif
