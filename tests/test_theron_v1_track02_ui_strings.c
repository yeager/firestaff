#include "theron_v1_track02_ui_strings.h"
#include <stdio.h>
#include <string.h>

#define ASSERT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 0; } } while (0)
#define TEST(name) printf("  %-60s", name)
#define PASS() do { printf("PASS\n"); return 1; } while (0)

static int test_flask_states(void) {
    TEST("4 flask states (EMPTY through FULL)");
    ASSERT(strcmp(theron_v1_track02_us_flask_state(0), "(EMPTY)") == 0, "0");
    ASSERT(strcmp(theron_v1_track02_us_flask_state(1), "(ALMOST EMPTY)") == 0, "1");
    ASSERT(strcmp(theron_v1_track02_us_flask_state(2), "(ALMOST FULL)") == 0, "2");
    ASSERT(strcmp(theron_v1_track02_us_flask_state(3), "(FULL)") == 0, "3");
    ASSERT(theron_v1_track02_us_flask_state(4) == NULL, "out of range");
    PASS();
}

static int test_directions(void) {
    TEST("4 compass directions");
    ASSERT(strcmp(theron_v1_track02_us_direction_name(0), "NORTH") == 0, "N");
    ASSERT(strcmp(theron_v1_track02_us_direction_name(1), "EAST") == 0, "E");
    ASSERT(strcmp(theron_v1_track02_us_direction_name(2), "SOUTH") == 0, "S");
    ASSERT(strcmp(theron_v1_track02_us_direction_name(3), "WEST") == 0, "W");
    ASSERT(theron_v1_track02_us_direction_name(4) == NULL, "out of range");
    PASS();
}

static int test_item_statuses(void) {
    TEST("Item statuses (POISONED/BROKEN/CURSED)");
    ASSERT(strcmp(theron_v1_track02_us_item_status(0), "POISONED") == 0, "0");
    ASSERT(strcmp(theron_v1_track02_us_item_status(1), "BROKEN") == 0, "1");
    ASSERT(strcmp(theron_v1_track02_us_item_status(2), "CURSED") == 0, "2");
    ASSERT(theron_v1_track02_us_item_status(3) == NULL, "out of range");
    PASS();
}

static int test_ui_labels(void) {
    TEST("UI labels (compass, weight, burnt out, etc.)");
    ASSERT(strcmp(theron_v1_track02_us_party_facing(), "PARTY FACING") == 0, "facing");
    ASSERT(strcmp(theron_v1_track02_us_weighs(), "WEIGHS") == 0, "weighs");
    ASSERT(strcmp(theron_v1_track02_us_kg_suffix(), " KG.") == 0, "kg");
    ASSERT(strcmp(theron_v1_track02_us_burnt_out(), "(BURNT OUT)") == 0, "burnt");
    ASSERT(strcmp(theron_v1_track02_us_consumable(), "CONSUMABLE") == 0, "consumable");
    PASS();
}

static int test_system_messages(void) {
    TEST("System messages (WAKE UP, GAME FROZEN, RESURRECTED)");
    ASSERT(strcmp(theron_v1_track02_us_wake_up(), "WAKE UP") == 0, "wake");
    ASSERT(strcmp(theron_v1_track02_us_game_frozen(), "GAME FROZEN") == 0, "frozen");
    ASSERT(strcmp(theron_v1_track02_us_resurrected(), "RESURRECTED.") == 0, "resurrected");
    PASS();
}

int main(void) {
    printf("Theron V1 Track 02 US UI Strings\n");
    int pass = 0, total = 0;
    total++; pass += test_flask_states();
    total++; pass += test_directions();
    total++; pass += test_item_statuses();
    total++; pass += test_ui_labels();
    total++; pass += test_system_messages();
    printf("\n%d/%d passed\n", pass, total);
    return pass == total ? 0 : 1;
}
