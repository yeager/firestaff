#include "dm1_v1_fmtowns_startup.h"
#include "entrance_frontend_pc34_compat.h"

#include <stdio.h>

static int failures;

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expression); \
        ++failures; \
    } \
} while (0)

int main(void) {
    /* NONE is what a failed title/Entrance compositor returns.  It must not
     * fall through to the already-opened game view. */
    CHECK(!dm1_v1_fmtowns_startup_handoff_allows_gameplay(
        0, ENTRANCE_COMPAT_COMMAND_PATH_NONE));
    CHECK(!dm1_v1_fmtowns_startup_handoff_allows_gameplay(
        0, ENTRANCE_COMPAT_COMMAND_PATH_QUIT));
    CHECK(!dm1_v1_fmtowns_startup_handoff_allows_gameplay(
        0, ENTRANCE_COMPAT_COMMAND_PATH_CREDITS));
    CHECK(dm1_v1_fmtowns_startup_handoff_allows_gameplay(
        0, ENTRANCE_COMPAT_COMMAND_PATH_ENTER));
    CHECK(dm1_v1_fmtowns_startup_handoff_allows_gameplay(
        0, ENTRANCE_COMPAT_COMMAND_PATH_RESUME));
    /* A boot probe explicitly requests exit before the second, interactive
     * entrance command, and therefore cannot be treated as a play session. */
    CHECK(dm1_v1_fmtowns_startup_handoff_allows_gameplay(
        1, ENTRANCE_COMPAT_COMMAND_PATH_NONE));

    if (failures) return 1;
    puts("PASS: DM1 FM Towns entrance handoff gate");
    return 0;
}
