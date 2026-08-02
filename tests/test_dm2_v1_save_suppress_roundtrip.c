/* DM2 SUPPRESS codec round-trip test: write -> flush -> read -> verify.
 * Exercises the save game buffer mask, hero mask, and arbitrary data
 * through the full writer/reader pipeline. */
#include "dm2_v1_save_load.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_savegame_buffer_roundtrip(void)
{
    uint8_t original[60], recovered[60], encoded[256];
    const uint8_t *mask = dm2_v1_save_mask_savegame_buffer();
    DM2_SuppressWriter wr;
    DM2_SuppressReader rd;

    for (int i = 0; i < 60; i++)
        original[i] = (uint8_t)(i * 7 + 13);

    dm2_suppress_writer_init(&wr);
    size_t total_written = 0;
    size_t w;
    int rc = dm2_suppress_writer_write(&wr, original, mask, 60,
        encoded, sizeof(encoded), &w);
    assert(rc == 0);
    total_written += w;
    rc = dm2_suppress_writer_flush(&wr,
        encoded + total_written, sizeof(encoded) - total_written, &w);
    assert(rc == 0);
    total_written += w;
    assert(total_written > 0);
    assert(total_written < 60);

    memset(recovered, 0, sizeof(recovered));
    dm2_suppress_reader_init(&rd, encoded, total_written);
    rc = dm2_suppress_reader_read(&rd, mask, 60, recovered, 0);
    assert(rc == 0);

    for (int i = 0; i < 60; i++) {
        uint8_t expected = original[i] & mask[i];
        uint8_t got = recovered[i] & mask[i];
        assert(expected == got);
    }
}

static void test_hero_mask_roundtrip(void)
{
    uint8_t original[263], recovered[263], encoded[512];
    const uint8_t *mask = dm2_v1_save_mask_hero();
    DM2_SuppressWriter wr;
    DM2_SuppressReader rd;

    for (int i = 0; i < 263; i++)
        original[i] = (uint8_t)(i ^ 0xA5);

    dm2_suppress_writer_init(&wr);
    size_t total_written = 0;
    size_t w;
    int rc = dm2_suppress_writer_write(&wr, original, mask, 263,
        encoded, sizeof(encoded), &w);
    assert(rc == 0);
    total_written += w;
    rc = dm2_suppress_writer_flush(&wr,
        encoded + total_written, sizeof(encoded) - total_written, &w);
    assert(rc == 0);
    total_written += w;

    memset(recovered, 0, sizeof(recovered));
    dm2_suppress_reader_init(&rd, encoded, total_written);
    rc = dm2_suppress_reader_read(&rd, mask, 263, recovered, 0);
    assert(rc == 0);

    for (int i = 0; i < 263; i++) {
        uint8_t expected = original[i] & mask[i];
        uint8_t got = recovered[i] & mask[i];
        assert(expected == got);
    }
}

static void test_single_bit_roundtrip(void)
{
    uint8_t encoded[16];
    DM2_SuppressWriter wr;
    DM2_SuppressReader rd;

    dm2_suppress_writer_init(&wr);
    size_t total = 0, w;
    for (int i = 0; i < 8; i++) {
        int rc = dm2_suppress_writer_write_bit(&wr, i & 1,
            encoded + total, sizeof(encoded) - total, &w);
        assert(rc == 0);
        total += w;
    }
    int rc = dm2_suppress_writer_flush(&wr,
        encoded + total, sizeof(encoded) - total, &w);
    assert(rc == 0);
    total += w;

    dm2_suppress_reader_init(&rd, encoded, total);
    for (int i = 0; i < 8; i++) {
        int bit;
        rc = dm2_suppress_reader_read_bit(&rd, &bit);
        assert(rc == 0);
        assert(bit == (i & 1));
    }
}

static void test_save_state_roundtrip(void)
{
    uint8_t original[6] = {0x11, 0x22, 0x33, 0xAA, 0xBB, 0xCC};
    uint8_t recovered[6], encoded[32];
    const uint8_t *mask = dm2_v1_save_mask_save_state();
    DM2_SuppressWriter wr;
    DM2_SuppressReader rd;

    dm2_suppress_writer_init(&wr);
    size_t total = 0, w;
    int rc = dm2_suppress_writer_write(&wr, original, mask, 6,
        encoded, sizeof(encoded), &w);
    assert(rc == 0);
    total += w;
    rc = dm2_suppress_writer_flush(&wr,
        encoded + total, sizeof(encoded) - total, &w);
    assert(rc == 0);
    total += w;

    memset(recovered, 0, sizeof(recovered));
    dm2_suppress_reader_init(&rd, encoded, total);
    rc = dm2_suppress_reader_read(&rd, mask, 6, recovered, 0);
    assert(rc == 0);

    for (int i = 0; i < 6; i++) {
        assert((recovered[i] & mask[i]) == (original[i] & mask[i]));
    }
}

int main(void) {
    test_savegame_buffer_roundtrip();
    test_hero_mask_roundtrip();
    test_single_bit_roundtrip();
    test_save_state_roundtrip();

    printf("PASS: dm2_v1_save_suppress_roundtrip (4 tests)\n");
    return 0;
}
