/* Pressure-plate admission probe: no representative catalog may affect DM2
 * before original dungeon sensor/actuator records are imported. */
#include "dm2_v1_pressure_plate.h"
#include <stdio.h>

int main(void)
{
    DM2_V1_PlateEvent event;
    int failures = 0;

    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_weight(500);
    dm2_v1_plate_set_party_position(12, 8, 0);

#define CHECK(condition, text) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", text); ++failures; } \
    else { printf("PASS: %s\n", text); } \
} while (0)
    CHECK(dm2_v1_plate_get_builtin_count() == 0,
          "no fixture pressure-plate catalog is admitted");
    CHECK(dm2_v1_plate_get_builtin(1) == NULL &&
          dm2_v1_plate_lookup_index(1) == -1,
          "fixture plate identity is unavailable");
    CHECK(dm2_v1_plate_check(1, 1000) == DM2_PLATE_RESULT_NOT_FOUND &&
          dm2_v1_plate_force_fire(1) == DM2_PLATE_RESULT_NOT_FOUND,
          "fixture activation and force-fire are rejected");
    CHECK(dm2_v1_plate_fire_total() == 0 &&
          !dm2_v1_plate_copy_last_event(&event),
          "fixture inputs cannot create an event");
#undef CHECK
    return failures ? 1 : 0;
}
