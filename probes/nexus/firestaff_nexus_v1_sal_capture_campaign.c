/* Write source-bound SAL/MAP capture targets from a real Nexus corpus.
 * The output is capture planning only, not audio decoding or playback. */
#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_level(const char *text, int *out_level)
{
    char *end = NULL;
    long value;

    if (!text || !out_level) return 0;
    value = strtol(text, &end, 10);
    if (!end || *end || value < 0L || value > 15L) return 0;
    *out_level = (int)value;
    return 1;
}

int main(int argc, char **argv)
{
    Nexus_V1_Engine engine;
    Nexus_V1_LevelSoundCaptureTargetReceipt target;
    char path[1024];
    int all_levels;
    int level;
    int final_level;
    int selector;
    int written = 0;

    if (argc != 4 ||
        (strcmp(argv[2], "all") != 0 && !parse_level(argv[2], &level))) {
        fprintf(stderr, "usage: %s <nexus-data-dir> <level-0..15|all> <output-directory>\n", argv[0]);
        return 2;
    }
    if (nexus_v1_init(&engine, argv[1]) != 0) {
        fprintf(stderr, "could not initialize a canonical Nexus corpus\n");
        return 1;
    }
    all_levels = strcmp(argv[2], "all") == 0;
    if (all_levels) level = 0;
    final_level = all_levels ? 15 : level;
    for (; level <= final_level; ++level) {
        if (nexus_v1_load_level(&engine, level) != 0) {
            fprintf(stderr, "could not load canonical LEV%02d\n", level);
            return 1;
        }
        for (selector = 0; selector <= 0xff; ++selector) {
            if (nexus_v1_engine_build_sal_capture_target(&engine, selector,
                                                         &target) != 1) {
                continue;
            }
            if (!target.valid || target.level_index != level ||
                !target.canonical_sal_md5[0] || !target.canonical_map_md5[0] ||
                !target.canonical_driver_md5[0] ||
                !target.original_saturn_driver_capture_required ||
                target.sal_decode_proven || target.playback_permitted ||
                !target.no_playback_only || target.fallback_visuals_permitted ||
                snprintf(path, sizeof(path), "%s/SNDLEV%02d-%02X.sal.target",
                         argv[3], level, selector) >= (int)sizeof(path) ||
                nexus_v1_engine_write_sal_capture_target(&engine, selector,
                                                         path, &target) != 1) {
                fprintf(stderr, "could not create LEV%02d SAL target %02X\n",
                        level, selector);
                return 1;
            }
            ++written;
        }
    }
    printf("wrote %d no-playback SAL capture targets for %s\n", written,
           all_levels ? "LEV00-LEV15" : "selected LEV");
    return 0;
}
