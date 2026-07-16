#ifndef NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_H
#define NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_H

#include <stdint.h>

#include "nexus_v1_dgn_mesh.h"
#include "nexus_v1_dungeon.h"

#define NEXUS_V1_DGN_SCENE_PLAN_MAX_COMMANDS \
    NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS
#define NEXUS_V1_DGN_SCENE_PLAN_MAX_OWNED_SOURCES 128
#define NEXUS_V1_DGN_SCENE_PLAN_MAX_TOPOLOGY_CANDIDATES 128

typedef enum {
    NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_READY_GEOMETRY_NO_DRAW = 0,
    NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_INPUT = 1,
    NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_VIEW_PLAN = 2,
    NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_STRUCTURE1F = 3,
    NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_ENTRY = 4,
    NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_BUILD = 5
} Nexus_V1_DgnSceneRuntimePlanStatus;

typedef struct {
    const Nexus_V1_Level *level;
    const uint8_t *dgn_data;
    int dgn_size;
    int level_index;
    int dgn_source_hash_verified;
    int party_x;
    int party_y;
    int party_dir;
    int vdp1_consumer_evidence_available;
} Nexus_V1_DgnSceneRuntimePlanInput;

typedef struct {
    Nexus_V1_DgnSceneRuntimePlanStatus status;
    int source_bound;
    int level_index;
    int party_x;
    int party_y;
    int party_dir;
    int forward_x;
    int forward_y;
    int left_x;
    int left_y;
    int right_x;
    int right_y;
    int party_cell_bound;
    int forward_cell_bound;
    int side_cells_bound;
    int view_command_count;
    int floor_command_count;
    int wall_command_count;
    int ceiling_command_count;
    int source_cell_count;
    int structure1f_visible_owned_entry_count;
    int structure1f_owned_source_count;
    int topology_candidate_count;
    int selected_structure1f_entry_index;
    int selected_owner_x;
    int selected_owner_y;
    int selected_structure1a_index;
    int selected_structure3_model_index;
    int selected_face_ordinal;
    int selected_z_rotation;
    int structure1f_face_selector_bound;
    int structure1a_model_rotation_bound;
    int face_ordinal_within_model_bound;
    int mesh_entry_bound;
    int mesh_vertex_count;
    int mesh_face_count;
    int mesh_normal_count;
    int mesh_triangle_count;
    int mesh_quad_count;
    int mesh_color_fill_count;
    int mesh_static_texture_face_count;
    int mesh_animated_texture_face_count;
    int material_index_bound;
    uint16_t selected_face_fill_selector;
    uint8_t selected_face_flags;
    Nexus_V1_DgnMeshFillKind selected_face_fill_kind;
    int geometry_consumer_ready;
    int texture_submit_blocked;
    int raster_submit_blocked;
    int m11_runtime_handoff_permitted;
    int fallback_geometry_permitted;
    int fallback_visuals_permitted;
    int no_draw_only;
} Nexus_V1_DgnSceneRuntimePlanReceipt;

int nexus_v1_dgn_scene_runtime_plan_build(
    const Nexus_V1_DgnSceneRuntimePlanInput *input,
    Nexus_V1_DgnSceneRuntimePlanReceipt *out_receipt);

const char *nexus_v1_dgn_scene_runtime_plan_status_name(
    Nexus_V1_DgnSceneRuntimePlanStatus status);

#endif
