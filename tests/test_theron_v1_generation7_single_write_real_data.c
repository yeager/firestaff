#include "theron_v1_raw_loader_trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *load(const char *path, size_t *size) {
    FILE *f = path ? fopen(path, "rb") : NULL; long n; unsigned char *p;
    *size = 0u;
    if (!f || fseek(f, 0, SEEK_END) || (n = ftell(f)) <= 0 || fseek(f, 0, SEEK_SET)) { if (f) fclose(f); return NULL; }
    p = malloc((size_t)n + 1u);
    if (!p || fread(p, 1u, (size_t)n, f) != (size_t)n) { free(p); fclose(f); return NULL; }
    fclose(f); p[n] = 0; *size = (size_t)n; return p;
}

int main(void) {
    const char *paths[8] = { getenv("FIRESTAFF_THERON_US_TRACK02_BIN"), getenv("FIRESTAFF_THERON_GENERATION6_CD_TRACE"), getenv("FIRESTAFF_THERON_GENERATION6_VDC_TRACE"), getenv("FIRESTAFF_THERON_GENERATION6_VRAM"), getenv("FIRESTAFF_THERON_GENERATION7_VDC_TRACE"), getenv("FIRESTAFF_THERON_GENERATION7_VDC_STATE"), getenv("FIRESTAFF_THERON_GENERATION7_VRAM"), getenv("FIRESTAFF_THERON_GENERATION7_VCE") };
    unsigned char *d[8] = {0}; size_t z[8] = {0}, i;
    Theron_V1RawLoaderTraceGameE009NextParametersReceipt n = {0};
    Theron_V1RawLoaderTraceGameE009VdcReceipt g;
    Theron_V1RawLoaderTraceGameE009VdcPresentationReceipt r;
    for (i = 0; i < 8; ++i) if (!paths[i]) { puts("SKIP: authentic generation-7 inputs are not configured"); return 77; }
    for (i = 0; i < 8; ++i) if (!(d[i] = load(paths[i], &z[i]))) { fputs("FAIL: cannot read generation-7 inputs\n", stderr); goto fail; }
    n.valid = 1; n.variant = THERON_TRACK02_VARIANT_US_BIN; memcpy(n.track02_md5, THERON_TRACK02_MD5_US_BIN, 33u); memcpy(n.next_parameters, "\x00\x20\x00\x10\x00\x06\xf8\xfe", 8u); n.consumer_output_writes_verified = n.tii_verified = n.next_parameters_verified = 1;
    if (!theron_v1_raw_loader_trace_bind_game_e009_vdc_payload(&n, (char *)d[1], (char *)d[2], d[0], z[0], d[3], z[3], THERON_TRACK02_MD5_US_BIN, &g) ||
        !theron_v1_raw_loader_trace_bind_game_e009_vdc_presentation(&g, (char *)d[4], (char *)d[5], d[3], z[3], d[6], z[6], d[7], z[7], d[0], z[0], THERON_TRACK02_MD5_US_BIN, &r) ||
        !r.valid || r.vdc_rows != 2187u || r.vwr_commits != 1024u || r.pre_vram_checksum != 0xedfc7797u || r.post_vram_checksum != 0xd9d48117u || r.bat_checksum != 0x4740a645u || r.unique_source_tiles != 124u || r.nonzero_background_pixels != 3373u || r.background_index_checksum != 0x2c2cfb4du || r.background_color_checksum != 0x3fde1dc5u || !r.vdc_replay_verified || !r.background_pixels_verified || r.tile_semantics_proven) { fputs("FAIL: generation-7 single-write presentation rejected\n", stderr); goto fail; }
    d[6][0] ^= 1u;
    if (theron_v1_raw_loader_trace_bind_game_e009_vdc_presentation(&g, (char *)d[4], (char *)d[5], d[3], z[3], d[6], z[6], d[7], z[7], d[0], z[0], THERON_TRACK02_MD5_US_BIN, &r)) { fputs("FAIL: corrupted generation-7 VRAM accepted\n", stderr); goto fail; }
    for (i = 0; i < 8; ++i) free(d[i]);
    puts("PASS: authentic generation-7 BAT uses one hardware write per word"); return 0;
fail:
    for (i = 0; i < 8; ++i) free(d[i]); return 1;
}
