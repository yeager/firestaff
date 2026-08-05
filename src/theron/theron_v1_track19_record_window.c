#include "theron_v1_track19_record_window.h"

/* FNV-1a over the identical 502-byte US/JP window in the real ISO corpus. */
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_FNV1A 0xC48424F2u

int theron_v1_track19_opaque_record_window_validate(
        const uint8_t *iso, size_t iso_size, int japanese_variant,
        size_t *out_offset, size_t *out_bytes) {
    const size_t offset = japanese_variant
        ? THERON_TRACK19_OPAQUE_RECORD_WINDOW_JP_OFFSET
        : THERON_TRACK19_OPAQUE_RECORD_WINDOW_US_OFFSET;
    uint32_t hash = 2166136261u;

    if (!iso || iso_size < offset + THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES) {
        return 0;
    }
    for (size_t i = 0u; i < THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES; ++i) {
        hash ^= iso[offset + i];
        hash *= 16777619u;
    }
    if (hash != THERON_TRACK19_OPAQUE_RECORD_WINDOW_FNV1A) return 0;
    if (out_offset) *out_offset = offset;
    if (out_bytes) *out_bytes = THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES;
    return 1;
}
