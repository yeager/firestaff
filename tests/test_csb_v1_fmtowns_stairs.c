#include "csb_v1_fmtowns_stairs.h"

#include <stdio.h>

static int check(int wall_set, int requested, unsigned int expected)
{
    unsigned int actual = 0u;
    if (!csb_v1_fmtowns_stairs_graphic_index(wall_set, requested, &actual) ||
        actual != expected) {
        fprintf(stderr, "FAIL: wallset %d graphic %d -> %u (expected %u)\n",
                wall_set, requested, actual, expected);
        return 0;
    }
    return 1;
}

int main(void)
{
    unsigned int untouched = 99u;
    if (!check(0, 108, 108u) || !check(0, 125, 125u) ||
        !check(1, 108, 148u) || !check(1, 125, 165u) ||
        !check(15, 125, 725u) ||
        csb_v1_fmtowns_stairs_graphic_index(0, 107, &untouched) ||
        untouched != 0u ||
        csb_v1_fmtowns_stairs_graphic_index(0, 126, &untouched) ||
        csb_v1_fmtowns_stairs_graphic_index(16, 108, &untouched) ||
        csb_v1_fmtowns_stairs_graphic_index(0, 108, NULL)) {
        fprintf(stderr, "FAIL: F31 M645/M647 stair mapping\n");
        return 1;
    }
    puts("PASS: F31 Towns stairs map into the active M647 wall-set bank");
    return 0;
}
