/*
 * test_dm2_v1_graphics_data_file_open.c
 *
 * Focused skproject GRAPHICS_DATA_OPEN/ORIGINAL__GRAPHICS_DATA_OPEN receipt.
 */

#include "dm2_v1_asset_loader.h"

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
    DM2_V1_GraphicsDataFileState state;
    DM2_V1_GraphicsDataOpenReceipt receipt;

    printf("DM2 GRAPHICS_DATA_OPEN file receipt\n");

    memset(&state, 0, sizeof(state));
    state.filetype2 = 1u;
    state.primary_file_size = 100u;
    CHECK(dm2_v1_graphics_data_open_receipt(
              &state, 1, 11, 1, 22, &receipt) &&
              receipt.valid &&
              receipt.counter_before == 0 &&
              receipt.counter_after == 1 &&
              receipt.opened_primary &&
              receipt.opened_secondary &&
              receipt.primary_handle == 11 &&
              receipt.secondary_handle == 22 &&
              state.file_open_counter == 1 &&
              state.file_handle == 11 &&
              state.xfile_handle == 22 &&
              receipt.receipt_hash != 0u,
          "first open binds primary and secondary handles");

    CHECK(dm2_v1_graphics_data_open_receipt(
              &state, 0, -1, 0, -1, &receipt) &&
              receipt.valid &&
              receipt.counter_before == 1 &&
              receipt.counter_after == 2 &&
              !receipt.opened_primary &&
              !receipt.opened_secondary &&
              receipt.primary_handle == 11 &&
              receipt.secondary_handle == 22 &&
              state.file_open_counter == 2,
          "nested open only increments the source open counter");

    memset(&state, 0, sizeof(state));
    CHECK(!dm2_v1_graphics_data_open_receipt(
              &state, 0, -1, 1, 22, &receipt) &&
              !receipt.valid &&
              receipt.counter_before == 0 &&
              receipt.counter_after == 1 &&
              receipt.blocked_primary_open &&
              receipt.syserr_code == 0x29u,
          "primary open failure reports source sys error 0x29");

    memset(&state, 0, sizeof(state));
    state.filetype2 = 1u;
    CHECK(!dm2_v1_graphics_data_open_receipt(
              &state, 1, 11, 0, -1, &receipt) &&
              !receipt.valid &&
              receipt.opened_primary &&
              receipt.blocked_secondary_open &&
              receipt.syserr_code == 0x1fu,
          "secondary open failure reports source sys error 0x1f");

    CHECK(!dm2_v1_graphics_data_open_receipt(
              NULL, 1, 11, 1, 22, &receipt) &&
              !receipt.valid,
          "missing file state rejects without synthetic handles");

    printf("DM2 GRAPHICS_DATA_OPEN file receipt: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
