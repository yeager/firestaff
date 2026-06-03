/*
 * Nexus V1 save/load missing-root regression.
 *
 * Headless synthetic coverage for Firestaff-native FNXS save paths. The
 * save format itself is implemented in src/nexus/nexus_v1_save_load.c and
 * cites ReDMCSB LOADSAVE.C F0433/F0434 plus SAVEHEAD.C F0429/F0430 for the
 * source-locked save/load lineage; this test stays at the filesystem/error
 * boundary and uses no original game data.
 */

#include "nexus_v1_save.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
#define FS_RMDIR(path) _rmdir(path)
#define FS_UNLINK(path) _unlink(path)
#define FS_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define FS_MKDIR(path) mkdir((path), 0700)
#define FS_RMDIR(path) rmdir(path)
#define FS_UNLINK(path) unlink(path)
#define FS_GETPID() getpid()
#endif

static int expect(int cond, const char *msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static int make_temp_root(char *buf, size_t bufsz) {
    const char *base = getenv("TMPDIR");
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)FS_GETPID();
    int i;

#ifdef _WIN32
    if (!base || !base[0]) base = getenv("TEMP");
#endif
    if (!base || !base[0]) base = "/tmp";

    for (i = 0; i < 64; ++i) {
        int n = snprintf(buf, bufsz, "%s/firestaff-nexus-save-missing-root-%u-%d",
                         base, seed, i);
        if (n < 0 || (size_t)n >= bufsz) return 0;
        if (FS_MKDIR(buf) == 0) return 1;
    }
    return 0;
}

