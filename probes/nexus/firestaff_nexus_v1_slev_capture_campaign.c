/* Write source-bound SLEV SH-2 capture targets from a real Nexus corpus.
 * This emits capture requests only: it neither dispatches task bytes nor
 * produces visual or audio output. */
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
    Nexus_V1_LevelScriptCaptureTargetReceipt target;
    char path[1024];
    int level;
    int all_levels;
    int final_level;
    int written = 0;

    if (argc != 4 ||
        (strcmp(argv[2], "all") != 0 && !parse_level(argv[2], &level))) {
        fprintf(stderr,
                "usage: %s <nexus-data-dir> <level-0..15|all> "
                "<output-target|output-directory>\n",
                argv[0]);
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
        if (all_levels) {
            if (snprintf(path, sizeof(path), "%s/SLEV%02d.sh2.target", argv[3],
                         level) >= (int)sizeof(path)) {
                fprintf(stderr, "capture target path is too long\n");
                return 2;
            }
        } else if (snprintf(path, sizeof(path), "%s", argv[3]) >=
                   (int)sizeof(path)) {
            fprintf(stderr, "capture target path is too long\n");
            return 2;
        }
        if (nexus_v1_engine_write_slev_capture_target(&engine, path, &target) !=
                1 ||
            !target.valid || target.level_index != level ||
            !target.canonical_slev_name[0] || !target.canonical_slev_md5[0] ||
            !target.original_saturn_execution_required ||
            target.task_body_dispatch_proven || !target.no_dispatch_only ||
            target.fallback_visuals_permitted) {
            fprintf(stderr, "could not create LEV%02d SLEV capture target\n",
                    level);
            return 1;
        }
        ++written;
    }
    printf("wrote %d no-dispatch SLEV capture target%s for %s\n", written,
           written == 1 ? "" : "s", all_levels ? "LEV00-LEV15" : "selected LEV");
    return 0;
}
