#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_EMPTY = 0,
    PIXEL_REAR_THING = 1,
    PIXEL_DOOR_MASK = 2,
    PIXEL_FRONT_THING = 3,
    CELL_COUNT = 5
};

static int failures = 0;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void check_contains(const char *id, const char *text, const char *needle)
{
    check_int(id, text && needle && strstr(text, needle) ? 1 : 0, 1);
}

static void paint_cells(uint8_t pixels[CELL_COUNT], DM1_ViewportCellOrder order, uint8_t value)
{
    for (int i = 0; i < order.cell_count; ++i) {
        unsigned char cell = order.cells[i];
        if (cell > 0 && cell < CELL_COUNT) {
            pixels[cell] = value;
        }
    }
}

static int order_contains_cell(DM1_ViewportCellOrder order, unsigned char cell)
{
    for (int i = 0; i < order.cell_count; ++i) {
        if (order.cells[i] == cell) {
            return 1;
        }
    }
    return 0;
}

static void paint_door_mask(uint8_t pixels[CELL_COUNT],
                            DM1_ViewportCellOrder rear,
                            DM1_ViewportCellOrder front)
{
    paint_cells(pixels, rear, PIXEL_DOOR_MASK);
    paint_cells(pixels, front, PIXEL_DOOR_MASK);
}

static void verify_spec_pixels(const DM1_ViewportDoorFrontOcclusionSpec *spec, size_t index)
{
    uint8_t pixels[CELL_COUNT] = {0, 0, 0, 0, 0};
    DM1_ViewportCellOrder rear;
    DM1_ViewportCellOrder front;
    char id[128];

    if (!spec) {
        snprintf(id, sizeof(id), "door_pixel.%zu.spec", index);
        check_int(id, 0, 1);
        return;
    }

    rear = dm1_viewport_3d_decode_cell_order(spec->rear_cell_order);
    front = dm1_viewport_3d_decode_cell_order(spec->front_cell_order);

    snprintf(id, sizeof(id), "door_pixel.%zu.rear_pass", index);
    check_int(id, rear.door_pass, 1);
    snprintf(id, sizeof(id), "door_pixel.%zu.front_pass", index);
    check_int(id, front.door_pass, 2);
    snprintf(id, sizeof(id), "door_pixel.%zu.disjoint_cell_1", index);
    check_int(id, !(order_contains_cell(rear, 1) && order_contains_cell(front, 1)), 1);
    snprintf(id, sizeof(id), "door_pixel.%zu.disjoint_cell_2", index);
    check_int(id, !(order_contains_cell(rear, 2) && order_contains_cell(front, 2)), 1);
    snprintf(id, sizeof(id), "door_pixel.%zu.disjoint_cell_3", index);
    check_int(id, !(order_contains_cell(rear, 3) && order_contains_cell(front, 3)), 1);
    snprintf(id, sizeof(id), "door_pixel.%zu.disjoint_cell_4", index);
    check_int(id, !(order_contains_cell(rear, 4) && order_contains_cell(front, 4)), 1);

    paint_cells(pixels, rear, PIXEL_REAR_THING);
    paint_door_mask(pixels, rear, front);
    for (int i = 0; i < rear.cell_count; ++i) {
        unsigned char cell = rear.cells[i];
        if (cell > 0 && cell < CELL_COUNT) {
            snprintf(id, sizeof(id), "door_pixel.%zu.rear_cell_%u_occluded_by_door", index, cell);
            check_int(id, pixels[cell], PIXEL_DOOR_MASK);
        }
    }

    paint_cells(pixels, front, PIXEL_FRONT_THING);
    for (int i = 0; i < rear.cell_count; ++i) {
        unsigned char cell = rear.cells[i];
        if (cell > 0 && cell < CELL_COUNT) {
            snprintf(id, sizeof(id), "door_pixel.%zu.rear_cell_%u_stays_hidden", index, cell);
            check_int(id, pixels[cell], PIXEL_DOOR_MASK);
        }
    }
    for (int i = 0; i < front.cell_count; ++i) {
        unsigned char cell = front.cells[i];
        if (cell > 0 && cell < CELL_COUNT) {
            snprintf(id, sizeof(id), "door_pixel.%zu.front_cell_%u_drawn_after_door", index, cell);
            check_int(id, pixels[cell], PIXEL_FRONT_THING);
        }
    }

    snprintf(id, sizeof(id), "door_pixel.%zu.floor_source", index);
    check_contains(id, spec->floor_source_lines, "DUNVIEW.C:");
    snprintf(id, sizeof(id), "door_pixel.%zu.rear_source", index);
    check_contains(id, spec->rear_pass_source_lines, "before");
    snprintf(id, sizeof(id), "door_pixel.%zu.frame_source", index);
    check_contains(id, spec->frame_source_lines, "frame");
    snprintf(id, sizeof(id), "door_pixel.%zu.door_source", index);
    check_contains(id, spec->door_source_lines, "F0111");
    snprintf(id, sizeof(id), "door_pixel.%zu.front_source", index);
    check_contains(id, spec->front_pass_source_lines, "after");
}

int main(void)
{
    size_t count = dm1_viewport_3d_door_front_occlusion_spec_count();

    /* ReDMCSB source anchors:
     * DUNVIEW.C F0116/F0117/F0118/F0119/F0120/F0121/F0122/F0123/F0124
     * draw rear F0115 cells before frame/F0111 door pixels and draw front
     * F0115 cells after those door pixels.  This synthetic pixel-zone gate
     * guards the ordering contract without claiming original framebuffer
     * parity or using bundled retail graphics. */
    check_int("door_pixel.spec_count", (int)count, 11);
    for (size_t i = 0; i < count; ++i) {
        verify_spec_pixels(dm1_viewport_3d_get_door_front_occlusion_spec(i), i);
    }
    check_int("door_pixel.out_of_range_null",
              dm1_viewport_3d_get_door_front_occlusion_spec(count) == NULL, 1);

    if (failures) {
        printf("FAIL dm1_v1_door_occlusion_pixel_gate failures=%d\n", failures);
        return 1;
    }
    printf("PASS dm1_v1_door_occlusion_pixel_gate\n");
    return 0;
}
