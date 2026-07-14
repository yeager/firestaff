/* ReDMCSB TEXT.C F0768_TEXT_PrintToZoneWithTrailingSpaces, PC 3.4 route. */
#ifndef FIRESTAFF_REDMCSB_F0768_PRINT_TO_ZONE_WITH_TRAILING_SPACES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0768_PRINT_TO_ZONE_WITH_TRAILING_SPACES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0768_LOCAL_STRING_CAPACITY_PC34 = 80
};

typedef void (*redmcsb_f0768_text_print_pc34_compat)(
    void *context,
    uint8_t *bitmap_destination,
    uint16_t width,
    int16_t zone_index,
    int16_t text_color,
    int16_t background_color,
    const char *string,
    int16_t height);

/*
 * The original uses an 80-byte stack array without recovery for oversized
 * strings or widths. This portable boundary therefore has the same valid-data
 * precondition: source_string and string_length must fit that 80-byte array.
 */
void redmcsb_f0768_print_to_zone_with_trailing_spaces_pc34_compat(
    redmcsb_f0768_text_print_pc34_compat print_text,
    void *context,
    uint8_t *bitmap_destination,
    uint16_t width,
    int16_t zone_index,
    int16_t text_color,
    int16_t background_color,
    const char *source_string,
    int16_t string_length,
    int16_t height);

const char *redmcsb_f0768_print_to_zone_with_trailing_spaces_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
