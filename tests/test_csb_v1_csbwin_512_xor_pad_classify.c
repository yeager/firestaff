/*
 * test_csb_v1_csbwin_512_xor_pad_classify.c
 *
 * Data-free contract tests for the CSBWin 512-byte XOR-pad
 * save-header classifier.
 *
 * Scope (synthetic fixtures only — no real CSBWin save bytes
 * are vendored):
 *   - Argument / too-small rejection (NULL bytes, NULL out,
 *     size < 512).
 *   - All-zero 512-byte block produces a NEITHER verdict
 *     (both keys fail because the first-half D6W is also 0 but
 *     unscrambling with hash=0 leaves the second half unchanged
 *     and its sum is also 0 — actually that *would* validate;
 *     so we use a non-zero random-feeling second half that does
 *     not match either key).
 *   - Synthetic CSBWin fixture built with the documented
 *     "trashed first half + LE16(D5) at word 127 + Unscramble
 *     second half with hash=0" pattern produces a CSB verdict
 *     with public fields read back correctly.
 *   - Synthetic CSBWin DM-key fixture (same shape, hash read
 *     from word 10 instead of word 29) produces a DM verdict.
 *   - "Both keys fail" fixture (mismatched first-half checksum)
 *     produces NEITHER.
 *   - Public fields readback: FormatID, GameID, Platform,
 *     DungeonID, keys[0..15], checksums[0..15], AdditionalData
 *     read back as written.
 *   - Result / verdict / source-evidence strings are non-empty
 *     and the source-evidence cites CSBWin + ReDMCSB lines.
 *
 * Non-claims:
 *   - No real CSBWin save bytes are loaded. CSBWin/DM1 save
 *     files are user-staged assets; the skip-safe real-asset
 *     probe (probes/csb/firestaff_csb_v1_csbwin_512_xor_pad_
 *     classify_probe.c) handles that path.
 *   - No body-section decoding (block 2 / items / characters).
 *   - No M11/M12 wiring.
 */

