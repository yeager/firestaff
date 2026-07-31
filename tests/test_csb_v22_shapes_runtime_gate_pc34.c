/* Product V2.2 shape API must never expose the historical synthetic book. */
#include "csb_v22_shapes.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(expression, message) do { \
    if (!(expression)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

int main(void) {
    CSB_V22_ShapeParams cell = csb_v22_shape_for_cell(0x04, 0, 0, 0);
    CSB_V22_ShapeParams prison = csb_v22_shape_for_prison_door(50);
    CSB_V22_WallShape wall = csb_v22_wall_shape_get(CSB_V22_WALL_VARIANT_D0_CENTER);
    CSB_V22_FloorShape floor = csb_v22_floor_shape_get(0x04, 0);
    const char* evidence = csb_v22_shapes_source_evidence();

    csb_v22_shapes_init();
    CHECK(csb_v22_material_count() == 0, "product has no inferred materials");
    CHECK(csb_v22_material_get(0) == NULL, "default material is not substituted");
    CHECK(csb_v22_material_get(-1) == NULL, "invalid material is not substituted");
    CHECK(memcmp(&cell, &(CSB_V22_ShapeParams){0}, sizeof(cell)) == 0,
          "cell has no inferred shape parameters");
    CHECK(memcmp(&prison, &(CSB_V22_ShapeParams){0}, sizeof(prison)) == 0,
          "prison door has no inferred modern parameters");
    CHECK(memcmp(&wall, &(CSB_V22_WallShape){0}, sizeof(wall)) == 0,
          "wall has no inferred modern geometry");
    CHECK(memcmp(&floor, &(CSB_V22_FloorShape){0}, sizeof(floor)) == 0,
          "floor has no inferred modern geometry");
    CHECK(evidence != NULL && strstr(evidence, "No reviewed binding") != NULL,
          "evidence records the source-material boundary");

    if (failures) return 1;
    puts("CSB V2.2 product shape gate: PASS");
    return 0;
}
