/*
 * song_dat_loader_v1.c — Pass 50.
 *
 * See song_dat_loader_v1.h and DM1_SONG_DAT_FORMAT.md for the
 * source-backed specification.
 */

#include "song_dat_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── small helpers ───────────────────────────────────────────────── */

static void v1_set_err(char* dst, size_t cap, const char* msg) {
    if (!dst || cap == 0) return;
    size_t n = strlen(msg);
    if (n >= cap) n = cap - 1;
    memcpy(dst, msg, n);
    dst[n] = '\0';
}

static unsigned short v1_le16(const unsigned char* p) {
    return (unsigned short)(p[0] | (p[1] << 8));
}

static unsigned short v1_be16(const unsigned char* p) {
    return (unsigned short)((p[0] << 8) | p[1]);
}

static long v1_file_size(FILE* f) {
    long cur = ftell(f);
    if (cur < 0) return -1;
    if (fseek(f, 0, SEEK_END) != 0) return -1;
    long end = ftell(f);
    if (fseek(f, cur, SEEK_SET) != 0) return -1;
    return end;
}

/* ── manifest ────────────────────────────────────────────────────── */

int V1_Song_ParseManifest(const char* songDatPath,
                          V1_SongManifest* outManifest,
                          char* errMsg,
                          size_t errMsgBytes) {
    if (!songDatPath || !outManifest) {
        v1_set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outManifest, 0, sizeof(*outManifest));

    FILE* f = fopen(songDatPath, "rb");
    if (!f) {
        v1_set_err(errMsg, errMsgBytes, "cannot open SONG.DAT");
        return 0;
    }

    long fileBytes = v1_file_size(f);
    if (fileBytes < 84) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "file too small to be SONG.DAT");
        return 0;
    }
    outManifest->fileBytes = (unsigned long)fileBytes;

    /* Header size for DM PC v3.4 SONG.DAT:
       2 (sig) + 2 (count) + 2*N (comp) + 2*N (decomp) + 4*N (attrs). */
    const unsigned int N = V1_SONG_DAT_ITEM_COUNT;
    unsigned int headerBytes = 4 + 2u * N + 2u * N + 4u * N; /* 84 for N=10 */
    outManifest->headerBytes = headerBytes;

    unsigned char hdr[84];
    if (headerBytes != sizeof(hdr)) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "unexpected N != 10");
        return 0;
    }
    if (fread(hdr, 1, headerBytes, f) != headerBytes) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "header read failed");
        return 0;
    }

    /* Signature.  dmweb says 0x8001 "in big endian", but the real PC
       v3.4 file has 01 80 on disk — i.e. 0x8001 when read LE.
       Accept either interpretation. */
    unsigned short sigLE = v1_le16(hdr + 0);
    unsigned short sigBE = v1_be16(hdr + 0);
    if (sigLE != V1_SONG_DAT_SIGNATURE && sigBE != V1_SONG_DAT_SIGNATURE) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "bad DMCSB2 signature (not 0x8001)");
        return 0;
    }
    outManifest->signature = V1_SONG_DAT_SIGNATURE;

    unsigned short itemCount = v1_le16(hdr + 2);
    if (itemCount != V1_SONG_DAT_ITEM_COUNT) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "unexpected item count (not 10)");
        return 0;
    }
    outManifest->itemCount = itemCount;

    const unsigned char* pComp   = hdr + 4;
    const unsigned char* pDecomp = pComp   + 2u * N;
    const unsigned char* pAttrs  = pDecomp + 2u * N;

    unsigned long cur = headerBytes;
    for (unsigned int i = 0; i < N; ++i) {
        V1_SongItemHeader* it = &outManifest->items[i];
        it->index              = i;
        it->fileOffset         = cur;
        it->compressedBytes    = v1_le16(pComp   + 2u * i);
        it->decompressedBytes  = v1_le16(pDecomp + 2u * i);
        it->attr0              = v1_le16(pAttrs  + 4u * i + 0);
        it->attr1              = v1_le16(pAttrs  + 4u * i + 2);
        it->type = (i == 0) ? V1_SONG_ITEM_SEQ2 : V1_SONG_ITEM_SND8;
        cur += it->compressedBytes;
    }

    if (cur != outManifest->fileBytes) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes,
                   "item offset sum does not match file size");
        return 0;
    }

    fclose(f);
    return 1;
}

/* ── SEQ2 ────────────────────────────────────────────────────────── */

int V1_Song_DecodeSequence(const char* songDatPath,
                           const V1_SongManifest* manifest,
                           V1_SongSequence* outSequence,
                           char* errMsg,
                           size_t errMsgBytes) {
    if (!songDatPath || !manifest || !outSequence) {
        v1_set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outSequence, 0, sizeof(*outSequence));

    const V1_SongItemHeader* seq = &manifest->items[V1_SONG_DAT_SEQ2_INDEX];
    if (seq->type != V1_SONG_ITEM_SEQ2) {
        v1_set_err(errMsg, errMsgBytes, "item 0 is not SEQ2");
        return 0;
    }
    if (seq->compressedBytes == 0 || (seq->compressedBytes & 1u)) {
        v1_set_err(errMsg, errMsgBytes, "SEQ2 size not a multiple of 2");
        return 0;
    }

    FILE* f = fopen(songDatPath, "rb");
    if (!f) {
        v1_set_err(errMsg, errMsgBytes, "cannot open SONG.DAT");
        return 0;
    }
    if (fseek(f, (long)seq->fileOffset, SEEK_SET) != 0) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "seek to SEQ2 failed");
        return 0;
    }

    unsigned int wordCount = seq->compressedBytes / 2u;
    unsigned int cap = (unsigned int)(sizeof(outSequence->words) /
                                      sizeof(outSequence->words[0]));
    if (wordCount > cap) wordCount = cap;

    for (unsigned int i = 0; i < wordCount; ++i) {
        unsigned char b[2];
        if (fread(b, 1, 2, f) != 2) {
            fclose(f);
            v1_set_err(errMsg, errMsgBytes, "SEQ2 read failed");
            return 0;
        }
        unsigned short w = v1_le16(b);
        outSequence->words[i] = w;
        outSequence->wordCount = i + 1;
        if (w & 0x8000u) {
            outSequence->hasEndMarker = 1;
            break;
        }
    }

    fclose(f);
    return 1;
}

