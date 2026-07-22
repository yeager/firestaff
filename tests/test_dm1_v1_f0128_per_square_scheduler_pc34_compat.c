/*
 * test_dm1_v1_f0128_per_square_scheduler_pc34_compat.c
 *
 * Contract test for the DM1 V1 F0128 complete per-square source
 * scheduler (merged F0104/F0107/F0108/F0111/F0113 material families
 * with real field-after-things and door/object occlusion capture).
 *
 * Source-locked per ReDMCSB DUNVIEW.C F0128:8318-8561,
 * F0676/F0677:6226-6360, F0678/F0679:6837-6899, F0116-F0127:6361-8317,
 * cell-order words DEFS.H:2658-2677. Data-free contract test; no game
 * data, no pixels, no host substitutes.
 */
#include "dm1_v1_f0128_per_square_scheduler_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, long got, long want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%ld want=%ld anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %ld anchor=%s\n", id, want, anchor);
    }
}

static void expect_str(const char *id, const char *got, const char *want,
                       const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=%s want=%s anchor=%s\n", id,
               got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %s anchor=%s\n", id, want, anchor);
    }
}

static void all_corridor(DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT])
{
    int i;
    for (i = 0; i < DM1_V1_F0128_VIEW_SQUARE_COUNT; i++) {
        squares[i].element = DM1_V1_F0128_ELEMENT_CORRIDOR;
        squares[i].pitOrTeleporterVisible = 0;
        squares[i].frontWallOrnamentIsAlcove = 0;
        squares[i].hasFloorOrnament = 0;
    }
}

static int count_op_for_square(const DM1_V1_F0128SchedulerPlanPc34 *plan,
                               int square, int op)
{
    int i, n = 0;
    for (i = 0; i < plan->stepCount; i++) {
        if (plan->steps[i].square == square && plan->steps[i].op == op) {
            n++;
        }
    }
    return n;
}

static int first_index(const DM1_V1_F0128SchedulerPlanPc34 *plan,
                       int square, int op)
{
    int i;
    for (i = 0; i < plan->stepCount; i++) {
        if (plan->steps[i].square == square && plan->steps[i].op == op) {
            return i;
        }
    }
    return -1;
}

typedef struct {
    DM1_V1_F0128SchedulerStepPc34 preflighted[DM1_V1_F0128_SCHEDULER_MAX_STEPS];
    DM1_V1_F0128SchedulerStepPc34 executed[DM1_V1_F0128_SCHEDULER_MAX_STEPS];
    int preflightCount;
    int executeCount;
    int rejectAt;
} DispatchTrace;

static int trace_preflight(void *context,
                           const DM1_V1_F0128SchedulerStepPc34 *step)
{
    DispatchTrace *trace = (DispatchTrace *)context;
    int index = trace->preflightCount++;
    trace->preflighted[index] = *step;
    return index != trace->rejectAt;
}

static void trace_execute(void *context,
                          const DM1_V1_F0128SchedulerStepPc34 *step)
{
    DispatchTrace *trace = (DispatchTrace *)context;
    trace->executed[trace->executeCount++] = *step;
}

