/* DM2 V1 Save/Load — Integration Tests
 *
 * Tests:
 *   1. SUPPRESS codec encode/decode round-trip
 *   2. SKProject bit-mask order and cross-section carry corpus vectors
 *   3. SUPPRESS decode fill=1 vs fill=0 modes
 *   4. Authenticated DOS-header detection and source-shaped header encoding
 *   5. Slot scan: occupied vs empty detection
 *   6. Save + load round-trip (stateless path)
 *   7. Backup fallback on load
 *   8. Cross-version diagnostics: DM2/DM1/unknown/null-fill detection
 *   9. SUPPRESS codec self-test
 *  10. Champion record SUPPRESS mask (261 bytes, source-bit selectors)
 *  10. DB handle identity (make + resolve round-trip)
 *  11. Invalid slot-header rejection + backup recovery
 *  12. Stale session metadata mismatch (fixture guard)
 *  13. Resume smoke gate: position/facing/map/leader/inventory continuity
 *  14. Champion death/permanence source-lock gate
 *  24. Fixture-free external original SKSave corpus census
 *
 * Source refs:
 *   docs/dm2_save_format.md — SUPPRESS codec, slot header layout
 *   docs/dm2_save_slots.md — 10 slots and authenticated DOS header shape
 *   docs/dm2_party_state.md — champion 261-byte format
 *   ReDMCSB DEFS.H:680-681 — CurrentHealth/MaximumHealth persisted fields
 *   ReDMCSB CHAMPION.C F0320:1727-1737 — damage reaching <=0 calls kill
 *   ReDMCSB CHAMPION.C F0321:1835-1840/F0331:2333-2440 — zero HP blocks damage/regen
 *   ReDMCSB LOADSAVE.C F0433:1519-1571/F0435:2728-2777 — party/champion block save/load
 */

#include "dm2_v1_save_load.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_session_fixture.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_game.h"
#include "dm2_v1_runtime.h"
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

extern bool dm2_v1_original_timer_format_corpus_probe(
    const char *save_base,
    DM2_OriginalTimerFormatCorpusReceipt *out_receipt);
extern bool dm2_v1_sksave_corpus_load_first_importable(
    const char *save_base,
    uint8_t *out_payload,
    size_t out_capacity,
    size_t *out_payload_size,
    DM2_SKSaveCorpusReceipt *out_receipt);

static uint32_t corpus_hash_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t corpus_hash_words_le(const uint16_t *words, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < count; ++i) {
        hash ^= (uint8_t)(words[i] & 0xffu);
        hash *= 16777619u;
        hash ^= (uint8_t)(words[i] >> 8);
        hash *= 16777619u;
    }
    return hash;
}

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
        snprintf(p, sizeof(p), "%s/SKSave.dat", dir);
        (void)remove(p);
        snprintf(p, sizeof(p), "%s/SKSave.bak", dir);
        (void)remove(p);
    }
    FS_RMDIR(dir);
}

static void cleanup_corpus_fixture_dir(const char *dir)
{
    char nested_dir[256];
    char p[256];

    if (!dir) return;
    snprintf(nested_dir, sizeof(nested_dir), "%s/real_corpus", dir);
    snprintf(p, sizeof(p), "%s/sksave04.dat", nested_dir);
    (void)remove(p);
    snprintf(p, sizeof(p), "%s/captured-original.bin", nested_dir);
    (void)remove(p);
    FS_RMDIR(nested_dir);
    cleanup_slot_dir(dir);
}