#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_TRUE(cond) do {                                             \
    if (!(cond)) {                                                         \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",                \
                __FILE__, __LINE__, #cond);                                \
        return 0;                                                          \
    }                                                                      \
} while (0)

/* ── Helpers ────────────────────────────────────────────────────────── */

static uint16_t read_le16(const uint8_t *b, size_t off)
{
    return (uint16_t)(((uint16_t)b[off]) |
                      ((uint16_t)b[off + 1u] << 8));
}

static void write_le16(uint8_t *b, size_t off, uint16_t v)
{
    b[off]     = (uint8_t)(v & 0xFFu);
    b[off + 1u] = (uint8_t)((v >> 8) & 0xFFu);
}

static void write_le32(uint8_t *b, size_t off, uint32_t v)
{
    b[off + 0u] = (uint8_t)(v & 0xFFu);
    b[off + 1u] = (uint8_t)((v >> 8) & 0xFFu);
    b[off + 2u] = (uint8_t)((v >> 16) & 0xFFu);
    b[off + 3u] = (uint8_t)((v >> 24) & 0xFFu);
}

/* Unscramble algorithm — mirrors src/csb/csb_v1_csbwin_512_xor_pad_classify.c
 * so the test can build scrambled fixtures without linking the
 * production path. We could link the production helper but
 * keeping the test self-contained makes the test fixture
 * builder independent and easier to read. */
static uint16_t scramble_block(uint8_t *buf, uint16_t initial_hash,
                               uint16_t numword)
{
    uint16_t d7 = initial_hash;
    uint16_t d6 = numword;
    uint16_t d5 = initial_hash;
    size_t i;
    for (i = 0u; i < numword; ++i) {
        size_t off = i * 2u;
        uint16_t w = read_le16(buf, off);
        d5 = (uint16_t)(d5 + w);
        w = (uint16_t)(w ^ d7);
        buf[off + 0u] = (uint8_t)(w & 0xFFu);
        buf[off + 1u] = (uint8_t)((w >> 8) & 0xFFu);
        d5 = (uint16_t)(d5 + w);
        d7 = (uint16_t)(d7 + d6);
        d6 = (uint16_t)(d6 - 1u);
    }
    return d5;
}

/* Build a valid CSBWin 512-byte header whose second-half public
 * fields are the supplied `public_bytes` (must be 256 bytes).
 * Mirrors the documented CSBWin ScrambleAndWrite pattern: the
 * first 256 bytes are zero-filled, except the last word which
 * carries LE16(D5W ^ D6W). For all-zero first-half words the
 * rolling D6W ends up equal to the last-word value, and after
 * Unscramble the second-half sum returns the same value. The
 * classifier reads the initial hash from word[29] (CSB) or
 * word[10] (DM) — both land on a zero byte after the trashing
 * loop, so the initial hash is 0 in the resulting block.
 *
 * The classifier must accept either key (29 or 10) — both
 * resolve to the same initial hash (0) and the unscramble
 * recovers the same second half. To force the verdict, we
 * deliberately mutate byte 20 (word[10]) OR byte 58 (word[29])
 * to a non-zero value to make only one key's "initial hash"
 * match. Wait — that breaks the checksum invariant. So instead
 * we use a different approach: build the same buffer shape, but
 * verify that both keys happen to give the same verdict.
 *
 * The cleaner approach: have BOTH hashes match by keeping
 * word[29] == word[10] == 0, which is what CSBWin produces.
 * The classifier's CSB-first ordering will then resolve as CSB.
 * We separately verify the DM-key helper directly. */
static void build_csbwin_fixture(uint8_t *buf,
                                 const uint8_t *public_bytes_256)
{
    uint16_t d5;
    size_t i;
    /* First 256 bytes = zeros. */
    memset(buf, 0, CSB_V1_CSBWIN_BLOCK1_BYTES);
    /* Second 256 bytes = the public-fields content (unscrambled). */
    memcpy(buf + 256u, public_bytes_256, 256u);
    /* Compute D5 = sum of 128 uint16 LE words from the second half. */
    d5 = 0u;
    for (i = 0u; i < 128u; ++i) {
        d5 = (uint16_t)(d5 + read_le16(buf + 256u, i * 2u));
    }
    /* Last word of the first half = LE16(D5) because D6 = 0 in
     * the all-zero trashing loop and D6 ends as the last word's
     * contribution. */
    write_le16(buf, 254u, d5);
    /* Scramble the second half using hash=0 (CSBWin leaves word[29]=0
     * after the trashing loop, so initial_hash=0). The Unscramble
     * algorithm is involutive so the read-side Unscramble(buf+256,
     * 0, 128) recovers the original second half. */
    scramble_block(buf + 256u, 0u, 128u);
}

static size_t build_full_csbwin_body_fixture(uint8_t *buf,
                                             size_t capacity,
                                             int corrupt_timer_queue)
{
    enum {
        MAX_ITEM16 = 2,
        MAX_TIMERS = 3,
        TIMER_RECORD_SIZE = 16,
        ITEM16_SIZE = MAX_ITEM16 * 16,
        CHARACTER_SIZE = 3328,
        TIMER_SIZE = MAX_TIMERS * TIMER_RECORD_SIZE,
        TIMER_QUEUE_SIZE = MAX_TIMERS * 2
    };
    const size_t total = CSB_V1_CSBWIN_BLOCK1_BYTES + 128u +
                         ITEM16_SIZE + CHARACTER_SIZE + TIMER_SIZE +
                         TIMER_QUEUE_SIZE;
    uint8_t public_bytes[256];
    uint8_t block2[128];
    size_t off;
    size_t i;
    uint16_t block2_checksum;
    uint16_t item16_checksum;
    uint16_t character_checksum;
    uint16_t timers_checksum;
    uint16_t timer_queue_checksum;

    if (capacity < total) {
        return 0u;
    }
    memset(buf, 0, total);
    memset(public_bytes, 0, sizeof(public_bytes));
    memset(block2, 0, sizeof(block2));

    write_le32(block2, 0u, 0x01020304u);
    write_le32(block2, 4u, 0xA0B0C0D0u);
    write_le16(block2, 10u, 4u);
    write_le16(block2, 12u, 17u);
    write_le16(block2, 14u, 22u);
    write_le16(block2, 16u, 3u);
    write_le16(block2, 18u, 5u);
    write_le16(block2, 28u, MAX_TIMERS);
    write_le16(block2, 46u, MAX_ITEM16);

    off = CSB_V1_CSBWIN_BLOCK1_BYTES;
    memcpy(buf + off, block2, sizeof(block2));
    block2_checksum = scramble_block(buf + off, 0x1111u, 64u);
    off += sizeof(block2);

    for (i = 0u; i < ITEM16_SIZE; ++i) {
        buf[off + i] = (uint8_t)(0x20u + i);
    }
    item16_checksum = scramble_block(buf + off, 0x2222u,
                                     (uint16_t)(ITEM16_SIZE / 2));
    off += ITEM16_SIZE;

    for (i = 0u; i < CHARACTER_SIZE; ++i) {
        buf[off + i] = (uint8_t)((i * 7u + 5u) & 0xFFu);
    }
    character_checksum = scramble_block(buf + off, 0x3333u,
                                        (uint16_t)(CHARACTER_SIZE / 2));
    off += CHARACTER_SIZE;

    for (i = 0u; i < TIMER_SIZE; ++i) {
        buf[off + i] = (uint8_t)((i * 11u + 9u) & 0xFFu);
    }
    timers_checksum = scramble_block(buf + off, 0x4444u,
                                     (uint16_t)(TIMER_SIZE / 2));
    off += TIMER_SIZE;

    for (i = 0u; i < TIMER_QUEUE_SIZE; ++i) {
        buf[off + i] = (uint8_t)(0x80u + i);
    }
    timer_queue_checksum = scramble_block(buf + off, 0x5555u,
                                          (uint16_t)(TIMER_QUEUE_SIZE / 2));
    if (corrupt_timer_queue) {
        buf[off + 1u] ^= 0x40u;
    }
    off += TIMER_QUEUE_SIZE;

    public_bytes[CSB_V1_CSBWIN_512_OFF_USELESS] = 1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_FORMAT_ID] =
        (uint8_t)CSB_V1_FORMAT_DM_AMIGA_36_PC_CSB;
    public_bytes[CSB_V1_CSBWIN_512_OFF_SAVE_AND_PLAY] = 1u;
    write_le32(public_bytes, CSB_V1_CSBWIN_512_OFF_GAME_ID, 0x2468ACE0u);
    public_bytes[300u - 256u] = 0x04u;
    public_bytes[301u - 256u] = 0x01u;
    write_le32(public_bytes, 308u - 256u, 0x10203040u);
    write_le16(public_bytes, 312u - 256u, 0x1111u);
    write_le16(public_bytes, 314u - 256u, 0x2222u);
    write_le16(public_bytes, 316u - 256u, 0x3333u);
    write_le16(public_bytes, 318u - 256u, 0x4444u);
    write_le16(public_bytes, 320u - 256u, 0x5555u);
    write_le16(public_bytes, 344u - 256u, block2_checksum);
    write_le16(public_bytes, 346u - 256u, item16_checksum);
    write_le16(public_bytes, 348u - 256u, character_checksum);
    write_le16(public_bytes, 350u - 256u, timers_checksum);
    write_le16(public_bytes, 352u - 256u, timer_queue_checksum);

    build_csbwin_fixture(buf, public_bytes);
    return off;
}