static void test_class_table(void)
{
    const DM1_V1_F0128SquareClassPc34 *d4l;
    const DM1_V1_F0128SquareClassPc34 *d3l2;
    const DM1_V1_F0128SquareClassPc34 *d3r2;
    const DM1_V1_F0128SquareClassPc34 *d2l2;
    const DM1_V1_F0128SquareClassPc34 *d1l;
    const DM1_V1_F0128SquareClassPc34 *d1r;
    const DM1_V1_F0128SquareClassPc34 *d1c;
    const DM1_V1_F0128SquareClassPc34 *d0c;

    DM1_V1_F0128_PerSquareSchedulerInitPc34Compat();

    expect_int("class.count",
               (long)DM1_V1_F0128_PerSquareSchedulerClassCountPc34Compat(),
               19, "ReDMCSB DUNVIEW.C:8479-8542 19 view squares D4L..D0C");
    expect_int("class.at_oob.null",
               DM1_V1_F0128_PerSquareSchedulerClassAtPc34Compat(19) == NULL,
               1, "contract-only accessor bounds");
    expect_int("class.for.neg.null",
               DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(-1) == NULL,
               1, "contract-only accessor bounds");

    d4l = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D4L);
    expect_int("d4l.early", d4l ? d4l->earlyOnly : 0, 1,
               "ReDMCSB DUNVIEW.C:8479-8481 early D4 F0115");
    expect_int("d4l.order", d4l ? d4l->corridorOrder : 0, 0x0001,
               "ReDMCSB DEFS.H:2659 C0x0001_CELL_ORDER_BACKLEFT");

    d3l2 = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D3L2);
    expect_str("d3l2.name", d3l2 ? d3l2->name : NULL, "D3L2",
               "ReDMCSB DUNVIEW.C:6226 F0676_DrawD3L2");
    expect_int("d3l2.pass1", d3l2 ? d3l2->doorPass1Order : 0, 0x0218,
               "ReDMCSB DEFS.H:2669 C0x0218 DOORPASS1_BACKLEFT_BACKRIGHT");
    expect_int("d3l2.pass2", d3l2 ? d3l2->doorPass2Order : 0, 0x0349,
               "ReDMCSB DEFS.H:2672 C0x0349 DOORPASS2_FRONTLEFT_FRONTRIGHT");
    expect_int("d3l2.side", d3l2 ? d3l2->doorSideOrder : 0, 0x0321,
               "ReDMCSB DUNVIEW.C:6267 C0x0321 door-side order");

    d3r2 = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D3R2);
    expect_int("d3r2.pass1", d3r2 ? d3r2->doorPass1Order : 0, 0x0128,
               "ReDMCSB DEFS.H:2668 C0x0128 DOORPASS1_BACKRIGHT_BACKLEFT");
    expect_int("d3r2.pass2", d3r2 ? d3r2->doorPass2Order : 0, 0x0439,
               "ReDMCSB DEFS.H:2675 C0x0439 DOORPASS2_FRONTRIGHT_FRONTLEFT");
    expect_int("d3r2.corridor", d3r2 ? d3r2->corridorOrder : 0, 0x4312,
               "ReDMCSB DEFS.H:2677 C0x4312 right-lane corridor order");

    d2l2 = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D2L2);
    expect_int("d2l2.no_thing_pass", d2l2 ? d2l2->noThingPass : 0, 1,
               "ReDMCSB DUNVIEW.C:6843 L2487_i_Order_Unreferenced");
    expect_int("d2l2.field", d2l2 ? d2l2->fieldCapable : 0, 1,
               "ReDMCSB DUNVIEW.C:6862-6865 F0678 teleporter F0113");

    d1l = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D1L);
    expect_int("d1l.pass1", d1l ? d1l->doorPass1Order : 0, 0x0028,
               "ReDMCSB DEFS.H:2663 C0x0028 DOORPASS1_BACKRIGHT");
    expect_int("d1l.pass2", d1l ? d1l->doorPass2Order : 0, 0x0039,
               "ReDMCSB DEFS.H:2665 C0x0039 DOORPASS2_FRONTRIGHT");
    expect_int("d1l.main", d1l ? d1l->corridorOrder : 0, 0x0032,
               "ReDMCSB DEFS.H:2664 C0x0032 BACKRIGHT_FRONTRIGHT");

    d1r = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D1R);
    expect_int("d1r.pass1", d1r ? d1r->doorPass1Order : 0, 0x0018,
               "ReDMCSB DEFS.H:2661 C0x0018 DOORPASS1_BACKLEFT");
    expect_int("d1r.main", d1r ? d1r->corridorOrder : 0, 0x0041,
               "ReDMCSB DEFS.H:2666 C0x0041 BACKLEFT_FRONTLEFT");

    d1c = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D1C);
    expect_int("d1c.pass1", d1c ? d1c->doorPass1Order : 0, 0x0218,
               "ReDMCSB DUNVIEW.C:7875 F0124 D1C door pass1");

    d0c = DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(
        DM1_V1_F0128_VIEW_SQUARE_D0C);
    expect_int("d0c.main", d0c ? d0c->corridorOrder : 0, 0x0021,
               "ReDMCSB DUNVIEW.C:8232 C0x0021 BACKLEFT_BACKRIGHT");
    expect_int("d0c.field", d0c ? d0c->fieldCapable : 0, 1,
               "ReDMCSB DUNVIEW.C:8315-8317 F0127 D0C F0113 field");

    expect_int("contract.mentions_f0128",
               strstr(DM1_V1_F0128_PerSquareSchedulerSourceContractPc34Compat(),
                      "F0128") != NULL,
               1, "ReDMCSB DUNVIEW.C F0128 anchor present");
    expect_int("contract.mentions_field_after_things",
               strstr(DM1_V1_F0128_PerSquareSchedulerSourceContractPc34Compat(),
                      "field-after-things") != NULL,
               1, "field-after-things contract marker");
}

