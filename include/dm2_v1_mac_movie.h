#ifndef DM2_V1_MAC_MOVIE_H
#define DM2_V1_MAC_MOVIE_H

#include <stddef.h>
#include <stdint.h>

/* A read-only, in-memory view of an authentic Macintosh MooV.  The original
 * HFS data and resource forks remain authoritative; this buffer only gives a
 * decoder the atom order it expects and is never written back to the game
 * data directory. */
typedef struct {
    uint8_t *bytes;
    size_t size;
    size_t moov_offset;
    size_t mdat_offset;
} DM2_V1_MacMovieView;

int dm2_v1_mac_movie_view_build(const uint8_t *data_fork, size_t data_size,
                                const uint8_t *moov, size_t moov_size,
                                DM2_V1_MacMovieView *out);
void dm2_v1_mac_movie_view_free(DM2_V1_MacMovieView *view);

#endif
