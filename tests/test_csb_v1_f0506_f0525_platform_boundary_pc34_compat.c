#include "csb_v1_f0506_f0525_platform_boundary_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("PASS: %s\n", message); } \
    else { ++failed; printf("FAIL: %s\n", message); } \
} while (0)

int main(void)
{
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0506F0525PlatformBoundaryReceiptPc34 receipt;

    memset(&cache, 0, sizeof(cache));
    memset(&package, 0, sizeof(package));
    cache.loaded = 1;
    cache.index.count = 1;
    package.valid = 1;
    package.hud_ready = 1;

    CHECK(csb_v1_f0506_f0525_platform_boundary_receipt_pc34(
              &cache, &package, &receipt) ==
              CSB_V1_F0506_F0525_PLATFORM_REJECT_UNPROVEN_AMIGA_OWNER &&
              !receipt.valid && receipt.amiga_owner_required &&
              receipt.floppy_owner_required,
          "F0506-F0525 rejects PC34 evidence as insufficient for Amiga owners");
    CHECK(csb_v1_f0506_f0525_platform_boundary_receipt_pc34(
              &cache, &package, NULL) ==
              CSB_V1_F0506_F0525_PLATFORM_REJECT_ARGUMENT,
          "missing receipt is rejected");

    printf("%d/%d\n", passed, passed + failed);
    return failed ? 1 : 0;
}
