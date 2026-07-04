#include "csbwin_resume_fixture.h"

#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdio.h>
#include <string.h>

static uint16_t test_read_le16(const uint8_t *b, size_t off)
{
    return (uint16_t)(((uint16_t)b[off]) |
                      ((uint16_t)b[off + 1u] << 8));
}

static void test_write_le16(uint8_t *b, size_t off, uint16_t v)
{
    b[off] = (uint8_t)(v & 0xFFu);
    b[off + 1u] = (uint8_t)((v >> 8) & 0xFFu);
}

static void test_write_le32(uint8_t *b, size_t off, uint32_t v)
{
    b[off + 0u] = (uint8_t)(v & 0xFFu);
    b[off + 1u] = (uint8_t)((v >> 8) & 0xFFu);
    b[off + 2u] = (uint8_t)((v >> 16) & 0xFFu);
    b[off + 3u] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint16_t test_scramble_block(uint8_t *buf, uint16_t initial_hash,
                                    uint16_t numword)
{
    uint16_t d7 = initial_hash;
    uint16_t d6 = numword;
    uint16_t d5 = initial_hash;
    size_t i;
    for (i = 0u; i < numword; ++i) {
        size_t off = i * 2u;
        uint16_t w = test_read_le16(buf, off);
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

static void test_build_csbwin_header(uint8_t *buf,
                                     const uint8_t *public_bytes_256)
{
    uint16_t d5 = 0u;
    size_t i;
    memset(buf, 0, CSB_V1_CSBWIN_BLOCK1_BYTES);
    memcpy(buf + 256u, public_bytes_256, 256u);
    for (i = 0u; i < 128u; ++i) {
        d5 = (uint16_t)(d5 + test_read_le16(buf + 256u, i * 2u));
    }
    test_write_le16(buf, 254u, d5);
    test_scramble_block(buf + 256u, 0u, 128u);
}

static void test_write_csbwin_champion(uint8_t *record,
                                       const char *name,
                                       const char *title,
                                       uint16_t slot0)
{
    size_t i;
    memset(record, 0, 800u);
    memcpy(record + 0u, name, strlen(name) < 8u ? strlen(name) : 8u);
    memcpy(record + 8u, title, strlen(title) < 16u ? strlen(title) : 16u);
    record[28u] = 2u;
    record[29u] = 3u;
    record[32u] = 5u;
    record[40u] = 1u;
    record[41u] = 23u;
    record[42u] = 4u;
    test_write_le16(record, 44u, 0xFFF0u);
    test_write_le16(record, 46u, 0x0011u);
    test_write_le16(record, 48u, 0x1234u);
    test_write_le16(record, 50u, 0x00A5u);
    test_write_le16(record, 52u, 321u);
    test_write_le16(record, 54u, 456u);
    test_write_le16(record, 56u, 1234u);
    test_write_le16(record, 58u, 2345u);
    test_write_le16(record, 60u, 67u);
    test_write_le16(record, 62u, 89u);
    test_write_le16(record, 66u, 1500u);
    test_write_le16(record, 68u, 1600u);
    for (i = 0u; i < 7u; ++i) {
        record[70u + i * 3u + 0u] = (uint8_t)(90u + i);
        record[70u + i * 3u + 1u] = (uint8_t)(50u + i);
        record[70u + i * 3u + 2u] = (uint8_t)(10u + i);
    }
    for (i = 0u; i < 20u; ++i) {
        test_write_le16(record, 92u + i * 6u, (uint16_t)(0x0100u + i));
        test_write_le32(record, 92u + i * 6u + 2u, 0x10000000u + (uint32_t)i);
    }
    test_write_le16(record, 92u + 0u * 6u, 0u);
    test_write_le32(record, 92u + 0u * 6u + 2u, 2000u);
    test_write_le16(record, 92u + 7u * 6u, 1000u);
    test_write_le32(record, 92u + 7u * 6u + 2u, 8000u);
    for (i = 0u; i < 30u; ++i) {
        test_write_le16(record, 212u + i * 2u, (uint16_t)(slot0 + i));
    }
    test_write_le16(record, 272u, 777u);
}

static void test_write_csbwin_character_tail(uint8_t *characters)
{
    uint8_t *tail = characters + 3200u;
    test_write_le16(tail, 0u, 0x0123u);
    test_write_le16(tail, 4u, 0x0022u);
    test_write_le16(tail, 6u, 0x0033u);
    test_write_le16(tail, 8u, 0x0044u);
}

static void test_write_csbwin_item16(uint8_t *record,
                                     uint16_t monster_index,
                                     uint8_t base)
{
    int i;
    test_write_le16(record, 0u, monster_index);
    for (i = 2; i < 16; ++i) {
        record[i] = (uint8_t)(base + (uint8_t)(i - 2));
    }
}

static void test_write_csbwin_timer(uint8_t *record,
                                    uint32_t time,
                                    uint8_t function,
                                    uint16_t sequence,
                                    uint8_t level)
{
    test_write_le32(record, 0u, time);
    record[4u] = function;
    record[5u] = 0xA5u;
    record[6u] = 0x06u;
    test_write_le16(record, 10u, sequence);
    record[12u] = level;
}

size_t firestaff_test_build_csbwin_resume_fixture(uint8_t *buf,
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

    if (capacity < total) return 0u;
    memset(buf, 0, total);
    memset(public_bytes, 0, sizeof(public_bytes));
    memset(block2, 0, sizeof(block2));

    test_write_le32(block2, 0u, 0x01020304u);
    test_write_le32(block2, 4u, 0xA0B0C0D0u);
    test_write_le16(block2, 8u, 0x4321u);
    test_write_le16(block2, 10u, 2u);
    test_write_le16(block2, 12u, 12u);
    test_write_le16(block2, 14u, 7u);
    test_write_le16(block2, 16u, 3u);
    test_write_le16(block2, 18u, 4u);
    test_write_le16(block2, 20u, 1u);
    test_write_le16(block2, 22u, 0u);
    test_write_le16(block2, 24u, 2u);
    test_write_le16(block2, 26u, 1u);
    test_write_le16(block2, 28u, MAX_TIMERS);
    test_write_le16(block2, 30u, 6u);
    test_write_le32(block2, 32u, 0x11121314u);
    test_write_le32(block2, 36u, 0x21222324u);
    test_write_le16(block2, 40u, 7u);
    test_write_le16(block2, 46u, MAX_ITEM16);
    test_write_le16(block2, 48u, 0x1357u);

    off = CSB_V1_CSBWIN_BLOCK1_BYTES;
    memcpy(buf + off, block2, sizeof(block2));
    block2_checksum = test_scramble_block(buf + off, 0x1111u, 64u);
    off += sizeof(block2);

    test_write_csbwin_item16(buf + off, 0x1234u, 0x20u);
    test_write_csbwin_item16(buf + off + 16u, 0x5678u, 0x40u);
    item16_checksum = test_scramble_block(buf + off, 0x2222u,
                                          (uint16_t)(ITEM16_SIZE / 2));
    off += ITEM16_SIZE;

    test_write_csbwin_champion(buf + off, "TIGGY", "APPRENTICE", 0x2200u);
    test_write_csbwin_champion(buf + off + 800u, "BORIS", "WIZARD", 0x3300u);
    test_write_csbwin_character_tail(buf + off);
    character_checksum = test_scramble_block(buf + off, 0x3333u,
                                             (uint16_t)(CHARACTER_SIZE / 2));
    off += CHARACTER_SIZE;

    test_write_csbwin_timer(buf + off, 0x01020304u, 70u, 0x2222u, 5u);
    test_write_csbwin_timer(buf + off + 16u, 0x11121314u, 78u, 0x3333u, 6u);
    test_write_csbwin_timer(buf + off + 32u, 0x21222324u, 49u, 0x4444u, 7u);
    timers_checksum = test_scramble_block(buf + off, 0x4444u,
                                          (uint16_t)(TIMER_SIZE / 2));
    off += TIMER_SIZE;

    test_write_le16(buf + off, 0u, 2u);
    test_write_le16(buf + off, 2u, 0u);
    test_write_le16(buf + off, 4u, 1u);
    timer_queue_checksum = test_scramble_block(buf + off, 0x5555u,
                                               (uint16_t)(TIMER_QUEUE_SIZE / 2));
    if (corrupt_timer_queue) {
        size_t corrupt_i;
        for (corrupt_i = 0u; corrupt_i < TIMER_QUEUE_SIZE; ++corrupt_i) {
            buf[off + corrupt_i] ^= (uint8_t)(0x5Au + corrupt_i);
        }
    }
    off += TIMER_QUEUE_SIZE;

    public_bytes[CSB_V1_CSBWIN_512_OFF_USELESS] = 1u;
    public_bytes[CSB_V1_CSBWIN_512_OFF_FORMAT_ID] =
        (uint8_t)CSB_V1_FORMAT_DM_AMIGA_36_PC_CSB;
    public_bytes[CSB_V1_CSBWIN_512_OFF_SAVE_AND_PLAY] = 1u;
    test_write_le32(public_bytes, CSB_V1_CSBWIN_512_OFF_GAME_ID, 0x2468ACE0u);
    public_bytes[300u - 256u] = 0x04u;
    public_bytes[301u - 256u] = 0x01u;
    test_write_le32(public_bytes, 308u - 256u, 0x10203040u);
    test_write_le16(public_bytes, 312u - 256u, 0x1111u);
    test_write_le16(public_bytes, 314u - 256u, 0x2222u);
    test_write_le16(public_bytes, 316u - 256u, 0x3333u);
    test_write_le16(public_bytes, 318u - 256u, 0x4444u);
    test_write_le16(public_bytes, 320u - 256u, 0x5555u);
    test_write_le16(public_bytes, 344u - 256u, block2_checksum);
    test_write_le16(public_bytes, 346u - 256u, item16_checksum);
    test_write_le16(public_bytes, 348u - 256u, character_checksum);
    test_write_le16(public_bytes, 350u - 256u, timers_checksum);
    test_write_le16(public_bytes, 352u - 256u, timer_queue_checksum);

    test_build_csbwin_header(buf, public_bytes);
    return off;
}

int firestaff_test_write_csbwin_resume_fixture(const char *path,
                                               int corrupt_timer_queue)
{
    uint8_t bytes[4096];
    size_t size;
    FILE *fp;
    int ok;

    if (!path || path[0] == '\0') {
        return 0;
    }
    size = firestaff_test_build_csbwin_resume_fixture(
        bytes, sizeof(bytes), corrupt_timer_queue);
    if (size == 0u) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    ok = fwrite(bytes, 1u, size, fp) == size && fclose(fp) == 0;
    return ok;
}
