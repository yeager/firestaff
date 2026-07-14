#include "dm1_v1_sensor_get_object_of_type_in_cell_pc34_compat.h"

uint16_t F0273_SENSOR_GetObjectOfTypeInCell_Compat(
    const DM1_V1_SensorCellObjectPc34 *objects,
    size_t objectCount,
    int16_t cell,
    uint16_t objectType)
{
    size_t index;

    if (!objects) {
        return DM1_V1_SENSOR_THING_NONE_PC34;
    }

    for (index = 0; index < objectCount; ++index) {
        if (objects[index].objectType == objectType &&
            (cell == DM1_V1_SENSOR_CELL_ANY_PC34 || objects[index].cell == cell)) {
            return objects[index].thing;
        }
    }

    return DM1_V1_SENSOR_THING_NONE_PC34;
}
