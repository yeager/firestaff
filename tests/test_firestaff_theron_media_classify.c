#include "firestaff_theron_media_classify.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_authentic_theron_rar_split(void) {
    const char *archive = getenv("FIRESTAFF_THERON_RAR");
    char virtual_track02[ASSET_PATH_MAX];
    char md5[33];
    FirestaffTheronMediaStatus media;
    if (!archive || !archive[0]) return 0;
    if (snprintf(virtual_track02, sizeof(virtual_track02),
                 "%s::@concat(TQUS19.iso,TQUS02End.iso)", archive) >=
            (int)sizeof(virtual_track02) ||
        !asset_file_md5_hex(virtual_track02, md5) ||
        strcmp(md5, "ceb02343868f80cec899e9b239aff2da") != 0) {
        fprintf(stderr,
                "FAIL: authentic Theron RAR split did not compose to its verified Track 02 identity\n");
        return 1;
    }
    printf("PASS: authentic Theron RAR split composes in memory as Track 02 (%s)\n",
           md5);
    if (FirestaffTheronMedia_ClassifyPath(archive, &media) != 0 ||
        !media.has_valid_track02_mode1 ||
        media.track02_mode1_sector_bytes != 2048 ||
        strcmp(media.track02_path, virtual_track02) ||
        strcmp(media.candidate_path, virtual_track02)) {
        fprintf(stderr, "classifier rc/status: layout=%d valid=%d pvd=%d sector=%d track='%s' expected='%s'\n",
                (int)media.layout, media.has_valid_track02_mode1,
                media.has_iso9660_pvd, media.track02_mode1_sector_bytes,
                media.track02_path, virtual_track02);
        fprintf(stderr,
                "FAIL: authentic Theron RAR CUE did not classify to its verified virtual Track 02\n");
        return 1;
    }
    printf("PASS: authentic Theron RAR CUE selects its verified virtual Track 02\n");
    return 0;
}

int main(void) {
    if (FirestaffTheronMedia_SelfTest() == 0 &&
        test_authentic_theron_rar_split() == 0) {
        printf("test_firestaff_theron_media_classify: PASS\n");
        return 0;
    }
    printf("test_firestaff_theron_media_classify: FAIL\n");
    return 1;
}
