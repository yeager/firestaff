#include "nexus_v1_structure1f_corpus_capture_plan.h"

#include <stdio.h>
#include <string.h>

static void set_fixture(Nexus_V1_Structure1FCorpusCapturePlanInput *input)
{
    uint32_t i;
    memset(input, 0, sizeof(*input));
    input->expected_package_fnv1a64 = input->observed_package_fnv1a64 = 0x1001U;
    input->expected_package_size = input->observed_package_size = 0x2000U;
    for (i = 0; i < NEXUS_V1_STRUCTURE1F_CORPUS_LEVEL_COUNT; ++i) {
        Nexus_V1_Structure1FCorpusLevelInput *level = &input->levels[i];
        level->level_index = i;
        level->expected_dgn_fnv1a64 = level->observed_dgn_fnv1a64 = 0x10000U + i;
        level->expected_descriptor_index = level->observed_descriptor_index = i + 3U;
        level->expected_descriptor_fnv1a64 = level->observed_descriptor_fnv1a64 = 0x20000U + i;
        level->expected_mesh_index = level->observed_mesh_index = i + 5U;
        level->expected_mesh_fnv1a64 = level->observed_mesh_fnv1a64 = 0x30000U + i;
        level->expected_face_index = level->observed_face_index = i + 7U;
        level->expected_face_fnv1a64 = level->observed_face_fnv1a64 = 0x40000U + i;
    }
}

static int rejects_mutation(Nexus_V1_Structure1FCorpusCapturePlanInput *input)
{
    Nexus_V1_Structure1FCorpusCapturePlan plan;
    return nexus_v1_structure1f_corpus_capture_plan_build(input, &plan) ||
        plan.valid || !plan.no_draw_only || !plan.blocks_real_dgn_mesh_render ||
        plan.fallback_visuals_permitted;
}

int main(void)
{
    Nexus_V1_Structure1FCorpusCapturePlanInput input;
    Nexus_V1_Structure1FCorpusCapturePlan plan;

    set_fixture(&input);
    if (!nexus_v1_structure1f_corpus_capture_plan_build(&input, &plan) ||
        !plan.valid || plan.package_fnv1a64 != 0x1001U ||
        plan.targets[0].level_index != 0 || plan.targets[15].level_index != 15 ||
        plan.targets[15].dgn_fnv1a64 != 0x1000fU ||
        !plan.targets[8].original_saturn_trace_required ||
        !plan.targets[8].no_draw_only || plan.targets[8].fallback_visuals_permitted ||
        !plan.targets[8].blocks_real_dgn_mesh_render) return 1;

    input.observed_package_fnv1a64++;
    if (rejects_mutation(&input)) return 1;
    set_fixture(&input);
    input.levels[4].observed_dgn_fnv1a64++;
    if (rejects_mutation(&input)) return 1;
    set_fixture(&input);
    input.levels[4].observed_descriptor_fnv1a64++;
    if (rejects_mutation(&input)) return 1;
    set_fixture(&input);
    input.levels[4].observed_mesh_fnv1a64++;
    if (rejects_mutation(&input)) return 1;
    set_fixture(&input);
    input.levels[4].observed_face_fnv1a64++;
    if (rejects_mutation(&input)) return 1;
    set_fixture(&input);
    input.levels[4].level_index = 5;
    if (rejects_mutation(&input)) return 1;
    set_fixture(&input);
    input.levels[4].observed_dgn_fnv1a64 = input.levels[3].observed_dgn_fnv1a64;
    input.levels[4].expected_dgn_fnv1a64 = input.levels[4].observed_dgn_fnv1a64;
    if (rejects_mutation(&input)) return 1;

    puts("structure1f corpus capture plan: PASS");
    return 0;
}
