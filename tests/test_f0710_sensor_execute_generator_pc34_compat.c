/*
 * Test F0710_SENSOR_Execute_Compat: generator sensor (type 6) emits
 * SENSOR_EXEC_EFFECT_GENERATOR with the sensor thing index.
 */

#include <stdio.h>
#include <string.h>
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } else { g_fail++; fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static void test_generator_sensor_emits_effect(void) {
    struct SensorOnSquare_Compat sensor;
    struct SensorEffectList_Compat effects;
    int ok;

    memset(&sensor, 0, sizeof(sensor));
    memset(&effects, 0, sizeof(effects));

    sensor.found = 1;
    sensor.sensorType = DM1_SENSOR_FLOOR_GROUP_GENERATOR;
    sensor.sensorIndex = 7;
    sensor.targetMapX = 3;
    sensor.targetMapY = 5;

    ok = F0710_SENSOR_Execute_Compat(NULL, NULL, &sensor,
                                     SENSOR_EVENT_WALK_ON, &effects);

    CHECK(ok == 1, "generator: returns success");
    CHECK(effects.count == 1, "generator: one effect emitted");
    CHECK(effects.effects[0].kind == SENSOR_EXEC_EFFECT_GENERATOR,
          "generator: kind is SENSOR_EXEC_EFFECT_GENERATOR");
    CHECK(effects.effects[0].sensorType == DM1_SENSOR_FLOOR_GROUP_GENERATOR,
          "generator: sensorType preserved");
    CHECK(effects.effects[0].textIndex == 7,
          "generator: textIndex carries sensorIndex");
    CHECK(effects.effects[0].destMapX == 3,
          "generator: destMapX preserved");
    CHECK(effects.effects[0].destMapY == 5,
          "generator: destMapY preserved");
}

static void test_generator_sensor_not_found(void) {
    struct SensorOnSquare_Compat sensor;
    struct SensorEffectList_Compat effects;

    memset(&sensor, 0, sizeof(sensor));
    memset(&effects, 0, sizeof(effects));

    sensor.found = 0;
    sensor.sensorType = DM1_SENSOR_FLOOR_GROUP_GENERATOR;

    F0710_SENSOR_Execute_Compat(NULL, NULL, &sensor,
                                SENSOR_EVENT_WALK_ON, &effects);
    CHECK(effects.count == 0, "not-found: no effects");
}

static void test_generator_null_safety(void) {
    int ok;
    ok = F0710_SENSOR_Execute_Compat(NULL, NULL, NULL,
                                     SENSOR_EVENT_WALK_ON, NULL);
    CHECK(ok == 0, "null: returns 0");
}

int main(void) {
    test_generator_sensor_emits_effect();
    test_generator_sensor_not_found();
    test_generator_null_safety();
    printf("test_f0710_sensor_execute_generator: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
