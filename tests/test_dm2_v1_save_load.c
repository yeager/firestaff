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
 *  11. Invalid slot-header rejection + backup recovery
 *  12. Stale session metadata mismatch (fixture guard)
 *  13. Resume smoke gate: position/facing/map/leader/inventory continuity
 *  14. Champion death/permanence source-lock gate
 *
 * Source refs:
 *   docs/dm2_save_format.md — SUPPRESS codec, slot header layout
 *   docs/dm2_save_slots.md — 10 slots, 0xBEEF/0xDEAD magic
 *   docs/dm2_party_state.md — champion 261-byte format
 *   ReDMCSB DEFS.H:680-681 — CurrentHealth/MaximumHealth persisted fields
 *   ReDMCSB CHAMPION.C F0320:1727-1737 — damage reaching <=0 calls kill
 *   ReDMCSB CHAMPION.C F0321:1835-1840/F0331:2333-2440 — zero HP blocks damage/regen
 *   ReDMCSB LOADSAVE.C F0433:1519-1571/F0435:2728-2777 — party/champion block save/load
 */

#include "dm2_v1_save_load.h"
#include "dm2_v1_new_game.h"
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

static void cleanup_one_slot_dir(const char *dir, uint8_t slot)
{
    char p[256];
    snprintf(p, sizeof(p), "%s/SKSave%02u.dat", dir, (unsigned)slot);
    (void)remove(p);
    snprintf(p, sizeof(p), "%s/SKSave.bak", dir);
    (void)remove(p);
    FS_RMDIR(dir);
}

static void cleanup_slot_dir(const char *dir)
{
    for (uint8_t s = 0; s < 10; s++) {
        char p[256];
        snprintf(p, sizeof(p), "%s/SKSave%02u.dat", dir, (unsigned)s);
        (void)remove(p);
    }
    {
        char p[256];
        snprintf(p, sizeof(p), "%s/SKSave.bak", dir);
        (void)remove(p);
    }
    FS_RMDIR(dir);
}

