#ifndef REDMCSB_F0434_DUNGEON_TAIL_PC34_COMPAT_H
#define REDMCSB_F0434_DUNGEON_TAIL_PC34_COMPAT_H

#include "redmcsb_f1918_hintload_pc34_compat.h"
#include "redmcsb_f7063_dungeon_stream_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB LOADSAVE.C F0434 and READWRIT.C F0421, PC34 dungeon-tail intake.
 *
 * F0434 reads the caller-admitted DUNGEON_HEADER, MAP, column, square-first-
 * thing, text, sixteen ThingData, and raw-map spans in F7063's fixed order.
 * F0421 adds each unsigned source byte to a wrapping 16-bit accumulator;
 * F0434 then reads
 * and compares the trailing checksum word. Struct layouts, allocations, map
 * counts, and runtime publication are deliberately outside this boundary. */

enum {
    REDMCSB_F0434_PC34_RESULT_OK = 1,
    REDMCSB_F0434_PC34_RESULT_PRECONDITION_FAILED = 0,
    REDMCSB_F0434_PC34_RESULT_PART_READ_FAILED = -1,
    REDMCSB_F0434_PC34_RESULT_CHECKSUM_READ_FAILED = -2,
    REDMCSB_F0434_PC34_RESULT_CHECKSUM_MISMATCH = -3
};

typedef struct {
    uint8_t *bytes;
    size_t byte_count;
} RedmcsbF0434DungeonTailPartPc34;

typedef struct {
    unsigned int parts_loaded;
    unsigned int failed_part;
    uint16_t calculated_checksum;
    uint16_t stored_checksum;
} RedmcsbF0434DungeonTailReceiptPc34;

/* Each part size must be the original media's already-admitted exact F0434
 * read size and no part may exceed the source int16 byte-count argument. */
int redmcsb_f0434_load_dungeon_tail_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context,
    RedmcsbF0434DungeonTailPartPc34
        parts[REDMCSB_F7063_DUNGEON_PART_COUNT],
    RedmcsbF0434DungeonTailReceiptPc34 *receipt);

const char *redmcsb_f0434_dungeon_tail_pc34_source_evidence(void);

#endif
