#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    PIXEL_OWNER_BACKGROUND = 0,
    PIXEL_OWNER_D0L_SIDE_CONTENT = 1,
    PIXEL_OWNER_D0R_SIDE_CONTENT = 2
} PixelOwner;

enum {
    VIEW_CELL_COUNT = 5
};

static int failures = 0;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", id, got, want);
        ++failures;
    }
}

static void check_bool(const char *id, bool got, bool want)
{
    check_int(id, got ? 1 : 0, want ? 1 : 0);
}

static void check_contains(const char *id, const char *text, const char *needle)
{
    check_bool(id, text && needle && strstr(text, needle), true);
}

static void paint_cells(uint8_t pixels[VIEW_CELL_COUNT],
                        DM1_ViewportCellOrder order,
                        PixelOwner owner)
{
    for (int i = 0; i < order.cell_count; ++i) {
        unsigned char cell = order.cells[i];
        if (cell > 0 && cell < VIEW_CELL_COUNT) {
            pixels[cell] = (uint8_t)owner;
        }
    }
}

static void verify_d0_side_door_cell(const DM1_ViewportSideOcclusionSpec *spec,
                                     DM1_ViewSquareIndex square,
                                     uint16_t expected_order,
                                     unsigned char expected_cell,
                                     PixelOwner owner,
                                     const char *name,
                                     const char *branch_line,
                                     const char *f0115_line)
{
    uint8_t pixels[VIEW_CELL_COUNT] = {
        PIXEL_OWNER_BACKGROUND,
        PIXEL_OWNER_BACKGROUND,
        PIXEL_OWNER_BACKGROUND,
        PIXEL_OWNER_BACKGROUND,
        PIXEL_OWNER_BACKGROUND
    };
    DM1_ViewportCellOrder order;
    char id[128];

    snprintf(id, sizeof(id), "%s.nonnull", name);
    check_bool(id, spec != NULL, true);
    if (!spec) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0125/F0126 route near side-door and teleporter
     * cells directly to F0115 with one mirrored back cell: D0L uses
     * C0x0002_CELL_ORDER_BACKRIGHT at line 8005, while D0R uses
     * C0x0001_CELL_ORDER_BACKLEFT at line 8115. */
    snprintf(id, sizeof(id), "%s.square", name);
    check_int(id, (int)spec->square, (int)square);
    snprintf(id, sizeof(id), "%s.order", name);
    check_int(id, spec->cell_order, expected_order);
    snprintf(id, sizeof(id), "%s.branch_source", name);
    check_contains(id, spec->branch_source_lines, branch_line);
    snprintf(id, sizeof(id), "%s.f0115_source", name);
    check_contains(id, spec->f0115_source_lines, f0115_line);

    order = dm1_viewport_3d_decode_cell_order(spec->cell_order);
    snprintf(id, sizeof(id), "%s.no_door_pass", name);
    check_int(id, order.door_pass, 0);
    snprintf(id, sizeof(id), "%s.cell_count", name);
    check_int(id, order.cell_count, 1);
    snprintf(id, sizeof(id), "%s.cell0", name);
    check_int(id, order.cells[0], expected_cell);
    snprintf(id, sizeof(id), "%s.not_alcove_order", name);
    check_bool(id, order.alcove, false);

    paint_cells(pixels, order, owner);
    snprintf(id, sizeof(id), "%s.expected_cell_owner", name);
    check_int(id, pixels[expected_cell], owner);

    for (unsigned char cell = 1; cell < VIEW_CELL_COUNT; ++cell) {
        if (cell == expected_cell) {
            continue;
        }
        snprintf(id, sizeof(id), "%s.cell_%u_background", name, cell);
        check_int(id, pixels[cell], PIXEL_OWNER_BACKGROUND);
    }
}

int main(void)
{
    const DM1_ViewportSideOcclusionSpec *d0l =
        dm1_viewport_3d_get_side_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0L);
    const DM1_ViewportSideOcclusionSpec *d0r =
        dm1_viewport_3d_get_side_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0R);

    printf("probe=firestaff_dm1_v1_d0_side_door_cell_pixel_gate\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceLocks=F0125:8000-8005,F0126:8110-8115\n");
    printf("claim=synthetic pixel-cell gate for D0L/D0R side-door F0115 order; no screenshot parity claim\n");

    check_int("side_door.spec_count", (int)dm1_viewport_3d_side_occlusion_spec_count(), 8);
    verify_d0_side_door_cell(d0l,
                             DM1_VIEW_SQUARE_D0L,
                             0x0002,
                             2,
                             PIXEL_OWNER_D0L_SIDE_CONTENT,
                             "d0l_side_door",
                             "8000-8005",
                             "8005");
    verify_d0_side_door_cell(d0r,
                             DM1_VIEW_SQUARE_D0R,
                             0x0001,
                             1,
                             PIXEL_OWNER_D0R_SIDE_CONTENT,
                             "d0r_side_door",
                             "8110-8115",
                             "8115");

    if (d0l && d0r) {
        DM1_ViewportCellOrder left = dm1_viewport_3d_decode_cell_order(d0l->cell_order);
        DM1_ViewportCellOrder right = dm1_viewport_3d_decode_cell_order(d0r->cell_order);
        check_bool("side_door.mirrored_cells", left.cells[0] != right.cells[0], true);
        check_int("side_door.left_backright", left.cells[0], 2);
        check_int("side_door.right_backleft", right.cells[0], 1);
    }
    check_bool("side_door.no_d0c_side_spec",
               dm1_viewport_3d_get_side_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C) != NULL,
               false);

    if (failures) {
        fprintf(stderr, "result=fail failures=%d\n", failures);
        return 1;
    }
    printf("result=pass\n");
    return 0;
}
