#include "nexus_v1_engine.h"
#include "nexus_v1_launcher.h"

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

int main(void)
{
    Nexus_V1_DgnRendererHandoffReceipt receipt;
    Nexus_V1_DgnRenderPlanReceipt plan;

    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH;
    receipt.can_render_dgn_mesh = 1;
    receipt.blocks_real_dgn_mesh_render = 0;
    nexus_v1_dgn_renderer_handoff_require_canonical_source(&receipt, 1);
    expect(receipt.canonical_source_verified &&
               receipt.status == NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH &&
               receipt.can_render_dgn_mesh &&
               !receipt.blocks_real_dgn_mesh_render,
           "canonical package bytes preserve an otherwise no-fallback handoff");

    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH;
    receipt.can_render_dgn_mesh = 1;
    receipt.fallback_visuals_permitted = 1;
    nexus_v1_dgn_renderer_handoff_require_canonical_source(&receipt, 0);
    expect(!receipt.canonical_source_verified &&
               receipt.status ==
                   NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_CANONICAL_SOURCE &&
               !receipt.can_render_dgn_mesh &&
               receipt.blocks_real_dgn_mesh_render &&
               !receipt.fallback_visuals_permitted,
           "noncanonical bytes block face and mesh provenance without fallback");
    expect(strcmp(nexus_v1_dgn_renderer_handoff_status_name(receipt.status),
                  "blocked-canonical-source") == 0,
           "canonical-source rejection has a stable host status");

    memset(&plan, 0, sizeof(plan));
    plan.status = NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
    plan.blocks_real_dgn_mesh_render = 1;
    plan.structure2_source_materialization_bound = 1;
    plan.structure2_vdp1_palette_binding_proven = 0;
    plan.fallback_visuals_permitted = 0;
    expect(strcmp(nexus_v1_launcher_dgn_visual_blocker_from_render_plan(&plan),
                  "blocked-structure2-vdp1-palette") == 0 &&
               !plan.fallback_visuals_permitted,
           "Structure2 materialization stays blocked until VDP1/palette proof exists");
    plan.structure2_vdp1_palette_binding_proven = 1;
    expect(strcmp(nexus_v1_launcher_dgn_visual_blocker_from_render_plan(&plan),
                  "blocked-structure2-source") == 0,
           "Structure2 VDP1/palette proof bit is required for the specific blocker name");

    memset(&plan, 0, sizeof(plan));
    plan.status = NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS;
    plan.blocks_real_dgn_mesh_render = 1;
    plan.structure1f_plan_item_entry_count = 1;
    plan.structure1f_plan_item_floor_command_count = 1;
    plan.item_ibs_vdp1_command_proven = 0;
    expect(strcmp(nexus_v1_launcher_dgn_visual_blocker_from_render_plan(&plan),
                  "blocked-item-ibs-vdp1-provenance") == 0 &&
               !plan.fallback_visuals_permitted,
           "ITEM.IBS floor material evidence names the missing VDP1 provenance blocker");
    plan.item_ibs_vdp1_command_proven = 1;
    expect(strcmp(nexus_v1_launcher_dgn_visual_blocker_from_render_plan(&plan),
                  "blocked-structure1f-semantics") == 0,
           "ITEM.IBS command proof bit is required for the specific blocker name");

    if (failures) {
        fprintf(stderr, "Nexus DGN source-provenance gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN source-provenance gate passed");
    return 0;
}
