/* Source-layout contract for skproject title-menu events 0xD7 and 0xD9. */

#include "dm2_v1_boot.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    DM2_V1_StartupMenuPointerLayout layout;
    DM2_V1_StartupMenuPointerHitReceipt receipt;

    memset(&layout, 0, sizeof(layout));
    layout.valid = 1;
    layout.table_hash = 0x4f524947u;
    layout.new_game = (DM2_V1_InterfaceRect){ 10, 20, 40, 12 };
    layout.resume_game = (DM2_V1_InterfaceRect){ 60, 20, 40, 12 };

    check(dm2_v1_boot_startup_menu_pointer_hit_from_layout(
              &layout, 30, 25, &receipt) == 1 && receipt.valid == 1 &&
              receipt.table_hash == layout.table_hash &&
              receipt.target == DM2_V1_STARTUP_POINTER_TARGET_NEW_GAME,
          "0xD7 hit remains the source-owned NEW target");
    check(dm2_v1_boot_startup_menu_pointer_hit_from_layout(
              &layout, 80, 25, &receipt) == 1 && receipt.valid == 1 &&
              receipt.table_hash == layout.table_hash &&
              receipt.target ==
                  DM2_V1_STARTUP_POINTER_TARGET_RESUME_GAME,
          "0xD9 hit remains the source-owned RESUME target");
    check(dm2_v1_boot_startup_menu_pointer_hit_from_layout(
              &layout, 50, 25, &receipt) == 0,
          "gap between source rectangles is not a synthetic click target");
    layout.valid = 0;
    check(dm2_v1_boot_startup_menu_pointer_hit_from_layout(
              &layout, 30, 25, &receipt) == 0,
          "unverified layouts fail closed");

    if (failures != 0) return 1;
    puts("PASS: DM2 title-menu 0xD7/0xD9 pointer receipt contract");
    return 0;
}
