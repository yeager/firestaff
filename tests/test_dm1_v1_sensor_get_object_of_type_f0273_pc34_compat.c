#include "dm1_v1_sensor_get_object_of_type_in_cell_pc34_compat.h"

#include <assert.h>
#include <stdio.h>

enum {
    kThingDoorButton = 0x1122,
    kThingFloorSensor = 0x2233,
    kThingSecondFloorSensor = 0x3344,
    kThingWallSensor = 0x4455,
    kObjectTypeFloorSensor = 4,
    kObjectTypeWallSensor = 5
};

static const DM1_V1_SensorCellObjectPc34 kSquareObjects[] = {
    {kThingDoorButton, kObjectTypeWallSensor, 2},
    {kThingFloorSensor, kObjectTypeFloorSensor, 1},
    {kThingSecondFloorSensor, kObjectTypeFloorSensor, 3},
    {kThingWallSensor, kObjectTypeWallSensor, 1}
};
(void)kSquareObjects;

static void test_source_named_boundary_returns_first_matching_object(void)
{
    assert(F0273_SENSOR_GetObjectOfTypeInCell(
               kSquareObjects, 4, 1, kObjectTypeFloorSensor) ==
           kThingFloorSensor);
}

static void test_cell_any_preserves_traversal_order(void)
{
    assert(F0273_SENSOR_GetObjectOfTypeInCell(
               kSquareObjects,
               4,
               DM1_V1_SENSOR_CELL_ANY_PC34,
               kObjectTypeWallSensor) == kThingDoorButton);
}

static void test_cell_and_type_are_both_required(void)
{
    assert(F0273_SENSOR_GetObjectOfTypeInCell(
               kSquareObjects, 4, 3, kObjectTypeFloorSensor) ==
           kThingSecondFloorSensor);
    assert(F0273_SENSOR_GetObjectOfTypeInCell(
               kSquareObjects, 4, 3, kObjectTypeWallSensor) ==
           DM1_V1_SENSOR_THING_NONE_PC34);
}

static void test_compat_boundary_delegates_to_source_named_boundary(void)
{
    assert(F0273_SENSOR_GetObjectOfTypeInCell_Compat(
               kSquareObjects, 4, 1, kObjectTypeWallSensor) ==
           kThingWallSensor);
}

static void test_invalid_or_empty_inputs_return_none(void)
{
    assert(F0273_SENSOR_GetObjectOfTypeInCell(
               NULL, 4, 1, kObjectTypeFloorSensor) ==
           DM1_V1_SENSOR_THING_NONE_PC34);
    assert(F0273_SENSOR_GetObjectOfTypeInCell(
               kSquareObjects, 0, 1, kObjectTypeFloorSensor) ==
           DM1_V1_SENSOR_THING_NONE_PC34);
}

int main(void)
{
    test_source_named_boundary_returns_first_matching_object();
    test_cell_any_preserves_traversal_order();
    test_cell_and_type_are_both_required();
    test_compat_boundary_delegates_to_source_named_boundary();
    test_invalid_or_empty_inputs_return_none();

    puts("ok: DM1 F0273 sensor object-of-type cell callable");
    return 0;
}
