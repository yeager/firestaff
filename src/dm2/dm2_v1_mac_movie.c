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

static int find_data_atom(const uint8_t *bytes, size_t size,
                          size_t *out_offset, size_t *out_size) {
    size_t offset;
    if (!bytes || !out_offset || !out_size) return 0;
    /* Some authentic MooV data forks carry a short, non-atom prefix before
     * the mdat atom.  It is part of the source fork, but not part of the
     * QuickTime atom stream consumed by the private decoder view. */
    for (offset = 0u; offset <= 64u && offset + 8u <= size; ++offset) {
        uint32_t atom_size = be32(bytes + offset);
        if ((atom_size == 0u ||
             (atom_size >= 8u && (size_t)atom_size == size - offset)) &&
            size - offset >= 8u &&
            memcmp(bytes + offset + 4u, "mdat", 4u) == 0) {
            *out_offset = offset;
            *out_size = size - offset;
            return 1;
        }
    }
    return 0;
}

static void put_be32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value >> 24); p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8); p[3] = (uint8_t)value;
}

static uint64_t be64(const uint8_t *p) {
    return ((uint64_t)be32(p) << 32) | be32(p + 4u);
}

static void put_be64(uint8_t *p, uint64_t value) {
    put_be32(p, (uint32_t)(value >> 32));
    put_be32(p + 4u, (uint32_t)value);
}

static int mac_movie_container(const uint8_t type[4]) {
    static const char containers[][4] = {
        "moov", "trak", "mdia", "minf", "dinf", "stbl", "edts", "udta", "meta"
    };
    size_t i;
    for (i = 0; i < sizeof(containers) / sizeof(containers[0]); ++i)
        if (memcmp(type, containers[i], 4u) == 0) return 1;
    return 0;
}

/* QuickTime's original resource fork stores chunk offsets against its
 * separate data fork.  The decoder view places the moov atom before mdat,
 * so preserve the source tables while rebasing only the private view. */
static void mac_movie_rebase_atoms(uint8_t *bytes, size_t size,
                                   size_t data_offset, size_t data_size) {
    size_t offset = 0u;
    while (offset + 8u <= size) {
        uint32_t atom_size = be32(bytes + offset);
        const uint8_t *type = bytes + offset + 4u;
        size_t header = 8u;
        size_t payload;
        if (atom_size == 1u) {
            if (offset + 16u > size) return;
            atom_size = 0u;
            if (be64(bytes + offset + 8u) > SIZE_MAX) return;
            atom_size = (uint32_t)be64(bytes + offset + 8u);
            header = 16u;
        } else if (atom_size == 0u) {
            atom_size = (uint32_t)(size - offset);
        }
        if (atom_size < header || (size_t)atom_size > size - offset) return;
        payload = offset + header;
        if (memcmp(type, "stco", 4u) == 0 && atom_size >= header + 8u) {
            uint32_t count = be32(bytes + payload + 4u);
            size_t entry = payload + 8u;
            uint32_t i;
            if ((size_t)count > ((size_t)atom_size - header - 8u) / 4u) return;
            for (i = 0u; i < count; ++i) {
                uint32_t value = be32(bytes + entry + (size_t)i * 4u);
                if ((size_t)value < data_size && value <= UINT32_MAX - (uint32_t)data_offset)
                    put_be32(bytes + entry + (size_t)i * 4u,
                             value + (uint32_t)data_offset);
            }
        } else if (memcmp(type, "co64", 4u) == 0 && atom_size >= header + 8u) {
            uint32_t count = be32(bytes + payload + 4u);
            size_t entry = payload + 8u;
            uint32_t i;
            if ((size_t)count > ((size_t)atom_size - header - 8u) / 8u) return;
            for (i = 0u; i < count; ++i) {
                uint64_t value = be64(bytes + entry + (size_t)i * 8u);
                if (value < (uint64_t)data_size && value <= UINT64_MAX - data_offset)
                    put_be64(bytes + entry + (size_t)i * 8u, value + data_offset);
            }
        } else if (mac_movie_container(type)) {
            size_t child = payload;
            if (memcmp(type, "meta", 4u) == 0) {
                if (atom_size < header + 4u) return;
                child += 4u;
            }
            mac_movie_rebase_atoms(bytes + child, offset + atom_size - child,
                                   data_offset, data_size);
        }
        offset += atom_size;
    }
}

int dm2_v1_mac_movie_view_build(const uint8_t *data_fork, size_t data_size,
                                const uint8_t *moov, size_t moov_size,
                                DM2_V1_MacMovieView *out) {
    uint8_t *bytes;
    size_t data_offset;
    size_t atom_size;
    size_t rebase_offset;
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!find_data_atom(data_fork, data_size, &data_offset, &atom_size) ||
        !exact_atom(moov, moov_size, "moov") ||
        moov_size > SIZE_MAX - atom_size || data_offset > moov_size) return -1;
    bytes = (uint8_t *)malloc(moov_size + atom_size);
    if (!bytes) return -1;
    memcpy(bytes, moov, moov_size);
    memcpy(bytes + moov_size, data_fork + data_offset, atom_size);
    rebase_offset = moov_size - data_offset;
    mac_movie_rebase_atoms(bytes, moov_size, rebase_offset, data_size);
    out->bytes = bytes;
    out->size = moov_size + atom_size;
    out->moov_offset = 0u;
    out->mdat_offset = moov_size;
    return 0;
}

void dm2_v1_mac_movie_view_free(DM2_V1_MacMovieView *view) {
    if (!view) return;
    free(view->bytes);
    memset(view, 0, sizeof(*view));
}
