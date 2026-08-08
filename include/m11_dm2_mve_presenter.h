#ifndef FIRESTAFF_M11_DM2_MVE_PRESENTER_H
#define FIRESTAFF_M11_DM2_MVE_PRESENTER_H

#include "dm2_v1_mve_audio_sdl_owner.h"
#include "dm2_v1_mve_presentation_owner.h"
#include "dm2_v1_mve_timeline.h"

#include <stddef.h>
#include <stdint.h>

/* M11's narrow, source-only seam for the DOS IBMIOP movie route.  A caller
 * gives this object one already hash-admitted INTRO/END byte span in RAM and
 * a monotonic microsecond clock.  The presenter does not open a file, cache
 * an extracted member, resample PCM or turn a missed clock deadline into a
 * fabricated image.
 *
 * SKProject SKWIN/SkWinCore.cpp::INIT enters SHOW_MENU_SCREEN only after the
 * outer launcher has returned to SKULL.  This seam deliberately ends before
 * that menu transition: it has no authority to draw a replacement title.
 */
typedef int (*M11_Dm2MvePresentIndexedFn)(
    void *context, const uint8_t *indexed_pixels,
    const uint8_t palette_rgb6[256][3], uint32_t presentation_index,
    uint64_t presentation_time_us);

typedef struct {
    DM2_V1_MvePresentationOwner source;
    DM2_V1_MveAudioSdlOwner audio;
    DM2_V1_MveDisplayAudioBoundary *boundaries;
    uint32_t boundary_count;
    uint32_t next_presentation_index;
    uint64_t clock_origin_us;
    uint64_t last_host_time_us;
    M11_Dm2MvePresentIndexedFn present_indexed;
    void *present_context;
    int initialized;
    int ended;
    int failed;
} M11_Dm2MvePresenter;

/* Establishes an M11 presentation seam only for a source stream which has
 * already been admitted by the DOS media owner.  `clock_origin_us` belongs
 * to the caller's monotonic clock; the MVE timer supplies every deadline
 * after it.  The SDL audio stream is opened in the exact source U8/stereo/
 * 22050 Hz format.  A failure leaves the object cleared. */
int m11_dm2_mve_presenter_open(
    M11_Dm2MvePresenter *presenter, const uint8_t *verified_movie_bytes,
    size_t verified_movie_byte_count, uint64_t clock_origin_us,
    M11_Dm2MvePresentIndexedFn present_indexed, void *present_context);

/* Advances at most one source presentation.  Returns 1 when it queues all
 * PCM packets that occur before that source display and presents that exact
 * PAL8 page; 0 while the next source deadline has not arrived or after the
 * final display; -1 for a clock/order/SDL/sink error.  It never skips or
 * synthesizes a frame to catch up with the host. */
int m11_dm2_mve_presenter_advance(M11_Dm2MvePresenter *presenter,
                                  uint64_t monotonic_now_us);

/* Releases only M11-owned boundary storage and the SDL stream.  Movie bytes
 * stay caller-owned; they are never written to disk by this interface. */
void m11_dm2_mve_presenter_close(M11_Dm2MvePresenter *presenter);

#endif /* FIRESTAFF_M11_DM2_MVE_PRESENTER_H */
