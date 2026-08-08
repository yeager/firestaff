#ifndef FIRESTAFF_DM2_V1_MVE_PRESENTATION_OWNER_H
#define FIRESTAFF_DM2_V1_MVE_PRESENTATION_OWNER_H

#include "dm2_v1_mve_pcm.h"
#include "dm2_v1_mve_video.h"

#include <stddef.h>
#include <stdint.h>

/* Private, memory-only owner for one verified PC-DOS DM2 MVE member.
 *
 * The owner is deliberately below M11: it neither creates an SDL texture nor
 * queues PCM to an audio device.  It advances the original display order,
 * retains the indexed video pages and the raw PCM transport directly from
 * caller-owned movie bytes. PCM stays separately ordered because MVE audio
 * is not one-to-one with display chunks. This keeps INTRO/END intact in RAM
 * while a later host owns the real presentation destinations. */
typedef struct {
    int valid;
    uint32_t presentation_index;
    uint64_t presentation_time_us;
    uint64_t presentation_interval_us;
    const uint8_t *indexed_pixels;
    const uint8_t *palette_rgb;
} DM2_V1_MvePresentationFrame;

typedef struct {
    const uint8_t *movie_bytes;
    size_t movie_byte_count;
    DM2_V1_MvePresentationIterator presentation_iterator;
    DM2_V1_MveAudioIterator audio_iterator;
    DM2_V1_MveVideo video;
    DM2_V1_MvePcm source_pcm;
    uint64_t clock_time_us;
    uint64_t presentation_interval_us;
    uint32_t presented_frame_count;
    uint64_t retained_pcm_bytes;
    int initialized;
    int ended;
    int failed;
} DM2_V1_MvePresentationOwner;

/* Opens only a stream accepted by the strict video and PCM iterators.  The
 * bytes remain caller-owned and must stay alive until owner reset. */
int dm2_v1_mve_presentation_owner_init(
    DM2_V1_MvePresentationOwner *owner, const uint8_t *movie_bytes,
    size_t movie_byte_count);

/* Advances exactly one source display.  Returns 1 with a RAM-only frame,
 * 0 at the source display end, or -1 on malformed media. The display clock
 * comes solely from the MVE timer; it does not infer a host refresh rate. */
int dm2_v1_mve_presentation_owner_next(
    DM2_V1_MvePresentationOwner *owner, DM2_V1_MvePresentationFrame *out);

/* Retains the raw MVE PCM transport in its original source order, including
 * the prebuffer before the first display. It is intentionally independent of
 * the display clock: the retail files prove that these streams are not
 * positionally one-to-one. Returns 1, 0 at audio EOS, or -1 on malformed
 * media. */
int dm2_v1_mve_presentation_owner_next_source_pcm(
    DM2_V1_MvePresentationOwner *owner, DM2_V1_MvePcmFrame *out);

#endif /* FIRESTAFF_DM2_V1_MVE_PRESENTATION_OWNER_H */
