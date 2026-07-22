/*
 * dm1_v1_f0128_per_square_scheduler_pc34_compat.h
 *
 * DM1 V1 F0128 complete per-square source scheduler.
 *
 * Source-locked per ReDMCSB DUNVIEW.C:
 *   - F0128_DUNGEONVIEW_Draw_CPSF body, DUNVIEW.C:8318-8561, visits
 *     D4L/D4R/D4C (early F0115 passes, DUNVIEW.C:8479-8490) and then
 *     D3L2/D3R2, D3L/D3R/D3C, D2L2/D2R2, D2L/D2R/D2C, D1L/D1R/D1C,
 *     D0L/D0R/D0C in that exact source square order
 *     (DUNVIEW.C:8491-8542).
 *   - Per-square draw functions F0676/F0677 (DUNVIEW.C:6226-6360),
 *     F0678/F0679 (DUNVIEW.C:6837-6899) and F0116..F0127
 *     (DUNVIEW.C:6361-8317), merging the F0104 floor/pit/stairs/
 *     door-frame material, F0107 wall-ornament alcove check, F0111
 *     door draw, and F0113 field families into the same per-square
 *     scheduler that F0115 already runs in source square order.
 *   - Cell-order words per DEFS.H:2658-2677.
 *
 * Contract-only model: this scheduler emits and verifies the ordered
 * material-family step list per view square, including real
 * field-after-things (F0113 always lands after the square's last
 * F0115 step) and door/object occlusion capture (door pass1 things
 * sit behind the F0111 door, pass2 things in front). It draws no
 * pixels and accepts no host substitutes.
 */
#ifndef DM1_V1_F0128_PER_SQUARE_SCHEDULER_PC34_COMPAT_H
#define DM1_V1_F0128_PER_SQUARE_SCHEDULER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* View squares in exact F0128 source visit order
 * (ReDMCSB DUNVIEW.C:8479-8542). */
enum {
    DM1_V1_F0128_VIEW_SQUARE_D4L = 0,  /* DUNVIEW.C:8479-8481 */
    DM1_V1_F0128_VIEW_SQUARE_D4R,      /* DUNVIEW.C:8482-8484 */
    DM1_V1_F0128_VIEW_SQUARE_D4C,      /* DUNVIEW.C:8485-8490 */
    DM1_V1_F0128_VIEW_SQUARE_D3L2,     /* F0676 DUNVIEW.C:6226-6292 */
    DM1_V1_F0128_VIEW_SQUARE_D3R2,     /* F0677 DUNVIEW.C:6293-6360 */
    DM1_V1_F0128_VIEW_SQUARE_D3L,      /* F0116 DUNVIEW.C:6361-6499 */
    DM1_V1_F0128_VIEW_SQUARE_D3R,      /* F0117 DUNVIEW.C:6500-6641 */
    DM1_V1_F0128_VIEW_SQUARE_D3C,      /* F0118 DUNVIEW.C:6642-6836 */
    DM1_V1_F0128_VIEW_SQUARE_D2L2,     /* F0678 DUNVIEW.C:6837-6867 */
    DM1_V1_F0128_VIEW_SQUARE_D2R2,     /* F0679 DUNVIEW.C:6868-6899 */
    DM1_V1_F0128_VIEW_SQUARE_D2L,      /* F0119 DUNVIEW.C:6900-7050 */
    DM1_V1_F0128_VIEW_SQUARE_D2R,      /* F0120 DUNVIEW.C:7051-7243 */
    DM1_V1_F0128_VIEW_SQUARE_D2C,      /* F0121 DUNVIEW.C:7244-7390 */
    DM1_V1_F0128_VIEW_SQUARE_D1L,      /* F0122 DUNVIEW.C:7391-7558 */
    DM1_V1_F0128_VIEW_SQUARE_D1R,      /* F0123 DUNVIEW.C:7559-7726 */
    DM1_V1_F0128_VIEW_SQUARE_D1C,      /* F0124 DUNVIEW.C:7727-7959 */
    DM1_V1_F0128_VIEW_SQUARE_D0L,      /* F0125 DUNVIEW.C:7960-8063 */
    DM1_V1_F0128_VIEW_SQUARE_D0R,      /* F0126 DUNVIEW.C:8064-8163 */
    DM1_V1_F0128_VIEW_SQUARE_D0C,      /* F0127 DUNVIEW.C:8164-8317 */
    DM1_V1_F0128_VIEW_SQUARE_COUNT
};

