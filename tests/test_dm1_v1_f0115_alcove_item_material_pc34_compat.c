#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>

static int failures;

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL %s got=%d expected=%d\n", name, actual, expected);
        ++failures;
    }
}

int main(void)
{
    DM1_F0115AlcoveItemMaterialPlanPc34 chest;
    DM1_F0115AlcoveItemMaterialPlanPc34 scroll;

    expect_int("chest.plan",
               dm1_v1_f0115_alcove_item_material_plan_pc34(
                   &chest, 9, 0, 1, 0, 2), 1);
    expect_int("chest.aspect", chest.aspect_index, 0);
    expect_int("chest.alcove_graphic", (int)chest.graphic_index, 499);
    expect_int("chest.alcove_variant", chest.use_alcove_object_image, 1);
    expect_int("chest.g2029_d1c", chest.alcove_view_row, 6);
    expect_int("chest.c2548", chest.source_zone, 2554);
    expect_int("chest.anchor_x", chest.anchor_x, 115);
    expect_int("chest.anchor_y", chest.anchor_y, 92);
    expect_int("chest.clip_w", chest.clip_w, 224);
    expect_int("chest.clip_h", chest.clip_h, 136);
    expect_int("chest.c10", chest.transparent_color, 10);
    expect_int("chest.c2548_bound", chest.coordinate_binding_ready, 1);

    {
        DM1_F0115FloorObjectMaterialReceiptPc34 receipt;
        expect_int("chest.alcove_receipt",
                   dm1_v1_f0115_floor_object_material_receipt_pc34(
                       THING_TYPE_CONTAINER, 0, chest.source_zone,
                       chest.source_zone, chest.transparent_color, 1,
                       chest.graphic_index, 32, 32, &receipt), 1);
        expect_int("chest.alcove_receipt_graphic",
                   (int)receipt.graphic_index, (int)chest.graphic_index);
        expect_int("chest.floor_bitmap_rejected_in_alcove",
                   dm1_v1_f0115_floor_object_material_receipt_pc34(
                       THING_TYPE_CONTAINER, 0, chest.source_zone,
                       chest.source_zone, chest.transparent_color, 1,
                       chest.graphic_index - 1, 32, 32, &receipt), 0);
    }

    expect_int("scroll.plan",
               dm1_v1_f0115_alcove_item_material_plan_pc34(
                   &scroll, 7, 0, 3, 0, 2), 1);
    expect_int("scroll.native_graphic", (int)scroll.graphic_index, 500);
    expect_int("scroll.no_alcove_variant", scroll.use_alcove_object_image, 0);
    expect_int("scroll.g2029_d3c", scroll.alcove_view_row, 0);
    expect_int("scroll.c2548", scroll.source_zone, 2555);
    expect_int("scroll.anchor_x", scroll.anchor_x, 112);
    expect_int("scroll.anchor_y", scroll.anchor_y, 60);

    expect_int("reject_wrong_cell",
               dm1_v1_f0115_alcove_item_material_plan_pc34(
                   &scroll, 7, 0, 1, 0, 3), 0);
    expect_int("reject_d0c",
               dm1_v1_f0115_alcove_item_material_plan_pc34(
                   &scroll, 7, 0, 0, 0, 2), 0);
    expect_int("reject_non_item",
               dm1_v1_f0115_alcove_item_material_plan_pc34(
                   &scroll, 1, 0, 1, 0, 2), 0);

    if (failures) {
        return 1;
    }
    printf("PASS dm1_v1_f0115_alcove_item_material_pc34_compat\n");
    return 0;
}
