#include "dm2_v1_mac_media.h"
#include "firestaff_zip_extract.h"

#include <stdlib.h>
#include <string.h>

/* Macintosh CDs in the verified DM2 set are raw MODE1/2352 images.  HFS is
 * addressed in 512-byte logical blocks inside the 2048-byte user area. */
typedef struct { const uint8_t *image; size_t size; size_t part; } MacDisk;
typedef struct { uint32_t start, count; } Extent;

static uint16_t be16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static const uint8_t *block(const MacDisk *d, size_t n) {
    size_t sector = n / 4u, sub = n % 4u;
    size_t off = sector * 2352u + 16u + sub * 512u;
    if (off > d->size || d->size - off < 512u) return NULL;
    return d->image + off;
}

static int locate_hfs(const uint8_t *image, size_t size, MacDisk *d,
                      uint32_t *alloc_size, uint16_t *alloc_start,
                      uint32_t *cat_size, uint16_t *cat_start) {
    size_t logical_blocks = size / 2352u * 4u;
    size_t candidate;
    d->image = image; d->size = size;
    for (candidate = 0; candidate < logical_blocks && candidate < 128u; ++candidate) {
        const uint8_t *mdb = block(&(MacDisk){image, size, candidate}, candidate + 2u);
        if (!mdb || be16(mdb) != 0x4244u) continue;
        *alloc_size = be16(mdb + 22);
        *alloc_start = be16(mdb + 28);
        *cat_size = be32(mdb + 130);
        *cat_start = be16(mdb + 150);
        if (*alloc_size < 512u || (*alloc_size % 512u) != 0u ||
            *cat_size == 0u || *cat_start == 0u) continue;
        d->part = candidate;
        return 0;
    }
    return -1;
}

static int read_catalog(const MacDisk *d, uint32_t alloc_size,
                        uint16_t alloc_start, uint16_t cat_start,
                        uint32_t cat_size, uint8_t **out) {
    size_t blocks = (cat_size + 511u) / 512u;
    size_t base = d->part + alloc_start + (size_t)cat_start * (alloc_size / 512u);
    uint8_t *catalog = (uint8_t *)malloc(blocks * 512u);
    size_t i;
    if (!catalog) return -1;
    for (i = 0; i < blocks; ++i) {
        const uint8_t *p = block(d, base + i);
        if (!p) { free(catalog); return -1; }
        memcpy(catalog + i * 512u, p, 512u);
    }
    *out = catalog;
    return 0;
}

static int find_file(const uint8_t *cat, size_t cat_size, const char *wanted,
                     Extent *extents, uint32_t *data_size) {
    size_t node;
    size_t wanted_len = strlen(wanted);
    for (node = 0; node + 512u <= cat_size; node += 512u) {
        const uint8_t *n = cat + node;
        uint16_t records, r;
        if (n[8] != 0xffu) continue;
        records = be16(n + 10);
        if (records == 0 || records > 64) continue;
        for (r = 0; r < records; ++r) {
            size_t oa = be16(n + 512u - 2u * (r + 1u));
            size_t ob = be16(n + 512u - 2u * (r + 2u));
            const uint8_t *rec;
            size_t key_len, name_len, data;
            if (oa < 14u || ob <= oa || ob > 512u) continue;
            rec = n + oa;
            key_len = rec[0];
            if (key_len < 7u || 1u + key_len > ob - oa || rec[6] > 31u) continue;
            name_len = rec[6];
            if (name_len != wanted_len || memcmp(rec + 7, wanted, name_len) != 0) continue;
            data = 1u + key_len + ((key_len & 1u) == 0u ? 1u : 0u);
            if (data + 74u + 8u > ob - oa || rec[data] != 2u) continue;
            *data_size = be32(rec + data + 26u);
            memset(extents, 0, sizeof(Extent) * 3u);
            for (size_t e = 0; e < 3u; ++e) {
                extents[e].start = be16(rec + data + 74u + e * 8u);
                extents[e].count = be16(rec + data + 76u + e * 8u);
            }
            return 0;
        }
    }
    return -1;
}

static int copy_fork(const MacDisk *d, uint32_t alloc_size,
                     uint16_t alloc_start,
                     const Extent extents[3], uint32_t file_size,
                     uint8_t **out, size_t *out_size) {
    uint8_t *bytes;
    size_t copied = 0, e;
    if (file_size == 0 || file_size > 64u * 1024u * 1024u) return -1;
    bytes = (uint8_t *)malloc(file_size);
    if (!bytes) return -1;
    for (e = 0; e < 3u && copied < file_size; ++e) {
        uint32_t b;
        for (b = 0; b < extents[e].count && copied < file_size; ++b) {
            size_t logical = d->part + (size_t)alloc_start +
                             (size_t)(extents[e].start + b) * (alloc_size / 512u);
            size_t page;
            for (page = 0; page < alloc_size / 512u && copied < file_size; ++page) {
                const uint8_t *p = block(d, logical + page);
                size_t take = file_size - copied;
                if (!p) { free(bytes); return -1; }
                if (take > 512u) take = 512u;
                memcpy(bytes + copied, p, take);
                copied += take;
            }
        }
    }
    if (copied != file_size) { free(bytes); return -1; }
    *out = bytes; *out_size = file_size;
    return 0;
}

