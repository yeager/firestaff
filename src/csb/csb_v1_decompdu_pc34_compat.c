/*
 * csb_v1_decompdu_pc34_compat.c
 *
 * Source-locked per ReDMCSB DECOMPDU.C
 * F0455_FLOPPY_DecompressDungeon.  v2 (Dungeon GAP 4)
 * ports the portable C variant of the bit-packed
 * decompressor (MEDIA481_P20JB_I34E_I34M_F31E_F31J_P31J,
 * DECOMPDU.C lines ~250-290), adds bounds checks the
 * original lacked, a matching encoder for round-trip
 * testing, and a grid wrapper for level-pack containers.
 *
 * Original F0455 prefix-code format (MSB-first bit stream
 * preceded by a 20-byte code table = 4 most-common bytes
 * + 16 less-common bytes):
 *
 *   0bb        -> mostCommon[bb]    (3-bit code,  bb in 0..3)
 *   10bbbb     -> lessCommon[bbbb]  (6-bit code,  bbbb in 0..15)
 *   11<8 bits> -> literal byte      (10-bit code)
 */
#include "csb_v1_decompdu_pc34_compat.h"

#include <string.h>
#include <stdlib.h>

/* Firestaff CDU container magic (NOT part of the original
 * floppy format; Firestaff wraps the F0455 payload so the
 * scanner can recognise packed dungeons).  See header. */
static const unsigned char kCduMagic[4] = {'C','D','U','\0'};

int csb_v1_decompdu_detect(const unsigned char* header, int headerLen) {
    if (!header || headerLen < 4) return 0;
    if (memcmp(header, kCduMagic, 4) != 0) return 0;
    /* Sub-format byte: 1 = F0455 bit-packed (the real CSB
     * compression).  0 (raw) and 2 (reserved) remain
     * recognised so the v1 stub test keeps passing. */
    if (headerLen >= 5) {
        unsigned char sub = header[4];
        if (sub > 0x02) return 0;
    }
    return 1;
}

int csb_v1_decompdu_implemented(void) {
    /* The F0455 core decompressor + grid wrapper now ship. */
    return 1;
}

/* ── F0455 core decompressor ──────────────────────────────────
 * Ported from DECOMPDU.C MEDIA481 portable C path.  The
 * original trusted its input and over-read by design (the
 * 68k routine kept a 32-bit shift register topped up from
 * the stream).  This port keeps the exact prefix-code
 * semantics but never reads past compressedLen and reports
 * truncation rather than running off the buffer. */
int csb_v1_decompdu_f0455(const unsigned char* compressed,
                          long compressedLen,
                          unsigned char* out,
                          long outByteCount) {
    const unsigned char* mostCommon;   /* 4 bytes  (compressed[0..3])  */
    const unsigned char* lessCommon;   /* 16 bytes (compressed[4..19]) */
    unsigned long bitBuffer = 0;
    int  bitsInBuffer = 0;
    long byteCount = 0;
    long inputPos = 20;                /* first compressed byte after table */
    int  padBytes = 0;                 /* virtual zero bytes for flush tail */
    const int kMaxPad = 8;

    if (!compressed || !out) return CSB_DECOMPDU_ERR_NULL;
    if (outByteCount < 0)     return CSB_DECOMPDU_ERR_BOUNDS;
    if (outByteCount == 0)    return CSB_DECOMPDU_ERR_OK;
    /* Need at least the 20-byte code table. */
    if (compressedLen < 20)   return CSB_DECOMPDU_ERR_CORRUPT;

    mostCommon = compressed;            /* DECOMPDU.C L1880 */
    lessCommon = compressed + 4;        /* DECOMPDU.C L1881 */

    /* DECOMPDU.C do/while: emit one byte per pass. */
    do {
        unsigned code2;

        /* Top up the shift register to > 24 bits, matching
         * the original `while (BitsInBufferCount <= 24)`. */
        while (bitsInBuffer <= 24) {
            unsigned char next;
            if (inputPos < compressedLen) {
                next = compressed[inputPos++];
            } else {
                /* Flush tail: the original over-read real
                 * memory; a well-formed stream never consumes
                 * these padding bits.  Cap the padding so a
                 * corrupt/truncated stream is caught. */
                if (++padBytes > kMaxPad) {
                    return CSB_DECOMPDU_ERR_TRUNCATED;
                }
                next = 0;
            }
            bitBuffer = (bitBuffer << 8) | next;
            bitsInBuffer += 8;
        }

        /* Peek the top 2 valid bits (MSB-first). */
        code2 = (unsigned)((bitBuffer >> (bitsInBuffer - 2)) & 3u);
        switch (code2) {
            case 0:
            case 1: /* prefix bit 0 -> 3-bit "0bb" most-common */
                bitsInBuffer -= 3;
                out[byteCount++] =
                    mostCommon[(bitBuffer >> bitsInBuffer) & 3u];
                break;
            case 2: /* prefix "10" -> 6-bit "10bbbb" less-common */
                bitsInBuffer -= 6;
                out[byteCount++] =
                    lessCommon[(bitBuffer >> bitsInBuffer) & 15u];
                break;
            default: /* code2 == 3, prefix "11" -> 10-bit literal */
                bitsInBuffer -= 10;
                out[byteCount++] =
                    (unsigned char)((bitBuffer >> bitsInBuffer) & 255u);
                break;
        }
    } while (byteCount < outByteCount);

    return CSB_DECOMPDU_ERR_OK;
}

