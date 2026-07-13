#include "dm1_v1_melee_action_f0402_pc34_compat.h"
#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int expect(int condition, const char* message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

static char* read_source(const char* path)
{
    FILE* file;
    long size;
    char* text;

    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    text = (char*)malloc((size_t)size + 1u);
    if (!text) {
        fclose(file);
        return NULL;
    }
    if (fread(text, 1u, (size_t)size, file) != (size_t)size) {
        free(text);
        fclose(file);
        return NULL;
    }
    text[size] = '\0';
    fclose(file);
    return text;
}

static int audit_m11_live_explosion_handoff(void)
{
    char* text = read_source("src/engine/m11_game_view.c");
    int ok = 1;

    if (!text) {
        fprintf(stderr, "could not read current M11 source\n");
        return 0;
    }
    ok &= expect(strstr(text, "input.liveExplosions = &state->world.explosions;") != NULL,
                 "M11 samples the live M10 explosion list");
    ok &= expect(strstr(text,
                         "dm1_v1_viewport_runtime_materialization_decide_pc34(&input,") != NULL,
                 "M11 passes live effects to the F0115 materialization receipt");
    ok &= expect(strstr(text, "m11_viewport_cell_explosion_material(") != NULL,
                 "M11 extracts each admitted F0115 explosion receipt");
    ok &= expect(strstr(text, "static int m11_draw_explosion_material(") != NULL,
                 "M11 consumes explosion material through its existing drawer");
    ok &= expect(strstr(text, "m11_draw_explosion_cue(") == NULL,
                 "M11 has no dedicated explosion cue fallback");
    free(text);
    return ok;
}

int main(void)
{
    DM1_MeleeF0231AftermathInputPc34 aftermathInput;
    DM1_MeleeF0231AftermathPlanPc34 aftermathPlan;
    DM1_MeleeF0231AftermathApplyPlanPc34 aftermathApply;
    DM1_MeleeF0190KilledAllAfterplayReceiptPc34 afterplay;
    DM1_V1_ViewportRuntimeMaterializationInputPc34 viewportInput;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 viewportDecision;
    struct ExplosionList_Compat explosions;
    struct TimelineEvent_Compat advance;
    int createdSlot = -1;
    int ok = 1;

    memset(&aftermathInput, 0, sizeof(aftermathInput));
    aftermathInput.groupIndex = 4;
    aftermathInput.creatureIndex = 2;
    aftermathInput.creatureType = 6;
    aftermathInput.creatureProperties = 0x0230;
    aftermathInput.groupBehavior = DM1_BEHAVIOR_ATTACK;
    aftermathInput.originalGroupCount = 3;
    aftermathInput.partyMapIndex = 3;
    aftermathInput.partyMapX = 7;
    aftermathInput.partyMapY = 9;
    aftermathInput.targetMapIndex = 3;
    aftermathInput.targetMapX = 8;
    aftermathInput.targetMapY = 9;
    aftermathInput.currentTick = 77u;
    aftermathInput.creatureAttributes = DM1_SIZE_FULL_SQUARE;
    aftermathInput.killedCell = EXPLOSION_CELL_CENTERED;
    aftermathInput.damageOutcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    aftermathInput.fallbackCombatOutcome = COMBAT_OUTCOME_HIT_DAMAGE;

    ok &= expect(dm1_v1_melee_aftermath_plan_f0231_pc34(
                     &aftermathInput, &aftermathPlan),
                 "F0231 builds the killed-all aftermath");
    ok &= expect(dm1_v1_melee_aftermath_apply_plan_f0231_pc34(
                     &aftermathPlan, &aftermathApply),
                 "F0231 builds the M10 apply receipt");
    ok &= expect(dm1_v1_melee_killed_all_afterplay_receipt_f0190_pc34(
                     &aftermathApply, &afterplay),
                 "F0190 builds the source-smoke afterplay receipt");
    ok &= expect(afterplay.shouldPresentSourceSmoke &&
                     afterplay.requiresKilledAllMutationFirst &&
                     afterplay.sourceSmokeCreateInput.explosionType ==
                         C040_EXPLOSION_SMOKE &&
                     afterplay.sourceSmokeCreateInput.attack == 255,
                 "F0190 retains only the original full-square C040 smoke");

    memset(&explosions, 0, sizeof(explosions));
    memset(&advance, 0, sizeof(advance));
    ok &= expect(F0821_EXPLOSION_Create_Compat(
                     &afterplay.sourceSmokeCreateInput, &explosions,
                     &createdSlot, &advance),
                 "M10 materializes the afterplay C040 into the live list");
    ok &= expect(createdSlot >= 0 && explosions.count == 1 &&
                     explosions.entries[createdSlot].reserved0 &&
                     explosions.entries[createdSlot].explosionType ==
                         C040_EXPLOSION_SMOKE &&
                     explosions.entries[createdSlot].attack == 255 &&
                     explosions.entries[createdSlot].mapIndex == 3 &&
                     explosions.entries[createdSlot].mapX == 8 &&
                     explosions.entries[createdSlot].mapY == 9,
                 "live C040 preserves the F0190 source record");

    memset(&viewportInput, 0, sizeof(viewportInput));
    viewportInput.relativeForward = 1;
    viewportInput.elementType = 1;
    viewportInput.mapIndex = 3;
    viewportInput.mapX = 8;
    viewportInput.mapY = 9;
    viewportInput.runtimeOrigin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
    viewportInput.liveExplosions = &explosions;
    ok &= expect(dm1_v1_viewport_runtime_materialization_decide_pc34(
                     &viewportInput, &viewportDecision),
                 "F0115 materialization receipt builds from the live list");
    ok &= expect(viewportDecision.liveExplosionSourceCount == 1 &&
                     viewportDecision.liveExplosionSourceSlots[0] == createdSlot &&
                     viewportDecision.liveExplosionSourceTypes[0] ==
                         C040_EXPLOSION_SMOKE &&
                     viewportDecision.liveExplosionSourceRoutes[0] ==
                         DM1_V1_C15_EXPLOSION_ROUTE_ORDINARY_F0114_PC34 &&
                     viewportDecision.liveRenderableExplosionCount == 1 &&
                     viewportDecision.liveRenderableExplosionTypes[0] ==
                         C040_EXPLOSION_SMOKE &&
                     viewportDecision.liveRenderableExplosionFrames[0] ==
                         explosions.entries[createdSlot].currentFrame &&
                     viewportDecision.liveRenderableExplosionAttacks[0] == 255,
                 "C040 reaches M11's existing F0115 material boundary unchanged");
    ok &= audit_m11_live_explosion_handoff();

    if (!ok) return 1;
    puts("ok: F0190 C040 M10-to-M11 integration audit");
    return 0;
}
