#include "csb_v1_f0182_group_stop_attacking_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>

typedef struct {
    int calls;
    int16_t map_x;
    int16_t map_y;
} DeleteEventsReceipt;

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static void delete_events(void *context, int16_t map_x, int16_t map_y)
{
    DeleteEventsReceipt *receipt = (DeleteEventsReceipt *)context;

    ++receipt->calls;
    receipt->map_x = map_x;
    receipt->map_y = map_y;
}

int main(void)
{
    CSB_V1_GroupActiveGroupPc34 active_group = {{0u, 0u, 0u, 0u}};
    DeleteEventsReceipt receipt = {0, 0, 0};
    int ok = 1;

    active_group.aspect[0] = 0x80u;
    active_group.aspect[1] = 0xffu;
    active_group.aspect[2] = 0x7fu;
    active_group.aspect[3] = 0x01u;

    csb_v1_f0182_group_stop_attacking_pc34(
        &active_group, (int16_t)-17, (int16_t)31, delete_events, &receipt);

    ok &= check(active_group.aspect[0] == 0x00u,
                "F0182 clears the first creature attacking bit");
    ok &= check(active_group.aspect[1] == 0x7fu,
                "F0182 clears only the attacking bit");
    ok &= check(active_group.aspect[2] == 0x7fu,
                "F0182 preserves a non-attacking aspect");
    ok &= check(active_group.aspect[3] == 0x01u,
                "F0182 visits all four aspects without changing other bits");
    ok &= check(receipt.calls == 1,
                "F0182 deletes group events exactly once");
    ok &= check(receipt.map_x == -17 && receipt.map_y == 31,
                "F0182 passes its supplied map square to F0181");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_f0182_group_stop_attacking_pc34_compat");
    return 0;
}