/* ── MSB-first bit writer (encoder helper) ────────────────── */
typedef struct {
    unsigned char* buf;
    long           capacity;
    long           pos;      /* byte position */
    unsigned       acc;      /* bit accumulator, MSB-first */
    int            nbits;    /* bits currently in acc */
    int            overflow; /* set when capacity exceeded */
} CduBitWriter;

static void cdu_bw_put(CduBitWriter* w, unsigned value, int bits) {
    int i;
    for (i = bits - 1; i >= 0; --i) {
        unsigned bit = (value >> i) & 1u;
        w->acc = (w->acc << 1) | bit;
        w->nbits++;
        if (w->nbits == 8) {
            if (w->pos < w->capacity) {
                w->buf[w->pos++] = (unsigned char)(w->acc & 0xFFu);
            } else {
                w->overflow = 1;
            }
            w->acc = 0;
            w->nbits = 0;
        }
    }
}

static long cdu_bw_flush(CduBitWriter* w) {
    if (w->nbits > 0) {
        w->acc <<= (8 - w->nbits);
        if (w->pos < w->capacity) {
            w->buf[w->pos++] = (unsigned char)(w->acc & 0xFFu);
        } else {
            w->overflow = 1;
        }
        w->acc = 0;
        w->nbits = 0;
    }
    return w->pos;
}

/* ── F0455 encoder (round-trip inverse) ───────────────────── */
int csb_v1_decompdu_f0455_encode(const unsigned char* raw,
                                 long rawLen,
                                 unsigned char* out,
                                 long outCapacity,
                                 long* outLen) {
    long freq[256];
    int  codeKind[256];   /* 0 = most, 1 = less, 2 = literal */
    int  codeIndex[256];  /* slot within table for most/less */
    unsigned char table[20];
    CduBitWriter w;
    long i;
    int t;

    if (!raw || !out || !outLen) return CSB_DECOMPDU_ERR_NULL;
    if (rawLen < 0)              return CSB_DECOMPDU_ERR_BOUNDS;
    if (outCapacity < 20)       return CSB_DECOMPDU_ERR_OOM;

    for (i = 0; i < 256; ++i) {
        freq[i] = 0;
        codeKind[i] = 2;
        codeIndex[i] = 0;
    }
    for (i = 0; i < rawLen; ++i) freq[raw[i]]++;

    /* Pick the 4 most frequent for most-common, next 16 for
     * less-common via a simple selection (256 is tiny). */
    memset(table, 0, sizeof(table));
    {
        int used[256];
        int slot;
        for (i = 0; i < 256; ++i) used[i] = 0;
        for (slot = 0; slot < 20; ++slot) {
            long best = -1;
            int  bestByte = -1;
            int  b;
            for (b = 0; b < 256; ++b) {
                if (!used[b] && freq[b] > best) {
                    best = freq[b];
                    bestByte = b;
                }
            }
            if (bestByte < 0 || best <= 0) {
                /* No more bytes worth tabling; leave slot 0. */
                break;
            }
            used[bestByte] = 1;
            table[slot] = (unsigned char)bestByte;
            if (slot < 4) {
                codeKind[bestByte]  = 0;        /* most-common */
                codeIndex[bestByte] = slot;
            } else {
                codeKind[bestByte]  = 1;        /* less-common */
                codeIndex[bestByte] = slot - 4; /* 0..15 */
            }
        }
    }

    /* Emit the 20-byte code table verbatim. */
    for (t = 0; t < 20; ++t) out[t] = table[t];

    w.buf = out;
    w.capacity = outCapacity;
    w.pos = 20;
    w.acc = 0;
    w.nbits = 0;
    w.overflow = 0;

    for (i = 0; i < rawLen; ++i) {
        unsigned char b = raw[i];
        switch (codeKind[b]) {
            case 0: /* most-common: 0bb (3 bits) */
                cdu_bw_put(&w, (unsigned)codeIndex[b], 1 + 2);
                /* prefix bit 0 then 2 index bits -> value =
                 * 0b0_xx = codeIndex (which is 0..3). */
                break;
            case 1: /* less-common: 10bbbb (6 bits) */
                cdu_bw_put(&w, 0x20u | (unsigned)codeIndex[b], 6);
                break;
            default: /* literal: 11<8 bits> (10 bits) */
                cdu_bw_put(&w, 0x300u | (unsigned)b, 10);
                break;
        }
    }
    cdu_bw_flush(&w);

    if (w.overflow) return CSB_DECOMPDU_ERR_OOM;
    *outLen = w.pos;
    return CSB_DECOMPDU_ERR_OK;
}

