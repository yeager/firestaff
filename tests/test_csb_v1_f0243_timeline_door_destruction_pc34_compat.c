#include "csb_v1_f0243_timeline_door_destruction_pc34_compat.h"

#include <stdio.h>

static int failed;

#define CHECK(condition, message) do { \
    if (condition) { printf("PASS: %s\n", message); } \
    else { ++failed; printf("FAIL: %s\n", message); } \
} while (0)

int main(void)
{
    uint8_t door_square = 0x84u;
    uint8_t destroyed_square = 0x85u;

    F0243_TIMELINE_ProcessEvent2_DoorDestruction(&door_square);

    CHECK(door_square == destroyed_square,
          "F0243 changes only a CSB door square state to C5_DESTROYED");
    CHECK((door_square & 0xf8u) == (destroyed_square & 0xf8u),
          "F0243 preserves the CSB square type and upper attributes");
    return failed != 0;
}
