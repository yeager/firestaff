#include "dm2_v1_fmtowns_cd_dat.h"
#include <string.h>

int dm2_v1_fmtowns_cd_dat_parse(const uint8_t *data, size_t size,
                                DM2_V1_FmtownsCdDatReceipt *out) {
    unsigned audio_idx = 0;
    unsigned i;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!data || size < DM2_FMTOWNS_CD_DAT_SIZE) return -1;

    for (i = 0; i < DM2_FMTOWNS_CD_DAT_ENTRY_COUNT; i++) {
        const uint8_t *p = data + i * DM2_FMTOWNS_CD_DAT_ENTRY_SIZE;
        out->entries[i].tbios_param0 = p[0];
        out->entries[i].tbios_param1 = p[1];
        out->entries[i].tbios_param2 = p[2];
        out->entries[i].type_flag    = p[3];

        if (p[3] == DM2_FMTOWNS_CD_TYPE_DATA) {
            out->data_entries++;
        } else if (p[3] == DM2_FMTOWNS_CD_TYPE_AUDIO_FIRST ||
                   p[3] == DM2_FMTOWNS_CD_TYPE_AUDIO) {
            if (audio_idx < DM2_FMTOWNS_CDDA_TRACK_COUNT) {
                out->audio_disc_tracks[audio_idx] = (uint8_t)(i + 1);
            }
            audio_idx++;
            out->audio_entries++;
        }
    }

    out->valid = 1;
    return 0;
}

uint32_t dm2_v1_fmtowns_cd_dat_audio_track_count(
    const DM2_V1_FmtownsCdDatReceipt *receipt) {
    if (!receipt || !receipt->valid) return 0;
    return receipt->audio_entries;
}

uint8_t dm2_v1_fmtowns_cd_dat_disc_track(
    const DM2_V1_FmtownsCdDatReceipt *receipt, int audio_index) {
    if (!receipt || !receipt->valid) return 0;
    if (audio_index < 0 || audio_index >= (int)DM2_FMTOWNS_CDDA_TRACK_COUNT)
        return 0;
    return receipt->audio_disc_tracks[audio_index];
}