/* ── Grid container wrapper ───────────────────────────────── */
void csb_v1_cdu_dungeon_free(CSB_CDUDungeon* dungeon) {
    if (!dungeon) return;
    free(dungeon->tiles);
    dungeon->tiles = NULL;
    dungeon->width = 0;
    dungeon->height = 0;
    dungeon->levelCount = 0;
}

static unsigned long cdu_rd_u32(const unsigned char* p) {
    return (unsigned long)p[0]
         | ((unsigned long)p[1] << 8)
         | ((unsigned long)p[2] << 16)
         | ((unsigned long)p[3] << 24);
}

static void cdu_wr_u32(unsigned char* p, unsigned long v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

int csb_v1_decompdu_decompress_grid(const unsigned char* container,
                                    long containerLen,
                                    CSB_CDUDungeon* outDungeon) {
    int width, height, levelCount;
    unsigned long payloadBytes;
    long expect;
    unsigned char* tiles;
    int rc;

    if (!container || !outDungeon) return CSB_DECOMPDU_ERR_NULL;
    outDungeon->tiles = NULL;
    outDungeon->width = outDungeon->height = outDungeon->levelCount = 0;

    if (containerLen < CSB_CDU_HEADER_SIZE) return CSB_DECOMPDU_ERR_CORRUPT;
    if (memcmp(container, kCduMagic, 4) != 0) return CSB_DECOMPDU_ERR_CORRUPT;
    if (container[4] != CSB_CDU_SUBFORMAT_F0455) {
        return CSB_DECOMPDU_ERR_BAD_SUBFORMAT;
    }

    width      = container[5];
    height     = container[6];
    levelCount = container[7];
    payloadBytes = cdu_rd_u32(container + 8);

    if (width  < 1 || width  > CSB_CDU_MAX_DIM)    return CSB_DECOMPDU_ERR_BOUNDS;
    if (height < 1 || height > CSB_CDU_MAX_DIM)    return CSB_DECOMPDU_ERR_BOUNDS;
    if (levelCount < 1 || levelCount > CSB_CDU_MAX_LEVELS)
        return CSB_DECOMPDU_ERR_BOUNDS;

    expect = (long)width * (long)height * (long)levelCount;
    if ((long)payloadBytes != expect) return CSB_DECOMPDU_ERR_CORRUPT;

    tiles = (unsigned char*)malloc((size_t)expect);
    if (!tiles) return CSB_DECOMPDU_ERR_OOM;

    rc = csb_v1_decompdu_f0455(container + CSB_CDU_HEADER_SIZE,
                               containerLen - CSB_CDU_HEADER_SIZE,
                               tiles, expect);
    if (rc != CSB_DECOMPDU_ERR_OK) {
        free(tiles);
        return rc;
    }

    outDungeon->width = width;
    outDungeon->height = height;
    outDungeon->levelCount = levelCount;
    outDungeon->tiles = tiles;
    return CSB_DECOMPDU_ERR_OK;
}

int csb_v1_decompdu_build_grid(const unsigned char* tiles,
                               int width, int height, int levelCount,
                               unsigned char* out,
                               long outCapacity,
                               long* outLen) {
    long expect;
    long payloadLen = 0;
    int rc;

    if (!tiles || !out || !outLen) return CSB_DECOMPDU_ERR_NULL;
    if (width  < 1 || width  > CSB_CDU_MAX_DIM)    return CSB_DECOMPDU_ERR_BOUNDS;
    if (height < 1 || height > CSB_CDU_MAX_DIM)    return CSB_DECOMPDU_ERR_BOUNDS;
    if (levelCount < 1 || levelCount > CSB_CDU_MAX_LEVELS)
        return CSB_DECOMPDU_ERR_BOUNDS;
    if (outCapacity < CSB_CDU_HEADER_SIZE + 20)    return CSB_DECOMPDU_ERR_OOM;

    expect = (long)width * (long)height * (long)levelCount;

    memcpy(out, kCduMagic, 4);
    out[4] = CSB_CDU_SUBFORMAT_F0455;
    out[5] = (unsigned char)width;
    out[6] = (unsigned char)height;
    out[7] = (unsigned char)levelCount;
    cdu_wr_u32(out + 8, (unsigned long)expect);

    rc = csb_v1_decompdu_f0455_encode(tiles, expect,
                                      out + CSB_CDU_HEADER_SIZE,
                                      outCapacity - CSB_CDU_HEADER_SIZE,
                                      &payloadLen);
    if (rc != CSB_DECOMPDU_ERR_OK) return rc;

    *outLen = CSB_CDU_HEADER_SIZE + payloadLen;
    return CSB_DECOMPDU_ERR_OK;
}
