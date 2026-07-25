#include "dm1_v1_sensor_get_object_of_type_in_cell_pc34_compat.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    const DM1_V1_SensorCellObjectPc34 objects[] = {
        { 0x0011u, 4u, 0 },
        { 0x0042u, 7u, 3 },
        { 0x0088u, 7u, 1 },
        { 0x00a0u, 7u, 3 }
    };
    (void)objects;

    assert(F0273_SENSOR_GetObjectOfTypeInCell_Compat(
               objects, sizeof(objects) / sizeof(objects[0]), 1, 7u) == 0x0088u);
    assert(F0273_SENSOR_GetObjectOfTypeInCell_Compat(
               objects, sizeof(objects) / sizeof(objects[0]),
               DM1_V1_SENSOR_CELL_ANY_PC34, 7u) == 0x0042u);
    assert(F0273_SENSOR_GetObjectOfTypeInCell_Compat(
               objects, sizeof(objects) / sizeof(objects[0]), 2, 7u) ==
           DM1_V1_SENSOR_THING_NONE_PC34);
    assert(F0273_SENSOR_GetObjectOfTypeInCell_Compat(NULL, 0, 0, 4u) ==
           DM1_V1_SENSOR_THING_NONE_PC34);

    puts("ok: DM1 F0273 typed cell object query");
    return 0;
}
