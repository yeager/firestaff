#ifndef REDMCSB_F0435_SAVE_TAIL_PC34_COMPAT_H
#define REDMCSB_F0435_SAVE_TAIL_PC34_COMPAT_H

#include "redmcsb_f1918_hintload_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB LOADSAVE.C F0435 continuation after F1918's first three parts.
 *
 * F0435 reads EVENTS with Keys[C3]/Checksums[C3], then the uint16 TIMELINE
 * array with Keys[C4]/Checksums[C4], and only then calls F0434 to load the
 * dungeon tail. EVENT layout and the F0434 media-specific tail remain owned
 * by the caller: this boundary neither allocates nor interprets either. */

#define REDMCSB_F0435_PC34_EVENTS_PART_INDEX 3U
#define REDMCSB_F0435_PC34_TIMELINE_PART_INDEX 4U

enum {
    REDMCSB_F0435_PC34_RESULT_OK = 1,
    REDMCSB_F0435_PC34_RESULT_PRECONDITION_FAILED = 0,
    REDMCSB_F0435_PC34_RESULT_EVENTS_FAILED = -1,
    REDMCSB_F0435_PC34_RESULT_TIMELINE_FAILED = -2,
    REDMCSB_F0435_PC34_RESULT_DUNGEON_TAIL_FAILED = -3
};

typedef int (*RedmcsbF0434LoadDungeonTailPc34)(void *context);

typedef struct {
    uint8_t *events;
    size_t events_byte_count;
    uint8_t *timeline;
    size_t timeline_byte_count;
    size_t event_maximum_count;
} RedmcsbF0435EventTimelineSpansPc34;

typedef struct {
    unsigned int events_loaded;
    unsigned int timeline_loaded;
    unsigned int dungeon_tail_loaded;
    uint16_t events_key;
    uint16_t events_checksum;
    uint16_t timeline_key;
    uint16_t timeline_checksum;
} RedmcsbF0435TailLoadReceiptPc34;

/* The initial receipt must be the accepted F1918 transaction at its source
 * cursor. timeline_byte_count is exactly EventMaximumCount * sizeof(int16_t).
 * No event record size is assumed because it varies by original media. */
int redmcsb_f0435_load_event_timeline_and_dungeon_tail_pc34(
    RedmcsbF1910ReadExactPc34 read, void *read_context,
    const RedmcsbF1918LoadReceiptPc34 *initial_receipt,
    const RedmcsbF0435EventTimelineSpansPc34 *spans,
    RedmcsbF0434LoadDungeonTailPc34 load_dungeon_tail, void *tail_context,
    RedmcsbF0435TailLoadReceiptPc34 *receipt);

const char *redmcsb_f0435_save_tail_pc34_source_evidence(void);

#endif
