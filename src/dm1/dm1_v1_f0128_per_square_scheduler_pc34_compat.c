/*
 * dm1_v1_f0128_per_square_scheduler_pc34_compat.c
 *
 * Source-locked per ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF
 * (DUNVIEW.C:8318-8561) and the per-square draw functions
 * F0676/F0677 (DUNVIEW.C:6226-6360), F0678/F0679
 * (DUNVIEW.C:6837-6899), F0116..F0127 (DUNVIEW.C:6361-8317).
 * Cell-order words per DEFS.H:2658-2677.
 *
 * Merges the F0104/F0107/F0108/F0111/F0113 material families into the
 * same per-square scheduler that F0115 already runs in source square
 * order (2026-07-15 F0115 square order D3L/D3R/D3C, D2L/D2R/D2C,
 * D1L/D1R/D1C extended here to the full F0128 D4..D0 visit order).
 * Real field-after-things: F0113 always lands after the square's last
 * F0115 step (e.g. DUNVIEW.C:6289, :6487-6498, :8315-8317). Real
 * door/object occlusion capture: F0115 door pass1 back cells sit
 * behind the F0111 door, pass2 front cells in front
 * (F0116:6444-6461). No host substitutes are permitted; the builder
 * fails closed on out-of-contract input.
 */
#include "dm1_v1_f0128_per_square_scheduler_pc34_compat.h"

#include <string.h>

/* Per-square class table in exact F0128 source visit order
 * (DUNVIEW.C:8479-8542). Order words verified per square function:
 *   D4L/D4R/D4C  F0128 body  C0x0001                  (DUNVIEW.C:8479-8490)
 *   D3L2 F0676   0x0218/0x0349/0x3421, side 0x0321    (DUNVIEW.C:6271-6286)
 *   D3R2 F0677   0x0128/0x0439/0x4312, side 0x0412    (DUNVIEW.C:6338-6353)
 *   D3L  F0116   0x0218/0x0349/0x3421, side 0x0321    (DUNVIEW.C:6444-6473)
 *   D3R  F0117   0x0128/0x0439/0x4312, side 0x0412
 *   D3C  F0118   0x0218/0x0349/0x3421
 *   D2L2 F0678   wall/field only, order unreferenced  (DUNVIEW.C:6843-6866)
 *   D2R2 F0679   wall/field only, order unreferenced  (DUNVIEW.C:6874-6898)
 *   D2L  F0119   0x0218/0x0349/0x3421, side 0x0342
 *   D2R  F0120   0x0128/0x0439/0x4312, side 0x0431
 *   D2C  F0121   0x0218/0x0349/0x3421
 *   D1L  F0122   0x0028/0x0039/0x0032
 *   D1R  F0123   0x0018/0x0049/0x0041
 *   D1C  F0124   0x0218/0x0349/0x3421, alcove 0x0000
 *   D0L  F0125   main 0x0002                          (DUNVIEW.C:8005)
 *   D0R  F0126   main 0x0001                          (DUNVIEW.C:8115)
 *   D0C  F0127   main 0x0021                          (DUNVIEW.C:8232) */
