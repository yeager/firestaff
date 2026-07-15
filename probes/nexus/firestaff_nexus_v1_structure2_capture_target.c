/* Write one source-bound Structure2 capture target from a real Nexus corpus.
 * This tool emits no pixels and does not interpret descriptor payload bytes. */
#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>

static int parse_nonnegative(const char *text, int *out_value)
{
    char *end = NULL;
    long value;

    if (!text || !out_value) return 0;
    value = strtol(text, &end, 10);
    if (!end || *end || value < 0L || value > 2147483647L) return 0;
    *out_value = (int)value;
    return 1;
}

int main(int argc, char **argv)
{
    Nexus_V1_Engine engine;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget target;
    char path[1024];
    int level;
    int descriptor;

    if (argc != 5 || !parse_nonnegative(argv[2], &level) || level > 15 ||
        !parse_nonnegative(argv[3], &descriptor)) {
        fprintf(stderr,
                "usage: %s <nexus-data-dir> <level-0..15> "
                "<descriptor-index> <output-target>\n",
                argv[0]);
        return 2;
    }
    if (snprintf(path, sizeof(path), "%s", argv[4]) >= (int)sizeof(path)) {
        fprintf(stderr, "capture target path is too long\n");
        return 2;
    }
    if (nexus_v1_init(&engine, argv[1]) != 0 ||
        nexus_v1_load_level(&engine, level) != 0 ||
        nexus_v1_engine_write_structure2_descriptor_capture_target(
            &engine, descriptor, path, &target) != 1 || !target.valid ||
        !target.no_draw_only || !target.capture_producer_required ||
        !target.original_saturn_capture_required) {
        fprintf(stderr,
                "could not create an authenticated no-draw Structure2 target\n");
        return 1;
    }
    printf("wrote LEV%02d descriptor %d target: %s\n", level, descriptor,
           path);
    return 0;
}
