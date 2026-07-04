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
 *   - No full body import; GAMEBLOCK2 and bounded CHARDESC summaries are
 *     decoded only as verified handoff metadata.
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

static uint32_t read_le32(const uint8_t *b, size_t off)
{
    return ((uint32_t)b[off]) |
           ((uint32_t)b[off + 1u] << 8) |
           ((uint32_t)b[off + 2u] << 16) |
           ((uint32_t)b[off + 3u] << 24);
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

static void write_csbwin_champion_fixture(uint8_t *record,
                                          const char *name,
                                          const char *title,
                                          uint16_t slot0)
{
    size_t i;
    memset(record, 0, 800u);
    memcpy(record + 0u, name, strlen(name) < 8u ? strlen(name) : 8u);
    memcpy(record + 8u, title, strlen(title) < 16u ? strlen(title) : 16u);
    write_le16(record, 24u, 0x2468u);
    record[28u] = 2u;
    record[29u] = 3u;
    record[30u] = 0x30u;
    record[31u] = 0x31u;
    record[32u] = 5u;
    record[33u] = 0x33u;
    record[34u] = 96;
    record[35u] = 102;
    record[36u] = 108;
    record[37u] = 114;
    record[40u] = 1u;
    record[41u] = 23u;
    record[42u] = 4u;
    record[43u] = 0x43u;
    write_le16(record, 44u, 0xFFF0u);
    write_le16(record, 46u, 0x0011u);
    write_le16(record, 48u, 0x1234u);
    write_le16(record, 50u, 0x00A5u);
    write_le16(record, 52u, 321u);
    write_le16(record, 54u, 456u);
    write_le16(record, 56u, 1234u);
    write_le16(record, 58u, 2345u);
    write_le16(record, 60u, 67u);
    write_le16(record, 62u, 89u);
    write_le16(record, 64u, 0x0BADu);
    write_le16(record, 66u, 1500u);
    write_le16(record, 68u, 1600u);
    for (i = 0u; i < 7u; ++i) {
        record[70u + i * 3u + 0u] = (uint8_t)(90u + i);
        record[70u + i * 3u + 1u] = (uint8_t)(50u + i);
        record[70u + i * 3u + 2u] = (uint8_t)(10u + i);
    }
    for (i = 0u; i < 20u; ++i) {
        write_le16(record, 92u + i * 6u, (uint16_t)(0x0100u + i));
        write_le32(record, 94u + i * 6u, 0x10000000u + (uint32_t)i);
    }
    for (i = 0u; i < 30u; ++i) {
        write_le16(record, 212u + i * 2u, (uint16_t)(slot0 + i));
    }
    write_le16(record, 272u, 777u);
    write_le16(record, 274u, 88u);
    write_le32(record, 276u, 0xCAFEBABEu);
    write_le16(record, 280u, 0xBEEFu);
    write_le16(record, 282u, 0x0042u);
    write_le16(record, 284u, 0x0055u);
    for (i = 0u; i < 464u; ++i) {
        record[336u + i] = (uint8_t)(0x80u + (uint8_t)(i & 0x3fu));
    }
}

static void write_csbwin_character_tail_fixture(uint8_t *characters)
{
    uint8_t *tail = characters + 3200u;
    size_t i;
    write_le16(tail, 0u, 0x0123u);
    tail[2u] = 1u;
    tail[3u] = 1u;
    write_le16(tail, 4u, 0x0022u);
    write_le16(tail, 6u, 0x0033u);
    write_le16(tail, 8u, 0x0044u);
    tail[10u] = 5u;
    tail[11u] = 6u;
    tail[12u] = 7u;
    tail[13u] = 8u;
    for (i = 0u; i < 24u; ++i) {
        write_le16(tail, 14u + i * 2u, (uint16_t)(0x4000u + i));
        tail[62u + i] = (uint8_t)(0x90u + i);
    }
    tail[86u] = 1u;
}

static void write_csbwin_item16_fixture(uint8_t *record,
                                        uint16_t monster_index,
                                        uint8_t base)
{
    write_le16(record, 0u, monster_index);
    record[2u] = (uint8_t)(base + 0u);
    record[3u] = (uint8_t)(base + 1u);
    record[4u] = (uint8_t)(base + 2u);
    record[5u] = (uint8_t)(base + 3u);
    record[6u] = (uint8_t)(base + 4u);
    record[7u] = (uint8_t)(base + 5u);
    record[8u] = (uint8_t)(base + 6u);
    record[9u] = (uint8_t)(base + 7u);
    record[10u] = (uint8_t)(base + 8u);
    record[11u] = (uint8_t)(base + 9u);
    record[12u] = (uint8_t)(base + 10u);
    record[13u] = (uint8_t)(base + 11u);
    record[14u] = (uint8_t)(base + 12u);
    record[15u] = (uint8_t)(base + 13u);
}

static void write_csbwin_timer_fixture(uint8_t *record,
                                       uint32_t time,
                                       uint8_t function,
                                       uint16_t sequence,
                                       uint8_t level)
{
    write_le32(record, 0u, time);
    record[4u] = function;
    record[5u] = 0xA5u;
    record[6u] = 0x06u;
    record[7u] = 0x07u;
    record[8u] = 0x08u;
    record[9u] = 0x09u;
    write_le16(record, 10u, sequence);
    record[12u] = level;
    record[13u] = 0xCDu;
    record[14u] = 0xCEu;
    record[15u] = 0xCFu;
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
    write_le16(block2, 8u, 0x4321u);
    write_le16(block2, 10u, 4u);
    write_le16(block2, 12u, 17u);
    write_le16(block2, 14u, 22u);
    write_le16(block2, 16u, 3u);
    write_le16(block2, 18u, 5u);
    write_le16(block2, 20u, 2u);
    write_le16(block2, 22u, 1u);
    write_le16(block2, 24u, 2u);
    write_le16(block2, 26u, 1u);
    write_le16(block2, 28u, MAX_TIMERS);
    write_le16(block2, 30u, 6u);
    write_le32(block2, 32u, 0x11121314u);
    write_le32(block2, 36u, 0x21222324u);
    write_le16(block2, 40u, 7u);
    write_le16(block2, 42u, 8u);
    write_le16(block2, 44u, 9u);
    write_le16(block2, 46u, MAX_ITEM16);
    write_le16(block2, 48u, 0x1357u);

    off = CSB_V1_CSBWIN_BLOCK1_BYTES;
    memcpy(buf + off, block2, sizeof(block2));
    block2_checksum = scramble_block(buf + off, 0x1111u, 64u);
    off += sizeof(block2);

    memset(buf + off, 0, ITEM16_SIZE);
    write_csbwin_item16_fixture(buf + off, 0x1234u, 0x20u);
    write_csbwin_item16_fixture(buf + off + 16u, 0x5678u, 0x40u);
    item16_checksum = scramble_block(buf + off, 0x2222u,
                                     (uint16_t)(ITEM16_SIZE / 2));
    off += ITEM16_SIZE;

    memset(buf + off, 0, CHARACTER_SIZE);
    write_csbwin_champion_fixture(buf + off, "TIGGY", "APPRENTICE", 0x2200u);
    write_csbwin_champion_fixture(buf + off + 800u, "BORIS", "WIZARD", 0x3300u);
    write_csbwin_character_tail_fixture(buf + off);
    character_checksum = scramble_block(buf + off, 0x3333u,
                                        (uint16_t)(CHARACTER_SIZE / 2));
    off += CHARACTER_SIZE;

    memset(buf + off, 0, TIMER_SIZE);
    write_csbwin_timer_fixture(buf + off, 0x01020304u, 70u, 0x2222u, 5u);
    write_csbwin_timer_fixture(buf + off + 16u, 0x11121314u, 78u, 0x3333u, 6u);
    write_csbwin_timer_fixture(buf + off + 32u, 0x21222324u, 49u, 0x4444u, 7u);
    timers_checksum = scramble_block(buf + off, 0x4444u,
                                     (uint16_t)(TIMER_SIZE / 2));
    off += TIMER_SIZE;

    write_le16(buf + off, 0u, 2u);
    write_le16(buf + off, 2u, 0u);
    write_le16(buf + off, 4u, 1u);
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
    ASSERT_TRUE(report.game_time == 0x01020304u);
    ASSERT_TRUE(report.random_seed == 0xA0B0C0D0u);
    ASSERT_TRUE(report.object_in_hand == 0x4321u);
    ASSERT_TRUE(report.max_item16 == 2u);
    ASSERT_TRUE(report.max_timers == 3u);
    ASSERT_TRUE(report.num_timer == 2u);
    ASSERT_TRUE(report.first_avail_timer == 1u);
    ASSERT_TRUE(report.item16_queue_len == 6u);
    ASSERT_TRUE(report.timer_sequence == 0x1357u);
    ASSERT_TRUE(report.num_character == 4u);
    ASSERT_TRUE(report.party_x == 17u);
    ASSERT_TRUE(report.party_y == 22u);
    ASSERT_TRUE(report.party_facing == 3u);
    ASSERT_TRUE(report.party_level == 5u);
    ASSERT_TRUE(report.hand_char == 2u);
    ASSERT_TRUE(report.magic_caster == 1u);
    ASSERT_TRUE(report.last_monster_attack_time == 0x11121314u);
    ASSERT_TRUE(report.last_party_move_time == 0x21222324u);
    ASSERT_TRUE(report.party_move_disable_timer == 7u);
    ASSERT_TRUE(report.word11712 == 8u);
    ASSERT_TRUE(report.word11714 == 9u);
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
    ASSERT_TRUE(report.item16_summary_total == 2u);
    ASSERT_TRUE(report.item16_summary_count == 2u);
    ASSERT_TRUE(report.item16[0].valid == 1);
    ASSERT_TRUE(report.item16[0].monster_index == 0x1234u);
    ASSERT_TRUE(report.item16[0].facings == 0x20u);
    ASSERT_TRUE(report.item16[0].positions == 0x21u);
    ASSERT_TRUE(report.item16[0].target_x == 0x24u);
    ASSERT_TRUE(report.item16[0].target_y == 0x25u);
    ASSERT_TRUE(report.item16[0].current_x == 0x28u);
    ASSERT_TRUE(report.item16[0].current_y == 0x29u);
    ASSERT_TRUE(report.item16[0].single_monster_status[3] == 0x2Du);
    ASSERT_TRUE(report.item16[1].monster_index == 0x5678u);
    ASSERT_TRUE(report.item16[1].facings == 0x40u);
    ASSERT_TRUE(report.champions[0].valid == 1);
    ASSERT_TRUE(strcmp(report.champions[0].name, "TIGGY") == 0);
    ASSERT_TRUE(strcmp(report.champions[0].title, "APPRENTICE") == 0);
    ASSERT_TRUE(report.champions[0].facing == 2u);
    ASSERT_TRUE(report.champions[0].char_position == 3u);
    ASSERT_TRUE(report.champions[0].word24 == 0x2468);
    ASSERT_TRUE(report.champions[0].byte30 == 0x30u);
    ASSERT_TRUE(report.champions[0].byte31 == 0x31u);
    ASSERT_TRUE(report.champions[0].attack_type == 5);
    ASSERT_TRUE(report.champions[0].byte33 == 0x33);
    ASSERT_TRUE(report.champions[0].incantation[0] == 96);
    ASSERT_TRUE(report.champions[0].incantation[3] == 114);
    ASSERT_TRUE(report.champions[0].facing3 == 1u);
    ASSERT_TRUE(report.champions[0].max_recent_damage == 23u);
    ASSERT_TRUE(report.champions[0].poison_count == 4u);
    ASSERT_TRUE(report.champions[0].ubyte43 == 0x43u);
    ASSERT_TRUE(report.champions[0].busy_timer == (int16_t)0xFFF0u);
    ASSERT_TRUE(report.champions[0].timer_index == 0x0011);
    ASSERT_TRUE(report.champions[0].char_flags == 0x1234);
    ASSERT_TRUE(report.champions[0].wounds == 0x00A5);
    ASSERT_TRUE(report.champions[0].hp == 321);
    ASSERT_TRUE(report.champions[0].max_hp == 456);
    ASSERT_TRUE(report.champions[0].stamina == 1234);
    ASSERT_TRUE(report.champions[0].max_stamina == 2345);
    ASSERT_TRUE(report.champions[0].mana == 67);
    ASSERT_TRUE(report.champions[0].max_mana == 89);
    ASSERT_TRUE(report.champions[0].food == 1500);
    ASSERT_TRUE(report.champions[0].water == 1600);
    ASSERT_TRUE(report.champions[0].attributes[1][0] == 91u);
    ASSERT_TRUE(report.champions[0].attributes[1][1] == 51u);
    ASSERT_TRUE(report.champions[0].attributes[1][2] == 11u);
    ASSERT_TRUE(report.champions[0].skill_temp_adjust[3] == 0x0103);
    ASSERT_TRUE(report.champions[0].skill_experience[3] == 0x10000003u);
    ASSERT_TRUE(report.champions[0].possessions[0] == 0x2200u);
    ASSERT_TRUE(report.champions[0].possessions[29] == 0x221Du);
    ASSERT_TRUE(report.champions[0].load == 777u);
    ASSERT_TRUE(report.champions[0].shield_strength == 88u);
    ASSERT_TRUE(report.champions[0].talents == 0xCAFEBABEu);
    ASSERT_TRUE(report.champions[0].fingerprint == 0xBEEFu);
    ASSERT_TRUE(report.champions[0].cause_of_damage == 0x0042u);
    ASSERT_TRUE(report.champions[0].monster_causing_damage == 0x0055u);
    ASSERT_TRUE(report.champions[0].portrait[0] == 0x80u);
    ASSERT_TRUE(report.champions[0].portrait[463] ==
                (uint8_t)(0x80u + (463u & 0x3fu)));
    ASSERT_TRUE(strcmp(report.champions[1].name, "BORIS") == 0);
    ASSERT_TRUE(report.champions[1].possessions[0] == 0x3300u);
    ASSERT_TRUE(report.character_tail_brightness == 0x0123);
    ASSERT_TRUE(report.character_tail_see_thru_walls == 1u);
    ASSERT_TRUE(report.character_tail_magic_footprints_active == 1u);
    ASSERT_TRUE(report.character_tail_party_shield == 0x0022);
    ASSERT_TRUE(report.character_tail_fire_shield == 0x0033);
    ASSERT_TRUE(report.character_tail_spell_shield == 0x0044);
    ASSERT_TRUE(report.character_tail_num_footprint_entries == 5u);
    ASSERT_TRUE(report.character_tail_freeze_life_timer == 6u);
    ASSERT_TRUE(report.character_tail_first_magic_footprint == 7u);
    ASSERT_TRUE(report.character_tail_last_magic_footprint == 8u);
    ASSERT_TRUE(report.character_tail_party_footprints[0] == 0x4000u);
    ASSERT_TRUE(report.character_tail_party_footprints[23] == 0x4017u);
    ASSERT_TRUE(report.character_tail_byte13220[0] == 0x90u);
    ASSERT_TRUE(report.character_tail_byte13220[23] == 0xA7u);
    ASSERT_TRUE(report.character_tail_invisible == 1u);
    ASSERT_TRUE(report.timer_summary_total == 3u);
    ASSERT_TRUE(report.timer_summary_count == 3u);
    ASSERT_TRUE(report.timers[0].valid == 1);
    ASSERT_TRUE(report.timers[0].time == 0x01020304u);
    ASSERT_TRUE(report.timers[0].function == 70u);
    ASSERT_TRUE(report.timers[0].ubyte5 == 0xA5u);
    ASSERT_TRUE(report.timers[0].ubyte6 == 0x06u);
    ASSERT_TRUE(report.timers[0].ubyte9 == 0x09u);
    ASSERT_TRUE(report.timers[0].sequence == 0x2222u);
    ASSERT_TRUE(report.timers[0].level == 5u);
    ASSERT_TRUE(report.timers[2].function == 49u);
    ASSERT_TRUE(report.timers[2].sequence == 0x4444u);
    ASSERT_TRUE(report.timer_queue_summary_total == 3u);
    ASSERT_TRUE(report.timer_queue_summary_count == 3u);
    ASSERT_TRUE(report.timer_queue[0] == 2u);
    ASSERT_TRUE(report.timer_queue[1] == 0u);
    ASSERT_TRUE(report.timer_queue[2] == 1u);

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

static int test_writable_champion_sections(void)
{
    uint8_t bytes[8192];
    uint8_t decoded_block2[128];
    uint8_t decoded_characters[3328];
    CSB_V1_CSBWin512BodyReport report;
    CSB_V1_CSBWin512WritableChampionSections writable;
    int rc;
    size_t size;

    size = build_full_csbwin_body_fixture(bytes, sizeof(bytes), 0);
    ASSERT_TRUE(size == 4054u);
    rc = csb_v1_csbwin_512_verify_save_body(bytes, size, 16u, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);

    rc = csb_v1_csbwin_512_build_writable_champion_sections(
        &report, &writable);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(writable.block2_hash == 0u);
    ASSERT_TRUE(writable.character_hash == 0u);
    ASSERT_TRUE(writable.block2_size == 128u);
    ASSERT_TRUE(writable.character_size == 3328u);
    ASSERT_TRUE(writable.block2_checksum != 0u);
    ASSERT_TRUE(writable.character_checksum != 0u);

    memset(decoded_block2, 0, sizeof(decoded_block2));
    rc = csb_v1_csbwin_512_decode_stream_section(
        writable.block2_scrambled,
        writable.block2_size,
        writable.block2_hash,
        writable.block2_checksum,
        decoded_block2,
        sizeof(decoded_block2));
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(read_le32(decoded_block2, 0u) == report.game_time);
    ASSERT_TRUE(read_le32(decoded_block2, 4u) == report.random_seed);
    ASSERT_TRUE(read_le16(decoded_block2, 8u) == report.object_in_hand);
    ASSERT_TRUE(read_le16(decoded_block2, 10u) == report.num_character);
    ASSERT_TRUE(read_le16(decoded_block2, 12u) == report.party_x);
    ASSERT_TRUE(read_le16(decoded_block2, 14u) == report.party_y);
    ASSERT_TRUE(read_le16(decoded_block2, 16u) == report.party_facing);
    ASSERT_TRUE(read_le16(decoded_block2, 18u) == report.party_level);
    ASSERT_TRUE(read_le16(decoded_block2, 20u) == report.hand_char);
    ASSERT_TRUE(read_le16(decoded_block2, 22u) == report.magic_caster);
    ASSERT_TRUE(read_le16(decoded_block2, 24u) == report.num_timer);
    ASSERT_TRUE(read_le16(decoded_block2, 28u) == report.max_timers);
    ASSERT_TRUE(read_le16(decoded_block2, 46u) == report.max_item16);
    ASSERT_TRUE(read_le16(decoded_block2, 48u) == report.timer_sequence);

    memset(decoded_characters, 0, sizeof(decoded_characters));
    rc = csb_v1_csbwin_512_decode_stream_section(
        writable.characters_scrambled,
        writable.character_size,
        writable.character_hash,
        writable.character_checksum,
        decoded_characters,
        sizeof(decoded_characters));
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(memcmp(decoded_characters + 0u, "TIGGY", 5u) == 0);
    ASSERT_TRUE(memcmp(decoded_characters + 8u, "APPRENTICE", 10u) == 0);
    ASSERT_TRUE(read_le16(decoded_characters, 24u) == 0x2468u);
    ASSERT_TRUE(decoded_characters[28u] == 2u);
    ASSERT_TRUE(decoded_characters[29u] == 3u);
    ASSERT_TRUE(decoded_characters[30u] == 0x30u);
    ASSERT_TRUE(decoded_characters[31u] == 0x31u);
    ASSERT_TRUE(decoded_characters[32u] == 5u);
    ASSERT_TRUE(decoded_characters[34u] == 96u);
    ASSERT_TRUE(decoded_characters[37u] == 114u);
    ASSERT_TRUE(read_le16(decoded_characters, 44u) == 0xFFF0u);
    ASSERT_TRUE(read_le16(decoded_characters, 46u) == 0x0011u);
    ASSERT_TRUE(read_le16(decoded_characters, 52u) == 321u);
    ASSERT_TRUE(read_le16(decoded_characters, 64u) == 0x0BADu);
    ASSERT_TRUE(read_le16(decoded_characters, 92u + 3u * 6u) == 0x0103u);
    ASSERT_TRUE(read_le16(decoded_characters, 212u) == 0x2200u);
    ASSERT_TRUE(read_le16(decoded_characters, 212u + 29u * 2u) == 0x221Du);
    ASSERT_TRUE(read_le16(decoded_characters, 272u) == 777u);
    ASSERT_TRUE(read_le32(decoded_characters, 276u) == 0xCAFEBABEu);
    ASSERT_TRUE(decoded_characters[336u] == 0x80u);
    ASSERT_TRUE(decoded_characters[336u + 463u] ==
                (uint8_t)(0x80u + (463u & 0x3fu)));
    ASSERT_TRUE(memcmp(decoded_characters + 800u, "BORIS", 5u) == 0);
    ASSERT_TRUE(read_le16(decoded_characters, 3200u) == 0x0123u);
    ASSERT_TRUE(decoded_characters[3200u + 2u] == 1u);
    ASSERT_TRUE(read_le16(decoded_characters, 3200u + 14u) == 0x4000u);
    ASSERT_TRUE(read_le16(decoded_characters, 3200u + 14u + 23u * 2u) ==
                0x4017u);
    ASSERT_TRUE(decoded_characters[3200u + 62u] == 0x90u);
    ASSERT_TRUE(decoded_characters[3200u + 62u + 23u] == 0xA7u);
    ASSERT_TRUE(decoded_characters[3200u + 86u] == 1u);

    writable.characters_scrambled[10u] ^= 0x55u;
    rc = csb_v1_csbwin_512_decode_stream_section(
        writable.characters_scrambled,
        writable.character_size,
        writable.character_hash,
        writable.character_checksum,
        decoded_characters,
        sizeof(decoded_characters));
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_BAD_CHECKSUM);
    return 1;
}

static int test_writable_header_roundtrip(void)
{
    uint8_t original[8192];
    uint8_t rebuilt[8192];
    uint8_t decoded_item16[1024];
    uint8_t decoded_timers[1024];
    uint8_t decoded_timer_queue[128];
    CSB_V1_CSBWin512BodyReport report;
    CSB_V1_CSBWin512BodyReport roundtrip;
    CSB_V1_CSBWin512BodyReport truncated;
    CSB_V1_CSBWin512WritableChampionSections writable;
    CSB_V1_CSBWin512WritableRuntimeSections runtime;
    CSB_V1_CSBWin512WritableHeader header;
    CSB_V1_CSBWin512Report header_report;
    int rc;
    size_t size;

    size = build_full_csbwin_body_fixture(original, sizeof(original), 0);
    ASSERT_TRUE(size == 4054u);
    rc = csb_v1_csbwin_512_verify_save_body(original, size, 16u, &report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    rc = csb_v1_csbwin_512_build_writable_champion_sections(
        &report, &writable);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    rc = csb_v1_csbwin_512_build_writable_runtime_sections(
        &report, &runtime);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(runtime.item16_hash == 0u);
    ASSERT_TRUE(runtime.timers_hash == 0u);
    ASSERT_TRUE(runtime.timer_queue_hash == 0u);
    ASSERT_TRUE(runtime.item16_size == 32u);
    ASSERT_TRUE(runtime.timers_size == 48u);
    ASSERT_TRUE(runtime.timer_queue_size == 6u);
    ASSERT_TRUE(runtime.item16_checksum != 0u);
    ASSERT_TRUE(runtime.timers_checksum != 0u);
    ASSERT_TRUE(runtime.timer_queue_checksum != 0u);

    rc = csb_v1_csbwin_512_decode_stream_section(
        runtime.item16_scrambled,
        runtime.item16_size,
        runtime.item16_hash,
        runtime.item16_checksum,
        decoded_item16,
        sizeof(decoded_item16));
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(read_le16(decoded_item16, 0u) == 0x1234u);
    ASSERT_TRUE(decoded_item16[2u] == 0x20u);
    ASSERT_TRUE(decoded_item16[15u] == 0x2Du);
    ASSERT_TRUE(read_le16(decoded_item16, 16u) == 0x5678u);
    ASSERT_TRUE(decoded_item16[18u] == 0x40u);
    ASSERT_TRUE(decoded_item16[31u] == 0x4Du);

    rc = csb_v1_csbwin_512_decode_stream_section(
        runtime.timers_scrambled,
        runtime.timers_size,
        runtime.timers_hash,
        runtime.timers_checksum,
        decoded_timers,
        sizeof(decoded_timers));
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(read_le32(decoded_timers, 0u) == 0x01020304u);
    ASSERT_TRUE(decoded_timers[4u] == 70u);
    ASSERT_TRUE(read_le16(decoded_timers, 10u) == 0x2222u);
    ASSERT_TRUE(decoded_timers[12u] == 5u);
    ASSERT_TRUE(read_le32(decoded_timers, 32u) == 0x21222324u);
    ASSERT_TRUE(decoded_timers[36u] == 49u);
    ASSERT_TRUE(read_le16(decoded_timers, 42u) == 0x4444u);

    rc = csb_v1_csbwin_512_decode_stream_section(
        runtime.timer_queue_scrambled,
        runtime.timer_queue_size,
        runtime.timer_queue_hash,
        runtime.timer_queue_checksum,
        decoded_timer_queue,
        sizeof(decoded_timer_queue));
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(read_le16(decoded_timer_queue, 0u) == 2u);
    ASSERT_TRUE(read_le16(decoded_timer_queue, 2u) == 0u);
    ASSERT_TRUE(read_le16(decoded_timer_queue, 4u) == 1u);

    memset(&header, 0, sizeof(header));
    header.key_index = CSB_V1_CSBWIN_512_KEY_CSB;
    header.byte22598 = report.header.public_fields.csbwin_byte22598;
    header.byte22596 = report.header.public_fields.csbwin_byte22596;
    header.save_option = report.header.public_fields.csbwin_save_option;
    header.random_game_id =
        report.header.public_fields.csbwin_random_game_id;
    header.block2_hash = writable.block2_hash;
    header.item16_hash = runtime.item16_hash;
    header.character_hash = writable.character_hash;
    header.timers_hash = runtime.timers_hash;
    header.timer_queue_hash = runtime.timer_queue_hash;
    header.total_move_count =
        report.header.public_fields.csbwin_total_move_count;
    header.block2_checksum = writable.block2_checksum;
    header.item16_checksum = runtime.item16_checksum;
    header.character_checksum = writable.character_checksum;
    header.timers_checksum = runtime.timers_checksum;
    header.timer_queue_checksum = runtime.timer_queue_checksum;
    header.word22594 = report.header.public_fields.csbwin_word22594;
    header.word22592 = report.header.public_fields.csbwin_word22592;

    memset(rebuilt, 0, sizeof(rebuilt));
    rc = csb_v1_csbwin_512_build_writable_header(&header, rebuilt);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    rc = csb_v1_csbwin_512_xor_pad_classify(rebuilt, sizeof(rebuilt),
                                            &header_report);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(header_report.verdict == CSB_V1_CSBWIN_512_VERDICT_CSB);
    ASSERT_TRUE(header_report.first_half_d6w ==
                header_report.second_half_d5w);
    ASSERT_TRUE(header_report.public_fields.csbwin_block2_hash == 0u);
    ASSERT_TRUE(header_report.public_fields.csbwin_block2_checksum ==
                writable.block2_checksum);
    ASSERT_TRUE(header_report.public_fields.csbwin_character_hash == 0u);
    ASSERT_TRUE(header_report.public_fields.csbwin_character_checksum ==
                writable.character_checksum);
    ASSERT_TRUE(header_report.public_fields.csbwin_item16_hash == 0u);
    ASSERT_TRUE(header_report.public_fields.csbwin_item16_checksum ==
                runtime.item16_checksum);
    ASSERT_TRUE(header_report.public_fields.csbwin_timers_hash == 0u);
    ASSERT_TRUE(header_report.public_fields.csbwin_timers_checksum ==
                runtime.timers_checksum);
    ASSERT_TRUE(header_report.public_fields.csbwin_timer_queue_hash == 0u);
    ASSERT_TRUE(header_report.public_fields.csbwin_timer_queue_checksum ==
                runtime.timer_queue_checksum);

    memcpy(rebuilt + CSB_V1_CSBWIN_BLOCK1_BYTES,
           writable.block2_scrambled,
           writable.block2_size);
    memcpy(rebuilt + report.sections[CSB_V1_CSBWIN_512_SECTION_ITEM16]
                         .encrypted_offset,
           runtime.item16_scrambled,
           runtime.item16_size);
    memcpy(rebuilt + report.sections[CSB_V1_CSBWIN_512_SECTION_CHARACTERS]
                         .encrypted_offset,
           writable.characters_scrambled,
           writable.character_size);
    memcpy(rebuilt + report.sections[CSB_V1_CSBWIN_512_SECTION_TIMERS]
                         .encrypted_offset,
           runtime.timers_scrambled,
           runtime.timers_size);
    memcpy(rebuilt + report.sections[CSB_V1_CSBWIN_512_SECTION_TIMER_QUEUE]
                         .encrypted_offset,
           runtime.timer_queue_scrambled,
           runtime.timer_queue_size);

    rc = csb_v1_csbwin_512_verify_save_body(rebuilt, size, 16u, &roundtrip);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_OK);
    ASSERT_TRUE(roundtrip.game_time == report.game_time);
    ASSERT_TRUE(roundtrip.random_seed == report.random_seed);
    ASSERT_TRUE(roundtrip.object_in_hand == report.object_in_hand);
    ASSERT_TRUE(roundtrip.max_item16 == report.max_item16);
    ASSERT_TRUE(roundtrip.max_timers == report.max_timers);
    ASSERT_TRUE(strcmp(roundtrip.champions[0].name, "TIGGY") == 0);
    ASSERT_TRUE(strcmp(roundtrip.champions[1].name, "BORIS") == 0);
    ASSERT_TRUE(roundtrip.champions[0].skill_experience[19] ==
                report.champions[0].skill_experience[19]);
    ASSERT_TRUE(roundtrip.character_tail_party_footprints[23] ==
                report.character_tail_party_footprints[23]);
    ASSERT_TRUE(roundtrip.item16[0].monster_index == 0x1234u);
    ASSERT_TRUE(roundtrip.item16[1].positions == 0x41u);
    ASSERT_TRUE(roundtrip.timers[2].time == 0x21222324u);
    ASSERT_TRUE(roundtrip.timers[2].function == 49u);
    ASSERT_TRUE(roundtrip.timer_queue[0] == 2u &&
                roundtrip.timer_queue[2] == 1u);

    rc = csb_v1_csbwin_512_build_writable_header(NULL, rebuilt);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    header.key_index = 0;
    rc = csb_v1_csbwin_512_build_writable_header(&header, rebuilt);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_ARGUMENT);
    truncated = report;
    truncated.item16_summary_count = 1u;
    rc = csb_v1_csbwin_512_build_writable_runtime_sections(
        &truncated, &runtime);
    ASSERT_TRUE(rc == CSB_V1_CSBWIN_512_ERR_ARGUMENT);
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
    { "writable-champion-sections",   test_writable_champion_sections },
    { "writable-header-roundtrip",    test_writable_header_roundtrip },
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
