/* Trigger admission probe: no representative trigger may affect DM2 before
 * original actuator/record-chain ownership is imported. */
#include "dm2_v1_trigger.h"
#include <stdio.h>

int main(void)
{
    DM2_V1_TriggerEvent event;
    int failures = 0;
    dm2_v1_trigger_reset_state();
    dm2_v1_trigger_set_now_ms(60000);

#define CHECK(condition, text) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", text); ++failures; } \
    else { printf("PASS: %s\n", text); } \
} while (0)
    CHECK(dm2_v1_trigger_get_builtin_count() == 0,
          "no fixture trigger catalog is admitted");
    CHECK(dm2_v1_trigger_get_builtin(1) == NULL &&
          dm2_v1_trigger_lookup_index(1) == -1,
          "fixture trigger identity is unavailable");
    CHECK(dm2_v1_trigger_fire(1) == DM2_TRIGGER_RESULT_NOT_FOUND &&
          dm2_v1_trigger_tick(60000) == 0 &&
          dm2_v1_trigger_signal_square_entered(15, 8, 0) == 0 &&
          dm2_v1_trigger_signal_item_used(1001) == 0 &&
          dm2_v1_trigger_signal_combat_ended(1) == 0,
          "fixture triggers cannot create actions");
    CHECK(dm2_v1_trigger_total_fires() == 0 &&
          !dm2_v1_trigger_copy_last_event(&event),
          "fixture triggers cannot create a receipt");
#undef CHECK
    return failures ? 1 : 0;
}