static int copy_dmfiles_entry(const uint8_t *dmfiles, size_t dmfiles_size,
                              const char *wanted, size_t expected_size,
                              uint8_t **out, size_t *out_size) {
    size_t off = 0;
    size_t wanted_len = strlen(wanted);
    if (!dmfiles || !wanted || !out || !out_size) return -1;
    while (off + 112u <= dmfiles_size) {
        const uint8_t *h = dmfiles + off;
        size_t name_len = h[2];
        uint32_t data_size = be32(h + 88u);
        if (name_len == 0u || name_len > 31u ||
            off + 112u > dmfiles_size ||
            (size_t)name_len > dmfiles_size - off - 3u) return -1;
        if (name_len == wanted_len && memcmp(h + 3u, wanted, name_len) == 0) {
            if (data_size != expected_size ||
                off + 112u + data_size > dmfiles_size) return -1;
            *out = (uint8_t *)malloc(data_size);
            if (!*out) return -1;
            memcpy(*out, dmfiles + off + 112u, data_size);
            *out_size = data_size;
            return 0;
        }
        if (data_size == 0u || off + 112u + data_size > dmfiles_size) return -1;
        off += 112u + data_size;
    }
    return -1;
}

static int read_demo_from_installer(const uint8_t *installer, size_t installer_size,
                                    DM2_V1_MacMedia *out) {
    const uint8_t *stuffit, *h, *dmfiles;
    size_t stuffit_size, pos, resource_compressed, data_compressed;
    uint8_t *demo_data = NULL;
    size_t demo_data_size;
    if (!installer || installer_size < 128u + 22u + 112u || !out) return -1;
    /* HFS contains a MacBinary file.  Its data fork starts at byte 128 and
     * is the authentic STi2 stream; the resource fork is not needed for the
     * demo's game-data handoff. */
    if (installer_size >= 4u && memcmp(installer, "STi2", 4u) == 0) {
        stuffit = installer;
        stuffit_size = installer_size;
    } else {
        stuffit = installer + 128u;
        stuffit_size = installer_size - 128u;
    }
    if (memcmp(stuffit, "STi2", 4u) != 0) return -1;
    pos = 22u;
    while (pos + 112u <= stuffit_size) {
        h = stuffit + pos;
        if (h[2] == 0u || h[2] > 31u || pos + 112u > stuffit_size) return -1;
        resource_compressed = be32(h + 92u);
        data_compressed = be32(h + 96u);
        if (resource_compressed > stuffit_size - pos - 112u ||
            data_compressed > stuffit_size - pos - 112u - resource_compressed)
            return -1;
        if (h[2] == 7u && memcmp(h + 3u, "DMFiles", 7u) == 0) {
            dmfiles = stuffit + pos + 112u + resource_compressed;
            demo_data_size = data_compressed;
            if (copy_dmfiles_entry(dmfiles, data_compressed, "Graphics.dat",
                                   3110116u, &out->graphics,
                                   &out->graphics_size) != 0) return -1;
            if (copy_dmfiles_entry(dmfiles, data_compressed, "Dungeon.dat",
                                   6535u, &demo_data, &demo_data_size) != 0) return -1;
            /* The 6,535-byte demo member is the authentic Dungeon.dat.  Its
             * leading 00 06 05 37 is the BE File_header (seed=6,
             * mapDataSize=1335), not a DCL stream.  TTComp's byte signature
             * is coincidental here; do not expand it. */
            out->dungeon = demo_data;
            out->dungeon_size = demo_data_size;
            out->demo = 1;
            return 0;
        }
        pos += 112u + resource_compressed + data_compressed;
        if (pos >= stuffit_size) break;
    }
    return -1;
}

int dm2_v1_mac_media_read_zip(const char *zip_path, DM2_V1_MacMedia *out) {
    uint8_t *image = NULL, *cat = NULL;
    size_t image_size = 0, cat_size;
    MacDisk disk;
    uint32_t alloc_size, catalog_size, file_size;
    uint16_t alloc_start, cat_start;
    Extent extents[3];
    int rc = -1;
    if (!zip_path || !out) return -1;
    memset(out, 0, sizeof(*out));
    if (firestaff_zip_extract_by_suffix(zip_path, ".bin", &image, &image_size) != 0) return -1;
    if (locate_hfs(image, image_size, &disk, &alloc_size, &alloc_start,
                   &catalog_size, &cat_start) != 0) goto done;
    cat_size = catalog_size;
    if (read_catalog(&disk, alloc_size, alloc_start, cat_start, cat_size, &cat) != 0) goto done;
    if (find_file(cat, cat_size, "Graphics.dat", extents, &file_size) == 0 &&
        copy_fork(&disk, alloc_size, alloc_start, extents, file_size,
                  &out->graphics, &out->graphics_size) == 0 &&
        find_file(cat, cat_size, "Dungeon.dat", extents, &file_size) == 0 &&
        copy_fork(&disk, alloc_size, alloc_start, extents, file_size,
                  &out->dungeon, &out->dungeon_size) == 0) {
        /* Full retail Mac media exposes the game files directly. */
    } else {
        uint8_t *installer = NULL;
        size_t installer_size = 0;
        dm2_v1_mac_media_free(out);
        if (find_file(cat, cat_size, "Install Dungeon MasterII Demo", extents,
                      &file_size) != 0 ||
            copy_fork(&disk, alloc_size, alloc_start, extents, file_size,
                      &installer, &installer_size) != 0 ||
            read_demo_from_installer(installer, installer_size, out) != 0) {
            free(installer);
            goto done;
        }
        free(installer);
    }
    if (find_file(cat, cat_size, "md.dat", extents, &file_size) == 0)
        (void)copy_fork(&disk, alloc_size, alloc_start, extents, file_size, &out->music_map, &out->music_map_size);
    rc = 0;
done:
    free(cat); free(image);
    if (rc != 0) dm2_v1_mac_media_free(out);
    return rc;
}

void dm2_v1_mac_media_free(DM2_V1_MacMedia *media) {
    if (!media) return;
    free(media->graphics); free(media->dungeon); free(media->music_map);
    memset(media, 0, sizeof(*media));
}