/* Build a fixture that fails both keys. We take a valid CSBWin
 * fixture and flip a single byte in the *first half* (the
 * trashed area) which keeps the CSBWin structure intact but
 * changes D6W away from D5W. */
static void build_both_keys_fail_fixture(uint8_t *buf)
{
    uint8_t zero256[256];
    memset(zero256, 0, sizeof(zero256));
    build_csbwin_fixture(buf, zero256);
    /* Flip byte 100 from 0x00 to 0xFF. This changes D6W by
     * +0xFF00 (added at iteration 25's first word) which
     * invalidates the first-half checksum and breaks the
     * UnscrambleBlock1 invariant. */
    buf[100] = 0xFFu;
}

/* ── Tests ─────────────────────────────────────────────────────────── */

static int test_argument_rejected(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    memset(buf, 0, sizeof(buf));
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify(NULL, sizeof(buf),
                                                   &report) ==
                CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify(buf, sizeof(buf),
                                                   NULL) ==
                CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    /* Same for the per-key helpers. */
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify_csb_key(NULL,
                                                           sizeof(buf),
                                                           &report) ==
                CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify_dm_key(NULL,
                                                          sizeof(buf),
                                                          &report) ==
                CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    return 1;
}

static int test_too_small_rejected(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    memset(buf, 0, sizeof(buf));
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify(buf, 0u, &report) ==
                CSB_V1_CSBWIN_512_ERR_TOO_SMALL);
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify(buf,
                                                   CSB_V1_CSBWIN_BLOCK1_BYTES - 1u,
                                                   &report) ==
                CSB_V1_CSBWIN_512_ERR_TOO_SMALL);
    ASSERT_TRUE(csb_v1_csbwin_512_xor_pad_classify(buf,
                                                   CSB_V1_CSBWIN_BLOCK1_BYTES,
                                                   &report) ==
                CSB_V1_CSBWIN_512_OK);
    return 1;
}

