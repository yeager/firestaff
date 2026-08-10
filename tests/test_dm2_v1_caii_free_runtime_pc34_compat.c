/*
 * test_dm2_v1_caii_free_runtime_pc34_compat.c — production CAII deletion
 * boundary.
 *
 * SKProject: c_1c9a.cpp:5896-5957 DM2_1c9a_0fcb calls
 * DM2_DELETE_CREATURE_RECORD only inside one live GAME_LOAD transaction.
 * Firestaff has source-study slices for parts of that operation, but no
 * complete c_map/3CE7D/DB allocation/CAII/timer owner yet.  This regression
 * proves the normal runtime cannot manufacture that deletion route from a
 * caller-authored CAII mode byte or expose a partial full-delete receipt.
 */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++passed; \
        printf("  PASS: %s\n", msg); \
    } else { \
        ++failed; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

int main(void)
{
    DM2_V1_BootProfile boot;
    DM2_V1_CaiiFreeReceipt free_receipt;

    memset(&boot, 0, sizeof(boot));
    dm2_v1_runtime_init(&boot);

    memset(&free_receipt, 0, sizeof(free_receipt));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_receipt) == 0,
          "no source-owned CAII session permits no slot deletion");
    CHECK(dm2_v1_runtime_caii_set_slot_mode_byte(0, 0x13) == 0,
          "caller-authored CAII delete mode is rejected");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 0,
          "no synthetic CAII allocation is retained");

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
