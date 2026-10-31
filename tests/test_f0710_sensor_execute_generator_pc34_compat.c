/* ReDMCSB MOVESENS.C F0276:1704-1705: a floor C006 is not walk-triggered. */

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

static void test_generator_sensor_does_not_fire_on_party_walk_on(void) {
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
    CHECK(effects.count == 0,
          "C006 floor sensor: party walk-on emits no effect");
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

static void test_version_checker_uses_dm1_v20_gate(void) {
    struct SensorOnSquare_Compat sensor;
    struct SensorEffectList_Compat effects;
    struct DungeonSensor_Compat sourceSensor;
    struct DungeonThings_Compat things;

    memset(&sensor, 0, sizeof(sensor));
    memset(&sourceSensor, 0, sizeof(sourceSensor));
    memset(&things, 0, sizeof(things));
    sourceSensor.sensorType = DM1_SENSOR_FLOOR_VERSION_CHECKER;
    sourceSensor.effect = DM1_EFFECT_TOGGLE;
    things.sensors = &sourceSensor;
    things.sensorCount = 1;
    sensor.found = 1;
    sensor.sensorType = DM1_SENSOR_FLOOR_VERSION_CHECKER;
    sensor.sensorIndex = 0;
    sensor.targetMapX = 3;
    sensor.targetMapY = 5;

    sourceSensor.sensorData = 20u;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 1 &&
              effects.effects[0].kind == SENSOR_EFFECT_TOGGLE_REMOTE,
          "C009 data 20 triggers for DM1 V2.0");

    sourceSensor.sensorData = 21u;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 0,
          "C009 data above 20 does not trigger for DM1 V2.0");

    sourceSensor.revertEffect = 1;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 1 &&
              effects.effects[0].kind == SENSOR_EFFECT_TOGGLE_REMOTE,
          "C009 inverted data above 20 triggers through the source record");
}

static void test_source_sensor_type_meanings(void) {
    struct SensorOnSquare_Compat sensor;
    struct SensorEffectList_Compat effects;

    memset(&sensor, 0, sizeof(sensor));
    sensor.found = 1;

    sensor.sensorType = 0;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, NULL, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 0,
          "C000 disabled sensor does not teleport");

    sensor.sensorType = 13;
    sensor.sensorData = 7u;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, NULL, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 1 &&
              effects.effects[0].kind == SENSOR_EFFECT_UNSUPPORTED,
          "wall C013 is not dispatched as a text sensor");
}

int main(void) {
    test_generator_sensor_does_not_fire_on_party_walk_on();
    test_generator_sensor_not_found();
    test_generator_null_safety();
    test_version_checker_uses_dm1_v20_gate();
    test_source_sensor_type_meanings();
    printf("test_f0710_sensor_execute_generator: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
