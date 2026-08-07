/* dm2_v1_save_timers_pc34_compat.c — DM2 V1 savegame timer payload
 * materialisation (DM2-009 bounded slice).
 *
 * skproject/SKULLWIN source lock:
 *   c_savegame.cpp:1517-1527  GAME_LOAD timer section order
 *   c_savegame.cpp:655-733    DM2_SUPPRESS_READER per-record mask re-arm
 *   c_timer.h:8-96            c_tim 12-byte layout + accessors
 *   c_timer.cpp:31-48         DM2_cmp_timers
 *   c_timer.cpp:126-194       DM2_SORT_TIMERS (identity + heapify)
 *   c_timer.cpp:97-122        DM2_REARRANGE_TIMERLIST
 *   dm2data.cpp:97-99         vsgame[120]; v1d6463 = vsgame + 0x00
 *                             (dm2data.h:608): the verified timer mask
 *
 * The saved timer-record byte layout is now source-proven (SKPROJECT-GAP-001
 * layout half), and the saved weather-timer owner side is bound by
 * dm2_v1_save_timer_weather_owner_receipt below (DM2-011).
 * No timer semantics beyond the wire layout and that owner binding are
 * assigned.
 */

#include "dm2_v1_save_timers_pc34_compat.h"

#include <string.h>

/* v1d6463 = vsgame + 0x00, dm2data.cpp:97-99 (first 12 of vsgame[120]):
 * w_00 (2 bytes full), dummy2 (full), b_03 (6 bits: map), ttype (7 bits),
 * actor (full), wvalueA (2 bytes full), wvalueB (2 bytes full), dummya
 * (never saved). */
static const uint8_t dm2_v1_save_timer_mask[DM2_V1_SAVE_TIMER_MASK_SIZE] = {
    0xffu, 0xffu, 0xffu, 0x3fu, 0x7fu, 0xffu,
    0xffu, 0xffu, 0xffu, 0xffu, 0x00u, 0x00u
};

const uint8_t *dm2_v1_save_timers_suppress_mask(void)
{
    return dm2_v1_save_timer_mask;
}

