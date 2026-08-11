/* Real HCSB.DAT structural probe.  This validates archive boundaries only;
 * it does not decode the platform-specific graphic stream. */
#include "csb_hint_oracle_dat.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *fp;
    long length;
    uint8_t *bytes;
    if (!path || !out || !out_size || !(fp = fopen(path, "rb"))) return 0;
    if (fseek(fp, 0, SEEK_END) != 0 || (length = ftell(fp)) < 0 ||
        fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    bytes = (uint8_t *)malloc((size_t)length ? (size_t)length : 1u);
    if (!bytes || fread(bytes, 1u, (size_t)length, fp) != (size_t)length) {
        free(bytes); fclose(fp); return 0;
    }
    fclose(fp); *out = bytes; *out_size = (size_t)length; return 1;
}

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSB_HCSB_DAT");
    uint8_t *bytes = NULL;
    size_t byte_count = 0u;
    CSB_HintOracleDAT archive;
    const uint8_t *oracle_segment;
    size_t oracle_size;
    uint8_t *pixels;
    uint16_t width, height;
    size_t i;
    int rc;
    if (!path || !path[0]) {
        puts("SKIP: set FIRESTAFF_CSB_HCSB_DAT to a real HCSB.DAT path.");
        return 0;
    }
    if (!read_file(path, &bytes, &byte_count)) {
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
    free(bytes);
    return 0;
}
