#ifndef FIRESTAFF_REDMCSB_F0674_VIEWPORT_BITMAP_COPY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0674_VIEWPORT_BITMAP_COPY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNVIEW.C F0674_F0128_sub, PC I34E/I34M route. */
typedef const uint8_t *(*redmcsb_f0674_get_bitmap_pc34_compat)(
    void *context, int16_t graphic_index);
typedef size_t (*redmcsb_f0674_get_bitmap_byte_count_pc34_compat)(
    void *context, const uint8_t *bitmap);

typedef struct redmcsb_f0674_viewport_bitmap_runtime_pc34_compat {
    redmcsb_f0674_get_bitmap_pc34_compat get_bitmap;
    redmcsb_f0674_get_bitmap_byte_count_pc34_compat get_bitmap_byte_count;
    void *context;
} redmcsb_f0674_viewport_bitmap_runtime_pc34_compat;

/*
 * Copies one caller-owned cached bitmap exactly as F0674 does. The destination
 * capacity is the bounded host adapter's only addition to the PC contract.
 */
int redmcsb_f0674_copy_viewport_bitmap_pc34_compat(
    int16_t graphic_index,
    uint8_t *destination_bitmap,
    size_t destination_byte_count,
    const redmcsb_f0674_viewport_bitmap_runtime_pc34_compat *runtime,
    size_t *copied_byte_count);

const char *redmcsb_f0674_viewport_bitmap_copy_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
