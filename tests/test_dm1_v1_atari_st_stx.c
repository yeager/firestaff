#include "dm1_v1_atari_st_stx.h"
#include "dm1_v1_atari_st_graphics_dat.h"
#include "firestaff_zip_extract.h"

/* The original STX path is a real-media regression.  Its source reads and
 * GEMDOS/graphics assertions must survive CI's Release configuration. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void write_optional_dungeon(const unsigned char *bytes, size_t size)
{
    const char *path = getenv("FIRESTAFF_DM1_ATARI_DUNGEON_OUTPUT");
    FILE *output;
    if (!path || !path[0]) return;
    output = fopen(path, "wb");
    assert(output != NULL);
    assert(fwrite(bytes, 1u, size, output) == size);
    assert(fclose(output) == 0);
    printf("DM1 Atari STX original DUNGEON.DAT extracted to %s (%zu bytes)\n",
           path, size);
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_ATARI_ARCHIVE");
    if (archive && archive[0]) {
        DM1_V1_AtariStx stx;
        DM1_V1_AtariStGraphicsDat graphics;
        size_t size = 0u;
        size_t graphics_size = 0u;
        size_t extracted_size = 0u;
        unsigned char *image = NULL;
        unsigned char *nested_zip = NULL;
        size_t nested_zip_size = 0u;
        size_t graphics_capacity = 1024u * 1024u;
        size_t dungeon_capacity = 128u * 1024u;
        unsigned char *graphics_bytes = (unsigned char *)malloc(graphics_capacity);
        unsigned char *dungeon_bytes = (unsigned char *)malloc(dungeon_capacity);
        unsigned char *pixels = (unsigned char *)malloc(320u * 200u);
        uint16_t width = 0u;
        uint16_t height = 0u;
        int decoded_records = 0;
        uint16_t i;
        /* Original Atari collections may wrap the disk image in an outer
         * ZIP which itself contains the ZIP holding the STX.  Prefer a
         * direct STX member for older collections, then inspect the nested
         * original ZIP in memory without writing or modifying game media. */
        if (firestaff_zip_extract_by_suffix(archive, ".stx", &image, &size) != 0) {
            assert(firestaff_zip_extract_by_suffix(
                       archive, ".zip", &nested_zip, &nested_zip_size) == 0 &&
                   nested_zip && nested_zip_size > 0u);
            assert(firestaff_zip_extract_memory_by_suffix(
                       nested_zip, nested_zip_size, ".stx", &image, &size) == 0 &&
                   image && size > 0u);
            free(nested_zip);
        }
        assert(image && size > 0u && graphics_bytes && dungeon_bytes && pixels);
        assert(dm1_v1_atari_st_stx_open(image, size, &stx));
        assert(stx.sector_count == 800u);
        assert(dm1_v1_atari_st_stx_extract_file(
            &stx, "GRAPHICS.DAT", graphics_bytes, graphics_capacity,
            &extracted_size));
        assert(extracted_size > 0u);
        graphics_size = extracted_size;
        assert(dm1_v1_atari_st_stx_extract_file(
            &stx, "DUNGEON.DAT", dungeon_bytes, dungeon_capacity,
            &extracted_size));
        assert(extracted_size > 0u);
        write_optional_dungeon(dungeon_bytes, extracted_size);
        assert(dm1_v1_atari_st_graphics_open(
            graphics_bytes, graphics_size, &graphics));
        for (i = 0u; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
            if (dm1_v1_atari_st_graphics_decode(
                    &graphics, i, pixels, 320u * 200u, &width, &height)) {
                ++decoded_records;
            }
        }
        printf("DM1 Atari STX decoded graphics records: %d\n", decoded_records);
        /* The supplied retail v1.2 STX contains 532 decodable records.
         * Keep this tied to the actual archive rather than the obsolete
         * 283-record fixture count from the earlier partial corpus. */
        assert(decoded_records == 532);
        free(pixels);
        free(dungeon_bytes);
        free(graphics_bytes);
        free(image);
    }
    puts("PASS dm1_v1_atari_st_stx");
    return 0;
}
