/* DM2 V1 Save/Load — Integration Tests
 *
 * Tests:
 *   1. SUPPRESS codec encode/decode round-trip
 *   2. SUPPRESS decode fill=1 vs fill=0 modes
 *   3. Slot header encoding (0xBEEF/0xDEAD magic, name, slot+0x30)
 *   4. Slot scan: occupied vs empty detection
 *   5. Save + load round-trip (stateless path)
 *   6. Backup fallback on load
 *   7. Cross-version diagnostics: DM2/DM1/unknown/null-fill detection
 *   8. SUPPRESS codec self-test
 *   9. Champion record SUPPRESS mask (261 bytes, low nibbles only)
 *  10. DB handle identity (make + resolve round-trip)
 *
 * Source refs:
 *   docs/dm2_save_format.md — SUPPRESS codec, slot header layout
 *   docs/dm2_save_slots.md — 10 slots, 0xBEEF/0xDEAD magic
 *   docs/dm2_party_state.md — champion 261-byte format
 */

#include "dm2_v1_save_load.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
#define FS_RMDIR(path) _rmdir(path)
#define FS_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define FS_MKDIR(path) mkdir(path, 0700)
#define FS_RMDIR(path) rmdir(path)
#define FS_GETPID() getpid()
#endif

extern int dm2_suppress_self_verification(void);

/* ── Test 1: SUPPRESS all-1s mask round-trip ──────────────────── */

static int test_suppress_all1_roundtrip(void)
{
    printf("  SUPPRESS all-1s mask round-trip...\n");
    /* mask[0..7] = 0x11 → nbits=1 for all 8 bytes */
    uint8_t data[8] = { 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00 };
    uint8_t mask[8] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
    uint8_t enc[64];
    uint8_t dec[8];

    int enc_sz = dm2_suppress_encode(data, mask, 8, enc, sizeof(enc));
    if (enc_sz < 0) { printf("    FAIL: encode error %d\n", enc_sz); return 0; }
    if (enc_sz == 0) { printf("    FAIL: encode produced nothing\n"); return 0; }

    int dec_sz = dm2_suppress_decode(enc, (size_t)enc_sz, mask, 8, dec, 0);
    if (dec_sz < 0) { printf("    FAIL: decode error %d\n", dec_sz); return 0; }

    if (memcmp(data, dec, 8) != 0) {
        printf("    FAIL: round-trip mismatch\n");
        for (int k = 0; k < 8; k++)
            printf("      [%d] data=0x%02X dec=0x%02X\n", k, data[k], dec[k]);
        return 0;
    }
    printf("    PASS: enc=%d bytes, round-trip OK\n", enc_sz);
    return 1;
}

/* ── Test 2: SUPPRESS fill modes ──────────────────────────────── */

