/* Real HCSB.DAT structural probe.  This validates archive boundaries only;
 * it does not decode the platform-specific graphic stream. */
#include "csb_hint_oracle_dat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSB_HCSB_DAT");
    uint8_t *bytes = NULL;
    size_t byte_count = 0u;
    CSB_HintOracleDAT archive;
    const uint8_t *oracle_segment;
    size_t oracle_size;
    uint8_t *pixels;
    uint8_t rgb4[48];
    uint16_t width, height;
    size_t i;
    int rc;
    if (!path || !path[0]) {
        puts("SKIP: set FIRESTAFF_CSB_HCSB_DAT to a real HCSB.DAT path.");
        return 0;
    }
    /* The original Utility Disk may be a protected STX image.  The shared
     * virtual-path reader keeps HCSB.DAT in memory rather than requiring an
     * extracted duplicate beside the supplied game media. */
    if (!asset_read_virtual_path_alloc(path, &bytes, &byte_count)) {
        fprintf(stderr, "FAIL: cannot read %s\n", path);
        return 1;
    }
    rc = csb_hint_oracle_dat_parse(bytes, byte_count, &archive);
    if (rc != CSB_HINT_ORACLE_DAT_OK) {
        fprintf(stderr, "FAIL: HCSB.DAT parse: %s\n",
                csb_hint_oracle_dat_result_name(rc));
        free(bytes); return 1;
    }
    printf("HCSB.DAT segments=%u payload_offset=%zu file_size=%zu\n",
           (unsigned)archive.segment_count, archive.payload_offset, byte_count);
    for (i = 0u; i < archive.segment_count; ++i)
        printf("  segment[%zu] offset=%zu size=%u\n", i,
               archive.segment_offsets[i], (unsigned)archive.segment_sizes[i]);
    if (archive.segment_count > 1u &&
        csb_hint_oracle_dat_get_segment(&archive, 1u, &oracle_segment,
                                        &oracle_size) == CSB_HINT_ORACLE_DAT_OK) {
        pixels = (uint8_t *)malloc(640u * 400u);
        if (!pixels || !csb_hint_oracle_dat_img2_decode(oracle_segment,
                oracle_size, &width, &height, pixels, 640u * 400u, NULL) ||
            width != 320u || height != 200u) {
            fprintf(stderr, "FAIL: Oracle IMG2 decode\n");
            free(pixels); free(bytes); return 1;
        }
        free(pixels);
    }
    if (archive.segment_count <= 3u ||
        csb_hint_oracle_dat_get_segment(&archive, 3u, &oracle_segment,
                                        &oracle_size) != CSB_HINT_ORACLE_DAT_OK ||
        !csb_hint_oracle_dat_palette_decode(oracle_segment, oracle_size, rgb4)) {
        fprintf(stderr, "FAIL: Hint Oracle palette decode\n");
        free(bytes); return 1;
    }
    free(bytes);
    return 0;
}
