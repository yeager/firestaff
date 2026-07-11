#ifndef THERON_V1_STARTUP_MEDIA_IDENTITY_H
#define THERON_V1_STARTUP_MEDIA_IDENTITY_H

#include "theron_v1_world.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Lightweight Track 02 identity capture for save/resume paths that do not
 * need the full startup bitmap/roster receipt graph. */
void theron_v1_startup_media_capture_track02_identity(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_RuntimeMediaIdentity *out_identity);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_MEDIA_IDENTITY_H */
