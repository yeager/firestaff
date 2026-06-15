/*
 * Nexus V1 save header rejection — malformed header regression.
 *
 * This test is intentionally narrow: one synthetic save file is written with a
 * valid-length header where only the magic is invalid. The expected behavior is:
 * 1) nexus_v1_save_probe() rejects it with an explicit unknown-magic diagnostic.
 * 2) nexus_v1_load_from_path() returns NEXUS_SAVE_ERR_UNKNOWN_VARIANT.
 */

#include "nexus_v1_save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#include <io.h>
#define NTEST_MKDIR(path) _mkdir(path)
#define NTEST_RMDIR(path) _rmdir(path)
#define NTEST_UNLINK(path) _unlink(path)
#define NTEST_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define NTEST_MKDIR(path) mkdir((path), 0700)
#define NTEST_RMDIR(path) rmdir(path)
#define NTEST_UNLINK(path) unlink(path)
#define NTEST_GETPID() getpid()
#endif

static int g_failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_temp_root(char *out, size_t out_size) {
    const char *base = getenv("TMPDIR");
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)NTEST_GETPID();
    int i;

#ifdef _WIN32
    if (!base || !base[0]) base = getenv("TEMP");
#endif
    if (!base || !base[0]) base = "/tmp";

    for (i = 0; i < 64; ++i) {
        int n = snprintf(out, out_size, "%s/firestaff-nexus-save-header-%u-%d",
                         base, seed, i);
        if (n < 0 || (size_t)n >= out_size) return 0;
        if (NTEST_MKDIR(out) == 0) return 1;
    }

    return 0;
}

static int write_bytes(const char *path, const void *data, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && fwrite(data, 1, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

int main(void) {
    char root[512];
    char path[512];
    Nexus_V1_SaveHeader malformed;
    char diagnostic[256] = {0};
    const char *probe_reason;
    size_t champion_size = 0;
    size_t world_size = 0;
    unsigned char champion_data[64] = {0};
    unsigned char world_data[64] = {0};
    Nexus_V1_SaveHeader out_header = {0};
    Nexus_SaveResult result;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary directory\n");
        return 1;
    }

    memset(&malformed, 0, sizeof(malformed));
    malformed.magic = 0x214D4147U; /* 'MAG!' */
    malformed.version = NEXUS_SAVE_VERSION;
    malformed.header_size = (uint16_t)sizeof(malformed);
    malformed.current_level = 1;
    malformed.party_x = 2;
    malformed.party_y = 3;
    malformed.party_dir = 1;
    malformed.game_time = 77U;
    malformed.state_hash = 0x12345678U;
    snprintf(path, sizeof(path), "%s/nexus_save_00.dat", root);

    expect(write_bytes(path, &malformed, sizeof(malformed)),
           "test writes one save file with bad magic and valid length");

    probe_reason = nexus_v1_save_probe(path, NULL, NULL);
    expect(probe_reason != NULL && probe_reason[0] != '\0',
           "save probe rejects malformed magic with a reason");
    expect(strstr(probe_reason, "unknown magic") != NULL,
           "malformed header reports unknown magic");

    memset(&out_header, 0x5A, sizeof(out_header));
    result = nexus_v1_load_from_path(path, &out_header,
                                     champion_data, sizeof(champion_data),
                                     &champion_size, world_data, sizeof(world_data),
                                     &world_size, diagnostic, sizeof(diagnostic));
    expect(result == NEXUS_SAVE_ERR_UNKNOWN_VARIANT,
           "loading malformed header returns variant-rejection error");
    expect(strcmp(diagnostic, probe_reason) == 0,
           "diagnostic from load mirrors probe reason for unknown magic");
    expect(out_header.magic != malformed.magic,
           "failed load keeps output header untouched when probing fails");

    NTEST_UNLINK(path);
    NTEST_RMDIR(root);

    if (g_failures) {
        return 1;
    }

    puts("ok: malformed Nexus header with bad magic is rejected with variant error");
    return 0;
}
