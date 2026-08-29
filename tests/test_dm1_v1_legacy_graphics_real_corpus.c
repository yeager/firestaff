#include "dm1_v1_fmtowns_iso9660.h"
#include "dm1_v1_legacy_graphics_dat.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DM1_LEGACY_GRAPHICS_COUNT 575u
/* `dm1_v1_legacy_graphics_is_bitmap_index()` admits 0..20 and 22..532.
 * The remaining table records are source code, sound, text or font material
 * and must be rejected by the IMAGE2 bitmap decoder. */
#define DM1_LEGACY_BITMAP_COUNT 532u
#define DM1_LEGACY_PIXEL_CAPACITY (1024u * 1024u)

static uint64_t fnv1a(uint64_t hash, const uint8_t *data, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

typedef struct {
    uint8_t *bytes;
    size_t size;
} AmigaGraphicsFile;

static int amiga_graphics_visitor(const char *name, const uint8_t *bytes,
                                  size_t size, void *user_data)
{
    AmigaGraphicsFile *graphics = (AmigaGraphicsFile *)user_data;
    uint8_t *copy;

    if (!name || !bytes || !graphics || strcmp(name, "graphics.dat") != 0 ||
        graphics->bytes || size == 0u) return 1;
    copy = (uint8_t *)malloc(size);
    if (!copy) return -1;
    memcpy(copy, bytes, size);
    graphics->bytes = copy;
    graphics->size = size;
    return 1;
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
    return decoded == DM1_LEGACY_BITMAP_COUNT;
}

static int audit_fmtowns_archive(const char *archive)
{
    DM1_V1_FmtownsIsoLayout layout;
    uint8_t *track;
    uint8_t *cue = NULL;
    size_t track_size;
    size_t cue_size = 0u;
    char image_member[256];
    unsigned int i;
    unsigned int graphics_found = 0u;
    int ok = 1;

    if (!archive ||
        firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) != 0 ||
        !cue || !fmtowns_cue_parse_image_member((const char *)cue, cue_size,
                                                 image_member,
                                                 sizeof(image_member)) ||
        firestaff_zip_extract_by_suffix(archive, image_member, &track,
                                        &track_size) != 0 || !track) {
        fprintf(stderr, "could not read FM Towns CUE/BIN from archive: %s\n",
                archive ? archive : "(null)");
        free(cue);
        free(track);
        return 0;
    }
    free(cue);
    if (dm1_v1_fmtowns_iso_parse(track, track_size, &layout) != 0) {
        fprintf(stderr, "FM Towns track is not the authenticated DM1 ISO\n");
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

static int audit_amiga_archive(const char *archive)
{
    static const char inner_member[] = "Dungeon Master v2.0 (1988)(FTL).zip";
    static const char adf_member[] = "Dungeon Master v2.0 (1988)(FTL).adf";
    AmigaGraphicsFile graphics;
    uint8_t *inner = NULL;
    uint8_t *adf = NULL;
    size_t inner_size = 0u;
    size_t adf_size = 0u;
    int visited;
    int ok;

    memset(&graphics, 0, sizeof(graphics));
    if (!archive ||
        firestaff_zip_extract_by_suffix(archive, inner_member, &inner,
                                        &inner_size) != 0 || !inner ||
        firestaff_zip_extract_memory_by_suffix(inner, inner_size, adf_member,
                                               &adf, &adf_size) != 0 || !adf) {
        fprintf(stderr, "could not read DM1 Amiga v2.0 ADF from archive: %s\n",
                archive ? archive : "(null)");
        free(inner);
        free(adf);
        return 0;
    }
    free(inner);
    visited = firestaff_amiga_adf_visit_ofs_files(adf, adf_size,
                                                   amiga_graphics_visitor,
                                                   &graphics);
    free(adf);
    ok = visited >= 0 && graphics.bytes &&
        audit_graphics("Amiga v2.0 GRAPHICS.DAT", graphics.bytes,
                       graphics.size, 1, NULL);
    free(graphics.bytes);
    return ok;
}

int main(void)
{
    const char *fmtowns = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    const char *amiga = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    int ran = 0;
    int ok = 1;

    if (fmtowns && fmtowns[0]) {
        ran = 1;
        ok = audit_fmtowns_archive(fmtowns) && ok;
    }
    if (amiga && amiga[0]) {
        ran = 1;
        ok = audit_amiga_archive(amiga) && ok;
    }
    if (!ran) {
        puts("SKIP: set FIRESTAFF_DM1_FMTOWNS_ARCHIVE and/or FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
        return 0;
    }
    return ok ? 0 : 1;
}
