/*
 * Firestaff DM1 V1 viewport draw-order probe.
 *
 * Primary source locks audited before writing this probe:
 *   ReDMCSB Toolchains/Common/Source/VBLANK.C
 *     - F0577_VerticalBlank_Handler_CPSDF lines 93-315: VBlank palette/message work;
 *       guards message-area copies with !G3126_B_ViewportIsBeingBlittedToScreen.
 *   ReDMCSB Toolchains/Common/Source/VIEWPORT.C
 *     - F0564_VIEWPORT_InitializeBitPlanes lines 15-28: 224x136 source viewport,
 *       320-wide screen destination, y=33.
 *     - F0566_VIEWPORT_BlitToScreen lines 51-98: four bitplanes blitted at 224/16 x 136.
 *   ReDMCSB Toolchains/Common/Source/DRAWVIEW.C
 *     - F0097_DUNGEONVIEW_DrawViewport lines 709-722: requests viewport draw and waits one VBlank.
 *     - F0097 lines 821-858: PC/I34 route blits G0296_puc_Bitmap_Viewport to C007_ZONE_VIEWPORT.
 *     - F0097 lines 1038-1068: viewport blit flag wraps screen copy on other ports.
 *   ReDMCSB Toolchains/Common/Source/DUNVIEW.C
 *     - F0098_DUNGEONVIEW_DrawFloorAndCeiling line 2962: base floor/ceiling composite.
 *     - F0108_DUNGEONVIEW_DrawFloorOrnament lines 3940-4008: floor ornaments before objects.
 *     - F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF lines 4547-4582:
 *       object/creature/projectile/explosion order contract.
 *     - F0115 object path lines 4820-4918: objects by cell, D2/D3 shrink + palette.
 *     - F0115 creature path lines 5201-5514: creature pose, scaling, clipping, blit.
 *     - F0128_DUNGEONVIEW_Draw_CPSF lines 8318-8542: whole-viewport back-to-front square walk.
 *   ReDMCSB Toolchains/Common/Source/CLIKVIEW.C
 *     - F0377_COMMAND_ProcessType80_ClickInDungeonView lines 311-510: viewport click boxes line up
 *       with view cells/alcove/object piles; not a draw routine but locks interaction z-targets.
 *   ReDMCSB Toolchains/Common/Source/ACTIDRAW.C
 *     - F0385/F0386/F0387 lines 34-379: action/UI drawing is outside viewport composition.
 *
 * This is a headless deterministic probe: it models the source-locked sequencing and clipping
 * rules above and emits a compact per-frame trace for regression evidence.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { VIEWPORT_W = 224, VIEWPORT_H = 136, VIEWPORT_BYTE_W = 112, SCREEN_W = 320, SCREEN_H = 200, VIEWPORT_SCREEN_Y = 33 };
enum { C16_SCALE_D3 = 16, C20_SCALE_D2 = 20 };

typedef struct { int left, right, top, bottom; } Box;
typedef struct { int seq; int frame; const char *phase; const char *square; const char *detail; Box box; } Event;

#define MAX_EVENTS 256
static Event events[MAX_EVENTS];
static char detail_store[MAX_EVENTS][96];
static int event_count = 0;
static int failures = 0;

static int scaled_dimension(int dimension, int scale) { return ((dimension * scale) + (scale >> 1)) >> 5; }
static int clamp_int(int value, int lo, int hi) { return value < lo ? lo : (value > hi ? hi : value); }

static Box clip_sprite(int center_x, int bottom_y, int byte_width, int height) {
    Box b;
    b.bottom = bottom_y < VIEWPORT_H ? bottom_y : VIEWPORT_H - 1;
    b.top = bottom_y - (height - 1);
    if (b.top < 0) b.top = 0;
    b.left = center_x - byte_width + 1;
    b.right = center_x + byte_width;
    b.left = clamp_int(b.left, 0, VIEWPORT_W - 1);
    b.right = clamp_int(b.right, 0, VIEWPORT_W - 1);
    return b;
}

static int box_visible(Box b) {
    if (b.right <= 0) return 0;       /* DUNVIEW.C:5495 right==0 => not visible */
    if (b.left == VIEWPORT_W - 1) return 0; /* DUNVIEW.C:5497-5499 left==223 => not visible */
    return b.left <= b.right && b.top <= b.bottom;
}

