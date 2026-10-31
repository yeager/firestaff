/* ReDMCSB MOVESENS.C F0276:1716-1723: DM1 floor version-gate sensor C009. */

#include <stdio.h>
#include <string.h>
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } else { g_fail++; fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static void test_version_gate_revert_effect(void) {
    struct DungeonSensor_Compat sourceSensor;
    struct DungeonThings_Compat things;
    struct SensorOnSquare_Compat sensor;
    struct SensorEffectList_Compat effects;

    memset(&sourceSensor, 0, sizeof(sourceSensor));
    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    sourceSensor.sensorType = DM1_SENSOR_FLOOR_VERSION_CHECKER;
    sourceSensor.sensorData = 21;
    sourceSensor.effect = DM1_EFFECT_SET;
    sourceSensor.revertEffect = 1;
    sourceSensor.value = 4;
    things.sensors = &sourceSensor;
    things.sensorCount = 1;

    sensor.found = 1;
    sensor.sensorIndex = 0;
    sensor.sensorType = DM1_SENSOR_FLOOR_VERSION_CHECKER;
    sensor.sensorData = sourceSensor.sensorData;
    sensor.effect = sourceSensor.effect;
    sensor.targetMapX = 3;
    sensor.targetMapY = 5;
    sensor.targetCell = 2;

    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1,
          "C009 inverted version gate: execution succeeds");
    CHECK(effects.count == 1,
          "C009 data 21 with RevertEffect triggers above the version threshold");
    CHECK(effects.count == 1 &&
              effects.effects[0].kind == SENSOR_EFFECT_TOGGLE_REMOTE &&
              effects.effects[0].textIndex == DM1_EFFECT_SET &&
              effects.effects[0].delayTicks == 4,
          "C009 inverted version gate preserves the source remote effect");

    sourceSensor.revertEffect = 0;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 0,
          "C009 data 21 without RevertEffect does not trigger");

    sourceSensor.sensorData = 20;
    sensor.sensorData = sourceSensor.sensorData;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 1,
          "C009 data 20 triggers for the DM1 V2.0 engine");

    sourceSensor.revertEffect = 1;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 0,
          "C009 RevertEffect inverts a matching version trigger");

    sourceSensor.revertEffect = 0;
    CHECK(F0710_SENSOR_Execute_Compat(NULL, &things, &sensor,
              SENSOR_EVENT_WALK_OFF, &effects) == 1 && effects.count == 0,
          "C009 does not trigger on party removal");

    CHECK(F0710_SENSOR_Execute_Compat(NULL, NULL, &sensor,
              SENSOR_EVENT_WALK_ON, &effects) == 1 && effects.count == 0,
          "C009 without its original sensor record does not guess an effect");
}

int main(void) {
    test_version_gate_revert_effect();
    printf("test_f0710_sensor_execute_version_gate: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