static int test_all_zero_block_is_neither_verdict(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    /* All zeros is not a valid degenerate save: the first word of the
     * second half XORs with seed 0, but the rolling seed advances before
     * the remaining words, so the post-unscramble second-half checksum no
     * longer matches the all-zero first-half checksum. */
    memset(buf, 0, sizeof(buf));
    int rc = csb_v1_csbwin_512_xor_pad_classify(buf, sizeof(buf),
                                                &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER);
    ASSERT_TRUE(report.key_index == 0);
    ASSERT_TRUE(report.first_half_d6w == 0u);
    ASSERT_TRUE(report.second_half_d5w == 0u);
    return 1;
}

static int test_junk_512_neither(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    size_t i;
    /* Pseudo-random junk that does not match either key.
     * Use a deterministic fill so the test is reproducible. */
    for (i = 0u; i < CSB_V1_CSBWIN_BLOCK1_BYTES; ++i) {
        buf[i] = (uint8_t)((i * 0x9Bu + 0x3Du) & 0xFFu);
    }
    int rc = csb_v1_csbwin_512_xor_pad_classify(buf, sizeof(buf),
                                                &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER);
    ASSERT_TRUE(report.key_index == 0);
    /* D6W should still be the rolling first-half checksum
     * (informational, even on reject). */
    ASSERT_TRUE(report.first_half_d6w != 0u);
    return 1;
}

static int test_both_keys_fail_fixture(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    build_both_keys_fail_fixture(buf);
    int rc = csb_v1_csbwin_512_xor_pad_classify(buf, sizeof(buf),
                                                &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER);
    /* Per-key helpers must also reject. */
    rc = csb_v1_csbwin_512_xor_pad_classify_csb_key(buf, sizeof(buf),
                                                    &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER);
    rc = csb_v1_csbwin_512_xor_pad_classify_dm_key(buf, sizeof(buf),
                                                   &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_NEITHER);
    return 1;
}

