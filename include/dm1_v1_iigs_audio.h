#ifndef DM1_V1_IIGS_AUDIO_H
#define DM1_V1_IIGS_AUDIO_H

#include "dm1_v1_iigs_metadata.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_IIGS_SOUND_COUNT 34U

typedef struct DM1_V1_IigsSoundSpec {
    int16_t graphic_index;
    uint8_t a_unreferenced;
    uint8_t period; /* Native DOC frequency increment, not a timer divisor. */
    uint8_t priority;
    uint8_t b_unreferenced;
    uint8_t loud_distance;
    uint8_t soft_distance;
} DM1_V1_IigsSoundSpec;

typedef struct DM1_V1_IigsAudioMetadata {
    DM1_V1_IigsProgramVersion program_version;
    DM1_V1_IigsSoundSpec sounds[DM1_V1_IIGS_SOUND_COUNT];
    uint8_t distance_volume[25][25];
} DM1_V1_IigsAudioMetadata;

typedef struct DM1_V1_IigsSoundVolume {
    uint8_t left;
    uint8_t right;
} DM1_V1_IigsSoundVolume;

typedef struct DM1_V1_IigsPcmReceipt {
    size_t sample_count;   /* N audio bytes, excluding the stop marker. */
    size_t doc_byte_count; /* N+1 including the terminal DOC zero byte. */
    uint8_t table_size_code;
    uint8_t resolution_code;
} DM1_V1_IigsPcmReceipt;

/* Authenticate the original G20/G21 program and copy its native tables. */
int dm1_v1_iigs_audio_metadata_read(const uint8_t *program, size_t size,
                                   DM1_V1_IigsAudioMetadata *out);

/* Structural sound-record API. Caller authenticates the containing bank and
 * selects a native SOUND_DATA record (not unused placeholders 538/548).
 * BE16 N, N signed8 samples and at most two alignment bytes; N<=32767.
 * Decode emits DOC unsigned8 samples: signed+128, with zero remapped to one,
 * followed by a separate zero stop marker. Input/output must not overlap.
 * Returns 1 on success; zeroed receipt on failure. Samples may be
 * partial on failure. No sample rate, device/backend or game admission. */
int dm1_v1_iigs_audio_pcm_query(const uint8_t *record, size_t size,
                               DM1_V1_IigsPcmReceipt *out);
int dm1_v1_iigs_audio_pcm_decode(const uint8_t *record, size_t size,
    uint8_t *doc_bytes, size_t capacity, DM1_V1_IigsPcmReceipt *out);

/* Native F0505 distance/channel lookup and F0060 integer quantization. */
int dm1_v1_iigs_audio_volume(const DM1_V1_IigsAudioMetadata *metadata,
    int party_x, int party_y, unsigned direction, int source_x, int source_y,
    DM1_V1_IigsSoundVolume *out);
int dm1_v1_iigs_audio_quantize_volume(DM1_V1_IigsSoundVolume requested,
    unsigned master_level, DM1_V1_IigsSoundVolume *out);

typedef struct DM1_V1_IigsPendingSounds {
    DM1_V1_IigsSoundVolume volumes[DM1_V1_IIGS_SOUND_COUNT];
} DM1_V1_IigsPendingSounds;
typedef void (*DM1_V1_IigsPendingVisitor)(unsigned sound_index,
    DM1_V1_IigsSoundVolume volume, void *user_data);
/* F0064 replaces only for greater L+R, without a priority tie-break. F0065
 * flushes in sound-index order only when BOTH channels are nonzero, clearing
 * delivered pairs. A one-sided pending pair remains pending, as in source. */
int dm1_v1_iigs_audio_pending_merge(DM1_V1_IigsPendingSounds *pending,
    unsigned sound_index, DM1_V1_IigsSoundVolume volume);
int dm1_v1_iigs_audio_pending_flush(DM1_V1_IigsPendingSounds *pending,
    DM1_V1_IigsPendingVisitor visitor, void *user_data);

#ifdef __cplusplus
}
#endif
#endif
