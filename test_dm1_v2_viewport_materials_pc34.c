#include "dm1_v2_viewport_renderer_pc34.h"

#include <stdio.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void test_source_locked_dimensions_and_bands(void) {
    CHECK(dm1_v2_vp_source_width() == 224);
    CHECK(dm1_v2_vp_source_height() == 136);
    CHECK(dm1_v2_vp_source_byte_width() == 112);

    CHECK(dm1_v2_vp_material_at(-1, 0) == DM1_V2_VIEW_MATERIAL_OUT_OF_BOUNDS);
    CHECK(dm1_v2_vp_material_at(224, 0) == DM1_V2_VIEW_MATERIAL_OUT_OF_BOUNDS);
    CHECK(dm1_v2_vp_material_at(0, 136) == DM1_V2_VIEW_MATERIAL_OUT_OF_BOUNDS);

    /* ReDMCSB DUNVIEW.C:2968-2971: 224x29 ceiling, 37-line black area,
     * floor starts at line 66 and spans 70 lines. */
    CHECK(dm1_v2_vp_material_at(0, 0) == DM1_V2_VIEW_MATERIAL_CEILING);
    CHECK(dm1_v2_vp_material_at(223, 28) == DM1_V2_VIEW_MATERIAL_CEILING);
    CHECK(dm1_v2_vp_material_at(0, 29) == DM1_V2_VIEW_MATERIAL_BLACK);
    CHECK(dm1_v2_vp_material_at(223, 36) == DM1_V2_VIEW_MATERIAL_BLACK);
    CHECK(dm1_v2_vp_material_at(112, 37) == DM1_V2_VIEW_MATERIAL_WALL);
    CHECK(dm1_v2_vp_material_at(112, 65) == DM1_V2_VIEW_MATERIAL_WALL);
    CHECK(dm1_v2_vp_material_at(0, 66) == DM1_V2_VIEW_MATERIAL_FLOOR);
    CHECK(dm1_v2_vp_material_at(223, 135) == DM1_V2_VIEW_MATERIAL_FLOOR);
}

static void test_field_aspects_and_wall_defaults(void) {
    const DM1_V2_FieldAspect *d0c = dm1_v2_vp_field_aspect(DM1_V2_FIELD_D0C);
    const DM1_V2_FieldAspect *d0l = dm1_v2_vp_field_aspect(DM1_V2_FIELD_D0L);
    const DM1_V2_FieldAspect *d2l = dm1_v2_vp_field_aspect(DM1_V2_FIELD_D2L);

    CHECK(dm1_v2_vp_field_aspect((DM1_V2_FieldAspectId)-1) == NULL);
    CHECK(dm1_v2_vp_field_aspect((DM1_V2_FieldAspectId)DM1_V2_FIELD_ASPECT_COUNT) == NULL);

    /* ReDMCSB DUNVIEW.C G0188_aauc_Graphic558_FieldAspects, PC MEDIA488 row. */
    CHECK(d0c != NULL);
    CHECK(d0c->baseStartUnitIndex == 59);
    CHECK(d0c->transparentColor == 0x8A);
    CHECK(d0c->mask == 0xFF);
    CHECK(d0c->byteWidth == 224);
    CHECK(d0c->height == 136);

    CHECK(d0l != NULL);
    CHECK(d0l->mask == 0x83);
    CHECK(d0l->byteWidth == 32);
    CHECK(d0l->height == 136);

    CHECK(d2l != NULL);
    CHECK(d2l->mask == 0x81);
    CHECK(d2l->byteWidth == 80);
    CHECK(d2l->height == 71);
    CHECK(d2l->x == 5);

    /* ReDMCSB DUNVIEW.C G2107_WallSet I34E row, also mirrored in V1 viewport init. */
    static const int expected[DM1_V2_WALL_SET_COUNT] = {
        -17, -16, -15, -14, -13, -9, -8, -12, -11, -10, -4, -3, -7, -6, -5
    };
    for (int i = 0; i < DM1_V2_WALL_SET_COUNT; i++) {
        CHECK(dm1_v2_vp_wall_set_default(i) == expected[i]);
    }
    CHECK(dm1_v2_vp_wall_set_default(-1) == 0);
    CHECK(dm1_v2_vp_wall_set_default(DM1_V2_WALL_SET_COUNT) == 0);
}

int main(void) {
    test_source_locked_dimensions_and_bands();
    test_field_aspects_and_wall_defaults();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_viewport_materials_pc34: ok");
    return 0;
}
