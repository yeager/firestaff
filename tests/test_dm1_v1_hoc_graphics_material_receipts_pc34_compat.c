#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int m11_consumes_material_receipts(void)
{
    FILE *file = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char *source;
    long length;
    int ok;

    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0L, SEEK_SET) != 0 ||
        !(source = malloc((size_t)length + 1U))) {
        if (file) fclose(file);
        return 0;
    }
    if (fread(source, 1U, (size_t)length, file) != (size_t)length) {
        free(source);
        fclose(file);
        return 0;
    }
    source[length] = '\0';
    fclose(file);
    ok = strstr(source, "DM1_V1_ChampionMirror_ValidateHostMaterialReceiptPc34") &&
         strstr(source, "dm1_v1_f0115_floor_object_material_receipt_pc34") &&
         strstr(source, "materialReceipt.graphic_index");
    free(source);
    return ok;
}

int main(void)
{
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 wall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorHostDrawReceiptPc34 mirror;
    DM1_FrontMirrorRenderPlanPc34 mirror_plan;
    DM1_F0115FloorObjectMaterialReceiptPc34 floor;
    unsigned int graphic;

    if (!m11_consumes_material_receipts()) {
        fputs("M11 does not consume HoC GRAPHICS.DAT material receipts\n", stderr);
        return 1;
    }

    if (!DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 1, 1, &wall) ||
        !DM1_V1_ChampionMirror_BuildRenderReceiptPc34(&wall, &render) ||
        !DM1_V1_ChampionMirror_BuildSourceOwnedHostDrawReceiptPc34(
            &render, 0, 1, &mirror) ||
        !dm1_v1_front_mirror_render_plan_pc34(mirror.renderIndex, &mirror_plan) ||
        !DM1_V1_ChampionMirror_ValidateHostMaterialReceiptPc34(
            &mirror, mirror_plan.backingSourceWidth,
            mirror_plan.backingSourceHeight, 256, 87)) {
        fputs("valid C346/C026 HoC material receipt rejected\n", stderr);
        return 1;
    }
    mirror.backingGraphicIndex = 0;
    if (DM1_V1_ChampionMirror_ValidateHostMaterialReceiptPc34(
            &mirror, mirror_plan.backingSourceWidth,
            mirror_plan.backingSourceHeight, 256, 87)) {
        fputs("substitute C346 material was admitted\n", stderr);
        return 1;
    }

    graphic = dm1_item_sprite_index(THING_TYPE_WEAPON, 0);
    if (!dm1_v1_f0115_floor_object_material_receipt_pc34(
            THING_TYPE_WEAPON, 0, -1, 0, 10, 1, graphic, 32, 32, &floor) ||
        !floor.valid || floor.graphic_index != graphic ||
        floor.source_zone != 2500 || floor.source_zone_row != 0) {
        fputs("valid C2500 F0115 material receipt rejected\n", stderr);
        return 1;
    }
    if (dm1_v1_f0115_floor_object_material_receipt_pc34(
            THING_TYPE_WEAPON, 0, -1, 0, 10, 1, graphic + 1, 32, 32,
            &floor)) {
        fputs("substitute F0115 GRAPHICS.DAT material was admitted\n", stderr);
        return 1;
    }
    puts("ok: HoC C346/C026 and F0115 C2500 material receipts are source-bound");
    return 0;
}
