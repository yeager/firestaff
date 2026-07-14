#include "nexus_v1_engine.h"

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

    if (failures) {
        fprintf(stderr, "Nexus DGN source-provenance gate: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN source-provenance gate passed");
    return 0;
}