/* Scheduler step operations: the merged F0104/F0107/F0108/F0111/
 * F0113/F0115 material families in per-square source order. */
enum {
    DM1_V1_F0128_STEP_F0115_EARLY = 0,     /* F0128 body early D4 F0115 (DUNVIEW.C:8479-8490) */
    DM1_V1_F0128_STEP_F0104_WALL_MATERIAL, /* wall bitmap (F0100/F0104 wall route) */
    DM1_V1_F0128_STEP_F0104_STAIRS,        /* stairs front bitmaps (F0116:6372-6394) */
    DM1_V1_F0128_STEP_F0104_PIT,           /* pit bitmaps (F0116:6453-6458) */
    DM1_V1_F0128_STEP_F0104_DOOR_FRAME,    /* door-frame bitmaps (F0116:6455-6459) */
    DM1_V1_F0128_STEP_F0107_ALCOVE_CHECK,  /* F0107 wall-ornament alcove predicate */
    DM1_V1_F0128_STEP_F0108_FLOOR_ORNAMENT,/* F0108 floor ornament under door back things */
    DM1_V1_F0128_STEP_F0115_DOOR_PASS1,    /* F0115 back cells behind the door */
    DM1_V1_F0128_STEP_F0111_DOOR,          /* F0111 door draw (object occluder) */
    DM1_V1_F0128_STEP_F0115_DOOR_PASS2,    /* F0115 front cells in front of the door */
    DM1_V1_F0128_STEP_F0115_MAIN,          /* F0115 main things pass */
    DM1_V1_F0128_STEP_F0113_FIELD,         /* F0113 teleporter field, always after things */
    DM1_V1_F0128_STEP_OP_COUNT
};

/* Square element values mirror ReDMCSB DEFS.H:1007-1017. */
enum {
    DM1_V1_F0128_ELEMENT_WALL = 0,          /* C00_ELEMENT_WALL */
    DM1_V1_F0128_ELEMENT_CORRIDOR = 1,      /* C01_ELEMENT_CORRIDOR */
    DM1_V1_F0128_ELEMENT_PIT = 2,           /* C02_ELEMENT_PIT */
    DM1_V1_F0128_ELEMENT_STAIRS = 3,        /* C03_ELEMENT_STAIRS */
    DM1_V1_F0128_ELEMENT_DOOR = 4,          /* C04_ELEMENT_DOOR */
    DM1_V1_F0128_ELEMENT_TELEPORTER = 5,    /* C05_ELEMENT_TELEPORTER */
    DM1_V1_F0128_ELEMENT_FAKEWALL = 6,      /* C06_ELEMENT_FAKEWALL */
    DM1_V1_F0128_ELEMENT_DOOR_SIDE = 16,    /* C16_ELEMENT_DOOR_SIDE */
    DM1_V1_F0128_ELEMENT_DOOR_FRONT = 17,   /* C17_ELEMENT_DOOR_FRONT */
    DM1_V1_F0128_ELEMENT_STAIRS_SIDE = 18,  /* C18_ELEMENT_STAIRS_SIDE */
    DM1_V1_F0128_ELEMENT_STAIRS_FRONT = 19  /* C19_ELEMENT_STAIRS_FRONT */
};