static void test_corridor_scene_visit_order(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    int i;
    int lastSquare = -1;
    int orderOk = 1;

    all_corridor(squares);
    expect_int("build.corridor.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "all-corridor 19-square view builds");
    expect_int("verify.corridor.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "all-corridor plan passes invariants");

    for (i = 0; i < plan.stepCount; i++) {
        if (plan.steps[i].square < lastSquare) {
            orderOk = 0;
        }
        lastSquare = plan.steps[i].square;
    }
    expect_int("corridor.visit_order", orderOk, 1,
               "ReDMCSB DUNVIEW.C:8491-8542 source square visit order");
    expect_int("corridor.early.count",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D4L,
                                   DM1_V1_F0128_STEP_F0115_EARLY) +
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D4R,
                                   DM1_V1_F0128_STEP_F0115_EARLY) +
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D4C,
                                   DM1_V1_F0128_STEP_F0115_EARLY),
               3, "ReDMCSB DUNVIEW.C:8479-8490 three early D4 passes");
    expect_int("corridor.d3l.main_order",
               plan.steps[first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                                      DM1_V1_F0128_STEP_F0115_MAIN)].cellOrderWord,
               0x3421, "ReDMCSB DEFS.H:2676 C0x3421 left/center corridor order");
    expect_int("corridor.d3r.main_order",
               plan.steps[first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D3R,
                                      DM1_V1_F0128_STEP_F0115_MAIN)].cellOrderWord,
               0x4312, "ReDMCSB DEFS.H:2677 C0x4312 right corridor order");
    expect_int("corridor.d0l.main_order",
               plan.steps[first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D0L,
                                      DM1_V1_F0128_STEP_F0115_MAIN)].cellOrderWord,
               0x0002, "ReDMCSB DUNVIEW.C:8005 C0x0002_CELL_ORDER_BACKRIGHT");
    expect_int("corridor.d0c.main_order",
               plan.steps[first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D0C,
                                      DM1_V1_F0128_STEP_F0115_MAIN)].cellOrderWord,
               0x0021, "ReDMCSB DUNVIEW.C:8232 C0x0021 BACKLEFT_BACKRIGHT");
    expect_int("corridor.no_field",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D2C,
                                   DM1_V1_F0128_STEP_F0113_FIELD) +
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D0C,
                                   DM1_V1_F0128_STEP_F0113_FIELD),
               0, "no F0113 without a teleporter");
}

static void test_field_after_things(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    int mainIdx, fieldIdx;

    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].element = DM1_V1_F0128_ELEMENT_TELEPORTER;
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].pitOrTeleporterVisible = 1;
    expect_int("build.teleporter.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "visible D2C teleporter builds");
    expect_int("verify.teleporter.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "visible D2C teleporter passes invariants");

    mainIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D2C,
                          DM1_V1_F0128_STEP_F0115_MAIN);
    fieldIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D2C,
                           DM1_V1_F0128_STEP_F0113_FIELD);
    expect_int("teleporter.main.present", mainIdx >= 0, 1,
               "ReDMCSB DUNVIEW.C:7316-7321 D2C thing pass");
    expect_int("teleporter.field.present", fieldIdx >= 0, 1,
               "ReDMCSB DUNVIEW.C:7380-7389 D2C F0113 field");
    expect_int("teleporter.field_after_things", fieldIdx > mainIdx, 1,
               "ReDMCSB DUNVIEW.C:6289/6487 field after things");
    expect_int("teleporter.field.flag",
               plan.steps[fieldIdx].fieldAfterThings, 1,
               "field-after-things capture flag");

    /* Invisible teleporter: no field, things still pass. */
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].pitOrTeleporterVisible = 0;
    expect_int("build.teleporter_hidden.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "invisible D2C teleporter builds");
    expect_int("teleporter_hidden.no_field",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D2C,
                                   DM1_V1_F0128_STEP_F0113_FIELD),
               0, "ReDMCSB DUNVIEW.C:7376-7379 visibility gate");

    /* Hand-broken plan: field moved before the things pass must be
     * rejected by the verifier (fail-closed). */
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].pitOrTeleporterVisible = 1;
    DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan);
    mainIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D2C,
                          DM1_V1_F0128_STEP_F0115_MAIN);
    fieldIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D2C,
                           DM1_V1_F0128_STEP_F0113_FIELD);
    {
        DM1_V1_F0128SchedulerStepPc34 tmp = plan.steps[mainIdx];
        plan.steps[mainIdx] = plan.steps[fieldIdx];
        plan.steps[fieldIdx] = tmp;
    }
    expect_int("verify.field_first.rejected",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               0, "fail-closed: field before things violates source order");
}

