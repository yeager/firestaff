/* test_dm2_v1_save_timers_pc34_compat.c — DM2-009 verification.
 *
 * Verifies the savegame timer payload materialisation against
 * skproject/SKULLWIN:
 *   - the verified v1d6463 (= vsgame+0x00) 12-byte SUPPRESS mask
 *     (dm2data.cpp:97-99)
 *   - GAME_LOAD order: per-record mask decode, clrtype remainder,
 *     DM2_SORT_TIMERS heapify, DM2_REARRANGE_TIMERLIST free chain
 *     (c_savegame.cpp:1517-1527, c_timer.cpp:126-194, 97-122)
 *   - DM2_cmp_timers tiebreak chain (c_timer.cpp:31-48)
 *   - fail-closed: bounds rejection, underflow leaves caller state
 *     untouched, dummya never restored
 */

#include "dm2_v1_save_timers_pc34_compat.h"

#include <assert.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static void make_timer(DM2_V1_SaveTimerRecord *t,
                       uint8_t map, int32_t ticks,
                       uint8_t type, uint8_t actor,
                       int16_t value_a, int16_t value_b)
{
    /* c_timer.h:59 setmticks: l_00 = (m << 24) | t */
    int32_t l_00 = ((int32_t)map << 24) | (ticks & 0xffffff);
    memset(t, 0, sizeof(*t));
    t->bytes[0] = (uint8_t)((uint32_t)l_00 & 0xffu);
    t->bytes[1] = (uint8_t)(((uint32_t)l_00 >> 8) & 0xffu);
    t->bytes[2] = (uint8_t)(((uint32_t)l_00 >> 16) & 0xffu);
    t->bytes[3] = (uint8_t)(((uint32_t)l_00 >> 24) & 0xffu);
    t->bytes[4] = type;
    t->bytes[5] = actor;
    t->bytes[6] = (uint8_t)((uint16_t)value_a & 0xffu);
    t->bytes[7] = (uint8_t)(((uint16_t)value_a >> 8) & 0xffu);
    t->bytes[8] = (uint8_t)((uint16_t)value_b & 0xffu);
    t->bytes[9] = (uint8_t)(((uint16_t)value_b >> 8) & 0xffu);
    t->bytes[10] = 0xaa; /* dummya: must NOT survive the mask */
    t->bytes[11] = 0xbb;
}

static size_t encode_timers(const DM2_V1_SaveTimerRecord *records,
                            uint16_t count, uint8_t *out, size_t cap)
{
    (void)cap;
    (void)out;
    (void)records;
    /* Source order: the mask is re-armed per record while the bit state
     * carries across records (c_savegame.cpp:655-733). */
    DM2_SuppressWriter writer;
    size_t pos = 0;
    size_t n = 0;
    dm2_suppress_writer_init(&writer);
    for (uint16_t i = 0; i < count; ++i) {
        assert(dm2_suppress_writer_write(&writer, records[i].bytes,
                                         dm2_v1_save_timers_suppress_mask(),
                                         DM2_V1_SAVE_TIMER_RECORD_SIZE,
                                         out + pos, cap - pos, &n) == 0);
        pos += n;
    }
    assert(dm2_suppress_writer_flush(&writer, out + pos, cap - pos, &n) == 0);
    pos += n;
    return pos;
}

