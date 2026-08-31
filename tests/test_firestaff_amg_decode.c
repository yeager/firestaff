#include "firestaff_amg_decode.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int probe_file(const char* path) {
    uint8_t* data;
    size_t size;
    FirestaffAmgSnd2 snd;
    int rc;

    data = NULL;
    size = 0u;
    if (!(strstr(path, "::")
              ? asset_read_virtual_path_alloc(path, &data, &size)
              : asset_read_path_alloc(path, &data, &size))) {
        fprintf(stderr, "test_firestaff_amg_decode: cannot read %s in RAM\n", path);
        return 1;
    }

    rc = FirestaffAmgSnd2_Decode(data, size, &snd);
    if (rc != 0) {
        fprintf(stderr, "test_firestaff_amg_decode: %s decode rc=%d\n", path, rc);
        free(data);
        return 1;
    }
    printf("%s: samples=%u trailing=%zu\n",
           path, (unsigned)snd.sample_count, snd.trailing_bytes);
    free(data);
    return 0;
}

static int probe_csb_amiga_real_media(const char* archive) {
    static const struct {
        const char* member;
        unsigned int samples;
        size_t trailing;
    } expected[] = {
        { "SWIPE.AMG", 995u, 2u },
        { "TELE2.AMG", 1506u, 1u },
        { "MAGEXPLO.AMG", 3107u, 2u },
        { "DRAGON.AMG", 4003u, 2u },
        { "EXPLOS1.AMG", 3970u, 1u }
    };
    size_t i;
    int failures = 0;

    for (i = 0u; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        char path[4096];
        uint8_t* data = NULL;
        size_t size = 0u;
        FirestaffAmgSnd2 snd;

        if (snprintf(path, sizeof(path), "%s::Chaos Strikes Back (FTL) B.adf::%s",
                     archive, expected[i].member) >= (int)sizeof(path) ||
            !asset_read_virtual_path_alloc(path, &data, &size)) {
            fprintf(stderr, "test_firestaff_amg_decode: cannot read original %s\n",
                    expected[i].member);
            ++failures;
            continue;
        }
        if (FirestaffAmgSnd2_Decode(data, size, &snd) != 0 ||
            snd.sample_count != expected[i].samples ||
            snd.trailing_bytes != expected[i].trailing) {
            fprintf(stderr,
                    "test_firestaff_amg_decode: original %s mismatch (samples=%u trailing=%zu)\n",
                    expected[i].member, (unsigned)snd.sample_count, snd.trailing_bytes);
            ++failures;
        }
        free(data);
    }
    return failures;
}

int main(int argc, char** argv) {
    const char* realArchive;
    int i;
    int failures = 0;

    if (FirestaffAmgSnd2_SelfTest() != 0) {
        printf("test_firestaff_amg_decode: FAIL\n");
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        failures += probe_file(argv[i]);
    }
    realArchive = getenv("FIRESTAFF_CSB_AMIGA_AMG_ARCHIVE");
    if (realArchive && realArchive[0] != '\0') {
        FILE* archive = fopen(realArchive, "rb");
        if (!archive) {
            printf("test_firestaff_amg_decode: SKIP original CSB Amiga archive unavailable\n");
            return 77;
        }
        fclose(archive);
        failures += probe_csb_amiga_real_media(realArchive);
    }
    if (failures) {
        printf("test_firestaff_amg_decode: FAIL\n");
        return 1;
    }

    printf("test_firestaff_amg_decode: PASS\n");
    return 0;
}
