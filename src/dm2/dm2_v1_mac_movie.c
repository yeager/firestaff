#include "dm2_v1_mac_movie.h"

#include <stdlib.h>
#include <string.h>

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int exact_atom(const uint8_t *bytes, size_t size, const char type[4]) {
    uint32_t atom_size;
    if (!bytes || size < 8u) return 0;
    atom_size = be32(bytes);
    if (atom_size < 8u || (size_t)atom_size != size) return 0;
    return memcmp(bytes + 4u, type, 4u) == 0;
}

int dm2_v1_mac_movie_view_build(const uint8_t *data_fork, size_t data_size,
                                const uint8_t *moov, size_t moov_size,
                                DM2_V1_MacMovieView *out) {
    uint8_t *bytes;
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!exact_atom(data_fork, data_size, "mdat") ||
        !exact_atom(moov, moov_size, "moov") ||
        moov_size > SIZE_MAX - data_size) return -1;
    bytes = (uint8_t *)malloc(moov_size + data_size);
    if (!bytes) return -1;
    memcpy(bytes, moov, moov_size);
    memcpy(bytes + moov_size, data_fork, data_size);
    out->bytes = bytes;
    out->size = moov_size + data_size;
    out->moov_offset = 0u;
    out->mdat_offset = moov_size;
    return 0;
}

void dm2_v1_mac_movie_view_free(DM2_V1_MacMovieView *view) {
    if (!view) return;
    free(view->bytes);
    memset(view, 0, sizeof(*view));
}
