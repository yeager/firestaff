/* AUTO platform selection must be a media policy, not catalogue order.
 * DM1/DM2 prefer their original PC routes.  CSB never had a DOS release:
 * it must prefer its original FM Towns route over Amiga, Atari and any
 * accidental compatibility catalogue row. */
#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    static const char *const pc_games[] = {"dm1", "dm2"};
    static const char *const pc_versions[] = {"pc34-en", "pc-en"};
    static const char *const fmtowns_versions[] = {"fmtowns-en", "fmtowns-ja"};
    M12_AssetStatus status;
    size_t i;

    memset(&status, 0, sizeof(status));
    for (i = 0u; i < sizeof(pc_games) / sizeof(pc_games[0]); ++i) {
        const int game_index = strcmp(pc_games[i], "dm1") == 0 ? 0 : 2;
        int pc = M12_AssetStatus_FindVersionIndex(pc_games[i], pc_versions[i]);
        int fmtowns = M12_AssetStatus_FindVersionIndex(pc_games[i],
                                                        fmtowns_versions[i]);
        int selected;
        if (pc < 0 || fmtowns < 0) {
            fprintf(stderr, "FAIL: missing catalogue identities for %s\n", pc_games[i]);
            return 1;
        }
        status.versions[game_index][pc].matched = 1;
        status.versions[game_index][fmtowns].matched = 1;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, pc_games[i], M12_ARCH_AUTO);
        if (selected != pc) {
            fprintf(stderr, "FAIL: AUTO selected the wrong PC route for %s\n", pc_games[i]);
            return 1;
        }
    }
    puts("PASS: AUTO keeps PC-first DM1/DM2 selection");
    {
        int fmtowns = M12_AssetStatus_FindVersionIndex("csb", "fmtowns-en");
        int amiga = M12_AssetStatus_FindVersionIndex("csb", "amiga31-en");
        int atari = M12_AssetStatus_FindVersionIndex("csb", "st20-21-en");
        int selected;
        memset(&status, 0, sizeof(status));
        if (fmtowns < 0 || amiga < 0 || atari < 0) {
            fprintf(stderr, "FAIL: missing CSB platform catalogue identities\n");
            return 1;
        }
        if (M12_AssetStatus_FindVersionIndex("csb", "pc34-en") >= 0) {
            fprintf(stderr, "FAIL: CSB must not advertise a DOS catalogue row\n");
            return 1;
        }
        if (M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
                &status, "csb", M12_ARCH_X68000) >= 0 ||
            M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
                &status, "csb", M12_ARCH_PC98) >= 0) {
            fprintf(stderr,
                    "FAIL: CSB must expose only FM Towns, Amiga and Atari ST\n");
            return 1;
        }
        status.versions[1][fmtowns].matched = 1;
        status.versions[1][amiga].matched = 1;
        status.versions[1][atari].matched = 1;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, "csb", M12_ARCH_AUTO);
        if (selected != fmtowns) {
            fprintf(stderr, "FAIL: AUTO did not keep CSB on FM Towns\n");
            return 1;
        }
    }
    puts("PASS: AUTO keeps CSB on original FM Towns media");
    {
        int a31e = M12_AssetStatus_FindVersionIndex("csb", "amiga31-en");
        int a31m = M12_AssetStatus_FindVersionIndex("csb", "amiga31-multi");
        int selected;
        memset(&status, 0, sizeof(status));
        if (a31e < 0 || a31m < 0) {
            fprintf(stderr, "FAIL: missing CSB Amiga catalogue identities\n");
            return 1;
        }
        status.versions[1][a31e].matched = 1;
        status.versions[1][a31m].matched = 1;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, "csb", M12_ARCH_AMIGA);
        if (selected != a31e) {
            fprintf(stderr, "FAIL: Amiga did not select verified A31E before A31M\n");
            return 1;
        }
    }
    puts("PASS: CSB Amiga selection admits verified native A31E");
    return 0;
}