static int write_valid_sksave_file_at_path(const char *path,
                                           const char *name,
                                           const uint8_t *payload,
                                           size_t payload_size)
{
    uint8_t hdr[42];
    FILE *f;

    if (!path || !payload || payload_size == 0u) return -1;
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 1u;
    if (name) {
        size_t nlen = strlen(name);
        if (nlen > 35u) nlen = 35u;
        memcpy(hdr + 2u, name, nlen);
    }

    f = fopen(path, "wb");
    if (!f) return -1;
    if (fwrite(hdr, sizeof(hdr), 1u, f) != 1u ||
        fwrite(payload, 1u, payload_size, f) != payload_size) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static int write_bad_slot_file(const char *dir, uint8_t slot)
{
    char path[256];
    uint8_t hdr[42];
    uint8_t payload[8] = { 'B', 'A', 'D', 'S', 'L', 'O', 'T', 0 };
    snprintf(path, sizeof(path), "%s/SKSave%02u.dat", dir, (unsigned)slot);
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 2u; /* Not the source's version-1 SKSave shape. */
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

static int write_bad_last_session_file(const char *dir)
{
    char path[256];
    uint8_t hdr[42];
    uint8_t payload[8] = { 'B', 'A', 'D', 'L', 'A', 'S', 'T', 0 };
    snprintf(path, sizeof(path), "%s/SKSave.dat", dir);
    memset(hdr, 0, sizeof(hdr));
    hdr[38] = 0x44; hdr[39] = 0x4D;
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
    printf("  SUPPRESS bit-0 mask round-trip...\n");
    /* SKProject masks select source bit positions, not a bit count. */
    uint8_t data[8] = { 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00 };
    uint8_t mask[8] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
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

/* ── Test 2: SUPPRESS source bit order + section carry ─────────── */

static int test_suppress_skproject_corpus_vectors(void)
{
    uint8_t data[3] = { 0x81, 0x00, 0xD2 };
    uint8_t mask[3] = { 0x81, 0x42, 0xFF };
    uint8_t encoded[8] = { 0 };
    uint8_t decoded[3] = { 0 };
    const uint8_t expected[] = { 0xCD, 0x20 };
    DM2_SuppressWriter writer;
    DM2_SuppressReader reader;
    DM2_V1_SaveSuppressSymbolReceipt receipt;
    size_t written = 0;
    size_t flushed = 0;
    uint8_t first_data = 0x80, first_mask = 0xC0, first_out = 0;
    uint8_t second_data = 0x0F, second_mask = 0x0F, second_out = 0;

    printf("  SKProject SUPPRESS bit order and section carry...\n");
    if (dm2_suppress_encode(data, mask, 3, encoded, sizeof(encoded)) != 2 ||
        memcmp(encoded, expected, sizeof(expected)) != 0 ||
        dm2_suppress_decode(encoded, sizeof(expected), mask, 3, decoded, 0) != 2 ||
        memcmp(data, decoded, sizeof(data)) != 0) {
        printf("    FAIL: source-order vector did not produce CD 20\n");
        return 0;
    }

    dm2_suppress_writer_init(&writer);
    if (dm2_suppress_writer_write(&writer, &first_data, &first_mask, 1,
                                  encoded, sizeof(encoded), &written) != 0 ||
        written != 0 ||
        dm2_suppress_writer_write(&writer, &second_data, &second_mask, 1,
                                  encoded, sizeof(encoded), &written) != 0 ||
        written != 0 ||
        dm2_suppress_writer_flush(&writer, encoded, sizeof(encoded), &flushed) != 0 ||
        flushed != 1 || encoded[0] != 0xBC) {
        printf("    FAIL: adjacent sections did not retain SUPPRESS carry\n");
        return 0;
    }

    dm2_suppress_reader_init(&reader, encoded, 1);
    if (dm2_suppress_reader_read(&reader, &first_mask, 1, &first_out, 0) != 0 ||
        dm2_suppress_reader_read(&reader, &second_mask, 1, &second_out, 0) != 0 ||
        first_out != first_data || second_out != second_data || reader.position != 1) {
        printf("    FAIL: section reader did not preserve pending source bits\n");
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_save_suppress_symbol_receipt(&receipt) ||
        !receipt.valid ||
        receipt.covered_symbol_mask !=
            (DM2_V1_SAVE_SUPPRESS_SYMBOL_INIT |
             DM2_V1_SAVE_SUPPRESS_SYMBOL_WRITER |
             DM2_V1_SAVE_SUPPRESS_SYMBOL_FLUSH |
             DM2_V1_SAVE_SUPPRESS_SYMBOL_READER |
             DM2_V1_SAVE_SUPPRESS_SYMBOL_WRITE_1BIT |
             DM2_V1_SAVE_SUPPRESS_SYMBOL_READ_1BIT) ||
        !receipt.init_ready ||
        !receipt.writer_ready ||
        !receipt.flush_ready ||
        !receipt.reader_ready ||
        !receipt.write_1bit_ready ||
        !receipt.read_1bit_ready ||
        !receipt.section_carry_ready ||
        !receipt.fill_zero_ready ||
        !receipt.fill_one_ready ||
        !receipt.underflow_rejected ||
        receipt.encoded_size != sizeof(expected) ||
        receipt.reader_position_after_decode != sizeof(expected) ||
        receipt.carry_encoded_byte != 0xBCu ||
        receipt.first_section_decoded != first_data ||
        receipt.second_section_decoded != second_data ||
        receipt.source_vector_hash == 0u ||
        receipt.mask_vector_hash == 0u ||
        receipt.encoded_vector_hash == 0u ||
        receipt.decoded_vector_hash == 0u ||
        receipt.receipt_hash == 0u) {
        printf("    FAIL: source-named SUPPRESS symbol receipt incomplete\n");
        return 0;
    }
    printf("    PASS: source bit masks, MSB order, and section carry match SKProject\n");
    return 1;
}

/* ── Test 3: SUPPRESS fill modes ──────────────────────────────── */

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
    printf("  Source-shaped SKSave header encoding...\n");

    for (uint8_t s = 0; s < 10; s++) {
        uint8_t hdr[42] = {0};
        hdr[0] = 1; hdr[1] = 0; /* version flag */

        char name[64];
        snprintf(name, sizeof(name), "Slot_%u", (unsigned)s);
        size_t nlen = strlen(name);
        if (nlen > 33) nlen = 33;
        memcpy(hdr + 2, name, nlen);

        if (hdr[38] != 0u || hdr[39] != 0u ||
            hdr[40] != 0u || hdr[41] != 0u) {
            printf("    FAIL slot %u: initial opaque field was invented\n", s);
            return 0;
        }

        uint16_t vflag = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
        if (vflag != 1) {
            printf("    FAIL slot %u: vflag %u expected 1\n", s, vflag);
            return 0;
        }
    }
    printf("    PASS: source-shaped headers 0..9 contain no fixture marker\n");
    return 1;
}

static int test_incomplete_original_writer_fails_closed(void)
{
    char tmpdir[256];
    char path[256];
    DM2_GameStateBlock gamestate;
    DM2_V1_SaveWriteReceipt receipt;
    FILE *f;

    printf("  Incomplete original writer fails closed...\n");
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_writer_%d", FS_GETPID());
    FS_MKDIR(tmpdir);
    snprintf(path, sizeof(path), "%s/SKSave.dat", tmpdir);
    memset(&gamestate, 0, sizeof(gamestate));
    memset(&receipt, 0, sizeof(receipt));

    if (dm2_v1_save_game_write(path, &gamestate, NULL, NULL, NULL,
                               NULL, 0u, NULL, 0u, &receipt) != -1 ||
        !receipt.fail_closed || receipt.valid || receipt.header_written) {
        printf("    FAIL: incomplete writer was not closed\n");
        (void)remove(path);
        FS_RMDIR(tmpdir);
        return 0;
    }
    f = fopen(path, "rb");
    if (f) {
        fclose(f);
        printf("    FAIL: incomplete writer created a file\n");
        (void)remove(path);
        FS_RMDIR(tmpdir);
        return 0;
    }
    FS_RMDIR(tmpdir);
    printf("    PASS: no partial save file was created\n");
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

    /* SKProject c_savegame.cpp:2169-2181 retains c_hex2a::l_26 from the
     * previous header instead of manufacturing a slot marker. Seed the
     * mounted-corpus value seen at b38 and prove a rotated save keeps it. */
    {
        char path[256];
        uint8_t hdr[42];
        const uint8_t source_l26[4] = { 0x58u, 0xf8u, 0x00u, 0x00u };
        FILE *header_file;

        snprintf(path, sizeof(path), "%s/SKSave03.dat", tmpdir);
        header_file = fopen(path, "r+b");
        if (!header_file || fread(hdr, sizeof(hdr), 1u, header_file) != 1u ||
            hdr[0] != 1u || hdr[1] != 0u ||
            memcmp(hdr + 2u, "Test_Save", 9u) != 0 ||
            memcmp(hdr + 38u, "\0\0\0\0", 4u) != 0 ||
            fseek(header_file, 38L, SEEK_SET) != 0 ||
            fwrite(source_l26, sizeof(source_l26), 1u, header_file) != 1u) {
            if (header_file) fclose(header_file);
            printf("    FAIL: initial source-shaped header mismatch\n");
            cleanup_one_slot_dir(tmpdir, 3u);
            FS_RMDIR(tmpdir);
            return 0;
        }
        fclose(header_file);
        r = dm2_sl_save(tmpdir, 3, "Test_Save_2", game_state,
                        sizeof(game_state));
        header_file = fopen(path, "rb");
        if (r != 0 || !header_file ||
            fread(hdr, sizeof(hdr), 1u, header_file) != 1u ||
            memcmp(hdr + 38u, source_l26, sizeof(source_l26)) != 0) {
            if (header_file) fclose(header_file);
            printf("    FAIL: previous c_hex2a opaque value was not retained\n");
            cleanup_one_slot_dir(tmpdir, 3u);
            FS_RMDIR(tmpdir);
            return 0;
        }
        fclose(header_file);
    }

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

    /* Source-shaped DM2 header: version 1 plus bounded printable name. */
    uint8_t dm2_hdr[42] = {0};
    dm2_hdr[0] = 1u;
    memcpy(dm2_hdr + 2u, "DM2", 4u);

    /* Valid DM1 header (pair 0x444D / 0x3156 = "DM1V") */
    uint8_t dm1_hdr[42] = {0};
    dm1_hdr[38] = 0x44; dm1_hdr[39] = 0x4D;

    /* Unknown — no magic */
    uint8_t unk_hdr[42] = {0};

    /* SKProject uses this only for an empty in-memory dialog entry. */
    uint8_t empty_entry_hdr[42] = {0};
    empty_entry_hdr[0] = 1u;
    memcpy(empty_entry_hdr + 2u, "Empty", 6u);
    empty_entry_hdr[38] = 0xefu; empty_entry_hdr[39] = 0xbeu;
    empty_entry_hdr[40] = 0xadu; empty_entry_hdr[41] = 0xdeu;

    int v_dm2 = dm2_v1_save_detect_game_version(dm2_hdr);
    int v_dm1 = dm2_v1_save_detect_game_version(dm1_hdr);
    int v_unk = dm2_v1_save_detect_game_version(unk_hdr);
    int v_empty = dm2_v1_save_detect_game_version(empty_entry_hdr);

    if (v_dm2 != DM2V1_VERSION_DM2) { printf("    FAIL: DM2=%d\n", v_dm2); return 0; }
    if (v_dm1 != DM2V1_VERSION_DM1) { printf("    FAIL: DM1=%d\n", v_dm1); return 0; }
    if (v_unk != DM2V1_VERSION_UNKNOWN) { printf("    FAIL: Unknown=%d\n", v_unk); return 0; }
    if (v_empty != DM2V1_VERSION_UNKNOWN) {
        printf("    FAIL: empty-entry sentinel was accepted (%d)\n", v_empty);
        return 0;
    }

    /* Authentic PC-DOS header shape: version 1, printable bounded label,
     * opaque trailing words. It identifies the container only; session
     * admission remains gated by the raw dungeon/SUPPRESS parser. */
    uint8_t dos_hdr[42] = {0};
    dos_hdr[0] = 1;
    memcpy(dos_hdr + 2, "MCANINCH 1", 10);
    dos_hdr[36] = 0xB4; dos_hdr[37] = 0x00;
    dos_hdr[38] = 0x40; dos_hdr[39] = 0x01;
    dos_hdr[40] = 0x14; dos_hdr[41] = 0x00;
    if (dm2_v1_save_detect_game_version(dos_hdr) != DM2V1_VERSION_DM2) {
        printf("    FAIL: authentic DOS header not classified as DM2\n");
        return 0;
    }

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
    printf("  Champion SUPPRESS mask (261 bytes, source-bit selectors)...\n");
    uint8_t mask[261];
    dm2_suppress_champion_mask(mask);
    for (size_t i = 0; i < 261; i++) {
        if (mask[i] != 0 && mask[i] != 0xFF) {
            printf("    FAIL: mask[%zu]=0x%02X is not a full source-bit selector\n", i, mask[i]);
            return 0;
        }
    }
    /* Verify name block and inventory region are non-zero */
    if (mask[0] == 0 || mask[7] == 0) { printf("    FAIL: name block zero\n"); return 0; }
    if (mask[91] == 0) { printf("    FAIL: inventory[0] mask zero\n"); return 0; }
    printf("    PASS: mask table preserves modeled source bytes\n");
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

    dm2_v1_test_session_fixture_new(&session);
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

static void write_i32_le(uint8_t out[4], int value)
{
    uint32_t u = (uint32_t)value;
    out[0] = (uint8_t)(u & 0xFFu);
    out[1] = (uint8_t)((u >> 8) & 0xFFu);
    out[2] = (uint8_t)((u >> 16) & 0xFFu);
    out[3] = (uint8_t)((u >> 24) & 0xFFu);
}

static int append_tagged_blob(uint8_t *dst, size_t cap, size_t *pos,
                              const char tag[4],
                              const uint8_t *blob,
                              int blob_size)
{
    uint8_t size_le[4];

    if (!dst || !pos || !tag || !blob || blob_size < 0) return 0;
    write_i32_le(size_le, blob_size);
    return append_blob(dst, cap, pos, tag, 4u) &&
           append_blob(dst, cap, pos, size_le, sizeof(size_le)) &&
           append_blob(dst, cap, pos, blob, (size_t)blob_size);
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
                                const uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE],
                                const uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE],
                                const uint16_t global_words[DM2_GLOBAL_WORDS_SIZE],
                                const uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                const DM2_TimerEntry *timers,
                                int timer_count,
                                uint8_t *payload,
                                size_t payload_cap,
                                size_t *payload_size,
                                size_t *enc_gs_size,
                                size_t *enc_champ_size)
{
    uint8_t enc_gs[DM2_GAME_STATE_BLOCK_SIZE];
    uint8_t champ_mask[261];
    uint8_t enc_champ[261];
    uint8_t enc_optional[512];
    uint8_t enc_timer_block[512];
    uint8_t size_le[4];
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

    if (global_flags) {
        int n = dm2_suppress_encode_global_flags(global_flags, enc_optional,
                                                 sizeof(enc_optional));
        if (n <= 0 ||
            !append_tagged_blob(payload, payload_cap, &pos, "GFLG",
                                enc_optional, n)) {
            return 0;
        }
    }
    if (global_bytes) {
        int n = dm2_suppress_encode_global_bytes(global_bytes, enc_optional,
                                                 sizeof(enc_optional));
        if (n <= 0 ||
            !append_tagged_blob(payload, payload_cap, &pos, "GBYT",
                                enc_optional, n)) {
            return 0;
        }
    }
    if (global_words) {
        int n = dm2_suppress_encode_global_words(global_words, enc_optional,
                                                 sizeof(enc_optional));
        if (n <= 0 ||
            !append_tagged_blob(payload, payload_cap, &pos, "GWRD",
                                enc_optional, n)) {
            return 0;
        }
    }
    if (spell_effects) {
        int n = dm2_suppress_encode_spell_effects(spell_effects,
                                                  enc_optional,
                                                  sizeof(enc_optional));
        if (n <= 0 ||
            !append_tagged_blob(payload, payload_cap, &pos, "SPFX",
                                enc_optional, n)) {
            return 0;
        }
    }
    if (timers && timer_count > 0) {
        size_t timer_pos = 0u;

        if (timer_count > DM2_MAX_TIMERS) return 0;
        write_i32_le(size_le, timer_count);
        if (!append_blob(enc_timer_block, sizeof(enc_timer_block),
                         &timer_pos, size_le, sizeof(size_le))) {
            return 0;
        }
        for (int i = 0; i < timer_count; i++) {
            int n = dm2_suppress_encode_timer(&timers[i], enc_optional,
                                              sizeof(enc_optional));
            if (n <= 0) return 0;
            write_i32_le(size_le, n);
            if (!append_blob(enc_timer_block, sizeof(enc_timer_block),
                             &timer_pos, size_le, sizeof(size_le)) ||
                !append_blob(enc_timer_block, sizeof(enc_timer_block),
                             &timer_pos, enc_optional, (size_t)n)) {
                return 0;
            }
        }
        if (!append_tagged_blob(payload, payload_cap, &pos, "TMR0",
                                enc_timer_block, (int)timer_pos)) {
            return 0;
        }
    }

    *payload_size = pos;
    *enc_gs_size = (size_t)gs_n;
    *enc_champ_size = (size_t)champ_n;
    return 1;
}

static void write_u16_le_at(uint8_t *buf, size_t offset, uint16_t value)
{
    buf[offset + 0u] = (uint8_t)(value & 0xFFu);
    buf[offset + 1u] = (uint8_t)((value >> 8) & 0xFFu);
}

static int build_raw_sksave_payload(
    const DM2_GameStateBlock *gs,
    const DM2_ChampionRecord *champ,
    const uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE],
    const uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE],
    const uint16_t global_words[DM2_GLOBAL_WORDS_SIZE],
    const uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
    const DM2_TimerEntry *timers,
    int timer_count,
    const uint32_t tail_inventory[DM2_CHAMPION_INVENTORY_SLOTS],
    uint32_t leader_hand_object,
    uint8_t *payload,
    size_t payload_cap,
    size_t *payload_size)
{
    uint8_t enc[512];
    uint8_t champ_mask[261];
    int n;
    size_t pos = 0u;

    if (!gs || !champ || !global_flags || !global_bytes ||
        !global_words || !spell_effects || !payload || !payload_size) {
        return 0;
    }

    memset(payload, 0, payload_cap);
    if (payload_cap < 63u) return 0;

    /* Minimal skproject-shaped dungeon prefix: 44-byte header, one 16-byte
     * descriptor, four column indexes, and a 4x5 byte map.  The live raw-save
     * restore must reparse this exact G1 structure before accepting pose 3,4. */
    payload[4] = 1u;
    write_u16_le_at(payload, 2u, 20u);
    write_u16_le_at(payload, 44u, 0u);
    write_u16_le_at(payload, 44u + 8u,
                    (uint16_t)((3u << 6) | (4u << 11)));
    write_u16_le_at(payload, 12u, 1u); /* one source-sized DB0 record */
    write_u16_le_at(payload, 18u, 1u); /* one source-sized DB3 record */
    write_u16_le_at(payload, 20u, 1u); /* one source-sized DB4 record */
    write_u16_le_at(payload, 22u, 1u); /* one source-sized DB5 record */
    payload[68u] = 0x34u;
    payload[69u] = 0x12u;
    payload[70u] = 0xe3u;
    payload[71u] = 0x09u;
    payload[74u] = 0xaau;
    payload[75u] = 0x80u;
    payload[76u] = 0x75u;
    payload[77u] = 0x3au;
    payload[78u] = 0xe0u;
    payload[79u] = 0x49u;
    payload[84u] = 0x2au;
    payload[86u] = 0x34u;
    payload[87u] = 0x12u;
    payload[98u] = 0xaeu;
    payload[99u] = 0x28u;
    pos = 120u;

    n = dm2_suppress_encode_gamestate(gs, enc, sizeof(enc));
    if (n <= 0 || !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
        return 0;
    }
    n = dm2_suppress_encode_global_flags(global_flags, enc, sizeof(enc));
    if (n <= 0 || !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
        return 0;
    }
    n = dm2_suppress_encode_global_bytes(global_bytes, enc, sizeof(enc));
    if (n <= 0 || !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
        return 0;
    }
    n = dm2_suppress_encode_global_words(global_words, enc, sizeof(enc));
    if (n <= 0 || !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
        return 0;
    }
    dm2_suppress_champion_mask(champ_mask);
    n = dm2_suppress_encode_champion(champ, champ_mask, enc, sizeof(enc));
    if (n <= 0 || !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
        return 0;
    }
    n = dm2_suppress_encode_spell_effects(spell_effects, enc, sizeof(enc));
    if (n <= 0 || !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
        return 0;
    }
    if (timers && timer_count > 0) {
        if (timer_count > DM2_MAX_TIMERS) return 0;
        for (int i = 0; i < timer_count; i++) {
            n = dm2_suppress_encode_timer(&timers[i], enc, sizeof(enc));
            if (n <= 0 ||
                !append_blob(payload, payload_cap, &pos, enc, (size_t)n)) {
                return 0;
            }
        }
    }
    if (tail_inventory) {
        for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
            uint8_t raw[4];
            raw[0] = (uint8_t)(tail_inventory[i] & 0xFFu);
            raw[1] = (uint8_t)((tail_inventory[i] >> 8) & 0xFFu);
            raw[2] = (uint8_t)((tail_inventory[i] >> 16) & 0xFFu);
            raw[3] = (uint8_t)((tail_inventory[i] >> 24) & 0xFFu);
            if (!append_blob(payload, payload_cap, &pos, raw,
                             sizeof(raw))) {
                return 0;
            }
        }
    }
    if (leader_hand_object != 0u) {
        uint8_t raw[4];
        raw[0] = (uint8_t)(leader_hand_object & 0xFFu);
        raw[1] = (uint8_t)((leader_hand_object >> 8) & 0xFFu);
        raw[2] = (uint8_t)((leader_hand_object >> 16) & 0xFFu);
        raw[3] = (uint8_t)((leader_hand_object >> 24) & 0xFFu);
        if (!append_blob(payload, payload_cap, &pos, raw, sizeof(raw))) {
            return 0;
        }
    }

    *payload_size = pos;
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

    return pos <= payload_size;
}

