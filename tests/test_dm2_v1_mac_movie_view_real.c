#include "dm2_v1_boot.h"

#include <stdio.h>

int main(void) {
    DM2_V1_BootProfile profile;
    const char *zip = "/Users/bosse/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip";
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, zip) != 0 ||
        profile.mac_movie_view_present_mask != 0x19u ||
        profile.mac_movie_view[DM2_V1_MAC_MOVIE_TITLE].mdat_offset != 3286u ||
        profile.mac_movie_view[DM2_V1_MAC_MOVIE_TITLE].size != 2406299u) {
        fprintf(stderr, "authentic Mac QuickTime view was not retained: data=0x%08x resource=0x%08x moov=0x%08x view=0x%08x title_data=%zu title_moov=%zu\n",
                profile.mac_movie_present_mask,
                profile.mac_movie_resource_present_mask,
                profile.mac_movie_moov_present_mask,
                profile.mac_movie_view_present_mask,
                profile.mac_movie_data_size[DM2_V1_MAC_MOVIE_TITLE],
                profile.mac_movie_moov_size[DM2_V1_MAC_MOVIE_TITLE]);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    dm2_v1_boot_cleanup(&profile);
    return 0;
}
