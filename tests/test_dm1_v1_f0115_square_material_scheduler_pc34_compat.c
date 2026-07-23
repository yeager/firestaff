#include "dm1_v1_f0115_square_material_scheduler_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(expr) do { if (!(expr)) { ++failures; printf("FAIL %s\n", #expr); } } while (0)

int main(void)
{
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 plan;
    DM1_V1_F0115SquareMaterialSchedulePc34 schedule;
    unsigned char objectPixels[1] = { 1 }, projectilePixels[1] = { 2 }, explosionPixels[1] = { 3 };
    DM1_V1_F0115SquareMaterialPc34 materials[3] = {
        { DM1_V1_F0128_VIEW_SQUARE_D2C, DM1_V1_F0115_MATERIAL_EXPLOSION_PC34, 489, explosionPixels, 1, 1, 10 },
        { DM1_V1_F0128_VIEW_SQUARE_D2C, DM1_V1_F0115_MATERIAL_PROJECTILE_PC34, 454, projectilePixels, 1, 1, 10 },
        { DM1_V1_F0128_VIEW_SQUARE_D2C, DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34, 210, objectPixels, 1, 1, 10 }
    };
    int i;
    for (i = 0; i < DM1_V1_F0128_VIEW_SQUARE_COUNT; ++i) {
        squares[i].element = DM1_V1_F0128_ELEMENT_CORRIDOR;
        squares[i].pitOrTeleporterVisible = 0;
        squares[i].frontWallOrnamentIsAlcove = 0;
        squares[i].hasFloorOrnament = 0;
    }
    CHECK(DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares, &plan));
    CHECK(dm1_v1_f0115_square_material_schedule_pc34(&plan, materials, 3, &schedule));
    CHECK(schedule.valid && schedule.materialCount == 3);
    CHECK(schedule.materials[0].kind == DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34);
    CHECK(schedule.materials[1].kind == DM1_V1_F0115_MATERIAL_PROJECTILE_PC34);
    CHECK(schedule.materials[2].kind == DM1_V1_F0115_MATERIAL_EXPLOSION_PC34);
    materials[1].pixels = NULL;
    CHECK(!dm1_v1_f0115_square_material_schedule_pc34(&plan, materials, 3, &schedule));
    CHECK(strstr(dm1_v1_f0115_square_material_scheduler_source_pc34(), "5645") != NULL);
    return failures ? 1 : 0;
}
