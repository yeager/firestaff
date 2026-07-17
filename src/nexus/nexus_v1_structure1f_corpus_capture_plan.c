#include "nexus_v1_structure1f_corpus_capture_plan.h"

#include <string.h>

static int level_is_exact(const Nexus_V1_Structure1FCorpusLevelInput *level,
                          uint32_t index)
{
    return level && level->level_index == index && level->expected_dgn_fnv1a64 &&
        level->expected_dgn_fnv1a64 == level->observed_dgn_fnv1a64 &&
        level->expected_descriptor_index == level->observed_descriptor_index &&
        level->expected_descriptor_fnv1a64 &&
        level->expected_descriptor_fnv1a64 == level->observed_descriptor_fnv1a64 &&
        level->expected_mesh_index == level->observed_mesh_index &&
        level->expected_mesh_fnv1a64 &&
        level->expected_mesh_fnv1a64 == level->observed_mesh_fnv1a64 &&
        level->expected_face_index == level->observed_face_index &&
        level->expected_face_fnv1a64 &&
        level->expected_face_fnv1a64 == level->observed_face_fnv1a64;
}

int nexus_v1_structure1f_corpus_capture_plan_build(
    const Nexus_V1_Structure1FCorpusCapturePlanInput *input,
    Nexus_V1_Structure1FCorpusCapturePlan *out_plan)
{
    Nexus_V1_Structure1FCorpusCapturePlan plan;
    uint32_t i;
    uint32_t j;

    if (!out_plan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.no_draw_only = 1;
    plan.blocks_real_dgn_mesh_render = 1;
    if (!input || !input->expected_package_fnv1a64 ||
        input->expected_package_fnv1a64 != input->observed_package_fnv1a64 ||
        !input->expected_package_size ||
        input->expected_package_size != input->observed_package_size) {
        *out_plan = plan;
        return 0;
    }
    for (i = 0; i < NEXUS_V1_STRUCTURE1F_CORPUS_LEVEL_COUNT; ++i) {
        const Nexus_V1_Structure1FCorpusLevelInput *level = &input->levels[i];
        Nexus_V1_Structure1FCorpusTraceTarget *target = &plan.targets[i];
        if (!level_is_exact(level, i)) {
            *out_plan = plan;
            return 0;
        }
        for (j = 0; j < i; ++j) {
            if (input->levels[j].observed_dgn_fnv1a64 ==
                level->observed_dgn_fnv1a64) {
                *out_plan = plan;
                return 0;
            }
        }
        target->valid = 1;
        target->level_index = i;
        target->package_fnv1a64 = input->observed_package_fnv1a64;
        target->package_size = input->observed_package_size;
        target->dgn_fnv1a64 = level->observed_dgn_fnv1a64;
        target->descriptor_index = level->observed_descriptor_index;
        target->descriptor_fnv1a64 = level->observed_descriptor_fnv1a64;
        target->mesh_index = level->observed_mesh_index;
        target->mesh_fnv1a64 = level->observed_mesh_fnv1a64;
        target->face_index = level->observed_face_index;
        target->face_fnv1a64 = level->observed_face_fnv1a64;
        target->original_saturn_trace_required = 1;
        target->no_draw_only = 1;
        target->blocks_real_dgn_mesh_render = 1;
    }
    plan.valid = 1;
    plan.package_fnv1a64 = input->observed_package_fnv1a64;
    plan.package_size = input->observed_package_size;
    *out_plan = plan;
    return 1;
}