static void test_door_occlusion_capture(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    int ornIdx, pass1Idx, frameIdx, doorIdx, pass2Idx;

    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element = DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].hasFloorOrnament = 1;
    expect_int("build.door.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D1C door-front view builds");
    expect_int("verify.door.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "D1C door-front plan passes invariants");

    ornIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                         DM1_V1_F0128_STEP_F0108_FLOOR_ORNAMENT);
    pass1Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS1);
    frameIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                           DM1_V1_F0128_STEP_F0104_DOOR_FRAME);
    doorIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                          DM1_V1_F0128_STEP_F0111_DOOR);
    pass2Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS2);

    expect_int("door.floor_ornament.first", ornIdx >= 0 && ornIdx < pass1Idx, 1,
               "ReDMCSB DUNVIEW.C:7873-7874 F0108 before back things");
    expect_int("door.pass1_order",
               plan.steps[pass1Idx].cellOrderWord, 0x0218,
               "ReDMCSB DUNVIEW.C:7875 C0x0218 D1C door pass1");
    expect_int("door.pass1.behind", plan.steps[pass1Idx].behindDoor, 1,
               "ReDMCSB DUNVIEW.C:7875 back cells behind the door");
    expect_int("door.frame_between", frameIdx > pass1Idx && frameIdx < doorIdx, 1,
               "ReDMCSB DUNVIEW.C:7877-7910 door frame between pass1 and door");
    expect_int("door.f0111.occluder", plan.steps[doorIdx].occluder, 1,
               "ReDMCSB DUNVIEW.C:7877-7910 F0111 door occludes pass1");
    expect_int("door.pass2_order",
               plan.steps[pass2Idx].cellOrderWord, 0x0349,
               "ReDMCSB DEFS.H:2672 C0x0349 door pass2");
    expect_int("door.pass2.after_door", pass2Idx > doorIdx, 1,
               "ReDMCSB DUNVIEW.C:7910-7937 front things after the door");
    expect_int("door.pass2.not_behind", plan.steps[pass2Idx].behindDoor, 0,
               "pass2 front cells are not door-occluded");

    /* Right-lane door uses the mirrored order words. */
    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D2R].element = DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    expect_int("build.door_d2r.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D2R door-front view builds");
    pass1Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D2R,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS1);
    pass2Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D2R,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS2);
    expect_int("door_d2r.pass1_order",
               plan.steps[pass1Idx].cellOrderWord, 0x0128,
               "ReDMCSB DEFS.H:2668 C0x0128 right-lane door pass1");
    expect_int("door_d2r.pass2_order",
               plan.steps[pass2Idx].cellOrderWord, 0x0439,
               "ReDMCSB DEFS.H:2675 C0x0439 right-lane door pass2");
    expect_int("verify.door_d2r.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "D2R door-front plan passes invariants");

    /* Door-front on a square with no source door pass (D0C) fails
     * closed rather than inventing a route. */
    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D0C].element = DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    expect_int("build.door_d0c.rejected",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               0, "fail-closed: D0C has no F0127 door-front pass");

    /* Hand-broken plan: pass2 before F0111 must be rejected. */
    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element = DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan);
    doorIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                          DM1_V1_F0128_STEP_F0111_DOOR);
    pass2Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS2);
    {
        DM1_V1_F0128SchedulerStepPc34 tmp = plan.steps[doorIdx];
        plan.steps[doorIdx] = plan.steps[pass2Idx];
        plan.steps[pass2Idx] = tmp;
    }
    expect_int("verify.pass2_before_door.rejected",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               0, "fail-closed: pass2 before F0111 violates occlusion capture");
}

