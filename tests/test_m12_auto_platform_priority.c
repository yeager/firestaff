/* AUTO platform selection must be a media policy, not catalogue order.
 * DM1 prefers its original PC route. DM2 prefers authenticated FM Towns
 * media, then Macintosh retail on macOS and PC elsewhere. CSB never had
 * a DOS release and defaults to verified native Amiga before FM Towns/Atari. */
#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

int main(void)
{
    static const char *const pc_games[] = {"dm1"};
    static const char *const pc_versions[] = {"pc34-en"};
    static const char *const fmtowns_versions[] = {"fmtowns-en", "fmtowns-ja"};
    M12_AssetStatus status;
    size_t i;

    memset(&status, 0, sizeof(status));
    for (i = 0u; i < sizeof(pc_games) / sizeof(pc_games[0]); ++i) {
        const int game_index = 0;
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
    puts("PASS: AUTO keeps PC-first DM1 selection");
    {
        int pc = M12_AssetStatus_FindVersionIndex("dm2", "pc-en");
        int mac = M12_AssetStatus_FindVersionIndex("dm2", "mac-en-retail");
        int fmtowns = M12_AssetStatus_FindVersionIndex("dm2", "fmtowns-ja");
        int selected;
        memset(&status, 0, sizeof(status));
        if (pc < 0 || mac < 0 || fmtowns < 0) {
            fprintf(stderr, "FAIL: missing DM2 AUTO platform identities\n");
            return 1;
        }
        status.versions[2][pc].matched = 1;
        status.versions[2][mac].matched = 1;
        status.versions[2][fmtowns].matched = 1;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, "dm2", M12_ARCH_AUTO);
        if (selected != fmtowns) {
            fprintf(stderr, "FAIL: DM2 AUTO did not prefer authenticated FM Towns\n");
            return 1;
        }
        puts("PASS: DM2 AUTO prefers FM Towns on every host");
        status.versions[2][fmtowns].matched = 0;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, "dm2", M12_ARCH_AUTO);
#if defined(__APPLE__) && TARGET_OS_OSX
        if (selected != mac) {
            fprintf(stderr, "FAIL: macOS AUTO did not prefer authenticated Macintosh DM2\n");
            return 1;
        }
        puts("PASS: macOS AUTO prefers Macintosh DM2 retail media");
#else
        if (selected != pc) {
            fprintf(stderr, "FAIL: non-macOS AUTO did not prefer PC DM2\n");
            return 1;
        }
        puts("PASS: non-macOS AUTO keeps PC-first DM2 selection");
#endif
#if defined(__APPLE__) && TARGET_OS_OSX
        memset(&status, 0, sizeof(status));
        status.versions[2][pc].matched = 1;
        selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
            &status, "dm2", M12_ARCH_AUTO);
        if (selected != pc) {
            fprintf(stderr, "FAIL: macOS AUTO did not fall back to PC without Mac retail\n");
            return 1;
        }
        puts("PASS: macOS AUTO falls back to PC when Mac retail is absent");
#endif
    }
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
        if (selected != amiga) {
            fprintf(stderr, "FAIL: AUTO did not keep CSB on Amiga\n");
            return 1;
        }
    }
    puts("PASS: AUTO keeps CSB on native Amiga media");
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
    {
        static const char *const pc98_versions[] = {
            "pc98-ja-demo"
        };
        static const char *const game_ids[] = {
            "dm1", "csb", "dm2", "nexus", "theron"
        };
        size_t version_index;
        size_t game_index;

        /* PC-9821 has no supported catalog identity. PC-9801 demo and
         * X68000 remain preservation-only and cannot enter AUTO priority. */
        if (M12_AssetStatus_FindVersionIndex("dm2", "pc9821-ja") >= 0) {
            fputs("FAIL: removed PC-9821 edition returned to the catalog\n", stderr);
            return 1;
        }
        for (version_index = 0u;
             version_index < sizeof(pc98_versions) / sizeof(pc98_versions[0]);
             ++version_index) {
            int pc98 = M12_AssetStatus_FindVersionIndex("dm2",
                                                         pc98_versions[version_index]);
            int selected;
            memset(&status, 0, sizeof(status));
            if (pc98 < 0) {
                fprintf(stderr, "FAIL: missing DM2 PC-98 catalogue identity %s\n",
                        pc98_versions[version_index]);
                return 1;
            }
            status.versions[2][pc98].versionId = pc98_versions[version_index];
            status.versions[2][pc98].matched = 1;
            selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
                &status, "dm2", M12_ARCH_PC98);
            if (strcmp(pc98_versions[version_index], "pc98-ja-demo") == 0 &&
                selected >= 0) {
                fprintf(stderr,
                        "FAIL: PC-9801 demo became launchable: %s\n",
                        pc98_versions[version_index]);
                return 1;
            }
            selected = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
                &status, "dm2", M12_ARCH_AUTO);
            if (selected >= 0) {
                fprintf(stderr,
                        "FAIL: AUTO selected PC-98 media: %s\n",
                        pc98_versions[version_index]);
                return 1;
            }
        }
        memset(&status, 0, sizeof(status));
        for (game_index = 0u;
             game_index < sizeof(game_ids) / sizeof(game_ids[0]);
             ++game_index) {
            if (M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
                    &status, game_ids[game_index], M12_ARCH_PC98) >= 0 ||
                M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
                    &status, game_ids[game_index], M12_ARCH_X68000) >= 0) {
                fprintf(stderr,
                        "FAIL: unsupported PC-98/X68000 route exposed for %s\n",
                        game_ids[game_index]);
                return 1;
            }
        }
    }
    puts("PASS: PC-9801 demo stays blocked; PC-9821 is absent from the catalog");
    return 0;
}