static int write_blocking_file(const char *path, const char *contents) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fputs(contents, fp) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int file_contains(const char *path, const char *contents) {
    char buf[64];
    FILE *fp = fopen(path, "rb");
    size_t n;

    if (!fp) return 0;
    memset(buf, 0, sizeof(buf));
    n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    return n == strlen(contents) && memcmp(buf, contents, n) == 0;
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

static int expect_no_launchable_slots(const Nexus_V1_SaveManager *mgr,
                                      const char *msg) {
    int i;

    for (i = 0; i < NEXUS_SAVE_MAX_SLOTS; ++i) {
        if (!expect(nexus_v1_save_get_slot(mgr, (uint8_t)i) == NULL, msg)) return 0;
        if (!expect(mgr->slots[i].occupied == 0, msg)) return 0;
        if (!expect(mgr->slots[i].label[0] == '\0', msg)) return 0;
    }
    return 1;
}

static int test_scan_missing_empty_and_invalid_roots_do_not_launch(const char *root) {
    char save_root[512];
    char path[512];
    Nexus_V1_SaveManager mgr;
    Nexus_V1_SaveHeader hdr;
    static const unsigned char tiny_invalid[] = { 'n', 'o' };

    snprintf(save_root, sizeof(save_root), "%s/scan-missing-root", root);
    nexus_v1_save_init(&mgr, save_root);
    if (!expect(FS_RMDIR(save_root) == 0,
                "test removes scan root after manager init")) return 0;
    if (!expect(nexus_v1_save_scan(&mgr) == 0,
                "scan of missing Nexus save root does not crash")) return 0;
    if (!expect_no_launchable_slots(&mgr,
                                    "missing Nexus save root has no launchable slots")) return 0;

    snprintf(save_root, sizeof(save_root), "%s/scan-empty-root", root);
    nexus_v1_save_init(&mgr, save_root);
    if (!expect(nexus_v1_save_scan(&mgr) == 0,
                "scan of empty Nexus save root does not crash")) return 0;
    if (!expect_no_launchable_slots(&mgr,
                                    "empty Nexus save root has no launchable slots")) return 0;

    snprintf(save_root, sizeof(save_root), "%s/scan-invalid-root", root);
    nexus_v1_save_init(&mgr, save_root);

    snprintf(path, sizeof(path), "%s/nexus_save_00.dat", save_root);
    if (!expect(write_bytes(path, tiny_invalid, sizeof(tiny_invalid)),
                "test writes too-small Nexus slot fixture")) return 0;

    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = 0x21444142U; /* "BAD!" */
    hdr.version = NEXUS_SAVE_VERSION;
    hdr.header_size = (uint16_t)sizeof(hdr);
    snprintf(path, sizeof(path), "%s/nexus_save_01.dat", save_root);
    if (!expect(write_bytes(path, &hdr, sizeof(hdr)),
                "test writes wrong-magic Nexus slot fixture")) return 0;

    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = NEXUS_SAVE_MAGIC;
    hdr.version = (uint16_t)(NEXUS_SAVE_VERSION + 1);
    hdr.header_size = (uint16_t)sizeof(hdr);
    snprintf(path, sizeof(path), "%s/nexus_save_02.dat", save_root);
    if (!expect(write_bytes(path, &hdr, sizeof(hdr)),
                "test writes unsupported-version Nexus slot fixture")) return 0;

    if (!expect(nexus_v1_save_scan(&mgr) == 0,
                "scan of invalid Nexus save entries does not crash")) return 0;
    if (!expect_no_launchable_slots(&mgr,
                                    "invalid Nexus save entries have no launchable slots")) return 0;

    snprintf(path, sizeof(path), "%s/nexus_save_00.dat", save_root);
    FS_UNLINK(path);
    snprintf(path, sizeof(path), "%s/nexus_save_01.dat", save_root);
    FS_UNLINK(path);
    snprintf(path, sizeof(path), "%s/nexus_save_02.dat", save_root);
    FS_UNLINK(path);
    FS_RMDIR(save_root);

    return 1;
}

static int test_load_from_missing_root_does_not_mutate_outputs(const char *root) {
    char save_root[512];
    Nexus_V1_SaveManager mgr;
    Nexus_V1_SaveHeader header_before;
    Nexus_V1_SaveHeader header_after;
    unsigned char champion_before[16];
    unsigned char champion_after[16];
    unsigned char world_before[16];
    unsigned char world_after[16];
    size_t champion_size;
    size_t world_size;
    char diagnostic[128];
    Nexus_SaveResult result;

    snprintf(save_root, sizeof(save_root), "%s/load-root", root);
    nexus_v1_save_init(&mgr, save_root);
    if (!expect(mgr.initialized == 1, "save manager initializes missing root")) return 0;
    if (!expect(FS_RMDIR(save_root) == 0, "test removes save root after init")) return 0;

    memset(&header_before, 0x5a, sizeof(header_before));
    header_before.current_level = 7;
    header_before.party_x = 11;
    header_before.party_y = 29;
    header_after = header_before;

    memset(champion_before, 0x31, sizeof(champion_before));
    memcpy(champion_after, champion_before, sizeof(champion_after));
    memset(world_before, 0x42, sizeof(world_before));
    memcpy(world_after, world_before, sizeof(world_after));
    champion_size = 1234U;
    world_size = 5678U;

    mgr.slots[0].occupied = 1;
    mgr.slots[0].slot_index = 0;
    mgr.slots[0].header.current_level = 99;
    snprintf(mgr.slots[0].label, sizeof(mgr.slots[0].label), "%s", "sentinel slot");
    mgr.slots[0].timestamp = 0x11223344U;

    memset(diagnostic, 0, sizeof(diagnostic));
    result = nexus_v1_load(&mgr, 0, &header_after,
                           champion_after, sizeof(champion_after), &champion_size,
                           world_after, sizeof(world_after), &world_size,
                           diagnostic, sizeof(diagnostic));

    if (!expect(result != NEXUS_SAVE_OK, "load from missing root is not successful")) return 0;
    if (!expect(result == NEXUS_SAVE_ERR_MAGIC,
                "load from missing root follows existing probe failure result")) return 0;
    if (!expect(diagnostic[0] != '\0', "load failure provides diagnostic text")) return 0;
    if (!expect(strstr(diagnostic, "file not found") != NULL ||
                strstr(diagnostic, "cannot be opened") != NULL,
                "load diagnostic explains missing or unavailable file")) return 0;
    if (!expect(strcmp(nexus_v1_save_strerror(result), "not a Nexus save file (bad magic)") == 0,
                "load failure has useful strerror mapping")) return 0;
    if (!expect(memcmp(&header_after, &header_before, sizeof(header_after)) == 0,
                "failed load leaves header output unchanged")) return 0;
    if (!expect(memcmp(champion_after, champion_before, sizeof(champion_after)) == 0,
                "failed load leaves champion buffer unchanged")) return 0;
    if (!expect(memcmp(world_after, world_before, sizeof(world_after)) == 0,
                "failed load leaves world buffer unchanged")) return 0;
    if (!expect(champion_size == 1234U && world_size == 5678U,
                "failed load leaves size outputs unchanged")) return 0;
    if (!expect(mgr.slots[0].occupied == 1 &&
                mgr.slots[0].header.current_level == 99 &&
                strcmp(mgr.slots[0].label, "sentinel slot") == 0 &&
                mgr.slots[0].timestamp == 0x11223344U,
                "failed load does not mutate cached slot metadata")) return 0;

    return 1;
}

static int test_save_to_unavailable_root_does_not_mark_slot_successful(const char *root) {
    static const char blocker_contents[] = "not a directory\n";
    char blocker[512];
    Nexus_V1_SaveManager mgr;
    const unsigned char champion_data[4] = { 1U, 2U, 3U, 4U };
    const unsigned char world_data[4] = { 5U, 6U, 7U, 8U };
    Nexus_SaveResult result;

    snprintf(blocker, sizeof(blocker), "%s/save-root-is-file", root);
    if (!expect(write_blocking_file(blocker, blocker_contents),
                "test creates regular file at save root path")) return 0;

    nexus_v1_save_init(&mgr, blocker);
    if (!expect(mgr.initialized == 1, "manager records unavailable root for save attempt")) return 0;
    if (!expect(mgr.slots[0].occupied == 0, "slot starts unoccupied")) return 0;

    result = nexus_v1_save(&mgr, 0,
                           3, 10, 12, 1,
                           77U, 0x123456789abcdef0ULL,
                           champion_data, sizeof(champion_data),
                           world_data, sizeof(world_data));

    if (!expect(result != NEXUS_SAVE_OK, "save to unavailable root is not successful")) return 0;
    if (!expect(result == NEXUS_SAVE_ERR_OPEN,
                "save to unavailable root returns open failure")) return 0;
    if (!expect(strcmp(nexus_v1_save_strerror(result), "cannot open file") == 0,
                "save failure has useful strerror mapping")) return 0;
    if (!expect(mgr.slots[0].occupied == 0 &&
                mgr.slots[0].label[0] == '\0' &&
                mgr.slots[0].timestamp == 0,
                "failed save does not mark slot occupied")) return 0;
    if (!expect(file_contains(blocker, blocker_contents),
                "failed save leaves blocking file unchanged")) return 0;

    FS_UNLINK(blocker);
    return 1;
}

int main(void) {
    char root[512];
    int ok = 1;

    if (!make_temp_root(root, sizeof(root))) {
        fprintf(stderr, "FAIL: could not create temporary test root\n");
        return 1;
    }

    ok = test_load_from_missing_root_does_not_mutate_outputs(root) && ok;
    ok = test_save_to_unavailable_root_does_not_mark_slot_successful(root) && ok;
    ok = test_scan_missing_empty_and_invalid_roots_do_not_launch(root) && ok;

    FS_RMDIR(root);

    if (ok) {
        puts("ok: Nexus V1 save/load missing-root and invalid-entry failures stay non-launchable");
        return 0;
    }
    return 1;
}
