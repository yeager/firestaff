#include "asset_status_m12.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BANK1F_FILE_OFFSET 0x1f0000u
#define CONSUMER_OFFSET 0x243eu

static const uint8_t k_consumer_bytes[] = {
    0xb2,0x2e,0x85,0x0e,0xe6,0x2e,0xd0,0x02,0xe6,0x2f,0x86,0x0f,
    0xa5,0x12,0xd0,0x02,0xc6,0x13,0xc6,0x12,0xd0,0xe0,0xa5,0x13,
    0xd0,0xdc,0x60,0xad,0x7e,0x3b,0x53,0x08,0xad,0x7f,0x3b,0x53,
    0x10,0xad,0x80,0x3b,0x53,0x20,0xad,0x81,0x3b,0x53,0x40,0x60,
    0xad,0x82,0x3b,0x53,0x08,0xad,0x83,0x3b,0x53,0x10,0xad,0x84,
    0x3b,0x53,0x20,0xad,0x85,0x3b,0x53,0x40,0x60,0xa5,0x11,0xf0,
    0x02,0x10,0x0d,0x44,0xe3,0xa5,0x10,0x92,0x30,0xe6,0x30,0xd0,
    0x02,0xe6,0x31,0x60,0x38,0xa5,0x10,0xe9,0x01,0x85,0x10,0xa5,
    0x11,0xe9,0x01,0x85,0x11,0x06,0x10,0x26,0x11,0x18,0xa5,0x10,
    0x65,0x32,0x85,0x10,0xa5,0x11,0x65,0x33,0x85,0x11,0x85,0x37,
    0xa5,0x10,0x85,0x36,0x44,0x9d,0xc2,0xb1,0x36,0x85,0x02,0xc8,
    0xb1,0x36
};

static const char *source_path(char out[512], const char *env_name,
                               const char *file_name) {
    const char *configured = getenv(env_name);
    const char *home;
    if (configured && configured[0]) return configured;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    if (snprintf(out, 512, "%s/.firestaff/data/theron/%s", home, file_name) >= 512)
        return NULL;
    return out;
}

static int verify_variant(const char *env_name, const char *file_name,
                          const char *expected_md5,
                          long expected_size) {
    char fallback[512];
    const char *path = source_path(fallback, env_name, file_name);
    char md5[33];
    FILE *file;
    uint8_t bytes[sizeof(k_consumer_bytes)];

    if (!path || !m12_file_md5_hex(path, md5)) {
        printf("SKIP: authentic %s is not staged\n", file_name);
        return 0;
    }
    if (strcmp(md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: unexpected %s MD5 %s\n", file_name, md5);
        return -1;
    }
    if (!(file = fopen(path, "rb")) || fseek(file, 0L, SEEK_END) != 0 ||
        ftell(file) != expected_size ||
        fseek(file, (long)(BANK1F_FILE_OFFSET + CONSUMER_OFFSET), SEEK_SET) != 0 ||
        fread(bytes, 1, sizeof(bytes), file) != sizeof(bytes)) {
        if (file) fclose(file);
        fprintf(stderr, "FAIL: %s bank-$1f consumer window\n", file_name);
        return -1;
    }
    fclose(file);
    if (memcmp(bytes, k_consumer_bytes, sizeof(bytes)) != 0) {
        fprintf(stderr, "FAIL: %s bank-$1f consumer bytes differ\n", file_name);
        return -1;
    }
    printf("PASS: %s MD5=%s bank=$1f offset=$%x bytes=%zu "
           "ram_consumer_2600=not_present\n", file_name, md5,
           CONSUMER_OFFSET, sizeof(bytes));
    return 1;
}

int main(void) {
    int checked = 0;
    int rc;

    rc = verify_variant("FIRESTAFF_THERON_US_TRACK19_ISO", "TQUS19.iso",
                        "51b40a17b92a30339957ba564aa0015c", 5984256L);
    if (rc < 0) return 1;
    checked |= rc;
    rc = verify_variant("FIRESTAFF_THERON_JP_TRACK19_ISO", "TQJP19.iso",
                        "f9f069a5e489b91207f3156059b756f1", 6291456L);
    if (rc < 0) return 1;
    checked |= rc;
    return checked ? 0 : 77;
}