/* ── SND8 DPCM decode ────────────────────────────────────────────── */

/* Clamp to signed 8-bit range. */
static signed char v1_clamp_s8(int v) {
    if (v >  127) return  127;
    if (v < -128) return -128;
    return (signed char)v;
}

int V1_Song_DecodeSnd8(const char* songDatPath,
                       const V1_SongManifest* manifest,
                       unsigned int itemIndex,
                       V1_SndBuffer* outBuffer,
                       char* errMsg,
                       size_t errMsgBytes) {
    if (!songDatPath || !manifest || !outBuffer) {
        v1_set_err(errMsg, errMsgBytes, "null argument");
        return 0;
    }
    memset(outBuffer, 0, sizeof(*outBuffer));

    if (itemIndex < V1_SONG_DAT_FIRST_SND8_INDEX ||
        itemIndex > V1_SONG_DAT_LAST_SND8_INDEX) {
        v1_set_err(errMsg, errMsgBytes, "itemIndex out of range for SND8");
        return 0;
    }
    const V1_SongItemHeader* it = &manifest->items[itemIndex];
    if (it->type != V1_SONG_ITEM_SND8) {
        v1_set_err(errMsg, errMsgBytes, "requested item is not SND8");
        return 0;
    }
    if (it->compressedBytes < 2) {
        v1_set_err(errMsg, errMsgBytes, "SND8 item too small");
        return 0;
    }

    FILE* f = fopen(songDatPath, "rb");
    if (!f) {
        v1_set_err(errMsg, errMsgBytes, "cannot open SONG.DAT");
        return 0;
    }
    if (fseek(f, (long)it->fileOffset, SEEK_SET) != 0) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "seek to SND8 failed");
        return 0;
    }

    unsigned char* raw = (unsigned char*)malloc(it->compressedBytes);
    if (!raw) {
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "out of memory (raw item)");
        return 0;
    }
    if (fread(raw, 1, it->compressedBytes, f) != it->compressedBytes) {
        free(raw);
        fclose(f);
        v1_set_err(errMsg, errMsgBytes, "SND8 body read failed");
        return 0;
    }
    fclose(f);

    /* SND8 header: big-endian u16 sample count. */
    unsigned int declared = v1_be16(raw);
    outBuffer->itemIndex = itemIndex;
    outBuffer->declaredSampleCount = declared;
    outBuffer->sampleRateHz = V1_SONG_DAT_SAMPLE_RATE_HZ;

    if (declared == 0) {
        outBuffer->samples = NULL;
        outBuffer->decodedSampleCount = 0;
        free(raw);
        return 1;
    }

    outBuffer->samples = (signed char*)malloc(declared);
    if (!outBuffer->samples) {
        free(raw);
        v1_set_err(errMsg, errMsgBytes, "out of memory (sample buffer)");
        return 0;
    }

    /*
     * DPCM nibble stream starts after the 2-byte header.
     * Each byte packs two nibbles, high nibble first.
     * Special value -8 (raw nibble 0x8) means the diff is the next
     * two nibbles interpreted as a signed byte.
     */
    const unsigned char* nibs = raw + 2;
    unsigned int nibByteCount = it->compressedBytes - 2u;
    unsigned int totalNibbles = nibByteCount * 2u;

    int prev = 0;
    unsigned int out = 0;
    unsigned int ni = 0;
    while (ni < totalNibbles && out < declared) {
        unsigned char nb = nibs[ni >> 1];
        int n1 = (ni & 1u) ? (nb & 0xF) : ((nb >> 4) & 0xF);
        ni++;
        if (n1 > 7) n1 -= 16; /* sign-extend to -8..7 */

        int diff;
        if (n1 != -8) {
            diff = n1;
        } else {
            if (ni + 1 >= totalNibbles) break;
            unsigned char nb2 = nibs[ni >> 1];
            int n2 = (ni & 1u) ? (nb2 & 0xF) : ((nb2 >> 4) & 0xF);
            ni++;
            unsigned char nb3 = nibs[ni >> 1];
            int n3 = (ni & 1u) ? (nb3 & 0xF) : ((nb3 >> 4) & 0xF);
            ni++;
            diff = (n2 << 4) | n3;
            if (diff > 127) diff -= 256;
        }

        prev += diff;
        outBuffer->samples[out++] = v1_clamp_s8(prev);
    }
    outBuffer->decodedSampleCount = out;

    free(raw);
    return 1;
}

void V1_Song_FreeSndBuffer(V1_SndBuffer* buffer) {
    if (!buffer) return;
    if (buffer->samples) {
        free(buffer->samples);
        buffer->samples = NULL;
    }
    buffer->declaredSampleCount = 0;
    buffer->decodedSampleCount = 0;
}