static const DM1_V1_F0128SquareClassPc34 kClasses[DM1_V1_F0128_VIEW_SQUARE_COUNT] = {
    { DM1_V1_F0128_VIEW_SQUARE_D4L,  "D4L",  "ReDMCSB DUNVIEW.C:8479-8481 F0128 early F0115", 1, 0, 0, DM1_V1_F0128_ORDER_BACKLEFT, 0, 0, 0, 0 },
    { DM1_V1_F0128_VIEW_SQUARE_D4R,  "D4R",  "ReDMCSB DUNVIEW.C:8482-8484 F0128 early F0115", 1, 0, 0, DM1_V1_F0128_ORDER_BACKLEFT, 0, 0, 0, 0 },
    { DM1_V1_F0128_VIEW_SQUARE_D4C,  "D4C",  "ReDMCSB DUNVIEW.C:8485-8490 F0128 early F0115", 1, 0, 0, DM1_V1_F0128_ORDER_BACKLEFT, 0, 0, 0, 0 },
    { DM1_V1_F0128_VIEW_SQUARE_D3L2, "D3L2", "ReDMCSB DUNVIEW.C:6226-6292 F0676_DrawD3L2", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTRIGHT, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D3R2, "D3R2", "ReDMCSB DUNVIEW.C:6293-6360 F0677_DrawD3R2", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT, DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT, DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTLEFT, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D3L,  "D3L",  "ReDMCSB DUNVIEW.C:6361-6499 F0116_DrawSquareD3L", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTRIGHT, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D3R,  "D3R",  "ReDMCSB DUNVIEW.C:6500-6641 F0117_DrawSquareD3R", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT, DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT, DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTLEFT, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D3C,  "D3C",  "ReDMCSB DUNVIEW.C:6642-6836 F0118_DrawSquareD3C", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT, 0, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D2L2, "D2L2", "ReDMCSB DUNVIEW.C:6837-6867 F0678_DrawD2L2", 0, 0, 0, 0, 0, 0, 1, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D2R2, "D2R2", "ReDMCSB DUNVIEW.C:6868-6899 F0679_DrawD2R2", 0, 0, 0, 0, 0, 0, 1, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D2L,  "D2L",  "ReDMCSB DUNVIEW.C:6900-7050 F0119_DrawSquareD2L", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKRIGHT_FRONTLEFT_FRONTRIGHT, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D2R,  "D2R",  "ReDMCSB DUNVIEW.C:7051-7243 F0120_DrawSquareD2R", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT, DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT, DM1_V1_F0128_ORDER_BACKLEFT_FRONTRIGHT_FRONTLEFT, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D2C,  "D2C",  "ReDMCSB DUNVIEW.C:7244-7390 F0121_DrawSquareD2C", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT, 0, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D1L,  "D1L",  "ReDMCSB DUNVIEW.C:7391-7558 F0122_DrawSquareD1L", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKRIGHT_FRONTRIGHT, 0, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D1R,  "D1R",  "ReDMCSB DUNVIEW.C:7559-7726 F0123_DrawSquareD1R", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT, DM1_V1_F0128_ORDER_BACKLEFT_FRONTLEFT, 0, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D1C,  "D1C",  "ReDMCSB DUNVIEW.C:7727-7959 F0124_DrawSquareD1C", 0, DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT, 0, 1, 0, 1 },
    { DM1_V1_F0128_VIEW_SQUARE_D0L,  "D0L",  "ReDMCSB DUNVIEW.C:7960-8063 F0125_DrawSquareD0L", 0, 0, 0, DM1_V1_F0128_ORDER_BACKRIGHT, 0, 0, 0, 0 },
    { DM1_V1_F0128_VIEW_SQUARE_D0R,  "D0R",  "ReDMCSB DUNVIEW.C:8064-8163 F0126_DrawSquareD0R", 0, 0, 0, DM1_V1_F0128_ORDER_BACKLEFT, 0, 0, 0, 0 },
    { DM1_V1_F0128_VIEW_SQUARE_D0C,  "D0C",  "ReDMCSB DUNVIEW.C:8164-8317 F0127_DrawSquareD0C", 0, 0, 0, DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT, 0, 0, 0, 1 },
};

static int is_valid_element(int element) {
    switch (element) {
    case DM1_V1_F0128_ELEMENT_WALL:
    case DM1_V1_F0128_ELEMENT_CORRIDOR:
    case DM1_V1_F0128_ELEMENT_PIT:
    case DM1_V1_F0128_ELEMENT_STAIRS:
    case DM1_V1_F0128_ELEMENT_DOOR:
    case DM1_V1_F0128_ELEMENT_TELEPORTER:
    case DM1_V1_F0128_ELEMENT_FAKEWALL:
    case DM1_V1_F0128_ELEMENT_DOOR_SIDE:
    case DM1_V1_F0128_ELEMENT_DOOR_FRONT:
    case DM1_V1_F0128_ELEMENT_STAIRS_SIDE:
    case DM1_V1_F0128_ELEMENT_STAIRS_FRONT:
        return 1;
    default:
        return 0;
    }
}

static int push_step(DM1_V1_F0128SchedulerPlanPc34 *plan, int square, int op,
                     int orderWord, int behindDoor, int occluder,
                     int fieldAfterThings) {
    DM1_V1_F0128SchedulerStepPc34 *step;
    if (plan->stepCount >= DM1_V1_F0128_SCHEDULER_MAX_STEPS) {
        return 0;
    }
    step = &plan->steps[plan->stepCount++];
    step->square = square;
    step->op = op;
    step->cellOrderWord = orderWord;
    step->behindDoor = behindDoor;
    step->occluder = occluder;
    step->fieldAfterThings = fieldAfterThings;
    return 1;
}