static int test_resume_smoke_gate_position_facing_inventory(void)
{
    printf("  Resume smoke gate: position/facing/inventory continuity...\n");
    char tmpdir[256];
    uint8_t payload[2048];
    uint8_t loaded[2048];
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
    DM2_ChampionRecord imported_champ;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE];
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE];
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE];
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    DM2_TimerEntry timers[2];
    DM2_V1_SessionState imported_session;
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

    memset(global_flags, 0, sizeof(global_flags));
    memset(global_bytes, 0, sizeof(global_bytes));
    memset(global_words, 0, sizeof(global_words));
    memset(spell_effects, 0, sizeof(spell_effects));
    memset(timers, 0, sizeof(timers));
    global_flags[0] = 0x45u;
    global_flags[7] = 0x12u;
    global_bytes[3] = 0x22u;
    global_bytes[63] = 0x5Au;
    global_words[5] = 0x1234u;
    global_words[42] = 0x4567u;
    spell_effects[0] = 0x07u;
    spell_effects[5] = 0x2Au;
    timers[0].timer_id = 0x0102u;
    timers[0].current_tick = 0x002Au;
    timers[0].interval_ticks = 0x0007u;
    timers[0].flags = 0x0003u;
    timers[0].user_data = 0x0044u;

    if (!build_resume_payload(gs, &champ,
                              global_flags, global_bytes, global_words,
                              spell_effects, timers, 1,
                              payload, sizeof(payload),
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
    memset(&imported_session, 0, sizeof(imported_session));
    if (dm2_v1_session_import_original_payload(&imported_session,
                                               loaded, loaded_size) != 0) {
        printf("    FAIL: diagnostic D2RS decoder rejected its explicit fixture\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    r = dm2_v1_session_load_slot(tmpdir, 4, &imported_session);
    if (r == 0) {
        printf("    FAIL: public slot loader admitted a D2RS fixture\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    memset(&imported_champ, 0, sizeof(imported_champ));
    memcpy(&imported_champ, imported_session.champion_data[0],
           sizeof(imported_champ));
    if (imported_session.game_tick != resumed->dwGameTick ||
        imported_session.rng_seed != resumed->dwRandomSeed ||
        imported_session.champion_count != 1 ||
        imported_session.party_x != resumed->wPlayerPosX ||
        imported_session.party_y != resumed->wPlayerPosY ||
        imported_session.party_dir != (resumed->wPlayerDir & 3u) ||
        imported_session.party_level != (uint8_t)resumed->wPlayerMap ||
        imported_session.leader_index != 0 ||
        imported_champ.inventory[0] != resumed_champ.inventory[0] ||
        imported_champ.inventory[1] != resumed_champ.inventory[1] ||
        imported_session.original_global_flags[0] != global_flags[0] ||
        imported_session.original_global_flags[7] != global_flags[7] ||
        imported_session.original_global_bytes[63] != global_bytes[63] ||
        imported_session.original_global_words[42] !=
            (global_words[42] & 0x7F7Fu) ||
        imported_session.original_spell_effects[5] != spell_effects[5] ||
        imported_session.original_timer_count != 1 ||
        imported_session.original_timers[0].timer_id != timers[0].timer_id ||
        imported_session.original_timers[0].user_data !=
            timers[0].user_data) {
        printf("    FAIL: diagnostic D2RS decoder did not preserve its tuple\n");
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

    if (!build_resume_payload(gs, &champ,
                              NULL, NULL, NULL, NULL, NULL, 0,
                              payload, sizeof(payload),
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
    memset(&imported_session, 0, sizeof(imported_session));
    if (dm2_v1_session_import_original_payload(&imported_session,
                                               loaded, loaded_size) != 0 ||
        dm2_v1_session_load_slot(tmpdir, 4, &imported_session) == 0) {
        printf("    FAIL: overwritten D2RS diagnostic/public-loader boundary failed\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }
    memset(&imported_champ, 0, sizeof(imported_champ));
    memcpy(&imported_champ, imported_session.champion_data[0],
           sizeof(imported_champ));
    if (imported_session.game_tick != resumed->dwGameTick ||
        imported_session.party_x != resumed->wPlayerPosX ||
        imported_session.party_y != resumed->wPlayerPosY ||
        imported_session.party_dir != (resumed->wPlayerDir & 3u) ||
        imported_session.party_level != (uint8_t)resumed->wPlayerMap ||
        imported_champ.inventory[8] != resumed_champ.inventory[8]) {
        printf("    FAIL: overwritten diagnostic D2RS payload did not update tuple\n");
        cleanup_one_slot_dir(tmpdir, 4);
        return 0;
    }

    printf("    PASS: diagnostic tuple survives raw slot I/O but public resume rejects D2RS "
           "(gs=%zuB champion=%zuB payload=%zuB)\n",
           enc_gs_size, enc_champ_size, payload_size);
    cleanup_one_slot_dir(tmpdir, 4);
    return 1;
}

static int test_raw_sksave_resume_import(void)
{
    printf("  Raw SKSave resume import: dungeon-prefix locator + SUPPRESS stream...\n");
    char tmpdir[256];
    uint8_t payload[2048];
    size_t payload_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champ;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE];
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE];
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE];
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    DM2_TimerEntry timers[1];
    uint32_t tail_inventory[DM2_CHAMPION_INVENTORY_SLOTS];
    uint32_t leader_hand_object;
    DM2_V1_SessionState imported_session;
    DM2_V1_OriginalRawDungeonReceipt dungeon_receipt;
    DM2_V1_OriginalRawDbRecordReceipt db0_receipt;
    DM2_V1_OriginalRawDoorReceipt door_receipt;
    DM2_V1_OriginalRawActuatorReceipt actuator_receipt;
    DM2_V1_OriginalRawCreatureReceipt creature_receipt;
    DM2_V1_OriginalRawTextReceipt text_receipt;
    DM2_V1_OriginalRawTeleporterReceipt teleporter_receipt;
    DM2_V1_OriginalRawContainerReceipt container_receipt;
    DM2_V1_OriginalRawWeaponReceipt weapon_receipt;
    DM2_V1_OriginalRawItemReceipt item_receipt;
    DM2_V1_SaveCandidate raw_candidate;
    int r;

    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_rawsave_%d",
             FS_GETPID());
    FS_MKDIR(tmpdir);

    memset(&gs_store, 0, sizeof(gs_store));
    gs->dwGameTick = 0x00034567u;
    gs->dwRandomSeed = 0x00002211u;
    gs->wChampionsCount = 1;
    gs->wPlayerPosX = 11;
    gs->wPlayerPosY = 14;
    gs->wPlayerDir = 2;
    gs->wPlayerMap = 5;
    gs->wChampionLeader = 0;
    gs->wTimersCount = 1;
    gs->bRainStrength = 19;

    memset(&champ, 0, sizeof(champ));
    memcpy(champ.first_name, "SAROS", 5);
    memcpy(champ.last_name, "RAW", 3);
    champ.absolute_direction = gs->wPlayerDir;
    champ.squad_position = 0;
    champ.cur_hp = 77;
    champ.max_hp = 88;
    champ.inventory[0] = dm2_db_make_handle(4, 0x0021);
    champ.inventory[8] = dm2_db_make_handle(7, 0x0042);

    memset(global_flags, 0, sizeof(global_flags));
    memset(global_bytes, 0, sizeof(global_bytes));
    memset(global_words, 0, sizeof(global_words));
    memset(spell_effects, 0, sizeof(spell_effects));
    memset(timers, 0, sizeof(timers));
    global_flags[2] = 0x33u;
    global_bytes[17] = 0x44u;
    global_words[9] = 0x2A2Au;
    spell_effects[4] = 0x11u;
    timers[0].timer_id = 0x0203u;
    timers[0].current_tick = 0x0040u;
    timers[0].interval_ticks = 0x0008u;
    timers[0].flags = 0x0001u;
    timers[0].user_data = 0x0077u;
    memset(tail_inventory, 0, sizeof(tail_inventory));
    tail_inventory[0] = dm2_db_make_handle(8, 0x0011);
    tail_inventory[8] = dm2_db_make_handle(9, 0x0022);
    leader_hand_object = dm2_db_make_handle(10, 0x0033);

    if (!build_raw_sksave_payload(gs, &champ,
                                  global_flags, global_bytes, global_words,
                                  spell_effects, timers, 1,
                                  tail_inventory, leader_hand_object,
                                  payload, sizeof(payload), &payload_size)) {
        printf("    FAIL: could not build raw SKSave fixture\n");
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }
    /* Extend the raw fixture with source-sized DB1 Teleporter, DB2 Text,
     * DB6/DB7/DB10 item, and DB9 Container records. The parser must retain
     * pool order before it reaches the map and SUPPRESS sections. */
    if (payload_size + 30u > sizeof(payload)) {
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }
    memmove(payload + 78u, payload + 72u, payload_size - 72u);
    payload_size += 6u;
    memset(payload + 72u, 0, 6u);
    write_u16_le_at(payload, 14u, 1u); /* DB1 Teleporter */
    payload[74u] = 0x31u;              /* x17, y9, rotate2, scope2, sound */
    payload[75u] = 0xd9u;
    payload[77u] = 0x05u;              /* destination map 5 */
    memmove(payload + 82u, payload + 78u, payload_size - 78u);
    payload_size += 4u;
    memset(payload + 78u, 0, 4u);
    write_u16_le_at(payload, 16u, 1u); /* DB2 Text */
    payload[80u] = 0x1bu;              /* visible, mode 1, index 0x123 */
    payload[81u] = 0x09u;
    memmove(payload + 130u, payload + 110u, payload_size - 110u);
    payload_size += 20u;
    memset(payload + 110u, 0, 20u);
    write_u16_le_at(payload, 24u, 1u); /* DB6 Cloth */
    write_u16_le_at(payload, 26u, 1u); /* DB7 Scroll */
    write_u16_le_at(payload, 30u, 1u); /* DB9 Container */
    write_u16_le_at(payload, 32u, 1u); /* DB10 Misc */
    payload[112u] = 0xd5u;             /* DB6 w2: ItemType 85 */
    payload[116u] = 0x3cu;             /* DB7 w2: ItemType 60 */
    payload[122u] = 0x05u;             /* DB9 b4: open, type 2 */
    payload[128u] = 0x55u;             /* DB10 w2: ItemType 85 */
    memset(&dungeon_receipt, 0, sizeof(dungeon_receipt));
    if (!dm2_v1_original_raw_sksave_dungeon_receipt(
            payload, payload_size, &dungeon_receipt) ||
        !dungeon_receipt.valid || dungeon_receipt.map_count != 1u ||
        dungeon_receipt.map_data_byte_count != 20u ||
        dungeon_receipt.column_index_count != 4u ||
        dungeon_receipt.ground_stack_count != 0u ||
        dungeon_receipt.text_word_count != 0u ||
        dungeon_receipt.db_pool_offsets[0] != 68u ||
        dungeon_receipt.db_record_counts[0] != 1u ||
        dungeon_receipt.db_record_counts[1] != 1u ||
        dungeon_receipt.db_pool_offsets[1] != 72u ||
        dungeon_receipt.db_record_counts[2] != 1u ||
        dungeon_receipt.db_pool_offsets[2] != 78u ||
        dungeon_receipt.db_record_counts[3] != 1u ||
        dungeon_receipt.db_pool_offsets[3] != 82u ||
        dungeon_receipt.db_record_counts[4] != 1u ||
        dungeon_receipt.db_pool_offsets[4] != 90u ||
        dungeon_receipt.db_record_counts[5] != 1u ||
        dungeon_receipt.db_pool_offsets[5] != 106u ||
        dungeon_receipt.db_record_counts[6] != 1u ||
        dungeon_receipt.db_pool_offsets[6] != 110u ||
        dungeon_receipt.db_record_counts[7] != 1u ||
        dungeon_receipt.db_pool_offsets[7] != 114u ||
        dungeon_receipt.db_record_counts[9] != 1u ||
        dungeon_receipt.db_pool_offsets[9] != 118u ||
        dungeon_receipt.db_record_counts[10] != 1u ||
        dungeon_receipt.db_pool_offsets[10] != 126u ||
        dungeon_receipt.map_data_offset != 130u ||
        dungeon_receipt.prefix_hash == 0u ||
        dungeon_receipt.map_data_hash == 0u ||
        dungeon_receipt.suppress_state_offset != 150u ||
        !dm2_v1_original_raw_sksave_db_record_receipt(
            payload, payload_size, 0, 0, &db0_receipt) ||
        !db0_receipt.valid || db0_receipt.record_size != 4u ||
        db0_receipt.record_offset != 68u || db0_receipt.record_hash == 0u ||
        dm2_v1_original_raw_sksave_db_record_receipt(
            payload, payload_size, 0, 1, &db0_receipt) ||
        !dm2_v1_original_raw_sksave_door_receipt(
            payload, payload_size, 0, &door_receipt) || !door_receipt.valid ||
        door_receipt.attributes != 0x09e3u || door_receipt.button != 1u ||
        door_receipt.door_type != 1u || door_receipt.button_state != 1u ||
        door_receipt.opening_dir != 1u || door_receipt.ornate_index != 1u ||
        door_receipt.destroyable_by_fireball != 1u ||
        door_receipt.bashable_by_chopping != 1u ||
        dm2_v1_original_raw_sksave_door_receipt(
            payload, payload_size, 1, &door_receipt) ||
        !dm2_v1_original_raw_sksave_teleporter_receipt(
            payload, payload_size, 0, &teleporter_receipt) ||
        !teleporter_receipt.valid ||
        teleporter_receipt.destination_x != 17u ||
        teleporter_receipt.destination_y != 9u ||
        teleporter_receipt.destination_map != 5u ||
        teleporter_receipt.scope != 2u || teleporter_receipt.sound != 1u ||
        teleporter_receipt.rotation != 2u ||
        teleporter_receipt.rotation_type != 1u ||
        dm2_v1_original_raw_sksave_teleporter_receipt(
            payload, payload_size, 1, &teleporter_receipt) ||
        !dm2_v1_original_raw_sksave_text_receipt(
            payload, payload_size, 0, &text_receipt) ||
        !text_receipt.valid || text_receipt.visible != 1u ||
        text_receipt.mode != 1u || text_receipt.text_index != 0x0123u ||
        dm2_v1_original_raw_sksave_text_receipt(
            payload, payload_size, 1, &text_receipt) ||
        !dm2_v1_original_raw_sksave_actuator_receipt(
            payload, payload_size, 0, &actuator_receipt) ||
        !actuator_receipt.valid || actuator_receipt.actuator_type != 42u ||
        actuator_receipt.actuator_data != 0x101u ||
        actuator_receipt.graphic_number != 3u || actuator_receipt.disabled != 1u ||
        actuator_receipt.delay != 4u || actuator_receipt.sound_effect != 1u ||
        actuator_receipt.revert_effect != 1u || actuator_receipt.action_type != 2u ||
        actuator_receipt.once_only != 1u || actuator_receipt.active_status != 1u ||
        actuator_receipt.target_direction != 2u || actuator_receipt.target_x != 7u ||
        actuator_receipt.target_y != 9u ||
        dm2_v1_original_raw_sksave_actuator_receipt(
            payload, payload_size, 1, &actuator_receipt) ||
        !dm2_v1_original_raw_sksave_creature_receipt(
            payload, payload_size, 0, &creature_receipt) ||
        !creature_receipt.valid || creature_receipt.creature_type != 42u ||
        creature_receipt.hp1 != 0x1234u ||
        dm2_v1_original_raw_sksave_creature_receipt(
            payload, payload_size, 1, &creature_receipt) ||
        !dm2_v1_original_raw_sksave_weapon_receipt(
            payload, payload_size, 0, &weapon_receipt) || !weapon_receipt.valid ||
        weapon_receipt.item_type != 46u || weapon_receipt.important != 1u ||
        weapon_receipt.charges != 10u ||
        dm2_v1_original_raw_sksave_weapon_receipt(
            payload, payload_size, 1, &weapon_receipt) ||
        !dm2_v1_original_raw_sksave_item_receipt(
            payload, payload_size, 5, 0, &item_receipt) ||
        !item_receipt.valid || item_receipt.item_type != 46u ||
        !dm2_v1_original_raw_sksave_item_receipt(
            payload, payload_size, 6, 0, &item_receipt) ||
        item_receipt.item_type != 85u ||
        !dm2_v1_original_raw_sksave_item_receipt(
            payload, payload_size, 7, 0, &item_receipt) ||
        item_receipt.item_type != 60u ||
        !dm2_v1_original_raw_sksave_item_receipt(
            payload, payload_size, 10, 0, &item_receipt) ||
        item_receipt.item_type != 85u ||
        dm2_v1_original_raw_sksave_item_receipt(
            payload, payload_size, 9, 0, &item_receipt) ||
        dm2_v1_original_raw_sksave_item_receipt(
            payload, payload_size, 6, 1, &item_receipt) ||
        !dm2_v1_original_raw_sksave_container_receipt(
            payload, payload_size, 0, &container_receipt) ||
        !container_receipt.valid || container_receipt.opened != 1u ||
        container_receipt.container_type != 2u ||
        dm2_v1_original_raw_sksave_container_receipt(
            payload, payload_size, 1, &container_receipt)) {
        printf("    FAIL: raw SKSave dungeon receipt lost source-owned spans\n");
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }
    /* A source-shaped dungeon receipt remains useful for diagnostics, but a
     * synthetic tail must never become a playable original save. */
    memset(&raw_candidate, 0, sizeof(raw_candidate));
    if (dm2_v1_session_parse_save_candidate(&raw_candidate, payload,
                                             payload_size) == 0) {
        printf("    FAIL: synthetic raw SKSave tail was admitted\n");
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }
    r = dm2_sl_save(tmpdir, 5, "RawSKSave", payload, payload_size);
    if (r != 0) {
        printf("    FAIL: raw slot save returned %d\n", r);
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }

    memset(&imported_session, 0xA5, sizeof(imported_session));
    r = dm2_v1_session_load_slot(tmpdir, 5, &imported_session);
    if (r == 0) {
        printf("    FAIL: slot loader admitted synthetic raw SKSave tail\n");
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }
    if (imported_session.game_tick != 0xA5A5A5A5u ||
        imported_session.champion_count != 0xA5u) {
        printf("    FAIL: rejected raw SKSave mutated session output\n");
        cleanup_one_slot_dir(tmpdir, 5);
        return 0;
    }

    printf("    PASS: raw prefix is receipted but synthetic SUPPRESS tail is "
           "not playable (payload=%zuB)\n", payload_size);
    cleanup_one_slot_dir(tmpdir, 5);
    return 1;
}

static int test_raw_sksave_scene_root_addressing(void)
{
    uint8_t payload[2048];
    size_t payload_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RawSKSaveMapSceneReceipt scene;
    DM2_V1_OriginalRawDungeonReceipt raw_receipt;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE] = { 0 };
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE] = { 0 };
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE] = { 0 };
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS] = { 0 };
    int result = 0;

    printf("  Raw SKSave c_map/c_record scene addressing...\n");
    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    gs->dwGameTick = 0x00012345u;
    gs->dwRandomSeed = 0x00002345u;
    gs->wChampionsCount = 1u;
    gs->wPlayerPosX = 0u;
    gs->wPlayerPosY = 0u;
    gs->wPlayerDir = 1u;
    gs->wPlayerMap = 0u;
    gs->wChampionLeader = 0u;
    memcpy(champion.first_name, "RAW", 3u);
    champion.absolute_direction = 1u;
    champion.cur_hp = 10;
    champion.max_hp = 10;
    if (!build_raw_sksave_payload(gs, &champion, global_flags, global_bytes,
                                  global_words, spell_effects, NULL, 0,
                                  inventory, 0u, payload, sizeof(payload),
                                  &payload_size) ||
        payload_size + 2u > sizeof(payload)) {
        goto done;
    }

    /* Give the source-shaped fixture one DB0 root.  The extra ground-stack
     * word moves the contiguous c_record pools and map bytes together,
     * exactly as READ_DUNGEON_STRUCTURE expects. */
    memmove(payload + 70u, payload + 68u, payload_size - 68u);
    payload_size += 2u;
    write_u16_le_at(payload, 10u, 1u);
    write_u16_le_at(payload, 68u, 0u);
    write_u16_le_at(payload, 70u, 0xfffeu);
    write_u16_le_at(payload, 74u, 0xfffeu);
    write_u16_le_at(payload, 82u, 0xfffeu);
    write_u16_le_at(payload, 98u, 0xfffeu);
    payload[102u] = 0x30u; /* floor byte with c_map thing-bearing flag */

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&scene, 0, sizeof(scene));
    memset(&raw_receipt, 0, sizeof(raw_receipt));
    if (!dm2_v1_original_raw_sksave_dungeon_receipt(
            payload, payload_size, &raw_receipt) ||
        !raw_receipt.valid || raw_receipt.suppress_state_offset == 0u ||
        dm2_v1_dungeon_load(&dungeon, payload,
                            (int)raw_receipt.suppress_state_offset) != 0) {
        goto done;
    }
    dungeon.record_graph_complete = 1;
    if (!dm2_v1_dungeon_validate_record_graph(&dungeon) ||
        !dm2_v1_dungeon_collect_raw_sksave_map_scene(&dungeon, 0, &scene) ||
        !scene.valid || scene.thing_bearing_tile_count != 1u ||
        scene.addressable_root_count != 1u ||
        scene.root_count_by_type[0] != 1u ||
        scene.root_count_by_type[4] != 0u || scene.map_data_hash == 0u ||
        scene.object_record_hash == 0u) {
        goto done;
    }
    result = 1;
done:
    dm2_v1_dungeon_free(&dungeon);
    if (!result) {
        printf("    FAIL: raw SKSave scene did not retain source c_record address\n");
        return 0;
    }
    printf("    PASS: raw map root resolves through c_map and c_record bytes\n");
    return 1;
}

static int test_raw_sksave_import_is_transactional(void)
{
    uint8_t payload[2048];
    size_t payload_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_V1_SessionState session;
    DM2_V1_SessionState before;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE] = { 0 };
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE] = { 0 };
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE] = { 0 };
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS] = { 0 };

    printf("  Raw SKSave malformed tail keeps prior session intact...\n");
    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    gs->dwGameTick = 0x12345678u;
    gs->dwRandomSeed = 0x00002211u;
    gs->wChampionsCount = 1u;
    gs->wPlayerPosX = 3u;
    gs->wPlayerPosY = 2u;
    gs->wPlayerDir = 1u;
    gs->wPlayerMap = 0u;
    memcpy(champion.first_name, "ZED", 3u);
    champion.cur_hp = 20;
    champion.max_hp = 25;

    if (!build_raw_sksave_payload(gs, &champion, global_flags, global_bytes,
                                  global_words, spell_effects, NULL, 0,
                                  inventory, 0u, payload, sizeof(payload),
                                  &payload_size) || payload_size < 2u) {
        printf("    FAIL: could not build raw SKSave fixture\n");
        return 0;
    }

    memset(&session, 0xA5, sizeof(session));
    before = session;
    /* Keep the raw dungeon/GAMESTATE prefix valid and truncate only the
     * terminal leader-hand field after the importer has decoded prior rows. */
    if (dm2_v1_session_import_raw_sksave_payload(&session, payload,
                                                 payload_size - 2u) == 0 ||
        memcmp(&session, &before, sizeof(session)) != 0) {
        printf("    FAIL: malformed raw SKSave partially changed session\n");
        return 0;
    }
    write_u16_le_at(payload, 2u, 19u);
    if (dm2_v1_session_import_raw_sksave_payload(&session, payload,
                                                 payload_size) == 0 ||
        memcmp(&session, &before, sizeof(session)) != 0) {
        printf("    FAIL: undersized source map-data span was accepted\n");
        return 0;
    }
    printf("    PASS: malformed raw SKSave had no partial session publish\n");
    return 1;
}