static void test_wall_alcove_and_far_lanes(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    int wallIdx, alcIdx, mainIdx;

    /* Plain wall: material + F0107 predicates, no thing pass
     * (F0116:6437 source return). */
    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D3L].element = DM1_V1_F0128_ELEMENT_WALL;
    expect_int("build.wall.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D3L wall view builds");
    expect_int("wall.material",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                                   DM1_V1_F0128_STEP_F0104_WALL_MATERIAL),
               1, "ReDMCSB DUNVIEW.C:6416-6440 D3L wall material");
    expect_int("wall.alcove_checks",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                                   DM1_V1_F0128_STEP_F0107_ALCOVE_CHECK),
               2, "ReDMCSB DUNVIEW.C:6441-6443 right+front F0107 predicates");
    expect_int("wall.no_things",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                                   DM1_V1_F0128_STEP_F0115_MAIN),
               0, "ReDMCSB DUNVIEW.C:6437 plain wall returns");
    expect_int("verify.wall.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "D3L wall plan passes invariants");

    /* Front alcove: wall jumps to the thing pass with the alcove
     * order word (F0116:6433-6436 goto T0116017). */
    squares[DM1_V1_F0128_VIEW_SQUARE_D3L].frontWallOrnamentIsAlcove = 1;
    expect_int("build.alcove.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D3L front-alcove wall builds");
    wallIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                          DM1_V1_F0128_STEP_F0104_WALL_MATERIAL);
    alcIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                         DM1_V1_F0128_STEP_F0107_ALCOVE_CHECK);
    mainIdx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D3L,
                          DM1_V1_F0128_STEP_F0115_MAIN);
    expect_int("alcove.order_wall_first", wallIdx >= 0 && wallIdx < alcIdx, 1,
               "wall material precedes the F0107 predicates");
    expect_int("alcove.things_after_checks", mainIdx > alcIdx, 1,
               "ReDMCSB DUNVIEW.C:6433-6436 alcove jumps to things");
    expect_int("alcove.order_word",
               plan.steps[mainIdx].cellOrderWord, 0x0000,
               "ReDMCSB DEFS.H:2658 C0x0000_CELL_ORDER_ALCOVE");
    expect_int("verify.alcove.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "D3L alcove plan passes invariants");

    /* D2L2/D2R2: wall material only; teleporter field only; no thing
     * pass ever (F0678/F0679, order unreferenced). */
    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D2L2].element = DM1_V1_F0128_ELEMENT_WALL;
    squares[DM1_V1_F0128_VIEW_SQUARE_D2R2].element = DM1_V1_F0128_ELEMENT_TELEPORTER;
    squares[DM1_V1_F0128_VIEW_SQUARE_D2R2].pitOrTeleporterVisible = 1;
    expect_int("build.far_lanes.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D2L2 wall + D2R2 teleporter builds");
    expect_int("d2l2.wall_only",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D2L2,
                                   DM1_V1_F0128_STEP_F0104_WALL_MATERIAL),
               1, "ReDMCSB DUNVIEW.C:6847-6856 F0678 wall bitmap");
    expect_int("d2l2.no_things",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D2L2,
                                   DM1_V1_F0128_STEP_F0115_MAIN),
               0, "ReDMCSB DUNVIEW.C:6843 order unreferenced on D2L2");
    expect_int("d2r2.field_only",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D2R2,
                                   DM1_V1_F0128_STEP_F0113_FIELD),
               1, "ReDMCSB DUNVIEW.C:6893-6897 F0679 teleporter field");
    expect_int("d2r2.no_things",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D2R2,
                                   DM1_V1_F0128_STEP_F0115_MAIN),
               0, "ReDMCSB DUNVIEW.C:6874 order unreferenced on D2R2");
    expect_int("verify.far_lanes.ok",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               1, "D2L2/D2R2 plan passes invariants");

    /* D0C door-side: door-frame material before the D0C thing pass
     * (F0127:8180-8204). */
    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D0C].element = DM1_V1_F0128_ELEMENT_DOOR_SIDE;
    expect_int("build.d0c_door_side.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D0C door-side view builds");
    expect_int("d0c.door_frame",
               count_op_for_square(&plan, DM1_V1_F0128_VIEW_SQUARE_D0C,
                                   DM1_V1_F0128_STEP_F0104_DOOR_FRAME),
               1, "ReDMCSB DUNVIEW.C:8180-8204 F0127 D0C door frame");
    expect_int("d0c.main_order",
               plan.steps[first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D0C,
                                      DM1_V1_F0128_STEP_F0115_MAIN)].cellOrderWord,
               0x0021, "ReDMCSB DUNVIEW.C:8232 C0x0021 D0C thing pass");
}

