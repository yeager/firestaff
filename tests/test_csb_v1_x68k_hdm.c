#include "csb_v1_x68k_hdm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void put_le16(unsigned char *p, unsigned int value) {
    p[0] = (unsigned char)value; p[1] = (unsigned char)(value >> 8);
}

static void put_le32(unsigned char *p, unsigned long value) {
    p[0] = (unsigned char)value; p[1] = (unsigned char)(value >> 8);
    p[2] = (unsigned char)(value >> 16); p[3] = (unsigned char)(value >> 24);
}

static int read_file(const char *path, unsigned char **out, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    long size;
    if (!fp || fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) <= 0 ||
        fseek(fp, 0, SEEK_SET) != 0 || !(*out = malloc((size_t)size)) ||
        fread(*out, 1u, (size_t)size, fp) != (size_t)size) {
        if (fp) fclose(fp);
        free(*out); return 0;
    }
    fclose(fp); *out_size = (size_t)size; return 1;
}

int main(int argc, char **argv) {
    enum { bytes = CSB_V1_X68K_HDM_BYTES_PER_DISK, root = 5 * 1024, data = 11 * 1024 };
    unsigned char *image = calloc(1u, bytes), out[3] = {0};
    size_t size = 0u;
    CSB_V1_X68kHdmReceipt receipt;
    CSB_V1_X68kHdmRootEntry entry;
    if (!image) return 1;
    image[0] = 0x60; image[1] = 0x1c;
    memcpy(image + 2, "Hudson soft 2.00", 16u);
    image[1024] = 0xfe; image[1025] = 0xff; image[1026] = 0xff;
    memcpy(image + root, "SAMPLE  DAT", 11u);
    put_le16(image + root + 26, 2u); put_le32(image + root + 28, 3u);
    image[data] = 'O'; image[data + 1] = 'K'; image[data + 2] = '!';
    if (!csb_v1_x68k_hdm_extract_root_file(image, bytes, "sample.dat", out,
                                            sizeof(out), &size, &receipt) ||
        size != 3u || memcmp(out, "OK!", 3u) != 0 || receipt.root_file_count != 1u ||
        receipt.data_offset != data ||
        !csb_v1_x68k_hdm_root_entry(image, bytes, 0u, &entry) ||
        strcmp(entry.name, "SAMPLE.DAT") != 0 || entry.byte_count != 3u ||
        csb_v1_x68k_hdm_root_entry(image, bytes, 1u, &entry) ||
        csb_v1_x68k_hdm_extract_root_file(image, bytes, "missing.dat", out,
                                           sizeof(out), &size, NULL)) {
        free(image); return 1;
    }
    put_le32(image + root + 28, 1025u);
    image[1027] = 2u;
    if (csb_v1_x68k_hdm_extract_root_file(image, bytes, "sample.dat", NULL,
                                           0u, &size, NULL)) {
        free(image); return 1;
    }
    image[1024] = 0;
    if (csb_v1_x68k_hdm_probe(image, bytes, NULL)) { free(image); return 1; }
    free(image);

    if (argc == 2) {
        unsigned char *real = NULL, *graphics = NULL, *dungeon = NULL;
        size_t real_size = 0u, graphics_size = 0u, dungeon_size = 0u;
        if (!read_file(argv[1], &real, &real_size) ||
            !csb_v1_x68k_hdm_probe(real, real_size, &receipt) ||
            receipt.root_file_count < 20u ||
            !csb_v1_x68k_hdm_root_entry(real, real_size, 4u, &entry) ||
            strcmp(entry.name, "CHAOS_ST.X") != 0 || entry.byte_count != 12284u ||
            !csb_v1_x68k_hdm_extract_root_file(real, real_size, "GRAPHICS.DAT", NULL,
                                                0u, &graphics_size, NULL) ||
            graphics_size != 0x5b34fu || !(graphics = malloc(graphics_size)) ||
            !csb_v1_x68k_hdm_extract_root_file(real, real_size, "graphics.dat", graphics,
                                                graphics_size, &size, NULL) ||
            size != graphics_size || graphics[0] != 0x80u ||
            !csb_v1_x68k_hdm_extract_root_file(real, real_size, "DUNGEON.DAT", NULL,
                                                0u, &dungeon_size, NULL) ||
            dungeon_size != 0x812u || !(dungeon = malloc(dungeon_size)) ||
            !csb_v1_x68k_hdm_extract_root_file(real, real_size, "DUNGEON.DAT", dungeon,
                                                dungeon_size, &size, NULL) ||
            size != dungeon_size || dungeon[0] != 0x81u) {
            free(real); free(graphics); free(dungeon); return 1;
        }
        free(real); free(graphics); free(dungeon);
        puts("PASS: original CSB X68000 HDM exposes GRAPHICS.DAT and DUNGEON.DAT");
    }
    puts("test_csb_v1_x68k_hdm: PASS");
    return 0;
}
