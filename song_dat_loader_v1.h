#ifndef FIRESTAFF_SONG_DAT_LOADER_V1_H
#define FIRESTAFF_SONG_DAT_LOADER_V1_H

/*
 * song_dat_loader_v1 — source-backed loader for SONG.DAT DM PC v3.4.
 *
 * Pass 50 (V1 original-faithful audio, step 1).
 *
 * Parses the DMCSB2 container, enumerates the 10 items, decodes:
 *   - SEQ2  (item 0)      : music sequence (LE u16 words, bit 15 = end)
 *   - SND8  (items 1..9)  : DPCM → signed-8-bit PCM at 11025 Hz
 *
 * No SDL/audio dependency.  Pure C.  Caller provides the SONG.DAT
 * path at runtime; the file itself is never vendored.
 *
 * Format references:
 *   - dmweb.free.fr data-files documentation
 *   - DM1_SONG_DAT_FORMAT.md (source-backed spec tracked in repo)
 *
 * M10 is not touched by this module.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM PC v3.4 invariants. */
#define V1_SONG_DAT_SIGNATURE         0x8001u
#define V1_SONG_DAT_ITEM_COUNT        10
#define V1_SONG_DAT_SEQ2_INDEX        0
#define V1_SONG_DAT_FIRST_SND8_INDEX  1
#define V1_SONG_DAT_LAST_SND8_INDEX   9
#define V1_SONG_DAT_SAMPLE_RATE_HZ    11025
#define V1_SONG_DAT_EXPECTED_SIZE     162482u

/* Item types used in SONG.DAT. */
typedef enum {
    V1_SONG_ITEM_NONE = 0,
    V1_SONG_ITEM_SEQ2,
    V1_SONG_ITEM_SND8
} V1_SongItemType;

typedef struct {
    unsigned int    index;
    unsigned long   fileOffset;
    unsigned int    compressedBytes;    /* bytes on disk for this item */
    unsigned int    decompressedBytes;  /* per DMCSB2 header */
    unsigned short  attr0;
    unsigned short  attr1;
    V1_SongItemType type;
} V1_SongItemHeader;

typedef struct {
    /* header section */
    unsigned short    signature;        /* expected V1_SONG_DAT_SIGNATURE */
    unsigned short    itemCount;        /* expected V1_SONG_DAT_ITEM_COUNT */
    unsigned long     headerBytes;      /* end-of-header offset */
    unsigned long     fileBytes;        /* size of file on disk */
    V1_SongItemHeader items[V1_SONG_DAT_ITEM_COUNT];
} V1_SongManifest;

typedef struct {
    /* Music part indices (1..9) as parsed from SEQ2. */
    unsigned short words[256];
    unsigned int   wordCount;
    /* Non-zero iff the last word had bit 15 set (well-formed end marker). */
    int            hasEndMarker;
} V1_SongSequence;

typedef struct {
    unsigned int   itemIndex;           /* 1..9 */
    unsigned int   declaredSampleCount; /* from SND8 big-endian word */
    unsigned int   decodedSampleCount;  /* filled in by the decoder */
    signed char*   samples;             /* malloc'd; caller frees via V1_Song_FreeSndBuffer */
    unsigned int   sampleRateHz;        /* always 11025 */
} V1_SndBuffer;

/* Parse only the header section and fill manifest.  Does not touch
   item bodies.  Returns 1 on success, 0 on failure (details via
   errMsg if non-NULL / errMsgBytes >= 1). */
int V1_Song_ParseManifest(const char* songDatPath,
                          V1_SongManifest* outManifest,
                          char* errMsg,
                          size_t errMsgBytes);

/* Decode the SEQ2 item (item 0) into a music-part sequence. */
int V1_Song_DecodeSequence(const char* songDatPath,
                           const V1_SongManifest* manifest,
                           V1_SongSequence* outSequence,
                           char* errMsg,
                           size_t errMsgBytes);

/* Decode a single SND8 item (1..9) into signed-8-bit mono PCM at
   11025 Hz.  On success fills *outBuffer (caller must free with
   V1_Song_FreeSndBuffer). */
int V1_Song_DecodeSnd8(const char* songDatPath,
                       const V1_SongManifest* manifest,
                       unsigned int itemIndex,
                       V1_SndBuffer* outBuffer,
                       char* errMsg,
                       size_t errMsgBytes);

void V1_Song_FreeSndBuffer(V1_SndBuffer* buffer);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SONG_DAT_LOADER_V1_H */
