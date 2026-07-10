#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int tests;
static int passes;

#define CHECK(condition, message) do { \
    ++tests; \
    if (condition) ++passes; else printf("FAIL: %s\n", message); \
} while (0)

static DM1_V1_ViewportRuntimeMaterializationInputPc34 base_input(int origin)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.relativeForward = 1;
    input.relativeSide = 0;
    input.elementType = 1;
    input.floorItemCount = 1;
    input.projectileCount = 1;
    input.projectileCell = 2;
    input.runtimeOrigin = (DM1_V1_ViewportRuntimeOriginPc34)origin;
    return input;
}

int main(void)
{
    int origin;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 d1c;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 side;

    for (origin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
         origin <= DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34;
         ++origin) {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(origin);
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "D1C runtime decision is built");
        CHECK(d1c.valid && d1c.consumedF0172SquareFacts &&
              d1c.consumedF0115ThingPass && d1c.noM11Fallback,
              "new-start and resume paths receive a DM1-owned receipt");
        CHECK(d1c.drawFloorItems && d1c.drawRuntimeProjectiles &&
              d1c.drawDeferredSpellEffects,
              "ordinary D1C F0115 layers remain independently materialized");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34);
        input.hasVisibleChampionMirrorPayload = 1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "D1C mirror decision is built after original-save load");
        CHECK(d1c.suppressMaterializedItemPayload &&
              d1c.suppressMirrorAsFloorItem && d1c.suppressMirrorAsProjectile &&
              d1c.suppressMirrorAsSpellEffect && !d1c.drawDeferredSpellEffects,
              "D1C mirror remains a wall overlay across save provenance");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34);
        input.relativeForward = 2;
        input.relativeSide = -1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &side),
              "remaining visible side zone receives a decision");
        CHECK(side.valid && side.drawFloorItems && side.drawRuntimeProjectiles &&
              side.drawDeferredSpellEffects && !side.suppressMaterializedItemPayload,
              "D2L uses its own F0115 materialization rather than D1C fallback");
    }

    printf("%d/%d passed\n", passes, tests);
    return passes == tests ? 0 : 1;
}
