#include "artpack_admission_m12.h"

#include <stdio.h>
#include <stdlib.h>
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
        fprintf(stderr, "FAIL: open fixture %s\n", path);
        ++g_failures;
        return;
    }
    if (fwrite(data, 1U, size, fp) != size) {
        fprintf(stderr, "FAIL: write fixture %s\n", path);
        ++g_failures;
    }
    fclose(fp);
}

int main(void) {
    M12_ArtpackAdmissionReceipt receipt;
    const unsigned char fsar[] = {'F','S','A','R','T','0','0','1'};
    const unsigned char zip[] = {'P','K',0x03,0x04,'x','x','x','x'};
    const unsigned char bad[] = {'N','O','P','E','x','x','x','x'};
    const unsigned char tiny[] = {'F','S','A','R'};

    write_bytes("/tmp/firestaff-artpack-ok.fsart", fsar, sizeof(fsar));
    write_bytes("/tmp/firestaff-artpack-upper.FSART", fsar, sizeof(fsar));
    write_bytes("/tmp/firestaff-artpack-okzip.fsart", zip, sizeof(zip));
    write_bytes("/tmp/firestaff-artpack-bad.fsart", bad, sizeof(bad));
    write_bytes("/tmp/firestaff-artpack-tiny.fsart", tiny, sizeof(tiny));

    expect_true(!M12_ArtpackAdmission_Check(NULL, &receipt),
                "null path rejected");
    expect_true(receipt.status == M12_ARTPACK_ADMISSION_EMPTY,
                "null status empty");
    expect_true(!receipt.fallbackVisualsPermitted,
                "null fallback forbidden");

    expect_true(!M12_ArtpackAdmission_Check("/tmp/firestaff-artpack-ok.zip",
                                            &receipt),
                "wrong extension rejected");
    expect_true(receipt.status == M12_ARTPACK_ADMISSION_PATH_NOT_FSART,
                "wrong extension status");

    expect_true(M12_ArtpackAdmission_Check("/tmp/firestaff-artpack-ok.fsart",
                                           &receipt),
                "FSAR accepted");
    expect_true(receipt.admitted == 1, "FSAR admitted flag");
    expect_true(receipt.status == M12_ARTPACK_ADMISSION_ACCEPTED_FSAR,
                "FSAR status");
    expect_true(!receipt.fallbackVisualsPermitted,
                "FSAR fallback forbidden");

    expect_true(M12_ArtpackAdmission_Check("/tmp/firestaff-artpack-upper.FSART",
                                           &receipt),
                "uppercase FSART accepted");
    expect_true(receipt.status == M12_ARTPACK_ADMISSION_ACCEPTED_FSAR,
                "uppercase FSART status");

    expect_true(M12_ArtpackAdmission_Check("/tmp/firestaff-artpack-okzip.fsart",
                                           &receipt),
                "zip accepted");
    expect_true(receipt.status == M12_ARTPACK_ADMISSION_ACCEPTED_ZIP,
                "zip status");

    expect_true(!M12_ArtpackAdmission_Check("/tmp/firestaff-artpack-bad.fsart",
                                            &receipt),
                "bad signature rejected");
    expect_true(receipt.status ==
                    M12_ARTPACK_ADMISSION_UNSUPPORTED_SIGNATURE,
                "bad signature status");
    expect_true(!receipt.fallbackVisualsPermitted,
                "bad fallback forbidden");

    expect_true(!M12_ArtpackAdmission_Check("/tmp/firestaff-artpack-tiny.fsart",
                                            &receipt),
                "tiny rejected");
    expect_true(receipt.status == M12_ARTPACK_ADMISSION_TOO_SMALL,
                "tiny status");

    remove("/tmp/firestaff-artpack-ok.fsart");
    remove("/tmp/firestaff-artpack-upper.FSART");
    remove("/tmp/firestaff-artpack-okzip.fsart");
    remove("/tmp/firestaff-artpack-bad.fsart");
    remove("/tmp/firestaff-artpack-tiny.fsart");

    if (g_failures) {
        fprintf(stderr, "%d failures\n", g_failures);
        return 1;
    }
    puts("m12 artpack admission: ok");
    return 0;
}
