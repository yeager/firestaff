/*
 * dm2_v1_pressure_plate.c - DM2 V1 pressure-plate admission boundary
 *
 * SKProject resolves pressure plates from loaded dungeon record chains and
 * actuators (SKULLWIN/c_sensor.cpp and c_actuator.cpp).  Firestaff does not
 * yet own that decoded chain, so this legacy public API deliberately exposes
 * no plate, coordinate, message, target or state.  In particular, it must
 * not substitute a representative fixture catalog for original DM2 data.
 */

#include "dm2_v1_pressure_plate.h"

void dm2_v1_plate_reset_state(void) {}

void dm2_v1_plate_set_party_weight(int weight)
{
    (void)weight;
}

void dm2_v1_plate_set_party_position(int x, int y, int level)
{
    (void)x;
    (void)y;
    (void)level;
}

void dm2_v1_plate_set_item_on_floor(int item_id, int x, int y, int level)
{
    (void)item_id;
    (void)x;
    (void)y;
    (void)level;
}

int dm2_v1_plate_get_builtin_count(void) { return 0; }

const DM2_V1_PressurePlate *dm2_v1_plate_get_builtin(int plate_id)
{
    (void)plate_id;
    return NULL;
}

int dm2_v1_plate_lookup_index(int plate_id)
{
    (void)plate_id;
    return -1;
}

int dm2_v1_plate_activate(int plate_id)
{
    (void)plate_id;
    return (int)DM2_PLATE_RESULT_NOT_FOUND;
}

int dm2_v1_plate_deactivate(int plate_id)
{
    (void)plate_id;
    return (int)DM2_PLATE_RESULT_NOT_FOUND;
}

int dm2_v1_plate_check(int plate_id, int now_ms)
{
    (void)plate_id;
    (void)now_ms;
    return (int)DM2_PLATE_RESULT_NOT_FOUND;
}

int dm2_v1_plate_force_fire(int plate_id)
{
    (void)plate_id;
    return (int)DM2_PLATE_RESULT_NOT_FOUND;
}

int dm2_v1_plate_reset_fire_count(int plate_id)
{
    (void)plate_id;
    return (int)DM2_PLATE_RESULT_NOT_FOUND;
}

int dm2_v1_plate_get_state_for(int plate_id)
{
    (void)plate_id;
    return -1;
}

int dm2_v1_plate_get_fire_count(int plate_id)
{
    (void)plate_id;
    return -1;
}

int dm2_v1_plate_get_door_state_after_fire(int plate_id)
{
    (void)plate_id;
    return -1;
}

const DM2_V1_PlateState *dm2_v1_plate_get_state(int plate_id)
{
    (void)plate_id;
    return NULL;
}

const char *dm2_v1_plate_get_target_message(int plate_id)
{
    (void)plate_id;
    return NULL;
}

const DM2_V1_PlateEvent *dm2_v1_plate_last_event(void) { return NULL; }

int dm2_v1_plate_copy_last_event(DM2_V1_PlateEvent *out)
{
    (void)out;
    return 0;
}

int dm2_v1_plate_fire_total(void) { return 0; }
int dm2_v1_plate_active_count(void) { return 0; }

const char *dm2_v1_pressure_plate_source_evidence(void)
{
    return
        "DM2 V1 pressure-plate admission boundary\n"
        "Source: skproject/SKULLWIN/c_sensor.cpp (sensor record logic)\n"
        "Source: skproject/SKULLWIN/c_actuator.cpp (target actuation)\n"
        "Source: skproject/SKWIN/DME.h (pressure_plate_descriptor_t)\n"
        "Admission: no decoded source dungeon-record/actuator chain is bound;\n"
        "           no fixture plate, message, coordinate or target is retained.\n";
}
