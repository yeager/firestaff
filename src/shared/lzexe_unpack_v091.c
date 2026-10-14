#include "lzexe_unpack_v091.h"

#include <stdlib.h>
#include <string.h>

typedef struct LZEXE_BitStream091 {
    const unsigned char* data;
    unsigned int dataBytes;
    unsigned int pos;
    unsigned int buf;
    unsigned int count;
} LZEXE_BitStream091;

static unsigned int lzexe_le16(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static int lzexe_bit(LZEXE_BitStream091* s, int* outBit) {
    if (!s || !outBit) return 0;
    *outBit = (int)(s->buf & 1u);
    if (--s->count == 0u) {
        if (s->pos + 1u >= s->dataBytes) return 0;
        s->buf = lzexe_le16(s->data + s->pos);
        s->pos += 2u;
        s->count = 16u;
    } else {
        s->buf >>= 1;
    }
    return 1;
}

static int lzexe_grow(unsigned char** bytes,
                      unsigned int* capacity,
                      unsigned int needed) {
    unsigned int grownCapacity;
    unsigned char* grown;
    if (needed <= *capacity) return 1;
    grownCapacity = *capacity ? *capacity : 4096u;
    while (grownCapacity < needed) {
        if (grownCapacity > 1024u * 1024u) return 0;
        grownCapacity *= 2u;
    }
    grown = (unsigned char*)realloc(*bytes, grownCapacity);
    if (!grown) return 0;
    *bytes = grown;
    *capacity = grownCapacity;
    return 1;
}

int V1_LZEXE_UnpackLoadModule091(const unsigned char* data,
                                 unsigned int dataBytes,
                                 unsigned char** outBytes,
                                 unsigned int* outByteCount) {
    unsigned int h[16];
    unsigned int inf[8];
    unsigned int i;
    unsigned int infOffset;
    unsigned int compOffset;
    unsigned char* out = 0;
    unsigned int outCount = 0u;
    unsigned int outCapacity = 0u;
    LZEXE_BitStream091 bits;

    if (outBytes) *outBytes = 0;
    if (outByteCount) *outByteCount = 0u;
    if (!data || dataBytes < 64u || !outBytes || !outByteCount) return 0;
    if (!((data[0] == 'M' && data[1] == 'Z') ||
          (data[0] == 'Z' && data[1] == 'M'))) return 0;
    for (i = 0u; i < 16u; ++i) h[i] = lzexe_le16(data + i * 2u);
    if (h[12] != 0x1cu || h[13] != 0u || h[12] + 4u > dataBytes ||
        memcmp(data + h[12], "LZ91", 4u) != 0) return 0;
    infOffset = (h[11] + h[4]) << 4;
    if (infOffset + 16u > dataBytes) return 0;
    for (i = 0u; i < 8u; ++i) inf[i] = lzexe_le16(data + infOffset + i * 2u);
    if (h[11] < inf[4]) return 0;
    compOffset = (h[11] - inf[4] + h[4]) << 4;
    if (compOffset + 2u > dataBytes) return 0;
    memset(&bits, 0, sizeof(bits));
    bits.data = data;
    bits.dataBytes = dataBytes;
    bits.pos = compOffset + 2u;
    bits.buf = lzexe_le16(data + compOffset);
    bits.count = 16u;
    for (;;) {
        int bit0;
        int bit1;
        int len;
        int span;
        if (!lzexe_bit(&bits, &bit0)) goto fail;
        if (bit0) {
            if (bits.pos >= dataBytes || outCount >= 1024u * 1024u ||
                !lzexe_grow(&out, &outCapacity, outCount + 1u)) goto fail;
            out[outCount++] = data[bits.pos++];
            continue;
        }
        if (!lzexe_bit(&bits, &bit1)) goto fail;
        if (!bit1) {
            int b2;
            int b3;
            if (!lzexe_bit(&bits, &b2) || !lzexe_bit(&bits, &b3)) goto fail;
            len = ((b2 << 1) | b3) + 2;
            if (bits.pos >= dataBytes) goto fail;
            span = (int)data[bits.pos++] | -0x100;
        } else {
            unsigned int spanByte;
            unsigned int lenByte;
            if (bits.pos + 1u >= dataBytes) goto fail;
            spanByte = data[bits.pos++];
            lenByte = data[bits.pos++];
            span = (int)(spanByte | ((lenByte & ~7u) << 5)) | -0x2000;
            len = (int)(lenByte & 7u) + 2;
            if (len == 2) {
                if (bits.pos >= dataBytes) goto fail;
                len = data[bits.pos++];
                if (len == 0) break;
                if (len == 1) continue;
                len++;
            }
        }
        if (len < 0 || outCount > 1024u * 1024u - (unsigned int)len ||
            (int)outCount + span < 0 ||
            !lzexe_grow(&out, &outCapacity, outCount + (unsigned int)len)) goto fail;
        for (i = 0u; i < (unsigned int)len; ++i) {
            out[outCount] = out[(unsigned int)((int)outCount + span)];
            outCount++;
        }
    }
    *outBytes = out;
    *outByteCount = outCount;
    return 1;
fail:
    free(out);
    return 0;
}
