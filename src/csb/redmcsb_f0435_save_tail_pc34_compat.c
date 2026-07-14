#include "redmcsb_f0435_save_tail_pc34_compat.h"

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int spans_are_source_admitted(
    const RedmcsbF0435EventTimelineSpansPc34 *spans)
{
    if (spans == NULL || spans->events == NULL || spans->timeline == NULL ||
        spans->events_byte_count == 0U ||
        (spans->events_byte_count & 1U) != 0U ||
        spans->event_maximum_count == 0U ||
        spans->event_maximum_count > (SIZE_MAX / 2U)) {
        return 0;
    }
    return spans->timeline_byte_count == spans->event_maximum_count * 2U;
}

int redmcsb_f0435_load_event_timeline_and_dungeon_tail_pc34(
    RedmcsbF1910ReadExactPc34 read, void *read_context,
    const RedmcsbF1918LoadReceiptPc34 *initial_receipt,
    const RedmcsbF0435EventTimelineSpansPc34 *spans,
    RedmcsbF0434LoadDungeonTailPc34 load_dungeon_tail, void *tail_context,
    RedmcsbF0435TailLoadReceiptPc34 *receipt)
{
    const size_t events_key_offset = REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET +
        REDMCSB_F0435_PC34_EVENTS_PART_INDEX * 2U;
    const size_t events_checksum_offset =
        REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET +
        REDMCSB_F0435_PC34_EVENTS_PART_INDEX * 2U;
    const size_t timeline_key_offset = REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET +
        REDMCSB_F0435_PC34_TIMELINE_PART_INDEX * 2U;
    const size_t timeline_checksum_offset =
        REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET +
        REDMCSB_F0435_PC34_TIMELINE_PART_INDEX * 2U;

    if (receipt != NULL) {
        *receipt = (RedmcsbF0435TailLoadReceiptPc34){0};
    }
    if (read == NULL || initial_receipt == NULL ||
        initial_receipt->header_valid == 0 ||
        initial_receipt->parts_loaded != REDMCSB_F1918_PC34_PART_COUNT ||
        !spans_are_source_admitted(spans) || load_dungeon_tail == NULL ||
        receipt == NULL) {
        return REDMCSB_F0435_PC34_RESULT_PRECONDITION_FAILED;
    }

    receipt->events_key = read_le16(initial_receipt->header + events_key_offset);
    receipt->events_checksum =
        read_le16(initial_receipt->header + events_checksum_offset);
    if (!redmcsb_f1913_load_and_deobfuscate_saved_game_part_pc34(
            read, read_context, spans->events, spans->events_byte_count,
            receipt->events_key, receipt->events_checksum)) {
        return REDMCSB_F0435_PC34_RESULT_EVENTS_FAILED;
    }
    receipt->events_loaded = 1U;

    receipt->timeline_key =
        read_le16(initial_receipt->header + timeline_key_offset);
    receipt->timeline_checksum =
        read_le16(initial_receipt->header + timeline_checksum_offset);
    if (!redmcsb_f1913_load_and_deobfuscate_saved_game_part_pc34(
            read, read_context, spans->timeline, spans->timeline_byte_count,
            receipt->timeline_key, receipt->timeline_checksum)) {
        return REDMCSB_F0435_PC34_RESULT_TIMELINE_FAILED;
    }
    receipt->timeline_loaded = 1U;

    if (load_dungeon_tail(tail_context) == 0) {
        return REDMCSB_F0435_PC34_RESULT_DUNGEON_TAIL_FAILED;
    }
    receipt->dungeon_tail_loaded = 1U;
    return REDMCSB_F0435_PC34_RESULT_OK;
}

const char *redmcsb_f0435_save_tail_pc34_source_evidence(void)
{
    return "ReDMCSB LOADSAVE.C F0435_STARTEND_LoadGame: "
           "EVENTS Keys[C3]/Checksums[C3], TIMELINE Keys[C4]/Checksums[C4], "
           "then F0434_STARTEND_IsLoadDungeonSuccessful_CPSC";
}
