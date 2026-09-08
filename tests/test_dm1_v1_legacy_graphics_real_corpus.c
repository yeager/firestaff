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
#define DM1_DUNGEON_HEADER_BYTES 44u
#define DM1_DUNGEON_MAP_DESCRIPTOR_BYTES 16u

/* MEDIA020 (F20E/F20J) DUNGEON.DAT is Motorola-order.  These are not
 * guessed layout constants: ReDMCSB DEFS.H M644/M645/M646/M647 gives the
 * floor, stair and wall families which DUNVIEW.C F0094/F0095 materialises
 * from each map descriptor's FloorSet and WallSet. */
#define F20_FIRST_FLOOR_SET 75u
#define F20_FIRST_WALL_SET 77u
#define F20_WALL_SET_GRAPHIC_COUNT 13u
#define F20_FIRST_STAIRS 90u
#define F20_STAIRS_GRAPHIC_COUNT 18u

/* Authentic F20E/F20J IMAGE2 geometry for M646_GRAPHIC_FIRST_WALL_SET.
 * C086..C088 are the source-owned LCR compounds consumed through F0635_
 * clipping in ReDMCSB, rather than three separately-scaled PC34 sprites. */
static const uint16_t k_f20_wall_widths[F20_WALL_SET_GRAPHIC_COUNT] = {
    32u, 25u, 21u, 13u, 14u, 102u, 70u, 32u, 32u, 248u, 136u, 117u, 16u
};
static const uint16_t k_f20_wall_heights[F20_WALL_SET_GRAPHIC_COUNT] = {
    123u, 94u, 65u, 44u, 43u, 4u, 3u, 136u, 136u, 111u, 71u, 51u, 49u
};

static uint64_t fnv1a(uint64_t hash, const uint8_t *data, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint16_t read_u16_be(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8u) | p[1]);
}

/* Audit the actual map set selectors in the two retail FM Towns dungeons.
 * It catches a particularly easy regression: a renderer can look correct at
 * the entrance (wall set zero) while addressing a PC34 40-record block, or
 * the wrong F20 stair stride, on later maps.  The game disc remains in RAM;
 * this only consumes ISO members already selected from the authenticated
 * archive. */
static int audit_fmtowns_dungeon_sets(const char *label, const uint8_t *data,
                                      size_t size)
{
    unsigned int map_count;
    unsigned int map_index;
    unsigned int distinct_wall_sets = 0u;
    unsigned int distinct_floor_sets = 0u;
    unsigned int max_wall_set = 0u;
    unsigned int max_floor_set = 0u;

    if (!data || size < DM1_DUNGEON_HEADER_BYTES) {
        fprintf(stderr, "%s DUNGEON.DAT has no complete header\n", label);
        return 0;
    }
    map_count = data[4];
    if (map_count == 0u || map_count > 32u ||
        map_count > (size - DM1_DUNGEON_HEADER_BYTES) /
                        DM1_DUNGEON_MAP_DESCRIPTOR_BYTES) {
        fprintf(stderr, "%s DUNGEON.DAT map table is invalid\n", label);
        return 0;
    }
    for (map_index = 0u; map_index < map_count; ++map_index) {
        const size_t at = DM1_DUNGEON_HEADER_BYTES +
            (size_t)map_index * DM1_DUNGEON_MAP_DESCRIPTOR_BYTES;
        const uint16_t graphic_sets = read_u16_be(data + at + 14u);
        const unsigned int floor_set = (unsigned int)(graphic_sets & 0x0fu);
        const unsigned int wall_set = (unsigned int)((graphic_sets >> 4u) & 0x0fu);
        const unsigned int floor_last = F20_FIRST_FLOOR_SET + floor_set * 2u + 1u;
        const unsigned int wall_last = F20_FIRST_WALL_SET +
            wall_set * F20_WALL_SET_GRAPHIC_COUNT +
            (F20_WALL_SET_GRAPHIC_COUNT - 1u);
        const unsigned int stairs_last = F20_FIRST_STAIRS +
            wall_set * F20_STAIRS_GRAPHIC_COUNT +
            (F20_STAIRS_GRAPHIC_COUNT - 1u);

        if (floor_last >= DM1_LEGACY_GRAPHICS_COUNT ||
            wall_last >= DM1_LEGACY_GRAPHICS_COUNT ||
            stairs_last >= DM1_LEGACY_GRAPHICS_COUNT) {
            fprintf(stderr,
                    "%s map %u selects F20 floor=%u wall=%u outside GRAPHICS.DAT"
                    " (floor=%u wall=%u stairs=%u)\n",
                    label, map_index, floor_set, wall_set, floor_last,
                    wall_last, stairs_last);
            return 0;
        }
        distinct_floor_sets |= 1u << floor_set;
        distinct_wall_sets |= 1u << wall_set;
        if (floor_set > max_floor_set) max_floor_set = floor_set;
        if (wall_set > max_wall_set) max_wall_set = wall_set;
    }
    printf("ok: %s %u maps; F20 floors=0x%04x (max=%u), walls=0x%04x (max=%u)\n",
           label, map_count, distinct_floor_sets, max_floor_set,
           distinct_wall_sets, max_wall_set);
    return 1;
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
        /* ReDMCSB MEDIA020 assigns F20 wall-set members 77..89 to the
         * native F0128 viewport zones.  Keep their actual retail dimensions
         * in this corpus receipt: a future renderer must not silently treat
         * a compound LCR source as three independent PC34 tiles. */
        if (!big_endian && index >= F20_FIRST_WALL_SET &&
            index < F20_FIRST_WALL_SET + F20_WALL_SET_GRAPHIC_COUNT) {
            const unsigned int wall_offset = index - F20_FIRST_WALL_SET;
            if (width != k_f20_wall_widths[wall_offset] ||
                height != k_f20_wall_heights[wall_offset]) {
                fprintf(stderr,
                        "%s F20 wall graphic %u geometry changed: got %ux%u, expected %ux%u\n",
                        label, index, (unsigned)width, (unsigned)height,
                        (unsigned)k_f20_wall_widths[wall_offset],
                        (unsigned)k_f20_wall_heights[wall_offset]);
                free(pixels);
                return 0;
            }
            printf("receipt: %s F20 wall graphic %u is %ux%u\n", label,
                   index, (unsigned)width, (unsigned)height);
        }
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
    unsigned int dungeon_found = 0u;
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
        uint8_t *member;
        uint64_t digest = 0u;
        if (strstr(entry->name, "GRAPHICS.DAT") == NULL &&
            strstr(entry->name, "DUNGEON.DAT") == NULL) continue;
        member = (uint8_t *)malloc(entry->size);
        if (!member || dm1_v1_fmtowns_iso_extract(
                track, track_size, entry, member, entry->size) != 0) {
            free(member);
            ok = 0;
            break;
        }
        if (strstr(entry->name, "GRAPHICS.DAT") != NULL) {
            if (!audit_graphics(entry->name, member, entry->size, 0,
                                &digest)) ok = 0;
            ++graphics_found;
        } else {
            if (!audit_fmtowns_dungeon_sets(entry->name, member,
                                             entry->size)) ok = 0;
            ++dungeon_found;
        }
        free(member);
        if (!ok) break;
    }
    free(track);
    if (graphics_found != 2u || dungeon_found != 2u) {
        fprintf(stderr,
                "FM Towns ISO is missing a DATA/JDATA GRAPHICS.DAT or DUNGEON.DAT pair"
                " (graphics=%u dungeons=%u)\n",
                graphics_found, dungeon_found);
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
