/* DM2 companions must not be created from host-authored data. */
#include "dm2_v1_companion.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(c, m) do { \
    if (c) { ++passed; printf("PASS %s\n", m); } \
    else { ++failed; printf("FAIL %s\n", m); } \
} while (0)

int main(void)
{
    DM2_V1_CompanionState state;
    DM2_V1_CompanionState before;
    const char *e;

    memset(&state, 0xA5, sizeof(state));
    dm2_v1_companion_init(&state);
    CHECK(state.companion_count == 0, "initial state contains no companion");
    before = state;
    CHECK(dm2_v1_companion_add(&state, "NPC", 100, 20, 10) == -1,
          "caller-authored companion is rejected");
    CHECK(memcmp(&state, &before, sizeof(state)) == 0,
          "rejection does not manufacture companion state");
    CHECK(dm2_v1_companion_add(NULL, "NPC", 100, 20, 10) == -1,
          "null state is rejected safely");
    dm2_v1_companion_tick(&state);
    CHECK(memcmp(&state, &before, sizeof(state)) == 0,
          "tick cannot turn an empty boundary into companion state");
    e = dm2_v1_companion_source_evidence();
    CHECK(e && strstr(e, "unavailable") != NULL,
          "evidence declares host-authored defaults unavailable");
    printf("# passed=%d failed=%d\n", passed, failed);
    return failed ? 1 : 0;
}