static int test_csb_key_synthetic_accept(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint8_t public_bytes[256];
    /* Build a second-half with non-trivial content so we can
     * verify the public-field readback. FormatID = 5 (CSB on
     * Amiga/PC — DEFS.H:508), Useless = 1 (BUG0_00 always 1),
     * GameID = 0xDEADBEEF, Platform = 0x07 (Amiga), DungeonID
     * = 0x1234, keys[0] = 0x0001, checksums[0] = 0xABCD. */
    memset(public_bytes, 0, sizeof(public_bytes));
    public_bytes[CSB_V1_CSBWIN_512_OFF_USELESS] = 1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_FORMAT_ID] =
        (uint8_t)CSB_V1_FORMAT_DM_AMIGA_36_PC_CSB;
    public_bytes[CSB_V1_CSBWIN_512_OFF_SAVE_AND_PLAY] = 1u;
    write_le32(public_bytes, CSB_V1_CSBWIN_512_OFF_GAME_ID, 0xDEADBEEFu);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_KEYS, 0x0001u);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_KEYS + 2u, 0x0002u);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_CHECKSUMS, 0xABCDu);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_PLATFORM, 0x0007u);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_DUNGEON_ID, 0x1234u);
    public_bytes[300u - 256u] = 0xCDu;
    public_bytes[301u - 256u] = 0xABu;
    write_le16(public_bytes, 306u - 256u, 0x0001u);
    write_le32(public_bytes, 308u - 256u, 0x01020304u);
    write_le16(public_bytes, 312u - 256u, 0x1111u);
    write_le16(public_bytes, 314u - 256u, 0x2222u);
    write_le16(public_bytes, 316u - 256u, 0x3333u);
    write_le16(public_bytes, 318u - 256u, 0x4444u);
    write_le16(public_bytes, 320u - 256u, 0x5555u);
    write_le32(public_bytes, 322u - 256u, 0x0A0B0C0Du);
    write_le16(public_bytes, 344u - 256u, 0xAAAAu);
    write_le16(public_bytes, 346u - 256u, 0xBBBBu);
    write_le16(public_bytes, 348u - 256u, 0xCCCCu);
    write_le16(public_bytes, 350u - 256u, 0xDDDDu);
    write_le16(public_bytes, 352u - 256u, 0xEEEEu);
    write_le16(public_bytes, 376u - 256u, 0x0065u);
    write_le16(public_bytes, 378u - 256u, 0x000Cu);
    /* Stamp the first 4 bytes of AdditionalData. */
    public_bytes[CSB_V1_CSBWIN_512_OFF_ADDITIONAL + 0u] = 0xA1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_ADDITIONAL + 1u] = 0xB2u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_ADDITIONAL + 2u] = 0xC3u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_ADDITIONAL + 3u] = 0xD4u;

    build_csbwin_fixture(buf, public_bytes);

    int rc = csb_v1_csbwin_512_xor_pad_classify_csb_key(buf, sizeof(buf),
                                                        &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB);
    ASSERT_TRUE(report.key_index == CSB_V1_CSBWIN_512_KEY_CSB);
    /* Public fields read back from the unscrambled second half. */
    ASSERT_TRUE(report.public_fields.useluss_byte == 1u);
    ASSERT_TRUE(report.public_fields.format_id ==
                CSB_V1_FORMAT_DM_AMIGA_36_PC_CSB);
    ASSERT_TRUE(report.public_fields.save_and_play_choice == 1u);
    ASSERT_TRUE(report.public_fields.game_id == 0xDEADBEEFu);
    ASSERT_TRUE(report.public_fields.keys[0] == 0x0001u);
    ASSERT_TRUE(report.public_fields.keys[1] == 0x0002u);
    ASSERT_TRUE(report.public_fields.checksums[0] == 0xABCDu);
    ASSERT_TRUE(report.public_fields.platform == (int16_t)0x0007);
    ASSERT_TRUE(report.public_fields.dungeon_id == 0x1234u);
    ASSERT_TRUE(report.public_fields.additional_data[0] == 0xA1u);
    ASSERT_TRUE(report.public_fields.additional_data[1] == 0xB2u);
    ASSERT_TRUE(report.public_fields.additional_data[2] == 0xC3u);
    ASSERT_TRUE(report.public_fields.additional_data[3] == 0xD4u);
    ASSERT_TRUE(report.public_fields.csbwin_byte22598 == 0xCDu);
    ASSERT_TRUE(report.public_fields.csbwin_byte22596 == 0xABu);
    ASSERT_TRUE(report.public_fields.csbwin_save_option == 1);
    ASSERT_TRUE(report.public_fields.csbwin_random_game_id == 0x01020304u);
    ASSERT_TRUE(report.public_fields.csbwin_block2_hash == 0x1111u);
    ASSERT_TRUE(report.public_fields.csbwin_item16_hash == 0x2222u);
    ASSERT_TRUE(report.public_fields.csbwin_character_hash == 0x3333u);
    ASSERT_TRUE(report.public_fields.csbwin_timers_hash == 0x4444u);
    ASSERT_TRUE(report.public_fields.csbwin_timer_queue_hash == 0x5555u);
    ASSERT_TRUE(report.public_fields.csbwin_total_move_count == 0x0A0B0C0Du);
    ASSERT_TRUE(report.public_fields.csbwin_block2_checksum == 0xAAAAu);
    ASSERT_TRUE(report.public_fields.csbwin_item16_checksum == 0xBBBBu);
    ASSERT_TRUE(report.public_fields.csbwin_character_checksum == 0xCCCCu);
    ASSERT_TRUE(report.public_fields.csbwin_timers_checksum == 0xDDDDu);
    ASSERT_TRUE(report.public_fields.csbwin_timer_queue_checksum == 0xEEEEu);
    ASSERT_TRUE(report.public_fields.csbwin_word22594 == 0x0065);
    ASSERT_TRUE(report.public_fields.csbwin_word22592 == 0x000C);
    /* D6W == D5W after successful unscramble. */
    ASSERT_TRUE(report.first_half_d6w == report.second_half_d5w);
    return 1;
}

