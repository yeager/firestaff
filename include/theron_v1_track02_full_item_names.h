#ifndef THERON_V1_TRACK02_FULL_ITEM_NAMES_H
#define THERON_V1_TRACK02_FULL_ITEM_NAMES_H

#include <stddef.h>

/* Track 02 per-bank item name table (the complete runtime table).
 * Source: US Track 02 BIN UD 0x09951F, null-separated ASCII.
 * This is the full 77-item table loaded per dungeon bank.
 * Superset of the Track 19 table (69 items) — adds TORCH, FLAMITT,
 * FALCHION, SWORD, GHI, PLATE OF LYTE, POLEYN OF LYTE, HELM OF LYTE,
 * WOODEN SHIELD, SHIELD DEFIANT, GREAVE OF LYTE, GOLD COIN, BOULDER,
 * RABBIT'S FOOT, TAZAHELM, CHEST, OPEN CHEST, CORN, BREAD, CHEESE,
 * SCREAMER SLICE, GOLD KEY, RA KEY. */

#define THERON_TRACK02_FULL_ITEM_COUNT 79u

const char *theron_v1_track02_us_full_item_name(unsigned int index);
size_t theron_v1_track02_us_full_item_count(void);

#endif