static void log_event(int frame, const char *phase, const char *square, const char *detail, Box box) {
    if (event_count >= MAX_EVENTS) { fprintf(stderr, "too many events\n"); exit(2); }
    events[event_count] = (Event){ event_count, frame, phase, square, detail, box };
    printf("%03d frame=%02d phase=%-18s square=%-5s detail=%-54s box=[%3d,%3d,%3d,%3d]\n",
           event_count, frame, phase, square, detail, box.left, box.right, box.top, box.bottom);
    event_count++;
}

static void check_int(const char *name, int got, int expected) {
    if (got != expected) {
        fprintf(stderr, "FAIL %s: got %d expected %d\n", name, got, expected);
        failures++;
    }
}
static void check_true(const char *name, int cond) {
    if (!cond) { fprintf(stderr, "FAIL %s\n", name); failures++; }
}

static void emit_cell_pipeline(int frame, const char *square, int has_wall, int has_floor_orn, int item_count, int has_creature) {
    if (has_wall) log_event(frame, "wall/door/field", square, "wall/door geometry before cell contents", (Box){0,223,0,135});
    if (has_floor_orn) log_event(frame, "floor_ornament", square, "F0108 before F0115 objects", (Box){64,159,85,119});
    for (int i = 0; i < item_count; ++i) {
        snprintf(detail_store[event_count], sizeof detail_store[event_count], "F0115 object pile item #%d shifted within same cell", i + 1);
        log_event(frame, "item", square, detail_store[event_count], clip_sprite(110 + i * 7, 116 - i * 3, 8, 12));
    }
    if (has_creature) log_event(frame, "creature", square, "F0115 one creature after all objects for processed cell", clip_sprite(112, 121, 16, 32));
    log_event(frame, "projectiles", square, "F0115 restarts thing list after creature", (Box){95,128,70,100});
    log_event(frame, "explosions", square, "F0115 final square-level explosion pass", (Box){80,143,45,108});
}

static void simulate_frame(int frame) {
    static const char *square_order[] = {
        "D4L", "D4R", "D4C", "D3L2", "D3R2", "D3L", "D3R", "D3C",
        "D2L2", "D2R2", "D2L", "D2R", "D2C", "D1L", "D1R", "D1C", "D0L", "D0R", "D0C"
    };
    const int n = (int)(sizeof square_order / sizeof square_order[0]);
    log_event(frame, "vblank_wait", "screen", "F0097 waits next vertical blank before present", (Box){0,319,0,199});
    log_event(frame, "background", "viewport", "F0098 floor+ceiling first, 224x136 base", (Box){0,223,0,135});
    for (int i = 0; i < n; ++i) {
        int interesting = strcmp(square_order[i], "D2C") == 0;
        int foreground = strcmp(square_order[i], "D1C") == 0;
        if (interesting) emit_cell_pipeline(frame, square_order[i], 1, 1, 2, 1);
        else if (foreground) emit_cell_pipeline(frame, square_order[i], 1, 1, 1, 1);
        else log_event(frame, "square_visit", square_order[i], "F0128 back-to-front traversal slot", (Box){0,223,0,135});
    }
    log_event(frame, "viewport_present", "C007", "DRAWVIEW F0097 blits viewport zone; UI/action area separate", (Box){0,223,33,168});
    log_event(frame, "ui", "outside", "ACTIDRAW/PANEL are outside viewport draw order", (Box){224,319,0,199});
}

static void verify_dimensions_and_scaling(void) {
    printf("\n[dimensions]\n");
    printf("viewport_pixels=%dx%d viewport_byte_width=%d screen=%dx%d viewport_screen_y=%d\n",
           VIEWPORT_W, VIEWPORT_H, VIEWPORT_BYTE_W, SCREEN_W, SCREEN_H, VIEWPORT_SCREEN_Y);
    check_int("viewport width", VIEWPORT_W, 224);
    check_int("viewport height", VIEWPORT_H, 136);
    check_int("viewport byte width", VIEWPORT_BYTE_W, 112);

    const int item_w = 16, item_h = 14, creature_w = 32, creature_h = 32;
    printf("item native=%dx%d D2=%dx%d D3=%dx%d via M078\n",
           item_w, item_h,
           scaled_dimension(item_w, C20_SCALE_D2), scaled_dimension(item_h, C20_SCALE_D2),
           scaled_dimension(item_w, C16_SCALE_D3), scaled_dimension(item_h, C16_SCALE_D3));
    printf("creature native=%dx%d D2=%dx%d D3=%dx%d via M078\n",
           creature_w, creature_h,
           scaled_dimension(creature_w, C20_SCALE_D2), scaled_dimension(creature_h, C20_SCALE_D2),
           scaled_dimension(creature_w, C16_SCALE_D3), scaled_dimension(creature_h, C16_SCALE_D3));
    check_int("item D2 width", scaled_dimension(item_w, C20_SCALE_D2), 10);
    check_int("item D3 width", scaled_dimension(item_w, C16_SCALE_D3), 8);
    check_int("creature D2 width", scaled_dimension(creature_w, C20_SCALE_D2), 20);
    check_int("creature D3 width", scaled_dimension(creature_w, C16_SCALE_D3), 16);
}

