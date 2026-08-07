/* Production gate: DM2 must not write a Firestaff-private session as an
 * original SKSave.dat before SKProject's DM2_GAME_SAVE writer is ported. */
#include "dm2_v1_boot.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_save_load.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define DM2_TEST_GETPID _getpid
#define DM2_TEST_MKDIR(path) _mkdir(path)
#define DM2_TEST_RMDIR(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define DM2_TEST_GETPID getpid
#define DM2_TEST_MKDIR(path) mkdir((path), 0700)
#define DM2_TEST_RMDIR(path) rmdir(path)
#endif

static int file_fingerprint(const char *path, uint32_t *out_hash,
                            size_t *out_size)
{
    FILE *file;
    uint8_t buffer[4096];
    size_t count;
    uint32_t hash = UINT32_C(2166136261);
    size_t size = 0u;

    if (!path || !out_hash || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    while ((count = fread(buffer, 1u, sizeof(buffer), file)) != 0u) {
        for (size_t i = 0u; i < count; ++i) {
            hash ^= buffer[i];
            hash *= UINT32_C(16777619);
        }
        size += count;
    }
    if (ferror(file)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_hash = hash;
    *out_size = size;
    return 1;
}

static int test_real_original_save_is_unchanged(void)
{
    const char *root = getenv("FIRESTAFF_DM2_SKSAVE_CORPUS");
    char path[512];
    uint32_t before_hash;
    uint32_t after_hash;
    size_t before_size;
    size_t after_size;
    DM2_V1_BootProfile profile;
    DM2_V1_QuicksaveReceipt receipt;

    if (!root || !root[0]) return 1;
    snprintf(path, sizeof(path), "%s/sksave0.dat", root);
    if (!file_fingerprint(path, &before_hash, &before_size)) return 1;

    dm2_v1_boot_profile_init(&profile);
    snprintf(profile.save_root, sizeof(profile.save_root), "%s", root);
    memset(&receipt, 0, sizeof(receipt));
    if (dm2_v1_runtime_quicksave_boot_profile_with_receipt(
            &profile, &receipt) != 0 ||
        receipt.result != DM2_V1_QUICKSAVE_ORIGINAL_WRITER_REQUIRED ||
        receipt.session_valid || receipt.save_path[0] != '\0' ||
        !file_fingerprint(path, &after_hash, &after_size) ||
        before_hash != after_hash || before_size != after_size) {
        return 0;
    }
    return 1;
}

int main(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_QuicksaveReceipt receipt;
    DM2_V1_SessionState session;
    uint8_t payload[DM2_SESSION_MAX_SIZE];
    size_t payload_size;
    char parent[256];
    char absent_root[320];
    FILE *file;

    snprintf(parent, sizeof(parent), "/tmp/firestaff_dm2_writer_%d",
             DM2_TEST_GETPID());
    (void)DM2_TEST_RMDIR(parent);
    if (DM2_TEST_MKDIR(parent) != 0) return 1;
    snprintf(absent_root, sizeof(absent_root), "%s/no-original-writer",
             parent);

    memset(&session, 0, sizeof(session));
    if (dm2_v1_session_save_slot(absent_root, 0u, "fixture", &session) !=
            DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED ||
        dm2_v1_session_save_last_session(absent_root, "fixture", &session) !=
            DM2_V1_SESSION_WRITE_ORIGINAL_WRITER_REQUIRED) {
        (void)DM2_TEST_RMDIR(parent);
        return 4;
    }

    /* A historic D2RS fixture may still be decoded for diagnostics, but it
     * must never re-enter the playable slot/resume path. */
    memset(&session, 0, sizeof(session));
    payload_size = (size_t)dm2_v1_session_serialize(&session, payload,
                                                      sizeof(payload));
    if (payload_size == 0u ||
        dm2_sl_save(parent, 0u, "D2RS fixture", payload, payload_size) != 0 ||
        dm2_v1_session_load_slot(parent, 0u, &session) == 0) {
        (void)dm2_sl_delete(parent, 0u);
        (void)DM2_TEST_RMDIR(parent);
        return 5;
    }
    (void)dm2_sl_delete(parent, 0u);

    dm2_v1_boot_profile_init(&profile);
    snprintf(profile.save_root, sizeof(profile.save_root), "%s", absent_root);
    memset(&receipt, 0, sizeof(receipt));
    if (dm2_v1_runtime_quicksave_boot_profile_with_receipt(&profile,
                                                            &receipt) != 0 ||
        receipt.result != DM2_V1_QUICKSAVE_ORIGINAL_WRITER_REQUIRED ||
        receipt.session_valid || receipt.save_path[0] != '\0' ||
        receipt.status != NULL) {
        (void)DM2_TEST_RMDIR(parent);
        return 2;
    }

    file = fopen(absent_root, "rb");
    if (file != NULL) {
        fclose(file);
        (void)DM2_TEST_RMDIR(parent);
        return 3;
    }
    if (getenv("FIRESTAFF_DM2_SKSAVE_CORPUS") &&
        !test_real_original_save_is_unchanged()) {
        (void)DM2_TEST_RMDIR(parent);
        return 6;
    }
    (void)DM2_TEST_RMDIR(parent);
    return 0;
}
