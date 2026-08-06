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
    int item_ids[2] = {-1, -1};
    int quantities[2] = {-1, -1};

    memset(table, 0xA5, sizeof(table));
    nexus_gold_init();
    check(nexus_drops_for_type(0, table, 8) == 0,
          "unproven creature type has no admitted drop table");
    check(nexus_drops_roll(0, 7, 9, item_ids, quantities, 2) == 0,
          "unproven creature type does not materialize loot");
    check(nexus_gold_at(7, 9) == 0,
          "blocked loot roll does not manufacture a gold pile");

    check(nexus_gold_add(7, 9, 12) < 0,
          "unproven gold producer cannot add a pile");
    check(nexus_gold_at(7, 9) == 0,
          "blocked gold add leaves no pile");

    if (failures) {
        fprintf(stderr, "test_nexus_v1_drops_gate: %d failure(s)\n", failures);
        return 1;
    }
    puts("ok: Nexus drop gate (6 checks)");
    return 0;
}
