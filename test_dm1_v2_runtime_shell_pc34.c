#include "dm1_v2_runtime_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

int main(void) {
    DM1_V2_RuntimeState rt;
    dm1_v2_runtime_init(&rt);
    CHECK(!dm1_v2_runtime_is_running(&rt));
    CHECK(rt.player.facingDir == 0);
    CHECK(dm1_v2_vp_is_dirty(&rt.viewport));

    dm1_v2_runtime_start(&rt, 1000);
    CHECK(dm1_v2_runtime_is_running(&rt));
    CHECK(rt.lastTickMs == 1000);

    CHECK(dm1_v2_runtime_apply_command(&rt, 1, 1100) == 1);
    CHECK(rt.lastCommand == 1);
    CHECK(dm1_v2_vp_is_scrolling(&rt.viewport));
    CHECK(dm1_v2_has_moved(&rt.player));

    dm1_v2_runtime_tick(&rt, 1350);
    CHECK(rt.tickCount == 1);
    CHECK(rt.lastTickMs == 1350);

    CHECK(dm1_v2_runtime_apply_command(&rt, 3, 1400) == 1);
    CHECK(rt.player.facingDir == 7);
    CHECK(dm1_v2_runtime_apply_command(&rt, 99, 1500) == 0);

    dm1_v2_runtime_stop(&rt);
    CHECK(!dm1_v2_runtime_is_running(&rt));
    CHECK(dm1_v2_runtime_apply_command(&rt, 1, 1600) == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_runtime_shell_pc34: ok");
    return 0;
}
