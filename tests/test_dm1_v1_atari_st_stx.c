#include "dm1_v1_atari_st_stx.h"
#include "dm1_v1_atari_st_graphics_dat.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, size_t *size)
{
    FILE *fp = fopen(path, "rb");
    unsigned char *data;
    long length;
    if (!fp) return NULL;
    assert(fseek(fp, 0, SEEK_END) == 0);
    length = ftell(fp);
    assert(length > 0 && fseek(fp, 0, SEEK_SET) == 0);
    data = (unsigned char *)malloc((size_t)length);
    assert(data && fread(data, 1u, (size_t)length, fp) == (size_t)length);
    fclose(fp);
    *size = (size_t)length;
    return data;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_DM1_ATARI_STX");
    if (path) {
        DM1_V1_AtariStx stx;
        DM1_V1_AtariStGraphicsDat graphics;
        size_t size = 0u;
        size_t extracted_size = 0u;
        unsigned char *image = read_file(path, &size);
        unsigned char *graphics_bytes = (unsigned char *)malloc(271911u);
        unsigned char *dungeon_bytes = (unsigned char *)malloc(33286u);
        assert(image && graphics_bytes && dungeon_bytes);
        assert(dm1_v1_atari_st_stx_open(image, size, &stx));
        assert(stx.sector_count == 800u);
        assert(dm1_v1_atari_st_stx_extract_file(
            &stx, "GRAPHICS.DAT", graphics_bytes, 271911u, &extracted_size));
        assert(extracted_size == 271911u);
        assert(dm1_v1_atari_st_stx_extract_file(
            &stx, "DUNGEON.DAT", dungeon_bytes, 33286u, &extracted_size));
        assert(extracted_size == 33286u);
        assert(dm1_v1_atari_st_graphics_open(
            graphics_bytes, 271911u, &graphics));
        assert(graphics.records[0].compressed_size != 0u);
        free(dungeon_bytes);
        free(graphics_bytes);
        free(image);
    }
    puts("PASS dm1_v1_atari_st_stx");
    return 0;
}
