/*
 * theron_v1_track02_experience_table.c — TQ US Track 02 experience thresholds
 *
 * Binary-verified from hash-locked US Track 02 BIN
 * (MD5: f23601102138f87c33025877767ebf76).
 *
 * 64-entry word table at UD 0x1DA890.  Monotonically increasing cumulative
 * experience thresholds (0–214).  Likely maps to 4 skill classes × 16 skill
 * levels (NEOPHYTE through ARCHMASTER), matching the 16 skill level names
 * at UD 0x1C9B6B and 4 champion classes at UD 0x1C9A32.
 *
 * Preceded at UD 0x1DA870 by what appears to be class base-stat parameters:
 *   0x1DA870: 0, 0, 60, 50, 256, 256, 256, 256 (words)
 *   0x1DA880: 3, 3, 3, 3, 0, 10, 54, 90 (words)
 */

#include "theron_v1_track02_experience_table.h"

static const unsigned int g_experience[THERON_TRACK02_EXPERIENCE_ENTRY_COUNT] = {
      0,   3,   8,  11,  15,  17,  21,  23,
     30,  32,  35,  37,  41,  45,  49,  53,
     54,  55,  57,  60,  69,  78,  87,  89,
     92,  93,  95,  97, 105, 115, 124, 132,
    139, 139, 141, 145, 151, 157, 165, 172,
    180, 185, 188, 190, 193, 194, 195, 195,
    197, 199, 199, 201, 201, 203, 205, 206,
    207, 210, 210, 211, 211, 212, 212, 214,
};

unsigned int theron_v1_track02_us_experience_threshold(unsigned int index)
{
    if (index >= THERON_TRACK02_EXPERIENCE_ENTRY_COUNT) return 0;
    return g_experience[index];
}