/* Cell-order words mirror ReDMCSB DEFS.H:2658-2677. */
enum {
    DM1_V1_F0128_ORDER_ALCOVE = 0x0000,                       /* C0x0000_CELL_ORDER_ALCOVE */
    DM1_V1_F0128_ORDER_BACKLEFT = 0x0001,                     /* C0x0001_CELL_ORDER_BACKLEFT */
    DM1_V1_F0128_ORDER_BACKRIGHT = 0x0002,                    /* C0x0002_CELL_ORDER_BACKRIGHT */
    DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT = 0x0018,           /* C0x0018 */
    DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT = 0x0021,           /* C0x0021 */
    DM1_V1_F0128_ORDER_DOORPASS1_BACKRIGHT = 0x0028,          /* C0x0028 */
    DM1_V1_F0128_ORDER_BACKRIGHT_FRONTRIGHT = 0x0032,         /* C0x0032 */
    DM1_V1_F0128_ORDER_DOORPASS2_FRONTRIGHT = 0x0039,         /* C0x0039 */
    DM1_V1_F0128_ORDER_BACKLEFT_FRONTLEFT = 0x0041,           /* C0x0041 */
    DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT = 0x0049,          /* C0x0049 */
    DM1_V1_F0128_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT = 0x0128, /* C0x0128 */
    DM1_V1_F0128_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT = 0x0218, /* C0x0218 */
    DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTRIGHT = 0x0321,/* C0x0321 */
    DM1_V1_F0128_ORDER_BACKRIGHT_FRONTLEFT_FRONTRIGHT = 0x0342,/* C0x0342 */
    DM1_V1_F0128_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT = 0x0349,/* C0x0349 */
    DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTLEFT = 0x0412, /* C0x0412 */
    DM1_V1_F0128_ORDER_BACKLEFT_FRONTRIGHT_FRONTLEFT = 0x0431,/* C0x0431 */
    DM1_V1_F0128_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT = 0x0439,/* C0x0439 */
    DM1_V1_F0128_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT = 0x3421, /* C0x3421 */
    DM1_V1_F0128_ORDER_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT = 0x4312  /* C0x4312 */
};

typedef struct {
    int element;                 /* DM1_V1_F0128_ELEMENT_* (DEFS.H:1007-1017) */
    int pitOrTeleporterVisible;  /* SquareAspect[M554] (DUNVIEW.C F0172) */
    int frontWallOrnamentIsAlcove; /* F0107 positive on M552 front ornament */
    int hasFloorOrnament;        /* SquareAspect[M558] floor ornament present */
} DM1_V1_F0128SchedulerSquarePc34;

typedef struct {
    int square;         /* DM1_V1_F0128_VIEW_SQUARE_* */
    int op;             /* DM1_V1_F0128_STEP_* */
    int cellOrderWord;  /* DEFS.H:2658-2677 order word, 0 when not an F0115 step */
    int behindDoor;     /* 1: drawn behind the F0111 door (door pass1 back cells) */
    int occluder;       /* 1: the F0111 door itself, occludes pass1 things */
    int fieldAfterThings; /* 1: F0113 field step pinned after the square's things */
} DM1_V1_F0128SchedulerStepPc34;

#define DM1_V1_F0128_SCHEDULER_MAX_STEPS 192

typedef struct {
    DM1_V1_F0128SchedulerStepPc34 steps[DM1_V1_F0128_SCHEDULER_MAX_STEPS];
    int stepCount;
    int fieldAfterThingsViolations;  /* F0113 not after the square's last F0115 */
    int doorOcclusionViolations;     /* pass1/F0111/pass2 order or flag broken */
    uint32_t scheduleHash;           /* FNV-1a over (square, op, orderWord) */
} DM1_V1_F0128SchedulerPlanPc34;

