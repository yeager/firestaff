/* DM2 trigger fixture gate: original actuator records must own all targets. */
#include "dm2_v1_trigger.h"
#include <stdio.h>
#include <string.h>

static int failures;
static void check(int ok, const char *label)
{
    if (!ok) { fprintf(stderr, "FAIL: %s\n", label); ++failures; }
    else printf("PASS: %s\n", label);
}

int main(void)
{
    DM2_V1_TriggerEvent event;
    dm2_v1_trigger_reset_state();
    dm2_v1_trigger_set_now_ms(60000);
    check(dm2_v1_trigger_get_builtin_count() == 0,
          "fixture trigger catalog is unavailable without source records");
    check(dm2_v1_trigger_get_builtin(1) == NULL &&
          dm2_v1_trigger_lookup_index(1) == -1,
          "fixture trigger identity cannot be resolved");
    check(dm2_v1_trigger_fire(1) == DM2_TRIGGER_RESULT_NOT_FOUND &&
          dm2_v1_trigger_tick(60000) == 0 &&
          dm2_v1_trigger_signal_square_entered(15, 8, 0) == 0 &&
          dm2_v1_trigger_signal_item_used(1001) == 0 &&
          dm2_v1_trigger_signal_combat_ended(1) == 0,
          "fixture signals cannot produce target actions");
    check(dm2_v1_trigger_total_fires() == 0 &&
          dm2_v1_trigger_get_fire_count(1) == -1 &&
          !dm2_v1_trigger_copy_last_event(&event),
          "fixture trigger cannot create an event receipt");
    check(strstr(dm2_v1_trigger_source_evidence(),
                 "legacy fixture triggers and targets are no-op") != NULL,
          "source evidence declares closed trigger admission");
    return failures ? 1 : 0;
}