static int16_t dm2_v1_save_timer_le16(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

int16_t dm2_v1_save_timer_get_dataw(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:55-56 w_00 @0 */
    return t ? dm2_v1_save_timer_le16(t->bytes) : 0;
}

uint8_t dm2_v1_save_timer_get_map(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:57 getmap = unsignedword(b_03) @3 */
    return t ? t->bytes[3] : 0;
}

int32_t dm2_v1_save_timer_get_ticks(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:58 getticks = l_00 & 0xffffff */
    int32_t l_00;
    if (!t) return 0;
    l_00 = (int32_t)((uint32_t)t->bytes[0] | ((uint32_t)t->bytes[1] << 8) |
                     ((uint32_t)t->bytes[2] << 16) |
                     ((uint32_t)t->bytes[3] << 24));
    return l_00 & 0xffffff;
}

uint8_t dm2_v1_save_timer_get_type(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:66 ttype @4 */
    return t ? t->bytes[4] : 0;
}

uint8_t dm2_v1_save_timer_get_actor(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:69 actor @5 */
    return t ? t->bytes[5] : 0;
}

int16_t dm2_v1_save_timer_get_a(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:76-77 wvalueA @6 */
    return t ? dm2_v1_save_timer_le16(t->bytes + 6) : 0;
}

int16_t dm2_v1_save_timer_get_b(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:88-89 wvalueB @8 */
    return t ? dm2_v1_save_timer_le16(t->bytes + 8) : 0;
}

int dm2_v1_save_timer_is_no_type(const DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:65 is_notype */
    return !t || t->bytes[4] == 0u;
}

void dm2_v1_save_timer_clr_type(DM2_V1_SaveTimerRecord *t)
{
    /* c_timer.h:64 clrtype */
    if (t) t->bytes[4] = 0u;
}

void dm2_v1_save_timer_set_dataw(DM2_V1_SaveTimerRecord *t, int16_t n)
{
    /* c_timer.h:55 setdataw writes w_00 @0 little-endian */
    if (!t) return;
    t->bytes[0] = (uint8_t)((uint16_t)n & 0xffu);
    t->bytes[1] = (uint8_t)(((uint16_t)n >> 8) & 0xffu);
}

void dm2_v1_save_timer_set_b(DM2_V1_SaveTimerRecord *t, int16_t n)
{
    if (!t) return;
    t->bytes[8] = (uint8_t)((uint16_t)n & 0xffu);
    t->bytes[9] = (uint8_t)(((uint16_t)n >> 8) & 0xffu);
}

int dm2_v1_save_timer_cmp(const DM2_V1_SaveTimerRecord *a, int a_index,
                          const DM2_V1_SaveTimerRecord *b, int b_index)
{
    /* DM2_cmp_timers, c_timer.cpp:31-48.  Returns nonzero when a sorts
     * before b.  The source's final pointer comparison is the record-array
     * address order, reproduced here by index. */
    int32_t a_ticks = dm2_v1_save_timer_get_ticks(a);
    int32_t b_ticks = dm2_v1_save_timer_get_ticks(b);
    if (a_ticks < b_ticks) return 1;
    if (a_ticks != b_ticks) return 0;
    if (dm2_v1_save_timer_get_type(a) > dm2_v1_save_timer_get_type(b))
        return 1;
    if (dm2_v1_save_timer_get_type(a) != dm2_v1_save_timer_get_type(b))
        return 0;
    if (dm2_v1_save_timer_get_actor(a) > dm2_v1_save_timer_get_actor(b))
        return 1;
    if (dm2_v1_save_timer_get_actor(a) != dm2_v1_save_timer_get_actor(b))
        return 0;
    if (a_index > b_index) return 0;
    return 1;
}

void dm2_v1_save_timer_sort(const DM2_V1_SaveTimerRecord *records,
                            uint16_t num_timers,
                            int16_t *indices)
{
    /* DM2_SORT_TIMERS, c_timer.cpp:126-194 (heapify only; the extraction
     * side lives in DM2_GET_AND_DELETE_NEXT_TIMER outside this slice). */
    if (!records || !indices || num_timers == 0u) return;
    for (uint16_t i = 0; i < num_timers; ++i)
        indices[i] = (int16_t)i;
    if (num_timers == 1u) return;
    for (int32_t vw_00 = ((int32_t)num_timers - 2) >> 1; vw_00 >= 0;
         --vw_00) {
        int32_t vw_04 = vw_00;
        /* c_timer.cpp:147: the sifted record pointer is captured once and
         * stays valid because only indices permute. */
        const DM2_V1_SaveTimerRecord *parent =
            &records[indices[vw_00]];
        int32_t parent_index = indices[vw_00];
        for (;;) {
            int32_t rg2w = 2 * vw_04 + 1;
            int32_t child_index;
            if (rg2w >= (int32_t)num_timers) break;
            child_index = indices[rg2w];
            if (rg2w + 1 >= (int32_t)num_timers) {
                /* c_timer.cpp:155-159 */
                if (dm2_v1_save_timer_cmp(parent, parent_index,
                                          &records[child_index],
                                          child_index))
                    break;
            } else {
                int32_t vw_10 = rg2w + 1;
                int32_t child2_index = indices[vw_10];
                if (dm2_v1_save_timer_cmp(parent, parent_index,
                                          &records[child_index],
                                          child_index)) {
                    /* c_timer.cpp:165-168 */
                    if (dm2_v1_save_timer_cmp(parent, parent_index,
                                              &records[child2_index],
                                              child2_index))
                        break;
                    rg2w = vw_10;
                } else {
                    /* c_timer.cpp:170-174 */
                    if (dm2_v1_save_timer_cmp(&records[child2_index],
                                              child2_index,
                                              &records[child_index],
                                              child_index))
                        rg2w = vw_10;
                }
            }
            /* c_timer.cpp:177-181 */
            {
                int16_t tmp = indices[vw_04];
                indices[vw_04] = indices[rg2w];
                indices[rg2w] = tmp;
            }
            vw_04 = rg2w;
        }
    }
}

void dm2_v1_save_timer_rearrange(DM2_V1_SaveTimerRecord *records,
                                 uint16_t max_timers,
                                 int16_t *out_num_timer_indices,
                                 int16_t *out_available_timeridx)
{
    /* DM2_REARRANGE_TIMERLIST, c_timer.cpp:97-122. */
    int16_t num_timer_indices = 0;
    int16_t available = -1;
    int16_t lastn = -1;
    if (!records) return;
    for (int16_t n = 0; n < (int16_t)max_timers; ++n) {
        if (!dm2_v1_save_timer_is_no_type(&records[n])) {
            num_timer_indices = (int16_t)(n + 1);
        } else {
            if (available != -1)
                dm2_v1_save_timer_set_dataw(&records[lastn], n);
            else
                available = n;
            dm2_v1_save_timer_set_dataw(&records[n], -1);
            lastn = n;
        }
    }
    if (out_num_timer_indices) *out_num_timer_indices = num_timer_indices;
    if (out_available_timeridx) *out_available_timeridx = available;
}

int dm2_v1_save_timer_materialize(DM2_SuppressReader *reader,
                                  uint16_t num_timers,
                                  uint16_t max_timers,
                                  DM2_V1_SaveTimerRecord *records,
                                  int16_t *indices,
                                  DM2_V1_SaveTimerReceipt *out_receipt)
{
    /* GAME_LOAD timer section, c_savegame.cpp:1517-1527. */
    DM2_V1_SaveTimerReceipt receipt;
    DM2_V1_SaveTimerRecord scratch[DM2_V1_SAVE_TIMER_MAX];
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.post_load_rebuild_pending = 1;
    if (!reader || !records || !indices || num_timers > max_timers ||
        max_timers > DM2_V1_SAVE_TIMER_MAX) {
        receipt.rejected_bounds = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    memset(scratch, 0, sizeof(scratch));
    /* c_savegame.cpp:1517: per-record SUPPRESS decode through the verified
     * v1d6463 mask.  The source re-arms the same 12-byte mask per record
     * (bp_00 -= record size, c_savegame.cpp:726) and carries the bit state
     * across records; dm2_suppress_reader_read on the shared reader
     * reproduces both.  Masked-off bytes decode to 0. */
    for (uint16_t i = 0; i < num_timers; ++i) {
        if (dm2_suppress_reader_read(reader, dm2_v1_save_timer_mask,
                                     DM2_V1_SAVE_TIMER_RECORD_SIZE,
                                     scratch[i].bytes, 0) != 0) {
            /* M_exit path: caller state untouched (scratch only). */
            receipt.rejected_stream_underflow = 1;
            receipt.decoded_count = i;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.decoded_count++;
    }
    /* c_savegame.cpp:1519-1524: clrtype() for [num_timers, max_timers). */
    for (uint16_t i = num_timers; i < max_timers; ++i) {
        dm2_v1_save_timer_clr_type(&scratch[i]);
        receipt.cleared_count++;
    }

    memcpy(records, scratch, sizeof(scratch[0]) * max_timers);

    /* c_savegame.cpp:1525 + c_timer.cpp:126-194, 97-122. */
    dm2_v1_save_timer_sort(records, num_timers, indices);
    dm2_v1_save_timer_rearrange(records, max_timers,
                                &receipt.num_timer_indices,
                                &receipt.available_timeridx);

    {
        uint32_t hash = 2166136261u;
        for (uint16_t i = 0; i < num_timers; ++i)
            for (uint16_t b = 0; b < DM2_V1_SAVE_TIMER_RECORD_SIZE; ++b) {
                hash ^= records[i].bytes[b];
                hash *= 16777619u;
            }
        receipt.payload_hash = hash;
    }
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_save_timer_weather_owner_receipt(
    const DM2_V1_SaveTimerRecord *record, int32_t restored_gametick,
    DM2_V1_SaveTimerWeatherOwnerReceipt *out_receipt)
{
    DM2_V1_SaveTimerWeatherOwnerReceipt receipt;
    int64_t remaining;
    uint32_t hash = 2166136261u;

    memset(&receipt, 0, sizeof(receipt));
    if (!record || !out_receipt) return 0;

    /* Owner identity (c_weather.cpp:22-30): the weather chain's own queue
     * call is the only 0x54 producer in the source; it always sets actor 0
     * and map 0.  Any other byte pattern is not a weather-chain timer and
     * must not be adopted by the restored chain. */
    receipt.type = dm2_v1_save_timer_get_type(record);
    receipt.actor = dm2_v1_save_timer_get_actor(record);
    receipt.map = dm2_v1_save_timer_get_map(record);
    if (receipt.type != DM2_V1_SAVE_TIMER_TYPE_UPDATE_WEATHER ||
        receipt.actor != DM2_V1_SAVE_TIMER_WEATHER_ACTOR ||
        receipt.map != 0u) {
        return 0;
    }

    /* Schedule identity: mticks was gametick + delay at queue time; the
     * savegame restores gametick from the same header block
     * (c_savegame.cpp:1486-1487), so the signed delta is the remaining
     * schedule.  A non-positive delta means the source timer proceed fires
     * it next (c_tim_proc.cpp:4179-4183 -> DM2_UPDATE_WEATHER(1)), which
     * re-queues the chain with RAND16(256)+50 (c_weather.cpp:85-88). */
    receipt.target_tick = dm2_v1_save_timer_get_ticks(record);
    receipt.restored_gametick = restored_gametick;
    remaining = (int64_t)receipt.target_tick - (int64_t)restored_gametick;
    if (remaining > INT32_MAX || remaining < INT32_MIN) return 0;
    receipt.remaining_ticks = (int32_t)remaining;
    receipt.fires_on_next_proceed = receipt.remaining_ticks <= 0;

    hash ^= receipt.type;
    hash *= 16777619u;
    hash ^= receipt.actor;
    hash *= 16777619u;
    hash ^= receipt.map;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt.target_tick;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt.restored_gametick;
    hash *= 16777619u;
    hash ^= (uint32_t)receipt.remaining_ticks;
    hash *= 16777619u;
    if (hash == 0u) return 0;
    receipt.owner_hash = hash;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

const char *dm2_v1_save_timers_source_evidence(void)
{
    return
        "DM2 V1 savegame timer materialisation — DM2-009 bounded slice\n"
        "skproject/SKULLWIN/c_savegame.cpp:1517-1527 GAME_LOAD timer order\n"
        "skproject/SKULLWIN/c_savegame.cpp:655-733 DM2_SUPPRESS_READER\n"
        "skproject/SKULLWIN/c_timer.h:8-96 c_tim 12-byte layout\n"
        "skproject/SKULLWIN/c_timer.cpp:31-48 DM2_cmp_timers\n"
        "skproject/SKULLWIN/c_timer.cpp:126-194 DM2_SORT_TIMERS heapify\n"
        "skproject/SKULLWIN/c_timer.cpp:97-122 DM2_REARRANGE_TIMERLIST\n"
        "skproject/SKULLWIN/dm2data.cpp:97-99 v1d6463 = vsgame+0x00 mask\n"
        "SKPROJECT-GAP-001 timer-record byte layout now source-proven;\n"
        "post-load READ_SKSAVE_DUNGEON/PROCEED_GLOBAL_EFFECT_TIMERS pending.\n";
}
