#include "csb_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *source = getenv("FIRESTAFF_CSB_ATARI_STX");
    CSB_V1_BootProfile profile;
    if (!source || !source[0]) {
        puts("SKIP: FIRESTAFF_CSB_ATARI_STX not set");
        return 77;
    }
    csb_v1_boot_profile_init(&profile);
    if (csb_v1_boot_scan_assets(&profile, source) != 0 ||
        (profile.variant_id != CSB_V1_VARIANT_ST20_EN &&
         profile.variant_id != CSB_V1_VARIANT_ST21_EN) ||
        csb_v1_boot_enter_game(&profile) != 0) {
        fprintf(stderr, "FAIL: supplied CSB Atari media did not enter runtime\n");
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    if (!profile.runtime.object_name_table_valid ||
        !profile.runtime.action_name_table_valid ||
        strcmp(csb_v1_runtime_action_name_c699(&profile.runtime, 1u),
               "BLOCK") != 0 ||
        strcmp(csb_v1_runtime_action_name_c699(&profile.runtime, 43u),
               "FUSE") != 0 ||
        profile.runtime.object_names[0][0] == '\0') {
        fprintf(stderr, "FAIL: Atari M564/C560 text did not reach runtime\n");
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    puts("PASS: Atari runtime owns source M564 and C560/G0490 text");
    csb_v1_boot_cleanup(&profile);
    return 0;
}