int main(void)
{
    static const uint8_t expected_mask[12] = {
        0xffu, 0xffu, 0xffu, 0x3fu, 0x7fu, 0xffu,
        0xffu, 0xffu, 0xffu, 0xffu, 0x00u, 0x00u
    };
    (void)expected_mask;
    DM2_V1_SaveTimerRecord src[5];
    DM2_V1_SaveTimerRecord out_records[DM2_V1_SAVE_TIMER_MAX];
    int16_t indices[DM2_V1_SAVE_TIMER_MAX];
    DM2_V1_SaveTimerReceipt receipt;
    (void)receipt;
    DM2_SuppressReader reader;
    uint8_t stream[512];
    size_t stream_size;

    /* ── verified mask (dm2data.cpp:97-99, v1d6463 = vsgame+0x00) ── */
    assert(memcmp(dm2_v1_save_timers_suppress_mask(), expected_mask,
                  12u) == 0);

    /* ── DM2_cmp_timers tiebreak chain (c_timer.cpp:31-48) ── */
    {
        DM2_V1_SaveTimerRecord a;
        DM2_V1_SaveTimerRecord b;
        make_timer(&a, 0, 100, 2, 1, 0, 0);
        make_timer(&b, 0, 100, 3, 1, 0, 0);
        assert(dm2_v1_save_timer_cmp(&a, 0, &b, 1) == 0); /* type asc loses */
        assert(dm2_v1_save_timer_cmp(&b, 1, &a, 0) == 1); /* type desc wins */
        make_timer(&a, 0, 100, 2, 1, 0, 0);
        make_timer(&b, 0, 100, 2, 9, 0, 0);
        assert(dm2_v1_save_timer_cmp(&a, 0, &b, 1) == 0);
        assert(dm2_v1_save_timer_cmp(&b, 1, &a, 0) == 1); /* actor desc */
        make_timer(&a, 0, 100, 2, 1, 0, 0);
        make_timer(&b, 0, 100, 2, 1, 0, 0);
        assert(dm2_v1_save_timer_cmp(&a, 2, &b, 5) == 1); /* index asc */
        assert(dm2_v1_save_timer_cmp(&b, 5, &a, 2) == 0);
        make_timer(&a, 0, 50, 1, 1, 0, 0);
        make_timer(&b, 0, 60, 9, 9, 0, 0);
        assert(dm2_v1_save_timer_cmp(&a, 0, &b, 1) == 1); /* ticks first */
    }

    /* ── round-trip through the source mask (fill=0) ── */
    make_timer(&src[0], 3, 0x00112233, 0x15, 0x7f, -1234, 23456);
    make_timer(&src[1], 0x3f, 0x00ffffff, 0x7f, 0xff, 32767, -32768);
    make_timer(&src[2], 1, 5, 0x0e, 0x00, 0, 0);
    stream_size = encode_timers(src, 3, stream, sizeof(stream));
    assert(stream_size > 0u);

    dm2_suppress_reader_init(&reader, stream, stream_size);
    memset(out_records, 0xcc, sizeof(out_records));
    memset(indices, 0x7f, sizeof(indices));
    assert(dm2_v1_save_timer_materialize(&reader, 3, 5, out_records,
                                         indices, &receipt) == 1);
    assert(receipt.valid && receipt.decoded_count == 3);
    assert(receipt.cleared_count == 2);
    assert(receipt.post_load_rebuild_pending);
    for (int i = 0; i < 3; ++i) {
        assert(dm2_v1_save_timer_get_map(&out_records[i]) ==
               (src[i].bytes[3] & 0x3fu)); /* mask 0x3f */
        assert(dm2_v1_save_timer_get_ticks(&out_records[i]) ==
               dm2_v1_save_timer_get_ticks(&src[i]));
        assert(dm2_v1_save_timer_get_type(&out_records[i]) ==
               (src[i].bytes[4] & 0x7fu)); /* mask 0x7f */
        assert(dm2_v1_save_timer_get_actor(&out_records[i]) ==
               src[i].bytes[5]);
        assert(dm2_v1_save_timer_get_a(&out_records[i]) ==
               dm2_v1_save_timer_get_a(&src[i]));
        assert(dm2_v1_save_timer_get_b(&out_records[i]) ==
               dm2_v1_save_timer_get_b(&src[i]));
        /* dummya is never restored (mask 0x00) */
        assert(out_records[i].bytes[10] == 0u);
        assert(out_records[i].bytes[11] == 0u);
    }
    /* clrtype remainder (c_savegame.cpp:1519-1524) */
    assert(dm2_v1_save_timer_is_no_type(&out_records[3]));
    assert(dm2_v1_save_timer_is_no_type(&out_records[4]));

    /* free chain over [0,5): slots 3,4 empty (c_timer.cpp:97-122) */
    assert(receipt.num_timer_indices == 3);
    assert(receipt.available_timeridx == 3);
    assert(dm2_v1_save_timer_get_dataw(&out_records[3]) == 4);
    assert(dm2_v1_save_timer_get_dataw(&out_records[4]) == -1);

    /* ── heapify order (c_timer.cpp:126-194) ── */
    {
        DM2_V1_SaveTimerRecord heap_src[5];
        DM2_V1_SaveTimerRecord heap_out[DM2_V1_SAVE_TIMER_MAX];
        (void)heap_out;
        int16_t heap_idx[DM2_V1_SAVE_TIMER_MAX];
        (void)heap_idx;
        static const int16_t expected_idx[5] = { 3, 1, 2, 0, 4 };
        (void)expected_idx;
        make_timer(&heap_src[0], 0, 50, 1, 0, 0, 0);
        make_timer(&heap_src[1], 0, 20, 1, 0, 0, 0);
        make_timer(&heap_src[2], 0, 30, 1, 0, 0, 0);
        make_timer(&heap_src[3], 0, 10, 1, 0, 0, 0);
        make_timer(&heap_src[4], 0, 40, 1, 0, 0, 0);
        stream_size = encode_timers(heap_src, 5, stream, sizeof(stream));
        dm2_suppress_reader_init(&reader, stream, stream_size);
        assert(dm2_v1_save_timer_materialize(&reader, 5, 5, heap_out,
                                             heap_idx, &receipt) == 1);
        for (int i = 0; i < 5; ++i)
            assert(heap_idx[i] == expected_idx[i]);
        /* heap root is the minimum-tick timer */
        assert(dm2_v1_save_timer_get_ticks(&heap_out[heap_idx[0]]) == 10);
        /* single timer: identity, no heapify (c_timer.cpp:138) */
        stream_size = encode_timers(heap_src, 1, stream, sizeof(stream));
        dm2_suppress_reader_init(&reader, stream, stream_size);
        assert(dm2_v1_save_timer_materialize(&reader, 1, 5, heap_out,
                                             heap_idx, &receipt) == 1);
        assert(heap_idx[0] == 0);
        /* zero timers: no decode, all cleared */
        dm2_suppress_reader_init(&reader, stream, 0);
        assert(dm2_v1_save_timer_materialize(&reader, 0, 5, heap_out,
                                             heap_idx, &receipt) == 1);
        assert(receipt.decoded_count == 0 && receipt.cleared_count == 5);
        assert(receipt.available_timeridx == 0);
    }

    /* ── fail-closed: bounds ── */
    dm2_suppress_reader_init(&reader, stream, stream_size);
    assert(dm2_v1_save_timer_materialize(&reader, 6, 5, out_records,
                                         indices, &receipt) == 0);
    assert(receipt.rejected_bounds);
    assert(dm2_v1_save_timer_materialize(&reader, 1,
                                         DM2_V1_SAVE_TIMER_MAX + 1u,
                                         out_records, indices,
                                         &receipt) == 0);
    assert(receipt.rejected_bounds);

    /* ── fail-closed: underflow leaves caller state untouched ── */
    {
        DM2_V1_SaveTimerRecord guard[DM2_V1_SAVE_TIMER_MAX];
        int16_t guard_idx[DM2_V1_SAVE_TIMER_MAX];
        memset(guard, 0x5a, sizeof(guard));
        memset(guard_idx, 0x11, sizeof(guard_idx));
        dm2_suppress_reader_init(&reader, stream, 2u); /* truncated */
        assert(dm2_v1_save_timer_materialize(&reader, 3, 5, guard,
                                             guard_idx, &receipt) == 0);
        assert(receipt.rejected_stream_underflow);
        assert(guard[0].bytes[0] == 0x5a && guard[2].bytes[11] == 0x5a);
        assert(guard_idx[0] == 0x1111);
    }

    assert(dm2_v1_save_timers_source_evidence() != 0);
    return 0;
}
