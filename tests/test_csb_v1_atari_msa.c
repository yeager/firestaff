#include "csb_v1_atari_msa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void put_be16(unsigned char *p, unsigned int value) {
    p[0] = (unsigned char)(value >> 8); p[1] = (unsigned char)value;
}

static void put_be32(unsigned char *p, unsigned long value) {
    p[0] = (unsigned char)(value >> 24); p[1] = (unsigned char)(value >> 16);
    p[2] = (unsigned char)(value >> 8); p[3] = (unsigned char)value;
}

int main(void) {
    enum { track_bytes = 9 * 512, image_bytes = 10 + 2 + track_bytes };
    unsigned char image[image_bytes], out[2];
    size_t out_size = 0u;
    CSB_V1_AtariMsaReceipt receipt;
    unsigned char *disk = image + 12;
    const char *real_path = getenv("FIRESTAFF_CSB_ATARI_MSA");

    memset(image, 0, sizeof(image));
    put_be16(image, 0x0e0f); put_be16(image + 2, 9); put_be16(image + 4, 0);
    put_be16(image + 6, 0); put_be16(image + 8, 0); put_be16(image + 10, track_bytes);
    put_be16(disk + 11, 512); disk[13] = 1; put_be16(disk + 14, 1);
    disk[16] = 1; put_be16(disk + 17, 1); put_be16(disk + 22, 1);
    disk[512] = 0xf8; disk[513] = 0xff; disk[514] = 0xff;
    disk[515] = 0xff; disk[516] = 0x0f;
    memcpy(disk + 1024, "SAVE    DAT", 11); put_be16(disk + 1024 + 26, 2);
    put_be32(disk + 1024 + 28, 2); disk[1536] = 'O'; disk[1537] = 'K';
    if (!csb_v1_atari_msa_extract_root_file(image, sizeof(image), "save.dat",
                                             out, sizeof(out), &out_size, &receipt) ||
        out_size != 2u || memcmp(out, "OK", 2u) != 0 || receipt.side_count != 1u ||
        receipt.sectors_per_track != 9u || receipt.root_file_count != 1u) {
        return 1;
    }
    image[10] = 0; image[11] = 4; image[12] = 0xe5; image[13] = 0;
    image[14] = 0xff; image[15] = 0xff;
    if (csb_v1_atari_msa_extract_root_file(image, sizeof(image), "SAVE.DAT",
                                            out, sizeof(out), &out_size, NULL)) return 1;
    if (real_path && real_path[0]) {
        FILE *fp = fopen(real_path, "rb");
        long file_size;
        unsigned char *real;
        if (!fp || fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0 ||
            fseek(fp, 0, SEEK_SET) != 0 || !(real = malloc((size_t)file_size)) ||
            fread(real, 1u, (size_t)file_size, fp) != (size_t)file_size) {
            if (fp) fclose(fp);
            return 1;
        }
        fclose(fp);
        if (!csb_v1_atari_msa_probe(real, (size_t)file_size, &receipt) ||
            receipt.sectors_per_track != 9u || receipt.side_count != 2u ||
            receipt.first_track != 0u || receipt.last_track != 79u ||
            receipt.decoded_disk_bytes != 737280u) {
            fprintf(stderr, "real MSA rejected: sectors=%u sides=%u first=%u last=%u bytes=%u\n",
                    receipt.sectors_per_track, receipt.side_count, receipt.first_track,
                    receipt.last_track, receipt.decoded_disk_bytes);
            free(real); return 1;
        }
        free(real);
        puts("PASS: original Atari CSB save disk MSA decodes to 720 KiB");
    }
    puts("test_csb_v1_atari_msa: PASS");
    return 0;
}
