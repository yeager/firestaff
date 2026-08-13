/* Opt-in real-media Mac NEW GAME/runtime handoff regression. */

#include "dm2_v1_boot.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_hand_action_gdat.h"
#include "dm2_v1_inventory_panel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_BootProfile *profile;
    DM2_V1_BootRuntimeReceipt runtime;

    if (!zip || !zip[0]) {
        puts("SKIP: FIRESTAFF_DM2_MAC_EN_ZIP is not set");
        return 0;
    }
    memset(&launch, 0, sizeof(launch));
    expect(dm2_v1_boot_startup_launch_alloc(zip, &launch) == 1 &&
               launch.profile && launch.profile->assets_verified &&
               launch.profile->platform == DM2_PLATFORM_MAC_EN,
           "authentic Mac retail archive is admitted");
    profile = launch.profile;
    if (!profile || !profile->assets_verified) {
        dm2_v1_boot_startup_launch_cleanup(&launch);
        return 1;
    }
    {
        DM2_V1_AssetLoader loader;
        uint8_t *pixels;
        int width = 0;
        int height = 0;
        memset(&loader, 0, sizeof(loader));
        expect(dm2_v1_asset_loader_init(&loader, profile->graphics_mem,
                                        profile->graphics_mem_size) == 0,
               "Mac graphics buffer can be inspected");
        pixels = dm2_v1_asset_load_image_field(
            &loader, DM2_GDAT_CATEGORY_INTERFACE_CHARSHEET, 0, 1,
            &width, &height, NULL);
        expect(pixels != NULL && width > 0 && height > 0,
               "Mac CHARSHEET survey image decodes");
        for (int possession = 0; possession < 2; ++possession) {
            for (int side = 0; side < 2; ++side) {
                DM2_V1_HandActionInput input;
                DM2_V1_HandActionGdatRoute route;
                DM2_V1_GdatRaw4BlitPlacement hand_placement;
                uint8_t *icon;
                int icon_width = 0;
                int icon_height = 0;
                memset(&input, 0, sizeof(input));
                input.possession_index = possession;
                input.left_or_right = side;
                input.player_position = 0;
                input.party_direction = 0;
                icon = dm2_v1_hand_action_gdat_load_image(
                    &loader, &input, &route, &icon_width, &icon_height, NULL);
                expect(icon != NULL && icon_width > 0 && icon_height > 0,
                       "Mac hand-action image decodes");
                memset(&hand_placement, 0, sizeof(hand_placement));
                expect(dm2_v1_gdat_door_overlay_query_raw4_blit_placement(
                           &loader, route.rectno, icon_width, icon_height,
                           &hand_placement) == 1,
                       "Mac hand-action RAW4 placement is source-owned");
                dm2_v1_asset_free_pixels(icon);
            }
        }
        dm2_v1_asset_free_pixels(pixels);
        dm2_v1_asset_loader_free(&loader);
    }
    expect(dm2_v1_boot_prepare_new_game_world(profile) == 1,
           "Mac retail materializes the source-owned NEW GAME candidate");
    expect(dm2_v1_boot_new_game_runtime_candidate_readonly(profile) != NULL,
           "Mac retail retains the private runtime candidate after STARTEND");
    memset(&runtime, 0, sizeof(runtime));
    expect(dm2_v1_boot_commit_new_game_session(profile) == 1,
           "Mac retail commits the source-owned NEW GAME session");
    expect(dm2_v1_boot_runtime_capture(profile, &runtime) == 1 &&
               runtime.runtime_ready && runtime.party_x == 1 &&
               runtime.party_y == 8 && runtime.party_dir == 0 &&
               runtime.leader_hand_object == 0xffffu,
           "Mac retail exposes the committed source party to runtime");
    expect(dm2_v1_boot_runtime_move(profile, runtime.party_dir, &runtime) == 1,
           "Mac retail accepts a source-owned movement command");
    dm2_v1_boot_startup_launch_cleanup(&launch);
    if (failures != 0) return 1;
    puts("PASS: authentic DM2 Macintosh retail NEW GAME reaches runtime");
    return 0;
}