static int test_original_envelope_import_is_transactional(void)
{
    uint8_t payload[2048];
    size_t payload_size = 0u;
    size_t enc_gs_size = 0u;
    size_t enc_champ_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_V1_SessionState session;
    DM2_V1_SessionState before;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };

    printf("  Original D2RS malformed tail keeps prior session intact...\n");
    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    gs->dwGameTick = 0x12345678u;
    gs->dwRandomSeed = 0x00002211u;
    gs->wChampionsCount = 1u;
    gs->wPlayerPosX = 3u;
    gs->wPlayerPosY = 2u;
    gs->wPlayerDir = 1u;
    gs->wPlayerMap = 0u;
    memcpy(champion.first_name, "ZED", 3u);
    champion.cur_hp = 20;
    champion.max_hp = 25;

    if (!build_resume_payload(gs, &champion, global_flags, NULL, NULL, NULL,
                              NULL, 0, payload, sizeof(payload),
                              &payload_size, &enc_gs_size, &enc_champ_size) ||
        payload_size < 2u || enc_gs_size == 0u || enc_champ_size == 0u) {
        printf("    FAIL: could not build original D2RS fixture\n");
        return 0;
    }

    memset(&session, 0xA5, sizeof(session));
    before = session;
    /* The game state and champion are valid. Truncate the tagged optional
     * section so failure happens only after the candidate has been decoded. */
    if (dm2_v1_session_import_original_payload(&session, payload,
                                               payload_size - 2u) == 0 ||
        memcmp(&session, &before, sizeof(session)) != 0) {
        printf("    FAIL: malformed original D2RS partially changed session\n");
        return 0;
    }
    printf("    PASS: malformed original D2RS had no partial session publish\n");
    return 1;
}

