#include "csb_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_missing_scan_keeps_inventory_closed(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_CSBGraphicsInventory empty;
    int rc;

    csb_v1_boot_profile_init(&profile);
    memset(&empty, 0, sizeof(empty));
    CHECK(!csb_v1_boot_csbgraphics_inventory_receipt_ready(&profile),
          "new profile has no CSBgraphics inventory receipt");
    CHECK(csb_v1_boot_csbgraphics_inventory(&profile) == NULL,
          "inventory accessor rejects an unscanned profile");
    /* No graphics bytes, hashes, overlay IDs, or generated fixture are given. */
    snprintf(profile.asset_root, sizeof(profile.asset_root), "%s",
             "/tmp/firestaff-csbgraphics-inventory-receipt-absent");
    rc = csb_v1_boot_scan_csbgraphics(&profile, NULL);
    CHECK(rc != CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
          "missing manifest-admitted scan does not report success");
    CHECK(profile.csbgraphics_inventory_ready == 0,
          "missing scan leaves the inventory receipt closed");
    CHECK(profile.csbgraphics_inventory_result ==
              CSB_V1_CSBGRAPHICS_INVENTORY_ERR_ARGUMENT,
          "missing scan retains fail-closed inventory result");
    CHECK(memcmp(&profile.csbgraphics_inventory, &empty, sizeof(empty)) == 0,
          "missing scan publishes no stale inventory data");
    CHECK(csb_v1_boot_csbgraphics_inventory(&profile) == NULL,
          "inventory accessor withholds absent scan data");
    csb_v1_boot_cleanup(&profile);
}

static void test_manifest_admitted_scan_publishes_when_configured(void)
{
    const char *root = getenv("FIRESTAFF_CSBGRAPHICS_DATA_ROOT");
    CSB_V1_BootProfile profile;
    const CSB_V1_CSBGraphicsInventory *inventory;

    if (!root || !root[0]) {
        printf("  SKIP: FIRESTAFF_CSBGRAPHICS_DATA_ROOT is not configured\n");
        return;
    }
    csb_v1_boot_profile_init(&profile);
    snprintf(profile.asset_root, sizeof(profile.asset_root), "%s", root);
    (void)csb_v1_boot_scan_csbgraphics(&profile, NULL);
    CHECK(profile.csbgraphics_scan_result == CSB_V1_CSBGRAPHICS_DAT_REAL_OK,
          "configured root supplies a manifest-admitted CSBgraphics cache");
    inventory = csb_v1_boot_csbgraphics_inventory(&profile);
    CHECK(inventory != NULL,
          "manifest-admitted cache publishes a runtime/profile inventory receipt");
    CHECK(inventory && inventory->count == profile.csbgraphics_cache.index.count &&
              inventory->payload_offset == profile.csbgraphics_cache.index.payload_offset &&
              inventory->payload_used == profile.csbgraphics_cache.index.total_compressed,
          "published inventory matches the scanner-owned parsed index");
    csb_v1_boot_cleanup(&profile);
}

int main(void)
{
    test_missing_scan_keeps_inventory_closed();
    test_manifest_admitted_scan_publishes_when_configured();
    printf("\nCSBgraphics boot inventory receipt: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