typedef struct {
    int square;                  /* DM1_V1_F0128_VIEW_SQUARE_* */
    const char *name;            /* "D3L" etc. */
    const char *sourceAnchor;    /* ReDMCSB DUNVIEW.C function + line range */
    int earlyOnly;               /* D4L/D4R/D4C: early F0115 only (DUNVIEW.C:8479-8490) */
    int doorPass1Order;          /* 0 when the square has no door pass1 */
    int doorPass2Order;
    int corridorOrder;           /* main F0115 order for corridor/pit/teleporter */
    int doorSideOrder;           /* door_side/stairs_side main order, 0 = corridorOrder */
    int alcoveChecks;            /* wall case runs the F0107 right/front predicates */
    int noThingPass;             /* F0678/F0679 D2L2/D2R2: wall/field only */
    int fieldCapable;            /* teleporter F0113 allowed on this square */
} DM1_V1_F0128SquareClassPc34;

/* A renderer binds real, already-decoded source material to a schedule in
 * two phases.  Preflight is deliberately separate from execution: every
 * source-owned material route must be admitted before the first draw call,
 * so a missing bitmap/palette cannot leave a partially composed viewport or
 * trigger a substitute draw.  Execute receives the exact verified F0128
 * step order and owns no fallback policy. */
typedef int (*DM1_V1_F0128SchedulerPreflightPc34)(
    void *context, const DM1_V1_F0128SchedulerStepPc34 *step);
typedef void (*DM1_V1_F0128SchedulerExecutePc34)(
    void *context, const DM1_V1_F0128SchedulerStepPc34 *step);

void DM1_V1_F0128_PerSquareSchedulerInitPc34Compat(void);
size_t DM1_V1_F0128_PerSquareSchedulerClassCountPc34Compat(void);
const DM1_V1_F0128SquareClassPc34 *
DM1_V1_F0128_PerSquareSchedulerClassAtPc34Compat(size_t index);
const DM1_V1_F0128SquareClassPc34 *
DM1_V1_F0128_PerSquareSchedulerClassForSquarePc34Compat(int square);

/* Builds the merged per-square plan for one 19-square view. Fail-closed:
 * returns 0 on NULL input/output or an out-of-range element; emits no
 * partial plan in that case. */
int DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(
    const DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT],
    DM1_V1_F0128SchedulerPlanPc34 *outPlan);

/* Re-verifies the plan invariants: exact source visit order, field-after-
 * things per square, and door pass1 -> F0111 -> pass2 occlusion capture.
 * Returns 1 when every invariant holds, 0 otherwise. */
int DM1_V1_F0128_PerSquareSchedulerVerifyPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan);

/* Returns the [start, start + count) step span for one view square in a
 * built plan, so a consumer (e.g. the M11 draw path) can iterate the
 * square's merged material-family steps in source order. Returns 0 when
 * the plan is NULL, the square is out of range, or the square emitted no
 * steps. */
int DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan, int square,
    int *outStart, int *outCount);

/* Compares a caller-observed step sequence (e.g. an M11 draw trace)
 * against the contract plan. Returns 1 on an exact match of every
 * (square, op, cellOrderWord) triple; returns 0 on mismatch and stores
 * the first diverging index in outMismatchIndex when non-NULL. */
int DM1_V1_F0128_PerSquareSchedulerMatchesObservedPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan,
    const DM1_V1_F0128SchedulerStepPc34 *observed, int observedCount,
    int *outMismatchIndex);

/* Runs a complete F0128 plan through a source-material transaction.  The
 * plan is re-verified, then every step is preflighted in source order before
 * any execute callback runs.  Returns 0 and runs no execute callback when a
 * plan/handler is invalid or preflight rejects a step.  outRejectedStep is
 * -1 on success, otherwise the invalid/rejected step index (0 for invalid
 * inputs).  This module supplies no pixels, palette, or fallback rendering. */
int DM1_V1_F0128_PerSquareSchedulerDispatchPc34Compat(
    const DM1_V1_F0128SchedulerPlanPc34 *plan,
    DM1_V1_F0128SchedulerPreflightPc34 preflight,
    DM1_V1_F0128SchedulerExecutePc34 execute,
    void *context, int *outRejectedStep);

const char *DM1_V1_F0128_PerSquareSchedulerSourceContractPc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_F0128_PER_SQUARE_SCHEDULER_PC34_COMPAT_H */
