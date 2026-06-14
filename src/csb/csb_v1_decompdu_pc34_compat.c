/*
 * csb_v1_decompdu_pc34_compat.c
 *
 * Source-locked per ReDMCSB DECOMPDU.C.  v1 implements the
 * header-detector + a graceful-fail shim.  Full CDU
 * decompression is OPEN-OMFATTANDE; this helper is the
 * entry point for the future DECOMPDU port.
 */
#include "csb_v1_decompdu_pc34_compat.h"

#include <string.h>

/* Per the DECOMPDU.C source, compressed CSB dungeons
 * start with the "CDU\0" 4-byte magic.  The fifth byte
 * is the sub-format version (0x00 = raw, 0x01 = RLE,
 * 0x02 = LZ77-like).  v1 supports detection only. */
static const unsigned char kCduMagic[4] = {'C','D','U','\0'};

int csb_v1_decompdu_detect(const unsigned char* header, int headerLen) {
    if (!header || headerLen < 4) return 0;
    if (memcmp(header, kCduMagic, 4) != 0) return 0;
    /* Sub-format byte: 0x00 = raw, 0x01 = RLE, 0x02 = LZ77. */
    if (headerLen >= 5) {
        unsigned char sub = header[4];
        if (sub > 0x02) return 0;
    }
    return 1;
}

int csb_v1_decompdu_implemented(void) {
    /* v1: full DECOMPDU decompressor is OPEN-OMFATTANDE.
     * The helper is wired for graceful fail; the launcher
     * can use csb_v1_decompdu_detect() to show a friendly
     * "this CSB dungeon is compressed; please run
     * DECOMPDU.EXE to decompress" message instead of
     * crashing on a packed file. */
    return 0;
}
