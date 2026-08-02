#include "dm2_v1_fmtowns_cd_dat.h"
#include <string.h>

int dm2_v1_fmtowns_cd_dat_parse(const uint8_t *data, size_t size,
                                DM2_V1_FmtownsCdDatReceipt *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!data || size < DM2_FMTOWNS_CD_DAT_SIZE) return -1;

    for (unsigned i = 0; i < DM2_FMTOWNS_CD_DAT_ENTRY_COUNT; i++) {
        const uint8_t *p = data + i * DM2_FMTOWNS_CD_DAT_ENTRY_SIZE;
        out->entries[i].byte0     = p[0];
        out->entries[i].byte1     = p[1];
        out->entries[i].byte2     = p[2];
        out->entries[i].type_flag = p[3];

        if (p[3] == 0x06) out->data_entries++;
        else if (p[3] == 0x02 || p[3] == 0x03) out->audio_entries++;
    }

    out->valid = 1;
    return 0;
}

uint32_t dm2_v1_fmtowns_cd_dat_audio_track_count(
    const DM2_V1_FmtownsCdDatReceipt *receipt) {
    if (!receipt || !receipt->valid) return 0;
    return receipt->audio_entries;
}