static void test_fail_closed_and_hash(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    DM1_V1_F0128SchedulerPlanPc34 plan2;

    all_corridor(squares);
    expect_int("build.null_input.rejected",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(NULL, &plan),
               0, "fail-closed on NULL squares");
    expect_int("build.null_output.rejected",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, NULL),
               0, "fail-closed on NULL plan");

    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element = 99;
    expect_int("build.bad_element.rejected",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               0, "fail-closed on out-of-contract element");
    expect_int("build.bad_element.no_partial",
               plan.stepCount, 0,
               "fail-closed emits no partial plan");

    all_corridor(squares);
    DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan);
    DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan2);
    expect_int("hash.deterministic",
               (long)plan.scheduleHash, (long)plan2.scheduleHash,
               "identical views produce identical receipts");
    expect_int("hash.nonzero", plan.scheduleHash != 0, 1,
               "receipt hash is populated");

    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].element = DM1_V1_F0128_ELEMENT_TELEPORTER;
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].pitOrTeleporterVisible = 1;
    DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan2);
    expect_int("hash.input_sensitive",
               plan.scheduleHash != plan2.scheduleHash, 1,
               "receipt hash tracks the schedule");

    expect_int("verify.null.rejected",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(NULL),
               0, "fail-closed on NULL plan verify");
    plan.steps[0].square = 99;
    expect_int("verify.bad_square.rejected",
               DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(&plan),
               0, "fail-closed on out-of-range square id");
}

static void test_span_and_observed_match(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    int start = -1, count = -1, mismatch = -1;
    int pass1Idx, pass2Idx;

    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element = DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    expect_int("build.span_scene.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "D1C door-front scene builds for span queries");

    /* D4L emits exactly one early step. */
    expect_int("span.d4l.ok",
               DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
                   &plan, DM1_V1_F0128_VIEW_SQUARE_D4L, &start, &count),
               1, "D4L span resolves");
    expect_int("span.d4l.start", start, 0,
               "ReDMCSB DUNVIEW.C:8479 D4L is the first emitted step");
    expect_int("span.d4l.count", count, 1,
               "ReDMCSB DUNVIEW.C:8479-8481 one early D4L F0115 step");

    /* D1C door-front: pass1, door frame, F0111, pass2 in one span. */
    expect_int("span.d1c.ok",
               DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
                   &plan, DM1_V1_F0128_VIEW_SQUARE_D1C, &start, &count),
               1, "D1C span resolves");
    expect_int("span.d1c.count", count, 4,
               "ReDMCSB DUNVIEW.C:7873-7937 D1C pass1/frame/door/pass2");
    pass1Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS1);
    pass2Idx = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                           DM1_V1_F0128_STEP_F0115_DOOR_PASS2);
    expect_int("span.d1c.start_is_pass1", start, pass1Idx,
               "D1C span starts at the door pass1 step");
    expect_int("span.d1c.end_is_pass2", start + count - 1, pass2Idx,
               "D1C span ends at the door pass2 step");

    /* D0R is the last square with steps in source order. */
    expect_int("span.d0c.ok",
               DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
                   &plan, DM1_V1_F0128_VIEW_SQUARE_D0C, &start, &count),
               1, "D0C span resolves");
    expect_int("span.d0c.ends_plan", start + count, plan.stepCount,
               "ReDMCSB DUNVIEW.C:8542 D0C is the final visit");

    /* Fail-closed span queries. */
    expect_int("span.bad_square.rejected",
               DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
                   &plan, 99, &start, &count),
               0, "fail-closed on out-of-range square span");
    expect_int("span.null_plan.rejected",
               DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
                   NULL, DM1_V1_F0128_VIEW_SQUARE_D4L, &start, &count),
               0, "fail-closed on NULL plan span");

    /* Observed-sequence comparison: exact self-match passes. */
    expect_int("observed.self_match",
               DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat(
                   &plan, plan.steps, plan.stepCount, &mismatch),
               1, "plan matches its own contract sequence");

    /* Truncated observation mismatches at the truncation point. */
    expect_int("observed.truncated.rejected",
               DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat(
                   &plan, plan.steps, plan.stepCount - 1, &mismatch),
               0, "truncated observation is rejected");
    expect_int("observed.truncated.index", mismatch, plan.stepCount - 1,
               "mismatch index marks the truncation point");

    /* A swapped step diverges at the swap index. */
    {
        DM1_V1_F0128SchedulerStepPc34 observed[DM1_V1_F0128_SCHEDULER_MAX_STEPS];
        DM1_V1_F0128SchedulerStepPc34 tmp;
        memcpy(observed, plan.steps, sizeof(observed));
        tmp = observed[pass1Idx];
        observed[pass1Idx] = observed[pass2Idx];
        observed[pass2Idx] = tmp;
        expect_int("observed.swapped.rejected",
                   DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat(
                       &plan, observed, plan.stepCount, &mismatch),
                   0, "swapped door steps are rejected");
        expect_int("observed.swapped.index", mismatch, pass1Idx,
                   "mismatch index marks the first diverging step");
    }

    expect_int("observed.null.rejected",
               DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat(
                   &plan, NULL, plan.stepCount, &mismatch),
               0, "fail-closed on NULL observation");
}

