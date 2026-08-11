#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP");
    DM2_V1_BootProfile profile;
    DM2_V1_BootChampionSelectionCensus roster;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac demo ZIP environment is not set");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    memset(&roster, 0, sizeof(roster));
    if (dm2_v1_boot_scan_assets(&profile, zip) != 0 ||
        !profile.assets_verified ||
        strcmp(profile.version_id, "mac-en-demo") != 0 ||
        dm2_v1_boot_enter_game(&profile) != 0 ||
        !dm2_v1_boot_prepare_new_game_world(&profile) ||
        !dm2_v1_boot_prepared_new_game_mirror_roster(&profile, &roster) ||
        !roster.valid || roster.candidate_count != 16 ||
        !profile.game_load_runtime_session_candidate) {
        fprintf(stderr,
                "DM2 Mac demo New Game failed: version=%s verified=%d prepared=%d mirrors=%d session=%p\n",
                profile.version_id, profile.assets_verified,
                dm2_v1_boot_prepared_new_game_world_readonly(&profile) != NULL,
                roster.candidate_count,
                profile.game_load_runtime_session_candidate);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    dm2_v1_boot_cleanup(&profile);
    puts("PASS: DM2 Macintosh demo starts authentic New Game from the original ZIP in RAM");
    return 0;
}