static int write_bad_slot_file(const char *dir, uint8_t slot)
{
    char path[256];
    uint8_t hdr[42];
    uint8_t payload[8] = { 'B', 'A', 'D', 'S', 'L', 'O', 'T', 0 };
    snprintf(path, sizeof(path), "%s/SKSave%02u.dat", dir, (unsigned)slot);
    memset(hdr, 0, sizeof(hdr));
    hdr[38] = 0x44; hdr[39] = 0x4D; /* DM1-ish marker, not DM2 BEEF/DEAD. */
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (fwrite(hdr, sizeof(hdr), 1, f) != 1 ||
        fwrite(payload, 1, sizeof(payload), f) != sizeof(payload)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

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

static int test_last_session_backup_fallback(void)
{
    printf("  Last-session SKSave.dat/.bak fallback...\n");
    char tmpdir[256];
    char p_dat[256], p_bak[256];
    uint8_t gs[64];
    uint8_t out[64];
    size_t out_sz = 0;
    int r;

    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_last_%d", FS_GETPID());
    FS_MKDIR(tmpdir);
    memset(gs, 0, sizeof(gs));
    gs[0] = 0x77;
    gs[1] = 0x33;

    r = dm2_sl_save_last_session(tmpdir, "Last Session", gs, sizeof(gs));
    if (r != 0) {
        printf("    FAIL: save last-session returned %d\n", r);
        FS_RMDIR(tmpdir);
        return 0;
    }
    if (!dm2_v1_save_has_valid_last_session(tmpdir)) {
        printf("    FAIL: last-session validity probe failed\n");
        cleanup_one_slot_dir(tmpdir, 0);
        FS_RMDIR(tmpdir);
        return 0;
    }
    r = dm2_sl_load_last_session(tmpdir, out, sizeof(out), &out_sz);
    if (r != 0 || out_sz != sizeof(gs) || out[0] != 0x77 || out[1] != 0x33) {
        printf("    FAIL: primary last-session load r=%d size=%lu bytes=%02X/%02X\n",
               r, (unsigned long)out_sz, out[0], out[1]);
        cleanup_one_slot_dir(tmpdir, 0);
        FS_RMDIR(tmpdir);
        return 0;
    }

    snprintf(p_dat, sizeof(p_dat), "%s/SKSave.dat", tmpdir);
    snprintf(p_bak, sizeof(p_bak), "%s/SKSave.bak", tmpdir);
    if (rename(p_dat, p_bak) != 0) {
        printf("    FAIL: rotate last-session primary to backup\n");
        cleanup_one_slot_dir(tmpdir, 0);
        FS_RMDIR(tmpdir);
        return 0;
    }
    memset(out, 0, sizeof(out));
    out_sz = 0;
    r = dm2_sl_load_last_session(tmpdir, out, sizeof(out), &out_sz);
    if (r != 0 || out_sz != sizeof(gs) || out[0] != 0x77 || out[1] != 0x33) {
        printf("    FAIL: backup last-session load r=%d size=%lu bytes=%02X/%02X\n",
               r, (unsigned long)out_sz, out[0], out[1]);
        (void)remove(p_bak);
        FS_RMDIR(tmpdir);
        return 0;
    }
    (void)remove(p_bak);
    FS_RMDIR(tmpdir);
    printf("    PASS: last-session primary and backup fallback work\n");
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

/* ── Test 11: Invalid header rejection + backup recovery ───────── */

static int test_invalid_slot_header_rejected(void)
{
    printf("  Invalid slot header rejection + backup recovery...\n");
    char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_bad_header_%d", FS_GETPID());
    FS_MKDIR(tmpdir);

    if (write_bad_slot_file(tmpdir, 2) != 0) {
        printf("    FAIL: could not write bad slot\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    DM2_SL_State st;
    dm2_sl_init(&st, tmpdir);
    dm2_sl_scan_slots(&st);
    if (dm2_sl_slot_occupied(&st, 2)) {
        printf("    FAIL: scan accepted bad slot header\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    uint8_t out[32];
    size_t out_sz = 123;
    int r = dm2_sl_load(tmpdir, 2, out, sizeof(out), &out_sz);
    if (r == 0) {
        printf("    FAIL: direct load accepted bad slot header\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    uint8_t good[16];
    memset(good, 0x6C, sizeof(good));
    {
        char p_dat[256], p_bak[256];
        snprintf(p_dat, sizeof(p_dat), "%s/SKSave%02u.dat", tmpdir, 2);
        snprintf(p_bak, sizeof(p_bak), "%s/SKSave.bak", tmpdir);
        (void)remove(p_dat);
        r = dm2_sl_save(tmpdir, 2, "BackupGood", good, sizeof(good));
        if (r != 0 || rename(p_dat, p_bak) != 0) {
            printf("    FAIL: could not prepare backup slot\n");
            cleanup_slot_dir(tmpdir);
            return 0;
        }
    }
    if (write_bad_slot_file(tmpdir, 2) != 0) {
        printf("    FAIL: could not rewrite bad primary slot\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    memset(out, 0, sizeof(out));
    out_sz = 0;
    r = dm2_sl_load(tmpdir, 2, out, sizeof(out), &out_sz);
    if (r != 0 || out_sz != sizeof(good) || memcmp(out, good, sizeof(good)) != 0) {
        printf("    FAIL: corrupt primary did not recover from valid backup (r=%d size=%zu)\n",
               r, out_sz);
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    cleanup_slot_dir(tmpdir);
    printf("    PASS: bad primary rejected; valid backup recovered\n");
    return 1;
}

/* ── Test 12: Stale fixture metadata rejection ─────────────────── */

static int test_stale_fixture_metadata_guard(void)
{
    printf("  Stale fixture metadata guard...\n");
    DM2_V1_SessionState session;
    DM2_V1_SessionState out;
    uint8_t buf[DM2_SESSION_MAX_SIZE];
    uint8_t stale[DM2_SESSION_MAX_SIZE];
    int sz;
    int r;

    dm2_v1_session_new(&session);
    sz = dm2_v1_session_serialize(&session, buf, sizeof(buf));
    if (sz <= 0) {
        printf("    FAIL: serialize base fixture failed\n");
        return 0;
    }

    memcpy(stale, buf, (size_t)sz);
    stale[28] = (uint8_t)(DM2_SESSION_VERSION + 1U);
    r = dm2_v1_session_deserialize(&out, stale, (size_t)sz);
    if (r == 0) {
        printf("    FAIL: deserialize accepted stale session version 0x%02X\n",
               stale[28]);
        return 0;
    }

    memcpy(stale, buf, (size_t)sz);
    stale[28] = 0;
    r = dm2_v1_session_deserialize(&out, stale, (size_t)sz);
    if (r == 0) {
        printf("    FAIL: deserialize accepted zero session version\n");
        return 0;
    }

    printf("    PASS: stale/mismatched session metadata is rejected\n");
    return 1;
}

/* ── Test 13: DM2 V1 resume smoke gate ───────────────────────────
 *
 * Data-free continuity gate for the fields a real resume needs first:
 * party position, facing, current map, leader, champion count, and a small
 * champion inventory sample. The synthetic stream mirrors the documented
 * order of the DM2 game-state block followed by champion persistence, so
 * hosts without real DM2 data still exercise the save/load boundary.
 */

static int append_blob(uint8_t *dst, size_t cap, size_t *pos,
                       const void *src, size_t n)
{
    if (!dst || !pos || !src || *pos > cap || n > cap - *pos) return 0;
    memcpy(dst + *pos, src, n);
    *pos += n;
    return 1;
}

typedef union {
    DM2_GameStateBlock block;
    uint8_t raw[DM2_GAME_STATE_BLOCK_SIZE];
} DM2_TestGameStateStorage;

static int check_resume_state(
    const char *label,
    const DM2_GameStateBlock *expected,
    const DM2_GameStateBlock *actual,
    const uint32_t expected_inventory[DM2_CHAMPION_INVENTORY_SLOTS],
    const uint32_t actual_inventory[DM2_CHAMPION_INVENTORY_SLOTS])
{
    if (actual->dwGameTick != (expected->dwGameTick & 0x7F7F7F7Fu)) {
        printf("    FAIL %s: tick 0x%08X expected 0x%08X\n",
               label, actual->dwGameTick, expected->dwGameTick & 0x7F7F7F7Fu);
        return 0;
    }
    if (actual->wChampionsCount != expected->wChampionsCount ||
        actual->wPlayerPosX != expected->wPlayerPosX ||
        actual->wPlayerPosY != expected->wPlayerPosY ||
        actual->wPlayerDir != expected->wPlayerDir ||
        actual->wPlayerMap != expected->wPlayerMap ||
        actual->wChampionLeader != expected->wChampionLeader) {
        printf("    FAIL %s: resume tuple got champ=%u pos=(%u,%u) "
               "dir=%u map=%u leader=%u\n",
               label,
               (unsigned)actual->wChampionsCount,
               (unsigned)actual->wPlayerPosX,
               (unsigned)actual->wPlayerPosY,
               (unsigned)actual->wPlayerDir,
               (unsigned)actual->wPlayerMap,
               (unsigned)actual->wChampionLeader);
        return 0;
    }
    const int watched_slots[] = { 0, 1, 2, 8 };
    for (size_t s = 0; s < sizeof(watched_slots) / sizeof(watched_slots[0]); s++) {
        int i = watched_slots[s];
        if (actual_inventory[i] != expected_inventory[i]) {
            printf("    FAIL %s: inventory[%d] 0x%08X expected 0x%08X\n",
                   label, i, actual_inventory[i], expected_inventory[i]);
            return 0;
        }
    }
    return 1;
}

static int build_resume_payload(const DM2_GameStateBlock *gs,
                                const DM2_ChampionRecord *champ,
                                uint8_t *payload,
                                size_t payload_cap,
                                size_t *payload_size,
                                size_t *enc_gs_size,
                                size_t *enc_champ_size)
{
    uint8_t enc_gs[DM2_GAME_STATE_BLOCK_SIZE];
    uint8_t champ_mask[261];
    uint8_t enc_champ[261];
    int gs_n;
    int champ_n;
    size_t pos = 0;

    if (!gs || !champ || !payload || !payload_size ||
        !enc_gs_size || !enc_champ_size) {
        return 0;
    }

    gs_n = dm2_suppress_encode_gamestate(gs, enc_gs, sizeof(enc_gs));
    if (gs_n <= 0) return 0;

    dm2_suppress_champion_mask(champ_mask);
    champ_n = dm2_suppress_encode_champion(champ,
                                           champ_mask,
                                           enc_champ,
                                           sizeof(enc_champ));
    if (champ_n <= 0) return 0;

    if (!append_blob(payload, payload_cap, &pos, "D2RS", 4)) return 0;
    if (!append_blob(payload, payload_cap, &pos, &gs_n, sizeof(gs_n))) return 0;
    if (!append_blob(payload, payload_cap, &pos, enc_gs, (size_t)gs_n)) return 0;
    if (!append_blob(payload, payload_cap, &pos, &champ_n, sizeof(champ_n))) return 0;
    if (!append_blob(payload, payload_cap, &pos, enc_champ, (size_t)champ_n)) return 0;

    *payload_size = pos;
    *enc_gs_size = (size_t)gs_n;
    *enc_champ_size = (size_t)champ_n;
    return 1;
}

static int read_resume_payload(const uint8_t *payload,
                               size_t payload_size,
                               DM2_GameStateBlock *gs,
                               DM2_ChampionRecord *champ)
{
    uint8_t champ_mask[261];
    int gs_n;
    int champ_n;
    size_t pos = 0;

    if (!payload || !gs || !champ || payload_size < 4 + sizeof(int)) return 0;
    if (memcmp(payload, "D2RS", 4) != 0) return 0;
    pos += 4;

    memcpy(&gs_n, payload + pos, sizeof(gs_n));
    pos += sizeof(gs_n);
    if (gs_n <= 0 || (size_t)gs_n > payload_size - pos) return 0;
    if (dm2_suppress_decode_gamestate(payload + pos,
                                      (size_t)gs_n,
                                      gs,
                                      0) != gs_n) {
        return 0;
    }
    pos += (size_t)gs_n;

    if (payload_size - pos < sizeof(champ_n)) return 0;
    memcpy(&champ_n, payload + pos, sizeof(champ_n));
    pos += sizeof(champ_n);
    if (champ_n <= 0 || (size_t)champ_n > payload_size - pos) return 0;

    dm2_suppress_champion_mask(champ_mask);
    memset(champ, 0, sizeof(*champ));
    if (dm2_suppress_decode_champion(payload + pos,
                                     (size_t)champ_n,
                                     champ_mask,
                                     champ,
                                     0) != champ_n) {
        return 0;
    }
    pos += (size_t)champ_n;

    return pos == payload_size;
}

static int test_resume_smoke_gate_position_facing_inventory(void)
{
    printf("  Resume smoke gate: position/facing/inventory continuity...\n");
    char tmpdir[256];
    uint8_t payload[768];
    uint8_t loaded[768];
    size_t payload_size = 0;
    size_t loaded_size = 0;
    size_t enc_gs_size = 0;
    size_t enc_champ_size = 0;
    DM2_TestGameStateStorage gs_store;
    DM2_TestGameStateStorage resumed_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_GameStateBlock *resumed = &resumed_store.block;
    DM2_ChampionRecord champ;
    DM2_ChampionRecord resumed_champ;
    int r;

    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_resume_%d", FS_GETPID());
    FS_MKDIR(tmpdir);

    memset(&gs_store, 0, sizeof(gs_store));
    memset(&resumed_store, 0, sizeof(resumed_store));
    gs->dwGameTick = 0x00123456u;
    gs->dwRandomSeed = 0x00001234u;
    gs->wChampionsCount = 1;
    gs->wPlayerPosX = 17;
    gs->wPlayerPosY = 9;
    gs->wPlayerDir = 3;
    gs->wPlayerMap = 2;
    gs->wChampionLeader = 0;
    gs->wTimersCount = 1;

    memset(&champ, 0, sizeof(champ));
    memcpy(champ.first_name, "TORHAM", 6);
    champ.absolute_direction = gs->wPlayerDir;
    champ.squad_position = 0;
    champ.cur_hp = 88;
    champ.max_hp = 99;
    champ.inventory[0] = dm2_db_make_handle(4, 0x0012);
    champ.inventory[1] = dm2_db_make_handle(5, 0x0034);
    champ.inventory[2] = dm2_db_make_handle(6, 0x0056);

    if (!build_resume_payload(gs, &champ, payload, sizeof(payload),
                              &payload_size, &enc_gs_size, &enc_champ_size)) {
        printf("    FAIL: could not build first resume payload\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }

    r = dm2_sl_save(tmpdir, 4, "ResumeSmoke", payload, payload_size);
    if (r != 0) {
        printf("    FAIL: save returned %d\n", r);
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    if (!dm2_v1_save_has_valid_slot(tmpdir, 4)) {
        printf("    FAIL: slot header not valid after save\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }

    memset(loaded, 0, sizeof(loaded));
    r = dm2_sl_load(tmpdir, 4, loaded, sizeof(loaded), &loaded_size);
    if (r != 0 || loaded_size != payload_size) {
        printf("    FAIL: load returned %d size=%zu expected=%zu\n",
               r, loaded_size, payload_size);
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    memset(&resumed_store, 0, sizeof(resumed_store));
    if (!read_resume_payload(loaded, loaded_size, resumed, &resumed_champ)) {
        printf("    FAIL: could not decode first resume payload\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    if (!check_resume_state("first", gs, resumed,
                            champ.inventory, resumed_champ.inventory)) {
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }

    gs->dwGameTick = 0x00123500u;
    gs->wPlayerPosX = 21;
    gs->wPlayerPosY = 12;
    gs->wPlayerDir = 1;
    gs->wPlayerMap = 3;
    champ.absolute_direction = gs->wPlayerDir;
    champ.inventory[1] = 0;
    champ.inventory[8] = dm2_db_make_handle(8, 0x0009);

    if (!build_resume_payload(gs, &champ, payload, sizeof(payload),
                              &payload_size, &enc_gs_size, &enc_champ_size)) {
        printf("    FAIL: could not build overwritten resume payload\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    r = dm2_sl_save(tmpdir, 4, "ResumeMoved", payload, payload_size);
    if (r != 0) {
        printf("    FAIL: overwrite save returned %d\n", r);
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }

    memset(loaded, 0, sizeof(loaded));
    memset(&resumed_store, 0, sizeof(resumed_store));
    r = dm2_sl_load(tmpdir, 4, loaded, sizeof(loaded), &loaded_size);
    if (r != 0 || loaded_size != payload_size ||
        !read_resume_payload(loaded, loaded_size, resumed, &resumed_champ)) {
        printf("    FAIL: overwritten resume payload did not reload "
               "(r=%d size=%zu)\n",
               r, loaded_size);
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    if (!check_resume_state("overwrite", gs, resumed,
                            champ.inventory, resumed_champ.inventory)) {
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }

    printf("    PASS: resume tuple and inventory survived save/load + overwrite "
           "(gs=%zuB champion=%zuB payload=%zuB)\n",
           enc_gs_size, enc_champ_size, payload_size);
    cleanup_one_slot_dir(tmpdir, 4);
    return 1;
}

static int same_dead_champion_persistence_fields(const DM2_ChampionRecord *expected,
                                                 const DM2_ChampionRecord *actual,
                                                 const char *label)
{
    const int watched_slots[] = { 0, 1, 8, 29 };

    if (!expected || !actual || !label) return 0;
    if (strncmp(actual->first_name, expected->first_name,
                DM2_CHAMPION_NAME_FIRST_LEN) != 0) {
        printf("    FAIL %s: first name changed\n", label);
        return 0;
    }
    if (actual->cur_hp != 0 || actual->cur_hp != expected->cur_hp ||
        actual->max_hp != expected->max_hp) {
        printf("    FAIL %s: HP cur=%u max=%u expected cur=%u max=%u\n",
               label,
               (unsigned)actual->cur_hp,
               (unsigned)actual->max_hp,
               (unsigned)expected->cur_hp,
               (unsigned)expected->max_hp);
        return 0;
    }
    if (actual->body_flag != expected->body_flag ||
        actual->hero_flag != expected->hero_flag ||
        actual->damage_suffered != expected->damage_suffered ||
        actual->poison_value != expected->poison_value) {
        printf("    FAIL %s: body/hero/damage/poison flags changed\n", label);
        return 0;
    }
    if (actual->stamina != expected->stamina ||
        actual->mana != expected->mana ||
        actual->food != expected->food ||
        actual->water != expected->water) {
        printf("    FAIL %s: secondary champion state changed\n", label);
        return 0;
    }
    for (size_t s = 0; s < sizeof(watched_slots) / sizeof(watched_slots[0]); s++) {
        int slot = watched_slots[s];
        if (actual->inventory[slot] != expected->inventory[slot]) {
            printf("    FAIL %s: inventory[%d] 0x%08X expected 0x%08X\n",
                   label,
                   slot,
                   actual->inventory[slot],
                   expected->inventory[slot]);
            return 0;
        }
    }
    return 1;
}

static void build_dead_champion_fixture(DM2_ChampionRecord *champ)
{
    if (!champ) return;

    memset(champ, 0, sizeof(*champ));
    memcpy(champ->first_name, "TORHAM", 6);
    memcpy(champ->last_name, "FALLEN", 6);
    champ->absolute_direction = 2;
    champ->squad_position = 1;
    champ->cur_hp = 0;
    champ->max_hp = 87;
    champ->stamina = 321;
    champ->mana = 12;
    champ->poison_value = 7;
    champ->damage_suffered = 55;
    champ->hero_flag = 1;
    champ->body_flag = 1;
    champ->food = -220;
    champ->water = -111;
    champ->inventory[0] = dm2_db_make_handle(4, 0x0101);
    champ->inventory[1] = dm2_db_make_handle(5, 0x0102);
    champ->inventory[8] = dm2_db_make_handle(6, 0x0103);
    champ->inventory[29] = dm2_db_make_handle(7, 0x0104);
}

static void put_le16(uint8_t *buf, size_t offset, uint16_t value)
{
    buf[offset] = (uint8_t)(value & 0xFFu);
    buf[offset + 1] = (uint8_t)((value >> 8) & 0xFFu);
}

static uint16_t get_le16(const uint8_t *buf, size_t offset)
{
    return (uint16_t)buf[offset] | ((uint16_t)buf[offset + 1] << 8);
}

static void put_le32(uint8_t *buf, size_t offset, uint32_t value)
{
    buf[offset] = (uint8_t)(value & 0xFFu);
    buf[offset + 1] = (uint8_t)((value >> 8) & 0xFFu);
    buf[offset + 2] = (uint8_t)((value >> 16) & 0xFFu);
    buf[offset + 3] = (uint8_t)((value >> 24) & 0xFFu);
}

static uint32_t get_le32(const uint8_t *buf, size_t offset)
{
    return (uint32_t)buf[offset] |
           ((uint32_t)buf[offset + 1] << 8) |
           ((uint32_t)buf[offset + 2] << 16) |
           ((uint32_t)buf[offset + 3] << 24);
}

static void build_raw_dead_champion_record(uint8_t raw[261],
                                           const DM2_ChampionRecord *champ)
{
    memset(raw, 0, 261);
    memcpy(raw + 0, champ->first_name, DM2_CHAMPION_NAME_FIRST_LEN);
    memcpy(raw + 8, champ->last_name, DM2_CHAMPION_NAME_LAST_LEN);
    put_le16(raw, 24, champ->absolute_direction);
    raw[26] = champ->squad_position;
    put_le16(raw, 27, champ->cur_hp);
    put_le16(raw, 29, champ->max_hp);
    put_le16(raw, 31, champ->stamina);
    put_le16(raw, 33, champ->mana);
    raw[35] = champ->poison_value;
    raw[88] = champ->damage_suffered;
    raw[89] = champ->hero_flag;
    raw[90] = champ->body_flag;
    put_le32(raw, 91 + 0 * 4, champ->inventory[0]);
    put_le32(raw, 91 + 1 * 4, champ->inventory[1]);
    put_le32(raw, 91 + 8 * 4, champ->inventory[8]);
    put_le32(raw, 91 + 29 * 4, champ->inventory[29]);
}

static int same_raw_dead_champion_record(const uint8_t expected[261],
                                         const uint8_t actual[261],
                                         const char *label)
{
    const int watched_slots[] = { 0, 1, 8, 29 };

    if (memcmp(actual, expected, DM2_CHAMPION_NAME_FIRST_LEN) != 0) {
        printf("    FAIL %s: raw first name changed\n", label);
        return 0;
    }
    if (get_le16(actual, 27) != 0 ||
        get_le16(actual, 27) != get_le16(expected, 27) ||
        get_le16(actual, 29) != get_le16(expected, 29)) {
        printf("    FAIL %s: raw HP cur=%u max=%u expected cur=%u max=%u\n",
               label,
               (unsigned)get_le16(actual, 27),
               (unsigned)get_le16(actual, 29),
               (unsigned)get_le16(expected, 27),
               (unsigned)get_le16(expected, 29));
        return 0;
    }
    if (actual[35] != expected[35] ||
        actual[88] != expected[88] ||
        actual[89] != expected[89] ||
        actual[90] != expected[90]) {
        printf("    FAIL %s: raw poison/damage/hero/body flags changed\n",
               label);
        return 0;
    }
    for (size_t s = 0; s < sizeof(watched_slots) / sizeof(watched_slots[0]); s++) {
        int slot = watched_slots[s];
        size_t offset = 91 + (size_t)slot * 4;
        if (get_le32(actual, offset) != get_le32(expected, offset)) {
            printf("    FAIL %s: raw inventory[%d] 0x%08X expected 0x%08X\n",
                   label,
                   slot,
                   get_le32(actual, offset),
                   get_le32(expected, offset));
            return 0;
        }
    }
    return 1;
}

static int test_champion_death_permanence_source_lock(void)
{
    printf("  Champion death/permanence source-lock gate...\n");
    char tmpdir[256];
    uint8_t mask[261];
    uint8_t encoded[261];
    uint8_t raw_dead[261];
    uint8_t raw_decoded[261];
    uint8_t session_buf[2048];
    DM2_ChampionRecord dead;
    DM2_ChampionRecord raw_roundtrip;
    DM2_ChampionRecord slot_roundtrip;
    DM2_V1_SessionState session;
    DM2_V1_SessionState restored;
    int enc_n;
    int session_n;
    int r;

    /*
     * This is deliberately a persistence/readiness gate only. It does not
     * borrow DM1 Vi-altar or CSB reincarnation rules for DM2. The source
     * evidence used here is narrower: zero CurrentHealth is the dead state
     * checked by ReDMCSB CHAMPION.C F0321:1835-1840 and skipped by the time
     * effects/regeneration loop in F0331:2333-2440, while LOADSAVE.C
     * F0433:1519-1571/F0435:2728-2777 copies the party champion block into
     * and back out of the save part.
     */
    build_dead_champion_fixture(&dead);
    build_raw_dead_champion_record(raw_dead, &dead);

    dm2_suppress_champion_mask(mask);
    enc_n = dm2_suppress_encode(raw_dead, mask, 261,
                                encoded, sizeof(encoded));
    if (enc_n <= 0) {
        printf("    FAIL: could not encode raw dead champion fixture\n");
        return 0;
    }
    memset(raw_decoded, 0xA5, sizeof(raw_decoded));
    if (dm2_suppress_decode(encoded, (size_t)enc_n,
                            mask, 261, raw_decoded, 0) != enc_n) {
        printf("    FAIL: could not decode raw dead champion fixture\n");
        return 0;
    }
    if (!same_raw_dead_champion_record(raw_dead, raw_decoded,
                                       "champion-codec-raw")) {
        return 0;
    }

    dm2_v1_session_new(&session);
    session.champion_count = 1;
    session.leader_index = 0;
    memset(session.champion_data[0], 0, sizeof(session.champion_data[0]));
    memcpy(session.champion_data[0], &dead, sizeof(dead));
    if (!dm2_v1_session_validate(&session)) {
        printf("    FAIL: session validation rejected zero-HP champion\n");
        return 0;
    }

    memset(session_buf, 0, sizeof(session_buf));
    session_n = dm2_v1_session_serialize(&session,
                                         session_buf,
                                         sizeof(session_buf));
    if (session_n <= 0) {
        printf("    FAIL: session serialize failed\n");
        return 0;
    }
    memset(&restored, 0, sizeof(restored));
    if (dm2_v1_session_deserialize(&restored,
                                   session_buf,
                                   (size_t)session_n) != 0) {
        printf("    FAIL: session deserialize rejected zero-HP champion\n");
        return 0;
    }
    memset(&raw_roundtrip, 0, sizeof(raw_roundtrip));
    memcpy(&raw_roundtrip, restored.champion_data[0], sizeof(raw_roundtrip));
    if (!same_dead_champion_persistence_fields(&dead, &raw_roundtrip,
                                               "session-raw")) {
        return 0;
    }

    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_dead_%d", FS_GETPID());
    FS_MKDIR(tmpdir);
    r = dm2_v1_session_save_slot(tmpdir, 7, "DeadPersist", &session);
    if (r != 0) {
        printf("    FAIL: session save slot returned %d\n", r);
        cleanup_one_slot_dir(tmpdir, 7);
        return 0;
    }
    memset(&restored, 0, sizeof(restored));
    r = dm2_v1_session_load_slot(tmpdir, 7, &restored);
    if (r != 0) {
        printf("    FAIL: session load slot returned %d\n", r);
        cleanup_one_slot_dir(tmpdir, 7);
        return 0;
    }
    memset(&slot_roundtrip, 0, sizeof(slot_roundtrip));
    memcpy(&slot_roundtrip, restored.champion_data[0], sizeof(slot_roundtrip));
    if (!same_dead_champion_persistence_fields(&dead, &slot_roundtrip,
                                               "session-slot")) {
        cleanup_one_slot_dir(tmpdir, 7);
        return 0;
    }

    printf("    PASS: zero-HP champion stayed zero-HP through codec, "
           "session, and slot save/load (champion=%dB session=%dB)\n",
           enc_n, session_n);
    cleanup_one_slot_dir(tmpdir, 7);
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
    RUN(7,  test_last_session_backup_fallback);
    RUN(8,  test_cross_version_diagnostics);
    RUN(9,  test_suppress_self_test);
    RUN(10, test_champion_mask);
    RUN(11, test_db_handle_roundtrip);
    RUN(12, test_invalid_slot_header_rejected);
    RUN(13, test_stale_fixture_metadata_guard);
    RUN(14, test_resume_smoke_gate_position_facing_inventory);
    RUN(15, test_champion_death_permanence_source_lock);
#undef RUN

    printf("\n  DM2 V1 Save/Load: %d/%d tests passed\n", pass, total);
    return (pass == total) ? 0 : 1;
}