static void test_source_transaction_dispatch(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    DispatchTrace trace;
    int rejected = -1;
    int i;

    all_corridor(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element =
        DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].element =
        DM1_V1_F0128_ELEMENT_TELEPORTER;
    squares[DM1_V1_F0128_VIEW_SQUARE_D2C].pitOrTeleporterVisible = 1;
    expect_int("dispatch.build.ok",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan),
               1, "mixed F0128 plan builds before source transaction");

    memset(&trace, 0, sizeof(trace));
    trace.rejectAt = -1;
    expect_int("dispatch.complete.ok",
               DM1_V1_F0128_PerSquareSchedulerDispatchPc34Compat(
                   &plan, trace_preflight, trace_execute, &trace, &rejected),
               1, "all real-material preflights admit the whole F0128 plan");
    expect_int("dispatch.complete.rejected_none", rejected, -1,
               "complete source transaction has no rejected step");
    expect_int("dispatch.complete.preflight_count", trace.preflightCount,
               plan.stepCount, "every F0128 step preflights before execution");
    expect_int("dispatch.complete.execute_count", trace.executeCount,
               plan.stepCount, "every admitted F0128 step executes once");
    for (i = 0; i < plan.stepCount; ++i) {
        expect_int("dispatch.complete.preflight_square", trace.preflighted[i].square,
                   plan.steps[i].square,
                   "preflight retains ReDMCSB F0128 source square order");
        expect_int("dispatch.complete.execute_op", trace.executed[i].op,
                   plan.steps[i].op,
                   "execute retains the preflighted material-family operation");
    }

    memset(&trace, 0, sizeof(trace));
    trace.rejectAt = first_index(&plan, DM1_V1_F0128_VIEW_SQUARE_D1C,
                                 DM1_V1_F0128_STEP_F0111_DOOR);
    expect_int("dispatch.reject.ok",
               DM1_V1_F0128_PerSquareSchedulerDispatchPc34Compat(
                   &plan, trace_preflight, trace_execute, &trace, &rejected),
               0, "missing door material rejects the whole source transaction");
    expect_int("dispatch.reject.step", rejected, trace.rejectAt,
               "rejected step identifies the unavailable source route");
    expect_int("dispatch.reject.preflight_through_step", trace.preflightCount,
               trace.rejectAt + 1, "preflight stops at the first unavailable route");
    expect_int("dispatch.reject.no_execute", trace.executeCount, 0,
               "no material executes after any preflight rejection");

    memset(&trace, 0, sizeof(trace));
    plan.steps[0].fieldAfterThings = 1;
    expect_int("dispatch.invalid_plan.rejected",
               DM1_V1_F0128_PerSquareSchedulerDispatchPc34Compat(
                   &plan, trace_preflight, trace_execute, &trace, &rejected),
               0, "invalid step shape fails before source-material preflight");
    expect_int("dispatch.invalid_plan.no_callbacks",
               trace.preflightCount + trace.executeCount, 0,
               "invalid source schedule cannot reach a renderer callback");
}

int main(void)
{
    test_class_table();
    test_corridor_scene_visit_order();
    test_field_after_things();
    test_door_occlusion_capture();
    test_wall_alcove_and_far_lanes();
    test_fail_closed_and_hash();
    test_span_and_observed_match();
    test_source_transaction_dispatch();

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
