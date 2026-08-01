#ifndef THERON_V1_TRACK02_DM1_ITEM_NAMES_H
#define THERON_V1_TRACK02_DM1_ITEM_NAMES_H

#include <stddef.h>

/* DM1-compatible item name table from US Track 02 BIN (UD 0x1D9737).
 * 63 items — the standard DM1 item set with plate armor, keys, and
 * EYE OF TIME/FURY but without TQ-unique items.
 * Hash: MD5 f23601102138f87c33025877767ebf76.
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

#define THERON_TRACK02_DM1_ITEM_NAME_COUNT 63u

const char *theron_v1_track02_dm1_item_name(unsigned int index);
size_t theron_v1_track02_dm1_item_name_count(void);

#endif
