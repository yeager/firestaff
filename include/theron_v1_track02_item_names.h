#ifndef THERON_V1_TRACK02_ITEM_NAMES_H
#define THERON_V1_TRACK02_ITEM_NAMES_H

#include <stddef.h>

#define THERON_TRACK02_US_ITEM_NAME_COUNT 66u

/* US item name table extracted from hash-verified US Track 02 BIN
 * (MD5: f23601102138f87c33025877767ebf76).
 * Source offset: user-data 0x21A08E (null-terminated ASCII strings).
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

const char *theron_v1_track02_us_item_name(unsigned int index);
size_t theron_v1_track02_us_item_name_count(void);

#endif
