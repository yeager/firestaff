/* Generates one real-data capture target for an external Nexus Saturn trace
 * producer. It never emits a trace, VDP1 command, palette, texture, decoder,
 * or rendered surface. */

#include "nexus_v1_engine.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static int parse_nonnegative(const char *text, int *out_value)
{
    char *end = NULL;
    long value;

    if (!text || !out_value) return 0;
    errno = 0;
    value = strtol(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || value < 0 ||
        value > 0x7fffffffL) return 0;
    *out_value = (int)value;
    return 1;
}

int main(int argc, char **argv)
{
    Nexus_V1_Engine engine;
    Nexus_V1_DgnStructure2SourceReceipt source;
    Nexus_V1_DgnStructure3CaptureTargetReceipt target;
    int level_index;
    int entry_index;
    int face_ordinal;
    int written;

    if (argc != 6 || !parse_nonnegative(argv[2], &level_index) ||
        !parse_nonnegative(argv[3], &entry_index) ||
        !parse_nonnegative(argv[4], &face_ordinal) || level_index > 15) {
        fprintf(stderr, "usage: %s DATA_DIR LEVEL ENTRY FACE OUTPUT\n", argv[0]);
        return 2;
    }
    if (nexus_v1_init(&engine, argv[1]) != 0 ||
        nexus_v1_load_level(&engine, level_index) != 0 ||
        nexus_v1_current_level_structure2_source_receipt(&engine, &source) != 0 ||
        !source.canonical_hash_verified || !source.loaded_bytes_bound) {
        fprintf(stderr, "canonical LEV%02d.DGN source unavailable\n", level_index);
        nexus_v1_shutdown(&engine);
        return 1;
    }
    written = nexus_v1_dgn_structure3_capture_target_build(
        &engine.current_level, engine.current_level_dgn_data,
        engine.current_level_dgn_size, level_index, source.canonical_hash_verified,
        (uint32_t)entry_index, (uint32_t)face_ordinal, &target) &&
        nexus_v1_dgn_structure3_capture_target_write(argv[5], &target);
    printf("canonical_dgn_bound=%d\n", source.canonical_hash_verified);
    printf("capture_target_written=%d\n", written ? 1 : 0);
    printf("original_saturn_capture_required=%d\n",
           target.original_saturn_capture_required);
    printf("no_draw_only=%d\n", target.no_draw_only);
    nexus_v1_shutdown(&engine);
    return written ? 0 : 1;
}
