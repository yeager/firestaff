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
        size_t graphics_capacity = 1024u * 1024u;
        size_t dungeon_capacity = 128u * 1024u;
        unsigned char *graphics_bytes = (unsigned char *)malloc(graphics_capacity);
        unsigned char *dungeon_bytes = (unsigned char *)malloc(dungeon_capacity);
        unsigned char *pixels = (unsigned char *)malloc(320u * 200u);
        uint16_t width = 0u;
        uint16_t height = 0u;
        int decoded_records = 0;
        uint16_t i;
        assert(firestaff_zip_extract_by_suffix(archive, ".stx", &image, &size) == 0 &&
               image && graphics_bytes && dungeon_bytes && pixels);
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
        assert(dm1_v1_atari_st_graphics_open(
            graphics_bytes, graphics_size, &graphics));
        for (i = 0u; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
            if (dm1_v1_atari_st_graphics_decode(
                    &graphics, i, pixels, 320u * 200u, &width, &height)) {
                ++decoded_records;
            }
        }
        printf("DM1 Atari STX decoded graphics records: %d\n", decoded_records);
        assert(decoded_records == 283);
        free(pixels);
        free(dungeon_bytes);
        free(graphics_bytes);
        free(image);
    }
    puts("PASS dm1_v1_atari_st_stx");
    return 0;
}