static int step_shape_is_valid(const DM1_V1_F0128SchedulerStepPc34 *step) {
    if (!step) {
        return 0;
    }
    switch (step->op) {
    case DM1_V1_F0128_STEP_F0115_EARLY:
        return step->cellOrderWord != 0 && !step->behindDoor &&
               !step->occluder && !step->fieldAfterThings;
    case DM1_V1_F0128_STEP_F0115_MAIN:
        /* F0107's positive alcove branch calls F0115 with
         * C0x0000_CELL_ORDER_ALCOVE (DUNVIEW.C:6433-6436,
         * DEFS.H:2658), unlike ordinary main passes. */
        return !step->behindDoor && !step->occluder &&
               !step->fieldAfterThings;
    case DM1_V1_F0128_STEP_F0115_DOOR_PASS1:
        return step->cellOrderWord != 0 && step->behindDoor &&
               !step->occluder && !step->fieldAfterThings;
    case DM1_V1_F0128_STEP_F0115_DOOR_PASS2:
        return step->cellOrderWord != 0 && !step->behindDoor &&
               !step->occluder && !step->fieldAfterThings;
    case DM1_V1_F0128_STEP_F0111_DOOR:
        return step->cellOrderWord == 0 && !step->behindDoor &&
               step->occluder && !step->fieldAfterThings;
    case DM1_V1_F0128_STEP_F0113_FIELD:
        return step->cellOrderWord == 0 && !step->behindDoor &&
               !step->occluder && step->fieldAfterThings;
    case DM1_V1_F0128_STEP_F0104_WALL_MATERIAL:
    case DM1_V1_F0128_STEP_F0104_STAIRS:
    case DM1_V1_F0128_STEP_F0104_PIT:
    case DM1_V1_F0128_STEP_F0104_DOOR_FRAME:
    case DM1_V1_F0128_STEP_F0107_ALCOVE_CHECK:
    case DM1_V1_F0128_STEP_F0108_FLOOR_ORNAMENT:
        return step->cellOrderWord == 0 && !step->behindDoor &&
               !step->occluder && !step->fieldAfterThings;
    default:
        return 0;
    }
}

void DM1_V1_F0128_PerSquareSchedulerInitPc34Compat(void) {
    /* Static table only; kept for symmetry with sibling contract modules. */
}

size_t DM1_V1_F0128_PerSquareSchedulerClassCountPc34Compat(void) {
    return DM1_V1_F0128_VIEW_SQUARE_COUNT;
}

const DM1_V1_F0128SquareClassPc34 *
DM1_V1_F0128_PerSquareSchedulerClassAtPc34Compat(size_t index) {
    if (index >= DM1_V1_F0128_VIEW_SQUARE_COUNT) {
        return NULL;
    }
    return &kClasses[index];
}

const DM1_V1_F0128SquareClassPc34 *
DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(int square) {
    if (square < 0 || square >= DM1_V1_F0128_VIEW_SQUARE_COUNT) {
        return NULL;
    }
    return &kClasses[square];
}

int DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(
    const DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT],
    DM1_V1_F0128SchedulerPlanPc34 *outPlan) {
    int i;
    if (!squares || !outPlan) {
        return 0;
    }
    memset(outPlan, 0, sizeof(*outPlan));
    for (i = 0; i < DM1_V1_F0128_VIEW_SQUARE_COUNT; i++) {
        const DM1_V1_F0128SquareClassPc34 *cls = &kClasses[i];
        const DM1_V1_F0128SchedulerSquarePc34 *sq = &squares[i];
        if (!is_valid_element(sq->element)) {
            memset(outPlan, 0, sizeof(*outPlan));
            return 0;
        }

        /* F0128 body: D4L/D4R/D4C run the early F0115 pass only,
         * unconditionally of element (DUNVIEW.C:8479-8490). */
        if (cls->earlyOnly) {
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_EARLY,
                           cls->corridorOrder, 0, 0, 0)) {
                return 0;
            }
            continue;
        }

        /* F0678/F0679 D2L2/D2R2: wall material or teleporter field only;
         * the order variable is unreferenced in source
         * (DUNVIEW.C:6843 L2487_i_Order_Unreferenced). */
        if (cls->noThingPass) {
            if (sq->element == DM1_V1_F0128_ELEMENT_WALL) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0104_WALL_MATERIAL,
                               0, 0, 0, 0)) {
                    return 0;
                }
            } else if (sq->element == DM1_V1_F0128_ELEMENT_TELEPORTER) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0113_FIELD,
                               0, 0, 0, 1)) {
                    return 0;
                }
            }
            continue;
        }

        switch (sq->element) {
        case DM1_V1_F0128_ELEMENT_WALL:
            /* Wall material first (F0116:6416-6440), then the F0107
             * right/front alcove predicates (F0116:6441-6443). A
             * front alcove jumps to the thing pass with the alcove
             * order word (F0116:6433-6436 goto T0116017); any other
             * wall returns without a thing pass (F0116:6437). */
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0104_WALL_MATERIAL,
                           0, 0, 0, 0)) {
                return 0;
            }
            if (cls->alcoveChecks) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0107_ALCOVE_CHECK,
                               0, 0, 0, 0) ||
                    !push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0107_ALCOVE_CHECK,
                               0, 0, 0, 0)) {
                    return 0;
                }
            }
            if (sq->frontWallOrnamentIsAlcove) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_MAIN,
                               DM1_V1_F0128_ORDER_ALCOVE, 0, 0, 0)) {
                    return 0;
                }
            }
            break;

        case DM1_V1_F0128_ELEMENT_STAIRS_FRONT:
            /* F0104 stairs bitmaps, then the thing pass with the
             * square's corridor order (F0116:6372-6394, :6472-6473). */
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0104_STAIRS,
                           0, 0, 0, 0) ||
                !push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_MAIN,
                           cls->corridorOrder, 0, 0, 0)) {
                return 0;
            }
            break;

        case DM1_V1_F0128_ELEMENT_DOOR_SIDE:
        case DM1_V1_F0128_ELEMENT_STAIRS_SIDE:
            /* Side door/stairs: D0C also draws its door-frame bitmaps
             * (F0127:8180-8204); then the thing pass with the side
             * order word (F0116:6445-6447). */
            if (cls->square == DM1_V1_F0128_VIEW_SQUARE_D0C &&
                sq->element == DM1_V1_F0128_ELEMENT_DOOR_SIDE) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0104_DOOR_FRAME,
                               0, 0, 0, 0)) {
                    return 0;
                }
            }
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_MAIN,
                           cls->doorSideOrder ? cls->doorSideOrder : cls->corridorOrder,
                           0, 0, 0)) {
                return 0;
            }
            break;

        case DM1_V1_F0128_ELEMENT_DOOR_FRONT:
            /* Door/object occlusion capture (F0116:6444-6461): F0108
             * floor ornament, F0115 pass1 back cells behind the door,
             * door-frame bitmaps, F0111 door occluder, F0115 pass2
             * front cells in front of the door. */
            if (sq->hasFloorOrnament) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0108_FLOOR_ORNAMENT,
                               0, 0, 0, 0)) {
                    return 0;
                }
            }
            if (cls->doorPass1Order == 0 || cls->doorPass2Order == 0) {
                /* D0L/D0R/D0C have no source door-front pass; fail
                 * closed rather than inventing one. */
                memset(outPlan, 0, sizeof(*outPlan));
                return 0;
            }
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_DOOR_PASS1,
                           cls->doorPass1Order, 1, 0, 0) ||
                !push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0104_DOOR_FRAME,
                           0, 0, 0, 0) ||
                !push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0111_DOOR,
                           0, 0, 1, 0) ||
                !push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_DOOR_PASS2,
                           cls->doorPass2Order, 0, 0, 0)) {
                return 0;
            }
            break;

        case DM1_V1_F0128_ELEMENT_PIT:
            /* F0104 pit bitmaps when the pit is not flagged visible
             * (F0116:6453-6458); then the thing pass. */
            if (!sq->pitOrTeleporterVisible) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0104_PIT,
                               0, 0, 0, 0)) {
                    return 0;
                }
            }
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_MAIN,
                           cls->corridorOrder, 0, 0, 0)) {
                return 0;
            }
            break;

        case DM1_V1_F0128_ELEMENT_TELEPORTER:
            /* Things first, then the F0113 field: real
             * field-after-things (F0116:6472-6487, F0127:8232/8315).
             * D0C has no wall-zone field route in F0127's teleporter
             * tail... F0127:8315-8317 does draw it, so every
             * field-capable square is admitted. */
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_MAIN,
                           cls->corridorOrder, 0, 0, 0)) {
                return 0;
            }
            if (sq->pitOrTeleporterVisible && cls->fieldCapable) {
                if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0113_FIELD,
                               0, 0, 0, 1)) {
                    return 0;
                }
            }
            break;

        case DM1_V1_F0128_ELEMENT_CORRIDOR:
        case DM1_V1_F0128_ELEMENT_STAIRS:
        case DM1_V1_F0128_ELEMENT_DOOR:
        case DM1_V1_F0128_ELEMENT_FAKEWALL:
        default:
            /* Corridor-like squares run the main thing pass with the
             * square's corridor order word (F0116:6470-6473). */
            if (!push_step(outPlan, cls->square, DM1_V1_F0128_STEP_F0115_MAIN,
                           cls->corridorOrder, 0, 0, 0)) {
                return 0;
            }
            break;
        }
    }

    /* FNV-1a receipt over (square, op, orderWord) triples. */
    {
        uint32_t hash = 2166136261u;
        for (i = 0; i < outPlan->stepCount; i++) {
            const DM1_V1_F0128SchedulerStepPc34 *s = &outPlan->steps[i];
            hash = (hash ^ (uint32_t)s->square) * 16777619u;
            hash = (hash ^ (uint32_t)s->op) * 16777619u;
            hash = (hash ^ (uint32_t)(s->cellOrderWord & 0xFFFF)) * 16777619u;
        }
        outPlan->scheduleHash = hash;
    }
    return 1;
}

int DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan) {
    int lastStepForSquare[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    int lastF0115ForSquare[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    int i;
    int violations = 0;

    if (!plan || plan->stepCount < 0 ||
        plan->stepCount > DM1_V1_F0128_SCHEDULER_MAX_STEPS) {
        return 0;
    }
    for (i = 0; i < DM1_V1_F0128_VIEW_SQUARE_COUNT; i++) {
        lastStepForSquare[i] = -1;
        lastF0115ForSquare[i] = -1;
    }

    for (i = 0; i < plan->stepCount; i++) {
        const DM1_V1_F0128SchedulerStepPc34 *s = &plan->steps[i];
        if (s->square < 0 || s->square >= DM1_V1_F0128_VIEW_SQUARE_COUNT ||
            s->op < 0 || s->op >= DM1_V1_F0128_STEP_OP_COUNT ||
            !step_shape_is_valid(s)) {
            return 0;
        }
        {
            int j;
            for (j = s->square + 1; j < DM1_V1_F0128_VIEW_SQUARE_COUNT; j++) {
                if (lastStepForSquare[j] >= 0 && lastStepForSquare[j] < i) {
                    /* A later square already emitted; this square is
                     * out of source order. */
                    violations++;
                    break;
                }
            }
        }
        lastStepForSquare[s->square] = i;

        if (s->op == DM1_V1_F0128_STEP_F0115_EARLY ||
            s->op == DM1_V1_F0128_STEP_F0115_DOOR_PASS1 ||
            s->op == DM1_V1_F0128_STEP_F0115_DOOR_PASS2 ||
            s->op == DM1_V1_F0128_STEP_F0115_MAIN) {
            lastF0115ForSquare[s->square] = i;
        }

        /* Field-after-things: an F0113 step must land after every
         * F0115 step of the same square (DUNVIEW.C:6289, :6487). */
        if (s->op == DM1_V1_F0128_STEP_F0113_FIELD) {
            if (!s->fieldAfterThings) {
                violations++;
            }
        }
    }

    /* Per-square: every F0113 field must sit after the square's last
     * F0115 step, and door pass1 -> F0111 -> pass2 occlusion capture
     * must hold (F0116:6444-6461). */
    for (i = 0; i < plan->stepCount; i++) {
        const DM1_V1_F0128SchedulerStepPc34 *s = &plan->steps[i];
        if (s->op == DM1_V1_F0128_STEP_F0113_FIELD &&
            lastF0115ForSquare[s->square] > i) {
            violations++;
        }
        if (s->op == DM1_V1_F0128_STEP_F0115_DOOR_PASS1) {
            int j;
            int foundDoor = 0;
            int foundPass2 = 0;
            if (!s->behindDoor) {
                violations++;
            }
            for (j = i + 1; j < plan->stepCount; j++) {
                const DM1_V1_F0128SchedulerStepPc34 *t = &plan->steps[j];
                if (t->square != s->square) {
                    break;
                }
                if (t->op == DM1_V1_F0128_STEP_F0111_DOOR && !foundDoor) {
                    foundDoor = 1;
                    if (!t->occluder) {
                        violations++;
                    }
                } else if (t->op == DM1_V1_F0128_STEP_F0115_DOOR_PASS2) {
                    if (!foundDoor) {
                        violations++;
                    }
                    if (t->behindDoor) {
                        violations++;
                    }
                    foundPass2 = 1;
                    break;
                }
            }
            if (!foundDoor || !foundPass2) {
                violations++;
            }
        }
    }

    return violations == 0;
}

int DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan, int square,
    int *outStart, int *outCount) {
    int i;
    int start = -1;
    int count = 0;
    if (outStart) {
        *outStart = 0;
    }
    if (outCount) {
        *outCount = 0;
    }
    if (!plan || square < 0 || square >= DM1_V1_F0128_VIEW_SQUARE_COUNT ||
        plan->stepCount < 0 ||
        plan->stepCount > DM1_V1_F0128_SCHEDULER_MAX_STEPS) {
        return 0;
    }
    for (i = 0; i < plan->stepCount; i++) {
        if (plan->steps[i].square == square) {
            if (start < 0) {
                start = i;
            }
            count++;
        } else if (start >= 0) {
            /* Source visit order keeps a square's steps contiguous
             * (DUNVIEW.C:8491-8542). */
            break;
        }
    }
    if (start < 0) {
        return 0;
    }
    if (outStart) {
        *outStart = start;
    }
    if (outCount) {
        *outCount = count;
    }
    return 1;
}

int DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan,
    const DM1_V1_F0128SchedulerStepPc34 *observed, int observedCount,
    int *outMismatchIndex) {
    int i;
    if (outMismatchIndex) {
        *outMismatchIndex = -1;
    }
    if (!plan || !observed || observedCount < 0 ||
        plan->stepCount < 0 ||
        plan->stepCount > DM1_V1_F0128_SCHEDULER_MAX_STEPS) {
        if (outMismatchIndex) {
            *outMismatchIndex = 0;
        }
        return 0;
    }
    if (observedCount != plan->stepCount) {
        if (outMismatchIndex) {
            *outMismatchIndex =
                observedCount < plan->stepCount ? observedCount : plan->stepCount;
        }
        return 0;
    }
    for (i = 0; i < plan->stepCount; i++) {
        if (observed[i].square != plan->steps[i].square ||
            observed[i].op != plan->steps[i].op ||
            observed[i].cellOrderWord != plan->steps[i].cellOrderWord) {
            if (outMismatchIndex) {
                *outMismatchIndex = i;
            }
            return 0;
        }
    }
    return 1;
}

