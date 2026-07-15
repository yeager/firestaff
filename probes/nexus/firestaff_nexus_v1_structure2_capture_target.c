/* Write source-bound Structure2 capture targets from a real Nexus corpus.
 * This tool emits no pixels and does not interpret descriptor payload bytes. */
#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int all_descriptors;
    int written = 0;

    if (argc != 5 || !parse_nonnegative(argv[2], &level) || level > 15 ||
        (strcmp(argv[3], "all") != 0 && !parse_nonnegative(argv[3], &descriptor))) {
        fprintf(stderr,
                "usage: %s <nexus-data-dir> <level-0..15> "
                "<descriptor-index|all> <output-target|output-directory>\n",
                argv[0]);
        return 2;
    }
    if (nexus_v1_init(&engine, argv[1]) != 0 ||
        nexus_v1_load_level(&engine, level) != 0) {
        fprintf(stderr,
                "could not load a canonical LEV for Structure2 capture\n");
        return 1;
    }
    all_descriptors = strcmp(argv[3], "all") == 0;
    if (!all_descriptors && descriptor >= engine.current_level.structure2_texture_count) {
        fprintf(stderr, "descriptor index is outside the selected LEV\n");
        return 2;
    }
    for (descriptor = all_descriptors ? 0 : descriptor;
         descriptor < engine.current_level.structure2_texture_count;
         ++descriptor) {
        if (all_descriptors) {
            if (snprintf(path, sizeof(path), "%s/LEV%02d-Structure2-%04d.target",
                         argv[4], level, descriptor) >= (int)sizeof(path)) {
                fprintf(stderr, "capture target path is too long\n");
                return 2;
            }
        } else if (snprintf(path, sizeof(path), "%s", argv[4]) >=
                   (int)sizeof(path)) {
            fprintf(stderr, "capture target path is too long\n");
            return 2;
        }
        if (nexus_v1_engine_write_structure2_descriptor_capture_target(
                &engine, descriptor, path, &target) != 1 || !target.valid ||
            !target.no_draw_only || !target.capture_producer_required ||
            !target.original_saturn_capture_required) {
            fprintf(stderr, "could not create Structure2 target %d\n", descriptor);
            return 1;
        }
        ++written;
        if (!all_descriptors) break;
    }
    printf("wrote %d no-draw Structure2 target%s for LEV%02d\n", written,
           all_descriptors ? "s" : "", level);
    return 0;
}
