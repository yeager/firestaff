#include "nexus_v1_raw_bin.h"
#include <string.h>

int nexus_v1_raw_bin_decode(const uint8_t *data, int data_size,
                             Nexus_V1_RawBinDecodeResult *out) {
    int i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 4) return 0;

    out->file_size = data_size;
    out->prs3_offset = -1;

    for (i = 0; i < data_size; i++) {
        if (data[i] != 0) out->non_zero_bytes++;
    }

    for (i = 0; i <= data_size - 4; i += 4) {
        if ((((uint32_t)data[i] << 24) | ((uint32_t)data[i + 1] << 16) |
             ((uint32_t)data[i + 2] << 8) | (uint32_t)data[i + 3]) ==
            0x50525333U) {
            out->prs3_offset = i;
            break;
        }
    }

    /*
     * Strict-fidelity boundary: byte-frequency/signature heuristics cannot
     * establish SH-2 ownership, VDP1/VDP2 format, or tilemap semantics.
     * Keep this decoder as a bounded retail receipt (size, non-zero count,
     * PRS3 marker, and hash) until Saturn capture or disassembly proves the
     * consumer.  In particular, NBG3.BIN, SWTCHR.BIN, and TM.BIN must not be
     * advertised as drawable data based on incidental opcode-like bytes.
     */
    out->content_type = NEXUS_RAW_TYPE_UNKNOWN;

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; i++) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->data_hash = fnv;

    out->valid = 1;
    return 1;
}
