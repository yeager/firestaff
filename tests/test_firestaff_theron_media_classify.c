#include "firestaff_theron_media_classify.h"
#include "asset_find_by_hash.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_authentic_theron_rar_split(void) {
    const char *archive = getenv("FIRESTAFF_THERON_RAR");
    char virtual_track02[ASSET_PATH_MAX];
    char md5[33];
    FirestaffTheronMediaStatus media;
    static const struct {
        const char *cue_member;
        const char *track02_member;
        const char *track19_member;
        const char *track02_tail_member;
        const char *track02_md5;
        const char *track01_member;
    } editions[] = {
        {"TQUS.cue", "TQUS02.iso", "TQUS19.iso", "TQUS02End.iso",
         THERON_TRACK02_MD5_US_ISO, "TQUS01.wav"},
        {"TQJP.cue", "TQJP02.iso", "TQJP19.iso", "TQJP02End.iso",
         THERON_TRACK02_MD5_JP_ISO, "TQJP01.wav"}
    };
    size_t edition_index;
    if (!archive || !archive[0]) return 0;
    for (edition_index = 0U;
         edition_index < sizeof(editions) / sizeof(editions[0]);
         ++edition_index) {
        char cue_path[ASSET_PATH_MAX];
        if (snprintf(cue_path, sizeof(cue_path), "%s::%s", archive,
                     editions[edition_index].cue_member) >=
                (int)sizeof(cue_path) ||
            snprintf(virtual_track02, sizeof(virtual_track02),
                     "%s::@concat(%s,%s)", archive,
                     editions[edition_index].track19_member,
                     editions[edition_index].track02_tail_member) >=
                (int)sizeof(virtual_track02) ||
            !asset_file_md5_hex(virtual_track02, md5) ||
            strcmp(md5, editions[edition_index].track02_md5) != 0) {
            fprintf(stderr, "FAIL: authentic Theron RAR %s did not compose to its verified Track 02 identity\n",
                    editions[edition_index].cue_member);
            return 1;
        }
        if (FirestaffTheronMedia_ClassifyPathForTrack02(
                archive, editions[edition_index].track02_md5, &media) != 0 ||
            !media.has_valid_track02_mode1 ||
            media.track02_mode1_sector_bytes != 2048 ||
            strcmp(media.cue_path, cue_path) != 0 ||
            strcmp(media.track01_path,
                   editions[edition_index].track01_member) != 0 ||
            strcmp(media.track02_path, virtual_track02) != 0 ||
            strcmp(media.candidate_path, virtual_track02) != 0) {
            fprintf(stderr, "FAIL: authentic Theron RAR CUE %s did not classify to its verified virtual Track 02 (track='%s', expected='%s')\n",
                    editions[edition_index].cue_member,
                    media.track02_path, virtual_track02);
            return 1;
        }
        printf("PASS: authentic Theron RAR %s selects verified Track 02 (%s) and Track 01 (%s)\n",
               editions[edition_index].cue_member, md5,
               media.track01_path);
    }
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
