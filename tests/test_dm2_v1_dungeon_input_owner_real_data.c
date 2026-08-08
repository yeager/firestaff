#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_input_owner.h"

typedef struct {
    int calls;
    int16_t event_index;
    int16_t x;
    int16_t y;
} Sink;

static void capture(void *ctx, int16_t event_index, int16_t x, int16_t y)
{
    Sink *sink = (Sink *)ctx;
    sink->calls++;
    sink->event_index = event_index;
    sink->x = x;
    sink->y = y;
}

static int require(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    DM2_V1_BootProfile profile;
    DM2_V1_DungeonInputOwner owner;
    DM2_V1_DungeonInputReceipt receipt;
    Sink sink;

    if (!root || !*root) {
        puts("SKIP: FIRESTAFF_DM2_DATA_DIR is unset");
        return 0;
    }
    /* Do not prove this owner with a caller-authored MD5 string.  The same
     * source pair scanner used by the startup transaction must authenticate
     * GRAPHICS.DAT before the PC route table becomes available. */
    dm2_v1_boot_profile_init(&profile);
    if (!require(dm2_v1_boot_scan_assets(&profile, root) == 0 &&
                 profile.assets_verified &&
                 profile.platform == DM2_PLATFORM_PC_EN &&
                 strcmp(profile.graphics_md5,
                        DM2_V1_DUNGEON_INPUT_PC_EN_GRAPHICS_MD5) == 0,
                 "startup scanner admits the real PC-English GRAPHICS.DAT") ||
        !require(dm2_v1_dungeon_input_owner_init(&owner,
                                                  profile.graphics_md5),
                 "owner accepts only the scanner's real graphics identity")) {
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }

    memset(&sink, 0, sizeof(sink));
    if (!require(dm2_v1_dungeon_input_owner_route(
                     &owner, 100, 100, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                     capture, &sink, &receipt),
                 "viewport click is admitted by source dungeon table") ||
        !require(receipt.accepted && receipt.event_index == 80 &&
                     receipt.source_zone_index == 45 &&
                     receipt.source_x == 0 && receipt.source_y == 40 &&
                     receipt.source_w == 224 && receipt.source_h == 136,
                 "event 0x50 preserves the original viewport rectangle") ||
        !require(sink.calls == 1 && sink.event_index == 80 &&
                     sink.x == 100 && sink.y == 100,
                 "c_input sink receives exact source event and point")) {
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }

    memset(&sink, 0, sizeof(sink));
    if (!require(dm2_v1_dungeon_input_owner_route(
                     &owner, 274, 140, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                     capture, &sink, &receipt),
                 "movement-arrow click is admitted by source table") ||
        !require(receipt.event_index == 3 && receipt.source_zone_index == 40 &&
                     sink.event_index == 3,
                 "forward arrow remains source event 0x03")) {
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }

    if (!require(!dm2_v1_dungeon_input_owner_route(
                     &owner, 100, 100, 0x8000u,
                     capture, &sink, &receipt) &&
                     receipt.blocked_no_source_zone,
                 "a point outside source rectangles does not become an event") ||
        !require(!dm2_v1_dungeon_input_owner_init(&owner, "00000000000000000000000000000000"),
                 "foreign graphics data cannot activate PC route table")) {
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }

    dm2_v1_boot_cleanup(&profile);
    puts("PASS: real DM2 PC GDAT-owned dungeon input route");
    return 0;
}
