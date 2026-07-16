#include "dm2_v1_pict_picst_helpers.h"

#include <limits.h>
#include <string.h>

static void dm2_pict_picst_receipt_begin(
    DM2_V1_PictPicstReceipt *receipt,
    const char *symbol,
    const char *source_path,
    int category,
    int index,
    int field)
{
    dm2_v1_pict_picst_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->category = category;
    receipt->index = index;
    receipt->field = field;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static int dm2_pict_bits_are_bounded(const DM2_V1_PictBits *bits)
{
    uint32_t pixels;

    if (!bits || bits->width == 0u || bits->height == 0u ||
        bits->bits_per_pixel == 0u || bits->bits_per_pixel > 8u ||
        bits->raw_byte_count == 0u || bits->source_hash == 0u) {
        return 0;
    }
    pixels = (uint32_t)bits->width * (uint32_t)bits->height;
    if (pixels == 0u || pixels > (uint32_t)1024u * 1024u) {
        return 0;
    }
    return 1;
}

void dm2_v1_pict_picst_receipt_clear(DM2_V1_PictPicstReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int dm2_v1_QUERY_PICT_BITS(
    DM2_V1_QueryPictBitsProvider provider,
    void *userdata,
    int category,
    int index,
    int field,
    DM2_V1_PictBits *out_bits,
    DM2_V1_PictPicstReceipt *out_receipt)
{
    DM2_V1_PictBits bits;

    if (out_bits) {
        memset(out_bits, 0, sizeof(*out_bits));
    }
    dm2_pict_picst_receipt_begin(out_receipt,
                                 "QUERY_PICT_BITS",
                                 "SKWIN/SkWinCore.cpp:6006",
                                 category,
                                 index,
                                 field);
    if (!provider || category < 0 || category > 0xff ||
        index < 0 || index > 0xff || field < 0 || field > 0xff) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    memset(&bits, 0, sizeof(bits));
    if (!provider(category, index, field, &bits, userdata) ||
        !dm2_pict_bits_are_bounded(&bits)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_bits) {
        *out_bits = bits;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->pixel_count =
            (uint32_t)bits.width * (uint32_t)bits.height;
        out_receipt->source_hash = bits.source_hash;
    }
    return 1;
}

int dm2_v1_QUERY_PICST_IMAGE(
    DM2_V1_QueryPicstImageProvider provider,
    void *userdata,
    int category,
    int index,
    int field,
    const uint8_t **out_pixels,
    size_t *out_pixel_count,
    DM2_V1_PictPicstReceipt *out_receipt)
{
    DM2_V1_PictBits bits;
    const uint8_t *pixels = 0;
    size_t pixel_count = 0u;
    uint32_t expected_pixels;

    if (out_pixels) {
        *out_pixels = 0;
    }
    if (out_pixel_count) {
        *out_pixel_count = 0u;
    }
    dm2_pict_picst_receipt_begin(out_receipt,
                                 "QUERY_PICST_IMAGE",
                                 "SKWIN/SkWinCore.cpp:6113",
                                 category,
                                 index,
                                 field);
    if (!provider || category < 0 || category > 0xff ||
        index < 0 || index > 0xff || field < 0 || field > 0xff) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    memset(&bits, 0, sizeof(bits));
    if (!provider(category, index, field, &pixels, &pixel_count, &bits,
                  userdata) ||
        !pixels || !dm2_pict_bits_are_bounded(&bits)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    expected_pixels = (uint32_t)bits.width * (uint32_t)bits.height;
    if (pixel_count != (size_t)expected_pixels ||
        pixel_count > (size_t)UINT_MAX) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_pixels) {
        *out_pixels = pixels;
    }
    if (out_pixel_count) {
        *out_pixel_count = pixel_count;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->pixel_count = expected_pixels;
        out_receipt->source_hash = bits.source_hash;
    }
    return 1;
}

int dm2_v1_QUERY_PICST_IT(
    const DM2_V1_PicstItEntry *entries,
    size_t entry_count,
    size_t selector,
    DM2_V1_PicstItEntry *out_entry,
    DM2_V1_PictPicstReceipt *out_receipt)
{
    const DM2_V1_PicstItEntry *entry;

    if (out_entry) {
        memset(out_entry, 0, sizeof(*out_entry));
    }
    dm2_pict_picst_receipt_begin(out_receipt,
                                 "QUERY_PICST_IT",
                                 "SKWIN/SkWinCore.cpp:6647",
                                 -1,
                                 -1,
                                 -1);
    if (!entries || selector >= entry_count || entry_count > 1024u) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    entry = &entries[selector];
    if (entry->category < 0 || entry->category > 0xff ||
        entry->index < 0 || entry->index > 0xff ||
        entry->field < 0 || entry->field > 0xff ||
        !dm2_pict_bits_are_bounded(&entry->bits)) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_entry) {
        *out_entry = *entry;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->category = entry->category;
        out_receipt->index = entry->index;
        out_receipt->field = entry->field;
        out_receipt->pixel_count =
            (uint32_t)entry->bits.width * (uint32_t)entry->bits.height;
        out_receipt->source_hash = entry->bits.source_hash;
    }
    return 1;
}

const char *dm2_v1_pict_picst_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp QUERY_PICT_BITS:6006 "
           "QUERY_PICST_IMAGE:6113 QUERY_PICST_IT:6647; bounded GDAT "
           "PICT/PICST receipt helpers only, with caller-owned source pixels.";
}
