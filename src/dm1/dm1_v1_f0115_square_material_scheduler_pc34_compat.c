#include "dm1_v1_f0115_square_material_scheduler_pc34_compat.h"

#include <string.h>

static int is_f0115_step(const DM1_V1_F0128SchedulerStepPc34 *step)
{
    return step && (step->op == DM1_V1_F0128_STEP_F0115_EARLY ||
        step->op == DM1_V1_F0128_STEP_F0115_MAIN ||
        step->op == DM1_V1_F0128_STEP_F0115_DOOR_PASS1 ||
        step->op == DM1_V1_F0128_STEP_F0115_DOOR_PASS2);
}

static int source_material_is_admitted(const DM1_V1_F0115SquareMaterialPc34 *m)
{
    return m && m->square >= 0 &&
        m->square < DM1_V1_F0128_VIEW_SQUARE_COUNT &&
        m->kind >= DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34 &&
        m->kind <= DM1_V1_F0115_MATERIAL_EXPLOSION_PC34 &&
        m->graphicIndex > 0 && m->pixels && m->width > 0 && m->height > 0 &&
        m->transparentColor == 10;
}

const char *dm1_v1_f0115_square_material_scheduler_source_pc34(void)
{
    return "ReDMCSB DUNVIEW.C F0115:4547-4581 walks normal objects; "
           "F0115:5645 restarts for projectiles; F0115:5916 restarts for "
           "explosions. F0128:8479-8542 supplies the outer square order.";
}

int dm1_v1_f0115_square_material_schedule_pc34(
    const DM1_V1_F0128SchedulerPlanPc34 *plan,
    const DM1_V1_F0115SquareMaterialPc34 *materials,
    int materialCount,
    DM1_V1_F0115SquareMaterialSchedulePc34 *outSchedule)
{
    int stepIndex;
    int kind;
    int i;
    DM1_V1_F0115SquareMaterialSchedulePc34 result;

    if (!outSchedule) return 0;
    memset(outSchedule, 0, sizeof(*outSchedule));
    if (!plan || !materials || materialCount < 0 ||
        materialCount > DM1_V1_F0115_SQUARE_MATERIAL_MAX_PC34 ||
        !DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(plan)) return 0;

    memset(&result, 0, sizeof(result));
    for (stepIndex = 0; stepIndex < plan->stepCount; ++stepIndex) {
        const DM1_V1_F0128SchedulerStepPc34 *step = &plan->steps[stepIndex];
        if (!is_f0115_step(step)) continue;
        for (kind = DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34;
             kind <= DM1_V1_F0115_MATERIAL_EXPLOSION_PC34; ++kind) {
            for (i = 0; i < materialCount; ++i) {
                const DM1_V1_F0115SquareMaterialPc34 *m = &materials[i];
                if (m->square != step->square || m->kind != kind) continue;
                if (!source_material_is_admitted(m) ||
                    result.materialCount >= DM1_V1_F0115_SQUARE_MATERIAL_MAX_PC34) {
                    return 0;
                }
                result.materials[result.materialCount++] = *m;
            }
        }
    }
    /* Reject material for squares not reached by F0115 rather than drawing it
     * through a nearby square or a synthetic fallback. */
    for (i = 0; i < materialCount; ++i) {
        int found = 0;
        for (stepIndex = 0; stepIndex < plan->stepCount; ++stepIndex) {
            if (is_f0115_step(&plan->steps[stepIndex]) &&
                plan->steps[stepIndex].square == materials[i].square) {
                found = 1;
                break;
            }
        }
        if (!found || !source_material_is_admitted(&materials[i])) return 0;
    }
    result.valid = 1;
    *outSchedule = result;
    return 1;
}
