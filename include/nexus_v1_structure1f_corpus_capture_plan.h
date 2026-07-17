#ifndef NEXUS_V1_STRUCTURE1F_CORPUS_CAPTURE_PLAN_H
#define NEXUS_V1_STRUCTURE1F_CORPUS_CAPTURE_PLAN_H

#include <stdint.h>

#define NEXUS_V1_STRUCTURE1F_CORPUS_LEVEL_COUNT 16U

/* One opaque Structure1F/Structure3 candidate declaration for a level. The
 * verifier compares every observed field to its independently declared value;
 * none of these bytes are interpreted as geometry, material, or pixels. */
typedef struct {
    uint32_t level_index;
    uint64_t expected_dgn_fnv1a64;
    uint64_t observed_dgn_fnv1a64;
    uint32_t expected_descriptor_index;
    uint32_t observed_descriptor_index;
    uint64_t expected_descriptor_fnv1a64;
    uint64_t observed_descriptor_fnv1a64;
    uint32_t expected_mesh_index;
    uint32_t observed_mesh_index;
    uint64_t expected_mesh_fnv1a64;
    uint64_t observed_mesh_fnv1a64;
    uint32_t expected_face_index;
    uint32_t observed_face_index;
    uint64_t expected_face_fnv1a64;
    uint64_t observed_face_fnv1a64;
} Nexus_V1_Structure1FCorpusLevelInput;

typedef struct {
    uint64_t expected_package_fnv1a64;
    uint64_t observed_package_fnv1a64;
    uint64_t expected_package_size;
    uint64_t observed_package_size;
    Nexus_V1_Structure1FCorpusLevelInput levels[NEXUS_V1_STRUCTURE1F_CORPUS_LEVEL_COUNT];
} Nexus_V1_Structure1FCorpusCapturePlanInput;

/* An immutable original-Saturn capture request. It is deliberately no-draw
 * and carries only exact source identities plus descriptor/mesh/face labels. */
typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint64_t package_size;
    uint64_t dgn_fnv1a64;
    uint32_t descriptor_index;
    uint64_t descriptor_fnv1a64;
    uint32_t mesh_index;
    uint64_t mesh_fnv1a64;
    uint32_t face_index;
    uint64_t face_fnv1a64;
    int original_saturn_trace_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_Structure1FCorpusTraceTarget;

typedef struct {
    int valid;
    uint64_t package_fnv1a64;
    uint64_t package_size;
    Nexus_V1_Structure1FCorpusTraceTarget targets[NEXUS_V1_STRUCTURE1F_CORPUS_LEVEL_COUNT];
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_Structure1FCorpusCapturePlan;

/* Requires all LEV00..LEV15 rows. Any absent, cross-level, duplicate-DGN, or
 * expected/observed identity drift rejects the whole plan. */
int nexus_v1_structure1f_corpus_capture_plan_build(
    const Nexus_V1_Structure1FCorpusCapturePlanInput *input,
    Nexus_V1_Structure1FCorpusCapturePlan *out_plan);

#endif
