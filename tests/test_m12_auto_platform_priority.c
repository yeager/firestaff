/* AUTO platform selection must be a media policy, not catalogue order. */
#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    static const char *const games[] = {"dm1", "csb", "dm2"};
    static const char *const pc_versions[] = {"pc34-en", "pc34-en", "pc-en"};
    static const char *const fmtowns_versions[] = {
        "fmtowns-en", "fmtowns-en", "fmtowns-ja"
    };
    M12_AssetStatus status;
    size_t i;

    memset(&status, 0, sizeof(status));
    for (i = 0u; i < sizeof(games) / sizeof(games[0]); ++i) {
        int pc = M12_AssetStatus_FindVersionIndex(games[i], pc_versions[i]);
        int fmtowns = M12_AssetStatus_FindVersionIndex(games[i],
                                                        fmtowns_versions[i]);
        int selected;
        if (pc < 0 || fmtowns < 0) {
            fprintf(stderr, "FAIL: missing catalogue identities for %s\n", games[i]);
            return 1;
        }
        status.versions[i][pc].matched = 1;
        status.versions[i][fmtowns].matched = 1;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, games[i], M12_ARCH_AUTO);
        if (selected != fmtowns) {
            fprintf(stderr, "FAIL: AUTO did not select FM Towns for %s\n", games[i]);
            return 1;
        }
    }
    puts("PASS: AUTO selects a verified FM Towns corpus for DM1, CSB and DM2");
    return 0;
}
