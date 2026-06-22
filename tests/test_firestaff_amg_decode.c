#include "firestaff_amg_decode.h"

#include <stdio.h>
#include <stdlib.h>

static int probe_file(const char* path) {
    FILE* f;
    long size;
    uint8_t* data;
    FirestaffAmgSnd2 snd;
    int rc;

    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "test_firestaff_amg_decode: cannot open %s\n", path);
        return 1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 1;
    }
    size = ftell(f);
    if (size < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 1;
    }
    data = (uint8_t*)malloc((size_t)size ? (size_t)size : 1u);
    if (!data) {
        fclose(f);
        return 1;
    }
    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 1;
    }
    fclose(f);

    rc = FirestaffAmgSnd2_Decode(data, (size_t)size, &snd);
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

int main(int argc, char** argv) {
    int i;
    int failures = 0;

    if (FirestaffAmgSnd2_SelfTest() != 0) {
        printf("test_firestaff_amg_decode: FAIL\n");
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        failures += probe_file(argv[i]);
    }
    if (failures) {
        printf("test_firestaff_amg_decode: FAIL\n");
        return 1;
    }

    printf("test_firestaff_amg_decode: PASS\n");
    return 0;
}
