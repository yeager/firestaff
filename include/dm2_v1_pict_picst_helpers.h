#ifndef FIRESTAFF_DM2_V1_PICT_PICST_HELPERS_H
#define FIRESTAFF_DM2_V1_PICT_PICST_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    uint8_t format;
    uint32_t raw_byte_count;
    uint32_t source_hash;
} DM2_V1_PictBits;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int category;
    int index;
    int field;
    uint32_t pixel_count;
    uint32_t source_hash;
    const char *symbol;
    const char *source_path;
} DM2_V1_PictPicstReceipt;

typedef struct {
    int category;
    int index;
    int field;
    DM2_V1_PictBits bits;
} DM2_V1_PicstItEntry;

typedef int (*DM2_V1_QueryPictBitsProvider)(
    int category,
    int index,
    int field,
    DM2_V1_PictBits *out_bits,
    void *userdata);

typedef int (*DM2_V1_QueryPicstImageProvider)(
    int category,
    int index,
    int field,
    const uint8_t **out_pixels,
    size_t *out_pixel_count,
    DM2_V1_PictBits *out_bits,
    void *userdata);

void dm2_v1_pict_picst_receipt_clear(DM2_V1_PictPicstReceipt *receipt);

int dm2_v1_QUERY_PICT_BITS(
    DM2_V1_QueryPictBitsProvider provider,
    void *userdata,
    int category,
    int index,
    int field,
    DM2_V1_PictBits *out_bits,
    DM2_V1_PictPicstReceipt *out_receipt);

int dm2_v1_QUERY_PICST_IMAGE(
    DM2_V1_QueryPicstImageProvider provider,
    void *userdata,
    int category,
    int index,
    int field,
    const uint8_t **out_pixels,
    size_t *out_pixel_count,
    DM2_V1_PictPicstReceipt *out_receipt);

int dm2_v1_QUERY_PICST_IT(
    const DM2_V1_PicstItEntry *entries,
    size_t entry_count,
    size_t selector,
    DM2_V1_PicstItEntry *out_entry,
    DM2_V1_PictPicstReceipt *out_receipt);

const char *dm2_v1_pict_picst_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PICT_PICST_HELPERS_H */
