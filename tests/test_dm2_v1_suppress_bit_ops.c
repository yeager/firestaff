/* DM2 SUPPRESS bit-level operations: WRITE_1BIT / READ_1BIT round-trip.
 * Source: sksvgame.cpp DM2_WRITE_1BIT, DM2_READ_1BIT. */
#include "dm2_v1_save_load.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    DM2_SuppressWriter writer;
    DM2_SuppressReader reader;
    uint8_t buf[16];
    size_t written;
    int bit;

    /* Write 8 individual bits, read them back */
    dm2_suppress_writer_init(&writer);
    written = 0;
    {
        int bits[] = {1, 0, 1, 1, 0, 0, 1, 0};
        size_t w;
        for (int i = 0; i < 8; i++) {
            assert(dm2_suppress_writer_write_bit(&writer, bits[i],
                buf + written, sizeof(buf) - written, &w) == 0);
            written += w;
        }
    }
    /* 8 bits = 1 byte: 0b10110010 = 0xB2 */
    assert(written == 1);
    assert(buf[0] == 0xB2);

    dm2_suppress_reader_init(&reader, buf, written);
    int expected[] = {1, 0, 1, 1, 0, 0, 1, 0};
    for (int i = 0; i < 8; i++) {
        assert(dm2_suppress_reader_read_bit(&reader, &bit) == 0);
        assert(bit == expected[i]);
    }

    /* Write fewer than 8 bits, flush, read back */
    dm2_suppress_writer_init(&writer);
    written = 0;
    {
        size_t w;
        assert(dm2_suppress_writer_write_bit(&writer, 1,
            buf, sizeof(buf), &w) == 0);
        written += w;
        assert(dm2_suppress_writer_write_bit(&writer, 0,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
        assert(dm2_suppress_writer_write_bit(&writer, 1,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
        assert(written == 0); /* not flushed yet */
        assert(dm2_suppress_writer_flush(&writer,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
        assert(written == 1);
        /* 3 bits: 101, padded to 10100000 = 0xA0 */
        assert(buf[0] == 0xA0);
    }

    dm2_suppress_reader_init(&reader, buf, written);
    assert(dm2_suppress_reader_read_bit(&reader, &bit) == 0 && bit == 1);
    assert(dm2_suppress_reader_read_bit(&reader, &bit) == 0 && bit == 0);
    assert(dm2_suppress_reader_read_bit(&reader, &bit) == 0 && bit == 1);

    /* Mixed: SUPPRESS_WRITER bytes + WRITE_1BIT interleaved */
    dm2_suppress_writer_init(&writer);
    memset(buf, 0, sizeof(buf));
    written = 0;
    {
        size_t w;
        uint8_t data = 0xAB;
        uint8_t mask = 0xFF;
        assert(dm2_suppress_writer_write(&writer, &data, &mask, 1,
            buf, sizeof(buf), &w) == 0);
        written += w;
        assert(dm2_suppress_writer_write_bit(&writer, 1,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
        assert(dm2_suppress_writer_write_bit(&writer, 0,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
        data = 0xCD;
        assert(dm2_suppress_writer_write(&writer, &data, &mask, 1,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
        assert(dm2_suppress_writer_flush(&writer,
            buf + written, sizeof(buf) - written, &w) == 0);
        written += w;
    }
    /* 8 + 1 + 1 + 8 = 18 bits = 3 bytes (padded) */
    assert(written == 3);

    dm2_suppress_reader_init(&reader, buf, written);
    {
        uint8_t out;
        uint8_t mask = 0xFF;
        assert(dm2_suppress_reader_read(&reader, &mask, 1, &out, 0) == 0);
        assert(out == 0xAB);
        assert(dm2_suppress_reader_read_bit(&reader, &bit) == 0 && bit == 1);
        assert(dm2_suppress_reader_read_bit(&reader, &bit) == 0 && bit == 0);
        assert(dm2_suppress_reader_read(&reader, &mask, 1, &out, 0) == 0);
        assert(out == 0xCD);
    }

    printf("PASS: dm2_v1_suppress_bit_ops\n");
    return 0;
}