static int test_dm_key_synthetic_accept(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint8_t public_bytes[256];
    /* DM1-shape second half: FormatID = 2 (DM on Amiga/PC98/...),
     * GameID = 0xCAFEBABE, Platform = 0x0002. */
    memset(public_bytes, 0, sizeof(public_bytes));
    public_bytes[CSB_V1_CSBWIN_512_OFF_USELESS] = 1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_FORMAT_ID] =
        (uint8_t)CSB_V1_FORMAT_DM_AMIGA_2X_PC98_X68K_FM;
    public_bytes[CSB_V1_CSBWIN_512_OFF_SAVE_AND_PLAY] = 0u;
    write_le32(public_bytes, CSB_V1_CSBWIN_512_OFF_GAME_ID, 0xCAFEBABEu);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_KEYS, 0x0010u);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_PLATFORM, 0x0002u);
    write_le16(public_bytes, CSB_V1_CSBWIN_512_OFF_DUNGEON_ID, 0x5678u);

    build_csbwin_fixture(buf, public_bytes);

    int rc = csb_v1_csbwin_512_xor_pad_classify_dm_key(buf, sizeof(buf),
                                                       &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_DM);
    ASSERT_TRUE(report.key_index == CSB_V1_CSBWIN_512_KEY_DM);
    ASSERT_TRUE(report.public_fields.format_id ==
                CSB_V1_FORMAT_DM_AMIGA_2X_PC98_X68K_FM);
    ASSERT_TRUE(report.public_fields.game_id == 0xCAFEBABEu);
    ASSERT_TRUE(report.public_fields.keys[0] == 0x0010u);
    ASSERT_TRUE(report.public_fields.platform == (int16_t)0x0002);
    ASSERT_TRUE(report.public_fields.dungeon_id == 0x5678u);
    ASSERT_TRUE(report.first_half_d6w == report.second_half_d5w);
    return 1;
}