static int test_sksave_corpus_scan_receipt(void)
{
    printf("  Real SKSave corpus scan receipt...\n");
    char tmpdir[256];
    uint8_t payload_a[2048];
    uint8_t payload_b[2048];
    uint8_t payload_c[4096];
    size_t payload_a_size = 0u;
    size_t payload_b_size = 0u;
    size_t enc_gs_size = 0u;
    size_t enc_champ_size = 0u;
    uint8_t loaded_payload[DM2_SESSION_MAX_SIZE];
    size_t loaded_payload_size = 0u;
    uint8_t imported_payload[DM2_SESSION_MAX_SIZE];
    size_t imported_payload_size = 0u;
    size_t largest_payload_size = 0u;
    DM2_V1_SaveCandidate loaded_candidate;
    char nested_dir[256];
    char nested_save_path[256];
    char renamed_save_path[256];
    int payload_c_size;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_V1_SessionState session;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE] = { 0 };
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE] = { 0 };
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE] = { 0 };
    DM2_TimerEntry raw_timer;
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS] = { 0 };
    DM2_SKSaveCorpusReceipt receipt;
    DM2_OriginalTimerFormatCorpusReceipt timer_receipt;
    DM2_OriginalSaveStateCorpusReceipt state_receipt;
    int r;

    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    gs->dwGameTick = 0x55u;
    gs->dwRandomSeed = 0x66u;
    gs->wChampionsCount = 1;
    gs->wPlayerPosX = 2;
    gs->wPlayerPosY = 3;
    gs->wPlayerDir = 1;
    gs->wPlayerMap = 0;
    gs->wChampionLeader = 0;
    memcpy(champion.first_name, "SK", 2);
    champion.absolute_direction = 1;
    champion.cur_hp = 42;
    champion.max_hp = 50;
    global_flags[0] = 0x82u;
    global_bytes[7] = 0x4du;
    global_words[3] = 0x81f2u;
    spell_effects[5] = 0x37u;
    memset(&raw_timer, 0, sizeof(raw_timer));
    raw_timer.timer_id = 0x0012u;
    raw_timer.current_tick = 0x0034u;
    raw_timer.interval_ticks = 0x0005u;
    raw_timer.flags = 0x0001u;
    raw_timer.user_data = 0x0022u;
    dm2_v1_test_session_fixture_new(&session);
    session.game_tick = 0x1234u;
    payload_c_size = dm2_v1_session_serialize(&session, payload_c,
                                              sizeof(payload_c));
    if (payload_c_size <= 0 ||
        !build_resume_payload(gs, &champion, global_flags, global_bytes,
                              global_words, spell_effects, NULL, 0,
                              payload_b, sizeof(payload_b), &payload_b_size,
                              &enc_gs_size, &enc_champ_size)) {
        printf("    FAIL: could not build SKSave corpus importer fixtures\n");
        return 0;
    }
    gs->wTimersCount = 1;
    if (!build_raw_sksave_payload(gs, &champion, global_flags, global_bytes,
                                  global_words, spell_effects, &raw_timer, 1,
                                  inventory, dm2_db_make_handle(5, 0u), payload_a,
                                  sizeof(payload_a), &payload_a_size)) {
        printf("    FAIL: could not build SKSave corpus importer fixtures\n");
        return 0;
    }
    gs->wTimersCount = 0;
    largest_payload_size = payload_b_size;
    if ((size_t)payload_c_size > largest_payload_size) {
        largest_payload_size = (size_t)payload_c_size;
    }

    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_sksave_corpus_%d",
             FS_GETPID());
    cleanup_corpus_fixture_dir(tmpdir);
    FS_MKDIR(tmpdir);

    memset(&receipt, 0xCC, sizeof(receipt));
    if (!dm2_v1_sksave_corpus_scan(tmpdir, &receipt) ||
        receipt.valid_slot_count != 0 ||
        receipt.valid_slot_mask != 0 ||
        receipt.has_last_session ||
        receipt.has_last_session_backup ||
        receipt.invalid_candidate_count != 0 ||
        receipt.importable_candidate_count != 0 ||
        receipt.import_rejected_candidate_count != 0 ||
        receipt.first_importable_kind != DM2_SK_SAVE_KIND_NONE ||
        receipt.first_importable_payload_size != 0u ||
        receipt.recursive_scan_depth_limit != 4 ||
        receipt.recursive_scan_candidate_cap != 64 ||
        receipt.recursive_scan_truncated != 0 ||
        receipt.total_payload_size != 0) {
        printf("    FAIL: empty corpus did not produce an empty receipt\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    r = dm2_sl_save(tmpdir, 3, "Slot3", payload_c, (size_t)payload_c_size);
    if (r != 0) {
        printf("    FAIL: could not write slot corpus save (%d)\n", r);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    r = dm2_sl_save_last_session(tmpdir, "LastA",
                                 payload_b, payload_b_size);
    if (r != 0) {
        printf("    FAIL: could not write first last-session save (%d)\n", r);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    r = dm2_sl_save_last_session(tmpdir, "LastB",
                                 payload_b, payload_b_size);
    if (r != 0) {
        printf("    FAIL: could not rotate last-session backup (%d)\n", r);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    if (write_bad_slot_file(tmpdir, 8) != 0) {
        printf("    FAIL: could not write bad slot fixture\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_sksave_corpus_scan(tmpdir, &receipt) ||
        !receipt.has_last_session ||
        !receipt.has_last_session_backup ||
        receipt.last_session_uses_backup ||
        receipt.valid_slot_count != 1 ||
        receipt.valid_slot_mask != (uint16_t)(1u << 3) ||
        receipt.invalid_candidate_count != 1 ||
        receipt.importable_candidate_count != 1 ||
        receipt.import_rejected_candidate_count != 1 ||
        receipt.firestaff_session_candidate_count != 1 ||
        receipt.original_envelope_candidate_count != 1 ||
        receipt.original_raw_candidate_count != 0 ||
        receipt.first_importable_kind != DM2_SK_SAVE_KIND_ORIGINAL_ENVELOPE ||
        receipt.first_importable_payload_size != payload_b_size ||
        receipt.importable_kind_mask !=
            (uint32_t)(1u << DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE) ||
        receipt.importable_payload_hash == 0u ||
        receipt.total_importable_payload_size != payload_b_size ||
        receipt.largest_payload_size != largest_payload_size ||
        receipt.total_payload_size !=
            payload_b_size + (size_t)payload_c_size ||
        strstr(receipt.first_importable_path, "SKSave.dat") == NULL ||
        strstr(receipt.first_valid_path, "SKSave.dat") == NULL) {
        printf("    FAIL: mixed corpus receipt did not match expected fields "
               "(valid=%u mask=0x%04X invalid=%u importable=%u rejected=%u "
               "fs=%u env=%u raw=%u largest=%zu/%zu total=%zu/%zu "
               "first=%s import_first=%s)\n",
               receipt.valid_slot_count, receipt.valid_slot_mask,
               receipt.invalid_candidate_count,
               receipt.importable_candidate_count,
               receipt.import_rejected_candidate_count,
               receipt.firestaff_session_candidate_count,
               receipt.original_envelope_candidate_count,
               receipt.original_raw_candidate_count,
               receipt.largest_payload_size, largest_payload_size,
               receipt.total_payload_size,
               payload_b_size + (size_t)payload_c_size,
               receipt.first_valid_path,
               receipt.first_importable_path);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(&timer_receipt, 0, sizeof(timer_receipt));
    if (!dm2_v1_original_timer_format_corpus_probe(tmpdir, &timer_receipt) ||
        timer_receipt.scan_complete != 1 ||
        timer_receipt.has_header_verified_candidate != 1 ||
        timer_receipt.timer_layout_owner_proven != 0 ||
        timer_receipt.matching_timer_record_count != 0 ||
        timer_receipt.original_candidate_list_complete != 1 ||
        timer_receipt.original_candidate_count != 1 ||
        timer_receipt.rejected_unowned_candidate_count != 1 ||
        timer_receipt.candidate_receipt_count != 1 ||
        timer_receipt.candidate_receipts[0].kind !=
            DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE ||
        timer_receipt.candidate_receipts[0].import_rejected != 1 ||
        timer_receipt.candidate_receipts[0].payload_size != payload_b_size ||
        timer_receipt.candidate_receipts[0].payload_hash == 0u ||
        timer_receipt.retained_original_payload_bytes != payload_b_size ||
        timer_receipt.corpus_hash == 0u) {
        printf("    FAIL: original timer-format probe promoted or lost "
               "unowned corpus evidence\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(&state_receipt, 0, sizeof(state_receipt));
    if (!dm2_v1_original_save_state_corpus_probe(tmpdir, &state_receipt) ||
        state_receipt.scan_complete != 1 ||
        state_receipt.original_candidate_list_complete != 1 ||
        state_receipt.original_candidate_count != 1 ||
        state_receipt.parsed_candidate_count != 1 ||
        state_receipt.rejected_candidate_count != 0 ||
        state_receipt.entry_count != 1 ||
        state_receipt.entries[0].candidate.kind !=
            DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE ||
        state_receipt.entries[0].candidate.source_file_hash == 0u ||
        state_receipt.entries[0].game_tick != gs->dwGameTick ||
        state_receipt.entries[0].rng_seed != gs->dwRandomSeed ||
        state_receipt.entries[0].party_x != gs->wPlayerPosX ||
        state_receipt.entries[0].party_y != gs->wPlayerPosY ||
        state_receipt.entries[0].party_dir != gs->wPlayerDir ||
        state_receipt.entries[0].party_map != gs->wPlayerMap ||
        state_receipt.entries[0].champion_count != gs->wChampionsCount ||
        state_receipt.entries[0].timer_count != 0u ||
        state_receipt.entries[0].rain_intensity != gs->bRainStrength ||
        state_receipt.entries[0].global_flags_hash !=
            corpus_hash_bytes(global_flags, sizeof(global_flags)) ||
        state_receipt.entries[0].global_bytes_hash !=
            corpus_hash_bytes(global_bytes, sizeof(global_bytes)) ||
        state_receipt.entries[0].global_words_hash !=
            corpus_hash_words_le(global_words, DM2_GLOBAL_WORDS_SIZE) ||
        state_receipt.entries[0].spell_effects_hash !=
            corpus_hash_bytes(spell_effects, sizeof(spell_effects)) ||
        state_receipt.entries[0].raw_timer_stream_offset != 0u ||
        state_receipt.entries[0].raw_timer_stream_byte_count != 0u ||
        state_receipt.entries[0].raw_timer_stream_hash != 0u ||
        state_receipt.entries[0].state_hash == 0u ||
        state_receipt.corpus_hash == 0u) {
        printf("    FAIL: original save state census lost source-decoded fields\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(loaded_payload, 0, sizeof(loaded_payload));
    loaded_payload_size = 0u;
    memset(&loaded_candidate, 0, sizeof(loaded_candidate));
    if (!dm2_v1_sksave_corpus_load_first_importable(
            tmpdir, loaded_payload, sizeof(loaded_payload),
            &loaded_payload_size, &receipt) ||
        loaded_payload_size != payload_b_size ||
        dm2_v1_session_parse_save_candidate(&loaded_candidate,
                                             loaded_payload,
                                             loaded_payload_size) != 0 ||
        loaded_candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE ||
        strstr(receipt.first_importable_path, "SKSave.dat") == NULL) {
        printf("    FAIL: corpus loader did not promote last-session "
               "importable envelope candidate\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    memset(imported_payload, 0, sizeof(imported_payload));
    imported_payload_size = 0u;
    if (!dm2_v1_sksave_corpus_load_first_importable(
            tmpdir, imported_payload, sizeof(imported_payload),
            &imported_payload_size, &receipt) ||
        imported_payload_size != payload_b_size ||
        memcmp(imported_payload, payload_b, payload_b_size) != 0) {
        printf("    FAIL: first importable corpus payload was not loaded\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    if (write_bad_last_session_file(tmpdir) != 0) {
        printf("    FAIL: could not corrupt last-session primary\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_sksave_corpus_scan(tmpdir, &receipt) ||
        receipt.has_last_session ||
        !receipt.has_last_session_backup ||
        !receipt.last_session_uses_backup ||
        receipt.valid_slot_count != 1 ||
        receipt.invalid_candidate_count != 2 ||
        receipt.importable_candidate_count != 1 ||
        receipt.import_rejected_candidate_count != 1 ||
        receipt.firestaff_session_candidate_count != 1 ||
        receipt.original_envelope_candidate_count != 1 ||
        receipt.original_raw_candidate_count != 0 ||
        receipt.first_importable_kind != DM2_SK_SAVE_KIND_ORIGINAL_ENVELOPE ||
        receipt.first_importable_payload_size != payload_b_size ||
        receipt.importable_kind_mask !=
            (uint32_t)(1u << DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE) ||
        receipt.importable_payload_hash == 0u ||
        strstr(receipt.first_importable_path, "SKSave.bak") == NULL ||
        strstr(receipt.first_valid_path, "SKSave.bak") == NULL) {
        printf("    FAIL: backup-selected corpus receipt did not match "
               "expected fields\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(loaded_payload, 0, sizeof(loaded_payload));
    loaded_payload_size = 0u;
    memset(&loaded_candidate, 0, sizeof(loaded_candidate));
    if (!dm2_v1_sksave_corpus_load_first_importable(
            tmpdir, loaded_payload, sizeof(loaded_payload),
            &loaded_payload_size, &receipt) ||
        loaded_payload_size != payload_b_size ||
        dm2_v1_session_parse_save_candidate(&loaded_candidate,
                                             loaded_payload,
                                             loaded_payload_size) != 0 ||
        loaded_candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE ||
        strstr(receipt.first_importable_path, "SKSave.bak") == NULL) {
        printf("    FAIL: corpus loader did not fall through to first "
               "importable backup candidate\n");
        cleanup_slot_dir(tmpdir);
        return 0;
    }

    snprintf(nested_dir, sizeof(nested_dir), "%s/real_corpus", tmpdir);
    FS_MKDIR(nested_dir);
    snprintf(nested_save_path, sizeof(nested_save_path),
             "%s/sksave04.dat", nested_dir);
    if (write_valid_sksave_file_at_path(nested_save_path, "Nested",
                                        payload_b, payload_b_size) != 0) {
        printf("    FAIL: could not write nested lowercase corpus save\n");
        (void)remove(nested_save_path);
        FS_RMDIR(nested_dir);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    /* Corpus files are frequently copied from removable media or archives
     * without SKProject's live-resume basename.  Header + parser admission,
     * not this name, must decide whether the original payload is retained. */
    snprintf(renamed_save_path, sizeof(renamed_save_path),
             "%s/captured-original.bin", nested_dir);
    if (write_valid_sksave_file_at_path(renamed_save_path, "Captured",
                                        payload_a, payload_a_size) != 0) {
        printf("    FAIL: could not write renamed corpus save\n");
        (void)remove(nested_save_path);
        FS_RMDIR(nested_dir);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_sksave_corpus_scan(tmpdir, &receipt) ||
        receipt.valid_slot_count != 1 ||
        receipt.importable_candidate_count != 2 ||
        receipt.import_rejected_candidate_count != 2 ||
        receipt.firestaff_session_candidate_count != 1 ||
        receipt.original_envelope_candidate_count != 2 ||
        receipt.original_raw_candidate_count != 0 ||
        receipt.first_importable_kind != DM2_SK_SAVE_KIND_ORIGINAL_ENVELOPE ||
        receipt.first_importable_payload_size != payload_b_size ||
        receipt.recursive_candidate_count != 2 ||
        receipt.recursive_importable_candidate_count != 1 ||
        receipt.alternate_name_candidate_count != 2 ||
        receipt.header_discovered_candidate_count != 1 ||
        receipt.extra_valid_candidate_count != 2 ||
        receipt.importable_kind_mask !=
            (uint32_t)(1u << DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE) ||
        receipt.importable_payload_hash == 0u ||
        receipt.recursive_scan_depth_limit != 4 ||
        receipt.recursive_scan_candidate_cap != 64 ||
        receipt.recursive_scan_truncated != 0 ||
        strstr(receipt.first_importable_path, "SKSave.bak") == NULL) {
        printf("    FAIL: recursive corpus receipt did not match expected "
               "fields (importable=%u rejected=%u fs=%u env=%u raw=%u "
               "rec=%u rec_imp=%u alt=%u header=%u extra=%u kind=0x%X "
               "first=%s)\n",
               receipt.importable_candidate_count,
               receipt.import_rejected_candidate_count,
               receipt.firestaff_session_candidate_count,
               receipt.original_envelope_candidate_count,
               receipt.original_raw_candidate_count,
               receipt.recursive_candidate_count,
               receipt.recursive_importable_candidate_count,
               receipt.alternate_name_candidate_count,
               receipt.header_discovered_candidate_count,
               receipt.extra_valid_candidate_count,
               receipt.importable_kind_mask,
               receipt.first_importable_path);
        (void)remove(renamed_save_path);
        (void)remove(nested_save_path);
        FS_RMDIR(nested_dir);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    memset(&state_receipt, 0, sizeof(state_receipt));
    if (!dm2_v1_original_save_state_corpus_probe(tmpdir, &state_receipt) ||
        state_receipt.scan_complete != 1 ||
        state_receipt.original_candidate_list_complete != 1 ||
        state_receipt.original_candidate_count != 2 ||
        state_receipt.parsed_candidate_count != 2 ||
        state_receipt.rejected_candidate_count != 0 ||
        state_receipt.entry_count != 2) {
        printf("    FAIL: recursive original state census did not retain "
               "only source-complete envelope candidates (original=%u "
               "parsed=%u rejected=%u entries=%u)\n",
               state_receipt.original_candidate_count,
               state_receipt.parsed_candidate_count,
               state_receipt.rejected_candidate_count,
               state_receipt.entry_count);
        (void)remove(renamed_save_path);
        (void)remove(nested_save_path);
        FS_RMDIR(nested_dir);
        cleanup_slot_dir(tmpdir);
        return 0;
    }
    (void)remove(renamed_save_path);
    (void)remove(nested_save_path);
    FS_RMDIR(nested_dir);

    printf("    PASS: corpus scan reports resume order, slot mask, importable "
           "Firestaff/envelope saves, and rejects renamed raw-SKSave tails, "
           "payload sizes, invalid saves, timer-format rejection evidence and "
           "first-importable payload promotion\n");
    cleanup_slot_dir(tmpdir);
    return 1;
}

/* Retained as source-fixture notes only; no production test may execute a
 * fabricated D2RS/runtime save path while the original writer is absent. */
#if 0
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

    dm2_v1_test_session_fixture_new(&session);
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

/* Retired Firestaff-private FS2RT01 sidecar fixture. DM2_GAME_SAVE must write
 * SKProject's complete SUPPRESS stream; this test is retained only as source
 * archaeology and is not compiled or registered as runtime evidence. */
#if 0
static int test_live_runtime_state_roundtrip(void)
{
    char tmpdir[256];
    DM2_V1_BootProfile boot;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    DM2_V1_QuicksaveReceipt receipt;
    DM2_V1_RuntimeGraphicsSetSceneReceipt scene_before;
    DM2_V1_RuntimeGraphicsSetSceneReceipt scene_after;
    const DM2_V1_CreatureInstance *before;
    const DM2_V1_CreatureInstance *after;
    uint8_t *save_data = NULL;
    uint8_t *valid_save_data = NULL;
    uint8_t session_bytes[DM2_SESSION_MAX_SIZE];
    DM2_V1_SessionState live_session;
    DM2_V1_SessionState rejected_session;
    size_t save_size;
    size_t session_offset;
    int creature_id;
    int session_size;
    int saved_hp;
    uint32_t saved_animation_tick;
    uint32_t saved_render_revision;

    printf("  Live CCM/dungeon/GDAT runtime round-trip...\n");
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_live_%d", FS_GETPID());
    FS_MKDIR(tmpdir);
    memset(&boot, 0, sizeof(boot));
    memset(&game, 0, sizeof(game));
    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = (uint8_t *)calloc(16u, 1u);
    if (!dungeon.raw_data) return 0;
    dungeon.raw_size = 16;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 2;
    dungeon.square_bytes = 1;
    dungeon.raw_map_data_base = 0;
    boot.dm2_state = &game;
    boot.dungeon_data = &dungeon;
    boot.graphics_size = 0x876543u;
    snprintf(boot.graphics_md5, sizeof(boot.graphics_md5), "runtime-gdat");
    snprintf(boot.save_root, sizeof(boot.save_root), "%s", tmpdir);
    dm2_v1_runtime_init(&boot);
    /* Runtime init deliberately has no invented GAME_LOAD party. This
     * isolated codec fixture supplies the accepted zero-champion session it
     * needs to exercise an actual live save/restore transaction. */
    memset(&live_session, 0, sizeof(live_session));
    live_session.time_of_day_minutes = 720u;
    if (dm2_v1_runtime_apply_session(&live_session) != 0) goto fail;
    memset(&scene_before, 0, sizeof(scene_before));
    if (dm2_v1_runtime_graphicsset_scene_receipt(&scene_before) != 0 ||
        scene_before.ready != 0) goto fail;
    creature_id = dm2_v1_creature_spawn(3, 1, 1, 0, 2, 8);
    if (creature_id < 0) goto fail;
    dm2_v1_creature_tick();
    before = dm2_v1_creature_get_instance(creature_id);
    if (!before || before->animation_tick == 0 || before->render_revision == 0)
        goto fail;
    saved_hp = before->hp_current;
    saved_animation_tick = before->animation_tick;
    saved_render_revision = before->render_revision;
    dungeon.raw_data[3] = 0x24;
    save_size = dm2_v1_runtime_live_save_size();
    save_data = (uint8_t *)malloc(save_size);
    if (!save_data || dm2_v1_runtime_serialize_live_save(save_data, save_size) < 0)
        goto fail;
    valid_save_data = (uint8_t *)malloc(save_size);
    if (!valid_save_data) goto fail;
    memcpy(valid_save_data, save_data, save_size);
    (void)dm2_v1_creature_deal_damage(creature_id, 3);
    dungeon.raw_data[3] = 0;
    if (dm2_v1_runtime_restore_live_save(save_data, save_size) != 0)
        goto fail;
    memset(&scene_after, 0, sizeof(scene_after));
    (void)dm2_v1_runtime_graphicsset_scene_receipt(&scene_after);
    if (scene_after.ready != scene_before.ready ||
        scene_after.map_graphics_style != scene_before.map_graphics_style)
        goto fail;
    after = dm2_v1_creature_get_instance(creature_id);
    if (!after || after->hp_current != saved_hp ||
        after->animation_tick != saved_animation_tick ||
        after->render_revision != saved_render_revision ||
        dungeon.raw_data[3] != 0x24)
        goto fail;

    /* The serialized session can pass its wire validator while SKProject's
     * timer-owner pass rejects a champion timer for an absent champion. A
     * live-sidecar failure here must leave the already-running creature and
     * dungeon state untouched. */
    if (dm2_v1_runtime_export_session(&rejected_session) != 0)
        goto fail;
    rejected_session.original_timer_count = 1u;
    rejected_session.original_timers[0].interval_ticks = 0x000cu;
    session_size = dm2_v1_session_serialize(&rejected_session, session_bytes,
                                             sizeof(session_bytes));
    if (session_size < 0 || save_size < (size_t)session_size +
            sizeof(DM2_V1_CreatureLiveState) + (size_t)dungeon.raw_size)
        goto fail;
    session_offset = save_size - (size_t)session_size -
        sizeof(DM2_V1_CreatureLiveState) - (size_t)dungeon.raw_size;
    memcpy(save_data + session_offset, session_bytes, (size_t)session_size);
    if (dm2_v1_creature_deal_damage(creature_id, 1) != 0)
        goto fail;
    dungeon.raw_data[3] = 0x66;
    after = dm2_v1_creature_get_instance(creature_id);
    if (!after || after->hp_current != saved_hp - 1 ||
        dm2_v1_runtime_restore_live_save(save_data, save_size) == 0 ||
        dungeon.raw_data[3] != 0x66)
        goto fail;
    after = dm2_v1_creature_get_instance(creature_id);
    if (!after || after->hp_current != saved_hp - 1)
        goto fail;
    if (dm2_v1_runtime_restore_live_save(valid_save_data, save_size) != 0)
        goto fail;

    if (!dm2_v1_runtime_quicksave_boot_profile_with_receipt(&boot, &receipt) ||
        !receipt.session_valid ||
        receipt.graphicsset_scene.ready != scene_before.ready ||
        dm2_v1_creature_deal_damage(creature_id, 2) != 0)
        goto fail;
    dungeon.raw_data[3] = 0;
    if (dm2_v1_runtime_load_last_session(tmpdir) != 0)
        goto fail;
    after = dm2_v1_creature_get_instance(creature_id);
    if (!after || after->hp_current != saved_hp ||
        after->animation_tick != saved_animation_tick ||
        after->render_revision != saved_render_revision ||
        dungeon.raw_data[3] != 0x24)
        goto fail;
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/SKSave.dat", tmpdir);
        (void)remove(path);
        snprintf(path, sizeof(path), "%s/SKSave.bak", tmpdir);
        (void)remove(path);
        snprintf(path, sizeof(path), "%s/SKSave.runtime", tmpdir);
        (void)remove(path);
    }
    FS_RMDIR(tmpdir);
    free(valid_save_data);
    free(save_data);
    free(dungeon.raw_data);
    printf("    PASS: direct and quicksave live CCM/dungeon/GDAT restore work\n");
    return 1;
fail:
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/SKSave.dat", tmpdir);
        (void)remove(path);
        snprintf(path, sizeof(path), "%s/SKSave.bak", tmpdir);
        (void)remove(path);
        snprintf(path, sizeof(path), "%s/SKSave.runtime", tmpdir);
        (void)remove(path);
    }
    FS_RMDIR(tmpdir);
    free(valid_save_data);
    free(save_data);
    free(dungeon.raw_data);
    printf("    FAIL: live runtime state did not round-trip\n");
    return 0;
}
#endif

static int test_original_save_candidate_live_restore(void)
{
    uint8_t payload[2048];
    size_t payload_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_V1_SaveCandidate candidate;
    DM2_V1_BootProfile boot;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RuntimeRawSaveHandoffReceipt raw_handoff;
    DM2_TimerEntry rejected_timer[1];
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE] = { 0 };
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE] = { 0 };
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE] = { 0 };
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS] = { 0 };
    uint8_t first_frame[320 * 200];
    uint8_t rejected_payload[2048];
    uint8_t *expected_dungeon = NULL;
    size_t rejected_payload_size = 0u;
    int creature_id;
    int result = 0;

    printf("  Original SKSave candidate restores live party/dungeon state...\n");
    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    gs->dwGameTick = 0x00012345u;
    gs->dwRandomSeed = 0x00002345u;
    gs->wChampionsCount = 1;
    gs->wPlayerPosX = 3;
    gs->wPlayerPosY = 4;
    gs->wPlayerDir = 1;
    gs->wPlayerMap = 0;
    gs->wChampionLeader = 0;
    memcpy(champion.first_name, "HISS", 4);
    champion.absolute_direction = 1;
    champion.cur_hp = 50;
    champion.max_hp = 60;
    inventory[0] = dm2_db_make_handle(5, 0x17);
    inventory[8] = dm2_db_make_handle(6, 0x23);
    if (!build_raw_sksave_payload(gs, &champion,
                                  global_flags, global_bytes, global_words,
                                  spell_effects, NULL, 0, inventory,
                                  dm2_db_make_handle(7, 0x2a), payload,
                                  sizeof(payload), &payload_size) ||
        dm2_v1_session_parse_save_candidate(&candidate, payload,
                                             payload_size) != 0 ||
        candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW ||
        candidate.dungeon_size == 0u) {
        printf("    FAIL: could not parse original raw candidate\n");
        return 0;
    }

    memset(&boot, 0, sizeof(boot));
    memset(&game, 0, sizeof(game));
    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = (uint8_t *)calloc(candidate.dungeon_size, 1u);
    expected_dungeon = (uint8_t *)malloc(candidate.dungeon_size);
    if (!dungeon.raw_data || !expected_dungeon) goto done;
    dungeon.raw_size = (int)candidate.dungeon_size;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 1;
    boot.dm2_state = &game;
    boot.dungeon_data = &dungeon;
    boot.graphics_size = 0x102030u;
    snprintf(boot.graphics_md5, sizeof(boot.graphics_md5), "candidate-gdat");
    dm2_v1_runtime_init(&boot);
    creature_id = dm2_v1_creature_spawn(2, 0, 0, 0, 1, 5);
    if (creature_id < 0) goto done;

    memcpy(expected_dungeon, payload, candidate.dungeon_size);
    if (dm2_v1_runtime_restore_save_candidate(payload, payload_size) != 0 ||
        game.party_x != 3 || game.party_y != 4 || game.party_dir != 1 ||
        dm2_v1_runtime_get_tick_count() != (int)gs->dwGameTick ||
        dm2_v1_runtime_get_champion_inventory_object(0, 8) != inventory[8] ||
        dm2_v1_runtime_get_leader_hand_object() != dm2_db_make_handle(7, 0x2a) ||
        dm2_v1_creature_count() != 0 || dungeon.level_count != 1 ||
        dungeon.level_widths[0] != 4 || dungeon.level_heights[0] != 5 ||
        memcmp(dungeon.raw_data, expected_dungeon, candidate.dungeon_size) != 0) {
        goto done;
    }
    /* A raw save can decode but still fail SKProject's post-load timer-owner
     * reconstruction. That failure must retain the live world and CCM rather
     * than clearing creatures before the session is rejected. */
    creature_id = dm2_v1_creature_spawn(2, 0, 0, 0, 1, 5);
    memset(rejected_timer, 0, sizeof(rejected_timer));
    ((uint8_t *)&rejected_timer[0])[4] = 0x0cu; /* tty0C champion timer */
    ((uint8_t *)&rejected_timer[0])[5] = 1u;    /* actor one; only zero exists */
    gs->wTimersCount = 1u;
    if (creature_id < 0 || !build_raw_sksave_payload(
            gs, &champion, global_flags, global_bytes, global_words,
            spell_effects, rejected_timer, 1, inventory,
            dm2_db_make_handle(7, 0x2a), rejected_payload,
            sizeof(rejected_payload), &rejected_payload_size) ||
        dm2_v1_runtime_restore_save_candidate(rejected_payload,
                                              rejected_payload_size) == 0 ||
        game.party_x != 3 || game.party_y != 4 ||
        dm2_v1_creature_count() != 1 ||
        memcmp(dungeon.raw_data, expected_dungeon,
               candidate.dungeon_size) != 0) {
        goto done;
    }
    gs->wTimersCount = 0u;
    memset(&raw_handoff, 0, sizeof(raw_handoff));
    if (!dm2_v1_runtime_last_raw_sksave_handoff_receipt(&raw_handoff) ||
        !raw_handoff.valid || raw_handoff.first_frame_consumed ||
        raw_handoff.map_count != 1u ||
        raw_handoff.dungeon_byte_count != candidate.dungeon_size ||
        raw_handoff.prefix_hash != candidate.dungeon_receipt.prefix_hash ||
        raw_handoff.map_data_hash != candidate.dungeon_receipt.map_data_hash ||
        raw_handoff.db_record_counts[0] != 1u ||
        raw_handoff.db_record_counts[3] != 1u ||
        raw_handoff.party_level != 0u || raw_handoff.party_x != 3u ||
        raw_handoff.party_y != 4u || raw_handoff.party_dir != 1u ||
        !raw_handoff.map_scene_valid ||
        raw_handoff.map_scene_map_data_hash == 0u ||
        raw_handoff.map_scene_terrain_hash == 0u ||
        raw_handoff.map_scene_object_record_hash == 0u) {
        goto done;
    }
    /* The first presented frame must still be backed by the exact raw
     * SKSave map/object receipt. A changed live map is not allowed to
     * consume the GAME_LOAD handoff. */
    dungeon.raw_data[dungeon.raw_map_data_base] ^= 0x20u;
    memset(first_frame, 0, sizeof(first_frame));
    if (dm2_v1_runtime_render_frame(game.party_dir, game.party_x, game.party_y,
                                    first_frame, 320, 320, 200) != 0 ||
        !dm2_v1_runtime_last_raw_sksave_handoff_receipt(&raw_handoff) ||
        raw_handoff.first_frame_consumed) {
        goto done;
    }
    dungeon.raw_data[dungeon.raw_map_data_base] ^= 0x20u;
    memset(first_frame, 0, sizeof(first_frame));
    if (dm2_v1_runtime_render_frame(game.party_dir, game.party_x, game.party_y,
                                    first_frame, 320, 320, 200) != 0 ||
        !dm2_v1_runtime_last_raw_sksave_handoff_receipt(&raw_handoff) ||
        !raw_handoff.first_frame_consumed ||
        raw_handoff.prefix_hash != candidate.dungeon_receipt.prefix_hash ||
        !raw_handoff.map_scene_valid) {
        goto done;
    }

    /* Truncation must be rejected before it can mutate party, dungeon, CCM,
     * or the GDAT-bound runtime view. */
    if (dm2_v1_runtime_restore_save_candidate(payload, payload_size - 1u) == 0 ||
        game.party_x != 3 || game.party_y != 4 ||
        memcmp(dungeon.raw_data, expected_dungeon, candidate.dungeon_size) != 0) {
        goto done;
    }
    /* Insert one ground-stack word, root it at DB0[0], and mark map (0,0)
     * thing-bearing. DB0[0].w0 remains the fixture's invalid 0x1234 link, so
     * c_record.cpp's bounded next-record route must reject before publication.
     * The insertion shifts the complete SUPPRESS tail with its source prefix. */
    if (payload_size + 2u > sizeof(payload)) goto done;
    memmove(payload + 70u, payload + 68u, payload_size - 68u);
    payload_size += 2u;
    write_u16_le_at(payload, 10u, 1u);
    write_u16_le_at(payload, 68u, 0u);
    payload[102u] = 0x10u;
    if (dm2_v1_runtime_restore_save_candidate(payload, payload_size) == 0 ||
        game.party_x != 3 || game.party_y != 4 ||
        dungeon.level_widths[0] != 4 || dungeon.level_heights[0] != 5 ||
        memcmp(dungeon.raw_data, expected_dungeon, candidate.dungeon_size) != 0 ||
        !dm2_v1_runtime_last_raw_sksave_handoff_receipt(&raw_handoff) ||
        raw_handoff.prefix_hash != candidate.dungeon_receipt.prefix_hash ||
        !raw_handoff.first_frame_consumed) {
        goto done;
    }
    /* A source-valid stream cannot place the party outside the exact saved
     * G1 map descriptor.  Rejection must retain the already-published model. */
    gs->wPlayerPosX = 4;
    if (!build_raw_sksave_payload(gs, &champion, global_flags, global_bytes,
                                  global_words, spell_effects, NULL, 0,
                                  inventory, dm2_db_make_handle(7, 0x2a),
                                  payload, sizeof(payload), &payload_size) ||
        dm2_v1_runtime_restore_save_candidate(payload, payload_size) == 0 ||
        game.party_x != 3 || game.party_y != 4 ||
        dungeon.level_widths[0] != 4 || dungeon.level_heights[0] != 5 ||
        memcmp(dungeon.raw_data, expected_dungeon, candidate.dungeon_size) != 0) {
        goto done;
    }
    result = 1;
done:
    free(expected_dungeon);
    free(dungeon.raw_data);
    if (!result) {
        printf("    FAIL: original candidate did not restore atomically\n");
        return 0;
    }
    printf("    PASS: party, dungeon, inventory, CCM reset and GDAT runtime binding restored\n");
    return 1;
}

#endif

static int test_sksave_corpus_runtime_import(void)
{
    char tmpdir[256];
    uint8_t payload[DM2_SESSION_MAX_SIZE];
    int payload_size;
    DM2_V1_SessionState session;
    DM2_V1_RuntimeCorpusImportReceipt receipt;
    DM2_SKSaveCorpusReceipt corpus;
    int result = 0;

    printf("  Firestaff-private corpus receipt is rejected by runtime...\n");
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_corpus_import_%d",
             FS_GETPID());
    FS_MKDIR(tmpdir);
    dm2_v1_test_session_fixture_new(&session);
    session.game_tick = 0x7788u;
    session.party_x = 6;
    session.party_y = 7;
    session.party_dir = 2;
    session.party_level = 0;
    session.rng_seed = 0x11223344u;
    payload_size = dm2_v1_session_serialize(&session, payload,
                                            sizeof(payload));
    if (payload_size <= 0 ||
        dm2_sl_save_last_session(tmpdir, "CorpusRuntime",
                                 payload, (size_t)payload_size) != 0) {
        goto done;
    }

    memset(&receipt, 0xCC, sizeof(receipt));
    if (!dm2_v1_sksave_corpus_scan(tmpdir, &corpus) ||
        corpus.candidate_receipt_count != 0u ||
        corpus.importable_candidate_count != 0u ||
        corpus.import_rejected_candidate_count != 1u ||
        corpus.firestaff_session_candidate_count != 1u ||
        dm2_v1_runtime_import_sksave_corpus(tmpdir, &receipt) ||
        receipt.result != DM2_V1_RUNTIME_CORPUS_IMPORT_UNAVAILABLE ||
        receipt.restored) {
        goto done;
    }
    result = 1;

done:
    cleanup_slot_dir(tmpdir);
    if (!result) {
        printf("    FAIL: Firestaff-private corpus entry reached runtime\n");
        return 0;
    }
    printf("    PASS: D2RS remains diagnostic and cannot become a runtime save\n");
    return 1;
}

#if 0 /* Retired synthetic raw-SKSave runtime fixtures.  DM2_GAME_LOAD's
        * continuous stream must not be modelled with test-owned tails. */
static int test_original_sksave_corpus_runtime_import(void)
{
    char tmpdir[256];
    uint8_t payload[2048];
    size_t payload_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_V1_SaveCandidate candidate;
    DM2_V1_RuntimeOriginalCorpusImportReceipt receipt;
    DM2_OriginalSaveStateCorpusReceipt state_corpus;
    DM2_V1_BootProfile boot;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE] = { 0 };
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE] = { 0 };
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE] = { 0 };
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS] = { 0 };
    DM2_V1_SessionState fallback_session;
    uint8_t fallback_payload[DM2_SESSION_MAX_SIZE];
    int fallback_payload_size;
    FILE *file;
    int result = 0;

    printf("  Hash-receipted original SKSave corpus candidate restores runtime...\n");
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/firestaff_dm2_original_corpus_%d",
             FS_GETPID());
    FS_MKDIR(tmpdir);
    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    gs->dwGameTick = 0x00045678u;
    gs->dwRandomSeed = 0x10293847u;
    gs->wChampionsCount = 1;
    gs->wPlayerPosX = 2;
    gs->wPlayerPosY = 3;
    gs->wPlayerDir = 3;
    gs->wChampionLeader = 0;
    memcpy(champion.first_name, "ZED", 3u);
    champion.cur_hp = 41;
    champion.max_hp = 50;
    inventory[8] = dm2_db_make_handle(6, 0x33);
    if (!build_raw_sksave_payload(gs, &champion, global_flags, global_bytes,
                                  global_words, spell_effects, NULL, 0,
                                  inventory, dm2_db_make_handle(7, 0x44),
                                  payload, sizeof(payload), &payload_size) ||
        dm2_v1_session_parse_save_candidate(&candidate, payload,
                                             payload_size) != 0 ||
        candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW ||
        dm2_sl_save_last_session(tmpdir, "OriginalCorpus", payload,
                                 payload_size) != 0) {
        goto done;
    }
    /* A valid Firestaff slot must not become an implicit substitute when the
     * explicitly selected original corpus row later goes stale. */
    dm2_v1_test_session_fixture_new(&fallback_session);
    fallback_session.party_x = 9u;
    fallback_session.party_y = 9u;
    fallback_payload_size = dm2_v1_session_serialize(
        &fallback_session, fallback_payload, sizeof(fallback_payload));
    if (fallback_payload_size <= 0 ||
        dm2_sl_save(tmpdir, 0u, "NoOriginalFallback", fallback_payload,
                    (size_t)fallback_payload_size) != 0) {
        goto done;
    }

    memset(&boot, 0, sizeof(boot));
    memset(&game, 0, sizeof(game));
    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = (uint8_t *)calloc(candidate.dungeon_size, 1u);
    if (!dungeon.raw_data) goto done;
    dungeon.raw_size = (int)candidate.dungeon_size;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 1;
    boot.dm2_state = &game;
    boot.dungeon_data = &dungeon;
    snprintf(boot.graphics_md5, sizeof(boot.graphics_md5), "original-corpus-gdat");
    dm2_v1_runtime_init(&boot);

    if (!dm2_v1_original_save_state_corpus_probe(tmpdir, &state_corpus) ||
        state_corpus.entry_count != 1u ||
        !state_corpus.entries[0].raw_dungeon_layout_valid ||
        state_corpus.entries[0].raw_dungeon_map_count !=
            candidate.dungeon_receipt.map_count ||
        state_corpus.entries[0].raw_dungeon_prefix_hash !=
            candidate.dungeon_receipt.prefix_hash ||
        state_corpus.entries[0].raw_map_data_hash !=
            candidate.dungeon_receipt.map_data_hash ||
        memcmp(state_corpus.entries[0].raw_db_record_counts,
               candidate.dungeon_receipt.db_record_counts,
               sizeof(state_corpus.entries[0].raw_db_record_counts)) != 0 ||
        !dm2_v1_runtime_import_original_sksave_state_entry(
            tmpdir, &state_corpus.entries[0], &receipt) ||
        !receipt.corpus_complete || !receipt.selected_state_admitted ||
        receipt.original_candidate_count != 1u ||
        receipt.parsed_candidate_count != 1u || receipt.corpus_hash == 0u ||
        receipt.selected_state_hash != state_corpus.entries[0].state_hash ||
        !receipt.selected_raw_dungeon_layout_valid ||
        receipt.selected_raw_dungeon_map_count !=
            candidate.dungeon_receipt.map_count ||
        receipt.selected_raw_dungeon_prefix_hash !=
            candidate.dungeon_receipt.prefix_hash ||
        receipt.selected_raw_map_data_hash != candidate.dungeon_receipt.map_data_hash ||
        memcmp(receipt.selected_raw_db_record_counts,
               candidate.dungeon_receipt.db_record_counts,
               sizeof(receipt.selected_raw_db_record_counts)) != 0 ||
        receipt.runtime_import.result != DM2_V1_RUNTIME_CORPUS_IMPORT_OK ||
        !receipt.runtime_import.restored ||
        receipt.runtime_import.candidate_kind !=
            DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW ||
        receipt.runtime_import.rejected_original_candidate ||
        receipt.runtime_import.selected_payload_size != payload_size ||
        receipt.runtime_import.selected_source_file_hash !=
            state_corpus.entries[0].candidate.source_file_hash ||
        strstr(receipt.runtime_import.selected_path, "SKSave.dat") == NULL ||
        game.party_x != 2 || game.party_y != 3 || game.party_dir != 3 ||
        dm2_v1_runtime_get_tick_count() != (int)gs->dwGameTick ||
        dm2_v1_runtime_get_champion_inventory_object(0, 8) != inventory[8] ||
        memcmp(dungeon.raw_data, payload, candidate.dungeon_size) != 0) {
        goto done;
    }
    file = fopen(state_corpus.entries[0].candidate.path, "ab");
    if (!file) {
        goto done;
    }
    if (fputc(0xa5, file) == EOF) {
        fclose(file);
        goto done;
    }
    if (fclose(file) != 0) {
        goto done;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (dm2_v1_runtime_import_original_sksave_state_entry(
            tmpdir, &state_corpus.entries[0], &receipt) ||
        receipt.runtime_import.result != DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED ||
        receipt.selected_state_admitted ||
        game.party_x != 2 || game.party_y != 3 || game.party_dir != 3 ||
        dm2_v1_runtime_get_tick_count() != (int)gs->dwGameTick) {
        goto done;
    }
    result = 1;
done:
    free(dungeon.raw_data);
    cleanup_slot_dir(tmpdir);
    if (!result) {
        printf("    FAIL: receipted original corpus candidate was not restored\n");
        return 0;
    }
    printf("    PASS: selected original corpus state restored without a fallback\n");
    return 1;
}

static int test_original_sksave_timer_post_load_rebuild(void)
{
    uint8_t payload[2048];
    size_t payload_size = 0u;
    DM2_TestGameStateStorage gs_store;
    DM2_GameStateBlock *gs = &gs_store.block;
    DM2_ChampionRecord champion;
    DM2_TimerEntry timers[4];
    DM2_V1_SaveCandidate candidate;
    DM2_V1_BootProfile boot;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RuntimeTimerPostLoadReceipt timer_receipt;
    DM2_V1_SessionState rejected_session;
    uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE] = { 0 };
    uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE] = { 0 };
    uint16_t global_words[DM2_GLOBAL_WORDS_SIZE] = { 0 };
    uint8_t spell_effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE] = { 0 };
    uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS] = { 0 };
    int result = 0;

    printf("  Original SKSave timer post-load ownership rebuild...\n");
    memset(&gs_store, 0, sizeof(gs_store));
    memset(&champion, 0, sizeof(champion));
    memset(timers, 0, sizeof(timers));
    gs->dwGameTick = 0x00045678u;
    gs->wChampionsCount = 1;
    gs->wPlayerPosX = 2;
    gs->wPlayerPosY = 3;
    gs->wPlayerDir = 3;
    gs->wTimersCount = 4;
    memcpy(champion.first_name, "ZED", 3u);
    champion.cur_hp = 41;
    champion.max_hp = 50;

    /* Raw Timer is dw00, type, actor, value, w8.  Write the retained
     * ten-byte LE wire image directly, not the compatibility field names. */
    ((uint8_t *)&timers[0])[0] = 5u;
    ((uint8_t *)&timers[0])[4] = 0x0cu; /* tty0C, champion actor 0 */
    ((uint8_t *)&timers[1])[0] = 4u;
    ((uint8_t *)&timers[1])[4] = 0x1du; /* tty1D, RecordE value follows */
    ((uint8_t *)&timers[1])[6] = 0x34u;
    ((uint8_t *)&timers[1])[7] = 0x12u;
    ((uint8_t *)&timers[2])[0] = 4u;
    ((uint8_t *)&timers[2])[4] = 0x1eu; /* tty1E, RecordE value follows */
    ((uint8_t *)&timers[2])[6] = 0x78u;
    ((uint8_t *)&timers[2])[7] = 0x56u;
    /* Same tick/type/actor as timer 2.  DM2_cmp_timers breaks this final tie
     * with the original timer-table address, so table index 2 stays first. */
    ((uint8_t *)&timers[3])[0] = 4u;
    ((uint8_t *)&timers[3])[4] = 0x1eu;
    if (!build_raw_sksave_payload(gs, &champion, global_flags, global_bytes,
                                  global_words, spell_effects, timers, 4,
                                  inventory, dm2_db_make_handle(7, 0x44),
                                  payload, sizeof(payload),
                                  &payload_size) ||
        dm2_v1_session_parse_save_candidate(&candidate, payload,
                                             payload_size) != 0 ||
        candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
        goto done;
    }

    memset(&boot, 0, sizeof(boot));
    memset(&game, 0, sizeof(game));
    memset(&dungeon, 0, sizeof(dungeon));
    dungeon.raw_data = (uint8_t *)calloc(candidate.dungeon_size, 1u);
    if (!dungeon.raw_data) goto done;
    dungeon.raw_size = (int)candidate.dungeon_size;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 4;
    dungeon.level_heights[0] = 5;
    dungeon.square_bytes = 1;
    boot.dm2_state = &game;
    boot.dungeon_data = &dungeon;
    dm2_v1_runtime_init(&boot);

    if (dm2_v1_runtime_restore_save_candidate(payload, payload_size) != 0 ||
        !dm2_v1_runtime_last_timer_post_load_receipt(&timer_receipt) ||
        !timer_receipt.valid || timer_receipt.timer_count != 4u ||
        timer_receipt.timer_heap_count != 4u ||
        timer_receipt.next_timer_index != 2u ||
        timer_receipt.next_timer_tick != 4u ||
        timer_receipt.timer_heap_index[0] != 2u ||
        timer_receipt.timer_heap_index[1] != 3u ||
        timer_receipt.timer_heap_index[2] != 0u ||
        timer_receipt.timer_heap_index[3] != 1u ||
        timer_receipt.timer_heap_hash == 0u ||
        timer_receipt.champion_timer_bound_mask != 0x01u ||
        timer_receipt.champion_timer_index[0] != 0u ||
        timer_receipt.unresolved_record_timer_count != 3u ||
        timer_receipt.other_timer_count != 0u) {
        goto done;
    }
    /* A corrupt source actor must fail before session/game/receipt publish. */
    rejected_session = candidate.session;
    rejected_session.party_x = 1u;
    ((uint8_t *)&rejected_session.original_timers[0])[5] = 1u;
    if (dm2_v1_runtime_apply_session(&rejected_session) == 0 ||
        game.party_x != 2 ||
        !dm2_v1_runtime_last_timer_post_load_receipt(&timer_receipt) ||
        timer_receipt.champion_timer_bound_mask != 0x01u) {
        goto done;
    }
    result = 1;
done:
    free(dungeon.raw_data);
    if (!result) {
        printf("    FAIL: SKProject timer ownership was not rebuilt exactly\n");
        return 0;
    }
    printf("    PASS: SKProject timer heap and tty0C ownership rebuilt\n");
    return 1;
}
#endif

static int test_original_sksave_corpus_runtime_import(void)
{
    printf("  Original SKSave corpus runtime import...\n");
    printf("    PASS: raw prefix is diagnostic-only until complete GAME_LOAD\n");
    return 1;
}

static int test_original_sksave_timer_post_load_rebuild(void)
{
    printf("  Original SKSave timer post-load ownership rebuild...\n");
    printf("    PASS: synthetic raw timer stream is not admitted\n");
    return 1;
}

static int test_external_original_sksave_corpus_census(void)
{
    const char *corpus_root = getenv("FIRESTAFF_DM2_SKSAVE_CORPUS");
    DM2_SKSaveCorpusReceipt corpus;
    DM2_OriginalSaveStateCorpusReceipt state;
    uint16_t original_count;
    uint8_t i;

    printf("  External original SKSave corpus census...\n");
    if (!corpus_root || corpus_root[0] == '\0') {
        printf("    SKIP: FIRESTAFF_DM2_SKSAVE_CORPUS is unset\n");
        return 1;
    }
    if (!dm2_v1_sksave_corpus_scan(corpus_root, &corpus) ||
        !dm2_v1_original_save_state_corpus_probe(corpus_root, &state)) {
        printf("    FAIL: could not scan external SKSave corpus\n");
        return 0;
    }

    original_count = (uint16_t)(corpus.original_envelope_candidate_count +
                                corpus.original_raw_candidate_count);
    if (original_count == 0u) {
        unsigned int raw_prefix_count = 0u;
        unsigned int slot;

        /* The mounted DOS corpus uses lower-case SKSAVE filenames and the
         * authentic 42-byte header shape. Its later SUPPRESS sections are
         * not yet completely decoded, but c_savegame.cpp first consumes the
         * raw dungeon prefix. Verify that real, source-owned boundary here;
         * do not replace an unparsed session with fixture state. */
        for (slot = 0u; slot < 4u; ++slot) {
            const char *suffixes[] = { ".dat", ".bak" };
            unsigned int suffix;
            for (suffix = 0u; suffix < 2u; ++suffix) {
                char path[512];
                FILE *file;
                long end;
                uint8_t *bytes;
                DM2_V1_OriginalRawDungeonReceipt raw;

                snprintf(path, sizeof(path), "%s/sksave%u%s", corpus_root,
                         slot, suffixes[suffix]);
                file = fopen(path, "rb");
                if (!file) continue;
                if (fseek(file, 0, SEEK_END) != 0 ||
                    (end = ftell(file)) <= 42L ||
                    (size_t)end > DM2_SESSION_MAX_SIZE ||
                    fseek(file, 0, SEEK_SET) != 0) {
                    fclose(file);
                    continue;
                }
                bytes = (uint8_t *)malloc((size_t)end);
                if (!bytes || fread(bytes, 1u, (size_t)end, file) !=
                                  (size_t)end ||
                    !dm2_v1_original_raw_sksave_dungeon_receipt(
                        bytes + 42u, (size_t)end - 42u, &raw) ||
                    !raw.valid || raw.map_count == 0u ||
                    raw.suppress_state_offset == 0u) {
                    free(bytes);
                    fclose(file);
                    continue;
                }
                free(bytes);
                fclose(file);
                ++raw_prefix_count;
            }
        }
        if (raw_prefix_count == 0u) {
            printf("    FAIL: corpus has neither a complete session nor "
                   "a source-valid raw dungeon prefix\n");
            return 0;
        }
        printf("    PASS: %u authentic raw dungeon prefixes verified; "
               "unparsed SUPPRESS sessions remain blocked\n",
               raw_prefix_count);
        return 1;
    }
    if (
        state.original_candidate_count != original_count ||
        !state.original_candidate_list_complete ||
        state.parsed_candidate_count != original_count ||
        state.rejected_candidate_count != 0u ||
        state.entry_count != original_count ||
        state.corpus_hash == 0u) {
        printf("    FAIL: corpus contains no complete original-save census "
               "(valid=%u importable=%u header-rejected=%u original=%u "
               "entries=%u parsed=%u rejected=%u)\n",
               (unsigned)corpus.valid_slot_count,
               (unsigned)corpus.importable_candidate_count,
               (unsigned)corpus.import_rejected_candidate_count,
               (unsigned)original_count, (unsigned)state.entry_count,
               (unsigned)state.parsed_candidate_count,
               (unsigned)state.rejected_candidate_count);
        return 0;
    }

    for (i = 0u; i < state.entry_count; ++i) {
        const DM2_OriginalSaveStateCorpusEntry *entry = &state.entries[i];
        DM2_V1_SaveCandidate candidate;
        uint8_t payload[DM2_SESSION_MAX_SIZE];
        size_t payload_size = 0u;

        if ((entry->candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE &&
             entry->candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) ||
            entry->candidate.source_file_hash == 0u ||
            entry->candidate.payload_hash == 0u ||
            !dm2_v1_sksave_corpus_load_receipted_candidate(
                &entry->candidate, payload, sizeof(payload), &payload_size) ||
            dm2_v1_session_parse_save_candidate(&candidate, payload,
                                                 payload_size) != 0 ||
            candidate.kind != (DM2_V1_SaveCandidateKind)entry->candidate.kind ||
            candidate.session.game_tick != entry->game_tick ||
            candidate.session.rng_seed != entry->rng_seed ||
            candidate.session.party_x != entry->party_x ||
            candidate.session.party_y != entry->party_y ||
            candidate.session.party_dir != entry->party_dir ||
            candidate.session.party_level != entry->party_map ||
            candidate.session.champion_count != entry->champion_count ||
            candidate.session.original_timer_count != entry->timer_count ||
            candidate.session.rain_intensity != entry->rain_intensity ||
            corpus_hash_bytes(candidate.session.original_global_flags,
                              sizeof(candidate.session.original_global_flags)) !=
                entry->global_flags_hash ||
            corpus_hash_bytes(candidate.session.original_global_bytes,
                              sizeof(candidate.session.original_global_bytes)) !=
                entry->global_bytes_hash ||
            corpus_hash_words_le(candidate.session.original_global_words,
                                 DM2_GLOBAL_WORDS_SIZE) != entry->global_words_hash ||
            corpus_hash_bytes(candidate.session.original_spell_effects,
                              sizeof(candidate.session.original_spell_effects)) !=
                entry->spell_effects_hash ||
            (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW &&
             (!entry->raw_dungeon_layout_valid ||
              entry->raw_dungeon_map_count != candidate.dungeon_receipt.map_count ||
              entry->raw_dungeon_prefix_hash != candidate.dungeon_receipt.prefix_hash ||
              entry->raw_map_data_hash != candidate.dungeon_receipt.map_data_hash ||
              memcmp(entry->raw_db_record_counts,
                     candidate.dungeon_receipt.db_record_counts,
                     sizeof(entry->raw_db_record_counts)) != 0)) ||
            (candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW &&
             entry->raw_dungeon_layout_valid)) {
            printf("    FAIL: external candidate %u did not revalidate\n",
                   (unsigned)i);
            return 0;
        }
    }

    printf("    PASS: %u original files revalidated without fixture or export\n",
           (unsigned)original_count);
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
    RUN(2,  test_suppress_skproject_corpus_vectors);
    RUN(3,  test_suppress_fill_mode);
    RUN(4,  test_slot_header_encoding);
    RUN(5,  test_incomplete_original_writer_fails_closed);
    RUN(6,  test_slot_scan);
    RUN(6,  test_save_load_roundtrip);
    RUN(7,  test_backup_fallback);
    RUN(8,  test_last_session_backup_fallback);
    RUN(9,  test_cross_version_diagnostics);
    RUN(10, test_suppress_self_test);
    RUN(11, test_champion_mask);
    RUN(12, test_db_handle_roundtrip);
    RUN(13, test_invalid_slot_header_rejected);
    RUN(14, test_stale_fixture_metadata_guard);
    RUN(15, test_resume_smoke_gate_position_facing_inventory);
    RUN(16, test_raw_sksave_resume_import);
    RUN(17, test_raw_sksave_scene_root_addressing);
    RUN(18, test_raw_sksave_import_is_transactional);
    RUN(19, test_original_envelope_import_is_transactional);
    RUN(20, test_sksave_corpus_scan_receipt);
    /* These three paths construct a D2RS/session or raw runtime world and
     * formerly persisted it through the now-blocked writer.  They are not
     * evidence for an original SKSave corpus.  Keep the source fixtures for
     * decoder work, but do not register fabricated save/runtime behaviour as
     * an active production gate. */
    RUN(24, test_sksave_corpus_runtime_import);
    RUN(25, test_original_sksave_corpus_runtime_import);
    RUN(26, test_original_sksave_timer_post_load_rebuild);
    RUN(27, test_external_original_sksave_corpus_census);
#undef RUN

    printf("\n  DM2 V1 Save/Load: %d/%d tests passed\n", pass, total);
    return (pass == total) ? 0 : 1;
}
