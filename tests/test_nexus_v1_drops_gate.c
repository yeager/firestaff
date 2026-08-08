#include "nexus_v1_drops.h"
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

int main(void) {
    Nexus_DropEntry table[8];

    memset(table, 0, sizeof(table));
    nexus_gold_init();

    /* No retail Nexus death/drop owner is authenticated.  The former
     * creature-type gold formula was synthetic and must stay empty. */
    check(nexus_drops_for_type(0, table, 8) == 0,
          "unproven creature type has no drop entry");
    check(table[0].item_id == 0 && table[0].min_qty == 0 &&
              table[0].max_qty == 0 && table[0].chance == 0,
          "blocked drop table does not retain synthetic values");

    /* gold_add works */
    check(nexus_gold_add(7, 9, 12) >= 0,
          "gold add succeeds");
    check(nexus_gold_at(7, 9) == 12,
          "gold pile has correct amount");

    /* gold stacks at same position */
    check(nexus_gold_add(7, 9, 8) >= 0,
          "gold add stacks at same position");
    check(nexus_gold_at(7, 9) == 20,
          "gold pile stacked to 20");

    if (failures) {
        fprintf(stderr, "test_nexus_v1_drops_gate: %d failure(s)\n", failures);
        return 1;
    }
    puts("ok: Nexus drop system verified (7 checks)");
    return 0;
}