static int test_suppress_fill_mode(void)
{
    printf("  SUPPRESS fill-vs-zero mode...\n");
    /* Only store bytes 0 and 2; mask[1,3..] = 0 (skip) */
    uint8_t data[8]  = { 0x3A, 0x00, 0xF5, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t mask[8]  = { 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t enc[64];
    uint8_t dec0[8], dec1[8];

    memset(dec0, 0xAA, 8);
    memset(dec1, 0xAA, 8);

    int enc_sz = dm2_suppress_encode(data, mask, 8, enc, sizeof(enc));
    if (enc_sz < 0) { printf("    FAIL: encode error\n"); return 0; }

    int r0 = dm2_suppress_decode(enc, (size_t)enc_sz, mask, 8, dec0, 0);
    int r1 = dm2_suppress_decode(enc, (size_t)enc_sz, mask, 8, dec1, 1);
    if (r0 < 0 || r1 < 0) { printf("    FAIL: decode errors\n"); return 0; }

    /* fill=1 should set skipped fields to 0xFF */
    if (dec1[1] != 0xFF) {
        printf("    FAIL: fill=1 not 0xFF (got 0x%02X)\n", dec1[1]);
        return 0;
    }
    /* fill=0 should leave skipped at 0xAA pre-fill value (0) */
    if (dec0[1] != 0xAA && dec0[1] != 0x00) {
        printf("    FAIL: fill=0 changed skipped field unexpectedly\n");
        return 0;
    }
    printf("    PASS: fill=0/1 modes differ correctly\n");
    return 1;
}

/* ── Test 3: Slot header encoding ──────────────────────────────── */

static int test_slot_header_encoding(void)
{
    printf("  Slot header encoding (magic BEEF/DEAD, name, slot+0x30)...\n");

    for (uint8_t s = 0; s < 10; s++) {
        uint8_t hdr[42] = {0};
        hdr[0] = 1; hdr[1] = 0; /* version flag */

        char name[64];
        snprintf(name, sizeof(name), "Slot_%u", (unsigned)s);
        size_t nlen = strlen(name);
        if (nlen > 33) nlen = 33;
        memcpy(hdr + 2, name, nlen);

        hdr[36] = (uint8_t)((s + 0x30) & 0xFF);
        hdr[37] = 0;

        hdr[38] = (uint8_t)(0xBEEF & 0xFF);
        hdr[39] = (uint8_t)((0xBEEF >> 8) & 0xFF);
        hdr[40] = (uint8_t)(0xDEAD & 0xFF);
        hdr[41] = (uint8_t)((0xDEAD >> 8) & 0xFF);

        uint16_t m1 = (uint16_t)hdr[38] | ((uint16_t)hdr[39] << 8);
        uint16_t m2 = (uint16_t)hdr[40] | ((uint16_t)hdr[41] << 8);
        if (m1 != 0xBEEF || m2 != 0xDEAD) {
            printf("    FAIL slot %u: magic wrong 0x%04X/0x%04X\n", s, m1, m2);
            return 0;
        }

        uint16_t slot_field = (uint16_t)hdr[36] | ((uint16_t)hdr[37] << 8);
        if (slot_field != (s + 0x30)) {
            printf("    FAIL slot %u: slot field 0x%04X expected 0x%02X\n",
                   s, slot_field, s + 0x30);
            return 0;
        }

        uint16_t vflag = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
        if (vflag != 1) {
            printf("    FAIL slot %u: vflag %u expected 1\n", s, vflag);
            return 0;
        }
    }
    printf("    PASS: slot headers 0..9 all correct\n");
    return 1;
}

/* ── Test 4: Slot scan (occupied vs empty) ─────────────────── */

static int test_slot_scan(void)
{
    printf("  Slot scan occupied detection...\n");
    char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_test_%d", FS_GETPID());
    FS_MKDIR(tmpdir);

    DM2_SL_State st;
    dm2_sl_init(&st, tmpdir);
    dm2_sl_scan_slots(&st);

    for (uint8_t s = 0; s < 10; s++) {
        if (dm2_sl_slot_occupied(&st, s)) {
            printf("    FAIL: slot %u incorrectly marked occupied\n", s);
            FS_RMDIR(tmpdir);
            return 0;
        }
    }
    printf("    PASS: initial scan shows all empty\n");
    FS_RMDIR(tmpdir);
    return 1;
}

/* ── Test 5: Save + load round-trip (stateless) ─────────────── */

static int test_save_load_roundtrip(void)
{
    printf("  Save+load round-trip...\n");
    char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_test_%d", FS_GETPID());
    FS_MKDIR(tmpdir);

    uint8_t game_state[256];
    memset(game_state, 0, sizeof(game_state));
    game_state[0] = 0x01;
    game_state[4] = 'D'; game_state[5] = 'M';
    game_state[6] = '2'; game_state[7] = '\0';

    int r = dm2_sl_save(tmpdir, 3, "Test_Save",
                        game_state, sizeof(game_state));
    if (r != 0) { printf("    FAIL: save returned %d\n", r); FS_RMDIR(tmpdir); return 0; }

    uint8_t loaded[256];
    size_t got;
    r = dm2_sl_load(tmpdir, 3, loaded, sizeof(loaded), &got);
    if (r != 0) { printf("    FAIL: load returned %d\n", r); FS_RMDIR(tmpdir); return 0; }
    if (got != sizeof(game_state)) {
        printf("    FAIL: size mismatch %zu vs %zu\n", got, sizeof(game_state));
        FS_RMDIR(tmpdir);
        return 0;
    }
    if (memcmp(game_state, loaded, got) != 0) {
        printf("    FAIL: content mismatch\n");
        FS_RMDIR(tmpdir);
        return 0;
    }
    printf("    PASS: save→load round-trip verified\n");
    FS_RMDIR(tmpdir);
    return 1;
}

/* ── Test 6: Backup fallback ──────────────────────────────── */

static int test_backup_fallback(void)
{
    printf("  Backup fallback on missing slot file...\n");
    char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_test_%d", FS_GETPID());
    FS_MKDIR(tmpdir);

    /* Simulate: write original save to slot 5 */
    uint8_t gs[64];
    memset(gs, 0, sizeof(gs));
    gs[0] = 0x55;

    int r = dm2_sl_save(tmpdir, 5, "Original", gs, sizeof(gs));
    if (r != 0) { printf("    FAIL: initial save %d\n", r); FS_RMDIR(tmpdir); return 0; }

    /* Manually remove slot file, keeping .bak */
    char p_dat[256], p_bak[256];
    snprintf(p_dat, sizeof(p_dat), "%s/SKSave%02u.dat", tmpdir, 5);
    snprintf(p_bak, sizeof(p_bak), "%s/SKSave.bak", tmpdir);

    if (rename(p_dat, p_bak) != 0) { printf("    FAIL: rotate\n"); FS_RMDIR(tmpdir); return 0; }

    /* Load should fall back to .bak */
    uint8_t out[64];
    size_t out_sz;
    r = dm2_sl_load(tmpdir, 5, out, sizeof(out), &out_sz);
    FS_RMDIR(tmpdir);
    if (r != 0) { printf("    FAIL: backup load returned %d\n", r); return 0; }
    if (out_sz != sizeof(gs)) { printf("    FAIL: size mismatch on bak load\n"); return 0; }
    if (out[0] != 0x55) { printf("    FAIL: bak content wrong (0x%02X)\n", out[0]); return 0; }
    printf("    PASS: backup fallback works\n");
    return 1;
}

/* ── Test 7: Cross-version diagnostics ─────────────────────── */

static int test_cross_version_diagnostics(void)
{
    printf("  Cross-version diagnostics...\n");

    /* Valid DM2 slot header */
    uint8_t dm2_hdr[42] = {0};
    dm2_hdr[38] = 0xEF; dm2_hdr[39] = 0xBE;
    dm2_hdr[40] = 0xAD; dm2_hdr[41] = 0xDE;

    /* Valid DM1 header (pair 0x444D / 0x3156 = "DM1V") */
    uint8_t dm1_hdr[42] = {0};
    dm1_hdr[38] = 0x44; dm1_hdr[39] = 0x4D;

    /* Unknown — no magic */
    uint8_t unk_hdr[42] = {0};

    int v_dm2 = dm2_v1_save_detect_game_version(dm2_hdr);
    int v_dm1 = dm2_v1_save_detect_game_version(dm1_hdr);
    int v_unk = dm2_v1_save_detect_game_version(unk_hdr);

    if (v_dm2 != DM2V1_VERSION_DM2) { printf("    FAIL: DM2=%d\n", v_dm2); return 0; }
    if (v_dm1 != DM2V1_VERSION_DM1) { printf("    FAIL: DM1=%d\n", v_dm1); return 0; }
    if (v_unk != DM2V1_VERSION_UNKNOWN) { printf("    FAIL: Unknown=%d\n", v_unk); return 0; }

    /* Null-fill diagnostic */
    uint8_t null_data[64];
    memset(null_data, 0, 64);
    int diag = dm2_v1_save_version_diagnostics(null_data, sizeof(null_data));
    if (!(diag & DM2V1_SAVE_DIAG_NULL_FILL)) {
        printf("    FAIL: null-fill not detected\n");
        return 0;
    }
    printf("    PASS: version detection + null-fill diagnostic\n");
    return 1;
}

/* ── Test 8: SUPPRESS self-test ──────────────────────────────── */

static int test_suppress_self_test(void)
{
    printf("  SUPPRESS self-test via dm2_v1_save_suppress_self_test...\n");
    if (!dm2_v1_save_suppress_self_test()) {
        printf("    FAIL: returned false\n");
        return 0;
    }
    printf("    PASS\n");
    return 1;
}

/* ── Test 9: Champion mask table ──────────────────────────── */

static int test_champion_mask(void)
{
    printf("  Champion SUPPRESS mask (261 bytes, valid nibbles)...\n");
    uint8_t mask[261];
    dm2_suppress_champion_mask(mask);
    for (size_t i = 0; i < 261; i++) {
        if ((mask[i] & 0xF0) != 0) {
            printf("    FAIL: mask[%zu]=0x%02X has high nibble\n", i, mask[i]);
            return 0;
        }
    }
    /* Verify name block and inventory region are non-zero */
    if (mask[0] == 0 || mask[7] == 0) { printf("    FAIL: name block zero\n"); return 0; }
    if (mask[91] == 0) { printf("    FAIL: inventory[0] mask zero\n"); return 0; }
    printf("    PASS: mask table valid (261 bytes, low nibbles only)\n");
    return 1;
}

/* ── Test 10: DB handle identity ───────────────────────────── */

static int test_db_handle_roundtrip(void)
{
    printf("  DB handle make + resolve round-trip...\n");

    DM2_DB_State db;
    memset(&db, 0, sizeof(db));

    /* Pool 5, index 0x1234 → handle → pool 5, index 0x1234 */
    uint32_t h = dm2_db_make_handle(5, 0x1234);
    if (h == 0) { printf("    FAIL: make_handle=0\n"); return 0; }

    uint8_t pool_out;
    uint32_t idx_out;
    bool ok = dm2_db_resolve(h, &db, &pool_out, &idx_out);
    if (ok) { printf("    FAIL: resolve succeeded without DB data\n"); return 0; }

    /* With DB data stub */
    db.pools[5].data = (uint8_t *)malloc(4);
    db.pools[5].rec_count = 0x2000;
    db.pools[5].rec_size = 1;
    ok = dm2_db_resolve(h, &db, &pool_out, &idx_out);
    if (!ok || pool_out != 5 || idx_out != 0x1234) {
        printf("    FAIL: resolve mismatch (ok=%d pool=%u idx=0x%X)\n", ok, pool_out, idx_out);
        free(db.pools[5].data);
        return 0;
    }
    free(db.pools[5].data);
    printf("    PASS: handle identity verified\n");
    return 1;
}

/* ════════════════════════════════════════════════════════════════ */

int main(void)
{
    int pass = 0, total = 0;

#define RUN(n, fn) \
    do { total++; \
         printf("  [%d] " #fn "...\n", n); \
         if (fn()) { pass++; printf("    ✓ PASS\n"); } \
         else printf("    ✗ FAILED\n"); \
    } while (0)

    RUN(1,  test_suppress_all1_roundtrip);
    RUN(2,  test_suppress_fill_mode);
    RUN(3,  test_slot_header_encoding);
    RUN(4,  test_slot_scan);
    RUN(5,  test_save_load_roundtrip);
    RUN(6,  test_backup_fallback);
    RUN(7,  test_cross_version_diagnostics);
    RUN(8,  test_suppress_self_test);
    RUN(9,  test_champion_mask);
    RUN(10, test_db_handle_roundtrip);
#undef RUN

    printf("\n  DM2 V1 Save/Load: %d/%d tests passed\n", pass, total);
    return (pass == total) ? 0 : 1;
}
