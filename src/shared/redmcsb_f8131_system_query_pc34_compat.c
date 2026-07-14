#include "redmcsb_f8131_system_query_pc34_compat.h"

void redmcsb_f8131_get_volume_name_pc34_compat(
    redmcsb_f8131_find_volume_label_pc34_compat find_volume_label,
    void *context,
    uint8_t drive,
    char out_volume_name[REDMCSB_F8131_VOLUME_NAME_CAPACITY_PC34])
{
    uint8_t volume_label[REDMCSB_F8131_VOLUME_LABEL_BYTES_PC34];
    unsigned int input_index;
    unsigned int output_index = 0U;

    out_volume_name[0] = '\0';
    if (find_volume_label == 0 ||
        !find_volume_label(context, drive, volume_label)) {
        return;
    }

    for (input_index = 0U;
         input_index < REDMCSB_F8131_VOLUME_LABEL_BYTES_PC34;
         ++input_index) {
        uint8_t character = volume_label[input_index];

        if (character != (uint8_t)' ' && character != (uint8_t)'.') {
            out_volume_name[output_index++] = (char)character;
        }
        if (character == 0U) {
            break;
        }
    }
    out_volume_name[output_index] = '\0';
}

uint16_t redmcsb_f8132_get_random_seed_pc34_compat(
    redmcsb_f8131_get_dos_time_pc34_compat get_dos_time,
    void *context)
{
    uint8_t seconds = 0U;
    uint8_t hundredths = 0U;

    if (get_dos_time != 0) {
        get_dos_time(context, &seconds, &hundredths);
    }
    return (uint16_t)(((uint16_t)seconds << 8) | hundredths);
}

void redmcsb_f8133_read_floppy_sector_pc34_compat(void)
{
}

const char *redmcsb_f8131_system_query_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:2259-2316; MEDIA701_I34E PC route";
}