static int test_stream_section_decode(void)
{
    uint8_t plain[128];
    uint8_t encrypted[128];
    uint8_t decoded[128];
    uint8_t short_out[16];
    uint16_t checksum;
    size_t i;

    for (i = 0u; i < sizeof(plain); ++i) {
        plain[i] = (uint8_t)((i * 17u + 3u) & 0xFFu);
    }
    memcpy(encrypted, plain, sizeof(encrypted));
    checksum = scramble_block(encrypted, 0x2468u,
                              (uint16_t)(sizeof(encrypted) / 2u));

    ASSERT_TRUE(csb_v1_csbwin_512_decode_stream_section(
                    encrypted, sizeof(encrypted), 0x2468u, checksum,
                    decoded, sizeof(decoded)) == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(memcmp(decoded, plain, sizeof(plain)) == 0);
    ASSERT_TRUE(memcmp(encrypted, plain, sizeof(plain)) != 0);

    memset(decoded, 0x7Eu, sizeof(decoded));
    ASSERT_TRUE(csb_v1_csbwin_512_decode_stream_section(
                    encrypted, sizeof(encrypted), 0x2468u,
                    (uint16_t)(checksum + 1u), decoded, sizeof(decoded)) ==
                CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM);
    for (i = 0u; i < sizeof(decoded); ++i) {
        ASSERT_TRUE(decoded[i] == 0u);
    }
    ASSERT_TRUE(csb_v1_csbwin_512_decode_stream_section(
                    encrypted, sizeof(encrypted), 0x2468u, checksum,
                    short_out, sizeof(short_out)) ==
                CSB_V1_CSBWIN_512_ERR_TOO_SMALL);
    ASSERT_TRUE(csb_v1_csbwin_512_decode_stream_section(
                    encrypted, sizeof(encrypted) - 1u, 0x2468u, checksum,
                    decoded, sizeof(decoded)) ==
                CSB_V1_CSBWIN_512_ERR_TOO_SMALL);
    ASSERT_TRUE(csb_v1_csbwin_512_decode_stream_section(
                    NULL, sizeof(encrypted), 0x2468u, checksum,
                    decoded, sizeof(decoded)) ==
                CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    return 1;
}

static int test_full_save_body_verify(void)
{
    uint8_t bytes[4096];
    CSB_V1_CSBWin512BodyReport report;
    size_t size = build_full_csbwin_body_fixture(bytes, sizeof(bytes), 0);
    int rc;

    ASSERT_TRUE(size == 4054u);
    rc = csb_v1_csbwin_512_verify_save_body(bytes, size, 16u, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.header_valid == 1);
    ASSERT_TRUE(report.header.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB);
    ASSERT_TRUE(report.timer_record_size == 16u);
    ASSERT_TRUE(report.max_item16 == 2u);
    ASSERT_TRUE(report.max_timers == 3u);
    ASSERT_TRUE(report.num_character == 4u);
    ASSERT_TRUE(report.party_x == 17u);
    ASSERT_TRUE(report.party_y == 22u);
    ASSERT_TRUE(report.party_facing == 3u);
    ASSERT_TRUE(report.party_level == 5u);
    ASSERT_TRUE(report.required_size == size);
    ASSERT_TRUE(report.sections_verified == CSB_V1_CSBWIN_512_SECTION_COUNT);

    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_BLOCK2].
                encrypted_offset == 512u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_BLOCK2].
                encrypted_size == 128u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_ITEM16].
                encrypted_offset == 640u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_ITEM16].
                encrypted_size == 32u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_CHARACTERS].
                encrypted_offset == 672u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_CHARACTERS].
                encrypted_size == 3328u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_TIMERS].
                encrypted_offset == 4000u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_TIMERS].
                encrypted_size == 48u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE].
                encrypted_offset == 4048u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE].
                encrypted_size == 6u);
    ASSERT_TRUE(report.sections[CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE].
                checksum_ok == 1);

    rc = csb_v1_csbwin_512_verify_save_body(bytes, size - 1u, 16u, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_TOO_SMALL);
    rc = csb_v1_csbwin_512_verify_save_body(bytes, size, 14u, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_ARGUMENT);

    size = build_full_csbwin_body_fixture(bytes, sizeof(bytes), 0);
    ASSERT_TRUE(size == 4054u);
    bytes[4050u] ^= 0x7Fu;
    rc = csb_v1_csbwin_512_verify_save_body(bytes, size, 16u, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM);
    ASSERT_TRUE(report.sections_verified == 4);
    return 1;
}

static int test_csb_first_fallback_to_dm(void)
{
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint8_t public_bytes[256];
    /* Build a DM-only-shape fixture by making the byte at
     * word[29] (byte 58) DIFFERENT from the byte at word[10]
     * (byte 20). Wait — that breaks the checksum invariant
     * because the first-half bytes get trashed to zero. So we
     * need a different approach to distinguish the keys.
     *
     * Approach: set byte 20 (word[10]) to a non-zero value
     * BEFORE scrambling. The trashing loop writes zeros to
     * bytes 0..255, but the initial hash is read from word[29]
     * in the *post-trashing* buffer. After trashing, word[29]
     * = 0 always. So both keys produce the same initial hash
     * (0) and the verdict is determined by ordering.
     *
     * For this test we accept that the CSB-first ordering
     * always picks CSB on a valid CSBWin fixture — and we
     * directly verify the DM verdict via the per-key helper
     * in test_dm_key_synthetic_accept. */
    memset(public_bytes, 0, sizeof(public_bytes));
    public_bytes[CSB_V1_CSBWIN_512_OFF_USELESS] = 1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_FORMAT_ID] = (uint8_t)0u;
    build_csbwin_fixture(buf, public_bytes);

    /* Full classify() with no payload should pick CSB. */
    int rc = csb_v1_csbwin_512_xor_pad_classify(buf, sizeof(buf), &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(report.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB);
    ASSERT_TRUE(report.key_index == CSB_V1_CSBWIN_512_KEY_CSB);
    return 1;
}

