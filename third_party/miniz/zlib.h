/*
 * Firestaff bundled zlib-compatible header.
 *
 * CMake points zlib-enabled targets at this directory only when the bundled
 * miniz provider is selected.  The Firestaff code uses the small zlib API
 * subset exposed by miniz: z_stream, deflate/inflate, compressBound, CRC-32,
 * and the standard Z_* constants.
 */
#ifndef FIRESTAFF_THIRD_PARTY_MINIZ_ZLIB_H
#define FIRESTAFF_THIRD_PARTY_MINIZ_ZLIB_H

#include "miniz.h"

#endif /* FIRESTAFF_THIRD_PARTY_MINIZ_ZLIB_H */
