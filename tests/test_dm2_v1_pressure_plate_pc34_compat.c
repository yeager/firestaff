/* DM2 pressure-plate fixture gate: original dungeon record ownership is
 * required before a plate can affect the runtime. */
#include "dm2_v1_pressure_plate.h"
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    } else {
        printf("PASS: %s\n", label);
    }
}

int main(void)
{
    DM2_V1_PlateEvent event;
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_weight(500);
    dm2_v1_plate_set_party_position(12, 8, 0);
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);

    check(dm2_v1_plate_get_builtin_count() == 0,
          "fixture plate catalog is unavailable without source records");
    check(dm2_v1_plate_get_builtin(1) == NULL &&
          dm2_v1_plate_lookup_index(1) == -1,
          "fixture plate identity cannot be resolved");
    check(dm2_v1_plate_check(1, 1000) == DM2_PLATE_RESULT_NOT_FOUND &&
          dm2_v1_plate_force_fire(1) == DM2_PLATE_RESULT_NOT_FOUND,
          "weight and force-fire routes reject fixture plate");
    check(dm2_v1_plate_get_fire_count(1) == -1 &&
          dm2_v1_plate_fire_total() == 0 &&
          dm2_v1_plate_active_count() == 0,
          "fixture input cannot create a plate event or active state");
    check(dm2_v1_plate_get_target_message(3) == NULL &&
          dm2_v1_plate_get_door_state_after_fire(1) == -1 &&
          !dm2_v1_plate_copy_last_event(&event),
          "fixture message and door targets remain unavailable");
    check(strstr(dm2_v1_pressure_plate_source_evidence(),
                 "no fixture plate, message, coordinate or target is retained") != NULL,
          "source evidence declares that fixture data is removed");

    return failures ? 1 : 0;
}