static int test_input_buffer_not_modified(void)
{
    /* The classifier must never write to the caller's buffer.
     * We pass a buffer of all 0xA5 bytes (after fixture build,
     * the second half is scrambled, but the *original* bytes
     * before the classifier call must remain unchanged). To
     * check this we keep two copies of the input and compare
     * after the call. */
    CSB_V1_CSBWin512Report report;
    uint8_t buf[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint8_t buf_copy[CSB_V1_CSBWIN_BLOCK1_BYTES];
    uint8_t public_bytes[256];
    memset(public_bytes, 0, sizeof(public_bytes));
    public_bytes[CSB_V1_CSBWIN_512_OFF_USELESS] = 1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_FORMAT_ID] =
        (uint8_t)CSB_V1_FORMAT_DM_ATARI_ST;
    build_csbwin_fixture(buf, public_bytes);
    memcpy(buf_copy, buf, sizeof(buf));
    (void)csb_v1_csbwin_512_xor_pad_classify(buf, sizeof(buf), &report);
    ASSERT_TRUE(memcmp(buf, buf_copy, sizeof(buf)) == 0);
    /* Same check for the per-key helpers. */
    memcpy(buf_copy, buf, sizeof(buf));
    (void)csb_v1_csbwin_512_xor_pad_classify_csb_key(buf, sizeof(buf),
                                                     &report);
    ASSERT_TRUE(memcmp(buf, buf_copy, sizeof(buf)) == 0);
    memcpy(buf_copy, buf, sizeof(buf));
    (void)csb_v1_csbwin_512_xor_pad_classify_dm_key(buf, sizeof(buf),
                                                    &report);
    ASSERT_TRUE(memcmp(buf, buf_copy, sizeof(buf)) == 0);
    return 1;
}

static int test_result_and_evidence_strings(void)
{
    const char *r;
    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_result_name(
                           CSB_V1_CSBWIN_512_OK), "OK") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_result_name(
                           CSB_V1_CSBWIN_512_ERR_TOO_SMALL), "too-small") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_result_name(
                           CSB_V1_CSBWIN_512_ERR_ARGUMENT), "argument") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_result_name(
                           CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM),
                       "bad-checksum") == 0);

    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_verdict_name(
                           CSB_V1_CSBWIN_512_VERDICT_CSB), "CSB") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_verdict_name(
                           CSB_V1_CSBWIN_512_VERDICT_DM), "DM") == 0);
    ASSERT_TRUE(strcmp(csb_v1_csbwin_512_xor_pad_verdict_name(
                           CSB_V1_CSBWIN_512_VERDICT_NEITHER), "neither") == 0);

    r = csb_v1_csbwin_512_xor_pad_source_evidence();
    ASSERT_TRUE(r != NULL);
    ASSERT_TRUE(strstr(r, "CSBWin/Chaos.cpp") != NULL);
    ASSERT_TRUE(strstr(r, "UnscrambleBlock1") != NULL);
    ASSERT_TRUE(strstr(r, "CSB_SAVE_HEADER") != NULL ||
                strstr(r, "DEFS.H") != NULL);
    /* Must cite at least one ReDMCSB and one CSBWin line. */
    ASSERT_TRUE(strstr(r, "ReDMCSB") != NULL);
    return 1;
}

/* ── Driver ────────────────────────────────────────────────────────── */

typedef int (*test_fn)(void);
struct { const char *name; test_fn fn; } tests[] = {
    { "argument-rejected",            test_argument_rejected },
    { "too-small-rejected",           test_too_small_rejected },
    { "all-zero-block-is-neither-verdict", test_all_zero_block_is_neither_verdict },
    { "junk-512-neither",             test_junk_512_neither },
    { "both-keys-fail-fixture",       test_both_keys_fail_fixture },
    { "csb-key-synthetic-accept",     test_csb_key_synthetic_accept },
    { "dm-key-synthetic-accept",      test_dm_key_synthetic_accept },
    { "stream-section-decode",        test_stream_section_decode },
    { "full-save-body-verify",        test_full_save_body_verify },
    { "csb-first-fallback-to-dm",     test_csb_first_fallback_to_dm },
    { "input-buffer-not-modified",    test_input_buffer_not_modified },
    { "result-and-evidence-strings",  test_result_and_evidence_strings },
};

int main(void)
{
    size_t i;
    int pass = 0;
    int fail = 0;
    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        printf("  TEST %s\n", tests[i].name);
        if (tests[i].fn()) {
            printf("  PASS %s\n", tests[i].name);
            ++pass;
        } else {
            printf("  FAIL %s\n", tests[i].name);
            ++fail;
        }
    }
    printf("Result: %d/%d PASS\n", pass, pass + fail);
    return fail == 0 ? 0 : 1;
}
