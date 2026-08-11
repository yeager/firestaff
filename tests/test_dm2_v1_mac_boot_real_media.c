#include "dm2_v1_boot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int demo = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP") != NULL;
    if (demo) zip = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP");
    DM2_V1_BootProfile p;
    if (!zip || !zip[0]) { puts("SKIP: DM2 Mac ZIP environment is not set"); return 0; }
    dm2_v1_boot_profile_init(&p);
    if (dm2_v1_boot_scan_assets(&p, zip) != 0 || !p.assets_verified ||
        p.platform != DM2_PLATFORM_MAC_EN ||
        strcmp(p.version_id, demo ? "mac-en-demo" : "mac-en") != 0 ||
        p.graphics_mem_size != (demo ? 3110116u : 8157169u) ||
        p.dungeon_mem_size != (demo ? 6535u : 39411u) ||
        (!demo && (!p.music_map_verified || p.music_map_size != 176u)) ||
        (!demo && (p.mac_movie_present_mask != 0x1du ||
                   p.mac_movie_resource_present_mask != 0x1du ||
                   p.mac_movie_moov_present_mask != 0x1du ||
                   p.mac_movie_moov_size[DM2_V1_MAC_MOVIE_TITLE] != 3286u)) ||
        dm2_v1_boot_enter_game(&p) != 0) {
        fprintf(stderr, "DM2 Mac boot failed: platform=%d version=%s verified=%d g=%zu d=%zu\n",
                p.platform, p.version_id, p.assets_verified,
                p.graphics_mem_size, p.dungeon_mem_size);
        fprintf(stderr, "hashes: %s %s\\n", p.graphics_md5, p.dungeon_md5);
        dm2_v1_boot_cleanup(&p);
        return 1;
    }
    dm2_v1_boot_cleanup(&p);
    puts(demo ? "PASS: DM2 Macintosh demo boots from the original ZIP in RAM"
              : "PASS: DM2 Macintosh retail boots from the original ZIP in RAM");
    return 0;
}
