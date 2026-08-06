#include "dm1_v1_fmtowns_iso9660.h"
#include "dm1_v1_legacy_graphics_dat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DM1_LEGACY_GRAPHICS_COUNT 575u
#define DM1_LEGACY_PIXEL_CAPACITY (1024u * 1024u)

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *data;

    if (!path || !out_size) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return data;
}

static uint64_t fnv1a(uint64_t hash, const uint8_t *data, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int audit_graphics(const char *label, const uint8_t *data, size_t size,
                          int big_endian, uint64_t *out_digest)
{
    uint8_t *pixels;
    uint64_t digest = UINT64_C(1469598103934665603);
    unsigned int decoded = 0u;
    unsigned int index;

    if (!dm1_v1_legacy_graphics_probe(data, size, big_endian)) {
        fprintf(stderr, "%s GRAPHICS.DAT failed the real IMAGE2 table probe\n",
                label);
        return 0;
    }
    pixels = (uint8_t *)malloc(DM1_LEGACY_PIXEL_CAPACITY);
    if (!pixels) return 0;
    for (index = 0u; index < DM1_LEGACY_GRAPHICS_COUNT; ++index) {
        uint16_t width = 0u;
        uint16_t height = 0u;
        size_t pixel_bytes;
        if (!dm1_v1_legacy_graphics_is_bitmap_index((uint16_t)index)) {
            if (dm1_v1_legacy_graphics_query(data, size, big_endian,
                                             (uint16_t)index, &width, &height) ||
                dm1_v1_legacy_graphics_decode(
                    data, size, big_endian, (uint16_t)index, pixels,
                    DM1_LEGACY_PIXEL_CAPACITY, &width, &height)) {
                fprintf(stderr, "%s non-bitmap record %u entered IMAGE2 decoder\n",
                        label, index);
                free(pixels);
                return 0;
            }
            continue;
        }
        if (!dm1_v1_legacy_graphics_query(data, size, big_endian,
                                          (uint16_t)index, &width, &height) ||
            (size_t)width * (size_t)height > DM1_LEGACY_PIXEL_CAPACITY ||
            !dm1_v1_legacy_graphics_decode(
                data, size, big_endian, (uint16_t)index, pixels,
                DM1_LEGACY_PIXEL_CAPACITY, &width, &height)) {
            fprintf(stderr, "%s GRAPHICS.DAT record %u failed IMAGE2 decode (%ux%u)\n",
                    label, index, (unsigned)width, (unsigned)height);
            free(pixels);
            return 0;
        }
        pixel_bytes = (size_t)width * (size_t)height;
        digest = fnv1a(digest, pixels, pixel_bytes);
        digest = fnv1a(digest, (const uint8_t *)&width, sizeof(width));
        digest = fnv1a(digest, (const uint8_t *)&height, sizeof(height));
        ++decoded;
    }
    free(pixels);
    if (out_digest) *out_digest = digest;
    printf("ok: %s IMAGE2 decoded %u records, digest=%016llx\n", label,
           decoded, (unsigned long long)digest);
    return decoded == DM1_LEGACY_GRAPHICS_COUNT;
}

static int audit_fmtowns(const char *path)
{
    DM1_V1_FmtownsIsoLayout layout;
    uint8_t *track;
    size_t track_size;
    unsigned int i;
    unsigned int graphics_found = 0u;
    int ok = 1;

    track = read_file(path, &track_size);
    if (!track) {
        fprintf(stderr, "could not read FM Towns track: %s\n", path);
        return 0;
    }
    if (dm1_v1_fmtowns_iso_parse(track, track_size, &layout) != 0) {
        fprintf(stderr, "FM Towns track is not the authenticated DM1 ISO: %s\n",
                path);
        free(track);
        return 0;
    }
    for (i = 0u; i < (unsigned int)layout.file_count; ++i) {
        const DM1_V1_FmtownsIsoEntry *entry = &layout.files[i];
        uint8_t *graphics;
        uint64_t digest = 0u;
        if (strstr(entry->name, "GRAPHICS.DAT") == NULL) continue;
        graphics = (uint8_t *)malloc(entry->size);
        if (!graphics || dm1_v1_fmtowns_iso_extract(
                track, track_size, entry, graphics, entry->size) != 0) {
            free(graphics);
            ok = 0;
            break;
        }
        if (!audit_graphics(entry->name, graphics, entry->size, 0,
                            &digest)) ok = 0;
        free(graphics);
        ++graphics_found;
        if (!ok) break;
    }
    free(track);
    if (graphics_found == 0u) {
        fprintf(stderr, "FM Towns ISO contains no DATA/JDATA GRAPHICS.DAT\n");
        return 0;
    }
    return ok;
}

int main(void)
{
    const char *fmtowns = getenv("FIRESTAFF_DM1_FMTOWNS_TRACK01");
    const char *amiga = getenv("FIRESTAFF_DM1_AMIGA_GRAPHICS_DAT");
    uint8_t *amiga_data;
    size_t amiga_size;
    int ran = 0;
    int ok = 1;

    if (fmtowns && fmtowns[0]) {
        ran = 1;
        ok = audit_fmtowns(fmtowns) && ok;
    }
    if (amiga && amiga[0]) {
        ran = 1;
        amiga_data = read_file(amiga, &amiga_size);
        if (!amiga_data) {
            fprintf(stderr, "could not read Amiga GRAPHICS.DAT: %s\n", amiga);
            ok = 0;
        } else {
            ok = audit_graphics("Amiga", amiga_data, amiga_size, 1, NULL) && ok;
            free(amiga_data);
        }
    }
    if (!ran) {
        puts("SKIP: set FIRESTAFF_DM1_FMTOWNS_TRACK01 and/or FIRESTAFF_DM1_AMIGA_GRAPHICS_DAT");
        return 0;
    }
    return ok ? 0 : 1;
}