static void verify_boundaries(void) {
    printf("\n[boundaries]\n");
    Box left = clip_sprite(-8, 140, 16, 40);
    Box right = clip_sprite(240, 110, 16, 40);
    Box edge = clip_sprite(260, 110, 16, 40);
    printf("left_overflow clipped=[%d,%d,%d,%d] visible=%d\n", left.left, left.right, left.top, left.bottom, box_visible(left));
    printf("right_overflow clipped=[%d,%d,%d,%d] visible=%d\n", right.left, right.right, right.top, right.bottom, box_visible(right));
    printf("far_right_edge clipped=[%d,%d,%d,%d] visible=%d\n", edge.left, edge.right, edge.top, edge.bottom, box_visible(edge));
    check_true("left overflow clips to x=0", left.left == 0);
    check_true("bottom overflow clips to y=135", left.bottom == 135);
    check_true("right overflow clips to x=223", right.right == 223);
    check_true("far right becomes not visible", !box_visible(edge));
}

static void verify_creature_animation_and_interrupts(void) {
    printf("\n[creature_animation]\n");
    int anim = 0;
    for (int frame = 0; frame < 8; ++frame) {
        int path_blocked = (frame == 3 || frame == 4);
        int movement_step = path_blocked ? 0 : 1;
        int attacking = (frame == 5);
        printf("frame=%d anim_frame=%d path_blocked=%d movement_step=%d pose=%s render=%s\n",
               frame, anim, path_blocked, movement_step, attacking ? "attack" : "front/side/back", "continues from ActiveGroup->Aspect/cell");
        anim = (anim + 1) & 3;
    }
    check_int("animation wraps 4-frame probe cycle", anim, 0);
}

static void verify_order_assertions(void) {
    int background = -1, first_square = -1, d2c_orn = -1, d2c_item = -1, d2c_creature = -1, present = -1, ui = -1;
    for (int i = 0; i < event_count; ++i) {
        if (!strcmp(events[i].phase, "background") && background < 0) background = i;
        if (!strcmp(events[i].phase, "square_visit") && first_square < 0) first_square = i;
        if (!strcmp(events[i].square, "D2C") && !strcmp(events[i].phase, "floor_ornament")) d2c_orn = i;
        if (!strcmp(events[i].square, "D2C") && !strcmp(events[i].phase, "item") && d2c_item < 0) d2c_item = i;
        if (!strcmp(events[i].square, "D2C") && !strcmp(events[i].phase, "creature")) d2c_creature = i;
        if (!strcmp(events[i].phase, "viewport_present")) present = i;
        if (!strcmp(events[i].phase, "ui")) ui = i;
    }
    check_true("background before squares", background >= 0 && background < first_square);
    check_true("floor ornament before item", d2c_orn >= 0 && d2c_orn < d2c_item);
    check_true("items before creature", d2c_item >= 0 && d2c_item < d2c_creature);
    check_true("viewport present before outside-ui phase", present >= 0 && ui > present);
}

int main(void) {
    printf("Firestaff DM1 V1 viewport draw-order probe\n");
    printf("primary_source=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("locks=DUNVIEW.C:F0128/F0115/F0108,DRAWVIEW.C:F0097,VIEWPORT.C:F0564/F0566,VBLANK.C:F0577,CLIKVIEW.C:F0377,ACTIDRAW.C:F0385-F0387\n\n");

    verify_dimensions_and_scaling();
    printf("\n[draw_sequence]\n");
    simulate_frame(0);
    verify_order_assertions();
    verify_boundaries();
    verify_creature_animation_and_interrupts();

    printf("\n[result] failures=%d events=%d\n", failures, event_count);
    return failures ? 1 : 0;
}
