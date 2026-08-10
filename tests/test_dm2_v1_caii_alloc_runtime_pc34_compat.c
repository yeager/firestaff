/* DM2_ALLOC_CAII_TO_CREATURE production boundary.  Source c_1c9a.cpp:5772-
 * 5894 requires the same authenticated c_map, DB4, CAII, GDAT and c_tim
 * transaction. A capacity alone is never creature evidence. */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void)
{
    DM2_V1_BootProfile boot;
    DM2_V1_CaiiAllocReceipt receipt;

    memset(&boot, 0, sizeof(boot));
    memset(&receipt, 0, sizeof(receipt));
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    CHECK(dm2_v1_runtime_caii_init(4) && dm2_v1_runtime_caii_ready(),
          "a private CAII capacity can be retained without activating gameplay");
    CHECK(!dm2_v1_runtime_alloc_caii_at(0, 0, &receipt) &&
              !receipt.valid && dm2_v1_runtime_caii_alloc_count() == 0,
          "CAII allocation rejects missing authentic DB4 and GAME_LOAD state");

    printf("DM2 CAII allocation production boundary: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
