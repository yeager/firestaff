#include "nexus_v1_engine.h"
#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    Nexus_V1_Engine engine;
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportRenderReceipt render;

    memset(&engine, 0, sizeof(engine));
    engine.level_loaded = 1;
    engine.game.current_level = 0;
    engine.game.party_x = 11;
    engine.game.party_y = 29;
    engine.current_level.geometry_info.dmweb_container = 1;

    if (nexus_v1_prepare_dgn_material_plan(&engine, 11, 29, 0) != NULL ||
        engine.dgn_material_plan.receipt.status ==
            NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH ||
        engine.dgn_material_plan.receipt.plan_ready ||
        engine.dgn_material_plan.receipt.command_count != 0 ||
        engine.dgn_material_plan.receipt.structure2_source_materialization_bound ||
        engine.dgn_material_plan.receipt.fallback_visuals_permitted) {
        fprintf(stderr, "Incomplete DGN data did not produce a no-draw plan\n");
        return 1;
    }

    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, &engine);
    if (nexus_viewport_last_dgn_render_receipt(&viewport, &render) != 0 ||
        !render.attempted || !render.used_real_dgn_route || !render.blocked ||
        render.no_draw_structure2_source || render.command_count != 0 ||
        render.fallback_visuals_permitted) {
        fprintf(stderr, "Incomplete DGN data did not produce a no-draw viewport receipt\n");
        return 1;
    }

    printf("Nexus incomplete-data no-draw receipt: OK\n");
    return 0;
}
