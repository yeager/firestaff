#include "artpack_admission_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void expect_true(int condition, const char* label) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++g_failures;
    }
}

static void write_bytes(const char* path, const unsigned char* data,
                        size_t size) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "FAIL: open %s\n", path);
        ++g_failures;
        return;
    }
    if (fwrite(data, 1U, size, fp) != size) {
        fprintf(stderr, "FAIL: write %s\n", path);
        ++g_failures;
    }
    fclose(fp);
}

int main(void) {
    const char* fontPath = "/tmp/firestaff-ui-unicode-test-font.ttf";
    const char* artpackPath = "/tmp/firestaff-v22-test-artpack.fsart";
    const unsigned char fontBytes[] = {'t','t','f','0'};
    const unsigned char artpackBytes[] = {'F','S','A','R','T','0','0','1'};
    M12_Config config;
    char resolvedFont[M12_CONFIG_DATA_DIR_CAPACITY];
    M12_ArtpackAdmissionReceipt artpack;

    write_bytes(fontPath, fontBytes, sizeof(fontBytes));
    write_bytes(artpackPath, artpackBytes, sizeof(artpackBytes));
    expect_true(FSP_SetEnv("FIRESTAFF_UI_FONT", fontPath, 1) == 0,
                "test font env set");

    expect_true(M12_Config_FindDefaultUnicodeFontPath(
                    resolvedFont, sizeof(resolvedFont)) == 1,
                "unicode font resolver finds override");
    expect_true(strcmp(resolvedFont, fontPath) == 0,
                "unicode font resolver returns override path");

    M12_Config_SetDefaults(&config);
    expect_true(strcmp(config.unicodeFontPath, fontPath) == 0,
                "startup config defaults to UTF-8 capable font path");

    expect_true(M12_ArtpackAdmission_Check(artpackPath, &artpack) == 1,
                "startup admits fsart files produced by artpack studio");
    expect_true(artpack.admitted == 1 &&
                    artpack.status == M12_ARTPACK_ADMISSION_ACCEPTED_FSAR,
                "fsart receipt is accepted-fsar");
    expect_true(!artpack.fallbackVisualsPermitted,
                "fsart selection never enables fallback visuals");

    remove(fontPath);
    remove(artpackPath);

    if (g_failures) {
        fprintf(stderr, "%d failures\n", g_failures);
        return 1;
    }
    puts("m12 startup font/artpack settings: ok");
    return 0;
}
