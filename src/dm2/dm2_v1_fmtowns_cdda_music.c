/*
 * dm2_v1_fmtowns_cdda_music.c — FM Towns HMP-to-CDDA track mapping.
 *
 * Source: HME-242 SKULL.EXP offset 0x3dac, read from the selected CD image.
 */

#include "dm2_v1_fmtowns_cdda_music.h"

#include <string.h>

static uint32_t dm2_v1_fmtowns_cdda_music_fnv1a(
    const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm2_v1_fmtowns_cdda_music_parse(
    const uint8_t *skull_data, size_t skull_size,
    DM2_V1_FmtownsCddaMusicReceipt *out)
{
    DM2_V1_FmtownsCddaMusicReceipt receipt;
    size_t i;

    memset(&receipt, 0, sizeof(receipt));
    if (!skull_data || skull_size < DM2_FMTOWNS_SKULL_HMP_CDDA_OFFSET +
                                      DM2_FMTOWNS_HMP_TRACK_COUNT) {
        if (out) *out = receipt;
        return 0;
    }
    memcpy(receipt.hmp_to_cdda,
           skull_data + DM2_FMTOWNS_SKULL_HMP_CDDA_OFFSET,
           sizeof(receipt.hmp_to_cdda));
    for (i = 0u; i < sizeof(receipt.hmp_to_cdda); ++i) {
        if (receipt.hmp_to_cdda[i] > 9u) {
            if (out) *out = receipt;
            return 0;
        }
    }
    receipt.valid = 1;
    receipt.source_offset = DM2_FMTOWNS_SKULL_HMP_CDDA_OFFSET;
    receipt.source_size = DM2_FMTOWNS_HMP_TRACK_COUNT;
    receipt.source_fnv1a = dm2_v1_fmtowns_cdda_music_fnv1a(
        receipt.hmp_to_cdda, sizeof(receipt.hmp_to_cdda));
    if (out) *out = receipt;
    return 1;
}

int dm2_v1_fmtowns_hmp_to_cdda(
    const DM2_V1_FmtownsCddaMusicReceipt *receipt, int hmp_track)
{
    if (!receipt || !receipt->valid ||
        hmp_track < 0 || hmp_track >= DM2_FMTOWNS_HMP_TRACK_COUNT) {
        return 0;
    }
    return receipt->hmp_to_cdda[hmp_track];
}

const uint8_t *dm2_v1_fmtowns_cdda_map_table(
    const DM2_V1_FmtownsCddaMusicReceipt *receipt)
{
    return receipt && receipt->valid ? receipt->hmp_to_cdda : NULL;
}
