#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"

#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <string.h>

const char *dm1_v1_viewport_runtime_materialization_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0172 lines 2466-2589 publishes each visible "
           "square aspect; DUNVIEW.C F0115 lines 4547-4581 separates the "
           "thing passes, line 4923 clips non-visible cells, line 5075 "
           "selects C2500 object zones, lines 5668-5683 select C2900 "
           "projectile zones, and lines 5915-5933 restart for deferred "
           "explosions. DUNVIEW.C lines 3913-3928 reserve the D1C champion "
           "mirror for the wall-overlay route.";
}

int dm1_v1_viewport_runtime_materialization_decide_pc34(
    const DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 *outDecision)
{
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 decision;
    int itemX;
    int itemY;
    int projectileX;
    int projectileY;
    int isD1c;

    if (!input || !outDecision) {
        return 0;
    }
    memset(&decision, 0, sizeof(decision));
    decision.viewSquare = dm1_viewport_3d_f0115_view_square_index(
        input->relativeForward, input->relativeSide);
    decision.row = dm1_viewport_3d_f0115_c2500_c2900_row(
        input->relativeForward, input->relativeSide);
    decision.itemZone = -1;
    decision.projectileZone = -1;
    decision.noM11Fallback = 1;
    decision.sourceAnchor = dm1_v1_viewport_runtime_materialization_source_evidence_pc34();

    /* The receipt is reconstructed from F0172/F0115 facts on every draw.
     * Save provenance never changes the source routing. */
    if (input->runtimeOrigin < DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34 ||
        input->runtimeOrigin > DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34 ||
        decision.viewSquare < 0 || decision.row < 0) {
        *outDecision = decision;
        return 1;
    }

    decision.valid = 1;
    decision.consumedF0172SquareFacts = 1;
    decision.consumedF0115ThingPass = 1;
    isD1c = input->relativeForward == 1 && input->relativeSide == 0;

    if (input->floorItemCount > 0 && input->elementType != DM1_VP_ELEMENT_WALL &&
        dm1_viewport_3d_c2500_object_raw_zone_point(decision.row, 2,
                                                     &itemX, &itemY)) {
        (void)itemX;
        (void)itemY;
        decision.itemZone = 2500 + decision.row * 4 + 2;
        decision.drawFloorItems = 1;
    }
    if (input->projectileCount > 0 &&
        dm1_viewport_3d_c2900_projectile_raw_zone_point(
            decision.row, input->projectileCell, &projectileX, &projectileY)) {
        (void)projectileX;
        (void)projectileY;
        decision.projectileZone = 2900 + decision.row * 4 + input->projectileCell;
        decision.drawRuntimeProjectiles = 1;
    }

    /* C127/C026 is a D1C wall overlay, never an F0115 item, projectile or
     * materialized spell payload. Independent real things still use F0115. */
    if (isD1c && input->hasVisibleChampionMirrorPayload) {
        decision.suppressMaterializedItemPayload = 1;
        decision.suppressMirrorAsFloorItem = 1;
        decision.suppressMirrorAsProjectile = 1;
        decision.suppressMirrorAsSpellEffect = 1;
    }
    decision.drawDeferredSpellEffects =
        decision.suppressMirrorAsSpellEffect ? 0 : 1;
    *outDecision = decision;
    return 1;
}
