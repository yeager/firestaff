
#ifndef NEXUS_V1_TEXT_H
#define NEXUS_V1_TEXT_H
#include <stdint.h>

/* Shift-JIS text extraction from Nexus Saturn data.
 * Japanese text in DM.BIN, SLEV*.BIN, and DGN files. */

int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max);
int nexus_v1_extract_strings(const uint8_t *data, int size,
    char **out_strings, int max_strings);

#endif

