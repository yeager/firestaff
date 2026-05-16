
#include "nexus_v1_text.h"
#include <string.h>
#include <stdio.h>

/* Basic Shift-JIS to UTF-8 converter.
 * Handles ASCII (0x20-0x7E) and katakana (0xA1-0xDF).
 * Full JIS X 0208 conversion would require lookup tables. */
int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max)
{
    int si = 0, ui = 0;
    if (!sjis || !utf8_out) return 0;

    while (si < sjis_len && ui < utf8_max - 3) {
        uint8_t b = sjis[si];
        if (b == 0) break;

        if (b >= 0x20 && b <= 0x7E) {
            /* ASCII */
            utf8_out[ui++] = (char)b;
            si++;
        } else if (b >= 0xA1 && b <= 0xDF) {
            /* Half-width katakana -> UTF-8 (U+FF61-U+FF9F) */
            uint16_t cp = 0xFF61 + (b - 0xA1);
            utf8_out[ui++] = (char)(0xE0 | (cp >> 12));
            utf8_out[ui++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            utf8_out[ui++] = (char)(0x80 | (cp & 0x3F));
            si++;
        } else if ((b >= 0x81 && b <= 0x9F) || (b >= 0xE0 && b <= 0xEF)) {
            /* Double-byte Shift-JIS (skip for now — needs lookup table) */
            utf8_out[ui++] = '?';
            si += 2;
        } else {
            si++;
        }
    }
    utf8_out[ui] = 0;
    return ui;
}

/* Extract printable strings from binary data */
int nexus_v1_extract_strings(const uint8_t *data, int size,
    char **out_strings, int max_strings)
{
    int count = 0, i = 0;
    if (!data || !out_strings) return 0;

    while (i < size && count < max_strings) {
        /* Find runs of printable bytes */
        int start = i;
        while (i < size && ((data[i] >= 0x20 && data[i] <= 0x7E) ||
               (data[i] >= 0xA1 && data[i] <= 0xDF) ||
               (data[i] >= 0x81 && data[i] <= 0x9F) ||
               (data[i] >= 0xE0 && data[i] <= 0xEF)))
            i++;
        if (i - start >= 4) {
            static char buf[256];
            nexus_v1_sjis_to_utf8(data + start, i - start, buf, sizeof(buf));
            out_strings[count++] = buf; /* Note: reuses buffer — copy in real use */
        }
        i++;
    }
    return count;
}

