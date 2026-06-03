/*
 * theron_v1_track02.c -- narrow Track 02 evidence helpers.
 *
 * This is not a Theron's Quest dungeon loader.  It locks one small,
 * hash-gated Track 02 bank-descriptor signal so later dungeon-bank work
 * can build from bytes that are regression-proved against real data.
 *
 * Source/evidence:
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md sections 1.3,
 *   1.5, and 4.2 record Track 02 provenance and state that the dungeon
 *   format remains unknown.  The offset below is from local byte inspection
 *   of the hash-verified US Track 02 ISO, not from ReDMCSB.
 */

#include "theron_v1_track02.h"

#include <string.h>

#define TQR_US_ISO_BANK_STRIDE_OFFSET 0x1584u
#define TQR_US_ISO_BANK_STRIDE_COUNT  9u
#define TQR_US_ISO_BANK_STRIDE_BYTES  (TQR_US_ISO_BANK_STRIDE_COUNT * 2u)
#define TQR_US_ISO_BANK_STRIDE_STEP   0x0400u

static const uint8_t g_us_iso_bank_stride_descriptor[TQR_US_ISO_BANK_STRIDE_BYTES] = {
    0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
    0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
};

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

Theron_Track02Variant theron_v1_track02_variant_for_md5(const char *md5_hex) {
    if (!md5_hex) return THERON_TRACK02_VARIANT_UNKNOWN;
    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_ISO) == 0) {
        return THERON_TRACK02_VARIANT_US_ISO;
    }
    if (strcmp(md5_hex, THERON_TRACK02_MD5_JP_REV1_ISO) == 0) {
        return THERON_TRACK02_VARIANT_JP_REV1_ISO;
    }
    return THERON_TRACK02_VARIANT_UNKNOWN;
}

static int track_is_all_zero(const uint8_t *data, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) {
        if (data[i] != 0) return 0;
    }
    return 1;
}

static size_t count_pattern_occurrences(const uint8_t *data,
                                        size_t size,
                                        const uint8_t *pattern,
                                        size_t pattern_size) {
    size_t count = 0;
    size_t i;

    if (!data || !pattern || pattern_size == 0 || size < pattern_size) return 0;
    for (i = 0; i <= size - pattern_size; ++i) {
        if (memcmp(data + i, pattern, pattern_size) == 0) {
            ++count;
        }
    }
    return count;
}

Theron_Track02SignalStatus theron_v1_track02_find_bank_signal(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02BankSignal *out_signal) {

    Theron_Track02Variant variant;
    size_t occurrence_count;

    if (out_signal) {
        memset(out_signal, 0, sizeof(*out_signal));
    }
    if (!track02_data || track02_size == 0 || !md5_hex || !out_signal) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    variant = theron_v1_track02_variant_for_md5(md5_hex);
    out_signal->variant = variant;

    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO) {
        return track_is_all_zero(track02_data, track02_size)
            ? THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE
            : THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (variant != THERON_TRACK02_VARIANT_US_ISO) {
        return THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT;
    }

    if (track02_size < TQR_US_ISO_BANK_STRIDE_OFFSET + TQR_US_ISO_BANK_STRIDE_BYTES) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    if (memcmp(track02_data + TQR_US_ISO_BANK_STRIDE_OFFSET,
               g_us_iso_bank_stride_descriptor,
               TQR_US_ISO_BANK_STRIDE_BYTES) != 0) {
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }

    occurrence_count = count_pattern_occurrences(track02_data,
                                                 track02_size,
                                                 g_us_iso_bank_stride_descriptor,
                                                 TQR_US_ISO_BANK_STRIDE_BYTES);
    out_signal->descriptor_offset = TQR_US_ISO_BANK_STRIDE_OFFSET;
    out_signal->descriptor_size = TQR_US_ISO_BANK_STRIDE_BYTES;
    out_signal->occurrence_count = occurrence_count;
    out_signal->first_value = rd16le(g_us_iso_bank_stride_descriptor);
    out_signal->last_value =
        rd16le(g_us_iso_bank_stride_descriptor + TQR_US_ISO_BANK_STRIDE_BYTES - 2u);
    out_signal->stride = TQR_US_ISO_BANK_STRIDE_STEP;
    out_signal->value_count = TQR_US_ISO_BANK_STRIDE_COUNT;

    return occurrence_count == 1u
        ? THERON_TRACK02_SIGNAL_OK
        : THERON_TRACK02_SIGNAL_NOT_FOUND;
}

const char *theron_v1_track02_signal_status_name(Theron_Track02SignalStatus status) {
    switch (status) {
    case THERON_TRACK02_SIGNAL_OK:
        return "ok";
    case THERON_TRACK02_SIGNAL_NOT_FOUND:
        return "not-found";
    case THERON_TRACK02_SIGNAL_BAD_INPUT:
        return "bad-input";
    case THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT:
        return "unsupported-variant";
    case THERON_TRACK02_SIGNAL_INSUFFICIENT_ZERO_IMAGE:
        return "insufficient-zero-image";
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_variant_name(Theron_Track02Variant variant) {
    switch (variant) {
    case THERON_TRACK02_VARIANT_JP_REV1_ISO:
        return "jp-rev1-iso";
    case THERON_TRACK02_VARIANT_US_ISO:
        return "us-iso";
    case THERON_TRACK02_VARIANT_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *theron_v1_track02_source_evidence(void) {
    return "theron_v1_track02.c: US Track 02 ISO MD5 "
           THERON_TRACK02_MD5_US_ISO
           " has a unique little-endian bank-stride descriptor at offset "
           "0x1584 (9 words, 0x0020..0x2020, stride 0x0400); JP Rev 1 ISO "
           THERON_TRACK02_MD5_JP_REV1_ISO
           " is hash-verified but zero-filled in the available image, so no "
           "JP dungeon-bank offset is claimed.";
}