int DM1_V1_F0128_PerSquareSchedulerDispatchPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan,
    DM1_V1_F0128SchedulerPreflightPc34 preflight,
    DM1_V1_F0128SchedulerExecutePc34 execute,
    void *context, int *outRejectedStep) {
    int i;

    if (outRejectedStep) {
        *outRejectedStep = -1;
    }
    if (!plan || !preflight || !execute ||
        !DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(plan)) {
        if (outRejectedStep) {
            *outRejectedStep = 0;
        }
        return 0;
    }

    /* ReDMCSB F0128 owns one ordered square transaction.  Validate every
     * source route first so execution never fills missing media with a host
     * stand-in and never starts a frame which the source cannot complete. */
    for (i = 0; i < plan->stepCount; ++i) {
        if (!preflight(context, &plan->steps[i])) {
            if (outRejectedStep) {
                *outRejectedStep = i;
            }
            return 0;
        }
    }
    for (i = 0; i < plan->stepCount; ++i) {
        execute(context, &plan->steps[i]);
    }
    return 1;
}

const char *DM1_V1_F0128_PerSquareSchedulerSourceContractPc34Compat(void) {
    return "ReDMCSB DUNVIEW.C F0128:8318-8561 visit order D4L/D4R/D4C "
           "(8479-8490) then D3L2/D3R2, D3L/D3R/D3C, D2L2/D2R2, "
           "D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C (8491-8542); "
           "F0676/F0677:6226-6360, F0678/F0679:6837-6899, "
           "F0116-F0127:6361-8317; merged F0104/F0107/F0108/F0111/"
           "F0113 material families, field-after-things (6289, 6487, "
           "8315), door pass1/F0111/pass2 occlusion (6444-6461); "
           "cell-order words DEFS.H:2658-2677. Contract-only; no host "
           "substitutes.";
}
